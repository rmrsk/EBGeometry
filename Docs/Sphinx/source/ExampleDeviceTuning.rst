.. _Chap:ExampleDeviceTuning:

DeviceTuning
============

Choosing the launch settings of
`GPU::transform() <doxygen/html/namespaceEBGeometry_1_1GPU.html>`__ for a query: the block size, and in
a CUDA build the threads the compiler is asked to keep resident on each multiprocessor, which fixes how
many registers a thread may use (see :ref:`Sec:LaunchSettings`). It is the counterpart, on a GPU, of
:ref:`Chap:ExampleHostTuning`. Every pair of settings is a kernel of its own, compiled in advance, so
the example compiles a few candidates, times each on a sample of the real queries and runs the rest
with the fastest: what an application that launches one query many times can do once at startup.

The example is meant for a GPU build; a plain C++ build runs ``GPU::transform()`` as a host loop, has
nothing to tune, and says so. It has no host section and opens no window. The source is at
:file:`Examples/DeviceTuning/main.cpp`, in the following sections.

The query
---------

The signed distance to a mesh (the armadillo by default) with a ball cut out of it, through a
`Tape <doxygen/html/classEBGeometry_1_1Tape.html>`__: the kind of query whose kernel uses many
registers, and whose speed depends on how many it gets.

The candidates
--------------

``GPU::transform<BlockSize, MinResidentThreads>()`` is a different kernel for every pair, so the
candidates are a table of launch functions, one per pair, each a line of its own:

.. code-block:: c++

   template <unsigned int BlockSize, unsigned int MinResidentThreads>
   void launch(const Distance& a_query, const In a_in, const Out a_out)
   {
     EBGeometry::GPU::transform<BlockSize, MinResidentThreads>(a_query, a_in, a_out);
   }

The table holds block sizes 64, 128 and 256, each with no minimum of resident threads (the compiler
picks the register budget), 384 (the default, at most 168 registers a thread) and 768 (at most 80).

On the device
-------------

The tape's pool is mirrored to the device once, and the million points and their results go into a
pool of their own, mirrored the same way, so that a launch over spans of that copy allocates and copies
nothing and its time is the kernel's. Each candidate is timed on the first 100,000 points (the fastest
of three launches, after one to warm up) and printed beside the defaults' time divided by its own, so
that above 1 is faster than the defaults; the other 900,000 then run with the fastest. On one
workstation GPU in single precision, for this query, 768 resident threads was a fifth to a quarter faster
than the defaults, whatever the block size; another device, query or precision may differ, which is
why the example measures rather than prescribes.

Building and running
--------------------

Build it for a GPU (see :ref:`Chap:Building` for the three ways to compile it, and
:ref:`Sec:ExamplesGPU`), in single precision, since many GPUs are much slower in double:

.. code-block:: bash

   cd Examples/DeviceTuning
   make GPU=CUDA PRECISION=float
   ./DeviceTuning.ex
