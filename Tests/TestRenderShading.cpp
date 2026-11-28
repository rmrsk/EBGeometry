// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

// Test suite for the renderer's shading: coloring by primitive color or normal, the light, fill light, ambient level,
// highlights and shoulder, the backdrop and the floor, and the looks that set them.

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestLocale.hpp"
#include "TestRenderHelpers.hpp"
#include "TestTapeUtils.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>

using namespace EBGeometry;
using namespace EBGeometryTestRender;

TEMPLATE_TEST_CASE("Render: a primitive color set in the table replaces the palette's for that primitive only",
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
  settings.m_colorBy   = Render::ColorBy::Primitive;

  Render::Image palette;
  Render::Image table;

  Render::Renderer<T> renderer(settings);

  (void)renderer.render(pair.tape(), camera, palette);

  renderer.settings().m_primitiveColors[0] = Vec3(T(0), T(0), T(1));

  (void)renderer.render(pair.tape(), camera, table);

  // The center of each sphere along the middle row: a quarter and three quarters across.
  const Channels left  = channels(table.at(size / 2 - size / 4, size / 2));
  const Channels right = channels(table.at(size / 2 + size / 4, size / 2));

  CHECK(left.b > left.r + 60);
  CHECK(left.b > left.g + 60);
  CHECK(table.at(size / 2 + size / 4, size / 2) == palette.at(size / 2 + size / 4, size / 2));
  CHECK(right.r + right.g + right.b > 0);
}

TEMPLATE_TEST_CASE("Render: coloring by normal shows the surface's direction", "[Render]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const SphereSDF<T> sphere(Vec3::zeros(), T(1));

  // Looking along -z: the center's normal is +z (blue), the left rim's -x (no red).
  const auto camera = Render::Camera<T>::fit(sphere.computeBoundingVolume(), Render::View::Front);

  constexpr uint32_t size = 32;

  Render::Settings<T> settings = plain<T>(size);

  // The light at the eye, and no fill light, which would light the center from there too and wash its color out.
  settings.m_light         = Vec3(T(0), T(0), T(1));
  settings.m_fillIntensity = T(0);
  settings.m_shadows       = false;
  settings.m_occlusion     = false;
  settings.m_specular      = T(0);
  settings.m_colorBy       = Render::ColorBy::Normal;

  Render::Image image;

  (void)Render::Renderer<T>(settings).render(sphere, camera, image);

  const Channels center = channels(image.at(size / 2, size / 2));
  const Channels left   = channels(image.at(size / 2 - size / 5, size / 2));

  CHECK(center.b > center.r + 40);
  CHECK(center.b > center.g + 40);
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

  // With no light of any kind the surface is black; the background is the color set.
  renderer.settings().m_lightIntensity = T(0);
  renderer.settings().m_ambient        = T(0);
  renderer.settings().m_fillIntensity  = T(0);
  renderer.settings().m_specular       = T(0);

  (void)renderer.render(sphere, camera, dark);

  const Channels corner = channels(dark.at(0, 0));
  const Channels center = channels(dark.at(size / 2, size / 2));

  CHECK(corner.r == 255);
  CHECK(corner.g == 0);
  CHECK(corner.b == 0);
  CHECK(center.r + center.g + center.b == 0);

  const Channels bright = channels(lit.at(size / 2, size / 2));

  CHECK(bright.r + bright.g + bright.b > 200);

  // A sky of one color fills the background with it.
  renderer.settings().m_backdrop   = Render::Backdrop::Sky;
  renderer.settings().m_skyHorizon = Vec3(T(0), T(1), T(0));
  renderer.settings().m_skyZenith  = Vec3(T(0), T(1), T(0));

  Render::Image sky;

  (void)renderer.render(sphere, camera, sky);

  const Channels green = channels(sky.at(0, 0));

  CHECK(green.g == 255);
  CHECK(green.r == 0);
}

