.. _Chap:ImplemCSG:

Geometries
===========

This page covers the concrete classes behind two Concepts pages: :ref:`Chap:GeometryRepresentations`
(signed distance fields and implicit functions in general) and :ref:`Chap:ConstructiveSolidGeometry`
(transformations and CSG combinators) -- read those first for the conceptual picture, and use this
page for the class/function-level detail and Doxygen links.

ImplicitFunction
-----------------

``ImplicitFunction<T>`` (:file:`Source/EBGeometry_ImplicitFunction.hpp`) is the root of
EBGeometry's polymorphic geometry hierarchy: the transformations and CSG combinators below derive
from it, and so do any implicit functions users write themselves. The analytic shapes (see
:ref:`Sec:AnalyticShapes`) and the mesh distance fields (see :ref:`Chap:MeshSDFClasses`) do *not*:
they are plain value types that can be evaluated on a GPU, and the virtual interface would prevent
that. It declares a single pure virtual member function,

.. math::

   \texttt{value}(\mathbf{x}) : \mathbb{R}^3 \rightarrow \mathbb{R},

returning a value that is negative for points inside the object and positive for points outside
it, plus a call operator (``operator()``) that simply forwards to ``value()``. Because every
concrete geometry type honors exactly this same contract, code elsewhere in the library (and in
user code) can hold a ``shared_ptr<ImplicitFunction<T>>`` and call ``value()`` on it through
ordinary virtual dispatch without ever needing to know which concrete class it actually points
to -- this is what lets the transform and CSG machinery below wrap or combine *any* implicit
function interchangeably.

The analytic shapes give their exact bounding box from ``computeBoundingVolume()``
(:ref:`Sec:AnalyticShapes`). For functions that have no closed-form bounding volume, the free function
``approximateBoundingVolumeOctree(function, lo, hi, depth, safety)`` estimates a bounding box by
octree subdivision of a caller-supplied initial box, and ``normal(function, point, delta)`` gives
the finite-difference normal of a function. Both take anything that gives a value at a point: an
analytic shape or mesh distance field (through its ``signedDistance()``), an
``ImplicitFunction<T>`` (through its ``value()``), or a callable such as a lambda.
``ImplicitFunction<T>`` also keeps a member function ``approximateBoundingVolumeOctree`` that calls
the free function with the object itself. See :ref:`Chap:ImplemOctree` for both. The results are
only meaningful when the function is reasonably close to a true signed distance, since the safety
margin and the normal are interpreted in units of distance.

An ``ImplicitFunction<T>`` only promises that the sign of its value indicates inside or outside.
A signed distance additionally promises that the *magnitude* of its output is the true Euclidean
distance to the surface (the Eikonal property, :math:`|\nabla S| = 1`; see
:ref:`Chap:GeometryRepresentations` for why this property matters). The distance fields shipped
with EBGeometry -- the analytic shapes (:ref:`Sec:AnalyticShapes`) and the mesh distance fields
(``FlatMeshSDF``, ``MeshSDF``, ``TriMeshSDF``, see :ref:`Chap:MeshSDFClasses`) -- are plain
device-callable value types that provide ``signedDistance()`` without a virtual interface, and are
not ``ImplicitFunction``\ s. A user-written signed distance that should be passed to the
transformations and CSG combinators below derives from ``ImplicitFunction<T>`` and returns the
distance from ``value()``.

For the full API, see the Doxygen page for
`ImplicitFunction <doxygen/html/classEBGeometry_1_1ImplicitFunction.html>`__.

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

Because the shapes are not ``ImplicitFunction<T>`` objects, they cannot be passed to the
transformations and to most of the CSG combinators below. The exception is the BVH-accelerated
unions (:ref:`Sec:BVHUnions`), which take many shapes of one type directly and run on a GPU too.
Other compositions return with the redesign of the CSG layer that replaces virtual dispatch with a
linear-SSA tape. Until then, user code can combine shapes directly, as in the lambda passed to
``approximateBoundingVolumeOctree`` in the :ref:`Chap:ExampleOctreeBoundingVolume` example.

