.. _Chap:ImplemBVH:

BVH
===

See :ref:`Chap:BVH` for the conceptual picture of bounding volume hierarchies (node types,
partitioning, tree pruning during traversal). This page describes building and storing a BVH;
:ref:`Chap:ImplemBVHTraversal` describes searching one, and :ref:`Chap:ImplemMeshSDF` the mesh distance
functions built on it.

.. contents:: On this page
   :local:
   :depth: 1

The BVH functionality is encapsulated in the namespace ``EBGeometry::BVH``, in three headers:

* :file:`Source/EBGeometry_PackedBVH.hpp` holds ``PackedBVH``, its node layout and traversals, and
  the types shared with the builders. It is all that code querying a BVH needs, device code in
  particular.
* :file:`Source/EBGeometry_TreeBVH.hpp` holds ``TreeBVH``, the partitioners, the default leaf
  predicate, and ``pack()``/``packWith()``. It is host-only.
* :file:`Source/EBGeometry_BVHBuild.hpp` holds the ``PackedBVH`` constructors behind ``pack()``/
  ``packWith()`` and ``PackedBVH``'s static builds, which partition a primitive list directly
  (``build()``, ``buildTopDown()``, ``buildBottomUp()`` and ``buildClusterSAH()``). It is host-only.

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
   the representation used for queries, see :ref:`Chap:PackedBVH`. See `the doxygen page
   for PackedBVH <doxygen/html/classEBGeometry_1_1BVH_1_1PackedBVH.html>`__.

The template parameters shared by both are:

*  ``T`` Floating-point precision.
*  ``P`` Primitive type. Neither representation imposes an interface requirement of its own:
   ``TreeBVH`` construction needs nothing from it except that ``PrimitiveCentroidPartitioner``
   calls ``getCentroid()`` (every other partitioner and the bottom-up build work from the bounding
   volumes alone), and
   ``PackedBVH`` holds primitives opaquely, handing them back only to whatever leaf-visit
   callback a caller supplies to ``traverse()`` (see below). Whether
   ``P`` needs a ``signedDistance(Vec3T<T>)`` member (or anything else) is entirely up to that
   callback -- see ``MeshSDF``/``TriMeshSDF::signedDistance()`` in :ref:`Chap:ImplemMeshSDF` for
   the signed-distance case; a callback could equally perform, say, a nearest-neighbor search over
   a point cloud whose primitive carries no ``signedDistance()`` at all.
*  ``BV`` Bounding volume type (``TreeBVH`` only — ``PackedBVH`` always uses
   ``BoundingVolumes::AABBT<T>`` internally).
*  ``K`` BVH degree. ``K=2`` will yield a binary tree, ``K=3`` yields a tertiary tree and so on.

``TreeBVH`` is the BVH builder node, i.e. it is the class through which we recursively build the
BVH, see :ref:`Chap:BVHConstruction`. ``TreeBVH`` has no constraint that ``P`` be wrapped in a
``shared_ptr`` itself, but the *containers* that hold primitives during construction
(``PrimitiveList<P>``, ``PrimAndBV<P, BV>``) wrap each primitive in a ``std::shared_ptr<const P>``,
so that the tree and the caller can share them.

Bounding volumes
----------------

EBGeometry supports the following bounding volume, which is defined in :file:`Source/EBGeometry_BoundingVolumes.hpp`:

*  **Axis-aligned bounding box**, which is templated as ``EBGeometry::BoundingVolumes::AABBT<T>``.
   See `the doxygen page for AABBT
   <doxygen/html/classEBGeometry_1_1BoundingVolumes_1_1AABBT.html>`__.

For full API details, see `the doxygen API <doxygen/html/namespaceEBGeometry_1_1BoundingVolumes.html>`_.
Other types of bounding volumes can in principle be added, with the only requirement being that they conform to the same interface as ``AABBT``. Note that
``PackedBVH`` hard-codes ``AABBT<T>`` as its bounding volume (see above), and ``pack()``/``packWith()``
require a ``TreeBVH`` built with ``AABBT<T>``, so a ``TreeBVH`` with a custom bounding volume cannot
be packed.

