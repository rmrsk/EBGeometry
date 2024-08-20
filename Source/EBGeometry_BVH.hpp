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
#include <memory>
#include <vector>
#include <array>
#include <functional>
#include <queue>

// Our includes
#include "EBGeometry_Vec.hpp"
#include "EBGeometry_SFC.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Namespace for various bounding volume hierarchy (BVH) functionality.
*/
namespace BVH {

  /*!
    @brief Forward declare the BVH node since it is needed for the polymorphic
    lambdas.
    @details T is the precision used in the BVH computations, P is the enclosing
    primitive and BV is the bounding volume used in the BVH. K is the tree degree.
  */
  template <class T, class P, class BV, size_t K>
  class NodeT;

  /*!
    @brief Forward declare linear node class.
    @details T is the precision used in the BVH computations, P is the enclosing
    primitive and BV is the bounding volume used in the BVH. K is the tree degree.
  */
  template <class T, class P, class BV, size_t K>
  class LinearNodeT;

  /*!
    @brief Forward declare linear BVH class.
    @details T is the precision used in the BVH computations, P is the enclosing
    primitive and BV is the bounding volume used in the BVH. K is the tree degree.
  */
  template <class T, class P, class BV, size_t K>
  class LinearBVH;

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
  using StopFunctionT = std::function<bool(const NodeT<T, P, BV, K>& a_node)>;

  /*!
    @brief Updater for tree traversal
    @param[in] a_primitives. 
  */
  template <class P>
  using Updater = std::function<void(const PrimitiveListT<P>& a_primitives)>;

  /*!
    @brief Visiter pattern for LinearBVH::traverse. Must return true if we should visit the node and false otherwise. 
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
    @brief Updater for when user wants to add some meta-data to his BVH traversal.
  */
  template <class NodeType, class Meta>
  using MetaUpdater = std::function<Meta(const NodeType& a_node)>;

