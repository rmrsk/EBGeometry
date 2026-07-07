// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_DCEL_FaceImplem.hpp
 * @brief  Implementation of EBGeometry_DCEL_Face.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_DCEL_FACEIMPLEM_HPP
#define EBGEOMETRY_DCEL_FACEIMPLEM_HPP

// Std includes
#include <cmath>
#include <cstddef>
#include <limits>
#include <memory>
#include <vector>

// Our includes
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_DCEL_Iterator.hpp"

namespace EBGeometry {

namespace DCEL {

template <class T, class Meta>
inline FaceT<T, Meta>::FaceT(const EdgePtr& a_edge)
{
  m_halfEdge = a_edge;
}

template <class T, class Meta>
inline FaceT<T, Meta>::FaceT(const Face& a_otherFace)
{
  m_normal   = a_otherFace.m_normal;
  m_halfEdge = a_otherFace.m_halfEdge;
  m_centroid = a_otherFace.m_centroid;
  m_area     = a_otherFace.m_area;
}

template <class T, class Meta>
inline FaceT<T, Meta>&
FaceT<T, Meta>::operator=(const Face& a_otherFace) noexcept
{
  if (this != &a_otherFace) {
    m_normal   = a_otherFace.m_normal;
    m_halfEdge = a_otherFace.m_halfEdge;
    m_centroid = a_otherFace.m_centroid;
    m_area     = a_otherFace.m_area;
  }

  return *this;
}

template <class T, class Meta>
inline void
FaceT<T, Meta>::define(const Vec3& a_normal, const EdgePtr& a_edge) noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_normal[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_normal[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_normal[2]));

