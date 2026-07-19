// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_DCEL_VertexImplem.hpp
 * @brief  Implementation of EBGeometry_DCEL_Vertex.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_DCEL_VERTEXIMPLEM_HPP
#define EBGEOMETRY_DCEL_VERTEXIMPLEM_HPP

// Std includes
#include <cmath>
#include <limits>
#include <vector>

// Our includes
#include "EBGeometry_DCEL_Vertex.hpp"
#include "EBGeometry_Macros.hpp"

namespace EBGeometry {

namespace DCEL {

template <class T, class Meta>
inline VertexT<T, Meta>::VertexT(const Vec3& a_position) : VertexT()
{
  EBGEOMETRY_EXPECT(std::isfinite(a_position[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_position[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_position[2]));

  m_position = a_position;
}

template <class T, class Meta>
inline VertexT<T, Meta>::VertexT(const Vec3& a_position, const Vec3& a_normal) : VertexT()
{
  EBGEOMETRY_EXPECT(std::isfinite(a_position[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_position[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_position[2]));
  EBGEOMETRY_EXPECT(std::isfinite(a_normal[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_normal[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_normal[2]));

  m_position = a_position;
  m_normal   = a_normal;
}

template <class T, class Meta>
inline void
VertexT<T, Meta>::define(const Vec3& a_position, const DCELIndex a_edge, const Vec3& a_normal) noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_position[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_position[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_position[2]));
  EBGEOMETRY_EXPECT(std::isfinite(a_normal[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_normal[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_normal[2]));

  // a_edge == InvalidIndex is valid here (e.g. a freshly-created vertex not yet wired into the mesh).
  m_position     = a_position;
  m_outgoingEdge = a_edge;
  m_normal       = a_normal;
}

template <class T, class Meta>
inline void
VertexT<T, Meta>::setPosition(const Vec3& a_position) noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_position[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_position[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_position[2]));

  m_position = a_position;
}

template <class T, class Meta>
inline void
VertexT<T, Meta>::setEdge(const DCELIndex a_edge) noexcept
{
  // a_edge == InvalidIndex is valid here; callers that follow m_outgoingEdge are responsible for
  // checking it first.
  m_outgoingEdge = a_edge;
}

template <class T, class Meta>
inline void
VertexT<T, Meta>::setMetaData(const Meta& a_metaData) noexcept
{
  m_metaData = a_metaData;
}

template <class T, class Meta>
inline void
VertexT<T, Meta>::setNormal(const Vec3& a_normal) noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_normal[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_normal[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_normal[2]));

  m_normal = a_normal;
}

template <class T, class Meta>
inline void
VertexT<T, Meta>::addFace(const DCELIndex a_face)
{
  EBGEOMETRY_EXPECT(a_face >= 0);

  m_faces.emplace_back(a_face);
}

template <class T, class Meta>
inline void
VertexT<T, Meta>::normalizeNormalVector() noexcept
{
  const T len = m_normal.length();

  EBGEOMETRY_EXPECT(len > std::numeric_limits<T>::epsilon());

  if (len > std::numeric_limits<T>::epsilon()) {
    m_normal = m_normal / len;
  }
}

template <class T, class Meta>
inline void
VertexT<T, Meta>::flipNormal() noexcept
{
  m_normal = -m_normal;
}

template <class T, class Meta>
inline Vec3T<T>&
VertexT<T, Meta>::getPosition() noexcept
{
  return m_position;
}

template <class T, class Meta>
inline const Vec3T<T>&
VertexT<T, Meta>::getPosition() const noexcept
{
  return m_position;
}

template <class T, class Meta>
inline Vec3T<T>&
VertexT<T, Meta>::getNormal() noexcept
{
  return m_normal;
}

template <class T, class Meta>
inline const Vec3T<T>&
VertexT<T, Meta>::getNormal() const noexcept
{
  return m_normal;
}

template <class T, class Meta>
inline DCELIndex
VertexT<T, Meta>::getOutgoingEdge() const noexcept
{
  return m_outgoingEdge;
}

template <class T, class Meta>
inline std::vector<DCELIndex>&
VertexT<T, Meta>::getFaces() noexcept
{
  return m_faces;
}

template <class T, class Meta>
inline const std::vector<DCELIndex>&
VertexT<T, Meta>::getFaces() const noexcept
{
  return m_faces;
}

template <class T, class Meta>
inline T
VertexT<T, Meta>::signedDistance(const Vec3& a_x0) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[2]));

  const auto delta = a_x0 - m_position;
  const T    dist  = delta.length();
  const T    dot   = m_normal.dot(delta);
  const int  sign  = (dot > T(0.)) ? 1 : -1;

  return dist * sign;
}

template <class T, class Meta>
inline T
VertexT<T, Meta>::unsignedDistance2(const Vec3& a_x0) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[2]));

  const auto d = a_x0 - m_position;

  return d.dot(d);
}

template <class T, class Meta>
inline Meta&
VertexT<T, Meta>::getMetaData() noexcept
{
  return m_metaData;
}

template <class T, class Meta>
inline const Meta&
VertexT<T, Meta>::getMetaData() const noexcept
{
  return m_metaData;
}
} // namespace DCEL

} // namespace EBGeometry

#endif
