.. _Chap:ImplemCSG:

Geometries
===========

This page covers the concrete classes behind two Concepts pages: :ref:`Chap:GeometryRepresentations`
(signed distance fields and implicit functions in general) and :ref:`Chap:ConstructiveSolidGeometry`
(transformations and CSG combinators) -- read those first for the conceptual picture, and use this
page for the class/function-level detail and Doxygen links.

Shapes
------

A *shape* is anything EBGeometry can evaluate and compose: a trivially copyable type with a const
``signedDistance(const Vec3T<T>&)`` returning ``T`` -- negative inside the object, positive outside
-- and a const ``computeBoundingVolume()`` returning the ``AABBT<T>`` that encloses the object. The
compile-time trait ``isShape<S>`` checks exactly this, and ``ShapePrecision<S>`` reads ``T`` from the
box type (see `EBGeometry_Expr.hpp <doxygen/html/EBGeometry__Expr_8hpp.html>`__). The analytic shapes
(:ref:`Sec:AnalyticShapes`), the mesh distance fields (:ref:`Chap:MeshSDFClasses`), the BVH unions
(:ref:`Sec:BVHUnions`) and a compiled ``Tape`` (:ref:`Sec:Tape`) are all shapes, and so is any type
of your own with those two members. None of them has a base class or a virtual function, so each can
be passed to a GPU kernel by value.

A shape may also carry a static member ``distanceQuality`` that says how far its value can be trusted
as a distance (see :ref:`Sec:AnalyticShapes`); a shape without one counts as ``Bound``. The
difference matters because a signed distance promises more than the sign: its *magnitude* is the
distance to the surface (the Eikonal property, :math:`|\nabla S| = 1`; see
:ref:`Chap:GeometryRepresentations`), which BVH pruning, sphere tracing and the octree bounding-volume
search rely on.

Every shape's value goes by the one name ``signedDistance``, whatever its distance quality, so the
name alone does not say how much the value promises; the name and the quality together do. The sign
is what every shape promises: negative inside, positive outside, zero on the surface. The magnitude is
what the distance quality says:

* ``Exact``: the distance to the surface;
* ``Bound``: at most the distance to the surface, so it may underestimate but never overestimates;
* ``NotADistance``: nothing; only the sign is meaningful.

A composed object's quality is worked out when it is built (``Expr::getDistanceQuality()``,
``Tape::getDistanceQuality()``): a smooth union is a bound, for instance, and anything built from
``PerlinSDF`` is not a distance.

For a function with no closed-form bounding volume, the free function
``approximateBoundingVolumeOctree(function, lo, hi, depth, safety)`` estimates a bounding box by
octree subdivision of a caller-supplied initial box, and ``normal(function, point, delta)`` gives the
finite-difference normal of a function. Both take anything that gives a value at a point: a shape
(through its ``signedDistance()``) or a callable such as a lambda. See :ref:`Chap:ImplemOctree` for both. The results are only meaningful when the function is
reasonably close to a true signed distance, since the safety margin and the normal are interpreted in
units of distance.

.. _Sec:AnalyticShapes:

Analytic shapes
----------------

:file:`Source/EBGeometry_AnalyticDistanceFunctions.hpp` declares twelve closed-form shapes:

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Class
     - Shape
   * - ``PlaneSDF``
     - A plane through a point, with a normal
   * - ``SphereSDF``
     - A sphere
   * - ``BoxSDF``
     - An axis-aligned box
   * - ``TorusSDF``
     - A torus whose ring lies in the xz-plane, around the y axis
   * - ``CylinderSDF``, ``InfiniteCylinderSDF``
     - A capped cylinder between two points; an infinite cylinder along a coordinate axis
   * - ``CapsuleSDF``
     - A cylinder with hemispherical end caps
   * - ``ConeSDF``, ``InfiniteConeSDF``
     - A finite and an infinite cone, opening along -y from the tip
   * - ``RoundedBoxSDF``, ``RoundedCylinderSDF``
     - An axis-aligned box and a cylinder along y, with rounded edges
   * - ``PerlinSDF``
     - Perlin noise (not a distance function, only its sign means anything)

