.. _Chap:LanguageModels:

Writing a model
===============

This page describes the parts of a model in the order a file usually has them: values, shapes, the
object and its regions, labels, and the files a model is spread over. Domains have a page of their own,
:ref:`Chap:LanguageDomains`, and :ref:`Chap:LanguageReference` has the grammar.

.. contents:: On this page
   :local:
   :depth: 1

Files and statements
--------------------

Every file starts with its version, ``ebg 0``. The language is unstable, with no promise of compatibility,
until it is frozen as version 1; from then on, a change that breaks a file raises the version, and a reader
refuses a version it does not know.

A file is a list of statements, one per line; a statement continues on the next line inside parentheses,
after an operator or a comma, and when the next line starts with ``.`` (a method continued). ``#`` starts a
comment. There are no units: the numbers mean what the program reading them takes them to mean. Angles are
in degrees, and there is one coordinate frame.

Values and params
-----------------

A statement ``name = value`` defines a value, once; a name is used only after its definition. Values are
numbers, vectors ``(x, y, z)``, strings in double quotes, ``true`` and ``false``, and shapes. Numbers and
vectors take ``+``, ``-``, ``*`` and ``/`` (a vector times or over a number), and ``pi`` is a number:

.. code-block:: text

   length = 6
   center = (0, 0, length / 2)
   angle  = 30

A ``param`` is a value that may be changed from outside the file: ``-D length=8`` to ``ebgstudio``, or an
override passed to the reader (see :ref:`Chap:ImplemLanguage`), written as it would be in the file
(``-D "shift=(0, 2, 0)"``). A plain value may not be overridden, so that a model declares what it exposes:

.. code-block:: text

   param length  = 6     # may be overridden
   half          = length / 2

Overriding a name that is not a param is an error naming the params there are. Numbers are read as
``double``, and the shapes are built in the precision of the program that reads the model.

There are no loops and no functions of your own: a model reuses by name. Every option is an argument of the
call it affects; nothing is set globally.

Shapes and operations
---------------------

A shape is a call of a built-in shape, and an operation a call of a built-in operation that takes shapes
and gives one:

.. code-block:: text

   ball   = sphere((0, 0, 0), 1)
   rod    = cylinder((0, -2, 0), (0, 2, 0), radius = 0.3)
   part   = difference(box((-1, -1, -1), (1, 1, 1)), ball)
   placed = translate(part, (5, 0, 0))

Arguments come in the order of the shape's parameters, then by name (``radius = 0.3``); a missing, extra or
repeated argument is an error that lists the parameters. A call can also be written as a method of its
first argument, ``a.op(args)`` meaning ``op(a, args)``, so that a chain of operations reads in the order it
is applied:

.. code-block:: text

   arm = box((0, 0, 0), (4, 0.5, 0.5)).rotate(30, "z").translate((1, 0, 0))

:ref:`Chap:LanguageShapes` lists every shape and :ref:`Chap:LanguageOperations` every operation. A value the
library refuses (a negative radius, a zero axis) is an error at its call, as a reader reports every error
instead of ending the program.

The object
----------

The model's geometry is its object, one shape: ``object <shape>``, or ``object <name> = <shape>``, which also
defines the name. There is no implicit union of everything defined: a shape the object does not use is left
out, which makes it easy to keep a part aside while trying another.

.. code-block:: text

   object union(placed, rod)

Regions and materials
---------------------

An object of several materials is an ``overlay`` of regions (see :ref:`Chap:Regions`): each line is a region,
a name, a shape and, optionally, a material, and where shapes overlap the later region lies on top.

.. code-block:: text

   object overlay {
     region air    = air_box material air
     region jacket = insulation material PTFE
     region core   = conductor material copper
   }

The region's name says which part of the object it is, its material what it is made of: two regions of
the same material keep names of their own. The space beyond every region is the region ``outside``.

An object that is not an overlay is one region with no name. A domain needs regions by name, so a model with
domains over a single solid makes it an overlay of one region:

.. code-block:: text

   object overlay {
     region part = difference(box((-1, -1, -1), (1, 1, 1)), ball) material steel
   }

An overlay can be a region of another overlay, where its name names the *group* of its regions: a subdomain
or a selector naming the group stands for all of them, and a material on the group is the default of its
regions.

