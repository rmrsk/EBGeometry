/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_BVH.hpp
  @brief  Declaration of a bounding volume hierarchy (BVH) class.
  @author Robert Marskar
*/

#ifndef EBGeometry_BVH
#define EBGeometry_BVH

// Std includes
#include <cstdint>
#include <limits>
#include <memory>
#include <vector>
#include <array>
#include <functional>
#include <queue>
#include <algorithm>
#include <type_traits>

#if defined(__SSE4_1__) || defined(__AVX__)
#include <immintrin.h>
#endif

// Our includes
#include "EBGeometry_Vec.hpp"
#include "EBGeometry_SFC.hpp"
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Namespace for various bounding volume hierarchy (BVH) functionality.
*/
namespace BVH {

  /*!
    @brief Enum for specifying whether or not the construction is top-down or bottom-up
  */
  enum class Build
  {
    TopDown,
    Morton,
    Nested
  };

  /*!
    @brief Forward declare the tree BVH node (needed by StopFunctionT and lambdas below).
  */
  template <class T, class P, class BV, size_t K>
  class TreeBVH;

  /*!
    @brief List of primitives.
    @details P is the primitive bounded by the BVH.
  */
  template <class P>
  using PrimitiveListT = std::vector<std::shared_ptr<const P>>;

  /*!
    @brief Alias for a list geometric primitive and BV
  */
  template <class P, class BV>
  using PrimAndBV = std::pair<std::shared_ptr<const P>, BV>;

  /*!
    @brief List of primitives and their bounding volumes.
    @details P is the primitive type and BV is the bounding volume enclosing the implicit surface of each P.
  */
  template <class P, class BV>
  using PrimAndBVListT = std::vector<PrimAndBV<P, BV>>;

  /*!
    @brief Polymorphic partitioner for splitting a list of primitives and BVs into K new subsets.
    @details P is the primitive type bound in the BVH and K is the BVH degrees. BV is the bounding volume type.
    @param[in] a_primsAndBVs Vector of primitives and their bounding volumes.
    @return Return a K-length array of subset lists.
  */
  template <class P, class BV, size_t K>
  using PartitionerT = std::function<std::array<PrimAndBVListT<P, BV>, K>(const PrimAndBVListT<P, BV>& a_primsAndBVs)>;

  /*!
    @brief Stop function for deciding when a BVH node can't be divided into
    sub-volumes.
    @details T is the precision used in the BVH computations, P is the enclosing
    primitive and BV is the bounding volume used in the BVH. K is the tree degree.
    @param[in] a_node BVH node
    @return True if the node can't be divided into subvolumes and false otherwise.
  */
  template <class T, class P, class BV, size_t K>
  using StopFunctionT = std::function<bool(const TreeBVH<T, P, BV, K>& a_node)>;

  /*!
    @brief Updater for tree traversal
    @param[in] a_primitives.
  */
  template <class P>
  using Updater = std::function<void(const PrimitiveListT<P>& a_primitives)>;

  /*!
    @brief Updater for PackedBVH traversal. Receives the full primitive list with an offset and count
    rather than a temporary sublist, avoiding a heap allocation per leaf visit.
  */
  template <class P>
  using LinearUpdater = std::function<void(const PrimitiveListT<P>& a_primitives, size_t a_offset, size_t a_count)>;

  /*!
    @brief Visiter pattern for PackedBVH::traverse. Must return true if we should visit the node and false otherwise.
    @details The Meta template parameter is a door left open to the user for attaching additional data to the
    sorter/visiter pattern.
    @param[in] a_node Node to visit or not
    @param[in] a_meta Meta-data for node visit.
  */
  template <class NodeType, class Meta>
  using Visiter = std::function<bool(const NodeType& a_node, const Meta& a_meta)>;

  /*!
    @brief Sorting criterion for which child node to visit first.
    This takes an input list of child nodes and sorts it. When further into the sub-tree, the first
    node is investigated first, then the second, etc. The Meta template parameter is a door left open to
    the user for attaching additional data to the sorter/visiter pattern.
  */
  template <class NodeType, class Meta, size_t K>
  using Sorter = std::function<void(std::array<std::pair<std::shared_ptr<const NodeType>, Meta>, K>& a_children)>;

