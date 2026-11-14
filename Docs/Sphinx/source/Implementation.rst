.. _Chap:Implementation:

Overview
========

EBGeometry is a header-only C++17 library with no external dependencies: ``#include <EBGeometry.hpp>``
gives all of it, in the namespace ``EBGeometry``. The pages of this section describe the classes and calls,
in the same topics as the concepts. To find one by task:

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

* **The same acceleration structure for two different problems.** Finding the closest facet in
  a surface mesh, and finding the closest object in a CSG union of many objects, are both,
  structurally, "find the closest of :math:`N` things" queries. EBGeometry solves both with the
  same bounding volume hierarchy machinery, reducing an :math:`\mathcal{O}(N)` linear scan to an
  :math:`\mathcal{O}(\log N)` tree traversal in either case. See :ref:`Chap:ImplemBVH`.

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
  macros -- never a runtime dispatch. See
  :ref:`Chap:SIMDClasses`.

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
