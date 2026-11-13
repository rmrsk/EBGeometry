.. _Chap:ImplemMeshSDF:

Mesh distance functions
=======================

A surface mesh becomes a signed distance function through one of three classes (see
:ref:`Chap:Meshes` for the concepts and for choosing between them). They share the sign convention
(negative inside, positive outside), are shapes that compose, ray trace exactly and run on a device, and
differ in what they store:

.. list-table::
   :header-rows: 1
   :widths: 22 26 52

   * - Class
     - Built from
     - Stores
   * - ``TriMeshSDF<T, K, W>``
     - a half-edge mesh or a triangle soup
     - a ``PackedBVH`` whose leaves are groups of ``W`` triangles (``TriangleAoSoA<T, W>``), copied from the
       mesh: the fastest, and the recommended default
   * - ``MeshSDF<T, K>``
     - a half-edge mesh
     - the mesh and a ``PackedBVH`` of its face ids: any planar convex polygons, and every query reads the
       mesh itself
   * - ``FlatMeshSDF<T>``
     - a half-edge mesh
     - the mesh alone: every query tests every face; for checks and tiny meshes

See the Doxygen pages of `TriMeshSDF <doxygen/html/classEBGeometry_1_1TriMeshSDF.html>`__,
`MeshSDF <doxygen/html/classEBGeometry_1_1MeshSDF.html>`__ and
`FlatMeshSDF <doxygen/html/classEBGeometry_1_1FlatMeshSDF.html>`__. The file readers build any of them in
one call (see :ref:`Chap:Parsers`):

.. code-block:: c++

   Pool pool(hostMemoryResource());

   const auto sdf = Parser::readIntoTriMeshSDF<T>("armadillo.obj", pool);   // or readIntoMeshSDF, readIntoFlatMeshSDF

   const T              d    = sdf.signedDistance(p);
   const ClosestFace<T> face = sdf.getClosestFace(p);                       // face.faceId: its position in the file

:ref:`Chap:ExampleMeshSDF` reads a mesh and queries it on the host and on a device.

Values and handles
------------------

All three are plain values with no virtual functions, trivially copyable, holding descriptors into the
one ``Pool`` passed to their constructor: ``MeshSDF`` the mesh and its BVH, ``TriMeshSDF`` its BVH,
``FlatMeshSDF`` the mesh. The pool must outlive them and every copy. As for every pool handle (see
:ref:`Chap:MemoryModel`), a copy shares the data, ``deepCopy(pool)`` gives storage of its own, and
``rebasedView(devicePool)`` is what a kernel takes after the pool is frozen and mirrored.
``signedDistance()``, ``getClosestFace()``, ``getRoot()`` and ``computeBoundingVolume()`` are callable on
the host and on a device.

A copy stored in a BVH union, or placed several times with ``Translate()`` in a tape, shares the same
data: the mesh exists once however many copies refer to it (see :ref:`Chap:ExampleNestedBVH`).

The closest face
----------------

``getClosestFace(point)`` returns a
`ClosestFace <doxygen/html/structEBGeometry_1_1ClosestFace.html>`__: the signed distance, as
``signedDistance()`` returns it, and the face id, the face's index in the mesh, which is its position in the
file (see :ref:`Sec:FaceIds`). Where several faces are equally close (at a point nearest an edge or a vertex
they share), the first one found is reported. The id indexes whatever per-face data the caller keeps, such
as a material or a boundary condition. A ``TriMeshSDF`` built from a triangle soup reports the id each
``Triangle`` carries (``Triangle::setFaceId()``).

``TriMeshSDF`` evaluates each leaf's triangles together, with SIMD instructions on the host; its values
can differ from a device's in the last bit at points nearly equally close to two triangles. What is
vectorized is described in :ref:`Chap:SIMDClasses`.

The mesh distance functions are also ray traced exactly: ``firstHit()`` and ``intersects()`` walk the BVH
along the ray and test the faces near it, reporting the face id and its normal (see
:ref:`Chap:ImplemRays`).

When the mesh changes
---------------------

* ``MeshSDF`` reads every face from its mesh, so it sees the mesh as it is: flipping the mesh
  (``MeshT::flip()``) flips every distance, and after moving vertices, ``mesh.reconcile()`` followed by
  ``getRoot().refit()`` with each face's new bounding box brings it up to date (see
  :ref:`Chap:BVHRefit`; on a device, ``DCEL::reconcile()``, :ref:`Sec:ReconcileWherePool`). The price
  of reading the faces from the mesh is that its queries are somewhat slower than over copies of them.
* ``TriMeshSDF`` copies the triangles when it is built and keeps no mesh, so a change to the mesh is not
  seen: rebuild it.
* ``FlatMeshSDF`` sees the mesh as it was when it was constructed.

The branching factor and the triangle width
-------------------------------------------

``TriMeshSDF`` takes the BVH's branching factor ``K`` and the triangle group width ``W`` as template
parameters, and ``MeshSDF`` takes ``K``. ``Parser::readIntoTriMeshSDF`` defaults them to
``BVH::DefaultBranchingRatio<T>()`` and ``TriangleSoA::DefaultWidth<T>()``, both 4, the same in every
translation unit and on a device; keep those for any type device code also uses. The host-tuned values,
the build switch that widens the default triangle group, and which of the two pays off are in
:ref:`Sec:DefaultKW`; :ref:`Chap:ExampleHostTuning` measures them on your machine. Any ``K`` and ``W``
compute the same values, with narrower registers or a scalar loop where no SIMD path matches.

A ``TriMeshSDF`` leaf holds up to ``maxLeafGroups`` groups of ``W`` triangles (4 in
``Parser::readIntoTriMeshSDF``, which builds with ``TriMeshSDF::defaultConstructionOptions(4)``); a leaf
of fewer than ``W`` triangles pads its group. To set each construction method's leaf size, see
:ref:`Sec:LeafSizes`.
