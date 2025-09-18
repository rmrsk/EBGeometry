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
    // Here is a message from the past: If one wants, one can precompute v21, v32, v13
    // as well as many other quantities (e.g., v21.cross(m_triangleNormal)). This might
    // be helpful in order to speed things up a little bit.
    T ret = std::numeric_limits<T>::max();
    
    auto sgn = [](const T x) -> int { return (x > 0.0) ? 1 : -1; };

<<<<<<< HEAD
    auto sgn = [](const T x) -> int { return (x > 0.0) ? 1 : -1; };

    const Vec3 v21 = m_vertexPositions[1] - m_vertexPositions[0];
    const Vec3 v32 = m_vertexPositions[2] - m_vertexPositions[1];
    const Vec3 v13 = m_vertexPositions[0] - m_vertexPositions[2];

    const Vec3 p1 = a_point - m_vertexPositions[0];
    const Vec3 p2 = a_point - m_vertexPositions[1];
    const Vec3 p3 = a_point - m_vertexPositions[2];

    const T s0 = sgn(dot(v21.cross(m_triangleNormal), p1));
    const T s1 = sgn(dot(v32.cross(m_triangleNormal), p2));
    const T s2 = sgn(dot(v13.cross(m_triangleNormal), p3));

    const T t1 = dot(p1, v21) / dot(v21, v21);
    const T t2 = dot(p2, v32) / dot(v32, v32);
    const T t3 = dot(p3, v13) / dot(v13, v13);

    const Vec3 y1 = p1 - t1 * v21;
    const Vec3 y2 = p2 - t2 * v32;
    const Vec3 y3 = p3 - t3 * v13;

    // Distance to vertices
    ret = (p1.length() > std::abs(ret)) ? ret : p1.length() * sgn(m_vertexNormals[0].dot(p1));
    ret = (p2.length() > std::abs(ret)) ? ret : p2.length() * sgn(m_vertexNormals[1].dot(p2));
    ret = (p3.length() > std::abs(ret)) ? ret : p3.length() * sgn(m_vertexNormals[2].dot(p3));
=======
    const Vec3 v21 = m_vertexPositions[1] - m_vertexPositions[0];
    const Vec3 v32 = m_vertexPositions[2] - m_vertexPositions[1];
    const Vec3 v13 = m_vertexPositions[0] - m_vertexPositions[2];

    const Vec3 p1 = a_point - m_vertexPositions[0];
    const Vec3 p2 = a_point - m_vertexPositions[1];
    const Vec3 p3 = a_point - m_vertexPositions[2];

    const T s0 = sgn(dot(v21.cross(m_triangleNormal), p1));
    const T s1 = sgn(dot(v32.cross(m_triangleNormal), p2));
    const T s2 = sgn(dot(v13.cross(m_triangleNormal), p3));

    const bool isPointInside = s0 + s1 + s2 >= 2.0;

    if (isPointInside) {
      ret = m_triangleNormal.dot(p1);
    }
    else {
      ret = (p1.length() > std::abs(ret)) ? ret : p1.length() * sgn(m_vertexNormals[0].dot(p1));
      ret = (p2.length() > std::abs(ret)) ? ret : p2.length() * sgn(m_vertexNormals[1].dot(p2));
      ret = (p3.length() > std::abs(ret)) ? ret : p3.length() * sgn(m_vertexNormals[2].dot(p3));
>>>>>>> 72a5133 (WIP)

    // Distance to edges
    ret = (t1 > 0.0 && t1 < 1.0 && y1.length() < std::abs(ret)) ? y1.length() * sgn(m_edgeNormals[0].dot(y1)) : ret;
    ret = (t2 > 0.0 && t2 < 1.0 && y2.length() < std::abs(ret)) ? y2.length() * sgn(m_edgeNormals[1].dot(y2)) : ret;
    ret = (t3 > 0.0 && t3 < 1.0 && y3.length() < std::abs(ret)) ? y3.length() * sgn(m_edgeNormals[2].dot(y3)) : ret;

    // Note that s0 + s1 + s2 >= 2.0 is a point-in-polygon test.
    return (s0 + s1 + s2 >= 2.0) ? m_triangleNormal.dot(p1) : ret;
  }
} // namespace EBGeometry

#endif
