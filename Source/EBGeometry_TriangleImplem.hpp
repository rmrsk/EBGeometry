/* EBGeometry
 * Copyright © 2024 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_TriangleImplem.hpp
  @brief  Implementation of EBGeometry_Triangle.hpp
  @author Robert Marskar
*/

#ifndef EBGeometry_TriangleImplem
#define EBGeometry_TriangleImplem

// Our includes
#include "EBGeometry_Triangle.hpp"

namespace EBGeometry {

  template <class T, class Meta>
  Triangle<T, Meta>::Triangle(const std::array<Vec3T<T>, 3>& a_vertexPositions) noexcept
  {
    this->setVertexPositions(a_vertexPositions);
  }

  template <class T, class Meta>
  void
  Triangle<T, Meta>::setNormal(const Vec3T<T>& a_normal) noexcept
  {
    this->m_triangleNormal = a_normal;
  }

  template <class T, class Meta>
  void
  Triangle<T, Meta>::setVertexPositions(const std::array<Vec3T<T>, 3>& a_vertexPositions) noexcept
  {
    m_vertexPositions = a_vertexPositions;

    this->computeNormal();
  }

  template <class T, class Meta>
  void
  Triangle<T, Meta>::setVertexNormals(const std::array<Vec3T<T>, 3>& a_vertexNormals) noexcept
  {
    m_vertexNormals = a_vertexNormals;
  }

  template <class T, class Meta>
  void
  Triangle<T, Meta>::setEdgeNormals(const std::array<Vec3T<T>, 3>& a_edgeNormals) noexcept
  {
    m_edgeNormals = a_edgeNormals;
  }

  template <class T, class Meta>
  void
  Triangle<T, Meta>::setMetaData(const Meta& a_metaData) noexcept
  {
    this->m_metaData = a_metaData;
  }

  template <class T, class Meta>
  void
  Triangle<T, Meta>::computeNormal() noexcept
  {
    const Vec3T<T> x2x0 = m_vertexPositions[2] - m_vertexPositions[0];
    const Vec3T<T> x2x1 = m_vertexPositions[2] - m_vertexPositions[1];

    m_triangleNormal = x2x1.cross(x2x0);
    m_triangleNormal = m_triangleNormal / m_triangleNormal.length();
  }

  template <class T, class Meta>
  Vec3T<T>&
  Triangle<T, Meta>::getNormal() noexcept
  {
    return (this->m_triangleNormal);
  }

  template <class T, class Meta>
  const Vec3T<T>&
  Triangle<T, Meta>::getNormal() const noexcept
  {
    return (this->m_triangleNormal);
  }

  template <class T, class Meta>
  std::array<Vec3T<T>, 3>&
  Triangle<T, Meta>::getVertexPositions() noexcept
  {
    return (this->m_vertexPositions);
  }

  template <class T, class Meta>
  const std::array<Vec3T<T>, 3>&
  Triangle<T, Meta>::getVertexPositions() const noexcept
  {
    return (this->m_vertexPositions);
  }

  template <class T, class Meta>
  std::array<Vec3T<T>, 3>&
  Triangle<T, Meta>::getVertexNormals() noexcept
  {
    return (this->m_vertexNormals);
  }

  template <class T, class Meta>
  const std::array<Vec3T<T>, 3>&
  Triangle<T, Meta>::getVertexNormals() const noexcept
  {
    return (this->m_vertexNormals);
  }
  template <class T, class Meta>
  std::array<Vec3T<T>, 3>&
  Triangle<T, Meta>::getEdgeNormals() noexcept
  {
    return (this->m_edgeNormals);
  }

  template <class T, class Meta>
  const std::array<Vec3T<T>, 3>&
  Triangle<T, Meta>::getEdgeNormals() const noexcept
  {
    return (this->m_edgeNormals);
  }

  template <class T, class Meta>
  Meta&
  Triangle<T, Meta>::getMetaData() noexcept
  {
    return (this->m_metaData);
  }

  template <class T, class Meta>
  const Meta&
  Triangle<T, Meta>::getMetaData() const noexcept
  {
    return (this->m_metaData);
  }

  template <class T, class Meta>
  T
  Triangle<T, Meta>::signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    auto sgn = [](const T x) -> int { return (x > 0.0) ? 1 : -1; };

    const Vec3 v21 = m_vertexPositions[1] - m_vertexPositions[0];
    const Vec3 v32 = m_vertexPositions[2] - m_vertexPositions[1];
    const Vec3 v13 = m_vertexPositions[0] - m_vertexPositions[2];

    const Vec3 p1 = a_point - m_vertexPositions[0];
    const Vec3 p2 = a_point - m_vertexPositions[1];
    const Vec3 p3 = a_point - m_vertexPositions[2];

    // Point-in-triangle test (s0+s1+s2 >= 2 means inside).
    // Early return avoids computing t-values and y-vectors for interior points.
    const T s0 = sgn(dot(v21.cross(m_triangleNormal), p1));
    const T s1 = sgn(dot(v32.cross(m_triangleNormal), p2));
    const T s2 = sgn(dot(v13.cross(m_triangleNormal), p3));

    if (s0 + s1 + s2 >= 2.0) {
      return m_triangleNormal.dot(p1);
    }

    // Track minimum squared distance to vertices/edges; one sqrt at the end.
    T    ret2       = p1.dot(p1);
    Vec3 ret_vec    = p1;
    Vec3 ret_normal = m_vertexNormals[0];

    const T p2d2 = p2.dot(p2);
    if (p2d2 < ret2) {
      ret2       = p2d2;
      ret_vec    = p2;
      ret_normal = m_vertexNormals[1];
    }

    const T p3d2 = p3.dot(p3);
    if (p3d2 < ret2) {
      ret2       = p3d2;
      ret_vec    = p3;
      ret_normal = m_vertexNormals[2];
    }

    const T t1 = dot(p1, v21) / dot(v21, v21);
    if (t1 > 0.0 && t1 < 1.0) {
      const Vec3 y1   = p1 - t1 * v21;
      const T    y1d2 = y1.dot(y1);
      if (y1d2 < ret2) {
        ret2       = y1d2;
        ret_vec    = y1;
        ret_normal = m_edgeNormals[0];
      }
    }

    const T t2 = dot(p2, v32) / dot(v32, v32);
    if (t2 > 0.0 && t2 < 1.0) {
      const Vec3 y2   = p2 - t2 * v32;
      const T    y2d2 = y2.dot(y2);
      if (y2d2 < ret2) {
        ret2       = y2d2;
        ret_vec    = y2;
        ret_normal = m_edgeNormals[1];
      }
    }

    const T t3 = dot(p3, v13) / dot(v13, v13);
    if (t3 > 0.0 && t3 < 1.0) {
      const Vec3 y3   = p3 - t3 * v13;
      const T    y3d2 = y3.dot(y3);
      if (y3d2 < ret2) {
        ret2       = y3d2;
        ret_vec    = y3;
        ret_normal = m_edgeNormals[2];
      }
    }

    return std::sqrt(ret2) * sgn(ret_normal.dot(ret_vec));
  }
} // namespace EBGeometry

#endif
