.. _Chap:LanguageShapes:

Shapes
======

A shape is a call: its C++ class name without ``SDF``, in snake_case (``RoundedBoxSDF`` is ``rounded_box``), with the
constructor's parameters, without ``a_``, in snake_case and in order. Arguments are positional, then by keyword
(``radius = 0.5``).

Bounded shapes
--------------

A shape's parts are its faces, which ``.label(part = "name")`` labels one by one (see :ref:`Chap:Language`). A
vector is ``(x, y, z)``; an angle is in degrees.

.. list-table::
   :header-rows: 1
   :widths: 20 50 30

   * - Shape
     - A call
     - Parts
   * - ``sphere``
     - ``sphere((0, 0, 0), 1)``: center, radius
     -
   * - ``box``
     - ``box((-1, -1, -1), (1, 1, 1))``: lo_corner, hi_corner
     - xmin, xmax, ymin, ymax, zmin, zmax
   * - ``rounded_box``
     - ``rounded_box((0, 0, 0), (2, 2, 2), 0.25)``: center, size, curvature
     - xmin, xmax, ymin, ymax, zmin, zmax
   * - ``torus``
     - ``torus((0, 0, 0), 0.8, 0.25)``: center, major_radius, minor_radius (in the xz plane)
     -
   * - ``capped_torus``
     - ``capped_torus((0, 0, 0), 0.8, 0.2, 270)``: center, major_radius, minor_radius, angle
     - tube, end1, end2
   * - ``cylinder``
     - ``cylinder((0, -1, 0), (0, 1, 0), 0.6)``: center1, center2, radius
     - side, cap1, cap2
   * - ``rounded_cylinder``
     - ``rounded_cylinder((0, 0, 0), 0.7, 0.2, 1.6)``: center, radius, curvature, height (along y)
     - side, cap1 (at -y), cap2 (at +y)
   * - ``capsule``
     - ``capsule((0, -0.6, 0), (0, 0.6, 0), 0.4)``: tip1, tip2, radius
     - side, tip1, tip2
   * - ``cone``
     - ``cone((0, 1, 0), 1.6, 50)``: tip, height, angle (opening toward -y)
     - side, base
   * - ``capped_cone``
     - ``capped_cone((0, -0.8, 0), (0, 0.8, 0), 0.8, 0.3)``: center1, center2, radius1, radius2
     - side, cap1, cap2
   * - ``round_cone``
     - ``round_cone((0, -0.6, 0), (0, 0.7, 0), 0.6, 0.25)``: center1, center2, radius1, radius2
     - side, end1, end2
   * - ``ellipsoid``
     - ``ellipsoid((0, 0, 0), (1, 0.6, 0.4))``: center, radii
     -
   * - ``polygon_prism``
     - ``polygon_prism((0, -0.8, 0), (0, 0.8, 0), 0.8, 6, (1, 0, 0))``: center1, center2, radius, sides (3 to 4096),
       vertex_direction
     - side0 ... side(n-1), cap1, cap2
   * - ``tube``
     - ``tube((0, -0.8, 0), (0, 0.8, 0), 0.4, 0.8)``: center1, center2, inner_radius, outer_radius
     - outer, inner, end1, end2
   * - ``cut_sphere``
     - ``cut_sphere((0, 0, 0), 1, 0.4)``: center, radius, height (of the cut above the center)
     - sphere, cut
   * - ``cut_hollow_sphere``
     - ``cut_hollow_sphere((0, 0, 0), 1, 0.4, 0.15)``: center, radius, height, thickness
     - shell, rim

Unbounded shapes
----------------

These reach to infinity. Used as a model's object they would fill the scene, so they are cut by a bounded shape
(``intersection(box(...), plane(...))``), or used to cut one.

.. list-table::
   :header-rows: 1
   :widths: 20 50 30

   * - Shape
     - A call
     - Parts
   * - ``plane``
     - ``plane((0, 0, 0), (0, 1, 0))``: point, normal (pointing outside: the solid is below)
     -
   * - ``infinite_cylinder``
     - ``infinite_cylinder((0, 0, 0), 0.5, "y")``: center, radius, axis (``"x"``, ``"y"`` or ``"z"``)
     -
   * - ``infinite_cone``
     - ``infinite_cone((0, 1, 0), 60)``: tip, angle (opening toward -y)
     -
   * - ``wedge``
     - ``wedge((0, 1, 0), 60)``: tip, angle (a knife edge along z, pointing along +y)
     - xmin, xmax
   * - ``perlin``
     - ``perlin(0.3, (2, 2, 2), 0.5, 3)``: noise_amplitude, noise_frequency, noise_persistence, noise_octaves; noise, not
       a distance
     -

Meshes
------

.. list-table::
   :header-rows: 1
   :widths: 20 50 30

   * - Shape
     - A call
     - Parts
   * - ``mesh``
     - ``mesh("part.stl")``: file, kind = ``"triangles"`` or ``"polygons"``, groups = ``true``, on_defect = ``"warn"``
       or ``"throw"``
     - its file's face groups

A ``mesh`` reads its file (STL, PLY, OBJ, VTK) relative to the file it is written in, once per model however often it
is used. ``"triangles"`` makes a `TriMeshSDF <doxygen/html/classEBGeometry_1_1TriMeshSDF.html>`__, ``"polygons"`` a
`MeshSDF <doxygen/html/classEBGeometry_1_1MeshSDF.html>`__. With ``groups = true`` its face groups (an OBJ's ``g`` and
``o``, ...) are its parts, each labeled by its group's name. A mesh with a topology defect (an edge shared by more than
two faces, faces folding back onto each other) is read with a warning, its distance's sign unreliable near the defect;
``on_defect = "throw"`` makes it an error. A face that visits a vertex twice is an error either way.
