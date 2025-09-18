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
    T ret = std::numeric_limits<T>::max();

    const T eps = std::numeric_limits<T>::epsilon();

    auto sgn = [](const T x) -> int { return (x >= 0.0) ? 1 : -1; };

    const Vec3 e0 = m_vertexPositions[1] - m_vertexPositions[0];
    const Vec3 e1 = m_vertexPositions[2] - m_vertexPositions[1];
    const Vec3 e2 = m_vertexPositions[0] - m_vertexPositions[2];

    const Vec3 dv0 = a_point - m_vertexPositions[0];
    const Vec3 dv1 = a_point - m_vertexPositions[1];
    const Vec3 dv2 = a_point - m_vertexPositions[2];

    const T s0 = dot(m_triangleNormal, e0.cross(dv0));
    const T s1 = dot(m_triangleNormal, e1.cross(dv1));
    const T s2 = dot(m_triangleNormal, e2.cross(dv2));

    const bool isPointInside = (s0 >= -eps) && (s1 >= -eps) && (s2 >= -eps);

    if (isPointInside) {
      ret = m_triangleNormal.dot(dv0);
    }
    else {
      ret = (dv0.length() > std::abs(ret)) ? ret : dv0.length() * sgn(m_vertexNormals[0].dot(dv0));
      ret = (dv1.length() > std::abs(ret)) ? ret : dv1.length() * sgn(m_vertexNormals[1].dot(dv1));
      ret = (dv2.length() > std::abs(ret)) ? ret : dv2.length() * sgn(m_vertexNormals[2].dot(dv2));

      // Check edges
      for (int i = 0; i < 3; i++) {
        const Vec3T<T>& a = m_vertexPositions[i];
        const Vec3T<T>& b = m_vertexPositions[(i + 1) % 3];

        const T t = this->projectPointToEdge(a_point, a, b);

        if (t > 0.0 && t < 1.0) {
          const Vec3T<T> delta = a_point - (a + t * (b - a));

          ret = (delta.length() > std::abs(ret)) ? ret : delta.length() * sgn(m_edgeNormals[i].dot(delta));
        }
      }
    }

    return ret;
  }

  template <class T, class Meta>
  T
  Triangle<T, Meta>::projectPointToEdge(const Vec3T<T>& a_point,
                                        const Vec3T<T>& a_x0,
                                        const Vec3T<T>& a_x1) const noexcept
  {
    const Vec3T<T> a = a_point - a_x0;
    const Vec3T<T> b = a_x1 - a_x0;

    return a.dot(b) / (b.dot(b));
  }
} // namespace EBGeometry

#endif
