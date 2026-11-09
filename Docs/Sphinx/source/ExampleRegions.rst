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

Looking at the regions
----------------------

``RegionView`` shows a tape's regions as one shape whose parts are the regions, so the viewer colors by region
(coloring by primitive) and names the region a click picked. V shows one region at a time, Shift + V hides the
picked one, and Ctrl + V shows them all.
