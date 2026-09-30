// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_BVH.hpp
 * @brief  Declaration of the packed bounding volume hierarchy (BVH).
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_BVH_HPP
#define EBGEOMETRY_BVH_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(__SSE4_1__) || defined(__AVX__)
#include <immintrin.h>
#endif

// Our includes
#include "EBGeometry_Array.hpp"
#include "EBGeometry_BVHBuild.hpp"
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_PODVector.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Namespace for various bounding volume hierarchy (BVH) functionalities.
 */
namespace BVH {

/**
 * @brief The default branching factor K: 4, for both float and double, in every translation unit.
 * @details This is the value the library's class templates default to. It never depends on
 * compiler flags, so a type spelled with it has the same layout in a host-only file, in a file
 * compiled with AVX, and in both passes of a CUDA or HIP compile -- which is what lets an object be
 * built on the host and used on a device. On the benchmarks in the Sphinx page on configuration
 * options, it is also as fast on the host as the ISA-tuned value.
 *
 * For host-only code, HostBranchingRatio<T>() gives the value tuned to the compiler's SIMD flags instead.
 * Usage: `size_t K = BVH::DefaultBranchingRatio<T>()` as a template-parameter default.
 * @tparam T Floating-point precision type (float or double).
 * @return 4.
 */
template <typename T>
[[nodiscard]] constexpr size_t
DefaultBranchingRatio() noexcept
{
  static_assert(std::is_floating_point_v<T>, "BVH::DefaultBranchingRatio requires a floating-point T");

  return 4;
}

/**
 * @brief The branching factor K that fills one SIMD register for type T under the compiler's SIMD flags.
 * @details Opt-in, for host-only code:
 *
 * | ISA       | T=float | T=double |
 * |-----------|---------|----------|
 * | AVX-512F  |   16    |    8     |
 * | AVX       |    8    |    4     |
 * | SSE4.1    |    4    |    4     |
 * | fallback  |    4    |    4     |
 *
 * The value depends on the flags each translation unit is compiled with, so a type spelled with it
 * can mean different types in different files, and different layouts in the host and device passes
 * of one CUDA or HIP compile. Never use it for a type that is shared with device code or with another
 * translation unit built with different flags; use DefaultBranchingRatio<T>() there.
 * @tparam T Floating-point precision type (float or double).
 * @return K for T under the current ISA.
 */
template <typename T>
[[nodiscard]] constexpr size_t
HostBranchingRatio() noexcept
{
  static_assert(std::is_floating_point_v<T>, "BVH::HostBranchingRatio requires a floating-point T");
#if defined(__AVX512F__)
  if constexpr (std::is_same_v<T, double>) {
    return 8;
  }
  else {
    return 16;
  }
#elif defined(__AVX__)
  if constexpr (std::is_same_v<T, double>) {
    return 4;
  }
  else {
    return 8;
  }
#else
  return 4;
#endif
}

/**
 * @brief Flat, pool-resident BVH: an array of wide nodes and an array of primitives in leaf order.
 * @details Built on the host by buildTopology() (see BuildSpec), queried on the host or a device by
 * pruneTraverse(). Each node holds up to K child slots, and each slot the bounding box of its child
 * together with either a child node or a leaf's primitive range (see WideNode). The primitives are
 * stored by value, in the order the leaves visit them, so every leaf is one contiguous range.
 *
 * A PackedBVH is a trivially copyable descriptor. Its two arrays live in the Pool passed to the
 * constructor, which must outlive it and every copy of it; copying the descriptor copies no data
 * (see deepCopy()). rebasedView() produces the copy a device kernel receives.
 * @tparam T Floating-point precision.
 * @tparam P Primitive type. Must be trivially copyable. PackedBVH asks nothing else of it; the
 * leaf callback passed to pruneTraverse() decides what a primitive is used for.
 * @tparam K Branching factor (number of child slots per node, at least 2).
 */
template <class T, class P, size_t K>
class PackedBVH final
{
  static_assert(std::is_floating_point_v<T>, "PackedBVH: T must be a floating-point type");
  static_assert(K >= 2, "PackedBVH: branching factor K must be at least 2");

public:
  /**
   * @brief AABB type used for all bounding volumes in this BVH.
   */
  using BV = EBGeometry::BoundingVolumes::AABBT<T>;

  /**
   * @brief Node type of the flat node array.
   */
  using Node = WideNode<T, K>;

  /**
   * @brief Deleted default constructor: a BVH is always built, or adopts arrays built elsewhere.
   */
  PackedBVH() = delete;

