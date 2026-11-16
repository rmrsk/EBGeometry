.. _Chap:LanguageDomains:

Domains and couplings
=====================

A domain is what one computation runs on (see :ref:`Chap:Regions` for the concepts): its subdomains, the
regions it computes in, and its surface groups, the surfaces it gives conditions. This page shows how they
are written, with a steel block cooled by water through a channel, ``channel.ebg``:

.. code-block:: text

   ebg 0

   block   = box((0, 0, 0), (4, 1, 2))
   channel = cylinder((0, 0.5, 1), (4, 0.5, 1), 0.25).label(cap1 = "inlet", cap2 = "outlet")

   object overlay {
     region solid = block material steel
     region water = channel material water
   }

   material steel { conductivity = 45 }
   material water { conductivity = 0.6, viscosity = 1.0e-3 }

   domain flow {
     subdomain fluid = water
     group inlet  = label("inlet") { velocity = (0.5, 0, 0) }
     group outlet = label("outlet") { pressure = 0 }
     group wall   = interface(water, solid) { kind = "no_slip" }
   }

.. contents:: On this page
   :local:
   :depth: 1

Subdomains
----------

``subdomain <name> = <regions>`` lists the regions a subdomain computes in, by name: usually one, or several
separated by commas, or a region group, which stands for all its regions. ``outside`` is the region beyond
the object. A region in no subdomain is excluded: the domain does not compute there, and a surface between
it and a subdomain is a boundary of the domain.

Properties
----------

A subdomain, a surface group and a coupling can carry properties, in braces after them: ``{ key = value, ... }``,
the values numbers, vectors, strings or ``true``/``false``. The language does not read them; it hands them to
the program, which knows what ``viscosity`` or ``kind = "no_slip"`` means. A key may have dots in it
(``inlet.profile = "parabolic"``), which a program may use to group keys.

A subdomain's properties are its own over its regions' materials', key by key. So the same region can be air
to one computation and a dielectric of another, and a subdomain overrides one key and keeps the rest:

.. code-block:: text

   material PTFE { permittivity = 2.1, density = 2200 }

   domain electrostatics {
     subdomain dielectric = jacket { permittivity = 2.5 }   # permittivity 2.5, density 2200
   }

A subdomain of regions whose materials differ on a key must set that key itself, since it would otherwise
have two values; a subdomain with no properties of its own and none from its materials is an error when its
domain is checked (below).

Surface groups and selectors
----------------------------

``group <name> = <selectors> { properties }`` is a surface group: the surfaces its selectors hold, and the
properties of the condition on them. A selector says what a surface is, never where:

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Selector
     - Holds
   * - ``label("inlet")``
     - the surfaces labeled ``inlet``
   * - ``label("inlet", between = (water, solid))``
     - those of them between two regions
   * - ``interface(water, solid)``
     - the surfaces between two regions, either way round
   * - ``region(water)``
     - every surface with the region on a side
   * - ``materials(steel, water)``
     - the surfaces between two materials, either way round; ``materials(steel)``, those with the material on
       a side
   * - ``remaining()``
     - every surface no other selector of the domain holds

A region named in a selector may be a region group, standing for its regions. A group may list several
selectors, separated by commas: ``group slip = label("far"), label("ends")``.

**Which group a surface is in.** A surface may be held by several selectors. It goes to the group of the
first selector that holds it, in the order the selectors are written in the domain, across all its groups;
``remaining()`` is tried after all the others, wherever it is written. So a few special surfaces can be
given a group before a broader one takes the rest:

.. code-block:: text

   domain heat {
     subdomain metal = solid
     group heated    = label("base") { heat_flux = 5.0e4 }   # the base, though it is also steel's surface
     group cooled    = interface(solid, water)
     group insulated = region(solid) { heat_flux = 0 }       # every other surface of the steel
   }

When completeness is checked
----------------------------

A domain is **complete** when every subdomain has properties, every surface it touches (a boundary, or an
interface between two of its subdomains) is in a surface group, and its couplings join what they say they
join. A program asks for the domains it computes on (see :ref:`Chap:ImplemLanguage`), and ``ebgstudio check
channel.ebg --domain flow`` checks one from the command line; nothing requires every domain of a file to be
complete.

A surface left out is reported by the shape it comes from and the regions on its two sides, at the line of
the domain. Without the ``wall`` group above:

.. code-block:: text

   channel.ebg:14:8: error: the cylinder at channel.ebg:4 between 'water' and 'solid' is in no surface group of domain 'flow'
   domain 'flow': incomplete

The check works from the model's description, so it lists every surface the object may have; a surface
the geometry turns out not to have (a shape buried under a later region) can be reported without existing.
A selector that holds only some of a surface's parts (two faces of a box's six) leaves the rest unassigned.

Couplings
---------

A coupling joins two domains of the model where they share something, for a program that couples their
computations. It is checked, and its properties pass through:

* ``couple <name> = volume(a.sub, b.sub)``: a subdomain of each domain, which must hold the same regions;
* ``couple <name> = surface(a.group, b.group)``: a surface group of each, which must hold the same surfaces.

The block's heat, in a setup that includes the flow, coupled to it through the channel's wall:

.. code-block:: text

   ebg 0

   include("channel.ebg")

   domain heat {
     subdomain metal = solid
     group cooled    = interface(solid, water)
     group insulated = remaining() { heat_flux = 0 }
   }

   couple wall_heat = surface(heat.cooled, flow.wall) { transfer = "heat" }

A coupling lives in a file that includes both domains; a setup that reads only one domain does not see it.
