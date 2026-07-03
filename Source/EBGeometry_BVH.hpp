/** EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/**
  @file   EBGeometry_BVH.hpp
  @brief  Declaration of bounding volume hierarchy (BVH) classes.
  @author Robert Marskar
*/

#ifndef EBGeometry_BVH
#define EBGeometry_BVH

// Std includes
#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <queue>
#include <type_traits>
#include <vector>

#if defined(__SSE4_1__) || defined(__AVX__)
#include <immintrin.h>
#endif

// Our includes
#include "EBGeometry_Vec.hpp"
#include "EBGeometry_SFC.hpp"
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/**
  @brief Namespace for various bounding volume hierarchy (BVH) functionalities.
*/
namespace BVH {

  /**
    @brief Enum for specifying the BVH construction strategy.
  */
  enum class Build
  {
    TopDown, ///< Recursive top-down partitioning.
    Morton,  ///< Bottom-up construction along a Morton space-filling curve.
    Nested   ///< Bottom-up construction along a Nested space-filling curve.
  };

  /**
    @brief Forward declaration of the tree-structured BVH. Needed by StopFunction and the
    default-partitioner lambdas defined below.
  */
  template <class T, class P, class BV, size_t K>
  class TreeBVH;

  /**
    @brief Forward declaration of the linearised BVH. Needed so that TreeBVH::pack() and
    TreeBVH::packWith() can name their return types before PackedBVH is fully defined.
  */
  template <class T, class P, size_t K>
  class PackedBVH;

  /**
    @brief Convenience alias for a list of shared primitive pointers.
    @tparam P Primitive type bounded by the BVH.
  */
  template <class P>
  using PrimitiveList = std::vector<std::shared_ptr<const P>>;

  /**
    @brief Convenience alias for a (primitive, bounding-volume) pair.
    @tparam P  Primitive type.
    @tparam BV Bounding volume type.
  */
  template <class P, class BV>
  using PrimAndBV = std::pair<std::shared_ptr<const P>, BV>;

  /**
    @brief Convenience alias for a list of (primitive, bounding-volume) pairs.
    @tparam P  Primitive type.
    @tparam BV Bounding volume type enclosing the implicit surface of each primitive.
  */
  template <class P, class BV>
  using PrimAndBVList = std::vector<PrimAndBV<P, BV>>;

  /**
    @brief Polymorphic partitioner: splits a list of (primitive, BV) pairs into K sub-lists.
    @tparam P  Primitive type.
    @tparam BV Bounding volume type.
    @tparam K  Tree branching factor.
    @param[in] a_primsAndBVs Input primitives and their bounding volumes.
    @return K-element array of sub-lists.
  */
  template <class P, class BV, size_t K>
  using Partitioner = std::function<std::array<PrimAndBVList<P, BV>, K>(const PrimAndBVList<P, BV>& a_primsAndBVs)>;

  /**
    @brief Predicate for deciding when a TreeBVH node should become a leaf (i.e., no further splitting).
    @tparam T  Floating-point precision.
    @tparam P  Primitive type.
    @tparam BV Bounding volume type.
    @tparam K  Tree branching factor.
    @param[in] a_node BVH node under consideration.
    @return True if the node should not be sub-divided further.
  */
  template <class T, class P, class BV, size_t K>
  using StopFunction = std::function<bool(const TreeBVH<T, P, BV, K>& a_node)>;

  /**
    @brief Leaf-update callback for TreeBVH::traverse.
    @details Called once for every leaf node visited during traversal.
    @tparam P Primitive type.
    @param[in] a_primitives Primitive list stored in the leaf.
  */
  template <class P>
  using Updater = std::function<void(const PrimitiveList<P>& a_primitives)>;