  /**
   * @brief Build a BVH over primitives, each with its bounding box.
   * @details Stores one primitive per input pair, reordered into leaf order.
   * @param[in,out] a_pool        Pool the arrays are reserved from; must outlive this object.
   * @param[in]     a_primsAndBVs Primitives and their bounding boxes. May be empty, which gives an
   * empty BVH.
   * @param[in]     a_spec        How to build the tree.
   */
  EBGEOMETRY_HOST
  inline PackedBVH(Pool& a_pool, std::vector<std::pair<P, BV>> a_primsAndBVs, const BuildSpec& a_spec = BuildSpec{});

  /**
   * @brief Store a BVH whose shape was built with buildTopology(), converting each leaf's input
   * primitives into the stored ones.
   * @details For a class whose stored primitive is not the one the tree was built over: TriMeshSDF
   * packs a leaf's triangles into SIMD groups, and PointCloudBVH a leaf's points. The callback is
   * called once per leaf, in leaf order, as
   *
   * @code
   * a_packLeaf(const uint32_t* a_items, uint32_t a_count, std::vector<P>& a_out)
   * @endcode
   *
   * where @p a_items are the indices of the leaf's input primitives (a range of
   * @c a_topology.order), and must append at least one primitive to @p a_out. The leaf's primitive
   * range becomes whatever it appended.
   * @tparam PackLeaf Callable, as above.
   * @param[in,out] a_pool     Pool the arrays are reserved from; must outlive this object.
   * @param[in]     a_topology Shape of the tree, from buildTopology().
   * @param[in]     a_packLeaf Leaf conversion.
   */
  template <class PackLeaf>
  EBGEOMETRY_HOST
  inline PackedBVH(Pool& a_pool, const Topology<T, K>& a_topology, PackLeaf&& a_packLeaf);

  /**
   * @brief Adopt a node array and a primitive array built on the host by other means.
   * @details The arrays are copied into @p a_pool. They are checked, in every build, to form a
   * well-formed tree (see requireWellFormed()) no deeper than HostTraversalDepth, since a malformed
   * array would otherwise surface as an out-of-bounds read in Release. An empty node array is
   * accepted only with an empty primitive array.
   * @param[in,out] a_pool       Pool the arrays are reserved from; must outlive this object.
   * @param[in]     a_nodes      Node array, root at index 0.
   * @param[in]     a_primitives Primitives in leaf order.
   */
  EBGEOMETRY_HOST
  inline PackedBVH(Pool& a_pool, const std::vector<Node>& a_nodes, const std::vector<P>& a_primitives);

  /**
   * @brief Adopt a node array and a primitive array already in @p a_pool.
   * @details For arrays built in place, for example by a device kernel into a pool in device
   * memory, so that no data passes through the host. Reserve the arrays from @p a_pool with
   * PODVector::reserveFrom() -- maxNodeCount() bounds the node array -- fill them, and set their
   * sizes with PODVector::setSize(). Nothing is copied.
   *
   * The arrays are checked as the host-array constructor checks them when the pool is readable on
   * the host. A pool in device-only memory cannot be read here, so its arrays are not checked, and
   * the caller is responsible for their shape.
   * @param[in,out] a_pool       Pool both arrays were reserved from; must outlive this object.
   * @param[in]     a_nodes      Node array, root at index 0.
   * @param[in]     a_primitives Primitives in leaf order.
   */
  EBGEOMETRY_HOST
  inline PackedBVH(Pool& a_pool, const PODVector<Node>& a_nodes, const PODVector<P>& a_primitives);

  /**
   * @brief The most nodes a BVH with @p a_numLeaves leaves needs.
   * @details Every node of a tree built by buildTopology() has at least two occupied slots, except
   * a root holding one leaf, so it has at most max(1, leaves - 1) nodes, and a tree over N
   * primitives at most max(1, N - 1). A builder that keeps to the same rule can reserve its node
   * array with this bound.
   * @param[in] a_numLeaves Number of leaves.
   * @return Upper bound on the number of nodes.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static constexpr size_t
  maxNodeCount(const size_t a_numLeaves) noexcept
  {
    return (a_numLeaves <= 1) ? size_t(1) : a_numLeaves - 1;
  }

  /**
   * @brief Destructor. Not virtual: PackedBVH is final, and a class that needs a BVH over its own
   * payload holds one by value (see PointCloudBVH).
   */
  inline ~PackedBVH() = default;

