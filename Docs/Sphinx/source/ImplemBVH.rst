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
   the signed-distance case; a callback could equally perform, say, a nearest-neighbor search over
   a point cloud whose primitive carries no ``signedDistance()`` at all.
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
lists whenever a leaf is subdivided. Four ready-made partitioners are provided:
``BVCentroidPartitioner`` (splits on bounding-volume centroids along the longest axis -- the
default), ``PrimitiveCentroidPartitioner`` (the same idea, but splits on primitive centroids
instead), ``BinnedSAHPartitioner`` (a Surface-Area-Heuristic partitioner, used automatically
when building via ``BVH::Build::SAH`` -- see below -- and typically producing the
best-performing trees at a higher construction cost), and ``MidpointPartitioner`` (splits on the
midpoint of the bounding-volume centroids' extent along the longest axis, with a single
``std::partition`` pass -- no sorting and no per-plane cost evaluation, making it the fastest of
the four to build, at the cost of not adapting to the primitive distribution the way the other
three do). ``BinnedSAHPartitioner`` and ``MidpointPartitioner`` both produce ``K`` groups by
recursively splitting into two (``std::floor(K/2)`` and ``std::ceil(K/2)``) halves, exact for
power-of-two ``K``. ``BinnedSAHPartitioner`` takes an optional final template argument
``LongestAxisOnly`` (default ``false``); setting it ``true`` bins candidate planes on only the
longest centroid-bounding-box axis instead of all three, cutting roughly a third of the binning
work (measured ~20% faster SAH builds on point clouds) for a tree-quality cost that is negligible
on near-uniform inputs. The leaf predicate takes a ``TreeBVH``
node and decides whether it should become a leaf (i.e. not be split any further); a default is
provided, but callers are free to supply their own of either kind.

Bottom-up construction
________________________

The bottom-up construction uses a space-filling curve (e.g., a Morton curve) for first building the leaf nodes.
This construction is done such that each leaf node contains approximately the number of primitives, and all leaf nodes exist on the same level.
To use bottom-up construction, one may use the member function template
``bottomUpSortAndPartition<S>()`` (no arguments).
The template argument is the space-filling curve that the user wants to apply, from namespace
``EBGeometry::SFC`` (:file:`Source/EBGeometry_SFC.hpp`, see `the doxygen API
<doxygen/html/namespaceEBGeometry_1_1SFC.html>`__).
Currently, we support Morton codes, Hilbert curves, and nested indices.
For Morton curves, one would e.g. call ``bottomUpSortAndPartition<SFC::Morton>``; the Hilbert curve
(better spatial locality than Morton, since consecutive codes are always spatially adjacent) is
``bottomUpSortAndPartition<SFC::Hilbert>``; while for nested indices (which are not recommended) the
signature is likewise ``bottomUpSortAndPartition<SFC::Nested>``.

Build times for SFC-based bottom-up construction are generally speaking faster than top-down construction, but it tends to produce worse trees such that traversal becomes slower. For the full API,
see the Doxygen reference for
`TreeBVH <doxygen/html/classEBGeometry_1_1BVH_1_1TreeBVH.html>`__.

.. _Chap:DirectSFCBuild:

Direct construction (no TreeBVH)
___________________________________

Both construction methods above build a ``TreeBVH`` first, then require a separate ``pack()``/
``packWith()`` call to obtain a ``PackedBVH``. For workloads with many small, cheaply-copyable
primitives (points, particles) built and rebuilt often, the per-node ``shared_ptr<TreeBVH>``
allocation this implies can dominate build time. ``PackedBVH`` has a constructor that skips
``TreeBVH`` entirely:

.. code-block:: cpp

   BVH::PackedBVH<T, P, K> packed(std::move(primsAndBVs), targetLeafSize);

