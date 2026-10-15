// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_PackedBVH.hpp
 * @brief  The flattened, device-callable bounding volume hierarchy: PackedBVH, its node layout,
 * traversals and the types shared with the builders.
 * @details Device code needs only this header. The host-side builders are TreeBVH and its
 * partitioners (EBGeometry_TreeBVH.hpp) and the PackedBVH constructors that build from a TreeBVH
 * or partition a primitive list (EBGeometry_BVHBuild.hpp). Both include this one.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_PACKEDBVH_HPP
#define EBGEOMETRY_PACKEDBVH_HPP

// Std includes
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(__SSE4_1__) || defined(__AVX__)
#include <immintrin.h>
#endif

// Our includes
#include "EBGeometry_Array.hpp"
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_PODVector.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

namespace SFC {
struct Morton;
} // namespace SFC

/**
 * @brief Namespace for various bounding volume hierarchy (BVH) functionalities.
 */
namespace BVH {

/**
 * @brief The preset BVH construction methods that the library's own BVH users accept.
 * @details Each value names the algorithm that groups the primitives; its comment says in which
 * direction the tree is built. Top-down methods split the root's primitives recursively until a
 * leaf predicate stops them; bottom-up methods sort the primitives along a space-filling curve, cut
 * the sorted list into leaves, and merge K neighbours at a time up to the root.
 *
 * MeshSDF, TriMeshSDF, BVHUnion and BVHSmoothUnion (and the parser functions that build them)
 * accept every value. Each of them aborts, in every build, on a value outside this list. A custom
 * partitioner or leaf predicate is not a preset: build a TreeBVH with it and pack() it instead.
 */
enum class Construction
{
  CentroidSplit, ///< Top-down: split at the median bounding-volume centroid along the longest axis
                 ///< (BVCentroidPartitioner).
  MidpointSplit, ///< Top-down: split at the spatial midpoint of the centroids' longest axis, with no
                 ///< sorting (MidpointPartitioner). The fastest top-down build; does not adapt to
                 ///< clustered input.
  SAH,           ///< Top-down: binned surface area heuristic (BinnedSAHPartitioner). The recommended
                 ///< default: generally the lowest traversal cost.
  ClusterSAH,    ///< Top-down: group the primitives into small spatial clusters, then run binned SAH
                 ///< over the clusters (see ClusterSpec). Builds several times faster than SAH, at
                 ///< the cost of leaves of up to (K-1) clusters.
  Morton,        ///< Bottom-up: leaves of consecutive primitives along a Morton (Z-order) curve.
  Nested,        ///< Bottom-up: leaves of consecutive primitives along a Nested (row-major) curve.
  Hilbert        ///< Bottom-up: leaves of consecutive primitives along a Hilbert curve.
};

/**
 * @brief Configuration for the ClusterSAH direct PackedBVH construction path.
 * @details ClusterSAH first groups primitives into small, spatially-tight *clusters* (buckets of at
 * most @c maxClusterSize primitives, formed by a cheap density-adaptive midpoint subdivision that
 * stops early), then runs binned SAH top-down over the clusters -- so SAH partitions ~N/maxClusterSize
 * boxes instead of all N primitives. The result is a near-SAH-quality tree at a fraction of the
 * single-threaded build cost, robust across uniform, surface, and clustered primitive distributions
 * (unlike a fixed Cartesian grid, which overcrowds on non-uniform data). @c maxClusterSize trades
 * build time (larger -> fewer, cheaper SAH units, faster build) against query quality (larger ->
 * coarser leaves). It bounds the cluster, not the leaf: SAH stops splitting once a node holds fewer
 * than K clusters, so a leaf may hold up to (K-1) * maxClusterSize primitives.
 */
struct ClusterSpec
{
  size_t maxClusterSize = 8; ///< Maximum primitives per cluster (bucket); a leaf holds 1 to K-1 clusters. Must be > 0.
};

/**
 * @brief Leaf-size settings for the preset construction methods, one per family of methods.
 * @details The methods have different natural settings, so each BVH::Construction value reads only
 * its own field and ignores the others:
 *
 * | Construction                        | Field            | Meaning                                                      |
 * |-------------------------------------|------------------|--------------------------------------------------------------|
 * | SAH, CentroidSplit, MidpointSplit   | maxLeafSize      | A node with at most this many primitives becomes a leaf. A split makes K non-empty children, so a node with fewer than K is a leaf too: the leaves hold at most max(maxLeafSize, K-1). |
 * | Morton, Nested, Hilbert             | targetLeafSize   | The fewest leaves holding at most this many primitives each, of the leaf counts the builder allows; it never makes more leaves than primitives, so a small target can be exceeded. |
 * | ClusterSAH                          | cluster          | The cluster size; a leaf holds 1 to K-1 clusters (ClusterSpec). |
 *
 * Every class that takes a BVH::Construction (MeshSDF, TriMeshSDF, BVHUnion, BVHSmoothUnion) has
 * a static defaultConstructionOptions() that reproduces the trees it builds without options. Start
 * from those and change the field of the method in use. A field left at 0 is rejected when its
 * method reads it.
 */
struct ConstructionOptions
{
  size_t      maxLeafSize    = 0; ///< SAH, CentroidSplit, MidpointSplit: most primitives in a leaf. Must be > 0.
  size_t      targetLeafSize = 0; ///< Morton, Nested, Hilbert: target primitives per leaf. Must be > 0.
  ClusterSpec cluster{};          ///< ClusterSAH: the cluster size.
};

/**
 * @brief Deepest tree, in node levels (the root alone is one level), that a host traversal handles.
 * @details The traversals keep a fixed stack with one entry per level below the root, so the limit is
 * the same for every K. Every PackedBVH is checked against it when it is built. The stack takes 255
 * entries of 4 + K bytes rounded up to a multiple of 4: 2 KB at K = 4.
 */
inline constexpr size_t HostTraversalDepth = 256;

/**
 * @brief Deepest tree, in node levels, that a device traversal handles.
 * @details Smaller than the host value, since device stack memory is per thread: 31 entries of 4 + K
 * bytes rounded up to a multiple of 4, 248 bytes at K = 4. PackedBVH::rebasedView() checks every
 * device view against it.
 */
inline constexpr size_t DeviceTraversalDepth = 32;

/**
 * @brief The default branching factor K: 4, for both float and double, in every translation unit.
 * @details This is the value the library's class templates default to. It never depends on
 * compiler flags, so a type spelled with it has the same layout in a host-only file, in a file
 * compiled with AVX, and in both passes of a CUDA or HIP compile -- which is what lets an object be
 * built on the host and used on a device. On the benchmarks in the Sphinx page on configuration
 * options, it is also as fast on the host as the ISA-tuned value.
 *
 * For host-only code, HostBranchingRatio<T>() gives the value tuned to the compiler's SIMD flags instead.
 * EBGEOMETRY_HOST_TUNED_DEFAULTS does not change this value: it only widens the triangle groups
 * (TriangleSoA::DefaultWidth), since a wider K measured no faster.
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
 * @brief Forward declaration of the tree-structured BVH, defined in EBGeometry_TreeBVH.hpp. Needed
 * by LeafPredicate and by PackedBVH's constructor from a TreeBVH.
 */
template <class T, class P, class BV, size_t K>
class TreeBVH;

/**
 * @brief Convenience alias for a list of shared primitive pointers.
 * @tparam P Primitive type bounded by the BVH.
 */
template <class P>
using PrimitiveList = std::vector<std::shared_ptr<const P>>;

/**
 * @brief Forward declaration of the linearised BVH, defined below.
 */
template <class T, class P, size_t K>
class PackedBVH;

/**
 * @brief Convenience alias for a (primitive, bounding-volume) pair.
 * @tparam P  Primitive type.
 * @tparam BV Bounding volume type.
 */
template <class P, class BV>
using PrimAndBV = std::pair<std::shared_ptr<const P>, BV>;

/**
 * @brief Convenience alias for a list of (primitive, bounding-volume) pairs.
 * @tparam P  Primitive type.
 * @tparam BV Bounding volume type enclosing the implicit surface of each primitive.
 */
template <class P, class BV>
using PrimAndBVList = std::vector<PrimAndBV<P, BV>>;

/**
 * @brief Polymorphic partitioner: splits a list of (primitive, BV) pairs into K sub-lists.
 * @details The input list is taken *by value* (a sink): callers move their list in, and an
 * implementation should partition it in place and *move* the K sub-lists out -- so a whole top-down
 * build reorders primitive handles rather than repeatedly copying them (the built-in partitioners
 * all do this). The split arithmetic is the cheap part; avoiding the copies is what keeps
 * construction fast.
 * @tparam P  Primitive type.
 * @tparam BV Bounding volume type.
 * @tparam K  Tree branching factor.
 * @param[in] a_primsAndBVs Input primitives and their bounding volumes (consumed).
 * @return K-element array of sub-lists.
 */
template <class P, class BV, size_t K>
using Partitioner = std::function<Array<PrimAndBVList<P, BV>, K>(PrimAndBVList<P, BV> a_primsAndBVs)>;

/**
 * @brief Predicate for deciding when a TreeBVH node should become a leaf (i.e., no further splitting).
 * @tparam T  Floating-point precision.
 * @tparam P  Primitive type.
 * @tparam BV Bounding volume type.
 * @tparam K  Tree branching factor.
 * @param[in] a_node BVH node under consideration.
 * @return True if the node should not be sub-divided further.
 */
template <class T, class P, class BV, size_t K>
using LeafPredicate = std::function<bool(const TreeBVH<T, P, BV, K>& a_node)>;

/**
 * @brief Leaf-evaluation callback for TreeBVH::traverse.
 * @details Called once for every leaf node visited during traversal.
 * @tparam P Primitive type.
 * @param[in] a_primitives Primitive list stored in the leaf.
 */
template <class P>
using LeafEvaluator = std::function<void(const PrimitiveList<P>& a_primitives)>;

/**
 * @brief Leaf-evaluation callback for PackedBVH::traverse, as a std::function for host code.
 * @details Receives a view into the global primitive array (offset + count) rather than a
 * temporary sub-list, avoiding a heap allocation per leaf visit. PackedBVH stores its primitives
 * by value, so the array's element type is P itself. PackedBVH::traverse() takes any callable with
 * this signature; device code passes a lambda or functor instead, since std::function is host-only.
 * @tparam P Primitive type.
 * @param[in] a_primitives Global primitive array.
 * @param[in] a_offset     Index of the first primitive belonging to this leaf.
 * @param[in] a_count      Number of primitives in this leaf.
 */
template <class P>
using PackedLeafEvaluator = std::function<void(PODSpan<const P> a_primitives, size_t a_offset, size_t a_count)>;

/**
 * @brief Node-visit predicate for BVH traversal.
 * @details Must return true to descend into the node and false to prune it.
 * @tparam NodeType Node type (TreeBVH or PackedBVH::Node).
 * @tparam NodeKey  Caller-supplied per-node key attached to each stack entry
 * (e.g. a running minimum distance).
 * @param[in] a_node Node under consideration.
 * @param[in] a_nodeKey Caller-supplied key for this stack entry.
 * @return True if the subtree rooted at a_node should be visited.
 */
template <class NodeType, class NodeKey>
using PrunePredicate = std::function<bool(const NodeType& a_node, const NodeKey& a_nodeKey)>;

/**
 * @brief Child-ordering callback for TreeBVH traversal.
 * @details Sorts an array of (child-node-pointer, key) pairs in-place so that
 * the most promising child is visited first.
 * @tparam NodeType Node type (TreeBVH).
 * @tparam NodeKey  Per-node key attached to each stack entry.
 * @tparam K        Tree branching factor.
 * @param[in,out] a_children K child nodes together with their node keys.
 */
template <class NodeType, class NodeKey, size_t K>
using ChildOrderer = std::function<void(Array<std::pair<std::shared_ptr<const NodeType>, NodeKey>, K>& a_children)>;

/**
 * @brief A child node of a PackedBVH together with its traversal key.
 * @details What PackedBVH::traverse() hands its child orderer, one per child. The members are named
 * like std::pair's, so an orderer written against the earlier std::pair form keeps working; it is
 * a plain struct because std::pair's assignment is not callable in device code.
 * @tparam NodeKey Per-node key attached to each stack entry.
 */
template <class NodeKey>
struct NodeAndKey
{
  uint32_t first;  ///< Index of the child node in the node array.
  NodeKey  second; ///< The child's key, from the node-key factory.
};

/**
 * @brief Child-ordering callback for PackedBVH traversal, as a std::function for host code.
 * @details Same role as ChildOrderer but uses 32-bit node indices instead of shared_ptrs,
 * halving the stack-entry size. PackedBVH::traverse() takes any callable with this signature;
 * device code passes a lambda or functor, which must not call std::sort (an insertion sort over the
 * K children does).
 * @tparam NodeKey Per-node key attached to each stack entry.
 * @tparam K    Tree branching factor.
 * @param[in,out] a_children K (node-index, key) pairs to sort.
 */
template <class NodeKey, size_t K>
using PackedChildOrderer = std::function<void(Array<NodeAndKey<NodeKey>, K>& a_children)>;

/**
 * @brief Node-key factory called once per node during BVH traversal.
 * @details Produces the NodeKey value that will be passed to PrunePredicate and ChildOrderer for
 * each child of the current node.
 * @tparam NodeType Node type (TreeBVH or PackedBVH::Node).
 * @tparam NodeKey  Per-node key type to produce.
 * @param[in] a_node Current node.
 * @return NodeKey value for a_node's children.
 */
template <class NodeType, class NodeKey>
using NodeKeyFactory = std::function<NodeKey(const NodeType& a_node)>;

/**
 * @brief Linearised, AABB-backed BVH with SIMD-accelerated traversal.
 * @details PackedBVH is the runtime query class. It stores a flat depth-first array of
 * Node structs, a contiguous primitive list, and a per-node SoA AABB cache that enables
 * SIMD child tests.
 *
 * Instances are obtained by calling TreeBVH::pack() (same primitive type) or
 * TreeBVH::packWith<Q>(converter) (type-converting pack).
 *
 * PackedBVH imposes no interface requirement on P by itself -- it holds primitives opaquely and
 * only ever hands them back to a caller-supplied callback (traverse()'s LeafEvaluator, or
 * pruneTraverse()'s LeafEvaluator). Nearest-surface (signed-distance-style) queries are not a
 * PackedBVH member; callers build their own thin wrapper around pruneTraverse(), supplying
 * whatever primitive interface their own query needs.
 *
 * SIMD paths for pruneTraverse()'s child-box test are selected at compile time via if constexpr,
 * in computeChildDistances2():
 * - K==4,  T==float  → SSE4.1  (__m128)
 * - K==4,  T==double → AVX     (__m256d)
 * - K==8,  T==float  → AVX     (__m256)
 * - K==8,  T==double → AVX-512F (__m512d), or AVX (two __m256d passes) without AVX-512F
 * - K==16, T==float  → AVX-512F (__m512)
 * All other combinations, and device compilation, use a scalar loop.
 *
 * Primitives are stored by value, inline in the flat array: no per-primitive heap allocation and
 * no pointer chase on a leaf visit. P must therefore be a self-contained, trivially copyable value
 * type -- a prerequisite for mirroring the whole BVH into a device address space with a byte copy.
 * That rules out a polymorphic primitive: a BVH over primitives of different types (a CSG union of
 * a mesh and a sphere, say) needs the runtime dispatch of the tape. A primitive that itself lives in
 * a Pool, such as a TriMeshSDF inside a BVHUnion, is stored as its layout and paired with this BVH's
 * location() when it is evaluated; storing the handle itself is a compile error (see PoolHandle).
 * TreeBVH is unaffected by any of this -- it always stores primitives as shared_ptr.
 *
 * @tparam T Floating-point precision.
 * @tparam P Primitive type.
 * @tparam K BVH branching factor.
 */
template <class T, class P, size_t K>
class PackedBVH final : public PoolHandle
{
  static_assert(std::is_floating_point_v<T>, "PackedBVH: T must be a floating-point type");
  static_assert(K >= 2, "PackedBVH: branching factor K must be at least 2");
  static_assert(K <= 256, "PackedBVH: branching factor K must be at most 256, so a child slot fits in a byte");
  static_assert(!isPoolHandle<P>,
                "PackedBVH: a primitive stored in a pool must hold no location; store the handle's Layout "
                "and pair it with this BVH's location() (see PoolHandle)");

public:
  /**
   * @brief AABB type used for all bounding volumes in this BVH.
   */
  using BV = EBGeometry::BoundingVolumes::AABBT<T>;

