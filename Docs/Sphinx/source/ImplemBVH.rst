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
*  ``BVH::PackedBVH<T, P, K>``, where the nodes are stored in depth-first order in a
   flat array and contain index offsets to children and primitives rather than pointers. This is
   the representation used for fast queries, see :ref:`Chap:PackedBVH`. See `the doxygen page
   for PackedBVH <doxygen/html/classEBGeometry_1_1BVH_1_1PackedBVH.html>`__.

The template parameters shared by both are:

*  ``T`` Floating-point precision.
*  ``P`` Primitive type. The requirement on ``P`` differs between the two representations, since
   each only calls what it actually needs: ``TreeBVH`` construction/partitioning only ever calls
   ``getCentroid()`` on it, while ``signedDistance(Vec3T<T>)`` is required only if the *packed*
   tree's own built-in ``PackedBVH::signedDistance()`` query is used (see below) -- a ``P`` driven
   entirely through a custom ``traverse()`` updater need not provide ``signedDistance()`` at all.
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
takes two optional arguments: a *partitioner* and a *stop function*.

The partitioner is a functor that splits a list of ``(primitive, BV)`` pairs into ``K`` new
lists whenever a leaf is subdivided. Three ready-made partitioners are provided:
``BVCentroidPartitioner`` (splits on bounding-volume centroids along the longest axis -- the
default), ``PrimitiveCentroidPartitioner`` (the same idea, but splits on primitive centroids
instead), and ``BinnedSAHPartitioner`` (a Surface-Area-Heuristic partitioner, used automatically
when building via ``BVH::Build::SAH`` -- see below -- and typically producing the
best-performing trees at a higher construction cost). The stop function takes a ``TreeBVH`` node
and decides whether it should be split any further; a default is provided, but callers are free
to supply their own of either kind.

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
   This is how ``TriMeshSDF`` turns a tree of individual ``Triangle<T, Meta>`` primitives into a
   ``PackedBVH`` whose leaves hold SIMD-width ``TriangleSoAT<T, W>`` groups instead — see
   :ref:`Chap:MeshSDFClasses`.

See `the doxygen page for TreeBVH <doxygen/html/classEBGeometry_1_1BVH_1_1TreeBVH.html>`__ for
the exact signatures of ``pack()`` and ``packWith()``.

Tree traversal
---------------

Both ``TreeBVH`` (full BVH) and ``PackedBVH`` (flattened BVH) include routines for traversing the
BVH with user-specified criteria. For both BVH representations, tree traversal is done through a
single ``traverse()`` member function taking four caller-supplied callbacks, and uses a
stack-based traversal pattern driven by those callbacks. The full type signatures for the four
callback roles below are documented on `the BVH namespace's doxygen page
<doxygen/html/namespaceEBGeometry_1_1BVH.html>`__ (look for ``Visiter``, ``Sorter``/
``PackedSorter``, ``Updater``/``PackedUpdater``, and ``MetaUpdater``).

Node visit
__________

The *visiter* callback decides, for a given node and its associated meta-data (see below),
whether that node's subtree should be investigated or pruned from the traversal: it is a
predicate taking the node and its meta-data, returning ``true`` to visit the subtree and
``false`` to prune it. Typically, the meta-data parameter will contain the necessary information
that determines whether or not to visit the subtree.

Traversal pattern
_________________

If a subtree is visited in the traversal, there is a question of which of the child nodes to visit first.
The *sorter* callback determines this order by letting the user sort the ``K`` children (each
paired with its meta-data) in-place based on order of importance -- for ``PackedBVH`` the
children are identified by their node index rather than a pointer, halving the per-entry stack
size relative to ``TreeBVH``. Note that a correct visitation pattern can yield large performance
benefits. Sorting the child nodes is completely optional; the user can leave this function empty
if it does not matter which subtrees are visited first.

Update rule
___________

If a leaf node is visited in the traversal, distance or other types of queries to the geometric
primitive(s) in the node are usually made. These are done by the *updater* callback. For
``PackedBVH`` this callback receives an offset and count into the BVH's global primitive array,
rather than a freshly-allocated sub-list, avoiding a heap allocation per leaf visit; for
``TreeBVH`` it receives the leaf's primitive list directly. Typically, the updater will modify
parameters that appear in a local scope outside of the tree traversal (e.g. updating the minimum
distance to a DCEL mesh).

Meta-data
_________