TEMPLATE_TEST_CASE("Render: the fill light lights the side the light leaves dark, and not the floor",
                   "[Render]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const SphereSDF<T> sphere(Vec3::zeros(), T(1));

  constexpr uint32_t size = 32;

  // Only the light and the fill light, on a red background: a pixel of the sphere the light does not reach is black
  // without the fill.
  Render::Settings<T> settings = plain<T>(size);

  settings.m_shadows    = false;
  settings.m_occlusion  = false;
  settings.m_ambient    = T(0);
  settings.m_specular   = T(0);
  settings.m_background = Vec3(T(1), T(0), T(0));

  const auto front = Render::Camera<T>::fit(sphere.computeBoundingVolume(), Render::View::Front);

  Render::Image without;
  Render::Image with;

  settings.m_fillIntensity = T(0);
  (void)Render::Renderer<T>(settings).render(sphere, front, without);

  settings.m_fillIntensity = T(0.3);
  (void)Render::Renderer<T>(settings).render(sphere, front, with);

  int dark   = 0;
  int filled = 0;

  for (uint32_t row = 0; row < size; row++) {
    for (uint32_t column = 0; column < size; column++) {
      const Channels a = channels(without.at(column, row));

      if (a.r + a.g + a.b == 0) {
        const Channels b = channels(with.at(column, row));

        dark++;
        filled += (b.r + b.g + b.b > 30) ? 1 : 0;
      }
    }
  }

  CHECK(dark > 20);
  CHECK(filled == dark);

  // The floor seen from above, as the default camera sees it: the fill light comes from below the horizon, so the
  // floor's pixels (the bottom row, far from the sphere) are the same with and without it.
  settings.m_floor   = true;
  settings.m_shadows = true;
  settings.m_ambient = T(0.35);

  const auto above = Render::Camera<T>::fit(sphere.computeBoundingVolume());

  settings.m_fillIntensity = T(0);
  (void)Render::Renderer<T>(settings).render(sphere, above, without);

  settings.m_fillIntensity = T(1);
  (void)Render::Renderer<T>(settings).render(sphere, above, with);

  for (uint32_t column = 0; column < size; column++) {
    CHECK(with.at(column, size - 1) == without.at(column, size - 1));
  }

  // Straight down the up axis, with the light straight ahead: the fill comes from straight below, and the upper half of
  // the sphere, all the camera sees, gets none of it (and no pixel is lost to a direction of zero length).
  const auto top = Render::Camera<T>::fit(sphere.computeBoundingVolume(), Render::View::Top);

  settings.m_floor   = false;
  settings.m_shadows = false;
  settings.m_light   = Vec3(T(0), T(0), T(1));

  settings.m_fillIntensity = T(0);
  (void)Render::Renderer<T>(settings).render(sphere, top, without);

  settings.m_fillIntensity = T(1);
  (void)Render::Renderer<T>(settings).render(sphere, top, with);

  CHECK(with.m_pixels == without.m_pixels);
  CHECK(channels(with.at(size / 2, size / 2)).r > 100);
}

TEMPLATE_TEST_CASE("Render: the shoulder rolls lit surfaces off towards white, and 1 clips them",
                   "[Render]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const SphereSDF<T> sphere(Vec3::zeros(), T(1));

  // An odd size, so that the center pixel's ray meets the sphere where its normal faces the camera, square to the up
  // axis: there, a white surface lit by the ambient term alone reflects c = ambient / 2.
  constexpr uint32_t size = 33;

  Render::Settings<T> settings = plain<T>(size);

  settings.m_shadows        = false;
  settings.m_occlusion      = false;
  settings.m_lightIntensity = T(0);
  settings.m_fillIntensity  = T(0);
  settings.m_specular       = T(0);
  settings.m_colorBy        = Render::ColorBy::Uniform;
  settings.m_color          = Vec3::ones();

  const auto camera = Render::Camera<T>::fit(sphere.computeBoundingVolume(), Render::View::Front);

  // The byte a linear value is encoded to.
  const auto encoded = [](const double a_c) -> int {
    return int(255.0 * std::pow(std::min(a_c, 1.0), 1.0 / 2.2) + 0.5);
  };

  // The center pixel's red channel for a reflected c.
  const auto center = [&](const T a_c) -> int {
    settings.m_ambient = T(2) * a_c;

    Render::Image image;

    (void)Render::Renderer<T>(settings).render(sphere, camera, image);

    return channels(image.at(size / 2, size / 2)).r;
  };

  // The shoulder at 0.8: c - (c - 0.8)^2 / 0.8 from 0.8 to 1.2, and 1 above. Increasing, and white only at 1.2.
  settings.m_shoulder = T(0.8);

  const int below = center(T(0.7));
  const int at09  = center(T(0.9));
  const int at11  = center(T(1.1));
  const int above = center(T(1.3));

  CHECK(std::abs(below - encoded(0.7)) <= 1);
  CHECK(std::abs(at09 - encoded(0.9 - 0.01 / 0.8)) <= 1);
  CHECK(std::abs(at11 - encoded(1.1 - 0.09 / 0.8)) <= 1);
  CHECK(above == 255);
  CHECK(below < at09);
  CHECK(at09 < at11);
  CHECK(at11 < 255);

  // At 1, no roll-off: the value itself, clipped at white.
  settings.m_shoulder = T(1);

  CHECK(std::abs(center(T(0.9)) - encoded(0.9)) <= 1);
  CHECK(center(T(1.1)) == 255);
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

  renderer.settings().m_floorCellSize = T(1.7);

  (void)renderer.render(sphere, camera, larger);

  renderer.settings().m_floorStyle = Render::FloorStyle::Plain;

  (void)renderer.render(sphere, camera, plainFloor);

  CHECK(checked.m_pixels != plainFloor.m_pixels);
  CHECK(checked.m_pixels != larger.m_pixels);
  CHECK(larger.m_pixels != plainFloor.m_pixels);
}

