// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Device renders against host renders: a tape in the shaded, orthographic, near-clipped, depth, x-ray and
// hidden-primitive views, the cost view, and a canceled device render.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"
#include "TestRenderHelpers.hpp"

#include <atomic>
#include <cstdint>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>

using namespace EBGeometry;
using namespace EBGeometryTestRender;

TEMPLATE_TEST_CASE("Render: the device render matches the host render", "[Render][gpu]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  Pool pool(hostMemoryResource());

  const Tape<T> tape(
    pool,
    Difference(Intersection(BoxSDF<T>(-Vec3::ones(), Vec3::ones()), SphereSDF<T>(Vec3::zeros(), T(1.35))),
               InfiniteCylinderSDF<T>(Vec3::zeros(), T(0.5), 1)));

  pool.freeze();

  const Pool    devicePool = Pool::mirror(pool, deviceTestResource());
  const Tape<T> view       = tape.rebasedView(devicePool);

  Render::Settings<T> settings;

  settings.m_width         = 48;
  settings.m_height        = 32;
  settings.m_supersampling = 2;
  settings.m_colorBy       = Render::ColorBy::Primitive;

  const Render::Renderer<T> renderer(settings);
  const auto                camera = Render::Camera<T>::fit(tape.computeBoundingVolume());

  Render::Image host;
  Render::Image device;

  const Render::Stats hostStats   = renderer.render(tape, camera, host);
  const Render::Stats deviceStats = renderer.renderOnDevice(view, camera, device);

  REQUIRE(host.m_pixels.size() == device.m_pixels.size());

  // A device may round differently (fused multiply-adds), so a channel may differ by a little.
  size_t different = 0;

  for (size_t i = 0; i < host.m_pixels.size(); i++) {
    const Channels a = channels(host.m_pixels[i]);
    const Channels b = channels(device.m_pixels[i]);

    if (std::abs(a.r - b.r) > 3 || std::abs(a.g - b.g) > 3 || std::abs(a.b - b.b) > 3) {
      different++;
    }
  }

  CHECK(different <= host.m_pixels.size() / 100);
  CHECK(hostStats.m_rays == deviceStats.m_rays);
  CHECK(std::abs(double(hostStats.m_hits) - double(deviceStats.m_hits)) <= double(hostStats.m_rays) / 100);
}

TEMPLATE_TEST_CASE("Render: the device render of a view of regions matches the host's: cut, filled, translucent",
                   "[Render][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Tape = Tape<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  Pool pool(hostMemoryResource());

  // The cable in a box of air, cut along its axis, the plane filled by material with lines of equal distance, the
  // jacket translucent and the core's surfaces in a translucent surface group: the region view's paths on a device.
  const Tape tape(
    pool,
    Overlay(MaterialLabel(BoxSDF<T>(Vec3(T(-1.5), T(-1.5), T(-3)), Vec3(T(1.5), T(1.5), T(3))), "air"),
            MaterialLabel(CylinderSDF<T>(Vec3(T(0), T(0), T(-2)), Vec3(T(0), T(0), T(2)), T(1)), "PTFE"),
            MaterialLabel(CylinderSDF<T>(Vec3(T(0), T(0), T(-2.5)), Vec3(T(0), T(0), T(2.5)), T(0.4)), "copper")));

  // The core's surfaces as a surface group, built in the pool before it is mirrored.
  DomainSetup groups;

  groups.add(groups.addSurfaceGroup(), SurfaceSelector::region(3));

  const SurfaceGroups built = groups.buildSurfaceGroups(pool);

  pool.freeze();

  const Pool             devicePool = Pool::mirror(pool, deviceTestResource());
  const RegionView<Tape> host(tape);
  const RegionView<Tape> device = host.rebasedView(devicePool);

  Render::Settings<T> settings;

  settings.m_width         = 48;
  settings.m_height        = 32;
  settings.m_supersampling = 1;
  settings.m_colorBy       = Render::ColorBy::Primitive;
  settings.m_section       = true;
  settings.m_sectionNormal = Vec3(T(1), T(0), T(0));
  settings.m_isoSpacing    = T(0.2);

  settings.m_regions.m_coloring = RegionColoring::Material;
  settings.m_regions.setVisible(1, false);
  settings.m_regions.setRegionSeenThrough(2, true);
  settings.m_regions.m_surfaceGroups = built;
  settings.m_regions.setSurfaceGroupSeenThrough(0, true);

  // On the device, the settings read the surface groups from the mirror, as the shape does.
  Render::Settings<T> deviceSettings = settings;

  deviceSettings.m_regions.m_surfaceGroups = built.rebasedView(devicePool);

  const Render::Renderer<T> renderer(settings);
  const Render::Renderer<T> deviceRenderer(deviceSettings);
  const auto                camera = Render::Camera<T>::fit(host.computeBoundingVolume());

  Render::Image onHost;
  Render::Image onDevice;

  (void)renderer.render(host, camera, onHost);
  (void)deviceRenderer.renderOnDevice(device, camera, onDevice);

  REQUIRE(onHost.m_pixels.size() == onDevice.m_pixels.size());

  size_t different = 0;

  for (size_t i = 0; i < onHost.m_pixels.size(); i++) {
    const Channels a = channels(onHost.m_pixels[i]);
    const Channels b = channels(onDevice.m_pixels[i]);

    if (std::abs(a.r - b.r) > 3 || std::abs(a.g - b.g) > 3 || std::abs(a.b - b.b) > 3) {
      different++;
    }
  }

  CHECK(different <= onHost.m_pixels.size() / 50);
}

