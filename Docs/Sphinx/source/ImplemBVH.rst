.. _Chap:ImplemBVH:

BVH
===

See :ref:`Chap:BVH` for the conceptual picture of bounding volume hierarchies (node types,
partitioning, tree pruning during traversal) before reading the concrete API below.

The BVH functionality is encapsulated in the namespace ``EBGeometry::BVH``
(:file:`Source/EBGeometry_BVH.hpp` / :file:`EBGeometry_BVHImplem.hpp`).
For the full API, see `the doxygen API <doxygen/html/namespaceEBGeometry_1_1BVH.html>`__.
There are two representations of a BVH:

*  ``BVH::TreeBVH<T, P, BV, K>``, a pointer-based tree used while *building* and
   *partitioning* the hierarchy. See `the doxygen page for TreeBVH
   <doxygen/html/classEBGeometry_1_1BVH_1_1TreeBVH.html>`__.
*  ``BVH::PackedBVH<T, P, K, StoragePolicy>``, where the nodes are stored in depth-first order in
   a flat array and contain index offsets to children and primitives rather than pointers. This is
   the representation used for fast queries, see :ref:`Chap:PackedBVH`. See `the doxygen page
   for PackedBVH <doxygen/html/classEBGeometry_1_1BVH_1_1PackedBVH.html>`__.

The template parameters shared by both are:

*  ``T`` Floating-point precision.
*  ``P`` Primitive type. Neither representation imposes an interface requirement of its own:
   ``TreeBVH`` construction/partitioning only ever calls ``getCentroid()`` on it, and
   ``PackedBVH`` holds primitives opaquely, handing them back only to whatever leaf-visit
   callback a caller supplies to ``traverse()`` or ``pruneTraverse()`` (see below). Whether
   ``P`` needs a ``signedDistance(Vec3T<T>)`` member (or anything else) is entirely up to that
   callback -- see ``MeshSDF``/``TriMeshSDF::signedDistance()`` in :ref:`Chap:MeshSDFClasses` for
   the signed-distance case, and the nearest-neighbor example in ``Tests/TestBVH.cpp`` for a
   primitive with no ``signedDistance()`` at all.
*  ``BV`` Bounding volume type (``TreeBVH`` only — ``PackedBVH`` always uses
   ``BoundingVolumes::AABBT<T>`` internally).
*  ``K`` BVH degree. ``K=2`` will yield a binary tree, ``K=3`` yields a tertiary tree and so on.

``TreeBVH`` is the BVH builder node, i.e. it is the class through which we recursively build the
BVH, see :ref:`Chap:BVHConstruction`. ``TreeBVH`` has no constraint that ``P`` be wrapped in a
``shared_ptr`` itself, but the *containers* that hold primitives during construction
(``PrimitiveList<P>``, ``PrimAndBV<P, BV>``) always wrap each primitive in a
``std::shared_ptr<const P>``, so that primitives can be safely shared between the tree and any
higher-level object that still refers to them by pointer (e.g. a ``DCEL::FaceT``).

Bounding volumes
----------------

EBGeometry supports the following bounding volumes, which are defined in :file:`Source/EBGeometry_BoundingVolumes.hpp`:

*  **Bounding sphere**, templated as ``EBGeometry::BoundingVolumes::SphereT<T>``.
   Various constructors are available. See `the doxygen page for SphereT
   <doxygen/html/classEBGeometry_1_1BoundingVolumes_1_1SphereT.html>`__.

*  **Axis-aligned bounding box**, which is templated as ``EBGeometry::BoundingVolumes::AABBT<T>``.
   See `the doxygen page for AABBT
   <doxygen/html/classEBGeometry_1_1BoundingVolumes_1_1AABBT.html>`__.

For full API details, see `the doxygen API <doxygen/html/namespaceEBGeometry_1_1BoundingVolumes.html>`_.
Other types of bounding volumes can in principle be added, with the only requirement being that they conform to the same interface as the ``AABB`` and ``BoundingSphere`` volumes. Note that
``PackedBVH`` hard-codes ``AABBT<T>`` as its bounding volume (see above), so a custom bounding
volume can only be used with ``TreeBVH`` while building, and must still be convertible to an AABB
before the tree is packed.

