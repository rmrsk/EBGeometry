.. _Chap:ImplemBVH:

BVH
===

See :ref:`Chap:BVH` for the conceptual picture of bounding volume hierarchies (node types,
partitioning, tree pruning during traversal) before reading the concrete API below.

The BVH functionality is encapsulated in the namespace ``EBGeometry::BVH``. For the full API, see
`the doxygen API <doxygen/html/namespaceEBGeometry_1_1BVH.html>`__. There is one builder, one node
layout and one traversal:

*  :file:`Source/EBGeometry_BVHBuild.hpp` holds the node layout ``BVH::WideNode<T, K>``, the build
   specification ``BVH::BuildSpec``, and the one host-side builder ``BVH::buildTopology<T, K>()``,
   which returns the shape of a tree as a ``BVH::Topology<T, K>`` (see :ref:`Chap:BVHConstruction`
   and :ref:`Sec:WideNode`).
*  :file:`Source/EBGeometry_BVH.hpp` holds ``BVH::PackedBVH<T, P, K>``: a flat array of wide nodes
   and an array of primitives in leaf order, reserved from a ``Pool``, queried on the host or on a
   device through ``pruneTraverse()`` (see :ref:`Chap:PackedBVH` and :ref:`Chap:PruneTraverse`).

Every class in the library that holds a BVH -- ``MeshSDF``, ``TriMeshSDF``, ``PointCloudBVH`` and the
BVH unions -- builds it through ``buildTopology()`` and stores it as a ``PackedBVH``. There is no
separate pointer-based tree and no packing step: the builder produces the final node layout
directly.

The template parameters are:

*  ``T`` Floating-point precision.
*  ``P`` Primitive type (``PackedBVH`` only). ``PackedBVH`` imposes no interface on it beyond being
   trivially copyable: it holds primitives opaquely and hands them back only to whatever leaf-visit
   callback a caller supplies to ``pruneTraverse()`` (see below). Whether ``P`` needs a
   ``signedDistance(Vec3T<T>)`` member (or anything else) is entirely up to that callback -- see
   ``MeshSDF``/``TriMeshSDF::signedDistance()`` in :ref:`Chap:MeshSDFClasses` for the
   signed-distance case; a callback could equally perform, say, a nearest-neighbor search over a
   point cloud whose primitive carries no ``signedDistance()`` at all. The builder never sees ``P``
   either: it works on one bounding box per primitive.
*  ``K`` Branching factor: the number of child slots per node, at least 2. ``K=2`` gives a binary
   tree, ``K=4`` (the library default, see :ref:`Sec:DefaultKW`) a 4-ary tree, and so on.

The bounding volume is always ``BoundingVolumes::AABBT<T>``.

Bounding volumes
----------------

EBGeometry supports the following bounding volumes, which are defined in :file:`Source/EBGeometry_BoundingVolumes.hpp`:

*  **Bounding sphere**, templated as ``EBGeometry::BoundingVolumes::SphereT<T>``.
   Various constructors are available. See `the doxygen page for SphereT
   <doxygen/html/classEBGeometry_1_1BoundingVolumes_1_1SphereT.html>`__.

*  **Axis-aligned bounding box**, which is templated as ``EBGeometry::BoundingVolumes::AABBT<T>``.
   See `the doxygen page for AABBT
   <doxygen/html/classEBGeometry_1_1BoundingVolumes_1_1AABBT.html>`__. Besides its constructors from
   corners or from a list of points or boxes, ``AABBT::merged()`` returns the union of two boxes
   without allocating, and is callable on a device. The default-constructed box is inverted
   (low corner at :math:`+\infty`, high corner at :math:`-\infty`) and is the identity of
   ``merged()``, which is how the builder and ``refit()`` accumulate a box over a range.

For full API details, see `the doxygen API <doxygen/html/namespaceEBGeometry_1_1BoundingVolumes.html>`_.
The BVH itself always uses ``AABBT<T>``: the builder takes one ``AABBT<T>`` per primitive, and every
node stores its children's boxes as ``AABBT<T>`` coordinates. A primitive described by some other
bounding volume must be given an enclosing axis-aligned box before it is handed to the builder.

.. _Chap:BVHConstruction:

Construction
------------

Building a BVH is one call, ``BVH::buildTopology<T, K>(boxes, spec)``, which takes one bounding box
per primitive and a ``BVH::BuildSpec``, and returns the shape of the tree. The ``PackedBVH``
constructors call it for you; it is public for classes that store a different primitive from the one
the tree was built over (see :ref:`Chap:PackedBVH`).

.. tip::

   The builder only ever sees the primitives' *bounding boxes*; where a strategy needs one point per
   primitive, it uses the centroid of its box. The build is therefore identical regardless of what
   kind of primitive (a mesh face, a triangle, a point, an analytic sphere) the boxes enclose.

The build specification
_______________________

``BVH::BuildSpec`` (see `its doxygen page <doxygen/html/structEBGeometry_1_1BVH_1_1BuildSpec.html>`__)
has three fields:

*  ``strategy`` (``BVH::Strategy``, default ``SAH``): how the primitives are partitioned.
*  ``curve`` (``BVH::Curve``, default ``Morton``): the space-filling curve, used by
   ``Strategy::SpaceFillingCurve`` only.
*  ``maxLeafSize`` (default 4): the maximum number of primitives in one leaf. It must be positive.

``BuildSpec{}`` is therefore a binned-SAH build with at most four primitives per leaf, and a
specification is spelled as an aggregate, e.g.
``BVH::BuildSpec{BVH::Strategy::SpaceFillingCurve, BVH::Curve::Hilbert, 8}``.

``maxLeafSize`` means the same thing to every strategy and every class that builds a BVH: **a leaf
never holds more than that many primitives**. It is an upper bound, not a target -- the top-down
strategies still split down to smaller leaves wherever the geometry calls for it. A class that
groups several primitives into one stored element counts the primitives it was *given*, before
grouping: ``TriMeshSDF`` counts triangles (each leaf's triangles are then packed into SoA groups of
``W``), and ``PointCloudBVH`` counts points.

The strategies are:

