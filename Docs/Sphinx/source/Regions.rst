.. _Chap:Regions:

Regions, materials and domains
==============================

A union is one solid. A copper core inside a plastic jacket, joined by a union, is a single body with one
outer surface, and the surface between the core and the jacket does not exist. A program that computes in
such an object (a field in the jacket and the air around it, a flow past it, heat through it) needs more:
which material each point is in, the surfaces where two materials meet, and which of those surfaces carry
which condition. EBGeometry describes this with *regions* and *domains*; it does not solve anything
itself.

.. _Fig:Regions:
.. figure:: /_static/Regions.png
   :width: 95%
   :align: center

   A cable in a box of air, in a cross-section. (a) Three regions, each of a material, over the outside,
   and the pieces of surface between them. (b) A domain computing in the air and the jacket: two
   subdomains, the core and the outside excluded. The surface between the two subdomains is an
   interface, and the surfaces where the domain meets the excluded regions are its boundary.

Regions
-------

An :term:`overlay` of shapes makes an object of :term:`regions <region>`: each shape is a region, and where
shapes overlap, the later one lies on top. In :numref:`Fig:Regions` (a) the air is a box, the jacket a
cylinder over it, and the core a thinner cylinder over the jacket: the jacket's region is the jacket minus the
core, the air's the box minus both. The regions tile the object, with no gaps and no overlaps, so that every
point is in exactly one of them. The space beyond every shape is the *outside*, region 0. A medium around an
object (the air around the cable) is a region like the others, the first of the overlay, whose outer surface
bounds what a program computes in.

Each region has a function of its own, negative exactly inside it, whose zero level is the region's whole
surface, the interfaces with its neighbors included: the function a cut-cell code cuts for that region. The
object as a whole is still one shape, the union of its regions, so it composes, renders and ray traces as
any shape does.

An overlay may be moved, clipped or drilled (as the first operand of an intersection or a difference, which
remove space from every region), and placed in another overlay as one of its regions: its regions then take
its place in the outer overlay's order and stay together as a *region group*. Anything that would make it
unclear which region a point belongs to, such as a union of an overlay with another solid, or an offset or a
blend of one, is rejected when the object is built.

Regions that touch
------------------

Parts of a model often meet face to face: two blocks side by side, an electrode seated on an insulator. Parts
drawn in different tools, or meshes exported from CAD, are rarely exactly flush. They overlap a little, or a
hair-thin gap is left between them. A gap is a layer of the outside, too thin for any grid to resolve, but it
still separates the two parts: a cut-cell code would see the outside between them, and the two faces would be
two surfaces where the model means one.

So two regions that come closer than a small *interface tolerance* are *welded*. The gap between them is
filled by the earlier region, up to the later one's face, and that face is the one interface between the two,
seen from either side: both regions' functions are zero on it, and a query from either region names the same
surface, with the other region and its material across it. Overlapping and exactly flush parts get the same
interface. Only faces that face each other across a gap are welded. Faces that lie side by side in one plane,
such as the end of a channel flush with the end of the block it is bored through, are not.

By default the tolerance is 0: only parts that overlap or are exactly flush are welded, and every gap stays
open. Which gaps are flaws and which are geometry depends on the model (a narrow channel between two parts may
be real), so a gap is closed only when the model or the program says how wide a gap may be closed: a length
small enough to stay below what a grid resolves. Two parts that nearly touch at a point (two spheres, a cylinder
lying on a plane) are then joined by a thin neck where they come within the tolerance. Gaps wider than the
tolerance are left as they are, as geometry, but they can be found and reported, so that a model with parts
meant to touch can be fixed, or given a tolerance that closes them.

Materials
---------

A region is named for *which* part of the object it is (the jacket) and labeled with a :term:`material`, *what*
it is made of (PTFE). Two regions of the same material are still two regions: an inner and an outer
insulation layer, say, may differ in their conditions though both are PTFE. A material carries no numbers in
EBGeometry; it is a name a program attaches properties to, in its own code or in a model file (see
:ref:`Chap:Language`).

Pieces of surface
-----------------

