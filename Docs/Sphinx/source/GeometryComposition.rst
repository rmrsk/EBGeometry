.. _Chap:GeometryComposition:

Composition
===========

Composing objects into one, compiling the composition into a program, asking which part of it a point belongs
to, and objects of several materials. Regions, materials, labels and surface groups are a layer of meaning on
the geometry: what a part of it is, and what a solver does with it.

* :ref:`Chap:ConstructiveSolidGeometry`: unions, intersections, differences, smooth blends and transforms, and
  what each does to the distance.
* :ref:`Chap:Tape`: the program a composition is compiled into, its sub-programs and its limits.
* :ref:`Chap:LeavesAndSurfaces`: which leaf, surface and part of a composition decides the value at a point,
  and labels.
* :ref:`Chap:Regions`: regions, materials, and the domains a solver computes in.
* :ref:`Chap:ImplemCSG`: the analytic shapes, the expression builder, the tape, fused composition and BVH
  unions.
* :ref:`Chap:ImplemRegions`: regions, materials, domains and their checks.
* :ref:`Chap:ImplemLanguage`: reading models written in the .ebg language, and their properties as tables.

.. toctree::
   :hidden:

   CSG.rst
   Tape.rst
   LeavesAndSurfaces.rst
   Regions.rst
   ImplemCSG.rst
   ImplemRegions.rst
   ImplemLanguage.rst
