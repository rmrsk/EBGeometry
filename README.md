EBGeometry
----------

A compact code for computing signed distance functions to watertight and orientable surface grids. 
Can be used with embedded-boundary (EB) codes like Chombo or AMReX.

EBGeometry is a compact code for creating signed distance functions from watertight 3D surface tesselations.
The tesselations must consist of planar polygons, but these polygons are not necessarily restricted to triangles.
Internally, the surface mesh is stored in a doubly-connected edge list (DCEL), i.e. a half-edge data structure. 
On watertight and orientable grids, the distance to any feature (facet, edge, vertex) is well defined, and can naively be computed by computing the distance to every facet. 

EBGeometry provides bounding volume hierarchies (BVHs) for bounding geometric primitives in space.
The BVHs are tree structures which permit accelerated closest-point searches.
We point out that the BVHs in EBGeometry are shallow implementations without deep performance optimizations. 
In the DCEL context the BVHs are used for bounding the facets on the surface mesh, but there are no fundamental limitations on which objects that can be bounded.
Thus, multiple objects (e.g., surface grids or analytic functions) can also be bound in the BVHs.
Querying the distance to the mesh through the BVH is much faster than the naive approach. 
On average, if the mesh consists of N facets then a BVH has O(log(N)) complexity while a direct search has O(N) complexity. 

Requirements
------------

* A C++ compiler which supports C++14.
* EBGeometry takes a watertight and orientable surface as input (only PLY files currently supported).
  Although EBGeometry does it's best at processing grids that contain self-intersections, holes, and hanging vertices the signed distance function is not well-defined.

Basic usage
-----------

The library is header-only, simple make EBGeometry.hpp visible to your code and include it.
To clone the code do

    git clone git@github.com:rmrsk/EBGeometry.git

Various examples are given in the Examples folder.
To run one of the examples, navigate to the example and compile and run it.

    cd Examples/Basic/Sphere
    g++ -std=c++14 main.cpp

All the examples take the following steps that are specific to EBGeometry:

1. Parse a surface mesh into a DCEL mesh object.
2. Partition the DCEL mesh object in a bounding volume hierarchy.
3. Create direct and BVH-accelerated signed distance functions and compute the distance to the mesh.

More complex examples that use Chombo or AMReX will also include application-specific code. 

Advanced usage
--------------

For more advanced usage, users can supply their own file parsers (only PLY files are currently supported), provide their own bounding volumes, or their own BVH partitioners.
EBGeometry is not too strict about these things, and uses rigorous templating for ensuring that the EBGeometry functionality can be extended.

License
-------

See LICENSE and Copyright.txt for redistribution rights. 
