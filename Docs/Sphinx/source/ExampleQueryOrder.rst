.. _Chap:ExampleQueryOrder:

QueryOrder
==========

Running queries at points that arrive in no spatial order in an order that keeps nearby points together (see
:ref:`Sec:QueryOrder`). `GPU::mortonOrder() <doxygen/html/namespaceEBGeometry_1_1GPU.html>`__ gives the order, and
``GPU::parallelFor`` and ``GPU::transform`` run in it; every result still lands at its input's index, so only the
time changes. The example times a million particles' distances to a mesh in the order they arrived and in Morton
order. A plain C++ build runs the launches as host loops on one core, a GPU build on the device. It opens no window.
The source is at :file:`Examples/QueryOrder/main.cpp`, in the following sections.

The user's data and the query
-----------------------------

The particles are stored as a simulation would hold them, each with a position and a velocity, in the order they
arrived. The order reads their positions through a *source*, a trivially copyable functor that returns point ``i``,
so they are not copied into an array of points:

.. code-block:: c++

   struct Positions
   {
     const Particle* particles;

     EBGEOMETRY_HOST_DEVICE Vec3 operator()(std::uint32_t i) const noexcept { return particles[i].position; }
   };

The query is a kernel of the program's own, run through ``GPU::parallelFor``: each particle's signed distance to the
mesh (a `TriMeshSDF <doxygen/html/classEBGeometry_1_1TriMeshSDF.html>`__, the armadillo by default), written at the
particle's index.

Where the launches read
-----------------------

The particles, room for the distances and room for the order go into one pool, mirrored where the launches run (a
GPU's memory in a GPU build, host memory otherwise) next to the mirrored mesh, so that a launch allocates and copies
nothing and its time is the work's.

In arrival order, then in Morton order
--------------------------------------

.. code-block:: c++

   GPU::parallelFor(numParticles, query);                                        // as they arrived
   GPU::mortonOrder(deviceIO, Positions{devParticles}, numParticles, order);     // once
   GPU::parallelFor(numParticles, query, order.bind(deviceBase));               // in Morton order

The order is computed where its pool is: on the device for the mirror of a GPU build, from the particles there. It
depends on the positions only, so a program computes it once and reuses it while the particles stay put. The example
prints the three times, and the first few distances, which are the same whichever order ran last. How much the order
gains depends on the query, the device and how scattered the points are, so measure your own.

Building and running
--------------------

See :ref:`Chap:Building` for the three ways to compile it, and :ref:`Sec:ExamplesGPU`:

.. code-block:: bash

   cd Examples/QueryOrder
   make                              # host loops
   make GPU=CUDA PRECISION=float     # on the GPU
   ./QueryOrder.ex

With no argument it reads ``armadillo.obj`` from the ``common-3d-test-models`` submodule; pass another mesh on the
command line.