Each is a plain, trivially copyable value type with no base class and no virtual functions, in
the same way as the mesh distance fields. Its ``signedDistance()`` and accessors are callable on
the host and on a GPU, so a shape can be passed to a kernel by value and evaluated there; the
constructors run on the host. The :ref:`Chap:ExampleShapes` example constructs every one of them.

The shapes share a few conventions:

* **Placement first.** Each constructor takes where the shape is (a center, a corner, a tip or a
  point on it) before its sizes.
* **One axis.** A shape with a built-in axis has it along y: the torus, both cones and the rounded
  cylinder, and also the default-constructed cylinder, infinite cylinder, capsule and plane.
  ``CylinderSDF`` and ``CapsuleSDF`` take two end points and ``InfiniteCylinderSDF`` an axis index,
  so they can point anywhere.
* **Outer sizes.** A size is the size of the finished shape, rounding included: ``RoundedBoxSDF``
  takes the box it fits exactly, and its rounding radius must be less than half of every side.
* **A bounding box.** ``computeBoundingVolume()`` returns the shape's axis-aligned bounding box, on
  the host and on a GPU, ready to pass to a BVH union. Along a direction in which a shape is
  unbounded, the box extends to plus or minus the largest finite value of ``T`` (not infinity, so
  that box arithmetic such as a center stays finite): the infinite cylinder along its axis, the
  infinite cone everywhere but above its tip, and the plane and the noise in every direction.
* **A distance quality.** A static member ``distanceQuality`` says how far ``signedDistance()``
  can be trusted as a distance, as one of three
  `DistanceQuality <doxygen/html/namespaceEBGeometry.html#aba480dba46b4ac1b2393b5f2872cdad5>`__ levels:
  ``Exact`` (the magnitude is the Euclidean distance to the surface), ``Bound`` (the magnitude
  never exceeds it) or ``NotADistance`` (only the sign means anything). Every shape is ``Exact``
  except ``PerlinSDF``, which is ``NotADistance``. The mesh distance fields are ``Exact``. A BVH
  union is ``Bound``, or ``NotADistance`` if its primitives are. ``distanceQualityOf<P>`` reads the
  member, and treats a type without one as ``Bound``.

Every shape can be composed with the expression builder and evaluated through a ``Tape``
(:ref:`Sec:Tape`); many shapes of one type can also be joined directly by a BVH union
(:ref:`Sec:BVHUnions`).

Composing objects
-----------------

Objects are composed with the expression builder in
`EBGeometry_Expr.hpp <doxygen/html/EBGeometry__Expr_8hpp.html>`__. Every shape converts to an
`Expr <doxygen/html/classEBGeometry_1_1Expr.html>`__, an immutable tree of shapes and operations, and
the free functions take shapes or ``Expr``\ s and return an ``Expr``:

.. code-block:: c++

   const SphereSDF<T> sphere(Vec3::zeros(), T(1));
   const BoxSDF<T>    box(-Vec3::ones(), Vec3::ones());

   const auto shape = SmoothDifference(Union(Translate(sphere, Vec3(T(2), T(0), T(0))), box), mesh, T(0.1));

An ``Expr`` is cheap to copy (copies share the tree) and is never evaluated itself: it is a
description, which a ``Tape`` compiles (:ref:`Sec:Tape`). The precision is deduced from the operands.
A pool-resident shape, such as a mesh distance field, is held by its descriptor, so its pool must be
the one the tape is compiled into and must outlive the tape.