.. _Chap:BVHConstruction:

Construction
------------

Building a ``TreeBVH`` always starts from a list of ``(primitive, bounding volume)`` pairs; at
that point the tree is a single, unpartitioned leaf holding every primitive. Partitioning it into
an actual hierarchy is then a separate step, done by calling one of the partitioning member
functions described below -- this recursively subdivides the tree in place. The root is an ordinary
object: it may live on the stack, in a container or behind a ``shared_ptr``, and only the nodes below
it are held by ``shared_ptr``.

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
longest centroid-bounding-box axis instead of all three, for a faster build at a tree-quality cost
that is negligible on near-uniform inputs. The leaf predicate takes a ``TreeBVH``
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
The curves are Morton codes, Hilbert curves and nested indices.
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
allocation this implies can dominate build time. ``PackedBVH`` has static build functions that skip
``TreeBVH`` entirely, one per family of methods, each named for what it does. The first builds
bottom-up along a space-filling curve:

.. code-block:: cpp

   const auto packed = BVH::PackedBVH<T, P, K>::buildBottomUp(pool, std::move(primsAndBVs), targetLeafSize);

Every build takes the ``Pool`` its flat arrays are reserved from as its first argument; that pool
must outlive the BVH, which each build returns by value (a handle into the pool, like every other
``PackedBVH``). It takes primitives **by value**
(``std::vector<std::pair<P, BV>>``, a sink parameter the caller can ``std::move`` in) rather than
requiring a ``shared_ptr``-wrapped list from the caller.

``buildBottomUp()`` sorts the primitives along a space-filling curve (``SFC::Morton`` by default;
name another as a template argument, ``buildBottomUp<SFC::Hilbert>(...)``, to select it), cuts the
sorted list into consecutive leaves near a caller-chosen **target leaf size**, and merges them upward
``K`` at a time. Unlike ``bottomUpSortAndPartition()``, whose leaf count is a power of ``K``, it
chooses from every leaf count a full ``K``-ary tree can have (:math:`L \equiv 1 \pmod{K-1}`), so the
leaves sit closer to the target (see :ref:`Sec:LeafSizes`).

Since this still produces an ordinary ``PackedBVH``, every existing traversal/query facility
(``traverse()`` with every query type, the SIMD dispatch) works with it identically, unchanged.

The second, ``buildTopDown()``, covers top-down (and SAH) construction rather than the SFC-based
one above:

.. code-block:: cpp

   using Packed = BVH::PackedBVH<T, P, K>;

   const auto centroid = Packed::buildTopDown(pool, primsAndBVs, BVH::BVCentroidPartitioner<T, P, AABBT<T>, K>);
   const auto sah      = Packed::buildTopDown(pool, primsAndBVs, BVH::BinnedSAHPartitioner<T, P, AABBT<T>, K>, stopCrit);

It takes the same partitioners and leaf predicates as ``topDownSortAndPartition()``
(``BVCentroidPartitioner``, ``BinnedSAHPartitioner``, ``PrimitiveCentroidPartitioner``,
``MidpointPartitioner``, or a caller-supplied one; without a leaf predicate it uses
``DefaultLeafPredicate``), but writes the flat node array directly rather than keeping a
``TreeBVH`` alive for the tree's lifetime, the dominant cost of the build-then-``pack()`` path. As for
``topDownSortAndPartition()``, every one of the ``K`` partitions a partitioner returns must hold at
least one primitive: an empty one would become a leaf with no primitives, which the packed layout
cannot represent, and it aborts the build, in every build, with a message naming the partition.
The :ref:`Chap:ExampleBuildBVH` example times the build paths against each other.

The third, ``buildClusterSAH()``, builds via **ClusterSAH**, a fast approximation of a full SAH tree:

.. code-block:: cpp

   const auto packed = BVH::PackedBVH<T, P, K>::buildClusterSAH(pool, std::move(primsAndBVs), BVH::ClusterSpec{maxClusterSize});

