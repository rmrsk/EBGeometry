// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Test helpers shared by the ray tests (TestRayAnalytic, TestRayMesh, TestRayCSG, TestRayOffsets): a slab
// test of the unit cube, a fine mesh of a cube, user shapes with and without ray forms, a marched wrapper,
// compositions with hand-computed crossings, the device query functors and the rays they are checked on.

#ifndef EBGEOMETRY_TESTRAYHELPERS_HPP
#define EBGEOMETRY_TESTRAYHELPERS_HPP

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

namespace EBGeometryTestRay {

using namespace EBGeometry;

inline std::string
unitCube()
{
  return std::string(EBGEOMETRY_TEST_DATA_DIR) + "/cube_quads.obj";
}

// The reference: where a ray first crosses the surface of the closed box [0,1]^3 in [tMin, tMax], from
// either side, by the slab test. Returns +infinity for no crossing. A ray lying in a face's plane is
// outside the open slab on that axis and inside the closed one: it counts as inside, so it crosses where it
// enters or leaves along the other axes.
template <class T>
T
slabCrossing(const Vec3T<T>& a_origin, const Vec3T<T>& a_direction, const T a_tMin, const T a_tMax)
{
  T in  = -std::numeric_limits<T>::infinity();
  T out = std::numeric_limits<T>::infinity();

  for (size_t axis = 0; axis < 3; axis++) {
    const T o = a_origin[axis];
    const T d = a_direction[axis];

    if (d == T(0)) {
      if (o < T(0) || o > T(1)) {
        return std::numeric_limits<T>::infinity();
      }
    }
    else {
      const T t0 = (T(0) - o) / d;
      const T t1 = (T(1) - o) / d;

      in  = std::max(in, std::min(t0, t1));
      out = std::min(out, std::max(t0, t1));
    }
  }

  if (in > out) {
    return std::numeric_limits<T>::infinity();
  }
  if (in >= a_tMin && in <= a_tMax) {
    return in;
  }
  if (out >= a_tMin && out <= a_tMax) {
    return out;
  }

  return std::numeric_limits<T>::infinity();
}

// A cube of side a_side at a_corner, each face an a_n x a_n grid of quadrilaterals, counter-clockwise seen
// from outside: a fine mesh far from the origin, where rounding in a ray's crossing point is large next
// to the faces.
template <class T>
PolygonSoup<T>
gridCube(const Vec3T<T>& a_corner, const T a_side, const size_t a_n)
{
  std::vector<Vec3T<T>>            vertices;
  std::vector<std::vector<size_t>> faces;

  // Each face: a fixed axis and side, and two in-plane axes (u, v) with u x v pointing outwards.
  struct Side
  {
    size_t axis;
    T      at;
    size_t u;
    size_t v;
  };

  const Side sides[6] = {
    {0, T(0), 2, 1}, {0, T(1), 1, 2}, {1, T(0), 0, 2}, {1, T(1), 2, 0}, {2, T(0), 1, 0}, {2, T(1), 0, 1}};

  for (const Side& side : sides) {
    const size_t first = vertices.size();

    for (size_t j = 0; j <= a_n; j++) {
      for (size_t i = 0; i <= a_n; i++) {
        Vec3T<T> p;

        p[side.axis] = side.at;
        p[side.u]    = T(i) / T(a_n);
        p[side.v]    = T(j) / T(a_n);

        vertices.push_back(a_corner + a_side * p);
      }
    }

    for (size_t j = 0; j < a_n; j++) {
      for (size_t i = 0; i < a_n; i++) {
        const size_t a = first + j * (a_n + 1) + i;

        faces.push_back({a, a + 1, a + a_n + 2, a + a_n + 1});
      }
    }
  }

  return PolygonSoup<T>("grid cube", vertices, faces);
}

// A shape with no ray form, so the queries march it: what the ray forms are compared with.
template <class T, class S>
struct Marched
{
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  S m_shape;

  EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    return m_shape.signedDistance(a_point);
  }

  EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    return m_shape.computeBoundingVolume();
  }
};

template <class T>
Vec3T<T>
normalized(const Vec3T<T>& a_v)
{
  return a_v / a_v.length();
}

template <class T>
Vec3T<T>
unit(const T a_x, const T a_y, const T a_z)
{
  const Vec3T<T> v(a_x, a_y, a_z);

  return v / v.length();
}

// The solid of the BasicCSG example: a cube with its corners rounded by a ball, drilled along each axis.
template <class T>
struct Part
{
  BoxSDF<T>      cube{-Vec3T<T>::ones(), Vec3T<T>::ones()};
  SphereSDF<T>   ball{Vec3T<T>::zeros(), T(1.35)};
  CylinderSDF<T> drillX{Vec3T<T>(-T(1.5), T(0), T(0)), Vec3T<T>(T(1.5), T(0), T(0)), T(0.55)};
  CylinderSDF<T> drillY{Vec3T<T>(T(0), -T(1.5), T(0)), Vec3T<T>(T(0), T(1.5), T(0)), T(0.55)};
  CylinderSDF<T> drillZ{Vec3T<T>(T(0), T(0), -T(1.5)), Vec3T<T>(T(0), T(0), T(1.5)), T(0.55)};

