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
     - ``HitStatus::Hit``, ``Miss``, or ``Unresolved`` (a march that ran out of steps); ``hit()`` tests
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
`RayForm <doxygen/html/structEBGeometry_1_1RayForm.html>`__ for its type with a static ``firstHit()``
taking the arguments above. It may also have a static ``spans()``, which compositions use (see
:ref:`Sec:RaySpans`), and a static ``intersects()``. These are found by their signatures: a ray form
whose ``firstHit()`` has another signature is not used. Otherwise the queries march the shape. The
library has ray forms for:

.. list-table::
   :header-rows: 1
   :widths: 35 65

   * - Shape
     - How
   * - ``PlaneSDF``, ``SphereSDF``, ``BoxSDF``, ``CylinderSDF``, ``InfiniteCylinderSDF``, ``CapsuleSDF``
     - In closed form (`EBGeometry_AnalyticRayForms.hpp
       <doxygen/html/EBGeometry__AnalyticRayForms_8hpp.html>`__).
   * - ``TriMeshSDF``
     - Its BVH walked with ``PackedBVH::traverse()`` and a ``BVH::RayQuery`` (see
       :ref:`Chap:PackedTraverse`), nearest box first, bounded by the nearest crossing so far; each leaf
       group asks ``TriangleAoSoA::rayCrossing()``, a Möller--Trumbore test of its ``W`` triangles at
       once (`EBGeometry_MeshRayForms.hpp <doxygen/html/EBGeometry__MeshRayForms_8hpp.html>`__).
       ``intersects()`` ends the walk at the first crossing.
   * - ``MeshSDF``
     - The same walk over its BVH of face ids; each face asks ``DCEL::FaceT::rayCrossing()``, which
       meets the face's plane and checks the point against every edge. The faces must be planar and
       convex, as for the mesh's signed distance.
   * - ``FlatMeshSDF``
     - Every face in turn, without a BVH, as its signed distance does: for small meshes and checks.
   * - ``Fused::Union``, ``Fused::Intersection``, ``Fused::Difference``, ``Fused::Complement``,
       ``Fused::Translate``, ``Rotate``, ``Scale``, ``Reflect``
     - Their operands' spans, combined (`EBGeometry_CSGRayForms.hpp
       <doxygen/html/EBGeometry__CSGRayForms_8hpp.html>`__). The other fused operations have no ray form.
   * - ``BVHUnion``
     - The union of the spans of the primitives whose boxes the ray meets, walking its BVH nearest box
       first and leaving out the boxes beyond what the union so far is known up to.
   * - ``Tape``
     - Its ray program, clause by clause (see :ref:`a tape's ray program <Sec:TapeRays>`).

The triangle test is written once, in ``TriangleRay::crossing()``, for one triangle and for a SIMD
pack of them, as the triangle distance is (see :ref:`Chap:SIMDClasses`): the host evaluates a leaf
group's triangles a register at a time, a device one at a time. Its barycentric tests, and the polygon
test's edge tests, are widened by the rounding error of the crossing point: a distance of a few ulps of
the coordinates (the ray origin's and the face's), whatever the face's size or the angle of the ray.
A ray through an edge or a vertex shared by two faces then crosses at least one of them however far it
comes from, and a ray passing that close outside a face counts as crossing it (the safe side, as for a
grazing marched ray). A ray lying in a face's plane does not cross it. A point counted by the
widening must also be that close to the edge's extent, not just its line, so that a sliver face is not
crossed far beyond its ends; and the polygon test takes the side of each edge that is inside from the
loop's own orientation.

Every other shape is marched: the other analytic shapes, ``BVHSmoothUnion``, the other fused operations
and shapes of your own without a ray form. The ray forms are included with their shapes' headers, so
``firstHit()`` on a shape is the same function in every translation unit.

To give a shape of your own a ray form, specialise ``RayForm`` in namespace ``EBGeometry``:

