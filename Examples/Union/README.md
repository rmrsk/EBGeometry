This folder contains a basic example of create multi-object scenes using EBGeometry.
This example creates a scene consisting of one million spheres (defined by analytic functions), and the scene is created from the union of these spheres.
Two unions are defined: :ne that looks through every sphere, whcih is the standard unoptimized approach. The other union uses a BVH for finding the closest sphere. 


Compiling
---------

To compile the example, do

    g++ -std=c++14 -O3 main.cpp

Running
-------

Run it with

    ./a.out
