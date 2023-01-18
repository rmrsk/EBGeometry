Examples/EBGeometry_F18
-----------------------

This is an advanced example, showing how multiple files (each containing a watertight polygon) can be embedded in a BVH-accelerated CSG union.

This example loads a series of STL files and creates a DCEL mesh from each file.
To speed up the signed distance calculation of each DCEL component, each DCEL mesh is put in a flattened BVH.
To construct the composite implicit function, which is a CSG union, the flattened BVHs are then put inside another BVH.
This creates a BVH-of-BVH type of scene:
The outer BVH provides fast lookup of the "nearest object".
The inner BVHs (stored in the leaf nodes of the outer BVH) provide fast lookup of the nearest triangle. 
The distance functions themselves are linear BVH hierarchies that bound the triangle sin each component.

This example also does a performance comparison between the four types of traversal structures:

* Standard CSG, and linear search through triangles.
* Standard CSG, and BVH-accelerated search through triangles.
* BVH-accelerated CSG, and linear search through triangles.
* BVH-accelerated CSG, and BVH-accelerated search through triangles.

Compiling
---------

To compile the example, do

    g++ -std=c++17 -O3 main.cpp -lstdc++fs

Running
-------

Run it with

    ./a.out
