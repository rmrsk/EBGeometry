.. _Chap:ConstructiveSolidGeometry:

Constructive solid geometry
=============================

Basic transformations
-----------------------

Implicit functions, and by extension also signed distance fields, can be manipulated using basic transformations (like rotations).
Every such transformation takes an implicit function :math:`I` and produces a new implicit
function :math:`I^\prime`, defined by evaluating :math:`I` at a suitably transformed query point.

.. warning::

   Some of these operations preserve the signed distance property, and others do not.

EBGeometry supports many such transformations, for example:

Translation
~~~~~~~~~~~~

Translating by a vector :math:`\mathbf{t}` shifts the query point back by :math:`\mathbf{t}`
before evaluating the original function:

.. math::

   I^\prime(\mathbf{x}) = I\left(\mathbf{x} - \mathbf{t}\right).

Rotation
~~~~~~~~~

Rotating by an angle :math:`\theta` about one of the coordinate axes :math:`a \in \{x, y, z\}`
applies the inverse rotation to the query point:

.. math::

   I^\prime(\mathbf{x}) = I\left(R_a(-\theta)\,\mathbf{x}\right),

where :math:`R_a(\theta)` is the standard rotation matrix by angle :math:`\theta` about axis
:math:`a`. Rotating the query point by :math:`-\theta` is what makes the *shape* itself appear
rotated by :math:`+\theta`.

Scaling
~~~~~~~~

Uniform scaling by a non-zero factor :math:`s` shrinks the query point by :math:`s` before
evaluating the original function, and scales the result back up by :math:`s`:

.. math::

   I^\prime(\mathbf{x}) = s \, I\left(\mathbf{x}/s\right).

Rescaling the value by :math:`s` alongside the query point is what preserves the signed
distance property (see :ref:`Chap:GeometryRepresentations`) for a scaled *signed distance*
function -- omitting it would still shrink or grow the shape correctly, but the result would no
longer report true distances.

Complement
~~~~~~~~~~~

The complement simply negates the function value, swapping the roles of "inside" and "outside":

.. math::

   I^\prime(\mathbf{x}) = -I(\mathbf{x}).

Reflection
~~~~~~~~~~~

Reflecting across one of the three coordinate planes (the :math:`yz`-, :math:`xz`-, or
:math:`xy`-plane) flips the sign of the one coordinate normal to that plane (:math:`x`,
:math:`y`, or :math:`z` respectively) before evaluating the original function. Writing
:math:`\mathbf{r}` for the vector that is :math:`+1` in the two unaffected coordinates and
:math:`-1` in the flipped one, and :math:`\odot` for the component-wise product:

.. math::

   I^\prime(\mathbf{x}) = I\left(\mathbf{r} \odot \mathbf{x}\right).

EBGeometry supports several further transformations beyond these five, including shell
extraction (hollowing out a solid into a shell of a given thickness) and mollification
(smoothing a sharp surface by locally averaging the function value), among others.

Combining objects
-------------------

EBGeometry supports the standard operations for combining implicit functions: union,
intersection, and difference. Smooth equivalents of each are also available, which smooth the
transition between the combined objects (controlled by a blending length) instead of leaving a
sharp crease where the objects meet.

Fast CSG operations are also supported by EBGeometry, e.g. the BVH-accelerated CSG union where one uses the BVH when searching for the relevant geometric primitive(s).
This functionality is motivated by the fact that a CSG union is normally implemented as :math:`\min\left(I_1, I_2, I_3, \ldots,I_N\right)`, which has :math:`\mathcal{O}\left(N\right)` complexity when there are :math:`N` objects.
BVH trees can reduce this to :math:`\mathcal{O}\left(\log N\right)` complexity, using the same
BVH traversal-and-pruning machinery described in :ref:`Chap:BVH` for mesh signed distances.
