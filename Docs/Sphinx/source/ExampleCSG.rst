.. _Chap:ExampleCSG:

CSG
===

Builds objects out of analytic shapes with constructive solid geometry (see
:ref:`Chap:ConstructiveSolidGeometry`), compiles them into a ``Tape`` (see :ref:`Sec:Tape`), and
evaluates them on the host and through the same query on a GPU. The source is at
:file:`Examples/CSG/main.cpp`. See :ref:`Chap:Building` for how to compile it with CMake, GNU Make, or a
direct compiler invocation.

.. code-block:: bash

   cd Examples/CSG
   ./CSG.ex

Describing the objects
----------------------

Every analytic shape (see :ref:`Sec:AnalyticShapes`) converts to an
`Expr <doxygen/html/classEBGeometry_1_1Expr.html>`__, and the builder functions in
`EBGeometry_Expr.hpp <doxygen/html/EBGeometry__Expr_8hpp.html>`__ combine and transform expressions
without evaluating anything. The program describes three objects:

* A **part**: a cube with its corners rounded off by a sphere, drilled through along all three axes.
  ``Intersection`` keeps what is inside both operands and ``Difference`` removes the second from the
  first. Like ``Union``, both take a ``std::vector`` of operands as well as a pair:

  .. code-block:: c++

     const Expr part = Difference(Intersection(cube, ball), Union(drills));

* A **blob**: two spheres joined by ``SmoothUnion``, which blends them over a length (here 0.3)
  instead of meeting at a sharp crease. ``SmoothIntersection`` and ``SmoothDifference`` do the same
  for the other two operations.

* A **ring**: eight copies of the part, each passed through ``Scale``, ``Rotate`` (about the y axis)
  and ``Translate`` onto a circle. A transform applies to a whole expression, and a copy shares the
  part's description rather than repeating it.

Compiling and using them on the host
------------------------------------

A `Tape <doxygen/html/classEBGeometry_1_1Tape.html>`__ compiles an expression into a
`Pool <doxygen/html/classEBGeometry_1_1Pool.html>`__: one array per primitive type, and a list of
clauses that refer to the primitives by type and index. The tape is a descriptor into the pool, so the
pool must outlive it and every copy of it (see :ref:`Chap:MemoryModel`).

.. code-block:: c++

   Pool pool(hostMemoryResource());

   const Tape<T> ringTape(pool, ring);
   const T       value = ringTape.signedDistance(point);   // negative inside, positive outside

For each object the program prints what the compiler made of it: the number of clauses and
sub-programs, the number of unions it evaluates through a BVH over the operands' bounding boxes (the
ring has one, since it has eight operands; see :ref:`Sec:BVHUnions`), the bounding box, and the
distance quality. An intersection or a difference of exact distances is only a lower bound on the
distance, so all three objects report ``bound``. It then prints each object's value at a few points.

The same query on a GPU
-----------------------

The query is a trivially copyable functor that holds the tape by value. To evaluate it in a kernel,
the program freezes the pool, mirrors it in one piece into ``GPU::memoryResource()`` (device memory in
a CUDA or HIP build), and takes a ``rebasedView()`` of the tape onto the copy.
`GPU::transform() <doxygen/html/EBGeometry__GPUTransform_8hpp.html>`__ then runs the query once per
point and returns the values to the host:

.. code-block:: c++

   pool.freeze();

   const Pool devicePool = Pool::mirror(pool, GPU::memoryResource());

   std::vector<T> values(points.size());

   GPU::transform(Distance{ringTape.rebasedView(devicePool)}, points, values);

In a plain C++ build the same lines run on the host: ``memoryResource()`` is host memory standing in
for a device and ``transform()`` is a loop. The program evaluates the ring along a line through the
first copy, crossing its material and the hole drilled along y. See :ref:`Sec:ExamplesGPU` for how to
build it for a GPU.

Looking at it
-------------

The example ends by opening a window on the drilled part, compiled into a tape:

.. code-block:: c++

   EBGeometry::Render::view(partTape, argc, argv);

The window needs SDL2 and is built with ``make VIEWER=SDL2`` or ``cmake -DEBGEOMETRY_VIEWER=SDL2`` (see
:ref:`Sec:ConfigViewer`); otherwise, and with ``--no-view`` or without a display, ``view()`` prints why
and returns. Drag to orbit, scroll to zoom, X cuts the shape open and P colours its primitives; see
:ref:`Chap:ImplemRender`.
