.. _Chap:ImplemBVH:

BVH
===

The BVH functionality is encapsulated in the namespace ``EBGeometry::BVH``
(:file:`Source/EBGeometry_BVH.hpp` / :file:`EBGeometry_BVHImplem.hpp`).
For the full API, see `the doxygen API <doxygen/html/namespaceBVH.html>`__.
There are two representations of a BVH:

*  :cpp:class:`BVH::TreeBVH\<T, P, BV, K\>`, a pointer-based tree used while *building* and
   *partitioning* the hierarchy.
*  :cpp:class:`BVH::PackedBVH\<T, P, K\>`, where the nodes are stored in depth-first order in a
   flat array and contain index offsets to children and primitives rather than pointers. This is
   the representation used for fast queries, see :ref:`Chap:PackedBVH`.

The template parameters shared by both are:

*  ``T`` Floating-point precision.
*  ``P`` Primitive type to be partitioned. Must provide ``getCentroid()`` and
   ``signedDistance(Vec3T<T>)``.
*  ``BV`` Bounding volume type (``TreeBVH`` only — ``PackedBVH`` always uses
   :cpp:class:`BoundingVolumes::AABBT\<T\>` internally).
*  ``K`` BVH degree. ``K=2`` will yield a binary tree, ``K=3`` yields a tertiary tree and so on.

``TreeBVH`` is the BVH builder node, i.e. it is the class through which we recursively build the
BVH, see :ref:`Chap:BVHConstruction`. ``TreeBVH`` has no constraint that ``P`` be wrapped in a
``shared_ptr`` itself, but the *containers* that hold primitives during construction
(``PrimitiveList<P>``, ``PrimAndBV<P, BV>``) always wrap each primitive in a
``std::shared_ptr<const P>``, so that primitives can be safely shared between the tree and any
higher-level object that still refers to them by pointer (e.g. a ``DCEL::FaceT``).

Bounding volumes
----------------

``EBGeometry`` supports the following bounding volumes, which are defined in :file:`Source/EBGeometry_BoundingVolumes.hpp`:

*  **Bounding sphere**, templated as ``EBGeometry::BoundingVolumes::SphereT<T>``.
   Various constructors are available.

*  **Axis-aligned bounding box**, which is templated as ``EBGeometry::BoundingVolumes::AABBT<T>``.

For full API details, see `the doxygen API <doxygen/html/namespaceBoundingVolumes.html>`_.
Other types of bounding volumes can in principle be added, with the only requirement being that they conform to the same interface as the ``AABB`` and ``BoundingSphere`` volumes. Note that
``PackedBVH`` hard-codes ``AABBT<T>`` as its bounding volume (see above), so a custom bounding
volume can only be used with ``TreeBVH`` while building, and must still be convertible to an AABB
before the tree is packed.

.. _Chap:BVHConstruction:

Construction
------------

Constructing a BVH is done by:

#.  Constructing a ``TreeBVH`` root node from a list of ``(primitive, bounding volume)`` pairs
    (``BVH::PrimAndBVList<P, BV>``, i.e. ``std::vector<std::pair<std::shared_ptr<const P>, BV>>``).
    At this point the tree is a single, unpartitioned leaf holding every primitive.
#.  Partitioning the tree by calling one of the partitioning member functions below. This
    recursively subdivides the tree in place.

.. tip::

   The default construction methods performs the hierarchical subdivision by only considering the *bounding volumes*.
   Consequently, the build process is identical regardless of what type of primitives (e.g., triangles or analytic spheres) are contained in the BVH.

Top-down construction
______________________

Top-down construction is done through the member function ``topDownSortAndPartition()``:

.. code-block:: c++

   template <class T, class P, class BV, size_t K>
   class TreeBVH
   {
   public:
     inline void
     topDownSortAndPartition(const BVH::Partitioner<P, BV, K>&    a_partitioner  = BVCentroidPartitioner<T, P, BV, K>,
                              const BVH::StopFunction<T, P, BV, K> a_stopCrit     = nullptr);
   };

The optional input arguments have the following responsibilities:

