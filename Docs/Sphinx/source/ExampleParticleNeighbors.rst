.. _Chap:ExampleParticleNeighbors:

ParticleNeighbors
=================

Particles that move, and every particle's 8 nearest neighbors after each step, with
`PointCloudHashGrid <doxygen/html/classEBGeometry_1_1PointCloudHashGrid.html>`__ (see
:ref:`Chap:ImplemPointCloud`). The particles live where the computation runs (device memory in a CUDA or HIP
build) in the program's own layout, one array per coordinate. The grid is built over them once, with room to
grow, and rebuilt in place after every step, without the positions going to the host and back.

The source is at :file:`Examples/ParticleNeighbors/main.cpp`. See :ref:`Chap:Building` for how to compile it
with CMake, GNU Make, or a direct compiler invocation.

.. code-block:: bash

   cd Examples/ParticleNeighbors
   ./ParticleNeighbors.ex

The code that runs on either side
---------------------------------

Every piece of work is a small, trivially copyable struct with an ``EBGEOMETRY_HOST_DEVICE`` call operator, run
by ``GPU::parallelFor()`` or ``GPU::transform()``: ``Start`` gives the particles their starting positions and
velocities, ``Move`` advances them one time step, and ``EighthNeighbor`` is the query. ``FromParticles`` tells the
grid where particle ``i`` is, reading it from the program's arrays; a struct like it works for any layout (a
struct per particle, another precision, an index map):

.. code-block:: c++

   struct FromParticles
   {
     Particles p;   // the program's arrays

     EBGEOMETRY_HOST_DEVICE Vec3 operator()(const uint32_t i) const noexcept { return Vec3(p.x[i], p.y[i], p.z[i]); }
   };

Building the grid
-----------------

``build()`` runs where the grid's pool is: here ``GPU::memoryResource()``, so on the device. Its last argument
reserves room for rebuilds of up to that many particles, so that a rebuild reserves nothing and the pool can be
frozen, which is what makes a view of the grid safe to pass to kernels:

.. code-block:: c++

   EBGeometry::Pool gridPool(EBGeometry::GPU::memoryResource());

   Grid grid = Grid::build(gridPool, FromParticles{particles}, numParticles, T(1), numParticles);

   gridPool.freeze();

Stepping
--------

After each step, ``rebuild()`` sorts the particles into cells again, in place, from the same source. A view
taken before a rebuild describes the old grid, so each step takes a new one, which copies nothing:

.. code-block:: c++

   EBGeometry::GPU::parallelFor(numParticles, Move{particles, timeStep});

   grid.rebuild(FromParticles{particles}, numParticles);

   EBGeometry::GPU::transform(EighthNeighbor{grid.rebasedView(gridPool)}, indices, eighth);

``nearestNeighbors(i, 8, hits)`` reports each neighbor by its index into the particles' arrays, with its squared
distance, so the program reads whatever it keeps per particle from its own arrays. In a plain C++ build the same
code runs on the host. See :ref:`Sec:ExamplesGPU`.