  /**
    @brief Leaf-update callback for PackedBVH::traverse.
    @details Receives a view into the global primitive array (offset + count) rather than a
    temporary sub-list, avoiding a heap allocation per leaf visit.
    @tparam P Primitive type.
    @param[in] a_primitives Global primitive list.
    @param[in] a_offset     Index of the first primitive belonging to this leaf.
    @param[in] a_count      Number of primitives in this leaf.
  */
  template <class P>
  using LinearUpdater = std::function<void(const PrimitiveList<P>& a_primitives, size_t a_offset, size_t a_count)>;

  /**
    @brief Node-visit predicate for BVH traversal.
    @details Must return true to descend into the node and false to prune it.
    @tparam NodeType Node type (TreeBVH or PackedBVH::Node).
    @tparam Meta     Caller-supplied auxiliary data type attached to each stack entry
                     (e.g. a running minimum distance).
    @param[in] a_node Node under consideration.
    @param[in] a_meta Caller-supplied meta-data for this stack entry.
    @return True if the subtree rooted at a_node should be visited.
  */
  template <class NodeType, class Meta>
  using Visiter = std::function<bool(const NodeType& a_node, const Meta& a_meta)>;

  /**
    @brief Child-ordering callback for TreeBVH traversal.
    @details Sorts an array of (child-node-pointer, meta) pairs in-place so that
    the most promising child is visited first.
    @tparam NodeType Node type (TreeBVH).
    @tparam Meta     Auxiliary data type attached to each stack entry.
    @tparam K        Tree branching factor.
    @param[in,out] a_children K child nodes together with their meta-data.
  */
  template <class NodeType, class Meta, size_t K>
  using Sorter = std::function<void(std::array<std::pair<std::shared_ptr<const NodeType>, Meta>, K>& a_children)>;

  /**
    @brief Child-ordering callback for PackedBVH traversal.
    @details Same role as Sorter but uses 32-bit node indices instead of shared_ptrs,
    halving the stack-entry size.
    @tparam Meta Auxiliary data type attached to each stack entry.
    @tparam K    Tree branching factor.
    @param[in,out] a_children K (node-index, meta) pairs to sort.
  */
  template <class Meta, size_t K>
  using LinearSorter = std::function<void(std::array<std::pair<uint32_t, Meta>, K>& a_children)>;

  /**
    @brief Meta-data factory called once per node during BVH traversal.
    @details Produces the Meta value that will be passed to Visiter and Sorter for
    each child of the current node.
    @tparam NodeType Node type (TreeBVH or PackedBVH::Node).
    @tparam Meta     Auxiliary data type to produce.
    @param[in] a_node Current node.
    @return Meta value for a_node's children.
  */
  template <class NodeType, class Meta>
  using MetaUpdater = std::function<Meta(const NodeType& a_node)>;

  /**
    @brief Utility: split a vector into K almost-equal contiguous chunks.
    @tparam X Element type.
    @tparam K Number of chunks.
    @param[in] a_primitives Input vector.
    @return Array of K sub-vectors whose sizes differ by at most 1.
  */
  template <class X, size_t K>
  auto equalCounts = [](const std::vector<X>& a_primitives) noexcept -> std::array<std::vector<X>, K> {
    int length = a_primitives.size() / K;
    int remain = a_primitives.size() % K;

    int begin = 0;
    int end   = 0;

    std::array<std::vector<X>, K> chunks;

    for (size_t k = 0; k < K; k++) {
      end += (remain > 0) ? length + 1 : length;
      remain--;

      chunks[k] = std::vector<X>(a_primitives.begin() + begin, a_primitives.begin() + end);

      begin = end;
    }

    return chunks;
  };