*  ``Partitioner`` is the partitioner function when splitting a leaf node into ``K`` new leaves.
   The function takes a list of ``(primitive, BV)`` pairs which it partitions into ``K`` new lists.
   Two ready-made partitioners are provided: ``BVCentroidPartitioner`` (splits on bounding-volume
   centroids, the default) and ``PrimitiveCentroidPartitioner`` (splits on primitive centroids).
   A third, SAH-driven partitioner (``BinnedSAHPartitioner``) is used automatically when building
   via ``BVH::Build::SAH`` (see below) and typically produces the best-performing trees at a
   higher construction cost.

*  ``StopFunction`` simply takes a ``TreeBVH`` as input argument and determines if the node should be partitioned further.

Default arguments for these are provided, but users are free to partition their BVHs in their own way should they choose.

Bottom-up construction
________________________

The bottom-up construction uses a space-filling curve (e.g., a Morton curve) for first building the leaf nodes.
This construction is done such that each leaf node contains approximately the number of primitives, and all leaf nodes exist on the same level.
To use bottom-up construction, one may use the member function

.. literalinclude:: ../../../Source/EBGeometry_BVH.hpp
   :language: c++
   :lines: 641-643
   :dedent: 2

The template argument is the space-filling curve that the user wants to apply, from namespace
``EBGeometry::SFC`` (:file:`Source/EBGeometry_SFC.hpp`).
Currently, we support Morton codes and nested indices.
For Morton curves, one would e.g. call ``bottomUpSortAndPartition<SFC::Morton>`` while for nested indices (which are not recommended) the signature is likewise ``bottomUpSortAndPartition<SFC::Nested>``.

Build times for SFC-based bottom-up construction are generally speaking faster than top-down construction, but tends to produce worse trees such that traversal becomes slower.

.. tip::

   Higher-level entry points such as :cpp:func:`Parser::readIntoPackedBVH` don't require you to
   call ``topDownSortAndPartition``/``bottomUpSortAndPartition`` directly — they take a single
   ``BVH::Build`` enum value (``TopDown``, ``Morton``, ``Nested``, or ``SAH``) and dispatch to the
   corresponding construction method internally. See :ref:`Chap:Parsers`.

.. _Chap:PackedBVH:

PackedBVH
---------

In addition to the standard BVH node ``TreeBVH<T, P, BV, K>``, ``EBGeometry`` provides a ``PackedBVH`` where nodes are stored in depth-first order in a flat array.
The ``PackedBVH`` can be automatically constructed from a ``TreeBVH`` but not vice versa.

.. figure:: /_static/CompactBVH.png
   :width: 240px
   :align: center

   PackedBVH representation.
   The original BVH is traversed from top-to-bottom along the branches and laid out in linear memory.
   Each interior node stores index offsets to its children and primitives.

The rationale for the ``PackedBVH`` is its tighter memory footprint and depth-first ordering, which allows more efficient traversal, particularly when primitives are sorted in the same order.
``PackedBVH<T, P, K>`` is templated on the same ``T``, ``P``, ``K`` as ``TreeBVH`` (its bounding
volume is always ``AABBT<T>``), and stores:

*  ``Node``, a nested public struct that plays the role of ``TreeBVH`` but stores offsets along
   flat arrays rather than pointers to children:

   .. code-block:: c++

      struct Node
      {
        BV                     m_bv{};        // Bounding volume for this node's subtree.
        uint32_t               m_primOff{};   // First primitive index (leaf nodes only).
        uint32_t               m_numPrims{};  // Primitive count (zero for interior nodes).
        std::array<uint32_t,K> m_childOff{};  // Depth-first indices of the K children.
      };

   A node is a leaf if ``m_numPrims > 0`` (``isLeaf()``); its children are then found via
   ``m_childOff``, and its primitives in the range ``[m_primOff, m_primOff + m_numPrims)`` of the
   ``PackedBVH``'s global primitive array.

*  The flat node array and global primitive array themselves, held as protected members:

   .. code-block:: c++

      std::vector<Node>                      m_linearNodes;  // Flat, depth-first node array.
      std::vector<std::shared_ptr<const P>>  m_primitives;   // Global primitive list, in leaf order.

   The root node is always at index 0 of ``m_linearNodes``.