  /**
   * @brief Compact BVH node stored in the flat node array.
   * @details Each node holds a bounding volume, a primitive range (leaves) or child
   * index table (interior nodes). Exposed as a public nested type so that users can
   * write traverse() callbacks (PrunePredicate, NodeKeyFactory) that inspect nodes.
   */
  struct Node
  {
    /**
     * @brief Axis-aligned bounding box for this node's subtree.
     */
    BV m_bv{};

    /**
     * @brief Index of the first primitive in the global primitive list (leaf nodes only).
     */
    uint32_t m_primOff{};

    /**
     * @brief Number of primitives in this leaf (zero for interior nodes).
     */
    uint32_t m_numPrims{};

    /**
     * @brief Depth-first indices of the K child nodes (interior nodes only).
     */
    Array<uint32_t, K> m_childOff{};

    /**
     * @brief Row of this interior node's children's boxes in PackedBVH's SIMD box array.
     * @details Owned by PackedBVH, which assigns it when the BVH is built or adopted, overwriting
     * whatever the node array held; leaves have no row. Only interior nodes get a row, so the box
     * array has one row per interior node rather than one per node.
     */
    uint32_t m_childBoxRow{};

    /**
     * @brief Set the bounding volume for this node.
     * @param[in] a_bv Bounding volume.
     */
    EBGEOMETRY_HOST_DEVICE
    inline void
    setBoundingVolume(const BV& a_bv) noexcept
    {
      m_bv = a_bv;
    }