  /**
    @brief Partitioner that sorts primitives by centroid along the longest axis and splits into K pieces.
    @tparam T  Floating-point precision used for centroid comparisons.
    @tparam P  Primitive type.
    @tparam BV Bounding volume type.
    @tparam K  Number of output sub-lists (tree branching factor).
    @param[in] a_primsAndBVs Input (primitive, BV) pairs.
    @return K sub-lists.
  */
  template <class T, class P, class BV, size_t K>
  auto PrimitiveCentroidPartitioner =
    [](const PrimAndBVList<P, BV>& a_primsAndBVs) noexcept -> std::array<PrimAndBVList<P, BV>, K> {
    Vec3T<T> lo = Vec3T<T>::max();
    Vec3T<T> hi = -Vec3T<T>::max();

    for (const auto& pbv : a_primsAndBVs) {
      lo = min(lo, pbv.first->getCentroid());
      hi = max(hi, pbv.first->getCentroid());
    }

    const size_t splitDir = (hi - lo).maxDir(true);

    PrimAndBVList<P, BV> sortedPrimsAndBVs(a_primsAndBVs);

    std::sort(sortedPrimsAndBVs.begin(),
              sortedPrimsAndBVs.end(),
              [splitDir](const PrimAndBV<P, BV>& pbv1, const PrimAndBV<P, BV>& pbv2) -> bool {
                return pbv1.first->getCentroid(splitDir) < pbv2.first->getCentroid(splitDir);
              });

    return BVH::equalCounts<PrimAndBV<P, BV>, K>(sortedPrimsAndBVs);
  };

  /**
    @brief Partitioner that sorts primitives by bounding-volume centroid along the longest axis and splits into K pieces.
    @tparam T  Floating-point precision used for centroid comparisons.
    @tparam P  Primitive type.
    @tparam BV Bounding volume type.
    @tparam K  Number of output sub-lists (tree branching factor).
    @param[in] a_primsAndBVs Input (primitive, BV) pairs.
    @return K sub-lists.
  */
  template <class T, class P, class BV, size_t K>
  auto BVCentroidPartitioner = [](const PrimAndBVList<P, BV>& a_primsAndBVs) -> std::array<PrimAndBVList<P, BV>, K> {
    Vec3T<T> lo = Vec3T<T>::max();
    Vec3T<T> hi = -Vec3T<T>::max();

    for (const auto& pbv : a_primsAndBVs) {
      lo = min(lo, pbv.second.getCentroid());
      hi = max(hi, pbv.second.getCentroid());
    }

    const size_t splitDir = (hi - lo).maxDir(true);

    PrimAndBVList<P, BV> sortedPrimsAndBVs(a_primsAndBVs);

    std::sort(sortedPrimsAndBVs.begin(),
              sortedPrimsAndBVs.end(),
              [splitDir](const PrimAndBV<P, BV>& pbv1, const PrimAndBV<P, BV>& pbv2) -> bool {
                return pbv1.second.getCentroid()[splitDir] < pbv2.second.getCentroid()[splitDir];
              });

    return BVH::equalCounts<PrimAndBV<P, BV>, K>(sortedPrimsAndBVs);
  };

  /**
    @brief Default stop function: stop partitioning when the node holds fewer than K primitives.
    @tparam T  Floating-point precision.
    @tparam P  Primitive type.
    @tparam BV Bounding volume type.
    @tparam K  Tree branching factor.
    @param[in] a_node BVH node.
    @return True if the node has fewer than K primitives.
  */
  template <class T, class P, class BV, size_t K>
  auto DefaultStopFunction =
    [](const BVH::TreeBVH<T, P, BV, K>& a_node) noexcept -> bool { return (a_node.getPrimitives()).size() < K; };

  /**
    @brief Tree-structured BVH node used during construction.
    @details Each node stores a bounding volume, a list of primitives (non-empty for leaf
    nodes only), and K child pointers (non-null for interior nodes only).

    Build the tree by constructing a root node from a set of (primitive, BV) pairs and
    then calling topDownSortAndPartition() or bottomUpSortAndPartition(). Once built, call
    pack() to obtain a cache-friendly PackedBVH for traversal.

    @tparam T  Floating-point precision.
    @tparam P  Primitive type. Must provide getCentroid() and signedDistance(Vec3T<T>).
    @tparam BV Bounding volume type.
    @tparam K  Tree branching factor (must be >= 2).
  */
  template <class T, class P, class BV, size_t K>
  class TreeBVH : public std::enable_shared_from_this<TreeBVH<T, P, BV, K>>
  {
  public:
    /**
      @brief Alias for the primitive list type.
    */
    using PrimitiveList = BVH::PrimitiveList<P>;