The operations, described mathematically in :ref:`Chap:ConstructiveSolidGeometry`:

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Function
     - Value of the result at :math:`\mathbf{x}`
   * - ``Translate(a, t)``
     - :math:`a(\mathbf{x} - \mathbf{t})`
   * - ``Rotate(a, degrees, axis)``
     - :math:`a(R^{-1}\mathbf{x})`, about a coordinate axis (0, 1, 2) or about any axis direction
   * - ``Scale(a, s)``
     - :math:`|s|\,a(\mathbf{x}/s)`, uniform; a negative factor also reflects through the origin
   * - ``Reflect(a, axis)``
     - :math:`a` at :math:`\mathbf{x}` with one coordinate negated
   * - ``Complement(a)``
     - :math:`-a(\mathbf{x})`
   * - ``Offset(a, r)``
     - :math:`a(\mathbf{x}) - r`: grows the object by :math:`r`, or shrinks it if :math:`r < 0`
   * - ``Annular(a, delta)``
     - :math:`|a(\mathbf{x})| - \delta`: a shell of thickness :math:`2\delta` around the surface
   * - ``Elongate(a, h)``
     - :math:`a(\mathbf{x} - \mathrm{clamp}(\mathbf{x}, -\mathbf{h}, \mathbf{h}))`: inserts a box of
       half-lengths :math:`\mathbf{h}` at the origin
   * - ``Blur(a, d, alpha)``
     - A weighted average of :math:`a` at the 27 points :math:`\mathbf{x} + d(i, j, k)`,
       :math:`i, j, k \in \{-1, 0, 1\}`, with weights :math:`\alpha^{3-m}((1-\alpha)/2)^m` for a
       point :math:`m` steps from the centre (``alpha = 1`` is no blur)
   * - ``Mollify(a, r, n)``
     - :math:`\sum_k w_k\,a(\mathbf{x} - \mathbf{o}_k)` over an :math:`n^3` grid of offsets spanning
       :math:`[-r, r]^3`, with the weights of a smooth bump kernel, normalized to sum to one
   * - ``FiniteRepetition(a, c, lo, hi)``
     - :math:`a` at :math:`\mathbf{x}` folded into its tile: :math:`x_i - c_i\,\mathrm{round}(\mathrm{clamp}(x_i/c_i, -lo_i, hi_i))`
   * - ``Union(a, b)``, ``Union(list)``
     - :math:`\min_i a_i(\mathbf{x})`
   * - ``Intersection(a, b)``, ``Intersection(list)``
     - :math:`\max_i a_i(\mathbf{x})`
   * - ``Difference(a, b)``, ``Difference(a, list)``
     - :math:`\max(a, -\min_i b_i)`
   * - ``Add(a, b)``
     - :math:`a(\mathbf{x}) + b(\mathbf{x})`: not a distance, and bounded nowhere
   * - ``Displace(a, d, amplitude)``
     - :math:`a(\mathbf{x}) + d(\mathbf{x})`, for a displacement field :math:`d` (noise, a texture)
       whose magnitude never exceeds ``amplitude``, which nothing checks
   * - ``SmoothUnion``, ``SmoothIntersection``, ``SmoothDifference``
     - A smooth minimum or maximum of the two extreme values, with smoothing length :math:`s` and a
       ``BlendKind`` (``Polynomial`` or ``Exponential``); a smooth difference is the smooth
       intersection of :math:`a` with the complement of each :math:`b_i`

The n-ary smooth operations blend only the two extreme values, which differs from folding a binary
smooth minimum over the list. ``Polynomial`` uses ``SmoothMinOp``/``SmoothMaxOp`` and
``Exponential`` uses ``ExpMinOp``/``ExpMaxOp``, the blend operators in
`EBGeometry_Blend.hpp <doxygen/html/EBGeometry__Blend_8hpp.html>`__: small, trivially copyable
function objects, callable on a GPU, with ready-made instances ``SmoothMin<T>``, ``SmoothMax<T>``,
``ExpMin<T>`` and ``ExpMax<T>``. ``ExpMin`` and ``ExpMax`` are evaluated in a form that cannot
overflow however far the inputs are from zero.

