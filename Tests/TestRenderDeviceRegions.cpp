// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

// Device renders of a view of regions against the host's: cut, filled and translucent. Apart from the device renders
// of a plain tape (TestRenderDevice.cpp), because each shape type compiles two large render kernels per precision:
// in two files they compile side by side.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"
#include "TestRenderHelpers.hpp"

#include <cstdint>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>

using namespace EBGeometry;
using namespace EBGeometryTestRender;

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

  // The pixels that differ by more than a few levels in a channel.
  const auto differing = [&](const Render::Renderer<T>& a_host, const Render::Renderer<T>& a_device) {
    Render::Image onHost;
    Render::Image onDevice;

    (void)a_host.render(host, camera, onHost);
    (void)a_device.renderOnDevice(device, camera, onDevice);

    REQUIRE(onHost.m_pixels.size() == onDevice.m_pixels.size());

    size_t different = 0;

    for (size_t i = 0; i < onHost.m_pixels.size(); i++) {
      const Channels a = channels(onHost.m_pixels[i]);
      const Channels b = channels(onDevice.m_pixels[i]);

      if (std::abs(a.r - b.r) > 3 || std::abs(a.g - b.g) > 3 || std::abs(a.b - b.b) > 3) {
        different++;
      }
    }

    return different;
  };

  const size_t pixels = size_t(settings.m_width) * settings.m_height;

  CHECK(differing(renderer, deviceRenderer) <= pixels / 50);

  // The removed half ghosted, and the plane's fill and the cut face see-through: a second trace and shading of the
  // uncut view, and the plane drawn behind the ghost.
  for (Render::Settings<T>* s : {&settings, &deviceSettings}) {
    s->m_ghostOpacity = T(0.3);
    s->m_sectionFill  = T(0.5);
    s->m_cutOpacity   = T(0.5);
  }

  CHECK(differing(Render::Renderer<T>(settings), Render::Renderer<T>(deviceSettings)) <= pixels / 50);
}
