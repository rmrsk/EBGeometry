.. _Chap:Tape:

The tape
========

A composed object (see :ref:`Chap:ConstructiveSolidGeometry`) is first *described* as an expression
tree, and then *compiled* into a tape: a flat program that a small interpreter runs to evaluate the
object at a point. This page explains what the tape is, how an expression becomes one, and the limits
on what one tape can hold, with how to stay within them. The classes and functions are in
:ref:`Sec:Tape`.

Why a tape
----------

A scene read from a file or assembled by a user is known only when the program runs: how many
objects, of which kinds, combined in which order. Evaluating such a scene on a GPU rules out the
usual run-time tools. A tree of objects linked by pointers is valid only in the memory it was built
in, and virtual functions cannot be called on an object copied to a device. The tape avoids both:

* **Primitives in one array per type.** The shapes at the leaves (spheres, boxes, meshes, ...) are
  stored by value, one array for each shape type the tape can hold. The tape's type lists those
  types, so the interpreter can reach every one of them by an index and a type number, with no
  virtual call.
* **Operations as a list of instructions.** The operations are written out as *clauses*, run one
  after another. A clause refers to a primitive by its type and its index in that type's array, and
  to the values it combines by their place on a stack.
* **One block of memory.** The arrays, the clauses and their parameters all live in one ``Pool``
  (see :ref:`Chap:MemoryModel`) and refer to each other by offsets, not addresses. A copy of the pool
  in device memory is therefore a working copy of the tape, and the same interpreter runs on the host
  and on a GPU.

When the structure of a composition is known when the program is written, it can instead be compiled
together with the program as a *fused* composition (see :ref:`Sec:Fused`): its type spells out the
whole tree, and the compiler evaluates it as it would the same object written out by hand, with no
clauses to interpret. A fused composition computes the same values with the same arithmetic, but its
structure cannot change at run time; a tape can hold fused compositions as leaves, so the two
combine.

From expression to tape
-----------------------

The expression builder records each operation as a node with its operands as children. Two
simplifications happen already there: a chain of point maps (``Translate``, ``Rotate``, ``Reflect``,
``Scale``) applied directly to one another is folded into one affine map, and a difference
``Difference(a, b)`` is written as the intersection of ``a`` with the complement of ``b``. Beyond
that, the tape keeps the tree as it is given: it does not reorder, regroup or flatten operations.

The compiler then walks the tree operands first (post-order) and writes one or more clauses per node.
The interpreter runs them with two small stacks: a *value stack*, on which each clause leaves its
result for the clauses after it, and a *point stack*, which starts with the query point and on which a
point map pushes the mapped point for its operand and pops it afterwards. A primitive pushes its value
at the point on top; a union pops two values and pushes their minimum; a complement negates the top
value. When the program ends, the one value left is the object's value at the query point.
:numref:`Fig:TapeCompile` shows a small example.

.. _Fig:TapeCompile:
.. figure:: /_static/TapeCompile.png
   :width: 85%
   :align: center

   Compiling ``Difference(Intersection(box, ball), cylinder)``. The builder writes the difference as
   the intersection with the complement of the cylinder (left). The compiler visits the tree operands
   first, numbering the nodes in the order their clauses are written, and stores each primitive in its
   type's array (bottom left). Running the six clauses (right) leaves the object's value at the query
   point as the one value on the stack; this program never holds more than two values at once.

Sub-programs
------------

Some operations evaluate their operand many times, at different points, for one query point. Their
operand is compiled into a separate *sub-program*, which the operation's clause calls:

.. list-table::
   :header-rows: 1
   :widths: 35 65

   * - Operation
     - Calls its operand's sub-program
   * - ``Blur``
     - 27 times, once at each tap of the :math:`3 \times 3 \times 3` stencil
   * - ``Mollify(a, r, n)``
     - :math:`n^3` times, once per tap of the grid
   * - ``FiniteRepetition``
     - once for each copy the search reaches: the copy in the tile the point falls in, then the
       copies whose box is nearer than the value so far
   * - A union or smooth union of at least ``TapeOptions::m_bvhUnionThreshold`` operands (8 by
       default), all of them distances
     - each operand is compiled into its own sub-program; a BVH over the operands' boxes picks which
       ones to call, and the others are pruned (see :ref:`Sec:Tape`)

A sub-program has stacks of its own: when it starts, only its input point is on its point stack, and
nothing its caller holds counts against it. A sub-program can itself contain one of these operations,
so calls nest; :numref:`Fig:TapeSubprograms` shows a main program, a blur, and a BVH union inside it.
Every other operation (the maps, offsets, annuli, elongations, complements, unions and intersections
below the threshold, and the smooth operations) is written into the program that contains it.

.. _Fig:TapeSubprograms:
.. figure:: /_static/TapeSubprograms.png
   :width: 85%
   :align: center

   The sub-programs of ``Union(box, Blur(Union({part_1, ..., part_8}), d, alpha))``, where each part is a
   placed mesh. The main program (call level 0) calls the blurred operand's program 27 times; that
   program is a BVH union of eight operands, each compiled into its own program at call level 2. Each
   program has its own stacks. Calls may nest ``Tape::MaxCallDepth`` (4) levels below the main
   program; a fifth is rejected when the tape is built.

The limits of a tape
--------------------

