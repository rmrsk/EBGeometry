.. _Chap:ImplemRays:

Ray queries
===========

This page covers the classes and functions behind :ref:`Chap:Rays`; read that first for the question
they answer, :ref:`Chap:RayTracing` for which shapes are traced exactly, and :ref:`Chap:SphereTracing` for
how the others are marched. Everything here is in
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
       deciding leaf of a tape (``Tape::closestPrimitive()``), what its members name for a ``BVHUnion``
       (a member mesh's face), and 0 for any other shape (see
       :ref:`Sec:RayTracedHits`).
   * - ``m_exact``
     - Whether the crossing was found exactly (by a ray form) rather than by marching.

``intersects()`` returns only the status, and where the shape allows it stops at the first crossing it
finds rather than the nearest. On a segment of a marched shape it first compares the signs of the value
at the two ends: different signs mean a crossing with no search at all. A shape with a ray form is
searched instead, which is cheaper and exact where the sign is not (a mesh with holes). A crossing at
``tMin`` counts, ``tMin = 0`` included; to step off a surface, pass a small ``tMin`` (for a marched
shape, more than its tolerance; see :ref:`Chap:SphereTracing`). An ``Unresolved`` query is returned as
such by ``intersects()`` too, so the caller decides what an undecided ray means.

``Tape`` also has the four as members (``tape.firstHit(...)``, ``tape.intersects(...)``), which do the
same.

Ray forms
---------

A shape is ray traced exactly when it has a *ray form*: a specialisation of
`RayForm <doxygen/html/structEBGeometry_1_1RayForm.html>`__ for its type. Every other shape is
marched (see :ref:`Chap:SphereTracing`). The library has ray forms for:

.. list-table::
   :header-rows: 1
   :widths: 35 65

   * - Shape
     - How
   * - ``PlaneSDF``, ``SphereSDF``, ``BoxSDF``, ``CylinderSDF``, ``InfiniteCylinderSDF``, ``CapsuleSDF``
     - In closed form (`EBGeometry_AnalyticRayForms.hpp
       <doxygen/html/EBGeometry__AnalyticRayForms_8hpp.html>`__).
   * - ``TriMeshSDF``, ``MeshSDF``
     - Their BVH, walked nearest box first, and the faces in the boxes the ray meets
       (`EBGeometry_MeshRayForms.hpp <doxygen/html/EBGeometry__MeshRayForms_8hpp.html>`__).
       ``intersects()`` stops at the first crossing it finds. ``MeshSDF``'s faces must be planar and convex,
       as for its signed distance.
   * - ``FlatMeshSDF``
     - Every face in turn, without a BVH, as its signed distance does: for small meshes and checks.
   * - ``Fused::Union``, ``Fused::Intersection``, ``Fused::Difference``, ``Fused::Complement``,
       ``Fused::Translate``, ``Rotate``, ``Scale``, ``Reflect``
     - Their operands' spans, combined (see :ref:`Sec:RaySpans`; `EBGeometry_CSGRayForms.hpp
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
     - Its ray program, clause by clause (see :ref:`a tape's ray program <Sec:TapeRays>`). Its ``Offset``
       and ``Annular`` of the shapes above are traced only in a ``Tape<T, TapeFeature::ExactOffsets>``
       (see :ref:`Sec:TapeFeatures`), and marched otherwise.

Every other shape is marched: the other analytic shapes, ``BVHSmoothUnion``, the other fused operations
and shapes of your own without a ray form. The ray forms of the analytic shapes and the mesh SDFs come
with their shapes' headers. Those of the fused operations, ``BVHUnion`` and the offsets come with
``EBGeometry_CSG.hpp`` (or ``EBGeometry.hpp``), not with ``EBGeometry_Fused.hpp`` alone: include it in
every translation unit that ray traces a composition, so that ``firstHit()`` on it is the same function
everywhere.

A ray form of your own
~~~~~~~~~~~~~~~~~~~~~~

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
filled in. The optional ``spans()`` writes where the ray is inside the shape; without it, an
intersection, difference or complement with the shape finds its spans by calling ``firstHit()`` from one
crossing to the next, which is slower. A static ``intersects()``, with ``firstHit()``'s arguments and
returning ``HitStatus``, may be added too, for a yes/no query that stops at the first crossing it finds.

The functions are found by their signatures, so a ``firstHit()`` with a mistyped argument still
compiles, and the shape is silently marched. Check it where the specialisation is written:

.. code-block:: c++

   static_assert(EBGeometry::hasRayForm<MyShape<double>, double>);
   static_assert(EBGeometry::hasRaySpans<MyShape<double>, double>); // if it has spans()

A ``spans()`` writes a `RaySpansT <doxygen/html/structEBGeometry_1_1RaySpansT.html>`__ (in
`EBGeometry_RaySpans.hpp <doxygen/html/EBGeometry__RaySpans_8hpp.html>`__): the spans of one object over
an interval, in order, each a pair of `SpanEndT <doxygen/html/structEBGeometry_1_1SpanEndT.html>`__ (a
distance, and whether it is a crossing, exact or marched, or an ``Open`` end where the interval ends rather
than the surface). ``addClipped()`` adds a span that may reach past the interval. ``a_firstOnly`` says that
only the first crossing is needed, and the list may then stop there. The list holds at most
``RaySpansT::Capacity`` (8) spans, and its ``m_horizon`` says how far it is complete.

Marching options
----------------

`RayOptions <doxygen/html/structEBGeometry_1_1RayOptions.html>`__, the last argument of every query, sets
the march's tolerance, length scale, step budget and fixed step, and ``m_march`` marches a shape even where
it has a ray form. Its members and defaults, and what each does to a march, are in :ref:`Sec:RayOptions`.

Walking the BVH yourself
------------------------

When the queries do not report what you need -- every face a ray crosses, say, or a condition of your
own to stop on -- walk a mesh's BVH with ``PackedBVH::traverse()`` and a ``BVH::RayQuery`` directly,
with your own state, leaf callback and bound. :ref:`Chap:ExampleRayTracing` does this beside the
queries, on the host and on a device.
