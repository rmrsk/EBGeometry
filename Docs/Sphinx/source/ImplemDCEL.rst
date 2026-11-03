.. _Chap:ImplemDCEL:

DCEL
====

See :ref:`Chap:DCEL` for the conceptual picture of half-edge meshes (what a DCEL is, and why
signed distance queries need one) before reading the concrete API below.

The DCEL functionality exists under the namespace ``EBGeometry::DCEL`` and contains the following functionality:

*  **Fundamental data types** like vertices, half-edges, polygons, and entire surface grids.
*  **BVH functionality** for putting DCEL grids into bounding volume hierarchies.

.. important::

   The DCEL functionality is *not* restricted to triangles, but supports N-sided polygons. Every
   face has an id, its index in the mesh, which the mesh distance functions report for the face
   closest to a point. Data attached to specific faces, such as boundary conditions, lives in the
   caller's own array indexed by that id; see :ref:`Sec:FaceIds`.

Main types
----------

The main DCEL functionality (vertices, edges, faces) is provided by classes templated on a
floating-point precision ``T``:

*  ``VertexT<T>`` stores the vertex position, its (outward) normal vector, and the index of
   an outgoing half-edge from the vertex (into the owning ``MeshT``'s edge array). It also has
   member functions for computing the vertex pseudonormal, see :ref:`Chap:NormalDCEL`. For the
   full API, see the Doxygen reference for
   `VertexT <doxygen/html/classEBGeometry_1_1DCEL_1_1VertexT.html>`__.

*  ``EdgeT<T>`` represents a half-edge: it stores the indices of its owning face, the next
   edge, its pair edge, and its starting vertex -- each an index into the owning ``MeshT``'s
   corresponding array, not a pointer. For the full API, see the Doxygen reference for
   `EdgeT <doxygen/html/classEBGeometry_1_1DCEL_1_1EdgeT.html>`__.

*  ``FaceT<T>`` represents a polygon face. Besides the index of its half-edge, it also
   stores the face normal vector, a 2D embedding of the polygon (which the signed distance
   computation needs), its centroid and its area. For the full API, see the Doxygen reference for
   `FaceT <doxygen/html/classEBGeometry_1_1DCEL_1_1FaceT.html>`__.

*  ``EdgeIteratorT<T>`` walks the half-edge loop around a face by following each edge's next-edge
   index until it is back at the face's first edge. It trusts the loop to close:
   ``MeshT::reconcile()`` checks every face's loop once and aborts, in every build, on one that does
   not, and ``MeshT::sanityCheck()`` reports it. A mesh read from a file never has a broken loop;
   one assembled by hand can, and must be reconciled before it is queried. For the full API, see
   the Doxygen reference for
   `EdgeIteratorT <doxygen/html/classEBGeometry_1_1DCEL_1_1EdgeIteratorT.html>`__.

.. note::

   ``VertexT``, ``EdgeT``, and ``FaceT`` cross-reference each other exclusively by index into the
   owning ``MeshT``'s own vertex/edge/face arrays -- never by pointer. Every member function that
   needs to resolve one of these indices back into an actual vertex/edge/face (``signedDistance()``,
   ``reconcile()``, the point-in-face test, ...) therefore takes that owning ``MeshT`` as an
   explicit argument. Since every member is consequently a plain value (``Vec3T<T>``, ``T``,
   ``uint32_t``, or an enum), all three classes are trivially copyable -- they can be ``memcpy``'d
   or mirrored to a different address space with no pointer patching, as long as they are
   interpreted against the same mesh's arrays on the other side.

*  ``MeshT<T>`` stores an entire DCEL mesh -- all of its vertices, half-edges, and faces --
   and provides brute-force (:math:`\mathcal{O}(N)`) distance queries, ``signedDistance()`` and
   ``unsignedDistance2()``, that scan every face directly. For anything beyond small meshes, one
   instead wraps a
   ``MeshT<T>`` in one of the BVH-accelerated classes described in
   :ref:`Chap:MeshSDFClasses`. ``MeshSDF`` keeps the mesh (its arrays, in the same pool) and reads
   every face it tests from it, since a face's half-edge index is only meaningful together with
   the mesh it was built from. A mesh is typically never
   constructed by hand -- it is built by a file parser reading vertices and faces from disk, see
   :ref:`Chap:Parsers`. Its vertex/edge/face arrays are reserved from a caller-supplied, non-owning
   `Pool <doxygen/html/classEBGeometry_1_1Pool.html>`__ -- see :ref:`Sec:DCELMemoryModel` below for
   how this works. For the full API, see the Doxygen reference for
   `MeshT <doxygen/html/classEBGeometry_1_1DCEL_1_1MeshT.html>`__.

.. _Sec:FaceIds:

Face ids
--------

A face's id is its index in the mesh's face array, ``mesh.getFace(id)``. The file readers number
faces in file order, leaving out degenerate faces, which they remove (see :ref:`Sec:PolygonSoup`). The
mesh distance functions report the id of the face closest to a point from ``getClosestFace()``,
on the host and on a device alike, and ``TriMeshSDF`` reports the id of the polygon face a
triangle was cut from. The DCEL classes carry no user data of their own: anything attached to a
face, such as a boundary condition or a material, is kept in an array of the caller's, indexed by
face id. The data a PLY or VTK file attaches to its faces and vertices is read that way: the
``PolygonSoup`` a reader returns holds it in arrays indexed by the same face and vertex ids (see
:ref:`Sec:PolygonSoup`). Vertices and half-edges are likewise identified by their indices.

.. _Sec:DCELMemoryModel:

Memory model
------------

See :ref:`Chap:MemoryModel` first for the generic ``Pool``/``PODVector``/``MemoryResource``
foundation (the control block, mirroring, the ``at()``-vs-``bind()`` access-style choice) --
this section covers only how ``MeshT`` specifically is built on top of it.

``MeshT<T>``'s vertex/edge/face arrays are three ``PODVector``\ s. The mesh attaches itself to
a ``Pool`` on its first ``reserveVertices()``/``reserveEdges()``/``reserveFaces()`` call and resolves
every access through that pool's control block from then on, so there is exactly one accessor of
each kind (``getVertex(i)``, ``signedDistance(p)``, ...) and no binding, freezing, or base-passing
step of any sort. A mesh is queryable as soon as it has data, including while the same pool is still
being built into -- which is what lets ``Soup``/``Parser`` (:ref:`Chap:Parsers`) reconcile and sanity
-check a mesh mid-build, and lets several meshes share one pool without coordinating.

The mutators (``reserveVertices()``/``reserveEdges()``/``reserveFaces()``,
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
`mirror <doxygen/html/classEBGeometry_1_1Pool.html#a1f1922525a40246fb93932f086e2756f>`__ of its own
pool. It copies only the descriptor -- the vertex/edge/face data is whatever the mirror already
contains -- and since every cross-reference is a byte offset, nothing needs patching on either side:

.. code-block:: c++

   hostPool.freeze();

   EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(hostPool, EBGeometry::deviceMemoryResource());
   const auto       deviceMesh = mesh.rebasedView(devicePool);    // rebase on the host ...

   myKernel<<<blocks, threads>>>(deviceMesh, ...);                // ... then copy by value

How the returned view resolves follows from the pool it is given, not from a separate choice by the
caller. A device-accessible target (``Device``, ``Managed``, ``Mapped``), which must be frozen,
yields a view holding a plain base address, since a kernel cannot follow a host control block; the
host may use such a view only if the memory is host-accessible (``Managed``, ``Mapped``), never for
a ``Device`` pool. A host-only target (``Host``, ``Pinned``) yields a view holding that
pool's control block, so it stays growth-immune exactly like the original mesh -- this is what makes
a host-to-host mirror usable, and testable without a GPU.

``rebasedView`` checks that the pool it is handed really is a mirror of the mesh's own pool
(directly, or through any number of intermediate mirrors) and that the mesh's arrays fit inside it,
which catches the realistic mistakes: the wrong pool, or a pool mirrored before the mesh's last
``reserve``. These checks are ``EBGEOMETRY_REQUIRE``\s, so they run in every build.

.. _Sec:ReconcileWherePool:

Reconciling a moved mesh where its pool is
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

After a mesh's vertices move, its faces' normals, centroids and areas, and the edge and vertex
pseudonormals, must be reconciled again before the mesh is queried. ``MeshT::reconcile()`` does that on the
host, checking the mesh's topology and finding the faces around each vertex on every call. To reconcile a
mesh whose vertices move in device memory, or to reconcile one often on the host, use
``DCEL::reconcile(pool, mesh, schedule)`` (``EBGeometry_DCEL_Reconcile.hpp``). It reconciles the copy of
the mesh in ``pool``, the mesh's own pool or any mirror of it, and runs where that pool's memory is: in
parallel on a device-accessible pool; on a host pool, in host loops, faster than ``MeshT::reconcile()`` and
with the same values.

What moving vertices never changes, the topology and which faces touch each vertex, is checked and stored
once by a
`DCEL::ReconcileSchedule <doxygen/html/classEBGeometry_1_1DCEL_1_1ReconcileSchedule.html>`__. It is opt-in,
for the meshes you reconcile this way. Build it after the mesh and before freezing the pool: it is stored in
the mesh's pool, so mirroring the pool carries it to the device. Building it on a frozen pool aborts, and so
does building it for a mesh with a broken topology, with ``MeshT::reconcile()``'s messages.

.. code-block:: c++

   auto sdf = Parser::readIntoMeshSDF<T>(file, pool);
   const auto mesh = sdf.getMesh();

   DCEL::ReconcileSchedule reconcileSchedule(pool, mesh);   // after the build, before the freeze
   BVH::RefitSchedule      refitSchedule(pool, sdf.getRoot());
   pool.freeze();

   Pool devicePool = Pool::mirror(pool, GPU::memoryResource());

   for (int step = 0; step < numSteps; step++) {
     moveMyVertices(devicePool);                              // your own kernels
     DCEL::reconcile(devicePool, mesh, reconcileSchedule);
     BVH::refit(devicePool, sdf.getRoot(), refitSchedule, FaceBox{devicePool});
   }

How the vertices move is up to you: your own kernels, another library, ``GPU::parallelFor()``, or a copy
from the host. Nothing but a status word comes back to the host in the loop: the mesh, and the mesh SDF over
it, are queried in device memory. The rules:

* **The moves must be finished.** ``DCEL::reconcile()`` launches its passes on the default stream and
  returns once they are done; synchronize work on a stream that does not synchronize with the default one
  before calling it.
* **The topology stays.** Move vertices, but do not add, remove or relink vertices, edges or faces; that
  calls for a new mesh (and a new schedule).
* **A mesh SDF needs its BVH refitted too**, with ``BVH::refit()`` (:ref:`Chap:ImplemBVH`), before it is
  queried again.

On a device the values agree with the host's to rounding, not bit for bit: device compilers fuse multiplies
and adds, and the device's math functions round differently in the last place. Moved vertices that leave a
vertex and its two neighbors on a face at the same position abort, with ``MeshT::reconcile()``'s message,
as on the host. A schedule built for another mesh, or in a pool of another mirror chain, is refused. For
the exact signatures, see the Doxygen reference for the
`DCEL namespace <doxygen/html/namespaceEBGeometry_1_1DCEL.html>`__.

.. _Chap:BVHIntegration:

BVH integration
---------------

A ``MeshT<T>`` is never queried directly for anything beyond tiny meshes -- see
:ref:`Chap:BVH` for why an :math:`\mathcal{O}(N)` scan over faces doesn't scale, and
:ref:`Chap:ImplemBVH` for how ``TreeBVH``/``PackedBVH`` are actually built and traversed. This
section covers only the DCEL-specific half of that integration: how a mesh's faces become BVH
primitives in the first place.

``MeshSDF`` puts the mesh's faces in a BVH as they are: each primitive is a face id, bounded by the
box around the face's vertices, and the tree is built with the requested ``BVH::Construction``
method (see :ref:`Sec:BuildPresets`). ``TriMeshSDF`` instead cuts each face into
``Triangle`` values and builds its BVH over SIMD-width groups of triangles, without keeping the
mesh. See :ref:`Chap:MeshSDFClasses` for how the two differ, and :ref:`Chap:Parsers` for the
file-reading entry points that build a ``MeshSDF``/``TriMeshSDF`` from a mesh file directly.
