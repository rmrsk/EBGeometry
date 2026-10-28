// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"
#include "TestTapeUtils.hpp"

#include <atomic>
#include <cmath>
#include <cstdint>
#include <string>
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
  settings.m_colourBy  = Render::ColourBy::Uniform;

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

  // Cut away the half towards the camera: it sees the flat cut face. Coloured by primitive and opaque, the
  // face takes the cut colour.
  renderer.settings().m_sectionNormal = Vec3(T(0), T(0), T(1));
  renderer.settings().m_shadows       = false;
  renderer.settings().m_occlusion     = false;
  renderer.settings().m_colourBy      = Render::ColourBy::Primitive;
  renderer.settings().m_cutOpacity    = T(1);

  (void)renderer.render(sphere, camera, opened);

  const Channels cut = channels(opened.at(size / 2, size / 2));

  CHECK(cut.r > cut.b + 40);
  CHECK(cut.g > cut.b + 30);
  CHECK(opened.at(size / 2, size / 2) != whole.at(size / 2, size / 2));
  CHECK(opened.at(size / 2 - 3, size / 2 + 2) != background);

  // Coloured by normal, the face takes the plane's normal, +z: it is blue, and the same across the face.
  renderer.settings().m_colourBy = Render::ColourBy::Normal;

  Render::Image byNormal;

  (void)renderer.render(sphere, camera, byNormal);

  const Channels right = channels(byNormal.at(size / 2 + size / 8, size / 2));

  CHECK(right.b > right.r + 40);
  CHECK(right.b > right.g + 40);
  CHECK(byNormal.at(size / 2 + size / 8, size / 2) == byNormal.at(size / 2 - size / 8, size / 2));

  // Partly transparent, the far wall of the sphere shows through the face.
  renderer.settings().m_cutOpacity = T(0.5);

  Render::Image seeThrough;

  (void)renderer.render(sphere, camera, seeThrough);

  CHECK(seeThrough.at(size / 2 + size / 8, size / 2) != byNormal.at(size / 2 + size / 8, size / 2));

  // What shows through is inside the solid, where shadows and occlusion would darken it everywhere: they are
  // left out there, so the face looks the same with them on as off.
  renderer.settings().m_shadows   = true;
  renderer.settings().m_occlusion = true;

  Render::Image shaded;

  (void)renderer.render(sphere, camera, shaded);

  CHECK(shaded.at(size / 2 + size / 8, size / 2) == seeThrough.at(size / 2 + size / 8, size / 2));
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

TEST_CASE("Render: rejects an image without pixels, bad supersampling, a degenerate camera, bad lighting and an "
          "unbounded fit",
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

      camera.m_pitch = T(91);

      Render::Image image;

      (void)Render::Renderer<T>().render(sphere, camera, image);
    },
    "Render::Renderer: the camera pitch must be from -90 to 90 degrees (91)"));

  REQUIRE(abortsWith(
    [&sphere] {
      Render::Camera<T> camera;

      camera.m_nearClip = T(-1);

      Render::Image image;

      (void)Render::Renderer<T>().render(sphere, camera, image);
    },
    "Render::Renderer: the near clip must be finite and not negative (-1)"));

  REQUIRE(abortsWith(
    [&sphere] {
      Render::Settings<T> settings;

      settings.m_shininess = T(0.5);

      Render::Image image;

      (void)Render::Renderer<T>(settings).render(sphere, Render::Camera<T>(), image);
    },
    "Render::Renderer: the shininess must be at least 1 (0.5)"));

  REQUIRE(abortsWith(
    [&sphere] {
      Render::Settings<T> settings;

      settings.m_ambient = T(-1);

      Render::Image image;

      (void)Render::Renderer<T>(settings).render(sphere, Render::Camera<T>(), image);
    },
    "Render::Renderer: the light intensity, ambient level, specular strength and checker size must not be "
    "negative"));

  REQUIRE(abortsWith(
    [&sphere] {
      Render::Settings<T> settings;

      settings.m_sheetOpacity = T(2);

      Render::Image image;

      (void)Render::Renderer<T>(settings).render(sphere, Render::Camera<T>(), image);
    },
    "Render::Renderer: the contour width must not be negative, and the sheet's opacity must be from 0 to 1 (2, 2)"));

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

