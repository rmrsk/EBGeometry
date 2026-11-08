.. _Chap:ClosestPrimitive:

Which part: the deciding primitive
==================================

A composed object answers one question at a point, its signed distance, but an application often needs
a second: which part of the object is that? A boundary condition depends on whether a wall is the inlet,
the outlet or the casing; a material on whether a surface is the machined bore or the cast body; a
picture or a label on which object a pixel shows. A tape answers this with ``closestLeaf(p)``. It
names the leaf of the composition whose surface the value at ``p`` comes from: the *deciding primitive*.

The deciding primitive, not the nearest one
-------------------------------------------

A composition takes its value from its operands' values: a union takes the smallest, an intersection the
largest, a difference the largest of the first operand's value and the negated values of the others.
Whichever operand the value is taken from is the one that decides it, and following that choice down
the composition ends at a leaf. That leaf is the one ``closestLeaf(p)`` names.

This is not the same as the primitive whose surface is geometrically nearest to ``p``. A composition
removes parts of its primitives' surfaces, and the nearest primitive surface may be one of those removed
parts, which is no longer part of the object. :numref:`Fig:ClosestPrimitive` shows a cross-section of
``Difference(box, sphere)``, a box with a ball carved out of one side. The plane is colored by the leaf
that decides the value: the sphere near the cavity, on both sides of its wall, and the box everywhere
else.

.. _Fig:ClosestPrimitive:
.. figure:: /_static/ClosestPrimitive.png
   :width: 70%
   :align: center

   ``Difference(box, sphere)`` in a plane through the box's and the ball's centers, colored by the
   deciding leaf (the box is leaf 0, the sphere leaf 1). The dashed lines are the parts of the two
   primitives' surfaces that the difference removed. At :math:`p_1`, outside the object beside the
   ball's removed cap, the nearest primitive surface is the sphere's, but the value comes from the box,
   and the box decides. At :math:`p_2`, just outside the box's removed face, it is the other way around.

The point :math:`p_1` is the typical case: the cap of the ball that sticks out of the box is not part of
the object, so the object's surface near :math:`p_1` is the box's, and an application tagging that
surface wants the box. The deciding leaf gives it; the nearest primitive would not.

Each operation passes on a leaf as follows:

* A **union** takes the leaf of the operand it takes the value from (the smallest), an **intersection**
  the leaf of the largest. A **difference** is an intersection with the complements of the subtracted
  objects.
* A **smooth union, intersection or difference** blends the two most extreme operands' values and takes
  the leaf of the more extreme one.
* A **blur** takes the leaf of its center sample, a **mollification** that of its sample with the
  largest weight.
* A **complement, offset, annulus, translation, rotation, scaling, reflection, elongation or repetition**
  passes on its operand's leaf. Every copy of a repeated object is the same leaf; the copy whose value
  is taken decides which local point is reported (see below).
* A **nested composite is one leaf**: a fused composition (see :ref:`Sec:Fused`), which is one shape
  with one surface however it is built, a tape listed as a primitive of another tape, and a BVH union of
  many objects. A scene is therefore a tape whose leaves are its parts: it names the part that decides
  the value, and each part can be as large a composition as it likes.

Asking for it
-------------

``closestLeaf(p)`` is a member of the
`Tape <doxygen/html/classEBGeometry_1_1Tape.html>`__ and returns a
`LeafRef <doxygen/html/structEBGeometry_1_1LeafRef.html>`__:

* ``m_leaf``: the leaf's number. The leaves are numbered in the order they appear in the expression,
  depth first, counting each occurrence, so a shape used twice is two leaves. In
  ``Difference(Intersection(cube, ball), Union(drills))`` the cube is leaf 0, the ball leaf 1 and the
  three drills leaves 2 to 4. The number is the one to index a table of your own with: materials,
  boundary conditions, colors. It is ``LeafRef::None`` when the query names no leaf, so check it
  before indexing.
* ``m_type`` and ``m_index``: which of the tape's primitive types the leaf is, and which primitive of that
  type. They are what the accessors below use; most programs need not read them directly.
* ``m_localPoint``: the point in the primitive's own frame, after the translations, rotations, scalings,
  reflections, elongations and repetitions above the leaf. This is where the primitive itself can be
  queried.

The tape also returns the primitive the reference names:

* ``isPrimitive<P>(ref)`` says whether the leaf is a ``P``. ``P`` must be one of the tape's primitive
  types; any other type is a compile error.
* ``primitive<P>(ref)`` returns it by value: the shape itself, or for a primitive that lives in a pool (a
  mesh distance field, a BVH union, a nested tape) a handle that resolves against the same pool as the
  tape, or the same mirror of it on a device.
* ``visitPrimitive(ref, f)`` calls ``f`` with the primitive whatever its type, as ``primitive<P>()``
  returns it. In host code a generic lambda will do; in a CUDA or HIP translation unit, pass a functor
  whose call operator is ``EBGEOMETRY_HOST_DEVICE``, since the device compiler treats a lambda's call
  operator as host code there.

