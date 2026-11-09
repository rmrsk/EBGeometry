.. _Chap:ClosestPrimitive:

Which part: leaves and surfaces
===============================

A composed object answers one question at a point, its signed distance, but an application often needs
a second: which part of the object is that? A boundary condition depends on whether a wall is the inlet,
the outlet or the casing; a material on whether a surface is the machined bore or the cast body; a
picture or a label on which object a pixel shows. A tape answers this with ``closestLeaf(p)``. It
names the leaf of the composition whose surface the value at ``p`` comes from: the *deciding primitive*.
When that leaf holds many things itself (a cloud of spheres, a BVH union of objects, another tape),
``closestSurface(p)`` goes on into it and names the one that decides there (see :ref:`Sec:Surfaces`).

The deciding primitive, not the nearest one
-------------------------------------------

A composition takes its value from its operands' values: a union takes the smallest, an intersection the
largest, a difference the largest of the first operand's value and the negated values of the others.
Whichever operand the value is taken from is the one that decides it, and following that choice down
the composition ends at a leaf. That leaf is the one ``closestLeaf(p)`` names.

This is not the same as the primitive whose surface is geometrically nearest to ``p``. A composition
removes parts of its primitives' surfaces, and the nearest primitive surface may be one of those removed
parts, which is no longer part of the object. :numref:`Fig:ClosestPrimitive` shows a cross-section of
``Difference(box, sphere)``, a box with a ball carved out of one side. The plane is colored by the leaf
that decides the value: the sphere near the cavity, on both sides of its wall, and the box everywhere
else.

.. _Fig:ClosestPrimitive:
.. figure:: /_static/ClosestPrimitive.png
   :width: 70%
   :align: center

   ``Difference(box, sphere)`` in a plane through the box's and the ball's centers, colored by the
   deciding leaf (the box is leaf 0, the sphere leaf 1). The dashed lines are the parts of the two
   primitives' surfaces that the difference removed. At :math:`p_1`, outside the object beside the
   ball's removed cap, the nearest primitive surface is the sphere's, but the value comes from the box,
   and the box decides. At :math:`p_2`, just outside the box's removed face, it is the other way around.

The point :math:`p_1` is the typical case: the cap of the ball that sticks out of the box is not part of
the object, so the object's surface near :math:`p_1` is the box's, and an application tagging that
surface wants the box. The deciding leaf gives it; the nearest primitive would not.

Each operation passes on a leaf as follows:

* A **union** takes the leaf of the operand it takes the value from (the smallest), an **intersection**
  the leaf of the largest. A **difference** is an intersection with the complements of the subtracted
  objects.
* A **smooth union, intersection or difference** blends the two most extreme operands' values and takes
  the leaf of the more extreme one.
* A **blur** takes the leaf of its center sample, a **mollification** that of its sample with the
  largest weight.
* A **complement, offset, annulus, translation, rotation, scaling, reflection, elongation or repetition**
  passes on its operand's leaf. Every copy of a repeated object is the same leaf; the copy whose value
  is taken decides which local point is reported (see below).
* A **nested composite is one leaf**: a fused composition (see :ref:`Sec:Fused`), which is one shape
  with one surface however it is built, a tape listed as a primitive of another tape, a BVH union of
  many objects and a sphere cloud. A scene is therefore a tape whose leaves are its parts: it names the
  part that decides the value, and each part can be as large a composition as it likes. The collections
  among them (all but the fused composition) are drilled into by ``closestSurface()``.

Asking for it
-------------

``closestLeaf(p)`` is a member of the
`Tape <doxygen/html/classEBGeometry_1_1Tape.html>`__ and returns a
`LeafRef <doxygen/html/structEBGeometry_1_1LeafRef.html>`__:

* ``m_leaf``: the leaf's number. The leaves are numbered in the order they appear in the expression,
  depth first, counting each occurrence, so a shape used twice is two leaves. In
  ``Difference(Intersection(cube, ball), Union(drills))`` the cube is leaf 0, the ball leaf 1 and the
  three drills leaves 2 to 4. The number is the one to index a table of your own with: materials,
  boundary conditions, colors. It is ``LeafRef::None`` when the query names no leaf, so check it
  before indexing.
* ``m_type`` and ``m_index``: which of the tape's primitive types the leaf is, and which primitive of that
  type. They are what the accessors below use; most programs need not read them directly.
