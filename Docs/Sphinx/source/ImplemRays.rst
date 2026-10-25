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
`RayForm <doxygen/html/structEBGeometry_1_1RayForm.html>`__ for its type, with ``available = true`` and
a static ``firstHit()`` with the arguments above (and optionally a static ``intersects()``). Otherwise
the queries march it. The library has ray forms for:

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

The triangle test is written once, in ``TriangleRay::crossing()``, for one triangle and for a SIMD
pack of them, as the triangle distance is (see :ref:`Chap:SIMDClasses`): the host evaluates a leaf
group's triangles a register at a time, a device one at a time. Its barycentric tests are widened by a
few ulps, so that a ray through an edge or a vertex shared by two triangles crosses one of them; a ray
lying in a triangle's plane does not cross it.

Every other shape is marched: the other analytic shapes, ``MeshSDF``, ``FlatMeshSDF``, the BVH unions,
the fused compositions, tapes and shapes of your own. The ray forms are included with their shapes'
headers, so ``firstHit()`` on a shape is the same function in every translation unit.

To give a shape of your own a ray form, specialise ``RayForm`` in namespace ``EBGeometry``:

.. code-block:: c++

   template <class T>
   struct EBGeometry::RayForm<MyShape<T>>
   {
     static constexpr bool available = true;

     EBGEOMETRY_HOST_DEVICE static HitT<T>
     firstHit(const MyShape<T>& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax,
              const RayOptions<T>& a_options) noexcept;
   };

Its ``firstHit()`` returns the first crossing in ``[a_tMin, a_tMax]`` with every field of ``HitT``
filled in.

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

A marched ray that comes within the tolerance of the surface counts as a hit, even where it only
grazes the surface (passes an edge without entering): conservative for a particle, which then never
slips through an edge. Whether such a ray is a hit is decided by rounding, so the host and a device
can answer it differently.

The ray forms ignore the options.

Walking the BVH yourself
------------------------

When the queries do not report what you need -- every face a ray crosses, say, or a condition of your
own to stop on -- walk a mesh's BVH with ``PackedBVH::traverse()`` and a ``BVH::RayQuery`` directly,
with your own state, leaf callback and bound. :ref:`Chap:ExampleRayTracing` does this beside the
queries, on the host and on a device.