Transformations
----------------

EBGeometry implements every transformation as a small wrapper class that stores a
``shared_ptr<ImplicitFunction<T>>`` to the wrapped function (so the wrapped function is an
``ImplicitFunction<T>``, not one of the analytic shapes or mesh distance fields; see
:ref:`Sec:AnalyticShapes`) together with the transformation's
own parameters, and implements ``value()`` by applying the (typically inverse) transformation to
the query point before evaluating the wrapped function. Since every such wrapper is itself an
``ImplicitFunction<T>``, transformations compose freely -- wrapping a wrapper is just as valid as
wrapping a leaf shape. Each wrapper class has a same-named free function that constructs it and
returns it already cast to ``shared_ptr<ImplicitFunction<T>>``, ready to be passed into further
transformations or CSG combinators without the caller ever naming the concrete wrapper type.
Both are declared in :file:`Source/EBGeometry_Transform.hpp`.

The five simplest transformations -- translation, rotation, scaling, the complement, and
reflection -- are described mathematically in :ref:`Chap:ConstructiveSolidGeometry`; the table
below just maps each to its wrapper class, free function, and Doxygen entry:

.. list-table::
   :header-rows: 1
   :widths: 20 20 20 40

   * - Transformation
     - Wrapper class
     - Free function
     - Doxygen
   * - Translation
     - ``TranslateIF``
     - ``Translate``
     - `class <doxygen/html/classEBGeometry_1_1TranslateIF.html>`__ /
       `function <doxygen/html/namespaceEBGeometry.html#a82e34c99a4fd50db47df9115d394e6c5>`__
   * - Rotation
     - ``RotateIF``
     - ``Rotate``
     - `class <doxygen/html/classEBGeometry_1_1RotateIF.html>`__ /
       `function <doxygen/html/namespaceEBGeometry.html#a124737f03c27db761acf090d374ef2d6>`__
   * - Scaling
     - ``ScaleIF``
     - ``Scale``
     - `class <doxygen/html/classEBGeometry_1_1ScaleIF.html>`__ /
       `function <doxygen/html/namespaceEBGeometry.html#aa9ef2cbd810f91168aa99734b939ee13>`__
   * - Complement
     - ``ComplementIF``
     - ``Complement``
     - `class <doxygen/html/classEBGeometry_1_1ComplementIF.html>`__ /
       `function <doxygen/html/namespaceEBGeometry.html#a98227d701f3c9dd00d061fa23ecb5182>`__
   * - Reflection
     - ``ReflectIF``
     - ``Reflect``
     - `class <doxygen/html/classEBGeometry_1_1ReflectIF.html>`__ /
       `function <doxygen/html/namespaceEBGeometry.html#acc585f06f08ab2331f93cc1a38f53241>`__

EBGeometry implements several further transformations that have no closed-form counterpart in
:ref:`Chap:ConstructiveSolidGeometry` and are only described here:

* **Offset** (``OffsetIF``/``Offset``) subtracts a constant from the wrapped function's value,
  growing the object if the offset is positive and shrinking it if negative. For a true signed
  distance function this simply moves the isosurface outward or inward by the offset distance.
* **Annular** (``AnnularIF``/``Annular``) turns a solid into a hollow shell of total thickness
  :math:`2\delta` by evaluating :math:`|I(\mathbf{x})| - \delta`: the two new zero-isosurfaces sit
  where :math:`I(\mathbf{x}) = \pm\delta`, i.e. the original surface offset inward and outward by
  :math:`\delta` each, hollowing out everything in between.
* **Blur** (``BlurIF``/``Blur``) smooths sharp features by passing the wrapped function through a
  3D box filter sampled at the face centers, edge centers, and corners of a cube of half-width
  equal to the blur distance, with per-sample weights controlled by a blend factor ``alpha``
  (``1`` = no blur, ``0`` = maximal blur).
* **Mollify** (``MollifyIF``/``Mollify``) is a more general smoothing operation: it convolves the
  wrapped function with a kernel sampled on a uniform grid of offsets, normalizing the sample
  weights to sum to one. The kernel returns a non-negative weight for each offset. ``Mollify`` uses
  a smooth bump that is largest at the centre; ``MollifyIF`` takes any kernel.
