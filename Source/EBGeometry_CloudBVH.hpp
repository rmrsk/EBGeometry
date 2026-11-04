// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file    EBGeometry_CloudBVH.hpp
 * @brief   The linear BVH build shared by PointCloudBVH and SphereCloudBVH, over a cloud of points or spheres.
 * @author  Robert Marskar
 */

#ifndef EBGEOMETRY_CLOUDBVH_HPP
#define EBGEOMETRY_CLOUDBVH_HPP

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
#include "EBGeometry_PointCloudDetail.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

namespace PointCloudDetail {

/**
 * @brief A primitive's sort key in a cloud BVH build: its center's Morton code, ties broken by its cloud index.
 * @details Internal; not part of the public API.
 */
struct MortonKey
{
  /// @brief The Morton code of the primitive center's cell.
  uint64_t m_code;

  /// @brief The primitive's cloud index.
  uint32_t m_index;

  /**
   * @brief Whether this key sorts before @p a_other: by code, then by cloud index.
   * @param[in] a_other The other key.
   * @return True if this key comes first.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  operator<(const MortonKey& a_other) const noexcept
  {
    return m_code < a_other.m_code || (m_code == a_other.m_code && m_index < a_other.m_index);
  }
};

/**
 * @brief One node of a cloud BVH build, in breadth-first order.
 * @details Internal; not part of the public API.
 */
struct BuildNode
{
  /// @brief First of the node's primitives, in sorted order.
  uint32_t m_lo;

  /// @brief One past the node's last primitive, in sorted order.
  uint32_t m_hi;

  /// @brief The parent's breadth-first index (0 for the root).
  uint32_t m_parent;

  /// @brief The node's position among its parent's children.
  uint32_t m_slot;

  /// @brief The first child's breadth-first index (interior nodes).
  uint32_t m_firstChild;

  /// @brief Nodes in the node's subtree, itself included.
  uint32_t m_subtree;

  /// @brief The node's index in the depth-first node array.
  uint32_t m_depthFirst;
};

/**
 * @brief The primitive of PointCloudBVH: a point, packed into PointAoSoA groups.
 * @details Internal; not part of the public API. A cloud BVH's primitive type gives what its source returns
 * (Item), the leaf group it is packed into (Group), the source reading an array of them (FromArray), the class
 * built over it (ClassName), the point its Morton key is taken at (center()), how it grows
 * a box (grow()), the check of what the center's finiteness does not cover (requireValid() on the host, and
 * requireValidWhereBuilt() where the build runs), and the words for the messages.
 * @tparam T Floating-point precision.
 * @tparam W Points per group.
 */
template <class T, size_t W>
struct PointPrimitive
{
  /// @brief What the source returns: a position.
  using Item = Vec3T<T>;

  /// @brief The leaf group.
  using Group = PointAoSoA<T, W>;

  /// @brief The source reading an array of primitives.
  using FromArray = PointCloud::FromArray<T>;

  /// @brief The class built over these primitives, for the messages.
  static constexpr const char* ClassName = "PointCloudBVH";

  /// @brief One primitive, for the messages.
  static constexpr const char* Singular = "point";

  /// @brief Primitives, for the messages.
  static constexpr const char* Plural = "points";

  /// @brief build()'s parameter giving the room for a rebuild, for the messages.
  static constexpr const char* MaxParameter = "a_maxPoints";

  /**
   * @brief The point a primitive's Morton key is taken at.
   * @param[in] a_item The primitive.
   * @return Its position.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline const Vec3T<T>&
  center(const Item& a_item) noexcept
  {
    return a_item;
  }

  /**
   * @brief Grow a box to hold a primitive.
   * @param[in,out] a_lo   The box's lower corner.
   * @param[in,out] a_hi   The box's upper corner.
   * @param[in]     a_item The primitive.
   */
  EBGEOMETRY_HOST_DEVICE
  static inline void
  grow(Vec3T<T>& a_lo, Vec3T<T>& a_hi, const Item& a_item) noexcept
  {
    a_lo = min(a_lo, a_item);
    a_hi = max(a_hi, a_item);
  }