    /**
      @brief Alias for the 3D vector type.
    */
    using Vec3 = Vec3T<T>;

    /**
      @brief Alias for this node type (used in traversal callbacks).
    */
    using Node = TreeBVH<T, P, BV, K>;

    /**
      @brief Alias for a shared pointer to a node.
    */
    using NodePtr = std::shared_ptr<Node>;

    /**
      @brief Alias for the partitioner type.
    */
    using Partitioner = BVH::Partitioner<P, BV, K>;

    /**
      @brief Alias for the stop-function type.
    */
    using StopFunction = BVH::StopFunction<T, P, BV, K>;

    /**
      @brief Default constructor. Creates an empty interior node.
    */
    TreeBVH() noexcept;

    /**
      @brief Construct a leaf node from a set of (primitive, BV) pairs.
      @param[in] a_primsAndBVs Primitives and their bounding volumes.
    */
    TreeBVH(const std::vector<PrimAndBV<P, BV>>& a_primsAndBVs) noexcept;

    /**
      @brief Destructor.
    */
    virtual ~TreeBVH() noexcept;

    /**
      @brief Recursively partition this node top-down.
      @details The stop criterion and partitioner determine the tree shape.
      @param[in] a_partitioner Partitioning function. Divides a (primitive, BV) list into K sub-lists.
      @param[in] a_stopCrit    Stop function. Returns true when a node should become a leaf.
    */
    inline void
    topDownSortAndPartition(const Partitioner&  a_partitioner = BVCentroidPartitioner<T, P, BV, K>,
                            const StopFunction& a_stopCrit    = DefaultStopFunction<T, P, BV, K>) noexcept;

    /**
      @brief Recursively partition this node bottom-up along a space-filling curve.
      @details S must provide encode() and decode() functions returning SFC indices.
      Primitives are sorted by their bounding-volume centroid projected onto the curve,
      then grouped into leaves of size K and merged upwards to the root.
      @tparam S Space-filling curve type (e.g. Morton, Nested).
    */
    template <typename S>
    inline void
    bottomUpSortAndPartition() noexcept;

    /**
      @brief Return true if this is a leaf node (no children, non-empty primitive list).
    */
    inline bool
    isLeaf() const noexcept;

    /**
      @brief Return true if the tree has already been partitioned.
    */
    inline bool
    isPartitioned() const noexcept;

    /**
      @brief Get the bounding volume for this node.
      @return Reference to m_boundingVolume.
    */
    inline const BV&
    getBoundingVolume() const noexcept;

    /**
      @brief Get the primitives stored in this node.
      @details Non-empty only for leaf nodes.
      @return Reference to m_primitives.
    */
    inline const PrimitiveList&
    getPrimitives() const noexcept;

    /**
      @brief Get the per-primitive bounding volumes stored in this node.
      @details Non-empty only for leaf nodes.
      @return Reference to m_boundingVolumes.
    */
    inline const std::vector<BV>&
    getBoundingVolumes() const noexcept;

    /**
      @brief Get the distance from a_point to this node's bounding volume.
      @details Returns zero if a_point is inside the bounding volume.
      @param[in] a_point Query point.
      @return Distance to the bounding volume surface, or zero if inside.
    */
    inline T
    getDistanceToBoundingVolume(const Vec3& a_point) const noexcept;

    /**
      @brief Get the K child nodes of this interior node.
      @details All K children are non-null for interior nodes; the array is unused for leaf nodes.
      @return Reference to m_children.
    */
    inline const std::array<std::shared_ptr<TreeBVH<T, P, BV, K>>, K>&
    getChildren() const noexcept;