TEMPLATE_TEST_CASE("Render: view returns at once when asked not to open a window",
                   "[Render]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const SphereSDF<T> sphere(Vec3T<T>::zeros(), T(1));

  // Built with or without the viewer, --no-view keeps the window shut, so this returns.
  char                program[] = "TestRender";
  char                noView[]  = "--no-view";
  char*               argv[]    = {program, noView, nullptr};
  const int           argc      = 2;
  Render::Settings<T> settings;

  Render::view(sphere, argc, argv);
  Render::view(sphere, Render::Camera<T>::fit(sphere.computeBoundingVolume()), settings, argc, argv);
  Render::view(sphere, sphere, argc, argv);
  Render::view(sphere, sphere, Render::Camera<T>(), settings, argc, argv);

  // A shape in a pool: with the window shut, the pool is not mirrored, nor frozen.
  Pool          pool(hostMemoryResource());
  const Tape<T> tape(pool, Expr<T>(sphere));

  Render::view(tape, pool, argc, argv);
  Render::view(tape, pool, Render::Camera<T>(), settings, argc, argv);

  CHECK_FALSE(pool.isFrozen());

  // An unbounded shape gets the default camera rather than a fit that would fail.
  Render::view(PlaneSDF<T>(Vec3T<T>::zeros(), Vec3T<T>(T(0), T(1), T(0))), argc, argv);

  SUCCEED();
}

TEMPLATE_TEST_CASE("Render: a ray-traced mesh cut by the section plane draws as the closed-form box does",
                   "[Render]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  const auto mesh = Parser::readIntoTriMeshSDF<T, 4, 4>(
    std::string(EBGEOMETRY_TEST_DATA_DIR) + "/cube_quads.obj", pool, BVH::Construction::SAH);
  const BoxSDF<T> box(Vec3::zeros(), Vec3::ones());

  constexpr uint32_t size = 40;

  Render::Settings<T> settings = plain<T>(size);

  settings.m_section       = true;
  settings.m_sectionPoint  = T(0.5) * Vec3::ones();
  settings.m_sectionNormal = Vec3(T(0.3), T(0.2), T(1));

  const Render::Renderer<T> renderer(settings);
  const auto                camera = Render::Camera<T>::fit(box.computeBoundingVolume());

  Render::Image traced;
  Render::Image marched;

  const auto tracedStats  = renderer.render(mesh, camera, traced);
  const auto marchedStats = renderer.render(box, camera, marched);

  // The same pixels are covered, give or take the rim.
  const uint32_t background = marched.at(0, 0);

  size_t differ = 0;

  for (size_t i = 0; i < traced.m_pixels.size(); i++) {
    differ += ((traced.m_pixels[i] == background) != (marched.m_pixels[i] == background)) ? 1 : 0;
  }

  CHECK(differ <= size_t(2 * size));
  CHECK(tracedStats.m_hits > 300U);
  CHECK(tracedStats.m_insideHits == 0U);
  CHECK(marchedStats.m_insideHits == 0U);

  // Through the middle of the image the camera looks at the cut face, in the cut colour on both.
  const Channels a = channels(traced.at(size / 2, size / 2));
  const Channels b = channels(marched.at(size / 2, size / 2));

  CHECK(std::abs(a.r - b.r) <= 6);
  CHECK(std::abs(a.g - b.g) <= 6);
  CHECK(std::abs(a.b - b.b) <= 6);
}