.. list-table:: ``BVH::Strategy``
   :widths: 22 78
   :header-rows: 1

   * - Strategy
     - How it partitions
   * - ``SAH`` (default)
     - Top-down, binned Surface Area Heuristic: 32 bins along each of the three axes, the split with
       the lowest area-weighted cost wins. The best trees for queries, at the highest build cost.
   * - ``Centroid``
     - Top-down, equal-count split along the longest axis of the box centroids. Every split divides
       a range in proportion to the number of child slots it has to fill, so it gives the shallowest
       tree; it is also what the builder falls back to when another strategy produces a tree too deep
       for a device (see :ref:`Sec:TraversalDepth`).
   * - ``Midpoint``
     - Top-down, split at the spatial midpoint of the centroids' longest axis -- a single
       ``std::partition`` pass with no sorting and no cost evaluation, the fastest top-down build. It
       does not adapt to the primitive distribution the way SAH does; for near-uniform inputs such as
       point clouds that costs little, which is why ``PointCloudBVH`` uses it.
   * - ``ClusterSAH``
     - Two phases: first group the primitives into leaf-sized *clusters* (at most ``maxLeafSize``
       primitives each) by a cheap, density-adaptive midpoint subdivision, then run binned SAH over
       one box per cluster, each weighted by its primitive count. SAH then partitions roughly
       ``N / maxLeafSize`` boxes instead of ``N``, giving near-SAH tree quality at a fraction of the
       build cost; a leaf holds whole clusters.
   * - ``SpaceFillingCurve``
     - Bottom-up: bin the box centroids onto an integer grid, sort them along the curve named by
       ``BuildSpec::curve``, cut the sorted sequence into consecutive leaves, and merge upward ``K``
       at a time.

The top-down strategies (``SAH``, ``Centroid``, ``Midpoint``, and the second phase of
``ClusterSAH``) share one builder. A range of primitives becomes a leaf once it holds at most
``maxLeafSize`` primitives; otherwise it is split into up to ``K`` groups by recursive bisection,
each group becoming a leaf slot or a new child node. Each bisection keeps at least as many primitives
on either side as that side has groups to fill, falling back to an equal-count split when the
strategy's own choice would leave a side too small -- which is what keeps coincident or tightly
clustered centroids from producing a deep, degenerate tree.

The ``SpaceFillingCurve`` strategy bins the centroids with ``SFC::computeBins()``, which uses one
cubic cell size for all three axes, set by the longest extent of the centroids: scaling each axis to
the grid separately would stretch a flat cloud's thin axis over as many cells as its long ones, and
the curve would no longer follow distance. The cells are then encoded along the chosen curve from
namespace ``EBGeometry::SFC`` (:file:`Source/EBGeometry_SFC.hpp`, see `the doxygen API
<doxygen/html/namespaceEBGeometry_1_1SFC.html>`__):

*  ``Curve::Morton`` -- Morton (Z-order) codes.
*  ``Curve::Hilbert`` -- the Hilbert curve, whose consecutive codes are always spatially adjacent,
   so it tends to produce tighter leaves than Morton.
*  ``Curve::Nested`` -- nested (row-major) indices, which are not recommended.

The sorted primitives are cut into :math:`\lceil N / \texttt{maxLeafSize}\rceil` leaves, as equal in
size as possible. Each level then merges consecutive runs of up to ``K`` entries into a node; a
single entry left over at the end of a level is carried up to the next level rather than given a node
of its own. Space-filling-curve builds are cheap, but generally give slower queries than the
top-down strategies, since the leaves follow the curve rather than the geometry.

The :ref:`Chap:ExampleBuildBVH` example times every strategy's build and a closest-point query
workload on the resulting trees.

.. tip::

   Higher-level entry points take a ``BVH::BuildSpec`` and pass it through: the ``MeshSDF`` and
   ``TriMeshSDF`` constructors, ``Parser::readIntoPackedBVH`` and ``Parser::readIntoTriangleBVH``
   (see :ref:`Chap:Parsers`), and the ``BVHUnionIF``/``BVHSmoothUnionIF`` constructors (see
   :ref:`Sec:BVHUnions`).

The topology
____________

``buildTopology()`` returns a ``BVH::Topology<T, K>`` (see `its doxygen page
<doxygen/html/structEBGeometry_1_1BVH_1_1Topology.html>`__): the node array ``nodes`` and a
permutation ``order`` of the input primitives. A leaf slot's primitive range ``[offset, offset +
count)`` indexes ``order``, and ``order[i]`` is the index of an input primitive, so the primitives in
``order`` are exactly the leaves' primitives in leaf order. Every topology the builder returns
satisfies:

*  The root is node 0, and every interior child has a higher index than its parent.
*  Every node has at least two occupied slots, except a root that holds a single leaf. A BVH over
   :math:`N` primitives therefore has at most :math:`\max(1, N - 1)` nodes.
*  Every leaf holds at most ``maxLeafSize`` primitives, and at least one.
*  A BVH over no primitives has no nodes.
*  The tree is no deeper than ``BVH::DeviceTraversalDepth`` node levels (see
   :ref:`Sec:TraversalDepth`).

``buildTopology()`` is host-only, and checks its input in every build, not only when assertions are
enabled: ``maxLeafSize`` must be positive, every box must be finite and not inverted, and the
strategy and curve must be known values. A violation aborts with a diagnostic naming the offending
primitive. ``BVH::treeDepth()`` returns the depth of a node array in node levels (the root alone is
depth 1; no nodes is depth 0).

.. _Sec:WideNode:

The node layout: ``WideNode``
_____________________________

A node of the tree is a ``BVH::WideNode<T, K>`` (see `its doxygen page
<doxygen/html/structEBGeometry_1_1BVH_1_1WideNode.html>`__): one interior node with ``K`` child
*slots*. Each slot holds its child's bounding box and one of three things:

*  **A leaf slot** holds a range of primitives: ``m_child[k]`` is the index of the leaf's first
   primitive and ``m_count[k]`` the number of primitives, at least one.
*  **An interior slot** holds a child node: ``m_child[k]`` is its index in the node array and
   ``m_count[k]`` is zero.
*  **An empty slot** holds nothing: ``m_child[k]`` is ``WideNode::EmptySlot`` (``0xFFFFFFFF``),
   ``m_count[k]`` is zero, and its box is inverted, so its distance to any point is :math:`+\infty`.