Each ``Expr`` knows a box that encloses its object (``computeBoundingVolume()``) and its distance
quality (``getDistanceQuality()``), worked out from its operands' when it is made:

* **Boxes.** A point map sends the operand's box through its inverse (exactly for translations,
  scalings, reflections and quarter turns; through the eight corners for other rotations). Offset,
  Annular, Elongate, Blur (by :math:`\sqrt{3}d`) and Mollify (by :math:`\sqrt{3}r`) grow it; a
  repetition sweeps it over the tiles; a union merges the boxes (a smooth union grows the result by
  :math:`s/4`, or :math:`s\ln 2` for the exponential blend), an intersection intersects them, and a
  difference keeps the first operand's. A complement covers all of space, and so does ``Add``;
  ``Displace`` grows the first operand's box by the amplitude.
* **Distance quality.** Point maps and Complement keep the operand's quality, as do Annular and a
  growing Offset; a shrinking Offset, Elongate, Blur, Mollify and every union, intersection and
  difference give at best ``Bound``. A repetition is ``Bound`` only if the operand fits inside its
  own tile, and ``NotADistance`` otherwise. ``Add`` and ``Displace`` are ``NotADistance``: a
  displacement that changes by :math:`L` per unit length makes the sum :math:`(1 + L)`-Lipschitz.
  Anything containing a ``NotADistance`` operand is ``NotADistance``.

.. warning::

   Not every operation preserves the signed distance property; ``getDistanceQuality()`` says what is
   left of it. Rotation, translation, reflection, scaling and the complement keep an exact distance
   exact; the other operations generally do not produce an exact signed distance even when their
   input is one.

.. _Sec:Tape:

The tape
--------

A `Tape <doxygen/html/classEBGeometry_1_1Tape.html>`__ compiles an ``Expr`` into a ``Pool`` and
evaluates it:

.. code-block:: c++

   Pool          pool(hostMemoryResource());
   const Tape<T> tape(pool, shape);           // shape is an Expr<T>, or any shape
   const T       d = tape.signedDistance(p);

It stores its primitives in one pool array per primitive type and its operations as a list of
clauses that refer to them by type and index, so evaluating it calls no virtual function and follows
no pointer. A clause list runs on a small stack of values and a stack of points: a point map pushes
the transformed point before its operand is evaluated, and a value operation combines the values its
operands leave. A chain of ``Translate``, ``Rotate``, ``Reflect`` and ``Scale`` is folded into one
affine map (`Affine3T <doxygen/html/structEBGeometry_1_1Affine3T.html>`__) when the ``Expr`` is
built.

* **Primitive types.** The twelve analytic shapes and the three mesh distance fields
  (``FlatMeshSDF``, and ``MeshSDF`` and ``TriMeshSDF`` with the default branching factor and SIMD
  width) are built in. Any other shape type -- a BVH union, a mesh distance field with another
  branching factor, another tape, or a type of your own -- is listed after ``T``:
  ``Tape<T, BVHUnion<T, SphereSDF<T>, 4>, MyShape>``. The constructor rejects a shape whose type is
  not listed, and a pool-resident shape that lives in another pool.
* **Unions of many objects.** A union or smooth union of at least
  ``TapeOptions::m_bvhUnionThreshold`` operands (8 by default) is evaluated with a BVH over the
  operands' boxes, exactly as ``BVHUnion`` and ``BVHSmoothUnion`` evaluate their primitives
  (:ref:`Sec:BVHUnions`), including keeping operands with unbounded boxes out of the tree. The
  operands can be of any type and composed in any way, such as several translations of one mesh. With
  ``Exact`` operands the value is the same as an operand-by-operand minimum. A ``Bound`` operand (a
  blur, an elongation, a shrinking offset) can have a value below its box's distance, so the BVH may
  skip an operand whose value would have been smaller: the result can then be larger than the plain
  minimum, but it keeps the sign and is still a bound on the distance. A union with an operand that is
  ``NotADistance`` cannot be pruned and is evaluated operand by operand.
  `TapeOptions <doxygen/html/structEBGeometry_1_1TapeOptions.html>`__ also chooses the BVH
  construction method.