  /**
   * @brief Abort with a message unless the primitives of a host array can be built over.
   * @param[in] a_who   The calling function, for the message.
   * @param[in] a_items The primitives.
   */
  EBGEOMETRY_HOST
  static inline void
  requireValid(const char* a_who, const std::vector<Item>& a_items) noexcept
  {
    requireValidCloud(a_who, a_items);
  }

  /**
   * @brief Check, where the build runs, what the box of the centers does not: nothing for a point.
   * @tparam Source The primitives' source.
   */
  template <class Source>
  EBGEOMETRY_HOST
  static inline void
  requireValidWhereBuilt(const Source&, uint32_t, bool, Vec3T<T>*, const char*) noexcept
  {}
};

/**
 * @brief A source of primitive centers, from a source of primitives.
 * @details Internal; not part of the public API.
 * @tparam Prim   The primitive type.
 * @tparam Source The primitives' source.
 */
template <class Prim, class Source>
struct Centers
{
  /// @brief The primitives.
  Source m_source;

  /**
   * @brief The center of a primitive.
   * @param[in] a_index Cloud index.
   * @return Its center.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  auto
  operator()(const uint32_t a_index) const noexcept
  {
    return Prim::center(m_source(a_index));
  }
};

/**
 * @brief The state and build of a linear BVH over a cloud of primitives (points or spheres), built where its pool
 * is; the base of PointCloudBVH and SphereCloudBVH.
 * @details Internal; not part of the public API. The build is the one PointCloudBVH describes, step by step: the
 * Morton keys and the cuts come from each primitive's center (Prim::center()), the boxes hold the primitives
 * (Prim::grow()), and the leaves are packed into Prim::Group groups of W.
 * @tparam T    Floating-point precision.
 * @tparam K    BVH branching factor.
 * @tparam W    Primitives per leaf group.
 * @tparam Prim The primitive type; see PointPrimitive.
 */
template <class T, size_t K, size_t W, class Prim>
class CloudBVH : public PoolHandle
{
public:
  /**
   * @brief What a source of the primitives returns.
   */
  using Item = typename Prim::Item;

  /**
   * @brief The SoA leaf primitive: a group of up to W primitives carrying their cloud indices.
   */
  using Group = typename Prim::Group;

  /**
   * @brief The packed BVH over the groups.
   */
  using Packed = BVH::PackedBVH<T, Group, K>;

  /**
   * @brief Flat node type of the held BVH.
   */
  using Node = typename Packed::Node;

  /**
   * @brief Axis-aligned bounding box type.
   */
  using AABB = BoundingVolumes::AABBT<T>;

protected:
  /**
   * @brief An empty BVH, to be reserved and filled.
   */
  CloudBVH() noexcept = default;

  /**
   * @brief The scratch arrays of a build: in the BVH's pool for a BVH with room for a rebuild, and in a pool of
   * their own, released after the build, otherwise.
   */
  struct Scratch
  {
    /// @brief The primitives' sort keys, a power of two of them (the padding sorts last).
    PODVector<MortonKey> m_keys;

    /// @brief GPU::boundingBoxOf()'s scratch.
    PODVector<Vec3T<T>> m_box;

    /// @brief The build's nodes, in breadth-first order.
    PODVector<BuildNode> m_nodes;

    /// @brief Each build node's box, indexed like m_nodes.
    PODVector<AABB> m_boxes;

    /// @brief A level's interior flags, scanned into ranks; then, by depth-first index, each interior node's
    /// row.
    PODVector<std::uint32_t> m_rows;

    /// @brief By depth-first index, each leaf's first group.
    PODVector<std::uint32_t> m_groups;

    /// @brief GPU::exclusiveScan()'s scratch.
    PODVector<std::uint32_t> m_scan;
  };

  /**
   * @brief The largest array sizes any cloud of a given size can need; see PointCloudBVH's build steps.
   */
  struct Sizes
  {
    /// @brief Nodes.
    std::uint32_t m_nodes;

    /// @brief Interior nodes, one child-box row each.
    std::uint32_t m_interior;

