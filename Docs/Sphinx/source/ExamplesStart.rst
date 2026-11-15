.. _Chap:ExamplesStart:

Start here
==========

Three examples that show the library's three kinds of geometry and the calls every program makes: analytic
shapes, a composition compiled into a tape, and a mesh read from a file. Each runs on the host and, built for
a GPU, on the device.

* :ref:`Chap:ExampleShapes`: the analytic shapes, their signed distance, bounding box and distance quality.
* :ref:`Chap:ExampleBasicCSG`: the classic CSG solid, with unions, intersections and differences, compiled
  into a ``Tape``.
* :ref:`Chap:ExampleMeshSDF`: reading a surface mesh into a signed distance function, and the three
  representations to choose from.

.. toctree::
   :hidden:

   ExampleShapes.rst
   ExampleBasicCSG.rst
   ExampleMeshSDF.rst