A mesh is one leaf. To tell its faces apart, ask the mesh itself for its closest face at the local
point:

.. code-block:: c++

   using Mesh = TriMeshSDF<T, BVH::DefaultBranchingRatio<T>(), TriangleSoA::DefaultWidth<T>()>;

   const LeafRef<T> ref = tape.closestLeaf(p);

   if (ref.m_leaf != LeafRef<T>::None && tape.isPrimitive<Mesh>(ref)) {
     const Mesh     mesh = tape.primitive<Mesh>(ref);
     const uint32_t face = mesh.getClosestFace(ref.m_localPoint).faceId;   // the face's position in the file
   }

The local point matters here: the mesh was translated, rotated or scaled into place in the expression,
and its own BVH knows only the mesh's own coordinates. A nested tape is asked the same way, for its own
deciding leaf at the local point.

Labels
------

A leaf number is fine for a program that builds its expression itself, but it depends on the expression's
order: an edit earlier in the expression renumbers everything after it. A name does not.
``Label(object, "name")`` names an object, and every leaf below it gets the label, unless a label nearer the
leaf names it. Labels change nothing in how the object is evaluated, and nothing has to be labeled: an
unlabeled leaf has no label (``LeafRef::None``). Several objects may share a name, and so a label.

.. code-block:: c++

   const Expr<T> part = Difference(Intersection(Label(box, "wall"), ball), Label(Union(drills), "bores"));

   const Tape<T>      tape(pool, part);
   const ExprIndex<T> index(part);

   std::vector<Wall> walls(tape.getNumLabels());   // per-label data of your own
   walls[index.findLabel("wall")]  = ...;          // findLabel() is LeafRef<T>::None for an unknown name
   walls[index.findLabel("bores")] = ...;

   const uint32_t label = tape.getLabel(tape.closestLeaf(p));   // or tape.getLabel(hit.m_primitive)

The tape reports a leaf's label with ``getLabel()``, on the host and inside a GPU kernel, so a kernel
(a particle hitting a wall, say) looks up per-label data in an array of its own. The label is a number;
the names live on the host, in an
`ExprIndex <doxygen/html/classEBGeometry_1_1ExprIndex.html>`__ built from the same expression. Both
number the labels in the order their names first appear in the expression, depth first, so the same
expression gives the same numbers to both. A tape whose expression has no label stores none.

An ``ExprIndex`` also numbers the expression's nodes, depth first like the leaves, and gives each node its
parent, its operation and the leaves below it. The leaves below a node are always a contiguous range of
leaf numbers, so a group of surfaces (a whole subexpression) can be handed to a kernel as two numbers, and
``pathOf(leaf)`` lists the nodes from the root down to a leaf: the levels a surface belongs to. The nodes are
those the expression is built of: ``Difference(a, b)`` is an intersection of ``a`` with the complement of
``b``, and translations, rotations and scalings applied one after another are one transform, unless one of
them is labeled.

Along a ray
-----------

A ray query (see :ref:`Chap:Rays`) reports which part it crossed in ``HitT::m_primitive``, with the same
meaning: for a tape, the leaf whose surface the ray crossed, numbered as ``closestLeaf()`` numbers
the leaves. Other shapes name what they can: a mesh distance
field the id of the face crossed, a BVH union (not a smooth one) the member crossed, and any other shape (an analytic
shape, a fused composition) 0. The renderer colors by this number with
``Render::ColorBy::Primitive``, from a table of colors of your own or a built-in palette, and a click in
the viewer picks the part under the cursor by it (see :ref:`Chap:Rendering`).

On the host and on a device
---------------------------

``closestLeaf()``, ``isPrimitive()``, ``primitive()`` and ``visitPrimitive()`` can be called on the
host and inside a GPU kernel, like ``signedDistance()``. ``closestLeaf()`` evaluates the composition
the way ``signedDistance()`` does, carrying the deciding leaf and its local point along with every
intermediate value, so it costs somewhat more than ``signedDistance()``. The two are compiled
separately: ``signedDistance()`` carries nothing extra, and a program that never asks for the deciding
primitive pays nothing for it.

Only a tape has ``closestLeaf()``. A fused composition is one shape with one surface and names no
parts; to tell parts apart, make them leaves of a tape (each part can be a fused composition itself).

Examples
--------

* :ref:`Chap:ExampleMaterials` bores a cylinder through a mesh, labels the two parts, names each point's
  part with ``closestLeaf()`` and each ray's with ``HitT::m_primitive``, looks its label up in a table
  of materials, and renders the object colored by part.
* :ref:`Chap:ExampleBasicCSG` prints the deciding leaf at a few points of the classic CSG solid, a cube
  intersected with a ball and drilled along three axes.
* The AMReX ``Materials`` integration (see :ref:`Chap:Integrations`) labels every cut cell with its
  deciding leaf and, on the mesh, with the mesh's closest face at the local point, on a GPU when AMReX is
  built for one.
