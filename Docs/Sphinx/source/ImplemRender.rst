.. _Chap:ImplemRender:

The renderer
============

The renderer is in `EBGeometry_Render.hpp <doxygen/html/EBGeometry__Render_8hpp.html>`__ and the window
in `EBGeometry_Viewer.hpp <doxygen/html/EBGeometry__Viewer_8hpp.html>`__; ``EBGeometry.hpp`` includes
both. It draws any shape through the shape's own ray queries (see :ref:`Chap:ImplemRays`): the shaded
view and the cost of finding hits use ``firstHit()``, the depth view and the cost of finding crossings
``firstCrossing()``. :ref:`Chap:Rendering` describes what it draws, and :ref:`Chap:Viewer` the window.

.. contents:: On this page
   :local:
   :depth: 1

Rendering an image
------------------

The classes are in namespace ``Render``:

.. code-block:: c++

   Render::Settings<T> settings;                       // 960 x 960, 2 x 2 rays per pixel, ...
   settings.m_colorBy = Render::ColorBy::Primitive;

   const Render::Renderer<T> renderer(settings);
   const auto                camera = Render::Camera<T>::fit(shape.computeBoundingVolume());

   Render::Image       image;                          // 0xRRGGBBAA, row by row from the top
   const Render::Stats stats = renderer.render(shape, camera, image);

   Render::writeImage(image, "shape.ppm");             // a binary PPM

* `Camera <doxygen/html/structEBGeometry_1_1Render_1_1Camera.html>`__ orbits a target: yaw about the up axis
  (+y by default), pitch above the horizontal plane, a distance and a vertical field of view.
  ``Camera::fit(box)`` aims it at a box from far enough to see all of it, ``Camera::fit(box, view)`` does so
  from a named direction (``View::Front``, ``Back``, ``Left``, ``Right``, ``Top`` or ``Bottom``), and
  ``Camera::lookAt(eye, target)`` places it directly. ``m_orthographic`` makes the rays parallel, and
  ``m_nearClip`` starts every camera ray that far from the eye. A shape with an unbounded box (a plane, a
  complement) has nothing to fit: give it a camera, or fit one to a box of your own.
* `Renderer <doxygen/html/classEBGeometry_1_1Render_1_1Renderer.html>`__ holds the settings.
  ``render(shape, camera, image)`` renders on the host's threads; ``renderOnDevice(deviceShape, camera, image)``
  renders on a device, from a device view of the shape (see :ref:`Chap:RunningOnGPU`). Overloads of both take a
  ``std::atomic<bool>`` that another thread sets to stop the render, which stops before the next pixel, and a
  counter of the rows done; a stopped render says so in ``Stats::m_complete``, and the pixels it did not reach
  stay black.
* ``pick(shape, camera, column, row)`` returns the hit (``HitT``: the primitive, the point, the normal) of the
  ray through a point of the image, cut as the render cuts it, looking through hidden primitives; ``pickAll()``
  lists every crossing under the point, front to back, hidden ones included (at most ``Renderer::MaxCrossings``).
* `Stats <doxygen/html/structEBGeometry_1_1Render_1_1Stats.html>`__ reports the rays cast, those that hit the
  shape, those that hit it from inside the solid (drawn tinted red), those that were unresolved (drawn as
  misses), and whether the eye is inside the solid; in a cost view, also the cost scale and the frame's rays
  per second (below). The renderer prints nothing.
* ``Render::toCode(camera, settings)`` writes a camera and settings as C++ statements, the fields that differ
  from their defaults: a view found by hand in the window, ready to paste into a program.

The settings
------------

