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
  @brief Namespace for various bounding volume heirarchy (BVH) functionality.
*/
namespace BVH {

  /*!
    @brief Forward declare the BVH node since it is needed for the polymorphic lambdas. 
    @details T is the precision used in the BVH computations, P is the enclosing primitive and BV is the bounding volume used in the BVH. 
  */
  template <class T, class P, class BV, int K>
  class NodeT;

  /*!
    @brief Alias to cut down on typing. 
  */
  template <class P>
  using PrimitiveListT = std::vector<std::shared_ptr<const P> >;

  /*!
    @brief Stop function for deciding when a BVH node can't be divided into sub-volumes
    @param[in] a_node BVH node
    @return True if the node can't be divided into subvolumes and false otherwise. 
  */
  template <class T, class P, class BV, int K>
  using StopFunctionT = std::function<bool(const NodeT<T, P, BV, K>& a_node)>;

  /*!
    @brief Polymorphic partitioner for splitting a list of primitives into K new lists of primitives
    @param[in] a_primitives List of primitives to be subdivided into sub-bounding volumes. 
    @return Returns a list (std::array) of new primitives which make up the new bounding volumes. 
  */
  template <class P, int K>
  using PartitionerT = std::function<std::array<PrimitiveListT<P>, K>(const PrimitiveListT<P>& a_primitives)>;

  /*!
    @brief Constructor method for creating bounding volumes from a list of primitives
    @param[in] a_primitives List of primitives. 
    @return Returns a new bounding volumes which is guaranteed to enclose all the input primitives.
  */
  template <class P, class BV>
  using BVConstructorT = std::function<BV(const std::shared_ptr<const P>& a_primitive)>;

  /*!
    @brief Enum for determining if a BVH node is a leaf or a regular node (only leaf nodes contain data)
  */
  enum class NodeType : bool {
    Regular,
    Leaf,
  };

  /*!
    @brief Typename for identifying algorithms used in subtree pruning.
  */
  enum class Prune {
    Ordered,
    Ordered2,
    Unordered,
    Unordered2,
  };  

  /*!
    @brief Class which encapsulates a node in a bounding volume hierarchy. 
    @details T is the precision for Vec3, P is the primitive type you want to enclose, BV is the bounding volume you use for it. 
    @note P MUST supply function signedDistance(...) and unsignedDistance2(Vec3). BV must supply a
    function getDistance (had this been C++20, we would have use concepts to enforce this). 
  */
  template <class T, class P, class BV, int K>
  class NodeT {
  public:

    /*!
      @brief Alias for cutting down on typing. 
    */
    using PrimitiveList = PrimitiveListT<P>;

    /*!
      @brief Alias for cutting down on typing
    */
    using Vec3 = Vec3T<T>;

    /*!
      @brief Alias for cutting down on typing. 
      @details In the below, 'Node' is a class which uses precision T for computations and encloses primitives P using a bounding volume BV. 
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
    using Partitioner = PartitionerT<P, K>;    

    /*!
      @brief Alias for cutting down on typing. 
    */                
    using BVConstructor = BVConstructorT<P, BV>;

    /*!
      @brief Default constructor which sets a regular node without any data (no parent/children and no depth)
    */
    NodeT();

    /*!
      @brief Construct node from parent
      @param[in] a_parent Parent node. 
      @details This sets the node's parent to be a_parent and the node's depth to be the parent's depth + 1.
      @note This node becomes a leaf node. 
    */
    NodeT(const NodePtr& a_parent);

    /*!
      @brief Construct node from a set of primitives. 
      @param[in] a_primitives Input primitives. 
      @details This sets the node's parent to be a_parent and the node's depth to be the parent's depth + 1.
      @note This node becomes a leaf node with depth=0 and which contains the input primitives. 
    */    
    NodeT(const std::vector<std::shared_ptr<P> >& a_primitives);

    /*!
      @brief Construct node from a set of primitives. 
      @param[in] a_primitives Input primitives. 
      @note This node becomes a leaf node with depth=0 and which contains the input primitives. 
    */        
    NodeT(const std::vector<std::shared_ptr<const P> >& a_primitives);

    /*!
      @brief Destructor (does nothing)
    */
    ~NodeT();

    /*!
      @brief Function for using top-down construction of the bounding volume hierarchy. 
      @param[in] a_stopFunc Termination function which tells us when to stop the recursion. 
      @param[in] a_partFunc Partitioning function. This is a polymorphic function which divides a set of primitives into two lists. 
      @param[in] a_bvFunc   Polymorphic function which builds a bounding volume from a set of primitives. 
      @details The rules for terminating the hierarchy construction, how to partition sets of primitives, and how to enclose them by bounding volumes are
      given in the input arguments (a_stopFunc, a_partFunc, a_bvFunc)
    */
    inline
    void topDownSortAndPartitionPrimitives(const BVConstructor& a_bvConstructor,
					   const Partitioner&   a_partitioner,
					   const StopFunction&  a_stopCrit) noexcept;


    /*!
      @brief Get the depth of the current node
      @return Depth of current node
    */
    inline
    int getDepth() const noexcept;

