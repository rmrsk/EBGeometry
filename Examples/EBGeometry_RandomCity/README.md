Examples/EBGeometry_RandomCity
------------------------------

This folder contains a basic example of create multi-object scenes using EBGeometry.
This example creates a scene consisting of random boxes (buildings), and the scene is created from the union of these buildings.
Two unions are defined: :

* A standard union of objects that looks through every object.
* A union that uses BVHs for finding the closest object(s).

This example program illustrates the benefits of using BVHs for closest-object queries.
Rather than computing the distance to all buildings, the BVH allows traversal and a dramatic reduction algorithmic complexity.

Compiling
---------

To compile the example, do

    g++ -std=c++14 -O3 main.cpp

Running
-------

Run it with

    ./a.out