  auto
  fused() const
  {
    return Fused::Difference(Fused::Intersection(cube, ball), drillX, drillY, drillZ);
  }

  Expr<T>
  expr() const
  {
    return Difference(Intersection(cube, ball), Union(std::vector<Expr<T>>{drillX, drillY, drillZ}));
  }
};

// The distance to the first crossing, or -1 for none: comparable between host and device.
template <class T, class Shape>
struct FirstCrossing
{
  Shape m_shape;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const RayT<T>& a_ray) const noexcept
  {
    const HitT<T> hit = firstHit(m_shape, a_ray, T(0), T(10));

    return hit.hit() ? hit.m_t : -T(1);
  }
};

// The distance to the first crossing by firstCrossing(), or -1 for none: comparable between host and device.
template <class T, class Shape>
struct CrossingDistance
{
  Shape m_shape;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const RayT<T>& a_ray) const noexcept
  {
    const CrossingT<T> crossing = firstCrossing(m_shape, a_ray, T(0), T(10));

    return crossing.hit() ? crossing.m_t : -T(1);
  }
};

// A ball whose ray form has only firstHit(): a composition finds its spans crossing by crossing.
template <class T>
struct FirstHitBall
{
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  SphereSDF<T> m_sphere;

  EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    return m_sphere.signedDistance(a_point);
  }

  EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    return m_sphere.computeBoundingVolume();
  }
};

// A ball without a ray form, so marched, that counts its evaluations at x in (m_lo, m_hi).
template <class T>
struct ProbedBall
{
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  SphereSDF<T> m_sphere;
  T            m_lo;
  T            m_hi;
  int*         m_count;

  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    if (a_point[0] > m_lo && a_point[0] < m_hi) {
      (*m_count)++;
    }

    return m_sphere.signedDistance(a_point);
  }

  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    return m_sphere.computeBoundingVolume();
  }
};

// A ball whose ray form never decides: each firstHit() is unresolved a step past where it starts.
template <class T>
struct CreepingBall : FirstHitBall<T>
{
  T m_step;
};

// A ball whose ray form's firstHit() lacks the options argument: it is not a ray form, and the ball is marched.
template <class T>
struct MistypedBall : FirstHitBall<T>
{
};

// Where a ray is inside a line of balls along the x axis, as intervals of x: each ball's [cx - r, cx + r]
// on the axis.
template <class T>
std::vector<std::pair<T, T>>
ballRow(const size_t a_count, const T a_spacing, const T a_radius)
{
  std::vector<std::pair<T, T>> row;

  for (size_t i = 0; i < a_count; i++) {
    const T cx = -T(1) + a_spacing * T(i);

    row.emplace_back(cx - a_radius, cx + a_radius);
  }

  return row;
}

// The reference for a traced crossing: the shape's value sampled along the ray. A crossing is on the
// surface, the ray stays on one side of it before it (the outside if it enters there), and a miss changes
// side nowhere. A_skip is how far before the crossing the samples stop, so that the sample next to the
// crossing does not round to its other side.
template <class T, class S>
void
requireSampledAgreement(
  const S& a_shape, const RayT<T>& a_ray, const T a_tMax, const HitT<T>& a_hit, const size_t a_samples, const T a_skip)
{
  REQUIRE(a_hit.m_status != HitStatus::Unresolved);

  const T end = a_hit.hit() ? a_hit.m_t - a_skip : a_tMax;

  if (a_hit.hit()) {
    REQUIRE_THAT(a_shape.signedDistance(a_hit.m_point), withinAbsT(T(0), 10 * looseMargin<T>()));

    // The normal is the leaf's, found again from the leaf the trace ended on: it must be the gradient of the
    // shape's value there (where the gradient is one: not on an edge between two leaves).
    const T  h = std::is_same_v<T, float> ? T(1e-3) : T(1e-6);
    Vec3T<T> gradient;

    for (size_t dir = 0; dir < 3; dir++) {
      Vec3T<T> step = Vec3T<T>::zeros();

      step[dir] = h;

      gradient[dir] =
        (a_shape.signedDistance(a_hit.m_point + step) - a_shape.signedDistance(a_hit.m_point - step)) / (T(2) * h);
    }

    if (std::abs(gradient.length() - T(1)) < T(1e-2)) {
      REQUIRE(dot(a_hit.m_normal, gradient / gradient.length()) > T(0.99));
    }
  }

  if (!(end > T(0))) {
    return;
  }

  const bool inside = a_shape.signedDistance(a_ray.at(T(0))) < T(0);

  for (size_t k = 1; k <= a_samples; k++) {
    const T t = end * T(k) / T(a_samples);

    REQUIRE((a_shape.signedDistance(a_ray.at(t)) < T(0)) == inside);
  }

  if (a_hit.hit()) {
    REQUIRE(a_hit.m_entering == !inside);
  }
}

