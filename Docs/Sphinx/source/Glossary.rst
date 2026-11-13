.. _Chap:Glossary:

Glossary
========

The words this guide uses with a meaning of its own, each with the page that explains it. Three kinds of
name share the word *label* in the API; this guide keeps them apart as :term:`surface label`,
:term:`material` and :term:`domain label`.

.. glossary::
   :sorted:

   signed distance
      A function of a point: the distance to an object's surface, negative inside, positive outside, zero
      on the surface. See :ref:`Chap:GeometryRepresentations`.

   implicit function
      A function of a point whose sign says inside (negative) or outside (positive), and whose magnitude may
      mean anything. Every signed distance is one. See :ref:`Chap:GeometryRepresentations`.

   distance quality
      How far a value can be trusted as a distance: ``Exact``, a ``Bound`` (never more than the distance),
      or ``NotADistance`` (only the sign). See :ref:`Chap:GeometryRepresentations`.

   shape
      Anything with a signed value at a point and a bounding box: an analytic shape, a mesh distance
      function, a cloud, a composition, or a type of your own. Shapes compose, ray trace, render and run on
      a GPU. See :ref:`Chap:GeometryRepresentations`.

   primitive
      A shape that is not built of other shapes in a composition: an analytic shape, a mesh distance
      function, a cloud. In a tape, each occurrence of a primitive is a :term:`leaf`.

   bounding box
      The axis-aligned box a shape lies in. Compositions work out theirs from their operands'; a function
      with none gets an estimated one. See :ref:`Chap:GeometryRepresentations`.

   expression
      A composition described as a tree of shapes and operations (``Expr``), before it is compiled. See
      :ref:`Chap:ImplemCSG`.

   tape
      An expression compiled into a flat program that runs on the host and on a GPU, with one array per
      kind of primitive. See :ref:`Chap:Tape`.

   fused composition
      A composition whose structure is fixed when the program is compiled, each operation a type: compiled
      code instead of a program to interpret. See :ref:`Chap:ConstructiveSolidGeometry`.

   BVH
      A bounding volume hierarchy: a tree of boxes over primitives, which lets a query skip whatever is far
      away. See :ref:`Chap:BVH`.

   BVH union
      A union of many objects through a BVH, so that a query visits only the objects near it. See
      :ref:`Sec:BVHUnions`.

   branching factor
      ``K``, the number of children of a BVH's interior nodes. See :ref:`Sec:DefaultKW`.

   triangle group
      ``W`` triangles stored together, coordinate by coordinate, so that one SIMD instruction handles all of
      them; ``W`` is the group's width. See :ref:`Chap:Triangles`.

   leaf
      One occurrence of a primitive in an expression, numbered in the order the expression is written.
      See :ref:`Chap:LeavesAndSurfaces`.

   deciding leaf
      The leaf a composition's value at a point is taken from: the part of the object whose surface the
      value comes from, which need not be the nearest primitive. See :ref:`Chap:LeavesAndSurfaces`.

   surface
      In a scene, one numbered surface: a leaf, or one member of a collection (a sphere of a cloud, an object
      of a BVH union). See :ref:`Chap:LeavesAndSurfaces`.

   part
      A piece of one shape's own surface that the shape tells apart: a box's six faces, a cylinder's side and
      caps, a mesh's faces. Parts belong to the shape, not to the composition. See :ref:`Sec:Parts`.

   face id
      A mesh face's position in the file it was read from, which the mesh distance functions report for the
      closest face. See :ref:`Sec:FaceIds`.

   surface label
      A name given to a surface, or to each of a shape's parts, so that conditions can be attached to it by
      name (``SurfaceLabel()``, ``SurfaceLabelId``). Not to be confused with a :term:`domain label`. See
      :ref:`Chap:LeavesAndSurfaces`.

   overlay
      A composition of regions: each operand is a :term:`region`, later ones on top where they overlap.
      See :ref:`Chap:Regions`.

   region
      One part of an object made of several, the part of an overlay's operand not covered by later operands.
      Regions tile the object; every point is in exactly one. See :ref:`Chap:Regions`.

   outside
      The region beyond every operand of an overlay, region 0; also called the background.

   region group
      An overlay used as a region of another overlay: its regions, named together.

   material
      What a region is made of, a name (``MaterialLabel()``, ``MaterialLabelId``). Two regions of the same
      material are still two regions. See :ref:`Chap:Regions`.

   piece
      A shape's surface between two given regions: the unit a domain assigns conditions to, and what a click
      in the viewer picks. See :ref:`Chap:Regions`.

   domain
      What one computation runs on: its subdomains and its surface groups. See :ref:`Chap:Regions`.

   subdomain
      A set of regions a domain computes in. See :ref:`Chap:Regions`.

   excluded
      The regions in none of a domain's subdomains, where the domain does not compute.

   domain label
      What a domain says a point is in: one of its subdomains, or excluded (``Domain::labelAt()``); each has
      a function negative exactly there. Not to be confused with a :term:`surface label`. See
      :ref:`Chap:ImplemRegions`.

   role
      What a surface is to a domain at a point: a *boundary* (one side in the domain), an *interface* (a
      different subdomain on each side), *internal* (two regions of one subdomain), or none. See
      :ref:`Chap:Regions`.

   surface group
      A set of surfaces a domain gives one condition (an inlet, a wall), chosen by selectors. See
      :ref:`Chap:Regions`.

   selector
      A condition on what a surface is (its label, the regions or materials on its sides, a piece), which puts
      the surfaces it holds in a surface group. See :ref:`Chap:ImplemRegions`.

   complete
      A domain is complete when every piece of surface it touches is in a surface group. See
      :ref:`Chap:Regions`.

   pool
      The block of memory a structure's arrays live in, copied to a GPU in one piece. See
      :ref:`Chap:MemoryModel`.

   pool handle
      A structure holding offsets into a pool rather than pointers (a mesh distance function, a BVH, a
      tape): cheap to copy, and valid as long as its pool lives. See :ref:`Chap:MemoryModel`.

   mirror
      A copy of a frozen pool in other memory, such as a GPU's; ``rebasedView()`` gives a pool handle that
      reads the mirror. See :ref:`Chap:MemoryModel`.

   ray form
      A shape's exact intersection with a ray. A shape without one is marched. See :ref:`Chap:RayTracing`.

   marching
      Finding where a ray crosses a surface by stepping along it, by the value where it is a distance (sphere
      tracing). See :ref:`Chap:SphereTracing`.

   span
      A stretch of a ray inside a shape, from where it enters to where it leaves; compositions are traced from
      their parts' spans. See :ref:`Chap:RayTracing`.

   tape feature
      Code a tape compiles only when its type lists it, such as exact offsets. See :ref:`Sec:TapeFeatures`.