  /*!
    @brief Sorting criterion for PackedBVH traversal. Uses 32-bit node indices instead of shared_ptrs,
    halving the stack element size (8 bytes vs 16 bytes for size_t+float).
  */
  template <class Meta, size_t K>
  using LinearSorter = std::function<void(std::array<std::pair<uint32_t, Meta>, K>& a_children)>;

  /*!
    @brief Updater for when user wants to add some meta-data to his BVH traversal.
  */
  template <class NodeType, class Meta>
  using MetaUpdater = std::function<Meta(const NodeType& a_node)>;

  /*!
    @brief Function for splitting a vector of some size into K almost-equal chunks. This is a utility function.
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

  /*!
    @brief Simple partitioner which sorts the primitives based on their centroids, and then splits into K pieces.
    @param[in] a_primsAndBVs Input primitives and their bounding volumes
  */
  template <class T, class P, class BV, size_t K>
  auto PrimitiveCentroidPartitioner =
    [](const PrimAndBVListT<P, BV>& a_primsAndBVs) noexcept -> std::array<PrimAndBVListT<P, BV>, K> {
    Vec3T<T> lo = Vec3T<T>::max();
    Vec3T<T> hi = -Vec3T<T>::max();

    for (const auto& pbv : a_primsAndBVs) {
      lo = min(lo, pbv.first->getCentroid());
      hi = max(hi, pbv.first->getCentroid());
    }

    const size_t splitDir = (hi - lo).maxDir(true);

    // Sort the primitives along the above coordinate direction.
    PrimAndBVListT<P, BV> sortedPrimsAndBVs(a_primsAndBVs);

    std::sort(sortedPrimsAndBVs.begin(),
              sortedPrimsAndBVs.end(),
              [splitDir](const PrimAndBV<P, BV>& pbv1, const PrimAndBV<P, BV>& pbv2) -> bool {
                return pbv1.first->getCentroid(splitDir) < pbv2.first->getCentroid(splitDir);
              });

    return BVH::equalCounts<PrimAndBV<P, BV>, K>(sortedPrimsAndBVs);
  };

  /*!
    @brief Simple partitioner which sorts the BVs based on their bounding volume centroids, and then splits into K pieces.
    @param[in] a_primsAndBVs Input primitives and their bounding volumes
  */
  template <class T, class P, class BV, size_t K>
  auto BVCentroidPartitioner = [](const PrimAndBVListT<P, BV>& a_primsAndBVs) -> std::array<PrimAndBVListT<P, BV>, K> {
    Vec3T<T> lo = Vec3T<T>::max();
    Vec3T<T> hi = -Vec3T<T>::max();

    for (const auto& pbv : a_primsAndBVs) {
      lo = min(lo, pbv.second.getCentroid());
      hi = max(hi, pbv.second.getCentroid());
    }

    const size_t splitDir = (hi - lo).maxDir(true);

    // Sort the primitives along the above coordinate direction.
    PrimAndBVListT<P, BV> sortedPrimsAndBVs(a_primsAndBVs);

    std::sort(sortedPrimsAndBVs.begin(),
              sortedPrimsAndBVs.end(),
              [splitDir](const PrimAndBV<P, BV>& pbv1, const PrimAndBV<P, BV>& pbv2) -> bool {
                return pbv1.second.getCentroid()[splitDir] < pbv2.second.getCentroid()[splitDir];
              });

    return BVH::equalCounts<PrimAndBV<P, BV>, K>(sortedPrimsAndBVs);
  };

  /*!
    @brief Simple stop function which ends the recursion when there aren't enough primitives in the node
    @param[in] a_node Input BVH node
  */
  template <class T, class P, class BV, size_t K>
  auto DefaultStopFunction =
    [](const BVH::TreeBVH<T, P, BV, K>& a_node) noexcept -> bool { return (a_node.getPrimitives()).size() < K; };

