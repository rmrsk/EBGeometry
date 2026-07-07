// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_DCEL_EdgeImplem.hpp
 * @brief  Implementation of EBGeometry_DCEL_Edge.hpp
 * @author Robert Marskar
 * @todo   Include m_face in constructors
 */

#ifndef EBGEOMETRY_DCEL_EDGEIMPLEM_HPP
#define EBGEOMETRY_DCEL_EDGEIMPLEM_HPP

// Std includes
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <memory>

// Our includes
#include "EBGeometry_DCEL_Edge.hpp"
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_DCEL_Vertex.hpp"
#include "EBGeometry_Macros.hpp"

namespace EBGeometry {

namespace DCEL {

template <class T, class Meta>
inline EdgeT<T, Meta>::EdgeT(const VertexPtr& a_vertex) noexcept
{
  m_vertex = a_vertex;
}

template <class T, class Meta>
inline EdgeT<T, Meta>::EdgeT(const Edge& a_otherEdge) noexcept
{
  m_normal   = a_otherEdge.m_normal;
  m_face     = a_otherEdge.m_face;
  m_vertex   = a_otherEdge.m_vertex;
  m_pairEdge = a_otherEdge.m_pairEdge;
  m_nextEdge = a_otherEdge.m_nextEdge;
}

template <class T, class Meta>
inline EdgeT<T, Meta>&
EdgeT<T, Meta>::operator=(const Edge& a_otherEdge) noexcept
{
  if (this != &a_otherEdge) {
    m_normal   = a_otherEdge.m_normal;
    m_face     = a_otherEdge.m_face;
    m_vertex   = a_otherEdge.m_vertex;
    m_pairEdge = a_otherEdge.m_pairEdge;
    m_nextEdge = a_otherEdge.m_nextEdge;
  }

  return *this;
}

template <class T, class Meta>
inline size_t
EdgeT<T, Meta>::size() const noexcept
{
  return 2U;
}

template <class T, class Meta>
inline void
EdgeT<T, Meta>::define(const VertexPtr& a_vertex, const EdgePtr& a_pairEdge, const EdgePtr& a_nextEdge) noexcept
{
  m_vertex   = a_vertex;
  m_pairEdge = a_pairEdge;
  m_nextEdge = a_nextEdge;
}

template <class T, class Meta>
inline void
EdgeT<T, Meta>::flipNormal() noexcept
{
  m_normal = -m_normal;
}

template <class T, class Meta>
inline void
EdgeT<T, Meta>::reconcile() noexcept
{
  m_normal = this->computeNormal();
}

template <class T, class Meta>
inline void
EdgeT<T, Meta>::setVertex(const VertexPtr& a_vertex) noexcept
{
  m_vertex = a_vertex;
}

template <class T, class Meta>
inline void
EdgeT<T, Meta>::setPairEdge(const EdgePtr& a_pairEdge) noexcept
{
  m_pairEdge = a_pairEdge;
}

template <class T, class Meta>
inline void
EdgeT<T, Meta>::setNextEdge(const EdgePtr& a_nextEdge) noexcept
{
  m_nextEdge = a_nextEdge;
}

template <class T, class Meta>
inline void
EdgeT<T, Meta>::setFace(const FacePtr& a_face) noexcept
{
  m_face = a_face;
}

template <class T, class Meta>
inline void
EdgeT<T, Meta>::setMetaData(const Meta& a_metaData) noexcept
{
  m_metaData = a_metaData;
}

template <class T, class Meta>
inline Vec3T<T>
EdgeT<T, Meta>::computeNormal() const noexcept
{
  // Every half-edge belongs to exactly one face by DCEL invariant, so m_face must be set. The pair
  // edge (and its face) may legitimately be null for a boundary edge on an open mesh -- that case
  // is handled gracefully below rather than asserted against.
  EBGEOMETRY_EXPECT(!m_face.expired());

  Vec3T<T> normal = Vec3T<T>::zeros();

  if (const auto face = m_face.lock()) {
    const Vec3T<T>& faceNormal = face->getNormal();

    EBGEOMETRY_EXPECT(std::isfinite(faceNormal[0]));
    EBGEOMETRY_EXPECT(std::isfinite(faceNormal[1]));
    EBGEOMETRY_EXPECT(std::isfinite(faceNormal[2]));

    normal += faceNormal;
  }
  if (const auto pairEdge = m_pairEdge.lock()) {
    if (const auto pairFace = pairEdge->getFace()) {
      const Vec3T<T>& pairFaceNormal = pairFace->getNormal();

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
inline std::shared_ptr<VertexT<T, Meta>>
EdgeT<T, Meta>::getVertex() noexcept
{
  return m_vertex.lock();
}

template <class T, class Meta>
inline std::shared_ptr<VertexT<T, Meta>>
EdgeT<T, Meta>::getVertex() const noexcept
{
  return m_vertex.lock();
}

template <class T, class Meta>
inline std::shared_ptr<VertexT<T, Meta>>
EdgeT<T, Meta>::getOtherVertex() noexcept
{
  EBGEOMETRY_EXPECT(!m_nextEdge.expired());

  return m_nextEdge.lock()->getVertex();
}

template <class T, class Meta>
inline std::shared_ptr<VertexT<T, Meta>>
EdgeT<T, Meta>::getOtherVertex() const noexcept
{
  EBGEOMETRY_EXPECT(!m_nextEdge.expired());

  return m_nextEdge.lock()->getVertex();
}

template <class T, class Meta>
inline std::shared_ptr<EdgeT<T, Meta>>
EdgeT<T, Meta>::getPairEdge() noexcept
{
  return m_pairEdge.lock();
}

template <class T, class Meta>
inline std::shared_ptr<EdgeT<T, Meta>>
EdgeT<T, Meta>::getPairEdge() const noexcept
{
  return m_pairEdge.lock();
}

template <class T, class Meta>
inline std::shared_ptr<EdgeT<T, Meta>>
EdgeT<T, Meta>::getNextEdge() noexcept
{
  return m_nextEdge.lock();
}

template <class T, class Meta>
inline std::shared_ptr<EdgeT<T, Meta>>
EdgeT<T, Meta>::getNextEdge() const noexcept
{
  return m_nextEdge.lock();
}

template <class T, class Meta>
inline Vec3T<T>
EdgeT<T, Meta>::getX2X1() const noexcept
{
  EBGEOMETRY_EXPECT(!m_vertex.expired());

  const auto& x1 = this->getVertex()->getPosition();
  const auto& x2 = this->getOtherVertex()->getPosition();

  return x2 - x1;
}

template <class T, class Meta>
inline std::shared_ptr<FaceT<T, Meta>>
EdgeT<T, Meta>::getFace() noexcept
{
  return m_face.lock();
}

template <class T, class Meta>
inline std::shared_ptr<FaceT<T, Meta>>
EdgeT<T, Meta>::getFace() const noexcept
{
  return m_face.lock();
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
EdgeT<T, Meta>::projectPointToEdge(const Vec3& a_x0) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[2]));
  EBGEOMETRY_EXPECT(!m_vertex.expired());

  const auto p    = a_x0 - m_vertex.lock()->getPosition();
  const auto x2x1 = this->getX2X1();

  EBGEOMETRY_EXPECT(x2x1.dot(x2x1) > T(0));

  return p.dot(x2x1) / (x2x1.dot(x2x1));
}

template <class T, class Meta>
inline T
EdgeT<T, Meta>::signedDistance(const Vec3& a_x0) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[2]));
  EBGEOMETRY_EXPECT(!m_vertex.expired());