Leaves are therefore slots, not nodes of their own: no box is stored twice, and no node is ever a
leaf with zero primitives. The occupied slots of a node always come first, followed by its empty
ones.

The boxes are stored as structure-of-arrays rows, ``m_lo[axis][slot]`` and ``m_hi[axis][slot]``, so
that one SIMD load reads one axis of all ``K`` boxes. Each row is aligned to its own width
(``sizeof(T) * K`` bytes) where that is a power of two, which is what the vectorised distance kernel
(see :ref:`Sec:SIMDDistanceKernel`) requires; for any other ``K`` no SIMD path exists and the natural
alignment of ``T`` is used. A ``WideNode`` is trivially copyable, and its accessors
(``isLeaf(k)``/``isInterior(k)``/``isEmpty(k)``, ``numChildren()``, ``getChild(k)``,
``getPrimitivesOffset(k)``/``getNumPrimitives(k)``, ``getBoundingVolume(k)``, the whole-node
``getBoundingVolume()``, and the per-slot squared distance ``getDistance2(k, point)``) are callable
on the host and on a device.

.. _Chap:PackedBVH:

PackedBVH
---------

A ``BVH::PackedBVH<T, P, K>`` (see `its doxygen page
<doxygen/html/classEBGeometry_1_1BVH_1_1PackedBVH.html>`__) stores two arrays: the node array, a
flat array of ``WideNode<T, K>`` with the root at index 0 and every child after its parent, and the
primitive array, holding every primitive by value in leaf order, so that every leaf is one contiguous
range. ``getNodes()`` and ``getPrimitives()`` return them as spans. ``getBoundingVolume()`` (and
``computeBoundingVolume()``, the same under the name other bounded objects use) returns the union of
the root's slot boxes by value, or the default, inverted box for an empty BVH.

.. figure:: /_static/CompactBVH.png
   :width: 240px
   :align: center

   Flattening a tree into a linear array (conceptual). The tree is laid out top-to-bottom in one
   array, and each node refers to its children by index rather than by pointer. In a ``PackedBVH``
   only interior nodes occupy entries of the node array; a leaf is a slot of its parent that names
   a range of the primitive array.

Constructors
____________

``PackedBVH`` has no default constructor. It is always built, or adopts arrays built elsewhere, and
every constructor takes the ``Pool`` its arrays are reserved from as its first argument:

#. **From primitives and their boxes.**
   ``PackedBVH(Pool&, std::vector<std::pair<P, BV>>, const BuildSpec& = BuildSpec{})`` builds the
   tree with ``buildTopology()`` and stores one primitive per input pair, reordered into leaf order.
   The pairs are a sink parameter the caller can ``std::move`` in. An empty input gives an empty
   BVH, which a traversal simply returns from. ``MeshSDF`` and the BVH unions build this way.

#. **From a topology and a leaf packer.**
   ``PackedBVH(Pool&, const Topology<T, K>&, PackLeaf&&)`` stores a tree whose shape was built with
   ``buildTopology()``, for a class whose stored primitive is not the one the tree was built over.
   The callback is called once per leaf, in leaf order, as
   ``packLeaf(const uint32_t* items, uint32_t count, std::vector<P>& out)``, where ``items`` are the
   indices of the leaf's input primitives (a range of ``Topology::order``); it must append at least
   one primitive to ``out``, and the leaf's range becomes whatever it appended. ``TriMeshSDF`` packs
   a leaf's triangles into ``TriangleAoSoA`` groups of ``W`` this way, and ``PointCloudBVH`` a leaf's
   points into ``PointAoSoA`` groups.

#. **Adopting arrays built elsewhere.** Two overloads take a node array and a primitive array built
   by other means:

   *  ``PackedBVH(Pool&, const std::vector<Node>&, const std::vector<P>&)`` copies host arrays into
      the pool.
   *  ``PackedBVH(Pool&, const PODVector<Node>&, const PODVector<P>&)`` adopts arrays that are
      already in the pool, for example filled in place by a device kernel, so that no data passes
      through the host. Reserve both arrays from the pool with ``PODVector::reserveFrom()`` --
      ``PackedBVH::maxNodeCount(numLeaves)``, which is :math:`\max(1, \text{leaves} - 1)`, bounds the
      node array for any builder that keeps every node at two or more occupied slots -- fill them,
      and set their sizes with ``PODVector::setSize()``. Nothing is copied.

   Both check, in every build, that the arrays form a well-formed tree, since a malformed array
   would otherwise surface as an out-of-bounds read in Release. The check is the public static
   ``PackedBVH::requireWellFormed(nodes, numNodes, numPrimitives)``: every occupied slot comes
   before every empty one and every node has at least one; an interior slot names a child that lies
   after its parent, inside the array, and has no other parent; a leaf's range lies inside the
   primitive array; every node other than the root is reachable; and an empty node array comes only
   with an empty primitive array. A violation aborts with a message starting
   ``BVH::PackedBVH: adopted node array is malformed``. The tree must also be no deeper than
   ``HostTraversalDepth`` (see :ref:`Sec:TraversalDepth`). The pool-resident overload always checks
   that the arrays end inside the pool's reserved bytes, but reads their contents only when the
   pool's memory resource is host-accessible: arrays in device-only memory cannot be checked on the
   host, and their shape is the caller's responsibility. Such a BVH is for kernels only: freeze the
   pool, take a ``rebasedView()`` onto it and pass that to a kernel. Nothing on the host reads its
   arrays, and ``deepCopy()`` refuses it.

Primitive storage
_________________

A ``PackedBVH`` stores each primitive **inline, by value**, in one flat array held in leaf order.
There is no indirection: no per-primitive heap allocation, no pointer chase on a leaf visit, and
because the build reorders primitives into leaf order, a leaf scan walks contiguous memory.

``P`` must therefore be trivially copyable. That is a hard requirement, not a coincidence: it is
what lets the whole BVH -- nodes and primitives alike -- be mirrored into a device address space by a
plain byte copy. Until the GPU port the primitive array could instead hold
``std::shared_ptr<const P>``; that was removed for exactly this reason, since a ``shared_ptr`` can
never cross into a device address space and keeping it would have forced a second, permanently
host-only primitive-array backend. See :ref:`Sec:PolymorphicPrimitives` below for the one use that
depended on it.

