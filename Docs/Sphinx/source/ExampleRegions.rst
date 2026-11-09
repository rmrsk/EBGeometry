.. _Chap:ExampleRegions:

Regions
=======

Several materials in one object: a cable, a copper core inside a PTFE jacket, in air, cut open by a slab. The
source is at :file:`Examples/Regions/main.cpp`.

.. figure:: /_static/example_Regions.jpg
   :width: 60%
   :align: center

   The cable colored by region: the jacket violet, the core green, seen through the cut.

Regions, not one solid
----------------------

A union makes one solid of its operands, and the core's surface would vanish inside the jacket.
``Overlay(jacket, core, "air")`` makes each operand a region instead, later ones on top where they overlap, and
the outside the background, region 0 (see :ref:`Chap:ClosestPrimitive`). ``MaterialLabel()`` names a region's
material. A ``Difference`` with the Overlay first cuts every region at once.

.. code-block:: c++

   const auto jacket = MaterialLabel(CylinderSDF<T>(a, b, T(1)), "PTFE");
   const auto core   = MaterialLabel(CylinderSDF<T>(c, d, T(0.4)), "copper");

   const Expr<T> object = Difference(Overlay(jacket, core, "air"), slab);

Which region, and each region's function
----------------------------------------

A tape listing ``TapeFeature::Regions`` says which region a point is in, and gives each region's implicit
function: negative inside the region, zero on its whole surface, the interface with the other regions included.
That function is what a cut-cell code cuts for one region; the whole object is still ``signedDistance(p)``.

.. code-block:: c++

   const Tape<T, TapeFeature::Regions> tape(pool, object);

   tape.region(p);              // 0 air, 1 the jacket, 2 the core
   tape.signedDistance(p, 1);   // the jacket's function

What a solver cuts
------------------

A solver says which regions it computes in (``Phases``), and each surface is then, at a point, a boundary of it
(one side solved, its condition given by its surface label), an interface (two phases meet), or nothing to it.
``closestSurface(p, region)`` names the surface and the regions on its two sides there. For the cable, the outer
surface is a boundary to a flow solver in the air and an interface to an electrostatic solver in the air and the
jacket, while the core's surface is that solver's conductor boundary.

.. code-block:: c++

   Phases electrostatics;            // the air and the jacket; the core is a conductor
   electrostatics.addPhase({0});
   electrostatics.addPhase({1});

   const SurfaceRef<T> s = tape.closestSurface(p, 1);   // a surface of the jacket, with its two sides at p
   electrostatics.role(s);                               // Interface on the outside, Boundary on the core

Looking at the regions
----------------------

``RegionView`` shows a tape's regions as a shape whose surfaces are the regions' own, the internal ones between
two regions included: along a camera ray, its crossings are the changes of region that touch a visible region,
found exactly (``firstRegionChange()``). What it shows is part of the render settings, ``Settings::m_regions``
(`RegionSettings <doxygen/html/structEBGeometry_1_1RegionSettings.html>`__): the regions shown, what a surface is
colored by (region, material, surface label, surface, part, interface, or the group of a solver's scheme), the
surfaces shown alone or seen through, and the scheme itself. The example sets the electrostatic scheme: every
conductor surface (dirichlet) and the jacket's outside (jump).

In the viewer, K cycles the coloring; V shows one region at a time, Shift + V hides the picked one, and Ctrl + V
shows them all; T sees through the picked surface (the core's surface inside the jacket shows), and L shows only
the surfaces like the picked one. Hidden regions cast no shadow.
