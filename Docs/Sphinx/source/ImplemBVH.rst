.. _Chap:ImplemBVH:

BVH
===

The BVH functionality is encapsulated in the namespace ``EBGeometry::BVH``.
For the full API, see `the doxygen API <doxygen/html/namespaceBVH.html>`__.
There are two types of BVHs supported.

*  **Full BVHs** where the nodes are stored in build order and contain references to their children.
*  **Compact BVHs** where the nodes are stored in depth-first order and contain index offsets to children and primitives.

The full BVH is encapsulated by a class

.. code-block:: c++

   template <class T, class P, class BV, size_t K>
   class NodeT;

The above template parameters are:

*  ``T`` Floating-point precision.
*  ``P`` Primitive type to be partitioned.
*  ``BV`` Bounding volume type.
*  ``K`` BVH degree. ``K=2`` will yield a binary tree, ``K=3`` yields a tertiary tree and so on. 

``NodeT`` describes regular and leaf nodes in the BVH, and has member functions for setting primitives, bounding volumes, and so on.
Importantly, ``NodeT`` is the BVH builder node, i.e. it is the class through which we recursively build the BVH, see :ref:`Chap:BVHConstruction`.
The compact BVH is discussed below in :ref:`Chap:LinearBVH`.


Bounding volumes
----------------

``EBGeometry`` supports the following bounding volumes, which are defined in :file:`EBGeometry_BoundingVolumes.hpp`:

*  **BoundingSphere**, templated as ``EBGeometry::BoundingVolumes::BoundingSphereT<T>`` and describes a bounding sphere.
   Various constructors are available.

*  **Axis-aligned bounding box**, which is templated as ``EBGeometry::BoundingVolumes::AABBT<T>``.

For full API details, see `the doxygen API <doxygen/html/namespaceBoundingVolumes.html>`_.
Other types of bounding volumes can in principle be added, with the only requirement being that they conform to the same interface as the ``AABB`` and ``BoundingSphere`` volumes.

.. _Chap:BVHConstruction:

Construction
------------

Constructing a BVH is done by:

#.  Creating a root node and providing it with the geometric primitives and their bounding volumes.
#.  Partitioning the BVH by providing a partitioning function. 

The first step is usually a matter of simply constructing the root node using the full constructor, which takes a list of primitives and their associated bounding volumes. 
The second step is to recursively build the BVH.
We currently support top-down and bottom-up construction (using space-filling curves).

.. tip::

   The default construction methods performs the hierarchical subdivision by only considering the *bounding volumes*.
   Consequently, the build process is identical regardless of what type of primitives (e.g., triangles or analytic spheres) are contained in the BVH.

Top-down construction
_____________________

Top-down construction is done through the function ``topDownSortAndPartition()``, `see the doxygen API for the BVH implementation <doxygen/html/doxygen/html/classBVH_1_1NodeT.html>`_.

The optional input arguments to ``topDownSortAndPartition`` are polymorphic functions of type indicated above, and have the following responsibilities:

*  ``PartitionerT`` is the partitioner function when splitting a leaf node into ``K`` new leaves.
   The function takes a list of primitives which it partitions into ``K`` new lists of primitives.

*  ``StopFunctionT`` simply takes a ``NodeT`` as input argument and determines if the node should be partitioned further.

Default arguments for these are provided, bubt users are free to partition their BVHs in their own way should they choose.

Bottom-up construction
______________________

The bottom-up construction uses a space-filling curve (e.g., a Morton curve) for first building the leaf nodes.
This construction is done such that each leaf node contains approximately the number of primitives, and all leaf nodes exist on the same level.
To use bottom-up construction, one may use the member function

.. literalinclude:: ../../../Source/EBGeometry_BVH.hpp
   :language: c++
   :lines: 298-309
   :dedent: 4

The template argument is the space-filling curve that the user wants to apply.
Currently, we support Morton codes and nested indices.
For Morton curves, one would e.g. call ``bottomUpSortAndPartition<SFC::Morton>`` while for nested indices (which are not recommended) the signature is likewise ``bottomUpSortAndPartition<SFC::Nested``.

Build times for SFC-based bottom-up construction are generally speaking faster than top-down construction, but tends to produce worse trees such that traversal becomes slower. 

.. _Chap:LinearBVH:

Compact form
------------

In addition to the standard BVH node ``NodeT<T, P, BV, K>``, ``EBGeometry`` provides a more compact formulation of the BVH hierarchy where the nodes are stored in depth-first order.
The "linearized" BVH can be automatically constructed from the standard BVH but not vice versa.

