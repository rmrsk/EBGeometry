.. _Chap:StudioCommands:

Commands
========

.. code-block:: text

   ebgstudio        MODEL                                       the window (the same as view)
   ebgstudio check  MODEL [--domain NAME]... [--all-domains] [--inventory]
   ebgstudio print  MODEL [-o FILE] [--flatten]
   ebgstudio params MODEL [--json]
   ebgstudio render MODEL -o FILE [--size W H]
   ebgstudio view   MODEL

``MODEL`` is a ``.ebg`` file, read with the files it includes (a setup that includes its ``model.ebg``, say). Every
command takes
``-D NAME=VALUE``, a value for a param, written as in a file (``-D "inflow=0.8"``, ``-D "shift=(0, 2, 0)"``).

``ebgstudio MODEL``, view
   The window on the model (see :ref:`Chap:StudioViewer`).

check
   Reads the model and reports every error and warning, with its file, line and column. ``--domain NAME`` checks that
   a domain is complete (every subdomain has properties, every piece of surface bounding it is in a surface group, and
   the surface groups its couplings join hold the same surfaces), reporting what is missing by the shape call it comes
   from; ``--all-domains`` checks every domain the model has. ``--inventory`` lists every name the model offers: its
   regions and materials, its labels, each shape's parts and their labels, and its domains. It exits with 1 on an error
   or an incomplete domain, so that a build or a CI job can check a model; warnings never fail it.

print
   The model in canonical form: one statement per line, comments kept. The model's first file, with its includes kept;
   ``--flatten`` writes the whole model, includes resolved, as one text.

params
   The params, with their values (after ``-D``) and where they are written; ``--json`` for a program to read.

render
   One frame, as the window opens on the model, written as a PPM image: the camera and colors of the model's ``view``
   block, regions colored by material.

A model is the file named and the files it includes, each read once; nothing is read that is not named. The geometry
usually goes in one file and what is computed on it (materials, domains, couplings, the view) in a setup that includes
it, so that the same geometry can have several setups (see :ref:`Chap:Language`). Open the geometry alone to work on
its shapes and labels, and a setup to work on its domains.