.. code-block:: c++

   template <class T>
   struct EBGeometry::RayForm<MyShape<T>>
   {
     EBGEOMETRY_HOST_DEVICE static HitT<T>
     firstHit(const MyShape<T>& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax,
              const RayOptions<T>& a_options) noexcept;

     // Optional: where the ray is inside the shape (see below).
     EBGEOMETRY_HOST_DEVICE static void
     spans(const MyShape<T>& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax,
           const RayOptions<T>& a_options, bool a_firstOnly, RaySpansT<T>& a_spans) noexcept;
   };

Its ``firstHit()`` returns the first crossing in ``[a_tMin, a_tMax]`` with every field of ``HitT``
filled in. Without ``spans()``, an intersection, difference or complement with the shape finds its
spans by calling ``firstHit()`` from one crossing to the next, a step (``RayOptions::m_tolerance``
times the scale) past each.

.. _Sec:RaySpans:

Spans
-----

A composition is traced from its operands' spans (see :ref:`Chap:Rays`), in
`EBGeometry_RaySpans.hpp <doxygen/html/EBGeometry__RaySpans_8hpp.html>`__. A
`RaySpansT <doxygen/html/structEBGeometry_1_1RaySpansT.html>`__ holds the spans of one object over an
interval, in order, at most ``RaySpansT::Capacity`` (8) of them, each a pair of
`SpanEndT <doxygen/html/structEBGeometry_1_1SpanEndT.html>`__: a distance, the leaf of the composition
it lies on, and whether it is a crossing found exactly, one found by marching, or an ``Open`` end, where
the interval (or what is known) ends rather than the surface. An end carries no normal: on a device the
lists live in each thread's local memory, and every word of a span copied or compared costs a memory
operation there, which is most of what tracing costs. ``m_horizon`` says how far the list is complete:
``add()`` drops a span past the capacity and lowers the horizon to where it begins. A ``spans()`` writes
the list over ``[a_tMin, a_tMax]``, with ``addClipped()`` for a span that may reach past the interval;
``a_firstOnly`` says that only the first crossing is needed, and the list may then stop there, its
horizon at it.

Union, intersection and complement combine lists, complete up to the smaller horizon (a union inserts
its operand's spans in place, copying no list); an affine map traces its operand along the mapped ray
(the distances scale by the map's stretch along it). A union asks its operands only for their first crossings, and
an intersection or a complement for all their spans; each operand of a union is traced only up to what
the union so far is known up to. The first crossing is the first crossing end in the final list that
comes no later than its horizon. If there is none and the horizon is short of the interval's end, the
trace starts again from the horizon, asking every operand for all its spans if it got no further. An
operand without a ray form is marched in its own box, and crossings found that way are not exact
(``HitT::m_exact``).

The normal and primitive are found once, for the crossing the trace ends at. The composition follows the
leaf its end names down to that leaf, through the affine maps above it, and calls the leaf's own
``firstHit()`` over a window of ``RayOptions::m_tolerance`` times its scale around the crossing: the leaf's
exact normal, or for a marched leaf its gradient. The normal is mapped back by the transposed matrices of
the maps, and turned to face against the ray where it enters and along it where it leaves, which is the
composition's outward normal whatever complements lie between. A fused composition numbers its leaves
depth first, a BVH union by primitive, and a tape by the clause of each leaf, which it finds again by
following its ray program, into the BVH union operand whose clauses hold it.

A shape's convex closed forms give one span at the cost of a first crossing. A mesh SDF collects the
nearest ``2 * Capacity + 1`` crossings along the ray in one walk of its BVH (bounded by the farthest kept,
once full) and pairs them by the side of the face each crosses from, skipping a repeated crossing of the
same side (a ray through an edge crosses both faces); the mesh must be closed for the pairs to mean
inside and outside. The triangle groups report every lane's crossing for this
(``TriangleAoSoA::rayCrossings()``).

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
