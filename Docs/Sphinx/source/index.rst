.. EBGeometry documentation master file, created by
   sphinx-quickstart on Mon Mar  7 22:24:17 2022.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

EBGeometry's user documentation
===============================

This is the user documentation for EBGeometry, a small C++ package for efficiently representing implicit functions and signed distance fields for complex geometries.
Although EBGeometry is a self-contained package, it is was originally written for usage with embedded boundary (EB) and immersed boundary (IB) codes.
EBGeometry provides the geometry representation through an implicit or signed distance function, but does not provide the discrete geometry generation, i.e. the generation of cut-cells for a given geometry.

The basic features of EBGeometry are as follows:

* Representation of water-tight surface grids as signed distance fields.
* Many analytic distance functions and transformations. 
* Bounding volume hierarchies (BVHs) for use as acceleration structures for polygon or full object lookup.
  The BVHs can be represented in full or compact (i.e., linearized) forms.
* Support for both conventional and accelerated (using BVHs) constructive solid geometry (CSG).
* Examples of how to couple EBGeometry to AMReX and Chombo.     

.. important::

   This is the user documentation for EBGeometry.
   The source code is found at `<https://github.com/rmrsk/EBGeometry>`_ and a separate Doxygen-generated API of EBGeometry is available at `<https://rmrsk.github.io/EBGeometry/doxygen/html/index.html>`_.

.. This is for getting rid of the TOC in html view. 
.. raw:: html

   <style>
   section#introduction,
   section#concepts,
   section#implementation,
   section#guided-examples,
   section#references,
   section#bibliography,
   section#epilogue {
	 display:none;
   }
   </style>
   
.. only:: latex

   .. toctree::
      :caption: Contents   

Introduction
************

.. toctree::
   :maxdepth: 3
   :caption: Introduction
   :hidden:

   Introduction.rst

Concepts
********

.. toctree::
   :maxdepth: 3
   :caption: Basic concepts
   :hidden:	     
	     
   Concepts.rst

Implementation
**************

.. toctree::
   :maxdepth: 3
   :caption: Implementation
   :hidden:	     
	     
   Implementation.rst
   ImplemVec.rst
   ImplemCSG.rst
   ImplemDCEL.rst   
   ImplemBVH.rst
   Parsers.rst

Examples
********

.. toctree::
   :maxdepth: 3
   :caption: Examples
   :hidden:	     

   Examples.rst

.. toctree::
   :maxdepth: 3
   :caption: References
   :hidden:	     

   ZZReferences.rst      
