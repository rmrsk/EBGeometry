This example uses the embedded boundary grid generation from AMReX.
This is an advanced example, showing how multiple files (each containing a watertight polygon) can be embedded in a BVH-accelerated CSG union.

This example loads a series of STL files and creates a DCEL mesh from each file.
To speed up the signed distance calculation of each DCEL component, each DCEL mesh is put in a flattened BVH.
To construct the composite implicit function, which is a CSG union, the flattened BVHs are then put inside another BVH.
This creates a BVH-of-BVH type of scene:
The outer BVH provides fast lookup of the "nearest object".
The inner BVHs (stored in the leaf nodes of the outer BVH) provide fast lookup of the nearest triangle. 
The distance functions themselves are linear BVH hierarchies that bound the triangle sin each component.

This example also does a performance comparison between:

* An optimized BVH-enabled CSG union (as above).
* A standard CSG union.

Compiling
---------

To compile the example, do

    g++ -std=c++17 -O3 main.cpp -lstdc++fs

Running
-------

Run it with

    ./a.out
