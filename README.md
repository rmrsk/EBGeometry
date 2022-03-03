EBGeometry
----------

Support for acceleration structures and signed-distance functions of tesselated surfaces.
Can be used with embedded-boundary (EB) codes like Chombo amr AMReX.

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

C++ 14

Basic usage
-----------

The library is header-only, simple make EBGeometry.hpp visible to your code and include it.
To clone the code do

   git clone git@github.com:rmrsk/EBGeometry.git

Various examples are given in the Examples folder.
To run one of the examples, navigate to the example and compile and run it.

   cd Examples/Sphere
   g++ -std=c++14 example.cpp -o example.out

The					

Caveats
-------

EBGeometry takes a watertight and orientable surface as input. 
Although EBGeometry does it's best at processing grids that contain self-intersections, holes, and hanging vertices the signed distance function is not well-defined.

License
-------

See LICENSE and Copyright.txt for redistribution rights. 
