.. _Chap:Implementation:

Overview
========

EBGeometry is a header-only C++17 library with zero external dependencies, implemented entirely
under the ``EBGeometry`` namespace (with sub-namespaces ``EBGeometry::DCEL``, ``EBGeometry::BVH``,
``EBGeometry::Octree``, ``EBGeometry::SFC``, and ``EBGeometry::Parser`` for the major
components). A handful of design choices recur throughout the implementation:

* **Precision templating.** Every class and function is templated on a floating-point type
  ``T`` (almost always ``float`` or ``double``). Precision is a compile-time choice: the library
  has no notion of a "current" precision at runtime, and nothing dispatches on it dynamically.

* **Plain value types for distance fields, a common interface for composition.** The analytic
  shapes and the surface-mesh distance fields are plain, trivially copyable value types with no
  virtual functions, so they can be copied to a GPU and evaluated there. The transformations and
  CSG combinators instead derive from a small polymorphic interface, ``ImplicitFunction<T>`` (with
  ``SignedDistanceFunction<T>`` as a refinement of it), and wrap or combine any object that
  implements it through ordinary virtual dispatch. That interface currently accepts user-written
  implicit functions only. The built-in distance fields can be combined through the BVH-accelerated
  unions, which are plain value types too and take many objects of one type; other compositions of
  them return with the redesign of the CSG layer that replaces virtual dispatch with a linear-SSA
  tape. See :ref:`Chap:ImplemCSG`.

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
  hand-written compiler intrinsics in three places (the SoA triangle and point blocks, and
  ``PackedBVH``'s child-bounding-box pruning), selected at compile time from the standard
  compiler-predefined ISA macros -- never a runtime dispatch. See
  :ref:`Chap:SIMDClasses`.

* **Three checking mechanisms.** ``static_assert`` guards template-parameter invariants decidable
  at compile time; the always-on ``EBGEOMETRY_REQUIRE`` macro checks what a caller controls, once,
  when an object is built; the opt-in ``EBGEOMETRY_EXPECT`` macro guards internal invariants,
  including on hot paths. See :ref:`Chap:ConfigurationOptions`.

The remaining pages in this section cover each component in more detail:

* :ref:`Chap:MemoryModel` -- the placement-independent ``Pool``/``PODVector`` storage foundation
  that ``DCEL::MeshT``, ``BVH::PackedBVH`` and the mesh SDFs, point clouds and BVH unions built
  from them rest on.
* :ref:`Chap:Vector` -- the ``Vec2T``/``Vec3T`` vector types used throughout the library.
* :ref:`Chap:ImplemCSG` -- the ``ImplicitFunction``/``SignedDistanceFunction`` interface, the
  analytic shapes, transforms, and CSG combinators.
* :ref:`Chap:ImplemDCEL` -- the half-edge (DCEL) surface mesh representation.
* :ref:`Chap:ImplemBVH` -- bounding volume hierarchy construction, traversal, and the packed
  mesh/CSG signed distance function classes built on top of it.
* :ref:`Chap:ImplemOctree` -- the octree implementation.
* :ref:`Chap:Parsers` -- reading surface meshes from STL/PLY/OBJ/VTK files.
* :ref:`Chap:SIMDClasses` -- the three SIMD-accelerated classes, and exactly what is vectorised in
  each.