.. _Sec:IndexedPrimitives:

Primitives named by index
_________________________

When several BVHs must share one primitive set, or when the primitives are large enough that
duplicating them is the binding constraint, make the index itself the primitive: build a
``PackedBVH<T, uint32_t, K>`` over ``(index, bounding volume)`` pairs and resolve each stored index
against the caller-owned array inside the leaf callback. That stores four bytes per primitive with
no duplication, keeps the BVH trivially copyable, and needs no special support anywhere: a
``uint32_t`` is an ordinary primitive type, so every constructor accepts it, and every
``BuildSpec`` builds it.

The trade is locality: the query pays a scattered load per primitive instead of a contiguous one,
and the array the indices resolve against must outlive every BVH indexing into it -- an index is not
an owner, and nothing checks that. Prefer storing the primitives themselves unless one of the two
reasons above actually applies.

An earlier ``BVH::IndexStorage`` policy wrapped this same pattern behind a template parameter on
``PackedBVH``. It was removed: ``PackedBVH<T, uint32_t, K>`` stores exactly the same four bytes per
primitive in exactly the same layout, needs no policy machinery to do it, and works through every
constructor.

Pool-backed storage
____________________

``PackedBVH``'s two arrays -- the node array and the primitive array -- are ``PODVector``\ s reserved
from a caller-supplied ``Pool`` (see :ref:`Chap:MemoryModel`), not ``std::vector``\ s. Every
constructor therefore takes a ``Pool&``, and so do ``MeshSDF``/``TriMeshSDF``/``PointCloudBVH`` and
the BVH unions, which pass along the pool they already take. The pool must outlive the BVH.

Construction itself still assembles the arrays in ordinary ``std::vector``\ s and copies them into
the pool in one shot at the end. That is deliberate: a ``PODVector`` never reallocates, so it has no
way to be filled incrementally without its final size fixed up front, and the node count is not
known until a build finishes. Growth belongs in the host-only build step; nothing that crosses to a
device is ever built incrementally. (The pool-resident adopting constructor is the exception that
proves the rule: its caller reserves the arrays at their maximum size up front.)

What this buys is that a ``PackedBVH`` is trivially copyable -- two 16-byte ``PODVector``
descriptors and a ``PoolLocation`` -- so a whole hierarchy crosses to a device as a byte copy with no
pointer patching. ``rebasedView(Pool&)`` is the single sanctioned crossing, exactly as for
``DCEL::MeshT``: mirror the pool, rebase on the host, then pass the returned value to a kernel. A
device view is also checked to be shallow enough for the device traversal stack (see
:ref:`Sec:TraversalDepth`).

``getPrimitives()`` and ``getNodes()`` return a ``PODSpan`` rather than a container reference. A span
is a *resolved* address into pool memory, so it must not outlive the next ``Pool::reserve`` on that
pool -- re-obtain it rather than caching it across a build step.

Holding a PackedBVH inside another class
_________________________________________

``PackedBVH`` is ``final``. A class that needs a BVH over its own payload holds one by value
instead, and keeps its own arrays in the same pool so that one ``rebasedView()`` of the held BVH
rebases everything; ``PointCloudBVH`` (:ref:`Chap:ImplemPointCloud`) is the worked case. Deriving
was ruled out because ``rebasedView()`` and ``deepCopy()`` return a ``PackedBVH`` by value, which on
a derived type silently slices away the payload the leaves refer to.

Such a class has four public tools. ``buildTopology()`` and the topology-and-leaf-packer constructor
build a BVH over a stored primitive of its own choosing, as ``PointCloudBVH`` does. The adopting
constructors take node and primitive arrays built by other means, checked as described above; the
BVH unions use the host-array one to rebuild a BVH whose primitives they deep-copied. ``getNodes()``
returns the node array as a read-only ``PODSpan``, for a class that walks the tree with a traversal
of its own, as ``PointCloudBVH``'s seeded self-query does. And ``traversalStackDepth()`` gives the
fixed traversal-stack size for the current compilation pass (host or device), which the build and
``rebasedView()`` validate the tree's depth against -- a custom traversal that pushes at most ``K``
entries per node it expands, as ``pruneTraverse()`` does, can size its own stack with it and inherit
the same guarantee.

Copy and move semantics
________________________

``PackedBVH`` allows both copying and moving, but a copy is **not** a deep copy. Its members are two
``PODVector`` descriptors plus a ``PoolLocation``, so copying one copies offsets: the copy resolves
against the same pool memory as the original, and writing through either (a ``refit()``, say) is
visible through the other. That shallowness is the point -- it is what makes the type trivially
copyable, and therefore what lets ``rebasedView()`` produce a value a kernel can consume directly.
Use ``deepCopy(Pool&)`` for storage of its own. (When the primitives are indices into a caller-owned
array -- see :ref:`Sec:IndexedPrimitives` -- even a deep copy still shares that array; only the BVH's
own arrays are duplicated.)

The destructor is non-virtual: ``PackedBVH`` is not intended to be subclassed or used
polymorphically, and is declared ``final`` to enforce it (see above).

.. _Sec:PolymorphicPrimitives:

Polymorphic primitives are not currently supported
___________________________________________________

``P`` must be a concrete type. ``PackedBVH`` stores ``P`` by value, so an abstract base either fails
to compile or slices the object to its static type; naming primitives by index instead
(:ref:`Sec:IndexedPrimitives`) does not help either, since that indexes a flat array of one concrete
type, which is meaningless when the elements are of different derived types and therefore different
sizes. The ``shared_ptr``-based primitive array that could hold a polymorphic hierarchy has been
removed, because a ``shared_ptr`` is not trivially copyable and so can never cross into a device
address space.

One part of the library depended on exactly that: the BVH-accelerated CSG unions
(:ref:`Chap:ImplemCSG`), whose primitive was ``ImplicitFunction<T>`` and whose leaf evaluator called
a virtual ``value()`` through a base pointer. They have returned restricted to a single primitive
type: ``BVHUnionIF<T, P, K>`` is a ``PackedBVH<T, P, K>`` over value-type primitives of one type
``P`` -- analytic shapes, mesh distance fields, or other unions -- and is itself trivially copyable
(:ref:`Sec:BVHUnions`). A union of primitives of *different* types still needs runtime dispatch,
which returns with the redesign of the implicit-function and CSG layer that replaces virtual
dispatch with a linear-SSA tape.