  /*!
    @brief SoA layout of K children's AABBs for a single interior PackedBVH node.
    @details lo[axis][child] / hi[axis][child] layout makes each row a contiguous T[K]
    array loadable as a single SIMD register. Used exclusively by PackedBVH.
  */
  template <class T, size_t K>
  struct ChildAabbSoA {
    alignas(sizeof(T) * K) T lo[3][K];
    alignas(sizeof(T) * K) T hi[3][K];
  };

  /*!
    @brief Linearised, AABB-backed BVH with SIMD-accelerated traversal.
    @details PackedBVH is the runtime traversal class. It stores a flat array of Node
    structs (depth-first order) together with the primitive list, and a SoA cache of
    children AABBs that enables SIMD node tests.

    Construct by calling TreeBVH::pack() or TreeBVH::packWith<Q>(converter).

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
    using BV = EBGeometry::BoundingVolumes::AABBT<T>;

    /*!
      @brief Internal BVH node: bounding volume + child/primitive index tables.
      @details This struct is the only internal representation needed for packed traversal.
      It is exposed as a public nested type so that users can write custom traverse()
      visitors (Visiter, MetaUpdater) without naming the old LinearNodeT class.
    */
    struct Node
    {
      BV                       bv{};
      uint32_t                 primOff{};
      uint32_t                 numPrims{};
      std::array<uint32_t, K>  childOff{};

      inline void
      setBoundingVolume(const BV& a_bv) noexcept
      {
        bv = a_bv;
      }
      inline void
      setPrimitivesOffset(uint32_t a_off) noexcept
      {
        primOff = a_off;
      }
      inline void
      setNumPrimitives(uint32_t a_n) noexcept
      {
        numPrims = a_n;
      }
      inline void
      setChildOffset(uint32_t a_off, size_t a_k) noexcept
      {
        childOff[a_k] = a_off;
      }
      inline const BV&
      getBoundingVolume() const noexcept
      {
        return bv;
      }
      inline uint32_t
      getPrimitivesOffset() const noexcept
      {
        return primOff;
      }
      inline uint32_t
      getNumPrimitives() const noexcept
      {
        return numPrims;
      }
      inline const std::array<uint32_t, K>&
      getChildOffsets() const noexcept
      {
        return childOff;
      }
      inline bool
      isLeaf() const noexcept
      {
        return numPrims > 0;
      }
      inline T
      getDistanceToBoundingVolume(const Vec3T<T>& a_point) const noexcept
      {
        return bv.getDistance(a_point);
      }
    };

    using LinearNode = Node; ///< Backward-compat alias; prefer Node in new code.

    PackedBVH() = delete;

    /*!
      @brief Construct by packing a TreeBVH (same primitive type).
      @details Walks the tree depth-first, fills m_linearNodes and m_primitives
      directly, then builds the SoA AABB cache. BV2 must be AABBT<T>.
    */
    template <class BV2>
    inline PackedBVH(const TreeBVH<T, P, BV2, K>& a_tree);

    /*!
      @brief Construct by packing a TreeBVH with primitive-type conversion.
      @details Same DFS as the identity constructor, but each leaf's primitives are
      passed through a_converter: (prims, 0, count) → vector<P> (by value).
      Returned values are stored contiguously with aliased shared_ptrs. BV2 must be AABBT<T>.
    */
    template <class P2, class BV2, class Converter>
    inline PackedBVH(const TreeBVH<T, P2, BV2, K>& a_tree, Converter&& a_converter);

    inline virtual ~PackedBVH() = default;

    /*!
      @brief Get the primitive list (sorted in leaf-traversal order).
    */
    inline const std::vector<std::shared_ptr<const P>>&
    getPrimitives() const noexcept;

    /*!
      @brief Get the bounding volume for the root node.
    */
    inline const BV&
    getBoundingVolume() const noexcept;

    /*!
      @brief Compute the bounding volume of this BVH (root node's BV).
      @details Enables PackedBVH to serve as a primitive in an outer TreeBVH hierarchy.
    */
    inline BV
    computeBoundingVolume() const noexcept;

