.. _Chap:Introduction:

Requirements
============

* A C++ compiler which supports C++14.

``EBGeometry`` is a header-only library and is comparatively simple to set up and use. 
To use it, make :file:`EBGeometry.hpp` (stored at the top level) visible to your code and include it.  

Quickstart
==========

To obtain ``EBGeometry``, clone the code from `github <https://github.com/rmrsk/EBGeometry>`_:

.. code-block:: bash

   git clone git@github.com:rmrsk/EBGeometry.git

To compile the ``EBGeometry`` example codes, navigate to the :file:`EBGeometry/Examples` folder.
Folders that are named :file:`EBGeometry_<something>` are pure ``EBGeometry`` examples and can be compiled without any third-party dependencies.

To run the ``EBGeometry`` examples, navigate to one of the folders in :file:`EBGeometry/Examples/EBGeometry_*` and execute

.. code-block:: bash

   g++ -O3 main.cpp && ./a.out

All ``EBGeometry`` examples can be run using this command.
README files present in each folder provide more information regarding the functionality and usage of each example code.

Third-party examples
====================

Exapmle folders that begin with e.g. ``AMReX_`` or ``Chombo3_`` are application code examples and require the user to install additional third-party software. 