  // a_edge == nullptr is valid here (e.g. a freshly-created face not yet wired into the mesh).
  m_normal   = a_normal;
  m_halfEdge = a_edge;
}

template <class T, class Meta>
inline void
FaceT<T, Meta>::reconcile()
{
  this->computeNormal();
  this->normalizeNormalVector();
  this->computeCentroid();
  this->computeArea();
  this->computePolygon2D();
}

template <class T, class Meta>
inline void
FaceT<T, Meta>::flipNormal() noexcept
{
  m_normal = -m_normal;
}

template <class T, class Meta>
inline void
FaceT<T, Meta>::setHalfEdge(const EdgePtr& a_halfEdge) noexcept
{
  m_halfEdge = a_halfEdge;
}

template <class T, class Meta>
inline void
FaceT<T, Meta>::setMetaData(const Meta& a_metaData) noexcept
{
  m_metaData = a_metaData;
}

template <class T, class Meta>
inline void
FaceT<T, Meta>::normalizeNormalVector() noexcept
{
  EBGEOMETRY_EXPECT(m_normal.length() > std::numeric_limits<T>::epsilon());

  m_normal = m_normal / m_normal.length();
}

template <class T, class Meta>
inline void
FaceT<T, Meta>::setInsideOutsideAlgorithm(typename Polygon2D<T>::InsideOutsideAlgorithm& a_algorithm) noexcept
{
  m_poly2Algorithm = a_algorithm;
}

template <class T, class Meta>
inline void
FaceT<T, Meta>::computeCentroid()
{
  m_centroid = Vec3::zeros();

  const auto vertices = this->gatherVertices();

  EBGEOMETRY_EXPECT(!vertices.empty());

  for (const auto& v : vertices) {
    m_centroid += v->getPosition();
  }

  m_centroid = m_centroid / vertices.size();
}

template <class T, class Meta>
inline void
FaceT<T, Meta>::computeNormal()
{
  const auto vertices = this->gatherVertices();

  const size_t N = vertices.size();

  // A polygon face needs at least 3 vertices to span a plane.
  EBGEOMETRY_EXPECT(N >= 3);

  // To compute the normal vector we find three vertices in this polygon face.
  // They span a plane, and we just compute the normal vector of that plane.
  for (size_t i = 0; i < N; i++) {
    const auto& x0 = vertices[i]->getPosition();
    const auto& x1 = vertices[(i + 1) % N]->getPosition();
    const auto& x2 = vertices[(i + 2) % N]->getPosition();

    m_normal = (x2 - x0).cross(x2 - x1);

    if (m_normal.length() > T(0.0)) {
      break; // Found one.
    }
  }

  // If every vertex triple was degenerate (collinear/coincident points), m_normal is still zero
  // here and normalizeNormalVector() below would divide by zero.
  EBGEOMETRY_EXPECT(m_normal.length() > std::numeric_limits<T>::epsilon());

  this->normalizeNormalVector();
}

template <class T, class Meta>
inline void
FaceT<T, Meta>::computePolygon2D()
{
  m_poly2 = std::make_shared<Polygon2D<T>>(m_normal, this->getAllVertexCoordinates());
}

template <class T, class Meta>
inline void
FaceT<T, Meta>::computeArea()
{
  T area = 0.0;

  // This computes the area of any N-side polygon.
  const auto   vertices = this->gatherVertices();
  const size_t N        = vertices.size();

  EBGEOMETRY_EXPECT(N >= 3);

  // Sum over ALL N edges, including the wraparound edge from vertices[N-1] back to vertices[0] --
  // this is the standard cross-product/shoelace formula for a planar polygon's area relative to
  // the origin, and omitting the wraparound edge silently produces a wrong (generally too large or
  // too small) area for any polygon that doesn't happen to pass through the origin.
  for (size_t i = 0; i < N; i++) {
    const auto& v1 = vertices[i]->getPosition();
    const auto& v2 = vertices[(i + 1) % N]->getPosition();

    area += m_normal.dot(v2.cross(v1));
  }

  m_area = T(0.5) * std::abs(area);
}

template <class T, class Meta>
inline T&
FaceT<T, Meta>::getArea() noexcept
{
  return m_area;
}

template <class T, class Meta>
inline const T&
FaceT<T, Meta>::getArea() const noexcept
{
  return m_area;
}

template <class T, class Meta>
inline T&
FaceT<T, Meta>::getCentroid(const size_t a_dir) noexcept
{
  return m_centroid[a_dir];
}

template <class T, class Meta>
inline const T&
FaceT<T, Meta>::getCentroid(const size_t a_dir) const noexcept
{
  return m_centroid[a_dir];
}

template <class T, class Meta>
inline Vec3T<T>&
FaceT<T, Meta>::getCentroid() noexcept
{
  return m_centroid;
}

template <class T, class Meta>
inline const Vec3T<T>&
FaceT<T, Meta>::getCentroid() const noexcept
{
  return m_centroid;
}

template <class T, class Meta>
inline Vec3T<T>&
FaceT<T, Meta>::getNormal() noexcept
{
  return m_normal;
}

template <class T, class Meta>
inline const Vec3T<T>&
FaceT<T, Meta>::getNormal() const noexcept
{
  return m_normal;
}

template <class T, class Meta>
inline std::shared_ptr<EdgeT<T, Meta>>
FaceT<T, Meta>::getHalfEdge() noexcept
{
  return m_halfEdge.lock();
}

template <class T, class Meta>
inline std::shared_ptr<EdgeT<T, Meta>>
FaceT<T, Meta>::getHalfEdge() const noexcept
{
  return m_halfEdge.lock();
}

template <class T, class Meta>
inline Meta&
FaceT<T, Meta>::getMetaData() noexcept
{
  return m_metaData;
}

template <class T, class Meta>
inline const Meta&
FaceT<T, Meta>::getMetaData() const noexcept
{
  return m_metaData;
}

template <class T, class Meta>
inline std::vector<std::shared_ptr<VertexT<T, Meta>>>
FaceT<T, Meta>::gatherVertices() const
{
  std::vector<VertexPtr> vertices;
  vertices.reserve(3);

  for (EdgeIterator iter(*this); iter.ok(); ++iter) {
    const EdgePtr& edge = iter();
    vertices.emplace_back(edge->getVertex());
  }

  return vertices;
}

template <class T, class Meta>
inline std::vector<std::shared_ptr<EdgeT<T, Meta>>>
FaceT<T, Meta>::gatherEdges() const
{
  std::vector<EdgePtr> edges;
  edges.reserve(3);

  for (EdgeIterator iter(*this); iter.ok(); ++iter) {
    const EdgePtr& edge = iter();

    edges.emplace_back(edge);
  }

  return edges;
}

template <class T, class Meta>
inline std::vector<Vec3T<T>>
FaceT<T, Meta>::getAllVertexCoordinates() const
{
  std::vector<Vec3> ret;
  ret.reserve(3);

  for (EdgeIterator iter(*this); iter.ok(); ++iter) {
    const EdgePtr& edge = iter();

    ret.emplace_back(edge->getVertex()->getPosition());
  }

  return ret;
}

template <class T, class Meta>
inline Vec3T<T>
FaceT<T, Meta>::getSmallestCoordinate() const
{
  const auto coords = this->getAllVertexCoordinates();

  EBGEOMETRY_EXPECT(!coords.empty());

  auto minCoord = coords.front();

  for (const auto& c : coords) {
    minCoord = min(minCoord, c);
  }

  return minCoord;
}

template <class T, class Meta>
inline Vec3T<T>
FaceT<T, Meta>::getHighestCoordinate() const
{
  const auto coords = this->getAllVertexCoordinates();

  EBGEOMETRY_EXPECT(!coords.empty());

  auto maxCoord = coords.front();

  for (const auto& c : coords) {
    maxCoord = max(maxCoord, c);
  }

  return maxCoord;
}

template <class T, class Meta>
inline Vec3T<T>
FaceT<T, Meta>::projectPointIntoFacePlane(const Vec3& a_p) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_p[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_p[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_p[2]));