It takes primitives **by value** (``std::vector<std::pair<P, BV>>``, a sink parameter the caller
can ``std::move`` in) rather than requiring a ``shared_ptr``-wrapped list, regardless of this
``PackedBVH``'s ``StoragePolicy`` (see :ref:`Chap:PackedBVH`'s "Storage policy" section) — combined
with ``BVH::ValueStorage<P>``, this is genuinely pointer-free from input to final storage.

Internally, this constructor:

#. Sorts primitives along a space-filling curve (``SFC::Morton`` by default; pass e.g.
   ``SFC::Hilbert{}`` or ``SFC::Nested{}`` as an optional trailing argument to select another curve —
   a constructor template's own parameters can't be explicitly named the way a regular function
   template's can, so this is a stateless tag value purely to let the curve type be deduced).
#. Cuts leaves via a single linear left-to-right scan at a caller-chosen **target leaf size**,
   rather than deriving a leaf count purely from primitive count and ``K`` the way
   ``bottomUpSortAndPartition()`` does — giving direct control over leaf occupancy.
#. Merges the resulting leaves upward in groups of ``K``, padding the leaf count up to the next
   power of ``K`` (by re-using the last real leaf's node in place of any missing child, rather than
   inventing an empty placeholder) whenever it isn't already one, so every interior node still has
   exactly ``K`` children — no change to ``Node``'s shape or to ``traverse()``/``pruneTraverse()``.

Since this still produces an ordinary ``PackedBVH``, every existing traversal/query facility
(``traverse()``, ``pruneTraverse()``, the SIMD dispatch) works with it identically, unchanged.

``PackedBVH`` also has a second, overloaded direct constructor covering top-down (and SAH)
construction rather than the SFC-based one above:

.. code-block:: cpp

   BVH::PackedBVH<T, P, K> packed(std::move(primsAndBVs));                                    // top-down
   BVH::PackedBVH<T, P, K> packed(std::move(primsAndBVs), BVH::BinnedSAHPartitioner<T, P, AABBT<T>, K>, stopCrit); // SAH

It reuses ``TreeBVH``'s own ``Partitioner``/``LeafPredicate`` machinery unchanged (any of
``BVCentroidPartitioner``, ``BinnedSAHPartitioner``, ``PrimitiveCentroidPartitioner``, or a
caller-supplied one), so it accepts the same arguments ``topDownSortAndPartition()`` does — but
writes nodes directly into the flat node array in depth-first pre-order as the recursion unwinds,
rather than building a persistent, ``shared_ptr``-linked ``TreeBVH`` first. Since top-down
recursion visits the root before its children, this needs no relayout pass (unlike the SFC-build
constructor above, where a bottom-up merge naturally produces the root last). Each split still
shared_ptr-wraps primitives once, up front (to reuse the existing ``Partitioner``/``LeafPredicate``
signatures) and constructs one lightweight, stack-local ``TreeBVH`` per split purely to evaluate
the stop criterion and read off its primitive list — proportionate to what
``topDownSortAndPartition()`` already does at every node, and immediately discarded rather than
kept alive as part of a persistent tree. What this constructor avoids is exactly the *persistent*
``shared_ptr<TreeBVH>`` node allocation kept alive for the tree's lifetime, which the ``Examples/BuildBVH``
benchmark measures as the traditional path's dominant build-time cost.

A third direct constructor builds via **ClusterSAH**, a fast approximation of a full SAH tree:

.. code-block:: cpp

   BVH::PackedBVH<T, P, K> packed(std::move(primsAndBVs), BVH::ClusterSpec{maxClusterSize});

It first groups the primitives into small, spatially-tight *clusters* (buckets of at most
``maxClusterSize`` primitives, formed by a cheap density-adaptive midpoint subdivision that stops
early), then runs binned SAH top-down over those clusters — so SAH partitions roughly
``N / maxClusterSize`` boxes instead of all ``N`` primitives. The result is near-SAH tree quality at
a fraction of the single-threaded SAH build cost, and it stays robust across uniform, surface, and
clustered primitive distributions (a fixed Cartesian grid, by contrast, overcrowds on non-uniform
data). ``BVH::ClusterSpec::maxClusterSize`` trades build time (larger → fewer, cheaper SAH units)
against query quality (larger → coarser leaves); ``Examples/BuildBVH`` benchmarks its build time
against the other strategies.