TEMPLATE_TEST_CASE("Render: a cancelled render stops early and says so", "[Render]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const SphereSDF<T> sphere(Vec3T<T>::zeros(), T(1));

  const Render::Renderer<T> renderer(plain<T>(16));
  const auto                camera = Render::Camera<T>::fit(sphere.computeBoundingVolume());

  Render::Image image;

  std::atomic<bool>     cancel{false};
  std::atomic<uint32_t> rows{0};

  // Not cancelled: every row is done.
  const auto whole = renderer.render(sphere, camera, image, cancel, &rows);

  CHECK(whole.m_complete);
  CHECK(rows == 16U);

  const auto onDevice = renderer.renderOnDevice(sphere, camera, image, cancel, &rows);

  CHECK(onDevice.m_complete);
  CHECK(rows == 32U);

  // Cancelled before it starts: no row is done, and the image still has its size.
  cancel = true;
  rows   = 0;

  const auto stopped = renderer.render(sphere, camera, image, cancel, &rows);

  CHECK_FALSE(stopped.m_complete);
  CHECK(rows == 0U);
  CHECK(image.m_pixels.size() == 256U);

  const auto stoppedOnDevice = renderer.renderOnDevice(sphere, camera, image, cancel, &rows);

  CHECK_FALSE(stoppedOnDevice.m_complete);
  CHECK(rows == 0U);
}

TEMPLATE_TEST_CASE("Render: a primitive colour set in the table replaces the palette's for that primitive only",
                   "[Render]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // Leaf 0 is the sphere on the left, leaf 1 the one on the right.
  const TestUtils::Compiled<T> pair(
    Union(SphereSDF<T>(Vec3(T(-1), T(0), T(0)), T(0.6)), SphereSDF<T>(Vec3(T(1), T(0), T(0)), T(0.6))));

  Render::Camera<T> camera = Render::Camera<T>::fit(pair.tape().computeBoundingVolume(), Render::View::Front);

  constexpr uint32_t size = 32;

  Render::Settings<T> settings = plain<T>(size);

  settings.m_light     = Vec3(T(0), T(0), T(1));
  settings.m_shadows   = false;
  settings.m_occlusion = false;
  settings.m_colourBy  = Render::ColourBy::Primitive;

  Render::Image palette;
  Render::Image table;

  Render::Renderer<T> renderer(settings);

  (void)renderer.render(pair.tape(), camera, palette);

  renderer.settings().m_primitiveColours[0] = Vec3(T(0), T(0), T(1));

  (void)renderer.render(pair.tape(), camera, table);

  // The centre of each sphere along the middle row: a quarter and three quarters across.
  const Channels left  = channels(table.at(size / 2 - size / 4, size / 2));
  const Channels right = channels(table.at(size / 2 + size / 4, size / 2));

  CHECK(left.b > left.r + 60);
  CHECK(left.b > left.g + 60);
  CHECK(table.at(size / 2 + size / 4, size / 2) == palette.at(size / 2 + size / 4, size / 2));
  CHECK(right.r + right.g + right.b > 0);
}

TEMPLATE_TEST_CASE("Render: colouring by normal shows the surface's direction", "[Render]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const SphereSDF<T> sphere(Vec3::zeros(), T(1));

  // Looking along -z: the centre's normal is +z (blue), the left rim's -x (no red).
  const auto camera = Render::Camera<T>::fit(sphere.computeBoundingVolume(), Render::View::Front);

  constexpr uint32_t size = 32;

  Render::Settings<T> settings = plain<T>(size);

  settings.m_light     = Vec3(T(0), T(0), T(1));
  settings.m_shadows   = false;
  settings.m_occlusion = false;
  settings.m_specular  = T(0);
  settings.m_colourBy  = Render::ColourBy::Normal;

  Render::Image image;

  (void)Render::Renderer<T>(settings).render(sphere, camera, image);

  const Channels centre = channels(image.at(size / 2, size / 2));
  const Channels left   = channels(image.at(size / 2 - size / 5, size / 2));

  CHECK(centre.b > centre.r + 40);
  CHECK(centre.b > centre.g + 40);
  CHECK(left.g > left.r + 20);
}

