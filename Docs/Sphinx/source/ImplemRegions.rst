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

* ``region(p)`` is the last overlay operand whose solid holds the point, or 0. Regions closer than the
  interface tolerance are welded (see :ref:`Chap:Regions`), and the gap between them is the earlier one's.
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

The interface tolerance is set when the tape is built
(`TapeOptions <doxygen/html/structEBGeometry_1_1TapeOptions.html>`__): as a fraction of the diagonal of the
object's box, ``m_interfaceTolerance`` (1e-5 by default; 0 welds only exact contacts), or as a length,
``m_interfaceLength``, which replaces the fraction when it is set. Give a length for parts that are only roughly
flush, or for a real gap that is small next to the whole object and must stay open:

.. code-block:: c++

   TapeOptions options;
   options.m_interfaceLength = 1e-6;              // weld gaps narrower than a micron (in a model in meters)

   const Tape<T> tape(pool, model, options);
   tape.getInterfaceTolerance();                  // the tolerance as a length

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
   * - ``SurfaceSelector::surfaces(first, count)``
     - every point of the surfaces ``first`` to ``first + count - 1``: a node of the expression and everything
       below it (``ExprIndex::Node``'s ``m_firstSurface`` and ``m_numSurfaces``), whatever labels the surfaces
       below it have
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

A domain on its own
-------------------

A domain computes in some regions, but a tape of the whole object evaluates every region at every point. A
solver of one domain can compile a smaller object instead: the domain's regions and their neighbors,
`regionNeighborhood(expr, regions) <doxygen/html/EBGeometry__RegionNeighborhood_8hpp.html>`__, a
`RegionNeighborhood <doxygen/html/classEBGeometry_1_1RegionNeighborhood.html>`__ (host only):

.. code-block:: c++

   const auto own = regionNeighborhood(expr, {index.findRegion("core"), index.findRegion("jacket")});

   const Tape<T> tape(pool, own.object());                          // the regions kept, numbered anew
   const Domain  heat(pool, tape, own.setup(setup));                // the setup written for the whole object

   own.wholeRegion(r);                                               // the whole object's region of its region r
   own.region(wholeRegion);                                          // and back; None for a region dropped

* **Which regions are kept.** Two regions are neighbors when their bounding boxes meet, touching faces and
  edges included. The regions given are kept, and every region whose box meets the box of one of them; every
  other region is dropped. A region given with the background (region 0) keeps them all, since the
  background meets every region.
* **The boxes are a conservative stand-in.** No region that touches one given is ever dropped, so every
  surface of the regions given keeps the regions on both of its sides, its labels and its materials. But a
  region whose box meets theirs is kept even where the region itself does not touch them: a region in the
  hollow of an L-shaped one, or one whose box is mostly cut away by later regions. Such a region costs time,
  never correctness. A domain gains most when its regions are a few among many, apart from the rest, and
  gains nothing when one of them is a medium whose box holds every other region.
* **What stays the same.** In the regions given and around them, every subdomain function has the same value as
  on the whole object's tape, and everywhere the same sign; ``closestSurface()`` finds the same surfaces, with
  the same regions on their two sides. Far from them, in the excluded regions, the values may differ. This
  holds when both tapes weld regions within the same length: the default tolerance is a fraction of the
  object's box, and the smaller object's box is smaller, so its tape is built with the whole tape's tolerance
  as a length (``options.m_interfaceLength = whole.getInterfaceTolerance()``).
* **What is numbered anew.** The kept regions keep their order and are numbered from 1; ``region()``,
  ``wholeRegion()``, ``surface()`` and ``wholeSurface()`` map regions and surfaces both ways. ``setup()``
  translates a setup written for the whole object: regions and surfaces through the maps, materials and labels
  by name. Its subdomains and surface groups keep their numbers, so tables indexed by them still apply; a
  selector naming only something dropped is left out, since it selects nothing of the domain.

A model read from a file does this per domain with ``model.domainObject("conduction")`` (see
:ref:`Chap:ImplemLanguage`).

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
   const auto gaps  = findNearGaps(tape);                        // regions that nearly touch, with the gap

* ``possiblePieces()`` lists, from the expression alone, each surface between each pair of regions it may
  separate (``ExprIndex::getSidesOfLeaf()``, conservative: nothing is left out, and a few listed may not
  occur).
* ``coverageOf()`` says whether the setup's surface groups hold all of a piece, part of it (some faces of a
  box or a mesh) or none, and which group holds it when one holds all of it.