* ``m_localPoint``: the point in the primitive's own frame, after the translations, rotations, scalings,
  reflections, elongations and repetitions above the leaf. This is where the primitive itself can be
  queried.

The tape also returns the primitive the reference names:

* ``isPrimitive<P>(ref)`` says whether the leaf is a ``P``. ``P`` must be one of the tape's primitive
  types; any other type is a compile error.
* ``primitive<P>(ref)`` returns it by value: the shape itself, or for a primitive that lives in a pool (a
  mesh distance field, a BVH union, a nested tape) a handle that resolves against the same pool as the
  tape, or the same mirror of it on a device.
* ``visitPrimitive(ref, f)`` calls ``f`` with the primitive whatever its type, as ``primitive<P>()``
  returns it. In host code a generic lambda will do; in a CUDA or HIP translation unit, pass a functor
  whose call operator is ``EBGEOMETRY_HOST_DEVICE``, since the device compiler treats a lambda's call
  operator as host code there.

A mesh is one leaf. To tell its faces apart, ask the mesh itself for its closest face at the local
point:

.. code-block:: c++

   using Mesh = TriMeshSDF<T, BVH::DefaultBranchingRatio<T>(), TriangleSoA::DefaultWidth<T>()>;

   const LeafRef<T> ref = tape.closestLeaf(p);

   if (ref.m_leaf != LeafRef<T>::None && tape.isPrimitive<Mesh>(ref)) {
     const Mesh     mesh = tape.primitive<Mesh>(ref);
     const uint32_t face = mesh.getClosestFace(ref.m_localPoint).faceId;   // the face's position in the file
   }

The local point matters here: the mesh was translated, rotated or scaled into place in the expression,
and its own BVH knows only the mesh's own coordinates. A nested tape is asked the same way, for its own
deciding leaf at the local point.

.. _Sec:Surfaces:

Surfaces: inside a cloud, a union or a nested tape
--------------------------------------------------

A leaf can hold many things: a ``SphereCloudBVH`` thousands of spheres, a ``BVHUnion`` many objects, a tape
listed as a primitive its own leaves. ``closestLeaf(p)`` names the leaf; ``closestSurface(p)`` goes on into
it and names the member whose surface decides the value there, at any depth (a sphere of a cloud inside a
union inside a nested tape). It returns a
`SurfaceRef <doxygen/html/structEBGeometry_1_1SurfaceRef.html>`__:

* ``m_surface``: the surface, numbered across the whole scene. Each leaf covers a contiguous range of
  surface numbers, in leaf order: one surface for a leaf that is one shape, and for a collection one per
  member, in the collection's own order (a cloud's cloud indices, a union's members in the order its
  constructor was given them). ``getFirstSurface(leaf)`` is where a leaf's range starts, so data of your
  own per member (per sphere, say) is an array indexed by ``m_surface - getFirstSurface(m_leaf)``: no
  sphere needs a label.
* ``m_leaf`` and ``m_surfaceLabel``: the leaf, as ``closestLeaf()`` numbers it, and the surface's label (see below).
* ``m_part``: the part of the deepest shape reached, a box's face or a mesh's face (see :ref:`Sec:Parts`).
* ``m_localPoint``: the point in the frame of the deepest shape reached.

.. code-block:: c++

   const Tape<T, SphereCloudBVH<T>> tape(pool, Difference(SurfaceLabel(box, "wall"), SurfaceLabel(cloud, "particles")));

   const SurfaceRef<T> s = tape.closestSurface(p);   // on a device too

   restitution[s.m_surfaceLabel];                                     // per-label data
   sphereData[s.m_surface - tape.getFirstSurface(s.m_leaf)];   // per-sphere data, in cloud order

A collection costs one more query of the deciding leaf for each level drilled (a second search of the cloud,
say), paid only by ``closestSurface()``; for a tape whose leaves are all single shapes, the surface is the
leaf, and the tape stores nothing more. A cloud numbers as many surfaces as it has room for
(``maxSpheres()``), so a rebuild with another number of spheres keeps the scene's numbering. A shape of your
own becomes a collection by defining ``getNumSurfaces()`` and ``closestSurface()`` (see
`hasSurfaces <doxygen/html/namespaceEBGeometry.html>`__). A nested tape's labels are its own, and the outer
tape does not report them: label within one expression. A mesh is one surface; its faces are its parts.

