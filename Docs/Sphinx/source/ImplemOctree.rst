.. _Chap:ImplemOctree:

Octree
======

The octree functionality is encapsulated in the namespace ``EBGeometry::Octree``; see
:ref:`Chap:Octree` for the conceptual picture (space partitioning, adaptivity) before reading the
concrete API below. For the full API, see
`the doxygen API <doxygen/html/namespaceEBGeometry_1_1Octree.html>`__. Currently, only full
octrees are supported (a pointer-based representation, not a linear/pointerless one).

Octrees are encapsulated by a class template ``Octree::Node<Meta, Data>``, where the template
parameters are:

* ``Meta`` -- meta-information stored in every node (e.g. the node's physical corners).
* ``Data`` -- payload data stored at leaf nodes.

``Node`` describes both regular and leaf nodes in the octree; a node with no children is a leaf.

.. warning::

   ``Octree::Node<Meta, Data>`` should only be used as ``std::shared_ptr<Octree::Node<Meta, Data>>``.

See the Doxygen reference for
`Node <doxygen/html/classEBGeometry_1_1Octree_1_1Node.html>`__ for the full member list.

.. _Chap:OctreeConstruction:

Construction
------------

An octree is built by first default-constructing a (leaf) root node, and then calling either
``buildDepthFirst`` or ``buildBreadthFirst`` on it, which refines the tree in depth-first or
breadth-first order respectively. Both take the same three user-supplied callables:

#. ``SplitFunction`` -- a predicate ``bool(const Node&)`` called on each leaf node; returning
   ``true`` subdivides that leaf into eight children, ``false`` leaves it as-is.
#. ``MetaConstructor`` -- constructs a child's ``Meta`` from its parent's ``Meta`` and its octant
   index. This is typically where the child's physical corners are computed from the parent's,
   but nothing requires that -- ``Meta`` can hold whatever the refinement criterion needs to
   decide whether to split further.
#. ``DataConstructor`` -- constructs a child's ``Data`` from its parent's ``Data`` and its octant
   index (e.g. partitioning the parent's data set among the eight children).

Refinement proceeds top-down: starting from the root, every leaf for which ``SplitFunction``
returns ``true`` is subdivided into eight children (using ``MetaConstructor``/``DataConstructor``
to populate them), and the process repeats on the new leaves. There is no separate "maximum
depth" parameter built into ``Node`` itself -- if a bounded depth is wanted, ``SplitFunction``
must encode that itself (e.g. by having ``Meta`` carry the current level and refusing to split
past some level), exactly as :ref:`the bounding-volume estimator below <Sec:OctreeBoundingVolume>`
does.

Tree traversal
---------------

Tree traversal is done through the member function ``traverse``, which visits nodes top-down using
a prune-order-evaluate pattern analogous to the BVH traversal described in :ref:`Chap:ImplemBVH`. The
input functions to ``traverse`` are as follows:

#. ``PrunePredicate`` -- a predicate ``bool(const Node&)`` called on every node (interior or leaf);
   returning ``false`` prunes that entire subtree from the traversal.
#. ``ChildOrderer`` -- reorders a node's (up to) eight children in-place before they are visited, so the
   traversal can, e.g., visit the closest child first. By default, no sorting is done and children
   are visited in lexicographical octant order.
#. ``LeafEvaluator`` -- called on every *leaf* node that ``PrunePredicate`` did not prune; this is where the
   caller actually consumes the leaf (e.g. to accumulate a result).

.. _Sec:OctreeBoundingVolume:

Estimating a bounding volume for an implicit function
--------------------------------------------------------

The octree machinery above is used directly by
`ImplicitFunction::approximateBoundingVolumeOctree <doxygen/html/classEBGeometry_1_1ImplicitFunction.html#a193199c514d6c35fd8553cef9affb767>`__,
which estimates a bounding volume of type ``BV`` for an implicit function :math:`I` that has no
closed-form bound -- for example, one built up from several nested CSG operations (see
:ref:`Chap:ConstructiveSolidGeometry`), where no simple formula for a bounding box or sphere is
available.

Given an initial search box and a maximum tree depth, the algorithm is:

#. Each node's meta-data records its two physical corners, its depth, and a boolean flag for
   whether it might contain the implicit surface.
#. A node is flagged as possibly containing the surface if

   .. math::

      \left|I\left(\mathbf{x}_c\right)\right| \leq (1 + \sigma)\left|\Delta\mathbf{x}\right|,

   where :math:`\mathbf{x}_c` is the node's center, :math:`\Delta\mathbf{x}` is its half-diagonal,
   and :math:`\sigma \geq 0` is a user-supplied safety factor. This is a direct consequence of the
   Eikonal property (see :ref:`Chap:GeometryRepresentations`): since :math:`I` changes by at most
   the distance moved, evaluating it at a single point bounds how far away the surface can be from
   that point, so a node whose center is farther from the surface than the node itself extends
   cannot contain it.
#. ``SplitFunction`` subdivides exactly the flagged nodes, and only up to the given maximum depth
   -- this is the "bounded refinement" mentioned above, implemented entirely inside the
   ``SplitFunction``/``MetaConstructor`` pair rather than by any feature of ``Node`` itself. The
   tree is built with ``buildBreadthFirst``.
#. The tree is then traversed (``PrunePredicate`` keeps only flagged nodes, ``LeafEvaluator`` collects each
   surviving leaf's eight corner points), and the final bounding volume is constructed directly
   from that point set: ``BV`` need only be constructible from a ``std::vector<Vec3T<T>>``.

If the initial box doesn't intersect the surface at all (or is degenerate), the routine falls back
to returning the maximally representable bounding volume instead, signalling that the initial box
needs to be chosen more generously.

.. tip::

   A deeper maximum tree depth gives a tighter bounding volume, at the cost of more evaluations of
   :math:`I` (one per candidate node, at every level).

.. warning::

   This is an approximation, not an exact bound: it is only as tight as the finest cell size
   actually reached, and an implicit function whose rate of change meaningfully exceeds unity
   (violating the Eikonal property) can, in principle, still have surface features that fall
   outside the estimate unless :math:`\sigma` is chosen generously enough to compensate.