    /**
     * @brief Set the primitive offset for this leaf node.
     * @param[in] a_off Index into the global primitive list.
     */
    EBGEOMETRY_HOST_DEVICE
    inline void
    setPrimitivesOffset(uint32_t a_off) noexcept
    {
      m_primOff = a_off;
    }

    /**
     * @brief Set the primitive count for this leaf node.
     * @param[in] a_n Number of primitives.
     */
    EBGEOMETRY_HOST_DEVICE
    inline void
    setNumPrimitives(uint32_t a_n) noexcept
    {
      m_numPrims = a_n;
    }

    /**
     * @brief Set the depth-first index of the k-th child.
     * @param[in] a_off Node index of the child.
     * @param[in] a_k   Child slot (0 … K-1).
     */
    EBGEOMETRY_HOST_DEVICE
    inline void
    setChildOffset(uint32_t a_off, size_t a_k) noexcept
    {
      EBGEOMETRY_EXPECT(a_k < K);
      m_childOff[a_k] = a_off;
    }

    /**
     * @brief Get the bounding volume.
     * @return Reference to m_bv.
     */
    [[nodiscard]] EBGEOMETRY_HOST_DEVICE
    inline const BV&
    getBoundingVolume() const noexcept
    {
      return m_bv;
    }

    /**
     * @brief Get the primitive offset (leaf nodes only).
     * @return Index of the first primitive in the global list.
     */
    [[nodiscard]] EBGEOMETRY_HOST_DEVICE
    inline uint32_t
    getPrimitivesOffset() const noexcept
    {
      return m_primOff;
    }

    /**
     * @brief Get the primitive count.
     * @return Number of primitives; zero for interior nodes.
     */
    [[nodiscard]] EBGEOMETRY_HOST_DEVICE
    inline uint32_t
    getNumPrimitives() const noexcept
    {
      return m_numPrims;
    }

    /**
     * @brief Get the child index table.
     * @return Reference to the K-element child-offset array.
     */
    [[nodiscard]] EBGEOMETRY_HOST_DEVICE
    inline const Array<uint32_t, K>&
    getChildOffsets() const noexcept
    {
      return m_childOff;
    }

    /**
     * @brief Get the row of this interior node's children's boxes in the SIMD box array.
     * @return Row index; meaningless for a leaf.
     */
    [[nodiscard]] EBGEOMETRY_HOST_DEVICE
    inline uint32_t
    getChildBoxRow() const noexcept
    {
      return m_childBoxRow;
    }

    /**
     * @brief Return true if this is a leaf node.
     * @return True if m_numPrims > 0 (leaf), false otherwise (interior).
     */
    [[nodiscard]] EBGEOMETRY_HOST_DEVICE
    inline bool
    isLeaf() const noexcept
    {
      return m_numPrims > 0;
    }

    /**
     * @brief Get the distance from a_point to this node's bounding volume.
     * @param[in] a_point Query point.
     * @return Distance to the bounding-box surface, or zero if inside.
     */
    [[nodiscard]] EBGEOMETRY_HOST_DEVICE
    inline T
    getDistanceToBoundingVolume(const Vec3T<T>& a_point) const noexcept
    {
      return m_bv.getDistance(a_point);
    }

