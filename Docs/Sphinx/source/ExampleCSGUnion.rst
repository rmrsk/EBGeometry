.. _Chap:ExampleCSGUnion:

CSGUnion
========

Composes objects of *different* types -- a mesh signed distance function and analytic spheres --
into one object and evaluates it through a ``Tape`` (see :ref:`Sec:Tape`). The mesh is read once
into a ``MeshSDF`` and placed six times with ``Translate``, which shares the mesh rather than
copying it, and a sphere is placed above each copy. ``Union`` combines the twelve objects into an
expression, and the ``Tape`` compiles it into the same ``Pool`` as the mesh. With that many
operands the tape evaluates the union with a BVH over their bounding boxes, so a query evaluates
only the objects near it.

The program prints the size of the compiled tape and checks it against the smallest distance over
every object, evaluated directly, at random points; the two must agree up to rounding.

The source for this example is at :file:`Examples/CSGUnion/main.cpp`. See :ref:`Chap:Building`
for how to compile it with CMake, GNU Make, or a direct compiler invocation. Like
:ref:`Chap:ExampleNestedBVH`, it defaults to the ``dodecahedron.stl`` fixture shipped in the
repository, so it needs no submodule.

.. code-block:: bash

   cd Examples/CSGUnion
   ./CSGUnion.ex                       # defaults to ../../Tests/data/dodecahedron.stl
   ./CSGUnion.ex path/to/mesh.stl      # place copies of your own mesh