    /*!
      @brief Get the primitives stored in this node. 
      @return List of primitives. 
    */
    inline
    const PrimitiveList& getPrimitives() const noexcept;

    /*!
      @brief Get bounding volume
    */
    inline
    const BV& getBoundingVolume() const noexcept;

    /*!
      @brief Function which computes the signed distance
      @param[in] a_point 3D point in space
      @param[in] a_whichPruning Pruning algorithm
      @return Signed distance to the input point
      @details This will select amongs the various implementations. 
    */
    inline
    T signedDistance(const Vec3& a_point, const Prune a_pruning = Prune::Ordered2) const noexcept;    

    /*!
      @brief Function which computes the signed distance using ordered pruning along the BVH branches. 
      @param[in] a_point 3D point in space
      @return Signed distance to the input point
      @details This routine computes the distance to a_point using ordered pruning. We begin at the top (root node of the tree) and descend downwards. This routine
      first descends along the sub-branch with the shortest distance to its bounding volume. Once we hit leaf node we update the shortest distance 'd' found so far.
      As we investigate more branches, they can be pruned if the distance 'd' is shorter than the distance to the node's bounding volume. 
    */
    inline
    T pruneOrdered(const Vec3& a_point) const noexcept;

    /*!
      @brief Function which computes the signed distance using ordered pruning along the BVH branches. 
      @param[in] a_point 3D point in space
      @return Signed distance to the input point
      @details This routine computes the distance to a_point using ordered pruning. We begin at the top (root node of the tree) and descend downwards. This routine
      first descends along the sub-branch with the shortest distance to its bounding volume. Once we hit leaf node we update the shortest distance 'd' found so far.
      As we investigate more branches, they can be pruned if the distance 'd' is shorter than the distance to the node's bounding volume.
      @note The difference between this and pruneOrdered(a_point) only consist of the fact that this routine uses the unsigned square distance to prune branches and
      primitives. This is more efficient than pruneOrdered(a_point) because it does e.g. not involve an extra square root for computing the distance. 
      @return Returns the signed distance from a_point to the primitives. 
    */    
    inline
    T pruneOrdered2(const Vec3& a_point) const noexcept;

    /*!
      @brief Function which computes the signed distance using unordered pruning along the BVH branches. 
      @param[in] a_point 3D point in space
      @return Signed distance to the input point
      @details This routine computes the distance to a_point using unordered pruning. We begin at the top (root node of the tree) and descend downwards. This routine
      visits nodes in the order they were created. Once we hit leaf node we update the shortest distance 'd' found so far.
      As we investigate more branches, they can be pruned if the distance 'd' is shorter than the distance to the node's bounding volume.
      @note The difference between this and pruneOrdered(a_point) is that this routine always does the nodes in the order they were created. In almost all cases
      this is more inefficient than pruneOrdered(a_point). 
      @return Returns the signed distance from a_point to the primitives. 
    */
    inline
    T pruneUnordered(const Vec3& a_point) const noexcept;

    /*!
      @brief Function which computes the signed distance using unordered pruning along the BVH branches. 
      @param[in] a_point 3D point in space
      @return Signed distance to the input point
      @details This routine computes the distance to a_point using unordered pruning. We begin at the top (root node of the tree) and descend downwards. This routine
      visits nodes in the order they were created. Once we hit leaf node we update the shortest distance 'd' found so far.
      As we investigate more branches, they can be pruned if the distance 'd' is shorter than the distance to the node's bounding volume. The only difference between
      this routine and pruneUnordered(a_point) is that this routine prunes based on the unsigned square distance first, and only computes the signed distance at the end.
      @note The difference between this and pruneOrdered2(a_point) is that this routine always does nodes in the order they were created. In almost all cases
      this is more inefficient than pruneOrdered2(a_point). 
      @return Returns the signed distance from a_point to the primitives.     
    */
    inline
    T pruneUnordered2(const Vec3& a_point) const noexcept;

  protected:

    /*!
      @brief Bounding volume object
    */
    BV m_boundingVolume;

    /*!
      @brief Node type (leaf or regular)
    */
    NodeType m_nodeType;

    /*!
      @brief Node depth
    */
    int m_depth;

    /*!
      @brief Primitives list. This will be empty for regular nodes
    */
    PrimitiveList m_primitives;

    /*!
      @brief Children nodes
    */
    std::array<NodePtr, K> m_children;

    /*!
      @brief Pointer to parent node
    */
    NodePtr m_parent;

    /*!
      @brief Set node type to leaf or regular
      @param[in] a_nodeType Node type
    */
    inline
    void setNodeType(const NodeType a_nodeType) noexcept;

    /*!
      @brief Set node depth
      @param[in] a_depth Node depth
    */
    inline
    void setDepth(const int a_depth) noexcept;

    /*!
      @brief Insert a new node in the tree. 
      @param[in] a_node       Node to insert
      @param[in] a_primitives Primitives provided to a_node
    */    
    inline
    void insertNode(NodePtr& a_node, const PrimitiveList& a_primitives) noexcept;

    /*!
      @brief Insert nodes with primitives
    */
    inline
    void insertNodes(const std::array<PrimitiveList, K>& a_primitives) noexcept;

