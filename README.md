EBGeometry
----------

EBGeometry is a code for computing signed distance functions to watertight and orientable surface grids. 
It was written to be used with embedded-boundary (EB) codes like Chombo or AMReX.

Tesselations must consist of planar polygons (not necessarily triangles).
Internally, the surface mesh is stored in a doubly-connected edge list (DCEL), i.e. a half-edge data structure. 
On watertight and orientable grids, the distance to any feature (facet, edge, vertex) is well defined, and can be computed in various ways:

* Directly, by iterating through all facets.
* With conventional bounding volume hierarchies (BVHs).
* With compact (linearized) BVHs.

The BVHs in EBGeometry are not limited to facets.
Users can also embed entire objects (e.g., analytic functions) in the BVHs, e.g. the BVH accelerator can be used to accelerate the signed distance computation when geometries contain many objects. 
BVHs can also be nested so that the BVH accelerator is used to embed objects that are themselves contained in a BVH. 
For example, a scene consisting of many objects described by surface grids can be embedded as a BVH-of-BVH type of scene.
In addition, EBGeometry provides standard operators for signed distance fields like rotations, translations, and scalings.

<img src="example.png" width="400" alt="Signed distance field from Armadillo geometry"/>

Requirements
------------

* A C++ compiler which supports C++14.
* An analytic signed distance function or a Watertight and orientable surface (only PLY files currently supported).

Basic usage
-----------

EBGeometry is a header-only library in C++.
To use it, simply make EBGeometry.hpp visible to your code and include it.

To clone EBGeometry:

    git clone git@github.com:rmrsk/EBGeometry.git

Various examples are given in the Examples folder.
To run one of the examples, navigate to the example and compile and run it.
E.g.,

    cd Examples/Basic
    g++ -O3 -std=c++14 main.cpp
    ./a.out porsche.ply

The examples take the following steps that are specific to EBGeometry:

1. Define an analytic signed distance function or parse a surface mesh into a DCEL mesh object.
2. Partition using BVHs. 
3. Compute the signed distance function. 

More complex examples that use Chombo or AMReX will also include application-specific code. 

Advanced usage
--------------

For more advanced usage, users can supply their own file parsers (only PLY files are currently supported), provide their own bounding volumes, or their own BVH partitioners.
EBGeometry is not too strict about these things, and uses rigorous templating for ensuring that the EBGeometry functionality can be extended.

License
-------

See LICENSE and Copyright.txt for redistribution rights. 
