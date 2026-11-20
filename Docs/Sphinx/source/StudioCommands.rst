.. _Chap:StudioCommands:

Commands
========

.. code-block:: text

   ebgstudio        MODEL                                       the window (the same as view)
   ebgstudio check  MODEL [--domain NAME]... [--all-domains] [--inventory] [--components H]
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
reported by the shape call it comes from (see :ref:`Chap:LanguageDomains`). Each domain's line also says what its
own object keeps: its regions and the regions whose bounding boxes meet theirs, what a solver of the domain alone
compiles (see :ref:`Chap:ImplemRegions`). ``--all-domains`` checks every domain the model has:

.. code-block:: text

   $ ebgstudio check ../Examples/LanguageDomains/cable/setup.ebg --all-domains
   domain 'electrostatics': complete; its own object keeps 4 of 4 regions (air, jacket, core, sensor)
   domain 'cfd': complete; its own object keeps 4 of 4 regions (air, jacket, core, sensor)
   domain 'conduction': complete; its own object keeps 3 of 4 regions (air, jacket, core)

``--components H`` also finds, for each domain checked, the connected components of its subdomains and of its
excluded regions at resolution ``H`` (see :ref:`Chap:Regions`; ``H`` is best the solver's cell size). It warns of a
subdomain in more than one component and of an enclosed cavity of the outside touching the domain (see
:ref:`Chap:LanguageDomains`), and lists every component with a point in it, its volume, and whether it is on the
object's outer boundary or enclosed, with the regions around it; an excluded component that is enclosed is a
floating body (a conductor touching nothing, say), listed but not warned of:

.. code-block:: text

   $ ebgstudio check ../Examples/LanguageDomains/cable/setup.ebg --domain cfd --components 0.05
   ../Examples/LanguageDomains/cable/setup.ebg:25:3: warning: subdomain 'fluid' of domain 'cfd' is in 3 connected components at resolution 0.05: (2.1, 0.7, 1.3), volume 179, on the object's outer boundary; (0.2, 0.2, 2.8), volume 0.395, on the object's outer boundary; (0.2, 0.2, -2.8), volume 0.395, on the object's outer boundary
   domain 'cfd': complete; its own object keeps 4 of 4 regions (air, jacket, core, sensor)
   domain 'cfd': connected components at resolution 0.05
     subdomain 'fluid': 3
       at (2.1, 0.7, 1.3), volume 179, on the object's outer boundary
       at (0.2, 0.2, 2.8), volume 0.395, on the object's outer boundary
       at (0.2, 0.2, -2.8), volume 0.395, on the object's outer boundary
     excluded regions: 2
       at (-3.07, -2.07, -3.07), volume 32.9, on the object's outer boundary
       at (3, 0, 0), volume 0.114, enclosed by 'air'

The cable's air is in three pieces: the air around the cable, and the air in each end of the jacket, between the
core's end and the box's face, which meets the rest of the air only through that face. The sensor is a body
floating in the air. A model whose value is not a distance is not searched.

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
