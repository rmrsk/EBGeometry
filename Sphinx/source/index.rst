.. EBGeometry documentation master file, created by
   sphinx-quickstart on Mon Mar  7 22:24:17 2022.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to EBGeometry's documentation!
======================================

This is the user documentation for EBGeometry, a small C++ package for computing signed distance fields from surface tesselations.
Although EBGeometry is a self-contained package, it is was originally written for usage with embedded boundary (EB) and immersed boundary (IB) codes.
A separate Doxygen-generated API of EBGeometry is found `here <doxygen/html/index.html>`_.

.. important::

   EBGeometry does not involve itself with the *discrete geometry generation*, i.e. the generation of cut-cells.
   It only takes care of the *geometry representation*, i.e. the creation of complex geometries as numerically efficient signed distance fields. 

.. toctree::
   :maxdepth: 3
   :caption: Contents

   Introduction.rst
   Concepts.rst
   DCEL.rst
   BVH.rst
   Implementation.rst
   Examples.rst
   ZZReferences.rst
