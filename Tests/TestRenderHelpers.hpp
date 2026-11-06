// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Test helpers shared by the render tests (TestRender, TestRenderDevice): small exact-to-check settings, a
// pixel's color channels, and the ray through a pixel's center.

#ifndef EBGEOMETRY_TESTRENDERHELPERS_HPP
#define EBGEOMETRY_TESTRENDERHELPERS_HPP

#include "EBGeometry.hpp"

#include <cmath>
#include <cstdint>

namespace EBGeometryTestRender {

using namespace EBGeometry;

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

// The color channels of a pixel.
struct Channels
{
  int r;
  int g;
  int b;
};

inline Channels
channels(const uint32_t a_rgba)
{
  return Channels{int((a_rgba >> 24) & 0xFFU), int((a_rgba >> 16) & 0xFFU), int((a_rgba >> 8) & 0xFFU)};
}

// The direction of the ray through the center of pixel (column, row), as PixelQuery computes it.
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

} // namespace EBGeometryTestRender

#endif
