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

Rotating by an angle :math:`\theta` about an axis :math:`a` through the origin (one of the coordinate
axes, or any other direction) applies the inverse rotation to the query point:

.. math::

   I^\prime(\mathbf{x}) = I\left(R_a(-\theta)\,\mathbf{x}\right),

where :math:`R_a(\theta)` is the rotation matrix by angle :math:`\theta` about axis :math:`a`,
counterclockwise seen from the positive end of the axis. Rotating the query point by :math:`-\theta` is what makes the *shape* itself appear
rotated by :math:`+\theta`.

Scaling
~~~~~~~~

Uniform scaling by a non-zero factor :math:`s` shrinks the query point by :math:`s` before
evaluating the original function, and scales the result back up by :math:`|s|`:

.. math::

   I^\prime(\mathbf{x}) = |s| \, I\left(\mathbf{x}/s\right).

Rescaling the value by :math:`|s|` alongside the query point is what preserves the signed
distance property (see :ref:`Chap:GeometryRepresentations`) for a scaled *signed distance*
function -- omitting it would still shrink or grow the shape correctly, but the result would no
longer report true distances. A negative :math:`s` also reflects the shape through the origin;
the absolute value keeps the inside negative.

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

EBGeometry supports several further transformations beyond these five: offsetting (growing or
shrinking an object by a distance), shell extraction (hollowing out a solid into a shell of a given
thickness), elongation (stretching an object by inserting a box at its center), finite repetition
(copies of an object on a grid), and blurring and mollification (smoothing a sharp surface by locally
averaging the function value). They are shown in :ref:`Chap:ExampleTransforms`,
:ref:`Chap:ExampleRepetition` and :ref:`Chap:ExampleValueOperations`.

Combining objects
-------------------

EBGeometry supports the standard operations for combining implicit functions: union,
intersection, and difference. Smooth equivalents of each are also available, which smooth the
transition between the combined objects (controlled by a blending length) instead of leaving a
sharp crease where the objects meet. See :ref:`Chap:ExampleBasicCSG` and :ref:`Chap:ExampleSmoothCSG`.

Fast CSG operations are also supported by EBGeometry, e.g. the BVH-accelerated CSG union where one uses the BVH when searching for the relevant geometric primitive(s).
This functionality is motivated by the fact that a CSG union is normally implemented as :math:`\min\left(I_1, I_2, I_3, \ldots,I_N\right)`, which has :math:`\mathcal{O}\left(N\right)` complexity when there are :math:`N` objects.
BVH trees can reduce this to :math:`\mathcal{O}\left(\log N\right)` complexity, using the same
BVH traversal-and-pruning machinery described in :ref:`Chap:BVH` for mesh signed distances. Pruning
skips objects whose bounding boxes are farther away than the best value found so far, which is only
sound when the objects' values are distances or bounds on them, not when only their signs mean
anything. :ref:`Chap:ExampleManyObjects` compares the two.

Describing and evaluating a composed object
--------------------------------------------

A composed object is first *described*: an expression tree whose leaves are the objects and whose
nodes are the transformations and combinations above. The description is then *compiled* into a
flat program -- a tape -- that stores the objects in one array per kind of object and the operations
as a list of instructions referring to them. Evaluating the tape is a single loop over its
instructions, with no virtual dispatch and no pointers, which is what lets the same composed object
be evaluated on a CPU and, copied as a block of memory, on a GPU. While compiling, each node's
bounding box and how far its value can be trusted as a distance are worked out from its children's,
and a union of many objects gets a BVH over their boxes. :ref:`Chap:Tape` explains how an expression
becomes a tape, which operations compile their operand into a separate sub-program, and how the shape
of an expression (a union nested on the left or on the right, say) uses the tape's fixed limits.

A composed object whose structure is known when the program is written can instead be compiled
together with the program: each operation becomes a small type, and the composed object's type
spells out the whole tree. The compiler then evaluates it as it would the same object written out
by hand, with no instructions to interpret, which matters most on a GPU. Its values are the
tape's, computed by the same arithmetic. The structure is then fixed when the program is compiled,
so geometry chosen at run time still goes through the tape, which can hold such compiled objects as
its leaves. See :ref:`Chap:ExampleFusedCSG`.

The tape is the scene, and it can also say which of its leaves decides its value at a point: the
primitive whose surface the value comes from there. A union passes on the leaf of the operand it takes
the value from, a difference the leaf of whichever object bounds the result, a blend the more extreme
operand's, a blur the center of its samples'. This is what tagging a boundary or a material needs, and it is not the same as the
geometrically nearest primitive, which may be one the composition cut away. A compiled object is one
shape with one surface, however it is built: in a tape it is one leaf, so a scene of compiled parts
names the part that decides the value. :ref:`Chap:ClosestPrimitive` explains it, and
:ref:`Chap:ExampleMaterials` uses it to look up a material.