    /**
      @brief Recursion-free BVH traversal using an explicit LIFO stack (depth-first order).
      @details The traversal maintains a stack of (node, Meta) pairs. It is seeded with the
      root node paired with @p a_metaUpdater applied to the root. On each iteration:

      1. Pop the top (node, meta) pair.
      2. Call @p a_visiter(node, meta). If it returns false the entire subtree rooted at
         that node is skipped (pruned) and the loop continues with the next stack entry.
      3. If the node is a leaf, call @p a_updater with the leaf's primitive list.
      4. If the node is an interior node:
         a. Compute a Meta value for each of the K children by calling
            @p a_metaUpdater on each child.
         b. Collect the K (child, Meta) pairs into a local array and pass them to
            @p a_sorter, which reorders the array in-place.
         c. Push all K pairs onto the stack in sorted order.

      Because the stack is LIFO, the child pushed last is visited first. @p a_sorter
      should therefore place the most promising child last in the array. For a
      nearest-distance query this means sorting children in descending order of
      distance — farthest child first, nearest child last — so the nearest child
      sits on top of the stack and is expanded next.

      @tparam Meta Auxiliary data type carried on the traversal stack (e.g. a running minimum distance).
      @param[in] a_updater     Called at each leaf with the leaf's primitive list.
      @param[in] a_visiter     Called at each node; return true to descend, false to prune.
      @param[in] a_sorter      Reorders the K (child, Meta) pairs in-place before they are
                               pushed; the last element after sorting is visited first.
      @param[in] a_metaUpdater Produces a Meta value for a node; called once for the root
                               and once per child of every interior node that is visited.
    */
    template <class Meta>
    inline void
    traverse(const BVH::Updater<P>&              a_updater,
             const BVH::Visiter<Node, Meta>&     a_visiter,
             const BVH::Sorter<Node, Meta, K>&   a_sorter,
             const BVH::MetaUpdater<Node, Meta>& a_metaUpdater) const noexcept;

    /**
      @brief Flatten this tree into a cache-friendly PackedBVH with the same primitive type.
      @details Requires BV == AABBT<T>; enforced by static_assert at instantiation.
      @return Shared pointer to the resulting PackedBVH.
    */
    inline std::shared_ptr<PackedBVH<T, P, K>>
    pack() const noexcept;

    /**
      @brief Flatten and convert this tree into a PackedBVH with a different primitive type Q.
      @details a_converter is called once per leaf:
        a_converter(leafPrims, 0, count) → std::vector<Q>
      All returned values are accumulated into a single contiguous buffer, then exposed
      through aliased shared_ptrs to preserve cache locality.
      Requires BV == AABBT<T>; enforced by static_assert at instantiation.
      @tparam Q         Destination primitive type.
      @tparam Converter Callable: (PrimitiveList<P>, uint32_t offset, uint32_t count) → std::vector<Q>.
      @param[in] a_converter Leaf-conversion function.
      @return Shared pointer to the resulting PackedBVH<T, Q, K>.
    */
    template <class Q, class Converter>
    inline std::shared_ptr<PackedBVH<T, Q, K>>
    packWith(Converter&& a_converter) const noexcept;

  protected:
    /**
      @brief Bounding volume enclosing all primitives in this subtree.
    */
    BV m_boundingVolume;

    /**
      @brief True after topDownSortAndPartition() or bottomUpSortAndPartition() has been called.
    */
    bool m_partitioned;

    /**
      @brief Primitives stored in this node. Non-empty for leaf nodes only.
    */
    std::vector<std::shared_ptr<const P>> m_primitives;

    /**
      @brief Per-primitive bounding volumes. Non-empty for leaf nodes only.
    */
    std::vector<BV> m_boundingVolumes;

