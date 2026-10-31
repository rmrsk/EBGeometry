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
   settings.m_colorBy = Render::ColorBy::Primitive;

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
  checkerboard, and the checker's size) and the sky (its horizon and zenith colors, or a plain
  background), how to color (``ColorBy::Normal``, the default, ``ColorBy::Uniform`` or
  ``ColorBy::Primitive``), the colors, the light (its direction, by default in the camera's frame so that
  it turns with the camera and lights the side in view, or in the scene with ``m_lightFromCamera`` false;
  its intensity, the ambient level, and the highlights' strength and shininess), the section plane
  (``m_section``, a point and a normal; the side the normal points to is removed; ``m_isoSpacing`` draws lines of equal distance on it; the cut face is
  colored as a surface facing along the plane's normal would be, and partly transparent, ``m_cutOpacity``, so that the shape's
  next surface behind it shows through; its contour, a line
  ``m_contourWidth`` pixels wide, and a sheet of opacity ``m_sheetOpacity`` outside the shape within its
  box), and the march (``RayOptions``). ``m_primitiveColors`` gives the first
  ``Settings::PrimitiveColors`` (16) primitive ids colors of their own; an entry left unset, and every
  later id, takes the built-in palette's (:ref:`Chap:ExampleMaterials` colors its bore steel gray).
  ``m_highlightPrimitive`` tints one primitive's surface in ``m_highlightColor``.
  ``Render::writeImage(image, file)`` writes an image as binary PPM. ``Render::toCode(camera, settings)`` writes a camera and
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
* Every shape is shaded the same way, through ``firstHit()`` (shadows, ambient occlusion and the section
  plane's cut included), so the renderer needs nothing of a shape beyond what makes it a shape.
* ``HitT::m_primitive`` colors by primitive: a tape's leaf or a mesh SDF's face; any other shape (a
  fused composition, an analytic shape) is one primitive, 0 (see :ref:`Chap:ImplemRays`).
* `Stats <doxygen/html/structEBGeometry_1_1Render_1_1Stats.html>`__ reports the rays cast, those that hit
  the shape, those that hit it from inside the solid (drawn tinted red), those that were unresolved
  (drawn as misses), and whether the eye is inside the solid. The renderer prints nothing.

The viewer
----------

``Render::view(shape, argc, argv)`` opens a window on a shape, from a camera fitted to its box;
overloads take a camera and settings, and a device view of the shape to render on a GPU.
``Render::view(shape, pool, argc, argv)`` takes instead the pool the shape lives in: in a GPU build with a
visible device it freezes the pool, mirrors it for as long as the window is open and renders through the
shape's ``rebasedView()`` of the copy, and otherwise renders on the host. The pool is frozen before the
window opens, so it stays frozen even if SDL then cannot open one.

While the camera moves the window shows a quick preview (one ray per pixel, no shadows, at a resolution
it adapts to keep each preview at a few tens of milliseconds). Once the camera rests it refines on a
background thread -- full resolution without shadows, then the full settings -- and moving the camera
again cancels the refinement at once, so the window always answers. The window can be resized, and
renders at its size in pixels.

An overlay in the top left corner shows the frame (its size and time, the hits), then each group of keys
below with the state it changes; H hides it. The axes are drawn in the bottom left corner. The keys are
also printed, in the same groups, when the window opens.

A click (the left button, without dragging) picks the object under the cursor: a tape's leaf (so that a
drilled solid's outer surface and its hole are picked apart), a member of a BVH union (not a smooth one), or a mesh SDF as a whole
(``primitivesAreFaces``), whose face is named too; any other shape, a fused composition among them, is
picked as a whole. The object is tinted
gold (``Settings::m_highlightPrimitive``, ``AllPrimitives`` for a whole mesh) and outlined, from the object
each pixel's first ray hit (``Image::m_primitives``, filled when ``Settings::m_recordPrimitives`` asks, as the
viewer does), and its id, point and normal are printed; a click on
the background clears it. With a section plane on, the overlay shows the point under the cursor on the
plane and the shape's value there; Shift and a click on two surfaces measures the distance between them.

.. _Sec:ViewerKeys:

.. list-table:: The viewer's keys
   :header-rows: 1
   :widths: 25 75

   * - Key
     - Action
   * - Drag, Ctrl + drag, wheel
     - Orbit the camera, pan it, zoom.
   * - 1, 2, 3; O; R
     - Look from the front, the side, the top; switch between perspective and orthographic; reset the
       camera.
   * - +, -; double-click
     - A narrower or wider field of view; center the camera on the point under the cursor.
   * - Right drag
     - Move the light.
   * - Left, Right (Shift + Left, Right)
     - Weaker or stronger highlights, ``m_specular`` (broader or tighter, ``m_shininess``).
   * - P
     - Cycle the coloring: by normal, uniform, by primitive.
   * - S, A, F, B
     - Shadows, ambient occlusion, the floor, the sky on or off.
   * - Shift + Up, Down
     - More or fewer rays per pixel, from 1 x 1 to 8 x 8.
   * - G
     - Render on the GPU or the CPU (with a device view).
   * - X; Ctrl + X; N
     - Cut with a section plane across x, then y, then z, through the box's center (a plane the settings
       turn on comes first); remove the cut; reverse it (remove the other side).
   * - Ctrl + wheel; Up, Down
     - Move the section plane in fine steps; in coarse ones.
   * - I
     - Lines of equal distance on the section plane: off, as the settings set them (if they do), every 1/20
       of the box's diagonal, every 1/50.
   * - Click; Shift + click
     - Pick the object under the cursor; mark a point to measure from or to (the second shows the distance
       and a line between them). On the background, either clears.
   * - C; W
     - Print the camera and settings as C++ (``toCode()``), to paste a view found by hand into a program;
       save the full frame as a PPM image (``writeImage()``) in the working directory.
   * - H; Esc, Q
     - Show or hide the overlay and the axes; close the window.

The window needs SDL2 and is compiled only on request (see :ref:`Sec:ConfigViewer`). Without it, and
whenever no window should open -- the program was given ``--no-view``, there is no display, or SDL
cannot start -- ``view()`` prints one line saying why and returns, so a program that calls it still
runs unattended (``ctest`` passes ``--no-view`` to the examples that call it).

The :ref:`Chap:ExampleMeshSDF` example ends by opening the viewer on the mesh it read.
