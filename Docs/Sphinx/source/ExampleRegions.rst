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

A tape says which region a point is in, and gives each region's implicit
function: negative inside the region, zero on its whole surface, the interface with the other regions included.
That function is what a cut-cell code cuts for one region; the whole object is still ``signedDistance(p)``.

.. code-block:: c++

   const Tape<T> tape(pool, object);

   tape.region(p);              // 0 air, 1 the jacket, 2 the core
   tape.signedDistance(p, 1);   // the jacket's function

What a solver cuts
------------------

A solver says which regions it computes in (``Subdomains``), and each surface is then, at a point, a boundary of it
(one side solved, its condition given by its surface label), an interface (two subdomains meet), or nothing to it.
``closestSurface(p, region)`` names the surface and the regions on its two sides there. For the cable, the outer
surface is a boundary to a flow solver in the air and an interface to an electrostatic solver in the air and the
jacket, while the core's surface is that solver's conductor boundary.

.. code-block:: c++

   Subdomains electrostatics;            // the air and the jacket; the core is a conductor
   electrostatics.addSubdomain({0});
   electrostatics.addSubdomain({1});

   const SurfaceRef<T> s = tape.closestSurface(p, 1);   // a surface of the jacket, with its two sides at p
   electrostatics.role(s);                               // Interface on the outside, Boundary on the core

Looking at the regions
----------------------

``RegionView`` shows a tape's regions as a shape whose surfaces are the regions' own, the internal ones between
two regions included: along a camera ray, its crossings are the changes of region that touch a visible region,
found exactly (``firstRegionChange()``). What it shows is part of the render settings, ``Settings::m_regions``
(`RegionSettings <doxygen/html/structEBGeometry_1_1RegionSettings.html>`__): the regions shown, what a surface is
colored by (region, material, surface label, surface, part, interface, or the group of a solver's scheme), the
surfaces shown alone or seen through, and the scheme itself. The example registers two domains, descriptions of
what two downstream solvers compute on (``Domain``): electrostatics in the air and the jacket, every conductor
surface dirichlet and the jacket's outside a jump; and flow in the air, every surface of it a wall.

In the viewer, the panel's "Color by" row sets the coloring (K cycles it); V shows one region at a time, Shift + V hides the picked one, and Ctrl + V
shows them all; T sees through the picked surface (the core's surface inside the jacket shows), and L shows only
the surfaces like the picked one. Hidden regions cast no shadow.

A click picks the piece of surface under the cursor: its surface, part, and the regions on its two sides
(``RegionView::pieceAt()`` gives it to a program). The panel at the top left shows it, with the ways to add it to
the current surface group of the scheme, each with how many pieces it takes in (this piece only, all of its surface,
all surfaces of its region, or everything between its two regions), or to remove it; below, the regions, each shown or hidden by its box and highlighted by a click on its name (the background
too), and the surface groups with their selectors, a
click making one current and highlighting its surfaces, ``[Hide]`` hiding them, ``[Rename]`` typing its name, ``[Delete]`` and ``[x]`` removing them, ``[+ New surface
group]`` starting one (its name typed first); last, the
count of pieces and of those in no surface group, with ``[Next unassigned]`` to go to the next of these, until
every piece has a surface group. Colored by surface group, those in none are gray; C prints the scheme as C++, to paste into a program. The window names everything by the
`RegionNames <doxygen/html/classEBGeometry_1_1RegionNames.html>`__ passed to ``view()``: built from the expression's
index (regions by their names and materials, "2 core (copper)", surfaces by their labels), with the surface groups
named by the program; the example also gives the materials and the surface groups colors (``setColor()``), and
opens colored by material.