TEMPLATE_TEST_CASE("Render: the light's intensity, the ambient level, the highlights and the background are settings",
                   "[Render]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const SphereSDF<T> sphere(Vec3::zeros(), T(1));

  const auto camera = Render::Camera<T>::fit(sphere.computeBoundingVolume(), Render::View::Front);

  constexpr uint32_t size = 24;

  Render::Settings<T> settings = plain<T>(size);

  settings.m_shadows    = false;
  settings.m_occlusion  = false;
  settings.m_background = Vec3(T(1), T(0), T(0));

  Render::Image lit;
  Render::Image dark;

  Render::Renderer<T> renderer(settings);

  (void)renderer.render(sphere, camera, lit);

  // With no light of any kind the surface is black; the background is the colour set.
  renderer.settings().m_lightIntensity = T(0);
  renderer.settings().m_ambient        = T(0);
  renderer.settings().m_specular       = T(0);

  (void)renderer.render(sphere, camera, dark);

  const Channels corner = channels(dark.at(0, 0));
  const Channels centre = channels(dark.at(size / 2, size / 2));

  CHECK(corner.r == 255);
  CHECK(corner.g == 0);
  CHECK(corner.b == 0);
  CHECK(centre.r + centre.g + centre.b == 0);

  const Channels bright = channels(lit.at(size / 2, size / 2));

  CHECK(bright.r + bright.g + bright.b > 200);

  // A sky of one colour fills the background with it.
  renderer.settings().m_sky        = true;
  renderer.settings().m_skyHorizon = Vec3(T(0), T(1), T(0));
  renderer.settings().m_skyZenith  = Vec3(T(0), T(1), T(0));

  Render::Image sky;

  (void)renderer.render(sphere, camera, sky);

  const Channels green = channels(sky.at(0, 0));

  CHECK(green.g == 255);
  CHECK(green.r == 0);
}

TEMPLATE_TEST_CASE("Render: the floor's checkerboard can be turned off and resized",
                   "[Render]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const SphereSDF<T> sphere(Vec3::zeros(), T(1));

  const auto camera = Render::Camera<T>::fit(sphere.computeBoundingVolume());

  Render::Settings<T> settings = plain<T>(32);

  settings.m_floor     = true;
  settings.m_shadows   = false;
  settings.m_occlusion = false;

  Render::Image checked;
  Render::Image plainFloor;
  Render::Image larger;

  Render::Renderer<T> renderer(settings);

  (void)renderer.render(sphere, camera, checked);

  renderer.settings().m_floorCheckerSize = T(1.7);

  (void)renderer.render(sphere, camera, larger);

  renderer.settings().m_floorChecker = false;

  (void)renderer.render(sphere, camera, plainFloor);

  CHECK(checked.m_pixels != plainFloor.m_pixels);
  CHECK(checked.m_pixels != larger.m_pixels);
  CHECK(larger.m_pixels != plainFloor.m_pixels);
}

TEMPLATE_TEST_CASE("Render: an orthographic camera shows a sphere as a disc of its own radius",
                   "[Render]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Vec3         center(T(0.2), T(-0.1), T(0));
  const T            radius = T(0.7);
  const SphereSDF<T> sphere(center, radius);

  constexpr uint32_t size = 40;

  Render::Camera<T> camera = Render::Camera<T>::fit(sphere.computeBoundingVolume(), Render::View::Front);

  camera.m_orthographic = true;

  Render::Image image;

  const Render::Stats stats = Render::Renderer<T>(plain<T>(size)).render(sphere, camera, image);

  // A pixel's ray runs along -z through (x, y) on the plane through the eye; the view is as tall as the
  // perspective view at the target.
  const T        half  = camera.m_distance * std::tan(T(0.5) * camera.m_fieldOfView * pi<T> / T(180));
  const T        pixel = T(2) * half / T(size);
  const uint32_t bg    = image.at(0, 0);
  size_t         hit   = 0;

  for (uint32_t row = 0; row < size; row++) {
    for (uint32_t column = 0; column < size; column++) {
      const T x = camera.m_target[0] - half + (T(column) + T(0.5)) * pixel;
      const T y = camera.m_target[1] + half - (T(row) + T(0.5)) * pixel;
      const T r = std::sqrt((x - center[0]) * (x - center[0]) + (y - center[1]) * (y - center[1]));

      if (r < T(0.95) * radius) {
        CHECK(image.at(column, row) != bg);

        hit++;
      }
      else if (r > T(1.05) * radius) {
        CHECK(image.at(column, row) == bg);
      }
    }
  }

  CHECK(hit > 100U);
  CHECK(stats.m_insideHits == 0U);
}

