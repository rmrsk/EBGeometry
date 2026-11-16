.. _Chap:Language:

The .ebg language
=================

A model can be built in C++, with the expression builder, or written as text in the ``.ebg`` language and
read at run time. A ``.ebg`` file describes:

* the **geometry**: shapes, composed with operations, as an object;
* its **regions** and the **materials** they are made of, when the object is made of several;
* **labels**, names for surfaces and parts of surfaces (a box's face, a mesh's face group);
* the **domains** that solvers compute in, each with its **subdomains** (the regions it computes in) and its
  **surface groups** (the surfaces it gives a boundary condition), and the **couplings** between domains;
* the **properties** of the materials, subdomains, surface groups and couplings: numbers, vectors, strings
  and flags, whose meaning is the program's.

The language describes; it computes nothing. A program reads the model with the C++ API (see
:ref:`Chap:ImplemLanguage`), compiles its object into a tape, builds its domains for that tape, and turns
the properties into tables of its own types that run on a GPU beside the geometry. The language checks what
can be checked without knowing what the properties mean: that every name used is defined, that every value
a shape is given is valid, and that a domain leaves no surface without a condition.

If the terms above are new, :ref:`Chap:Regions` explains regions, materials, domains and surface groups,
with a picture of the cable below, and the :ref:`Chap:Glossary` defines each in a line.

A first model
-------------

A cable along *z*: a copper core in a plastic (PTFE) jacket, in a box of air with a flow across it. The
model is two files, as most models are. The geometry, ``cable/model.ebg``:

.. code-block:: text

   ebg 0

   # A cable along z, in a box of air with a cross-flow along x: a copper core inside a PTFE jacket.
   param length = 6
   param r_core = 0.5
   param r_out  = 0.8

   conductor  = cylinder((0, 0, -length / 2), (0, 0, length / 2), radius = r_core)
   insulation = tube((0, 0, -length / 2), (0, 0, length / 2), inner_radius = r_core, outer_radius = r_out)
   air_box    = box((-3, -2, -length / 2), (5, 2, length / 2))
     .label(xmin = "inlet", xmax = "outlet", ymin = "far", ymax = "far", zmin = "ends", zmax = "ends")

   object overlay {  # each region lies on top of the ones before it
     region air    = air_box material air
     region jacket = insulation material PTFE
     region core   = conductor material copper
   }

Line by line:

* ``ebg 0`` is the language's version, on the first line of every file.
* A ``param`` is a value that can be changed from outside the file (``-D length=8`` to ``ebgstudio``, or an
  override passed to the reader), so that one file describes a family of cables.
* ``conductor``, ``insulation`` and ``air_box`` are shapes, calls of the built-in shapes
  (:ref:`Chap:LanguageShapes`). Arguments are given in order, or by name (``radius = r_core``).
* ``.label(...)`` names the box's faces: its ``xmin`` face is the inlet, its ``xmax`` face the outlet. The
  names are what surface groups select later.
* ``object`` is the model's one shape. Here it is an ``overlay`` of three regions, each a shape and a
  material: the jacket on top of the air, the core on top of the jacket.

What is computed on the geometry goes in a second file, ``cable/setup.ebg``, which includes the first. It is
the file a program reads:

.. code-block:: text

   ebg 0

   include("model.ebg")

   param mu_air = 1.8e-5

   material copper { conductivity = 5.8e7 }
   material PTFE { permittivity = 2.1, density = 2200 }
   material air { permittivity = 1.0, density = 1.2, viscosity = mu_air }

   # Electrostatics: the field in the insulation and the air; the conductor is a boundary held at its potential.
   domain electrostatics {
     subdomain insulation = jacket
     subdomain gas = air
     group conductor = interface(core, jacket) { potential = 1000 }
     group dielectric = interface(jacket, air) { surface_charge = 0 }
     group ground = label("inlet"), label("outlet"), label("far") { potential = 0 }
     group ends = remaining() { kind = "zero_flux" }
   }

   # Flow: air past the cable.
   domain cfd {
     subdomain fluid = air
     group inlet = label("inlet") { velocity = (2, 0, 0) }
     group outlet = label("outlet") { pressure = 0 }
     group walls = interface(air, jacket) { kind = "no_slip" }
     group slip = label("far"), label("ends") { kind = "slip" }
   }

   # Coupling: the electric body force acts on the gas; the cable surface carries charge into the flow.
   couple body_force = volume(electrostatics.gas, cfd.fluid)
   couple cable_surface = surface(electrostatics.dielectric, cfd.walls) { transfer = "charge" }

   view { camera = (7, 5, 9), target = (1, 0, 0), color.copper = (0.8, 0.5, 0.2), color.air = (0.55, 0.7, 0.9) }

* A ``material`` gives a material's properties. The names (``conductivity``, ``viscosity``) mean nothing to
  the language; they are what the program reads.
* A ``domain`` is one computation. The electrostatic one computes in two subdomains, the jacket and the air;
  the core is in neither, so it is excluded, and its surface is a boundary of the domain. Each ``group`` is a
  surface group: a name, the surfaces it holds (selected by what they are: an interface between two
  regions, a label, or ``remaining()``, every surface no other group holds), and its properties.
* The flow computes in the air alone; its walls are the cable's surface.
* A ``couple`` joins two domains, here through a subdomain each (the gas, which both compute in) and through a
  surface group each (the cable's surface).
* ``view`` says how a viewer opens the model; programs ignore it.

:ref:`Chap:ExampleLanguage` reads ``cable/setup.ebg`` in C++ (see :ref:`Chap:ImplemLanguage` for the
calls).

Writing a model is easiest with ``ebgstudio`` (see :ref:`Chap:Studio`) open on it: its window reloads the model
whenever a file is saved, and ``ebgstudio check`` reports what is wrong, line by line. The tutorial,
:ref:`Chap:StudioCoolingBlock`, builds a model that way, step by step, and is the place to start.

The pages of this section
-------------------------

* :ref:`Chap:LanguageModels`: values and params, shapes and operations, the object, regions and
  materials, labels, files, and how a larger model is organized.
* :ref:`Chap:LanguageDomains`: subdomains, properties, surface groups and their selectors, when a
  domain is complete, and couplings.
* :ref:`Chap:LanguageShapes` and :ref:`Chap:LanguageOperations`: every built-in shape and operation, with
  a line to copy.
* :ref:`Chap:LanguageReference`: the grammar, every keyword, the view block, the limits, and the errors and
  warnings a reader reports.
