.. _Chap:ImplemRender:

The renderer and the viewer
===========================

The renderer is in `EBGeometry_Render.hpp <doxygen/html/EBGeometry__Render_8hpp.html>`__ and the window
in `EBGeometry_Viewer.hpp <doxygen/html/EBGeometry__Viewer_8hpp.html>`__; ``EBGeometry.hpp`` includes
both. Every ray it casts is a ``firstHit()`` on the shape (see :ref:`Chap:ImplemRays`), so it renders any
shape, exactly where the shape has a ray form. :ref:`Chap:Rendering` describes the method.

.. contents:: On this page
   :local:
   :depth: 1

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
  through ``GPU::parallelFor()`` on a device view of the shape, as described in :ref:`Chap:RunningOnGPU`.
  Overloads of both take a ``std::atomic<bool>`` that another thread sets to stop the render early (between
  rows on the host, between bands of rows on a device), and a counter of the rows done; a stopped render
  says so in ``Stats::m_complete``. Each pixel is `PixelQuery <doxygen/html/structEBGeometry_1_1Render_1_1PixelQuery.html>`__, a
  trivially copyable functor that reads everything it needs about the shape (its box, its scale, where
  the floor goes) from the shape itself.
* Every shape is shaded the same way, through ``firstHit()``: a shadow from one ray towards the light,
  and ambient occlusion from four short rays around the normal. The section plane is the intersection
  of the shape with a half-space along each ray: the shape is traced over the part of the ray on the
  kept side, and the plane is the crossing where the ray reaches it inside the shape.
* ``HitT::m_primitive`` colours by primitive: a tape's leaf, a fused composition's leaf, a mesh's face
  (see :ref:`Chap:ImplemRays`).
* `Stats <doxygen/html/structEBGeometry_1_1Render_1_1Stats.html>`__ reports the rays cast, those that hit
  the shape, those that hit it from inside the solid (drawn tinted red), those that were unresolved
  (drawn as misses), and whether the eye is inside the solid. The renderer prints nothing.

The viewer
----------

``Render::view(shape, argc, argv)`` opens a window on a shape, from a camera fitted to its box;
overloads take a camera and settings, and a device view of the shape to render on a GPU.
``Render::view(shape, pool, argc, argv)`` takes instead the pool the shape lives in: in a GPU build with a
visible device it freezes the pool, mirrors it for as long as the window is open and renders through the
shape's ``rebasedView()`` of the copy, and otherwise renders on the host. Drag to orbit,
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
