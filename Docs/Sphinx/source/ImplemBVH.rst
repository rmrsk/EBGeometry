.. _Chap:ImplemBVH:

BVH
===

See :ref:`Chap:BVH` for the conceptual picture of bounding volume hierarchies (node types,
partitioning, tree pruning during traversal) before reading the concrete API below.

The BVH functionality is encapsulated in the namespace ``EBGeometry::BVH``, in three headers:

* :file:`Source/EBGeometry_PackedBVH.hpp` holds ``PackedBVH``, its node layout and traversals, and
  the types shared with the builders. It is all that code querying a BVH needs, device code in
  particular.
* :file:`Source/EBGeometry_TreeBVH.hpp` holds ``TreeBVH``, the partitioners, the default leaf
  predicate, and ``pack()``/``packWith()``. It is host-only.
* :file:`Source/EBGeometry_BVHBuild.hpp` holds the ``PackedBVH`` constructors that build from a
  ``TreeBVH`` or partition a primitive list directly (top-down, along a space-filling curve, or
  ClusterSAH). It is host-only.

The last two include each other and :file:`Source/EBGeometry_PackedBVH.hpp`, so either one gives
the whole builder API. :file:`Source/EBGeometry_BVH.hpp` includes all three, as does
``EBGeometry.hpp``.
For the full API, see `the doxygen API <doxygen/html/namespaceEBGeometry_1_1BVH.html>`__.
There are two representations of a BVH:

*  ``BVH::TreeBVH<T, P, BV, K>``, a pointer-based tree used while *building* and
   *partitioning* the hierarchy. See `the doxygen page for TreeBVH
   <doxygen/html/classEBGeometry_1_1BVH_1_1TreeBVH.html>`__.
*  ``BVH::PackedBVH<T, P, K>``, where the nodes are stored in depth-first order in
   a flat array and contain index offsets to children and primitives rather than pointers. This is
   the representation used for fast queries, see :ref:`Chap:PackedBVH`. See `the doxygen page
   for PackedBVH <doxygen/html/classEBGeometry_1_1BVH_1_1PackedBVH.html>`__.

The template parameters shared by both are:

*  ``T`` Floating-point precision.
*  ``P`` Primitive type. Neither representation imposes an interface requirement of its own:
   ``TreeBVH`` construction needs nothing from it except that ``PrimitiveCentroidPartitioner``
   calls ``getCentroid()`` (every other partitioner and the bottom-up build work from the bounding
   volumes alone), and
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
Other types of bounding volumes can in principle be added, with the only requirement being that they conform to the same interface as the ``AABBT`` and ``SphereT`` volumes. Note that
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
when building via ``BVH::Construction::SAH`` -- see below -- and typically producing the
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
``bottomUpSortAndPartition<S>(targetLeafSize = 1)``. It takes the fewest leaves, a power of ``K``,
that hold at most ``targetLeafSize`` primitives each, but never more leaves than primitives. The
default target of one gives the deepest such tree, with at most ``K`` primitives per leaf.
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

   BVH::PackedBVH<T, P, K> packed(pool, std::move(primsAndBVs), targetLeafSize);

Every ``PackedBVH`` constructor takes the ``Pool`` its flat arrays are reserved from as its first
argument; that pool must outlive the BVH. It takes primitives **by value**
(``std::vector<std::pair<P, BV>>``, a sink parameter the caller can ``std::move`` in) rather than
requiring a ``shared_ptr``-wrapped list, so the whole path from input to final storage is
pointer-free.

Internally, this constructor:

#. Sorts primitives along a space-filling curve (``SFC::Morton`` by default; pass e.g.
   ``SFC::Hilbert{}`` or ``SFC::Nested{}`` as an optional trailing argument to select another curve —
   a constructor template's own parameters can't be explicitly named the way a regular function
   template's can, so this is a stateless tag value purely to let the curve type be deduced).
#. Splits the sorted primitives into consecutive leaves of a caller-chosen **target leaf size**,
   as ``bottomUpSortAndPartition()`` does, but from a finer set of leaf counts than its powers of
   ``K``, so the leaves can sit closer to the target. Every interior
   node has exactly ``K`` children, and a tree like that has a leaf count ``L`` with
   ``L = 1 (mod K - 1)``, so the constructor picks the smallest such ``L`` that keeps leaves within
   the target (or, when that would leave a leaf empty, the largest one below it) and splits the
   primitives evenly across the leaves.
