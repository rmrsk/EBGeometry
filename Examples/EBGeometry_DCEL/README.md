This folder contains a basic example of using EBGeometry, with three different representations of the signed distance field.

* A naive approach which iterates through all facets and computed the signed distance.
* Using a conventional bounding volume hierarchy with (references to) primitives stored directly in the nodes.
* A flattened, more compact bounding volume hierarchy.

Note that SDF queries have different complexity for different geometries and input points.
For example, a tessellated sphere has a "blind spot" in it's center where even BVHs will visit most, if not all, primitives. 

Compiling
---------

To compile the example, do

    g++ -std=c++14 -O3 main.cpp

Running
-------

Run it with

    ./a.out 'filename'

where 'filename' is one of the files in ../PLY/. 