Nesting a BVH inside a BVH -- a ``BVHUnion`` over several ``TriMeshSDF`` objects, say -- works
today. The inner BVHs live in the same ``Pool`` as the outer one, and the outer union relocates each
inner descriptor to its own pool location as it evaluates it, so the nested hierarchy mirrors to a
device as one piece.

Nesting itself is not the expensive part, incidentally. A ``PackedBVH`` is two ``PODVector``
descriptors plus a ``PoolLocation`` -- 56 bytes on a 64-bit platform, whatever the size of the BVH it
describes -- so a ``PackedBVH`` whose primitive is another ``PackedBVH`` copies 56 bytes per inner
BVH, not the inner BVH's contents, and the two share the pool memory those descriptors resolve
against. It is only a primitive that owns its data *inline* whose size the outer array pays per
placement.

.. _Chap:BVHRefit:

Refitting for moving geometries
-------------------------------

``PackedBVH::refit()`` recomputes every box in place, leaving the shape of the tree (the node
hierarchy and each leaf's primitive range) untouched -- the cheap way to keep a BVH valid for a
geometry whose primitives have *moved* between frames, without a rebuild. It takes a single functor
mapping one primitive to its current ``AABBT<T>``, and makes one reverse sweep over the node array:
since every child follows its parent, each node's children are refitted before the node itself. A
leaf slot's box becomes the union of its primitives' boxes, and an interior slot's box the union of
its child node's slot boxes. There is no separate cache to rebuild; the slot boxes *are* what
``pruneTraverse()`` reads. ``getPrimitives()`` has a non-const overload for moving the primitives in
place before refitting.

Because it never re-partitions, a geometry that deforms enough for primitives to migrate across the
tree accumulates looser boxes over time and should periodically be rebuilt instead; see
:ref:`Chap:BVH` for that trade-off. For the exact signature, see the Doxygen reference for
`PackedBVH <doxygen/html/classEBGeometry_1_1BVH_1_1PackedBVH.html>`__.

Refitting recomputes bounding volumes only, never the primitives themselves. That matters for
``MeshSDF`` (:ref:`Chap:MeshSDFClasses`), whose packed faces are by-value copies with a normal,
centroid and projection axes cached at build time: refitting its BVH (through ``getRoot()``) after
moving the mesh's vertices leaves those cached values stale, so ``signedDistance()`` would mix live
vertex positions with stale face data. After moving vertices, reconcile the mesh and build a new
``MeshSDF``.

.. _Chap:PruneTraverse:

Tree traversal: ``pruneTraverse()``
------------------------------------

``PackedBVH`` has no ``signedDistance()`` of its own, and does not privilege any one query. It has
exactly one traversal, ``PackedBVH::pruneTraverse(point, state, evalLeaf, pruneDist2)``: a
depth-first, distance-pruned search that visits the nearest box first and skips anything already
known to be farther than the best answer so far. The search is expressed through three cooperating
pieces supplied by the caller:

* **State** -- whatever the search needs to remember between leaf visits. Often just "the best
  value found so far", but it can be richer (e.g. a running best paired with the primitive that
  produced it). This is the only thing that persists across the whole traversal.
* **Leaf-eval** -- called once per leaf slot, as ``evalLeaf(state, offset, count)``, with the leaf's
  range in the BVH's primitive array (never a freshly-allocated sub-list). It is the *only* place
  primitives are actually touched, and the only place ``State`` is allowed to change.
* **Pruning rule** -- called on the *current* ``State`` to produce a squared-distance bound: a slot
  whose box is farther than this (in squared distance) is skipped without being visited. It never
  touches primitives directly, only whatever ``Leaf-eval`` has already written into ``State``.

Only the pruning bound is customisable. The per-slot quantity it is compared against is always the
squared Euclidean distance from the query point to the slot's box, which is a true lower bound on the
distance to anything inside it -- the condition for branch-and-bound pruning to be exact.

Traversal algorithm
___________________

The traversal keeps a stack of *slots*, not nodes. It starts by expanding the root node:

#. Compute the squared distances from the query point to all ``K`` slot boxes of the node at once
   (see :ref:`Sec:SIMDDistanceKernel`).
#. Read the pruning bound from the current ``State`` -- once per node, since nothing in this step can
   change it.
#. Collect the occupied slots whose distance is within the bound, sort them into descending distance
   with a stable insertion sort (``K`` is a handful of elements, ``std::sort`` is not callable from
   device code, and a stable sort keeps equidistant slots in slot order, so the traversal is
   deterministic), and push them farthest first, so that the **nearest slot is on top** and is
   visited next.

It then pops entries until the stack is empty. A popped entry is skipped if its stored distance now
exceeds the pruning bound, which is re-read from the current ``State`` for every entry: a leaf
visited anywhere since the entry was pushed, in any subtree, may have tightened it. Otherwise, a leaf
slot is handed to leaf-eval, and an interior slot's child node is expanded as above. An empty BVH
returns at once, without calling either callback.

Each stack entry is **8 bytes**: a ``uint32_t`` holding ``node * K + slot``, and a ``float`` lower
bound on the slot's squared distance, recorded when it was pushed. For ``T = float`` that is the
distance itself. For ``T = double`` the distance is rounded *down* to a float -- shrunk by
:math:`2^{-22}` relative before narrowing, so that rounding to the nearest float can never land above
it, with values too small to be a normal float going to zero and values past the float range going
to the largest float. The stored bound therefore never exceeds the true distance, and the check on
pop **never prunes a slot that could hold the answer**; at worst it lets a slot through that the
exact distance would have skipped, and the leaf-eval then finds nothing closer there. The decision to
push a slot at all uses the full-precision distance.

There is exactly **one** such loop, and it runs whether or not the ``(K, T)`` pair in use has a
compiled SIMD path; only the per-slot distance computation differs (see below), and every path gives
the same bits. A query answered on a build with no SIMD therefore returns exactly what the same query
returns on an AVX-512 build.

.. _Sec:TraversalDepth:

Stack size and tree depth
_________________________

The stack is a fixed-size array on the call stack -- no heap allocation, in host or device code --
of ``traversalStackDepth()`` entries. Expanding a node pops one entry and pushes at most ``K``, so a
root-to-leaf path through ``depth`` nodes peaks at :math:`(K - 1)\cdot\text{depth} + 1` entries, and
the stack is sized for a fixed depth per compilation pass:

.. list-table:: Traversal depth limits
   :widths: 30 20 50
   :header-rows: 1

   * - Constant
     - Node levels
     - Stack for ``K = 4``
   * - ``BVH::HostTraversalDepth``
     - 64
     - 193 entries (1544 bytes), in a host pass
   * - ``BVH::DeviceTraversalDepth``
     - 32
     - 97 entries (776 bytes) per thread, in a device pass

The device limit is smaller because device stack memory is per thread. Overflowing the fixed stack
would be an out-of-bounds write that a Release build performs silently, so the depth is guaranteed
up front instead, in every build:

*  **The builder never makes a tree deeper than** ``DeviceTraversalDepth``. If the requested
   strategy produces one, ``buildTopology()`` repeats the build with ``Strategy::Centroid``, whose
   equal-count splits divide every range by up to ``K`` and so give the shallowest tree. Every tree
   the library builds therefore fits both stacks.
*  **Every constructor checks the depth against** ``HostTraversalDepth``, adopted arrays included
   (with the exception, noted above, of arrays in device-only memory, which the host cannot read).
*  **rebasedView() checks every device view against** ``DeviceTraversalDepth``, when it produces a
   snapshot for a device-accessible pool -- the moment the caller commits to a device traversal, and
   the last one that still runs on the host where it can say so. A host-to-host mirror is traversed
   on the host, and is not checked again; arrays adopted in device-only memory are not checked here
   either, for the same reason as above.

A violation aborts with a message saying that the tree is so many levels deep, more than the levels a
traversal stack holds. Because the limit is in node levels, it holds for every ``K``; a wider node
makes each level's stack share larger, not the number of levels smaller.

.. _Sec:SIMDDistanceKernel:

The vectorised distance kernel
______________________________

The squared distances from the query point to all ``K`` slot boxes of a node are computed in one
batch, dispatched at compile time on ``(T, K)`` and the compiled ISA:

*  AVX-512F: ``(double, 8)`` and ``(float, 16)``, one 512-bit load per box row.
*  AVX: ``(double, 4)`` and ``(float, 8)`` in one 256-bit pass each, and ``(double, 8)`` as two
   4-wide passes when AVX-512F is not available.
*  SSE4.1: ``(float, 4)``, one 128-bit load per box row.
*  Every other combination, and all device code, uses a scalar loop over ``WideNode::getDistance2()``.

Every path computes :math:`\max(0, \max(lo - p, p - hi))` per axis, then
:math:`d_x^2 + (d_y^2 + d_z^2)` in that association order, so all of them agree bit for bit. The SIMD
paths load whole box rows, which is why ``WideNode`` aligns them (see :ref:`Sec:WideNode`); a
``static_assert`` fires at compile time if a row's alignment ever stops matching the load. See
:ref:`Chap:SIMDClasses` for the instructions themselves.

Splitting the pruning rule apart from the leaf-eval like this is what lets a primitive with no
notion of "signed distance" reuse the same vectorised box test: a nearest-neighbor search over a
point cloud can track a plain running squared distance as its ``State`` (no ``abs()``, no extra
squaring, no square root anywhere in the hot path) with a pruning rule that returns the state
unchanged, whereas ``MeshSDF``/``TriMeshSDF::signedDistance()`` (see :ref:`Chap:MeshSDFClasses`)
track a signed distance and square its magnitude for the bound -- both are ordinary instantiations
of the same ``pruneTraverse()``, not special cases hardcoded into ``PackedBVH``.

For the exact template signature and callback contracts, see `the doxygen page for
PackedBVH <doxygen/html/classEBGeometry_1_1BVH_1_1PackedBVH.html>`__.

Traversal examples
__________________

Below, we consider two examples for BVH traversal.
The examples show how we compute the signed distance from a DCEL mesh, and how to perform a *smooth* CSG union where the search for the two closest objects is done by BVH traversal.

Signed distance
^^^^^^^^^^^^^^^

The DCEL mesh distance fields use a traversal pattern based on

* Only visit bounding volumes that are closer than the minimum distance computed (so far).
* When visiting a subtree, investigate the closest bounding volume first.
* When visiting a leaf, check if the primitives are closer than the minimum distance computed so far.

``MeshSDF::signedDistance()`` implements these rules through ``pruneTraverse()``: its leaf-eval
scans a leaf's faces and keeps the signed distance with the smallest magnitude seen so far, and its
pruning rule returns the square of that magnitude, while ``pruneTraverse()`` itself visits the
closest slot first. For the full API, see the Doxygen reference for
`MeshSDF <doxygen/html/classEBGeometry_1_1MeshSDF.html>`__.

CSG Union
^^^^^^^^^

Combinations of implicit functions in EBGeometry into aggregate objects can be done by means of CSG unions.
One such union is known as the *smooth union*, in which the transition between two objects is gradual rather than abrupt.

``BVHSmoothUnionIF::signedDistance()`` drives ``pruneTraverse()`` with a ``State`` holding the two
smallest values seen so far, ``a`` and ``b`` (``a`` the closest, ``b`` the second-closest): the
leaf-evaluator updates both as leaves are scanned, and the pruning rule returns ``max(0, b)``
squared -- pruning against the *second*-smallest value rather than the nearest, so a primitive that
is not the single closest but still contributes to the blend is never pruned away. Once traversal
completes, the two values are blended with the stored smooth-minimum operator.
``BVHUnionIF::signedDistance()`` is the same pattern with a single running minimum and a
``max(0, minDist)``-squared pruning bound. See :ref:`Chap:ImplemCSG` for the CSG combinators
themselves, and the Doxygen reference for
`BVHSmoothUnionIF <doxygen/html/classEBGeometry_1_1BVHSmoothUnionIF.html>`__ /
`BVHUnionIF <doxygen/html/classEBGeometry_1_1BVHUnionIF.html>`__ for the exact API.

