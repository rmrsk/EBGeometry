.. EBGeometry documentation master file, created by
   sphinx-quickstart on Mon Mar  7 22:24:17 2022.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

EBGeometry's user documentation
===============================

This is the user documentation for EBGeometry, a small C++ package for computing signed distance fields from surface tesselations and analytic shapes. 
Although EBGeometry is a self-contained package, it is was originally written for usage with embedded boundary (EB) and immersed boundary (IB) codes.

EBGeometry only provides the geometry representation through an implicit or signed distance function.
It does not the discrete geometry generation, i.e. the generation of cut-cells from a given geometry. 

.. important::

   The EBGeometry source code is found `here <https://github.com/rmrsk/EBGeometry>`_.
   A separate Doxygen-generated API of EBGeometry is `available here <doxygen/html/index.html>`_.   

.. This is for getting rid of the TOC in html view. 
.. raw:: html

   <style>
   /* front page: hide chapter titles
    * needed for consistent HTML-PDF-EPUB chapters
    */
   <style>
   section#introduction,
   section#design,
   section#discretization,
   section#solvers,
   section#multi-physics-applications,
   section#single-solver-applications,
   section#tutorial,
   section#utilities,
   section#contributing,
   section#references,
   section#bibliography,
   section#epilogue {
	 display:none;
   }
   </style>   

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
   :caption: Concepts
   :hidden:	     
	     
   Concepts.rst
   DCEL.rst
   BVH.rst

Implementation
**************

.. toctree::
   :maxdepth: 3
   :caption: Implementation
   :hidden:	     
	     
   Implementation.rst
   ImplemVec.rst
   ImplemBVH.rst
   ImplemDCEL.rst
   ImplemSDF.rst
   ImplemUnion.rst
   Parsers.rst

Guided examples
***************  

.. toctree::
   :maxdepth: 3
   :caption: Guided examples
   :hidden:	     

   Examples.rst
   Example_Basic.rst
   Example_Union.rst
   Example_AMReX.rst
   Example_Chombo3.rst   

References
**********

.. toctree::
   :maxdepth: 3
   :caption: References
   :hidden:	     
   
   ZZReferences.rst      
