// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file    EBGeometry_PointCloudBVH.hpp
 * @brief   A point-cloud BVH built on PackedBVH: a linear BVH built where its pool is, and high-level
 *          nearest-neighbor / closest-point queries.
 * @author  Robert Marskar
 */

#ifndef EBGEOMETRY_POINTCLOUDBVH_HPP
#define EBGEOMETRY_POINTCLOUDBVH_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_CloudBVH.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_PODVector.hpp"
#include "EBGeometry_PackedBVH.hpp"
#include "EBGeometry_PointAoSoA.hpp"
#include "EBGeometry_PointCloud.hpp"
#include "EBGeometry_PointCloudDetail.hpp"
#include "EBGeometry_PointSoA.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief A BVH over a point cloud, with a fast build and turnkey queries.
 * @details Holds a PackedBVH over SoA point groups (rather than deriving from one) together with
 * the cloud data the queries need. Unlike the general PackedBVH builds (which take a list of
 * pre-made primitives and partition it via SAH/Midpoint/SFC), PointCloudBVH is built directly from a
 * raw point cloud, as a linear BVH: the points sorted along a Morton curve, and every node's range of
 * them cut where their codes first differ. build() runs it where the pool's memory is, on a device
 * included, and every side gives the same tree, array for array.
 *
 * The build, step by step (EBGeometry_CloudBVHImplem.hpp, shared with SphereCloudBVH):
 *
 * 1. The cloud's box, every coordinate checked finite.
 * 2. Each point's key: its 63-bit Morton code over the box, its longest side scaled to the code's span,
 *    and its cloud index, which breaks ties. The keys are sorted (std::sort on the host, a bitonic sort
 *    through GPU::parallelFor()); the keys are distinct, so both give one order.
 * 3. The tree is grown one level at a time, breadth first. A node holds a range of the sorted points;
 *    one of at most max(a_targetLeafSize, K) points is a leaf, and any other is cut into K ranges, its
 *    children. The K ranges come from K - 1 binary cuts, made as a midpoint build makes them (the
 *    first cut gives K/2 of the parts to the left): a range is cut at the first point whose code has
 *    the highest bit where the range's first and last codes differ set, its points so far left of the
 *    Morton curve's split plane, or at its middle when every code is equal. Each cut keeps at least
 *    floor(n / (4K)) points (and one) per part on either side, n the node's size: a cut further out is
 *    moved in to that quota. The quota bounds the depth (a child holds at most about (3K + 1) / (4K)
 *    of its parent) and the node count (every leaf below an interior node holds at least
 *    floor((max(a_targetLeafSize, K) + 1) / (4K)) points), which is what lets build() reserve room
 *    for a rebuild of any cloud of a given size. Each level is one launch: a node's children go at
 *    the next level's offset plus K times its rank among the level's interior nodes (an exclusive
 *    scan).
 * 4. Each node's subtree size, the levels from the deepest up, with each node's box; then each node's
 *    depth-first index, from the root down. The node array is in depth-first order, and its leaves
 *    are in the sorted points' order.
 * 5. The child-box rows (one per interior node, in depth-first order), and each leaf's groups of W
 *    points (in depth-first order), placed by two exclusive scans over the nodes in depth-first order.
 * 6. Every node written by its own thread: its children's depth-first indices, its row, its children's
 *    boxes into that row, its box slot; a leaf also packs its groups and writes, for each of its
 *    points, the position (by cloud index), the leaf tables and the point's place in the leaf order.
 *
 * A cut that needs no median or sort of its range is what makes the tree the same on every side, and
 * cheap to build on a device. The alternatives: a top-down longest-axis midpoint build, the reference
 * below, partitions each range around a coordinate and has no cheap device form; spreading the Morton-sorted points evenly over
 * a perfect K-ary tree queried 1.2-8.6x slower than it (500,000 points, host and an RTX 3080); a
 * prototype of these radix cuts with a quarter-share quota per binary cut queried within about 10% of
 * it (faster on a uniform cube, slower on a sphere's surface). A half-share quota cost about 10% more
 * on the sphere.
 *
 * Every leaf lane carries its point's **cloud index** (its position in the input positions array),
 * and queries return that index in a PointCloud::Hit. Per-point user data lives in the caller's own
 * array, indexed by it. PointCloudHashGrid answers the same queries with the same Hit type. The
 * class hides all of traverse()/the SoA leaf kernel/the seed-from-own-leaf optimization behind
 * a few high-level query methods.
 *
 * @note Queries come in two flavours. *External* queries (closestPoint / closestPoints) take an
 * arbitrary point and traverse top-down. *Self* queries (nearestNeighbor / nearestNeighbors), and
 * the batch allNearestNeighbors(), take a point already in the cloud and additionally **seed the
 * search bound from the leaf that point lives in** (skipping that leaf during traversal) -- a
 * strictly cheaper search that an external point cannot use.
 *
 * @note Device portability. The held PackedBVH and every cloud array (positions, the seeding
 * tables, the leaf order) are reserved from the same Pool, and the class stores only offsets into
 * it, so a PointCloudBVH is trivially copyable. Mirror the pool, call rebasedView(), and pass the
 * returned value into a kernel; every query that answers through a caller-supplied buffer
 * (closestPoint(s), nearestNeighbor(s)) is device-callable. allNearestNeighbors() returns a
 * std::vector and stays host-only.
 *
 * @tparam T Floating-point precision.
 * @tparam K BVH branching factor. Defaults to BVH::DefaultBranchingRatio<T>().
 * @tparam W Points per SoA leaf lane group. Defaults to PointSoA::DefaultWidth<T>().
 */
template <class T, size_t K = BVH::DefaultBranchingRatio<T>(), size_t W = PointSoA::DefaultWidth<T>()>
class PointCloudBVH : public PointCloudDetail::CloudBVH<T, K, W, PointCloudDetail::PointPrimitive<T, W>>
{
public:
  /**
   * @brief The SoA leaf primitive: a group of up to W points carrying their cloud indices.
   */
  using PointGroup = PointAoSoA<T, W>;

  /**
   * @brief The packed BVH this class holds over its point groups.
   */
  using Packed = BVH::PackedBVH<T, PointGroup, K>;

  /**
   * @brief Flat node type of the held BVH.
   */
  using Node = typename Packed::Node;

  /**
   * @brief Axis-aligned bounding box type.
   */
  using AABB = BoundingVolumes::AABBT<T>;

  /**
   * @brief One query result: the cloud index of a matched point and its squared distance.
   * @details Shared with PointCloudHashGrid. A miss, and every result slot a multi-result query could
   * not fill, has Hit::valid() false; those queries also return the count found.
   */
  using Hit = PointCloud::Hit<T>;

  /**
   * @brief Build a BVH over a point cloud on the host: shorthand for build(a_pool, a_positions, a_targetLeafSize).
   * @details A point's cloud index, which the queries report, is its position in @p a_positions.
   * @param[in,out] a_pool       Pool the packed BVH and the cloud arrays are reserved from; must outlive
   *                             this object.
   * @param[in] a_positions      Point positions. At most 2^31, all finite.
   * @param[in] a_targetLeafSize Target points per leaf (a node of at most max(a_targetLeafSize, K) points
   *                             is a leaf). Must be at least 1.
   */
  EBGEOMETRY_HOST
  inline PointCloudBVH(Pool& a_pool, const std::vector<Vec3T<T>>& a_positions, std::size_t a_targetLeafSize = 16 * W);

  /**
   * @brief Build a BVH over a point cloud, where the pool's memory is.
   * @details A point's cloud index, which the queries report, is the index the source is called with.
   * The build (see the class) runs where the pool's memory is. In device-accessible memory (device or
   * managed memory in a CUDA or HIP build; emulated device memory in a host build) it runs through
   * GPU::parallelFor(): kernels in a CUDA or HIP translation unit, which read @p a_source on the
   * device, and host loops otherwise. In host memory it runs on the host, which reads @p a_source
   * there. Both give the same tree, array for array. The build reads each point from the source four
   * times (for the box, its key, its leaf's box and its group), so an expensive source pays that. A
   * point with a non-finite coordinate is rejected: on the host by its index, through
   * GPU::parallelFor() only as "a point". A tree deeper than the traversal stack allows (BVH::HostTraversalDepth
   * on the host, BVH::DeviceTraversalDepth through GPU::parallelFor()) aborts the build, as every
   * EBGEOMETRY_REQUIRE does; only a cloud of many points packed far closer than its extent comes near
   * either. A larger a_targetLeafSize makes the tree shallower, and a host build allows the deeper tree.
   * The quota of each cut (see the class) only bounds a child at about (3K + 1) / (4K) of its parent,
   * which does not keep every cloud of 2^31 points within 32 levels; Morton cuts of real clouds are
   * close to halves. Freeze the pool and take
   * rebasedView() to query a BVH in device memory in a kernel.
   *
   * The scratch arrays (the keys, the box, the build's nodes and two scans' values) are reserved from
   * a second pool on the same memory resource and released on return, and the BVH's arrays at the
   * sizes this cloud needs, unless @p a_maxPoints is given: then the arrays are reserved at the
   * largest sizes any cloud of that many points can need (more than a uniform cloud uses), and
   * the scratch in the BVH's pool too, so that rebuild() reserves nothing and works on a frozen pool.
   * @tparam Source A source of positions: a trivially copyable type whose const call operator takes a
   * cloud index (uint32_t) and returns that point's Vec3T<T>; see PointCloud::FromArray.
   * @param[in,out] a_pool           Pool the BVH's arrays are reserved from; must outlive the BVH.
   * @param[in]     a_source         The positions.
   * @param[in]     a_numPoints      Number of points. At most 2^31, all finite.
   * @param[in]     a_targetLeafSize Target points per leaf (a node of at most max(a_targetLeafSize, K)
   *                                 points is a leaf). Must be at least 1.
   * @param[in]     a_maxPoints      Most points a rebuild() may have; 0 for no rebuild(), and the arrays
   *                                 this cloud needs only. At least a_numPoints otherwise.
   * @return The BVH.
   */
  template <class Source>
  [[nodiscard]] EBGEOMETRY_HOST
  static inline PointCloudBVH
  build(Pool&         a_pool,
        const Source& a_source,
        uint32_t      a_numPoints,
        std::size_t   a_targetLeafSize = 16 * W,
        uint32_t      a_maxPoints      = 0);

  /**
   * @brief Build a BVH over points on the host, where the pool's memory is; see the build() above.
   * @details For a pool in device memory, the positions are first copied to scratch there.
   * @param[in,out] a_pool           Pool the BVH's arrays are reserved from; must outlive the BVH.
   * @param[in]     a_positions      Point positions. At most 2^31, all finite.
   * @param[in]     a_targetLeafSize Target points per leaf. Must be at least 1.
   * @param[in]     a_maxPoints      Most points a rebuild() may have (see the build() above).
   * @return The BVH.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static inline PointCloudBVH
  build(Pool&                        a_pool,
        const std::vector<Vec3T<T>>& a_positions,
        std::size_t                  a_targetLeafSize = 16 * W,
        uint32_t                     a_maxPoints      = 0);

  /**
   * @brief Build the BVH again, in the arrays build() reserved, from new positions.
   * @details The same tree as a new build() from these positions with the same target leaf size,
   * reserving nothing in the pool: it runs where the build did, and works on a frozen pool (the host keeps
   * one small vector of level offsets). Needs room reserved by
   * build()'s a_maxPoints; more points than maxPoints() is rejected, and a non-finite point or a tree
   * too deep aborts, as in build(): a code that rebuilds every step meets this only if its cloud becomes
   * that clustered. A view taken with rebasedView() before the rebuild still describes
   * the old tree (its array sizes): take a new one, which copies nothing. Call it on the BVH build()
   * returned, not on a view or a deepCopy().
   * @tparam Source A source of positions, as for build().
   * @param[in] a_source    The positions.
   * @param[in] a_numPoints Number of points, at most maxPoints(). All finite.
   */
  template <class Source>
  EBGEOMETRY_HOST
  inline void
  rebuild(const Source& a_source, uint32_t a_numPoints);

  /**
   * @brief The most points rebuild() can take.
   * @return build()'s a_maxPoints; 0 for a BVH built without room for a rebuild.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  maxPoints() const noexcept
  {
    return m_maxItems;
  }

  /**
   * @brief Closest cloud point to an arbitrary query point.
   * @param[in] a_query Query point (need not be in the cloud).
   * @return The nearest point and its squared distance.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Hit
  closestPoint(const Vec3T<T>& a_query) const noexcept;

  /**
   * @brief The a_k closest cloud points to an arbitrary query point, nearest first.
   * @param[in]  a_query Query point (need not be in the cloud).
   * @param[in]  a_k     Number of neighbors requested.
   * @param[out] a_out   Buffer of at least a_k Hits; filled [0, returned count), ascending by distance.
   * @return The number of neighbors found (min(a_k, cloud size)).
   */
  EBGEOMETRY_HOST_DEVICE
  inline std::size_t
  closestPoints(const Vec3T<T>& a_query, std::size_t a_k, Hit* a_out) const noexcept;

  /**
   * @brief Nearest *other* point to a point already in the cloud (seed-from-own-leaf search).
   * @param[in] a_point Cloud index of the query point; excluded from its own result.
   * @return The nearest other point and its squared distance.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Hit
  nearestNeighbor(uint32_t a_point) const noexcept;

  /**
   * @brief The a_k nearest *other* points to a point already in the cloud, nearest first.
   * @param[in]  a_point Cloud index of the query point; excluded from its own result.
   * @param[in]  a_k     Number of neighbors requested.
   * @param[out] a_out   Buffer of at least a_k Hits; filled [0, returned count), ascending.
   * @return The number of neighbors found (min(a_k, cloud size - 1)).
   */
  EBGEOMETRY_HOST_DEVICE
  inline std::size_t
  nearestNeighbors(uint32_t a_point, std::size_t a_k, Hit* a_out) const noexcept;

  /**
   * @brief For every point, its a_k nearest *other* points (the k-nearest-neighbor graph).
   * @details Processes points in leaf (build) order -- already spatially coherent, so consecutive
   * queries touch nearby leaves and stay hot in cache, with no per-call sort -- and seeds each from
   * its own leaf, so the whole batch is cheaper than the sum of independent queries. Result is
   * flattened row-major: entry [i*a_k + j] is the j-th nearest neighbor of point i (ascending by
   * distance).
   * @param[in] a_k Number of neighbors per point.
   * @return A vector of size numPoints()*a_k of Hits.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline std::vector<Hit>
  allNearestNeighbors(std::size_t a_k = 1) const;

  /**
   * @brief Number of points in the cloud.
   * @return The point count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline std::size_t
  numPoints() const noexcept
  {
    return m_items.size();
  }

  /**
   * @brief Position of the point with the given cloud index.
   * @param[in] a_index Cloud index.
   * @return The point's position.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const Vec3T<T>&
  position(uint32_t a_index) const noexcept
  {
    EBGEOMETRY_EXPECT(a_index < m_items.size());

    return m_items.at(this->base(), a_index);
  }

  /**
   * @brief The held packed BVH over the point groups.
   * @details For callers that want the general PackedBVH interface (bounding volume, a custom
   * traverse(), ...) on top of the turnkey queries above.
   * @return The packed BVH.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Packed
  getBVH() const noexcept
  {
    return Packed(m_bvhLayout, this->location());
  }

  /**
   * @brief The base address this object's arrays are offsets from; see PoolHandle::base().
   * @return The base address: const void* for a const object, void* otherwise.
   */
  using PoolHandle::base;

  /**
   * @brief Produce a copy of this object that resolves against @p a_pool.
   * @details Same contract as PackedBVH::rebasedView(): mirror the pool first, then rebase, then
   * copy the returned value into a kernel. The BVH and the cloud arrays are offsets into one pool,
   * so rebasing the one location rebases everything. A snapshot for a kernel is rejected for a tree
   * deeper than BVH::DeviceTraversalDepth.
   *
   * @code
   * Pool devicePool = Pool::mirror(hostPool, deviceMemoryResource());
   * const auto deviceCloud = cloud.rebasedView(devicePool);
   * myKernel<<<blocks, threads>>>(deviceCloud, ...);
   * @endcode
   * @param[in] a_pool Pool to rebase onto: the object's own pool or one in its mirror chain.
   * @return A copy of this object resolving against @p a_pool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline PointCloudBVH
  rebasedView(const Pool& a_pool) const noexcept;

  /**
   * @brief Duplicate this object's storage into @p a_dstPool.
   * @details The copy constructor copies descriptors only, leaving both objects resolving against
   * the same pool memory. This is the operation that gives genuinely independent storage. It reads
   * this object's arrays on the host, so their pool must be host-accessible (mirror a pool in device
   * memory to the host first), and @p a_dstPool must be a host pool. The copy holds the arrays at the
   * sizes the tree uses, with no room for a rebuild().
   * @param[in,out] a_dstPool Pool to reserve the copy's arrays from.
   * @return A PointCloudBVH with the same tree and cloud, backed by @p a_dstPool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline PointCloudBVH
  deepCopy(Pool& a_dstPool) const;

private:
  /**
   * @brief The build and its state.
   */
  using Base = PointCloudDetail::CloudBVH<T, K, W, PointCloudDetail::PointPrimitive<T, W>>;

  using Base::m_bvhLayout;
  using Base::m_items;
  using Base::m_leafCnt;
  using Base::m_leafOff;
  using Base::m_maxItems;
  using Base::m_order;

  /**
   * @brief An empty BVH, to be reserved and filled.
   */
  PointCloudBVH() noexcept = default;

  /**
   * @brief The shared query core: fill the a_k nearest to a_query into a_out.
   * @param[in]  a_query   Query point.
   * @param[in]  a_k       Neighbors requested.
   * @param[out] a_out     Buffer of at least a_k Hits.
   * @param[in]  a_exclude Cloud index to exclude (self-queries); PointCloud::InvalidIndex to exclude
   *                       nothing.
   * @param[in]  a_seedOff Group offset of the own leaf to seed from (self-queries; ignored if
   *                       a_seedCnt == 0).
   * @param[in]  a_seedCnt Group count of the own leaf; 0 means "no seed" (external queries).
   * @return The number of neighbors found.
   */
  EBGEOMETRY_HOST_DEVICE
  inline std::size_t
  query(const Vec3T<T>& a_query,
        std::size_t     a_k,
        Hit*            a_out,
        uint32_t        a_exclude,
        std::uint32_t   a_seedOff,
        std::uint32_t   a_seedCnt) const noexcept;
};

/**
 * @brief A PointCloudBVH must be trivially copyable, for the same reason a PackedBVH must: a
 * rebasedView() is byte-copied into a device address space with no pointer patching.
 */
static_assert(std::is_trivially_copyable_v<PointCloudBVH<float>>, "PointCloudBVH<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<PointCloudBVH<double>>, "PointCloudBVH<double> must be trivially copyable");

} // namespace EBGeometry

#include "EBGeometry_PointCloudBVHImplem.hpp"

#endif