  return a_p - m_normal * (m_normal.dot(a_p - m_centroid));
}

template <class T, class Meta>
inline bool
FaceT<T, Meta>::isPointInsideFace(const Vec3& a_p) const noexcept
{
  EBGEOMETRY_EXPECT(m_poly2 != nullptr);

  const Vec3 p = this->projectPointIntoFacePlane(a_p);

  return m_poly2->isPointInside(p, m_poly2Algorithm);
}

template <class T, class Meta>
inline T
FaceT<T, Meta>::signedDistance(const Vec3& a_x0) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[2]));
  EBGEOMETRY_EXPECT(!m_halfEdge.expired());

  T retval = std::numeric_limits<T>::infinity();

  const bool inside = this->isPointInsideFace(a_x0);

  if (inside) {
    retval = m_normal.dot(a_x0 - m_centroid);
  }
  else {
    const EdgePtr startEdge = m_halfEdge.lock();
    EdgePtr       cur       = startEdge;

    while (true) {
      const T curDist = cur->signedDistance(a_x0);

      retval = (std::abs(curDist) < std::abs(retval)) ? curDist : retval;

      cur = cur->getNextEdge();

      if (cur == nullptr || cur == startEdge) {
        break;
      }
    }
  }

  return retval;
}

template <class T, class Meta>
inline T
FaceT<T, Meta>::unsignedDistance2(const Vec3& a_x0) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[2]));
  EBGEOMETRY_EXPECT(!m_halfEdge.expired());

  T retval = std::numeric_limits<T>::infinity();

  const bool inside = this->isPointInsideFace(a_x0);

  if (inside) {
    const T normDist = m_normal.dot(a_x0 - m_centroid);

    retval = normDist * normDist;
  }
  else {
    const EdgePtr startEdge = m_halfEdge.lock();
    EdgePtr       cur       = startEdge;

    while (true) {
      const T curDist2 = cur->unsignedDistance2(a_x0);

      retval = (curDist2 < retval) ? curDist2 : retval;

      cur = cur->getNextEdge();

      if (cur == nullptr || cur == startEdge) {
        break;
      }
    }
  }

  return retval;
}
} // namespace DCEL

} // namespace EBGeometry

#endif
