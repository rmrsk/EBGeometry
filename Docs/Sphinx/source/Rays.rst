.. _Chap:Rays:

Rays and segments
=================

Many uses of a geometry ask about a line rather than a point. A particle moving from one position to
the next must know whether its step crossed a wall, and where, so that it can stop or reflect there.
A renderer casts one ray per pixel and needs the first surface each ray meets. A line-of-sight test
only needs to know whether anything lies between two points.

All of these are one question: along the points :math:`\mathbf{x}(t) = \mathbf{o} + t\mathbf{d}` of a
ray, with :math:`\mathbf{d}` of unit length so that :math:`t` is a distance, where in an interval
:math:`[t_\mathrm{min}, t_\mathrm{max}]` does the ray first cross the surface? A particle's step from
:math:`\mathbf{a}` to :math:`\mathbf{b}` is the ray from :math:`\mathbf{a}` towards :math:`\mathbf{b}`
over :math:`[0, |\mathbf{b} - \mathbf{a}|]`. The answer is the crossing's distance and point, the
outward normal there, whether the ray enters the object or leaves it, and, where the geometry has
parts, which part it crossed (a mesh's face, say). A cheaper variant asks only *whether* the ray
crosses, and can stop at the first crossing it comes across instead of searching for the nearest.

A crossing at :math:`t = 0` counts. A particle that has stopped on a wall and moves on therefore
starts its next search a little past the wall, at a small :math:`t_\mathrm{min}`; otherwise it would
find the wall it is sitting on.

Two ways to find a crossing
---------------------------

**Exactly.** Some geometry can be intersected with a line in closed form. A plane gives one linear
equation in :math:`t`, a sphere or an infinite cylinder a quadratic, and a box the overlap of three
slabs. A mesh is crossed face by face, solving for the point where the ray meets each face's plane and
checking that it lies inside the face (a triangle, or a planar convex polygon); its BVH keeps that to
the faces near the ray (see :ref:`Chap:BVH`). Each node's box is keyed by the distance along the ray at which
the ray enters it, so the walk goes nearest box first and skips every box that the ray enters beyond
the nearest crossing found so far. The answer is exact, up to rounding.

**By marching.** Any geometry with a signed distance can be marched instead: evaluate the value along
the ray and look for the point where its sign changes. Where the value is a distance, or a lower bound
on one, a step of the value's size cannot step over the surface, since no surface is nearer than that.
This is *sphere tracing*: the steps are long far from the surface and shrink as the ray approaches it,
and the march stops where the value falls below a tolerance. Where the value is not a distance at all
(see :ref:`Chap:ConstructiveSolidGeometry` for which compositions keep a distance), the march takes fixed steps and, at a
sign change, bisects to the crossing. Its normal is the gradient of the value there. Marching finds a
crossing to within its tolerance, and it can miss a feature thinner than its step.

A ray that passes within the tolerance of the surface without crossing it, grazing an edge, is
reported as a crossing: for a particle, the safe side.

A march also needs a budget: a ray that grazes the surface takes ever shorter steps, and may run out
of steps without deciding. It then says so, rather than reporting a miss: for a particle, a miss lets
it through the wall.

Compositions
------------

A composition of shapes (a union, an intersection, a difference, a complement, a translated, rotated
or scaled shape) is traced from its parts. Along the ray, each part is inside over a few *spans*,
stretches from where the ray enters it to where it leaves. The composition's spans follow from its
parts' as sets of points do: a union's are where the ray is inside either part, an intersection's
where it is inside both, a complement's the gaps between them, and a difference is an intersection with
a complement. The first crossing is where the first of the composition's spans begins or ends. A
translated, rotated or scaled shape is traced along the ray mapped into its own frame, and its spans
mapped back.

Take a box with a ball cut out of its middle, and a ray through both: the box's span is
:math:`[2, 8]`, the ball's :math:`[4, 6]`, so the difference has the two spans :math:`[2, 4]` and
:math:`[6, 8]`, and the first crossing is at 2. The ball's own first crossing, at 4, is not a crossing of
the difference: there the ray leaves the solid. Under an intersection or a difference every span of a
part is needed, not only its first crossing. Under a union only the first crossings are: the nearest of
the parts' first crossings is the union's, unless the ray starts inside a part, which the spans also
show.

A part that has no exact method of its own (a blend, an offset, an elongation, a repetition), and every
part inside it, is marched by itself, in its own bounding box: under a union only to its first
crossing, under an intersection or a difference from one crossing to the next. An intersection or a
difference traces its exact parts first and marches such a part only within their spans, since
elsewhere the result is empty whatever the marched part does, and, where only the first crossing is
needed, only up to the first span it finds there. In the box with a ball cut out, with the ball marched,
the ball is marched only past the box's crossing at 2. Everything else stays exact.

A list of spans has a fixed size, so that it fits a device thread. A ray through more pieces than it
holds keeps the first ones, and the list then says how far along the ray it is complete. If the first
crossing is not found before that point, the trace starts again from there: the answer is never wrong,
only slower for a ray through very many pieces.

A query can also be asked to march any shape, composition or not: a reference to compare the exact
answer with.
