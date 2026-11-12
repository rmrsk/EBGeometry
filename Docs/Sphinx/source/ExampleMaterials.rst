.. _Chap:ExampleMaterials:

Materials
=========

Several materials in one object: a cable, a copper core inside a PTFE jacket, in a box of air. The
source is at :file:`Examples/Materials/main.cpp`.

.. figure:: /_static/example_Materials.jpg
   :width: 60%
   :align: center

   The cable cut open by the section plane, the cut filled by material: the copper core, the PTFE jacket, the air
   (blue), and beyond the air's box the outside (gray).

Regions, not one solid
----------------------

A union makes one solid of its operands, and the core's surface would vanish inside the jacket.
``Overlay(air, jacket, core)`` stacks them as regions instead, each on top of the ones before it where they overlap
(see :ref:`Chap:ClosestPrimitive`). ``MaterialLabel()`` names a region's material. The air is a region like the
others, a box around the whole cable, under it: its outer faces bound what a solver computes on. Beyond the box is
the outside, region 0, with no material.

.. code-block:: c++

   const auto air    = MaterialLabel(BoxSDF<T>(low, high), "air");
   const auto jacket = MaterialLabel(CylinderSDF<T>(a, b, T(1)), "PTFE");
   const auto core   = MaterialLabel(CylinderSDF<T>(c, d, T(0.4)), "copper");

   const Expr<T> object = Overlay(air, jacket, core);

Which region, and each region's function
----------------------------------------

A tape says which region a point is in, and gives each region's implicit
function: negative inside the region, zero on its whole surface, the interface with the other regions included.
That function is what a cut-cell code cuts for one region; the whole object is still ``signedDistance(p)``.

.. code-block:: c++

   const Tape<T> tape(pool, object);

   tape.region(p);              // 0 the outside, 1 the air, 2 the jacket, 3 the core
   tape.signedDistance(p, 2);   // the jacket's function

What a solver cuts
------------------

A solver says which regions it computes in (its subdomains, in a ``DomainSetup``), and each surface is then, at a point, a boundary of it
(one side solved, its condition given by its surface label), an interface (two subdomains meet), or nothing to it.
``closestSurface(p, region)`` names the surface and the regions on its two sides there. For the cable, the outer
surface is a boundary to a flow solver in the air and an interface to an electrostatic solver in the air and the
jacket, while the core's surface is that solver's conductor boundary.

.. code-block:: c++

   DomainSetup electrostatics;           // the air and the jacket; the core is a conductor
   electrostatics.addSubdomain({1});
   electrostatics.addSubdomain({2});

   const SurfaceRef<T> s = tape.closestSurface(p, 2);   // a surface of the jacket, with its two sides at p
   electrostatics.role(s);                               // Interface on the outside, Boundary on the core

Looking at the regions
----------------------

``RegionView`` shows a tape's regions as a shape whose surfaces are the regions' own, the internal ones between
two regions included: along a camera ray, its crossings are the changes of region that touch a visible region,
found exactly (``firstRegionChange()``). What it shows is part of the render settings, ``Settings::m_regions``
(`RegionSettings <doxygen/html/structEBGeometry_1_1RegionSettings.html>`__): the regions shown, what a surface is
colored by (region, material, surface label, surface, part, interface, or a domain's surface group), the
surfaces shown alone or seen through, and the surface groups themselves. The example registers two domains' setups,
descriptions of what two downstream solvers compute on: electrostatics in the air and the jacket, every conductor
surface dirichlet, the box grounded and the jacket's outside a jump; and flow in the air, the box its far field and
every other surface of it a wall. It builds the electrostatic ``Domain`` for the tape (``Domain(pool, tape,
setup)``) and samples every label's function at a few nodes, as a cut-cell code does.

The window opens with the air hidden, so that the cable shows, and cut along its axis by the section plane, filled by
material (``RegionSettings::setVisible()`` and the section settings, in the program); V and the panel show the air
again. In the viewer, the panel's "Color by" row sets the coloring (K cycles it); V shows one region at a time, Shift + V hides the picked one, and Ctrl + V
shows them all; T sees through the picked surface (the core's surface inside the jacket shows), and L shows only
the surfaces like the picked one. Hidden regions cast no shadow.

A click picks the piece of surface under the cursor: its surface, part, and the regions on its two sides
(``RegionView::pieceAt()`` gives it to a program). The panel at the top left shows it, with the ways to add it to
the current surface group, each with how many pieces it takes in (this piece only, all of its surface,
all surfaces of its region, or everything between its two regions), or to remove it; below, the regions, each with a box a click cycles through shown, translucent and off, and a name a click
highlights (the background too), and the surface groups with their selectors, a
click making one current and highlighting its surfaces, ``[Translucent]``, ``[Hide]`` and ``[Show]`` cycling them through translucent, hidden and shown, ``[Rename]`` typing its name, ``[Delete]`` and ``[x]`` removing them, ``[+ New surface
group]`` starting one (its name typed first); last, the
count of pieces and of those in no surface group, with ``[Next unassigned]`` to go to the next of these, until
every piece has a surface group. Colored by surface group, those in none are gray; C prints the active domain's setup as C++, to paste into a program. The window names everything by the
`RegionNames <doxygen/html/classEBGeometry_1_1RegionNames.html>`__ passed to ``view()``: built from the expression's
index (regions by their names and materials, "3 core (copper)", surfaces by their labels), with the surface groups
named by the program; the example also gives the materials and the surface groups colors (``setColor()``), and
opens colored by material.
