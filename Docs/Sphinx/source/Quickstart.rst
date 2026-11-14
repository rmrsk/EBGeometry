.. _Chap:Quickstart:

.. _Chap:ObtainingEBGeometry:

Quickstart
==========

.. _Sec:Cloning:

Getting the code
----------------

Clone the repository from `GitHub <https://github.com/rmrsk/EBGeometry>`_, with its submodule:

.. code-block:: bash

   git clone --recurse-submodules https://github.com/rmrsk/EBGeometry.git

The library itself is header-only and needs nothing else. The submodule,
`common-3d-test-models <https://github.com/alecjacobson/common-3d-test-models>`_, holds the meshes some
examples read by default (in ``common-3d-test-models/data/``); a clone without it gets it later with
``git submodule update --init``.

A first program
---------------

There is nothing to build or link: include :file:`EBGeometry.hpp`, from the repository root, and compile
with C++17.

.. code-block:: cpp

   #include <iostream>

   #include <EBGeometry.hpp>

   using namespace EBGeometry;
   using T = double;

   int
   main()
   {
     // A box with a ball cut out of one corner.
     const BoxSDF<T>    box(Vec3T<T>(-1, -1, -1), Vec3T<T>(1, 1, 1));
     const SphereSDF<T> ball(Vec3T<T>(1, 1, 1), T(0.8));

     // Compile the composition into a tape, whose memory lives in a pool.
     Pool          pool(hostMemoryResource());
     const Tape<T> object(pool, Difference(box, ball));

     // Negative inside, positive outside.
     for (const Vec3T<T>& p : {Vec3T<T>(0, 0, 0), Vec3T<T>(0.9, 0.9, 0.9), Vec3T<T>(2, 0, 0)}) {
       std::cout << p << ": " << object.signedDistance(p) << '\n';
     }
   }

.. code-block:: bash

   g++ -std=c++17 -O3 -I/path/to/EBGeometry main.cpp -o first.ex && ./first.ex

The origin is inside the box, about 0.93 from the ball's surface; the point near the corner is in the hole
the ball cut; the last is outside, 1 from the box's face. :ref:`Chap:Building` describes adding EBGeometry to
a CMake or make project.

Where to go next
----------------

* **The examples** (:ref:`Chap:Examples`) build on each other, from the analytic shapes to meshes, many
  objects, rays and GPUs, each a short program with a page. The first runs without the submodule:

  .. code-block:: bash

     cd Examples/Shapes
     make && ./Shapes.ex

* **The concepts** (:ref:`Chap:GeometryRepresentations`) explain what the library computes and how.
* **The API guide** (:ref:`Chap:Implementation`) lists the classes and calls by task.
