// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_DCEL_EdgeImplem.hpp
 * @brief  Implementation of EBGeometry_DCEL_Edge.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_DCEL_EDGEIMPLEM_HPP
#define EBGEOMETRY_DCEL_EDGEIMPLEM_HPP

// Std includes
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>

// Our includes
#include "EBGeometry_DCEL_Edge.hpp"
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_DCEL_Vertex.hpp"
#include "EBGeometry_Macros.hpp"

namespace EBGeometry {

namespace DCEL {

template <class T, class Meta>
inline EdgeT<T, Meta>::EdgeT(const uint32_t a_vertexIndex) noexcept
{
  m_vertex = a_vertexIndex;
}

template <class T, class Meta>
inline size_t
EdgeT<T, Meta>::size() const noexcept
{
  return 2U;
}

template <class T, class Meta>
inline void
EdgeT<T, Meta>::define(const uint32_t a_vertexIndex,
                       const uint32_t a_pairEdgeIndex,
                       const uint32_t a_nextEdgeIndex) noexcept
{
  m_vertex   = a_vertexIndex;
  m_pairEdge = a_pairEdgeIndex;
  m_nextEdge = a_nextEdgeIndex;
}

template <class T, class Meta>
inline void
EdgeT<T, Meta>::flipNormal() noexcept
{
  m_normal = -m_normal;
}

template <class T, class Meta>
inline void
EdgeT<T, Meta>::reconcile(const Mesh& a_mesh) noexcept
{
  m_normal = this->computeNormal(a_mesh);
}

template <class T, class Meta>
inline void
EdgeT<T, Meta>::setVertex(const uint32_t a_vertexIndex) noexcept
{
  m_vertex = a_vertexIndex;
}

template <class T, class Meta>
inline void
EdgeT<T, Meta>::setPairEdge(const uint32_t a_pairEdgeIndex) noexcept
{
  m_pairEdge = a_pairEdgeIndex;
}

template <class T, class Meta>
inline void
EdgeT<T, Meta>::setNextEdge(const uint32_t a_nextEdgeIndex) noexcept
{
  m_nextEdge = a_nextEdgeIndex;
}

template <class T, class Meta>
inline void
EdgeT<T, Meta>::setFace(const uint32_t a_faceIndex) noexcept
{
  m_face = a_faceIndex;
}

template <class T, class Meta>
inline void
EdgeT<T, Meta>::setMetaData(const Meta& a_metaData) noexcept
{
  m_metaData = a_metaData;
}

template <class T, class Meta>
inline uint32_t
EdgeT<T, Meta>::getVertexIndex() const noexcept
{
  return m_vertex;
}

template <class T, class Meta>
inline uint32_t
EdgeT<T, Meta>::getPairEdgeIndex() const noexcept
{
  return m_pairEdge;
}

template <class T, class Meta>
inline uint32_t
EdgeT<T, Meta>::getNextEdgeIndex() const noexcept
{
  return m_nextEdge;
}

template <class T, class Meta>
inline uint32_t
EdgeT<T, Meta>::getFaceIndex() const noexcept
{
  return m_face;
}

template <class T, class Meta>
inline Vec3T<T>
EdgeT<T, Meta>::computeNormal(const Mesh& a_mesh) const noexcept
{
  // Every half-edge belongs to exactly one face by DCEL invariant, so m_face must be set. The pair
  // edge (and its face) may legitimately be unset for a boundary edge on an open mesh -- that case
  // is handled gracefully below rather than asserted against.
  EBGEOMETRY_EXPECT(m_face != UINT32_MAX);

  Vec3T<T> normal = Vec3T<T>::zeros();

  const Vec3T<T>& faceNormal = this->getFace(a_mesh).getNormal();

  EBGEOMETRY_EXPECT(std::isfinite(faceNormal[0]));
  EBGEOMETRY_EXPECT(std::isfinite(faceNormal[1]));
  EBGEOMETRY_EXPECT(std::isfinite(faceNormal[2]));

  normal += faceNormal;

  if (m_pairEdge != UINT32_MAX) {
    const Edge&    pairEdge      = this->getPairEdge(a_mesh);
    const uint32_t pairFaceIndex = pairEdge.getFaceIndex();

    if (pairFaceIndex != UINT32_MAX) {
      const Vec3T<T>& pairFaceNormal = pairEdge.getFace(a_mesh).getNormal();

      EBGEOMETRY_EXPECT(std::isfinite(pairFaceNormal[0]));
      EBGEOMETRY_EXPECT(std::isfinite(pairFaceNormal[1]));
      EBGEOMETRY_EXPECT(std::isfinite(pairFaceNormal[2]));

      normal += pairFaceNormal;
    }
  }

  const T len = normal.length();

  EBGEOMETRY_EXPECT(len > T(0));

  return (len > std::numeric_limits<T>::epsilon()) ? normal / len : Vec3T<T>::zeros();
}

template <class T, class Meta>
inline Vec3T<T>&
EdgeT<T, Meta>::getNormal() noexcept
{
  return m_normal;
}

template <class T, class Meta>
inline const Vec3T<T>&
EdgeT<T, Meta>::getNormal() const noexcept
{
  return m_normal;
}

template <class T, class Meta>
inline VertexT<T, Meta>&
EdgeT<T, Meta>::getVertex(Mesh& a_mesh) noexcept
{
  EBGEOMETRY_EXPECT(m_vertex != UINT32_MAX);

  return a_mesh.getVertices()[m_vertex];
}

template <class T, class Meta>
inline const VertexT<T, Meta>&
EdgeT<T, Meta>::getVertex(const Mesh& a_mesh) const noexcept
{
  EBGEOMETRY_EXPECT(m_vertex != UINT32_MAX);

  return a_mesh.getVertices()[m_vertex];
}

template <class T, class Meta>
inline VertexT<T, Meta>&
EdgeT<T, Meta>::getOtherVertex(Mesh& a_mesh) noexcept
{
  EBGEOMETRY_EXPECT(m_nextEdge != UINT32_MAX);

  return this->getNextEdge(a_mesh).getVertex(a_mesh);
}

template <class T, class Meta>
inline const VertexT<T, Meta>&
EdgeT<T, Meta>::getOtherVertex(const Mesh& a_mesh) const noexcept
{
  EBGEOMETRY_EXPECT(m_nextEdge != UINT32_MAX);

  return this->getNextEdge(a_mesh).getVertex(a_mesh);
}

template <class T, class Meta>
inline EdgeT<T, Meta>&
EdgeT<T, Meta>::getPairEdge(Mesh& a_mesh) noexcept
{
  EBGEOMETRY_EXPECT(m_pairEdge != UINT32_MAX);

  return a_mesh.getEdges()[m_pairEdge];
}

template <class T, class Meta>
inline const EdgeT<T, Meta>&
EdgeT<T, Meta>::getPairEdge(const Mesh& a_mesh) const noexcept
{
  EBGEOMETRY_EXPECT(m_pairEdge != UINT32_MAX);

  return a_mesh.getEdges()[m_pairEdge];
}

template <class T, class Meta>
inline EdgeT<T, Meta>&
EdgeT<T, Meta>::getNextEdge(Mesh& a_mesh) noexcept
{
  EBGEOMETRY_EXPECT(m_nextEdge != UINT32_MAX);

  return a_mesh.getEdges()[m_nextEdge];
}

template <class T, class Meta>
inline const EdgeT<T, Meta>&
EdgeT<T, Meta>::getNextEdge(const Mesh& a_mesh) const noexcept
{
  EBGEOMETRY_EXPECT(m_nextEdge != UINT32_MAX);

  return a_mesh.getEdges()[m_nextEdge];
}

template <class T, class Meta>
inline Vec3T<T>
EdgeT<T, Meta>::getX2X1(const Mesh& a_mesh) const noexcept
{
  EBGEOMETRY_EXPECT(m_vertex != UINT32_MAX);

  const auto& x1 = this->getVertex(a_mesh).getPosition();
  const auto& x2 = this->getOtherVertex(a_mesh).getPosition();

  return x2 - x1;
}

template <class T, class Meta>
inline FaceT<T, Meta>&
EdgeT<T, Meta>::getFace(Mesh& a_mesh) noexcept
{
  EBGEOMETRY_EXPECT(m_face != UINT32_MAX);

  return a_mesh.getFaces()[m_face];
}

template <class T, class Meta>
inline const FaceT<T, Meta>&
EdgeT<T, Meta>::getFace(const Mesh& a_mesh) const noexcept
{
  EBGEOMETRY_EXPECT(m_face != UINT32_MAX);

  return a_mesh.getFaces()[m_face];
}

template <class T, class Meta>
inline Meta&
EdgeT<T, Meta>::getMetaData() noexcept
{
  return m_metaData;
}

template <class T, class Meta>
inline const Meta&
EdgeT<T, Meta>::getMetaData() const noexcept
{
  return m_metaData;
}

template <class T, class Meta>
inline T
EdgeT<T, Meta>::projectPointToEdge(const Vec3& a_x0, const Mesh& a_mesh) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[2]));
  EBGEOMETRY_EXPECT(m_vertex != UINT32_MAX);

  const auto p    = a_x0 - this->getVertex(a_mesh).getPosition();
  const auto x2x1 = this->getX2X1(a_mesh);

  EBGEOMETRY_EXPECT(x2x1.dot(x2x1) > T(0));

  return p.dot(x2x1) / (x2x1.dot(x2x1));
}

