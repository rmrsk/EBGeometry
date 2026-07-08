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
EBGeometry's polymorphic geometry hierarchy. Every concrete piece of geometry the library can
represent -- analytic shapes, DCEL/triangle-mesh distance fields, and CSG combinations of either
-- ultimately derives from it. It declares a single pure virtual member function,

.. math::

   \texttt{value}(\mathbf{x}) : \mathbb{R}^3 \rightarrow \mathbb{R},

returning a value that is negative for points inside the object and positive for points outside
it, plus a call operator (``operator()``) that simply forwards to ``value()``. Because every
concrete geometry type honors exactly this same contract, code elsewhere in the library (and in
user code) can hold a ``shared_ptr<ImplicitFunction<T>>`` and call ``value()`` on it through
ordinary virtual dispatch without ever needing to know which concrete class it actually points
to -- this is what lets the transform and CSG machinery below wrap or combine *any* implicit
function interchangeably.

``ImplicitFunction<T>`` also provides one concrete (non-virtual) member function,
``approximateBoundingVolumeOctree``, for shapes that have no closed-form bounding volume. It
refines an octree over a caller-supplied initial box, marking a cell as intersecting the surface
once the implicit function's value at the cell center falls within a safety-scaled margin of the
cell's half-width, then builds a bounding volume of the caller-chosen type ``BV`` from the
corners of the intersected leaf cells. This is the same octree-refinement idea described
conceptually in :ref:`Chap:Octree`; see that page for how the subdivision itself works. The
result is only meaningful when ``value()`` is reasonably close to a true signed distance, since
the safety margin is interpreted in units of distance.

For the full API (including the exact parameters and defaults of
``approximateBoundingVolumeOctree``), see the Doxygen page for
`ImplicitFunction <doxygen/html/classEBGeometry_1_1ImplicitFunction.html>`__.

SignedDistanceFunction
-----------------------

``SignedDistanceFunction<T>`` (:file:`Source/EBGeometry_SignedDistanceFunction.hpp`) inherits
from ``ImplicitFunction<T>`` and refines its contract, without adding to the public ``value()``
interface itself: it implements ``value()`` (marked ``final``) to delegate to a new pure virtual
member function, ``signedDistance(point)``, which subclasses must implement instead. The
distinction is one of guarantee rather than signature -- an arbitrary ``ImplicitFunction<T>``
only promises that the sign of its output indicates inside/outside, whereas a
``SignedDistanceFunction<T>`` additionally promises that the *magnitude* of its output is the
true Euclidean distance to the surface (the Eikonal property, :math:`|\nabla S| = 1`; see
:ref:`Chap:GeometryRepresentations` for why this property matters). Every analytic shape and
mesh distance field shipped with EBGeometry is a ``SignedDistanceFunction<T>``.

Because the true distance is available, ``SignedDistanceFunction<T>`` also provides a concrete
``normal(point, delta)`` member function that estimates the outward unit normal from central
finite differences of ``signedDistance`` with step size ``delta`` -- something that cannot be
done reliably from an arbitrary implicit function's value alone.

For the full API, see the Doxygen page for
`SignedDistanceFunction <doxygen/html/classEBGeometry_1_1SignedDistanceFunction.html>`__.

.. tip::

   Various ready-to-use implementations of both interfaces are declared in
   :file:`Source/EBGeometry_AnalyticDistanceFunctions.hpp` (spheres, boxes, planes, cylinders,
   tori, and other closed-form primitives) and in
   :file:`Source/EBGeometry_MeshDistanceFunctions.hpp` (the DCEL/triangle-mesh-backed classes
   ``FlatMeshSDF``, ``MeshSDF``, and ``TriMeshSDF`` -- see :ref:`Chap:MeshSDFClasses`).

Transformations
----------------

