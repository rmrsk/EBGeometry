.. _Chap:Vector:

Vector types
============

EBGeometry implements its own 2D and 3D vector types, ``Vec2T`` and ``Vec3T``. Like every other
class in the library, both are templated on a floating-point precision ``T`` (``float`` or
``double``), so a single header defines both precisions with no code duplication and no runtime
precision switch.

``Vec2T`` is a two-dimensional Cartesian vector. Most of EBGeometry is written as
three-dimensional code, but ``Vec2T`` is needed for DCEL functionality when determining whether a
point projects onto the interior or exterior of a planar polygon, see :ref:`Chap:DCEL`. Its main
features are:

* Public ``x``/``y`` components, settable either directly or through a two-argument constructor.
* Named static factories for common vectors: ``zeros()``, ``ones()``, ``min()``, ``max()``, and
  ``infinity()``.
* The usual arithmetic operators (``+``, ``-``, unary ``-``, scalar ``*``/``/``, and their
  in-place ``+=``/``-=``/``*=``/``/=`` counterparts), plus a scalar-left multiplication and
  division as free functions.
* ``dot``, ``length``, and ``length2`` member functions, mirrored by free-function equivalents
  (``dot``, ``length``) and free-function component-wise ``min``/``max``.

``Vec3T`` is a three-dimensional Cartesian vector, likewise templated on precision ``T``. Unlike
``Vec2T``, its components are stored internally and reached only through ``operator[]`` (mutable
and const overloads), not as public named fields. Its main features are:

* Element access through ``operator[]`` (indices 0, 1, 2), plus a three-argument constructor.
* Named static factories: ``zeros()``, ``ones()``, ``unit(dir)`` (a basis vector along a given
  axis), ``min()``, ``max()``, and ``infinity()``.
* The usual arithmetic operators, including both scalar and component-wise ``*``/``/``, and their
  in-place counterparts, plus a scalar-left multiplication/division as free functions.
* ``cross`` and ``dot`` products, mirrored by free-function equivalents, plus free-function
  component-wise ``min``, ``max``, and ``clamp``.
* Full comparison support: ``==``, ``!=``, componentwise ``<``/``>``/``<=``/``>=``, and a
  lexicographic ``lessLX`` for use in ordered containers.
* ``minDir``/``maxDir`` to find the index of the smallest/largest component (optionally by
  magnitude), and ``length``/``length2``.
* A stream insertion operator (``operator<<``) for printing.

For the full API -- every constructor, operator, and free function -- see the Doxygen reference
for `Vec2T <doxygen/html/classEBGeometry_1_1Vec2T.html>`__ and
`Vec3T <doxygen/html/classEBGeometry_1_1Vec3T.html>`__.
