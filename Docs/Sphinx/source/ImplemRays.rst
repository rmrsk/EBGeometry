.. _Chap:ImplemRays:

Ray queries
===========

This page covers the classes and functions behind :ref:`Chap:Rays`; read that first for the
conceptual picture. Everything here is in
`EBGeometry_Ray.hpp <doxygen/html/EBGeometry__Ray_8hpp.html>`__, apart from the ray forms, which sit
next to the shapes they trace.

The queries
-----------

Four free functions take any shape (see :ref:`Chap:ImplemCSG` for what a shape is) and are callable on
a device:

.. code-block:: c++

   HitT<T>   firstHit  (shape, RayT<T>{origin, direction}, tMin, tMax, options = {});
   HitT<T>   firstHit  (shape, from, to, tMin = 0, options = {});
   HitStatus intersects(shape, RayT<T>{origin, direction}, tMin, tMax, options = {});
   HitStatus intersects(shape, from, to, tMin = 0, options = {});

`RayT <doxygen/html/structEBGeometry_1_1RayT.html>`__ is an origin and a direction, which must be of
unit length so that ``t`` is a distance. A segment from ``from`` to ``to`` is the ray from ``from``
towards ``to`` over ``[tMin, |to - from|]``, so its ``t`` is the distance from ``from``; a segment of
zero length crosses nothing. ``firstHit()`` returns the first crossing in the interval, entering or
leaving, as a `HitT <doxygen/html/structEBGeometry_1_1HitT.html>`__:

.. list-table::
   :header-rows: 1
   :widths: 20 80

   * - Member
     - Meaning
   * - ``m_status``
     - ``HitStatus::Hit``, ``Miss``, or ``Unresolved`` (a march that ran out of steps, or a
       traced composition that ran out of passes); ``hit()`` tests
       for ``Hit``.
   * - ``m_t``, ``m_point``
     - The crossing's distance along the ray, and its point (for ``Unresolved``, the last point
       reached; for ``Miss``, ``m_t`` is infinity).
   * - ``m_normal``
     - The outward unit normal at the crossing.
   * - ``m_entering``
     - Whether the ray enters the object there, rather than leaving it.
   * - ``m_primitive``
     - The finest part the shape can name: the face of a mesh SDF (its index in the file), the
       deciding leaf of a tape (``Tape::closestPrimitive()``), and 0 for any other shape.
   * - ``m_exact``
     - Whether the crossing was found exactly (by a ray form) rather than by marching.

``intersects()`` returns only the status, and where the shape allows it stops at the first crossing it
finds rather than the nearest. On a segment of a marched shape it first compares the signs of the value
at the two ends: different signs mean a crossing with no search at all. A shape with a ray form is
searched instead, which is cheaper and exact where the sign is not (a mesh with holes). A crossing at ``tMin`` counts, ``tMin = 0``
included; to step off a surface, pass a small ``tMin``.

``Tape`` also has the four as members (``tape.firstHit(...)``, ``tape.intersects(...)``), which do the
same.

Ray forms
---------

A shape is ray traced exactly when it has a *ray form*: a specialisation of
`RayForm <doxygen/html/structEBGeometry_1_1RayForm.html>`__ for its type. Every other shape is
marched (see `The march`_ below). The library has ray forms for:

.. list-table::
   :header-rows: 1
   :widths: 35 65

   * - Shape
     - How
   * - ``PlaneSDF``, ``SphereSDF``, ``BoxSDF``, ``CylinderSDF``, ``InfiniteCylinderSDF``, ``CapsuleSDF``
     - In closed form (`EBGeometry_AnalyticRayForms.hpp
       <doxygen/html/EBGeometry__AnalyticRayForms_8hpp.html>`__). The round surfaces are solved without
       cancellation, so that in ``float`` too a small ball or thin cylinder far from the ray's start is not
       hit or missed by rounding.
   * - ``TriMeshSDF``, ``MeshSDF``
     - Their BVH, walked nearest box first, and the faces in the boxes the ray meets
       (`EBGeometry_MeshRayForms.hpp <doxygen/html/EBGeometry__MeshRayForms_8hpp.html>`__).
       ``intersects()`` stops at the first crossing it finds. ``MeshSDF``'s faces must be planar and convex,
       as for its signed distance.
   * - ``FlatMeshSDF``
     - Every face in turn, without a BVH, as its signed distance does: for small meshes and checks.
   * - ``Fused::Union``, ``Fused::Intersection``, ``Fused::Difference``, ``Fused::Complement``,
       ``Fused::Translate``, ``Rotate``, ``Scale``, ``Reflect``
     - Their operands' spans, combined (see `Compositions`_; `EBGeometry_CSGRayForms.hpp
       <doxygen/html/EBGeometry__CSGRayForms_8hpp.html>`__).
   * - ``Fused::Offset``, ``Fused::Annular`` of ``PlaneSDF``, ``SphereSDF``, ``BoxSDF``,
       ``InfiniteCylinderSDF``, ``CapsuleSDF``, ``TriMeshSDF``, ``MeshSDF``, ``FlatMeshSDF``, or of a fused
       affine map of one
     - The operand's level sets, see :ref:`Sec:OffsetRays` (`EBGeometry_OffsetRayForms.hpp
       <doxygen/html/EBGeometry__OffsetRayForms_8hpp.html>`__).
   * - ``BVHUnion``
     - The union of the spans of the primitives whose boxes the ray meets, walking its BVH nearest box
       first.
   * - ``Tape``
     - Its ray program, clause by clause (see :ref:`a tape's ray program <Sec:TapeRays>`).

A mesh's face test counts a ray through an edge or a vertex shared by two faces as crossing at least one
of them, however far the ray comes from, and a ray passing within rounding error outside a face as
crossing it (the safe side, as for a grazing marched ray). A ray lying in a face's plane does not cross
it.

Every other shape is marched: the other analytic shapes, ``BVHSmoothUnion``, the other fused operations
and shapes of your own without a ray form. The ray forms of the analytic shapes and the mesh SDFs come
with their shapes' headers. Those of the fused operations, ``BVHUnion`` and the offsets come with
``EBGeometry_CSG.hpp`` (or ``EBGeometry.hpp``), not with ``EBGeometry_Fused.hpp`` alone: include it in
every translation unit that ray traces a composition, so that ``firstHit()`` on it is the same function
everywhere.

To give a shape of your own a ray form, specialise ``RayForm`` in namespace ``EBGeometry``:

.. code-block:: c++

   template <class T>
   struct EBGeometry::RayForm<MyShape<T>>
   {
     EBGEOMETRY_HOST_DEVICE static HitT<T>
     firstHit(const MyShape<T>& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax,
              const RayOptions<T>& a_options) noexcept;

     // Optional: where the ray is inside the shape (see Compositions below).
     EBGEOMETRY_HOST_DEVICE static void
     spans(const MyShape<T>& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax,
           const RayOptions<T>& a_options, bool a_firstOnly, RaySpansT<T>& a_spans) noexcept;
   };

Its ``firstHit()`` returns the first crossing in ``[a_tMin, a_tMax]`` with every field of ``HitT``
filled in. The optional ``spans()`` writes where the ray is inside the shape (see `Compositions`_);
without it, an intersection, difference or complement with the shape finds its spans by calling
``firstHit()`` from one crossing to the next, which is slower. A static ``intersects()``, with
``firstHit()``'s arguments and returning ``HitStatus``, may be added too, for a yes/no query that stops
at the first crossing it finds.

The functions are found by their signatures, so a ``firstHit()`` with a mistyped argument still
compiles, and the shape is silently marched. Check it where the specialisation is written:

.. code-block:: c++

   static_assert(EBGeometry::hasRayForm<MyShape<double>, double>);
   static_assert(EBGeometry::hasRaySpans<MyShape<double>, double>); // if it has spans()

.. _Sec:OffsetRays:

Offsets and annuli
~~~~~~~~~~~~~~~~~~

An offset or an annulus is traced exactly, with exact normals, where its operand's level sets have a
closed form: a sphere, capsule or infinite cylinder (the radius grows or shrinks), a plane (it moves), a
box (it shrinks, or grows into a box with rounded edges and corners), a mesh SDF (its surface grown is
the union of its faces, each grown into a thin slab with a rounded rim), or a fused affine map of one of
these. A fused ``Offset`` or ``Annular`` of such an operand always is. A tape is only when it lists
``TapeFeature::ExactOffsets`` (``Tape<T, TapeFeature::ExactOffsets>``, see :ref:`Sec:TapeFeatures`);
without it, the tape marches them.
An offset or annulus of anything else (a composition, ``CylinderSDF``, whose grown rim is a torus,
``TorusSDF``, or another offset or annulus) is marched.

