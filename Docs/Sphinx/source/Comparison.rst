.. _Chap:Comparison:

How EBGeometry compares
=======================

Several good libraries answer some of the questions EBGeometry answers. This page says what each of
them is built for, where it is the better choice, and where EBGeometry differs, so that you can pick
the right tool. EBGeometry's own position is this: one header with no dependencies, in which meshes,
analytic shapes, compositions of them and point clouds are all queryable geometry, and in which the
same query code runs on a CPU and on a GPU.

Timings depend on the machine, the compiler and the instruction set, so this page makes no speed
claims of its own. The companion repository
`EBGeometry-benchmarks <https://github.com/rmrsk/EBGeometry-benchmarks>`_ runs EBGeometry beside
some of these libraries on tasks they share, cross-checks every answer against the others or a brute
force, and shows single-machine snapshots that you can rerun on your own hardware.

At a glance
-----------

.. list-table::
   :header-rows: 1
   :widths: 22 40 38

   * - Library
     - Built for
     - Where EBGeometry differs
   * - `TriangleMeshDistance <https://github.com/InteractiveComputerGraphics/TriangleMeshDistance>`_
     - Signed distance from a point to one watertight triangle mesh; header-only, double precision,
       scalar.
     - Several mesh representations, SIMD triangle kernels in either precision, composition with
       other shapes, and GPU evaluation.
   * - `fcpw <https://github.com/rohan-sawhney/fcpw>`_
     - Fast closest-point and ray queries on triangle and line-segment meshes, vectorized on the CPU
       and with a GPU path.
     - Signed distance as the core query, analytic shapes and compiled CSG of any of them, point
       clouds, and no dependencies.
   * - `libfive <https://libfive.com>`_
     - Solid modelling with functional representations: scripted CAD, interval arithmetic on a
       compiled evaluator, and meshing.
     - Meshes as signed distance fields, BVH-accelerated unions of many objects, and GPU evaluation.
       EBGeometry does not mesh its output or offer a modelling front end.
   * - `nanoflann <https://github.com/jlblancoc/nanoflann>`_
     - Nearest-neighbour and radius search in point clouds with a k-d tree; header-only.
     - Point clouds are one kind of geometry among others, searched by a BVH or a hash grid that
       also runs on a GPU. For neighbour search alone, nanoflann had the fastest per-query traversal
       in the companion benchmark.
   * - `Embree <https://www.embree.org>`_
     - Production ray tracing on CPUs (and on Intel GPUs), with point queries through user
       callbacks.
     - Distances and composition rather than rays first; header-only, and the same code on NVIDIA
       and AMD GPUs.
   * - `OptiX <https://developer.nvidia.com/rtx/ray-tracing/optix>`_
     - Ray tracing on NVIDIA GPUs, accelerated by their ray-tracing hardware.
     - Portable across CPUs and GPUs from several vendors, and answers distance queries, which ray
       tracing hardware does not. For triangle ray tracing on NVIDIA hardware alone, OptiX is the
       faster choice.

When to use something else
---------------------------

* **You only trace rays against triangles on NVIDIA or Intel hardware.** OptiX or Embree use the
  hardware and decades of tuning for that one query.
* **You only search point clouds for neighbours.** nanoflann is small, mature and fast.
* **You model solids by hand and want meshes out.** libfive comes with a modelling language, a GUI
  and meshing.
* **You need exact answers.** See CGAL below.

When EBGeometry fits
--------------------

* **You need signed distances, not only closest points or hits.** Embedded-boundary and level-set
  codes, collision and contact, and SDF rendering all need them.
* **Your geometry mixes kinds.** A mesh drilled through by an analytic cylinder, a scene of thousands
  of parts, a point cloud beside a CAD part: all of these are queried the same way.
* **The same code must run on a CPU and on a GPU.** The structures live in one memory pool that is
  mirrored to the device, and a query is one functor that runs on either side.
* **You cannot take on dependencies.** EBGeometry is one header and C++17.

An honorable mention: CGAL
--------------------------

`CGAL <https://www.cgal.org>`_ is the reference library for computational geometry. Its AABB tree
answers closest-point and intersection queries on polyhedral surfaces, its exact predicates and
kernels give answers that are robust to round-off, and it covers mesh Booleans, meshing and much
more. It is the right choice when exactness and breadth matter more than throughput or GPUs.
EBGeometry works in floating point, keeps to the queries a simulation or a renderer asks millions
of times, and stays one header.