  /*!
    @brief Function for splitting a vector of some size into K almost-equal chunks. This is a utility function.
  */
  template <class X, size_t K>
  auto equalCounts = [](const std::vector<X>& a_primitives) -> std::array<std::vector<X>, K> {
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
    [](const PrimAndBVListT<P, BV>& a_primsAndBVs) -> std::array<PrimAndBVListT<P, BV>, K> {
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
    [](const BVH::NodeT<T, P, BV, K>& a_node) -> bool { return (a_node.getPrimitives()).size() < K; };

  /*!
    @brief Class which encapsulates a node in a bounding volume hierarchy.
    @details T is the precision, P is the primitive type you want to enclose, BV
    is the bounding volume type used at the nodes. The parameter K (which must be
    > 1) is the tree degree. K=2 is a binary tree, K=3 is a tertiary tree and so
    on.
  */
  template <class T, class P, class BV, size_t K>
  class NodeT : public std::enable_shared_from_this<NodeT<T, P, BV, K>>
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
    using Node = NodeT<T, P, BV, K>;

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
    NodeT() noexcept;

    /*!
      @brief Construct a BVH node from a set of primitives and their bounding volumes
      @param[in] a_primsAndBVs Primitives and their bounding volumes
    */
    NodeT(const std::vector<PrimAndBV<P, BV>>& a_primsAndBVs) noexcept;

    /*!
      @brief Destructor (does nothing)
    */
    virtual ~NodeT() noexcept;

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

#if __cplusplus >= 202002L
    /*!
      @brief Function for doing bottom-up construction using a specified space-filling curve.
      @details The template parameter is the space-filling curve type. This function will partition the BVH
      by first sorting the bounding volume centroids along the space-filling curve. The tree is then constructed
      by placing at least K primitives in each leaf, and the leaves are then merged upwards until we reach the
      root node.
    */
    template <SFC::Encodable S>
    inline void
    bottomUpSortAndPartition() noexcept;
#endif

    /*!
      @brief Get node type
    */
    inline bool
    isLeaf() const noexcept;

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
    inline const std::array<std::shared_ptr<NodeT<T, P, BV, K>>, K>&
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
      @brief Flatten everything beneath this node into a depth-first sorted BVH
      hierarchy.
      @details This will compute the flattening of the standard BVH tree and
      return a pointer to the linear corresponding to the current node.
    */
    inline std::shared_ptr<LinearBVH<T, P, BV, K>>
    flattenTree() const noexcept;

    /*!
      @brief Check if BVH is already partitioned
    */
    inline bool
    isPartitioned() const noexcept;

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
    std::array<std::shared_ptr<NodeT<T, P, BV, K>>, K> m_children;

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
      @brief Flatten tree method.
      @details This function will flatten everything beneath the current node and
      linearize all the nodes and primitives beneath it to a_linearNodes and
      a_sortedPrimitives. This function is called recursively.
      @param[in,out] a_linearNodes      BVH nodes, linearized onto a vector.
      @param[in,out] a_sortedPrimitives Sorted primitives (in leaf node order).
      @param[in,out] a_offset           Supporting integer for figuring out where
      in the tree we are.
      @note When called from the root node, a_linearNodes and a_sortedPrimitives
      should be empty and a_offset=0UL.
    */
    inline size_t
    flattenTree(std::vector<std::shared_ptr<LinearNodeT<T, P, BV, K>>>& a_linearNodes,
                std::vector<std::shared_ptr<const P>>&                  a_sortedPrimitives,
                size_t&                                                 a_offset) const noexcept;
  };

  /*!
    @brief Node type for linearized (flattened) BVH. This will be constructed from
    the other (conventional) BVH type.

    @details T is the precision for Vec3, P is the primitive type you want to
    enclose, BV is the bounding volume you use for it.

    @note P MUST supply function signedDistance(...) BV must supply a function
    getDistance (had this been C++20, we would have use concepts to enforce this).
    Note that LinearNode is the result of a flattened BVH hierarchy where nodes
    are stored with depth-first ordering for improved cache-location in the
    downward traversal.

    @note This class exists so that we can fit the nodes with a smaller memory
    footprint. The standard BVH node (NodeT) is very useful when building the tree
    but less useful when traversing it since it stores references to the
    primitives in the node itself. It will span multiple cache lines. This node
    exists so that we can fit all the BVH info onto fewer cache lines. The number
    of cache lines will depend on the tree degree, precision, and bounding volume
    that is chosen.

    @todo There's a minor optimization that can be made to the memory alignment,
    which is as follows: For a leaf node we never really need the m_childOffsets
    array, and for a regular node we never really need the m_primitivesOffset
    member. Moreover, m_childOffsets could be made into a K-1 sized array because
    we happen to know that the linearized hierarchy will store the first child
    node immediately after the regular node. We could shave off 16 bytes of
    storage, which would mean that a double-precision binary tree only takes up
    one word of CPU memory.
  */
  template <class T, class P, class BV, size_t K>
  class LinearNodeT
  {
  public:
    /*!
      @brief Alias for vector type
    */
    using Vec3 = Vec3T<T>;

    /*!
      @brief Constructor.
    */
    inline LinearNodeT() noexcept;

    /*!
      @brief Destructor.
    */
    inline virtual ~LinearNodeT();

    /*!
      @brief Set the bounding volume
      @param[in] a_boundingVolume Bounding volume for this node.
    */
    inline void
    setBoundingVolume(const BV& a_boundingVolume) noexcept;

    /*!
      @brief Set the offset into the primitives array.
    */
    inline void
    setPrimitivesOffset(const size_t a_primitivesOffset) noexcept;

    /*!
      @brief Set number of primitives.
      @param[in] a_numPrimitives Number of primitives.
    */
    inline void
    setNumPrimitives(const size_t a_numPrimitives) noexcept;

    /*!
      @brief Set the child offsets.
      @param[in] a_childOffset Offset in node array.
      @param[in] a_whichChild  Child index in m_childrenOffsets. Must be [0,K-1]
    */
    inline void
    setChildOffset(const size_t a_childOffset, const size_t a_whichChild) noexcept;

    /*!
      @brief Get the node bounding volume.
      return m_boundingVolume
    */
    inline const BV&
    getBoundingVolume() const noexcept;

    /*!
      @brief Get the primitives offset
      @return Returns m_primitivesOffset
    */
    inline const size_t&
    getPrimitivesOffset() const noexcept;

    /*!
      @brief Get the number of primitives.
      @return Returns m_numPrimitives
    */
    inline const size_t&
    getNumPrimitives() const noexcept;

    /*!
      @brief Get the child offsets
      @return Returns m_childOffsets
    */
    inline const std::array<size_t, K>&
    getChildOffsets() const noexcept;

    /*!
      @brief Is leaf or not
    */
    inline bool
    isLeaf() const noexcept;

    /*!
      @brief Get the distance from a 3D point to the bounding volume
      @param[in] a_point 3D point
      @return Returns distance to bounding volume. A zero distance implies that
      the input point is inside the bounding volume.
    */
    inline T
    getDistanceToBoundingVolume(const Vec3& a_point) const noexcept;

    /*!
      @brief Compute signed distance to primitives.
      @param[in] a_point      Point
      @param[in] a_primitives List of primitives
      @note Only call if this is a leaf node.
    */
    inline std::vector<T>
    getDistances(const Vec3& a_point, const std::vector<std::shared_ptr<const P>>& a_primitives) const noexcept;

  protected:
    /*!
      @brief Bounding volume.
    */
    BV m_boundingVolume;

    /*!
      @brief Offset into primitives array
    */
    size_t m_primitivesOffset;

    /*!
      @brief Number of primitives
    */
    size_t m_numPrimitives;

    /*!
      @brief Offset to child nodes.
    */
    std::array<size_t, K> m_childOffsets;
  };

  /*!
    @brief Linear root node for BVH hierarchy
  */
  template <class T, class P, class BV, size_t K>
  class LinearBVH
  {
  public:
    /*!
      @brief Alias for vector type
    */
    using Vec3 = Vec3T<T>;

    /*!
      @brief Alias for linear node type
    */
    using LinearNode = LinearNodeT<T, P, BV, K>;

    /*!
      @brief Alias for list of primitives
    */
    using PrimitiveList = std::vector<std::shared_ptr<const P>>;

    /*!
      @brief Disallowed. Use the full constructor please.
    */
    inline LinearBVH() = default;

    /*!
      @brief Copy constructor
    */
    inline LinearBVH(const LinearBVH&) = default;

    /*!
      @brief Full constructor. Associates the nodes and primitives.
      @param[in] a_linearNodes Linearized BVH nodes.
      @param[in] a_primitives  Primitives.
    */
    inline LinearBVH(const std::vector<std::shared_ptr<const LinearNodeT<T, P, BV, K>>>& a_linearNodes,
                     const std::vector<std::shared_ptr<const P>>&                        a_primitives);

    /*!
      @brief Full constructor. Associates the nodes and primitives.
      @param[in] a_linearNodes Linearized BVH nodes.
      @param[in] a_primitives  Primitives.
    */
    inline LinearBVH(const std::vector<std::shared_ptr<LinearNodeT<T, P, BV, K>>>& a_linearNodes,
                     const std::vector<std::shared_ptr<const P>>&                  a_primitives);

    /*!
      @brief Destructor. Does nothing
    */
    inline virtual ~LinearBVH();

    /*!
      @brief Get the bounding volume for this BVH. 
    */
    inline const BV&
    getBoundingVolume() const noexcept;

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
    traverse(const BVH::Updater<P>&                    a_updater,
             const BVH::Visiter<LinearNode, Meta>&     a_visiter,
             const BVH::Sorter<LinearNode, Meta, K>&   a_sorter,
             const BVH::MetaUpdater<LinearNode, Meta>& a_metaUpdater) const noexcept;

  protected:
    /*!
      @brief List of linearly stored nodes
    */
    std::vector<std::shared_ptr<const LinearNodeT<T, P, BV, K>>> m_linearNodes;

    /*!
      @brief Global list of primitives. Note that this is ALL primitives, sorted
      so that LinearNodeT can interface into it.
    */
    std::vector<std::shared_ptr<const P>> m_primitives;
  };
} // namespace BVH

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_BVHImplem.hpp"

#endif