Tracing a mesh's offset over a whole ray costs far more than finding its first crossing. Under an
intersection or a difference, an offset or annulus of a mesh is therefore traced only within the spans of
the other operands, as a marched operand is (see `Compositions`_).

.. _Sec:RaySpans:

Compositions
------------

A composition is traced from its operands' *spans* (see :ref:`Chap:Rays`), in
`EBGeometry_RaySpans.hpp <doxygen/html/EBGeometry__RaySpans_8hpp.html>`__. You meet them only when
writing a ray form's ``spans()``: a `RaySpansT <doxygen/html/structEBGeometry_1_1RaySpansT.html>`__ holds
the spans of one object over an interval, in order, each a pair of
`SpanEndT <doxygen/html/structEBGeometry_1_1SpanEndT.html>`__ (a distance, and whether it is a crossing,
exact or marched, or an ``Open`` end where the interval ends rather than the surface). ``addClipped()``
adds a span that may reach past the interval. ``a_firstOnly`` says that only the first crossing is
needed, and the list may then stop there. The list holds at most ``RaySpansT::Capacity`` (8) spans, and
its ``m_horizon`` says how far it is complete.

What a user sees of this:

* A composition whose leaves all have ray forms is exact: ``HitT::m_exact`` is true, and the normal is the
  leaf's own at the crossing, mapped through the affine maps above it and turned to point out of the
  composition. ``HitT::m_primitive`` is 0 for a fused composition (it is one shape), the primitive for a
  ``BVHUnion`` (a mesh's face id, or 0), and the deciding leaf for a tape.
* An operand without a ray form is marched in its own box, and a crossing on it is not exact. Under an
  intersection or a difference, it is marched only within the spans of the other operands, which is much
  cheaper. That holds for an operand that is itself marched (or the complement of one); a marched part
  deeper inside an operand (a union of a ball and a blend) is traced as written. Where you can, make the
  marched shape a direct operand of the intersection.
* A ray through more pieces than a list holds is traced again from where the list ends: never wrong, only
  slower. A trace that has not decided after many passes is ``Unresolved``, like a march out of steps, and
  ``intersects()`` returns that status too, so the caller decides what an undecided ray means.
* A mesh SDF under an intersection or a complement must be closed for its spans to mean inside and
  outside.

The march
---------

A marched query clips the interval to the shape's bounding box (grown by the tolerance), then steps
along the ray. Where the shape's distance quality (see :ref:`Chap:ImplemCSG`) is ``Exact`` or
``Bound`` it sphere traces, stepping by the value; where it is ``NotADistance`` it steps a fixed
length. At a sign change it bisects to the crossing, and the normal is the normalised central
difference of the value there (six evaluations, only for a crossing).
`RayOptions <doxygen/html/structEBGeometry_1_1RayOptions.html>`__ sets how:

.. list-table::
   :header-rows: 1
   :widths: 20 15 65

   * - Member
     - Default
     - Meaning
   * - ``m_tolerance``
     - ``1e-4``
     - A crossing is accepted where the value is below this, times the scale.
   * - ``m_scale``
     - ``0``
     - The shape's length scale; 0 takes the diagonal of its bounding box. A shape whose box is
       unbounded (a plane, a complement) has no diagonal: give it a scale, or the scale is 1.
   * - ``m_maxSteps``
     - ``512``
     - Steps before the query is ``Unresolved``.
   * - ``m_step``
     - ``0.005``
     - The fixed step, times the scale, for a value that is not a distance. Features thinner than
       this can be missed, and such a miss is reported as ``Miss``, not ``Unresolved``.
   * - ``m_march``
     - ``false``
     - March the shape even where it has a ray form: a reference for the exact answer, or a way round
       a ray form.

A marched ray that comes within the tolerance of the surface counts as a hit, even where it only
grazes the surface (passes an edge without entering): conservative for a particle, which then never
slips through an edge. Whether such a ray is a hit is decided by rounding, so the host and a device
can answer it differently.

The closed forms and the mesh SDFs ignore the options other than ``m_march``; the compositions pass
them to the operands they march.

Walking the BVH yourself
------------------------

When the queries do not report what you need -- every face a ray crosses, say, or a condition of your
own to stop on -- walk a mesh's BVH with ``PackedBVH::traverse()`` and a ``BVH::RayQuery`` directly,
with your own state, leaf callback and bound. :ref:`Chap:ExampleRayTracing` does this beside the
queries, on the host and on a device.
