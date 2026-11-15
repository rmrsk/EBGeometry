.. _Chap:ImplemRegions:

Regions and domains
===================

This page lists the calls behind :ref:`Chap:Regions`: building an object of regions, asking a tape about
them, describing a domain and building it for a tape, and checking that its surface groups cover the
object. Everything here is in ``EBGeometry.hpp``. :ref:`Chap:ExampleMaterials` uses all of it on a cable.

.. contents:: On this page
   :local:
   :depth: 1

Regions and materials
---------------------

``Overlay(a, b, ...)`` makes an object of regions from expressions, later ones on top
(``Overlay(std::vector<Expr<T>>{...})`` and a braced list ``Overlay({a, b, c})`` work too).
``RegionName(object, "name")`` names a region and ``MaterialLabel(object, "name")`` gives it a material;
both are expression nodes like ``SurfaceLabel()`` (see :ref:`Chap:ImplemCSG`), and naming an object again
replaces its name. Regions are numbered from 1 in the order of the overlay's operands, 0 being the outside:

.. code-block:: c++

   const auto air    = RegionName(MaterialLabel(BoxSDF<T>(low, high), "air"), "air");
   const auto jacket = RegionName(MaterialLabel(SphereSDF<T>(center, T(1.0)), "PTFE"), "jacket");
   const auto core   = RegionName(MaterialLabel(SphereSDF<T>(center, T(0.5)), "copper"), "core");

   const Expr<T> cable = Overlay(air, jacket, core);     // region 0 the outside, 1 the air, 2 the jacket, 3 the core

   const ExprIndex<T> index(cable);
   index.findRegion("core");                             // 3
   index.getMaterialLabelName(index.getMaterialOf(3));   // "copper"

An overlay placed in another overlay is a region group, ``ExprIndex::getRegionGroup()``, and a material
given to the inner overlay is the default of its regions; ``getRegionsOfRegionGroup()`` and
``getRegionsOfMaterial()`` list the regions of a group or a material. An object with no overlay is one
region, region 1. An expression that would make a region unclear (see :ref:`Chap:Regions`) is rejected
when it is built.

Asking a tape
-------------

Every `Tape <doxygen/html/classEBGeometry_1_1Tape.html>`__ answers for its regions, on the host and on a
device:

.. code-block:: c++

   const Tape<T> tape(pool, cable);

   tape.getNumRegions();          // 4: the outside, the air, the jacket, the core
   tape.region(p);                // the region p is in
   tape.signedDistance(p, 2);     // the jacket's function
   tape.regionDistances(p, [&](uint32_t k, T value) { /* every region's function, in one pass */ });

* ``region(p)`` is the last overlay operand whose solid holds the point, or 0.
* ``signedDistance(p, k)`` is region k's function: negative exactly inside it, zero on its whole surface,
  a distance bound that is exact near a smooth piece of surface. ``signedDistance(p)`` remains the whole
  object's.
* ``regionDistances(p, visit)`` gives every region's function at a point at once, cheaper than asking
  region by region.
* ``closestSurface(p, k)`` is region k's surface nearest to ``p``, reported as any
  `SurfaceRef <doxygen/html/structEBGeometry_1_1SurfaceRef.html>`__ is (leaf, part, label; see
  :ref:`Chap:LeavesAndSurfaces`), with the regions on its two sides at that point: ``m_ownerRegion``, the
  region asked about, and ``m_neighborRegion``, the one across. The normal points from the owner to the
  neighbor, and the surface carries the materials of both sides.
* ``firstRegionChange(ray, tMin, tMax)``, or ``firstRegionChange(from, to)`` for a particle's step, finds the
  first point along a ray where the region changes, as a
  `SurfaceHitT <doxygen/html/structEBGeometry_1_1SurfaceHitT.html>`__: the crossing, its normal (into the
  region entered), the surface crossed with its labels, and the regions left (``m_ownerRegion``) and entered
  (``m_neighborRegion``). Crossings closer than the march's tolerance to ``tMin`` are not reported, so a
  particle crossing several layers asks again from the last crossing:

.. code-block:: c++

   for (T t = 0;;) {                                          // a particle through the layers of the cable
     const SurfaceHitT<T> c = tape.firstRegionChange(ray, t, tMax);
     if (!c.m_hit.hit()) break;
     // c.m_surface.m_ownerRegion -> c.m_surface.m_neighborRegion at c.m_hit.m_point
     t = c.m_hit.m_t;
   }

``firstSurface(ray, tMin, tMax, subdomains, subdomain)`` finds where a ray enters or leaves a set of regions
(a subdomain of a ``Subdomains`` table, below), passing through the surfaces between them.

Describing a domain
-------------------

A `DomainSetup <doxygen/html/classEBGeometry_1_1DomainSetup.html>`__ describes a domain: a plain value on the
host, built, copied, compared and edited like any value.

* ``addSubdomain(regions)`` adds a subdomain of the regions listed and returns its number, from 0.
* ``addSurfaceGroup()`` adds a surface group and returns its number, from 0, and ``add(group, selector)``
  gives it a selector. ``remove()`` and ``removeSurfaceGroup()`` take them away again (the groups after a
  removed one move down by one).
* ``role(surface)`` and ``surfaceGroupOf(surface)`` answer for a surface reported with its sides, on the host.

The selectors are `SurfaceSelector <doxygen/html/structEBGeometry_1_1SurfaceSelector.html>`__ values:

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Selector
     - Holds
   * - ``SurfaceSelector::piece(surface)``
     - the piece a surface point is on: its shape's surface between its two regions (what a click picks)
   * - ``SurfaceSelector::surface(s)``
     - every point of surface ``s``, whatever its sides
   * - ``SurfaceSelector::label(id)``
     - every surface with a surface label
   * - ``SurfaceSelector::interface(a, b)``
     - every surface between regions ``a`` and ``b``, either way round
   * - ``SurfaceSelector::region(a)``
     - every surface with region ``a`` on a side
   * - ``SurfaceSelector::materials(m1, m2)``
     - every surface between two materials, either way round (one material: on either side)
   * - ``SurfaceSelector::rest()``
     - every surface no other selector holds
   * - ``selector.withPart(part)``
     - the same, restricted to one part (a box's face, a mesh's face)

A surface is in the group of the first selector that holds it, in the order the selectors were added across
all groups; ``rest()`` is tried after all the others, wherever it was added. The cable's electrostatics, the
air and the jacket computed in, the core excluded:

.. code-block:: c++

   DomainSetup setup;
   setup.addSubdomain({index.findRegion("air")});        // subdomain 0
   setup.addSubdomain({index.findRegion("jacket")});     // subdomain 1

   const uint32_t conductor = setup.addSurfaceGroup();
   const uint32_t far       = setup.addSurfaceGroup();
   setup.add(conductor, SurfaceSelector::interface(index.findRegion("core"), index.findRegion("jacket")));
   setup.add(far, SurfaceSelector::rest());

Building it for a tape
----------------------

A `Domain <doxygen/html/classEBGeometry_1_1Domain.html>`__ is a setup built for one tape,
``Domain(pool, tape, setup)``: a handle whose tables live in the pool and which holds its tape, so that its
queries take no tape and it cannot be used with one it was not built for. It is moved to a device with
``rebasedView()``, as the tape is, and every query is callable there. Editing the setup afterwards changes
no domain already built: build another.

.. code-block:: c++

   const Domain electrostatics(pool, tape, setup);

   // At a node, in a kernel: every subdomain's function, then the excluded regions'.
   T values[3];                                       // at least electrostatics.getNumSubdomainFunctions()
   electrostatics.signedDistances(p, values);         // values[0]: the air's, [1]: the jacket's, [2]: excluded
   electrostatics.subdomainAt(p);                         // 0, 1 or Domain<T>::Excluded

   // At a cut cell: the subdomain's surface, its role and its surface group.
   const SurfaceRef<T> s = electrostatics.closestSurface(p, 1);
   electrostatics.role(s);                            // SurfaceRole::Boundary, Interface, Internal or None
   electrostatics.surfaceGroupOf(s);

