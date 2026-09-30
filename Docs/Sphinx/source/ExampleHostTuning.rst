.. _Chap:ExampleHostTuning:

HostTuning
==========

Builds a point cloud (:ref:`Chap:PointCloud`) and a triangle mesh signed distance function with
the default branching factor and SIMD width, and again with the values tuned to the host's SIMD
flags. Checks that both give the same answers and times their queries. See :ref:`Sec:DefaultKW`
for the difference between the two and when to use which.

The source for this example is at :file:`Examples/HostTuning/main.cpp`. See
:ref:`Chap:Building` for how to compile it with CMake, GNU Make, or a direct compiler
invocation.

.. code-block:: bash

   cd Examples/HostTuning
   ./HostTuning.ex ../../common-3d-test-models/data/armadillo.obj
