/* EBGeometry
 * Copyright © 2024 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */
/*!
  @file   EBGeometry_Triangle2DImplem.hpp
  @brief  Implementation of EBGeometry_Triangle2DImplem.hpp
  @author Robert Marskar
*/

#ifndef EBGeometry_Triangle2DImplem
#define EBGeometry_Triangle2DImplem

// Std includes
#include <cmath>

// Our includes
#include "EBGeometry_Triangle2D.hpp"

namespace EBGeometry {

  template <class T>
  Triangle2D<T>::Triangle2D(const Vec3T<T>& a_normal, const std::array<Vec3T<T>, 3>& a_vertices) noexcept
  {
    this->define(a_normal, a_vertices);
  }

  template <class T>
  void
  Triangle2D<T>::define(const Vec3T<T>& a_normal, const std::array<Vec3T<T>, 3>& a_vertices) noexcept
  {
    int ignoreDir = 0;

    for (int dir = 1; dir < 3; dir++) {
      if (std::abs(a_normal[dir]) > std::abs(a_normal[ignoreDir])) {
        ignoreDir = dir;
      }
    }

    m_xDir = 3;
    m_yDir = 0;

    for (int dir = 0; dir < 3; dir++) {
      if (dir != ignoreDir) {
        m_xDir = std::min(m_xDir, dir);
        m_yDir = std::max(m_yDir, dir);
      }
    }

    for (int i = 0; i < 3; i++) {
      m_vertices[i] = this->projectPoint(a_vertices[i]);
    }
  }

  template <class T>
  bool
  Triangle2D<T>::isPointInside(const Vec3T<T>& a_point, const InsideOutsideAlgorithm a_algorithm) const noexcept
  {
    bool ret = false;

    switch (a_algorithm) {
    case InsideOutsideAlgorithm::SubtendedAngle: {
      ret = this->isPointInsideSubtend(a_point);

      break;
    }
    case InsideOutsideAlgorithm::CrossingNumber: {
      ret = this->isPointInsideCrossingNumber(a_point);

      break;
    }
    case InsideOutsideAlgorithm::WindingNumber: {
      ret = this->isPointInsideWindingNumber(a_point);

      break;
    }
    }

    return ret;
  }

  template <class T>
  bool
  Triangle2D<T>::isPointInsideWindingNumber(const Vec3T<T>& a_point) const noexcept
  {
    const Vec2T<T> projectedPoint = this->projectPoint(a_point);

    const int windingNumber = this->computeWindingNumber(projectedPoint);

    return windingNumber != 0;
  }

  template <class T>
  bool
  Triangle2D<T>::isPointInsideCrossingNumber(const Vec3T<T>& a_point) const noexcept
  {
    const Vec2T<T> projectedPoint = this->projectPoint(a_point);

    const int crossingNumber = this->computeCrossingNumber(projectedPoint);

    return (crossingNumber & 1);
  }

  template <class T>
  bool
  Triangle2D<T>::isPointInsideSubtend(const Vec3T<T>& a_point) const noexcept
  {
    const Vec2T<T> projectedPoint = this->projectPoint(a_point);

    T sumTheta = this->computeSubtendedAngle(projectedPoint);

    sumTheta = std::abs(sumTheta) / (T(2.0) * M_PI); // NOLINT

    return (round(sumTheta) == 1);
  }

  template <class T>
  Vec2T<T>
  Triangle2D<T>::projectPoint(const Vec3T<T>& a_point) const noexcept
  {
    return Vec2T<T>(a_point[m_xDir], a_point[m_yDir]);
  }

  template <class T>
  int
  Triangle2D<T>::computeWindingNumber(const Vec2T<T>& a_point) const noexcept
  {
    int windingNumber = 0;

    constexpr T zero = T(0.0);

    auto isLeft = [](const Vec2T<T>& p0, const Vec2T<T>& p1, const Vec2T<T>& p2) {
      return (p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y);
    };

    // Loop through all edges of the polygon
    for (int i = 0; i < 3; i++) {

      const Vec2T<T>& P  = a_point;
      const Vec2T<T>& p1 = m_vertices[i];
      const Vec2T<T>& p2 = m_vertices[(i + 1) % 3];

      const T res = isLeft(p1, p2, P);

      if (p1.y <= P.y) {
        if (p2.y > P.y && res > zero) {
          windingNumber += 1;
        }
      }
      else {
        if (p2.y <= P.y && res < zero) {
          windingNumber -= 1;
        }
      }
    }

    return windingNumber;
  }

  template <class T>
  int
  Triangle2D<T>::computeCrossingNumber(const Vec2T<T>& a_point) const noexcept
  {
    int crossingNumber = 0;

    for (int i = 0; i < 3; i++) {
      const Vec2T<T>& p1 = m_vertices[i];
      const Vec2T<T>& p2 = m_vertices[(i + 1) % 3];

      const bool upwardCrossing   = (p1.y <= a_point.y) && (p2.y > a_point.y);
      const bool downwardCrossing = (p1.y > a_point.y) && (p2.y <= a_point.y);

      if (upwardCrossing || downwardCrossing) {
        const T t = (a_point.y - p1.y) / (p2.y - p1.y);

        if (a_point.x < p1.x + t * (p2.x - p1.x)) {
          crossingNumber += 1;
        }
      }
    }

    return crossingNumber;
  }

  template <class T>
  T
  Triangle2D<T>::computeSubtendedAngle(const Vec2T<T>& a_point) const noexcept
  {
    T sumTheta = 0.0;

    for (int i = 0; i < 3; i++) {
      const Vec2T<T> p1 = m_vertices[i] - a_point;
      const Vec2T<T> p2 = m_vertices[(i + 1) % 3] - a_point;

      const T theta1 = static_cast<T>(atan2(p1.y, p1.x));
      const T theta2 = static_cast<T>(atan2(p2.y, p2.x));

      T dTheta = theta2 - theta1;

      while (dTheta > M_PI) {
        dTheta -= 2.0 * M_PI;
      }
      while (dTheta < -M_PI) {
        dTheta += 2.0 * M_PI;
      }

      sumTheta += dTheta;
    }

    return sumTheta;
  }
} // namespace EBGeometry

#endif