    /**
     * @brief Get the squared distance from a_point to this node's bounding volume.
     * @details Avoids the sqrt that getDistanceToBoundingVolume() pays. For a caller-written
     * scalar branch-and-bound that compares against a squared pruning bound (e.g. PointCloudBVH's
     * seeded single-nearest search), rather than taking a square root only to square it again.
     * pruneTraverse() itself does not use it; it tests all K children at once through the SoA cache.
     * @param[in] a_point Query point.
     * @return Squared distance to the bounding-box surface, or zero if inside.
     */
    [[nodiscard]] EBGEOMETRY_HOST_DEVICE
    inline T
    getDistanceToBoundingVolume2(const Vec3T<T>& a_point) const noexcept
    {
      return m_bv.getDistance2(a_point);
    }
  };

  /**
   * @brief Deleted default constructor. Use TreeBVH::pack() or TreeBVH::packWith() to construct.
   */
  PackedBVH() = delete;

  /**
   * @brief The BVH's arrays as offsets into its pool, with no location: what another pool-resident
   * object stores when it holds this BVH (see PoolHandle). Defined below.
   */
  struct Layout;

  /**
   * @brief Rebuild a handle from a stored layout and the location of the pool that holds it.
   * @details How an outer pool-resident object evaluates a BVH it stores as a layout: it pairs the
   * layout with its own location. @p a_location must belong to the pool the layout was reserved from,
   * or to a mirror of it; nothing is checked, since a device has no pool to check against.
   * @param[in] a_layout   The BVH's arrays.
   * @param[in] a_location Where they live.
   */
  EBGEOMETRY_HOST_DEVICE
  inline PackedBVH(const Layout& a_layout, const PoolLocation& a_location) noexcept;

  /**
   * @brief The BVH's arrays without its location, for storing it inside another pool-resident object.
   * @return The layout.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Layout
  layout() const noexcept;

  /**
   * @brief Construct by packing a TreeBVH (identity primitive type).
   * @details Walks the tree depth-first, fills m_linearNodes and m_primitives directly,
   * then builds the SoA AABB cache. The source tree must have been built with
   * BV == AABBT<T>; bounding volumes are reused without conversion.
   * @param[in,out] a_pool Pool the packed arrays are reserved from; must outlive this object.
   * @param[in] a_tree Source tree.
   */
  EBGEOMETRY_HOST
  inline PackedBVH(Pool& a_pool, const TreeBVH<T, P, BV, K>& a_tree);

  /**
   * @brief Construct by packing a TreeBVH with primitive-type conversion.
   * @details The source tree holds primitives of type Q; the packed BVH holds
   * primitives of type P.  The most common reason for this mismatch is an SoA packing
   * step: a tree built over individual triangles (Q = Triangle<T>) can be repacked
   * into a BVH whose leaves hold SIMD-width groups (P = TriangleSoAT<T,W>), enabling
   * vectorised distance evaluation at every leaf visit.
   *
   * The converter is called once per leaf of the source tree and must return a
   * @c std::vector<P> containing all target primitives for that leaf:
   *
   * @code
   * a_converter(leafPrims, offset, count) → std::vector<P>
   * @endcode
   *
   * where @p leafPrims is the leaf's @c PrimitiveList<Q>, @p offset is the index of the leaf's first
   * primitive within @p leafPrims (always 0U, since the whole leaf list is passed), and @p count is
   * the number of primitives in the leaf. All returned vectors are appended to one contiguous
   * buffer, which is then copied into this PackedBVH's pool-backed primitive array.
   *
   * The source tree must have been built with BV == AABBT<T>; bounding volumes are reused
   * without conversion.
   *
   * @tparam Q         Source primitive type stored in the source tree.
   * @tparam Converter Callable: (PrimitiveList<Q>, uint32_t offset, uint32_t count) → std::vector<P>.
   * @param[in,out] a_pool  Pool the packed arrays are reserved from; must outlive this object.
   * @param[in] a_tree      Source tree.
   * @param[in] a_converter Leaf-conversion function.
   */
  template <class Q, class Converter>
  EBGEOMETRY_HOST
  inline PackedBVH(Pool& a_pool, const TreeBVH<T, Q, BV, K>& a_tree, Converter&& a_converter);

  /**
   * @brief Construct directly from a flat primitive list, without ever building a TreeBVH.
   * @details Bypasses TreeBVH entirely: no per-node shared_ptr<TreeBVH> allocation, and no
   * per-primitive shared_ptr allocation either. Primitives are sorted along the space-filling
   * curve @p S (same normalization as TreeBVH::bottomUpSortAndPartition(), via
   * SFC::computeBins()), then split into consecutive leaves and merged bottom-up into a K-ary tree.
   *
   * Every interior node has exactly K children, and such a tree has a leaf count L with
   * L = 1 (mod K - 1). The constructor takes the smallest such L that keeps every leaf at or below
   * @p a_targetLeafSize primitives, and splits the primitives as evenly as possible, so leaf sizes
   * differ by at most one. When that L would exceed the number of primitives, it takes the largest
   * such L below it instead, and leaves may then hold slightly more than the target (for example,
   * five primitives with K = 4 and a target of one give four leaves). When a level of the merge has
   * a node count that is not a multiple of K, the remainder is carried up to the next level. Every
   * node has exactly one parent, so a traversal reaches each primitive exactly once.
   *
   * @tparam S Space-filling curve type (e.g. SFC::Morton, SFC::Nested). Defaults to SFC::Morton;
   * a constructor template's own parameters cannot be explicitly specified the way a named
   * function template's can (constructors have no name of their own to attach a template-argument
   * list to), so @p a_sfc is a stateless tag value purely so @p S can be deduced from it -- pass
   * e.g. @c SFC::Nested{} to select a different curve, or omit it entirely for the default.
   * @param[in] a_primsAndBVs   Primitives and their bounding volumes, taken by value (a sink
   * parameter the caller can std::move in) -- never requires shared_ptr-wrapping.
   * @param[in,out] a_pool Pool the packed arrays are reserved from; must outlive this object.
   * @param[in] a_targetLeafSize Target (maximum, where the leaf-count rule above allows it) number
   * of primitives per leaf. Must be > 0.
   * @param[in] a_sfc Unused tag value; see @p S.
   */
  template <class S = SFC::Morton>
  EBGEOMETRY_HOST
  inline PackedBVH(Pool& a_pool, std::vector<std::pair<P, BV>> a_primsAndBVs, size_t a_targetLeafSize, S a_sfc = S{});

