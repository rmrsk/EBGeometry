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

This page describes the second, and ends with the launch settings (:ref:`Sec:LaunchSettings`), which
a kernel written by hand can copy. See :ref:`Sec:BuildingGPU` for how to compile for a GPU and
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

Rendering on a GPU
------------------

``Render::Renderer::renderOnDevice(deviceShape, camera, image)`` renders an image through
``GPU::parallelFor``: one ``Render::PixelQuery`` per pixel index, with the shape passed as a view of a
mirrored pool, exactly as for a query (:ref:`Chap:ImplemRender`). It renders the image in up to eight
bands of rows, one launch each (so that a render can be stopped between them), into one buffer in device
memory, copied back to the host once. ``Render::view()`` takes the same device view to render its window
on the GPU, or the pool, which it then mirrors itself.

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
address, and would copy the whole query into every thread's local memory first: for a ``Tape``, a few
hundred bytes per thread, which cost more than the evaluation itself (issue #221). Under CUDA 11.7 and
later the kernel's parameter is therefore a grid constant (``EBGEOMETRY_GRID_CONSTANT``,
``__grid_constant__``), which the threads read in place. That needs compute capability 7.0 or later, the
oldest architecture an EBGeometry CUDA build defaults to; when a build also compiles for an older
architecture, the macro is empty in that architecture's device code, which copies the query as before.
HIP has no such qualifier; there the macro is empty. A kernel written by hand (see
:ref:`Chap:ExampleCustomShape`) can mark its own query parameter the same way.

.. _Sec:LaunchSettings:

Launch settings
---------------

Every ``parallelFor`` and ``transform`` takes two optional template arguments. Each pair of values is a
kernel of its own, compiled in advance:

.. list-table::
   :header-rows: 1
   :widths: 30 20 50

   * - Argument
     - Default
     - What it sets
   * - ``BlockSize``
     - ``GPU::DefaultBlockSize`` (128)
     - The threads per block. A positive multiple of 64 (an AMD wavefront, and two NVIDIA warps), at
       most 1024, checked at compile time.
   * - ``MinResidentThreads``
     - ``GPU::DefaultMinResidentThreads`` (384)
     - The threads the CUDA compiler is asked to keep resident on each multiprocessor, at least, which
       fixes how many registers each thread may use. At most 2048; 0 asks for no minimum. A HIP build
       ignores it.

.. code-block:: c++

   using namespace EBGeometry;

   GPU::transform(query, devicePool, in, out);             // the defaults: 128 threads per block, 384 resident
   GPU::transform<256>(query, devicePool, in, out);        // 256 threads per block, 384 resident
   GPU::transform<128, 768>(query, devicePool, in, out);   // 768 resident: at most 80 registers a thread
   GPU::transform<128, 0>(query, devicePool, in, out);     // no minimum: the compiler picks the budget
   GPU::parallelFor<64>(n, f);                             // the same arguments for parallelFor

Why the block size is a template argument
   It lets the kernel carry it as ``__launch_bounds__``, which tells the compiler no block is larger,
   so it can give each thread as many registers as that block size allows rather than planning for the
   largest possible block (1024 threads). Queries that traverse a BVH use many registers per thread, so
   this matters.

Why there is a minimum of resident threads
   Left to itself, the CUDA compiler picks one register budget for a whole kernel, every function it
   calls included, and the pick jumps with small changes to code the kernel may not even run: a tape's
   ray kernel once went from 255 registers to 40, and ran up to twice as slow, after a few lines were
   added elsewhere in the ray code. Asking for a minimum fixes the budget at what that minimum allows.
   A multiprocessor has 65,536 registers, shared by its resident threads and allocated a warp at a
   time, and a thread may use at most 255. Both hold on every NVIDIA GPU since Maxwell, so the budget is
   about 65,536 divided by the minimum, rounded down, whatever the device:

   .. list-table::
      :header-rows: 1
      :widths: 40 60

      * - ``MinResidentThreads``
        - Registers per thread, at most
      * - 0
        - whatever the compiler picks (up to 255)
      * - 256
        - 255
      * - 384 (the default)
        - 168
      * - 768
        - 80
      * - 1024
        - 64

   The kernel asks for blocks, not threads: the minimum, divided by the block size and rounded up. What
   differs between devices is how many threads a multiprocessor can hold at all (1,024 on Turing, 1,536
   on consumer Ampere and Ada, 2,048 on Volta, the A100 and Hopper), which bounds how much a smaller
   budget can gain. Those blocks must fit: when they hold more threads than the architecture being
   compiled for can keep resident (1,024 rounded up to blocks of 384 is 1,152 threads, more than Turing
   holds; 2,048 is more than consumer Ampere holds), ``ptxas`` warns that ``.minnctapersm will be ignored`` and the kernel gets no minimum
   at all, as with 0. Keep the minimum, rounded up to whole blocks, within the smallest of these for the
   architectures a build compiles for.

   The budget is a ceiling the compiler tends to fill. A kernel that needed fewer registers may then use
   more, and fewer of its threads fit on a multiprocessor: under the default, rays traced through a
   fused composition (``Fused::``) went from 56 registers to 161 and ran up to a third slower, and the
   renderer's frame of a mesh from 128 to 161, 10% slower. Such a kernel can run better with a larger
   minimum, or with none.

Which setting is fastest
   It depends on the query, the device and the precision. Measured on one desktop GPU, ray tracing
   through a tape took 2.5 times as long with no minimum as with the default, and 1.45 times as long at
   768; a mesh's signed distance took the same time at every setting. The default has not been tuned for
   every query, and a kernel that runs best with few registers may be slower under it. An application
   that launches one query many times can time a few settings on a sample of its queries at startup and
   keep the fastest, as :ref:`Chap:ExampleDeviceTuning` does. Time the pool overload, which allocates and
   copies nothing.

The same in a kernel of your own
   An application that writes its own kernel (see :ref:`Chap:ExampleCustomShape`) meets the same
   register budget, and can fix it the same way, with a launch bound of its own. This is what
   ``GPU::transform<128, 384>`` compiles to, written by hand:

   .. code-block:: c++

      constexpr unsigned int BlockSize          = 128;
      constexpr unsigned int MinResidentThreads = 384;

      template <class Query, class In, class Out>
      __global__ void
      __launch_bounds__(BlockSize, (MinResidentThreads + BlockSize - 1) / BlockSize) // 3 blocks: <= 168 registers
      evaluate(EBGEOMETRY_GRID_CONSTANT const Query query, const In* in, Out* out, std::size_t n)
      {
        const std::size_t i = std::size_t(blockIdx.x) * BlockSize + threadIdx.x;

        if (i < n) {
          out[i] = query(in[i]);
        }
      }

      evaluate<<<(n + BlockSize - 1) / BlockSize, BlockSize>>>(query, in, out, n);

   The grid must be launched with the block size the bound names; a larger block fails to launch.
   Kokkos's ``LaunchBounds<MaxThreads, MinBlocks>`` takes the same two numbers; AMReX's ``ParallelFor``
   takes only the block size (``MaxThreads``). Check what the compiler did with
   ``-Xptxas -v``, which prints each kernel's registers and the bytes it spills.
