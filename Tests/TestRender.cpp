// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"
#include "TestTapeUtils.hpp"

#include <cmath>
#include <cstdint>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>

using namespace EBGeometry;

namespace {

// Settings for small, exact-to-check images: one ray per pixel, no floor, and a plain background.
template <class T>
Render::Settings<T>
plain(const uint32_t a_size)
{
  Render::Settings<T> settings;

  settings.m_width         = a_size;
  settings.m_height        = a_size;
  settings.m_supersampling = 1;
  settings.m_threads       = 3;
  settings.m_floor         = false;
  settings.m_sky           = false;

  return settings;
}

// The colour channels of a pixel.
struct Channels
{
  int r;
  int g;
  int b;
};

Channels
channels(const uint32_t a_rgba)
{
  return Channels{int((a_rgba >> 24) & 0xFFU), int((a_rgba >> 16) & 0xFFU), int((a_rgba >> 8) & 0xFFU)};
}

// The direction of the ray through the centre of pixel (column, row), as PixelQuery computes it.
template <class T>
Vec3T<T>
pixelRay(const Render::Camera<T>& a_camera, const uint32_t a_size, const uint32_t a_column, const uint32_t a_row)
{
  Vec3T<T> forward;
  Vec3T<T> right;
  Vec3T<T> up;

  a_camera.frame(forward, right, up);

  const T tanHalf = std::tan(T(0.5) * a_camera.m_fieldOfView * pi<T> / T(180));
  const T n       = T(a_size);
  const T x       = (T(2) * (T(a_column) + T(0.5)) - n) / n * tanHalf;
  const T y       = (n - T(2) * (T(a_row) + T(0.5))) / n * tanHalf;

  const Vec3T<T> d = forward + x * right + y * up;

  return d / d.length();
}

} // namespace

TEMPLATE_TEST_CASE("Render: the camera fits a box and places its eye where asked",
                   "[Render]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const BoundingVolumes::AABBT<T> box(Vec3(T(-1), T(0), T(2)), Vec3(T(3), T(1), T(4)));

  const auto camera = Render::Camera<T>::fit(box);

  CHECK((camera.m_target - Vec3(T(1), T(0.5), T(3))).length() < T(tightMargin<T>()));

  // The sphere around the box fills at most the view: its angular radius is below half the field of view.
  const T radius = T(0.5) * (box.getHighCorner() - box.getLowCorner()).length();

  CHECK(std::asin(radius / camera.m_distance) < T(0.5) * camera.m_fieldOfView * pi<T> / T(180));
  CHECK(std::abs((camera.eye() - camera.m_target).length() - camera.m_distance) <
        T(looseMargin<T>()) * camera.m_distance);

  // With +y up, yaw 0 looks from +z and yaw 90 from +x.
  Render::Camera<T> side;

  side.m_yaw   = T(0);
  side.m_pitch = T(0);

  CHECK((side.eye() - Vec3(T(0), T(0), side.m_distance)).length() < T(looseMargin<T>()));

  side.m_yaw = T(90);

  CHECK((side.eye() - Vec3(side.m_distance, T(0), T(0))).length() < T(looseMargin<T>()));

  // lookAt() puts the eye where it is told, for another up axis too.
  for (const Vec3& up : {Vec3(0, 1, 0), Vec3(0, 0, 1)}) {
    const Vec3 eye(T(2), T(-3), T(1.5));
    const Vec3 target(T(0.5), T(0.2), T(-0.1));

    const auto placed = Render::Camera<T>::lookAt(eye, target, up);

    CHECK((placed.eye() - eye).length() < T(looseMargin<T>()) * T(10));

    Vec3 forward;
    Vec3 right;
    Vec3 screenUp;

    placed.frame(forward, right, screenUp);

    CHECK((forward - (target - eye) / (target - eye).length()).length() < T(looseMargin<T>()) * T(10));
    CHECK(std::abs(dot(right, up)) < T(looseMargin<T>()));
    CHECK(dot(screenUp, up) > T(0));
  }
}

