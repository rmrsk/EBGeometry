.. _Chap:Examples:

Overview
========

The :file:`Examples/` folder contains pure EBGeometry examples with no third-party
dependencies.  Each one can be built directly with a compiler, with GNU Make, or with CMake
(see :ref:`Chap:Building`), and every folder ships all three (a ``GNUmakefile``, a
``CMakeLists.txt``, and a plain ``main.cpp`` compilable with a single compiler invocation).

Examples that couple EBGeometry to a third-party application code's embedded-boundary grid
generation live separately under :file:`Integrations/` instead,
and additionally require that platform to be installed — see :ref:`Chap:Integrations`.

.. important::

   Several examples read a mesh from the ``common-3d-test-models`` git submodule by default
   (:ref:`Chap:ExampleMeshSDF`, :ref:`Chap:ExampleMeshCSG`, :ref:`Chap:ExampleShell`,
   :ref:`Chap:ExampleMaterials`, :ref:`Chap:ExampleRepetition`, :ref:`Chap:ExampleTwist`,
   :ref:`Chap:ExampleRayTracing`, :ref:`Chap:ExampleDeviceTuning`). Clone the repository with ``--recurse-submodules``, or run
   ``git submodule update --init`` in it (see :ref:`Sec:Cloning`).

The examples teach; the unit tests verify. An example shows the calls a user would write, in the
order they would write them, and prints results a reader can interpret: a few distances, the
closest point to a query, the time a BVH saves where that time is the lesson. It does not check its
own answers against hand-worked values or brute force -- the unit tests under :file:`Tests/` do that --
so its source is only the code you would adapt. Every example that builds a shape ends by opening a
window on it with ``Render::view()`` when built with the viewer (``make VIEWER=SDL2``, see
:ref:`Sec:ConfigViewer`), and its page shows what that window first shows
(rendered by ``Scripts/render-example-figures.sh``); otherwise, with
``--no-view`` or without a display, ``view()`` prints why and returns, so the examples still run
unattended. Each example folder also ships its own ``README.md`` with the exact build/run
commands reproduced on its page.

Reading order
-------------

The examples build on each other. A new user can read them in this order:

#. :ref:`Chap:ExampleShapes`: the analytic shapes, their signed distance, bounding box and distance
   quality.
#. :ref:`Chap:ExampleBasicCSG`: the classic CSG solid, with unions, intersections and differences,
   compiled into a ``Tape``.
#. :ref:`Chap:ExampleSmoothCSG`: smooth unions, intersections and differences, and the blend kinds.
#. :ref:`Chap:ExampleTransforms`: one object moved, turned, scaled, mirrored and stretched.
#. :ref:`Chap:ExampleRepetition`: many copies of a mesh from one ``FiniteRepetition``.
#. :ref:`Chap:ExampleValueOperations`: offsets, shells, blurring and mollifying.
#. :ref:`Chap:ExampleFusedCSG`: the classic solid as a fused composition, against the tape.
#. :ref:`Chap:ExampleMeshSDF`: reading a surface mesh into a signed distance function, and the
   three representations to choose from.
#. :ref:`Chap:ExampleMeshCSG`: a mesh joined to analytic shapes, the dragon on a table.
#. :ref:`Chap:ExampleShell`: a thin shell around a mesh (Lucy), cut open.
#. :ref:`Chap:ExampleMaterials`: which part of an object a query found, and a material table indexed by it.
#. :ref:`Chap:ExampleNestedBVH`: a mesh placed many times, in a tape and in a BVH union of meshes.
#. :ref:`Chap:ExampleManyObjects`: a BVH union over many primitives, and how much it saves over a
   plain loop.
#. :ref:`Chap:ExamplePackedSpheres`: thousands of spheres at random places in a cylinder, joined by a
   smooth BVH union.
#. :ref:`Chap:ExamplePointClouds`: closest points and nearest neighbors in a point cloud, with a
   BVH or a hash grid.
#. :ref:`Chap:ExampleBVHTraversal`: walking a BVH with a search of your own -- the nearest object to
   a point, the first one a ray hits, whether a segment is blocked.
#. :ref:`Chap:ExampleRayTracing`: casting rays at a triangle mesh -- where a ray or a particle's step
   first crosses the surface, and the same walk of the BVH written by hand.
#. :ref:`Chap:ExampleOctreeBoundingVolume`: estimating a bounding box for a function with no
   closed-form one.
#. :ref:`Chap:ExampleCustomShape`: a shape type of your own, and calling EBGeometry from your own
   kernel.
#. :ref:`Chap:ExampleTwist`: an operation of your own, a twist, applied to a mesh.
#. :ref:`Chap:ExampleHostTuning`: the branching factor and SIMD width tuned to the host.
#. :ref:`Chap:ExampleDeviceTuning`: the block size and register budget of a GPU launch, tuned to the
   query.
#. :ref:`Chap:ExampleBuildBVH`: comparing the BVH construction methods by build time.

Every example that runs on a device has the same four parts, in this order: the query functor
("the code that runs on either side"), the construction of the geometry, its use on the host
through plain calls, and, last, the same query on a GPU. A reader interested only in the host can
stop before the last part.

