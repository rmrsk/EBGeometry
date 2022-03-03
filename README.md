EBGeometry
----------

Support for signed-distance functions of tesselated surfaces. Used with embedded-boundary (EB) codes.

Usage
-----

The library is header-only, simple make EBGeometry.hpp visible to your code and include it.

Features
--------

EBGeometry is a compact code for creating signed distance functions from watertight 3D surface tesselations.
The surface mesh is stored in a doubly-connected edge list (DCEL), i.e. a half-edge data structure.
Since the distance to any feature (facet, edge, vertex) in a surface mesh is well defined, the distance to the mesh can be computed from any point in space.
For computing the *signed

License
-------

See LICENSE and Copyright.txt for redistribution rights. 
