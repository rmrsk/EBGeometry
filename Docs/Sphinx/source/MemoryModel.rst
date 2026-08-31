.. _Chap:MemoryModel:

Memory model
============

Underneath the DCEL mesh representation (:ref:`Chap:ImplemDCEL`), and progressively the rest of
the library, sits a small, self-contained memory foundation: a placement policy
(`MemoryResource <doxygen/html/classEBGeometry_1_1MemoryResource.html>`__), a bump-allocating
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
   base is supplied by the caller.** Everything else -- ``Pool``'s control block, ``PODVector``'s
   two access styles, ``mirror()`` -- follows from that one decision.

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

``Pool``: the arena
------------------------------------------

A `Pool <doxygen/html/classEBGeometry_1_1Pool.html>`__ is a single contiguous byte block obtained
from a ``MemoryResource``.
`reserve(count, elemSize, alignment) <doxygen/html/classEBGeometry_1_1Pool.html#a0c1f5b771e29cce6ae10fae19b766fb7>`__
bump-allocates a sub-region and returns its byte offset from
`base() <doxygen/html/classEBGeometry_1_1Pool.html#a049dfaa90b3aa55a410ce19b358fc583>`__. Nothing is
ever individually freed. If the block is too small, ``reserve`` transparently grows it: it allocates
a new, larger block (geometric doubling), ``memcpy``\ s every byte currently in use into it, and
**frees the old block** -- so ``base()`` can change on any ``reserve()`` call.

Building and querying are not separate phases. A pool-resident object is usable from the moment its
storage is reserved, including while the same pool keeps being built into, because it does not
remember an address: it holds a pointer to the pool's *control block*
(`PoolControl <doxygen/html/structEBGeometry_1_1PoolControl.html>`__), a small heap-resident record
holding the current base, and re-reads the base through it on every access. ``grow()`` publishes the
new base there, so a block move is invisible to every object resolving through it.

.. code-block:: c++

   EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

   auto first  = EBGeometry::Parser::readIntoMesh<T>("a.stl", pool);
   const T d   = first->signedDistance(x);      // queryable immediately

   auto second = EBGeometry::Parser::readIntoMesh<T>("b.stl", pool);   // may grow and move the block

   const T same = first->signedDistance(x);     // ... which changes nothing here

The control block is also why a ``Pool`` can be moved freely -- into a container, into a class
member, out of a factory -- without disturbing anything built into it. A move transfers ownership of
the control block, whose address does not change. What a pool-resident object cannot survive is the
pool being *destroyed*: the block dies with it, so the pool must outlive everything reserved from
it.

``freeze()``: the mirror precondition
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

`freeze() <doxygen/html/classEBGeometry_1_1Pool.html#a7c5696404d11babfad42fc7d99b37ebd>`__ seals a
pool: ``reserve()`` is forbidden afterwards (an ``EBGEOMETRY_EXPECT``-checked precondition, see
:ref:`Sec:Assertions`) and ``base()`` can no longer move. It exists for exactly one reason -- it is
the precondition for
`mirror() <doxygen/html/classEBGeometry_1_1Pool.html#aa141bf4919e1aeb78aaee9667da8ebc3>`__, since
you cannot take a byte-for-byte copy of a block that might still be reallocated. Freezing is *not*
required to query anything.

``mirror(src, dstResource)`` copies a frozen source pool's entire block, byte-for-byte, into a fresh
block from ``dstResource`` (a plain ``memcpy`` for a host-to-host copy, or a ``GPU::memcpy`` for a
host/device transfer), and returns the result already frozen. Because every offset stored inside the
block is relative rather than absolute, the copy is immediately usable against its own (possibly
device-resident) ``base()`` -- this is the host-to-device upload, and, used host-to-host, an exact
independent copy. A mirror records the identity of the pool it ultimately came from
(``mirrorOf()``), which is what lets an object built in the original pool verify that it is being
rebased onto a faithful copy of its own storage; mirroring through intermediate pools (host to
pinned staging to device) preserves that identity across every hop.