.. _Chap:MeshSDFClasses:

Mesh SDF classes
----------------

EBGeometry provides three concrete classes for evaluating signed distances to surface meshes.
They share the same sign convention (negative inside, positive outside) but differ in data layout,
BVH type, and supported geometry:

.. list-table:: Mesh SDF classes
   :widths: 22 18 22 18 20
   :header-rows: 1

   * - Class
     - Input
     - BVH type
     - Traversal
     - Notes
   * - ``FlatMeshSDF<T, Meta>``
     - DCEL mesh
     - None
     - O(N) scan
     - Debug / tiny meshes only; no build cost
   * - ``MeshSDF<T, Meta, K>``
     - DCEL mesh
     - ``PackedBVH`` over ``DCEL::FaceT``
     - ``pruneTraverse()`` (SIMD when ``(K, T)`` matches a compiled ISA path)
     - Any polygon mesh; not restricted to triangles
   * - ``TriMeshSDF<T, Meta, K, W>``
     - DCEL mesh or triangle soup
     - ``PackedBVH`` over ``TriangleAoSoA`` groups
     - ``pruneTraverse()`` over SoA-packed leaves
     - Triangle meshes only; highest throughput; metadata via ``getClosestTriangle()``

``FlatMeshSDF`` is useful for correctness checks and tiny meshes. See `its doxygen page
<doxygen/html/classEBGeometry_1_1FlatMeshSDF.html>`__.

``FlatMeshSDF`` is a plain value type that can be evaluated on a GPU. It holds the DCEL mesh
descriptor by value and nothing else, so it is trivially copyable, and ``signedDistance()`` and
``computeBoundingVolume()`` (the vertex AABB) are callable on both host and device. It deliberately
does not derive from ``SignedDistanceFunction``: a class with virtual functions carries a pointer to
a host-side function table and can never be passed to a kernel. As a consequence it cannot currently
be used where an ``ImplicitFunction`` is expected, such as the CSG and transform factories. As for
``PackedBVH`` (see :ref:`Chap:MemoryModel`), freeze and mirror the pool, call
``rebasedView(devicePool)``, and pass the returned ``FlatMeshSDF`` to a kernel; ``deepCopy(pool)``
gives independent storage. Copies share the pool memory, and a ``FlatMeshSDF`` sees the mesh as it
was when it was constructed. ``MeshSDF`` and ``TriMeshSDF`` follow the same pattern; see below.

``MeshSDF`` handles arbitrary polygon meshes. Its constructor, ``MeshSDF(mesh, pool, spec)``, pairs
every face with the bounding box of its vertices and builds the BVH from those pairs with the given
``BVH::BuildSpec`` (``maxLeafSize`` counts faces); it has no default arguments, and
``Parser::readIntoPackedBVH`` supplies the recommended default. Its ``signedDistance()`` supplies the
leaf-eval and pruning rule shown above to ``PackedBVH::pruneTraverse()``, picking up the vectorised
box test whenever ``(K, T)`` matches a compiled ISA path and testing the slots with a scalar loop
otherwise. See `its doxygen page <doxygen/html/classEBGeometry_1_1MeshSDF.html>`__.

``MeshSDF`` and ``TriMeshSDF`` are plain value types exactly like ``FlatMeshSDF``: ``MeshSDF``
holds the mesh descriptor and its ``PackedBVH`` by value, ``TriMeshSDF`` just its ``PackedBVH``,
all reserved from the one ``Pool`` passed to the constructor. Both are trivially copyable,
constructed without ``shared_ptr``\ s, and neither derives from ``SignedDistanceFunction``.
``signedDistance()``, ``TriMeshSDF::getClosestTriangle()``, ``getRoot()`` and
``computeBoundingVolume()`` are callable on host and device; ``rebasedView(pool)`` and
``deepCopy(pool)`` return the class itself, so a rebased copy is what a kernel receives.
``MeshSDF::getClosestFaces()`` runs on ``pruneTraverse()`` too, keeping every face that is at least
as close as all faces visited before it, but stays host-only, since it returns a ``std::vector``. As
for ``FlatMeshSDF``, none of the three can currently be used as an ``ImplicitFunction`` (in the CSG
or transform factories).

``TriMeshSDF`` is the recommended default for triangle meshes. Its constructors,
``TriMeshSDF(mesh, pool, spec)`` and ``TriMeshSDF(triangles, pool, spec)``, build the tree over one
box per triangle with ``buildTopology()``, then pack each leaf's triangles into Structure-of-Arrays
groups of width ``W`` through the topology-and-leaf-packer constructor (see :ref:`Chap:PackedBVH`),
with no batching across leaves. ``BuildSpec::maxLeafSize`` counts triangles, so a leaf of up to
``maxLeafSize`` triangles becomes up to :math:`\lceil \texttt{maxLeafSize} / W \rceil` groups, the
last one padded. ``signedDistance()`` is the same kind of thin ``pruneTraverse()`` wrapper as
``MeshSDF``'s, over ``TriangleAoSoA<T, Meta, W>`` leaves instead of individual faces, so that on a
matching ``(K, T)`` combination each group evaluates ``W`` triangles with a single SIMD register
operation, and even the box-vs-running-best comparisons during descent are done on squared distances
(no square root) until the very last step. See `its doxygen page
<doxygen/html/classEBGeometry_1_1TriMeshSDF.html>`__, and `the doxygen page for TriangleSoAT
<doxygen/html/structEBGeometry_1_1TriangleSoAT.html>`__ for the SoA storage itself.

Just as ``MeshSDF::getClosestFaces()`` recovers the nearest face (and its ``Meta``) for a DCEL mesh,
``TriMeshSDF::getClosestTriangle()`` recovers the nearest triangle's signed distance *and* its
metadata through the SIMD SoA path -- the supported route when you need both maximum SIMD throughput
and per-triangle metadata retrieval. Each leaf group is a ``TriangleAoSoA<T, Meta, W>``: a
geometry-only ``TriangleSoAT<T, W>`` plus a physically-separate per-lane ``Array<Meta, W>`` (the
same metadata-carrying wrapper relationship ``PointAoSoA`` has with ``PointSoAT``). The hot
``signedDistance()`` path never reads the metadata array; only ``getClosestTriangle()`` does, taking a
scalar per-lane step to recover the winning lane. See `the doxygen page for TriangleAoSoA
<doxygen/html/structEBGeometry_1_1TriangleAoSoA.html>`__.