    /*!
      @brief Recursion-less BVH traversal.
      @param[in] a_updater     Leaf update rule.
      @param[in] a_visiter     Node visit predicate (return true to visit).
      @param[in] a_sorter      Children sort function.
      @param[in] a_metaUpdater Meta-data updater.
    */
    template <class Meta>
    inline void
    traverse(const BVH::LinearUpdater<P>&              a_updater,
             const BVH::Visiter<Node, Meta>&            a_visiter,
             const BVH::LinearSorter<Meta, K>&          a_sorter,
             const BVH::MetaUpdater<Node, Meta>&        a_metaUpdater) const noexcept;

    /*!
      @brief Compute signed distance from a_point to the nearest primitive.
      @details Uses SIMD traversal where available; falls back to scalar otherwise.
    */
    inline T
    signedDistance(const Vec3T<T>& a_point) const noexcept;

  protected:
    /*!
      @brief Flat array of BVH nodes (depth-first order).
    */
    std::vector<Node> m_linearNodes;

    /*!
      @brief Global primitive list, sorted in leaf-traversal order.
    */
    std::vector<std::shared_ptr<const P>> m_primitives;

    /*!
      @brief Per-node SoA cache of K children's AABBs for SIMD node tests.
    */
    std::vector<ChildAabbSoA<T, K>> m_childAabbSoA;

