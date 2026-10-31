.. _Chap:ExampleHostTuning:

HostTuning
==========

Choosing the BVH branching factor ``K`` and the SIMD width ``W`` (the number of triangles or points a
leaf evaluates together): the portable defaults, which any code can use, against the values tuned to
the SIMD flags of the build, which only host-only code should. It builds a point cloud
(:ref:`Chap:PointCloud`) and a triangle mesh signed distance function both ways and times their
queries, so you can see whether host tuning pays off on your machine. See :ref:`Sec:DefaultKW` for
the difference between the two and when to use which.

The example is host-only: device code always uses the defaults, so it has no GPU section. The source
is at :file:`Examples/HostTuning/main.cpp`, in the following sections.

The two choices
---------------

The defaults,
`BVH::DefaultBranchingRatio<T>() <doxygen/html/namespaceEBGeometry_1_1BVH.html>`__ and
`TriangleSoA::DefaultWidth<T>() <doxygen/html/namespaceEBGeometry_1_1TriangleSoA.html>`__ /
``PointSoA::DefaultWidth<T>()``, are 4 whatever the compiler flags. A type spelled with them is the same
type in every file and in both passes of a CUDA or HIP compile, so an object built on the host can be
copied to a GPU and used there. Leaving ``K`` and ``W`` out of a type gives these:

.. code-block:: c++

   static_assert(std::is_same_v<PointCloudBVH<T>,
                                PointCloudBVH<T, DefaultK, PointSoA::DefaultWidth<T>()>>);

The host-tuned values, ``BVH::HostBranchingRatio<T>()``, ``TriangleSoA::HostWidth<T>()`` and
``PointSoA::HostWidth<T>()``, fill one SIMD register under the flags the file is compiled with.

A point cloud, built both ways
------------------------------

The program builds a 200,000-point
`PointCloudBVH <doxygen/html/classEBGeometry_1_1PointCloudBVH.html>`__ with the default and with the
host-tuned ``K`` and ``W`` in one pool, and times 200,000 ``closestPoint()`` queries on each.

A triangle mesh, built both ways
--------------------------------

It reads the mesh twice with ``Parser::readIntoTriMeshSDF``, once with the defaults and once as
``readIntoTriMeshSDF<T, HostK, HostW>``, and times 200,000 ``signedDistance()`` queries on each. The
two group the triangles differently, so they give the same distances to rounding; the program prints
the first query's result from each, next to the times.

Building and running
--------------------

Build it on its own (with CMake, ``make`` or a direct compiler invocation in its folder, all of which
use ``-march=native``) to compare with the portable defaults: EBGeometry's own CMake build turns on
``EBGEOMETRY_HOST_TUNED_DEFAULTS``, which makes the default triangle width the host-tuned one. See
:ref:`Chap:Building` for the three ways to compile it.

Without an argument the mesh is the small dodecahedron in :file:`Examples/data`, which is too small
for the timing to mean much; pass a larger mesh to compare:

.. code-block:: bash

   cd Examples/HostTuning
   ./HostTuning.ex ../../common-3d-test-models/data/armadillo.obj
