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
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <vector>

// Our includes
#include "EBGeometry_Constants.hpp"
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
  m_normal                 = a_otherFace.m_normal;
  m_halfEdge               = a_otherFace.m_halfEdge;
  m_centroid               = a_otherFace.m_centroid;
  m_area                   = a_otherFace.m_area;
  m_xDir                   = a_otherFace.m_xDir;
  m_yDir                   = a_otherFace.m_yDir;
  m_insideOutsideAlgorithm = a_otherFace.m_insideOutsideAlgorithm;
  m_metaData               = a_otherFace.m_metaData;
}

template <class T, class Meta>
inline FaceT<T, Meta>&
FaceT<T, Meta>::operator=(const Face& a_otherFace) noexcept
{
  if (this != &a_otherFace) {
    m_normal                 = a_otherFace.m_normal;
    m_halfEdge               = a_otherFace.m_halfEdge;
    m_centroid               = a_otherFace.m_centroid;
    m_area                   = a_otherFace.m_area;
    m_xDir                   = a_otherFace.m_xDir;
    m_yDir                   = a_otherFace.m_yDir;
    m_insideOutsideAlgorithm = a_otherFace.m_insideOutsideAlgorithm;
    m_metaData               = a_otherFace.m_metaData;
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
  this->computeProjectionDirections();
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
FaceT<T, Meta>::setInsideOutsideAlgorithm(InsideOutsideAlgorithm a_algorithm) noexcept
{
  m_insideOutsideAlgorithm = a_algorithm;
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
FaceT<T, Meta>::computeProjectionDirections()
{
  EBGEOMETRY_EXPECT(m_normal.length() > std::numeric_limits<T>::epsilon());

  // Drop the coordinate of the largest normal component (projecting along it maximizes the projected
  // area), keeping the other two as the 2D x- and y-axes with m_xDir < m_yDir.
  uint32_t ignoreDir = 0;

  for (uint32_t dir = 1; dir < 3; dir++) {
    if (std::abs(m_normal[dir]) > std::abs(m_normal[ignoreDir])) {
      ignoreDir = dir;
    }
  }

  m_xDir = 3;
  m_yDir = 0;

  for (uint32_t dir = 0; dir < 3; dir++) {
    if (dir != ignoreDir) {
      m_xDir = std::min(m_xDir, dir);
      m_yDir = std::max(m_yDir, dir);
    }
  }

  EBGEOMETRY_EXPECT(m_xDir < 3);
  EBGEOMETRY_EXPECT(m_yDir < 3);
  EBGEOMETRY_EXPECT(m_xDir != m_yDir);
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
inline Vec2T<T>
FaceT<T, Meta>::projectPoint(const Vec3& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(m_xDir < 3);
  EBGEOMETRY_EXPECT(m_yDir < 3);

  return Vec2T<T>(a_point[m_xDir], a_point[m_yDir]);
}

template <class T, class Meta>
inline int
FaceT<T, Meta>::computeWindingNumber(const Vec2T<T>& a_point) const noexcept
{
  int wn = 0;

  const auto isLeft = [](const Vec2T<T>& P0, const Vec2T<T>& P1, const Vec2T<T>& P2) {
    return (P1.x - P0.x) * (P2.y - P0.y) - (P2.x - P0.x) * (P1.y - P0.y);
  };

  for (EdgeIterator iter(*this); iter.ok(); ++iter) {
    const EdgePtr& edge = iter();
    const Vec2T<T> P1   = this->projectPoint(edge->getVertex()->getPosition());
    const Vec2T<T> P2   = this->projectPoint(edge->getNextEdge()->getVertex()->getPosition());
    const T        res  = isLeft(P1, P2, a_point);

    if (P1.y <= a_point.y) {
      if (P2.y > a_point.y) {
        if (res > T(0.)) {
          ++wn;
        }
      }
    }
    else {
      if (P2.y <= a_point.y) {
        if (res < T(0.)) {
          --wn;
        }
      }
    }
  }

  return wn;
}

template <class T, class Meta>
inline size_t
FaceT<T, Meta>::computeCrossingNumber(const Vec2T<T>& a_point) const noexcept
{
  size_t cn = 0;

  for (EdgeIterator iter(*this); iter.ok(); ++iter) {
    const EdgePtr& edge = iter();
    const Vec2T<T> P1   = this->projectPoint(edge->getVertex()->getPosition());
    const Vec2T<T> P2   = this->projectPoint(edge->getNextEdge()->getVertex()->getPosition());

    // clang-format off
    const bool upwardCrossing   = (P1.y <= a_point.y) && (P2.y >  a_point.y);
    const bool downwardCrossing = (P1.y >  a_point.y) && (P2.y <= a_point.y);
    // clang-format on

    if (upwardCrossing || downwardCrossing) {
      // Guaranteed non-zero by construction, as in the winding/crossing convention above.
      EBGEOMETRY_EXPECT(P2.y != P1.y);

      const T t = (a_point.y - P1.y) / (P2.y - P1.y);

      if (a_point.x < P1.x + t * (P2.x - P1.x)) {
        cn += 1;
      }
    }
  }

  return cn;
}

template <class T, class Meta>
inline T
FaceT<T, Meta>::computeSubtendedAngle(const Vec2T<T>& a_point) const noexcept
{
  constexpr T pi = EBGeometry::pi<T>;

  T sumTheta = T(0);

  for (EdgeIterator iter(*this); iter.ok(); ++iter) {
    const EdgePtr& edge = iter();
    const Vec2T<T> p1   = this->projectPoint(edge->getVertex()->getPosition()) - a_point;
    const Vec2T<T> p2   = this->projectPoint(edge->getNextEdge()->getVertex()->getPosition()) - a_point;

    const T theta1 = std::atan2(p1.y, p1.x);
    const T theta2 = std::atan2(p2.y, p2.x);

    T dTheta = theta2 - theta1;

    while (dTheta > pi) {
      dTheta -= T(2) * pi;
    }

    while (dTheta < -pi) {
      dTheta += T(2) * pi;
    }

    sumTheta += dTheta;
  }

  return sumTheta;
}

template <class T, class Meta>
inline bool
FaceT<T, Meta>::isPointInsideFace(const Vec3& a_p) const noexcept
{
  EBGEOMETRY_EXPECT(m_xDir < 3);
  EBGEOMETRY_EXPECT(m_yDir < 3);

  const Vec3     pInPlane = this->projectPointIntoFacePlane(a_p);
  const Vec2T<T> p2D      = this->projectPoint(pInPlane);

  switch (m_insideOutsideAlgorithm) {
  case InsideOutsideAlgorithm::WindingNumber: {
    return this->computeWindingNumber(p2D) != 0;
  }
  case InsideOutsideAlgorithm::CrossingNumber: {
    return (this->computeCrossingNumber(p2D) % 2) == 1;
  }
  case InsideOutsideAlgorithm::SubtendedAngle: {
    constexpr T pi = EBGeometry::pi<T>;

    const T sumTheta = std::abs(this->computeSubtendedAngle(p2D)) / (T(2) * pi);

    return std::abs(sumTheta - T(1)) < T(0.5);
  }
  }

  return false;
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
