.. _Chap:ExampleMeshSDF:

MeshSDF
========

Reads a surface mesh into a signed distance function in each of the three representations described
in :ref:`Chap:MeshSDFClasses`, times them, and evaluates one on a GPU. The source is at
:file:`Examples/MeshSDF/main.cpp`.

.. figure:: /_static/example_MeshSDF.jpg
   :width: 60%
   :align: center

   The armadillo as the viewer first shows it.

The code that runs on either side
---------------------------------

The query handed to ``GPU::transform`` holds a
`TriMeshSDF <doxygen/html/classEBGeometry_1_1TriMeshSDF.html>`__ by value. A mesh SDF owns no memory:
it is a descriptor of a mesh and a BVH that live in a
`Pool <doxygen/html/classEBGeometry_1_1Pool.html>`__, so copying it into a kernel is cheap, once it
describes a pool the device can read.

Read the mesh
-------------

The mesh comes from the command line or, by default, ``armadillo.obj`` from the
``common-3d-test-models`` submodule (see :ref:`Sec:Cloning`). Any watertight, orientable mesh in STL,
PLY, VTK or OBJ works. Each of the three readers in
`EBGeometry_Parser.hpp <doxygen/html/EBGeometry__Parser_8hpp.html>`__ parses the file into one pool:

* ``readIntoFlatMeshSDF``: a `FlatMeshSDF <doxygen/html/classEBGeometry_1_1FlatMeshSDF.html>`__,
  the half-edge mesh with no acceleration. Every query visits every face.
* ``readIntoMeshSDF``: a `MeshSDF <doxygen/html/classEBGeometry_1_1MeshSDF.html>`__, the
  half-edge mesh under a BVH, for any planar convex polygon faces.
* ``readIntoTriMeshSDF``: a ``TriMeshSDF``, triangles grouped for SIMD under a BVH. Usually the
  fastest, and the one to pick unless you need the polygon faces themselves.

An application would pick one. The pool must outlive every SDF built into it, and every copy of one.
The example reads with ``Parser::OnDefect::Warn``, so a mesh with small defects (two faces oriented
against each other, or an edge shared by more than two faces, as in the xyzrgb dragon) loads with a
warning instead of being refused; the sign of the distance near such a defect is unreliable.

Use it on the host
------------------

The program evaluates all three at the same random points around the mesh and prints the time per
query. The values are the same; only the number of faces visited differs: the flat scan visits every
face, a BVH only the few near the point, so on a large mesh such as the armadillo the BVH-backed
representations are far faster.
``getClosestFace(point)`` (`ClosestFace <doxygen/html/structEBGeometry_1_1ClosestFace.html>`__) also
returns which face is nearest, as its index in the file, which is how to look up per-face data of
your own.

.. tip::

   SDF query cost depends on both the geometry and the query point. A tessellated sphere has a
   "blind spot" at its center where even a BVH must visit most faces.

The same query on a GPU
-----------------------

A kernel cannot read host memory, so the pool goes to the device first:

.. code-block:: c++

   pool.freeze();                                                       // nothing more is added
   const Pool devicePool = Pool::mirror(pool, GPU::memoryResource());   // one copy of everything
   const Distance query{triSDF.rebasedView(devicePool)};                // resolves against the copy

   std::vector<T> onDevice(points.size());

   GPU::transform(query, points, onDevice);

``GPU::memoryResource()`` is device memory in a CUDA or HIP build, and host memory standing in for
it otherwise, so these lines run in both builds. See :ref:`Chap:MemoryModel` and
:ref:`Chap:RunningOnGPU`.

Looking at it
-------------

The example ends by opening a window on the mesh:

.. code-block:: c++

   EBGeometry::Render::view(triSDF, pool, argc, argv);

The window needs SDL2 and is built with ``make VIEWER=SDL2`` or ``cmake -DEBGEOMETRY_VIEWER=SDL2`` (see
:ref:`Sec:ConfigViewer`); otherwise, and with ``--no-view`` or without a display, ``view()`` prints why
and returns. Drag to orbit and scroll to zoom; the window lists its other keys (see
:ref:`Sec:ViewerKeys`). A surface seen from inside the solid is drawn red, which is also how a mesh whose
faces are wound the wrong way looks from outside. ``TriMeshSDF`` is ray traced, so the window follows the
mouse; see :ref:`Chap:ImplemRender`. The example passes the pool the mesh lives in, so in a GPU build the
window renders on the GPU (G switches to the CPU and back); build in single precision
(``PRECISION=float``) there, since many GPUs are much slower in double precision.

Building and running
--------------------

See :ref:`Chap:Building` for how to compile it, and :ref:`Sec:ExamplesGPU` for a GPU build.

.. code-block:: bash

   cd Examples/MeshSDF
   ./MeshSDF.ex                                            # defaults to armadillo.obj
   ./MeshSDF.ex ../../common-3d-test-models/data/cow.obj   # or pick another mesh
   ./MeshSDF.ex --no-view                                  # without the window
