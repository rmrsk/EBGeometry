.. _Chap:Overview:

Overview
========

EBGeometry answers spatial queries on geometry, on the host and on a device. A mesh, an analytic shape, a
composition of thousands of objects and a cloud of particles are kinds of geometry; how far away, which part is
closest, where a ray meets it and what lies nearest are kinds of queries. The library is the table of which
queries each kind of geometry answers, the acceleration structures that make them fast, and the memory model
that lets the same query run on a GPU.

Geometry and queries
--------------------

.. list-table::
   :header-rows: 1
   :stub-columns: 1
   :widths: 22 22 22 20 14

   * -
     - Distance and inside
     - Closest
     - Rays
     - Neighbors
   * - Analytic shapes
     - signed distance
     - the surface
     - yes
     - --
   * - Meshes
     - signed distance
     - the face
     - yes
     - --
   * - Compositions (CSG, regions)
     - signed distance, plus the region at a point
     - the leaf and the surface, with the regions on its sides
     - yes, plus where the region changes
     - --
   * - Sphere clouds
     - signed distance (a shape)
     - the sphere and its surface
     - yes
     - contacts within a gap
   * - Point clouds
     - unsigned distance to the nearest point
     - the k nearest points
     - --
     - the k nearest of a cloud's own points

The rows are described in :ref:`Sec:OverviewGeometry`, the columns in :ref:`Chap:Queries`.

Several cells are the same question asked of different geometry:

* **A nearest neighbor is a closest-point query on a set of points.** A point cloud has no surface and no
  inside, so its distance is unsigned and its closest part is a point, but the query is the one a mesh
  answers with its closest face.
* **A sphere cloud is both a cloud and a shape.** Give each point a radius and the cloud has a surface, an
  inside and a signed distance: it composes, ray traces and renders like any shape, and it still answers the
  cloud's own question, which spheres touch which.
* **Every shape answers every point and ray query.** A composition of meshes, analytic shapes and sphere
  clouds is itself a shape, so a query written for one kind of geometry runs on all of them.

The empty cells are choices, not missing pieces of one design:

* **No rays against a point cloud.** A point has no extent, so a ray almost surely misses every point of a
  cloud. Give the points a radius and trace the sphere cloud instead.
* **No neighbor searches on a mesh or a composition.** Their parts are faces and leaves, found through the
  closest query; neighbor searches are for discrete sets of particles.
* **No search within a radius on a point cloud.** A point cloud answers for the k nearest; a sphere cloud
  finds the spheres within a gap of a sphere.

.. _Sec:OverviewGeometry:

The four sections
-----------------

The rest of this guide follows the table.

* **Geometry**: what you can describe. A shape is anything with a signed value at a point and a bounding box
  (:ref:`Chap:GeometryRepresentations`): analytic shapes and your own, meshes read from files
  (:ref:`Chap:Meshes`), clouds of points and spheres (:ref:`Chap:PointCloud`), and compositions of all of them
  (:ref:`Chap:ConstructiveSolidGeometry`). Regions, materials, labels and surface groups
  (:ref:`Chap:LeavesAndSurfaces`, :ref:`Chap:Regions`) are a layer of meaning on that geometry: what a part of
  it is, and what a solver does with it.
* **Queries**: what you can ask of it (:ref:`Chap:Queries`), from the value at a point to a ray's first
  crossing (:ref:`Chap:Rays`) and a picture made of many rays (:ref:`Chap:Rendering`).
* **Acceleration**: why the queries are fast. Bounding volume hierarchies (:ref:`Chap:BVH`) and uniform grids
  (:ref:`Chap:CloudSearch`) are one idea, ruling out most of the geometry without measuring the distance to
  it, used for the faces of a mesh, the objects of a union and the points of a cloud alike.
* **Where it runs**: memory pools that hold the geometry in one block, copied whole to a GPU and read there at
  their new address (:ref:`Chap:MemoryModel`), and running the queries there (:ref:`Chap:RunningOnGPU`).

Each section explains the ideas first, then the classes and calls. The :ref:`Chap:Glossary` defines the
words these pages give a meaning of their own.

Finding a call by task
----------------------