TEMPLATE_TEST_CASE("Render: the near clip hides what is nearer than it", "[Render]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const SphereSDF<T> sphere(Vec3::zeros(), T(1));

  Render::Camera<T> camera = Render::Camera<T>::fit(sphere.computeBoundingVolume(), Render::View::Front);

  const Render::Renderer<T> renderer(plain<T>(24));

  Render::Image image;

  // Clipped just past the centre, the rays through the sphere start inside it and see its far side from
  // within.
  camera.m_nearClip = camera.m_distance + T(0.1);

  const Render::Stats inside = renderer.render(sphere, camera, image);

  CHECK(inside.m_hits > 0U);
  CHECK(inside.m_insideHits == inside.m_hits);

  // Clipped beyond the sphere, nothing is seen.
  camera.m_nearClip = camera.m_distance + T(1.5);

  const Render::Stats beyond = renderer.render(sphere, camera, image);

  CHECK(beyond.m_hits == 0U);
}

TEMPLATE_TEST_CASE("Render: the camera fits a box from a named view, straight along the up axis too",
                   "[Render]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const BoundingVolumes::AABBT<T> box(Vec3(T(-1), T(0), T(2)), Vec3(T(3), T(1), T(4)));

  for (const Vec3& up : {Vec3(0, 1, 0), Vec3(0, 0, 1)}) {
    const auto fitted = Render::Camera<T>::fit(box, up);

    const std::vector<std::pair<Render::View, Vec3>> views{
      {Render::View::Top, up}, {Render::View::Bottom, -up}, {Render::View::Front, Vec3::zeros()}};

    for (const auto& [view, along] : views) {
      const auto camera = Render::Camera<T>::fit(box, view, up);

      // The same target and distance as the default fit, from another direction.
      CHECK((camera.m_target - fitted.m_target).length() < T(tightMargin<T>()));
      CHECK(camera.m_distance == fitted.m_distance);

      Vec3 forward;
      Vec3 right;
      Vec3 screenUp;

      camera.frame(forward, right, screenUp);

      CHECK(std::abs(right.length() - T(1)) < T(looseMargin<T>()));
      CHECK(std::abs(screenUp.length() - T(1)) < T(looseMargin<T>()));
      CHECK(std::abs(dot(right, forward)) < T(looseMargin<T>()));
      CHECK(std::abs(dot(right, up)) < T(looseMargin<T>()));

      if (along.length() > T(0)) {
        CHECK((camera.eye() - camera.m_target - camera.m_distance * along).length() <
              T(looseMargin<T>()) * T(10) * camera.m_distance);
        CHECK((forward + along).length() < T(looseMargin<T>()) * T(10));
      }
      else {
        CHECK(std::abs(dot(forward, up)) < T(looseMargin<T>()));
      }
    }
  }

  // Looking straight down renders like any other view.
  const SphereSDF<T> sphere(Vec3::zeros(), T(1));

  Render::Image image;

  const Render::Stats stats =
    Render::Renderer<T>(plain<T>(16))
      .render(sphere, Render::Camera<T>::fit(sphere.computeBoundingVolume(), Render::View::Top), image);

  CHECK(stats.m_hits > 50U);
  CHECK(stats.m_insideHits == 0U);
}

