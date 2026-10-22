.. _Chap:Rendering:

Rays and rendering
==================

A signed distance function can be looked at directly, without first turning it into a mesh: follow a
ray from the eye through each pixel to where it first meets the surface, and shade that point. The same
question -- where does a ray, or a segment, first cross the surface? -- is also useful on its own, for
visibility, for collisions, or for where a grid line cuts the geometry. EBGeometry answers it for every
shape, and builds a renderer on top.

.. contents:: On this page
   :local:
   :depth: 1

Sphere tracing
--------------

The signed distance :math:`\phi(\mathbf{x})` says how far the nearest surface is from
:math:`\mathbf{x}`, but not in which direction. Along a ray :math:`\mathbf{x}(t) = \mathbf{o} + t\,\mathbf{d}`
with a unit direction :math:`\mathbf{d}`, no surface can lie closer than :math:`|\phi(\mathbf{x}(t))|`,
so the ray can safely step that far:

.. math::

   t_{k+1} = t_k + \left|\phi\left(\mathbf{x}(t_k)\right)\right|.

The steps shrink as the ray approaches the surface, and the march stops where :math:`|\phi|` falls
below a tolerance :cite:`Hart1996`. This is *sphere tracing*: each step moves to the edge of a ball that
contains no surface. It works as well with a lower bound on the distance (a value that is never larger
than the true distance, which many CSG operations give) as with the exact distance, since a smaller
step is still safe. It never steps over a surface; what it can do is run out of steps, along a ray that
grazes a surface and creeps along it. A query that runs out of steps says so (it is *unresolved*)
rather than report a miss, so a caller never mistakes "could not decide" for "nothing there".

A value that is not a distance at all (noise, say) gives no safe step. There the march steps a fixed
length and, where the value changes sign, halves the interval until it is shorter than the tolerance.
Features thinner than the step can then be stepped over.

Only the part of the ray inside the shape's bounding box can meet the surface, so the march starts
where the ray enters the box and stops where it leaves it.

Entering and leaving
--------------------

The first crossing along a ray may be where it enters the solid or where it leaves it: a ray that starts
inside the object (negative :math:`\phi`) first meets the surface on its way out. EBGeometry reports the
first crossing either way, and says which it is. This matters for what a picture shows. A hole (the
complement of a ball, say) is solid everywhere but in the ball, so a camera outside the ball sits inside
the solid, and what it sees first is where its rays leave the solid on the ball's near side. A mesh
whose faces are wound the wrong way is, to the signed distance, inside out, and looks like that too.

The renderer therefore lights a surface seen from inside from the eye's side and tints it red, and
reports how many such hits there were and whether the eye itself is in the solid. An inside-out object
cannot then pass for an outside.

Section planes
--------------

To look inside an object, the renderer can cut it with a plane: it marches
:math:`\max\left(\phi(\mathbf{x}),\, (\mathbf{x} - \mathbf{p})\cdot\mathbf{n}\right)`, which removes
everything on the side the plane's normal :math:`\mathbf{n}` points to. The maximum of two bounds is a
bound, so the cut object is sphere traced like any other. Where the plane gives the value, the surface
is a cut face, and it gets a colour of its own.

What a pixel shows
------------------

For a pixel, the renderer casts one ray (or :math:`n \times n` rays, averaged, with the light moved a
little for each, as a light of some size would be) and, where it meets the surface, combines diffuse and
specular light from one direction, light from the sky, a shadow and ambient occlusion. A floor under the
shape catches its shadow. Surfaces can be coloured uniformly or by primitive: by the leaf of a composed
expression the surface belongs to, or by the face of a mesh.

The shadow comes from one ray towards the light, softened at the edges by the light moving a little
between a pixel's rays, and the occlusion from four short rays tilted around the normal: the share of
them that hit the shape darkens the point.

Ray tracing
-----------

Sphere tracing needs a few to a few hundred distance evaluations per ray: fast enough for a handful of
analytic shapes, slower through a composed expression, and slow for a mesh, where every evaluation is a
closest-face search. Many shapes can instead answer a ray directly, without a distance at all.

* **Analytic shapes in closed form.** Where a ray enters and leaves a plane, a sphere, a box, a
  cylinder or a capsule is the root of a quadratic or a slab test.
* **A triangle mesh under a BVH.** Follow the ray through the BVH, nearest box first, test the triangles
  in each leaf it reaches, and stop at boxes that begin beyond the nearest crossing found: one tree
  traversal per ray instead of dozens of closest-face searches, and the normal there from the
  triangle's vertex normals.

EBGeometry ray traces a shape this way whenever the shape has a *ray form*, and sphere traces it
otherwise.

Composed objects are ray traced node by node. Along a ray, every node of the expression is described by
its *spans*, the intervals of the ray inside it. A primitive with a ray form gives its spans from its
crossings; a union of spans is their union, an intersection their intersection, a complement the gaps
between them; an affine map (translation, rotation, scaling, reflection) moves the ray into the
operand's frame, where the ray's parameter is unchanged, so the operand's spans are the node's. A union
of many objects tests only the objects whose boxes the ray meets. A node with no ray form -- a smooth
blend, an offset, a blur, an elongation, a repetition, a shape that cannot answer a ray -- is sphere
traced along the ray inside its box, and its spans join the others. The object's first crossing is
then the first end of its spans: exact, unless a sphere-traced node was crossed.

:math:`\max` and :math:`\min` of distances become intersection and union of spans, so sharp CSG keeps
its sharp edges exactly; a sphere tracer stops within its tolerance of an edge, and near-coincident
surfaces can then make it report a sliver thinner than that tolerance.

:ref:`Chap:ImplemRender` describes the classes, and :ref:`Chap:ExampleMeshSDF` opens the viewer on a mesh.