  /**
   * @brief Construct directly from a flat primitive list via top-down (optionally SAH) recursive
   * partitioning, without ever building a TreeBVH.
   * @details Reuses the existing (shared_ptr-based) Partitioner/LeafPredicate machinery --
   * BVCentroidPartitioner, BinnedSAHPartitioner, PrimitiveCentroidPartitioner, or any
   * caller-supplied one -- exactly as TreeBVH::topDownSortAndPartition() does, but writes nodes
   * directly into m_linearNodes in depth-first pre-order as the recursion unwinds, instead of
   * building a persistent, shared_ptr<TreeBVH>-linked tree first. Unlike the SFC-build
   * constructor above, no relayout pass is needed here: top-down recursion visits the root before
   * its children, matching PackedBVH's "root at index 0" invariant for free.
   *
   * This still shared_ptr-wraps each primitive once up front (needed to reuse the existing
   * Partitioner/LeafPredicate signatures, which operate on PrimAndBVList), and a lightweight,
   * stack-local TreeBVH is constructed (and immediately discarded) at every split purely to
   * evaluate the LeafPredicate and read off its primitive list -- exactly the same primitive-handle
   * copying TreeBVH::topDownSortAndPartition() already does at every node. What this constructor
   * avoids is the *persistent*, heap-allocated shared_ptr<TreeBVH> node kept alive for the tree's
   * lifetime at every level -- measured as the dominant cost of the traditional
   * build-then-pack() path (see the "Direct construction" section of the Sphinx docs).
   *
   * @param[in] a_primsAndBVs Primitives and their bounding volumes, taken by value (a sink
   * parameter the caller can std::move in) -- never requires shared_ptr-wrapping by the caller.
   * @param[in,out] a_pool Pool the packed arrays are reserved from; must outlive this object.
   * @param[in] a_partitioner Partitioning function. Divides a (primitive, BV) list into K non-empty
   * sub-lists (an empty one aborts; see TreeBVH::topDownSortAndPartition()). The overloads without
   * it use BVCentroidPartitioner; pass BinnedSAHPartitioner for an SAH build.
   * @param[in] a_stopCrit Stop function. Returns true when a node should become a leaf. The
   * overloads without it use DefaultLeafPredicate.
   */
  EBGEOMETRY_HOST
  inline PackedBVH(Pool&                                  a_pool,
                   std::vector<std::pair<P, BV>>          a_primsAndBVs,
                   const BVH::Partitioner<P, BV, K>&      a_partitioner,
                   const BVH::LeafPredicate<T, P, BV, K>& a_stopCrit);

  /**
   * @brief Construct directly from a primitive list, top-down, with DefaultLeafPredicate.
   * @details Same as the four-argument constructor with @c a_stopCrit = DefaultLeafPredicate. These
   * overloads stand in for default arguments, which would make this header depend on the
   * partitioners in EBGeometry_TreeBVH.hpp.
   * @param[in,out] a_pool        Pool the packed arrays are reserved from; must outlive this object.
   * @param[in]     a_primsAndBVs Primitives and their bounding volumes.
   * @param[in]     a_partitioner Partitioning function.
   */
  EBGEOMETRY_HOST
  inline PackedBVH(Pool&                             a_pool,
                   std::vector<std::pair<P, BV>>     a_primsAndBVs,
                   const BVH::Partitioner<P, BV, K>& a_partitioner);

  /**
   * @brief Construct directly from a primitive list, top-down, with BVCentroidPartitioner and
   * DefaultLeafPredicate.
   * @param[in,out] a_pool        Pool the packed arrays are reserved from; must outlive this object.
   * @param[in]     a_primsAndBVs Primitives and their bounding volumes.
   */
  EBGEOMETRY_HOST
  inline PackedBVH(Pool& a_pool, std::vector<std::pair<P, BV>> a_primsAndBVs);

  /**
   * @brief Construct directly via ClusterSAH: cluster primitives, then SAH over the clusters.
   * @details A single-threaded build that gets close to full-SAH tree quality at a fraction of the
   * build cost, by shrinking what SAH has to partition. In two passes, both keeping @p P by value
   * (no shared_ptr anywhere): (1) group the primitives into buckets of at most
   * @c a_spec.maxClusterSize each, using a cheap density-adaptive midpoint subdivision that stops as
   * soon as a bucket is small enough -- so buckets are spatially tight and follow the primitive
   * density (robust on surface/clustered data, where a fixed Cartesian grid would overcrowd); (2) run
   * binned SAH top-down over the @em buckets (their bounding boxes) to build the flat node array,
   * with each leaf holding one or a few buckets' primitives. SAH thus partitions ~N/maxClusterSize
   * boxes rather than all N primitives -- the source of the speedup. Requires BV == AABBT<T>;
   * enforced by static_assert at instantiation. Disambiguated from the SFC-build constructor by the
   * @c ClusterSpec parameter type.
   * @param[in] a_primsAndBVs Primitives and their bounding volumes, taken by value (a sink parameter
   * the caller can std::move in) -- never requires shared_ptr-wrapping.
   * @param[in,out] a_pool Pool the packed arrays are reserved from; must outlive this object.
   * @param[in] a_spec Clustering configuration (bucket size). See ClusterSpec.
   */
  EBGEOMETRY_HOST
  inline PackedBVH(Pool& a_pool, std::vector<std::pair<P, BV>> a_primsAndBVs, BVH::ClusterSpec a_spec);

  /**
   * @brief Adopt a node array and primitive array built elsewhere.
   * @details For a specialized builder that produces the flat representation itself -- e.g.
   * PointCloudBVH, which runs its own index-based point-cloud build -- rather than going through a
   * TreeBVH or a list of (primitive, bounding volume) pairs. The arrays are copied into @p a_pool and
   * the SoA child-AABB cache is built from them, exactly as for every other constructor.
   *
   * The node array must be a depth-first pre-order flattening with the root at index 0: every
   * interior node's K children lie strictly after it and inside the array, and every leaf's
   * primitive range lies inside @p a_primitives. refit() and the traversal-depth bound both rely on
   * that shape. It is checked by requireWellFormed(), always on rather than as an EBGEOMETRY_EXPECT,
   * because a malformed array would otherwise surface as an out-of-bounds read in Release; an empty
   * node array (an empty BVH) is accepted only with an empty primitive array.
   * @param[in,out] a_pool        Pool the three arrays are reserved from; must outlive this object.
   * @param[in]     a_linearNodes Flattened node array (copied into the pool).
   * @param[in]     a_primitives  Global primitive list in leaf-traversal order (copied into the pool).
   */
  EBGEOMETRY_HOST
  inline PackedBVH(Pool& a_pool, const std::vector<Node>& a_linearNodes, const std::vector<P>& a_primitives)
  {
    this->finalize(a_pool, a_linearNodes, a_primitives);
  }

  /**
   * @brief Destructor.
   * @details Not virtual: PackedBVH is final and is not used polymorphically. A class that needs a
   * BVH over its own payload holds one by value instead -- see PointCloudBVH.
   */
  inline ~PackedBVH() = default;

