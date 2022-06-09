.. _Chap:Introduction:

EBGeometry is a comparatively compact code for computing signed distance functions to watertight and orientable surface grids. 
Originally, it was written to be used with embedded-boundary (EB) codes like Chombo or AMReX.

Requirements
============

* A C++ compiler which supports C++14.

Quickstart
==========

To obtained EBGeometry, clone the code from `github <https://github.com/rmrsk/EBGeometry>`_:

.. code-block:: bash

   git clone git@github.com:rmrsk/EBGeometry.git

EBGeometry is a header-only library and is comparatively simple to set up and use. 
To use it, make :file:`EBGeometry.hpp` (stored at the top level) visible to your code and include it.

To compile the examples, navigate to the examples folder.
Examples that begin by *EBGeometry_* are pure ``EBGeometry`` examples.
Other examples that begin with e.g. ``AMReX_`` or ``Chombo3_`` are application code examples.
These examples require the user to install additional third-party software.

To run the EBGeometry examples, navigate to e.g. :file:`Examples/EBGeometry_DCEL` and compile and run the application code as follows:

.. code-block:: bash

   g++ -O3 -std=c++14 main.cpp
   ./a.out porsche.ply

This will read :file:`porsche.ply` (stored under :file:`Examples/PLY`) and create a signed distance function from it. 


Features
========

The basic features of EBGeometry are as follows:

* Representation of water-tight surface grids as signed distance fields.
  EBGeometry uses a doubly-connected edge list (DCEL) representation of the mesh.
* Various analytic distance functions. 
* Bounding volume hierarchies (BVHs) for use as acceleration structures.
  The BVHs can be represented in full or compact (i.e., linearized) forms.
* Use of metaprogramming, which exists e.g. in order to permit higher-order trees and flexibility in BVH partitioning.
* Examples of how to couple EBGeometry to AMReX and Chombo.  
