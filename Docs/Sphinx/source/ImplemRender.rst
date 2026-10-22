.. _Chap:ImplemRender:

Ray queries and the renderer
============================

The ray queries are in `EBGeometry_Ray.hpp <doxygen/html/EBGeometry__Ray_8hpp.html>`__, the renderer in
`EBGeometry_Render.hpp <doxygen/html/EBGeometry__Render_8hpp.html>`__ and the window in
`EBGeometry_Viewer.hpp <doxygen/html/EBGeometry__Viewer_8hpp.html>`__; ``EBGeometry.hpp`` includes all
three. :ref:`Chap:Rendering` describes the method.

.. contents:: On this page
   :local:
   :depth: 1

.. _Sec:RayQueries:

Ray queries
-----------

Two function templates work on any shape (anything with ``signedDistance()`` and
``computeBoundingVolume()``, see :ref:`Chap:ImplemCSG`), on the host and on a device:

.. code-block:: c++

   const RayT<T> ray{origin, direction};               // direction of unit length
   const HitT<T> hit = firstHit(shape, ray, tMin, tMax);

   if (hit.hit()) {
     // hit.m_t, hit.m_point, hit.m_entering, hit.m_primitive
   }

   const HitStatus crossing = intersects(shape, a, b); // does the segment a-b cross the surface?

* ``firstHit()`` returns the first crossing of the surface in :math:`[t_{min}, t_{max}]`, entering or
  leaving, as a `HitT <doxygen/html/structEBGeometry_1_1HitT.html>`__: its status, where it is, whether
  the ray enters the object there (``m_entering``), and the finest element the shape can name there
  (``m_primitive``: the leaf of a ``Tape``, see below; the closest face of a mesh distance field; 0 for
  any other shape). ``m_exact`` says whether the crossing is exact; for a sphere-traced one it is within
  the tolerance.
* ``intersects()`` says whether the segment between two points crosses the surface. When the value has
  different signs at the two ends it answers at once; otherwise it calls ``firstHit()`` along the
  segment, which finds a segment that goes in and out again.
* The status is a `HitStatus <doxygen/html/namespaceEBGeometry.html>`__: ``Hit``, ``Miss``, or
  ``Unresolved`` when the march ran out of steps (``m_t`` then says how far it got). A query never
  reports a miss it could not establish.
* `RayOptions <doxygen/html/structEBGeometry_1_1RayOptions.html>`__ sets the march: the tolerance, the
  step budget, and the step for values that are not distances, all relative to the shape's scale (its
  box diagonal, or ``m_scale``, which a shape with an unbounded box needs).
* How a shape is marched follows its distance quality: ``Exact`` and ``Bound`` shapes are sphere traced,
  ``NotADistance`` ones stepped and bisected. A ``Tape``'s quality is read at run time.
* A shape with a *ray form* is ray traced instead: `RayForm <doxygen/html/structEBGeometry_1_1RayForm.html>`__
  is a trait a shape type specializes, with ``available = true`` and a
  ``firstHit(shape, ray, tMin, tMax, options)`` of its own, and ``firstHit()`` and ``intersects()`` use it
  whenever it exists (``options`` are for a ray form that sphere traces part of its shape).
* The plane, the sphere, the box, the finite and infinite cylinders and the capsule have ray forms in
  closed form, with outward normals in ``HitT::m_normal``. ``TriMeshSDF`` has one through its BVH: the
  ray follows it, nearest box first, and is tested against the triangles in each leaf it reaches with
  ``TriangleAoSoA::rayHit()`` (Möller--Trumbore), skipping boxes that begin beyond the nearest crossing
  found; its hits name the face and carry the normal interpolated from the vertex normals. The torus,
  the cones, the rounded shapes, Perlin noise, ``MeshSDF`` and ``FlatMeshSDF`` are sphere traced.

`Tape <doxygen/html/classEBGeometry_1_1Tape.html>`__ is ray traced too. Beside the clauses it
evaluates a value with, it compiles a *ray program*: clauses that work on the spans of a ray inside each
node (see :ref:`Chap:Rendering`). Primitives with a ray form, affine maps, complements, unions,
intersections and BVH unions (over ray sub-programs, culled by ray--box tests) are followed exactly;
every other node is compiled into a sub-program of ordinary clauses and sphere traced along the ray
inside its box. An expression with no node a ray can follow, or one whose ray program needs more than
``Tape::RayValueStackSize`` span sets at once or nests deeper than ``MaxCallDepth``, is sphere traced as
a whole. ``tape.firstHit(ray, tMin, tMax)`` is the tape's ray form, and ``tape.intersects(a, b)`` the
segment test on it; ``HitT::m_primitive`` is the leaf at the crossing, ``HitT::m_exact`` is true unless
the ray crossed a sphere-traced node, and ``HitT::m_normal`` is zero (the renderer takes the normal from
the distance). The ray program runs on a device like the rest of the tape.

