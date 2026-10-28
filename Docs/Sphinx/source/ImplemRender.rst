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
  ``Camera::fit(box)`` aims it at a box from far enough to see all of it, and ``Camera::fit(box, view)``
  does so from a named direction (``View::Front``, ``Back``, ``Left``, ``Right``, ``Top`` or ``Bottom``;
  the pitch may be 90 degrees, straight along the up axis); ``Camera::lookAt(eye, target)`` places it
  directly. ``m_orthographic`` makes the rays parallel, the view as tall as the perspective view at the
  target, and ``m_nearClip`` starts every camera ray that far from the eye. A shape with an unbounded box
  (a plane, a complement) has nothing to fit; give it a camera, or fit the camera to a box of your own.
* `Settings <doxygen/html/structEBGeometry_1_1Render_1_1Settings.html>`__ is a plain value: the image
  size and rays per pixel, the host threads, shadows, ambient occlusion, the floor (with or without its
  checkerboard, and the checker's size) and the sky (its horizon and zenith colours, or a plain
  background), how to colour (``ColourBy::Normal``, the default, ``ColourBy::Uniform`` or
  ``ColourBy::Primitive``), the colours, the light (its direction, by default in the camera's frame so that
  it turns with the camera and lights the side in view, or in the scene with ``m_lightFromCamera`` false;
  its intensity, the ambient level, and the highlights' strength and shininess), the section plane
  (``m_section``, a point and a normal; the side the normal points to is removed; the cut face is
  coloured as a surface facing along the plane's normal would be, and partly transparent, ``m_cutOpacity``, so that the shape's
  next surface behind it shows through; its contour, a line
  ``m_contourWidth`` pixels wide, and a sheet of opacity ``m_sheetOpacity`` outside the shape within its
  box), and the march (``RayOptions``). ``m_primitiveColours`` gives the first
  ``Settings::PrimitiveColours`` (16) primitive ids colours of their own; an entry left unset, and every
  later id, takes the built-in palette's (:ref:`Chap:ExampleMaterials` colours its bore steel grey).
  ``m_highlightPrimitive`` tints one primitive's surface in ``m_highlightColour``.
  ``Render::toCode(camera, settings)`` writes a camera and
  settings as C++ statements, the fields that differ from their defaults.
* `Renderer <doxygen/html/classEBGeometry_1_1Render_1_1Renderer.html>`__ holds the settings.
  ``render(shape, camera, image)`` runs on the host's threads (``Settings::m_threads``, by default one
  per hardware thread); ``renderOnDevice(deviceShape, camera, image)`` runs the same pixel functor
  through ``GPU::parallelFor()`` on a device view of the shape, as described in :ref:`Chap:RunningOnGPU`.
  Overloads of both take a ``std::atomic<bool>`` that another thread sets to stop the render early (between
  rows on the host, between bands of rows on a device), and a counter of the rows done; a stopped render
  says so in ``Stats::m_complete``. ``pick(shape, camera, column, row)`` returns the hit (``HitT``: the
  primitive, the point, the normal) of the ray through a point of the image, cut as the render cuts it, so
  that it names what a render draws there. Each pixel is `PixelQuery <doxygen/html/structEBGeometry_1_1Render_1_1PixelQuery.html>`__, a
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
shape's ``rebasedView()`` of the copy, and otherwise renders on the host. Drag to orbit, drag with Ctrl
held to pan, drag with the right button to move the light, and scroll to zoom; the left and right arrow
keys make the highlights weaker or stronger (``m_specular``), and with Shift held broader or tighter
(``m_shininess``); S, A and F toggle shadows, ambient occlusion and the floor, Shift and Up or Down take more or fewer rays
per pixel (from 1 x 1 to 8 x 8), P
cycles the colouring (by normal, uniform, by primitive), X cuts the shape with a section plane across x,
then y, then z, through the box's centre (a plane the settings turn on comes first), Ctrl and X turn it
off, N reverses it (removes the other side), scrolling with Ctrl held moves it in fine steps (the up and
down arrow keys in coarse ones), O toggles the orthographic camera, 1, 2 and 3 look from the front, the
side and the top, C prints the camera and settings as C++ (``toCode()``) to paste a view found by hand
into a program, G switches between the GPU and the CPU, H shows or hides the overlay, R resets the camera,
and Esc or Q closes the window. A click (the left button, without dragging) picks the object under the
cursor: a tape's leaf (so that a drilled solid's outer surface and its hole are picked apart), a fused
composition's part, a union's member, or a mesh SDF as a whole (``primitivesAreFaces``), whose face is
named too. It highlights the object (``Settings::AllPrimitives`` highlights a whole shape) and prints its
id, the point and the normal; a click on the background clears it. The overlay, in the top left corner, shows the frame (its size and time,
the hits) and then, in groups (the camera and the light, rendering, the section and its contours,
picking, the window), each group's state and the keys that change it; the keys are also printed, in the same groups,
when the window opens. The window can be resized, and renders at its size in
pixels.
While the camera moves the window
shows a quick preview (one ray per pixel, no shadows, at a resolution it adapts to keep each preview at a
few tens of milliseconds). Once the camera rests it refines on a background thread -- full resolution
without shadows, then the full settings -- and moving the camera again cancels the refinement at once, so
the window always answers.

The window needs SDL2 and is compiled only on request (see :ref:`Sec:ConfigViewer`). Without it, and
whenever no window should open -- the program was given ``--no-view``, there is no display, or SDL
cannot start -- ``view()`` prints one line saying why and returns, so a program that calls it still
runs unattended (``ctest`` passes ``--no-view`` to the examples that call it).

The :ref:`Chap:ExampleMeshSDF` example ends by opening the viewer on the mesh it read.
