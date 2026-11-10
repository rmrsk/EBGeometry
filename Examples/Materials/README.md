Examples/Materials
------------------

Several materials in one object. A cable, a copper core inside a PTFE jacket, in a box of air:
`Overlay(air, jacket, core)` stacks the parts as regions (later ones on top) rather than one solid, so the core
keeps its surface inside the jacket, and `MaterialLabel()` names each region's material. The air is a region like
the others, whose box bounds what a solver computes on; beyond it is the outside, region 0, with no material. An `ExprIndex` lists the
regions and their materials; the tape says which region a point is in
(`region(p)`) and gives each region's own implicit function (`signedDistance(p, region)`), the one a cut-cell
code cuts for that region. The viewer shows the regions' surfaces, internal ones included, through a
`RegionView`: K colors them by region, material, surface label, surface or part; V shows one region at a time,
Shift + V hides the picked one, Ctrl + V shows them all; T on a picked region sees through it. The window opens
with the air hidden and the cable cut along its axis by the section plane, filled by material (X moves the plane).

Building
--------