    /**
      @brief K child nodes. Non-null for interior nodes only.
    */
    std::array<std::shared_ptr<TreeBVH<T, P, BV, K>>, K> m_children;

    /**
      @brief Non-const accessor for the primitive list (used during construction).
      @return Reference to m_primitives.
    */
    inline PrimitiveList&
    getPrimitives() noexcept;

    /**
      @brief Non-const accessor for the per-primitive bounding volumes (used during construction).
      @return Reference to m_boundingVolumes.
    */
    inline std::vector<BV>&
    getBoundingVolumes() noexcept;

    /**
      @brief Set the K child nodes, converting this node from a leaf into an interior node.
      @param[in] a_children New child nodes.
    */
    inline void
    setChildren(const std::array<std::shared_ptr<TreeBVH<T, P, BV, K>>, K>& a_children) noexcept;
  };

  /**
    @brief Linearised, AABB-backed BVH with SIMD-accelerated traversal.
    @details PackedBVH is the runtime query class. It stores a flat depth-first array of
    Node structs, a contiguous primitive list, and a per-node SoA AABB cache that enables
    SIMD child tests.

    Instances are obtained by calling TreeBVH::pack() (same primitive type) or
    TreeBVH::packWith<Q>(converter) (type-converting pack).

    SIMD paths are selected at compile time via if constexpr:
      - K==4, T==float  → SSE4.1 (__m128)
      - K==4, T==double → AVX    (__m256d)
      - K==8, T==float  → AVX    (__m256)
      - K==8, T==double → AVX    (two __m256d passes)
    All other combinations fall back to scalar traversal.

    @tparam T Floating-point precision.
    @tparam P Primitive type. Must provide signedDistance(Vec3T<T>).
    @tparam K BVH branching factor.
  */
  template <class T, class P, size_t K>
  class PackedBVH
  {
  public:
    /**
      @brief AABB type used for all bounding volumes in this BVH.
    */
    using BV = EBGeometry::BoundingVolumes::AABBT<T>;

    /**
      @brief Compact BVH node stored in the flat node array.
      @details Each node holds a bounding volume, a primitive range (leaves) or child
      index table (interior nodes). Exposed as a public nested type so that users can
      write traverse() callbacks (Visiter, MetaUpdater) that inspect nodes.
    */
    struct Node
    {
      /**
        @brief Axis-aligned bounding box for this node's subtree.
      */
      BV bv{};

      /**
        @brief Index of the first primitive in the global primitive list (leaf nodes only).
      */
      uint32_t primOff{};

      /**
        @brief Number of primitives in this leaf (zero for interior nodes).
      */
      uint32_t numPrims{};

      /**
        @brief Depth-first indices of the K child nodes (interior nodes only).
      */
      std::array<uint32_t, K> childOff{};

      /**
        @brief Set the bounding volume for this node.
        @param[in] a_bv Bounding volume.
      */
      inline void
      setBoundingVolume(const BV& a_bv) noexcept
      {
        bv = a_bv;
      }

      /**
        @brief Set the primitive offset for this leaf node.
        @param[in] a_off Index into the global primitive list.
      */
      inline void
      setPrimitivesOffset(uint32_t a_off) noexcept
      {
        primOff = a_off;
      }

      /**
        @brief Set the primitive count for this leaf node.
        @param[in] a_n Number of primitives.
      */
      inline void
      setNumPrimitives(uint32_t a_n) noexcept
      {
        numPrims = a_n;
      }

      /**
        @brief Set the depth-first index of the k-th child.
        @param[in] a_off Node index of the child.
        @param[in] a_k   Child slot (0 … K-1).
      */
      inline void
      setChildOffset(uint32_t a_off, size_t a_k) noexcept
      {
        childOff[a_k] = a_off;
      }

      /**
        @brief Get the bounding volume.
        @return Reference to bv.
      */
      inline const BV&
      getBoundingVolume() const noexcept
      {
        return bv;
      }