EBGeometry implements every transformation as a small wrapper class that stores a
``shared_ptr<ImplicitFunction<T>>`` to the wrapped function together with the transformation's
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
  wrapped function with a caller-supplied mollifier implicit function (typically a small sphere
  SDF) sampled on a uniform grid of offsets, normalizing the sample weights to sum to one.
* **Elongate** (``ElongateIF``/``Elongate``) stretches a shape along one or more axes without
  changing its cross-section: the query point is clamped component-wise to
  :math:`[-\mathbf{e}, \mathbf{e}]` for a per-axis elongation vector :math:`\mathbf{e}`, and the
  clamped offset is subtracted from the point before evaluating the wrapped function.

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
using a caller-replaceable blending functor rather than a plain ``min``/``max``. Two are provided
as ``std::function`` template variables in :file:`Source/EBGeometry_CSG.hpp`: ``SmoothMin<T>``
(a cheap polynomial smooth-minimum, the default for ``SmoothUnion``) and ``SmoothMax<T>`` (its
symmetric counterpart, the default for both ``SmoothIntersection`` and ``SmoothDifference`` --
difference is implemented internally as the intersection of ``A`` with the complement of ``B``,
which is why it defaults to the same operator as intersection rather than to ``SmoothMin<T>``),
plus a more expensive exponential alternative, ``ExpMin<T>``. Any of the three -- or a
user-supplied functor of the same signature, ``T(const T&, const T&, const T&)`` -- can be passed
as the smoothing operator to the smooth combinators' constructors/free functions.

Because a plain CSG union is evaluated as :math:`\min(I_1, \ldots, I_N)`, querying it costs
:math:`\mathcal{O}(N)` per point for :math:`N` objects. ``BVHUnionIF``/``BVHUnion`` and
``BVHSmoothUnionIF``/``BVHSmoothUnion`` accelerate this by placing the objects' bounding volumes
in a ``PackedBVH`` and reducing a closest-object query to an :math:`\mathcal{O}(\log N)` tree
traversal instead of a linear scan -- see :ref:`Chap:ImplemBVH` for how the BVH itself is built
and traversed; this section only concerns how CSG uses it. Unlike the plain combinators, the BVH
variants take the objects' bounding volumes explicitly (a ``std::vector<BV>``, one per object)
since these are needed up front to build the tree.

.. list-table::
   :header-rows: 1
   :widths: 20 20 20 40

   * - Combinator
     - Wrapper class
     - Free function
     - Doxygen
   * - BVH-accelerated union
     - ``BVHUnionIF``
     - ``BVHUnion``
     - `class <doxygen/html/classEBGeometry_1_1BVHUnionIF.html>`__ /
       `function <doxygen/html/namespaceEBGeometry.html#aa213a6a488504ce79c222c8c37f9c6fa>`__
   * - BVH-accelerated smooth union
     - ``BVHSmoothUnionIF``
     - ``BVHSmoothUnion``
     - `class <doxygen/html/classEBGeometry_1_1BVHSmoothUnionIF.html>`__ /
       `function <doxygen/html/namespaceEBGeometry.html#a5625be266d4f77c6b7035b3c6f4ef11c>`__

Finally, ``FiniteRepetitionIF``/``FiniteRepetition`` tiles a single base implicit function
periodically over a finite number of repetitions per axis, by mapping the query point into the
nearest tile before evaluating the wrapped function -- effectively a cheap way to instance the
same shape many times without constructing a separate ``ImplicitFunction<T>`` (or a CSG union) per
copy. See its Doxygen entries for
`FiniteRepetitionIF <doxygen/html/classEBGeometry_1_1FiniteRepetitionIF.html>`__ and
`FiniteRepetition <doxygen/html/namespaceEBGeometry.html#a46761e494f4b02faadb31fce9ebb8d80>`__.

For the complete, single-page API listing of every class and free function on this page, see the
Doxygen page for `EBGeometry_CSG.hpp <doxygen/html/EBGeometry__CSG_8hpp.html>`__.
