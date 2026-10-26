// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Scratch: a sphere tracer over any shape with signedDistance(), shared by render.cpp and viewer.cpp.
// Everything a pixel needs is EBGEOMETRY_HOST_DEVICE and trivially copyable, so the same Pixel functor
// runs in an OpenMP loop on the host or through GPU::transform() on a device.
#ifndef SCRATCH_TRACER_HPP
#define SCRATCH_TRACER_HPP

#include "EBGeometry.hpp"

#include <cmath>
#include <cstdint>
#include <vector>

namespace Tracer {

using T    = float;
using Vec3 = EBGeometry::Vec3T<T>;

namespace Math = EBGeometry::Math;

EBGEOMETRY_HOST_DEVICE
inline Vec3
normalized(const Vec3& v) noexcept
{
  return v / v.length();
}

// A camera orbiting a target point: yaw and pitch in degrees, distance from the target.
struct Orbit
{
  T    yaw      = 35;
  T    pitch    = 28;
  T    distance = T(5.2);
  T    focal    = T(2.6);
  Vec3 target   = Vec3::zeros();

  Vec3 eye, fwd, right, up;

  void
  update()
  {
    const T y = yaw * T(M_PI / 180);
    const T p = pitch * T(M_PI / 180);

    eye   = target + distance * Vec3(std::cos(p) * std::sin(y), std::sin(p), std::cos(p) * std::cos(y));
    fwd   = normalized(target - eye);
    right = normalized(EBGeometry::cross(fwd, Vec3(0, 1, 0)));
    up    = EBGeometry::cross(right, fwd);
  }
};

struct Settings
{
  bool shadows   = true;
  bool occlusion = true;
  bool floor     = true;
  T    floorY    = T(-1.2);

  // Far from the shape, step by the distance to its bounding box instead (a cheaper lower bound).
  bool boxBound  = false;
  T    boxMargin = T(0.1);
  Vec3 boxLo     = Vec3::zeros();
  Vec3 boxHi     = Vec3::zeros();
};

// The shape's colour at a surface point: one colour everywhere unless a Tint says otherwise.
struct NoTint
{
  EBGEOMETRY_HOST_DEVICE
  Vec3
  albedo(const Vec3&, const Vec3& a_default) const noexcept
  {
    return a_default;
  }
};

// A shape (held by value: a Tape is a trivially copyable descriptor), lit by one directional light.
template <class Shape, class Tint = NoTint>
struct Scene
{
  Shape    m_shape;
  Settings m_settings;
  Tint     m_tint;

  // The shape, plus a floor plane that exists only in the renderer. a_exact is false when the value is
  // only the distance to the shape's bounding box (a cheaper lower bound, used far from the box).
  EBGEOMETRY_HOST_DEVICE
  T
  sdf(const Vec3& p, bool& a_exact) const noexcept
  {
    T d;

    a_exact = true;

    if (m_settings.boxBound) {
      const Vec3 q = max(max(m_settings.boxLo - p, p - m_settings.boxHi), Vec3::zeros());
      const T    b = q.length();

      a_exact = !(b > m_settings.boxMargin);
      d       = a_exact ? m_shape.signedDistance(p) : b;
    }
    else {
      d = m_shape.signedDistance(p);
    }

    if (m_settings.floor && p[1] - m_settings.floorY < d) {
      a_exact = true;

      return p[1] - m_settings.floorY;
    }

    return d;
  }

  EBGEOMETRY_HOST_DEVICE
  T
  sdf(const Vec3& p) const noexcept
  {
    bool exact;

    return this->sdf(p, exact);
  }

  // The colour of the ray from the camera through image-plane coordinates (u, v), v in [-1, 1].
  EBGEOMETRY_HOST_DEVICE
  Vec3
  shade(const Orbit& a_cam, const T u, const T v) const noexcept
  {
    const Vec3 light = normalized(Vec3(0.7, 0.8, 0.3));
    const Vec3 dir   = normalized(u * a_cam.right + v * a_cam.up + a_cam.focal * a_cam.fwd);

    Vec3 c = (T(1) - T(0.5) * (v + 1)) * Vec3(0.80, 0.82, 0.86) + T(0.5) * (v + 1) * Vec3(0.30, 0.42, 0.62);

    T t = 0;

    if (this->march(a_cam.eye, dir, t)) {
      const Vec3 p = a_cam.eye + t * dir;
      const Vec3 n = this->normal(p);
      const Vec3 h = normalized(light - dir);

      const T lit  = m_settings.shadows ? this->shadow(p + T(1e-3) * n, light) : T(1);
      const T diff = Math::max(T(0), dot(n, light)) * lit;
      const T spec = std::pow(Math::max(T(0), dot(n, h)), T(48)) * diff;
      const T sky  = T(0.5) + T(0.5) * n[1];
      const T ao   = m_settings.occlusion ? this->occlusion(p, n) : T(1);

      const bool onFloor = m_settings.floor && p[1] < m_settings.floorY + T(1e-2);
      const Vec3 albedo  = onFloor ? Vec3(0.55, 0.56, 0.58) : m_tint.albedo(p, Vec3(0.62, 0.20, 0.06));

      c = albedo * (T(1.1) * diff + T(0.35) * sky * ao) + T(0.45) * spec * Vec3::ones();
    }

    return c;
  }