Constructing the ``PackedBVH`` is simply a matter of flattening an already-partitioned ``TreeBVH``.
This is done by calling the ``TreeBVH`` member function ``pack()``:

.. code-block:: c++

   template <class T, class P, class BV, size_t K>
   class TreeBVH
   {
   public:
     [[nodiscard]] inline std::shared_ptr<PackedBVH<T, P, K>>
     pack() const;
   };

For example:

.. code-block:: c++

   // Assume that we have built and partitioned a TreeBVH already.
   std::shared_ptr<EBGeometry::BVH::TreeBVH<T, P, BV, K>> builderBVH;

   // Flatten the tree. The original tree is untouched; let it go out of scope
   // (or reset the shared_ptr) once you no longer need it.
   std::shared_ptr<EBGeometry::BVH::PackedBVH<T, P, K>> packedBVH = builderBVH->pack();

A second member function, ``packWith<Q, Converter>()``, additionally *converts* the primitive
type while flattening: the source tree holds primitives of type ``Q``, and a user-supplied
``Converter`` is called once per leaf to produce the ``P`` values that the resulting
``PackedBVH<T, P, K>`` will store. This is how :cpp:class:`TriMeshSDF` turns a tree of individual
``Triangle<T, Meta>`` primitives into a ``PackedBVH`` whose leaves hold SIMD-width
``TriangleSoAT<T, W>`` groups instead — see :ref:`Chap:MeshSDFClasses`.

Tree traversal
---------------

Both ``TreeBVH`` (full BVH) and ``PackedBVH`` (flattened BVH) include routines for traversing the BVH with user-specified criteria.
For both BVH representations, tree traversal is done using a routine

.. code-block:: c++

    template <class Meta>
    inline void
    traverse(const BVH::PackedUpdater<P>&         a_updater,
             const BVH::Visiter<Node, Meta>&      a_visiter,
             const BVH::PackedSorter<Meta, K>&    a_sorter,
             const BVH::MetaUpdater<Node, Meta>&  a_metaUpdater) const noexcept;

The BVH trees use a stack-based traversal pattern based on visit-sort rules supplied by the user.

Node visit
__________

Here, ``a_visiter`` is a lambda function for determining if the node/subtree should be investigated or pruned from the traversal.
This function has a signature

.. code-block:: c++

  template <class NodeType, class Meta>
  using Visiter = std::function<bool(const NodeType& a_node, const Meta& a_meta)>;

where ``NodeType`` is the type of node (which is different for full/flat BVHs), and the ``Meta`` template parameter is discussed below.
If this function returns true, the node will be visisted and if the function returns false then the node will be pruned from the tree traversal. Typically, the ``Meta`` parameter will contain the necessary information that determines whether or not to visit the subtree.

Traversal pattern
_________________

If a subtree is visited in the traversal, there is a question of which of the child nodes to visit first.
The ``a_sorter`` argument determines the order by letting the user sort the nodes based on order of importance.
Note that a correct visitation pattern can yield large performance benefits.
The user is given the option to sort the child nodes based on what he/she thinks is a good order, which is done by supplying a lambda which sorts the children.
For ``PackedBVH`` this function has the signature:

.. code-block:: c++

  template <class Meta, size_t K>
  using PackedSorter = std::function<void(std::array<std::pair<uint32_t, Meta>, K>& a_children)>;

Sorting the child nodes is completely optional.
The user can leave this function empty if it does not matter which subtrees are visited first.

Update rule
___________

If a leaf node is visited in the traversal, distance or other types of queries to the geometric primitive(s) in the nodes are usually made.
These are done by a user-supplied update-rule:

.. code-block:: c++

  template <class P>
  using PackedUpdater = std::function<void(const PrimitiveList<P>& a_primitives, size_t a_offset, size_t a_count)>;

Typically, the ``PackedUpdater`` will modify parameters that appear in a local scope outside of the tree traversal (e.g. updating the minimum distance to a DCEL mesh). It receives an offset and
count into the BVH's global primitive array rather than a freshly-allocated sub-list, avoiding a
heap allocation per leaf visit.

Meta-data
_________

