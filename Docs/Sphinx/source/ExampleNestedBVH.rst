.. _Chap:ExampleNestedBVH:

NestedBVH
=========

Places one surface mesh many times, in the two ways EBGeometry offers: shared between translated
placements in a ``Tape`` alongside analytic shapes, and as separate copies under a ``BVHUnion``,
which nests one BVH (over the placements) above another (over each mesh's triangles). Both are
evaluated on the host and, through the same query, on a GPU.

The source is at :file:`Examples/NestedBVH/main.cpp`. See :ref:`Chap:Building` for how to compile
it with CMake, GNU Make, or a direct compiler invocation.

.. code-block:: bash

   cd Examples/NestedBVH
   ./NestedBVH.ex                       # defaults to ../data/dodecahedron.stl
   ./NestedBVH.ex path/to/mesh.stl      # place your own mesh

.. figure:: /_static/example_NestedBVH.jpg
   :width: 60%
   :align: center

   The tape that places the mesh among the spheres.

Reading the mesh
----------------

The mesh is read into a
`TriMeshSDF <doxygen/html/classEBGeometry_1_1TriMeshSDF.html>`__ with
`Parser::readIntoTriMeshSDF <doxygen/html/namespaceEBGeometry_1_1Parser.html>`__, using the default
BVH branching factor and triangle-group width. A ``TriMeshSDF`` with those defaults is a built-in
primitive of ``Tape<T>``; one with any other must be listed as a user type. Everything the program
builds (meshes, BVHs, the tape, the union) is reserved from one
`Pool <doxygen/html/classEBGeometry_1_1Pool.html>`__, which must outlive all of it.

One mesh, shared: a tape
------------------------

The expression builder (`EBGeometry_Expr.hpp <doxygen/html/EBGeometry__Expr_8hpp.html>`__)
describes the object. ``Translate(mesh, shift)`` places the mesh without copying it, and ``Union``
combines objects of any type, here six placements of the mesh and six spheres:

.. code-block:: c++

   objects.emplace_back(EBGeometry::Translate(mesh, shift));
   objects.emplace_back(EBGeometry::SphereSDF<T>(center + shift + lift, radius));

   const EBGeometry::Tape<T> composed(pool, EBGeometry::Union(objects));

The `Tape <doxygen/html/classEBGeometry_1_1Tape.html>`__ compiles the expression into the pool (see
:ref:`Sec:Tape`). A union of eight or more operands is evaluated through a BVH over the operands'
bounding boxes, so a query only evaluates the objects near it; the program prints the tape's number
of clauses and BVH unions.

Many copies: a nested BVH
-------------------------

The mesh's triangles are read once with ``Parser::readIntoTriangles``, and each of 16 placements on
a 4 x 4 grid gets its own translated copy, built into its own ``TriMeshSDF`` with its own inner BVH.
A `BVHUnion <doxygen/html/classEBGeometry_1_1BVHUnion.html>`__ holds primitives of one type by value
and builds an outer BVH over their bounding boxes (see :ref:`Sec:BVHUnions`):

.. code-block:: c++

   const EBGeometry::BVHUnion<T, Mesh, K> nested(pool, copies, boxes);

One query therefore descends two levels: the outer BVH finds the placements near the point, and each
of those meshes' own BVH finds the nearest triangle (see :ref:`Chap:BVH`). Since each placement is a
separate mesh, genuinely different meshes of the same type can be placed the same way. Sharing one
mesh between placements is what the tape is for.

Using them on the host
----------------------

Both scenes are shapes: ``signedDistance(point)`` is negative inside any placement and positive
outside all of them. The program prints both at points along the first row of placements, which the
two scenes share: through the center of a placement, then halfway to the next one, and so on.

The same query on a GPU
-----------------------

The query is a trivially copyable functor holding the tape and the nested union by value. Both are
descriptors of what they built in the pool, so the last section freezes the pool, mirrors it in one
piece into ``GPU::memoryResource()``, and rebases both onto the copy, every mesh with them.
`GPU::transform <doxygen/html/EBGeometry__GPUTransform_8hpp.html>`__ then runs the query once per
point, both levels of BVH traversal included: in a kernel in a CUDA or HIP build, and in a host loop
otherwise. See :ref:`Sec:ExamplesGPU`.

Looking at it
-------------

The example ends with ``Render::view(composed, argc, argv)``, a window on the tape that places the mesh
among the spheres.

The window needs SDL2 and is built with ``make VIEWER=SDL2`` or ``cmake -DEBGEOMETRY_VIEWER=SDL2`` (see
:ref:`Sec:ConfigViewer`); otherwise, and with ``--no-view`` or without a display, ``view()`` prints why
and returns. Drag to orbit, scroll to zoom, X cuts the object open, P cycles the colouring and H hides the overlay; see
:ref:`Chap:ImplemRender`.
