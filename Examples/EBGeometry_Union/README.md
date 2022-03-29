This folder contains a basic example of create multi-object scenes using EBGeometry.
This example creates a scene consisting of one million spheres (defined by analytic functions), and the scene is created from the union of these spheres.
Two unions are defined: :

* A standard union of objects that looks through every object.
* A union that uses BVHs for finding the closest object(s).

The scene is defined as an array of spheres (if size N^3), separated by their radii.
This example program illustrates the benefits of using BVHs for closest-object queries.
Rather than computing the distance to all spheres, the BVH allows traversal and a dramatic reduction algorithmic complexity.

Compiling
---------

To compile the example, do

    g++ -std=c++14 -O3 main.cpp

Running
-------

Run it with

    ./a.out

Typical output
--------------

    Partitioning spheres
    Computing distance with slow union
    Computing distance with fast union
    Distance and time using standard union = -1, which took 26.7353 ms
    Distance and time using optimize union = -1, which took 0.003527 ms
    Speedup = 7580.19

One can see that the optimized union took about 3.5 microseconds to compute, while the standard union (which iterates through every sphere) was about 7500 times slower.
