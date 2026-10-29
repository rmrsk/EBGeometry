.. EBGeometry documentation master file, created by
   sphinx-quickstart on Mon Mar  7 22:24:17 2022.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

EBGeometry's user documentation
===============================

EBGeometry is a header-only C++17 library of fast, portable geometry queries. It turns meshes,
analytic shapes, point clouds and compositions of them into geometry you can ask questions: how far
away is it, which surface is closest, where a ray hits it, what lies nearest. It answers them on CPUs
and on GPUs, from the same code, with no dependencies.

* **Geometry in:** triangle meshes (STL, PLY, OBJ, VTK) as a half-edge mesh or SIMD triangle blocks,
  analytic shapes and your own, and point clouds.
* **Composition:** constructive solid geometry (unions, intersections, differences, smooth blends,
  offsets, repetitions and transforms), compiled into a tape for a scene or fused into compiled code
  for a part, and BVH-accelerated unions of thousands of objects.
* **Queries:** signed distance, normals and bounding boxes; the primitive that decides the value at a
  point; ray and segment hits; closest points and nearest neighbours in point clouds.
* **Looking at it:** a renderer, on the host or a GPU, for any shape, with an optional interactive
  window (SDL2).
* **Performance:** packed BVHs with any branching factor and a choice of builds, SIMD on the host
  (SSE to AVX-512), CUDA and HIP on the device, and a performance comparison on every change.
* **No dependencies:** include ``EBGeometry.hpp`` in any C++17 project, in ``float`` or ``double``.

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
   Rays.rst
   Rendering.rst
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
   MemoryModel.rst
   RunningOnGPU.rst
   ImplemVec.rst
   ImplemCSG.rst
   ImplemDCEL.rst
   ImplemBVH.rst
   ImplemRays.rst
   ImplemRender.rst
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
   ExampleBasicCSG.rst
   ExampleSmoothCSG.rst
   ExampleTransforms.rst
   ExampleRepetition.rst
   ExampleValueOperations.rst
   ExampleFusedCSG.rst
   ExampleMeshSDF.rst
   ExampleMeshCSG.rst
   ExampleShell.rst
   ExampleMaterials.rst
   ExampleNestedBVH.rst
   ExampleManyObjects.rst
   ExamplePackedSpheres.rst
   ExamplePointClouds.rst
   ExampleBVHTraversal.rst
   ExampleRayTracing.rst
   ExampleOctreeBoundingVolume.rst
   ExampleCustomShape.rst
   ExampleTwist.rst
   ExampleHostTuning.rst
   ExampleDeviceTuning.rst
   ExampleBuildBVH.rst
   Integrations.rst

Contributing and testing
************************

.. toctree::
   :maxdepth: 3
   :caption: Contributing and testing
   :hidden:

   Contributing.rst
   TestingLocally.rst
   PerformanceTesting.rst
   ContinuousIntegration.rst
   ContributionGuidelines.rst

.. toctree::
   :maxdepth: 3
   :caption: References
   :hidden:

   ZZReferences.rst