.. figure:: /_static/CompactBVH.png
   :width: 240px
   :align: center

   Compact BVH representation.
   The original BVH is traversed from top-to-bottom along the branches and laid out in linear memory.
   Each interior node gets a reference (index offset) to their children nodes.

The rationale for reorganizing the BVH in compact form is it's tighter memory footprint and depth-first ordering which occasionally allows a more efficient traversal downwards in the BVH tree, particularly if the geometric primitives are sorted in the same order. 
To encapsulate the compact BVH we provide two classes:

*  ``LinearNodeT`` which encapsulates a node, but rather than storing the primitives and pointers to child nodes it stores offsets along the 1D arrays.
   Just like ``NodeT`` the class is templated:

   .. code-block:: c++
		   
      template <class T, class P, class BV, size_t K>
      class LinearNodeT		       

   ``LinearNodeT`` has a smaller memory footprint and should fit in one CPU word in floating-point precision and two CPU words in double point precision.
   The performance benefits of further memory alignment have not been investigated.

   Note that ``LinearNodeT`` only stores offsets to child nodes and primitives, which are assumed to be stored (somewhere) as

   .. code-block:: c++

     std::vector<std::shared_ptr<LinearNodeT<T, P, BV, K> > > linearNodes;
     std::vector<std::shared_ptr<const P> > primitives;

   Thus, for a given node we can check if it is a leaf node (``m_numPrimitives > 0``) and if it is we can get the children through the ``m_childOffsets`` array.
   Primitives can likewise be obtained; they are stored in the primitives array from index ``m_primitivesOffset`` to ``m_primitivesOffset + m_numPrimities - 1``. 

*  ``LinearBVH`` which stores the compact BVH *and* primitives as class members.
   That is, ``LinearBVH`` contains the nodes and primitives as class members.

   .. code-block:: c++

      template <class T, class P, class BV, size_t K>
      class LinearBVH
      {
      public:

      protected:

        std::vector<std::shared_ptr<const LinearNodeT<T, P, BV, K>>> m_linearNodes;
	std::vector<std::shared_ptr<const P>> m_primitives;	
      };

   The root node is, of course, found at the front of the ``m_linearNodes`` vector.
   Note that the list of primitives ``m_primitives`` is stored in the order in which the leaf nodes appear in ``m_linearNodes``. 

Constructing the compact BVH is simply a matter of letting ``NodeT`` aggregate the nodes and primitives into arrays, and return a ``LinearBVH``.
This is done by calling the ``NodeT`` member function ``flattenTree()``:

.. code-block:: c++

   template <class T, class P, class BV, size_t K>
   class NodeT
   {
   public:
   
     inline std::shared_ptr<LinearBVH<T, P, BV, K>>
     flattenTree() const noexcept;
   };

which returns a pointer to a ``LinearBVH``.
For example:

.. code-block:: c++

   // Assume that we have built the conventional BVH already
   std::shared_ptr<EBGeometry::BVH::NodeT<T, P, BV, K> > builderBVH;

   // Flatten the tree.
   std::shared_ptr<LinearBVH> compactBVH = builderBVH->flattenTree();

   // Release the original BVH.
   builderBVH = nullptr;

.. warning::

   When calling ``flattenTree``, the original BVH tree is *not* destroyed.
   To release the memory, deallocate the original BVH tree.
   E.g., the set pointer to the root node to ``nullptr`` or ensure correct scoping.

Tree traversal
---------------

Both ``NodeT`` (full BVH) and ``LinearBVH`` (flattened BVH) include routines for traversing the BVH with user-specified criteria.
For both BVH representations, tree traversal is done using a routine

.. code-block:: c++
		
    template <class Meta>
    inline void
    traverse(const BVH::Updater<P>&                    a_updater,
             const BVH::Visiter<LinearNode, Meta>&     a_visiter,
             const BVH::Sorter<LinearNode, Meta, K>&   a_sorter,
             const BVH::MetaUpdater<LinearNode, Meta>& a_metaUpdater) const noexcept;

The BVH trees use a stack-based traversal pattern based on visit-sort rules supplied by the user.

Node visit
__________

Here, ``a_visiter`` is a lambda function for determining if the node/subtree should be investigated or pruned from the traversal.
This function has a signature

.. code-block:: c++

  template <class NodeType, class Meta>
  using Visiter = std::function<bool(const NodeType& a_node, const Meta a_meta)>;

