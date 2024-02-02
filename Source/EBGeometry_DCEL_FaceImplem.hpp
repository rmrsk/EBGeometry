/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_DCEL_FaceImplem.hpp
  @brief  Implementation of EBGeometry_DCEL_Face.hpp
  @author Robert Marskar
*/

#ifndef EBGeometry_DCEL_FaceImplem
#define EBGeometry_DCEL_FaceImplem

// Our includes
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_DCEL_Iterator.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

namespace DCEL {

  template <class T, class Meta>
  inline FaceT<T, Meta>::FaceT()
  {
    m_halfEdge       = nullptr;
    m_normal         = Vec3::zero();
    m_poly2Algorithm = Polygon2D<T>::InsideOutsideAlgorithm::CrossingNumber;
  }

  template <class T, class Meta>
  inline FaceT<T, Meta>::FaceT(const EdgePtr& a_edge) : Face()
  {
    m_halfEdge = a_edge;
  }

  template <class T, class Meta>
  inline FaceT<T, Meta>::FaceT(const Face& a_otherFace) : Face()
  {
    m_normal   = a_otherFace.getNormal();
    m_halfEdge = a_otherFace.getHalfEdge();
  }

  template <class T, class Meta>
  inline FaceT<T, Meta>::~FaceT()
  {}

  template <class T, class Meta>
  inline void
  FaceT<T, Meta>::define(const Vec3& a_normal, const EdgePtr& a_edge) noexcept
  {
    m_normal   = a_normal;
    m_halfEdge = a_edge;
  }

  template <class T, class Meta>
  inline void
  FaceT<T, Meta>::reconcile() noexcept
  {
    this->computeNormal();
    this->normalizeNormalVector();
    this->computeCentroid();
    this->computeArea();
    this->computePolygon2D();
  }