#. Merges the leaves upward in groups of ``K``. When a level's node count is not a multiple of
   ``K``, the remainder is carried up to the next level unmerged. Every node has exactly one parent,
   so a traversal reaches each primitive exactly once.

Since this still produces an ordinary ``PackedBVH``, every existing traversal/query facility
(``traverse()``, ``pruneTraverse()``, the SIMD dispatch) works with it identically, unchanged.

``PackedBVH`` also has a second, overloaded direct constructor covering top-down (and SAH)
construction rather than the SFC-based one above:

.. code-block:: cpp

   BVH::PackedBVH<T, P, K> packed(pool, std::move(primsAndBVs));                                    // top-down
   BVH::PackedBVH<T, P, K> packed(pool, std::move(primsAndBVs), BVH::BinnedSAHPartitioner<T, P, AABBT<T>, K>, stopCrit); // SAH

It reuses ``TreeBVH``'s own ``Partitioner``/``LeafPredicate`` machinery unchanged (any of
``BVCentroidPartitioner``, ``BinnedSAHPartitioner``, ``PrimitiveCentroidPartitioner``, or a
caller-supplied one), so it accepts the same arguments ``topDownSortAndPartition()`` does — but
writes nodes directly into the flat node array in depth-first pre-order as the recursion unwinds,
rather than building a persistent, ``shared_ptr``-linked ``TreeBVH`` first. As for
``topDownSortAndPartition()``, every one of the ``K`` partitions a partitioner returns must hold at
least one primitive: an empty one would become a leaf with no primitives, which the packed layout
cannot represent, and it aborts the build, in every build, with a message naming the partition. Since top-down
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

   BVH::PackedBVH<T, P, K> packed(pool, std::move(primsAndBVs), BVH::ClusterSpec{maxClusterSize});

It first groups the primitives into small, spatially-tight *clusters* (buckets of at most
``maxClusterSize`` primitives, formed by a cheap density-adaptive midpoint subdivision that stops
early), then runs binned SAH top-down over those clusters — so SAH partitions roughly
``N / maxClusterSize`` boxes instead of all ``N`` primitives. The result is near-SAH tree quality at
a fraction of the single-threaded SAH build cost, and it stays robust across uniform, surface, and
clustered primitive distributions (a fixed Cartesian grid, by contrast, overcrowds on non-uniform
data). ``BVH::ClusterSpec::maxClusterSize`` trades build time (larger → fewer, cheaper SAH units)
against query quality (larger → coarser leaves); ``Examples/BuildBVH`` benchmarks its build time
against the other strategies.

.. _Sec:BuildPresets:

Preset construction methods
---------------------------

The library's own BVH users -- ``MeshSDF``, ``TriMeshSDF``, ``BVHUnionIF``, ``BVHSmoothUnionIF``
and the parser functions that build them (see :ref:`Chap:Parsers`) -- don't ask for a partitioner
and a leaf predicate. They take one ``BVH::Construction`` value, which names the algorithm that groups the
primitives:

.. list-table::
   :header-rows: 1
   :widths: 20 15 65

   * - ``BVH::Construction``
     - Direction
     - Method
   * - ``CentroidSplit``
     - top-down
     - Split at the median bounding-volume centroid along the longest axis
       (``BVCentroidPartitioner``).
   * - ``MidpointSplit``
     - top-down
     - Split at the spatial midpoint of the centroids' longest axis, with no sorting
       (``MidpointPartitioner``). The fastest top-down build; does not adapt to clustered input.
   * - ``SAH``
     - top-down
     - Binned surface area heuristic (``BinnedSAHPartitioner``). The recommended default.
   * - ``ClusterSAH``
     - top-down
     - Binned SAH over small spatial clusters (see above). Builds several times faster than
       ``SAH``; a leaf holds up to ``K-1`` clusters.
   * - ``Morton``
     - bottom-up
     - Leaves of consecutive primitives along a Morton curve, merged ``K`` at a time.
   * - ``Nested``
     - bottom-up
     - The same, along a Nested curve.
   * - ``Hilbert``
     - bottom-up
     - The same, along a Hilbert curve.