  // Sphere tracing: step by the distance bound until close to the surface.
  EBGEOMETRY_HOST_DEVICE
  bool
  march(const Vec3& o, const Vec3& d, T& t) const noexcept
  {
    for (int i = 0; i < 160 && t < T(40); i++) {
      const T h = this->sdf(o + t * d);

      if (h < T(1e-4) * t) {
        return true;
      }

      t += h;
    }

    return false;
  }

  EBGEOMETRY_HOST_DEVICE
  Vec3
  normal(const Vec3& p) const noexcept
  {
    const T    e = T(5e-4);
    const Vec3 a(1, -1, -1), b(-1, -1, 1), c(-1, 1, -1), d(1, 1, 1);

    return normalized(a * sdf(p + e * a) + b * sdf(p + e * b) + c * sdf(p + e * c) + d * sdf(p + e * d));
  }

  // Soft shadow toward the light, from the closest approach of the shadow ray.
  EBGEOMETRY_HOST_DEVICE
  T
  shadow(const Vec3& o, const Vec3& d) const noexcept
  {
    T s = 1;
    T t = T(0.02);

    for (int i = 0; i < 64 && t < T(10); i++) {
      bool    exact;
      const T h = this->sdf(o + t * d, exact);

      if (h < T(1e-4)) {
        return T(0);
      }

      // The penumbra needs the true distance; a box distance only moves the ray along.
      if (exact) {
        s = Math::min(s, T(10) * h / t);
      }
      t += Math::clamp(h, T(0.01), T(0.3));
    }

    return Math::clamp(s, T(0), T(1));
  }

  // Ambient occlusion from five samples along the normal.
  EBGEOMETRY_HOST_DEVICE
  T
  occlusion(const Vec3& p, const Vec3& n) const noexcept
  {
    T occ = 0;
    T w   = 1;

    for (int i = 1; i <= 5; i++) {
      const T h = T(0.03) * T(i);

      occ += w * (h - this->sdf(p + h * n));
      w *= T(0.7);
    }

    return Math::clamp(T(1) - T(4) * occ, T(0), T(1));
  }
};

// One pixel of a w x h image with ss x ss supersampling, as RGBA8888: the query GPU::transform() runs
// once per pixel index.
template <class Shape, class Tint = NoTint>
struct Pixel
{
  Scene<Shape, Tint> m_scene;
  Orbit        m_cam;
  int          m_w;
  int          m_h;
  int          m_ss;

  EBGEOMETRY_HOST_DEVICE
  uint32_t
  operator()(const uint32_t& a_index) const noexcept
  {
    const int i = int(a_index % uint32_t(m_w));
    const int j = int(a_index / uint32_t(m_w));

    Vec3 color = Vec3::zeros();

    for (int s = 0; s < m_ss * m_ss; s++) {
      const T di = (T(s % m_ss) + T(0.5)) / T(m_ss);
      const T dj = (T(s / m_ss) + T(0.5)) / T(m_ss);
      const T u  = (T(2) * (T(i) + di) - T(m_w)) / T(m_h);
      const T v  = (T(m_h) - T(2) * (T(j) + dj)) / T(m_h);

      color += m_scene.shade(m_cam, u, v);
    }

    color = color / T(m_ss * m_ss);

    uint32_t rgba = 0xFF;

    for (int k = 0; k < 3; k++) {
      const T g = std::pow(Math::clamp(color[k], T(0), T(1)), T(1) / T(2.2));

      rgba |= uint32_t(T(255) * g + T(0.5)) << (24 - 8 * k);
    }

    return rgba;
  }
};

// Renders on the host, one OpenMP thread per row.
template <class Shape, class Tint>
void
renderHost(const Pixel<Shape, Tint>& a_pixel, std::vector<uint32_t>& a_pixels)
{
  const int n = a_pixel.m_w * a_pixel.m_h;

  a_pixels.resize(size_t(n));

#pragma omp parallel for schedule(dynamic, 256)
  for (int k = 0; k < n; k++) {
    a_pixels[size_t(k)] = a_pixel(uint32_t(k));
  }
}

// Renders through GPU::transform(): a kernel in a CUDA or HIP build, a host loop otherwise. a_indices
// holds 0, 1, ..., w*h - 1.
template <class Shape, class Tint>
void
renderDevice(const Pixel<Shape, Tint>& a_pixel, const std::vector<uint32_t>& a_indices, std::vector<uint32_t>& a_pixels)
{
  a_pixels.resize(a_indices.size());

  EBGeometry::GPU::transform(a_pixel, a_indices, a_pixels);
}

// The classic CSG solid: a cube with its corners cut by a ball, drilled through along x, y and z.
inline EBGeometry::Expr<T>
classicCSG()
{
  const EBGeometry::Expr<T> cube = EBGeometry::BoxSDF<T>(Vec3(-1, -1, -1), Vec3(1, 1, 1));
  const EBGeometry::Expr<T> ball = EBGeometry::SphereSDF<T>(Vec3::zeros(), T(1.35));

  std::vector<EBGeometry::Expr<T>> drills;

  for (size_t axis = 0; axis < 3; axis++) {
    drills.push_back(EBGeometry::InfiniteCylinderSDF<T>(Vec3::zeros(), T(0.55), axis));
  }

  return EBGeometry::Difference(EBGeometry::Intersection(cube, ball), EBGeometry::Union(drills));
}

} // namespace Tracer

#endif