`Settings <doxygen/html/structEBGeometry_1_1Render_1_1Settings.html>`__ is a plain value; every field has a
default.

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Fields
     - Set
   * - ``m_width``, ``m_height``, ``m_supersampling``, ``m_threads``
     - The image size in pixels (960 x 960); rays per pixel along each axis (2, so 2 x 2; from 1 to 15); host
       threads (0: one per hardware thread).
   * - ``m_viewMode``, ``m_xrayOpacity``
     - What is drawn: ``ViewMode::Shaded`` (the default), ``Depth``, ``XRay``, ``CostHit`` or ``CostCrossing``
       (see :ref:`Chap:Rendering`); how opaque each surface is in an x-ray.
   * - ``m_colorBy``, ``m_color``, ``m_primitiveColors``
     - How surfaces are colored: ``ColorBy::Normal`` (the default), ``Uniform`` (in ``m_color``) or
       ``Primitive``, by ``HitT::m_primitive`` (a tape's leaf, a mesh's face); ``m_primitiveColors`` gives the
       first ``Settings::PrimitiveColors`` (16) primitives colors of their own, and the others take the
       built-in palette's.
   * - ``m_shadows``, ``m_occlusion``
     - Soft shadows, and ambient occlusion (darker creases); both on.
   * - ``m_light``, ``m_lightFromCamera``, ``m_lightIntensity``, ``m_ambient``, ``m_specular``, ``m_shininess``
     - The direction toward the light, by default in the camera's frame so that it lights the side in view
       wherever the camera goes (``m_lightFromCamera`` false fixes it in the scene); how strongly it lights;
       the light from the sky; the highlights' brightness and size.
   * - ``m_floor``, ``m_floorColor``, ``m_floorChecker``, ``m_floorCheckerSize``
     - A floor under the shape that catches its shadow, with or without a checkerboard.
   * - ``m_sky``, ``m_skyHorizon``, ``m_skyZenith``, ``m_background``
     - A sky gradient behind the shape, or, with ``m_sky`` false, a plain background.
   * - ``m_section``, ``m_sectionPoint``, ``m_sectionNormal``
     - A section plane, removing the part of the shape on the side its normal points to.
   * - ``m_cutOpacity``, ``m_cutColor``, ``m_contourWidth``, ``m_contourColor``, ``m_sheetOpacity``,
       ``m_sheetColor``, ``m_sheetEdgeColor``
     - How the section is drawn: the cut face (opaque unless ``m_cutOpacity`` is below 1, when the next
       surface behind it shows through), the contour where the plane meets the surface, and a faint sheet
       where the plane lies outside the shape, within its box.
   * - ``m_isoSpacing``, ``m_isoColor``
     - Lines of equal distance on the section plane, inside and outside, this far apart (0: none).
   * - ``m_sectionFill``, ``m_materialColors``, ``m_fillSecondaryColor``, ``m_isoSecondary``,
       ``m_isoSecondaryColor``
     - For a view of regions: the plane filled by each point's material, in the material's color (gray where
       a region has none, or where a domain shown on the plane does not compute), and whether lines are drawn
       there, in another color.
   * - ``m_highlightPrimitive``, ``m_highlightCount``, ``m_highlightColor``
     - Primitives tinted, and outlined, in a highlight color (what a viewer has picked).
   * - ``m_hidden``
     - Up to ``Settings::HiddenPrimitives`` (8) primitives seen through: a ray that meets one goes on.
   * - ``m_volumePrimitive``, ``m_volumeColor``, ``m_volumeOpacity``
     - A part whose inside is filled with a translucent color (``AllPrimitives``: the whole solid).
   * - ``m_insideColor``
     - The color a surface seen from inside the solid is tinted toward.
   * - ``m_regions``, ``m_seenThroughOpacity``
     - For a view of regions (below): what is shown, hidden and seen through, and how opaque a surface seen
       through is.
   * - ``m_recordPrimitives``
     - Record each pixel's primitive in ``Image::m_primitives`` (for picking in a window).
   * - ``m_rays``
     - How rays march where a shape is not traced exactly (``RayOptions``, see :ref:`Chap:SphereTracing`).

Views of regions
----------------

An object of regions (see :ref:`Chap:Regions`) renders as any shape does, as its outer surface. To see its
regions, the surfaces between them, and its domains' surface groups, render a
`RegionView <doxygen/html/classEBGeometry_1_1RegionView.html>`__ of its tape instead:

.. code-block:: c++

   const RegionView<Tape<T>> regions(tape);

   settings.m_regions.m_coloring = RegionColoring::Material;   // or Region, SurfaceGroup, SurfaceLabel, ...
   settings.m_regions.setVisible(1, false);                    // hide region 1 (the air)

   renderer.render(regions, camera, image);

A ``RegionView`` is a shape: it shows the surfaces of the visible regions, the interfaces between them
included, and passes through the rest. What it shows is set by the
`RegionSettings <doxygen/html/structEBGeometry_1_1RegionSettings.html>`__ in ``Settings::m_regions``: the
regions hidden (``setVisible()``, ``showOnly()``), what surfaces are colored and named by (``m_coloring``: by
region, material, surface label, surface, part, interface or surface group), a key shown alone, the
surfaces, regions and surface groups seen through, a domain's surface groups (``m_surfaceGroups``, from
``Domain::surfaceGroups()``), and what is highlighted. Regions are hidden and seen through by bits, so only
the first 64 can be; later ones are always shown.

The cost views
--------------

The cost views (``ViewMode::CostHit`` and ``ViewMode::CostCrossing``) trace one ray per pixel and color each
pixel by what its ray cost, on a log scale over the frame. ``Stats::m_costUnit`` says what a cost counts:
nanoseconds on the host, each pixel's ray timed on the thread that traced it; clock cycles on a device, as
the device counts them. A device traces the 32 threads of a warp together, so each pixel includes waiting
for the slowest ray of its warp (the picture shows blocks of 32 pixels along a row), and the device runs
other warps meanwhile. The two units differ, and so do the scales: the colors compare the pixels of one
frame, never the host with the device. ``Stats::m_costLow`` and ``m_costHigh`` are the costs of the
darkest and the brightest colors; the cheapest and dearest hundredth of the pixels take those colors too, so
that a few interrupted rays do not stretch the scale.

To compare the host with a device, use ``Stats::m_raysPerSecond``, set in the cost views only: the frame's
rays over the time their tracing took. It times the tracing only, not the whole ``render()`` call (not
allocating the device's buffers, copying the image back or coloring it), and a frame with much background
traces many cheap misses. To time your own queries, use the ``Performance/`` probes (see
:ref:`Chap:PerformanceTesting`). A program that draws its own colorbar takes a cost's place on the frame's
scale from ``Render::costPlace(stats, cost)`` (0 at ``m_costLow``, 1 at ``m_costHigh``) and its color from
``Render::costColor(place)``. A view of regions shows the cost of its tape's queries, the whole object's
``firstHit()`` and ``firstCrossing()``, whatever regions are shown.

.. figure:: _static/render_cost_firstCrossing.jpg
   :width: 100%
   :align: center

   The cost of ``firstCrossing()`` per pixel on a mesh, on the host in nanoseconds (left) and on a GPU in
   clock cycles (right), with each frame's rays per second in its colorbar.

Opening the window
------------------

``Render::view(shape, argc, argv)`` opens a window on a shape, from a camera fitted to its box. Overloads
take a camera and settings, a device view of the shape to render on a GPU, and a
`RegionNames <doxygen/html/classEBGeometry_1_1RegionNames.html>`__, the names of a view of regions' regions,
materials, surfaces and domains (see :ref:`Chap:ImplemRegions`). ``Render::view(shape, pool, argc, argv)``
takes instead the pool the shape lives in: in a GPU build with a visible device, it freezes the pool, mirrors
it for as long as the window is open and renders on the device (G switches to the host and back), and
otherwise renders on the host.

The window needs SDL2 and is compiled only on request (see :ref:`Sec:ConfigViewer`). Without it, and
whenever no window should open (the program was given ``--no-view``, there is no display, or SDL cannot
start), ``view()`` prints one line saying why and returns, so a program that calls it still runs
unattended. :ref:`Chap:Viewer` describes the window, and :ref:`Chap:ExampleMeshSDF` ends by opening it on a
mesh.
