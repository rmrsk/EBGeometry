EBGeometry
----------

A compact code for computing signed distance functions to watertight and orientable surface grids. 
Can be used with embedded-boundary (EB) codes like Chombo or AMReX.

The tesselations must consist of planar polygons, but these polygons are not necessarily restricted to triangles.
Internally, the surface mesh is stored in a doubly-connected edge list (DCEL), i.e. a half-edge data structure. 
On watertight and orientable grids, the distance to any feature (facet, edge, vertex) is well defined, and can naively be computed in various ways:

* Directly, by iterating through all facets.
* With conventional bounding volume hierarchies (BVHs).
* With compact (linearized) BVHs.

The BVHs in EBGeometry are not limited to facets.
By providing a bounding volume constructor, users can also embed entire objects (e.g., analytic functions) in the BVHs.
This permits the construction of multi-object scenes, for example BVHs-of-BVHs type of scenes where multiple DCEL objects are embedded into another BVH layer.

Requirements
------------

* A C++ compiler which supports C++14.
* EBGeometry takes a watertight and orientable surface as input (only PLY files currently supported).

Basic usage
-----------

EBGeometry is a header-only library in C++.
To use it, simply make EBGeometry.hpp visible to your code and include it.

To clone the code do

    git clone git@github.com:rmrsk/EBGeometry.git

Various examples are given in the Examples folder.
To run one of the examples, navigate to the example and compile and run it.
E.g.,

    cd Examples/Basic
    g++ -O3 -std=c++14 main.cpp
    ./a.out porsche.ply

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