    /// @brief Groups.
    std::uint32_t m_groups;
  };

  /**
   * @brief Build over the primitives of a source, where the pool's memory is: the body of PointCloudBVH::build().
   * @tparam Source The primitives' source.
   * @param[in,out] a_pool           Pool the arrays are reserved from.
   * @param[in]     a_source         The primitives.
   * @param[in]     a_numItems       Number of primitives.
   * @param[in]     a_targetLeafSize Target primitives per leaf.
   * @param[in]     a_maxItems       Most primitives a rebuild may have; 0 for none.
   * @param[in]     a_who            The calling function, for the messages.
   */
  template <class Source>
  EBGEOMETRY_HOST
  inline void
  initialize(Pool&         a_pool,
             const Source& a_source,
             uint32_t      a_numItems,
             std::size_t   a_targetLeafSize,
             uint32_t      a_maxItems,
             const char*   a_who);

  /**
   * @brief Build over the primitives of a host array, where the pool's memory is: for a pool in device memory, the
   * primitives are checked on the host and copied to scratch there first.
   * @param[in,out] a_pool           Pool the arrays are reserved from.
   * @param[in]     a_items          The primitives.
   * @param[in]     a_targetLeafSize Target primitives per leaf.
   * @param[in]     a_maxItems       Most primitives a rebuild may have; 0 for none.
   * @param[in]     a_who            The calling function, for the messages.
   */
  EBGEOMETRY_HOST
  inline void
  initialize(Pool&                    a_pool,
             const std::vector<Item>& a_items,
             std::size_t              a_targetLeafSize,
             uint32_t                 a_maxItems,
             const char*              a_who);

  /**
   * @brief Build again in the arrays initialize() reserved: the body of PointCloudBVH::rebuild().
   * @tparam Source The primitives' source.
   * @param[in] a_source   The primitives.
   * @param[in] a_numItems Number of primitives, at most m_maxItems.
   * @param[in] a_who      The calling function, for the messages.
   */
  template <class Source>
  EBGEOMETRY_HOST
  inline void
  reinitialize(const Source& a_source, uint32_t a_numItems, const char* a_who);

  /**
   * @brief Resolve this object against @p a_pool: the body of PointCloudBVH::rebasedView(), on a copy.
   * @param[in] a_pool The pool to resolve against.
   */
  EBGEOMETRY_HOST
  inline void
  rebase(const Pool& a_pool) noexcept;

  /**
   * @brief Duplicate this object's storage into @p a_dstPool, for @p a_copy, a copy of this object: the body of
   * PointCloudBVH::deepCopy().
   * @param[out]    a_copy    The copy, whose arrays are reserved and written.
   * @param[in,out] a_dstPool Pool to reserve the copy's arrays from.
   */
  EBGEOMETRY_HOST
  inline void
  copyInto(CloudBVH& a_copy, Pool& a_dstPool) const;

  /**
   * @brief Bounds on the array sizes of a build over @p a_numItems primitives.
   * @param[in] a_numItems Number of primitives.
   * @param[in] a_leafSize The largest leaf, max(a_targetLeafSize, K).
   * @param[in] a_who      The calling function, for the messages.
   * @return The bounds.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static inline Sizes
  sizesFor(uint32_t a_numItems, uint32_t a_leafSize, const char* a_who) noexcept;

  /**
   * @brief Reserve a build's scratch from @p a_pool, for up to @p a_numItems primitives.
   * @param[in,out] a_pool     Pool to reserve from.
   * @param[in]     a_numItems Most primitives.
   * @param[in]     a_leafSize The largest leaf.
   * @param[in]     a_who      The calling function, for the messages.
   * @return The scratch arrays.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static inline Scratch
  reserveScratch(Pool& a_pool, uint32_t a_numItems, uint32_t a_leafSize, const char* a_who);

  /**
   * @brief Reserve the BVH's and the cloud's arrays from @p a_pool.
   * @param[in,out] a_pool     Pool to reserve from; this object is attached to it.
   * @param[in]     a_sizes    Array sizes.
   * @param[in]     a_numItems Number of primitives.
   */
  EBGEOMETRY_HOST
  inline void
  reserveArrays(Pool& a_pool, const Sizes& a_sizes, uint32_t a_numItems);

