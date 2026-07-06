// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_BoundingVolumesImplem.hpp
 * @brief  Inline implementations of BoundingSphereT<T> and AABBT<T>.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_BOUNDINGVOLUMESIMPLEM_HPP
#define EBGEOMETRY_BOUNDINGVOLUMESIMPLEM_HPP

// Std includes
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <vector>

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_Constants.hpp"

namespace EBGeometry {

namespace BoundingVolumes {

template <class T>
inline BoundingSphereT<T>::BoundingSphereT(const Vec3T<T>& a_center, const T& a_radius) noexcept
{
  EBGEOMETRY_EXPECT(a_radius >= T(0));
  EBGEOMETRY_EXPECT(std::isfinite(a_center[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_center[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_center[2]));

  m_center = a_center;
  m_radius = a_radius;
}

template <class T>
BoundingSphereT<T>::BoundingSphereT(const BoundingSphereT& a_other) noexcept
{
  EBGEOMETRY_EXPECT(a_other.m_radius >= T(0));

  m_radius = a_other.m_radius;
  m_center = a_other.m_center;
}

template <class T>
BoundingSphereT<T>::BoundingSphereT(const std::vector<BoundingSphereT<T>>& a_otherSpheres) noexcept
{
  EBGEOMETRY_EXPECT(!a_otherSpheres.empty());

  // Enclosing a set of spheres is a hard problem. We approximate by constructing
  // the AABB corners for each input sphere, then fitting a sphere to those points.
  std::vector<Vec3T<T>> points;
  points.reserve(2 * a_otherSpheres.size());

  for (const auto& sphere : a_otherSpheres) {
    EBGEOMETRY_EXPECT(sphere.m_radius >= T(0));

    const T&        radius = sphere.getRadius();
    const Vec3T<T>& center = sphere.getCentroid();

    points.emplace_back(center + radius * Vec3T<T>::one());
    points.emplace_back(center - radius * Vec3T<T>::one());
  }

  this->define(points, BoundingVolumeAlgorithm::Ritter);
}

template <class T>
template <class P>
BoundingSphereT<T>::BoundingSphereT(const std::vector<Vec3T<P>>&   a_points,
                                    const BoundingVolumeAlgorithm& a_algorithm) noexcept
{
  EBGEOMETRY_EXPECT(!a_points.empty());

  this->define(a_points, a_algorithm);
}

template <class T>
BoundingSphereT<T>::~BoundingSphereT() noexcept
{}

template <class T>
template <class P>
inline void
BoundingSphereT<T>::define(const std::vector<Vec3T<P>>& a_points, const BoundingVolumeAlgorithm& a_algorithm) noexcept
{
  EBGEOMETRY_EXPECT(!a_points.empty());

  switch (a_algorithm) {
  case BoundingVolumeAlgorithm::Ritter: {
    this->buildRitter(a_points);

    break;
  }
  default: {
    std::cerr << "BoundingSphereT::define - unsupported algorithm requested\n";
  }
  }
}

template <class T>
inline bool
BoundingSphereT<T>::intersects(const BoundingSphereT& a_other) const noexcept
{
  EBGEOMETRY_EXPECT(m_radius >= T(0));
  EBGEOMETRY_EXPECT(a_other.m_radius >= T(0));

  // Two spheres intersect if and only if the distance between their centres is
  // less than the sum of their radii: |c1 - c2| < r1 + r2.
  // Squaring both sides avoids a square root: |c1 - c2|^2 < (r1 + r2)^2.
  const Vec3 deltaV = m_center - a_other.getCentroid();
  const T    sumR   = m_radius + a_other.getRadius();

  return deltaV.dot(deltaV) < sumR * sumR;
}

template <class T>
inline T&
BoundingSphereT<T>::getRadius() noexcept
{
  return m_radius;
}

template <class T>
inline const T&
BoundingSphereT<T>::getRadius() const noexcept
{
  return m_radius;
}

template <class T>
inline Vec3T<T>&
BoundingSphereT<T>::getCentroid() noexcept
{
  return m_center;
}

template <class T>
inline const Vec3T<T>&
BoundingSphereT<T>::getCentroid() const noexcept
{
  return m_center;
}

template <class T>
inline T
BoundingSphereT<T>::getOverlappingVolume(const BoundingSphereT<T>& a_other) const noexcept
{
  EBGEOMETRY_EXPECT(m_radius >= T(0));
  EBGEOMETRY_EXPECT(a_other.m_radius >= T(0));

  constexpr T zero = T(0);

  if (!this->intersects(a_other)) {
    return zero;
  }

  const T r1 = m_radius;
  const T r2 = a_other.getRadius();
  const T d  = (m_center - a_other.getCentroid()).length();

  // Two spheres can intersect in one of two ways:
  //
  // Case 1: one sphere is entirely inside the other (d <= |r1 - r2|).
  //   The overlapping volume equals the volume of the smaller sphere.
  if (d <= std::abs(r1 - r2)) {
    const T rMin = std::min(r1, r2);

    return T(4.0 / 3.0) * pi<T> * rMin * rMin * rMin;
  }

  // Case 2: partial overlap (|r1 - r2| < d < r1 + r2).
  //   The intersection is a spherical lens. Its volume is given by the standard
  //   lens formula, which sums the two spherical cap volumes on either side of the
  //   intersection circle. See e.g. Weisstein, "Sphere-Sphere Intersection",
  //   MathWorld: V = pi/(12d) * (r1+r2-d)^2 * (d^2 + 2d(r1+r2) - 3(r1-r2)^2).
  return pi<T> / (T(12) * d) * (r1 + r2 - d) * (r1 + r2 - d) *
         (d * d + T(2) * d * (r1 + r2) - T(3) * (r1 - r2) * (r1 - r2));
}

template <class T>
inline T
BoundingSphereT<T>::getDistance(const Vec3& a_x0) const noexcept
{
  EBGEOMETRY_EXPECT(m_radius >= T(0));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[2]));

  // The distance from a_x0 to the sphere surface is |a_x0 - center| - radius.
  // This is negative when a_x0 is inside the sphere; clamping to zero gives the
  // unsigned distance, which is zero for interior points.
  return std::max(T(0), (a_x0 - m_center).length() - m_radius);
}

template <class T>
inline T
BoundingSphereT<T>::getVolume() const noexcept
{
  EBGEOMETRY_EXPECT(m_radius >= T(0));

  return T(4) * pi<T> * m_radius * m_radius * m_radius / T(3);
}

template <class T>
inline T
BoundingSphereT<T>::getArea() const noexcept
{
  EBGEOMETRY_EXPECT(m_radius >= T(0));

  return T(4) * pi<T> * m_radius * m_radius;
}

template <class T>
template <class P>
inline void
BoundingSphereT<T>::buildRitter(const std::vector<Vec3T<P>>& a_points) noexcept
{
  EBGEOMETRY_EXPECT(!a_points.empty());

  m_radius = T(0);
  m_center = Vec3::zero();

  constexpr T      half = T(0.5);
  constexpr size_t DIM  = 3;

  // For each axis find the points with the minimum and maximum coordinate.
  std::vector<Vec3> minPt(DIM, a_points[0]);
  std::vector<Vec3> maxPt(DIM, a_points[0]);

  for (size_t i = 1; i < a_points.size(); i++) {
    for (size_t dir = 0; dir < DIM; dir++) {
      if (a_points[i][dir] < minPt[dir][dir]) {
        minPt[dir] = a_points[i];
      }

      if (a_points[i][dir] > maxPt[dir][dir]) {
        maxPt[dir] = a_points[i];
      }
    }
  }

  // Choose the axis with the widest span as the initial diameter.
  T    dist = T(-1);
  Vec3 p1, p2;
  for (size_t dir = 0; dir < DIM; dir++) {
    const T len = (maxPt[dir] - minPt[dir]).length();

    if (len > dist) {
      dist = len;
      p1   = minPt[dir];
      p2   = maxPt[dir];
    }
  }

  m_center = (p1 + p2) * half;
  m_radius = half * (p2 - p1).length();

  // Expand the sphere to include any point that lies outside.
  for (size_t i = 0; i < a_points.size(); i++) {
    const T excess = (a_points[i] - m_center).length() - m_radius;

    if (excess > T(0)) {
      const Vec3 v = a_points[i] - m_center;
      p1           = a_points[i];
      p2           = m_center - m_radius * v / v.length();

      m_center = half * (p1 + p2);
      m_radius = half * (p1 - p2).length();
    }
  }

  // Ritter's algorithm is an approximation; grow slightly for safety.
  m_radius *= T(1) + T(1e-2);
}

template <class T>
AABBT<T>::AABBT(const Vec3T<T>& a_lo, const Vec3T<T>& a_hi) noexcept
{
  EBGEOMETRY_EXPECT(a_lo[0] <= a_hi[0]);
  EBGEOMETRY_EXPECT(a_lo[1] <= a_hi[1]);
  EBGEOMETRY_EXPECT(a_lo[2] <= a_hi[2]);

  m_loCorner = a_lo;
  m_hiCorner = a_hi;
}

template <class T>
AABBT<T>::AABBT(const AABBT<T>& a_other) noexcept
{
  EBGEOMETRY_EXPECT(a_other.m_loCorner[0] <= a_other.m_hiCorner[0]);
  EBGEOMETRY_EXPECT(a_other.m_loCorner[1] <= a_other.m_hiCorner[1]);
  EBGEOMETRY_EXPECT(a_other.m_loCorner[2] <= a_other.m_hiCorner[2]);

  m_loCorner = a_other.m_loCorner;
  m_hiCorner = a_other.m_hiCorner;
}

template <class T>
AABBT<T>::AABBT(const std::vector<AABBT<T>>& a_others) noexcept
{
  EBGEOMETRY_EXPECT(!a_others.empty());

  m_loCorner = a_others.front().getLowCorner();
  m_hiCorner = a_others.front().getHighCorner();

  for (const auto& other : a_others) {
    EBGEOMETRY_EXPECT(other.m_loCorner[0] <= other.m_hiCorner[0]);
    EBGEOMETRY_EXPECT(other.m_loCorner[1] <= other.m_hiCorner[1]);
    EBGEOMETRY_EXPECT(other.m_loCorner[2] <= other.m_hiCorner[2]);

    m_loCorner = min(m_loCorner, other.getLowCorner());
    m_hiCorner = max(m_hiCorner, other.getHighCorner());
  }
}

template <class T>
template <class P>
AABBT<T>::AABBT(const std::vector<Vec3T<P>>& a_points) noexcept
{
  EBGEOMETRY_EXPECT(!a_points.empty());

  this->define(a_points);
}

template <class T>
AABBT<T>::~AABBT() noexcept
{}

template <class T>
template <class P>
inline void
AABBT<T>::define(const std::vector<Vec3T<P>>& a_points) noexcept
{
  EBGEOMETRY_EXPECT(!a_points.empty());

  m_loCorner = a_points.front();
  m_hiCorner = a_points.front();

  for (const auto& p : a_points) {
    m_loCorner = min(m_loCorner, p);
    m_hiCorner = max(m_hiCorner, p);
  }
}

template <class T>
inline bool
AABBT<T>::intersects(const AABBT& a_other) const noexcept
{
  EBGEOMETRY_EXPECT(m_loCorner[0] <= m_hiCorner[0]);
  EBGEOMETRY_EXPECT(m_loCorner[1] <= m_hiCorner[1]);
  EBGEOMETRY_EXPECT(m_loCorner[2] <= m_hiCorner[2]);
  EBGEOMETRY_EXPECT(a_other.m_loCorner[0] <= a_other.m_hiCorner[0]);
  EBGEOMETRY_EXPECT(a_other.m_loCorner[1] <= a_other.m_hiCorner[1]);
  EBGEOMETRY_EXPECT(a_other.m_loCorner[2] <= a_other.m_hiCorner[2]);

  // Two AABBs overlap if and only if they overlap on every axis simultaneously.
  // On each axis the intervals [lo, hi] and [other.lo, other.hi] overlap when
  // lo < other.hi  AND  hi > other.lo  (strict inequalities: touching edges are not overlapping).
  const bool overlapX = (m_loCorner[0] < a_other.m_hiCorner[0]) && (m_hiCorner[0] > a_other.m_loCorner[0]);
  const bool overlapY = (m_loCorner[1] < a_other.m_hiCorner[1]) && (m_hiCorner[1] > a_other.m_loCorner[1]);
  const bool overlapZ = (m_loCorner[2] < a_other.m_hiCorner[2]) && (m_hiCorner[2] > a_other.m_loCorner[2]);

  return overlapX && overlapY && overlapZ;
}

template <class T>
inline Vec3T<T>&
AABBT<T>::getLowCorner() noexcept
{
  return m_loCorner;
}

template <class T>
inline const Vec3T<T>&
AABBT<T>::getLowCorner() const noexcept
{
  return m_loCorner;
}

template <class T>
inline Vec3T<T>&
AABBT<T>::getHighCorner() noexcept
{
  return m_hiCorner;
}

template <class T>
inline const Vec3T<T>&
AABBT<T>::getHighCorner() const noexcept
{
  return m_hiCorner;
}

template <class T>
inline Vec3T<T>
AABBT<T>::getCentroid() const noexcept
{
  EBGEOMETRY_EXPECT(m_loCorner[0] <= m_hiCorner[0]);
  EBGEOMETRY_EXPECT(m_loCorner[1] <= m_hiCorner[1]);
  EBGEOMETRY_EXPECT(m_loCorner[2] <= m_hiCorner[2]);

  return T(0.5) * (m_loCorner + m_hiCorner);
}

template <class T>
inline T
AABBT<T>::getOverlappingVolume(const AABBT<T>& a_other) const noexcept
{
  EBGEOMETRY_EXPECT(m_loCorner[0] <= m_hiCorner[0]);
  EBGEOMETRY_EXPECT(m_loCorner[1] <= m_hiCorner[1]);
  EBGEOMETRY_EXPECT(m_loCorner[2] <= m_hiCorner[2]);
  EBGEOMETRY_EXPECT(a_other.m_loCorner[0] <= a_other.m_hiCorner[0]);
  EBGEOMETRY_EXPECT(a_other.m_loCorner[1] <= a_other.m_hiCorner[1]);
  EBGEOMETRY_EXPECT(a_other.m_loCorner[2] <= a_other.m_hiCorner[2]);

  // The overlap length on each axis is the length of the intersection of the two intervals.
  // For intervals [lo, hi] and [olo, ohi], this is max(0, min(hi, ohi) - max(lo, olo)).
  // The overlapping volume is the product of the three overlap lengths; it is zero
  // if the boxes do not intersect on at least one axis.
  const Vec3& lo  = m_loCorner;
  const Vec3& hi  = m_hiCorner;
  const Vec3& olo = a_other.m_loCorner;
  const Vec3& ohi = a_other.m_hiCorner;

  const T dx = std::max(T(0), std::min(hi[0], ohi[0]) - std::max(lo[0], olo[0]));
  const T dy = std::max(T(0), std::min(hi[1], ohi[1]) - std::max(lo[1], olo[1]));
  const T dz = std::max(T(0), std::min(hi[2], ohi[2]) - std::max(lo[2], olo[2]));

  return dx * dy * dz;
}

template <class T>
inline T
AABBT<T>::getDistance(const Vec3& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(m_loCorner[0] <= m_hiCorner[0]);
  EBGEOMETRY_EXPECT(m_loCorner[1] <= m_hiCorner[1]);
  EBGEOMETRY_EXPECT(m_loCorner[2] <= m_hiCorner[2]);
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  // For each axis, compute the signed gap between a_point and the nearest box face:
  //
  //   gap = max(lo - p, p - hi)
  //
  // This is negative when p is inside [lo, hi] on that axis, and positive when outside.
  // Clamping the gap vector to zero and taking its length gives the distance to the box:
  // points inside the box contribute zero on every axis, and the Euclidean norm of the
  // positive gaps gives the shortest path to the nearest corner or face otherwise.
  const Vec3 gap(std::max(m_loCorner[0] - a_point[0], a_point[0] - m_hiCorner[0]),
                 std::max(m_loCorner[1] - a_point[1], a_point[1] - m_hiCorner[1]),
                 std::max(m_loCorner[2] - a_point[2], a_point[2] - m_hiCorner[2]));

  return max(Vec3::zero(), gap).length();
}

template <class T>
inline T
AABBT<T>::getVolume() const noexcept
{
  EBGEOMETRY_EXPECT(m_loCorner[0] <= m_hiCorner[0]);
  EBGEOMETRY_EXPECT(m_loCorner[1] <= m_hiCorner[1]);
  EBGEOMETRY_EXPECT(m_loCorner[2] <= m_hiCorner[2]);

  const Vec3 delta = m_hiCorner - m_loCorner;

  return delta[0] * delta[1] * delta[2];
}

template <class T>
inline T
AABBT<T>::getArea() const noexcept
{
  EBGEOMETRY_EXPECT(m_loCorner[0] <= m_hiCorner[0]);
  EBGEOMETRY_EXPECT(m_loCorner[1] <= m_hiCorner[1]);
  EBGEOMETRY_EXPECT(m_loCorner[2] <= m_hiCorner[2]);

  const Vec3 d = m_hiCorner - m_loCorner;

  return T(2) * (d[0] * d[1] + d[1] * d[2] + d[2] * d[0]);
}

template <class T>
[[nodiscard]] bool
intersects(const BoundingSphereT<T>& u, const BoundingSphereT<T>& v) noexcept
{
  return u.intersects(v);
}

template <class T>
[[nodiscard]] bool
intersects(const AABBT<T>& u, const AABBT<T>& v) noexcept
{
  return u.intersects(v);
}

template <class T>
[[nodiscard]] T
getOverlappingVolume(const BoundingSphereT<T>& u, const BoundingSphereT<T>& v) noexcept
{
  return u.getOverlappingVolume(v);
}

template <class T>
[[nodiscard]] T
getOverlappingVolume(const AABBT<T>& u, const AABBT<T>& v) noexcept
{
  return u.getOverlappingVolume(v);
}

} // namespace BoundingVolumes

} // namespace EBGeometry

#endif