  /**
   * @brief Copy constructor.
   * @details Copies the descriptor only: m_linearNodes, m_primitives and m_childAabbSoA are
   * PODVector handles into pool memory, so the copy resolves against the same pool storage as the
   * original rather than owning its own. Nothing is duplicated, and a refit() through either object
   * is seen by both. Use deepCopy() for genuinely independent storage.
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
   * @details Explicitly defaulted: the user-declared destructor above would otherwise suppress
   * the implicitly-generated move constructor.
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
   * @brief Get the global primitive list (in leaf-traversal order).
   * @details The returned span is a resolved address into pool memory. It must not outlive the next
   * Pool::reserve on the owning pool, which may move the block; re-obtain it rather than caching it
   * across a build step.
   * @return Span over m_primitives.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline PODSpan<const P>
  getPrimitives() const noexcept;

  /**
   * @brief Get the global primitive list for modification (in leaf-traversal order).
   * @details The packed BVH owns its primitives outright -- packing copies them out of whatever
   * they were built from -- so this is the only way to move a packed geometry in place. Mutating a
   * primitive's position invalidates every node bounding volume that encloses it, so a caller that
   * moves primitives must follow up with refit() before querying again.
   *
   * Note that changing which leaf a primitive *belongs* to is not supported: the node array's
   * primitive ranges are fixed at build time, and refit() only recomputes bounding volumes. Moving
   * primitives far enough to want a different partitioning needs a rebuild.
   *
   * Same lifetime caveat as the const overload: the span must not outlive the next Pool::reserve.
   * @return Mutable span over m_primitives.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline PODSpan<P>
  getPrimitives() noexcept;

  /**
   * @brief Get the flat node array (depth-first pre-order, root at index 0).
   * @details Read-only: a node's child and primitive offsets define the tree's shape, which is fixed
   * at build time. For a class that holds a PackedBVH and walks it with its own traversal rather
   * than through pruneTraverse() -- PointCloudBVH's seeded self-query, for instance. Same lifetime
   * caveat as getPrimitives(): the span must not outlive the next Pool::reserve.
   * @return Span over the node array; empty for an empty BVH.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline PODSpan<const Node>
  getNodes() const noexcept;

  /**
   * @brief Traversal stack size, in entries, for the current compilation pass.
   * @details The bound traverse() and pruneTraverse() size their stacks by: one entry per level below
   * the root of a tree HostTraversalDepth levels deep in a host pass, and DeviceTraversalDepth levels
   * in a device pass. Every BVH is checked against the host depth when it is built, and against the
   * device depth when rebasedView() produces a device view, so a caller's own traversal that keeps one
   * entry per interior node on the path from the root to the node it visits -- the siblings still to
   * visit at that level -- cannot overflow a stack of this many entries.
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
   * @details Reads through m_control on the host (so a Pool::reserve that moves the block is
   * invisible) and through m_base on device. Exactly one of the two is set; which one is asserted.
   * @return Base address of the pool block holding this BVH.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const void*
  base() const noexcept;

  /**
   * @brief Produce a copy of this BVH that resolves against @p a_pool.
   * @details The one sanctioned crossing, mirroring DCEL::MeshT::rebasedView(). Mirror the pool
   * first, then rebase, then copy the returned value into a kernel -- never copy a host-resident
   * BVH into a kernel and try to repair it afterwards.
   *
   * @code
   * Pool devicePool = Pool::mirror(hostPool, deviceMemoryResource());
   * const auto deviceBVH = bvh->rebasedView(devicePool);
   * myKernel<<<blocks, threads>>>(deviceBVH, ...);
   * @endcode
   *
   * A host-to-host mirror is supported too and follows the target's control block instead of
   * snapshotting its base. The rules are DCEL::MeshT::rebasedView()'s (see PoolLocation::rebasedOnto()).
   * @param[in] a_pool Pool to rebase onto: this BVH's own pool or one in its mirror chain.
   * @return A copy of this BVH resolving against @p a_pool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline PackedBVH
  rebasedView(const Pool& a_pool) const noexcept;

  /**
   * @brief Duplicate this BVH's storage into @p a_dstPool.
   * @details The copy constructor copies descriptors only, leaving both objects resolving against
   * the same pool memory. This is the operation that gives genuinely independent storage.
   * @param[in,out] a_dstPool Pool to reserve the copy's arrays from.
   * @return A BVH with the same structure, backed by @p a_dstPool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline PackedBVH
  deepCopy(Pool& a_dstPool) const;

  /**
   * @brief Get the bounding volume of the root node.
   * @details An empty BVH has no root, so this must not be called on one (an EBGEOMETRY_EXPECT
   * checks it); computeBoundingVolume() handles that case.
   * @return Reference to the root node's bounding volume.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const BV&
  getBoundingVolume() const noexcept;

  /**
   * @brief Compute and return the bounding volume of this BVH.
   * @details Identical to getBoundingVolume(), but returns by value under the
   * computeBoundingVolume() name other bounded objects use (BVHUnion forwards to it), and
   * returns the empty (inverted) box for an empty BVH instead of reading a root that does not exist.
   * @return Root node bounding volume.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline BV
  computeBoundingVolume() const noexcept;

  /**
   * @brief Depth-first traversal with caller-supplied pruning, child order and per-node keys.
   * @details The general traversal, for searches that pruneTraverse() cannot express (its pruning
   * bound is always the squared distance from one point to a child's box). Callable from host and
   * device code: the four callbacks are template parameters, so a device caller passes lambdas or
   * functors, while host code may still pass the std::function aliases BVH::PackedLeafEvaluator,
   * BVH::PrunePredicate, BVH::PackedChildOrderer and BVH::NodeKeyFactory.
   *
   * The visit is depth first, starting at the root with its key from @p a_nodeKeyFactory. At each
   * node:
   *
   * 1. Call @p a_prunePredicate(node, key). If it returns false, the node's subtree is skipped.
   * 2. If the node is a leaf, call @p a_leafEvaluator with the global primitive array, the leaf's
   * primitive offset and its primitive count.
   * 3. Otherwise give each of the K children its key from @p a_nodeKeyFactory, and let
   * @p a_childOrderer reorder the K entries in place. The children are then visited from the last
   * entry to the first, each subtree finished before the next child is visited.
   *
   * The pruning test runs when a child is visited, not when its parent is expanded, so a leaf visited
   * in between can tighten it. For a nearest-distance search, order the children farthest first.
   * An empty BVH visits nothing.
   *
   * The stack keeps one entry per level, not per child: the parent's index and the order of the
   * children still to visit, sized from traversalStackDepth(), which every build has checked the tree
   * against. It does not keep the keys. A child visited after its first sibling gets its key from
   * @p a_nodeKeyFactory again, so the factory must give the same key for the same node and have no
   * side effects, and @p a_childOrderer must only reorder the entries: a key it changes is seen only
   * by the child visited first.
   *
   * @tparam NodeKey        Per-node key type. Deduced from @p a_nodeKeyFactory's return type when
   * not given explicitly.
   * @tparam LeafEvaluator  Callable as (PODSpan<const P> primitives, size_t offset, size_t count).
   * @tparam PrunePredicate Callable as (const Node&, const NodeKey&) -> bool; true descends.
   * @tparam ChildOrderer   Callable as (Array<BVH::NodeAndKey<NodeKey>, K>&); reorders in place.
   * @tparam NodeKeyFactory Callable as (const Node&) -> NodeKey.
   * @param[in] a_leafEvaluator  Called at each leaf that is not pruned.
   * @param[in] a_prunePredicate Called at each node as it is popped; return false to prune it.
   * @param[in] a_childOrderer   Reorders the K children of a node before they are visited; the last
   * element after reordering is visited first.
   * @param[in] a_nodeKeyFactory Produces a node's key; called for the root, for every child of every
   * interior node that is expanded, and again for each child visited after its first sibling.
   */
  template <class NodeKey = void, class LeafEvaluator, class PrunePredicate, class ChildOrderer, class NodeKeyFactory>
  EBGEOMETRY_HOST_DEVICE
  inline void
  traverse(LeafEvaluator&&  a_leafEvaluator,
           PrunePredicate&& a_prunePredicate,
           ChildOrderer&&   a_childOrderer,
           NodeKeyFactory&& a_nodeKeyFactory) const noexcept;