.. _Chap:BVHConstruction:

Construction
------------

Building a ``TreeBVH`` always starts from a list of ``(primitive, bounding volume)`` pairs; at
that point the tree is a single, unpartitioned leaf holding every primitive. Partitioning it into
an actual hierarchy is then a separate step, done by calling one of the partitioning member
functions described below -- this recursively subdivides the tree in place.

.. tip::

   The default construction methods perform the hierarchical subdivision by only considering the *bounding volumes*.
   Consequently, the build process is identical regardless of what type of primitives (e.g., triangles or analytic spheres) are contained in the BVH.

Top-down construction
______________________

Top-down construction is done through the member function ``topDownSortAndPartition()``, which
takes two optional arguments: a *partitioner* and a *leaf predicate*.

The partitioner is a functor that splits a list of ``(primitive, BV)`` pairs into ``K`` new
lists whenever a leaf is subdivided. Three ready-made partitioners are provided:
``BVCentroidPartitioner`` (splits on bounding-volume centroids along the longest axis -- the
default), ``PrimitiveCentroidPartitioner`` (the same idea, but splits on primitive centroids
instead), and ``BinnedSAHPartitioner`` (a Surface-Area-Heuristic partitioner, used automatically
when building via ``BVH::Build::SAH`` -- see below -- and typically producing the
best-performing trees at a higher construction cost). The leaf predicate takes a ``TreeBVH`` node
and decides whether it should become a leaf (i.e. not be split any further); a default is provided,
but callers are free to supply their own of either kind.

Bottom-up construction
________________________

The bottom-up construction uses a space-filling curve (e.g., a Morton curve) for first building the leaf nodes.
This construction is done such that each leaf node contains approximately the number of primitives, and all leaf nodes exist on the same level.
To use bottom-up construction, one may use the member function template
``bottomUpSortAndPartition<S>()`` (no arguments).
The template argument is the space-filling curve that the user wants to apply, from namespace
``EBGeometry::SFC`` (:file:`Source/EBGeometry_SFC.hpp`, see `the doxygen API
<doxygen/html/namespaceEBGeometry_1_1SFC.html>`__).
Currently, we support Morton codes and nested indices.
For Morton curves, one would e.g. call ``bottomUpSortAndPartition<SFC::Morton>`` while for nested indices (which are not recommended) the signature is likewise ``bottomUpSortAndPartition<SFC::Nested>``.

Build times for SFC-based bottom-up construction are generally speaking faster than top-down construction, but it tends to produce worse trees such that traversal becomes slower. For the full API,
see the Doxygen reference for
`TreeBVH <doxygen/html/classEBGeometry_1_1BVH_1_1TreeBVH.html>`__.

.. tip::

   Higher-level entry points such as ``Parser::readIntoPackedBVH`` don't require you to
   call ``topDownSortAndPartition``/``bottomUpSortAndPartition`` directly — they take a single
   ``BVH::Build`` enum value (``TopDown``, ``Morton``, ``Nested``, or ``SAH``) and dispatch to the
   corresponding construction method internally. See :ref:`Chap:Parsers`.

.. _Chap:PackedBVH:

PackedBVH
---------

In addition to the standard BVH node ``TreeBVH<T, P, BV, K>``, EBGeometry provides a ``PackedBVH`` where nodes are stored in depth-first order in a flat array.
The ``PackedBVH`` can be automatically constructed from a ``TreeBVH`` but not vice versa.

.. figure:: /_static/CompactBVH.png
   :width: 240px
   :align: center

   PackedBVH representation.
   The original BVH is traversed from top-to-bottom along the branches and laid out in linear memory.
   Each interior node stores index offsets to its children and primitives.

The rationale for the ``PackedBVH`` is its tighter memory footprint and depth-first ordering, which allows more efficient traversal, particularly when primitives are sorted in the same order.
``PackedBVH<T, P, K, StoragePolicy>`` is templated on the same ``T``, ``P``, ``K`` as ``TreeBVH``
(its bounding volume is always ``AABBT<T>``), plus a fourth ``StoragePolicy`` parameter described
below, and internally stores two things: a flat, depth-first array of nodes, and a global
primitive array holding every primitive in leaf order.