TEMPLATE_TEST_CASE("Render: a gradient runs from the top of the view to its bottom, a vignette from its center out",
                   "[Render]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // A small sphere in the middle, so that the image's edges and corners are the backdrop.
  const SphereSDF<T> sphere(Vec3::zeros(), T(0.1));

  const auto camera = Render::Camera<T>::fit(SphereSDF<T>(Vec3::zeros(), T(1)).computeBoundingVolume());

  constexpr uint32_t size = 40;

  Render::Settings<T> settings = plain<T>(size);

  settings.m_backdrop     = Render::Backdrop::Gradient;
  settings.m_gradientFrom = Vec3(T(1), T(0), T(0));
  settings.m_gradientTo   = Vec3(T(0), T(0), T(1));

  Render::Renderer<T> renderer(settings);
  Render::Image       gradient;
  Render::Image       vignette;

  (void)renderer.render(sphere, camera, gradient);

  // Red at the top, blue at the bottom, the same across each row, and halfway (gamma-encoded) in between.
  const Channels top    = channels(gradient.at(3, 0));
  const Channels bottom = channels(gradient.at(3, size - 1));
  const Channels middle = channels(gradient.at(3, size / 2));

  CHECK(top.r > 245);
  CHECK(top.b < 50);
  CHECK(bottom.b > 245);
  CHECK(bottom.r < 50);
  CHECK(gradient.at(0, 7) == gradient.at(size - 1, 7));
  CHECK(std::abs(middle.r - middle.b) < 12);
  CHECK(middle.r > 150);

  renderer.settings().m_backdrop = Render::Backdrop::Vignette;

  (void)renderer.render(sphere, camera, vignette);

  // Red in the middle of an edge (a little way out), blue in the corners, alike in all four.
  const Channels corner = channels(vignette.at(0, 0));
  const Channels edge   = channels(vignette.at(size / 2, 0));

  CHECK(corner.b > 240);
  CHECK(corner.r < 60);
  CHECK(edge.r > corner.r);
  CHECK(vignette.at(0, 0) == vignette.at(size - 1, size - 1));
  CHECK(vignette.at(0, size - 1) == vignette.at(size - 1, 0));
}

