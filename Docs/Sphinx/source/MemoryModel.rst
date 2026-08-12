.. _Chap:MemoryModel:

Memory model
============

Underneath the DCEL mesh representation (:ref:`Chap:ImplemDCEL`), and progressively the rest of
the library, sits a small, self-contained memory foundation: a placement policy
(`MemoryResource <doxygen/html/classEBGeometry_1_1MemoryResource.html>`__), a build-then-freeze
arena over it (`Pool <doxygen/html/classEBGeometry_1_1Pool.html>`__), and a placement-independent
array descriptor stored inside it
(`PODVector <doxygen/html/structEBGeometry_1_1PODVector.html>`__/
`PODSpan <doxygen/html/structEBGeometry_1_1PODSpan.html>`__). None of this is a general-purpose
allocator: it exists to solve one specific problem, described below, and its API is shaped
entirely around that problem. Most users reading a mesh from a file (:ref:`Chap:Parsers`) only
ever touch a ``Pool`` at the call site (construct one, pass it in, keep it alive) without needing
the rest of this page -- it is aimed at readers who want to understand *why* the API looks the way
it does, who are storing their own data alongside a mesh, or who are debugging a use-after-move/
use-after-grow style failure.

.. contents:: On this page
   :local:
   :depth: 1

Why a custom memory model?
---------------------------

EBGeometry is header-only with no external dependencies, and one of its design goals (see the GPU
port described throughout this section) is that the exact same data -- a DCEL mesh's vertices,
half-edges, and faces -- can be built once on the host and then queried either on the host or on a
CUDA/HIP device, without maintaining two separate representations. A ``std::vector``, or any
container that stores an absolute pointer to its own data, cannot do this: a pointer that is valid
in host address space is meaningless (or outright invalid) once the same bytes are copied into
device memory. Every structure built on this memory model instead stores a **byte offset relative
to a base address that is supplied separately**, so the identical bit pattern resolves correctly
against a host base *and* against a device base after a byte-for-byte mirror -- with no pointer
patching pass required.

.. important::

   The recurring idea across every class on this page is: **store an offset, not a pointer; the
   base is supplied by the caller.** Everything else -- ``Pool``'s two phases, ``PODVector``'s two
   access styles, ``mirror()`` -- follows from that one decision.

``MemoryResource``: where bytes live
--------------------------------------

`MemoryResource <doxygen/html/classEBGeometry_1_1MemoryResource.html>`__ is an abstract,
runtime-virtual placement policy: it decides *where* a block of bytes physically lives, without
its caller needing to know which. Concrete resources are always available:

.. list-table::
   :header-rows: 1
   :widths: 30 20 20 30

   * - Resource
     - Host-accessible
     - Device-accessible
     - Notes
   * - `HostMemoryResource <doxygen/html/classEBGeometry_1_1HostMemoryResource.html>`__
     - Yes
     - No
     - Always compiled; plain aligned host allocation. Obtain the process-wide default via
       ``EBGeometry::hostMemoryResource()``.
   * - `DeviceMemoryResource <doxygen/html/classEBGeometry_1_1DeviceMemoryResource.html>`__
     - No
     - Yes
     - CUDA/HIP builds only. A host pointer dereference of a returned block is undefined
       behaviour.
   * - `ManagedMemoryResource <doxygen/html/classEBGeometry_1_1ManagedMemoryResource.html>`__
     - Yes
     - Yes
     - CUDA/HIP builds only. One allocation reachable from both host and device, at the cost of
       driver-managed page migration.
   * - `PinnedMemoryResource <doxygen/html/classEBGeometry_1_1PinnedMemoryResource.html>`__
     - Yes
     - No
     - CUDA/HIP builds only. Page-locked host memory; faster/asynchronous transfers to a device
       than ordinary pageable memory.
   * - `MappedMemoryResource <doxygen/html/classEBGeometry_1_1MappedMemoryResource.html>`__
     - Yes
     - Yes
     - CUDA/HIP builds only. Page-locked *and* mapped into the device address space -- no
       migration, the device reads host RAM in place over the interconnect.

Allocation and deallocation are always host-only operations (memory is never obtained from device
code, even for a device-resident resource), and every allocated block is aligned to
``EBGeometry::PoolBaseAlign`` (256 bytes), which is at least as strict as everything the library
ever stores in a pool, including SIMD-width SoA blocks.

.. note::

   A ``MemoryResource`` is non-copyable: its identity is meaningful (two ``Pool``\ s built over the
   *same* resource share a placement), and copying an allocator has no meaning. Pass it by
   reference, and it must outlive every ``Pool`` built over it.

``Pool``: the build/freeze/query arena
------------------------------------------

A `Pool <doxygen/html/classEBGeometry_1_1Pool.html>`__ is a single contiguous byte block obtained
from a ``MemoryResource``. It has exactly two phases, and moving between them is a one-way
operation:

#. **Build.**
   `reserve(count, elemSize, alignment) <doxygen/html/classEBGeometry_1_1Pool.html#a0c1f5b771e29cce6ae10fae19b766fb7>`__
   bump-allocates a sub-region and returns its byte offset from
   `base() <doxygen/html/classEBGeometry_1_1Pool.html#a049dfaa90b3aa55a410ce19b358fc583>`__.
   Nothing is ever individually freed. If the block is too small, ``reserve`` transparently grows
   it: it allocates a new, larger block (geometric doubling), ``memcpy``\ s every byte currently in
   use into it, and frees the old block -- so ``base()`` can change on *any* ``reserve()`` call
   during this phase.
