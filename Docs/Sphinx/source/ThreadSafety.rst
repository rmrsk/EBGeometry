.. _Chap:ThreadSafety:

Thread safety
=============

EBGeometry starts no threads of its own, except in the renderer and the viewer. A program that queries geometry
from several host threads, for example an OpenMP loop over particles that each ask for the distance to a wall,
needs one rule: **build first, then query from as many threads as you like.** This page says what that rule
covers, and what it does not.

.. contents:: On this page
   :local:
   :depth: 1

Queries are safe to call concurrently
-------------------------------------

Every query is a ``const`` function that keeps its working state on the calling thread's stack and changes
nothing in the object or anywhere else. Any number of threads may therefore call queries on the same object at
the same time, and get the same answers as one thread would. This holds for:

- the distance and closest-point queries of every shape, mesh SDF, BVH union, fused composition and ``Tape``,
  including ``closestLeaf()``, ``closestSurface()`` and the region and label queries;
- ray queries (``firstHit()``, ``intersects()``, a tape's first crossing);
- the point and sphere cloud queries of ``PointCloudBVH``, ``PointCloudHashGrid`` and ``SphereCloudBVH``,
  each thread passing its own output array to a k-nearest query;
- a model read from the :ref:`.ebg language <Chap:Language>` and the domains built from it;
- ``PackedBVH::traverse()``, as long as the query functor a thread passes does not share writable state with
  another thread's;
- ``GPU::transform()`` and ``GPU::parallelFor()``. In a host build they are plain loops on the calling thread.
  In a GPU build each call waits for the device to finish before it returns, so calls from several threads
  are safe but take turns on the device.

A query that a ``Tape`` or a model makes on a mesh or a cloud inside it is one of these, so it is safe too.

Building and changing an object are not
---------------------------------------

The objects live in a ``Pool`` (see :ref:`Chap:MemoryModel`), and a pool is not synchronized. Anything that
writes to a pool must not run while another thread uses the same pool. That means:

- **building into a pool** (a mesh SDF, a BVH, a cloud, a ``Tape``, a model read from a file) while another
  thread queries any object in that pool, even a different object: adding to a pool can move its whole block
  of memory;
- **two builds into the same pool** at once;
- **changing an object in place** while it is queried: ``BVH::refit()``, ``DCEL::reconcile()``, flipping a
  mesh, or rebuilding a point or sphere cloud.

Copying a handle does not help. A copy of a ``MeshSDF``, a ``Tape`` or a cloud refers to the same pool, so a
change made through one copy is a change to all of them. A thread gets an object of its own only with
``deepCopy()`` into a pool of its own.

Calling ``Pool::freeze()`` once building is done turns an accidental later build into that pool into an
immediate abort rather than a race.

What may be built in parallel
-----------------------------

Builds into **different pools** may run at the same time, from the same sources. The sources are only read:
one ``Expr`` may be compiled into tapes on several threads, one ``PolygonSoup`` converted into meshes on several
threads, and several files read or several ``.ebg`` models parsed at once.

An OpenMP particle loop
-----------------------

.. code-block:: c++

   Pool pool(hostMemoryResource());

   const auto    model = Language::read<double>("geometry.ebg", pool);
   const Tape<double> wall(pool, model.object());

   pool.freeze();   // nothing is built into it from here on

   #pragma omp parallel for
   for (std::size_t i = 0; i < particles.size(); i++) {
     const double d = wall.signedDistance(particles[i].position);   // safe: a const query
     ...
   }

A loop that also moves the geometry refits it between parallel sections, not during one.

Other things to know
--------------------

- **The renderer is parallel by itself.** ``Renderer::render()`` runs on its own threads
  (``Settings::m_threads``, all hardware threads by default), so calling it from inside a parallel region
  oversubscribes the machine. Two renders of the same shape may run at once.
- **One viewer at a time.** ``Render::view()`` drives a window through SDL, whose state is process-wide.
- **Failed checks abort on other threads.** ``EBGEOMETRY_REQUIRE`` throws ``RequireError`` only inside a
  ``ThrowOnRequire`` scope opened on the same thread. On any other thread, OpenMP workers included, a failed check
  prints its message and aborts.
- **Relative paths.** The ``.ebg`` reader resolves relative file names against the process's current directory,
  which a ``chdir`` on another thread changes.
- **Locales.** The mesh readers and the ``.ebg`` reader read numbers, and ``Render::toCode()`` writes them, with a
  decimal point whatever locale the program has set (the C library's or the global C++ locale), so a program that
  uses a decimal-comma locale needs to do nothing.
