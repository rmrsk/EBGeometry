EBGeometry
----------

Support for signed-distance functions of tesselated surfaces. Used with embedded-boundary (EB) codes.

EBGeometry is a compact code for creating signed distance functions from watertight 3D surface tesselations.
The surface mesh is stored in a doubly-connected edge list (DCEL), i.e. a half-edge data structure.
Since the distance to any feature (facet, edge, vertex) in a surface mesh is well defined, the distance to the mesh can be computed from any point in space.
Naively, this can be done by computing the shortest distance to every facet/edge/vertex in the surface mesh. 
For computing the signed distance one needs to define a suitable normal vector for edges and vertices.

EBGeometry provides bounding volume hierarchies (BVHs) for bounding geometric primitives in space.
The BVHs are tree structures which permit accelerated closest-point searches.
We point out that the BVHs in EBGeometry are shallow implementations without deep performance optimizations. 
In the DCEL context the BVHs are used for bounding the facets on the surface mesh, but there are no fundamental limitations on which objects that can be bounded. 
Querying the distance to the mesh through the BVH is much faster than directly computing the distance.
On average, if the mesh consists of N facets then a BVH has O(log(N)) complexity while a direct search has O(N) complexity. 

Requirements
------------

C++14

Usage
-----

The library is header-only, simple make EBGeometry.hpp visible to your code and include it.
Examples are given in Examples.

Caveats
-------

EBGeometry takes, as input, a watertight and orientable surface.
Although EBGeometry will process grids that contain self-intersections, it does not warn about these, and the signed distance functions is not well-defined either.

License
-------

See LICENSE and Copyright.txt for redistribution rights. 
