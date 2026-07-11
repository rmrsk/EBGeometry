.. _Chap:Benchmark:

Benchmarks
==========

.. important::

   Like the :ref:`Chap:Integrations` examples, the benchmarks are illustrative and **not built or run
   as part of EBGeometry's continuous integration** -- they depend on external libraries with their
   own build systems. They pin those libraries as git submodules under :file:`Submodules/`. See the
   benchmark tracking issue on GitHub for context and planned additions.

The :file:`Benchmark/` folder compares EBGeometry against other open-source geometry-query libraries
on tasks they have in common. Fetch the comparison libraries (and the mesh submodule) with:

.. code-block:: bash

   git submodule update --init Submodules/nanoflann Submodules/picoflann Submodules/fcpw \
                               Submodules/TriangleMeshDistance Submodules/common-3d-test-models
   git -C Submodules/fcpw submodule update --init deps/eigen   # fcpw's Eigen (skip the GPU dep)

Each benchmark ships a ``GNUmakefile`` (``make && ./<name>.ex``) and cross-checks every result against
a brute-force / independent baseline. See each folder's ``README.md`` for the full detail and
representative numbers.

* :file:`Benchmark/NearestNeighbor` -- all-nearest-neighbor over a point cloud: ``PointCloudBVH`` vs
  `nanoflann <https://github.com/jlblancoc/nanoflann>`_ vs
  `picoflann <https://github.com/rmsalinas/picoflann>`_.
* :file:`Benchmark/MeshSDF` -- closest-point on a triangle mesh: ``TriMeshSDF`` vs
  `fcpw <https://github.com/rohan-sawhney/fcpw>`_ (built with its Enoki CPU vectorization) vs
  `TriangleMeshDistance <https://github.com/InteractiveComputerGraphics/TriangleMeshDistance>`_.