During the traversal, it might be necessary to compute meta-data that is helpful during the traversal, and this meta-data is attached to each node that is queried.
This meta-data is usually, but not necessarily, equal to the distance to the nodes' bounding volumes.
The *meta-updater* callback produces this meta-data for a node's children, given the node itself.
The biggest difference between the updater and the meta-updater is that the updater is *only*
called on leaf nodes whereas the meta-updater is also called for internal nodes. One typical
example for DCEL meshes is that the updater computes the distance from an input point to the
triangles in a leaf node, whereas the meta-updater computes the distance from the input point to
the bounding volumes of a child node. This information is then used by the sorter in order to
determine a preferred child visit pattern when descending along subtrees.

Traversal algorithm
___________________

``PackedBVH::traverse()`` implements this with a non-recursive, vector-backed stack rather than
recursion. Each stack entry holds a node index together with that node's already-computed
meta-data. The root is pushed first; then, until the stack is empty, the traversal pops an entry,
asks the visiter whether to visit it, and if so either runs the updater (if it is a leaf) or
computes the meta-updater for each of its ``K`` children, lets the sorter reorder them, and
pushes them all onto the stack. For the full API, see the Doxygen reference for
`PackedBVH <doxygen/html/classEBGeometry_1_1BVH_1_1PackedBVH.html>`__.

.. note::

   ``BVH::PackedBVH::signedDistance`` does **not** go through this generic traversal on
   the SIMD code paths — it uses a hand-written, SIMD-batched stack walk instead, described in
   :ref:`Chap:MeshSDFClasses` below and in :ref:`Chap:SIMDClasses`. The generic ``traverse()``
   above is what the *scalar* fallback of ``signedDistance()`` uses, and what every other
   BVH-accelerated query in EBGeometry (mesh nearest-face queries, BVH-accelerated CSG unions) is
   built on.

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
the updater scans a leaf's faces and keeps the signed distance with the smallest magnitude seen so
far; the visiter prunes any node whose bounding-volume distance already exceeds that magnitude;
the sorter visits the closest child first; and the meta-updater supplies each node's distance to
its bounding volume. For the full API, see the Doxygen reference for
`MeshSDF <doxygen/html/classEBGeometry_1_1MeshSDF.html>`__.

CSG Union
^^^^^^^^^

Combinations of implicit functions in EBGeometry into aggregate objects can be done by means of CSG unions.
One such union is known as the *smooth union*, in which the transition between two objects is gradual rather than abrupt.

``BVHSmoothUnionIF::value()`` traverses the tree while tracking the two smallest values seen so
far, ``a`` and ``b`` (``a`` the closest, ``b`` the second-closest): the updater updates both as
leaves are scanned, the visiter prunes any node whose bounding-volume distance exceeds both,
the sorter visits the closest child first, and the meta-updater again supplies each node's
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
     - ``PackedBVH`` over ``DCEL::FaceT``
     - Generic ``traverse()`` (scalar)
     - Any polygon mesh; not restricted to triangles
   * - ``TriMeshSDF<T, Meta, K, W>``
     - DCEL mesh or triangle soup
     - ``PackedBVH`` over SoA triangle groups
     - SIMD-batched ``signedDistance()``
     - Triangle meshes only; highest throughput

``FlatMeshSDF`` is useful for correctness checks and tiny meshes. See `its doxygen page
<doxygen/html/classEBGeometry_1_1FlatMeshSDF.html>`__.

``MeshSDF`` handles arbitrary polygon meshes; its ``signedDistance()`` builds the traversal
criteria shown above and drives them through the generic, scalar ``PackedBVH::traverse()``. See
`its doxygen page <doxygen/html/classEBGeometry_1_1MeshSDF.html>`__.

``TriMeshSDF`` is the recommended default for triangle meshes: it packs triangles into
Structure-of-Arrays groups of width ``W`` (via ``TreeBVH::packWith()``, see above) and calls
``PackedBVH::signedDistance()`` directly, so that on a matching ``(K, T)`` combination each BVH
leaf evaluates ``W`` triangles with a single SIMD register operation, and even the
AABB-vs-running-best comparisons during descent are done on squared distances (no square root)
until the very last step. See `its doxygen page
<doxygen/html/classEBGeometry_1_1TriMeshSDF.html>`__, and `the doxygen page for TriangleSoAT
<doxygen/html/structEBGeometry_1_1TriangleSoAT.html>`__ for the SoA storage itself.

What is actually vectorised in ``TriMeshSDF``/``PackedBVH`` is covered in
:ref:`Chap:SIMDClasses` -- see that page for the full detail rather than repeating it here.

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