Each entry of the node array plays the same role a ``TreeBVH`` node plays, but stores offsets
into the flat arrays rather than pointers to children: a bounding volume for the node's subtree,
a primitive offset and count identifying its range in the global primitive array (used only if
the node is a leaf), and the depth-first indices of its ``K`` children. A node is a leaf exactly
when its primitive count is non-zero; the root node is always at index 0 of the node array. See
`the doxygen page for PackedBVH::Node
<doxygen/html/structEBGeometry_1_1BVH_1_1PackedBVH_1_1Node.html>`__ for the exact member list.

Constructing a ``PackedBVH`` is simply a matter of flattening an already-partitioned ``TreeBVH``,
via one of two ``TreeBVH`` member functions:

*  ``pack()`` performs a straight flatten: the resulting ``PackedBVH<T, P, K>`` stores exactly
   the same primitive type ``P`` that the source tree held. The original tree is left untouched
   and can simply be discarded (or allowed to go out of scope) once it is no longer needed.
*  ``packWith<Q, Converter>()`` additionally *converts* the primitive type while flattening: the
   source tree holds primitives of type ``P``, and a user-supplied ``Converter`` is called once
   per leaf to produce the ``Q`` values that the resulting ``PackedBVH<T, Q, K>`` will store.
   This is how ``TriMeshSDF`` turns a tree of individual ``Triangle<T, Meta>`` primitives into a
   ``PackedBVH`` whose leaves hold SIMD-width ``TriangleSoAT<T, W>`` groups instead — see
   :ref:`Chap:MeshSDFClasses`.

See `the doxygen page for TreeBVH <doxygen/html/classEBGeometry_1_1BVH_1_1TreeBVH.html>`__ for
the exact signatures of ``pack()`` and ``packWith()``.

Storage policy
______________

``pack()``/``packWith()`` both take an optional ``StoragePolicy`` template argument controlling
*how* the resulting ``PackedBVH``'s global primitive array stores each primitive. A storage
policy is a stateless struct exposing a ``StorageType`` type alias plus the static methods
``get()``, ``appendTreeLeaf()``, and ``appendAliased()`` that ``PackedBVH`` calls internally when
packing and querying; see `the BVH namespace's doxygen page
<doxygen/html/namespaceEBGeometry_1_1BVH.html>`__ for the exact member list each one must provide.
Two are provided out of the box:

*  ``BVH::SharedPtrStorage<P>`` (the default for both ``pack()`` and ``packWith()``) stores each
   primitive as a ``std::shared_ptr<const P>``, matching the pre-existing behavior of every
   caller that does not name a ``StoragePolicy`` explicitly. Primitives are shared with whatever
   else still holds a pointer to them (e.g. a ``DCEL::FaceT`` referenced by a ``TreeBVH``), at the
   cost of one pointer indirection per access during traversal.
*  ``BVH::ValueStorage<P>`` stores each primitive inline, by value: ``StorageType`` is ``P``
   itself, with no indirection at all. This trades away sharing (the ``PackedBVH`` now owns an
   independent copy of every primitive) for a smaller, more cache-friendly primitive array — most
   useful when ``P`` is a small, cheaply-copyable value type (e.g. a point or particle) rather
   than something large or already reference-counted elsewhere.

Both policies are drop-in compatible with every existing ``PackedBVH`` consumer: swapping the
policy only changes the element type of ``getPrimitives()`` and the leaf-primitive vector handed
to ``LeafEvaluator``/``PackedLeafEvaluator`` callbacks, never the tree structure, traversal order,
or query results. A caller that wants ``ValueStorage`` instead of the default simply names it
explicitly, e.g. ``tree->pack<BVH::ValueStorage<P>>()``.

Copy and move semantics
________________________

``TreeBVH`` and ``PackedBVH`` differ in whether copying is allowed, precisely because of the
storage-sharing question above:

*  ``TreeBVH`` deletes its copy constructor and copy assignment operator. It is a recursive
   structure of ``shared_ptr``-linked children, so a naive (compiler-generated) copy would only
   alias the same child subtrees rather than cloning them, which
   ``topDownSortAndPartition()``/``bottomUpSortAndPartition()`` could then mutate out from under a
   supposedly independent "copy". Copying is disallowed outright rather than silently doing the
   wrong thing. Its move constructor and move assignment operator are explicitly defaulted and
   fully supported. To *replicate* a tree independently -- e.g. to build once and then partition
   two copies with different strategies, or to keep a pristine copy alongside one you go on to
   mutate -- use ``deepCopy()``, which recursively clones the node hierarchy (returning a new
   ``std::shared_ptr<TreeBVH>``) while still sharing the immutable ``std::shared_ptr<const P>``
   primitives by handle. (Copying a ``std::shared_ptr<TreeBVH>`` is, of course, always fine -- that
   is shared ownership of the *same* tree, not a replica.)
*  ``PackedBVH`` allows both copying and moving. Its members (the flattened node array, the
   primitive array, and the SIMD AABB cache) are all owned value containers with no shared
   mutable substructure, so the compiler-generated deep copy is correct and safe under both
   ``BVH::SharedPtrStorage`` (primitives are aliased ``shared_ptr``, the same sharing model as
   ``TreeBVH``) and ``BVH::ValueStorage`` (primitives are copied by value -- safe as long as the
   primitive type's own copy constructor is complete; see the note on ``DCEL::FaceT`` in
   :ref:`Chap:MeshSDFClasses` for a case where it deliberately is not).

Both classes' destructors are non-virtual: neither is intended to be subclassed or used
polymorphically.

When ``ValueStorage`` is the wrong choice
_________________________________________

There are two situations where ``ValueStorage`` should not be used and ``SharedPtrStorage`` (the
default) must be kept:

*  **Polymorphic primitives.** ``ValueStorage<P>`` stores ``P`` by value, so it requires ``P`` to
   be a concrete, copyable value type. If ``P`` is an abstract base (or you rely on virtual
   dispatch through a base pointer), value storage either fails to compile or slices the object to
   its static type. This is exactly the situation of the BVH-accelerated CSG unions
   (:ref:`Chap:ImplemCSG`), whose primitive is ``ImplicitFunction<T>`` and whose leaf evaluator
   calls a virtual ``value()`` through a ``std::shared_ptr<const ImplicitFunction<T>>``; those
   classes therefore always use ``SharedPtrStorage`` and do not expose the policy. Use
   ``ValueStorage`` only when ``P`` is a self-contained value type (a point, a particle, an SoA
   triangle group), never for a polymorphic hierarchy.
*  **Nesting a BVH inside a BVH.** A ``PackedBVH`` whose primitive is itself another
   ``PackedBVH`` (or a mesh SDF that owns one) is a supported construction, and nothing stops it
   recursing further — ``PackedBVH`` of ``PackedBVH`` of ``PackedBVH``, to any depth. At every
   level the *outer* ``PackedBVH`` should stay on ``SharedPtrStorage`` so it shares each inner BVH
   by pointer. Naming ``ValueStorage`` on an outer level instead copies every inner ``PackedBVH``
   wholesale — its node, SoA, and primitive arrays — into the outer array: packing draws each
   primitive from the source tree as a ``std::shared_ptr<const P>``, so each inner BVH is *copied*
   (not moved) into place, even though ``PackedBVH`` is otherwise fully movable (see *Copy and move
   semantics* above). The cost of that copy is proportional to the *entire* memory footprint
   reachable below the primitive, so it becomes exceedingly expensive whenever an inner BVH is
   large, and compounds with nesting depth: each by-value level duplicates everything beneath it,
   which may itself be duplicating everything beneath *it*. ``SharedPtrStorage`` at every outer
   level avoids this for no loss of correctness. The common realisation of nesting — a
   ``BVHUnion`` over several mesh SDFs, each holding its own inner ``PackedBVH`` — is exactly the
   polymorphic-primitive case above, so it already sits on ``SharedPtrStorage`` at the outer level
   and shares each mesh SDF by pointer.