.. tip::

   Higher-level entry points such as ``Parser::readIntoPackedBVH`` don't require you to
   call ``topDownSortAndPartition``/``bottomUpSortAndPartition`` directly — they take a single
   ``BVH::Build`` enum value (``TopDown``, ``Morton``, ``Nested``, or ``SAH``) and dispatch to the
   corresponding construction method internally. See :ref:`Chap:Parsers`.

.. _Chap:BVHRefit:

Refitting for moving geometries
-------------------------------

Both representations expose a ``refit()`` member that recomputes every node's bounding volume in
place, leaving the tree topology (node hierarchy and each leaf's primitive assignment) untouched --
the cheap way to keep a BVH valid for a geometry whose primitives have *moved* between frames,
without a full rebuild-and-repack. It takes a single functor mapping one primitive to its current
bounding volume and unions volumes bottom-up: each leaf's from its primitives, each interior node's
from its children. ``PackedBVH::refit()`` also rebuilds the per-node SoA AABB cache used by the SIMD
``pruneTraverse()`` (see :ref:`Chap:PruneTraverse`) so queries stay consistent. Because it never
re-partitions, a geometry that deforms enough for primitives to migrate across the tree accumulates
looser bounding volumes over time and should periodically be rebuilt instead; see :ref:`Chap:BVH`
for that trade-off. For the exact signatures, see the Doxygen references for `TreeBVH
<doxygen/html/classEBGeometry_1_1BVH_1_1TreeBVH.html>`__ and `PackedBVH
<doxygen/html/classEBGeometry_1_1BVH_1_1PackedBVH.html>`__.

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
   ``PackedBVH`` whose leaves hold SIMD-width ``TriangleAoSoA<T, Meta, W>`` groups instead — see
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
Every policy's ``StorageType`` is trivially copyable. That is a hard requirement, not a
coincidence: it is what lets ``PackedBVH`` keep a single primitive-array backend rather than a
separate, permanently host-only one, and it is what will let a packed BVH be mirrored into a device
address space by a plain byte copy. A ``SharedPtrStorage`` policy existed until the GPU port and was
removed for exactly that reason — see :ref:`Sec:PolymorphicPrimitives` below for the one use that
depended on it. Two policies are provided:

*  ``BVH::ValueStorage<P>`` (**the default**, for ``pack()``, ``packWith()`` and the direct
   constructors) stores each primitive inline, by value: ``StorageType`` is ``P`` itself, with no
   indirection at all. The ``PackedBVH`` owns an independent copy of every primitive, and because
   packing reorders primitives into leaf order, a leaf scan walks contiguous memory.
*  ``BVH::IndexStorage<P>`` stores a ``uint32_t`` index into a primitive array the *caller* owns.
   ``get()`` resolves an index against a base pointer supplied by the caller, so several BVHs can
   index one primitive set and an edit to that set is seen by all of them at once. Four bytes per
   primitive and no duplication, traded against a scattered load per primitive instead of a
   contiguous one. It does not manage lifetime: the array the indices resolve against must outlive
   every BVH indexing into it, and nothing checks that.

Prefer ``ValueStorage`` — it is the default for good reason — unless the duplication is the binding
constraint on a large primitive set, or the sharing itself is the point.

``IndexStorage`` cannot be built from a ``TreeBVH``: a tree's leaves hold ``shared_ptr<const P>``
and carry no index into any owning array, so there is nothing for ``appendTreeLeaf()`` to record and
calling ``pack()``/``packWith()`` with it is a compile error. Build one through ``PackedBVH``'s
direct constructors instead, passing ``(index, bounding volume)`` pairs; those partition on the
bounding volumes alone and never need the primitive itself.

Swapping the policy only changes the element type of ``getPrimitives()`` and the leaf-primitive
vector handed to ``LeafEvaluator``/``PackedLeafEvaluator`` callbacks, never the tree structure,
traversal order, or query results.

Pool-backed storage
____________________

``PackedBVH``'s three arrays -- the flat node array, the primitive array, and the SoA child-AABB
cache -- are ``PODVector``\ s reserved from a caller-supplied ``Pool`` (see :ref:`Chap:MemoryModel`),
not ``std::vector``\ s. Every construction entry point therefore takes a ``Pool&``: ``pack()``,
``packWith()``, all three direct constructors, and ``MeshSDF``/``TriMeshSDF``/``PointCloudBVH``,
which pass along the pool they already take. The pool must outlive the BVH.

Construction itself still assembles the arrays in ordinary ``std::vector``\ s and copies them into
the pool in one shot at the end. That is deliberate: a ``PODVector`` never reallocates, so it has no
way to be filled incrementally without its final size fixed up front, and the node count is not
known until a build finishes. Growth belongs in the host-only build step; nothing that crosses to a
device is ever built incrementally.

What this buys is that a ``PackedBVH`` is trivially copyable -- three 16-byte descriptors, a control
block pointer, and a base pointer -- so a whole hierarchy crosses to a device as a byte copy with no
pointer patching. ``rebasedView(Pool&)`` is the single sanctioned crossing, exactly as for
``DCEL::MeshT``: mirror the pool, rebase on the host, then pass the returned value to a kernel.

``getPrimitives()`` returns a ``PODSpan`` rather than a container reference. A span is a *resolved*
address into pool memory, so it must not outlive the next ``Pool::reserve`` on that pool -- re-obtain
it rather than caching it across a build step.

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
*  ``PackedBVH`` allows both copying and moving, but a copy is **not** a deep copy. Its members are
   three ``PODVector`` descriptors plus two address fields, so copying one copies offsets: the copy
   resolves against the same pool memory as the original, and writing through either is visible
   through the other. That shallowness is the point -- it is what makes the type trivially copyable,
   and therefore what lets ``rebasedView()`` produce a value a kernel can consume directly. Use
   ``deepCopy(Pool&)`` for storage of its own. (Under ``BVH::IndexStorage`` even a deep copy still
   shares the caller-owned primitive array the indices refer to; only the BVH's own arrays are
   duplicated.)

Both classes' destructors are non-virtual: neither is intended to be subclassed or used
polymorphically.

.. _Sec:PolymorphicPrimitives:

Polymorphic primitives are not currently supported
___________________________________________________

Both policies require ``P`` to be a concrete type. ``ValueStorage<P>`` stores ``P`` by value, so an
abstract base either fails to compile or slices the object to its static type; ``IndexStorage<P>``
indexes a flat array of ``P``, which is meaningless when the elements are of different derived types
and therefore different sizes. Neither can hold a polymorphic hierarchy, and the ``SharedPtrStorage``
policy that could has been removed because a ``shared_ptr`` is not trivially copyable and so can
never cross into a device address space.

One part of the library depended on exactly that: the BVH-accelerated CSG unions
(:ref:`Chap:ImplemCSG`), whose primitive is ``ImplicitFunction<T>`` and whose leaf evaluator calls a
virtual ``value()`` through a base pointer. ``BVHUnionIF``, ``BVHSmoothUnionIF`` and their
``BVHUnion``/``BVHSmoothUnion`` factories are therefore **compiled out** at present, behind
``EBGEOMETRY_ENABLE_BVH_CSG_UNION`` in :file:`Source/EBGeometry_CSG.hpp`, along with the examples
and tests that exercise them.

The resolution is not another storage policy but the index-based redesign of the implicit-function
and CSG layer as a whole, which replaces virtual dispatch with a linear-SSA tape. Nesting a BVH
inside a BVH — the common realisation of which was a ``BVHUnion`` over several mesh SDFs — returns
with it.

Note that a ``PackedBVH`` whose primitive is a concrete BVH-backed type is still a by-value copy of
everything reachable below that primitive, which is expensive for a large inner BVH and compounds
with nesting depth. ``IndexStorage`` is the tool for that case: it stores four bytes per inner
object and leaves the objects themselves in one caller-owned array.

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
pruning bound.

There is exactly **one** such loop, and it runs whether or not the ``(K, T)`` pair in use has a
compiled SIMD path. Only the per-child squared-distance computation differs: a vector batch when
one of the ISA paths matches, and an ordinary scalar loop over the ``K`` children otherwise (which
is also what device code runs). Both compute the same quantity in the same association order, so
they agree bit-for-bit -- a query answered on a build with no SIMD returns exactly what the same
query returns on an AVX-512 build, and the unit tests pin this by sweeping ``K`` across values
that do and do not have a vector path and requiring exact equality. Stack handling, pruning,
child ordering and leaf dispatch are shared code in every configuration. Because the bound is re-read from the current ``State`` at every node visited --
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
The former needs nothing more than a bare point struct with no ``signedDistance()`` member at all,
searched for its nearest neighbor via ``pruneTraverse()`` against a running squared distance.

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

``BVHSmoothUnionIF::value()`` drives the SIMD-accelerated ``pruneTraverse()`` (see
:ref:`Chap:PruneTraverse`) with a ``State`` holding the two smallest values seen so far, ``a`` and
``b`` (``a`` the closest, ``b`` the second-closest): the leaf-evaluator updates both as leaves are
scanned, and the pruning rule returns ``max(0, b)`` squared -- pruning against the *second*-smallest
value rather than the nearest, so a primitive that is not the single closest but still contributes to
the blend is never pruned away. Once traversal completes, the two values are blended with the stored
smooth-minimum operator. ``BVHUnionIF::value()`` is the same pattern with a single running minimum
and a ``max(0, minDist)``-squared pruning bound. See :ref:`Chap:ImplemCSG` for the CSG combinators
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
     - ``PackedBVH`` over ``DCEL::FaceT`` (always ``BVH::ValueStorage``)
     - ``pruneTraverse()`` (SIMD when ``(K, T)`` matches a compiled ISA path)
     - Any polygon mesh; not restricted to triangles
   * - ``TriMeshSDF<T, Meta, K, W, StoragePolicy>``
     - DCEL mesh or triangle soup
     - ``PackedBVH`` over ``TriangleAoSoA`` groups (``BVH::ValueStorage`` by default)
     - ``pruneTraverse()`` over SoA-packed leaves
     - Triangle meshes only; highest throughput; metadata via ``getClosestTriangle()``

``FlatMeshSDF`` is useful for correctness checks and tiny meshes. See `its doxygen page
<doxygen/html/classEBGeometry_1_1FlatMeshSDF.html>`__.

``MeshSDF`` handles arbitrary polygon meshes; its ``signedDistance()`` builds the traversal
criteria shown above (a leaf-eval and a pruning rule, not the full four-callback ``traverse()``
shape) and drives them through ``PackedBVH::pruneTraverse()``, picking up SIMD node pruning
whenever ``(K, T)`` matches a compiled ISA path and falling back to the generic, scalar
``traverse()`` otherwise. See `its doxygen page <doxygen/html/classEBGeometry_1_1MeshSDF.html>`__.

``TriMeshSDF`` is the recommended default for triangle meshes: it packs triangles into
Structure-of-Arrays groups of width ``W`` (via ``TreeBVH::packWith()``, see above) and builds the
same kind of thin ``pruneTraverse()`` wrapper as ``MeshSDF``, over ``TriangleAoSoA<T, Meta, W>``
leaves instead of individual faces, so that on a matching ``(K, T)`` combination each BVH leaf
evaluates ``W`` triangles with a single SIMD register operation, and even the AABB-vs-running-best
comparisons during descent are done on squared distances (no square root) until the very last
step. See `its doxygen page <doxygen/html/classEBGeometry_1_1TriMeshSDF.html>`__, and `the doxygen
page for TriangleSoAT <doxygen/html/structEBGeometry_1_1TriangleSoAT.html>`__ for the SoA storage
itself.

Just as ``MeshSDF::getClosestFaces()`` recovers the nearest face (and its ``Meta``) for a DCEL mesh,
``TriMeshSDF::getClosestTriangle()`` recovers the nearest triangle's signed distance *and* its
metadata through the SIMD SoA path -- the supported route when you need both maximum SIMD throughput
and per-triangle metadata retrieval. Each leaf group is a ``TriangleAoSoA<T, Meta, W>``: a
geometry-only ``TriangleSoAT<T, W>`` plus a physically-separate per-lane ``std::array<Meta, W>`` (the
same metadata-carrying wrapper relationship ``PointAoSoA`` has with ``PointSoAT``). The hot
``signedDistance()`` path never reads the metadata array; only ``getClosestTriangle()`` does, taking a
scalar per-lane step to recover the winning lane. See `the doxygen page for TriangleAoSoA
<doxygen/html/structEBGeometry_1_1TriangleAoSoA.html>`__.

What is actually vectorised in ``TriMeshSDF``/``PackedBVH`` is covered in
:ref:`Chap:SIMDClasses` -- see that page for the full detail rather than repeating it here.

Primitive storage: Facets or triangles
______________________________________

Both classes store their primitives inline, by value, though only one of them lets you say so:

*  ``MeshSDF`` always uses ``BVH::ValueStorage<DCEL::FaceT<T, Meta>>`` and does not expose a
   ``StoragePolicy`` template parameter at all. A ``DCEL::FaceT`` is a plain, trivially-copyable
   value -- every member is a scalar, a ``Vec3``, or a ``uint32_t`` index -- so copying one into the
   packed array is cheap and free of aliasing. What a copied face is *not* is self-contained: it
   stores its half-edge as an index into its owning mesh's edge array rather than a self-resolving
   reference, so it is only meaningful together with that mesh. ``MeshSDF`` therefore retains the
   source mesh and passes it to every face query that has to resolve topology (point-in-face tests,
   signed distance). Sharing the faces with the mesh instead would not avoid that, which is why
   there is no policy to choose.
*  ``TriMeshSDF`` also defaults to ``BVH::ValueStorage<TriangleAoSoA<T, Meta, W>>``, and *does*
   expose ``StoragePolicy`` as an overridable template parameter (its 5th, after ``W``). Unlike
   ``DCEL::FaceT``, each ``TriangleAoSoA<T, Meta, W>`` group is fully self-contained -- a plain
   aggregate of coordinate arrays plus a per-lane metadata array, with no index into anything else,
   built fresh by ``groupTrianglesIntoSoA()`` during packing.

Neither default is affected by instancing the same mesh multiple times (e.g. placing several
``Translate``/``Rotate``/``Scale``-wrapped copies of one mesh into a ``Union``): those wrappers
hold a ``shared_ptr`` to the whole ``MeshSDF``/``TriMeshSDF`` object (see :ref:`Chap:ImplemCSG`),
so its packed data is shared once per wrapper regardless of how that one object's own
``PackedBVH`` stores its primitives internally.

``Parser::readIntoPackedBVH`` mirrors ``MeshSDF`` (no ``StoragePolicy`` parameter);
``Parser::readIntoTriangleBVH`` mirrors ``TriMeshSDF`` (``StoragePolicy`` as its 5th template
parameter, defaulting to ``BVH::ValueStorage<TriangleAoSoA<T, Meta, W>>``). See :ref:`Chap:Parsers`.

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