* ``subdomainAt(p)`` is the subdomain the point is in, or ``Domain<T>::Excluded``.
* ``signedDistance(p, subdomain)`` is a subdomain's function, negative exactly in the subdomain, the function
  to cut: zero only where one of its regions meets one outside it. Given ``Domain<T>::Excluded``, it is the
  excluded regions' function, the negative of all the subdomains' together.
* ``signedDistances(p, values)`` gives them all at once, the subdomains' in order and the excluded regions'
  last, into a buffer of at least ``getNumSubdomainFunctions()`` values (the subdomains plus one).
* ``closestSurface(p, subdomain)`` is the subdomain's surface nearest to ``p``, never a contact between two
  of its own regions; ``role()`` and ``surfaceGroupOf()`` read it.
* ``subdomains()`` and ``surfaceGroups()`` are the domain's tables,
  `Subdomains <doxygen/html/classEBGeometry_1_1Subdomains.html>`__ and
  `SurfaceGroups <doxygen/html/classEBGeometry_1_1SurfaceGroups.html>`__, for code that takes them alone (a
  renderer showing a domain's surface groups).

The tables usually live in the tape's pool, built before it is mirrored: one mirror, one lifetime. A domain
built after the tape's pool was mirrored (surface groups edited while a program runs) may keep its tables in
a pool of its own, ``Domain(otherPool, tape, setup)``; ``rebasedView(tapePool, tablesPool)`` then moves it,
and only the small tables are copied, never the geometry. Both pools, or their mirrors, must outlive the
domain and every copy and view of it.

Checking the surface groups
---------------------------

Whether a domain's surface groups hold every piece of surface it touches is checked on the host, from the
expression's index (`EBGeometry_SurfacePieces.hpp <doxygen/html/EBGeometry__SurfacePieces_8hpp.html>`__):

.. code-block:: c++

   const SurfaceInventory inventory(index);                      // the surfaces, their parts, labels and sides

   for (const PossiblePiece& piece : possiblePieces(inventory)) {   // every piece the object may have
     const PieceCoverage c = coverageOf(piece, inventory, setup);   // Coverage::Full, Partly or None
   }

   const auto found = findSurfacePieces(tape);                   // the pieces the object has, with a point on each

* ``possiblePieces()`` lists, from the expression alone, each surface between each pair of regions it may
  separate (``ExprIndex::getSidesOfLeaf()``, conservative: nothing is left out, and a few listed may not
  occur).
* ``coverageOf()`` says whether the setup's surface groups hold all of a piece, part of it (some faces of a
  box or a mesh) or none, and which group holds it when one holds all of it.
* ``findSurfacePieces(tape)`` finds the pieces the object actually has, with a point and a normal on each,
  by crossing it with rays on a grid; a piece too small for the grid is missed there, but not by
  ``possiblePieces()``.

``ExprIndex::checkLabels(expr, tape)`` warns, among its other checks (see :ref:`Chap:LeavesAndSurfaces`),
about a region with no volume, buried under later regions, and about a material no region shows.

Names for a viewer
------------------

`RegionNames <doxygen/html/classEBGeometry_1_1RegionNames.html>`__ holds the names a viewer shows: the
regions, materials and labels of an index, the surfaces named as a program likes ("cylinder, model.ebg:6"),
and the domains, registered with ``addDomain("electrostatics", setup)`` with their subdomains' and surface
groups' names (``setSubdomain()``, ``setSurfaceGroup()``), and colors given to names (``setColor()``). A
`RegionView <doxygen/html/classEBGeometry_1_1RegionView.html>`__ of a tape is the shape the renderer draws
for an object of regions: it shows some regions and hides or sees through others, as its
``RegionSettings`` say. :ref:`Chap:ImplemRender` and :ref:`Chap:Viewer` describe how they are drawn and
edited.
