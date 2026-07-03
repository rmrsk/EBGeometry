// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  @file   EBGeometry_Polygon2DImplem.hpp
  @brief  Implementation of EBGeometry_Polygon2D.hpp
  @author Robert Marskar
*/

#ifndef EBGEOMETRY_POLYGON2DIMPLEM_HPP
#define EBGEOMETRY_POLYGON2DIMPLEM_HPP

// Std includes
#include <iostream>

// Our includes
#include "EBGeometry_Polygon2D.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

template <class T>
inline Polygon2D<T>::Polygon2D(const Vec3& a_normal, const std::vector<Vec3>& a_points)
{
  this->define(a_normal, a_points);
}

template <class T>
inline bool
Polygon2D<T>::isPointInside(const Vec3& a_point, const InsideOutsideAlgorithm a_algorithm) const noexcept
{
  bool ret = false;

  switch (a_algorithm) {
  case InsideOutsideAlgorithm::SubtendedAngle: {
    ret = this->isPointInsidePolygonSubtend(a_point);

    break;
  }
  case InsideOutsideAlgorithm::CrossingNumber: {
    ret = this->isPointInsidePolygonCrossingNumber(a_point);

    break;
  }
  case InsideOutsideAlgorithm::WindingNumber: {
    ret = this->isPointInsidePolygonWindingNumber(a_point);

    break;
  }
  default:
    std::cerr << "In file 'EBGeometry_Polygon2DImplem.hpp' function "
                 "Polygon2D<T>::isPointInside - logic bust.\n";
  }

  return ret;
}

template <class T>
inline Vec2T<T>
Polygon2D<T>::projectPoint(const Vec3& a_point) const noexcept
{
  return Vec2(a_point[m_xDir], a_point[m_yDir]);
}

template <class T>
inline void
Polygon2D<T>::define(const Vec3& a_normal, const std::vector<Vec3>& a_points)
{
  size_t ignoreDir = 0;

  for (size_t dir = 1; dir < 3; dir++) {
    if (std::abs(a_normal[dir]) > std::abs(a_normal[ignoreDir])) {
      ignoreDir = dir;
    }
  }

  m_xDir = 3;
  m_yDir = 0;

  for (size_t dir = 0; dir < 3; dir++) {
    if (dir != ignoreDir) {
      m_xDir = std::min(m_xDir, dir);
      m_yDir = std::max(m_yDir, dir);
    }
  }

  for (const auto& p3 : a_points) {
    m_points.emplace_back(this->projectPoint(p3));
  }
}

template <class T>
inline int
Polygon2D<T>::computeWindingNumber(const Vec2& P) const noexcept
{
  int wn = 0; // the  winding number counter

  const size_t N = m_points.size();

  auto isLeft = [](const Vec2& P0, const Vec2& P1, const Vec2& P2) {
    return (P1.x - P0.x) * (P2.y - P0.y) - (P2.x - P0.x) * (P1.y - P0.y);
  };

  // loop through all edges of the polygon
  for (size_t i = 0; i < N; i++) { // edge from V[i] to  V[i+1]

    const Vec2& P1 = m_points[i];
    const Vec2& P2 = m_points[(i + 1) % N];

    const T res = isLeft(P1, P2, P);

    if (P1.y <= P.y) { // start y <= P.y
      if (P2.y > P.y)  // an upward crossing
        if (res > 0.)  // P left of  edge
          ++wn;        // have  a valid up intersect
    }
    else {             // start y > P.y (no test needed)
      if (P2.y <= P.y) // a downward crossing
        if (res < 0.)  // P right of  edge
          --wn;        // have  a valid down intersect
    }
  }

  return wn;
}

template <class T>
inline size_t
Polygon2D<T>::computeCrossingNumber(const Vec2& P) const noexcept
{
  size_t cn = 0;

  const size_t N = m_points.size();

  for (size_t i = 0; i < N; i++) { // edge from V[i]  to V[i+1]
    const Vec2& P1 = m_points[i];
    const Vec2& P2 = m_points[(i + 1) % N];

    // clang-format off
    const bool upwardCrossing   = (P1.y <= P.y) && (P2.y >  P.y);
    const bool downwardCrossing = (P1.y >  P.y) && (P2.y <= P.y);
    // clang-format on    

    if (upwardCrossing || downwardCrossing) {
      const T t = (P.y - P1.y) / (P2.y - P1.y);

      if (P.x < P1.x + t * (P2.x - P1.x)) { // P.x < intersect
        cn += 1;                            // a valid crossing of y=P.y right of P.x
      }
    }
  }

  return cn;
}

template <class T>
inline T
Polygon2D<T>::computeSubtendedAngle(const Vec2& p) const noexcept
{
  constexpr T pi = T(4) * std::atan(T(1));

  T sumTheta = T(0);

  const size_t N = m_points.size();

  for (size_t i = 0; i < N; i++) {
    const Vec2 p1 = m_points[i] - p;
    const Vec2 p2 = m_points[(i + 1) % N] - p;

    const T theta1 = std::atan2(p1.y, p1.x);
    const T theta2 = std::atan2(p2.y, p2.x);

    T dTheta = theta2 - theta1;

    while (dTheta > pi)
      dTheta -= T(2) * pi;
    while (dTheta < -pi)
      dTheta += T(2) * pi;

    sumTheta += dTheta;
  }

  return sumTheta;
}

template <class T>
inline bool
Polygon2D<T>::isPointInsidePolygonWindingNumber(const Vec3& a_point) const noexcept
{
  const Vec2 p = this->projectPoint(a_point);

  const int wn = this->computeWindingNumber(p);

  return wn != 0;
}

template <class T>
inline bool
Polygon2D<T>::isPointInsidePolygonCrossingNumber(const Vec3& a_point) const noexcept
{
  const Vec2 p = this->projectPoint(a_point);

  const size_t cn = this->computeCrossingNumber(p);

  const bool ret = (cn & 1);

  return ret;
}

template <class T>
inline bool
Polygon2D<T>::isPointInsidePolygonSubtend(const Vec3& a_point) const noexcept
{
  const Vec2 p = this->projectPoint(a_point);

  constexpr T pi = T(4) * std::atan(T(1));

  T sumTheta = this->computeSubtendedAngle(p); // Should be = 2*pi if point is inside.

  sumTheta = std::abs(sumTheta) / (T(2) * pi);

  // Tolerance-based comparison: inside if sumTheta rounds to 1 (i.e. the total
  // subtended angle is 2*pi). Using std::abs instead of round() is more robust
  // near the polygon boundary where floating-point accumulation can push sumTheta
  // just past 0.5, causing round() to misclassify.
  const bool ret = (std::abs(sumTheta - T(1)) < T(0.5));

  return ret;
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