// A scene of balls and boxes in [-1, 1]^3, and rays through it from outside and from inside.
template <class T>
struct Scene
{
  std::vector<SphereSDF<T>> balls;
  std::vector<BoxSDF<T>>    boxes;
  std::vector<SphereSDF<T>> holes;
  std::vector<RayT<T>>      rays;

  explicit Scene(const unsigned a_seed)
  {
    std::mt19937                      rng(a_seed);
    std::uniform_real_distribution<T> u(-T(1), T(1));

    for (int i = 0; i < 6; i++) {
      balls.emplace_back(Vec3T<T>(u(rng), u(rng), u(rng)), T(0.25) + T(0.15) * u(rng));
    }

    for (int i = 0; i < 6; i++) {
      const Vec3T<T> c(u(rng), u(rng), u(rng));
      const Vec3T<T> h(T(0.2) + T(0.1) * u(rng), T(0.2) + T(0.1) * u(rng), T(0.2) + T(0.1) * u(rng));

      boxes.emplace_back(c - h, c + h);
    }

    for (int i = 0; i < 4; i++) {
      holes.emplace_back(Vec3T<T>(u(rng), u(rng), u(rng)), T(0.3) + T(0.1) * u(rng));
    }

    for (int i = 0; i < 80; i++) {
      // From outside, at a point near the middle; from inside, in any direction.
      const Vec3T<T> from = (i % 2 == 0) ? T(3) * unit(u(rng), u(rng), u(rng)) : Vec3T<T>(u(rng), u(rng), u(rng));
      const Vec3T<T> to(T(0.5) * u(rng), T(0.5) * u(rng), T(0.5) * u(rng));

      rays.push_back(RayT<T>{from, (i % 2 == 0) ? normalized(to - from) : unit(u(rng), u(rng), u(rng))});
    }
  }

  std::vector<Expr<T>>
  solids() const
  {
    std::vector<Expr<T>> e;

    for (const auto& b : balls) {
      e.emplace_back(b);
    }

    for (const auto& b : boxes) {
      e.emplace_back(b);
    }

    return e;
  }

  std::vector<Expr<T>>
  drills() const
  {
    return std::vector<Expr<T>>(holes.begin(), holes.end());
  }
};
// Rays from a sphere of radius 3 towards points near the origin, for the device checks.
template <class T>
std::vector<RayT<T>>
deviceRays()
{
  using Vec3 = Vec3T<T>;

  std::vector<RayT<T>> rays;

  for (const Vec3& target : EBGeometryTestGPU::queryGrid<T>(-Vec3::ones(), Vec3::ones(), 5)) {
    const Vec3 origin(T(3) * std::cos(target[0] * T(7)), T(3) * std::sin(target[0] * T(7)), T(1.3) * target[2]);

    rays.push_back(RayT<T>{origin, normalized(target - origin)});
  }

  return rays;
}

} // namespace EBGeometryTestRay

namespace EBGeometry {

// FirstHitBall's ray form: the sphere's first crossing, and nothing else.
template <class T>
struct RayForm<EBGeometryTestRay::FirstHitBall<T>>
{
  EBGEOMETRY_HOST_DEVICE
  static HitT<T>
  firstHit(const EBGeometryTestRay::FirstHitBall<T>& a_shape,
           const RayT<T>&                            a_ray,
           T                                         a_tMin,
           T                                         a_tMax,
           const RayOptions<T>&                      a_options) noexcept
  {
    return RayForm<SphereSDF<T>>::firstHit(a_shape.m_sphere, a_ray, a_tMin, a_tMax, a_options);
  }
};

template <class T>
struct RayForm<EBGeometryTestRay::MistypedBall<T>>
{
  EBGEOMETRY_HOST_DEVICE
  static HitT<T>
  firstHit(const EBGeometryTestRay::MistypedBall<T>& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax) noexcept
  {
    return RayForm<SphereSDF<T>>::firstHit(a_shape.m_sphere, a_ray, a_tMin, a_tMax, RayOptions<T>());
  }
};

template <class T>
struct RayForm<EBGeometryTestRay::CreepingBall<T>>
{
  EBGEOMETRY_HOST_DEVICE
  static HitT<T>
  firstHit(const EBGeometryTestRay::CreepingBall<T>& a_shape,
           const RayT<T>&                            a_ray,
           T                                         a_tMin,
           T                                         a_tMax,
           const RayOptions<T>&                      a_options) noexcept
  {
    (void)a_options;

    HitT<T> hit;

    hit.m_status = HitStatus::Unresolved;
    hit.m_t      = Math::min(a_tMin + a_shape.m_step, a_tMax);
    hit.m_point  = a_ray.at(hit.m_t);

    return hit;
  }
};

} // namespace EBGeometry

#endif