TEMPLATE_TEST_CASE("Render: a device render's pixels skip their work once the cancel flag is set, say so, and count "
                   "each row once",
                   "[Render]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const SphereSDF<T> sphere(Vec3T<T>::zeros(), T(1));

  const Render::Settings<T> settings = plain<T>(8);
  const auto                camera   = Render::Camera<T>::fit(sphere.computeBoundingVolume());

  const Render::PixelQuery<T, SphereSDF<T>> query{sphere, camera, settings};

  std::vector<Render::RenderDetail::StoredPixel> results(64);
  std::vector<uint32_t>                          primitives(64, 0U);

  uint32_t          cancel   = 0U;
  uint32_t          skipped  = 0U;
  uint32_t          rows     = 0U;
  volatile uint32_t progress = 0U;

  const Render::RenderDetail::FrameOfPixels<Render::PixelQuery<T, SphereSDF<T>>> frame{
    query, results.data(), primitives.data(), &cancel, &skipped, &rows, &progress, 0U, 8U};

  // Not canceled: every pixel is shaded as the plain query shades it, and each row counted once, by its last pixel.
  for (size_t i = 0; i < 64; i++) {
    frame(i);
  }

  CHECK(rows == 8U);
  CHECK(progress == 8U);
  CHECK(skipped == 0U);

  for (uint32_t i = 0; i < 64; i++) {
    CHECK(results[i].m_rgba == query(i).m_rgba);
    CHECK(primitives[i] == query(i).m_primitive);
  }

  // Canceled after a row's last pixel but before its others: the skipped pixels say the render stopped, though
  // the row was counted.
  cancel = 1U;

  frame(0);

  CHECK(skipped == 1U);
  CHECK(results[0].m_rgba == 0U);

  // Canceled: no pixel is shaded or counted, and each is left black with no object.
  for (size_t i = 0; i < 64; i++) {
    frame(i);
  }

  CHECK(rows == 8U);

  for (size_t i = 0; i < 64; i++) {
    CHECK(results[i].m_rgba == 0U);
    CHECK(results[i].m_hits == 0U);
    CHECK(primitives[i] == 0xFFFFFFFFU);
  }
}