  /**
   * @brief Build steps 1 to 5 (see PointCloudBVH): the box, the sorted keys, the tree's levels, the subtree
   * sizes, boxes and depth-first indices, and the scans that place the rows and the groups.
   * @tparam Source The primitives' source.
   * @param[in] a_scratch  The scratch arrays.
   * @param[in] a_base     Base address of the scratch arrays' pool.
   * @param[in] a_source   The primitives.
   * @param[in] a_numItems Number of primitives.
   * @param[in] a_who      The calling function, for the messages.
   * @return The tree's array sizes.
   */
  template <class Source>
  EBGEOMETRY_HOST
  inline Sizes
  shape(const Scratch& a_scratch, void* a_base, const Source& a_source, uint32_t a_numItems, const char* a_who);

  /**
   * @brief Build step 6 (see PointCloudBVH): write the arrays reserved for the tree shape() gave.
   * @tparam Source The primitives' source.
   * @param[in] a_scratch  The scratch arrays, as shape() left them.
   * @param[in] a_base     Base address of the scratch arrays' pool.
   * @param[in] a_sizes    The tree's array sizes.
   * @param[in] a_source   The primitives.
   * @param[in] a_numItems Number of primitives.
   */
  template <class Source>
  EBGEOMETRY_HOST
  inline void
  write(const Scratch& a_scratch, void* a_base, const Sizes& a_sizes, const Source& a_source, uint32_t a_numItems);

  /**
   * @brief Run a build step: through GPU::parallelFor() for a build there, and in a host loop otherwise.
   * @tparam F The step's functor.
   * @param[in] a_count Number of indices.
   * @param[in] a_f     The functor.
   */
  template <class F>
  EBGEOMETRY_HOST
  inline void
  run(std::size_t a_count, const F& a_f) const;

  /**
   * @brief An exclusive scan, where the build runs; see GPU::exclusiveScan().
   * @param[in,out] a_values  The values.
   * @param[in]     a_count   Number of values.
   * @param[out]    a_scratch GPU::exclusiveScanScratchSize(a_count) elements, for a build through
   *                          GPU::parallelFor().
   * @return The values' sum.
   */
  EBGEOMETRY_HOST
  inline std::uint32_t
  scan(std::uint32_t* a_values, std::uint32_t a_count, std::uint32_t* a_scratch) const;

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
   * @brief The packed BVH over the groups, as a layout: it shares this object's location.
   */
  typename Packed::Layout m_bvhLayout;

  /**
   * @brief The primitives, indexed by cloud index.
   */
  PODVector<Item> m_items;

  /**
   * @brief Per-primitive own-leaf group offset (for seeding).
   */
  PODVector<std::uint32_t> m_leafOff;

  /**
   * @brief Per-primitive own-leaf group count (for seeding).
   */
  PODVector<std::uint32_t> m_leafCnt;

  /**
   * @brief Cloud indices in leaf (build) order; the spatially coherent batch-query order.
   */
  PODVector<std::uint32_t> m_order;

  /**
   * @brief The scratch arrays a rebuild uses; empty for a BVH without room for one.
   */
  Scratch m_scratch;

  /**
   * @brief The most primitives a rebuild can take, 0 for none.
   */
  std::uint32_t m_maxItems = 0;

  /**
   * @brief The largest leaf, max(a_targetLeafSize, K): a node of more primitives is cut.
   */
  std::uint32_t m_leafSize = 0;

  /**
   * @brief The tree's depth in node levels (the root is level 1), 0 for an empty cloud.
   */
  std::uint32_t m_depth = 0;

  /**
   * @brief Whether the build runs through GPU::parallelFor() (the pool's memory is device-accessible).
   */
  bool m_parallel = false;
};

} // namespace PointCloudDetail

} // namespace EBGeometry

#include "EBGeometry_CloudBVHImplem.hpp"

#endif
