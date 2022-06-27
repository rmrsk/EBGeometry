This folder contains an advanced example of using EBGeometry, turning composite geometries into BVH-of-BVH types of scenes. 
It shows how to read a set of files containing tesslated polygons, and turning them each into a linear BVH representation.
The BVH representations are then put in:

* A standard CSG union.
* An optimized BVH-enabled CSG union.

The optimized union will be a BVH-of-BVH type of scene where each primitive in the outer BVH is a signed distance function.
The distance functions themselves are linear BVH hierarchies that bound the triangle sin each component. 

Compiling
---------

To compile the example, do

    g++ -std=c++17 -O3 main.cpp -lstdc++fs

Running
-------

Run it with

    ./a.out