Every one of these users accepts every value, and aborts, in every build, on a value outside the
enum. ``MeshSDF`` and ``TriMeshSDF`` build the tree methods through a ``TreeBVH`` and ``ClusterSAH``
through the direct ``ClusterSpec`` constructor; ``TriMeshSDF`` then regroups each leaf into SIMD
triangle groups. The BVH unions use the direct constructors throughout. A custom partitioner or leaf
predicate is not a preset: build a ``TreeBVH`` with it and ``pack()`` it, or use the direct top-down
constructor.

.. _Sec:LeafSizes:

Leaf sizes
__________

Leaf size is a tuning parameter, and its natural form differs per method, so each method has its
own setting. They are collected in one ``BVH::ConstructionOptions`` (`doxygen
<doxygen/html/structEBGeometry_1_1BVH_1_1ConstructionOptions.html>`__), and a method reads only its
own field:

.. list-table::
   :header-rows: 1
   :widths: 30 20 50

   * - Methods
     - Field
     - Meaning
   * - ``SAH``, ``CentroidSplit``, ``MidpointSplit``
     - ``maxLeafSize``
     - A node with at most this many primitives becomes a leaf. A split makes ``K`` non-empty
       children, so a node with fewer than ``K`` is a leaf too.
   * - ``Morton``, ``Nested``, ``Hilbert``
     - ``targetLeafSize``
     - The fewest leaves holding at most this many primitives each, among the leaf counts the
       builder can make (powers of ``K`` through a ``TreeBVH``; :math:`L \equiv 1 \pmod{K-1}` in
       the direct constructor). There are never more leaves than primitives, so a target below ``K``
       may be exceeded.
   * - ``ClusterSAH``
     - ``cluster``
     - The ``ClusterSpec``; a leaf holds 1 to ``K-1`` clusters.

``MeshSDF``, ``TriMeshSDF``, ``BVHUnionIF`` and ``BVHSmoothUnionIF`` each have a constructor that
takes the options after the ``BVH::Construction`` value, counted in that class's primitives (faces,
triangles or union members). Each also has a static ``defaultConstructionOptions()`` returning what
its constructor without options uses, so a change to one method's setting starts from it:

.. code-block:: cpp

   using SDF = TriMeshSDF<T, K, W>;

   auto options = SDF::defaultConstructionOptions(4); // 4 groups of W triangles per leaf

   options.targetLeafSize = 32;

   const SDF sdf(mesh, pool, BVH::Construction::Hilbert, options);

A field left at zero is rejected, in every build, when the chosen method reads it. The defaults are:

.. list-table::
   :header-rows: 1
   :widths: 25 25 25 25

   * - Class
     - ``maxLeafSize``
     - ``targetLeafSize``
     - ``cluster``
   * - ``MeshSDF``
     - ``K-1``
     - 1 (the deepest tree)
     - ``ClusterSpec{}``
   * - ``TriMeshSDF`` (``maxLeafGroups`` :math:`g`)
     - :math:`gW`
     - :math:`gW`
     - :math:`\max(1, \lfloor gW/(K-1) \rfloor)`, so a leaf of up to ``K-1`` clusters stays
       within :math:`\max(gW, K-1)`
   * - ``BVHUnionIF``, ``BVHSmoothUnionIF``
     - ``K-1``
     - ``K``
     - ``ClusterSpec{}``

The file readers (:ref:`Chap:Parsers`) build with these defaults; to set a leaf size, build the
class with its own constructor.

.. _Chap:BVHRefit:

Refitting for moving geometries
-------------------------------

