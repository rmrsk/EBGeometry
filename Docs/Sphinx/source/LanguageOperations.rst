.. _Chap:LanguageOperations:

Operations
==========

Every built-in operation, with a call to copy. An operation takes shapes and gives one; the method form
``a.op(args)`` means ``op(a, args)``, so that a chain reads in order: ``box(lo, hi).rotate(30, "z")``. In the
calls, ``a`` and ``b`` stand for shapes. :ref:`Chap:ConstructiveSolidGeometry` describes what each does to the
distance, and which keep it exact. Every operation below is used once in ``Studio/Models/gallery`` (the rows behind
the shapes in the figure of :ref:`Chap:LanguageShapes`).

Combining shapes
----------------

.. list-table::
   :header-rows: 1
   :widths: 22 45 33

   * - Operation
     - A call
     - What it gives
   * - ``union``
     - ``union(a, b, c)``: two shapes or more
     - the solid inside any of them
   * - ``intersection``
     - ``intersection(a, b)``: two shapes or more
     - the solid inside all of them
   * - ``difference``
     - ``difference(a, b, c)``: a shape, then one or more to remove from it
     - ``a`` with the others cut out
   * - ``smooth_union``
     - ``smooth_union(a, b, smooth = 0.3)``: two shapes or more, smooth, blend = ``"polynomial"`` or
       ``"exponential"``, bvh = ``false``
     - a union whose seams are filleted over about ``smooth``
   * - ``smooth_intersection``
     - ``smooth_intersection(a, b, smooth = 0.2)``: two shapes or more, smooth, blend
     - an intersection with rounded edges
   * - ``smooth_difference``
     - ``smooth_difference(a, b, smooth = 0.2)``: a shape, one or more to remove, smooth, blend
     - a difference with rounded edges
   * - ``bvh_union``
     - ``bvh_union(a, b, c)``: two shapes or more
     - a union evaluated through a bounding volume hierarchy: for many shapes
   * - ``bvh_difference``
     - ``bvh_difference(a, b, c)``: a shape, then one or more to remove
     - a difference whose removed shapes are searched through a bounding volume hierarchy

A plain ``union`` evaluates every shape at every point; ``bvh_union`` visits only the shapes near it, which pays from
a few tens of shapes on (``smooth_union(..., bvh = true)`` does the same for a smooth one). Each counts toward the
nesting of calls a tape allows (see :ref:`Chap:LanguageReference`).

Moving shapes
-------------

.. list-table::
   :header-rows: 1
   :widths: 22 45 33

   * - Operation
     - A call
     - What it gives
   * - ``translate``
     - ``translate(a, (0, 1, 0))``: a, shift
     - ``a`` moved
   * - ``rotate``
     - ``rotate(a, 30, "y")``: a, degrees, axis (``"x"``, ``"y"``, ``"z"`` or a vector)
     - ``a`` turned about an axis through the origin
   * - ``scale``
     - ``scale(a, 0.6)``: a, factor
     - ``a`` scaled about the origin, by the same factor along every axis (a negative factor also reflects it
       through the origin)
   * - ``reflect``
     - ``reflect(a, "y")``: a, axis
     - ``a`` mirrored in the coordinate plane through the origin normal to the axis

Changing shapes
---------------

.. list-table::
   :header-rows: 1
   :widths: 22 45 33

   * - Operation
     - A call
     - What it gives
   * - ``complement``
     - ``complement(a)``: a
     - inside and outside swapped (unbounded: cut it, ``intersection(box(...), complement(a))``)
   * - ``offset``
     - ``offset(a, 0.3)``: a, offset
     - ``a`` grown by ``offset`` (shrunk when negative), its edges rounded
   * - ``annular``
     - ``annular(a, 0.1)``: a, delta
     - a shell of half-thickness ``delta`` around ``a``'s surface
   * - ``elongate``
     - ``elongate(a, (0.5, 0, 0))``: a, elongation
     - ``a`` stretched, a box of half-lengths ``elongation`` inserted at the origin
   * - ``blur``
     - ``blur(a, 0.2)``: a, blur, alpha = 0.5
     - ``a`` averaged over ``blur``: corners and edges softened
   * - ``mollify``
     - ``mollify(a, 0.2, 3)``: a, dist, samples (1 to 64 per axis)
     - ``a`` averaged over a cube of half-width ``dist``, smoother than ``blur`` and dearer
   * - ``repeat``
     - ``repeat(a, (1, 1, 1), (1, 1, 1), (2, 2, 2))``: a, period, repeat_lo, repeat_hi
     - copies of ``a`` one ``period`` apart: ``repeat_lo`` tiles down each axis, ``repeat_hi`` up, plus the original

Labeling shapes
---------------

.. list-table::
   :header-rows: 1
   :widths: 22 45 33

   * - Operation
     - A call
     - What it gives
   * - ``label``
     - ``a.label("skin")``, ``a.label(xmin = "inlet", xmax = "outlet")``, or both
     - ``a``, its surface (or its parts, by name) labeled, so that surface groups can select them (see
       :ref:`Chap:LanguageModels`)

A label names a shape's own parts only, before it is moved or combined: ``box(...).label(xmin = "inlet")``, not
``translate(box(...), v).label(xmin = "inlet")``. A shape's parts are listed in :ref:`Chap:LanguageShapes`.
