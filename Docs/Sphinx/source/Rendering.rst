.. _Chap:Rendering:

Rendering
=========

A signed distance function can be looked at directly, without first turning it into a mesh: follow a
ray from the eye through each pixel to where it first meets the surface, and shade that point. That is a
ray query (see :ref:`Chap:Rays`): exact for the shapes and compositions that have one, sphere traced for
the rest, so the renderer works on every shape and is as exact as its ray queries are.

.. contents:: On this page
   :local:
   :depth: 1

Entering and leaving
--------------------

The first crossing along a ray may be where it enters the solid or where it leaves it: a ray that starts
inside the object (negative :math:`\phi`) first meets the surface on its way out. A ray query reports
the first crossing either way, and says which it is. This matters for what a picture shows. A hole (the
complement of a ball, say) is solid everywhere but in the ball, so a camera outside the ball sits inside
the solid, and what it sees first is where its rays leave the solid on the ball's near side. A mesh
whose faces are wound the wrong way is, to the signed distance, inside out, and looks like that too.

The renderer therefore lights a surface seen from inside from the eye's side and tints it red, and
reports how many such hits there were and whether the eye itself is in the solid. An inside-out object
cannot then pass for an outside.

Section planes
--------------

To look inside an object, the renderer can cut it with a plane, removing everything on the side the
plane's normal :math:`\mathbf{n}` points to: the intersection of the object with a half-space. Along
each ray it finds the part of the ray on the kept side, and traces the object over that part only. Where
the ray reaches the plane while inside the object, the plane itself is the first crossing: a cut face,
exact, with the plane's normal, and a colour of its own.

What a pixel shows
------------------

For a pixel, the renderer casts one ray (or :math:`n \times n` rays, averaged, with the light moved a
little for each, as a light of some size would be) and, where it meets the surface, combines diffuse and
specular light from one direction, light from the sky, a shadow and ambient occlusion. A floor under the
shape catches its shadow. Surfaces can be coloured uniformly, by primitive (by the leaf of a composed
expression the surface belongs to, or by the face of a mesh, with colours chosen per primitive or from a
palette), or by the direction of the surface's normal.

The camera looks either in perspective, every ray leaving the eye, or orthographically, the rays parallel
and leaving a plane through the eye, so that sizes do not shrink with distance; a near clip starts every
ray some way from the eye, so that whatever is nearer is not seen.

The shadow comes from one ray towards the light, softened at the edges by the light moving a little
between a pixel's rays, and the occlusion from four short rays tilted around the normal: the share of
them that hit the shape darkens the point. All of them are ray queries on the shape, like the ray from
the eye.

:ref:`Chap:ImplemRender` describes the classes, and :ref:`Chap:ExampleMeshSDF` opens the viewer on a mesh.