``closestSurface(shape, p)`` is also a free function, for code templated on a shape: a collection answers as
above, and any other shape with surface 0 and its part.

.. _Sec:Parts:

Parts: a face of a shape
------------------------

A shape can have parts: the pieces of its own surface its formula tells apart, a box's six faces, a cylinder's
side and caps, a mesh's faces. ``SurfaceRef::m_part`` is the part of the deepest shape reached, read from the
shape's own formula at the local point: the term that decides the value there. A part belongs to the shape,
never to the composition: there is no boundary representation, so a box face that a difference cuts in two is
one part, and two boxes whose faces line up into one flat face of the result are two surfaces.

=====================================================  ===========================================================
Shape                                                  Parts
=====================================================  ===========================================================
``BoxSDF``, ``RoundedBoxSDF``                          6: -x, +x, -y, +y, -z, +z
``CylinderSDF``, ``CapsuleSDF``, ``CappedConeSDF``,    3: the side, the end at the first center, the end at the
``RoundConeSDF``, ``RoundedCylinderSDF``               second (-y and +y for the rounded cylinder)
``ConeSDF``                                            2: the side, the base
``TubeSDF``                                            4: the outer wall, the inner wall, the two ends
``PolygonPrismSDF``                                    the sides around the axis from the first vertex, then the
                                                       two caps
``CappedTorusSDF``                                     3: the tube, the two ends
``CutSphereSDF``, ``CutHollowSphereSDF``               2: the sphere and the cut face; the shell and its rim
``WedgeSDF``                                           2: the face on either side of the edge
the mesh distance fields                               one per face: the face's id, its position in the file
any other shape (a sphere, a torus, a fused part)      1: part 0
=====================================================  ===========================================================

A shape of your own gets parts by defining ``getNumParts()`` and ``closestPart(localPoint)`` (see
`hasParts <doxygen/html/namespaceEBGeometry.html>`__). The part is computed only by ``closestSurface()`` and
``firstSurface()``, once, for the deciding leaf; a mesh's costs one more closest-face search of that mesh.

Surface labels
--------------

A leaf number is fine for a program that builds its expression itself, but it depends on the expression's
order: an edit earlier in the expression renumbers everything after it. A name does not.
``SurfaceLabel(object, "name")`` names the surfaces of an object, and every leaf below it gets the label, unless
a label nearer the leaf names it. Labels change nothing in how the object is evaluated, and nothing has to be
labeled: an unlabeled leaf has no label (``SurfaceLabelId::none()``). Several objects may share a name, and so
a label.

A label is a
`SurfaceLabelId <doxygen/html/structEBGeometry_1_1LabelId.html>`__, a number typed by what it labels, so it
cannot be mixed up with another kind of number; ``value()`` is the number, for indexing an array of your own.

.. code-block:: c++

   const Expr<T> part = Difference(Intersection(SurfaceLabel(box, "wall"), ball), SurfaceLabel(Union(drills), "bores"));

   const Tape<T>      tape(pool, part);
   const ExprIndex<T> index(part);

   std::vector<Wall> walls(tape.getNumSurfaceLabels());   // per-label data of your own
   walls[index.findSurfaceLabel("wall").value()]  = ...;   // findSurfaceLabel() is none for an unknown name
   walls[index.findSurfaceLabel("bores").value()] = ...;

   const SurfaceLabelId label = tape.getSurfaceLabel(tape.closestLeaf(p));   // or tape.getSurfaceLabel(hit.m_primitive)

**A mesh's named patches.** A mesh file can name groups of its faces: an OBJ file's ``g`` and ``o`` groups,
an ASCII STL file's named solids. The readers keep them as
`FaceGroups <doxygen/html/structEBGeometry_1_1FaceGroups.html>`__ (``PolygonSoup::getFaceGroups()``), and
``SurfaceLabel(mesh, groups)`` labels each face by its group's name. A face's group label wins over every label
above the mesh; a face in no group keeps those. The group names are numbered with the other labels, so
``findSurfaceLabel("inlet")`` works as for any name, and ``SurfaceRef::m_surfaceLabel`` is the face's label:

.. code-block:: c++

   const auto soup = Parser::readIntoPolygonSoup<T>("pump.obj");       // groups inlet, outlet, casing
   const auto pump = Parser::readIntoTriMeshSDF<T>("pump.obj", pool);

   const Expr<T> part = Difference(SurfaceLabel(pump, soup.getFaceGroups()), SurfaceLabel(bore, "bore"));

   const Tape<T>       tape(pool, part);
   const SurfaceRef<T> s = tape.closestSurface(p);   // s.m_surfaceLabel: inlet, outlet, casing or bore

Any shape with parts can be labeled this way, with one group per part. ``SurfaceLabel(object, groups)`` takes one
shape, not a composition: label it before translating or combining it.

The tape reports a leaf's label with ``getSurfaceLabel()``, on the host and inside a GPU kernel, so a kernel
(a particle hitting a wall, say) looks up per-label data in an array of its own. The label is a number;
the names live on the host, in an
`ExprIndex <doxygen/html/classEBGeometry_1_1ExprIndex.html>`__ built from the same expression. Both
number the labels in the order their names first appear in the expression, depth first (a mesh's group names
after its own label), so the same expression gives the same numbers to both. ``getPartSurfaceLabel(leaf, part)`` is
a part's label. A tape whose expression has no label stores none, and one whose shapes' parts are not
labeled stores no table of part labels.

An ``ExprIndex`` also numbers the expression's nodes, depth first like the leaves, and gives each node its
parent, its operation and the leaves below it. The leaves below a node are always a contiguous range of
leaf numbers, and so are the surfaces below it (``m_firstSurface`` and ``m_numSurfaces``), so a group of
surfaces (a whole subexpression) can be handed to a kernel as two numbers. ``getLeafOfSurface(surface)``
finds a surface's leaf, and ``pathOf(leaf)`` lists the nodes from the root down to a leaf: the levels a
surface belongs to. The nodes are
those the expression is built of: ``Difference(a, b)`` is an intersection of ``a`` with the complement of
``b``, and translations, rotations and scalings applied one after another are one transform, unless one of
them is labeled.

Regions and materials
---------------------

A union is one solid: a core inside a jacket has no surface of its own. When the core is another material,
the object is a set of **regions** instead, made with ``Overlay(operands, background)``: each operand is a
region, later ones on top of earlier ones where they overlap, and the outside is the background. The regions
tile the object, with no gaps and no overlaps. ``MaterialLabel(object, "name")`` names a region's material, a
volume's, apart from the surface labels above:

.. code-block:: c++

   const auto core   = MaterialLabel(SphereSDF<T>(center, T(0.5)), "copper");
   const auto jacket = MaterialLabel(SphereSDF<T>(center, T(1.0)), "PTFE");

   const Expr<T> cable = Overlay(jacket, core, "air");   // region 0 air, 1 the jacket, 2 the core

   const ExprIndex<T> index(cable);
   index.getMaterialLabelName(index.getMaterialOf(2));   // "copper"

An Overlay may be moved (``Translate``, ``Rotate``, ...), clipped or drilled as the first operand of an
``Intersection`` or a ``Difference`` (the other operands are plain solids that remove space from every
region), and placed in another Overlay, where its regions take its place in the priority order and stay
together as a group (`ExprIndex <doxygen/html/classEBGeometry_1_1ExprIndex.html>`__'s ``getGroup()``); a
material on the inner Overlay is the default of its regions. Anything else that would make it unclear which
region a result belongs to (a union of an Overlay with a solid, an offset or a blend of one, a material on part
of a solid) is rejected when the expression is built. A tape evaluates an Overlay as the whole object, so
``signedDistance()`` is the union of the regions.

A tape that lists ``TapeFeature::Regions`` answers for each region, on the host and on a device:

.. code-block:: c++

   const Tape<T, TapeFeature::Regions> tape(pool, cable);

   tape.getNumRegions();          // 3: the air, the jacket, the core
   tape.region(p);                // the region p is in
   tape.signedDistance(p, 1);     // the jacket's implicit function

``region(p)`` is the last operand whose solid holds the point, or the background; every point is in exactly
one region. ``signedDistance(p, k)`` is negative exactly inside region k, and its zero contour is the
region's whole surface, the interfaces with the other regions included: the function a cut-cell code cuts
for that region. It is a distance bound, exact near a smooth piece of surface. An object with no Overlay is
one region, and a tape without the feature compiles none of this.

A region's surface at a point, ``closestSurface(p, region)``, is reported as any surface is (its leaf, part and
surface label), with the regions on its two sides **at that point**: ``m_minusRegion`` (the region asked about)
and ``m_plusRegion`` (the one across). The sides belong to the point, not to the surface: a sphere centered on
another's surface faces the air on one half and the other sphere on the other.

