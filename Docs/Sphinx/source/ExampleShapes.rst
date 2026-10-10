.. _Chap:ExampleShapes:

Shapes
=======

A very basic example of using EBGeometry for creating analytic signed distance fields
(see :ref:`Sec:AnalyticShapes`): it constructs every built-in shape. Each is a plain value type
whose ``signedDistance()`` can be called on the host or inside a GPU kernel. The program then
checks each shape's distance at a few points against the value worked out by hand from the shape's
definition, and exits with a nonzero status if any disagrees.

The source for this example is at :file:`Examples/Shapes/main.cpp`. See :ref:`Chap:Building`
for how to compile it with CMake, GNU Make, or a direct compiler invocation.

.. code-block:: bash

   cd Examples/Shapes
   ./Shapes.ex