.. warning::

   ``freeze()`` has no inverse. Once a pool is frozen it can never be reserved into again. Since
   freezing is only needed in order to mirror, the practical rule is simply to do it last.

.. _Sec:ResolvedAddresses:

Resolved addresses do not survive a ``reserve``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The control block protects anything that *re-resolves*. It cannot protect an address that has
already been resolved, and a ``reserve`` that grows the pool deallocates the block such an address
points into:

.. code-block:: c++

   auto& e = mesh->getEdge(3);      // a raw address, resolved right now
   mesh->reserveFaces(pool, n);     // may grow -- the old block is freed
   e.setFace(7);                    // undefined behaviour: write into freed memory

The hazard is intermittent (a grow only happens when a request exceeds the current capacity) and its
quiet form is worse than a crash: if the freed block is still mapped, the write lands in the
abandoned copy and is silently lost, while the object itself now reads from the new block.

.. important::

   **Resolve, use, discard.** A pointer, reference or ``PODSpan`` obtained from pool storage must
   not outlive the next ``reserve()`` on that pool. To carry data across a ``reserve``, carry it by
   *value* and write it back through a fresh resolution:

   .. code-block:: c++

      Edge e = mesh->getEdge(3);      // snapshot, independent of any base
      mesh->reserveFaces(pool, n);    // may grow
      e.setFace(7);
      mesh->getEdge(3) = e;           // fresh resolution against the current base

   Everything that returns *by value* -- ``signedDistance()``, ``getAllVertexCoordinates()``, and so
   on -- is unaffected, which is the overwhelming majority of the query surface.

.. note::

   For the same reason, querying an object while another thread reserves from the same pool is a
   data race: the reader may observe a base being republished, or have the block reallocated
   underneath it. Pools are not internally synchronized. Finish building before sharing a pool
   across threads, or serialize the reserves.

.. note::

   ``Pool`` is move-only, RAII, and deliberately *not* trivially copyable -- it owns a block, a
   control block and a ``MemoryResource*`` with real lifetime semantics. This is by design: it is
   one of only two non-trivial types anywhere in this memory model (``MemoryResource`` is the
   other). Everything *stored inside* a ``Pool`` -- every ``PODVector``, and every structure built
   entirely from ``PODVector``\ s and plain values -- is trivially copyable POD; the ``Pool`` itself
   never is, and is never mirrored to a device or passed to device code directly.

``PODVector`` and ``PODSpan``: placement-independent storage
------------------------------------------------------------------

A `PODVector\<T\> <doxygen/html/structEBGeometry_1_1PODVector.html>`__ is a trivially-copyable,
16-byte descriptor (``uint64_t`` offset + ``uint32_t`` size + ``uint32_t`` capacity -- no pointer
at all) of a contiguous array of ``T`` living inside a ``Pool``. ``T`` itself must be trivially
copyable, since it is device-visible storage.

.. code-block:: c++

   EBGeometry::PODVector<MyPodType> vec;

   vec.reserveFrom(pool, 128);              // must not be frozen
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
     - The default. Resolves ``base + offset`` fresh on every call, so it is correct while the pool
       is still being built into, after ``freeze()``, and against any base you have in hand -- host
       or (mirrored) device. Costs one extra addition per access.
   * - Bind once
     - ``bind(base) -> PODSpan<T>``
     - A raw ``pointer + size`` pair for a genuinely hot inner loop (SIMD traversal, a tight BVH
       query), where re-deriving the pointer every iteration is measurable overhead. Safe only for
       as long as no further ``reserve()`` happens on the pool.

.. important::

   ``PODSpan<T>`` (returned by ``bind()``) captures a raw pointer, so it is exactly the
   resolved-address hazard of :ref:`Sec:ResolvedAddresses` in container form: it must not outlive
   the next ``reserve()``. Taking one against a frozen (or already-mirrored) pool removes the
   question entirely, since such a pool can never be reserved into again. Prefer ``at()``/``data()``
   unless you have measured that the resolve-every-call cost actually matters.

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