* **Sub-programs and limits.** Blur, Mollify and the operands of a BVH union are compiled into
  sub-programs that their clause evaluates several times. The evaluator has no recursion: a call
  depth is a template parameter, so calls nest at most ``Tape::MaxCallDepth`` (4) deep, and a
  sub-program holds at most ``ValueStackSize`` values and ``PointStackSize`` points at once. The
  constructor rejects an expression that needs more, with a message saying which limit it reached.
* **Metadata.** ``computeBoundingVolume()`` and ``getDistanceQuality()`` return the expression's
  box and quality. The quality is a runtime value, so a tape used as another object's primitive
  counts as ``Bound`` there. ``getNumPrograms()``, ``getNumClauses()``, ``getNumBVHUnions()`` and
  ``getCallDepth()`` describe what was compiled.
* **A pool handle.** A tape is trivially copyable and holds offsets into its pool and one location
  (see :ref:`Chap:MemoryModel`): ``rebasedView(mirror)`` evaluates it against a mirror of its pool,
  and ``deepCopy(pool)`` copies it, with every pool-resident primitive and BVH it holds, into another
  pool. The pool must outlive the tape and every copy and view of it.
* **On a GPU.** A tape is evaluated in a kernel like any pool-backed type: freeze its pool, mirror
  it into device memory, and pass ``rebasedView(devicePool)`` to the kernel by value. Every
  operation, primitive type and BVH union runs there; the test suite checks each against the host.
  Each call level of the evaluator is a separate function in a device compile, kept out of line so
  that the compile stays small (HIP otherwise inlines every device function, and the copies multiply
  with every level). Because the call depth is a template parameter, a kernel reserves stack for all
  ``MaxCallDepth`` + 1 levels whatever the tape needs: about 5.6 KB per thread in double precision
  and 3.6 KB in single precision, measured for an AMD GPU, of which each level takes about 1.1 KB in
  double precision (its value and point stacks, a BVH union's traversal stack and saved registers).
  A tape listed as another tape's primitive adds its own levels on top.

The :ref:`Chap:ExampleCSG` example composes analytic shapes through a tape, and
:ref:`Chap:ExampleNestedBVH` unions several placements of one mesh with spheres.

.. _Sec:BVHUnions:

BVH-accelerated unions
______________________

Because a plain CSG union is evaluated as :math:`\min(I_1, \ldots, I_N)`, querying it costs
:math:`\mathcal{O}(N)` per point for :math:`N` objects. ``BVHUnion`` and ``BVHSmoothUnion``
accelerate this by placing the objects' bounding boxes in a ``PackedBVH`` and reducing a
closest-object query to an :math:`\mathcal{O}(\log N)` tree traversal instead of a linear scan --
see :ref:`Chap:ImplemBVH` for how the BVH itself is built and traversed; this section only concerns
how the unions use it. ``BVHSmoothUnion`` blends the two nearest objects with a smooth-minimum
functor, ``SmoothMinOp<T>`` by default. A tape evaluates a union of many operands the same way
(:ref:`Sec:Tape`); the classes here are for many primitives of one type, without compiling an
expression. Each is a plain, trivially copyable value type, in the same way as the analytic shapes
and the mesh distance fields, whose ``signedDistance()`` can be called on the host or inside a GPU
kernel:

* **One primitive type per union.** ``BVHUnion<T, P, K>`` holds its primitives by value in the
  ``PackedBVH``, so every primitive has the same type ``P``: an analytic shape, a mesh distance
  field, or another BVH union. No runtime dispatch is needed to evaluate them. For a union of objects
  of *different* types, compose them with ``Union`` and compile the result into a tape.
* **Bounding boxes that enclose their primitives.** The traversal skips a primitive whose bounding
  box is further away than the best value found so far. If every box encloses its primitive's
  object (where the primitive's value is not positive), a skipped primitive is positive at the query
  point, so skipping it never changes the union's sign, whatever the primitives' values are. The
  value is then as good a distance as the primitives': with ``Exact`` primitives it equals the plain
  minimum, with ``Bound`` primitives it is still a bound, and with ``NotADistance`` primitives only
  its sign means anything, which the union's ``distanceQuality`` reports.
* **Primitives with unbounded boxes.** A primitive whose box reaches plus or minus the largest
  finite value of ``T`` in some direction, such as an infinite cylinder, would make the box of
  every BVH node above it unbounded and give the build heuristics nothing to work with. The union
  keeps such primitives out of the BVH and evaluates them at every query, before the traversal, so
  that their values can prune it: a union of many bounded primitives and a few unbounded ones costs
  the BVH query plus one evaluation per unbounded primitive. ``getBVH()`` holds the bounded
  primitives and ``getUnboundedPrimitives()`` the others, and ``computeBoundingVolume()`` covers
  both.
* **Built in a** ``Pool``. The constructor takes the ``Pool`` to reserve the BVH from, the
  primitives, and one bounding box per primitive, which the BVH needs up front (an analytic shape's
  ``computeBoundingVolume()``, for example), plus an optional
  ``BVH::Construction`` strategy (SAH by default). A further constructor also takes the
  ``BVH::ConstructionOptions`` that set the chosen method's leaf size (see :ref:`Sec:LeafSizes`).
* **Mirrored to a GPU like any pool-backed type.** Freeze the pool, mirror it, and pass
  ``rebasedView(devicePool)`` to a kernel; ``deepCopy(pool)`` duplicates the storage.
* **Primitives that live in a pool.** A mesh distance field, or a nested union, stored as a
  primitive must have been built in the same ``Pool`` as the union; the constructor checks this in
  every build, not only when assertions are enabled. The union stores such a primitive as its
  layout, which holds offsets but no location, and rebuilds it with the union's own location as it
  evaluates it (see :ref:`Chap:MemoryModel` and
  `PoolHandle <doxygen/html/classEBGeometry_1_1PoolHandle.html>`__). That is what makes a union of
  ``TriMeshSDF`` objects -- a BVH whose primitives are themselves BVHs -- evaluable on a mirrored pool
  and on a GPU. ``getBVH().getPrimitives()`` therefore returns the layouts of such primitives.

The :ref:`Chap:ExampleManyObjects` example times a BVH union of spheres and of boxes against a plain
loop over every object, and :ref:`Chap:ExampleNestedBVH` builds a union of mesh distance fields.

.. list-table::
   :header-rows: 1
   :widths: 30 30 40

   * - Combinator
     - Class
     - Doxygen
   * - BVH-accelerated union
     - ``BVHUnion``
     - `class <doxygen/html/classEBGeometry_1_1BVHUnion.html>`__
   * - BVH-accelerated smooth union
     - ``BVHSmoothUnion``
     - `class <doxygen/html/classEBGeometry_1_1BVHSmoothUnion.html>`__

For a single-page API listing, see the Doxygen pages for the headers this page covers:
`EBGeometry_Expr.hpp <doxygen/html/EBGeometry__Expr_8hpp.html>`__ (the expression builder),
`EBGeometry_Tape.hpp <doxygen/html/EBGeometry__Tape_8hpp.html>`__ (the tape),
`EBGeometry_Affine.hpp <doxygen/html/EBGeometry__Affine_8hpp.html>`__ (the affine maps),
`EBGeometry_Blend.hpp <doxygen/html/EBGeometry__Blend_8hpp.html>`__ (the blend operators) and
`EBGeometry_BVHUnion.hpp <doxygen/html/EBGeometry__BVHUnion_8hpp.html>`__ (the BVH unions).
`EBGeometry_CSG.hpp <doxygen/html/EBGeometry__CSG_8hpp.html>`__ includes all of them.