TEMPLATE_TEST_CASE("Render: toCode() writes the fields that differ from their defaults as C++",
                   "[Render]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  CHECK(Render::toCode(Render::Camera<T>(), Render::Settings<T>()) ==
        "EBGeometry::Render::Camera<T> camera;\nEBGeometry::Render::Settings<T> settings;\n");

  Render::Camera<T>   camera;
  Render::Settings<T> settings;

  camera.m_yaw                   = T(0.1);
  camera.m_orthographic          = true;
  camera.m_target                = Vec3(T(1), T(-2.5), T(0));
  settings.m_width               = 1280;
  settings.m_shadows             = false;
  settings.m_colourBy            = Render::ColourBy::Uniform;
  settings.m_primitiveColours[3] = Vec3(T(0.5), T(0.5), T(0.5));
  settings.m_rays.m_maxSteps     = 100;

  CHECK(Render::toCode(camera, settings) ==
        "EBGeometry::Render::Camera<T> camera;\n"
        "camera.m_target = EBGeometry::Vec3T<T>(T(1), T(-2.5), T(0));\n"
        "camera.m_yaw = T(0.1);\n"
        "camera.m_orthographic = true;\n"
        "EBGeometry::Render::Settings<T> settings;\n"
        "settings.m_width = 1280;\n"
        "settings.m_shadows = false;\n"
        "settings.m_colourBy = EBGeometry::Render::ColourBy::Uniform;\n"
        "settings.m_primitiveColours[3] = EBGeometry::Vec3T<T>(T(0.5), T(0.5), T(0.5));\n"
        "settings.m_rays.m_maxSteps = 100;\n");
}

TEMPLATE_TEST_CASE("Render: the device render matches the host's with an orthographic camera, a near clip, a "
                   "primitive colour table and the surface settings",
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

  settings.m_width               = 40;
  settings.m_height              = 32;
  settings.m_supersampling       = 2;
  settings.m_colourBy            = Render::ColourBy::Primitive;
  settings.m_primitiveColours[1] = Vec3(T(0.2), T(0.3), T(0.9));
  settings.m_shininess           = T(12);
  settings.m_specular            = T(0.6);
  settings.m_ambient             = T(0.2);
  settings.m_floorCheckerSize    = T(0.5);
  settings.m_section             = true;
  settings.m_sectionPoint        = Vec3(T(0.1), T(0), T(0));
  settings.m_sectionNormal       = Vec3(T(1), T(0.2), T(0));
  settings.m_sheetOpacity        = T(0.3);

  for (const bool orthographic : {false, true}) {
    auto camera = Render::Camera<T>::fit(tape.computeBoundingVolume(), Render::View::Top);

    camera.m_orthographic = orthographic;
    camera.m_nearClip     = T(0.5) * camera.m_distance;

    for (const Render::ColourBy colourBy : {Render::ColourBy::Primitive, Render::ColourBy::Normal}) {
      settings.m_colourBy = colourBy;

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
      CHECK(std::abs(double(hostStats.m_hits) - double(deviceStats.m_hits)) <= double(hostStats.m_rays) / 100);
    }
  }
}

