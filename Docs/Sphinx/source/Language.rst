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

A model is a file, or a folder holding ``model.ebg`` (the geometry) and ``setup.ebg`` (materials, domains, couplings
and the view, read after it, the file a viewer writes). A cable along *z*, in a box of air with a flow across it:

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

and its ``setup.ebg``:

.. code-block:: text

   ebg 0

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

Files, folders and includes
---------------------------

``include("file.ebg")`` brings in another file's names and declarations, in one namespace: a name defined twice is an
error naming both places. Materials, domains and couplings are resolved after the whole model is read, so they may live
in their own file. A ``view`` block holds what a viewer shows (the camera, colors by name) and programs ignore.

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