  /**
   * @brief Generic SIMD-accelerated, distance-pruned traversal.
   * @details Same box-pruning strategy as @c traverse() above (skip subtrees already farther than
   * the current best; visit the closest-looking child first), but the box-vs-point distance test
   * is vectorised across all @c K children at once (@c if @c constexpr dispatch on @c (K, T) in
   * computeChildDistances2(); a scalar loop over the same SoA cache when no compiled ISA path
   * matches), and the
   * search itself is expressed through three caller-supplied pieces instead of four fixed
   * callbacks. @c State is whatever the search remembers between leaf visits. @c LeafEvaluator is
   * called only at leaves and is the sole place @c State may change. @c PruneDistSquared turns the
   * *current* @c State into a squared-distance pruning bound; it is re-read fresh at every node
   * visited (never cached), so a leaf visited anywhere earlier immediately tightens the pruning
   * applied to nodes visited afterwards, regardless of which subtree it came from.
   * Splitting the bound from the leaf scan like this is what lets a primitive with no notion of
   * "signed distance" reuse the same SIMD box test -- e.g. a nearest-neighbor search can track a
   * plain running squared distance in @c State (no @c abs(), no sqrt anywhere in the hot path),
   * while MeshSDF::signedDistance() and TriMeshSDF::signedDistance() track a signed value and
   * square its magnitude for the bound. Both are ordinary instantiations of this one method.
   *
   * The children of an expanded node within the bound are sorted nearest first, and the nearest is
   * visited at once. The stack keeps one entry per level for the rest: the parent's index and the
   * order of the children still to visit. When the traversal returns to that level, it computes the
   * next child's distance again from the parent's row of child boxes and compares it with the current
   * bound; if it is out of bound, so is every child after it, and the level is dropped.
   *
   * @note Only the pruning *bound* (@c PruneDistSquared) is customisable; the per-child quantity it is
   * compared against -- Euclidean squared distance to a child's AABB -- is not, since that is the
   * one computation vectorised uniformly across @c K children, and branch-and-bound pruning is
   * only sound if it is a true lower bound on the distance to anything inside that box. PackedBVH
   * also hardcodes AABBT<T> as its sole bounding volume. See
   * https://github.com/rmrsk/EBGeometry/issues/96 for a sketch of what generalizing this would
   * look like, if that is ever needed.
   *
   * @tparam State        Caller-defined running search state, carried by reference through the whole traversal.
   * @tparam LeafEvaluator Callable: (State&, size_t offset, size_t count) noexcept -> void. Scans
   * primitives [offset, offset+count) and updates a_state in place.
   * @tparam PruneDistSquared    Callable: (const State&) noexcept -> T. Returns the current squared-distance
   * pruning bound derived from a_state; a node farther than this (in squared distance) is pruned.
   * @param[in]     a_point      Query point.
   * @param[in,out] a_state      Running search state; mutated by a_evalLeaf, read by a_pruneDist2.
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
   * @brief Refit every node's bounding volume in place after the primitives have moved.
   * @details The flat-array counterpart of TreeBVH::refit(): keeps the node array, the primitive
   * array, and every leaf's primitive range exactly as they are, recomputing only the bounding
   * volumes. Because m_linearNodes is a depth-first pre-order flattening (root at index 0, every
   * child at a higher index than its parent), a single reverse sweep over the array refits children
   * before parents with no recursion or explicit stack: each leaf's box becomes the union of its
   * primitives' boxes (recomputed via @p a_bvConstructor), and each interior node's box the union of
   * its K children's freshly-refitted boxes. The per-node SoA AABB cache used by the SIMD traversal
   * is rebuilt at the end, so pruneTraverse() stays consistent.
   *
   * Same use and caveats as TreeBVH::refit(): cheap per-frame maintenance for a geometry whose
   * primitives shift without migrating between leaves, and not a substitute for a rebuild once a
   * deformation is large enough to degrade the tree. @p a_bvConstructor receives each primitive
   * (drawn from the primitive array) and must return its current bounding volume.
   *
   * @tparam BVConstructor Callable: (const P&) -> BV, returning one primitive's current bounding volume.
   * @param[in] a_bvConstructor Bounding-volume constructor for a single primitive.
   */
  template <class BVConstructor>
  inline void
  refit(const BVConstructor& a_bvConstructor);

private:
  /**
   * @brief Copy a completed host-side build into pool storage and rebuild the SoA cache.
   * @details The single finalize path shared by every constructor. Each of them assembles the node
   * and primitive arrays in ordinary std::vectors first -- a host-only build step, where growth is
   * natural and the final sizes are not always known up front -- and then hands them here to be
   * reserved and copied into the pool in one shot. Nothing that crosses to a device is ever built
   * incrementally.
   * @param[in,out] a_pool        Pool to reserve from.
   * @param[in]     a_linearNodes Completed flat node array.
   * @param[in]     a_primitives  Completed primitive array.
   */
  EBGEOMETRY_HOST
  inline void
  finalize(Pool& a_pool, const std::vector<Node>& a_linearNodes, const std::vector<P>& a_primitives);

  /**
   * @brief Flat depth-first node array.
   */
  PODVector<Node> m_linearNodes;

  /**
   * @brief Global primitive list in leaf-traversal order.
   */
  PODVector<P> m_primitives;

  /**
   * @brief Alignment of one SoA axis row, in bytes.
   * @details A row is @c sizeof(T)*K bytes wide, and every ISA path in computeChildDistances2()
   * loads a whole row with one aligned SIMD load, so the row wants to be aligned to its own width.
   * That width is only a legal alignment when it is a power of two, which it is for exactly the
   * (T, K) combinations that have a SIMD path -- 16, 32 and 64 bytes. For every other K (3, 5, 6,
   * 7, ...) no SIMD path is compiled, nothing performs an aligned vector load on the row, and the
   * natural alignment of T is both sufficient and legal. Asking for @c sizeof(T)*K unconditionally
   * is what made @c PackedBVH<double, P, 3> and its odd-K siblings fail to compile at all
   * ("requested alignment 24 is not a positive power of 2").
   */
  static constexpr size_t s_soaRowAlignment =
    (((sizeof(T) * K) & ((sizeof(T) * K) - 1)) == 0) ? (sizeof(T) * K) : alignof(T);

  /**
   * @brief SoA layout of K children's AABBs for a single interior node.
   * @details m_lo[axis][child] / m_hi[axis][child] layout places each axis row in a
   * contiguous T[K] buffer that can be loaded as a single SIMD register.
   * Private implementation detail of PackedBVH; never exposed in any public interface.
   */
  struct ChildAABBSoA
  {
    /**
     * @brief Lower corners: m_lo[axis][child], axis in {0,1,2}, child in {0,…,K-1}.
     */
    alignas(s_soaRowAlignment) T m_lo[3][K];

    /**
     * @brief Upper corners: m_hi[axis][child], axis in {0,1,2}, child in {0,…,K-1}.
     */
    alignas(s_soaRowAlignment) T m_hi[3][K];
  };

  /**
   * @brief SoA child-box rows used by the SIMD traversal in pruneTraverse(): one per interior node,
   * found through Node::m_childBoxRow.
   */
  PODVector<ChildAABBSoA> m_childAabbSoA;

public:
  /**
   * @brief The BVH's arrays as offsets into its pool: a PackedBVH minus its location.
   */
  struct Layout
  {
    /**
     * @brief Flat depth-first node array.
     */
    PODVector<Node> m_linearNodes;

