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

* **A common interface for composability.** Every signed distance field and implicit function --
  analytic shapes, surface meshes, CSG combinations of either -- derives from the same small
  polymorphic interface, ``ImplicitFunction<T>`` (with ``SignedDistanceFunction<T>`` as a
  refinement of it). Because every concrete type honors exactly the same interface, the
  transform and CSG combinators can wrap or combine *any* of them interchangeably through
  ordinary virtual dispatch, without needing to know which concrete type they are actually
  holding. See :ref:`Chap:ImplemCSG`.

* **The same acceleration structure for two different problems.** Finding the closest facet in
  a surface mesh, and finding the closest object in a CSG union of many objects, are both,
  structurally, "find the closest of :math:`N` things" queries. EBGeometry solves both with the
  same bounding volume hierarchy machinery, reducing an :math:`\mathcal{O}(N)` linear scan to an
  :math:`\mathcal{O}(\log N)` tree traversal in either case. See :ref:`Chap:ImplemBVH`.

* **Value types where topology permits it.** A DCEL mesh needs pointer-based topology (a
  half-edge, its pair, and its owning face are genuinely different objects referencing each
  other), so it uses ``weak_ptr`` back-references to avoid reference cycles. Where no topology is
  needed, however, EBGeometry prefers small, self-contained value types instead -- a triangle
  with no half-edge structure, or a bounding-volume-hierarchy node stored by index offset rather
  than by pointer -- since these can be packed tightly in memory and, where the data layout
  allows it, evaluated with SIMD instructions. See :ref:`Chap:ImplemDCEL` and
  :ref:`Chap:ImplemBVH`.

* **SIMD as an opt-in, compile-time-detected layer.** SIMD acceleration is implemented with
  hand-written compiler intrinsics in exactly two places, selected at compile time from the
  standard compiler-predefined ISA macros -- never a runtime dispatch. See
  :ref:`Chap:SIMDClasses`.

* **Two complementary assertion mechanisms.** ``static_assert`` guards template-parameter
  invariants decidable at compile time; the opt-in ``EBGEOMETRY_EXPECT`` macro guards runtime
  preconditions on actual argument values. See :ref:`Chap:ConfigurationOptions`.

The remaining pages in this section cover each component in more detail:

* :ref:`Chap:Vector` -- the ``Vec2T``/``Vec3T`` vector types used throughout the library.
* :ref:`Chap:ImplemCSG` -- the ``ImplicitFunction``/``SignedDistanceFunction`` interface, the
  analytic shape library, transforms, and CSG combinators.
* :ref:`Chap:ImplemDCEL` -- the half-edge (DCEL) surface mesh representation.
* :ref:`Chap:ImplemBVH` -- bounding volume hierarchy construction, traversal, and the packed
  mesh/CSG signed distance function classes built on top of it.
* :ref:`Chap:ImplemOctree` -- the octree implementation.
* :ref:`Chap:Parsers` -- reading surface meshes from STL/PLY/OBJ/VTK files.
* :ref:`Chap:SIMDClasses` -- the two SIMD-accelerated classes, and exactly what is vectorised in
  each.
