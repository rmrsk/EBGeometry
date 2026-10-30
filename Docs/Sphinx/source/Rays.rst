.. _Chap:Rays:

Rays and segments
=================

Many uses of a geometry ask about a line rather than a point. A particle moving from one position to
the next must know whether its step crossed a wall, and where, so that it can stop or reflect there.
A renderer casts one ray per pixel and needs the first surface each ray meets. A line-of-sight test
only needs to know whether anything lies between two points.

The question
------------

All of these are one question: along the points :math:`\mathbf{x}(t) = \mathbf{o} + t\mathbf{d}` of a
ray, with :math:`\mathbf{d}` of unit length so that :math:`t` is a distance, where in an interval
:math:`[t_\mathrm{min}, t_\mathrm{max}]` does the ray first cross the surface? A particle's step from
:math:`\mathbf{a}` to :math:`\mathbf{b}` is the ray from :math:`\mathbf{a}` towards :math:`\mathbf{b}`
over :math:`[0, |\mathbf{b} - \mathbf{a}|]`, so that :math:`t` is the distance from :math:`\mathbf{a}`.

EBGeometry asks it in two ways, both on any shape and both callable on a device (see
:ref:`Chap:ImplemRays`):

* ``firstHit()`` finds the *first* crossing in the interval, whichever way the ray goes through it: a
  ray that starts inside the object reports where it leaves.
* ``intersects()`` asks only *whether* the ray crosses, and where the shape allows it stops at the first
  crossing it comes across instead of searching for the nearest.

What a crossing reports
-----------------------

A query ends in one of three ways: the ray crosses the surface (a *hit*), it does not (a *miss*), or the
query could not decide (*unresolved*; see :ref:`Chap:SphereTracing`). An unresolved query is never
reported as a miss: for a particle, a miss would let it through the wall, so the caller decides what an
undecided ray means.

A hit reports:

* the crossing's distance :math:`t` along the ray, and its point;
* the outward unit normal there;
* whether the ray *enters* the object there or leaves it;
* which part it crossed, where the shape can name one: the face of a mesh (its index in the file), or the
  primitive of a tape that decides the surface there;
* whether the crossing was found *exactly* or by marching (below).

The interval
------------

A crossing at :math:`t_\mathrm{min}` counts, :math:`t_\mathrm{min} = 0` included. A particle that has
stopped on a wall and moves on therefore starts its next search a little past the wall, at a small
:math:`t_\mathrm{min}`; otherwise it would find the wall it is sitting on. A segment of zero length
crosses nothing.

Two ways to find a crossing
---------------------------

**Exactly.** Some geometry can be intersected with a line directly. A plane gives one linear equation in
:math:`t`, a sphere a quadratic, a box the overlap of three slabs; a mesh is crossed face by face, with
its BVH keeping that to the faces near the ray. A composition of such shapes (a union, an intersection, a
difference, a complement, a moved, turned or scaled shape, and the offsets and shells of the simplest
shapes and meshes) is traced from where the ray is inside each of its parts. The answer is exact, up to
rounding. :ref:`Chap:RayTracing` describes which shapes and compositions are traced this way, and how.

**By marching.** Any shape with a signed value can be marched instead: walk along the ray, evaluating
the value, until it says the surface is reached. Where the value is a distance, or a lower bound on one,
a step of the value's size cannot step over the surface; this is *sphere tracing*. A march finds a
crossing to within a tolerance, needs a budget of steps, and costs many evaluations of the value per ray.
:ref:`Chap:SphereTracing` describes how a march steps, stops and fails, and what can be set.

The choice is made per shape, and per part of a composition, from its type (for a tape, when the tape is
built): a shape that can be traced exactly is, and everything else is marched. A composition with a part that cannot be traced
exactly marches only that part (see :ref:`Sec:RayMarchedParts`). A query can also be asked to march any
shape, exact or not: a reference to compare the exact answer with.