TEMPLATE_TEST_CASE("Render: the section plane draws the shape's contour, and a sheet outside it within the box",
                   "[Render]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const SphereSDF<T> sphere(Vec3::zeros(), T(1));

  // Looking along -z with parallel rays, the half towards the camera removed: the cut face is the disc of
  // radius 1 in the plane z = 0, and a pixel's ray meets the plane at the pixel's own (x, y).
  Render::Camera<T> camera = Render::Camera<T>::fit(sphere.computeBoundingVolume(), Render::View::Front);

  camera.m_orthographic = true;

  constexpr uint32_t size = 64;

  Render::Settings<T> settings = plain<T>(size);

  settings.m_section       = true;
  settings.m_sectionPoint  = Vec3::zeros();
  settings.m_sectionNormal = Vec3(T(0), T(0), T(1));
  settings.m_contourWidth  = T(3);
  settings.m_sheetOpacity  = T(0.5);
  settings.m_sheetColour   = Vec3(T(0), T(0), T(1));

  Render::Renderer<T> renderer(settings);

  Render::Image drawn;

  (void)renderer.render(sphere, camera, drawn);

  renderer.settings().m_contourWidth = T(0);
  renderer.settings().m_sheetOpacity = T(0);

  Render::Image bare;

  (void)renderer.render(sphere, camera, bare);

  const T half  = camera.m_distance * std::tan(T(0.5) * camera.m_fieldOfView * pi<T> / T(180));
  const T pixel = T(2) * half / T(size);

  size_t onContour = 0;

  for (uint32_t row = 0; row < size; row++) {
    for (uint32_t column = 0; column < size; column++) {
      const T x = -half + (T(column) + T(0.5)) * pixel;
      const T y = half - (T(row) + T(0.5)) * pixel;
      const T r = std::sqrt(x * x + y * y);

      const Channels c = channels(drawn.at(column, row));

      if (std::abs(r - T(1)) < T(0.5) * pixel) {
        // On the contour: its colour, 0.04 of full scale before the display's gamma (59 of 255 after it).
        CHECK(c.r + c.g + c.b < 200);
        CHECK(drawn.at(column, row) != bare.at(column, row));

        onContour++;
      }
      else if (r < T(1) - T(3) * pixel) {
        // The cut face, as without the contour.
        CHECK(drawn.at(column, row) == bare.at(column, row));
      }
      else if (r > T(1) + T(3) * pixel && std::abs(x) < T(1) && std::abs(y) < T(1)) {
        // Outside the sphere but within its box: the sheet, blue over the background.
        const Channels b = channels(bare.at(column, row));

        CHECK(c.b > b.b + 40);
      }
      else if (std::abs(x) > T(1.2) || std::abs(y) > T(1.2)) {
        // Beyond the enlarged box: nothing drawn.
        CHECK(drawn.at(column, row) == bare.at(column, row));
      }
    }
  }

  CHECK(onContour > 20U);
}

TEMPLATE_TEST_CASE("Render: pick() names the primitive under a point of the image, and the highlight tints it",
                   "[Render]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // Leaf 0 is the sphere on the left, leaf 1 the one on the right.
  const TestUtils::Compiled<T> pair(
    Union(SphereSDF<T>(Vec3(T(-1), T(0), T(0)), T(0.6)), SphereSDF<T>(Vec3(T(1), T(0), T(0)), T(0.6))));

  // Orthographic along -z: the point (column, row) of the image sees (x, y) of the scene.
  Render::Camera<T> camera = Render::Camera<T>::fit(pair.tape().computeBoundingVolume(), Render::View::Front);

  camera.m_orthographic = true;

  constexpr uint32_t size = 40;

  Render::Settings<T> settings = plain<T>(size);

  settings.m_shadows   = false;
  settings.m_occlusion = false;

  const T half  = camera.m_distance * std::tan(T(0.5) * camera.m_fieldOfView * pi<T> / T(180));
  const T scale = T(size) / (T(2) * half);

  const auto column = [&](const T a_x) { return (a_x - camera.m_target[0] + half) * scale; };
  const auto row    = [&](const T a_y) { return (camera.m_target[1] + half - a_y) * scale; };

  Render::Renderer<T> renderer(settings);

  for (const auto& [x, leaf] : std::vector<std::pair<T, uint32_t>>{{T(-1), 0U}, {T(1), 1U}}) {
    const HitT<T> hit = renderer.pick(pair.tape(), camera, column(x), row(T(0)));

    REQUIRE(hit.hit());
    CHECK(hit.m_primitive == leaf);
    CHECK((hit.m_point - Vec3(x, T(0), T(0.6))).length() < T(1e-2));
    CHECK(hit.m_normal[2] > T(0.99));
  }

  CHECK_FALSE(renderer.pick(pair.tape(), camera, T(0.5), T(0.5)).hit());

  // The highlighted sphere is tinted, the other drawn as before.
  Render::Image plainImage;
  Render::Image highlighted;

  (void)renderer.render(pair.tape(), camera, plainImage);

  renderer.settings().m_highlightPrimitive = 1;

  (void)renderer.render(pair.tape(), camera, highlighted);

  const auto at = [&](const Render::Image& a_image, const T a_x) {
    return a_image.at(uint32_t(column(a_x)), uint32_t(row(T(0))));
  };

  CHECK(at(highlighted, T(-1)) == at(plainImage, T(-1)));
  CHECK(at(highlighted, T(1)) != at(plainImage, T(1)));
}
