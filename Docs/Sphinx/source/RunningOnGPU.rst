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

     std::vector<T> distances(points.size());

     GPU::transform(Distance{sdf.rebasedView(devicePool)}, points, distances);
   }

Every call below is synchronous: it returns once the kernel has finished. Like ``std::transform`` and
``thrust::transform``, every ``transform`` writes into an output the caller provides: it requires as
many results as inputs, and results of exactly the query's result type, and it never resizes or
allocates the output. The results may overwrite the inputs in place (the same array, when the input
and result types are the same), but otherwise must not overlap them; a partial overlap aborts with a
message. Every runtime call
they make is checked, and an error aborts with the call that failed. Their functors must be
trivially copyable, which is checked at compile time in both builds, so a functor that could not
reach a kernel fails to compile on the host too. Write a functor as a struct with an
``EBGEOMETRY_HOST_DEVICE`` call operator, not as a lambda: nvcc accepts device lambdas only with
``--extended-lambda``, which EBGeometry does not require.

There are three ``transform`` overloads, which differ only in where the inputs and the results
live:

.. list-table::
   :header-rows: 1
   :widths: 40 30 15 15

   * - Call
     - Inputs and results
     - Copies
     - Memory checked
   * - ``transform(query, in, out)``, ``std::vector``
     - host vectors
     - inside the call
     - yes
   * - ``transform(query, pool, in, out)``, ``PODVector``
     - one frozen, device-accessible pool
     - none
     - yes
   * - ``transform(query, in, out)``, ``PODSpan``
     - anywhere
     - none
     - no

GPU::transform over vectors
---------------------------

``GPU::transform(query, in, out)`` over two ``std::vector`` objects writes ``query(in[i])`` to
``out[i]``. The query is a functor that maps one input to one result and holds what it evaluates by
value:

.. code-block:: c++

   struct Distance
   {
     TriMeshSDF<T, K, W> m_sdf; // rebased onto a mirror in GPU::memoryResource()

     EBGEOMETRY_HOST_DEVICE T
     operator()(const Vec3T<T>& a_point) const noexcept { return m_sdf.signedDistance(a_point); }
   };

   std::vector<T> distances(points.size());   // sized by the caller, reusable across calls

   GPU::transform(Distance{deviceSDF}, points, distances);

In a CUDA or HIP build, one call does all of this:

#. allocates device memory for the inputs and for the results;
#. copies the inputs to the device;
#. launches a kernel with one thread per input, and waits for it to finish;
#. copies the results back into ``out``;
#. frees both allocations.

It is the simplest way to evaluate a batch of points, and the one most examples use. **Timing it
includes the two allocations and the two copies**, which for a cheap query over many points can
cost more than the kernel. Repeated queries over the same points also copy them every time. Use the
pool overload below for either.

GPU::transform over a pool
--------------------------

``GPU::transform(query, pool, in, out)`` takes the inputs and the results as two ``PODVector``
arrays in one pool, and writes ``out[i] = query(in[i])`` there. It allocates nothing and copies
nothing: the data stays in the pool. That is the call to time when a query's cost is what is being
measured, and the one to use for repeated queries over the same points.

A ``PODVector`` is an offset into a pool, so the call takes the pool too. Before it launches, it
checks that the pool is frozen (as ``rebasedView()`` requires of a device-accessible pool) and that
its memory is device-accessible, and that both arrays end inside it, and aborts with a message
otherwise. (A ``PODVector`` is an offset, not a reference to its pool, so that an array really
belongs to *this* pool or a mirror of it remains the caller's responsibility.) The inputs are bound
read-only. A pool left in
``hostMemoryResource()`` is therefore caught in every build, before any kernel runs. Put the points
and room for the results in a pool, freeze it, and mirror it into ``GPU::memoryResource()``:

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

   GPU::transform(query, devicePool, in, out);                         // the kernel only

   const Pool back = Pool::mirror(devicePool, hostMemoryResource());   // results to the host

Freezing forbids new reservations, not writes, so the kernel can write the results into the mirror.
A pool built directly in managed memory and frozen works the same way. :ref:`Chap:ExampleManyObjects`
times its device queries like this.

GPU::transform over spans
-------------------------

``GPU::transform(query, in, out)`` over two ``PODSpan`` objects (a pointer and a count) is the call
the other two are built on. Like the pool overload it allocates and copies nothing, but a span may
point anywhere: at managed memory, at a buffer an application already owns on the device (an AMReX
or Kokkos array), or at a ``PODVector`` bound with ``bind()`` against a pool's base. That is also its
one hazard: since a span is only a pointer and a count, ``transform`` cannot tell where its memory
lives, so spans over ordinary host memory (a ``std::vector``'s data, a pool in
``hostMemoryResource()``) compile and run in a host build but fault in a kernel. Prefer the pool
overload whenever the data is in an EBGeometry pool.

GPU::parallelFor
----------------

``GPU::parallelFor(n, f)`` runs ``f(i)`` for every ``i < n``, one thread per index, in no particular
order. It is the primitive that every ``transform`` overload is built on. A grid holds at most
:math:`2^{31} - 1` blocks, so ``n`` may not exceed that many times the block size; a larger count
aborts with a message, in every build. Use it directly for work
indexed by an integer rather than by an input value, for example a query on each point of a cloud by
its index. The functor writes its own results, through a span it holds.

Large queries
-------------

``parallelFor`` and ``transform`` pass the query (or the functor) to the kernel by value. When the query
calls a function that is not inlined -- a ``Tape``'s evaluator, say -- the compiler needs the query's
address, and would copy the whole query into every thread's local memory first: for a ``Tape``, 376
bytes per thread, which cost more than the evaluation itself (issue #221). Under CUDA 11.7 and later the
kernel's parameter is therefore a grid constant (``EBGEOMETRY_GRID_CONSTANT``, ``__grid_constant__``),
which the threads read in place. That needs compute capability 7.0 or later, the default architecture
of every EBGeometry CUDA build; when a build also compiles for an older architecture, the macro is
empty in that architecture's device code, which copies the query as before. HIP has no such qualifier;
there the macro is empty. A kernel written by hand (see :ref:`Chap:ExampleCustomShape`) can mark its
own query parameter the same way.

Block size
----------

Every launch takes the number of threads per block as its first template argument. The default is
``GPU::DefaultBlockSize`` (128):

.. code-block:: c++

   GPU::transform(query, points, distances);        // 128 threads per block
   GPU::transform<256>(query, points, distances);   // 256
   GPU::parallelFor<64>(n, f);                      // 64

The block size is a template parameter rather than a function argument so that the kernel can carry
it as ``__launch_bounds__``. That tells the compiler no block is larger, so it can give each thread
as many registers as that block size allows rather than planning for the largest possible block
(1024 threads). Queries that traverse a BVH use many registers per thread, so this can matter. A
block size must be a positive multiple of 64 (an AMD wavefront, and two NVIDIA warps) and at most
1024, which is checked at compile time.

The default of 128 has not been tuned. The best block size depends on the device and on the query,
so measure it for your own case with the pool overload.