  T retval = std::numeric_limits<T>::max();

  // Project point to edge.
  const T t = this->projectPointToEdge(a_x0);

  if (t <= 0.0) {
    // Closest point is the starting vertex
    retval = this->getVertex()->signedDistance(a_x0);
  }
  else if (t >= 1.0) {
    // Closest point is the end vertex
    retval = this->getOtherVertex()->signedDistance(a_x0);
  }
  else {
    // Closest point is the edge itself.
    const Vec3 x2x1      = this->getX2X1();
    const Vec3 linePoint = m_vertex.lock()->getPosition() + t * x2x1;
    const Vec3 delta     = a_x0 - linePoint;
    const T    dot       = m_normal.dot(delta);

    const int sgn = (dot > 0.0) ? 1 : -1;

    retval = sgn * delta.length();
  }

  return retval;
}

template <class T, class Meta>
inline T
EdgeT<T, Meta>::unsignedDistance2(const Vec3& a_x0) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[2]));
  EBGEOMETRY_EXPECT(!m_vertex.expired());

  constexpr T zero = 0.0;
  constexpr T one  = 1.0;

  // Project point to edge and restrict to edge length.
  const auto t = std::clamp(this->projectPointToEdge(a_x0), zero, one);

  // Compute distance to this edge.
  const Vec3T<T> x2x1      = this->getX2X1();
  const Vec3T<T> linePoint = m_vertex.lock()->getPosition() + t * x2x1;
  const Vec3T<T> delta     = a_x0 - linePoint;

  return delta.dot(delta);
}
} // namespace DCEL

} // namespace EBGeometry

#endif
