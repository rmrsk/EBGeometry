.. _Chap:SIMDClasses:

SIMD-accelerated classes
==========================

SIMD acceleration in EBGeometry is written with compiler intrinsics (SSE4.1, AVX and AVX-512F), not
left to compiler auto-vectorization. It is confined to the three places listed below: the triangle
groups at the leaves of a ``TriMeshSDF``, the point groups at the leaves of a ``PointCloudBVH``, and the
bounding-box test that decides which children of a ``PackedBVH`` node a point query visits. Everything
else in the library is scalar code. Each of the three stores its data as a *structure of arrays*
(one aligned array per coordinate) so that one instruction works on several triangles, points or
boxes at once. The groups are therefore over-aligned (up to 64 bytes, for AVX-512F). The library's
own storage respects that, and a misplaced member is a compile error (a ``static_assert``); a group
you allocate yourself, outside a pool or a ``std::vector``, must be aligned to its ``alignof``.

Which version of this code a file uses is fixed when it is compiled, by the compiler's SIMD flags
(``__SSE4_1__``, ``__AVX__``, ``__AVX512F__``). Compile every file of a program that uses EBGeometry
with the same flags; see "Mixing SIMD flags in one program" under
:ref:`branching factor and SIMD width <Sec:DefaultKW>` for what happens otherwise. Device code always
takes the scalar path.

.. contents:: On this page
   :local:
   :depth: 1

Triangle groups: ``TriangleSoAT<T, W>`` / ``TriangleAoSoA<T, W>``
--------------------------------------------------------------------

:file:`Source/EBGeometry_TriangleSoA.hpp` and :file:`Source/EBGeometry_TriangleAoSoA.hpp`. See
:ref:`Chap:Triangles` for why triangles are packed this way.

**What it holds:** the geometry of :math:`W` triangles (vertex positions, face, vertex and edge
normals). `TriangleSoAT <doxygen/html/structEBGeometry_1_1TriangleSoAT.html>`__ is geometry only;
`TriangleAoSoA <doxygen/html/structEBGeometry_1_1TriangleAoSoA.html>`__ adds the id of the mesh face
each triangle was cut from, stored apart from the coordinates so that a distance query reads the same
bytes either way. It is what lets ``TriMeshSDF`` report which face is closest (``getClosestFace()``, see
:ref:`Chap:MeshSDFClasses`) at no cost to the plain distance query.

**What is vectorized:** the whole signed-distance computation, ``signedDistance(point)``, for the
:math:`W` triangles at once, one triangle per SIMD lane; the result is the signed distance of smallest
magnitude in the group. ``signedDistances(point)`` returns every lane's value instead, and
``TriangleAoSoA::signedDistance(point, faceId)`` also reports the winning face. The ray-crossing test,
``rayCrossing()``, is vectorized the same way, through one kernel written for a scalar and for every
SIMD width (`TriangleRay::crossing() <doxygen/html/namespaceEBGeometry_1_1TriangleRay.html>`__). A group of :math:`W`
triangles then costs roughly what one triangle costs in scalar code; this is the cost of every leaf a
``TriMeshSDF`` query visits. A :math:`W` that is not a multiple of a register's lane count is still
correct, and evaluated one triangle at a time.

**Choosing** :math:`W`: the default, ``TriangleSoA::DefaultWidth<T>()``, is 4 for both precisions and
does not depend on compiler flags, so the type is the same in every file and on a GPU -- unless a
host-only build defines ``EBGEOMETRY_HOST_TUNED_DEFAULTS``, which makes it the host-tuned width.
``TriangleSoA::HostWidth<T>()`` is the width that fills one SIMD register under the compiler's flags,
for host-only code. See :ref:`Sec:DefaultKW` for when to use which, and :ref:`Chap:MeshSDFClasses` for
how to pass :math:`W` explicitly.

Point groups: ``PointSoAT<T, W>`` / ``PointAoSoA<T, W>``
----------------------------------------------------------