The interpreter has no recursion and allocates no memory while it runs: its stacks are fixed-size
arrays, and each call level is a separate, fixed copy of the interpreter. This is what keeps it usable
on a GPU, and it bounds what one tape can hold. Every limit is checked when the tape is constructed,
in every build (not only with assertions enabled): an expression that exceeds one stops the program
with a message naming the limit and the amount the expression needs. Nothing is checked during
evaluation, and an expression the constructor accepts is evaluated without any limit being reached.

Values held at once
~~~~~~~~~~~~~~~~~~~~

A sub-program holds at most ``Tape::ValueStackSize`` (16) values at once. A primitive needs one place
on the stack. A union or intersection evaluates its first operand, then each further operand followed
by the minimum or maximum, so while an operand is evaluated, the result of everything to its left
waits on the stack. How much a composition needs therefore depends on its shape, not only on its size
(:numref:`Fig:TapeUnionNesting`):

* **Nested on the left,** ``Union(Union(Union(a, b), c), d)``, each union's first operand is the
  inner union, which is finished before the next operand starts: two values for any number of
  operands. This is what a loop such as ``u = Union(u, next)`` builds.
* **Nested on the right,** ``Union(a, Union(b, Union(c, d)))``, each first operand waits while the
  rest is evaluated: :math:`n` operands nested this way hold :math:`n` values, so seventeen are
  rejected. The same holds for intersections, and for differences nested on the subtracted side,
  ``Difference(a, Difference(b, ...))``.
* **One union of a list,** ``Union({a, b, c, d})``, holds two values, like the left-nested form. With
  at least ``TapeOptions::m_bvhUnionThreshold`` operands, all of them distances, each operand becomes
  its own sub-program, with fresh stacks, and the union itself holds one value in its caller.
* **Smooth unions and intersections** keep the two most extreme values of their operands while they
  evaluate them, so each one holds two values plus what its operand needs, whichever side it is nested
  on: seven binary smooth operations of primitives nested in each other fit in a sub-program, eight do
  not. A smooth union of a list (below the BVH threshold) holds two values plus what its largest
  operand needs; it blends only the two most extreme values, which is a different object from nested
  binary smooth unions.

.. _Fig:TapeUnionNesting:
.. figure:: /_static/TapeUnionNesting.png
   :width: 85%
   :align: center

   The same four-way union nested on the left, nested on the right, and written as one union of a list.
   The bars show how many values are on the stack after each clause. Nested on the right, every operand
   but the last waits for the ones after it; the other two forms never hold more than two values.

To stay within the limit, build unions, intersections and differences of many objects from a list
(``Union(list)``, ``Intersection(list)``, ``Difference(a, list)``), or nest them on the left. A list is
also the faster choice for a union: only a union whose operands are given as one list can reach the
BVH threshold, since a binary ``Union(a, b)`` has two operands however deeply it is nested, and is
evaluated operand by operand.

Point maps held at once
~~~~~~~~~~~~~~~~~~~~~~~

A sub-program holds at most ``Tape::PointStackSize`` (8) points, the query point included, so seven
point maps can be in effect at once. Each map whose operand contains another map with an operation in
between (``Translate(Offset(Rotate(...)))``) counts, and so does each ``Elongate``. Maps applied
directly to one another fold into one and count once, however many there are.

Call depth
~~~~~~~~~~

Calls nest at most ``Tape::MaxCallDepth`` (4) deep below the main program: a ``Blur`` of a
``FiniteRepetition`` of a ``Mollify`` of a large union fills it. ``Tape::getCallDepth()`` reports how
deep a built tape's calls nest. A tape listed as a primitive of another tape (see below) is one leaf
there and is evaluated by its own interpreter, with its own call levels, which do not count against the
outer tape's. On a GPU, a kernel that evaluates a tape reserves stack for every call level the tape
type allows, whatever a particular tape needs, and a nested tape adds its own levels on top.

Primitive types
~~~~~~~~~~~~~~~

A tape can hold the twelve analytic shapes and the three mesh distance fields with their default
parameters without being told. Any other shape type (a BVH union, a mesh distance field with another
branching factor, a fused composition, another tape, or a type of your own) must be listed in the
tape's type, ``Tape<T, MyShape, BVHUnion<T, SphereSDF<T>, 4>>``, since the interpreter can only reach
the types its type names. A listed type must be a shape of the tape's precision, which is checked when
the program is compiled. An expression containing a type that is not listed compiles, but is rejected
when the tape is constructed, as is a mesh, BVH union or tape that lives in another pool than the one
the tape is built in. Asking a tape whether a leaf is of a type it cannot hold is a compile error.

Optional code that only some expressions need, such as exact ray tracing of offsets, is a *tape
feature*, listed in the tape's type in the same way (see :ref:`Sec:TapeFeatures`). A tape without a
feature still computes the same object, by the general path.

Ray programs
~~~~~~~~~~~~

Besides its clauses, a tape is compiled into a ray program for ray queries (see :ref:`a tape's ray program <Sec:TapeRays>`),
which holds at most ``Tape::RayStackSize`` (4) lists of spans at once. This limit never rejects an
expression: a part of the expression that would need more is marched along the ray instead of traced
exactly.

Further reading
---------------

The :ref:`Chap:ExampleBasicCSG` example compiles a composition of analytic shapes into a tape;
:ref:`Chap:ExampleValueOperations` and :ref:`Chap:ExampleRepetition` use operations that compile into
sub-programs; :ref:`Chap:ExampleNestedBVH` builds a union of several placements of one mesh, evaluated
through a BVH; and :ref:`Chap:ExampleFusedCSG` builds the same solid as a tape and as a fused
composition.
