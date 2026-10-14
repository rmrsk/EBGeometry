.. _Chap:Examples:

Overview
========

The :file:`Examples/` folder contains pure EBGeometry examples with no third-party
dependencies.  Each one can be built directly with a compiler, with GNU Make, or with CMake
(see :ref:`Chap:Building`), and every folder ships all three (a ``GNUmakefile``, a
``CMakeLists.txt``, and a plain ``main.cpp`` compilable with a single compiler invocation).

Examples that couple EBGeometry to a third-party application code's embedded-boundary grid
generation (`AMReX <https://amrex-codes.github.io/amrex/>`_, `Chombo
<https://commons.lbl.gov/display/chombo/>`_) live separately under :file:`Integrations/` instead,
and additionally require that platform to be installed — see :ref:`Chap:Integrations`.

None of the EBGeometry examples produce output for visualization; they print timing
and/or correctness information to the terminal.  Each example folder also ships its own
``README.md`` with the exact build/run commands reproduced on its page.

Building and running any of them
----------------------------------

Every example needs the path to the EBGeometry root — the
directory containing ``EBGeometry.hpp`` — which is two levels up (``../..``) when building in
place. All three build methods accept the same two overrides: ``PRECISION``/``EBGEOMETRY_PRECISION``
(``float`` or ``double``, default ``double``) and the location of the EBGeometry tree
itself (``EBGEOMETRY_HOME``, default ``../..``); all three also produce the binary
(``<Example>.ex``) directly in the example's own folder, regardless of where the CMake build
tree itself lives:

.. code-block:: bash

   # CMake
   cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry
   cmake --build build
   ./<Example>.ex

   # GNU Make
   make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry
   ./<Example>.ex

   # Direct compilation
   g++ -std=c++17 -O3 -march=native -DEBGEOMETRY_PRECISION=float -I../.. main.cpp -o <Example>.ex
   ./<Example>.ex

Built on its own with CMake, an example defaults to an optimised (``Release``) build; add
``-DCMAKE_CXX_FLAGS=-march=native`` to tune it for the local machine. Built as part of the
EBGeometry project (see :ref:`Chap:Building`), the examples follow the project's build type and
take their SIMD flags from ``EBGEOMETRY_SIMD``.

Every example checks its own results, against a brute-force scan or against distances worked out
by hand, and exits with a nonzero status if they disagree.

Run every example from inside its own folder — several resolve a default input mesh relative to
the run directory. See :ref:`Chap:Building` for the full detail on each build method.

.. _Sec:ExamplesGPU:

Running on a GPU
----------------

Every example whose queries can run on a device builds from the same source as plain C++ (the
default) or as CUDA or HIP. Compiled for a GPU, the same run does its usual work on the host and then
repeats its queries on the device, comparing every device result with the host's. They agree to
rounding rather than bit for bit, since a device compiler may fuse a multiply and an add where the
host compiler does not. Without a visible GPU the device part is skipped with a message, and the
program still checks its host results and exits normally. ``BuildBVH`` and ``HostTuning`` stay
host-only: they are about the BVH builders and the host-tuned types, neither of which a device uses.

Each query is a small trivially copyable functor whose call operator is ``EBGEOMETRY_HOST_DEVICE``,
holding what it evaluates by value. ``EBGeometry::GPU::transform(query, inputs)`` (see
:ref:`Sec:BuildingGPU`) runs it once per input in a kernel, and in a plain C++ build in a host loop.
:ref:`Chap:ExampleCustomShape` writes that kernel out instead, to show what ``GPU::transform`` does.

The examples show three ways of getting the geometry to the device:

* **Nothing to copy.** Analytic shapes are plain values, so a query holds them and goes straight to
  the kernel (:ref:`Chap:ExampleShapes`, :ref:`Chap:ExampleOctreeBoundingVolume`).
* **A mirror in device memory.** The pool is built on the host, frozen and copied to device memory in
  one piece, and the object is rebased onto the copy. This is the pattern for large geometry: mesh
  SDFs, BVH unions, tapes and point clouds (:ref:`Chap:ExampleMeshSDF`, :ref:`Chap:ExampleNestedBVH`,
  :ref:`Chap:ExampleCSGUnion`, :ref:`Chap:ExamplePackedSpheres`, :ref:`Chap:ExampleRandomCity`,
  :ref:`Chap:ExampleClosestPointBVH`, :ref:`Chap:ExampleNearestNeighborBVH`,
  :ref:`Chap:ExampleNearestNeighborHashGrid`).
* **Managed memory.** The pool is built in managed memory, frozen, and the object rebased onto it:
  one handle for host and device, with no copy (:ref:`Chap:ExampleCustomShape`).

All three build methods take a GPU switch:

.. code-block:: bash

   # CMake (CMAKE_HIP_ARCHITECTURES / CMAKE_CUDA_ARCHITECTURES pick the target)
   cmake -S . -B build -DEBGEOMETRY_EXAMPLE_GPU=HIP     # or CUDA
   cmake --build build

   # GNU Make (HIP_ARCH / CUDA_ARCH pick the target; make clean when switching)
   make GPU=HIP                                         # or GPU=CUDA

   # Direct compilation
   hipcc -std=c++17 -O3 -x hip --offload-arch=gfx90a -I../.. main.cpp -o <Example>.ex
   nvcc  -std=c++17 -O3 -x cu -arch=sm_80 -I../.. main.cpp -o <Example>.ex

Built as part of the EBGeometry project with ``EBGEOMETRY_ENABLE_HIP`` or ``EBGEOMETRY_ENABLE_CUDA``,
these examples are compiled for the device and collected in the ``EBGeometry_GPUDeviceExamples``
target. Continuous integration runs every example as plain C++ and compiles them for the device in
the HIP and CUDA jobs. Those runners have no GPU, so the device part is compiled but not run there.