TEMPLATE_TEST_CASE("Render: a floor of only the shadow is the backdrop, darkened under the shape",
                   "[Render]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const SphereSDF<T> sphere(Vec3::zeros(), T(1));

  auto camera = Render::Camera<T>::fit(sphere.computeBoundingVolume());

  camera.m_pitch = T(40);

  constexpr uint32_t size = 48;

  Render::Settings<T> settings = plain<T>(size);

  settings.m_floor           = true;
  settings.m_floorStyle      = Render::FloorStyle::ShadowOnly;
  settings.m_backdrop        = Render::Backdrop::Plain;
  settings.m_background      = Vec3(T(0.5), T(0.5), T(0.5));
  settings.m_lightFromCamera = false;
  settings.m_light           = Vec3(T(0), T(1), T(0));
  settings.m_colorBy         = Render::ColorBy::Uniform;
  settings.m_color           = Vec3(T(1), T(0), T(0));

  Render::Renderer<T> renderer(settings);
  Render::Image       image;

  (void)renderer.render(sphere, camera, image);

  // The light is straight above: the shadow lies under the (red) sphere, the floor elsewhere is exactly the backdrop.
  const uint32_t backdrop = image.at(0, 0);

  uint32_t darker  = 0;
  uint32_t lighter = 0;

  for (uint32_t row = 0; row < size; row++) {
    for (uint32_t column = 0; column < size; column++) {
      const Channels c = channels(image.at(column, row));

      if (image.at(column, row) != backdrop && c.r == c.g && c.g == c.b) {
        (c.r < channels(backdrop).r) ? darker++ : lighter++;
      }
    }
  }

  CHECK(image.at(0, size - 1) == backdrop);
  CHECK(image.at(size - 1, size - 1) == backdrop);
  CHECK(darker > 20);
  CHECK(lighter == 0);
}

TEMPLATE_TEST_CASE("Render: a grid floor draws its lines, and a mirror floor reflects the shape",
                   "[Render]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const SphereSDF<T> sphere(Vec3::zeros(), T(1));

  auto camera = Render::Camera<T>::fit(sphere.computeBoundingVolume());

  camera.m_pitch = T(20);

  constexpr uint32_t size = 64;

  Render::Settings<T> settings = plain<T>(size);

  settings.m_floor          = true;
  settings.m_shadows        = false;
  settings.m_occlusion      = false;
  settings.m_floorStyle     = Render::FloorStyle::Grid;
  settings.m_floorColor     = Vec3(T(0.5), T(0.5), T(0.5));
  settings.m_floorLineColor = Vec3(T(1), T(0), T(0));
  settings.m_floorCellSize  = T(0.5);
  settings.m_colorBy        = Render::ColorBy::Uniform;
  settings.m_color          = Vec3(T(0), T(1), T(0));

  Render::Renderer<T> renderer(settings);
  Render::Image       grid;
  Render::Image       mirror;
  Render::Image       matte;

  (void)renderer.render(sphere, camera, grid);

  // Lines are red on a gray floor: some floor pixels are redder than they are blue.
  uint32_t lines = 0;

  for (const uint32_t pixel : grid.m_pixels) {
    const Channels c = channels(pixel);

    lines += (c.r > c.b + 20 && c.g < c.r) ? 1U : 0U;
  }

  CHECK(lines > 20);

  // On a mirror floor, the green sphere shows below itself; with no reflectance, the floor there is gray.
  renderer.settings().m_floorStyle       = Render::FloorStyle::Mirror;
  renderer.settings().m_floorReflectance = T(1);

  (void)renderer.render(sphere, camera, mirror);

  renderer.settings().m_floorReflectance = T(0);

  (void)renderer.render(sphere, camera, matte);

  uint32_t reflected = 0;

  for (uint32_t row = size / 2; row < size; row++) {
    for (uint32_t column = 0; column < size; column++) {
      const Channels m = channels(mirror.at(column, row));
      const Channels d = channels(matte.at(column, row));

      reflected += (m.g > m.r + 30 && !(d.g > d.r + 30)) ? 1U : 0U;
    }
  }

  CHECK(reflected > 20);
}