A solver says which regions it computes in with `Phases <doxygen/html/classEBGeometry_1_1Phases.html>`__, each
phase a set of regions (usually one; ``ExprIndex::getRegionsOfGroup()`` and ``getRegionsOfMaterial()`` select
several). Each surface then has a role for it at a point, from its two sides: a **boundary** (one side solved,
where its surface label gives the boundary condition), an **interface** (two phases meet), **internal** (two
regions of the same phase touch), or none. A phase's own function, ``signedDistance(p, phases, phase)``, is the
one to cut:

.. code-block:: c++

   Phases electrostatics;                         // the air and the jacket; the core is a conductor
   electrostatics.addPhase({0});
   electrostatics.addPhase({1});

   const SurfaceRef<T> s = tape.closestSurface(p, electrostatics, 1);   // on a device too
   electrostatics.role(s);                        // the outer sphere: Interface; the inner one: Boundary

Along a ray
-----------

The point queries have ray counterparts (see :ref:`Chap:Rays`), named after what they find: ``closest...``
asks about a point, ``first...`` about a ray.

=========================================  =========================  ===============================
Finds                                      At a point                 Along a ray
=========================================  =========================  ===============================
the value, or the crossing                 ``signedDistance(p)``      ``firstHit(ray)``
the tape's leaf                            ``closestLeaf(p)``         ``firstHit(ray).m_primitive``
the surface, drilled, its part and label   ``closestSurface(p)``      ``firstSurface(ray)``
=========================================  =========================  ===============================

``firstSurface(ray)`` (a member of ``Tape``, and a free function for any shape, with a segment form like
``firstHit()``'s) returns a
`SurfaceHitT <doxygen/html/structEBGeometry_1_1SurfaceHitT.html>`__: the crossing as ``firstHit()`` reports it
(``m_hit``), and the surface crossed (``m_surface``), drilled into collections down to the member and its part,
with its label. A miss costs what ``firstHit()`` costs; a hit, one more evaluation at the crossing. A kernel
that bounces particles off a geometry reads the surface's label, the mesh face crossed and the sphere of a
cloud from one call.

``HitT::m_primitive`` is the finest thing a shape can name without drilling: for a tape, the leaf crossed,
numbered as ``closestLeaf()`` numbers the leaves; for a mesh distance field, the face (its part); for a BVH
union (not a smooth one), the member crossed (its surface), in the order its constructor was given them; for a
sphere cloud, the sphere; and for any other shape (an analytic shape, a fused composition), 0. The renderer
colors by this number with
``Render::ColorBy::Primitive``, from a table of colors of your own or a built-in palette, and a click in
the viewer picks the part under the cursor by it (see :ref:`Chap:Rendering`).

On the host and on a device
---------------------------

``closestLeaf()``, ``closestSurface()``, ``firstSurface()``, ``getFirstSurface()``, ``isPrimitive()``,
``primitive()`` and ``visitPrimitive()`` can be called on the host and inside a GPU kernel, like
``signedDistance()``. ``closestLeaf()`` evaluates the composition
the way ``signedDistance()`` does, carrying the deciding leaf and its local point along with every
intermediate value, so it costs somewhat more than ``signedDistance()``. The two are compiled
separately: ``signedDistance()`` carries nothing extra, and a program that never asks for the deciding
primitive pays nothing for it.

Only a tape has ``closestLeaf()``. A fused composition is one shape with one surface and names no
parts; to tell parts apart, make them leaves of a tape (each part can be a fused composition itself).

Examples
--------

* :ref:`Chap:ExampleMaterials` bores a cylinder through a mesh, labels the two parts, names each point's
  part with ``closestLeaf()`` and each ray's with ``HitT::m_primitive``, looks its label up in a table
  of materials, and renders the object colored by part.
* :ref:`Chap:ExampleBasicCSG` prints the deciding leaf at a few points of the classic CSG solid, a cube
  intersected with a ball and drilled along three axes.
* The AMReX ``Materials`` integration (see :ref:`Chap:Integrations`) labels every cut cell with its
  deciding leaf and, on the mesh, with the mesh's closest face at the local point, on a GPU when AMReX is
  built for one.
