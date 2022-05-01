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
#include <functional>
#include <queue>

// Our includes
#include "EBGeometry_Vec.hpp"
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
  @brief Alias to cut down on typing.
  @details P is the primitive bounded by the BVH.
*/
template <class P>
using PrimitiveListT = std::vector<std::shared_ptr<const P>>;

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
  @brief Polymorphic partitioner for splitting a list of primitives into K new
  lists of primitives
  @details P is the primitive type bound in the BVH and K is the BVH degree.
  @param[in] a_primitives List of primitives to be subdivided into sub-bounding
  volumes.
  @return Returns a list (std::array) of new primitives which make up the new
  bounding volumes.
*/
template <class P, class BV, size_t K>
using PartitionerT = std::function<std::array<PrimitiveListT<P>, K>(const PrimitiveListT<P>& a_primitives)>;

/*!
  @brief Constructor method for creating bounding volumes from a list of
  primitives
  @details P is the primitive type bound in the BVH and BV is the bounding
  volume type.
  @param[in] a_primitives List of primitives.
  @return Returns a new bounding volumes which is guaranteed to enclose all the
  input primitives.
*/
template <class P, class BV>
using BVConstructorT = std::function<BV(const std::shared_ptr<const P>& a_primitive)>;

/*!
  @brief Typename for identifying algorithms various algorithms during tree
  traversel.
  @details Stack     => Use stack/priority queue (ordered traversal).
           Ordered   => Use recursive ordered traversal.
           Unordered => Use recursive unordered traversal.
*/
enum class Prune
{
  Stack,
  Ordered,
  Unordered,
};

/*!
  @brief Class which encapsulates a node in a bounding volume hierarchy.
  @details T is the precision, P is the primitive type you want to enclose, BV
  is the bounding volume type used at the nodes. The parameter K (which must be
  > 1) is the tree degree. K=2 is a binary tree, K=3 is a tertiary tree and so
  on.
  @note Template constraints are as following:

  P MUST supply function signedDistance(...) .
  BV MUST supply a function getDistance

  Had this been C++20, we would have use concepts to enforce this.
*/
template <class T, class P, class BV, size_t K>
class NodeT : public SignedDistanceFunction<T>
{
public:
  /*!
    @brief Alias for cutting down on typing. This is a
    std::vector<std::shared_ptr<const P> >.
  */
  using PrimitiveList = PrimitiveListT<P>;

  /*!
    @brief Alias for cutting down on typing.
  */
  using Vec3 = Vec3T<T>;

  /*!
    @brief Alias for cutting down on typing.
  */
  using Node = NodeT<T, P, BV, K>;

  /*!
    @brief Alias for cutting down on typing.
  */
  using NodePtr = std::shared_ptr<Node>;

  /*!
    @brief Alias for cutting down on typing.
  */
  using StopFunction = StopFunctionT<T, P, BV, K>;

  /*!
    @brief Alias for cutting down on typing
  */
  using Partitioner = PartitionerT<P, BV, K>;

  /*!
    @brief Alias for cutting down on typing.
  */
  using BVConstructor = BVConstructorT<P, BV>;

  /*!
    @brief Default constructor which sets a regular node.
  */
  NodeT();

  /*!
    @brief Construct node from a set of primitives.
    @details This node becomes a leaf node which contains the input primitives.
    @param[in] a_primitives Input primitives.
  */
  NodeT(const std::vector<std::shared_ptr<P>>& a_primitives);

  /*!
    @brief Construct node from a set of primitives.
    @details This node becomes a leaf node which contains the input primitives.
    @param[in] a_primitives Input primitives.
  */
  NodeT(const std::vector<std::shared_ptr<const P>>& a_primitives);

  /*!
    @brief Destructor (does nothing)
  */
  virtual ~NodeT();

  /*!
    @brief Function for using top-down construction of the bounding volume
    hierarchy.
    @details The rules for terminating the hierarchy construction, how to
    partition sets of primitives, and how to enclose them by bounding volumes
    are given in the input arguments (a_stopFunc, a_partFunc, a_bvFunc)
    @param[in] a_bvConstructor Polymorphic function which builds a bounding
    volume from a set of primitives.
    @param[in] a_partitioner   Partitioning function. This is a polymorphic
    function which divides a set of primitives into two lists.
    @param[in] a_stopCrit      Termination function which tells us when to stop
    the recursion.
  */
  inline void
  topDownSortAndPartitionPrimitives(
    const BVConstructor& a_bvConstructor, const Partitioner& a_partitioner, const StopFunction& a_stopCrit) noexcept;

  /*!
    @brief Get node type
  */
  inline bool
  isLeaf() const noexcept;

  /*!
    @brief Get the primitives stored in this node.
    @return m_primitives.
  */
  inline const PrimitiveList&
  getPrimitives() const noexcept;

  /*!
    @brief Get bounding volume
    @return m_bv
  */
  inline const BV&
  getBoundingVolume() const noexcept;

  /*!
    @brief Return this node's children.
    @return m_children.
  */
  inline const std::array<std::shared_ptr<NodeT<T, P, BV, K>>, K>&
  getChildren() const noexcept;

  /*!
    @brief Function which computes the signed distance
    @param[in] a_point   3D point in space
    @return Signed distance to the input point.
  */
  inline T
  signedDistance(const Vec3T<T>& a_point) const noexcept override;