:file:`Source/EBGeometry_PointSoA.hpp` and :file:`Source/EBGeometry_PointAoSoA.hpp`.

**What it holds:** the positions of :math:`W` points.
`PointSoAT <doxygen/html/structEBGeometry_1_1PointSoAT.html>`__ holds positions only;
`PointAoSoA <doxygen/html/structEBGeometry_1_1PointAoSoA.html>`__ adds each point's id (in a point
cloud, its index in the cloud), stored apart from the positions, as ``TriangleAoSoA`` does for face ids.
A group of fewer than :math:`W` points repeats its last point in the remaining lanes; ``numValid()``
gives the real count.

**What is vectorized:** the squared distance from a query point to all :math:`W` points.
``getDistances2()`` returns all of them (what a k-nearest-neighbor search wants),
``getMinimumDistance2()``/``getMaximumDistance2()`` the nearest and farthest (what a closest-point search
wants), and the forms without the ``2`` take the square root. This is the leaf cost of a
``PointCloudBVH`` search; see :ref:`Chap:ExamplePointClouds`. Only a :math:`W` that fills one register
(or, for ``double`` under AVX, two) is vectorized -- the widths ``PointSoA::HostWidth<T>()`` gives, and
4 ``float`` under SSE4.1 -- and any other :math:`W` is evaluated with a scalar loop.

**Choosing** :math:`W`: as for the triangle groups, the default ``PointSoA::DefaultWidth<T>()`` is 4, and
``PointSoA::HostWidth<T>()`` is the host-tuned width (see :ref:`Sec:DefaultKW`). Pass either, or any
other width, as the last template argument. ``EBGEOMETRY_HOST_TUNED_DEFAULTS`` does not change this
default: wider point groups are no faster.

Bounding-box pruning: ``BVH::PackedBVH<T, P, K>``
---------------------------------------------------

:file:`Source/EBGeometry_PackedBVH.hpp`, with the queries in :file:`Source/EBGeometry_BVHQuery.hpp`.
See :ref:`Chap:BVH` for bounding volume hierarchies and pruning, and :ref:`Chap:PackedBVH` for the
packed representation.

**What it holds:** the bounding boxes of each interior node's :math:`K` children, together, one row
per node (`BVH::ChildBoxes <doxygen/html/structEBGeometry_1_1BVH_1_1ChildBoxes.html>`__).

**What is vectorized:** the distance from the query point to all :math:`K` child boxes of a node, the
test that decides which children
`PackedBVH::traverse() <doxygen/html/classEBGeometry_1_1BVH_1_1PackedBVH.html>`__ descends into with a
``PointQuery``. A ``RayQuery`` tests the child boxes against a ray on SIMD lanes the same way. Every
search built on ``traverse()`` gets this, including ``MeshSDF``/``TriMeshSDF::signedDistance()`` and a
search with your own leaf callback (see :ref:`Chap:PackedTraverse`). For a :math:`(K, T)` pair that no
compiled register fits, this step is a scalar loop over the children, with the same results (unless
the compiler fuses the scalar loop's multiplies and adds).

**What this means in practice:** the cost of choosing which children to visit grows little with
:math:`K` as long as the :math:`K` boxes fit in one SIMD batch, which is why :math:`K` has a host-tuned
value.

**Choosing** :math:`K`: the default ``BVH::DefaultBranchingRatio<T>()`` is 4, and
``BVH::HostBranchingRatio<T>()`` is the host-tuned value, chosen from the same SIMD flags as
``TriangleSoA::HostWidth<T>()`` so that the two give a matching :math:`(K, W)` pair.
``EBGEOMETRY_HOST_TUNED_DEFAULTS`` does not change this default: a wider :math:`K` is no faster, and
:math:`K = 16` is markedly slower. See :ref:`Sec:DefaultKW` and :ref:`Chap:MeshSDFClasses`, and
:ref:`Chap:ExampleHostTuning` to measure your own case.
