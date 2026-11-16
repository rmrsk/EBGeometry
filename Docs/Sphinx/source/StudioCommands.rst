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

``MODEL`` is a ``.ebg`` file, read with the files it includes, each once; nothing is read that is not named. The
geometry usually goes in one file and what is computed on it (materials, domains, couplings, the view) in a setup
that includes it, so that one geometry can have several setups (see :ref:`Chap:LanguageModels`): open the geometry
alone to work on its shapes and labels, and a setup to work on its domains.

Every command takes ``-D NAME=VALUE``, a value for a param, written as in a file (``-D "inflow=0.8"``,
``-D "shift=(0, 2, 0)"``).

``ebgstudio MODEL``, view
-------------------------

The window on the model (see :ref:`Chap:StudioViewer`).

check
-----

Reads the model and reports every error and warning, with its file, line and column:

.. code-block:: text

   $ ebgstudio check mine/model.ebg
   mine/model.ebg:15:33: warning: material 'steel' is not declared: it has no properties (write 'material steel { ... }')
   mine/model.ebg:16:35: warning: material 'water' is not declared: it has no properties (write 'material water { ... }')

``--domain NAME`` checks that a domain is complete: every subdomain has properties, every piece of surface bounding
it is in a surface group, and the surface groups its couplings join hold the same surfaces. What is missing is
reported by the shape call it comes from (see :ref:`Chap:LanguageDomains`). ``--all-domains`` checks every domain the
model has:

.. code-block:: text

   $ ebgstudio check Models/cooling_block/setup.ebg --all-domains
   domain 'flow': complete
   domain 'heat': complete

``--inventory`` lists every name the model offers: its regions and materials with their properties' keys, its
labels, each shape's parts and their labels, and its domains:

.. code-block:: text

   $ ebgstudio check Models/cooling_block/flow.ebg --inventory
   regions:
     0 outside
     1 'solid', material 'steel'
     2 'water', material 'water'
   materials:
     steel: conductivity density heat_capacity
     water: conductivity density viscosity heat_capacity
   surface labels:
     "end"
     "base"
     ...
   shapes and their parts:
     box at Models/cooling_block/model.ebg:13:9, parts: xmin="end" xmax="end" ymin="base" ymax="top" zmin="side" zmax="side"
     cylinder at Models/cooling_block/model.ebg:17:11, parts: side="bore" cap1="inlet" cap2="outlet"
   domains:
     flow:
       subdomain fluid: 'water'
       group inlet
       group outlet
       group wall

``check`` exits with 1 on an error or an incomplete domain, and 0 otherwise, so that a build or a CI job can check a
model; warnings never fail it.

print
-----

The model in canonical form: one statement per line, comments kept. The model's first file, with its includes kept;
``--flatten`` writes the whole model, includes resolved, as one text; ``-o FILE`` writes it to a file.

params
------

The params, with their values (after ``-D``) and where they are written; ``--json`` for a program to read:

.. code-block:: text

   $ ebgstudio params Models/cooling_block/flow.ebg -D inflow=0.8
   length = 4  (number, Models/cooling_block/model.ebg:7)
   width = 2  (number, Models/cooling_block/model.ebg:8)
   height = 1  (number, Models/cooling_block/model.ebg:9)
   r_channel = 0.25  (number, Models/cooling_block/model.ebg:10)
   inflow = 0.8  (number, Models/cooling_block/flow.ebg:7, overridden)

render
------

One frame, as the window opens on the model, written as a PPM image: the camera and colors of the model's ``view``
block, regions colored by material. ``--size W H`` sets its size in pixels.
