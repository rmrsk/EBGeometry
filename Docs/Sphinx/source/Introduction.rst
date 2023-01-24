.. _Chap:Introduction:

Requirements
============

* A C++ compiler which supports C++14.

Quickstart
==========

To obtain EBGeometry, clone the code from `github <https://github.com/rmrsk/EBGeometry>`_:

.. code-block:: bash

   git clone git@github.com:rmrsk/EBGeometry.git

EBGeometry is a header-only library and is comparatively simple to set up and use. 
To use it, make :file:`EBGeometry.hpp` (stored at the top level) visible to your code and include it.

To compile the EBGeometry example codes, navigate to the EBGeometry/Examples folder.
Folders that are named ``EBGeometry_<something>`` are pure ``EBGeometry`` examples and can be compiled without any third-party dependencies.
Other folders that begin with e.g. ``AMReX_`` or ``Chombo3_`` are application code examples and require the user to install additional third-party software. 

To run the EBGeometry examples, navigate to one of the folders and execute

.. code-block:: bash

   g++ -O3 -std=c++14 main.cpp && ./a.out

All EBGeometry examples should run using this command.
README files present in each folder provide more information regarding the functionality and usage of each example code.
