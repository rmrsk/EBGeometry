.. _Chap:ExampleShell:

Shell extraction
================

A thin wall around a mesh's surface; here Lucy, rotated upright, with a quarter cut away. The source is
at :file:`Examples/Shell/main.cpp`.

.. figure:: /_static/example_Shell.jpg
   :width: 60%
   :align: center

   Lucy hollowed into a shell, a quarter cut away.

.. important::

   The default mesh comes from the ``common-3d-test-models`` git submodule. Clone with
   ``--recurse-submodules``, or run ``git submodule update --init`` in the repository (see
   :ref:`Sec:Cloning`); otherwise pass a mesh of your own as the first argument.

Placing the mesh
----------------

Lucy stands along :math:`z` in the file. ``Rotate`` turns her about :math:`x` so that she stands along
:math:`y` and about :math:`y` so that she faces the camera; ``Scale`` and ``Translate`` then give her height
2 and stand her on :math:`y = 0`.

Hollowing it
------------

``Annular(e, d)`` keeps the points within :math:`d` of the surface (see
:ref:`Chap:ExampleValueOperations`): a wall :math:`2d` thick centred on it, the kind of thin wall an
embedded-boundary grid often needs from a closed surface mesh. A ``Difference`` with a box removes the
quarter towards the camera:

.. code-block:: c++

   const Tape<T> shell(pool, Difference(Annular(statue, T(0.5) * thickness), quarter));

The program prints the solid's and the shell's values at two points inside the statue: negative in the
solid, positive in the shell, whose inside is now hollow.

Looking at it
-------------

The example ends with ``Render::view(shell, argc, argv)``, a window on the shell with its quarter cut
away.

The window needs SDL2 and is built with ``make VIEWER=SDL2`` or ``cmake -DEBGEOMETRY_VIEWER=SDL2`` (see
:ref:`Sec:ConfigViewer`); otherwise, and with ``--no-view`` or without a display, ``view()`` prints why
and returns. Drag to orbit, scroll to zoom, X cuts the object open and P colours each primitive; see
:ref:`Chap:ImplemRender`.

Building and running
--------------------

See :ref:`Chap:Building` for how to compile it with CMake, GNU Make, or a direct compiler invocation.
Built for a GPU (see :ref:`Sec:ExamplesGPU`), the window renders on the device.

.. code-block:: bash

   cd Examples/Shell
   ./Shell.ex