It first groups the primitives into small spatial *clusters* of at most ``maxClusterSize``
primitives, then runs binned SAH over the clusters instead of over every primitive: a tree of nearly
SAH quality, built several times faster, on uniform, surface and clustered inputs alike.
``BVH::ClusterSpec::maxClusterSize`` trades build time (larger: fewer, cheaper units) against query
speed (larger: coarser leaves); the :ref:`Chap:ExampleBuildBVH` example times its build against the
other strategies.

.. _Sec:BuildPresets:

Preset construction methods
---------------------------

The library's own BVH users -- ``MeshSDF``, ``TriMeshSDF``, ``BVHUnion``, ``BVHSmoothUnion``
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
     - Binned SAH over small spatial clusters (see above). Builds much faster than
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

``PackedBVH::build()`` takes the same value and the leaf-size settings described below, and calls
the matching named build: ``buildTopDown()`` for the three top-down splits, ``buildBottomUp()`` for
the three curves, and ``buildClusterSAH()``:

.. code-block:: cpp

   const auto packed = BVH::PackedBVH<T, P, K>::build(pool, std::move(primsAndBVs), BVH::Construction::SAH);

Without options it uses ``PackedBVH::defaultConstructionOptions()``: at most ``K - 1`` primitives
in a top-down leaf, a target of ``K`` for the curves, and the default ``ClusterSpec``.

Every one of these users accepts every value, and aborts, in every build, on a value outside the
enum. ``MeshSDF`` and ``TriMeshSDF`` build the tree methods through a ``TreeBVH`` and ``ClusterSAH``
through ``buildClusterSAH()``; ``TriMeshSDF`` then regroups each leaf into SIMD triangle groups. The
BVH unions call ``PackedBVH::build()``. A custom partitioner or leaf predicate is not a preset:
build a ``TreeBVH`` with it and ``pack()`` it, or pass it to ``buildTopDown()``.

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
       ``buildBottomUp()``). There are never more leaves than primitives, so a target below ``K``
       may be exceeded.
   * - ``ClusterSAH``
     - ``cluster``
     - The ``ClusterSpec``; a leaf holds 1 to ``K-1`` clusters.

``MeshSDF``, ``TriMeshSDF``, ``BVHUnion`` and ``BVHSmoothUnion`` each have a constructor that
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
   * - ``BVHUnion``, ``BVHSmoothUnion``
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
from its children. ``PackedBVH::refit()`` updates the boxes in place, allocating nothing, so every
query and every copy of the BVH sees them. Because it
never re-partitions, a geometry that deforms enough for primitives to migrate across the tree
accumulates looser bounding volumes over time and should periodically be rebuilt instead; see
:ref:`Chap:BVH` for that trade-off. For the exact signatures, see the Doxygen references for
`TreeBVH <doxygen/html/classEBGeometry_1_1BVH_1_1TreeBVH.html>`__ and `PackedBVH
<doxygen/html/classEBGeometry_1_1BVH_1_1PackedBVH.html>`__.

Refitting recomputes bounding volumes only, never the primitives themselves. A primitive that
caches data derived from the geometry has to be brought up to date some other way. ``MeshSDF``
(:ref:`Chap:ImplemMeshSDF`) stores face ids, not faces, so after moving the mesh's vertices,
``mesh.reconcile()`` recomputes the faces' cached normals, centroids and projection axes in the
mesh itself, and refitting its BVH (through ``getRoot()``) then updates the boxes. On a device, the mesh
is reconciled with ``DCEL::reconcile()`` (:ref:`Sec:ReconcileWherePool`). ``TriMeshSDF``'s
triangles are copies taken at build time, so after moving vertices it has to be rebuilt.

Refitting where the pool is, on a device included
_________________________________________________

``PackedBVH::refit()`` is one sweep on the host. To refit a BVH whose primitives move in device memory,
use ``BVH::refit(pool, bvh, schedule, boxOf)`` (``EBGeometry_BVHRefit.hpp``). It refits the copy of the
BVH in ``pool``, the BVH's own pool or any mirror of it, and runs where that pool's memory is: on a
device-accessible pool, one tree level at a time with a thread per node; on a host pool, the same sweep
as ``PackedBVH::refit()``. Both give the same boxes.

