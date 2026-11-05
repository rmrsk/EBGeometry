// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_SphereAoSoAImplem.hpp
 * @brief  Implementation of EBGeometry_SphereAoSoA.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_SPHEREAOSOAIMPLEM_HPP
#define EBGEOMETRY_SPHEREAOSOAIMPLEM_HPP

// Std includes
#include <cmath>
#include <cstddef>
#include <cstdint>

#include "EBGeometry_Array.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_SphereAoSoA.hpp"

namespace EBGeometry {

template <class T, size_t W>
EBGEOMETRY_HOST
void
SphereAoSoA<T, W>::pack(const SphereCloud::Sphere<T>* a_spheres, const uint32_t* a_sphereIds, uint32_t a_count) noexcept
{
  EBGEOMETRY_REQUIRE(a_spheres != nullptr && a_sphereIds != nullptr,
                     "SphereAoSoA::pack: the sphere and sphere id arrays must not be null");
  EBGEOMETRY_REQUIRE(a_count >= 1U && a_count <= W,
                     "SphereAoSoA::pack: the sphere count must be between 1 and %zu (%u)",
                     W,
                     unsigned(a_count));

  for (uint32_t j = 0; j < a_count; j++) {
    const SphereCloud::Sphere<T>& sphere = a_spheres[j];

    EBGEOMETRY_REQUIRE(std::isfinite(sphere.m_center[0]) && std::isfinite(sphere.m_center[1]) &&
                         std::isfinite(sphere.m_center[2]) && std::isfinite(sphere.m_radius) && sphere.m_radius >= T(0),
                       "SphereAoSoA::pack: sphere %u needs a finite center and a finite radius of at least 0",
                       unsigned(j));
  }

  this->packUnchecked(a_spheres, a_sphereIds, a_count);
}

template <class T, size_t W>
EBGEOMETRY_HOST_DEVICE
void
SphereAoSoA<T, W>::packUnchecked(const SphereCloud::Sphere<T>* a_spheres,
                                 const uint32_t*               a_sphereIds,
                                 uint32_t                      a_count) noexcept
{
  EBGEOMETRY_EXPECT(a_spheres != nullptr);
  EBGEOMETRY_EXPECT(a_count >= 1U && a_count <= W);

  // Same padding convention as PointAoSoA::pack(): lanes count..W-1 repeat the last real sphere. The last index is
  // clamped at 0 so that an optimizer that cannot see a_count >= 1 finds no out-of-bounds subscript.
  const uint32_t last = (a_count > 0U) ? a_count - 1U : 0U;

  // Zeroed, so that an optimizer that cannot see a_count >= 1 reads nothing uninitialized.
  Array<Vec3T<T>, W> centers{};

  for (uint32_t j = 0; j < W; j++) {
    const SphereCloud::Sphere<T>& sphere = a_spheres[Math::min(j, last)];

    centers[j] = sphere.m_center;
    m_radii[j] = sphere.m_radius;
  }

  m_centers.packUnchecked(centers.data(), a_sphereIds, a_count);
}

template <class T, size_t W>
EBGEOMETRY_HOST_DEVICE
Array<T, W>
SphereAoSoA<T, W>::getSignedDistances(const Vec3T<T>& a_point) const noexcept
{
  Array<T, W> distances = m_centers.getDistances(a_point);

  for (size_t j = 0; j < W; j++) {
    distances[j] -= m_radii[j];
  }

  return distances;
}

template <class T, size_t W>
EBGEOMETRY_HOST_DEVICE
Array<T, W>
SphereAoSoA<T, W>::getSeparations(const Vec3T<T>& a_center, const T a_radius) const noexcept
{
  Array<T, W> separations = m_centers.getDistances(a_center);

  for (size_t j = 0; j < W; j++) {
    separations[j] -= a_radius + m_radii[j];
  }

  return separations;
}

template <class T, size_t W>
EBGEOMETRY_HOST_DEVICE
void
SphereAoSoA<T, W>::rayIntervals(const Vec3T<T>& a_origin,
                                const Vec3T<T>& a_direction,
                                Array<T, W>&    a_entry,
                                Array<T, W>&    a_exit) const noexcept
{
  const uint32_t numValid = m_centers.numValid();

  // The arithmetic of SphereSDF's ray form, lane by lane, so that a sphere in a cloud is crossed where the same
  // sphere alone is (to rounding: an optimized build may fuse multiply-adds differently in the two): the
  // discriminant is r^2 less the squared distance of the center from the line, |oc x d|^2 (not b^2 - c, which
  // cancels for a small sphere far from the origin), and the nearer root is c / q.
  for (uint32_t j = 0; j < W; j++) {
    const Vec3T<T> oc   = a_origin - m_centers.getPosition(j);
    const Vec3T<T> l    = cross(oc, a_direction);
    const T        b    = dot(oc, a_direction);
    const T        r    = m_radii[j];
    const T        disc = r * r - dot(l, l);

    if (j >= numValid || disc < T(0)) {
      a_entry[j] = Math::Limits<T>::infinity();
      a_exit[j]  = -Math::Limits<T>::infinity();
    }
    else {
      const T root = std::sqrt(disc);
      const T q    = -b - ((b >= T(0)) ? root : -root);
      const T c    = dot(oc, oc) - r * r;

      // With q = 0 both roots are zero: the ray starts on the surface, along it.
      const T nearer = (q == T(0)) ? T(0) : c / q;

      a_entry[j] = Math::min(q, nearer);
      a_exit[j]  = Math::max(q, nearer);
    }
  }
}

template <class T, size_t W>
EBGEOMETRY_HOST_DEVICE
uint32_t
SphereAoSoA<T, W>::getSphereId(size_t a_lane) const noexcept
{
  return m_centers.getPointId(a_lane);
}

template <class T, size_t W>
template <class BV>
EBGEOMETRY_HOST_DEVICE
BV
SphereAoSoA<T, W>::computeBoundingVolume() const noexcept
{
  EBGEOMETRY_EXPECT(this->numValid() >= 1U);

  Vec3T<T> lo = +Vec3T<T>::max();
  Vec3T<T> hi = -Vec3T<T>::max();

  for (uint32_t j = 0; j < this->numValid(); j++) {
    const Vec3T<T> center = m_centers.getPosition(j);
    const Vec3T<T> radius = m_radii[j] * Vec3T<T>::ones();

    lo = min(lo, center - radius);
    hi = max(hi, center + radius);
  }

  return BV(lo, hi);
}

} // namespace EBGeometry

#endif