    /*!
      @brief Set to regular node
      @note This sets m_nodeType to regular and clears the primitives list
    */
    inline
    void setToRegularNode() noexcept;

    /*!
      @brief Set primitives in this node
      @param[in] a_primitives Primitives
    */
    inline
    void setPrimitives(const PrimitiveList& a_primitives) noexcept;

    /*!
      @brief Get the distance from a 3D point to the bounding volume 
      @param[in] a_point 3D point
      @return Returns distance to bounding volume. A zero distance implies that the input point is inside the bounding volume. 
    */
    inline
    T getDistanceToBoundingVolume(const Vec3& a_point) const noexcept;

    /*!
      @brief Get the unsigned square from a 3D point to the bounding volume 
      @param[in] a_point 3D point
      @return Returns squared distance to bounding volume. A zero distance implies that the input point is inside the bounding volume. 
    */    
    inline
    T getDistanceToBoundingVolume2(const Vec3& a_point) const noexcept;

    /*!
      @brief Compute the shortest distance to the primitives in this node
      @param[in] a_point 3D point
      @return Returns the signed distance to the primitives.
    */
    inline
    T getDistanceToPrimitives(const Vec3& a_point) const noexcept;

    /*!
      @brief Get the node type
      @return Node type
    */
    inline
    NodeType getNodeType() const noexcept;

    /*!
      @brief Get the list of primitives in this node. 
      @return Primitives list
    */
    inline
    PrimitiveList& getPrimitives() noexcept;

    /*!
      @brief Set parent node
      @param[in] a_parent Parent node
    */
    inline
    void setParent(const NodePtr& a_parent) noexcept;

    /*!
      @brief Implementation function for pruneOrdered (it requires a different signature). 
      @param[inout] a_closest Shortest distance to primitives so far. 
      @param[inout] a_point   Input 3D point
    */            
    inline
    void pruneOrdered(T& a_closest, const Vec3& a_point) const noexcept;

    /*!
      @brief Implementation function for pruneOrdered2 (it requires a different signature). 
      @param[inout] a_minDist2 Shortest square distance so far. 
      @param[inout] a_closest  Closest primitive so far. 
      @param[inout] a_point    Input 3D point
    */                
    inline
    void pruneOrdered2(T& a_minDist2, std::shared_ptr<const P>& a_closest, const Vec3& a_point) const noexcept;

    /*!
      @brief Implementation function for pruneUnordered (it requires a different signature). 
      @param[inout] a_closest Shortest distance to primitives so far. 
      @param[inout] a_point   Input 3D point
    */                    
    inline
    void pruneUnordered(T& a_closest, const Vec3& a_point) const noexcept;

    /*!
      @brief Implementation function for pruneUnordered2 (it requires a different signature). 
      @param[inout] a_minDist2 Shortest square distance so far. 
      @param[inout] a_closest  Closest primitive so far. 
      @param[inout] a_point    Input 3D point    
    */
    inline
    void pruneUnordered2(T& a_minDist2, std::shared_ptr<const P>& a_closest, const Vec3& a_point) const noexcept;
  };

  /*!
    @brief Node type for linearized (flattened) BVH. This will be constructed from the other (conventional) BVH type.
    @details T is the precision for Vec3, P is the primitive type you want to enclose, BV is the bounding volume you use for it. 
    @note P MUST supply function signedDistance(...) and unsignedDistance2(Vec3). BV must supply a
    function getDistance (had this been C++20, we would have use concepts to enforce this). 
  */
  template <class T, class P, class BV, int K>
  class LinearNodeT {
  public:

    /*!
      @brief Alias for cutting down on typing. 
    */
    using PrimitiveList = PrimitiveListT<P>;

    /*!
      @brief Alias for cutting down on typing
    */
    using Vec3 = Vec3T<T>;

    /*!
      @brief Constructor.
    */
    LinearNodeT();

    /*!
      @brief Destructor.
    */
    virtual ~LinearNodeT();

    /*!
      @brief Get the primitives offset
    */
    inline
    const unsigned long& getPrimitivesOffset() const noexcept;

    /*!
      @brief Get the number of primitives. 
    */
    inline
    const unsigned long& getNumPrimitives() const noexcept;

    /*!
      @brief Get the bounding volume
    */
    inline
    const BV& getBoundingVolume() const noexcept;

  protected:

    /*!
      @brief Bounding volume. An AABB box will be 6*T big => 24/48 bytes. 
    */
    BV m_bv;

    /*!
      @brief We assume that, outside of this class, is a data structure std::vector<std::shared_ptr<const P> >that holds all primitives. This member
      is the starting index in that vector. 
    */
    unsigned long m_primitivesOffset; // 8 bytes

    /*!
      @ Number of primitives. m_numPrimitives = 0 is an interior node. Other it's a leaf node. 
    */
    unsigned int m_numPrimitives; // 8 bytes

    /*!
      @brief Offset to child nodes. 
    */
    std::array<unsigned long, K-1> m_childrenOffsets; // (K-1)*8 bytes.
  };
}

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_BVHImplem.hpp"

#endif
