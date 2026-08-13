.. _Chap:ImplemDCEL:

DCEL
====

See :ref:`Chap:DCEL` for the conceptual picture of half-edge meshes (what a DCEL is, and why
signed distance queries need one) before reading the concrete API below.

The DCEL functionality exists under the namespace ``EBGeometry::DCEL`` and contains the following functionality:

*  **Fundamental data types** like vertices, half-edges, polygons, and entire surface grids.
*  **BVH functionality** for putting DCEL grids into bounding volume hierarchies.

.. important::

   The DCEL functionality is *not* restricted to triangles, but supports N-sided polygons, including *meta-data* attached to the vertices, edges, and facets. The latter is particularly useful in case one wants to associate e.g. boundary conditions to specific triangles.

Main types
----------

The main DCEL functionality (vertices, edges, faces) is provided by classes templated on both a
floating-point precision ``T`` and a user-defined meta-data type ``Meta`` attached to each
instance:

*  ``VertexT<T, Meta>`` stores the vertex position, its (outward) normal vector, and the index of
   an outgoing half-edge from the vertex (into the owning ``MeshT``'s edge array). It also has
   member functions for computing the vertex pseudonormal, see :ref:`Chap:NormalDCEL`. For the
   full API, see the Doxygen reference for
   `VertexT <doxygen/html/classEBGeometry_1_1DCEL_1_1VertexT.html>`__.

*  ``EdgeT<T, Meta>`` represents a half-edge: it stores the indices of its owning face, the next
   edge, its pair edge, and its starting vertex -- each an index into the owning ``MeshT``'s
   corresponding array, not a pointer. For the full API, see the Doxygen reference for
   `EdgeT <doxygen/html/classEBGeometry_1_1DCEL_1_1EdgeT.html>`__.

*  ``FaceT<T, Meta>`` represents a polygon face. Besides the index of its half-edge, it also
   stores the face normal vector, a 2D embedding of the polygon, and its centroid position: the
   normal and 2D embedding exist because the signed distance computation needs them, and the
   centroid exists because BVH partitioners use it when partitioning the surface mesh. For the
   full API, see the Doxygen reference for
   `FaceT <doxygen/html/classEBGeometry_1_1DCEL_1_1FaceT.html>`__.

.. note::

   ``VertexT``, ``EdgeT``, and ``FaceT`` cross-reference each other exclusively by index into the
   owning ``MeshT``'s own vertex/edge/face arrays -- never by pointer. Every member function that
   needs to resolve one of these indices back into an actual vertex/edge/face (``signedDistance()``,
   ``reconcile()``, the point-in-face test, ...) therefore takes that owning ``MeshT`` as an
   explicit argument. Since every member is consequently a plain value (``Vec3T<T>``, ``T``,
   ``uint32_t``, an enum, or ``Meta``), all three classes are trivially copyable whenever ``T`` and
   ``Meta`` are -- they can be ``memcpy``'d or mirrored to a different address space with no pointer
   patching, as long as they are interpreted against the same mesh's arrays on the other side.

*  ``MeshT<T, Meta>`` stores an entire DCEL mesh -- all of its vertices, half-edges, and faces --
   and provides brute-force (:math:`\mathcal{O}(N)`) distance queries, ``signedDistance()`` and
   ``unsignedDistance2()``, that scan every face directly. It is not itself a
   ``SignedDistanceFunction<T>``: for anything beyond small meshes, one instead wraps a
   ``MeshT<T, Meta>`` in one of the BVH-accelerated classes described in
   :ref:`Chap:MeshSDFClasses`, which hold a ``shared_ptr<MeshT<T, Meta>>`` internally and pass it to
   every packed face's ``signedDistance()``/``unsignedDistance2()`` call, since a face's half-edge
   index is only meaningful together with the mesh it was built from. A mesh is typically never
   constructed by hand -- it is built by a file parser reading vertices and faces from disk, see
   :ref:`Chap:Parsers`. Its vertex/edge/face arrays are reserved from a caller-supplied, non-owning
   `Pool <doxygen/html/classEBGeometry_1_1Pool.html>`__ -- see :ref:`Sec:DCELMemoryModel` below for
   how this works. For the full API, see the Doxygen reference for
   `MeshT <doxygen/html/classEBGeometry_1_1DCEL_1_1MeshT.html>`__.

Meta-data can be attached to the DCEL primitives by selecting an appropriate type for ``Meta`` above.

.. _Sec:DCELMemoryModel:

Memory model
------------

See :ref:`Chap:MemoryModel` first for the generic ``Pool``/``PODVector``/``MemoryResource``
foundation (the control block, mirroring, the ``at()``-vs-``bind()`` access-style choice) --
this section covers only how ``MeshT`` specifically is built on top of it.

``MeshT<T, Meta>``'s vertex/edge/face arrays are three ``PODVector``\ s. The mesh attaches itself to
a ``Pool`` on its first ``reserveVertices()``/``reserveEdges()``/``reserveFaces()`` call and resolves
every access through that pool's control block from then on, so there is exactly one accessor of
each kind (``getVertex(i)``, ``signedDistance(p)``, ...) and no binding, freezing, or base-passing
step of any sort. A mesh is queryable as soon as it has data, including while the same pool is still
being built into -- which is what lets ``Soup``/``Parser`` (:ref:`Chap:Parsers`) reconcile and sanity
-check a mesh mid-build, and lets several meshes share one pool without coordinating.

Build-phase mutators (``reserveVertices()``/``reserveEdges()``/``reserveFaces()``,
``addVertex()``/``addEdge()``/``addFace()``) still take the ``Pool&`` explicitly, since they reserve
from it. ``isAttachedTo(pool)`` reports whether a mesh's storage came from a given pool, which is how
a caller holding both can confirm they belong together.

Because ``MeshT`` stores nothing but ``PODVector``\ s and plain values, it -- like
``VertexT``/``EdgeT``/``FaceT`` themselves -- is trivially copyable, so a mesh built and reconciled
entirely on the host can cross into another address space by value.

.. warning::

   A reference handed back by ``getVertex(i)``/``getEdge(i)``/``getFace(i)`` is an address resolved
   at the moment of the call, and any ``Pool::reserve()`` may free the block it points into. This is
   the general rule of :ref:`Sec:ResolvedAddresses` applied to ``MeshT``: resolve, use, discard, and
   carry elements across a ``reserve`` by value rather than by reference. Everything returning by
   value -- ``signedDistance()``, ``getAllVertexCoordinates()``, and so on -- is unaffected.

.. note::

   **One ``Pool`` backing many meshes is the normal case.** Building several meshes one after
   another into the same pool is both safe and cheaper than giving each its own (see
   :ref:`Chap:Parsers`): a ``grow()`` triggered while building the fifth mesh ``memcpy``\ s
   everything reserved so far, including the first four, and every mesh keeps resolving correctly
   against the new base without being told. The pool must simply outlive every mesh in it.

Crossing address spaces
^^^^^^^^^^^^^^^^^^^^^^^^^^^

``rebasedView(const Pool&)`` is the one sanctioned way to point a mesh at a
`mirror <doxygen/html/classEBGeometry_1_1Pool.html#aa141bf4919e1aeb78aaee9667da8ebc3>`__ of its own
pool. It copies only the descriptor -- the vertex/edge/face data is whatever the mirror already
contains -- and since every cross-reference is a byte offset, nothing needs patching on either side:

.. code-block:: c++

   hostPool.freeze();

   EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(hostPool, EBGeometry::deviceMemoryResource());
   const auto       deviceMesh = mesh->rebasedView(devicePool);   // rebase on the host ...

   myKernel<<<blocks, threads>>>(deviceMesh, ...);                // ... then copy by value

How the returned view resolves follows from the pool it is given, not from a separate choice by the
caller. A device-accessible target (``Device``, ``Managed``, ``Mapped``) yields a view holding a
plain base address, since a kernel cannot follow a host control block; such a view must not be
dereferenced on the host. A host-only target (``Host``, ``Pinned``) yields a view holding that
pool's control block, so it stays growth-immune exactly like the original mesh -- this is what makes
a host-to-host mirror usable, and testable without a GPU.

``rebasedView`` checks that the pool it is handed really is a mirror of the mesh's own pool
(directly, or through any number of intermediate mirrors) and that the mesh's arrays fit inside it,
which catches the realistic mistakes: the wrong pool, or a pool mirrored before the mesh's last
``reserve``. See :ref:`Sec:Assertions` for when those checks are compiled in.

.. _Chap:BVHIntegration:

BVH integration
---------------

A ``MeshT<T, Meta>`` is never queried directly for anything beyond tiny meshes -- see
:ref:`Chap:BVH` for why an :math:`\mathcal{O}(N)` scan over faces doesn't scale, and
:ref:`Chap:ImplemBVH` for how ``TreeBVH``/``PackedBVH`` are actually built and traversed. This
section covers only the DCEL-specific half of that integration: how a mesh's faces become BVH
primitives in the first place.

Embedding a mesh in a BVH is a matter of pairing each ``FaceT<T, Meta>`` with a bounding volume
and handing the resulting list to a ``TreeBVH``. Concretely,
``MeshDistanceFunctionsDetail::buildDCELTreeBVH<T, Meta, BV, K>`` (in
:file:`Source/EBGeometry_MeshDistanceFunctionsImplem.hpp`, the shared helper behind both
``MeshSDF`` and ``TriMeshSDF``'s construction) does this by:

#. Building each face's bounding volume ``BV`` directly from its vertex coordinates
   (``FaceT::getAllVertexCoordinates(mesh)``, which walks the face's half-edge loop and resolves
   each vertex index against the owning mesh).
#. Constructing a ``TreeBVH<T, FaceT<T, Meta>, BV, K>`` from the resulting
   ``(face, bounding volume)`` pairs.
#. Partitioning that tree according to the requested ``BVH::Build`` strategy (``TopDown``,
   ``Morton``, ``Nested``, or ``SAH`` -- see :ref:`Chap:BVHConstruction`), where the
   ``BVCentroidPartitioner``/``BinnedSAHPartitioner`` used by the default and SAH strategies
   consult ``FaceT::getCentroid()`` (see above) when deciding how to split a set of faces.

``MeshSDF`` then packs this ``TreeBVH`` into a ``PackedBVH`` of faces directly (``pack()``), while
``TriMeshSDF`` additionally converts each face into a triangle and groups triangles into
SIMD-width ``TriangleSoAT`` blocks while packing (``packWith()``) -- see :ref:`Chap:MeshSDFClasses`
for how the two differ, and :ref:`Chap:Parsers` for the file-reading entry points that produce a
``MeshSDF``/``TriMeshSDF`` from a mesh file directly, without driving any of the steps above by
hand.