#. **Query.**
   `freeze() <doxygen/html/classEBGeometry_1_1Pool.html#a7c5696404d11babfad42fc7d99b37ebd>`__ is
   the single synchronization point between the two phases: afterwards, ``reserve()`` is forbidden
   (an ``EBGEOMETRY_EXPECT``-checked precondition, see :ref:`Sec:Assertions`), ``base()`` is
   guaranteed stable for the remaining lifetime of the pool, and
   `mirror() <doxygen/html/classEBGeometry_1_1Pool.html#aa141bf4919e1aeb78aaee9667da8ebc3>`__
   becomes available.

.. code-block:: c++

   EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

   // Build phase: reserve()/push_back()-style calls, offsets resolved against a freshly-read
   // pool.base() every time -- never a cached pointer.
   ...

   pool.freeze();                 // one-way: no unfreeze().
   void* base = pool.base();      // now guaranteed stable.

``mirror(src, dstResource)`` copies a *frozen* source pool's entire block, byte-for-byte, into a
fresh block from ``dstResource`` (a plain ``memcpy`` for a host-to-host copy, or a
``GPU::memcpy`` for a host/device transfer), and returns the result already frozen. Because every
offset stored inside the block is relative rather than absolute, the copy is immediately usable
against its own (possibly device-resident) ``base()`` -- this is the host-to-device upload, and,
used host-to-host, an exact independent copy.

.. warning::

   ``freeze()`` has no inverse. Once a pool is frozen it can never be reserved into again, even to
   append more of the same kind of data. If you need to build several batches of data (e.g. several
   mesh files) into one shared pool, do all of the building *before* the first ``freeze()`` call --
   see the pitfalls in :ref:`Sec:DCELMemoryModel` for what this means in practice for
   ``DCEL::MeshT``.

.. warning::

   Never cache the result of ``base()`` across a call that might trigger another ``reserve()``
   (directly, or indirectly through anything still in its build phase). A block move during
   ``grow()`` silently invalidates any pointer computed against the old base; there is no way to
   detect this after the fact; the failure mode is a wild pointer, not a diagnosable error. Always
   re-read ``base()`` immediately before using it, and treat it as disposable in between.

.. note::

   ``Pool`` is move-only, RAII, and deliberately *not* trivially copyable -- it owns a block and a
   ``MemoryResource*`` with real lifetime semantics. This is by design: it is one of only two
   non-trivial types anywhere in this memory model (``MemoryResource`` is the other). Everything
   *stored inside* a ``Pool`` -- every ``PODVector``, and every structure built entirely from
   ``PODVector``\ s and plain values -- is trivially copyable POD; the ``Pool`` itself never is,
   and is never mirrored to a device or passed to device code directly.

``PODVector`` and ``PODSpan``: placement-independent storage
------------------------------------------------------------------

A `PODVector\<T\> <doxygen/html/structEBGeometry_1_1PODVector.html>`__ is a trivially-copyable,
16-byte descriptor (``uint64_t`` offset + ``uint32_t`` size + ``uint32_t`` capacity -- no pointer
at all) of a contiguous array of ``T`` living inside a ``Pool``. ``T`` itself must be trivially
copyable, since it is device-visible storage.

.. code-block:: c++

   EBGeometry::PODVector<MyPodType> vec;

   vec.reserveFrom(pool, 128);              // build phase, must not be frozen
   vec.push_back(pool.base(), someValue);   // never reallocates -- capacity is fixed at reserveFrom()

Two access styles are provided, and choosing between them is the main day-to-day decision this
memory model asks of a caller:

.. list-table::
   :header-rows: 1
   :widths: 25 35 40

   * - Style
     - Methods
     - When to use it
   * - Resolve every call
     - ``at(base, i)``, ``data(base)``
     - The default. Resolves ``base + offset`` fresh on every call, so it is correct during the
       build phase, after ``freeze()``, and against any base you have in hand -- host or (mirrored)
       device. Costs one extra addition per access.
   * - Bind once
     - ``bind(base) -> PODSpan<T>``
     - A raw ``pointer + size`` pair for a genuinely hot inner loop (SIMD traversal, a tight BVH
       query), where re-deriving the pointer every iteration is measurable overhead. Only valid
       against a **frozen** pool -- the freeze contract is exactly what makes a captured raw
       pointer safe here.

.. important::

   ``PODSpan<T>`` (returned by ``bind()``) captures a raw pointer. It is only ever safe to take
   against a frozen (or already-mirrored) pool, whose base cannot move again -- taking it during
   the build phase and holding it across a later ``reserve()``/``grow()`` reproduces exactly the
   dangling-pointer hazard described above for ``base()`` itself, just one level removed. Prefer
   ``at()``/``data()`` unless you have measured that the resolve-every-call cost actually matters.

.. code-block:: c++

   pool.freeze();

   const auto span = vec.bind(pool.base());   // safe: pool is frozen, base is stable

   for (const auto& v : span) { ... }          // hot loop, no per-element re-resolution

.. tip::

   If a type you are adding needs to be part of this memory model (stored inside a ``Pool``,
   mirrored to a device), give it only ``PODVector`` members and other plain, trivially-copyable
   values -- never a raw pointer or a container that owns its own allocation. A
   ``static_assert(std::is_trivially_copyable_v<...>)`` at the bottom of the class, matching the one
   on every class described here, is cheap insurance that a later change does not silently break
   the contract.

See :ref:`Chap:ImplemDCEL` for how ``DCEL::MeshT`` -- the main consumer of this memory model today
-- is built on top of ``Pool``/``PODVector``, including the pitfalls specific to it.
