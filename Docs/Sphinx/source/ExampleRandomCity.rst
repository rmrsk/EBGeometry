.. _Chap:ExampleRandomCity:

RandomCity
===========

Creates a scene of randomly placed boxes ("buildings") and, like ``PackedSpheres``,
compares a standard union against a BVH-accelerated union for closest-object queries.

The source for this example is at :file:`Examples/RandomCity/main.cpp`. See :ref:`Chap:Building`
for how to compile it with CMake, GNU Make, or a direct compiler invocation.

.. code-block:: bash

   cd Examples/RandomCity
   ./RandomCity.ex
