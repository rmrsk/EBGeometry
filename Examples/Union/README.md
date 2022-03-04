This folder contains a basic example of create multi-object scenes using EBGeometry.
This example creates a scene consisting of one million spheres (defined by analytic functions), and the scene is created from the union of these spheres.
Two unions are defined: :

* A standard union of objects that looks through every object.
* A union that uses BVHs for finding the closest object(s).

The scene is defined as an array of analytic spheres bound by axis-aligned bounding boxes.
To shift the computational load to the signed distance function itself, computing the distance function has been given a delay of about 1 millisecond. 

Compiling
---------

To compile the example, do

    g++ -std=c++14 -O3 main.cpp

Running
-------

Run it with

    ./a.out