Building and running any of them
----------------------------------

Every example needs the path to the EBGeometry root — the
directory containing ``EBGeometry.hpp`` — which is two levels up (``../..``) when building in
place. All three build methods accept the same three overrides: ``PRECISION``/``EBGEOMETRY_PRECISION``
(``float`` or ``double``, default ``double``), the location of the EBGeometry tree
itself (``EBGEOMETRY_HOME``, default ``../..``), and ``ASSERTIONS``/``EBGEOMETRY_ENABLE_ASSERTIONS``
(``ON`` or ``OFF``, default ``OFF``) for the ``EBGEOMETRY_EXPECT()`` checks; all three also produce the binary
(``<Example>.ex``) directly in the example's own folder, regardless of where the CMake build
tree itself lives:

.. code-block:: bash

   # CMake
   cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry
   cmake --build build
   ./<Example>.ex

   # GNU Make (a host build also takes CXX and CXXFLAGS; -std=c++17 is always added)
   make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry
   ./<Example>.ex

   # Direct compilation
   g++ -std=c++17 -O3 -march=native -DEBGEOMETRY_PRECISION=float -I../.. main.cpp -o <Example>.ex
   ./<Example>.ex

Built on its own with CMake, an example defaults to an optimized (``Release``) build; add
``-DCMAKE_CXX_FLAGS=-march=native`` to tune it for the local machine. Built as part of the
EBGeometry project (see :ref:`Chap:Building`), the examples follow the project's build type and
take their SIMD flags from ``EBGEOMETRY_SIMD``.

``ctest`` runs every example (label ``example``) as a smoke test: it fails if the program throws
or, in a build with assertions, if an ``EBGEOMETRY_EXPECT()`` check fires.

Run every example from inside its own folder — several resolve a default input mesh relative to
the run directory. See :ref:`Chap:Building` for the full detail on each build method.

.. _Sec:ExamplesGPU:

Running on a GPU
----------------

Every example except ``BuildBVH`` and ``HostTuning`` builds from the same source as plain C++ (the
default) or as CUDA or HIP. In a GPU build its window renders on the device: ``Render::view()``, given
the pool the shape lives in, mirrors the pool for as long as the window is open and renders through the
shape's view of the copy (G switches to the CPU and back). An example with a section of queries runs it
through ``GPU::transform`` (see
:ref:`Chap:RunningOnGPU`): in a kernel in a GPU build, and in a host loop otherwise, so that section
runs in every build. Pool-resident geometry is mirrored into ``GPU::memoryResource()``, which is
device memory in a GPU build and host memory standing in for it otherwise. Only a GPU build with no
visible GPU skips the section, with a message. Device results agree with the host's to rounding
rather than bit for bit, since a device compiler may fuse a multiply and an add where the host
compiler does not. ``BuildBVH`` and ``HostTuning`` stay host-only: they are about the BVH builders
and the host-tuned types, neither of which a device uses, and open no window.

Each query is a small trivially copyable struct whose call operator is ``EBGEOMETRY_HOST_DEVICE``,
holding what it evaluates by value. :ref:`Chap:ExampleCustomShape` writes the kernel out instead, to
show how an application calls EBGeometry from its own kernels.

The examples show three ways of getting the geometry to the device:

* **Nothing to copy.** Analytic shapes are plain values, so a query holds them and goes straight to
  the kernel (:ref:`Chap:ExampleShapes`, :ref:`Chap:ExampleFusedCSG`,
  :ref:`Chap:ExampleOctreeBoundingVolume`).
* **A mirror in device memory.** The pool is built on the host, frozen and copied to device memory in
  one piece, and the object is rebased onto the copy. This is the pattern for large geometry: mesh
  SDFs, BVH unions, tapes and point clouds (:ref:`Chap:ExampleBasicCSG`, :ref:`Chap:ExampleMeshSDF`,
  :ref:`Chap:ExampleNestedBVH`, :ref:`Chap:ExampleManyObjects`, :ref:`Chap:ExamplePointClouds`,
  :ref:`Chap:ExampleBVHTraversal`, :ref:`Chap:ExampleRayTracing`).
  :ref:`Chap:ExampleManyObjects` also keeps its query points on the device, so that its device
  timing measures the kernel and not the copies.
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
   nvcc  -std=c++17 -O3 -x cu -arch=native -I../.. main.cpp -o <Example>.ex

A CUDA build is for the GPU in the machine unless told otherwise (``nvidia-smi`` names it; without one,
compute capability 7.0). The binary then carries that GPU's machine code. A binary without it carries
only PTX, which the driver compiles when the program starts, and a driver older than the CUDA toolkit
refuses that (``the provided PTX was compiled with an unsupported toolchain``): build for the GPU, or
update the driver.

Built as part of the EBGeometry project with ``EBGEOMETRY_ENABLE_HIP`` or ``EBGEOMETRY_ENABLE_CUDA``,
these examples are compiled for the device and collected in the ``EBGeometry_GPUDeviceExamples``
target. Continuous integration runs every example as plain C++ and compiles them for the device in
the HIP and CUDA jobs. Those runners have no GPU, so the device part is compiled but not run there.