* **Elongate** (``ElongateIF``/``Elongate``) stretches a shape along one or more axes without
  changing its cross-section: the query point is clamped component-wise to
  :math:`[-\mathbf{e}, \mathbf{e}]` for a per-axis elongation vector :math:`\mathbf{e}`, and the
  clamped offset is subtracted from the point before evaluating the wrapped function. For a signed
  distance function the result is exact outside; inside the elongated core it is constant, so it
  underestimates the depth there.

.. list-table::
   :header-rows: 1
   :widths: 20 20 20 40

   * - Transformation
     - Wrapper class
     - Free function
     - Doxygen
   * - Offset
     - ``OffsetIF``
     - ``Offset``
     - `class <doxygen/html/classEBGeometry_1_1OffsetIF.html>`__ /
       `function <doxygen/html/namespaceEBGeometry.html#a5a1c84b1c3726185ba5bfe6e581e18cd>`__
   * - Annular (shell)
     - ``AnnularIF``
     - ``Annular``
     - `class <doxygen/html/classEBGeometry_1_1AnnularIF.html>`__ /
       `function <doxygen/html/namespaceEBGeometry.html#ac373d3e0b9907711909806b81be6f470>`__
   * - Blur
     - ``BlurIF``
     - ``Blur``
     - `class <doxygen/html/classEBGeometry_1_1BlurIF.html>`__ /
       `function <doxygen/html/namespaceEBGeometry.html#a950895e40eb371d83a71208cdeb78656>`__
   * - Mollification
     - ``MollifyIF``
     - ``Mollify``
     - `class <doxygen/html/classEBGeometry_1_1MollifyIF.html>`__ /
       `function <doxygen/html/namespaceEBGeometry.html#adbce6c11cdd79925132a445ba3c3012e>`__
   * - Elongation
     - ``ElongateIF``
     - ``Elongate``
     - `class <doxygen/html/classEBGeometry_1_1ElongateIF.html>`__ /
       `function <doxygen/html/namespaceEBGeometry.html#abce875ee58575ba10c2605ab54c795ff>`__

.. warning::

   Not every transformation preserves the signed distance property. Rotation, translation, and
   the complement always do; scaling does provided the value is rescaled alongside the query
   point (which ``ScaleIF`` does); offset, annular, blur, mollification, and elongation generally
   do not produce an exact signed distance even when the input is one.

Every wrapper class and free function above is declared in
:file:`Source/EBGeometry_Transform.hpp`; see the Doxygen page for the file itself,
`EBGeometry_Transform.hpp <doxygen/html/EBGeometry__Transform_8hpp.html>`__, for the complete,
single-page API listing.

CSG
----

The Boolean combinators for combining multiple implicit functions into one -- union,
intersection, difference, and their smooth-blended variants -- are described conceptually in
:ref:`Chap:ConstructiveSolidGeometry`. EBGeometry implements each with the same wrapper-class
pattern as the transformations above: a class stores the combined implicit functions and
implements ``value()`` as the appropriate combination over them, with a matching free function
returning it as a ``shared_ptr<ImplicitFunction<T>>``. All of the following are declared in
:file:`Source/EBGeometry_CSG.hpp`:

.. list-table::
   :header-rows: 1
   :widths: 20 20 20 40

   * - Combinator
     - Wrapper class
     - Free function
     - Doxygen
   * - Union
     - ``UnionIF``
     - ``Union``
     - `class <doxygen/html/classEBGeometry_1_1UnionIF.html>`__ /
       `function <doxygen/html/namespaceEBGeometry.html#a463dfaffacab670e9683a134a9ba9eb0>`__
   * - Smooth union
     - ``SmoothUnionIF``
     - ``SmoothUnion``
     - `class <doxygen/html/classEBGeometry_1_1SmoothUnionIF.html>`__ /
       `function <doxygen/html/namespaceEBGeometry.html#ae4e6d40d6390b0943d7268c559716927>`__
   * - Intersection
     - ``IntersectionIF``
     - ``Intersection``
     - `class <doxygen/html/classEBGeometry_1_1IntersectionIF.html>`__ /
       `function <doxygen/html/namespaceEBGeometry.html#adc49cd0dbae6acda769be1b7597ac9a8>`__
   * - Smooth intersection
     - ``SmoothIntersectionIF``
     - ``SmoothIntersection``
     - `class <doxygen/html/classEBGeometry_1_1SmoothIntersectionIF.html>`__ /
       `function <doxygen/html/namespaceEBGeometry.html#ac254cf222340955631cee59cfbadf5e5>`__
   * - Difference
     - ``DifferenceIF``
     - ``Difference``
     - `class <doxygen/html/classEBGeometry_1_1DifferenceIF.html>`__ /
       `function <doxygen/html/namespaceEBGeometry.html#a34b51394d513d569ae3f1372a3475f17>`__
   * - Smooth difference
     - ``SmoothDifferenceIF``
     - ``SmoothDifference``
     - `class <doxygen/html/classEBGeometry_1_1SmoothDifferenceIF.html>`__ /
       `function <doxygen/html/namespaceEBGeometry.html#ab81c36234354c386be1d9d9a0f618ec9>`__

``Union``, ``SmoothUnion``, ``Intersection``, and ``SmoothIntersection`` each have two overloads
in the Doxygen listing above -- one taking a ``std::vector`` of any number of implicit functions,
and one taking exactly two -- both constructing the same underlying wrapper class.

The "smooth" combinators blend the transition between objects instead of leaving a sharp crease,
using a caller-replaceable blending functor rather than a plain ``min``/``max``. Four are provided
in :file:`Source/EBGeometry_Blend.hpp`, each a small, trivially copyable function object that can
also be called on a GPU -- ``SmoothMinOp<T>``, ``SmoothMaxOp<T>``, ``ExpMinOp<T>`` and
``ExpMaxOp<T>`` -- together with ready-made instances of them, ``SmoothMin<T>``, ``SmoothMax<T>``,
``ExpMin<T>`` and ``ExpMax<T>``, called as
``SmoothMin<T>(a, b, s)``. ``SmoothMin`` is a cheap polynomial smooth-minimum and the default for
``SmoothUnion``; ``SmoothMax`` is its symmetric counterpart and the default for both
``SmoothIntersection`` and ``SmoothDifference`` (difference is implemented internally as the
intersection of ``A`` with the complement of ``B``, which is why it defaults to the same operator
as intersection rather than to ``SmoothMin``); ``ExpMin`` and ``ExpMax`` are more expensive
exponential alternatives, evaluated in a form that cannot overflow however far the inputs are from
zero. The smooth combinators take the operator as a
``std::function<T(const T&, const T&, const T&)>``, to which any of the four -- or a
user-supplied functor of the same signature -- converts.

``FiniteRepetitionIF``/``FiniteRepetition`` tiles a single base implicit function
periodically over a finite number of repetitions per axis, by mapping the query point into the
nearest tile before evaluating the wrapped function -- effectively a cheap way to instance the
same shape many times without constructing a separate ``ImplicitFunction<T>`` (or a CSG union) per
copy. The repetition counts are whole numbers of tiles, rounded if fractional. The result is a
distance bound only if the base shape fits inside its own cell (half a period either side of the
origin on each axis); a shape reaching into a neighbouring cell is cut off where the cells meet.
See its Doxygen entries for
`FiniteRepetitionIF <doxygen/html/classEBGeometry_1_1FiniteRepetitionIF.html>`__ and
`FiniteRepetition <doxygen/html/namespaceEBGeometry.html#a46761e494f4b02faadb31fce9ebb8d80>`__.

.. _Sec:BVHUnions:

BVH-accelerated unions
______________________