Tree traversal
---------------

Both ``TreeBVH`` (full BVH) and ``PackedBVH`` (flattened BVH) include routines for traversing the
BVH with user-specified criteria. For both BVH representations, tree traversal is done through a
single ``traverse()`` member function taking four caller-supplied callbacks, and uses a
stack-based traversal pattern driven by those callbacks. The full type signatures for the four
callback roles below are documented on `the BVH namespace's doxygen page
<doxygen/html/namespaceEBGeometry_1_1BVH.html>`__ (look for ``PrunePredicate``, ``ChildOrderer``/
``PackedChildOrderer``, ``LeafEvaluator``/``PackedLeafEvaluator``, and ``NodeKeyFactory``).

Node visit
__________

The *prune-predicate* callback decides, for a given node and its associated node key (see below),
whether that node's subtree should be investigated or pruned from the traversal: it is a
predicate taking the node and its node key, returning ``true`` to visit the subtree and
``false`` to prune it. Typically, the node key will contain the necessary information
that determines whether or not to visit the subtree.

Child ordering
______________

If a subtree is visited in the traversal, there is a question of which of the child nodes to visit first.
The *child-orderer* callback determines this order by letting the user sort the ``K`` children (each
paired with its node key) in-place based on order of importance -- for ``PackedBVH`` the
children are identified by their node index rather than a pointer, halving the per-entry stack
size relative to ``TreeBVH``. Note that a correct visitation pattern can yield large performance
benefits. Ordering the child nodes is completely optional; the user can leave this function empty
if it does not matter which subtrees are visited first.

Leaf evaluation
_______________

If a leaf node is visited in the traversal, distance or other types of queries to the geometric
primitive(s) in the node are usually made. These are done by the *leaf-evaluator* callback. For
``PackedBVH`` this callback receives an offset and count into the BVH's global primitive array,
rather than a freshly-allocated sub-list, avoiding a heap allocation per leaf visit; for
``TreeBVH`` it receives the leaf's primitive list directly. Typically, the leaf-evaluator will modify
parameters that appear in a local scope outside of the tree traversal (e.g. updating the minimum
distance to a DCEL mesh).

Node key
________

During the traversal, it might be necessary to compute a per-node key that is helpful during the traversal, and this key is attached to each node that is queried.
This key is usually, but not necessarily, equal to the distance to the nodes' bounding volumes.
The *node-key-factory* callback produces this key for a node's children, given the node itself.
The biggest difference between the leaf-evaluator and the node-key-factory is that the leaf-evaluator is *only*
called on leaf nodes whereas the node-key-factory is also called for internal nodes. One typical
example for DCEL meshes is that the leaf-evaluator computes the distance from an input point to the
triangles in a leaf node, whereas the node-key-factory computes the distance from the input point to
the bounding volumes of a child node. This information is then used by the child-orderer in order to
determine a preferred child visit pattern when descending along subtrees.

Traversal algorithm
___________________

``PackedBVH::traverse()`` implements this with a non-recursive, vector-backed stack rather than
recursion. Each stack entry holds a node index together with that node's already-computed
node key. The root is pushed first; then, until the stack is empty, the traversal pops an entry,
asks the prune-predicate whether to visit it, and if so either runs the leaf-evaluator (if it is a leaf) or
computes the node-key-factory for each of its ``K`` children, lets the child-orderer reorder them, and
pushes them all onto the stack. For the full API, see the Doxygen reference for
`PackedBVH <doxygen/html/classEBGeometry_1_1BVH_1_1PackedBVH.html>`__.

.. _Chap:PruneTraverse:

Distance-pruned traversal: ``pruneTraverse()``
________________________________________________

``PackedBVH`` has no ``signedDistance()`` of its own, and does not privilege any one query.
Alongside the generic ``traverse()`` described above, it exposes a second traversal,
``PackedBVH::pruneTraverse()``, that implements the same "closest bounding volume first, prune
anything already known to be farther than the best answer so far" strategy, but with two
differences: the box-vs-point distance test is SIMD-batched across all ``K`` children at once
(see :ref:`Chap:SIMDClasses` for exactly which instructions run for which ``(K, T)``, and the
list-table below for the ISA-to-``K`` mapping), and -- unlike ``traverse()``'s four independent
callbacks -- the search is expressed through exactly three cooperating pieces supplied by the
caller:

* **State** -- whatever the search needs to remember between leaf visits. Often just "the best
  value found so far", but it can be richer (e.g. a running best paired with the primitive that
  produced it). This is the only thing that persists across the whole traversal.
* **Leaf-eval** -- called once per leaf, with the leaf's offset and count into the BVH's global
  primitive array (never a freshly-allocated sub-list). It is the *only* place primitives are
  actually touched, and the only place ``State`` is allowed to change.
* **Pruning rule** -- called on the *current* ``State`` to produce a squared-distance bound: a
  node farther than this (in squared distance) is skipped without being visited. It never touches
  primitives directly, only whatever ``Leaf-eval`` has already written into ``State``.

Precisely, the traversal seeds a stack with the root node (at distance zero, so it is never
pruned), then repeatedly pops the top entry and: skips it outright if its already-known squared
distance to the query point exceeds the pruning rule's *current* bound; otherwise, if it is a
leaf, calls leaf-eval once for the whole leaf; otherwise (an interior node), computes all ``K``
children's squared distances to the query point in a single SIMD batch, sorts them so the
closest child is visited next, and pushes every child still within the (freshly re-evaluated)
pruning bound. Because the bound is re-read from the current ``State`` at every node visited --
never cached from the start of the traversal -- a leaf visited anywhere earlier on the stack
immediately tightens the pruning applied to every node visited afterwards, regardless of which
subtree it came from.

Splitting the pruning rule apart from the leaf-eval like this is what lets a primitive with no
notion of "signed distance" reuse the same SIMD box test: a nearest-neighbor search over a point
cloud can track a plain running squared distance as its ``State`` (no ``abs()``, no extra
squaring, no square root anywhere in the hot path) with a pruning rule that returns the state
unchanged, whereas ``MeshSDF``/``TriMeshSDF::signedDistance()`` (see :ref:`Chap:MeshSDFClasses`)
track a signed distance and square its magnitude for the bound -- both are ordinary
instantiations of the same ``pruneTraverse()``, not special cases hardcoded into ``PackedBVH``.
``Tests/TestBVH.cpp`` has a worked example of the former: a bare point struct with no
``signedDistance()`` member at all, searched for its nearest neighbor via ``pruneTraverse()``,
checked against a brute-force scan.

For the exact template signature and callback contracts, see `the doxygen page for
PackedBVH::pruneTraverse <doxygen/html/classEBGeometry_1_1BVH_1_1PackedBVH.html>`__.

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

``MeshSDF::signedDistance()`` implements these rules directly as the four traversal callbacks:
the leaf-evaluator scans a leaf's faces and keeps the signed distance with the smallest magnitude seen so
far; the prune-predicate prunes any node whose bounding-volume distance already exceeds that magnitude;
the child-orderer visits the closest child first; and the node-key-factory supplies each node's distance to
its bounding volume. For the full API, see the Doxygen reference for
`MeshSDF <doxygen/html/classEBGeometry_1_1MeshSDF.html>`__.

CSG Union
^^^^^^^^^

Combinations of implicit functions in EBGeometry into aggregate objects can be done by means of CSG unions.
One such union is known as the *smooth union*, in which the transition between two objects is gradual rather than abrupt.

``BVHSmoothUnionIF::value()`` traverses the tree while tracking the two smallest values seen so
far, ``a`` and ``b`` (``a`` the closest, ``b`` the second-closest): the leaf-evaluator updates both as
leaves are scanned, the prune-predicate prunes any node whose bounding-volume distance exceeds both,
the child-orderer visits the closest child first, and the node-key-factory again supplies each node's
distance to its bounding volume. Once traversal completes, the two values are blended with the
stored smooth-minimum operator. See :ref:`Chap:ImplemCSG` for the CSG combinators themselves, and
the Doxygen reference for
`BVHSmoothUnionIF <doxygen/html/classEBGeometry_1_1BVHSmoothUnionIF.html>`__ /
`BVHUnionIF <doxygen/html/classEBGeometry_1_1BVHUnionIF.html>`__ for the exact API.

