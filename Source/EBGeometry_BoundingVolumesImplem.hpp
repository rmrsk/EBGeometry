// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_BoundingVolumesImplem.hpp
 * @brief  Inline implementations of AABBT<T>.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_BOUNDINGVOLUMESIMPLEM_HPP
#define EBGEOMETRY_BOUNDINGVOLUMESIMPLEM_HPP

// Std includes
#include <cmath>
#include <cstddef>
#include <vector>

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_Macros.hpp"

namespace EBGeometry {

namespace BoundingVolumes {

template <class T>
EBGEOMETRY_HOST_DEVICE
AABBT<T>::AABBT(const Vec3T<T>& a_lo, const Vec3T<T>& a_hi) noexcept
{
  EBGEOMETRY_EXPECT(a_lo[0] <= a_hi[0]);
  EBGEOMETRY_EXPECT(a_lo[1] <= a_hi[1]);
  EBGEOMETRY_EXPECT(a_lo[2] <= a_hi[2]);

  m_loCorner = a_lo;
  m_hiCorner = a_hi;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline AABBT<T>
AABBT<T>::merged(const AABBT<T>& a_other) const noexcept
{
  // Set the corners directly rather than through the corner constructor, which rejects an inverted
  // box: the union of two default (inverted) boxes is itself inverted.
  AABBT<T> box;

  box.m_loCorner = min(m_loCorner, a_other.m_loCorner);
  box.m_hiCorner = max(m_hiCorner, a_other.m_hiCorner);

  return box;
}

template <class T>
EBGEOMETRY_HOST
AABBT<T>::AABBT(const std::vector<AABBT<T>>& a_others) noexcept
{
  EBGEOMETRY_REQUIRE(!a_others.empty(), "AABBT: cannot enclose an empty list of bounding boxes");

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
EBGEOMETRY_HOST
AABBT<T>::AABBT(const std::vector<Vec3T<P>>& a_points) noexcept
{
  EBGEOMETRY_REQUIRE(!a_points.empty(), "AABBT: cannot enclose an empty list of points");

  this->define(a_points);
}

template <class T>
template <class P>
EBGEOMETRY_HOST_DEVICE
AABBT<T>::AABBT(const Vec3T<P>* a_points, size_t a_numPoints) noexcept
{
  EBGEOMETRY_EXPECT(a_points != nullptr);
  EBGEOMETRY_EXPECT(a_numPoints > 0);

  this->define(a_points, a_numPoints);
}

template <class T>
template <class P>
EBGEOMETRY_HOST
inline void
AABBT<T>::define(const std::vector<Vec3T<P>>& a_points) noexcept
{
  EBGEOMETRY_REQUIRE(!a_points.empty(), "AABBT::define: cannot enclose an empty list of points");

  this->define(a_points.data(), a_points.size());
}

template <class T>
template <class P>
EBGEOMETRY_HOST_DEVICE
inline void
AABBT<T>::define(const Vec3T<P>* a_points, size_t a_numPoints) noexcept
{
  EBGEOMETRY_EXPECT(a_points != nullptr);
  EBGEOMETRY_EXPECT(a_numPoints > 0);

  m_loCorner = a_points[0];
  m_hiCorner = a_points[0];

  for (size_t i = 0; i < a_numPoints; i++) {
    m_loCorner = min(m_loCorner, a_points[i]);
    m_hiCorner = max(m_hiCorner, a_points[i]);
  }
}

template <class T>
EBGEOMETRY_HOST_DEVICE
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
EBGEOMETRY_HOST_DEVICE
inline Vec3T<T>&
AABBT<T>::getLowCorner() noexcept
{
  return m_loCorner;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline const Vec3T<T>&
AABBT<T>::getLowCorner() const noexcept
{
  return m_loCorner;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline Vec3T<T>&
AABBT<T>::getHighCorner() noexcept
{
  return m_hiCorner;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline const Vec3T<T>&
AABBT<T>::getHighCorner() const noexcept
{
  return m_hiCorner;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline Vec3T<T>
AABBT<T>::getCentroid() const noexcept
{
  EBGEOMETRY_EXPECT(m_loCorner[0] <= m_hiCorner[0]);
  EBGEOMETRY_EXPECT(m_loCorner[1] <= m_hiCorner[1]);
  EBGEOMETRY_EXPECT(m_loCorner[2] <= m_hiCorner[2]);

  return T(0.5) * (m_loCorner + m_hiCorner);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
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

  // Per-axis overlap length as a vector: max(0, min(hi, ohi) - max(lo, olo)). The overlapping
  // volume is the product of the three components.
  const Vec3 overlap = max(Vec3::zeros(), min(hi, ohi) - max(lo, olo));

  return overlap[0] * overlap[1] * overlap[2];
}

template <class T>
EBGEOMETRY_HOST_DEVICE
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
  const Vec3 gap = max(m_loCorner - a_point, a_point - m_hiCorner);

  return max(Vec3::zeros(), gap).length();
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline T
AABBT<T>::getDistance2(const Vec3& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(m_loCorner[0] <= m_hiCorner[0]);
  EBGEOMETRY_EXPECT(m_loCorner[1] <= m_hiCorner[1]);
  EBGEOMETRY_EXPECT(m_loCorner[2] <= m_hiCorner[2]);
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  // Same per-axis gap construction as getDistance() -- see its comment -- but squared and
  // summed directly, without ever taking a square root.
  const Vec3 gap = max(m_loCorner - a_point, a_point - m_hiCorner);

  return max(Vec3::zeros(), gap).length2();
}

template <class T>
EBGEOMETRY_HOST_DEVICE
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
EBGEOMETRY_HOST_DEVICE
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
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
bool
intersects(const AABBT<T>& u, const AABBT<T>& v) noexcept
{
  return u.intersects(v);
}

template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
T
getOverlappingVolume(const AABBT<T>& u, const AABBT<T>& v) noexcept
{
  return u.getOverlappingVolume(v);
}

} // namespace BoundingVolumes

} // namespace EBGeometry

#endif