  /**
   * @brief Copy constructor.
   * @details Copies the descriptor only: the copy resolves against the same pool storage as the
   * original, and a refit() through either is seen by both. Use deepCopy() for independent storage.
   * @param[in] a_other Other instance to copy.
   */
  PackedBVH(const PackedBVH& a_other) = default;

  /**
   * @brief Copy assignment operator.
   * @param[in] a_other Other instance to copy.
   * @return Reference to *this.
   */
  PackedBVH&
  operator=(const PackedBVH& a_other) = default;

  /**
   * @brief Move constructor.
   * @param[in,out] a_other Other instance to move from.
   */
  PackedBVH(PackedBVH&& a_other) noexcept = default;

  /**
   * @brief Move assignment operator.
   * @param[in,out] a_other Other instance to move from.
   * @return Reference to *this.
   */
  PackedBVH&
  operator=(PackedBVH&& a_other) noexcept = default;

  /**
   * @brief Get the primitives, in leaf order.
   * @details The returned span is a resolved address into pool memory. It must not outlive the next
   * Pool::reserve on the owning pool, which may move the block.
   * @return Span over the primitive array.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline PODSpan<const P>
  getPrimitives() const noexcept;

  /**
   * @brief Get the primitives, in leaf order, for modification.
   * @details The only way to move a packed geometry in place. Moving a primitive invalidates the
   * boxes that enclose it, so follow up with refit() before querying again. Which leaf a primitive
   * belongs to is fixed at build time; moving primitives far enough to want a different partition
   * needs a rebuild. Same lifetime caveat as the const overload.
   * @return Mutable span over the primitive array.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline PODSpan<P>
  getPrimitives() noexcept;

  /**
   * @brief Get the node array (root at index 0, children after parents).
   * @details Read-only: the shape is fixed at build time. For a class that walks the tree with its
   * own traversal (PointCloudBVH's seeded self-query). Same lifetime caveat as getPrimitives().
   * @return Span over the node array; empty for an empty BVH.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline PODSpan<const Node>
  getNodes() const noexcept;

  /**
   * @brief Traversal stack size, in entries, for the current compilation pass.
   * @details (K - 1) * depth + 1 entries hold the traversal of a tree of the given depth, with
   * HostTraversalDepth in a host pass and DeviceTraversalDepth in a device pass. The builder never
   * makes a deeper tree than the device depth; an adopted array is checked against the host depth
   * when it is adopted and against the device depth when rebasedView() produces a device view --
   * except arrays adopted in device-only memory, which the host cannot read and the caller vouches
   * for. A traversal that pushes at most K entries per node expanded, as pruneTraverse() does, then
   * cannot overflow a stack this size.
   * @return Number of stack entries.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static constexpr size_t
  traversalStackDepth() noexcept
  {
#if defined(EBGEOMETRY_DEVICE_COMPILE)
    return PackedBVH::stackEntriesFor(DeviceTraversalDepth);
#else
    return PackedBVH::stackEntriesFor(HostTraversalDepth);
#endif
  }

  /**
   * @brief Resolve the base address this BVH's arrays are offsets from.
   * @return Base address of the pool block holding this BVH.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const void*
  base() const noexcept;

  /**
   * @brief Produce a copy of this BVH that resolves against @p a_pool.
   * @details The one sanctioned crossing, mirroring DCEL::MeshT::rebasedView(). Mirror the pool
   * first, then rebase, then copy the returned value into a kernel:
   *
   * @code
   * Pool devicePool = Pool::mirror(hostPool, deviceMemoryResource());
   * const auto deviceBVH = bvh.rebasedView(devicePool);
   * myKernel<<<blocks, threads>>>(deviceBVH, ...);
   * @endcode
   *
   * A host-to-host mirror is supported too and follows the target's control block instead of
   * snapshotting its base. The rules are DCEL::MeshT::rebasedView()'s (see PoolLocation::rebasedOnto()).
   * A device view is also checked to be no deeper than DeviceTraversalDepth.
   * @param[in] a_pool Pool to rebase onto: this BVH's own pool or one in its mirror chain.
   * @return A copy of this BVH resolving against @p a_pool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline PackedBVH
  rebasedView(const Pool& a_pool) const noexcept;

  /**
   * @brief The pool location this BVH resolves against.
   * @return This BVH's control block (host) or base address (device view).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline PoolLocation
  location() const noexcept;

  /**
   * @brief A copy of this BVH resolving against @p a_location instead of its own location.
   * @details For a descriptor stored inside another object's pool (see PoolLocation). Unlike
   * rebasedView() it checks nothing: @p a_location must belong to the pool this BVH was reserved
   * from, or to a mirror of it.
   * @param[in] a_location Location to resolve against.
   * @return A copy of this BVH resolving against @p a_location.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline PackedBVH
  relocatedTo(const PoolLocation& a_location) const noexcept;

  /**
   * @brief Duplicate this BVH's storage into @p a_dstPool.
   * @param[in,out] a_dstPool Pool to reserve the copy's arrays from.
   * @return A BVH with the same structure, backed by @p a_dstPool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline PackedBVH
  deepCopy(Pool& a_dstPool) const;

  /**
   * @brief Check whether this BVH's arrays were reserved from @p a_pool.
   * @param[in] a_pool Pool to test against.
   * @return True if this BVH is attached to @p a_pool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline bool
  isAttachedTo(const Pool& a_pool) const noexcept;

  /**
   * @brief Bounding box of the whole BVH.
   * @return The union of the root's slot boxes; the empty (inverted) box for an empty BVH.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline BV
  getBoundingVolume() const noexcept;

  /**
   * @brief Bounding box of the whole BVH, under the name other bounded objects use.
   * @return Same as getBoundingVolume().
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline BV
  computeBoundingVolume() const noexcept;

  /**
   * @brief Depth-first, distance-pruned traversal: the one traversal of a PackedBVH.
   * @details Visits leaves nearest-box first, and skips every slot whose box is farther from
   * @p a_point than the current pruning bound. @c State is whatever the search remembers between
   * leaf visits. @c LeafEvaluator is called at leaves only, and is the only place @c State may
   * change. @c PruneDistSquared turns the current @c State into a squared-distance bound; it is
   * re-read at every step, so a leaf visited anywhere tightens the pruning of everything after it.
   *
   * The distances from the query point to all K boxes of a node are computed at once, vectorised
   * where a SIMD path exists for (T, K) and scalar otherwise, and every path gives the same bits.
   * Each node's slots are pushed farthest first, so the nearest is expanded next. The stack holds a
   * node-and-slot index and a float lower bound on the slot's squared distance, 8 bytes per entry,
   * and has traversalStackDepth() entries.
   *
   * @note Only the pruning bound is customisable; the per-slot quantity it is compared against is
   * the squared Euclidean distance to the slot's box, which is a true lower bound on the distance to
   * anything inside it -- the condition for branch-and-bound pruning to be exact.
   * @tparam State            Running search state, carried by reference through the traversal.
   * @tparam LeafEvaluator    Callable: (State&, size_t offset, size_t count) noexcept -> void. Scans
   * primitives [offset, offset+count) and updates the state.
   * @tparam PruneDistSquared Callable: (const State&) noexcept -> T. The current squared-distance
   * bound; a slot farther than this is skipped.
   * @param[in]     a_point      Query point.
   * @param[in,out] a_state      Running search state.
   * @param[in]     a_evalLeaf   Leaf-visit callback.
   * @param[in]     a_pruneDist2 Pruning-bound callback.
   */
  template <class State, class LeafEvaluator, class PruneDistSquared>
  EBGEOMETRY_HOST_DEVICE
  inline void
  pruneTraverse(const Vec3T<T>&    a_point,
                State&             a_state,
                LeafEvaluator&&    a_evalLeaf,
                PruneDistSquared&& a_pruneDist2) const noexcept;

