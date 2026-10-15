.. _Chap:ExampleCustomShape:

CustomShape
===========

A shape of your own, composed with built-in shapes into a tape (see :ref:`Sec:Tape`), evaluated on
the host and, in a GPU build, in a GPU kernel too.

The shape is an octahedron, written in the example as an ordinary struct. It is trivially copyable,
with an ``EBGEOMETRY_HOST_DEVICE`` ``signedDistance()`` and ``computeBoundingVolume()``, and a static
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
a small object like this one, a few KB. Large geometry is better mirrored to device memory, as the
other examples do: managed memory migrates between host and device as it is touched, and on AMD
MI200-class GPUs it migrates only with XNACK enabled. Managed memory needs a device, so a GPU build
that finds none builds the tape in host memory and runs only the host part.

**The kernel.** A query is a trivially copyable functor whose call operator is
``EBGEOMETRY_HOST_DEVICE``. This example writes out the kernel that runs it, a few lines on the
``EBGeometry::GPU`` runtime wrappers, to show what ``EBGeometry::GPU::transform()`` does; the other
examples call ``GPU::transform()`` instead (see :ref:`Sec:ExamplesGPU`).

The program checks the values at points where they are known: the centers of the sphere and of each
octahedron, a point beyond a vertex and a point beyond a face. It also evaluates a grid of 64,000
points, and in a GPU build compares each device value with the host's, to rounding. It exits with a
nonzero status if any value is wrong.

The source for this example is at :file:`Examples/CustomShape/main.cpp`. See :ref:`Chap:Building`
for how to compile it with CMake, GNU Make, or a direct compiler invocation, and
:ref:`Sec:ExamplesGPU` for the GPU builds.

.. code-block:: bash

   cd Examples/CustomShape
   ./CustomShape.ex