  template <class T, class Meta>
  inline void
  FaceT<T, Meta>::flip() noexcept
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
  FaceT<T, Meta>::normalizeNormalVector() noexcept
  {
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
  FaceT<T, Meta>::computeCentroid() noexcept
  {
    m_centroid = Vec3::zero();

    const auto vertices = this->gatherVertices();

    for (const auto& v : vertices) {
      m_centroid += v->getPosition();
    }

    m_centroid = m_centroid / vertices.size();
  }

  template <class T, class Meta>
  inline void
  FaceT<T, Meta>::computeNormal() noexcept
  {
    const auto vertices = this->gatherVertices();

    const size_t N = vertices.size();

    // To compute the normal vector we find three vertices in this polygon face.
    // They span a plane, and we just compute the normal vector of that plane.
    for (size_t i = 0; i < N; i++) {
      const auto& x0 = vertices[i]->getPosition();
      const auto& x1 = vertices[(i + 1) % N]->getPosition();
      const auto& x2 = vertices[(i + 2) % N]->getPosition();

      m_normal = (x2 - x1).cross(x2 - x0);

      if (m_normal.length() > 0.0) {
        break; // Found one.
      }
    }

    this->normalizeNormalVector();
  }

  template <class T, class Meta>
  inline void
  FaceT<T, Meta>::computePolygon2D() noexcept
  {
    m_poly2 = std::make_shared<Polygon2D<T>>(m_normal, this->getAllVertexCoordinates());
  }

  template <class T, class Meta>
  inline T
  FaceT<T, Meta>::computeArea() noexcept
  {

    T area = 0.0;

    // This computes the area of any N-side polygon.
    const auto vertices = this->gatherVertices();

    for (size_t i = 0; i < vertices.size() - 1; i++) {
      const auto& v1 = vertices[i]->getPosition();
      const auto& v2 = vertices[i + 1]->getPosition();

      area += m_normal.dot(v2.cross(v1));
    }

    area = 0.5 * std::abs(area);

    return area;
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
    return (m_centroid);
  }

  template <class T, class Meta>
  inline const Vec3T<T>&
  FaceT<T, Meta>::getCentroid() const noexcept
  {
    return (m_centroid);
  }

  template <class T, class Meta>
  inline Vec3T<T>&
  FaceT<T, Meta>::getNormal() noexcept
  {
    return (m_normal);
  }

  template <class T, class Meta>
  inline const Vec3T<T>&
  FaceT<T, Meta>::getNormal() const noexcept
  {
    return (m_normal);
  }

  template <class T, class Meta>
  inline std::shared_ptr<EdgeT<T, Meta>>&
  FaceT<T, Meta>::getHalfEdge() noexcept
  {
    return (m_halfEdge);
  }

  template <class T, class Meta>
  inline const std::shared_ptr<EdgeT<T, Meta>>&
  FaceT<T, Meta>::getHalfEdge() const noexcept
  {
    return (m_halfEdge);
  }

  template <class T, class Meta>  
  inline Meta&
  FaceT<T, Meta>::getMetaData() noexcept {
    return (m_metaData);
  }

  template <class T, class Meta>  
  inline const Meta&
  FaceT<T, Meta>::getMetaData() const noexcept {
    return (m_metaData);
  }    

  template <class T, class Meta>
  inline std::vector<std::shared_ptr<VertexT<T, Meta>>>
  FaceT<T, Meta>::gatherVertices() const noexcept
  {
    std::vector<VertexPtr> vertices;

    for (EdgeIterator iter(*this); iter.ok(); ++iter) {
      EdgePtr& edge = iter();
      vertices.emplace_back(edge->getVertex());
    }

    return vertices;
  }

  template <class T, class Meta>
  inline std::vector<Vec3T<T>>
  FaceT<T, Meta>::getAllVertexCoordinates() const noexcept
  {
    std::vector<Vec3> ret;

    for (EdgeIterator iter(*this); iter.ok(); ++iter) {
      EdgePtr& edge = iter();
      ret.emplace_back(edge->getVertex()->getPosition());
    }

    return ret;
  }

  template <class T, class Meta>
  inline Vec3T<T>
  FaceT<T, Meta>::getSmallestCoordinate() const noexcept
  {
    const auto coords = this->getAllVertexCoordinates();

    auto minCoord = coords.front();

    for (const auto& c : coords) {
      minCoord = min(minCoord, c);
    }

    return minCoord;
  }

  template <class T, class Meta>
  inline Vec3T<T>
  FaceT<T, Meta>::getHighestCoordinate() const noexcept
  {
    const auto coords = this->getAllVertexCoordinates();

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
    return a_p - m_normal * (m_normal.dot(a_p - m_centroid));
  }

  template <class T, class Meta>
  inline bool
  FaceT<T, Meta>::isPointInsideFace(const Vec3& a_p) const noexcept
  {
    const Vec3 p = this->projectPointIntoFacePlane(a_p);

    return m_poly2->isPointInside(p, m_poly2Algorithm);
  }

  template <class T, class Meta>
  inline T
  FaceT<T, Meta>::signedDistance(const Vec3& a_x0) const noexcept
  {
    T retval = std::numeric_limits<T>::infinity();

    const bool inside = this->isPointInsideFace(a_x0);

    if (inside) {
      retval = m_normal.dot(a_x0 - m_centroid);
    }
    else {
      EdgePtr cur = m_halfEdge;

      while (true) {
        const T curDist = cur->signedDistance(a_x0);

        retval = (std::abs(curDist) < std::abs(retval)) ? curDist : retval;

        cur = cur->getNextEdge();

        if (cur == nullptr || cur == m_halfEdge) {
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
    T retval = std::numeric_limits<T>::infinity();

    const bool inside = this->isPointInsideFace(a_x0);

    if (inside) {
      const T normDist = m_normal.dot(a_x0 - m_centroid);

      retval = normDist * normDist;
    }
    else {
      EdgePtr cur = m_halfEdge;

      while (true) {
        const T curDist2 = cur->unsignedDistance2(a_x0);

        retval = (curDist2 < retval) ? curDist2 : retval;

        cur = cur->getNextEdge();

        if (cur == nullptr || cur == m_halfEdge) {
          break;
        }
      }
    }

    return retval;
  }
} // namespace DCEL

#include "EBGeometry_NamespaceFooter.hpp"

#endif