  /**
   * @brief Recompute every box in place after the primitives have moved.
   * @details Keeps the shape and every leaf's primitive range, and recomputes only the boxes: a
   * single reverse sweep over the node array, since every child follows its parent. Cheap
   * per-frame maintenance for primitives that move without migrating between leaves; not a
   * substitute for a rebuild once the tree has degraded.
   * @tparam BVConstructor Callable: (const P&) -> BV, one primitive's current bounding box.
   * @param[in] a_bvConstructor Bounding-box constructor for a single primitive.
   */
  template <class BVConstructor>
  EBGEOMETRY_HOST
  inline void
  refit(const BVConstructor& a_bvConstructor);

  /**
   * @brief Abort unless a node array is a well-formed BVH over @p a_numPrimitives primitives.
   * @details Every occupied slot comes before every empty one; an interior slot's child lies after
   * its parent, inside the array, and has no other parent; a leaf's range lies inside the primitive
   * array; every node other than the root is reachable; and an empty node array comes with no
   * primitives. Always on, since a malformed array would otherwise surface as an out-of-bounds
   * read in Release.
   * @param[in] a_nodes         Node array.
   * @param[in] a_numNodes      Number of nodes.
   * @param[in] a_numPrimitives Size of the primitive array the leaves index.
   */
  EBGEOMETRY_HOST
  static inline void
  requireWellFormed(const Node* a_nodes, size_t a_numNodes, size_t a_numPrimitives);

private:
  /**
   * @brief One entry of pruneTraverse()'s stack.
   * @details A node-and-slot index (node * K + slot) and a lower bound on the squared distance from
   * the query point to that slot's box, rounded down to float. The bound lets a popped entry be
   * skipped if the pruning bound has tightened since it was pushed; rounding it down, never up,
   * keeps that exact.
   */
  struct StackEntry
  {
    /// @brief node * K + slot.
    uint32_t m_slot;