During the traversal, it might be necessary to compute meta-data that is helpful during the traversal, and this meta-data is attached to each node that is queried.
This meta-data is usually, but not necessarily, equal to the distance to the nodes' bounding volumes.
The signature for meta-data construction is

.. code-block:: c++

  template <class NodeType, class Meta>
  using MetaUpdater = std::function<Meta(const NodeType& a_node)>;

The biggest difference between ``PackedUpdater`` and ``MetaUpdater`` is that ``PackedUpdater`` is *only* called on leaf nodes whereas ``MetaUpdater`` is also called for internal nodes.
One typical example for DCEL meshes is that ``PackedUpdater`` computes the distance from an input point to the triangles in a leaf node, whereas ``MetaUpdater`` computes the distance from the input point to the bounding volumes of a child nodes.
This information is then used in ``PackedSorter`` in order to determine a preferred child visit pattern when descending along subtrees.

Traversal algorithm
___________________

The code-block below shows the implementation of the ``PackedBVH`` traversal.
The implementation uses a non-recursive, vector-backed stack when descending along subtrees.
Observe that each entry in the stack contains both the node index itself *and* any meta-data we want to attach to the node.
If the traversal decides to visit a node, it immediately computes the specified meta-data of the node, and the user can then sort the children based on that data.

.. literalinclude:: ../../../Source/EBGeometry_BVHImplem.hpp
   :language: c++
   :lines: 507-549
   :dedent: 0
   :caption: Generic tree traversal algorithm for PackedBVH.

.. note::

   :cpp:func:`BVH::PackedBVH::signedDistance` does **not** go through this generic traversal on
   the SIMD code paths — it uses a hand-written, SIMD-batched stack walk instead, described in
   :ref:`Chap:MeshSDFClasses` below. The generic ``traverse()`` above is what the *scalar*
   fallback of ``signedDistance()`` uses, and what every other BVH-accelerated query in
   ``EBGeometry`` (mesh nearest-face queries, BVH-accelerated CSG unions) is built on.

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
   :lines: 277-306
   :dedent: 2
   :caption: Tree traversal criterion for computing the signed distance to a DCEL mesh using the BVH accelerator (:cpp:func:`MeshSDF::signedDistance`).
	     See :file:`Source/EBGeometry_MeshDistanceFunctionsImplem.hpp` for details.

CSG Union
^^^^^^^^^

Combinations of implicit functions in ``EBGeometry`` into aggregate objects can be done by means of CSG unions.
One such union is known as the *smooth union*, in which the transition between two objects is gradual rather than abrupt.
Below, we show the traversal code for this union, where we traverse through the tree and obtains the distance to the *two* closest objects rather than a single one.

.. literalinclude:: ../../../Source/EBGeometry_CSGImplem.hpp
   :language: c++
   :lines: 551-595
   :dedent: 0
   :caption: Tree traversal when computing the smooth CSG union (:cpp:func:`BVHSmoothUnionIF::value`).
	     See :file:`Source/EBGeometry_CSGImplem.hpp` for details.

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

``FlatMeshSDF`` is useful for correctness checks and tiny meshes.
``MeshSDF`` handles arbitrary polygon meshes; its ``signedDistance()`` builds the traversal
criteria shown above and drives them through the generic, scalar ``PackedBVH::traverse()``.
``TriMeshSDF`` is the recommended default for triangle meshes: it packs triangles into
Structure-of-Arrays groups of width ``W`` and calls ``PackedBVH::signedDistance()`` directly, so
that on a matching ``(K, T)`` combination each BVH leaf evaluates ``W`` triangles with a single
SIMD register operation, and even the AABB-vs-running-best comparisons during descent are done on
squared distances (no square root) until the very last step.

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

The K=16/float and K=8/double paths use ``_mm512_load_ps`` / ``_mm512_load_pd`` respectively
and require the ``ChildAABBSoA`` struct to be 64-byte aligned, which is guaranteed by
``alignas(sizeof(T)*K)`` on the struct.  The K=8/float and K=4/double paths use 256-bit AVX
instructions (``_mm256_load_ps`` / ``_mm256_load_pd``).  All other (K, T) combinations fall back
to a scalar loop that goes through the generic ``traverse()`` described above.