Because a plain CSG union is evaluated as :math:`\min(I_1, \ldots, I_N)`, querying it costs
:math:`\mathcal{O}(N)` per point for :math:`N` objects. ``BVHUnion`` and ``BVHSmoothUnion``
accelerate this by placing the objects' bounding boxes in a ``PackedBVH`` and reducing a
closest-object query to an :math:`\mathcal{O}(\log N)` tree traversal instead of a linear scan --
see :ref:`Chap:ImplemBVH` for how the BVH itself is built and traversed; this section only concerns
how the unions use it. ``BVHSmoothUnion`` blends the two nearest objects with a smooth-minimum
functor, ``SmoothMinOp<T>`` by default.

Unlike the combinators above, the BVH unions are not ``ImplicitFunction<T>`` objects. Each is a
plain, trivially copyable value type, in the same way as the analytic shapes and the mesh distance
fields, whose ``signedDistance()`` can be called on the host or inside a GPU kernel:

* **One primitive type per union.** ``BVHUnion<T, P, K>`` holds its primitives by value in the
  ``PackedBVH``, so every primitive has the same type ``P``: an analytic shape, a mesh distance
  field, or another BVH union. No runtime dispatch is needed to evaluate them. A union of objects
  of *different* types needs exactly that dispatch, and returns with the redesign of the CSG layer
  that replaces virtual dispatch with a linear-SSA tape.
* **Bounding boxes that enclose their primitives.** The traversal skips a primitive whose bounding
  box is further away than the best value found so far. If every box encloses its primitive's
  object (where the primitive's value is not positive), a skipped primitive is positive at the query
  point, so skipping it never changes the union's sign, whatever the primitives' values are. The
  value is then as good a distance as the primitives': with ``Exact`` primitives it equals the plain
  minimum, with ``Bound`` primitives it is still a bound, and with ``NotADistance`` primitives only
  its sign means anything, which the union's ``distanceQuality`` reports.
* **Built in a** ``Pool``. The constructor takes the ``Pool`` to reserve the BVH from, the
  primitives, and one bounding box per primitive, which the BVH needs up front (an analytic shape's
  ``computeBoundingVolume()``, for example), plus an optional
  ``BVH::Construction`` strategy (SAH by default). A further constructor also takes the
  ``BVH::ConstructionOptions`` that set the chosen method's leaf size (see :ref:`Sec:LeafSizes`).
* **Mirrored to a GPU like any pool-backed type.** Freeze the pool, mirror it, and pass
  ``rebasedView(devicePool)`` to a kernel; ``deepCopy(pool)`` duplicates the storage.
* **Primitives that live in a pool.** A mesh distance field, or a nested union, stored as a
  primitive must have been built in the same ``Pool`` as the union; the constructor checks this in
  every build, not only when assertions are enabled. Mirroring the pool copies such a
  primitive's bytes verbatim, including the host bookkeeping that locates its own arrays, so
  rebasing the union cannot rewrite it in place. Instead the union applies its own pool location to
  each such primitive as it evaluates it (``relocatedTo()``, see
  `PoolLocation <doxygen/html/structEBGeometry_1_1PoolLocation.html>`__). That is what makes a union
  of ``TriMeshSDF`` objects -- a BVH whose primitives are themselves BVHs -- evaluable on a mirrored
  pool and on a GPU.

The :ref:`Chap:ExamplePackedSpheres` and :ref:`Chap:ExampleRandomCity` examples compare a BVH union
of spheres and of boxes against a brute-force scan, and :ref:`Chap:ExampleNestedBVH` builds a union
of mesh distance fields.

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

For a single-page API listing, see the Doxygen pages for the three headers this page covers:
`EBGeometry_CSG.hpp <doxygen/html/EBGeometry__CSG_8hpp.html>`__ (the ``shared_ptr`` combinators),
`EBGeometry_Blend.hpp <doxygen/html/EBGeometry__Blend_8hpp.html>`__ (the blend operators) and
`EBGeometry_BVHUnion.hpp <doxygen/html/EBGeometry__BVHUnion_8hpp.html>`__ (the BVH unions). The
blend operators and the BVH unions do not depend on the ``shared_ptr`` layer, so device code can
include their headers without it; ``EBGeometry_CSG.hpp`` includes both.