      /**
        @brief Get the primitive offset (leaf nodes only).
        @return Index of the first primitive in the global list.
      */
      inline uint32_t
      getPrimitivesOffset() const noexcept
      {
        return primOff;
      }

      /**
        @brief Get the primitive count.
        @return Number of primitives; zero for interior nodes.
      */
      inline uint32_t
      getNumPrimitives() const noexcept
      {
        return numPrims;
      }

      /**
        @brief Get the child index table.
        @return Reference to the K-element child-offset array.
      */
      inline const std::array<uint32_t, K>&
      getChildOffsets() const noexcept
      {
        return childOff;
      }

      /**
        @brief Return true if this is a leaf node.
      */
      inline bool
      isLeaf() const noexcept
      {
        return numPrims > 0;
      }

      /**
        @brief Get the distance from a_point to this node's bounding volume.
        @param[in] a_point Query point.
        @return Distance to the bounding-box surface, or zero if inside.
      */
      inline T
      getDistanceToBoundingVolume(const Vec3T<T>& a_point) const noexcept
      {
        return bv.getDistance(a_point);
      }
    };

    /**
      @brief Deleted default constructor. Use TreeBVH::pack() or TreeBVH::packWith() to construct.
    */
    PackedBVH() = delete;

    /**
      @brief Construct by packing a TreeBVH (identity primitive type).
      @details Walks the tree depth-first, fills m_linearNodes and m_primitives directly,
      then builds the SoA AABB cache. Requires BV2 == AABBT<T>.
      @tparam BV2 Bounding volume type of the source tree (must equal AABBT<T>).
      @param[in] a_tree Source tree.
    */
    template <class BV2>
    inline PackedBVH(const TreeBVH<T, P, BV2, K>& a_tree);

    /**
      @brief Construct by packing a TreeBVH with primitive-type conversion.
      @details Walks the tree depth-first. For each leaf, calls
        a_converter(leafPrims, 0, count) → std::vector<P>
      All returned values are stored contiguously; shared_ptrs alias into that buffer.
      Requires BV2 == AABBT<T>.
      @tparam P2        Source primitive type.
      @tparam BV2       Bounding volume type of the source tree (must equal AABBT<T>).
      @tparam Converter Callable: (PrimitiveList<P2>, uint32_t, uint32_t) → std::vector<P>.
      @param[in] a_tree      Source tree.
      @param[in] a_converter Leaf-conversion function.
    */
    template <class P2, class BV2, class Converter>
    inline PackedBVH(const TreeBVH<T, P2, BV2, K>& a_tree, Converter&& a_converter);

    /**
      @brief Virtual destructor.
    */
    inline virtual ~PackedBVH() = default;

    /**
      @brief Get the global primitive list (in leaf-traversal order).
      @return Reference to m_primitives.
    */
    inline const std::vector<std::shared_ptr<const P>>&
    getPrimitives() const noexcept;

    /**
      @brief Get the bounding volume of the root node.
      @return Reference to the root node's bounding volume.
    */
    inline const BV&
    getBoundingVolume() const noexcept;

    /**
      @brief Compute and return the bounding volume of this BVH.
      @details Identical to getBoundingVolume() but presents a getCentroid()-compatible
      interface, enabling PackedBVH to serve as a primitive in an outer TreeBVH hierarchy.
      @return Root node bounding volume.
    */
    inline BV
    computeBoundingVolume() const noexcept;