    /// @brief Lower bound on the squared distance to the slot's box, at push time.
    float m_dist2;
  };

  /**
   * @brief Stack entries needed to traverse a tree of @p a_depth node levels.
   * @details The traversal pops one entry and pushes up to K per node it expands, so a root-to-leaf
   * path through @p a_depth nodes peaks at (K - 1) * depth + 1 entries.
   * @param[in] a_depth Tree depth in node levels.
   * @return Number of entries.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static constexpr size_t
  stackEntriesFor(const size_t a_depth) noexcept
  {
    return (K - 1) * a_depth + 1;
  }

  /**
   * @brief @p a_dist2 rounded down to float.
   * @param[in] a_dist2 Non-negative squared distance.
   * @return A float no larger than @p a_dist2.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline float
  lowerBound(T a_dist2) noexcept;

  /**
   * @brief Copy a completed host-side build into pool storage and check its depth.
   * @param[in,out] a_pool       Pool to reserve from.
   * @param[in]     a_nodes      Completed node array.
   * @param[in]     a_primitives Completed primitive array.
   */
  EBGEOMETRY_HOST
  inline void
  finalize(Pool& a_pool, const std::vector<Node>& a_nodes, const std::vector<P>& a_primitives);

  /**
   * @brief Attach to @p a_pool on the first reservation, and check every later one uses it too.
   * @param[in,out] a_pool Pool this BVH's arrays live in.
   */
  EBGEOMETRY_HOST
  inline void
  attachTo(Pool& a_pool) noexcept;

  /**
   * @brief Abort if the tree is deeper than @p a_depth node levels.
   * @details Always on: the failure it prevents is an out-of-bounds write into pruneTraverse()'s
   * fixed stack, which a Release build would perform silently.
   * @param[in] a_base    Base address the node array resolves against.
   * @param[in] a_depth   Deepest allowed tree, in node levels.
   * @param[in] a_context Short label naming the caller, for the diagnostic.
   */
  EBGEOMETRY_HOST
  inline void
  requireDepthFits(const void* a_base, size_t a_depth, const char* a_context) const;

  /**
   * @brief Squared distances from a query point to all K slot boxes of one node.
   * @details Dispatches at compile time on (T, K) and the compiled ISA: AVX-512F for (double, 8)
   * and (float, 16), AVX for (double, 4), (float, 8) and (double, 8) as two 4-wide passes, SSE4.1
   * for (float, 4), and WideNode::getDistance2() for every other combination and in device code.
   * Every path computes max(0, max(lo - p, p - hi)) per axis, then dx*dx + (dy*dy + dz*dz), so all
   * of them agree bit for bit.
   * @param[in]  a_node  Node.
   * @param[in]  a_point Query point.
   * @param[out] a_dist2 One squared distance per slot; +inf for an empty slot.
   */
  EBGEOMETRY_HOST_DEVICE
  static inline void
  computeChildDistances2(const Node& a_node, const Vec3T<T>& a_point, T (&a_dist2)[K]) noexcept;

  /**
   * @brief Where the arrays live: the Pool it follows, or a snapshot made by rebasedView().
   */
  PoolLocation m_location;

  /**
   * @brief Node array; root at index 0.
   */
  PODVector<Node> m_nodes;

  /**
   * @brief Primitives in leaf order.
   */
  PODVector<P> m_primitives;
};

/**
 * @brief A PackedBVH must be trivially copyable: that is what lets a rebasedView() be byte-copied
 * into a device address space with no pointer patching.
 */
static_assert(std::is_trivially_copyable_v<PackedBVH<float, Vec3T<float>, 4>>,
              "PackedBVH<float, ...> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<PackedBVH<double, Vec3T<double>, 4>>,
              "PackedBVH<double, ...> must be trivially copyable");

} // namespace BVH

} // namespace EBGeometry

#include "EBGeometry_BVHImplem.hpp"

#endif
