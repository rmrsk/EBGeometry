// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

// The viewer's exploded view of regions (Render::ViewerDetail::ExplodedView): its value against the closed form of the
// moved balls, the region and the point moved back, a bore left where a nested region moved out, hidden regions, the
// boxes, and a device render against the host's.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"
#include "TestRenderHelpers.hpp"

#include <cstdint>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>

using namespace EBGeometry;
using namespace EBGeometryTestRender;

namespace {

// Two balls of radius 0.5 at x = -1 (steel, region 1) and x = 1 (copper, region 2).
template <class T>
Expr<T>
twoBalls()
{
  using Vec3 = Vec3T<T>;

  return Overlay(MaterialLabel(SphereSDF<T>(Vec3(T(-1), T(0), T(0)), T(0.5)), "steel"),
                 MaterialLabel(SphereSDF<T>(Vec3(T(1), T(0), T(0)), T(0.5)), "copper"));
}

} // namespace

TEMPLATE_TEST_CASE("ExplodedView: moves each region away from the middle by the factor",
                   "[ExplodedView]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Tape = Tape<T>;

  Pool pool(hostMemoryResource());

  const Expr<T>          expr = twoBalls<T>();
  const Tape             tape(pool, expr);
  const RegionView<Tape> view(tape);

  const auto boxes = Render::ViewerDetail::regionBoxes(expr);

  // The background has no box; each ball's is its own.
  REQUIRE(boxes.size() == 3U);
  CHECK_FALSE(boxes[0].m_some);
  CHECK_THAT(boxes[1].m_lo[0], withinAbsT<T>(T(-1.5), 1e-6));
  CHECK_THAT(boxes[2].m_hi[0], withinAbsT<T>(T(1.5), 1e-6));

  // Factor 1: the centers at -1 and 1 move away from the middle (0) by their distance from it, to -2 and 2.
  const Render::ViewerDetail::ExplodedView<RegionView<Tape>> exploded(view, boxes, T(1));

  const auto ball = [](const Vec3& a_point, const T a_x) {
    return (a_point - Vec3(a_x, T(0), T(0))).length() - T(0.5);
  };

  for (const Vec3& p : {Vec3(T(2), T(0), T(0)),
                        Vec3(T(0), T(0), T(0)),
                        Vec3(T(-2.3), T(0.1), T(0)),
                        Vec3(T(1), T(1), T(0)),
                        Vec3(T(2), T(0.3), T(-0.2))}) {
    const T expected = Math::min(ball(p, T(-2)), ball(p, T(2)));

    CHECK_THAT(exploded.signedDistance(p), withinAbsT<T>(expected, 1e-4));
  }

  // The region at a point, and the point moved back into the view's frame.
  Vec3 back;

  CHECK(exploded.regionAt(Vec3(T(2.2), T(0), T(0)), back) == 2U);
  CHECK_THAT(back[0], withinAbsT<T>(T(1.2), 1e-6));
  CHECK(exploded.regionAt(Vec3(T(-2.2), T(0), T(0)), back) == 1U);
  CHECK_THAT(back[0], withinAbsT<T>(T(-1.2), 1e-6));

  // The box holds the moved balls.
  const auto box = exploded.computeBoundingVolume();

  CHECK_THAT(box.getLowCorner()[0], withinAbsT<T>(T(-2.5), 1e-6));
  CHECK_THAT(box.getHighCorner()[0], withinAbsT<T>(T(2.5), 1e-6));

  // The section's fill: the material of the moved region a point is in.
  CHECK(exploded.sectionKey(Vec3(T(2), T(0), T(0))) == tape.getMaterialOf(2U).value());

  // Factor 0 moves nothing: the regions' surfaces where they are.
  const Render::ViewerDetail::ExplodedView<RegionView<Tape>> still(view, boxes, T(0));

  CHECK_THAT(still.signedDistance(Vec3(T(1), T(0), T(0))), withinAbsT<T>(T(-0.5), 1e-4));
  CHECK_THAT(still.signedDistance(Vec3(T(0), T(0), T(0))), withinAbsT<T>(T(0.5), 1e-4));
}

TEMPLATE_TEST_CASE("ExplodedView: a region moved out of another leaves its bore, and hidden regions are not drawn",
                   "[ExplodedView]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Tape = Tape<T>;

  // A jacket (region 1, radius 1) around a core (region 2, radius 0.4), both at the origin.
  Pool pool(hostMemoryResource());

  const Expr<T>          expr = Overlay(MaterialLabel(SphereSDF<T>(Vec3::zeros(), T(1)), "PTFE"),
                               MaterialLabel(SphereSDF<T>(Vec3::zeros(), T(0.4)), "copper"));
  const Tape             tape(pool, expr);
  const RegionView<Tape> view(tape);

  const auto boxes = Render::ViewerDetail::regionBoxes(expr);

  // The core moved 3 along x, the jacket left where it is.
  const std::vector<Vec3> offsets{Vec3::zeros(), Vec3::zeros(), Vec3(T(3), T(0), T(0))};

  const Render::ViewerDetail::ExplodedView<RegionView<Tape>> exploded(view, boxes, offsets);

  // At the center the jacket's bore: 0.4 from its wall; inside the jacket's wall, negative; the core at x = 3.
  CHECK_THAT(exploded.signedDistance(Vec3::zeros()), withinAbsT<T>(T(0.4), 1e-4));
  CHECK_THAT(exploded.signedDistance(Vec3(T(0.7), T(0), T(0))), withinAbsT<T>(T(-0.3), 1e-4));
  CHECK_THAT(exploded.signedDistance(Vec3(T(3), T(0), T(0))), withinAbsT<T>(T(-0.4), 1e-4));

  // With the core hidden, it is not drawn where it moved to: the jacket's outside is all there is.
  const Render::ViewerDetail::ExplodedView<RegionView<Tape>> jacketOnly(view.withVisible(2U, false), boxes, offsets);

  CHECK_THAT(jacketOnly.signedDistance(Vec3(T(3), T(0), T(0))), withinAbsT<T>(T(2), 1e-4));
}

TEMPLATE_TEST_CASE("ExplodedView: renders, and a device render matches the host's",
                   "[ExplodedView][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Tape = Tape<T>;

  using namespace EBGeometryTestGPU;

  Pool pool(hostMemoryResource());

  const Expr<T>          expr = twoBalls<T>();
  const Tape             tape(pool, expr);
  const RegionView<Tape> view(tape);

  const auto boxes = Render::ViewerDetail::regionBoxes(expr);

  const Render::ViewerDetail::ExplodedView<RegionView<Tape>> exploded(view, boxes, T(1));

  Render::Settings<T> settings;

  settings.m_width         = 40;
  settings.m_height        = 24;
  settings.m_supersampling = 1;
  settings.m_colorBy       = Render::ColorBy::Primitive;

  const auto                camera = Render::Camera<T>::fit(exploded.computeBoundingVolume());
  const Render::Renderer<T> renderer(settings);

  Render::Image       host;
  const Render::Stats hostStats = renderer.render(exploded, camera, host);

  // The moved balls are seen, on both sides of the middle, which the unmoved ones leave empty.
  CHECK(hostStats.m_hits > 0U);

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  pool.freeze();

  const Pool devicePool = Pool::mirror(pool, deviceTestResource());

  // Built on the host, then given the view on the mirror (whose memory the host cannot read).
  const Render::ViewerDetail::ExplodedView<RegionView<Tape>> onDevice = exploded.withView(view.rebasedView(devicePool));

  Render::Image       device;
  const Render::Stats deviceStats = renderer.renderOnDevice(onDevice, camera, device);

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
