.. _Chap:ThirdParty:

Third party
============

.. important::

   These examples are illustrative only, meant to show how EBGeometry can be integrated into a
   third-party application code -- they are not actively maintained or built as part of
   EBGeometry's continuous integration. Unlike everything under :ref:`Chap:Examples`, which is
   compiled and run on every pull request, changes to a third-party platform (or to EBGeometry
   itself) may or may not break these examples without anyone noticing. Treat them as a
   starting point for your own integration, not a guarantee that they work out of the box.

AMReX
------

The :file:`ThirdParty/AMReX/<something>` folders couple EBGeometry to
`AMReX <https://amrex-codes.github.io/amrex/>`_'s embedded-boundary grid generation: an
EBGeometry signed distance function is handed to AMReX, which uses it to cut cells at the
implicit surface. AMReX must be installed separately, with the ``AMREX_HOME`` environment
variable pointing to it. Available examples:

* :file:`ThirdParty/AMReX/Shapes`
* :file:`ThirdParty/AMReX/MeshSDF`
* :file:`ThirdParty/AMReX/PackedSpheres`
* :file:`ThirdParty/AMReX/RandomCity`
* :file:`ThirdParty/AMReX/PaintEB`

See the README in each folder for exact build and run instructions.

Chombo
-------

The :file:`ThirdParty/Chombo/<something>` folders couple EBGeometry to
`Chombo <https://commons.lbl.gov/display/chombo/>`_'s embedded-boundary grid generation, in the
same spirit as the AMReX examples above. Chombo must be installed separately, with the
``CHOMBO_HOME`` environment variable pointing to it. Available examples:

* :file:`ThirdParty/Chombo/Shapes`
* :file:`ThirdParty/Chombo/MeshSDF`
* :file:`ThirdParty/Chombo/PackedSpheres`
* :file:`ThirdParty/Chombo/RandomCity`

See the README in each folder for exact build and run instructions.