TEMPLATE_TEST_CASE("Render: a larger light softens the shadow's edge over a pixel's rays",
                   "[Render]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const SphereSDF<T> sphere(Vec3::zeros(), T(1));

  auto camera = Render::Camera<T>::fit(sphere.computeBoundingVolume());

  camera.m_pitch = T(60);

  constexpr uint32_t size = 64;

  Render::Settings<T> settings = plain<T>(size);

  settings.m_floor         = true;
  settings.m_floorStyle    = Render::FloorStyle::Plain;
  settings.m_occlusion     = false;
  settings.m_supersampling = 4;
  settings.m_lightSize     = T(0);

  Render::Renderer<T> renderer(settings);
  Render::Image       hard;
  Render::Image       soft;

  (void)renderer.render(sphere, camera, hard);

  renderer.settings().m_lightSize = T(0.4);

  (void)renderer.render(sphere, camera, soft);

  // The floor's shades: with a point light, its pixels are lit, shadowed, or (at the shadow's edge, split among a
  // pixel's rays) between; a large light widens the edge, so more pixels lie between.
  const auto between = [&](const Render::Image& a_image) {
    std::vector<uint32_t> shades(a_image.m_pixels.begin(), a_image.m_pixels.end());

    std::sort(shades.begin(), shades.end());

    return std::unique(shades.begin(), shades.end()) - shades.begin();
  };

  CHECK(between(soft) > between(hard));
}

TEMPLATE_TEST_CASE("Render: every look has its own name, and sets the backdrop and the floor alone",
                   "[Render]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  std::vector<std::string> names;
  std::vector<std::string> codes;

  for (uint8_t k = 0; k < Render::NumLooks; k++) {
    Render::Settings<T> settings;

    settings.m_color         = Vec3(T(0.1), T(0.2), T(0.3));
    settings.m_supersampling = 3;

    Render::setLook(settings, Render::Look(k));

    names.emplace_back(Render::lookName(Render::Look(k)));
    codes.push_back(Render::toCode(Render::Camera<T>(), settings));

    // What a look does not set stays.
    CHECK(settings.m_supersampling == 3U);
    CHECK(settings.m_color[2] == T(0.3));
  }

  CHECK(names[0] == "sky");
  CHECK(names[5] == "studio");

  // The sky is the defaults; any look over another gives the same settings as over the defaults.
  Render::Settings<T> sky;

  Render::setLook(sky, Render::Look::Blueprint);
  Render::setLook(sky, Render::Look::Sky);

  CHECK(Render::toCode(Render::Camera<T>(), sky) == Render::toCode(Render::Camera<T>(), Render::Settings<T>()));

  std::sort(names.begin(), names.end());
  std::sort(codes.begin(), codes.end());

  CHECK(std::unique(names.begin(), names.end()) == names.end());
  CHECK(std::unique(codes.begin(), codes.end()) == codes.end());
}

TEMPLATE_TEST_CASE("Render: on every look, the section plane's lines of equal distance stand out from the plane",
                   "[Render]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // A torus cut in half through its ring, seen from straight above: most of the plane within its box lies outside it,
  // under the sheet and over the backdrop.
  const TorusSDF<T> torus(Vec3::zeros(), T(1), T(0.2));

  constexpr uint32_t size = 48;

  const auto camera = Render::Camera<T>::fit(torus.computeBoundingVolume(), Render::View::Top);

  for (uint8_t k = 0; k < Render::NumLooks; k++) {
    Render::Settings<T> settings;

    Render::setLook(settings, Render::Look(k));

    settings.m_width         = size;
    settings.m_height        = size;
    settings.m_supersampling = 1;
    settings.m_threads       = 3;
    settings.m_floor         = false;
    settings.m_section       = true;
    settings.m_sectionNormal = Vec3(T(0), T(1), T(0));
    settings.m_contourWidth  = T(0);

    Render::Image without;
    Render::Image with;

    (void)Render::Renderer<T>(settings).render(torus, camera, without);

    settings.m_isoSpacing = T(0.15);

    (void)Render::Renderer<T>(settings).render(torus, camera, with);

    // The pixels the lines changed, by how much: the median is that of a line outside the torus.
    std::vector<int> changes;

    for (size_t i = 0; i < with.m_pixels.size(); i++) {
      const Channels a = channels(without.m_pixels[i]);
      const Channels b = channels(with.m_pixels[i]);
      const int      d = std::abs(a.r - b.r) + std::abs(a.g - b.g) + std::abs(a.b - b.b);

      if (d > 0) {
        changes.push_back(d);
      }
    }

    REQUIRE(changes.size() > 20);

    std::sort(changes.begin(), changes.end());

    INFO("look " << Render::lookName(Render::Look(k)));
    CHECK(changes[changes.size() / 2] > 90);
  }
}