.. _Chap:MeshSDFClasses:

Mesh Signed Distance Function Classes
--------------------------------------

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
     - ``PackedBVH`` over ``DCEL::FaceT`` (always ``BVH::SharedPtrStorage``)
     - ``pruneTraverse()`` (SIMD when ``(K, T)`` matches a compiled ISA path)
     - Any polygon mesh; not restricted to triangles
   * - ``TriMeshSDF<T, Meta, K, W, StoragePolicy>``
     - DCEL mesh or triangle soup
     - ``PackedBVH`` over SoA triangle groups (``BVH::ValueStorage`` by default)
     - ``pruneTraverse()`` over SoA-packed leaves
     - Triangle meshes only; highest throughput

``FlatMeshSDF`` is useful for correctness checks and tiny meshes. See `its doxygen page
<doxygen/html/classEBGeometry_1_1FlatMeshSDF.html>`__.

``MeshSDF`` handles arbitrary polygon meshes; its ``signedDistance()`` builds the traversal
criteria shown above (a leaf-eval and a pruning rule, not the full four-callback ``traverse()``
shape) and drives them through ``PackedBVH::pruneTraverse()``, picking up SIMD node pruning
whenever ``(K, T)`` matches a compiled ISA path and falling back to the generic, scalar
``traverse()`` otherwise. See `its doxygen page <doxygen/html/classEBGeometry_1_1MeshSDF.html>`__.

``TriMeshSDF`` is the recommended default for triangle meshes: it packs triangles into
Structure-of-Arrays groups of width ``W`` (via ``TreeBVH::packWith()``, see above) and builds the
same kind of thin ``pruneTraverse()`` wrapper as ``MeshSDF``, over ``TriangleSoAT<T, W>`` leaves
instead of individual faces, so that on a matching ``(K, T)`` combination each BVH leaf evaluates
``W`` triangles with a single SIMD register operation, and even the AABB-vs-running-best
comparisons during descent are done on squared distances (no square root) until the very last
step. See `its doxygen page <doxygen/html/classEBGeometry_1_1TriMeshSDF.html>`__, and `the doxygen
page for TriangleSoAT <doxygen/html/structEBGeometry_1_1TriangleSoAT.html>`__ for the SoA storage
itself.

What is actually vectorised in ``TriMeshSDF``/``PackedBVH`` is covered in
:ref:`Chap:SIMDClasses` -- see that page for the full detail rather than repeating it here.

Primitive storage: why MeshSDF and TriMeshSDF default differently
___________________________________________________________________

Both classes' underlying ``PackedBVH`` accepts the ``StoragePolicy`` axis described above, but
they default -- and, for ``MeshSDF``, are restricted -- differently:

*  ``MeshSDF`` always uses ``BVH::SharedPtrStorage<DCEL::FaceT<T, Meta>>`` and does not expose a
   ``StoragePolicy`` template parameter at all. This is not merely a default: ``DCEL::FaceT``'s
   copy constructor deliberately does *not* copy its cached 2D polygon embedding (used by
   ``signedDistance()``'s point-in-face test) or its inside/outside algorithm choice, since
   ``FaceT``'s half-edge back-reference is only topologically meaningful relative to a specific
   mesh (the same reasoning documented for ``VertexT``/``EdgeT``'s copy constructors). A
   ``BVH::ValueStorage``-style plain copy would therefore leave every packed face with an
   uninitialized embedding, crashing on the first query rather than merely losing sharing --
   so ``MeshSDF`` simply never offers that choice.
*  ``TriMeshSDF`` defaults to ``BVH::ValueStorage<TriangleSoAT<T, W>>`` instead of
   ``BVH::SharedPtrStorage``, and *does* expose ``StoragePolicy`` as an overridable template
   parameter (its 5th, after ``W``). Unlike ``DCEL::FaceT``, each ``TriangleSoAT<T, W>`` group is
   a plain aggregate of coordinate arrays with no cached derived state or back-references, built
   fresh by ``groupTrianglesIntoSoA()`` during packing and shared with nothing else -- so storing
   it inline is both safe and, avoiding one heap allocation and pointer indirection per group,
   the better default.

Neither default is affected by instancing the same mesh multiple times (e.g. placing several
``Translate``/``Rotate``/``Scale``-wrapped copies of one mesh into a ``Union``): those wrappers
hold a ``shared_ptr`` to the whole ``MeshSDF``/``TriMeshSDF`` object (see :ref:`Chap:ImplemCSG`),
so its packed data is shared once per wrapper regardless of how that one object's own
``PackedBVH`` stores its primitives internally.

``Parser::readIntoPackedBVH`` mirrors ``MeshSDF`` (no ``StoragePolicy`` parameter);
``Parser::readIntoTriangleBVH`` mirrors ``TriMeshSDF`` (``StoragePolicy`` as its 5th template
parameter, defaulting to ``BVH::ValueStorage<TriangleSoAT<T, W>>``). See :ref:`Chap:Parsers`.

SIMD-optimal K and W by ISA
______________________________

The helper ``BVH::DefaultBranchingRatio<T>()`` returns the SIMD-optimal branching factor for
the current compilation target.  ``EBGeometry::TriangleSoA::DefaultWidth<T>()`` gives the
matching SoA width. Both are used as template defaults for ``TriMeshSDF`` and
``Parser::readIntoTriangleBVH``.

.. list-table:: Default K and W by ISA and precision
   :widths: 25 25 25 25
   :header-rows: 1

   * - ISA
     - Precision
     - ``DefaultBranchingRatio<T>()``
     - ``TriangleSoA::DefaultWidth<T>()``
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

The K=16/float and K=8/double paths use 512-bit-wide SIMD loads and require the ``ChildAABBSoA``
struct to be 64-byte aligned, which is guaranteed by ``alignas(sizeof(T)*K)`` on the struct. The
K=8/float and K=4/double paths use 256-bit-wide loads instead. All other (K, T) combinations fall
back to a scalar loop that goes through the generic ``traverse()`` described above.

Each ``TriangleSoAT<T, W>`` block is likewise ``alignas``-aligned to its own SIMD register width
(64 bytes for ``<float, 16>``/``<double, 8>``, 32 bytes for ``<float, 8>``, 16 bytes for
``<float, 4>``/``<double, 4>``), and the library inserts ``static_assert`` checks that fire at
compile time if the alignment invariant is violated.

Choosing W and K explicitly
______________________________

``W`` and the BVH branching factor ``K`` are explicit template parameters on ``TriMeshSDF`` and
``Parser::readIntoTriangleBVH`` -- both default to ``BVH::DefaultBranchingRatio<T>()`` and
``TriangleSoA::DefaultWidth<T>()`` respectively, but either can be overridden by supplying them
explicitly (e.g. requesting an 8-wide SoA packing together with a 4-ary BVH, regardless of what
the current compilation target would otherwise default to). See `the doxygen page for
Parser::readIntoTriangleBVH <doxygen/html/namespaceEBGeometry_1_1Parser.html>`__ for the exact
signature.

Rules of thumb:

* Keep ``W`` equal to ``EBGeometry::TriangleSoA::DefaultWidth<T>()`` unless you
  have a specific reason to deviate.  The library is tuned for this default.
* ``a_maxLeafSize`` (the maximum number of raw triangles per BVH leaf, before
  SoA packing) defaults to ``2 * W``: leaves land on up to two full SoA blocks,
  while the SAH/TopDown partitioner is still free to split down to smaller,
  tighter leaves wherever the geometry calls for it. A leaf smaller than ``W``
  simply pads its SoA block's unused lanes.
* ``K = BVH::DefaultBranchingRatio<T>()`` is a good default. With AVX-512F
  available you can try ``K = 16`` (float) — the child-AABB test is evaluated in
  a single SIMD batch, and the wider fan-out reduces tree depth.