The tape also has ``primitiveAt(p)``: the leaf of the expression that decides the value at a point,
numbered 0, 1, ... in the order the leaves appear in the expression, depth first, each occurrence
counted (a shape used twice is two leaves). A union or intersection takes the leaf of the operand whose
value it takes, a smooth one that of its more extreme operand, Blur and Mollify that of their centre
tap, and maps, offsets and complements keep their operand's. ``getNumPrimitives()`` counts the leaves.
``signedDistance()`` carries no leaf ids and is as fast as before.

The renderer
------------

The classes are in namespace ``Render``:

.. code-block:: c++

   Render::Settings<T> settings;                       // 960 x 960, 2 x 2 rays per pixel, ...
   settings.m_colourBy = Render::ColourBy::Primitive;

   const Render::Renderer<T> renderer(settings);
   const auto                camera = Render::Camera<T>::fit(shape.computeBoundingVolume());

   Render::Image       image;                          // 0xRRGGBBAA, row by row from the top
   const Render::Stats stats = renderer.render(shape, camera, image);

* `Camera <doxygen/html/structEBGeometry_1_1Render_1_1Camera.html>`__ orbits a target: yaw about the
  up axis (+y by default), pitch above the horizontal plane, a distance and a vertical field of view.
  ``Camera::fit(box)`` aims it at a box from far enough to see all of it; ``Camera::lookAt(eye, target)``
  places it directly. A shape with an unbounded box (a plane, a complement) has nothing to fit; give it a
  camera, or fit the camera to a box of your own.
* `Settings <doxygen/html/structEBGeometry_1_1Render_1_1Settings.html>`__ is a plain value: the image
  size and rays per pixel, the host threads, shadows, ambient occlusion, the floor and the sky, how to
  colour (``ColourBy::Uniform`` or ``ColourBy::Primitive``), the colours, the light's direction, the
  section plane (``m_section``, a point and a normal; the side the normal points to is removed), and the
  march (``RayOptions``).
* `Renderer <doxygen/html/classEBGeometry_1_1Render_1_1Renderer.html>`__ holds the settings.
  ``render(shape, camera, image)`` runs on the host's threads (``Settings::m_threads``, by default one
  per hardware thread); ``renderOnDevice(deviceShape, camera, image)`` runs the same pixel functor
  through ``GPU::transform()`` on a device view of the shape, as described in :ref:`Chap:RunningOnGPU`.
  Overloads of both take a ``std::atomic<bool>`` that another thread sets to stop the render early (between
  rows on the host, between bands of rows on a device), and a counter of the rows done; a stopped render
  says so in ``Stats::m_complete``. Each pixel is `PixelQuery <doxygen/html/structEBGeometry_1_1Render_1_1PixelQuery.html>`__, a
  trivially copyable functor that reads everything it needs about the shape (its box, its scale, where
  the floor goes) from the shape itself.
* Every shape is shaded the same way, through ``firstHit()``: a shadow from one ray towards the light,
  and ambient occlusion from four short rays around the normal. The section plane is the intersection
  of the shape with a half-space along each ray.
* `Stats <doxygen/html/structEBGeometry_1_1Render_1_1Stats.html>`__ reports the rays cast, those that hit
  the shape, those that hit it from inside the solid (drawn tinted red), those that were unresolved
  (drawn as misses), and whether the eye is inside the solid. The renderer prints nothing.

The viewer
----------

``Render::view(shape, argc, argv)`` opens a window on a shape, from a camera fitted to its box;
overloads take a camera and settings, and a device view of the shape to render on a GPU. Drag to orbit,
scroll to zoom; S, A and F toggle shadows, ambient occlusion and the floor, P colours by primitive, X
cuts the shape with a section plane facing the camera (the arrow keys move it), G switches between the
GPU and the CPU, R resets the camera, and Esc or Q closes the window. While the camera moves the window
shows a quick preview (one ray per pixel, no shadows, at a resolution it adapts to keep each preview at a
few tens of milliseconds). Once the camera rests it refines on a background thread -- full resolution
without shadows, then the full settings -- and moving the camera again cancels the refinement at once, so
the window always answers.

The window needs SDL2 and is compiled only on request (see :ref:`Sec:ConfigViewer`). Without it, and
whenever no window should open -- the program was given ``--no-view``, there is no display, or SDL
cannot start -- ``view()`` prints one line saying why and returns, so a program that calls it still
runs unattended (``ctest`` passes ``--no-view`` to the examples that call it).

The :ref:`Chap:ExampleMeshSDF` example ends by opening the viewer on the mesh it read.
