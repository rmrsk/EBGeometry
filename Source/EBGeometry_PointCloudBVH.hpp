// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file    EBGeometry_PointCloudBVH.hpp
 * @brief   A point-cloud BVH built on PackedBVH: fast index-based build and high-level
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
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_PODVector.hpp"
#include "EBGeometry_PackedBVH.hpp"
#include "EBGeometry_PointAoSoA.hpp"
#include "EBGeometry_PointCloud.hpp"
#include "EBGeometry_PointSoA.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief A BVH over a point cloud, with a fast build and turnkey queries.
 * @details Holds a PackedBVH over SoA point groups (rather than deriving from one) together with
 * the cloud data the queries need. Unlike the general PackedBVH constructors (which take a list of pre-made primitives and
 * partition it via SAH/Midpoint/SFC), PointCloudBVH is built directly from a raw point cloud. It
 * uses an index-based, copy-free top-down build (partition an index permutation in place by
 * longest-axis midpoint, pack leaves into PointAoSoA<T, W> groups inline), which is far cheaper than
 * the general path and produces a tree just as tight for near-uniform clouds.
 *
 * Every leaf lane carries its point's **cloud index** (its position in the input positions array),
 * and queries return that index in a PointCloud::Hit. Per-point user data lives in the caller's own
 * array, indexed by it. PointCloudHashGrid answers the same queries with the same Hit type. The
 * class hides all of pruneTraverse()/the SoA leaf kernel/the seed-from-own-leaf optimization behind
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
class PointCloudBVH
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
   * @brief No default construction -- a point cloud is required.
   */
  PointCloudBVH() = delete;

  /**
   * @brief Build a BVH over a point cloud.
   * @details A point's cloud index, which the queries report, is its position in @p a_positions.
   * @param[in,out] a_pool       Pool the packed BVH's arrays are reserved from; must outlive this object.
   * @param[in] a_positions      Point positions. Fewer than PointCloud::InvalidIndex, all finite.
   * @param[in] a_targetLeafSize Target points per leaf (the build stops splitting at or below it).
   */
  EBGEOMETRY_HOST
  inline PointCloudBVH(Pool& a_pool, const std::vector<Vec3T<T>>& a_positions, std::size_t a_targetLeafSize = 16 * W);

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
    return m_positions.size();
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
    EBGEOMETRY_EXPECT(a_index < m_positions.size());

    return m_positions.at(this->base(), a_index);
  }

  /**
   * @brief The held packed BVH over the point groups.
   * @details For callers that want the general PackedBVH interface (bounding volume, a custom
   * pruneTraverse(), ...) on top of the turnkey queries above.
   * @return The packed BVH.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const Packed&
  getBVH() const noexcept
  {
    return m_bvh;
  }

  /**
   * @brief Resolve the base address every array of this object is an offset from.
   * @details The held BVH and the cloud arrays share one pool, so this is the BVH's own base().
   * @return Base address of the pool block holding this object.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const void*
  base() const noexcept
  {
    return m_bvh.base();
  }

  /**
   * @brief Produce a copy of this object that resolves against @p a_pool.
   * @details Same contract as PackedBVH::rebasedView(): mirror the pool first, then rebase, then
   * copy the returned value into a kernel. The cloud arrays are offsets into the same pool as the
   * BVH, so rebasing the held BVH rebases everything.
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
   * the same pool memory. This is the operation that gives genuinely independent storage.
   * @param[in,out] a_dstPool Pool to reserve the copy's arrays from.
   * @return A PointCloudBVH with the same tree and cloud, backed by @p a_dstPool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline PointCloudBVH
  deepCopy(Pool& a_dstPool) const;

  /**
   * @brief Check whether this object's arrays were reserved from @p a_pool.
   * @param[in] a_pool Pool to test against.
   * @return True if this object is attached to @p a_pool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline bool
  isAttachedTo(const Pool& a_pool) const noexcept
  {
    return m_bvh.isAttachedTo(a_pool);
  }

private:
  /**
   * @brief The arrays produced by the index-based build, copied into pool storage by the delegated
   * constructor.
   */
  struct BuildResult
  {
    std::vector<Node>          nodes;      ///< Flat BVH nodes in depth-first layout.
    std::vector<PointGroup>    primitives; ///< Packed SoA leaf groups referenced by the leaf nodes.
    std::vector<std::uint32_t> leafOff;    ///< Per-point own-leaf group offset (for seeding).
    std::vector<std::uint32_t> leafCnt;    ///< Per-point own-leaf group count (for seeding).
    std::vector<std::uint32_t> order;      ///< Point indices in leaf (build) order -- spatially coherent.
  };

  /**
   * @brief Delegated-to constructor: adopt a completed build result and retain the cloud data.
   * @details Hands the BVH arrays of a_build to the held PackedBVH, then copies the point positions
   * and per-point seeding tables into the same pool.
   * @param[in,out] a_pool      Pool every array is reserved from; must outlive this object.
   * @param[in]     a_build     Completed index-based build result.
   * @param[in]     a_positions Point positions (indexed by cloud index).
   */
  EBGEOMETRY_HOST
  inline PointCloudBVH(Pool& a_pool, const BuildResult& a_build, const std::vector<Vec3T<T>>& a_positions);

  /**
   * @brief Reserve the cloud arrays from @p a_pool and copy the given host arrays into them.
   * @details Reserves first and resolves the base only afterwards, since a reserve can grow the pool
   * and move the block. The sources must therefore be host arrays outside @p a_pool.
   * @param[in,out] a_pool      Pool to reserve from; must be the pool the held BVH was built in.
   * @param[in]     a_positions Point positions (indexed by cloud index).
   * @param[in]     a_leafOff   Per-point own-leaf group offset.
   * @param[in]     a_leafCnt   Per-point own-leaf group count.
   * @param[in]     a_order     Point indices in leaf (build) order.
   */
  EBGEOMETRY_HOST
  inline void
  storeCloud(Pool&                             a_pool,
             const std::vector<Vec3T<T>>&      a_positions,
             const std::vector<std::uint32_t>& a_leafOff,
             const std::vector<std::uint32_t>& a_leafCnt,
             const std::vector<std::uint32_t>& a_order);

  /**
   * @brief Copy one pool-resident array back into a host std::vector.
   * @tparam U Element type.
   * @param[in] a_array Array to read.
   * @param[in] a_base  Base address of the pool holding @p a_array.
   * @return A host copy of the array's elements.
   */
  template <class U>
  [[nodiscard]] EBGEOMETRY_HOST
  static inline std::vector<U>
  toHost(const PODVector<U>& a_array, const void* a_base);

  /**
   * @brief Run the index-based, copy-free top-down midpoint build over a point cloud.
   * @details Static so it can run before base construction (its result is forwarded to the delegated
   * constructor above). Partitions an index permutation in place by longest-axis midpoint and packs
   * the resulting leaves into PointGroup SoA groups inline.
   * @param[in] a_positions Point positions to build over.
   * @param[in] a_leafSize  Target points per leaf; the build stops splitting at or below it.
   * @return The completed build result (nodes, packed leaves, and per-point seeding tables).
   */
  EBGEOMETRY_HOST
  static inline BuildResult
  buildTree(const std::vector<Vec3T<T>>& a_positions, std::size_t a_leafSize);

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

  /**
   * @brief The packed BVH over the point groups. Owns the pool attachment every array below shares.
   */
  Packed m_bvh;

  /**
   * @brief Point positions, indexed by cloud index. Kept for self-query points and spatial ordering.
   */
  PODVector<Vec3T<T>> m_positions;

  /**
   * @brief Per-point own-leaf group offset (for seeding).
   */
  PODVector<std::uint32_t> m_leafOff;

  /**
   * @brief Per-point own-leaf group count (for seeding).
   */
  PODVector<std::uint32_t> m_leafCnt;

  /**
   * @brief Point indices in leaf (build) order; the spatially coherent batch-query order, reused so
   * allNearestNeighbors needn't recompute a space-filling-curve sort per call.
   */
  PODVector<std::uint32_t> m_order;
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