    /*!
      @brief Build the per-node SoA AABB cache from m_linearNodes.
      @details Called at the end of every constructor after m_linearNodes is populated.
    */
    inline void
    buildSoA() noexcept;
  };

  /*!
    @brief Class which encapsulates a node in a bounding volume hierarchy.
    @details T is the precision, P is the primitive type you want to enclose, BV
    is the bounding volume type used at the nodes. The parameter K (which must be
    > 1) is the tree degree. K=2 is a binary tree, K=3 is a tertiary tree and so
    on.
  */
  template <class T, class P, class BV, size_t K>
  class TreeBVH : public std::enable_shared_from_this<TreeBVH<T, P, BV, K>>
  {
  public:
    /*!
      @brief Alias for list of primitives
    */
    using PrimitiveList = PrimitiveListT<P>;

    /*!
      @brief Alias for list of primitives
    */
    using Vec3 = Vec3T<T>;

    /*!
      @brief Alias for node type
    */
    using Node = TreeBVH<T, P, BV, K>;

    /*!
      @brief Alias for node type pointer
    */
    using NodePtr = std::shared_ptr<Node>;

    /*!
      @brief Alias for partitioner
    */
    using Partitioner = PartitionerT<P, BV, K>;

    /*!
      @brief Alias for stop function
    */
    using StopFunction = StopFunctionT<T, P, BV, K>;

    /*!
      @brief Default constructor which sets a regular node.
    */
    TreeBVH() noexcept;

    /*!
      @brief Construct a BVH node from a set of primitives and their bounding volumes
      @param[in] a_primsAndBVs Primitives and their bounding volumes
    */
    TreeBVH(const std::vector<PrimAndBV<P, BV>>& a_primsAndBVs) noexcept;

    /*!
      @brief Destructor (does nothing)
    */
    virtual ~TreeBVH() noexcept;

    /*!
      @brief Function for using top-down construction of the bounding volume hierarchy.
      @details The rules for terminating the hierarchy construction, and how to partition them
      are encoded in the input arguments (a_partitioner, a_stopCrit).
      @param[in] a_partitioner Partitioning function. This is a polymorphic function which divides a set of
      primitives into two or more sub-lists.
      @param[in] a_stopCrit Termination function which tells us when to stop the recursion.
    */
    inline void
    topDownSortAndPartition(const Partitioner&  a_partitioner = BVCentroidPartitioner<T, P, BV, K>,
                            const StopFunction& a_stopCrit    = DefaultStopFunction<T, P, BV, K>) noexcept;

    /*!
      @brief Function for doing bottom-up construction using a specified space-filling curve.
      @details The template parameter is the space-filling curve type. This function will partition the BVH
      by first sorting the bounding volume centroids along the space-filling curve. The tree is then constructed
      by placing at least K primitives in each leaf, and the leaves are then merged upwards until we reach the
      root node.
      @note S must have an encode and decode function which returns an SFC index. See the SFC namespace for
      examples for Morton and Nested indices.
    */
    template <typename S>
    inline void
    bottomUpSortAndPartition() noexcept;

    /*!
      @brief Get node type
    */
    inline bool
    isLeaf() const noexcept;

    /*!
      @brief Check if BVH is already partitioned
    */
    inline bool
    isPartitioned() const noexcept;

    /*!
      @brief Get bounding volume
      @return m_bv
    */
    inline const BV&
    getBoundingVolume() const noexcept;

    /*!
      @brief Get the primitives stored in this node.
      @return m_primitives.
    */
    inline const PrimitiveList&
    getPrimitives() const noexcept;

    /*!
      @brief Get the bounding volumes for the primitives in this node (can be empty if regular node)
      @return m_boundingVolumes
    */
    inline const std::vector<BV>&
    getBoundingVolumes() const noexcept;

    /*!
      @brief Get the distance from a 3D point to the bounding volume
      @param[in] a_point 3D point
      @return Returns distance to bounding volume. A zero distance implies that
      the input point is inside the bounding volume.
    */
    inline T
    getDistanceToBoundingVolume(const Vec3& a_point) const noexcept;

    /*!
      @brief Return this node's children.
      @return m_children.
    */
    inline const std::array<std::shared_ptr<TreeBVH<T, P, BV, K>>, K>&
    getChildren() const noexcept;

    /*!
      @brief Recursion-less BVH traversal algorithm.
      The user inputs the update rule, a pruning criterion, and a criterion of who to visit first.
      @param[in] a_updater     Update rule (for updating whatever the user is interested in updated)
      @param[in] a_visiter     Visiter rule. Must return true if we should visit the node.
      @param[in] a_sorter      Children sort function for deciding which subtrees and investigated first.
      @param[in] a_metaUpdater Updater for meta-information.
    */
    template <class Meta>
    inline void
    traverse(const BVH::Updater<P>&              a_updater,
             const BVH::Visiter<Node, Meta>&     a_visiter,
             const BVH::Sorter<Node, Meta, K>&   a_sorter,
             const BVH::MetaUpdater<Node, Meta>& a_metaUpdater) const noexcept;

    /*!
      @brief Flatten this tree into a PackedBVH with the same primitive type.
      @details BV must be AABBT<T>; a static_assert enforces this at instantiation.
    */
    inline std::shared_ptr<PackedBVH<T, P, K>>
    pack() const noexcept;

    /*!
      @brief Flatten and convert this tree into a PackedBVH with a different primitive type Q.
      @details a_converter is called once per leaf:
        (prims, offset, count) → std::vector<Q>   (primitives by value)
      packWith accumulates all returned values into a single contiguous buffer and exposes
      them through aliased shared_ptrs, preserving cache locality. BV must be AABBT<T>.
    */
    template <class Q, class Converter>
    inline std::shared_ptr<PackedBVH<T, Q, K>>
    packWith(Converter&& a_converter) const noexcept;

  protected:
    /*!
      @brief Bounding volume object for enclosing everything in this node.
    */
    BV m_boundingVolume;

    /*!
      @brief Determines whether or not the partitioning function has already been called
    */
    bool m_partitioned;

    /*!
      @brief Primitives list. This will be empty for regular nodes
    */
    std::vector<std::shared_ptr<const P>> m_primitives;

    /*!
      @brief List of bounding volumes for the primitives. This will be empty for regular nodes
    */
    std::vector<BV> m_boundingVolumes;

    /*!
      @brief Children nodes
    */
    std::array<std::shared_ptr<TreeBVH<T, P, BV, K>>, K> m_children;

    /*!
      @brief Get the list of primitives in this node.
      @return Primitives list
    */
    inline PrimitiveList&
    getPrimitives() noexcept;

    /*!
      @brief Get the bounding volumes for the primitives in this node (can be empty if regular node)
      @return m_boundingVolumes
    */
    inline std::vector<BV>&
    getBoundingVolumes() noexcept;

    /*!
      @brief Explicitly set this node's children.
      @details This will turn this node into the parent node of the input children, i.e. a regular node.
      @param[in] a_children Child nodes.
    */
    inline void
    setChildren(const std::array<std::shared_ptr<TreeBVH<T, P, BV, K>>, K>& a_children) noexcept;

  };

} // namespace BVH

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_BVHImplem.hpp"

#endif
