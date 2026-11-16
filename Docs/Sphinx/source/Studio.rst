.. _Chap:Studio:

ebgstudio
=========

``ebgstudio`` is a program for working with models written in :ref:`Chap:Language`, with no code to write: it opens a
model in a window that reloads it whenever its files change, checks it, prints it, lists its params and renders it. In
the window, the surface groups of a model's domains are made by clicking on its surfaces, and written back into its
files.

.. code-block:: text

   ebgstudio Models/cooling_block/setup.ebg                       open the model in the window
   ebgstudio check Models/cooling_block/setup.ebg --all-domains   check it, and that its domains are complete

``Studio/`` holds the program (``main.cpp``, ``Writer.hpp``), its build files, and ``Studio/Models``, the models it
comes with: starting points to copy, each checked in every CI run. :ref:`Chap:StudioCoolingBlock` builds a model step
by step, and is the place to start; :ref:`Chap:StudioCommands` describes the commands and :ref:`Chap:StudioViewer` the
window.

Getting it
----------

``ebgstudio`` builds on its own in ``Studio/``, as an example does, or with the library. Each way puts the binary in
one place.

.. list-table::
   :header-rows: 1
   :widths: 30 40 30

   * - Built
     - How
     - The binary
   * - On its own, with make
     - ``cd Studio && make``
     - ``Studio/ebgstudio``
   * - On its own, with CMake
     - ``cd Studio && cmake -S . -B build && cmake --build build``
     - ``Studio/ebgstudio``
   * - With the library
     - ``-DEBGEOMETRY_BUILD_STUDIO=ON`` (on whenever the tests are built)
     - ``<build>/Studio/ebgstudio``
   * - Installed
     - the above, then ``cmake --install <build>``
     - ``<prefix>/bin/ebgstudio``

Built on its own, it takes the examples' switches: ``make GPU=CUDA`` (or ``HIP``) renders on a GPU, ``VIEWER=OFF``
builds it without the window (``view`` then says so), and ``EBGEOMETRY_HOME`` points at the library elsewhere; with
CMake, ``-DEBGEOMETRY_STUDIO_GPU=CUDA`` and ``-DEBGEOMETRY_VIEWER=OFF``. The window needs SDL2, which it uses when it
finds it (see :ref:`Chap:ImplemRender`); the other commands run anywhere.

It builds models in single precision (``float``): a GPU computes in ``double`` many times slower, which would make a
GPU build slower than the host. ``PRECISION=double`` (CMake: ``-DEBGEOMETRY_STUDIO_PRECISION=double``) builds it in
double precision, for a model whose features are too small for ``float`` at its coordinates (about seven significant
digits).

Models in a program
-------------------

``ebgstudio`` reads models the way any program can: through ``Language::read()`` (see :ref:`Chap:ImplemLanguage`).
:ref:`Chap:ExampleLanguage` reads a model's setup file, builds its domain and fills a table of its materials, on the host and
on a GPU; ``ebgstudio`` is how such a model is written and checked before a program reads it.
