.. _Chap:ImplemOctree:

Octree
======

The octree functionality is encapsulated in the namespace ``EBGeometry::Octree``.
For the full API, see `the doxygen API <doxygen/html/namespaceOctree.html>`__.
Currently, only full octrees are supported (i.e., using a pointer-based representation).

Octrees are encapsulated by a class

.. literalinclude:: ../../../Source/EBGeometry_Octree.hpp
   :language: c++
   :lines: 74-75


where the template parameters are:

*  ``Meta`` Meta-information contained in the node (e.g. upper/lower corners). 
*  ``Data`` Data contained in the node.

``Node`` describes both regular and leaf nodes in the octree.

.. warning::

   ``Octree::Node<Meta, Data>`` should only be used as ``std::shared_ptr<Octree::Node<Meta, Data>>``.

.. _Chap:OctreeConstruction:

Construction
------------

Constructing the octree is done by first initializing the root node and then building it in either depth-first or breadth-first ordering.

.. literalinclude:: ../../../Source/EBGeometry_Octree.hpp
   :language: c++
   :lines: 74-76, 83, 90, 97-98,104,111,117-118,183-187,194-198,229,230

The input functions to ``buildDepthFirst`` and ``buildBreadthFirst`` are as follows:

#. ``StopFunction`` determines if the node should be split or not. If it returns true, the node will *not* be split.
#. ``MetaConstructor`` constructs meta-data in the child nodes. This can/should include the physical corners of the node, but this is not a requirement.
#. ``DataConstructor`` constructs data in the child node. This can e.g. be a partitioning of the parent data.

Tree traversal
---------------

.. literalinclude:: ../../../Source/EBGeometry_Octree.hpp
   :language: c++
   :lines: 74-76,205-211,239

The input functions to ``traverse`` are as follows:

#. ``Updater`` executes a user-specified update rule when visiting a leaf node.
#. ``Visiter`` determines if the node should be visited during the traversal or not.
#. ``Sorter`` permits the user to sort the nodes in the current subtrees and visit them in a specified pattern.
   By default, no sorting is done and the nodes are visited in lexicographical order.

Example
-------

An example of how to use the Octree functionality is given in :file:`EBGeometry_ImplicitFunctionImplem.hpp` where the octree functionality is used for spatial partitioning of an implicit function.
This includes both the octree construction and traversal.
