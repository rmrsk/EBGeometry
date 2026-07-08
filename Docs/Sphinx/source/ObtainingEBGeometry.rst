.. _Chap:ObtainingEBGeometry:
.. _Sec:Cloning:

Obtaining EBGeometry
======================

Clone the repository from `GitHub <https://github.com/rmrsk/EBGeometry>`_:

.. code-block:: bash

   git clone https://github.com/rmrsk/EBGeometry.git

The core library is header-only and completely self-contained once cloned.
However, the ready-to-run examples in :file:`Examples/` read surface meshes from the
`common-3d-test-models <https://github.com/alecjacobson/common-3d-test-models>`_ collection,
which is bundled as a git submodule (``common-3d-test-models/``) at the repository root.
If you intend to run the bundled examples, clone with the submodule in one step instead:

.. code-block:: bash

   git clone --recurse-submodules https://github.com/rmrsk/EBGeometry.git

If you already cloned without ``--recurse-submodules``, fetch the submodule afterwards:

.. code-block:: bash

   git submodule update --init --recursive

The meshes are then available as ``.obj`` files under ``common-3d-test-models/data/``.
Some mesh-based examples take a mesh path on the command line, resolved relative to the run
directory (each example is run from its own source folder), for example:

.. code-block:: bash

   ./a.out ../../common-3d-test-models/data/armadillo.obj

Running an example with no argument falls back to a default mesh from the submodule, so
the submodule must be checked out for the examples to run.

.. note::

   The submodule is only needed for the bundled examples.  The core library and your
   own applications do not depend on it.