.. code-block:: text

   insulation = overlay {
     region outer_layer = cylinder((0, 0, -3), (0, 0, 3), 0.9)
     region inner_layer = cylinder((0, 0, -3), (0, 0, 3), 0.7)
   }

   object overlay {
     region air        = box((-3, -3, -3), (3, 3, 3)) material air
     region insulation = insulation material PTFE       # both layers are PTFE
     region core       = cylinder((0, 0, -3), (0, 0, 3), 0.5) material copper
   }

An overlay can be the object, be moved (``translate``, ``rotate``, ``scale``, ``reflect``), clipped or drilled
(as the first operand of ``intersection`` or ``difference``, whose other operands remove space from every
region), labeled, or be a region of another overlay. Anywhere else, in a union or under an offset, say, which
region a point belongs to would not be clear, and it is an error.

A ``material`` statement gives a material's properties, the defaults its regions take in every domain (see
:ref:`Chap:LanguageDomains`):

.. code-block:: text

   material copper { conductivity = 5.8e7 }
   material PTFE   { permittivity = 2.1, density = 2200 }

A material named by a region and never declared has no properties, and one declared but used by no region
does nothing; both are warnings.

Labels and the parts of shapes
------------------------------

A label names a surface, so that surface groups can select it by what it is. ``.label("skin")`` names a
shape's whole surface, and ``.label(xmin = "inlet", xmax = "outlet")`` names its parts, the pieces of its
surface the shape tells apart: a box's six faces, a cylinder's side and two caps, a mesh's face groups (every
shape's parts are in :ref:`Chap:LanguageShapes`). Both can be given at once, the parts' names overriding the
whole surface's.

.. code-block:: text

   pipe = cylinder((0, 0, 0), (4, 0, 0), 0.25).label("bore", cap1 = "inlet", cap2 = "outlet")

A label names a shape's own parts, so it goes on the shape before the shape is moved or combined:
``box(...).label(xmin = "inlet").translate(v)``, not ``translate(box(...), v).label(xmin = "inlet")``.

A shape with a name can also be labeled on a line of its own, in any file of the model, before or after its
definition:

.. code-block:: text

   air_box.label(xmin = "inlet", xmax = "outlet")

The labels then apply wherever the shape is used, so the geometry can stay in one file and the names a
setup needs in another. Only a shape made by one call can be labeled this way, not a composition, and a part
labeled twice is an error naming both places. The statement's arguments are read where the shape is defined,
so a name used in them must be defined before the shape.

Geometry is never named by a number: a number changes when the geometry does, a label does not.
``ebgstudio check --inventory`` lists every label, every shape's parts, the regions and the materials.

Files and includes
------------------

``include("file.ebg")`` reads another file into the model, with one namespace for all of them: a name
defined twice, in one file or two, is an error naming both places. A path is relative to the file it is
written in, and so is a mesh's. A file is read once however many files include it, by whatever path, and a
file that includes itself, through others, is an error.

A model is the file a reader is given and the files it includes; nothing else is read. The materials,
domains and couplings are resolved after the whole model is read, so they may refer to regions and labels
of files included later, and live in files of their own.

Organizing a model
------------------

A small model is one file. A larger one keeps its geometry in ``model.ebg`` and what is computed on it (the
materials, the domains, the couplings, the view) in ``setup.ebg``, which begins with ``include("model.ebg")``
and is the file a program reads. When one geometry has several setups, each is a file of its own that
includes the geometry:

.. code-block:: text

   cable/
     model.ebg           the geometry: shapes, regions and the materials they are of
     materials.ebg       the materials' properties, shared by the setups
     cfd.ebg             include("model.ebg"), include("materials.ebg"), domain cfd
     electrostatics.ebg  include("model.ebg"), include("materials.ebg"), domain electrostatics
     coupled.ebg         include("cfd.ebg"), include("electrostatics.ebg"), the couplings between them

Each setup reads on its own, and ``coupled.ebg`` combines two: the geometry and the materials both include
are read once. A domain or a coupling lives in one file, which every setup that needs it includes. A
geometry opened on its own is the way to work on its shapes and labels; a setup, on its domains.

The view
--------

A ``view`` block says how a viewer opens the model: the camera, the colors of names, the background.
Programs reading the model ignore it. It is usually in the setup, once per model:

.. code-block:: text

   view { camera = (7, 5, 9), target = (1, 0, 0), color.copper = (0.8, 0.5, 0.2), floor = false }

:ref:`Sec:LanguageView` lists its keys.