  /*!
    @brief Function which computes the signed distance. This version allows the
    user to manually select a traversal algorithm.
    @param[in] a_point   3D point in space
    @param[in] a_pruning Pruning algorithm
    @return Signed distance to the input point.
  */
  inline T
  signedDistance(const Vec3T<T>& a_point, const Prune a_pruning) const noexcept;

  /*!
    @brief Flatten everything beneath this node into a depth-first sorted BVH
    hierarchy.
    @details This will compute the flattening of the standard BVH tree and
    return a pointer to the linear corresponding to the current node.
  */
  inline std::shared_ptr<LinearBVH<T, P, BV, K>>
  flattenTree() const noexcept;

protected:
  /*!
    @brief Bounding volume object.
  */
  BV m_boundingVolume;

  /*!
    @brief Primitives list. This will be empty for regular nodes
  */
  std::vector<std::shared_ptr<const P>> m_primitives;

  /*!
    @brief Children nodes
  */
  std::array<std::shared_ptr<NodeT<T, P, BV, K>>, K> m_children;

  /*!
    @brief Insert nodes with primitives.
    @param[in] a_primitives Primitives for children.
  */
  inline void
  insertChildren(const std::array<PrimitiveList, K>& a_primitives) noexcept;

  /*!
    @brief Set primitives in this node
    @param[in] a_primitives Primitives
  */
  inline void
  setPrimitives(const PrimitiveList& a_primitives) noexcept;

  /*!
    @brief Get the distance from a 3D point to the bounding volume
    @param[in] a_point 3D point
    @return Returns distance to bounding volume. A zero distance implies that
    the input point is inside the bounding volume.
  */
  inline T
  getDistanceToBoundingVolume(const Vec3& a_point) const noexcept;

  /*!
    @brief Compute the shortest distance to the primitives in this node.
    @param[in] a_point 3D point
    @return Returns the signed distance to the primitives.
  */
  inline T
  getDistanceToPrimitives(const Vec3& a_point) const noexcept;

  /*!
    @brief Get the list of primitives in this node.
    @return Primitives list
  */
  inline PrimitiveList&
  getPrimitives() noexcept;

  /*!
    @brief Iterative ordered pruning along the BVH tree.
    @param[inout] a_point   Input 3D point
  */
  inline T
  pruneStack(const Vec3& a_point) const noexcept;

  /*!
    @brief Recursively ordered pruning along the BVH tree.
    @param[inout] a_closest Shortest distance to primitives so far.
    @param[inout] a_point   Input 3D point
  */
  inline void
  pruneOrdered(T& a_closest, const Vec3& a_point) const noexcept;

  /*!
    @brief Recursive unordered pruning along the BVH tree.
    @param[inout] a_closest Shortest distance to primitives so far.
    @param[inout] a_point   Input 3D point
  */
  inline void
  pruneUnordered(T& a_closest, const Vec3& a_point) const noexcept;

  /*!
    @brief Flatten tree method.
    @details This function will flatten everything beneath the current node and
    linearize all the nodes and primitives beneath it to a_linearNodes and
    a_sortedPrimitives. This function is called recursively.
    @param[inout] a_linearNodes      BVH nodes, linearized onto a vector.
    @param[inout] a_sortedPrimitives Sorted primitives (in leaf node order).
    @param[inout] a_offset           Supporting integer for figuring out where
    in the tree we are.
    @note When called from the root node, a_linearNodes and a_sortedPrimitives
    should be empty and a_offset=0UL.
  */
  inline size_t
  flattenTree(
    std::vector<std::shared_ptr<LinearNodeT<T, P, BV, K>>>& a_linearNodes,
    std::vector<std::shared_ptr<const P>>& a_sortedPrimitives,
    size_t& a_offset) const noexcept;
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
    @brief Alias for cutting down on typing.
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
  inline T
  getDistanceToPrimitives(
    const Vec3& a_point, const std::vector<std::shared_ptr<const P>>& a_primitives) const noexcept;

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
class LinearBVH : public SignedDistanceFunction<T>
{
public:
  /*!
    @brief Cut down on typing
  */
  using Vec3 = Vec3T<T>;

  /*!
    @brief Alias for cutting down on typing
  */
  using LinearNode = LinearNodeT<T, P, BV, K>;

  /*!
    @brief List of primitives
  */
  using PrimitiveList = std::vector<std::shared_ptr<const P>>;

  /*!
    @brief Disallowed. Use the full constructor please.
  */
  LinearBVH() = delete;

  /*!
    @brief Full constructor. Associates the nodes and primitives.
    @param[in] a_linearNodes Linearized BVH nodes.
    @param[in] a_primitives  Primitives.
  */
  inline LinearBVH(
    const std::vector<std::shared_ptr<const LinearNodeT<T, P, BV, K>>>& a_linearNodes,
    const std::vector<std::shared_ptr<const P>>& a_primitives);

  /*!
    @brief Full constructor. Associates the nodes and primitives.
    @param[in] a_linearNodes Linearized BVH nodes.
    @param[in] a_primitives  Primitives.
  */
  inline LinearBVH(
    const std::vector<std::shared_ptr<LinearNodeT<T, P, BV, K>>>& a_linearNodes,
    const std::vector<std::shared_ptr<const P>>& a_primitives);

  /*!
    @brief Destructor. Does nothing
  */
  inline virtual ~LinearBVH();

  /*!
    @brief Function which computes the signed distance. This calls the other
    version.
    @param[in] a_point   3D point in space
  */
  inline T
  signedDistance(const Vec3& a_point) const noexcept override;

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