TEMPLATE_TEST_CASE("Render: the device render matches the host's with an orthographic camera, a near clip, a "
                   "primitive color table and the surface settings",
                   "[Render][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  Pool pool(hostMemoryResource());

  const Tape<T> tape(pool,
                     Union(Difference(BoxSDF<T>(-Vec3::ones(), Vec3::ones()), SphereSDF<T>(Vec3::zeros(), T(1.2))),
                           SphereSDF<T>(Vec3(T(0), T(0), T(0)), T(0.5))));

  pool.freeze();

  const Pool    devicePool = Pool::mirror(pool, deviceTestResource());
  const Tape<T> view       = tape.rebasedView(devicePool);

  Render::Settings<T> settings;

  settings.m_width              = 40;
  settings.m_height             = 32;
  settings.m_supersampling      = 2;
  settings.m_colorBy            = Render::ColorBy::Primitive;
  settings.m_primitiveColors[1] = Vec3(T(0.2), T(0.3), T(0.9));
  settings.m_shininess          = T(12);
  settings.m_specular           = T(0.6);
  settings.m_ambient            = T(0.2);
  settings.m_floorCheckerSize   = T(0.5);
  settings.m_section            = true;
  settings.m_sectionPoint       = Vec3(T(0.1), T(0), T(0));
  settings.m_sectionNormal      = Vec3(T(1), T(0.2), T(0));
  settings.m_sheetOpacity       = T(0.3);
  settings.m_recordPrimitives   = true;
  settings.m_isoSpacing         = T(0.2);

  for (const bool orthographic : {false, true}) {
    auto camera = Render::Camera<T>::fit(tape.computeBoundingVolume(), Render::View::Top);

    camera.m_orthographic = orthographic;
    camera.m_nearClip     = T(0.5) * camera.m_distance;

    for (const Render::ColorBy colorBy : {Render::ColorBy::Primitive, Render::ColorBy::Normal}) {
      settings.m_colorBy = colorBy;

      const Render::Renderer<T> renderer(settings);

      Render::Image host;
      Render::Image device;

      const Render::Stats hostStats   = renderer.render(tape, camera, host);
      const Render::Stats deviceStats = renderer.renderOnDevice(view, camera, device);

      REQUIRE(host.m_pixels.size() == device.m_pixels.size());

      size_t different = 0;

      for (size_t i = 0; i < host.m_pixels.size(); i++) {
        const Channels a = channels(host.m_pixels[i]);
        const Channels b = channels(device.m_pixels[i]);

        if (std::abs(a.r - b.r) > 3 || std::abs(a.g - b.g) > 3 || std::abs(a.b - b.b) > 3) {
          different++;
        }
      }

      CHECK(different <= host.m_pixels.size() / 100);
      CHECK(hostStats.m_hits > 0U);

      // The same objects under the same pixels, but for rays that graze a surface.
      size_t otherObject = 0;

      for (size_t i = 0; i < host.m_primitives.size(); i++) {
        otherObject += (host.m_primitives[i] != device.m_primitives[i]) ? 1U : 0U;
      }

      CHECK(otherObject <= host.m_primitives.size() / 100);
      CHECK(std::abs(double(hostStats.m_hits) - double(deviceStats.m_hits)) <= double(hostStats.m_rays) / 100);
    }
  }
}

TEMPLATE_TEST_CASE("Render: the device render matches the host's in the depth and x-ray views, and with a hidden "
                   "primitive and a filled part",
                   "[Render][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  Pool pool(hostMemoryResource());

  const Tape<T> tape(
    pool, Difference(BoxSDF<T>(-Vec3::ones(), Vec3::ones()), SphereSDF<T>(Vec3(T(0.2), T(0), T(0)), T(0.6))));

  pool.freeze();

  const Pool    devicePool = Pool::mirror(pool, deviceTestResource());
  const Tape<T> view       = tape.rebasedView(devicePool);

  const auto camera = Render::Camera<T>::fit(tape.computeBoundingVolume());

  Render::Settings<T> base;

  base.m_width         = 40;
  base.m_height        = 32;
  base.m_supersampling = 1;

  std::vector<Render::Settings<T>> cases(4, base);

  cases[0].m_viewMode        = Render::ViewMode::Depth;
  cases[1].m_viewMode        = Render::ViewMode::XRay;
  cases[2].m_hidden[0]       = 0U;
  cases[3].m_volumePrimitive = 1U;

  for (const Render::Settings<T>& settings : cases) {
    const Render::Renderer<T> renderer(settings);

    Render::Image host;
    Render::Image device;

    const Render::Stats hostStats   = renderer.render(tape, camera, host);
    const Render::Stats deviceStats = renderer.renderOnDevice(view, camera, device);

    REQUIRE(host.m_pixels.size() == device.m_pixels.size());

    size_t different = 0;

    for (size_t i = 0; i < host.m_pixels.size(); i++) {
      const Channels a = channels(host.m_pixels[i]);
      const Channels b = channels(device.m_pixels[i]);

      if (std::abs(a.r - b.r) > 3 || std::abs(a.g - b.g) > 3 || std::abs(a.b - b.b) > 3) {
        different++;
      }
    }

    CHECK(different <= host.m_pixels.size() / 50);
    CHECK(std::abs(double(hostStats.m_hits) - double(deviceStats.m_hits)) <= double(hostStats.m_rays) / 50);
  }

  // A cost view renders on the device too, in clock cycles.
  Render::Settings<T> cost = base;

  cost.m_viewMode = Render::ViewMode::CostCrossing;

  Render::Image image;

  const Render::Stats stats = Render::Renderer<T>(cost).renderOnDevice(view, camera, image);

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  CHECK(stats.m_costUnit == Render::CostUnit::Cycles);
#else
  CHECK(stats.m_costUnit == Render::CostUnit::Nanoseconds);
#endif
  CHECK(stats.m_costLow > 0.0);
  CHECK(stats.m_costHigh >= stats.m_costLow);
  CHECK(stats.m_raysPerSecond > 0.0);
}
