.. _Chap:RayTracing:

Exact ray tracing
=================

A shape is ray traced *exactly* when its first crossing with a ray can be computed directly, rather than
approached step by step as a march does (see :ref:`Chap:SphereTracing`). The answer is then exact up to
rounding, its normal is the surface's own, and the cost does not depend on how close the ray passes to the
surface. This page describes which shapes are traced exactly and how; :ref:`Chap:ImplemRays` lists the
classes, and how to give a shape of your own an exact method.

Shapes in closed form
---------------------

The simplest shapes are intersected with a line in closed form. A plane gives one linear equation in
:math:`t`; a sphere, or an infinite cylinder, a quadratic; a box the overlap of three slabs, one per axis.
A finite cylinder is an infinite cylinder cut by the slab between its caps, and a capsule the hull of a
cylinder and the two balls at its ends. Each of these is convex, so a ray is inside it over a single
stretch, from where it enters to where it leaves, and its first crossing is the start of that stretch, or
its end for a ray that starts inside. The round surfaces are solved in a form that does not lose precision
to cancellation, so that a small ball or a thin cylinder far from the ray's start is hit or missed by its
geometry, not by rounding, in single precision too.

The other analytic shapes (a torus, the cones, a rounded box or cylinder, Perlin noise) are marched.

Meshes
------

A mesh is crossed face by face: the ray meets each face's plane at one point, which either lies inside the
face (a triangle, or a planar convex polygon) or does not. Testing every face would cost as much as the mesh
is large, so the mesh's BVH (see :ref:`Chap:BVH`) keeps the test to the faces near the ray. Each box of the
BVH is keyed by the distance along the ray at which the ray enters it; the walk goes to the nearest box
first, and skips every box the ray enters beyond the nearest crossing found so far. A ray that crosses
nothing touches only the boxes along its path. ``intersects()`` stops at the first crossing it finds,
nearest or not.

At an edge or a vertex shared by several faces, rounding could let a ray slip between them. The face test
is therefore widened by its own rounding error: a ray through a shared edge or vertex crosses at least one
of the faces there, however far it comes from, and a ray passing within rounding error outside a face
counts as crossing it (the safe side, as for a marched ray that grazes a surface). A ray lying in a face's
plane does not cross that face.

A hit names the face crossed by its index in the file, and its normal is that face's normal. A mesh with
holes is traced as it is: the ray crosses whatever faces it meets, and only a closed mesh has an inside
(see `Compositions`_).

.. _Sec:OffsetRays:

Offsets and annuli
------------------

An offset by :math:`r` keeps where an object's value is at most :math:`r`, and an annulus of
half-thickness :math:`d` where it is between :math:`-d` and :math:`d` (see :ref:`Chap:ImplemCSG`): both
are *level sets* of the object's value. Where the object's level sets have a simple form of their own, a
ray finds them exactly, with exact normals:

* a sphere's, a capsule's or an infinite cylinder's radius grows or shrinks by the level;
* a plane moves by it;
* a box shrinks into a smaller box, or grows into a box with rounded edges and corners;
* a mesh's surface grown by :math:`d` is the union of its faces, each grown into a thin slab with a
  rounded rim.

A translation, rotation, reflection or scale between the offset and the object passes it on. The shell
around a mesh in :ref:`Chap:ExampleShell` is traced this way, entering and leaving its wall exactly.

.. figure:: /_static/OffsetRays.png
   :width: 70%
   :align: center

   Offsets traced from level sets, drawn in a plane through the ray. (a) A box grown by :math:`r` is a
   box with rounded edges and corners; the ray meets the rounded corner exactly, with the corner's normal.
   (b) An annulus of a mesh is the union of its faces, each grown into a slab with a rounded rim; the
   ray's stretches inside the shell are exact.

A fused composition always traces such an offset or annulus exactly. A tape does when its type asks for it,
by listing ``TapeFeature::ExactOffsets`` (see :ref:`Sec:TapeFeatures`), and otherwise marches it. An offset
or annulus of anything else is marched: of a composition, of a finite cylinder (whose grown rim is a
torus), of a torus, or of another offset or annulus.

Tracing a mesh's grown surface along a whole ray costs much more than finding its first crossing. Under an
intersection or a difference, an offset or annulus of a mesh is therefore traced only where the other
operands leave something to find, as a marched part is (see :ref:`Sec:RayMarchedParts`).

.. _Sec:RaySpans:

Compositions
------------

A composition of shapes (a union, an intersection, a difference, a complement, a translated, rotated,
reflected or scaled shape) is traced from its parts. Along the ray, each part is inside over a few
*spans*, stretches from where the ray enters it to where it leaves. The composition's spans follow from
its parts' as sets of points do:

* a union's are where the ray is inside either part;
* an intersection's, where it is inside both;
* a complement's, the gaps between the part's spans;
* a difference is an intersection with a complement;
* a translated, rotated, reflected or scaled shape is traced along the ray mapped into its own frame, and
  its spans are mapped back.

The first crossing of the composition is where the first of its spans begins or ends.

.. figure:: /_static/RaySpans.png
   :width: 70%
   :align: center

   A box with a ball cut out of it, along a ray. The box is inside over :math:`[2, 8]` and the ball over
   :math:`[4, 6]`; outside the ball is the rest of the interval, whose ends are no crossings. The
   difference is inside where the ray is inside the box and outside the ball, over :math:`[2, 4]` and
   :math:`[6, 8]`, and its first crossing is at 2.

In the figure, the ball's own first crossing, at 4, is not where the difference is first crossed: there
the ray leaves the solid. Under an intersection or a difference every span of a part is needed, not only
its first crossing. Under a union only the first crossings are: the nearest of the parts' first crossings
is the union's, unless the ray starts inside a part, which the spans also show. A union of many shapes in
a BVH (see :ref:`Sec:BVHUnions`) walks its BVH along the ray, nearest box first, as a mesh does, and stops
at the first box beyond the nearest crossing.

A list of spans has a fixed size, so that it fits a device thread. A ray through more pieces than it holds
keeps the first ones, and the list then says how far along the ray it is complete. If the first crossing
is not found before that point, the trace starts again from there: the answer is never wrong, only slower
for a ray through very many pieces. A trace that still has not decided after many passes is unresolved,
as a march out of steps is.

A mesh under an intersection or a complement must be closed, since its spans are the stretches between
the ray entering and leaving it, and only a closed surface has an inside to enter.

.. _Sec:RayMarchedParts:

Marched parts of an exact composition
-------------------------------------

A part that has no exact method of its own is marched, by itself and in its own bounding box, while the
rest of the composition stays exact. Such parts are a smooth union or other blend, an offset or annulus
of anything but the shapes above, an elongation, a repetition, a blur, and a shape of your own without an
exact method, together with every part inside them: a blend of two balls is marched as a whole, even
though each ball could be traced.

How far a marched part is marched depends on where it sits:

* under a union, only to its first crossing;
* under an intersection or a difference, from one crossing to the next, and only within the spans of the
  composition's exact parts, which are traced first. Elsewhere the result is empty whatever the marched
  part does. Where only the first crossing is needed, the march stops at the first span it finds there.

In the box with a ball cut out, with the ball marched, the ball would be marched only within the box's
span, past the box's crossing at 2. This holds for a marched part that is a direct operand of the
intersection or difference (or the complement of one). A marched part deeper inside an operand, such as a
union of a ball and a blend, is traced in the order written; where you can, make the marched shape a direct
operand of the intersection.

A tape traces the same way, from a program the tape compiler writes beside its value program (see
:ref:`a tape's ray program <Sec:TapeRays>`). It holds a fixed number of span lists at once, so it also marches a subtree that
would need more of them, which only a deeply nested composition does.

.. _Sec:RayTracedHits:

What an exact hit reports
-------------------------

A hit on a composition is the crossing of one of its parts, and reports that part's surface:

* **Exactness.** A hit is exact when its crossing lies on a part traced exactly, and not when it lies on a
  marched part. A composition with a marched part can therefore report exact hits for some rays and marched
  ones for others.
* **Normal.** The normal is the crossed part's own, mapped through the rotations, reflections and scalings
  above it, and turned to point out of the composition: the normal of a ball's surface where it bounds a
  hole in a box points into the ball.
* **Entering or leaving.** Whether the ray enters or leaves the composition, which need not be what it does
  to the part: in the figure, the ray enters the difference at 6, where it leaves the ball.
* **Part.** A mesh names its face; a tape the primitive it crossed, or for a marched subtree the primitive
  deciding the value at the crossing (see :ref:`Sec:DecidingLeaf`); a BVH union of meshes the face of the
  mesh crossed; and any other shape 0. A fused composition is one shape, and names no part.