What is actually vectorised in ``TriMeshSDF``/``PackedBVH`` is covered in
:ref:`Chap:SIMDClasses` -- see that page for the full detail rather than repeating it here.

Primitive storage: Facets or triangles
______________________________________

Both classes store their primitives inline, by value, but what that copy *means* differs:

*  ``MeshSDF``'s primitive is a ``DCEL::FaceT<T, Meta>``: a plain, trivially-copyable value -- every
   member is a scalar, a ``Vec3``, or a ``uint32_t`` index -- so copying one into the packed array
   is cheap and free of aliasing. What a copied face is *not* is self-contained: it stores its
   half-edge as an index into its owning mesh's edge array rather than a self-resolving reference,
   so it is only meaningful together with that mesh. ``MeshSDF`` therefore retains the source mesh
   and passes it to every face query that has to resolve topology (point-in-face tests, signed
   distance).
*  ``TriMeshSDF``'s primitive is a ``TriangleAoSoA<T, Meta, W>`` group, which, unlike
   ``DCEL::FaceT``, is fully self-contained -- a plain aggregate of coordinate arrays plus a
   per-lane metadata array, with no index into anything else, built fresh by the leaf packer during
   construction. Nothing outside the BVH owns those groups, so there is no array for an index to
   refer to even in principle.

Neither is affected by copying the distance field itself: a copy of a ``MeshSDF``/``TriMeshSDF``
is a copy of its descriptors, which resolve against the same pool memory, so the packed data exists
exactly once no matter how many copies refer to it -- including copies stored as the primitives of
a BVH union (:ref:`Sec:BVHUnions`). Placing one mesh at several different positions is not
currently possible, however: the ``Translate``/``Rotate``/``Scale`` wrappers take an
``ImplicitFunction``, which the mesh distance fields are not (see :ref:`Chap:ImplemCSG`).

Default and host-tuned K and W
______________________________

``BVH::DefaultBranchingRatio<T>()`` and ``EBGeometry::TriangleSoA::DefaultWidth<T>()`` are the
template defaults for ``Parser::readIntoTriangleBVH`` (``TriMeshSDF`` itself has no defaults). Both
are 4 for ``float`` and ``double``, whatever the compiler flags, so a type spelled with them is the
same type in every file and in both passes of a GPU compile. ``BVH::HostBranchingRatio<T>()`` and
``TriangleSoA::HostWidth<T>()`` give the values that fill one SIMD register under the compiler's
flags, for host-only code; see :ref:`Sec:DefaultKW` for when to use which, and a measurement.

.. list-table:: ``HostBranchingRatio<T>()`` and ``TriangleSoA::HostWidth<T>()`` by ISA and precision
   :widths: 25 25 25 25
   :header-rows: 1

   * - ISA
     - Precision
     - ``HostBranchingRatio<T>()``
     - ``TriangleSoA::HostWidth<T>()``
   * - AVX-512F
     - ``float``
     - 16
     - 16
   * - AVX-512F
     - ``double``
     - 8
     - 8
   * - AVX (256-bit)
     - ``float``
     - 8
     - 8
   * - AVX (256-bit)
     - ``double``
     - 4
     - 4
   * - SSE4.1 / scalar
     - ``float`` or ``double``
     - 4
     - 4

The K=16/float and K=8/double paths use 512-bit-wide SIMD loads on AVX-512F and require each
``WideNode`` box row to be 64-byte aligned, which ``alignas(sizeof(T)*K)`` on the rows guarantees.
The K=8/float and K=4/double paths use 256-bit-wide AVX loads instead (as does K=8/double without
AVX-512F, in two passes), and K=4/float uses 128-bit SSE4.1 loads. All other (K, T) combinations
test the slots with a scalar loop over the same rows, inside the same ``pruneTraverse()``.

Each ``TriangleSoAT<T, W>`` block is likewise ``alignas``-aligned to its own SIMD register width
(64 bytes for ``<float, 16>``/``<double, 8>``, 32 bytes for ``<float, 8>``, 16 bytes for
``<float, 4>``/``<double, 4>``), and the library inserts ``static_assert`` checks that fire at
compile time if the alignment invariant is violated.

Choosing W and K explicitly
______________________________

``W`` and the BVH branching factor ``K`` are explicit template parameters on ``TriMeshSDF`` and
``Parser::readIntoTriangleBVH`` -- the latter defaults to ``BVH::DefaultBranchingRatio<T>()`` and
``TriangleSoA::DefaultWidth<T>()``, but either can be supplied explicitly (e.g. an 8-wide SoA
packing together with a 4-ary BVH). See `the doxygen page for
Parser::readIntoTriangleBVH <doxygen/html/namespaceEBGeometry_1_1Parser.html>`__ for the exact
signature.

Rules of thumb:

* Keep ``W`` equal to ``EBGeometry::TriangleSoA::DefaultWidth<T>()`` unless you
  have a specific reason to deviate, and never deviate for a type that device code also uses.
* ``BuildSpec::maxLeafSize`` counts triangles. ``Parser::readIntoTriangleBVH`` defaults it to
  ``4 * W`` -- four full SoA groups per leaf -- with binned SAH (the ``TriMeshSDF`` constructors have
  no default), while SAH is still free to split down to smaller, tighter leaves wherever the geometry
  calls for it. A multiple of ``W`` keeps every lane of a full leaf busy; a leaf whose triangle count
  is not a multiple of ``W`` pads its last group's unused lanes.
* ``K = BVH::DefaultBranchingRatio<T>()`` is a good default. With AVX-512F
  available you can try ``K = 16`` (float) -- the slot-box test is evaluated in
  a single SIMD batch, and the wider fan-out reduces tree depth -- but measure: on the
  benchmark in :ref:`Sec:DefaultKW` it was no faster. The traversal stack also grows with ``K``:
  :math:`(K - 1)\cdot 32 + 1` entries of 8 bytes per device thread, 481 entries (about 3.8 KB) for
  ``K = 16`` (see :ref:`Sec:TraversalDepth`).
