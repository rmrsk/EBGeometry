.. EBGeometry documentation master file, created by
   sphinx-quickstart on Mon Mar  7 22:24:17 2022.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

EBGeometry's user documentation
===============================

EBGeometry is a header-only C++17 library for constructive solid geometry (CSG) with implicit
functions. It can turn surface geometries into fast, queryable signed distance functions (SDFs),
and manipulate them with CSG operations.

Main features:

* Turn surface meshes into SDFs, via a half-edge (DCEL) mesh representation or raw triangles.
* Fast SDF evaluation using bounding volume hierarchies (BVHs).
* Supports both pointer-based tree BVHs, and flattened SIMD-accelerated packed BVHs.
* A library of analytic signed distance functions and implicit functions (spheres, boxes, and
  more).
* Composable with transforms (translation, rotation, scaling, rounding, blending).
* BVH-accelerated constructive solid geometry (CSG): unions, intersections, differences, and
  smooth blends, of both meshes and analytic shapes.
* Readers for triangulated surface meshes in STL, PLY, OBJ, and VTK format.
* Drop-in precision-templated for flexible usage.
* No external dependencies -- drop ``EBGeometry.hpp`` into any C++17 project and include it.

.. important::

   This is the user documentation for EBGeometry.

   .. only:: html

      A `PDF version <https://rmrsk.github.io/EBGeometry/ebgeometry.pdf>`_ of this documentation is
      also available.

   * If you are looking for the source code, it is hosted on
     `GitHub <https://github.com/rmrsk/EBGeometry>`_.
   * The developer documentation is a separate
     `Doxygen-generated API <https://rmrsk.github.io/EBGeometry/doxygen/html/index.html>`_
     of EBGeometry.

.. This is for getting rid of the TOC in html view.
.. raw:: html

   <style>
   section#getting-started,
   section#concepts,
   section#implementation,
   section#examples,
   section#contributing-and-testing,
   section#references,
   section#bibliography {
     display:none;
   }
   </style>

.. only:: latex

   .. toctree::
      :caption: Contents

Getting started
****************

.. toctree::
   :maxdepth: 3
   :caption: Getting started
   :hidden:

   ObtainingEBGeometry.rst
   Quickstart.rst
   Building.rst
   SIMDAcceleration.rst
   ConfigurationOptions.rst

Concepts
********

.. toctree::
   :maxdepth: 3
   :caption: Concepts
   :hidden:

   GeometryRepresentations.rst
   DCEL.rst
   BVH.rst
   PointCloud.rst
   Octree.rst
   CSG.rst
   Triangles.rst

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
   ImplemPointCloud.rst
   ImplemOctree.rst
   Parsers.rst
   SIMDClasses.rst

Examples
********

.. toctree::
   :maxdepth: 3
   :caption: Examples
   :hidden:

   Examples.rst
   ExampleShapes.rst
   ExampleMeshSDF.rst
   ExampleCSGUnion.rst
   ExampleNestedBVH.rst
   ExamplePackedSpheres.rst
   ExampleRandomCity.rst
   ExampleOctreeBoundingVolume.rst
   Integrations.rst

Contributing and testing
************************

.. toctree::
   :maxdepth: 3
   :caption: Contributing and testing
   :hidden:

   Contributing.rst
   TestingLocally.rst
   ContinuousIntegration.rst
   ContributionGuidelines.rst

.. toctree::
   :maxdepth: 3
   :caption: References
   :hidden:

   ZZReferences.rst