    /**
      @brief Recursion-free BVH traversal using a vector-backed LIFO stack (depth-first order).
      @details The traversal mirrors TreeBVH::traverse() in structure but works directly on the
      flat node array, using 32-bit indices instead of shared_ptr<const Node>. This halves the
      stack-entry size and avoids reference-count traffic on every push and pop.

      The stack is a std::vector used as a LIFO queue via push_back/pop_back, pre-reserved to
      64 entries to avoid reallocation for typical tree depths. It is seeded with node index 0
      (the root) paired with @p a_metaUpdater applied to the root node. On each iteration:

      1. Pop the back entry to obtain a (nodeIdx, meta) pair.
      2. Look up the node at m_linearNodes[nodeIdx].
      3. Call @p a_visiter(node, meta). If it returns false the entire subtree rooted at
         that node is skipped (pruned) and the loop continues.
      4. If the node is a leaf, call @p a_updater with the global primitive list
         m_primitives, the leaf's primitive offset, and its primitive count. The updater
         receives a view into the shared list rather than a freshly allocated sub-list,
         avoiding a heap allocation per leaf visit.
      5. If the node is an interior node:
         a. Collect the K child indices from node.getChildOffsets(), look each child up in
            m_linearNodes, and call @p a_metaUpdater on each to produce a Meta value.
         b. Bundle the K (childIdx, Meta) pairs into a local array and pass it to
            @p a_sorter, which reorders the array in-place.
         c. Push all K pairs onto the back of the stack in sorted order.

      Because the stack is LIFO, the child pushed last is visited first. @p a_sorter
      should therefore place the most promising child last in the array. For a
      nearest-distance query this means sorting children in descending order of
      distance — farthest child first, nearest child last — so the nearest child
      sits at the back of the vector and is expanded next.

      @tparam Meta Auxiliary data type carried on the traversal stack (e.g. a running minimum distance).
      @param[in] a_updater     Called at each leaf with the global primitive list, offset, and count.
      @param[in] a_visiter     Called at each node; return true to descend, false to prune.
      @param[in] a_sorter      Reorders the K (childIdx, Meta) pairs in-place before they are
                               pushed; the last element after sorting is visited first.
      @param[in] a_metaUpdater Produces a Meta value for a node; called once for the root
                               and once per child of every interior node that is visited.
    */
    template <class Meta>
    inline void
    traverse(const BVH::LinearUpdater<P>&       a_updater,
             const BVH::Visiter<Node, Meta>&    a_visiter,
             const BVH::LinearSorter<Meta, K>&  a_sorter,
             const BVH::MetaUpdater<Node, Meta>& a_metaUpdater) const noexcept;

    /**
      @brief Compute the signed distance from a_point to the nearest primitive.
      @details Uses SIMD traversal where available and falls back to scalar otherwise.
      @param[in] a_point Query point.
      @return Signed distance to the nearest primitive surface.
    */
    inline T
    signedDistance(const Vec3T<T>& a_point) const noexcept;

  protected:
    /**
      @brief Flat depth-first node array.
    */
    std::vector<Node> m_linearNodes;

    /**
      @brief Global primitive list in leaf-traversal order.
    */
    std::vector<std::shared_ptr<const P>> m_primitives;

    /**
      @brief SoA layout of K children's AABBs for a single interior node.
      @details lo[axis][child] / hi[axis][child] layout places each axis row in a
      contiguous T[K] buffer that can be loaded as a single SIMD register.
      Private implementation detail of PackedBVH; never exposed in any public interface.
    */
    struct ChildAABBSoA
    {
      /**
        @brief Lower corners: lo[axis][child], axis in {0,1,2}, child in {0,…,K-1}.
      */
      alignas(sizeof(T) * K) T lo[3][K];

      /**
        @brief Upper corners: hi[axis][child], axis in {0,1,2}, child in {0,…,K-1}.
      */
      alignas(sizeof(T) * K) T hi[3][K];
    };

    /**
      @brief Per-node SoA AABB cache used by the SIMD traversal in signedDistance().
    */
    std::vector<ChildAABBSoA> m_childAabbSoA;

    /**
      @brief Populate m_childAabbSoA from the completed m_linearNodes array.
      @details Called at the end of every constructor after m_linearNodes is fully built.
    */
    inline void
    buildSoA() noexcept;
  };

} // namespace BVH

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_BVHImplem.hpp"

#endif
