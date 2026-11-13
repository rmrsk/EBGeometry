.. _Chap:Language:

The .ebg language
=================

A model can be written in C++, composed with the expression builder and compiled into a tape, or it can be written as
text and read at run time. The ``.ebg`` language is that text: the geometry, its regions and their materials, and the
domains a solver computes in, with their subdomains, boundary conditions and couplings, in a file a program reads (with
:ref:`Chap:ImplemLanguage`) and a person writes, checks and looks at (with ``ebgstudio``), with no compiler. A
viewer writes it too: the surface groups made by clicking surfaces in the window go into the same files.

The language describes; it computes nothing. What a property means (a density, a boundary condition's kind) is the
program's to say: the language passes numbers, vectors and strings through unread, checks that every one a domain needs
is given, and gives a program each subdomain's and surface group's properties as rows of its own type, in the pool, read
on a device by the numbers EBGeometry computes there.

A first model
-------------

A model is a file, and the files it includes. Usually the geometry is in one, ``model.ebg``, and what is computed on it
in another, ``setup.ebg``, which includes the geometry and is the file a program reads. A cable along *z*, in a box of
air with a flow across it, ``model.ebg``:

.. code-block:: text

   ebg 0

   # A cable along z, in a box of air with a cross-flow along x.
   param length = 6
   param r_core = 0.5

   conductor  = cylinder((0, 0, -length/2), (0, 0, length/2), radius = r_core)
   insulation = tube((0, 0, -length/2), (0, 0, length/2), inner_radius = r_core, outer_radius = 0.8)
   air_box    = box((-3, -2, -length/2), (5, 2, length/2))
                  .label(xmin = "inlet", xmax = "outlet", ymin = "far", ymax = "far", zmin = "ends", zmax = "ends")

   object overlay {                 # each region lies on top of the ones before it
     region air    = air_box     material air
     region jacket = insulation  material PTFE
     region core   = conductor   material copper
   }

and its ``setup.ebg``, which names the geometry it is a setup of:

.. code-block:: text

   ebg 0

   include("model.ebg")

   material copper { conductivity = 5.8e7 }
   material PTFE   { permittivity = 2.1, density = 2200 }
   material air    { permittivity = 1.0, density = 1.2, viscosity = 1.8e-5 }

   domain cfd {
     subdomain fluid = air
     group inlet  = label("inlet")                { velocity = (2, 0, 0) }
     group outlet = label("outlet")               { pressure = 0 }
     group walls  = interface(air, jacket)        { kind = "no_slip" }
     group slip   = label("far"), label("ends")   { kind = "slip" }
   }

   view { camera = (7, 5, 9), target = (1, 0, 0), color.copper = (0.8, 0.5, 0.2) }

Every file starts with its version, ``ebg 0``: the language is unstable, with no promise of compatibility, until it is
frozen as version 1; from then on a change that breaks a file raises the version. Numbers are read as ``double`` and the
model is built in the program's precision. There are no units: the program interprets the values. Angles are in degrees,
and there is one frame.

Values and params
-----------------

A statement ``name = value`` defines a value, once; it is used after its definition. Values are numbers, vectors
``(x, y, z)``, strings, ``true`` and ``false``, and shapes, with arithmetic (``+ - * /``) on numbers and vectors and
``pi``. A ``param`` is a value a reader may override from outside, ``-D length=8`` to ``ebgstudio``, or the overrides of
``Language::read()``; a plain value may not be overridden, so that what a model exposes is declared. A value with an
error makes no further errors where it is used.

Nothing is global: every option is an argument of the call it affects. There are no loops and no user functions: a
model reuses by name.

Shapes and operations
---------------------

A shape is a call of a built-in shape (``sphere``, ``box``, ``cylinder``, a ``mesh`` read from a file, ...), and an
operation a call of a built-in operation (``union``, ``difference``, ``smooth_union``, ``translate``, ``rotate``,
``offset``, ``repeat``, ...). Arguments are positional, in the order of the shape's parameters, then by keyword, the
parameter's name (``radius = 0.5``); a missing, extra or repeated argument is an error listing the parameters. The
method form ``a.op(args)`` means ``op(a, args)``, so that a chain of operations reads in order:
``box(lo, hi).rotate(30, "z").translate(v)``. Every shape, with a line to copy, is in :ref:`Chap:LanguageShapes`,
and every operation in :ref:`Chap:LanguageOperations`.

The object is the model's one shape, ``object <shape>``, or ``object <name> = <shape>``, which also defines the name
for later lines: there is no implicit union. A shape defined and not used in
it is kept out of the object, which makes it easy to keep a part aside while trying another.

Regions and materials
---------------------

An ``overlay`` holds the regions of an object, each on top of the ones before it (see :ref:`Chap:ClosestPrimitive`):
``region <name> = <shape> material <name>``. The region's name says which region it is, its material what it is made
of; the material is optional. An overlay can be the object, be moved (``translate``, ``rotate``, ``scale``,
``reflect``), clipped (the first operand of ``intersection`` or ``difference``), labeled, or be a region of another
overlay, where its name names the group of its regions; anywhere else, which region a point belongs to would not be
clear, and it is an error. The outside, region 0, is named ``outside``.

A ``material`` statement gives a material's properties: the defaults its regions take in every domain. A material named
by a region and never declared has none (a warning).

Labels and the parts of shapes
------------------------------

Surfaces are named by labels: ``.label("inlet")`` names a shape's surface, ``.label(xmin = "inlet", ymax = "far")``
names its parts. Each built-in shape has fixed part names (a box's ``xmin`` ... ``zmax``, a cylinder's ``side``,
``cap1`` and ``cap2``), and a mesh's parts are its file's face groups, labeled by their names unless ``groups = false``
is given. A shape with a name can also be labeled on a line of its own, ``air_box.label(xmin = "inlet")``, in any file
of the model and before or after the shape's definition: the labels apply wherever the shape is used, so that the
geometry can stay in ``model.ebg`` and its names in ``setup.ebg``. Only a shape made by one call can be labeled so (not
a composition), and a part labeled twice is an error naming both places. Since the labels apply where the shape is
defined, the statement's arguments are read there too: a name in them must be defined before the shape. Geometry is
never named by a number: a number changes when the geometry does, a label does not.
``ebgstudio check --inventory`` lists every label, every shape's parts, the regions and the materials.

Domains
-------

A ``domain`` is what a solver computes on: its **subdomains**, each a set of regions by name (a region group's name
takes its regions, ``outside`` is region 0), and its **surface groups**, each a list of selectors, the first that holds
a surface wins. The selectors name what a surface is, never where:

* ``label("x")`` the surfaces with a label, ``label("x", between = (a, b))`` those of them between two regions;
* ``interface(a, b)`` the surfaces between two regions, either way round;
* ``region(a)`` every surface of a region;
* ``materials(m1, m2)`` the surfaces between two materials (one material: either side);
* ``remaining()`` every surface no other selector holds, whatever the order.

A subdomain's properties are its own over its regions' material's, key by key, so that the same region is air to one
solver and a dielectric to another; regions of materials that differ on a key need the subdomain to set it. A program
builds the domain for its tape from the model's setup (see :ref:`Chap:ImplemLanguage`).

A domain is **complete** when every subdomain has properties and every piece of surface bounding it (a boundary, or an
interface between two of its subdomains) is held by a surface group. Completeness is checked for the domains a solver
asks for, not the whole file: ``ebgstudio check --domain cfd``. A missing piece is reported by the shape call it comes
from and the regions on its two sides.

A **coupling** joins two domains: ``couple name = volume(a.sub, b.sub)``, two subdomains holding the same regions, or
``couple name = surface(a.group, b.group)``, two surface groups holding the same pieces; both are checked, and their
properties pass through.

Files and includes
------------------

``include("file.ebg")`` brings in another file's names and declarations, in one namespace: a name defined twice is an
error naming both places. A file is read once however many files include it, by whatever path; a file including
itself, through others, is an error. A path is relative to the file it is written in. Nothing is read that a file does
not include: a reader is given one file (a folder is an error). Materials, domains and couplings are resolved after the
whole model is read, so they may live in their own file. A ``view`` block holds what a viewer shows (see below) and programs ignore.

Organizing a model
------------------

A small model is one file. A larger one keeps its geometry in ``model.ebg`` and what is computed on it (materials,
domains, couplings, the view) in ``setup.ebg``, which begins with ``include("model.ebg")`` and is the file read.

When one geometry has several setups, each setup is a file of its own that includes the geometry, and is read by
itself:

.. code-block:: text

   cable/
     model.ebg           the geometry: shapes, regions and the materials they are of
     materials.ebg       the materials' properties, shared by the setups
     cfd.ebg             include("model.ebg"), include("materials.ebg"), domain cfd
     electrostatics.ebg  include("model.ebg"), include("materials.ebg"), domain electrostatics
     coupled.ebg         include("cfd.ebg"), include("electrostatics.ebg"), the couplings between them

Each setup reads on its own (``cable/cfd.ebg``), and ``coupled.ebg`` combines them: the geometry and the materials both
include are read once. A domain or a coupling lives in one file, which every setup needing it includes.

The view: camera and colors
---------------------------

A ``view`` block, once per model and usually in the setup file, says how a viewer opens the model:

.. code-block:: text

   view { camera = (7, 5, 9), target = (1, 0, 0), background = (1, 1, 1), floor = false, color.copper = (0.8, 0.5, 0.2) }

* ``camera`` is where the eye is; ``target`` what it looks at (by default the center of the object's box); ``up`` the
  direction that is up in the window (by default ``(0, 1, 0)``). Without a camera, the viewer fits one to the object.
* Or the camera as a viewer prints it: ``target``, ``yaw`` and ``pitch`` (in degrees) and ``distance``, each over the
  fitted camera's when left out; not together with ``camera``. ``field_of_view`` (in degrees) and ``orthographic =
  true`` go with either.
* ``background = (r, g, b)`` is a plain background in that color, each component from 0 to 1, instead of the sky's
  gradient; ``sky = false`` gives the plain default one, and ``floor = false`` leaves out the floor under the object.
* ``color.NAME = (r, g, b)`` gives a name a color, each component from 0 to 1: a material's, a region's, a surface
  label's or a surface group's. The viewer uses it when it colors by that kind of name (by material, region, label or
  surface group); a name without one takes the built-in palette's.

Programs reading the model ignore the block; a program that views the model gets the colors with its names
(``Model::names()``).

Errors
------

Every error and warning has a file, a line and a column, and a model with errors reports them all, with one
exception: after an error inside braces (a ``domain``, or the properties of a ``material``), the rest of the braces is
skipped, so a second error there shows once the first is fixed. Values the library refuses (a negative radius, a zero
axis) are errors at their call, as are misplaced overlays and names given twice: reading a model reports them instead
of ending the program.

The canonical form
------------------

``print`` writes a model in canonical form, one statement per line, comments kept: the form a viewer writes, and a
formatter. It keeps ``include()``; ``--flatten`` writes one text.