``#include <EBGeometry.hpp>`` gives all of the library, in the namespace ``EBGeometry``.

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - To
     - Read
   * - keep geometry in memory that can be copied to a GPU
     - :ref:`Chap:MemoryModel`, :ref:`Chap:RunningOnGPU`
   * - work with points and vectors
     - :ref:`Chap:Vector`
   * - read a mesh from a file, and query it
     - :ref:`Chap:Parsers`, :ref:`Chap:ImplemMeshSDF`, :ref:`Chap:ImplemDCEL`
   * - build a BVH over your own primitives, and search it
     - :ref:`Chap:ImplemBVH`, :ref:`Chap:ImplemBVHTraversal`
   * - find nearest neighbors in a cloud of points or spheres
     - :ref:`Chap:ImplemPointCloud`
   * - use analytic shapes, compose them, and compile a composition
     - :ref:`Chap:ImplemCSG`
   * - build objects of several materials, and the domains a solver computes in
     - :ref:`Chap:ImplemRegions`
   * - read models written in the ``.ebg`` language
     - :ref:`Chap:ImplemLanguage`
   * - trace rays and particles' steps through a shape
     - :ref:`Chap:ImplemRays`
   * - render a shape, and look at it in a window
     - :ref:`Chap:ImplemRender`, :ref:`Chap:Viewer`
   * - estimate a bounding box, or a normal, of any function
     - :ref:`Chap:ImplemOctree`
   * - see what runs with SIMD instructions
     - :ref:`Chap:SIMDClasses`

The namespaces follow the components: ``DCEL``, ``BVH``, ``Fused``, ``PointCloud``, ``SphereCloud``,
``TapeFeature``, ``GPU``, ``Render``, ``SFC``, ``Parser`` and ``Language``. The Doxygen reference
(`EBGeometry <doxygen/html/namespaceEBGeometry.html>`__) lists every class and function.

Design choices
--------------

A handful of choices recur throughout the library:

* **Precision templating.** The geometry is templated on a floating-point type ``T``, ``float``
  or ``double``. Precision is a compile-time choice: the library has no "current" precision at run
  time, and nothing dispatches on it.

* **Plain value types everywhere, and a compiled tape for composition.** The analytic shapes, the
  surface-mesh distance fields and the BVH unions are plain, trivially copyable value types with no
  virtual functions, so they can be copied to a GPU and evaluated there. Transformations and CSG
  combinations are described with an expression builder and compiled into a ``Tape``, a program
  over arrays of primitives that needs no virtual dispatch either. See :ref:`Chap:ImplemCSG`.

* **The same acceleration structure for different problems.** Finding the closest facet in a surface
  mesh, the closest object in a CSG union of many objects, and the closest point in a cloud are all,
  structurally, "find the closest of :math:`N` things" queries. EBGeometry solves them with the same
  bounding volume hierarchy machinery, reducing an :math:`\mathcal{O}(N)` linear scan to an
  :math:`\mathcal{O}(\log N)` tree traversal in each case. See :ref:`Chap:ImplemBVH`.

* **Value types, even where topology is needed.** A DCEL mesh's half-edges, their pairs, and
  their owning faces are genuinely different objects referencing each other -- but rather than
  pointers or reference-counted back-references, every such cross-reference is a plain
  ``uint32_t`` index into the owning mesh's own vertex/edge/face arrays. This makes
  ``VertexT``/``EdgeT``/``FaceT`` themselves trivially copyable value types, the same as a
  bounding-volume-hierarchy node stored by index offset rather than by pointer, so both can be
  packed tightly in memory and, where the data layout allows it, evaluated with SIMD instructions.
  See :ref:`Chap:ImplemDCEL` and :ref:`Chap:ImplemBVH`.

* **SIMD as an opt-in, compile-time-detected layer.** SIMD acceleration is implemented with
  compiler intrinsics in three places (the SoA triangle and point blocks, and ``PackedBVH``'s
  child-bounding-box pruning), selected at compile time from the standard compiler-predefined ISA
  macros -- never a runtime dispatch. See :ref:`Chap:SIMDClasses`.

* **Device-callable basics.** Scalar helpers and numeric limits (``Math::min``, ``Math::max``,
  ``Math::clamp``, ``Math::Limits``) and the fixed-size ``Array<T, N>`` replace their ``std``
  counterparts throughout the library, so its device code needs no special compiler flags. See
  :ref:`Sec:WritingDeviceCode`. ``GPU::parallelFor`` and ``GPU::transform`` run a functor over many
  indices or inputs, in a kernel in a CUDA or HIP build and in a host loop otherwise; see
  :ref:`Chap:RunningOnGPU`.

* **Three checking mechanisms.** ``static_assert`` guards template-parameter invariants decidable
  at compile time; the always-on ``EBGEOMETRY_REQUIRE`` macro checks what a caller controls, once,
  when an object is built; the opt-in ``EBGEOMETRY_EXPECT`` macro guards internal invariants,
  including on hot paths. See :ref:`Chap:ConfigurationOptions`.