A surface between two regions has a region, and a material, on each side. Where a shape's surface separates
two regions, it is a :term:`piece` of the object's surface: the jacket's cylinder between the jacket and the air
is one piece, and between the jacket and the core (where the core's surface lies) there is none, since the
core's own cylinder is the surface there. A piece is named by the shape it comes from and the regions on
its two sides; it is the unit a domain assigns conditions to, and the thing a viewer picks when a surface is
clicked.

The sides belong to the point, not to the shape: one shape's surface can separate different regions in
different places. A ball centered on another ball's surface faces the air on one half and the other ball on
the other half, and is two pieces.

Domains
-------

A :term:`domain` is what one computation runs on (:numref:`Fig:Regions` (b)). It has two parts.

**Subdomains.** Each subdomain is a set of regions the computation runs in, usually one, sometimes several (a
region group, or every region of a material). Regions in no subdomain are *excluded*: the computation does
not run there (a conductor held at a potential, the outside). Every point is therefore in one subdomain, or
excluded, and each subdomain, and the excluded regions together, has a function negative exactly there. A
cut-cell code samples these at the nodes of its grid: an edge whose two ends are in different subdomains (or
one is excluded) crosses a surface, and the root of the right function along the edge is where.

**Roles.** From the sides of a surface at a point, the domain gives it a role:

* a **boundary**, with one side in the domain and the other excluded (the box's outer faces, and the core's
  surface, in the figure);
* an **interface**, with a different subdomain on each side (the jacket's outer surface);
* **internal**, with two regions of the same subdomain on its two sides, which a computation usually does
  not cut;
* or none, when neither side is in the domain.

**Surface groups.** A domain also sorts the surfaces it touches into *surface groups*, one per condition it
applies: "inlet", "wall", "held at 0 V". A surface group is a list of *selectors*, each saying what a
surface is, never where: a piece, every surface with a surface label (see :ref:`Chap:LeavesAndSurfaces`), the
interface between two regions, every surface of a region, the surfaces between two materials, or *the
rest*, every surface no other selector holds. The selectors of a domain are tried in the order they were
added, across all its groups, and the first that holds a surface puts it in its group; the rest is tried
last, wherever it was added. So a group of a few special pieces can be followed by a broader one that takes
what remains.

A domain is *complete* when every piece of surface it touches is in a surface group. EBGeometry checks that
from the object's description alone, listing every piece the object may have (a few listed may turn out to
have no area) and how far the surface groups hold each one: wholly, partly (some faces of a box or a mesh)
or not at all. It can also find which pieces the object actually has, by crossing it with rays.

Several domains can share one object, each a different computation: the same air is the fluid of a flow
computation and a dielectric of an electrostatic one, and the same surface can be a boundary of one and an
interface of the other.

Connected components
--------------------

A region is a part of space, not necessarily one piece of it. Two balls in one region may or may not touch, a
difference can cut a bar into three, and a hollow shell seals off a cavity that is part of the outside: the
expression says what each region is, never how many pieces it has. A :term:`connected component` is one such
piece: a connected part of space all in one region (or in one subdomain, or in one material). A solver needs to
know about them: an enclosed pocket of gas, cut off from the rest, needs its own initial state, a conductor
touching nothing needs its own potential, and many solvers ask for a point in each piece to name it.

Only the geometry decides, so EBGeometry finds the components by searching space at a resolution *h*, the
size of the smallest cells of the search, which are placed only near surfaces. For each component it gives a
point inside it, its volume and box (approximate, within about *h*), the components it touches, and whether
it reaches the box searched; one that does not is enclosed, a cavity or a floating body.

**What h means.** Two pieces joined by a neck thinner than *h*, or separated by a wall thinner than *h*, may
count as one, and a piece thinner than *h* may be missed. With *h* the cell size of the solver the components
are for, the components found are the ones that solver sees, which is usually what is wanted: a gap the solver
cannot resolve does not separate two volumes for it.

**Memory.** The search keeps its cells, so its memory grows like its time, with the area of the surfaces over *h*
squared: halving *h* takes about four times as much. It stops at a cap on the cells (with a message saying so), set
by default to keep it under ten gigabytes; a finer search needs a smaller box or a raised cap, both options of the
C++ call (see :ref:`Chap:ImplemRegions`).

The search needs the functions to be distances or bounds on them (a :term:`distance quality` other than
``NotADistance``): it shows
that a cell lies wholly in one region from the distance to its surface, and an object whose value is only a
sign is rejected.

Along a ray
-----------

A particle crossing the object crosses from region to region. The first point along a ray, or along a
particle's step, where the region changes is found exactly where the shapes have exact ray forms (see
:ref:`Chap:RayTracing`), with the surface crossed, its surface labels, and the regions left and entered. A ray can
also be asked where it first enters or leaves a set of regions, passing through the surfaces between them:
what a renderer needs to show some regions and see through the others.

:ref:`Chap:ImplemRegions` describes the classes and calls, and :ref:`Chap:ExampleMaterials` builds the
cable, its regions and a domain, and renders them.