A level-by-level refit needs the BVH's nodes grouped by depth, which a
`BVH::RefitSchedule <doxygen/html/classEBGeometry_1_1BVH_1_1RefitSchedule.html>`__ holds. It is opt-in,
for the BVHs you refit this way, and built once on the host from the tree's shape, which a refit never
changes. Build it after the BVH and before freezing the pool: its node list is stored
in the BVH's pool, so mirroring the pool carries it to the device. Building it on a frozen pool aborts.

.. code-block:: c++

   auto bvh = BVH::PackedBVH<T, uint32_t, K>::buildBottomUp(pool, primitives, leafSize);

   BVH::RefitSchedule schedule(pool, bvh);   // after the build, before the freeze
   pool.freeze();

   Pool devicePool = Pool::mirror(pool, GPU::memoryResource());

   for (int step = 0; step < numSteps; step++) {
     moveMyPrimitives(devicePool);                                  // your own kernels
     BVH::refit(devicePool, bvh, schedule, CurrentBox{devicePool});  // your box functor
   }

How the primitives move is up to you: your own kernels, another library, ``GPU::parallelFor()``, or a
copy from the host. ``BVH::refit()`` only calls your box functor once per primitive, so the primitives
can live in your own device arrays (a BVH of ids, as a ``MeshSDF``'s) or in the BVH's primitive array.
Two rules apply:

* **The moves must be finished.** ``BVH::refit()`` launches its levels on the default stream and returns
  once the boxes are done; synchronize work on a stream that does not synchronize with the default one
  before calling it.
* **The tree's shape stays.** Move the primitives, but do not add, remove or reorder them; that, or a
  deformation large enough to make the tree loose, calls for a rebuild (and a new schedule).

On a device, the box functor runs in the kernels: it must be trivially copyable and
``EBGEOMETRY_HOST_DEVICE``, and read only memory the device can reach. In a CUDA or HIP translation unit
it must be ``EBGEOMETRY_HOST_DEVICE`` whatever the pool, since the kernels are compiled for it. A
schedule built for another BVH (another BVH of the same pool included, or this one before a rebuild), or in a
pool of another mirror chain, is refused. For the exact
signatures, see the Doxygen reference for the
`BVH namespace <doxygen/html/namespaceEBGeometry_1_1BVH.html>`__.

.. _Chap:PackedBVH:

PackedBVH
---------

In addition to the standard BVH node ``TreeBVH<T, P, BV, K>``, EBGeometry provides a ``PackedBVH`` where nodes are stored in depth-first order in a flat array.
The ``PackedBVH`` can be automatically constructed from a ``TreeBVH`` but not vice versa.

.. figure:: /_static/CompactBVH.png
   :width: 25%
   :align: center

   PackedBVH representation.
   The original BVH is traversed from top-to-bottom along the branches and laid out in linear memory.
   Each interior node stores index offsets to its children and primitives.

The rationale for the ``PackedBVH`` is its tighter memory footprint and depth-first ordering, which allows more efficient traversal, particularly when primitives are sorted in the same order.
``PackedBVH<T, P, K>`` is templated on the same ``T``, ``P``, ``K`` as ``TreeBVH`` (its bounding
volume is always ``AABBT<T>``), and internally stores two things: a flat, depth-first array of
nodes, and a global primitive array holding every primitive in leaf order.

Each node stores offsets into the flat arrays rather than pointers: a leaf its range of primitives,
an interior node the indices of its ``K`` children; the root is node 0. See `the doxygen page for
PackedBVH::Node <doxygen/html/structEBGeometry_1_1BVH_1_1PackedBVH_1_1Node.html>`__. A node's box is
read from the BVH, not the node: ``getBoundingVolume(node)`` returns it, and
``getDistanceToBoundingVolume2(node, point)`` the squared distance to it. A descent that tests a
node's children one at a time should use ``getDistanceToChildBoundingVolume2(parent, k, point)``, which
reads child ``k``'s box without loading the child. A traversal callback that needs a node's box, such as
a ``visit`` callback, asks the BVH it traverses for it.

Constructing a ``PackedBVH`` from a ``TreeBVH`` is simply a matter of flattening the
already-partitioned tree, via one of two ``TreeBVH`` member functions. Both return the
``PackedBVH`` by value: it is a handle into the pool, cheap to copy, and the pool is what must stay
alive.

*  ``pack()`` performs a straight flatten: the resulting ``PackedBVH<T, P, K>`` stores exactly
   the same primitive type ``P`` that the source tree held. The original tree is left untouched
   and can simply be discarded (or allowed to go out of scope) once it is no longer needed.
*  ``packWith<Q, Converter>()`` additionally *converts* the primitive type while flattening: the
   source tree holds primitives of type ``P``, and a user-supplied ``Converter`` is called once
   per leaf to produce the ``Q`` values that the resulting ``PackedBVH<T, Q, K>`` will store.
   This is how ``TriMeshSDF`` turns a tree of individual ``Triangle<T>`` primitives into a
   ``PackedBVH`` whose leaves hold SIMD-width ``TriangleAoSoA<T, W>`` groups instead — see
   :ref:`Chap:ImplemMeshSDF`.

See `the doxygen page for TreeBVH <doxygen/html/classEBGeometry_1_1BVH_1_1TreeBVH.html>`__ for
the exact signatures of ``pack()`` and ``packWith()``.

Primitive storage
_________________

A ``PackedBVH`` stores each primitive **inline, by value**, in one flat array held in leaf order.
There is no indirection: no per-primitive heap allocation, no pointer chase on a leaf visit, and
because packing reorders primitives into leaf order, a leaf scan walks contiguous memory.

``P`` must therefore be trivially copyable. That is a hard requirement, not a coincidence: it is
what lets the whole BVH — nodes, primitives and SoA cache alike — be mirrored into a device address
space by a plain byte copy, which a ``std::shared_ptr`` could never cross. See
:ref:`Sec:PolymorphicPrimitives` below for what this rules out.

.. _Sec:IndexedPrimitives:

Primitives named by index
_________________________

When several BVHs must share one primitive set, or when the primitives are large enough that
duplicating them is the binding constraint, make the index itself the primitive: build a
``PackedBVH<T, uint32_t, K>`` over ``(index, bounding volume)`` pairs and resolve each stored index
against the caller-owned array inside the leaf callback. That stores four bytes per primitive with
no duplication, keeps the BVH trivially copyable, and needs no special support anywhere: a
``uint32_t`` is an ordinary primitive type, so every build path accepts it — ``build()``,
``buildBottomUp()``, ``buildTopDown()``, ``buildClusterSAH()``, and ``TreeBVH::pack()`` over a
``TreeBVH<T, uint32_t, AABBT<T>, K>``.

The trade is locality: the query pays a scattered load per primitive instead of a contiguous one,
and the array the indices resolve against must outlive every BVH indexing into it — an index is not
an owner, and nothing checks that. Prefer storing the primitives themselves unless one of the two
reasons above actually applies.

Pool-backed storage
____________________

``PackedBVH``'s four arrays -- the flat node array, the primitive array, the SoA child-AABB
rows and the root box -- are ``PODVector``\ s reserved from a caller-supplied ``Pool`` (see
:ref:`Chap:MemoryModel`),
not ``std::vector``\ s. Every construction entry point therefore takes a ``Pool&``: ``pack()``,
``packWith()``, every static build, the adopting constructor, and ``MeshSDF``/``TriMeshSDF``/``PointCloudBVH``,
which pass along the pool they already take. The pool must outlive the BVH.

What this buys is that a ``PackedBVH`` is trivially copyable, a few array descriptors and the
location of its pool, so a whole hierarchy crosses to a device as a byte copy with no
pointer patching. ``rebasedView(Pool&)`` is the single sanctioned crossing, exactly as for
``DCEL::MeshT``: mirror the pool, rebase on the host, then pass the returned value to a kernel.

``getPrimitives()`` returns a ``PODSpan`` rather than a container reference. A span is a *resolved*
address into pool memory, so it must not outlive the next ``Pool::reserve`` on that pool -- re-obtain
it rather than caching it across a build step.

Holding a PackedBVH inside another class
_________________________________________

``PackedBVH`` is ``final``. A class that needs a BVH over its own payload holds one by value
instead, and keeps its own arrays in the same pool so that one ``rebasedView()`` of the held BVH
rebases everything; ``PointCloudBVH`` (:ref:`Chap:ImplemPointCloud`) is the worked case. (A derived
class would lose its payload: ``rebasedView()`` and ``deepCopy()`` return a ``PackedBVH``.)

Three public members exist for such a class. The adopting constructor
``PackedBVH(Pool&, const std::vector<Node>&, const std::vector<BV>&, const std::vector<P>&)`` takes
a node array, the box of every node and a primitive array built by some other means, such as a
builder of your own that produces the flat arrays directly. The boxes go alongside the nodes because a node
does not store its own; the constructor puts them in the box rows and assigns every node's box row
and box slot, whatever the node array held. Every
constructor, this one included, checks in every build that the node array is a well-formed
depth-first flattening (every leaf holding at least one primitive), and aborts with a message
otherwise. ``getNodes()`` returns the flat node array
as a read-only ``PODSpan``, for a class that walks the tree with a traversal of its own. And
``traversalStackDepth()`` gives the fixed traversal-stack size for the current compilation pass
(host or device; see :ref:`Sec:TraversalStack`), which the build and ``rebasedView()`` validate the
tree's depth against -- a custom traversal that keeps one entry per level, as ``traverse()``
does, can size its own stack with it and inherit the same guarantee.

Copy and move semantics
________________________

*  ``TreeBVH`` can be moved but not copied, since a copy would share the subtrees that partitioning
   changes. ``deepCopy()`` clones the node hierarchy, sharing the (immutable) primitives: to partition
   one input two ways, say.
*  ``PackedBVH`` can be copied and moved, but a copy is **not** a deep copy: it resolves against the
   same pool memory, and a write through either is seen through the other. That is what makes it
   trivially copyable, and what lets ``rebasedView()`` give a value a kernel takes directly.
   ``deepCopy(Pool&)`` gives storage of its own (indices into a caller-owned array, see
   :ref:`Sec:IndexedPrimitives`, still resolve against that array).

Neither is meant to be derived from, and ``PackedBVH`` is ``final``.

.. _Sec:PolymorphicPrimitives:

Primitives are of concrete types
________________________________

``P`` must be a concrete type. ``PackedBVH`` stores ``P`` by value, so an abstract base either fails
to compile or slices the object to its static type; naming primitives by index instead
(:ref:`Sec:IndexedPrimitives`) does not help either, since that indexes a flat array of one concrete
type, which is meaningless when the elements are of different derived types and therefore different
sizes. A ``shared_ptr`` to a polymorphic base cannot be stored either, since it is not trivially
copyable and so can never cross into a device address space.

The BVH-accelerated CSG unions (:ref:`Chap:ImplemCSG`) therefore know their primitive types at compile
time: ``BVHUnion<T, K, P>`` is a ``PackedBVH<T, P, K>`` over value-type primitives of one type ``P`` --
analytic shapes, mesh distance fields, or other unions -- and ``BVHUnion<T, K, Ps...>`` over several types
keeps one array per type, its BVH's leaves holding a type and an index, dispatched on the type instead of
a virtual call. Either is trivially copyable (:ref:`Sec:BVHUnions`). A tape (:ref:`Sec:Tape`) does the same
for an expression, its BVH holding sub-program indices and its clauses a primitive type index.

Nesting a BVH inside a BVH -- a ``BVHUnion`` over several ``TriMeshSDF`` objects, say -- works. The inner BVHs live in the same ``Pool`` as the outer one, and the outer union stores each
inner object as its layout and pairs it with its own pool location as it evaluates it (see
:ref:`Chap:MemoryModel`), so the nested hierarchy mirrors to a device as one piece.

Nesting costs little: an inner BVH is stored as its layout, a few array descriptors whatever its
size, and resolves against the shared pool. Only a primitive that holds its data inline costs its size
per placement.