TEMPLATE_TEST_CASE("Render: a sphere covers the pixels whose rays meet it", "[Render]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Vec3         center(T(0.2), T(-0.1), T(0));
  const T            radius = T(0.7);
  const SphereSDF<T> sphere(center, radius);

  constexpr uint32_t size = 40;

  const Render::Renderer<T> renderer(plain<T>(size));

  auto camera = Render::Camera<T>::fit(sphere.computeBoundingVolume());

  Render::Image image;

  const Render::Stats stats = renderer.render(sphere, camera, image);

  REQUIRE(image.m_width == size);
  REQUIRE(image.m_height == size);
  REQUIRE(image.m_pixels.size() == size_t(size * size));

  const uint32_t background = image.at(0, 0);

  size_t covered = 0;

  for (uint32_t row = 0; row < size; row++) {
    for (uint32_t column = 0; column < size; column++) {
      const Vec3 d  = pixelRay(camera, size, column, row);
      const Vec3 oc = camera.eye() - center;

      // The ray's closest approach to the centre; rays close to the rim can go either way.
      const T b       = dot(oc, d);
      const T closest = std::sqrt(Math::max(T(0), dot(oc, oc) - b * b));

      if (closest < T(0.95) * radius) {
        CHECK(image.at(column, row) != background);

        covered++;
      }
      else if (closest > T(1.05) * radius) {
        CHECK(image.at(column, row) == background);
      }
    }
  }

  CHECK(covered > 100U);
  CHECK(stats.m_rays == uint64_t(size * size));
  CHECK(stats.m_hits >= covered);
  CHECK(stats.m_insideHits == 0U);
  CHECK(stats.m_unresolved == 0U);
  CHECK_FALSE(stats.m_cameraInside);
}

TEMPLATE_TEST_CASE("Render: a hole seen from inside the solid is tinted, and the camera is reported inside",
                   "[Render]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const SphereSDF<T>           ball(Vec3::zeros(), T(1));
  const TestUtils::Compiled<T> hole(Complement(ball));

  // The complement's box is unbounded, so the camera is fitted to the ball's.
  const auto camera = Render::Camera<T>::fit(ball.computeBoundingVolume());

  constexpr uint32_t size = 24;

  Render::Settings<T> settings = plain<T>(size);

  settings.m_shadows   = false;
  settings.m_occlusion = false;

  const Render::Renderer<T> renderer(settings);

  Render::Image inside;
  Render::Image outside;

  const Render::Stats holeStats = renderer.render(hole.tape(), camera, inside);
  const Render::Stats ballStats = renderer.render(ball, camera, outside);

  CHECK(holeStats.m_cameraInside);
  CHECK(holeStats.m_hits > 0U);
  CHECK(holeStats.m_insideHits == holeStats.m_hits);

  CHECK_FALSE(ballStats.m_cameraInside);
  CHECK(ballStats.m_insideHits == 0U);

  // The centre pixel sees the ball's near side either way; from inside the solid it is red.
  const Channels tinted = channels(inside.at(size / 2, size / 2));
  const Channels plainC = channels(outside.at(size / 2, size / 2));

  CHECK(tinted.r > 2 * tinted.g);
  CHECK(tinted.r > 2 * tinted.b);
  CHECK(plainC.r < 2 * plainC.g);
}

TEMPLATE_TEST_CASE("Render: colouring by primitive tells the leaves apart", "[Render]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const TestUtils::Compiled<T> pair(
    Union(SphereSDF<T>(Vec3(T(-1), T(0), T(0)), T(0.6)), SphereSDF<T>(Vec3(T(1), T(0), T(0)), T(0.6))));

  // Looking along -z with the light behind the camera, the scene is mirror symmetric in x.
  Render::Camera<T> camera = Render::Camera<T>::fit(pair.tape().computeBoundingVolume());

  camera.m_yaw   = T(0);
  camera.m_pitch = T(0);

  constexpr uint32_t size = 32;

  Render::Settings<T> settings = plain<T>(size);

  settings.m_light     = Vec3(T(0), T(0), T(1));
  settings.m_shadows   = false;
  settings.m_occlusion = false;

  Render::Image uniform;
  Render::Image byPrimitive;

  Render::Renderer<T> renderer(settings);

  (void)renderer.render(pair.tape(), camera, uniform);

  renderer.settings().m_colourBy = Render::ColourBy::Primitive;

  (void)renderer.render(pair.tape(), camera, byPrimitive);

  // The pixels in the middle row whose rays pass closest to each sphere's centre, in the plane z = 0.
  uint32_t left  = 0;
  uint32_t right = 0;
  T        bestL = Math::Limits<T>::infinity();
  T        bestR = Math::Limits<T>::infinity();

  for (uint32_t column = 0; column < size; column++) {
    const Vec3 d = pixelRay(camera, size, column, size / 2);
    const T    x = camera.eye()[0] - camera.eye()[2] / d[2] * d[0];

    if (std::abs(x + T(1)) < bestL) {
      bestL = std::abs(x + T(1));
      left  = column;
    }
    if (std::abs(x - T(1)) < bestR) {
      bestR = std::abs(x - T(1));
      right = column;
    }
  }

  REQUIRE(left + right == size - 1U);

  const Channels a = channels(uniform.at(left, size / 2));
  const Channels b = channels(uniform.at(right, size / 2));

  CHECK(std::abs(a.r - b.r) <= 2);
  CHECK(std::abs(a.g - b.g) <= 2);
  CHECK(std::abs(a.b - b.b) <= 2);

  const Channels c = channels(byPrimitive.at(left, size / 2));
  const Channels e = channels(byPrimitive.at(right, size / 2));

  CHECK(std::abs(c.r - e.r) + std::abs(c.g - e.g) + std::abs(c.b - e.b) > 30);
}