    /**
     * @brief Global primitive list in leaf-traversal order.
     */
    PODVector<P> m_primitives;

    /**
     * @brief SoA child-box rows, one per interior node.
     */
    PODVector<ChildAABBSoA> m_childAabbSoA;
  };

private:
  /**
   * @brief One level on the traversal stack of traverse() and pruneTraverse(): 8 bytes at K = 4.
   * @details An interior node whose first child is being visited, and the child slots still to visit
   * after it, in visit order from the back: m_slots[m_count - 1] is next. A traversal returning to
   * this level reads the next child's offset (and, in pruneTraverse(), its box) from the node, so the
   * entry holds neither, and no key or distance either: those are computed again when the child is
   * visited.
   */
  struct StackEntry
  {
    /// @brief Index into m_linearNodes of the interior node.
    uint32_t m_node;

    /// @brief Child slots still to visit; the last of the first m_count is next.
    uint8_t m_slots[K - 1];

    /// @brief Number of slots in m_slots still to visit, at least 1 while the entry is on the stack.
    uint8_t m_count;
  };

  static_assert(K != 4 || sizeof(StackEntry) == 8, "PackedBVH: a stack entry is 8 bytes at K = 4");

  /**
   * @brief Stack entries needed to traverse a tree @p a_depth levels deep.
   * @details One entry per interior node on the path from the root to the node being visited, so a
   * tree of @p a_depth levels needs a_depth - 1 entries.
   * @param[in] a_depth Tree depth in node levels (at least 2).
   * @return Number of stack entries.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static constexpr size_t
  stackEntriesFor(const size_t a_depth) noexcept
  {
    return a_depth - 1;
  }

  /**
   * @brief Maximum root-to-leaf depth of the finalized node array (root counts as depth 1).
   * @details One O(number of child slots) walk of the pre-order array, used only by the build-time
   * traversal-stack bound below. Host-only: it is a build/mirror step, never a query step, so it
   * may use std::vector for its own working stack.
   * @param[in] a_base Base address the node array resolves against.
   * @return Depth of the deepest leaf, or 0 for an empty BVH.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline size_t
  maxNodeDepth(const void* a_base) const;

  /**
   * @brief Abort if the finalized tree is deeper than @p a_maxDepth levels.
   * @details Always on, not EBGEOMETRY_EXPECT: the failure it prevents is an out-of-bounds write
   * into pruneTraverse's fixed stack, which Release builds would otherwise perform silently. Same
   * reasoning, and the same shape, as Pool::reserve's moved-from check.
   * @param[in] a_base     Base address the node array resolves against.
   * @param[in] a_maxDepth Deepest allowed tree, in node levels: HostTraversalDepth or DeviceTraversalDepth.
   * @param[in] a_context  Short label naming the caller, for the diagnostic.
   */
  EBGEOMETRY_HOST
  inline void
  requireDepthFits(const void* a_base, const size_t a_maxDepth, const char* a_context) const;

  /**
   * @brief Abort unless a node array is a well-formed pre-order flattening.
   * @details Run on every build, by finalize(), so it guards the library's own builders as well as
   * the public adopt constructor. Always on, for the same reason as requireDepthFits(). Checks that
   * every interior node's children lie strictly after it and inside the array (which also rules out
   * cycles), that every node but the root has exactly one parent, that every leaf's primitive range
   * lies inside the primitive array, and that an empty node array comes with an empty primitive
   * array. A leaf with no primitives reads as an interior node whose children are all node 0, so the
   * first check rejects it.
   * @param[in] a_linearNodes   Node array to validate.
   * @param[in] a_numPrimitives Size of the primitive array it indexes into.
   */
  EBGEOMETRY_HOST
  static inline void
  requireWellFormed(const std::vector<Node>& a_linearNodes, const size_t a_numPrimitives);

  /**
   * @brief Compute the squared distances from a query point to all K children of one interior node.
   * @details The single vectorised kernel of pruneTraverse(), and the only part of that traversal
   * that differs between instruction sets. Dispatches at compile time on (T, K) and the compiled
   * ISA: AVX-512F for (double, K=8) and (float, K=16), AVX for (double, K=4), (float, K=8) and
   * (double, K=8) as two 4-wide passes, SSE4.1 for (float, K=4), and a scalar loop for every other
   * combination and for device compilation. Every path computes the same quantity in the same
   * association order -- max(0, max(lo-p, p-hi)) per axis, then dx*dx + (dy*dy + dz*dz) -- so all of
   * them agree bit-for-bit for finite inputs (a NaN coordinate can propagate differently).
   * @param[in]  a_soa   SoA child bounding boxes for the node being expanded.
   * @param[in]  a_point Query point.
   * @param[out] a_dist2 Receives one squared distance per child. Needs no particular alignment --
   * every SIMD path writes it with an unaligned store.
   */
  EBGEOMETRY_HOST_DEVICE
  static inline void
  computeChildDistances2(const ChildAABBSoA& a_soa, const Vec3T<T>& a_point, T (&a_dist2)[K]) noexcept;

  /**
   * @brief The squared distance from a query point to one child's box, bit for bit the value
   * computeChildDistances2() gives that child.
   * @details What pruneTraverse() compares with the bound when it returns to a level, so that the
   * comparison sees exactly the distances the level was sorted by. A device pass has only the scalar
   * path and computes the one child; a host pass recomputes the row through the path that sorted it.
   * @param[in] a_soa   SoA child bounding boxes of the node.
   * @param[in] a_point Query point.
   * @param[in] a_slot  Child slot, below K.
   * @return Squared distance from @p a_point to child @p a_slot's box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline T
  childDistance2(const ChildAABBSoA& a_soa, const Vec3T<T>& a_point, size_t a_slot) noexcept;

  /**
   * @brief The scalar path of computeChildDistances2(), for one child.
   * @param[in] a_soa   SoA child bounding boxes of the node.
   * @param[in] a_point Query point.
   * @param[in] a_slot  Child slot, below K.
   * @return Squared distance from @p a_point to child @p a_slot's box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline T
  scalarChildDistance2(const ChildAABBSoA& a_soa, const Vec3T<T>& a_point, size_t a_slot) noexcept;

  /**
   * @brief Populate m_childAabbSoA from the completed m_linearNodes array, one row per interior node.
   * @details Called from finalize() once m_linearNodes is fully built, and again by refit(). Assigns
   * each interior node's Node::m_childBoxRow.
   * @param[in,out] a_pool Pool to reserve the cache from, or nullptr to refill a cache that already
   * exists (refit's case, where the node count cannot have changed).
   */
  EBGEOMETRY_HOST
  inline void
  buildSoA(Pool* a_pool);
};

/**
 * @brief A PackedBVH must be trivially copyable: that is what lets a rebasedView() be byte-copied
 * into a device address space with no pointer patching. Asserted on concrete instantiations at both
 * precisions, exactly as DCEL::MeshT does.
 */
static_assert(std::is_trivially_copyable_v<PackedBVH<float, Vec3T<float>, 4>>,
              "PackedBVH<float, ...> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<PackedBVH<double, Vec3T<double>, 4>>,
              "PackedBVH<double, ...> must be trivially copyable");

} // namespace BVH

} // namespace EBGeometry

#include "EBGeometry_PackedBVHImplem.hpp"

#endif
