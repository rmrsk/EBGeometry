.. _Chap:RunningOnGPU:

Running queries on a GPU
========================

Every EBGeometry object that can be evaluated on a device (the analytic shapes, the mesh signed
distance functions, the BVH unions, tapes, the point clouds) is a trivially copyable value. A kernel
receives it by value and calls it like any other function. There are two ways to get it into a
kernel:

* **Write the kernel yourself.** This is how an application uses EBGeometry inside its own device
  loops (an AMReX ``ParallelFor``, a Chombo kernel, a Kokkos ``parallel_for``), on data that already
  lives on the device. :ref:`Chap:ExampleCustomShape` shows such a kernel.
* **Let EBGeometry launch it**, with ``GPU::parallelFor`` or ``GPU::transform``. These are
  conveniences for batches of queries that a program sets up itself: the examples, the tests, a
  one-off evaluation of a grid of points.

This page describes the second. See :ref:`Sec:BuildingGPU` for how to compile for a GPU and
:ref:`Chap:MemoryModel` for how an object's memory reaches the device.

What runs where
---------------

``parallelFor`` and ``transform`` are declared in
`EBGeometry_GPUTransform.hpp <doxygen/html/EBGeometry__GPUTransform_8hpp.html>`__ (included by
``EBGeometry.hpp``). In a translation unit compiled as CUDA or HIP they launch a kernel. In any
other translation unit they run the same functor in a host loop, so one source compiles and runs in
both builds and a plain C++ build needs no GPU toolkit. The ``GPU::`` prefix says where a call *can*
run, not where it always does:

* ``GPU::enabled`` is ``true`` in a CUDA or HIP translation unit, and ``false`` otherwise.
* ``GPU::backendName`` is ``"CUDA"``, ``"HIP"`` or ``"host"``.
* ``GPU::deviceAvailable()`` says whether a device is visible at run time. It is always ``false``
  in a host build.
* ``GPU::memoryResource()`` is the memory that a launched functor's data must live in:
  ``deviceMemoryResource()`` in a CUDA or HIP build, and ``emulatedDeviceMemoryResource()``
  otherwise. The emulated resource is host memory that reports itself device-accessible, so that
  mirroring a pool into it and taking a ``rebasedView()`` onto the mirror takes the same path as it
  would onto device memory.

A program can therefore write its device part once, guarded only against a GPU build with no
device:

.. code-block:: c++

   if (GPU::enabled && !GPU::deviceAvailable()) {
     // A CUDA or HIP build on a machine without a GPU: skip the device part.
   }
   else {
     pool.freeze();

     const Pool devicePool = Pool::mirror(pool, GPU::memoryResource());

     const auto distances = GPU::transform(Distance{sdf.rebasedView(devicePool)}, points);
   }

All three calls below are synchronous: they return once the kernel has finished. Every runtime call
they make is checked, and an error aborts with the call that failed. Their functors must be
trivially copyable, which is checked at compile time in both builds, so a functor that could not
reach a kernel fails to compile on the host too. Write a functor as a struct with an
``EBGEOMETRY_HOST_DEVICE`` call operator, not as a lambda: nvcc accepts device lambdas only with
``--extended-lambda``, which EBGeometry does not require.

GPU::transform over vectors
---------------------------

``GPU::transform(query, inputs)`` works like ``std::transform`` or ``thrust::transform``: result
``i`` is ``query(inputs[i])``. The query is a functor that maps one input to one result and holds what
it evaluates by value. The inputs and the results are host ``std::vector`` objects:

.. code-block:: c++

   struct Distance
   {
     TriMeshSDF<T, K, W> m_sdf; // rebased onto a mirror in GPU::memoryResource()

     EBGEOMETRY_HOST_DEVICE T
     operator()(const Vec3T<T>& a_point) const noexcept { return m_sdf.signedDistance(a_point); }
   };

   const std::vector<T> distances = GPU::transform(Distance{deviceSDF}, points);

In a CUDA or HIP build, one call does all of this:

#. allocates device memory for the inputs and for the results;
#. copies the inputs to the device;
#. launches a kernel with one thread per input, and waits for it to finish;
#. copies the results back to the host;
#. frees both allocations.

It is the simplest way to evaluate a batch of points, and the one the examples use. **Timing it
includes the two allocations and the two copies**, which for a cheap query over many points can
cost more than the kernel. Repeated queries over the same points also copy them every time. Use the
span overload below for either.

GPU::transform over spans
-------------------------

``GPU::transform(query, in, out)`` takes the inputs and the results as ``PODSpan`` objects (a pointer
and a count) and writes ``out[i] = query(in[i])``. It allocates nothing and copies nothing: the data
stays wherever the caller put it. That is the call to time when a query's cost is what is being
measured, and the one to use for repeated queries over the same points.

The spans come from the same memory model as everything else. Put the points and room for the
results in a pool, freeze it, mirror it into ``GPU::memoryResource()``, and bind each ``PODVector``
against the mirror's base:

.. code-block:: c++

   Pool                hostPool(hostMemoryResource());
   PODVector<Vec3T<T>> in;
   PODVector<T>        out;

   in.reserveFrom(hostPool, n);
   out.reserveFrom(hostPool, n);
   in.assign(hostPool.base(), points.data(), n);
   out.assign(hostPool.base(), zeros.data(), n);
   hostPool.freeze();

   const Pool devicePool = Pool::mirror(hostPool, GPU::memoryResource());

   GPU::transform(query, in.bind(devicePool.base()), out.bind(devicePool.base())); // the kernel only

   const Pool back = Pool::mirror(devicePool, hostMemoryResource());               // results to the host

Freezing forbids new reservations, not writes, so the kernel can write the results into the
mirror. A span may equally point at managed memory, or at a buffer an application already owns on
the device. Since a span is only a pointer and a count, ``transform`` cannot tell where its memory
lives: spans over ordinary host memory (a ``std::vector``'s data, a pool in ``hostMemoryResource()``)
compile and run in a host build but fault in a kernel. Bind them against a pool in
``GPU::memoryResource()`` or in managed memory. :ref:`Chap:ExampleManyObjects` times its device queries this way.

GPU::parallelFor
----------------

``GPU::parallelFor(n, f)`` runs ``f(i)`` for every ``i < n``, one thread per index, in no particular
order. It is the primitive that both ``transform`` overloads are built on. A grid holds at most
:math:`2^{31} - 1` blocks, so ``n`` may not exceed that many times the block size; a larger count
aborts with a message, in every build. Use it directly for work
indexed by an integer rather than by an input value, for example a query on each point of a cloud by
its index. The functor writes its own results, through a span it holds.

Block size
----------

Every launch takes the number of threads per block as its first template argument. The default is
``GPU::DefaultBlockSize`` (128):

.. code-block:: c++

   GPU::transform(query, points);        // 128 threads per block
   GPU::transform<256>(query, points);   // 256
   GPU::parallelFor<64>(n, f);           // 64

The block size is a template parameter rather than a function argument so that the kernel can carry
it as ``__launch_bounds__``. That tells the compiler no block is larger, so it can give each thread
as many registers as that block size allows rather than planning for the largest possible block
(1024 threads). Queries that traverse a BVH use many registers per thread, so this can matter. A
block size must be a positive multiple of 64 (an AMD wavefront, and two NVIDIA warps) and at most
1024, which is checked at compile time.

The default of 128 has not been tuned. The best block size depends on the device and on the query,
so measure it for your own case with the span overload.