TEMPLATE_TEST_CASE("Render: the section plane removes the shape on one side and shows the cut",
                   "[Render]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const SphereSDF<T> sphere(Vec3::zeros(), T(1));

  Render::Camera<T> camera = Render::Camera<T>::fit(sphere.computeBoundingVolume());

  camera.m_yaw   = T(0);
  camera.m_pitch = T(0);

  constexpr uint32_t size = 32;

  Render::Settings<T> settings = plain<T>(size);

  Render::Image whole;
  Render::Image halved;
  Render::Image opened;

  Render::Renderer<T> renderer(settings);

  (void)renderer.render(sphere, camera, whole);

  // Cut away x > 0: the right half of the image is background.
  renderer.settings().m_section       = true;
  renderer.settings().m_sectionPoint  = Vec3::zeros();
  renderer.settings().m_sectionNormal = Vec3(T(1), T(0), T(0));

  (void)renderer.render(sphere, camera, halved);

  const uint32_t background = whole.at(0, 0);

  CHECK(halved.at(size / 2 + size / 8, size / 2) == background);
  CHECK(halved.at(size / 2 - size / 8, size / 2) != background);
  CHECK(whole.at(size / 2 + size / 8, size / 2) != background);

  // Cut away the half towards the camera: it sees the flat cut face, in the cut colour.
  renderer.settings().m_sectionNormal = Vec3(T(0), T(0), T(1));
  renderer.settings().m_shadows       = false;
  renderer.settings().m_occlusion     = false;

  (void)renderer.render(sphere, camera, opened);

  const Channels cut = channels(opened.at(size / 2, size / 2));

  CHECK(cut.r > cut.b + 40);
  CHECK(cut.g > cut.b + 30);
  CHECK(opened.at(size / 2, size / 2) != whole.at(size / 2, size / 2));
  CHECK(opened.at(size / 2 - 3, size / 2 + 2) != background);
}

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
  settings.m_colourBy      = Render::ColourBy::Primitive;

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

TEST_CASE("Render: rejects an image without pixels, bad supersampling, a degenerate camera and an unbounded fit",
          "[Render]")
{
  using T    = double;
  using Vec3 = Vec3T<T>;

  const SphereSDF<T> sphere(Vec3::zeros(), T(1));

  REQUIRE(abortsWith(
    [&sphere] {
      Render::Settings<T> settings;

      settings.m_width = 0;

      Render::Image image;

      (void)Render::Renderer<T>(settings).render(sphere, Render::Camera<T>(), image);
    },
    "Render::Renderer: the image must have pixels (0 x 960)"));

  REQUIRE(abortsWith(
    [&sphere] {
      Render::Settings<T> settings;

      settings.m_supersampling = 0;

      Render::Image image;

      (void)Render::Renderer<T>(settings).render(sphere, Render::Camera<T>(), image);
    },
    "Render::Renderer: supersampling must be from 1 to 15 (0)"));

  REQUIRE(abortsWith(
    [&sphere] {
      Render::Camera<T> camera;

      camera.m_pitch = T(90);

      Render::Image image;

      (void)Render::Renderer<T>().render(sphere, camera, image);
    },
    "Render::Renderer: the camera pitch must be between -90 and 90 degrees (90)"));

  REQUIRE(
    abortsWith([] { (void)Render::Camera<T>::fit(PlaneSDF<T>(Vec3::zeros(), Vec3(0, 1, 0)).computeBoundingVolume()); },
               "Render::Camera: fit() needs a bounded, non-empty box"));

  REQUIRE(abortsWith([] { (void)Render::Camera<T>::lookAt(Vec3::ones(), Vec3::ones()); },
                     "Render::Camera: the eye must not be at the target"));
}

TEMPLATE_TEST_CASE("Render: faces that lie on the shape's bounding box are seen from outside",
                   "[Render]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // A box's faces are its bounding box's, so every march starts on the surface.
  const BoxSDF<T> box(Vec3(T(-1), T(-0.5), T(-0.25)), Vec3(T(0.5), T(1), T(0.75)));

  const Render::Renderer<T> renderer(plain<T>(48));

  Render::Image image;

  const Render::Stats stats = renderer.render(box, Render::Camera<T>::fit(box.computeBoundingVolume()), image);

  CHECK(stats.m_hits > 300U);
  CHECK(stats.m_insideHits == 0U);
}