* ``inventory.levelsOf(surface)`` lists the levels a surface belongs to, from the surface itself up to the
  whole object (`SurfaceLevel <doxygen/html/structEBGeometry_1_1SurfaceLevel.html>`__: the node, the range of
  surfaces it covers, its label, what it is), each a ``SurfaceSelector::surfaces()`` range: what a viewer steps
  a pick through.
* ``findSurfacePieces(tape)`` finds the pieces the object actually has, with a point and a normal on each,
  by crossing it with rays on a grid; a piece too small for the grid is missed there, but not by
  ``possiblePieces()``.
* ``findNearGaps(tape, width)`` finds the pairs of regions that a thin layer of the outside separates, wider
  than the interface tolerance (narrower ones are welded) and narrower than ``width`` (a fraction of the box's
  diagonal, 1e-3 by default), with the narrowest gap of each pair and where it is: parts that were probably
  meant to touch.

``ExprIndex::checkLabels(expr, tape)`` warns, among its other checks (see :ref:`Chap:LeavesAndSurfaces`),
about a region with no volume, buried under later regions, and about a material no region shows.

Connected components
--------------------

`connectedComponents() <doxygen/html/EBGeometry__ConnectedComponents_8hpp.html>`__ finds the connected
components (see :ref:`Chap:Regions`) of a tape's regions, of groups of them, or of a domain's subdomains, on
the host:

.. code-block:: c++

   ComponentOptions<T> options;

   options.m_resolution = T(0.02);   // h: the solver's cell size, say
   options.m_box        = box;       // the space searched; empty, the default: the tape's box grown by two cells
   options.m_maxCells   = ComponentOptions<T>::DefaultMaxCells;   // the most cells it keeps, which bounds its memory

   const ConnectedComponents<T> byRegion   = connectedComponents(tape, options);           // group = region
   const ConnectedComponents<T> byMaterial = connectedComponents(tape, materialOf, options); // a group per region
   const ConnectedComponents<T> byDomain   = connectedComponents(domain, options);  // subdomains, then the excluded

   for (const uint32_t c : byRegion.componentsOf(0U)) {           // the outside's components
     const ConnectedComponent<T>& component = byRegion.component(c);

     if (!component.m_reachesBox) {
       // An enclosed cavity: component.m_seed is a point in it, component.m_volume its volume.
     }
   }

* `ConnectedComponents <doxygen/html/classEBGeometry_1_1ConnectedComponents.html>`__ numbers the components by
  group, those reaching the box first, then by decreasing volume. ``componentsOf(group)`` lists those of a
  group, ``neighbors(c)`` the components a component touches, and ``componentAt(point)`` the component holding a
  point (near a surface, within *h*, possibly the one across it).
* Each `ConnectedComponent <doxygen/html/structEBGeometry_1_1ConnectedComponent.html>`__ has its group, an
  approximate volume and box, a seed point inside it, and whether it reaches the box searched.
* The box is the space the components are of: a component that reaches it may continue beyond it. An unbounded
  object (a half-space) needs one.
* The cost grows with the area of the surfaces over *h* squared, and with the number of regions: every cell
  of the search evaluates every region's function once, and keeps the cell. ``m_maxCells`` caps the cells, and so the
  memory: a search that reaches it aborts with a message naming the resolution and the cap (``completeness()`` warns
  instead). ``getNumCells()`` says how many cells it took; the
  ``ConnectedComponents`` performance probe measures it (see :ref:`Chap:PerformanceTesting`).

A model's domains are checked for components with ``Language::completeness(model, tape, domain,
resolution)`` (see :ref:`Chap:LanguageDomains`).

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
