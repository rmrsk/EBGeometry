.. _Chap:ExampleUnion:

Unions
======

This example is given in :file:`Examples/EBGeometry_Union/main.cpp` and shows the following steps:

#. The creation of scene composed of an array of spheres.
#. Instantiation of a standard union for the signed distance (see :ref:`Chap:Union`).
#. Instantiation of a BVH-enabled union for the signed distance (see :ref:`Chap:Union`).

We focus on the following parts of the code:

.. literalinclude:: ../../Examples/EBGeometry_Union/main.cpp
   :language: c++
   :lines: 24-25, 29-67,69-70

Creating the spheres
--------------------

In the first block of code we are defining one million spheres that lie on a three-dimensional lattice, where each sphere has a radius of one:

.. literalinclude:: ../../Examples/EBGeometry_Union/main.cpp
   :language: c++
   :lines: 29-48

Creating standard union
-----------------------

In the second block of code we are simply creating a standard signed distance function union:

.. literalinclude:: ../../Examples/EBGeometry_Union/main.cpp
   :language: c++
   :lines: 52

For implementation details regarding the standard union, see :ref:`Chap:Union`.

Creating BVH-enabled union
--------------------------

In the third block of code we create a BVH-enabled union.
To do so, we must first provide a function which can create bounding volumes around each object:

.. literalinclude:: ../../Examples/EBGeometry_Union/main.cpp
   :language: c++
   :lines: 56-66

Here, we use axis-aligned boxes but we could also have used other types of bounding volumes.

Typical output
--------------

The above example shows two methods of creating unions.
When running the example the typical output is something like:

.. code-block:: text

    Partitioning spheres
    Computing distance with slow union
    Computing distance with fast union
    Distance and time using standard union = -1, which took 26.7353 ms
    Distance and time using optimize union = -1, which took 0.003527 ms
    Speedup = 7580.19		

where we note that the optimized union was about 7500 times faster than the "standard" union.