template <class T, class Meta>
inline T
EdgeT<T, Meta>::signedDistance(const Vec3& a_x0, const Mesh& a_mesh) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[2]));
  EBGEOMETRY_EXPECT(m_vertex != UINT32_MAX);

  T retval = std::numeric_limits<T>::max();

  // Project point to edge.
  const T t = this->projectPointToEdge(a_x0, a_mesh);

  if (t <= T(0.0)) {
    // Closest point is the starting vertex
    retval = this->getVertex(a_mesh).signedDistance(a_x0);
  }
  else if (t >= T(1.0)) {
    // Closest point is the end vertex
    retval = this->getOtherVertex(a_mesh).signedDistance(a_x0);
  }
  else {
    // Closest point is the edge itself.
    const Vec3 x2x1      = this->getX2X1(a_mesh);
    const Vec3 linePoint = this->getVertex(a_mesh).getPosition() + t * x2x1;
    const Vec3 delta     = a_x0 - linePoint;
    const T    dot       = m_normal.dot(delta);

    const int sgn = (dot > T(0.0)) ? 1 : -1;

    retval = sgn * delta.length();
  }

  return retval;
}

template <class T, class Meta>
inline T
EdgeT<T, Meta>::unsignedDistance2(const Vec3& a_x0, const Mesh& a_mesh) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[2]));
  EBGEOMETRY_EXPECT(m_vertex != UINT32_MAX);

  constexpr T zero = 0.0;
  constexpr T one  = 1.0;

  // Project point to edge and restrict to edge length.
  const auto t = std::clamp(this->projectPointToEdge(a_x0, a_mesh), zero, one);

  // Compute distance to this edge.
  const Vec3T<T> x2x1      = this->getX2X1(a_mesh);
  const Vec3T<T> linePoint = this->getVertex(a_mesh).getPosition() + t * x2x1;
  const Vec3T<T> delta     = a_x0 - linePoint;

  return delta.dot(delta);
}
} // namespace DCEL

} // namespace EBGeometry

#endif
