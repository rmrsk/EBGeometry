Examples/Regions
----------------

Several materials in one object. A cable, a copper core inside a PTFE jacket, in air, cut open by a slab:
`Overlay(jacket, core, "air")` makes the parts regions (later ones on top) rather than one solid, so the core
keeps its surface inside the jacket, and `MaterialLabel()` names each region's material. An `ExprIndex` lists the
regions and their materials; a tape listing `TapeFeature::Regions` says which region a point is in
(`region(p)`) and gives each region's own implicit function (`signedDistance(p, region)`), the one a cut-cell
code cuts for that region. The viewer shows the regions through a `RegionView`, one color each: V shows one
region at a time, Shift + V hides the picked one, Ctrl + V shows them all.

Building
--------


