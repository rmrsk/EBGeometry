.. _Chap:Benchmark:

Benchmarks
==========

.. important::

   The benchmarks are illustrative, not a scoreboard: reported numbers vary widely across machines,
   compilers, and ISAs, so the tables are single-machine snapshots meant to be re-run rather than
   quoted. CI *compiles* each benchmark as a smoke test against bit-rot, but does **not** run or time
   them -- they depend on external libraries with their own build systems and release cadences. Those
   libraries are pinned as git submodules directly under :file:`Benchmark/`. See the benchmark
   tracking issue on GitHub for context and planned additions.

The :file:`Benchmark/` folder places EBGeometry next to other open-source geometry-query libraries on
tasks they have in common, so the tradeoffs are visible and reproducible. Every result is
cross-checked against an independent baseline -- correctness is the part that transfers across
machines even when the timings do not. Fetch the comparison libraries (and the top-level mesh
submodule) with:

.. code-block:: bash

   git submodule update --init Benchmark/nanoflann Benchmark/picoflann Benchmark/kd3 Benchmark/fcpw \
                               Benchmark/TriangleMeshDistance common-3d-test-models
   git -C Benchmark/fcpw submodule update --init deps/eigen   # fcpw's Eigen (skip the GPU dep)

Each benchmark ships a ``GNUmakefile`` (``make && ./<name>.ex``). See each folder's ``README.md`` for
the full detail and representative numbers.

* :file:`Benchmark/NearestNeighbor` -- all-nearest-neighbor over a point cloud: ``PointCloudBVH`` and
  ``PointCloudHashGrid`` vs `nanoflann <https://github.com/jlblancoc/nanoflann>`_ vs
  `picoflann <https://github.com/rmsalinas/picoflann>`_ vs
  `kd3 <https://github.com/KaruroChori/kd3>`_ (a SoA/SIMD KD-tree; needs C++23, run here in double --
  halving its float SIMD width -- for a same-precision comparison).
* :file:`Benchmark/MeshSDF` -- closest-point on a triangle mesh: ``TriMeshSDF`` vs
  `fcpw <https://github.com/rohan-sawhney/fcpw>`_ (built with its Enoki CPU vectorization) vs
  `TriangleMeshDistance <https://github.com/InteractiveComputerGraphics/TriangleMeshDistance>`_.