where ``NodeType`` is the type of node (which is different for full/flat BVHs), and the ``Meta`` template parameter is discussed below.
If this function returns true, the node will be visisted and if the function returns false then the node will be pruned from the tree traversal. Typically, the ``Meta`` parameter will contain the necessary information that determines whether or not to visit the subtree.

Traversal pattern
_________________

If a subtree is visited in the traversal, there is a question of which of the child nodes to visit first.
The ``a_sorter`` argument determines the order by letting the user sort the nodes based on order of importance.
Note that a correct visitation pattern can yield large performance benefits.
The user is given the option to sort the child nodes based on what he/she thinks is a good order, which is done by supplying a lambda which sorts the children.
This function has the signature:

.. code-block:: c++

  template <class NodeType, class Meta, size_t K>
  using Sorter = std::function<void(std::array<std::pair<std::shared_ptr<const NodeType>, Meta>, K>& a_children)>;

Sorting the child nodes is completely optional.
The user can leave this function empty if it does not matter which subtrees are visited first. 

Update rule
___________

If a leaf node is visited in the traversal, distance or other types of queries to the geometric primitive(s) in the nodes are usually made.
These are done by a user-supplied update-rule:

.. code-block:: c++
		
  template <class P>
  using Updater = std::function<void(const PrimitiveListT<P>& a_primitives)>;

Typically, the ``Updater`` will modify parameters that appear in a local scope outside of the tree traversal (e.g. updating the minimum distance to a DCEL mesh).

Meta-data
_________

During the traversal, it might be necessary to compute meta-data that is helpful during the traversal, and this meta-data is attached to each node that is queried.
This meta-data is usually, but not necessarily, equal to the distance to the nodes' bounding volumes.
The signature for meta-data construction is

.. code-block:: c++

  template <class NodeType, class Meta>
  using MetaUpdater = std::function<Meta(const NodeType& a_node)>;

The biggest difference between ``Updater`` and ``MetaUpdater`` is that ``Updater`` is *only* called on leaf nodes whereas ``MetaUpdater`` is also called for internal nodes.
One typical example for DCEL meshes is that ``Updater`` computes the distance from an input point to the triangles in a leaf node, whereas ``MetaUpdater`` computes the distance from the input point to the bounding volumes of a child nodes.
This information is then used in ``Sorter`` in order to determine a preferred child visit pattern when descending along subtrees. 

Traversal algorithm
___________________

The code-block below shows the implementation of the BVH traversal.
The implementation uses a non-recursive queue-based formulation when descending along subtrees.
Observe that each entry in the stack contains both the node itself *and* any meta-data we want to attach to the node.
If the traversal decides to visit a node, it immediately computes the specified meta-data of the node, and the user can then sort the children based on that data.

.. literalinclude:: ../../../Source/EBGeometry_BVHImplem.hpp
   :language: c++
   :lines: 284-322
   :dedent: 2
   :caption: Tree traversal algorithm for the BVH tree.

Traversal examples
__________________

Below, we consider two examples for BVH traversal.
The examples show how we compute the signed distance from a DCEL mesh, and how to perform a *smooth* CSG union where the search for the two closest objects is done by BVH traversal.

Signed distance
^^^^^^^^^^^^^^^

The DCEL mesh distance fields use a traversal pattern based on

* Only visit bounding volumes that are closer than the minimum distance computed (so far).
* When visiting a subtree, investigate the closest bounding volume first.
* When visiting a leaf node, check if the primitives are closer than the minimum distance computed so far.

These rules are given below.

.. literalinclude:: ../../../Source/EBGeometry_MeshDistanceFunctionsImplem.hpp
   :language: c++
   :lines: 145-181
   :caption: Tree traversal criterion for computing the signed distance to a DCEL mesh using the BVH accelerator.
	     See :file:`Source/EBGeometry_MeshDistanceFunctionsImplem.hpp` for details.

CSG Union
^^^^^^^^^

Combinations of implicit functions in ``EBGeometry`` into aggregate objects can be done by means of CSG unions.
One such union is known as the *smooth union*, in which the transition between two objects is gradual rather than abrupt.
Below, we show the traversal code for this union, where we traverse through the tree and obtains the distance to the *two* closest objects rather than a single one.

.. literalinclude:: ../../../Source/EBGeometry_CSGImplem.hpp
   :language: c++
   :lines: 369-415
   :caption: Tree traversal when computing the smooth CSG union.
	     See :file:`Source/EBGeometry_CSGImplem.hpp` for details.
