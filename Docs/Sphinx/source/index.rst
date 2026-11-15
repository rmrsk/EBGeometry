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
  point; ray and segment hits; closest points and nearest neighbors in point clouds.
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

   Quickstart.rst
   Building.rst
   ConfigurationOptions.rst
   Glossary.rst

Concepts
********

.. toctree::
   :maxdepth: 3
   :caption: Concepts
   :hidden:

   GeometryRepresentations.rst
   Meshes.rst
   BVH.rst
   PointCloud.rst
   ConceptsComposition.rst
   ConceptsRays.rst

Implementation
**************

.. toctree::
   :maxdepth: 3
   :caption: Implementation
   :hidden:

   Implementation.rst
   ImplemFoundations.rst
   ImplemMeshes.rst
   ImplemBVHs.rst
   ImplemPointCloud.rst
   ImplemComposition.rst
   ImplemRaysPictures.rst
   ImplemUtilities.rst

Examples
********

.. toctree::
   :maxdepth: 3
   :caption: Examples
   :hidden:

   Examples.rst
   ExamplesStart.rst
   ExamplesComposition.rst
   ExamplesMeshes.rst
   ExamplesMaterials.rst
   ExamplesManyObjects.rst
   ExamplesRays.rst
   ExamplesExtending.rst
   ExamplesTuning.rst
   Integrations.rst

Contributing and testing
************************

.. toctree::
   :maxdepth: 3
   :caption: Contributing and testing
   :hidden:

   Contributing.rst
   ContributionGuidelines.rst
   TestingLocally.rst
   PerformanceTesting.rst
   ContinuousIntegration.rst
   History.rst

.. toctree::
   :maxdepth: 3
   :caption: References
   :hidden:

   ZZReferences.rst
