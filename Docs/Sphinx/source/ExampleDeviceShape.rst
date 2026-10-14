.. _Chap:ExampleDeviceShape:

DeviceShape
===========

A shape of your own, composed with built-in shapes into a tape (see :ref:`Sec:Tape`) and evaluated in
a GPU kernel. The same source builds as plain C++, where the same queries run in a host loop instead.

The shape is an octahedron, written in the example as an ordinary struct: trivially copyable, with an
``EBGEOMETRY_HOST_DEVICE`` ``signedDistance()`` and ``computeBoundingVolume()``, and a static
``distanceQuality`` that says its value is an exact distance. That is all a shape needs, and
``isShape`` checks it at compile time (see :ref:`Chap:ImplemCSG`). Inside device code it uses
``EBGeometry::Math::clamp`` rather than ``std::clamp`` (see :ref:`Sec:WritingDeviceCode`).

Twelve rotated copies of it sit on a ring around a sphere, and their union is compiled into a
``Tape<T, Octahedron<T>>``. The octahedron is listed after ``T`` because it is not a built-in type.
A union of thirteen objects is evaluated with a BVH over their boxes.

**Memory.** In a GPU build the tape is compiled straight into managed memory
(``managedMemoryResource()``), which the host and the device can both read:

.. code-block:: c++

   Pool pool(managedMemoryResource());           // hostMemoryResource() in a plain C++ build
   const Tape<T, Octahedron<T>> tape(pool, scene);
   pool.freeze();
   const auto view = tape.rebasedView(pool);     // one handle for the host and the device

The view goes to the kernel by value, and nothing is copied (see :ref:`Chap:MemoryModel`). That suits
a small object like this one, a few KB. For large geometry, mirror the pool to device memory instead,
as :ref:`Chap:ExampleDevicePointCloud` does: managed memory migrates between host and device as it is
touched, and on AMD MI200-class GPUs it migrates only with XNACK enabled.

**The query.** A query is a trivially copyable functor whose call operator is
``EBGEOMETRY_HOST_DEVICE``. It holds the tape by value and maps one point to one value. A few lines of
kernel run it once per point on the device, and in a plain C++ build a loop runs it on the host.

The program checks the values at points where they are known: the centers of the sphere and of each
octahedron, a point beyond a vertex and a point beyond a face. It also evaluates a grid of 64,000
points. In a GPU build each grid value is compared with the host's, to rounding, since a device
compiler may fuse a multiply and an add where the host does not. It exits with a nonzero status if
any value is wrong.

The source for this example is at :file:`Examples/DeviceShape/main.cpp`. See :ref:`Chap:Building`
for how to compile it with CMake, GNU Make, or a direct compiler invocation, and
:ref:`Sec:ExamplesGPU` for the GPU builds.

.. code-block:: bash

   cd Examples/DeviceShape
   make                  # plain C++: the queries run in a host loop
   make clean
   make GPU=HIP          # or GPU=CUDA: the queries run in a kernel
   ./DeviceShape.ex