Both representations expose a ``refit()`` member that recomputes every node's bounding volume in
place, leaving the tree topology (node hierarchy and each leaf's primitive assignment) untouched --
the cheap way to keep a BVH valid for a geometry whose primitives have *moved* between frames,
without a full rebuild-and-repack. It takes a single functor mapping one primitive to its current
bounding volume and unions volumes bottom-up: each leaf's from its primitives, each interior node's
from its children. ``PackedBVH::refit()`` merges the boxes pairwise, allocating nothing, and also
rebuilds the SoA child-box rows used by the SIMD ``pruneTraverse()`` (see :ref:`Chap:PruneTraverse`)
so queries stay consistent. Because it never
re-partitions, a geometry that deforms enough for primitives to migrate across the tree accumulates
looser bounding volumes over time and should periodically be rebuilt instead; see :ref:`Chap:BVH`
for that trade-off. For the exact signatures, see the Doxygen references for `TreeBVH
<doxygen/html/classEBGeometry_1_1BVH_1_1TreeBVH.html>`__ and `PackedBVH
<doxygen/html/classEBGeometry_1_1BVH_1_1PackedBVH.html>`__.

Refitting recomputes bounding volumes only, never the primitives themselves. A primitive that
caches data derived from the geometry has to be brought up to date some other way. ``MeshSDF``
(:ref:`Chap:MeshSDFClasses`) stores face ids, not faces, so after moving the mesh's vertices,
``mesh.reconcile()`` recomputes the faces' cached normals, centroids and projection axes in the
mesh itself, and refitting its BVH (through ``getRoot()``) then updates the boxes. ``TriMeshSDF``'s
triangles are copies taken at build time, so after moving vertices it has to be rebuilt.

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
``PackedBVH<T, P, K>`` is templated on the same ``T``, ``P``, ``K`` as ``TreeBVH`` (its bounding
volume is always ``AABBT<T>``), and internally stores two things: a flat, depth-first array of
nodes, and a global primitive array holding every primitive in leaf order.

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
   This is how ``TriMeshSDF`` turns a tree of individual ``Triangle<T>`` primitives into a
   ``PackedBVH`` whose leaves hold SIMD-width ``TriangleAoSoA<T, W>`` groups instead — see
   :ref:`Chap:MeshSDFClasses`.

See `the doxygen page for TreeBVH <doxygen/html/classEBGeometry_1_1BVH_1_1TreeBVH.html>`__ for
the exact signatures of ``pack()`` and ``packWith()``.

Primitive storage
_________________

A ``PackedBVH`` stores each primitive **inline, by value**, in one flat array held in leaf order.
There is no indirection: no per-primitive heap allocation, no pointer chase on a leaf visit, and
because packing reorders primitives into leaf order, a leaf scan walks contiguous memory.

``P`` must therefore be trivially copyable. That is a hard requirement, not a coincidence: it is
what lets the whole BVH — nodes, primitives and SoA cache alike — be mirrored into a device address
space by a plain byte copy. Until the GPU port the primitive array could instead hold
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
``uint32_t`` is an ordinary primitive type, so every build path accepts it — the SFC-build,
partitioner/SAH and ``ClusterSpec`` constructors, and ``TreeBVH::pack()`` over a
``TreeBVH<T, uint32_t, AABBT<T>, K>``.

The trade is locality: the query pays a scattered load per primitive instead of a contiguous one,
and the array the indices resolve against must outlive every BVH indexing into it — an index is not
an owner, and nothing checks that. Prefer storing the primitives themselves unless one of the two
reasons above actually applies.

An earlier ``BVH::IndexStorage`` policy wrapped this same pattern behind a template parameter on
``PackedBVH``. It was removed: ``PackedBVH<T, uint32_t, K>`` stores exactly the same four bytes per
primitive in exactly the same layout, needs no policy machinery to do it, and — unlike the policy,
which could only be built through the two constructors that never touch a primitive — works through
every build path including ``TreeBVH::pack()``.

Pool-backed storage
____________________

``PackedBVH``'s three arrays -- the flat node array, the primitive array, and the SoA child-AABB
cache -- are ``PODVector``\ s reserved from a caller-supplied ``Pool`` (see :ref:`Chap:MemoryModel`),
not ``std::vector``\ s. Every construction entry point therefore takes a ``Pool&``: ``pack()``,
``packWith()``, every direct constructor (the adopting one included), and ``MeshSDF``/``TriMeshSDF``/``PointCloudBVH``,
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

Holding a PackedBVH inside another class
_________________________________________

``PackedBVH`` is ``final``. A class that needs a BVH over its own payload holds one by value
instead, and keeps its own arrays in the same pool so that one ``rebasedView()`` of the held BVH
rebases everything; ``PointCloudBVH`` (:ref:`Chap:ImplemPointCloud`) is the worked case. Deriving
was ruled out because ``rebasedView()`` and ``deepCopy()`` return a ``PackedBVH`` by value, which on
a derived type silently slices away the payload the leaves refer to.

Three public members exist for such a class. The adopting constructor
``PackedBVH(Pool&, const std::vector<Node>&, const std::vector<P>&)`` takes a node and primitive
array built by some other means -- ``PointCloudBVH`` runs its own index-based build. Every
constructor, this one included, checks always on rather than only under assertions that the node
array is a well-formed depth-first pre-order flattening: every child strictly after its parent and
inside the array, every node but the root with exactly one parent, every leaf's primitives inside
the primitive array. A malformed array aborts with a diagnostic instead of surfacing later as an
out-of-bounds read. A leaf with no primitives is one such defect: it reads as an interior node whose
children are all node 0, which the first check rejects. ``getNodes()`` returns the flat node array
as a read-only ``PODSpan``, for a class that walks the tree with a traversal of its own. And
``traversalStackDepth()`` gives the fixed traversal-stack size for the current compilation pass
(host or device; see :ref:`Sec:TraversalStack`), which the build and ``rebasedView()`` validate the
tree's depth against -- a custom traversal that pushes at most ``K`` children per visited node, as
``pruneTraverse()`` does, can size its own stack with it and inherit the same guarantee.

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
   ``deepCopy(Pool&)`` for storage of its own. (When the primitives are indices into a
   caller-owned array — see :ref:`Sec:IndexedPrimitives` — even a deep copy still shares that array;
   only the BVH's own arrays are duplicated.)

Both classes' destructors are non-virtual: neither is intended to be subclassed or used
polymorphically, and ``PackedBVH`` is declared ``final`` to enforce it (see above).

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

Nesting itself is not the expensive part, incidentally. A ``PackedBVH`` is three ``PODVector``
descriptors plus two address fields — 64 bytes, whatever the size of the BVH it describes — so a
``PackedBVH`` whose primitive is another ``PackedBVH`` copies 64 bytes per inner BVH, not the inner
BVH's contents, and the two share the pool memory those descriptors resolve against. It is only a
primitive that owns its data *inline* whose size the outer array pays per placement.

Tree traversal
---------------

Both ``TreeBVH`` (full BVH) and ``PackedBVH`` (flattened BVH) include routines for traversing the
BVH with user-specified criteria. For both BVH representations, tree traversal is done through a
single ``traverse()`` member function taking four caller-supplied callbacks, and uses a
stack-based traversal pattern driven by those callbacks. The full type signatures for the four
callback roles below are documented on `the BVH namespace's doxygen page
<doxygen/html/namespaceEBGeometry_1_1BVH.html>`__ (look for ``PrunePredicate``, ``ChildOrderer``/
``PackedChildOrderer``, ``LeafEvaluator``/``PackedLeafEvaluator``, and ``NodeKeyFactory``).

``PackedBVH::traverse()`` is callable from device code as well as from the host. Its callbacks are
template parameters: host code may pass the ``std::function`` aliases above, while device code
passes lambdas or functors (``std::function`` is host-only). The key type is deduced from the
node-key factory's return type, or can be given explicitly (``bvh.template traverse<double>(...)``).
``TreeBVH::traverse()`` is host-only.

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
size relative to ``TreeBVH``, and each child arrives as a ``BVH::NodeAndKey`` whose ``first`` is
the node index and ``second`` its key. In device code the orderer must not call ``std::sort``; an
insertion sort over the ``K`` children does the job. Note that a correct visitation pattern can yield large performance
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

``PackedBVH::traverse()`` implements this with a non-recursive, fixed-size stack rather than
recursion. Each stack entry holds a node index together with that node's already-computed
node key, and the stack is sized from the tree depth exactly as ``pruneTraverse()``'s is (see
:ref:`Sec:TraversalStack`), so a key of ``n`` bytes costs roughly ``n + 4`` bytes per entry. An empty
``PackedBVH`` visits nothing. The root is pushed first; then, until the stack is empty, the traversal pops an entry,
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

.. _Sec:TraversalStack:

The traversal stack
___________________

The stack is a fixed-size array, so the traversal allocates nothing and runs unchanged on a device.
Each entry is 8 bytes in either precision: a ``uint32_t`` node index and the node's squared distance
from the query point, stored as a ``float`` rounded *down*. The pop-time test compares that stored
bound with the current pruning bound, and since it never exceeds the true distance, it can only keep
an entry an exact comparison would have dropped, never drop one it would have kept. The push-time
test and the child ordering use the exact distances.

The traversal pops one entry and pushes up to ``K`` per interior node it expands, so a tree ``D``
levels deep needs at most :math:`1 + (K - 1)(D - 1)` entries. The stack is sized for a depth, the
same for every ``K``:

.. list-table::
   :header-rows: 1
   :widths: 30 20 25 25

   * - Pass
     - Depth (levels)
     - Entries at K = 4
     - Entries at K = 16
   * - Host (``BVH::HostTraversalDepth``)
     - 256
     - 766 (6 KB)
     - 3826 (30 KB)
   * - Device (``BVH::DeviceTraversalDepth``)
     - 32
     - 94 (752 B)
     - 466 (3.7 KB)

Every ``PackedBVH`` is checked against the host depth when it is built, and every device view
against the device depth when ``rebasedView()`` makes it, in every build: a tree too deep aborts with
a message giving its depth, rather than overflowing the stack, which in Release would be a silent
out-of-bounds write. (The stack used to be a fixed 256 entries on the host and 64 on a device, of 16
bytes at ``double``, which held 22 levels on a device at ``K = 4`` and only 5 at ``K = 16``.)

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

``MeshSDF::signedDistance()`` implements these rules through ``pruneTraverse()`` (see
:ref:`Chap:PruneTraverse`): its leaf-eval scans a leaf's faces and keeps the signed distance with the
smallest magnitude seen so far, and its pruning rule prunes any node whose bounding-volume distance
already exceeds that magnitude, while ``pruneTraverse()`` itself visits the closest child first. For
the full API, see the Doxygen reference for
`MeshSDF <doxygen/html/classEBGeometry_1_1MeshSDF.html>`__.

CSG Union
^^^^^^^^^

Combinations of implicit functions in EBGeometry into aggregate objects can be done by means of CSG unions.
One such union is known as the *smooth union*, in which the transition between two objects is gradual rather than abrupt.

``BVHSmoothUnionIF::signedDistance()`` drives the SIMD-accelerated ``pruneTraverse()`` (see
:ref:`Chap:PruneTraverse`) with a ``State`` holding the two smallest values seen so far, ``a`` and
``b`` (``a`` the closest, ``b`` the second-closest): the leaf-evaluator updates both as leaves are
scanned, and the pruning rule returns ``max(0, b)`` squared -- pruning against the *second*-smallest
value rather than the nearest, so a primitive that is not the single closest but still contributes to
the blend is never pruned away. Once traversal completes, the two values are blended with the stored
smooth-minimum operator. ``BVHUnionIF::signedDistance()`` is the same pattern with a single running minimum
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
   * - ``FlatMeshSDF<T>``
     - DCEL mesh
     - None
     - O(N) scan
     - Debug / tiny meshes only; no build cost
   * - ``MeshSDF<T, K>``
     - DCEL mesh
     - ``PackedBVH`` over face ids
     - ``pruneTraverse()`` (SIMD when ``(K, T)`` matches a compiled ISA path)
     - Any polygon mesh; not restricted to triangles
   * - ``TriMeshSDF<T, K, W>``
     - DCEL mesh or triangle soup
     - ``PackedBVH`` over ``TriangleAoSoA`` groups
     - ``pruneTraverse()`` over SoA-packed leaves
     - Triangle meshes only; highest throughput

``FlatMeshSDF`` is useful for correctness checks and tiny meshes. See `its doxygen page
<doxygen/html/classEBGeometry_1_1FlatMeshSDF.html>`__.

``FlatMeshSDF`` is a plain value type that can be evaluated on a GPU. It holds the DCEL mesh
descriptor by value and nothing else, so it is trivially copyable, and ``signedDistance()``,
``getClosestFace()`` and ``computeBoundingVolume()`` (the vertex AABB) are callable on both host
and device. It deliberately
does not derive from ``SignedDistanceFunction``: a class with virtual functions carries a pointer to
a host-side function table and can never be passed to a kernel. As a consequence it cannot currently
be used where an ``ImplicitFunction`` is expected, such as the CSG and transform factories. As for
``PackedBVH`` (see :ref:`Chap:MemoryModel`), freeze and mirror the pool, call
``rebasedView(devicePool)``, and pass the returned ``FlatMeshSDF`` to a kernel; ``deepCopy(pool)``
gives independent storage. Copies share the pool memory, and a ``FlatMeshSDF`` sees the mesh as it
was when it was constructed. ``MeshSDF`` and ``TriMeshSDF`` follow the same pattern; see below.

``MeshSDF`` handles arbitrary polygon meshes; its ``signedDistance()`` builds the traversal
criteria shown above (a leaf-eval and a pruning rule, not the full four-callback ``traverse()``
shape) and drives them through ``PackedBVH::pruneTraverse()``, picking up SIMD node pruning
whenever ``(K, T)`` matches a compiled ISA path and testing the children with a scalar loop
otherwise. Its BVH stores face ids, four bytes each, and every leaf test reads the face from the
mesh; see "Primitive storage" below. See `its doxygen page
<doxygen/html/classEBGeometry_1_1MeshSDF.html>`__.

``MeshSDF`` and ``TriMeshSDF`` are plain value types exactly like ``FlatMeshSDF``: ``MeshSDF``
holds the mesh descriptor and its ``PackedBVH`` by value, ``TriMeshSDF`` just its ``PackedBVH``,
all reserved from the one ``Pool`` passed to the constructor. Both are trivially copyable,
constructed without ``shared_ptr``\ s, and neither derives from ``SignedDistanceFunction``.
``signedDistance()``, ``getClosestFace()``, ``getRoot()`` and ``computeBoundingVolume()`` are
callable on host and device; ``rebasedView(pool)`` and ``deepCopy(pool)`` return the class itself,
so a rebased copy is what a kernel receives. As for ``FlatMeshSDF``, none of the three can currently
be used as an ``ImplicitFunction`` (in the CSG or transform factories).

All three report the face closest to a point with ``getClosestFace(point)``, which returns a
`ClosestFace <doxygen/html/structEBGeometry_1_1ClosestFace.html>`__: the signed distance, equal to
what ``signedDistance()`` returns (up to rounding for ``TriMeshSDF``, whose ``signedDistance()``
reduces each leaf group with SIMD instructions), and the face id, the face's index in the mesh (see :ref:`Sec:FaceIds`). ``MeshSDF`` and ``TriMeshSDF`` run the same
pruned traversal as ``signedDistance()``, keeping the winning face's id as they go. Where several
faces are equally close, at a point nearest an edge or vertex they share, the first one found is
reported. The returned id indexes whatever per-face data the caller keeps, such as a material or
boundary condition.

``TriMeshSDF`` is the recommended default for triangle meshes: it packs triangles into
Structure-of-Arrays groups of width ``W`` (via ``TreeBVH::packWith()``, see above) and builds the
same kind of thin ``pruneTraverse()`` wrapper as ``MeshSDF``, over ``TriangleAoSoA<T, W>``
leaves instead of individual faces, so that on a matching ``(K, T)`` combination each BVH leaf
evaluates ``W`` triangles with a single SIMD register operation, and even the AABB-vs-running-best
comparisons during descent are done on squared distances (no square root) until the very last
step. See `its doxygen page <doxygen/html/classEBGeometry_1_1TriMeshSDF.html>`__, and `the doxygen
page for TriangleSoAT <doxygen/html/structEBGeometry_1_1TriangleSoAT.html>`__ for the SoA storage
itself.

``TriMeshSDF::getClosestFace()`` recovers the nearest triangle's face id through the SIMD SoA path.
Each leaf group is a ``TriangleAoSoA<T, W>``: a geometry-only ``TriangleSoAT<T, W>`` plus a
physically separate per-lane ``Array<uint32_t, W>`` of face ids, the index of the mesh face each
triangle was cut from (the same wrapper relationship ``PointAoSoA`` has with ``PointSoAT``). The hot
``signedDistance()`` path never reads the id array; only ``getClosestFace()`` does, taking a scalar
per-lane step to recover the winning lane. A ``TriMeshSDF`` built from a triangle soup reports the
id each ``Triangle`` carries (``Triangle::setFaceId()``). See `the doxygen page for TriangleAoSoA
<doxygen/html/structEBGeometry_1_1TriangleAoSoA.html>`__.

What is actually vectorised in ``TriMeshSDF``/``PackedBVH`` is covered in
:ref:`Chap:SIMDClasses` -- see that page for the full detail rather than repeating it here.

Primitive storage: Face ids or triangles
________________________________________

The two classes store very different primitives:

*  ``MeshSDF``'s primitive is a face id, a ``uint32_t`` index into the mesh's face array. A face
   is only meaningful together with its mesh anyway -- it stores its half-edge as an index into the
   mesh's edge array -- so ``MeshSDF`` holds the mesh and reads every face it tests from it. The
   mesh is the only copy of the geometry: flipping the mesh after the build (``MeshT::flip()``)
   flips every distance, and after moving vertices, ``mesh.reconcile()`` followed by
   ``getRoot().refit()`` with each face's new bounding box brings the BVH up to date (see `the
   doxygen page for MeshSDF <doxygen/html/classEBGeometry_1_1MeshSDF.html>`__). The primitive
   array takes 4 bytes per face rather than the 88 of a face copy. The price is an indirection:
   the faces are read in mesh order rather than leaf order, which made queries on the armadillo
   mesh (100k faces) about 15% slower than with copies, near the surface and away from it alike.
*  ``TriMeshSDF``'s primitive is a ``TriangleAoSoA<T, W>`` group, which is fully self-contained --
   a plain aggregate of coordinate arrays plus a per-lane face-id array, with no index into
   anything else, built fresh by ``groupTrianglesIntoSoA()`` during packing. ``TriMeshSDF`` does
   not keep the mesh, so a change to the mesh after the build is not seen.

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
are 4 for ``float`` and ``double``, so a type spelled with them is the same type in every file and
in both passes of a GPU compile. The one exception is a host-only build that defines
``EBGEOMETRY_HOST_TUNED_DEFAULTS``, as EBGeometry's own CMake build does when it is the top-level
project without a GPU backend: there ``TriangleSoA::DefaultWidth<T>()`` is the host-tuned width.
``BVH::HostBranchingRatio<T>()`` and ``TriangleSoA::HostWidth<T>()`` give the values that fill one
SIMD register under the compiler's flags, for host-only code; see :ref:`Sec:DefaultKW` for when to
use which, the build switch, and measurements.

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

The K=16/float and K=8/double paths use 512-bit-wide SIMD loads on AVX-512F and require the
``ChildAABBSoA`` rows to be 64-byte aligned, which is guaranteed by ``alignas(sizeof(T)*K)`` on
the struct. Only interior nodes have children, so only they get a row: each node records its row in
``Node::m_childBoxRow``, and the row array has one entry per interior node rather than one per node. The K=8/float and K=4/double paths use 256-bit-wide AVX loads instead (as does
K=8/double without AVX-512F, in two passes), and K=4/float uses 128-bit SSE4.1 loads. All other
(K, T) combinations test the children with a scalar loop over the same ``ChildAABBSoA`` cache,
inside the same ``pruneTraverse()``.

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
* ``a_maxLeafGroups`` (the maximum number of full ``W``-sized SoA groups per BVH
  leaf, so at most ``a_maxLeafGroups * W`` raw triangles before SoA packing)
  defaults to ``4`` in ``Parser::readIntoTriangleBVH`` (the ``TriMeshSDF``
  constructors have no default), while the top-down partitioners are still free
  to split down to smaller, tighter leaves wherever the geometry calls for it. The
  space-filling curves take ``a_maxLeafGroups * W`` as their target leaf size. A
  leaf smaller than ``W`` simply pads its SoA block's unused lanes. To set each
  method's leaf size separately, see :ref:`Sec:LeafSizes`.
* ``K = BVH::DefaultBranchingRatio<T>()`` is a good default. With AVX-512F
  available you can try ``K = 16`` (float) — the child-AABB test is evaluated in
  a single SIMD batch, and the wider fan-out reduces tree depth — but measure: on the
  benchmarks in :ref:`Sec:DefaultKW` it was slower, roughly doubling the query time. A wider tree also takes a larger traversal
  stack for the same depth (see :ref:`Sec:TraversalStack`).
