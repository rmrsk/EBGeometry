.. _Chap:SphereTracing:

Sphere tracing
==============

Any shape with a signed value can be ray traced without knowing anything about it but that value and its
bounding box: walk along the ray, evaluating the value, until it says the surface is reached. This is
*marching*, and where the value is a distance, *sphere tracing*. It is what a ray query does for every shape
that cannot be traced exactly (see :ref:`Chap:RayTracing`), for the parts of a composition that cannot (see
:ref:`Sec:RayMarchedParts`), and for any shape when the query is asked to march.

Stepping by the value
---------------------

A march first cuts the ray's interval to the shape's bounding box, grown by the tolerance (below): outside
the box there is no surface to find. It then steps along the ray from the start of what is left.

Where the value is a signed distance, its magnitude at a point is the distance to the nearest point of the
surface, in any direction. A ball of that radius around the point holds no surface, so the ray can move
forward by the value without stepping over the surface. Far from the surface the steps are long; as the ray
nears the surface they shrink, and the march stops where the value falls below the tolerance. The same
holds inside the object, stepping by the value's magnitude towards where the ray leaves.

.. figure:: /_static/SphereTracing.png
   :width: 70%
   :align: center

   Marching a ray. (a) Each step is the value at the point the ray has reached, the radius of a ball that
   holds no surface; the steps shrink near the surface, and the march stops inside the tolerance band. (b)
   A ray that passes the surface within the tolerance, without crossing it, also stops in the band, and is
   reported as a hit. (c) A value that is not a distance gives no safe step: the march takes fixed steps,
   and bisects the step in which the sign changes. A feature thinner than the step can be missed.

The march ends in one of three ways:

* the value falls below the tolerance: a **hit**, at that point. The point lies within the tolerance of the
  surface, and is not refined further;
* the value changes sign between two steps (which rounding, or a value that overestimates, can cause): the
  step is **bisected** until the bracket around the crossing is shorter than the tolerance, and that is the
  hit;
* the ray passes :math:`t_\mathrm{max}`, or the end of the box: a **miss**.

The normal at a marched hit is the direction in which the value grows fastest (its gradient), estimated from
the value a tolerance's length on either side of the point along each axis. Whether the ray enters or leaves
there is read from the sign of the value where the march started.

Distance quality
----------------

A value can promise more or less about the distance to the surface (see :ref:`Chap:ImplemCSG`), and the
march takes its step from what the value promises:

.. list-table::
   :header-rows: 1
   :widths: 20 40 40

   * - Distance quality
     - The value's magnitude is
     - The march
   * - ``Exact``
     - the distance to the surface
     - steps by the value
   * - ``Bound``
     - at most the distance to the surface
     - steps by the value: still safe, but the steps are shorter where the value underestimates, so it
       takes more of them
   * - ``NotADistance``
     - nothing; only the sign means something
     - takes fixed steps, and bisects the step in which the sign changes

A fixed-step march is slower than sphere tracing, and less safe: a part of the object thinner than the step
can lie between two samples of the same sign and is then missed, reported as a miss, not as unresolved.
Make the step smaller to find thinner features. A composition's quality is worked out when it is built: a
smooth union is a bound, for instance, and anything with Perlin noise in it is not a distance.

The tolerance and the scale
---------------------------

The tolerance and the fixed step are given relative to the shape's *scale*, so that the defaults suit a
shape of any size: by default the diagonal of its bounding box. A shape whose box is unbounded (a plane, a
complement) has no diagonal; give it a scale, or the scale is 1. A marched part of a composition takes the
scale of its own box.

The tolerance sets how close to the surface a hit is, the length over which the normal is estimated, and
how near the surface a ray may pass and still count as crossing it.

Grazing rays are hits
---------------------

A ray that comes within the tolerance of the surface without crossing it, passing an edge or a bump, stops
there like a ray that crosses (panel (b) of the figure), and is reported as a hit. For a particle this is the
safe side: it never slips through an edge. Whether such a ray is a hit is decided by rounding, so the host
and a device can answer it differently. This applies where the value is a distance or a bound; a fixed-step
march reports only sign changes.

Starting on the surface
-----------------------

A march that starts within the tolerance of the surface reports a hit at its start, entering or leaving as
the value a little further along says. A particle sitting on a
marched wall must therefore start its next search further from the wall than the tolerance (times the
scale), not merely a little past it, or it finds the wall it is sitting on again.

The step budget, and unresolved rays
------------------------------------

A march has a budget of steps. A ray that runs along the surface without reaching it, close but not within
the tolerance, takes ever shorter steps and may run out of them without deciding. The query then reports
*unresolved*, with the point it reached, rather than a miss. What to do with it is the caller's choice: march
again with a larger budget or a larger tolerance, or treat it as a hit, which for a particle is the safe
side. A fixed-step march with the default step, budget and scale covers more than twice the box's diagonal,
so it reaches the end of a bounded box before its budget runs out. A shape whose box is unbounded, marched to an
infinite :math:`t_\mathrm{max}`, never reaches the end of the interval: a ray that misses it runs out of steps
and is unresolved, so give such a query a finite :math:`t_\mathrm{max}`.

.. _Sec:RayOptions:

What you can set
----------------

`RayOptions <doxygen/html/structEBGeometry_1_1RayOptions.html>`__, the last argument of every ray query,
sets how a march runs:

.. list-table::
   :header-rows: 1
   :widths: 20 15 65

   * - Member
     - Default
     - Meaning
   * - ``m_tolerance``
     - ``1e-4``
     - A march stops where the value's magnitude is below this, times the scale.
   * - ``m_scale``
     - ``0``
     - The shape's length scale; 0 takes the diagonal of its bounding box, or 1 where the box is unbounded.
   * - ``m_maxSteps``
     - ``512``
     - Steps a march may take before it is unresolved.
   * - ``m_step``
     - ``0.005``
     - The fixed step, times the scale, where the value is not a distance.
   * - ``m_march``
     - ``false``
     - March the shape even where it can be traced exactly.

Shapes traced exactly ignore every option but ``m_march``; a composition passes the options on to the parts
it marches, and uses the tolerance to look up the normal of the part its crossing lies on.

When a shape is marched
-----------------------

A query marches:

* a shape with no exact method: the analytic shapes other than the plane, sphere, box, cylinders and
  capsule; a smooth union of many shapes in a BVH; the blends, elongations, repetitions, blurs and the
  offsets that cannot be traced exactly (see :ref:`Sec:OffsetRays`); and a shape of your own without one;
* those parts of an exact composition, each by itself, in its own box (see :ref:`Sec:RayMarchedParts`);
* any shape, when ``m_march`` is set: a reference for an exact answer, or a way round an exact method you
  suspect.

What it costs
-------------

Each step of a march evaluates the shape's whole value at a point: for a mesh, a nearest-face search in its
BVH; for a tape, its program. A ray takes many steps, more the closer it passes to the surface, and a
grazing ray the most. An exact trace instead visits each part, or each box of a mesh's BVH along the ray,
once, and its cost does not depend on how close the ray passes to the surface. Tracing exactly is therefore
much faster wherever it applies, and a composition is fastest when its marched parts are few, small and
direct operands of its intersections. To see the difference for your own shape, time a query with and
without ``m_march``, or run the ray probes in ``Performance/`` (see :ref:`Chap:PerformanceTesting`).
