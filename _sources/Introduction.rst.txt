.. _Chap:Introduction:

EBGeometry is a comparatively compact code for computing signed distance functions to watertight and orientable surface grids. 
Originally, it was written to be used with embedded-boundary (EB) codes like Chombo or AMReX.

Requirements
============

* A C++ compiler which supports C++14.
* An analytic signed distance function, or a watertight and orientable surface grid. 

Quickstart
==========

To obtained EBGeometry, clone the code from `github <https://github.com/rmrsk/EBGeometry>`_:

.. code-block:: bash

   git clone git@github.com:rmrsk/EBGeometry.git

EBGeometry is a header-only library and is simple to use. 
To use it, make :file:`EBGeometry.hpp` (stored at the top level) visible to your code and include it.

To compile the examples, navigate to the examples folder.
The following two examples show two usages of EBGeometry:

#. :file:`Examples/Basic` shows how to construct signed distance fields from surface grids, and how to perform signed distance calculations.
#. :file:`Examples/Union` shows how to create multi-object scenes and embed them in bounding volume hierarchies. 

Features
========

The basic features of EBGeometry are as follows:

* Representation of water-tight surface grids as signed distance fields.
  EBGeometry uses a doubly-connected edge list (DCEL) representation of the mesh.
* Straightforward implementations of bounding volume hierarchies (BVHs) for use as acceleration structures.
  The BVHs can be represented in full or compact (i.e., linearized) forms.
* Support for signed distance calculations and transformations (rotations, translations, and scaling).
* Heavy use of metaprogramming, which exists e.g. in order to permit higher-order trees and flexibility in BVH partitioning.
* Examples of how to couple EBGeometry to AMReX and Chombo.  
