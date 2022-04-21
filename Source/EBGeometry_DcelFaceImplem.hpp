/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_DcelFaceImplem.hpp
  @brief  Implementation of EBGeometry_DcelFace.hpp
  @author Robert Marskar
*/

#ifndef EBGeometry_DcelFaceImplem
#define EBGeometry_DcelFaceImplem

// Our includes
#include "EBGeometry_DcelFace.hpp"
#include "EBGeometry_DcelIterator.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

namespace Dcel {

  template <class T>
  inline
  FaceT<T>::FaceT(){
    m_halfEdge       = nullptr;
    m_normal         = Vec3::zero();
    m_poly2Algorithm = Polygon2D<T>::InsideOutsideAlgorithm::CrossingNumber;
  }

  template <class T>
  inline
  FaceT<T>::FaceT(const EdgePtr& a_edge) : Face() {
    m_halfEdge = a_edge;
  }

  template <class T>
  inline
  FaceT<T>::FaceT(const Face& a_otherFace) : Face() {
    m_normal   = a_otherFace.getNormal();
    m_halfEdge = a_otherFace.getHalfEdge();
  }

  template <class T>
  inline
  FaceT<T>::~FaceT(){
  }

  template <class T>
  inline
  void FaceT<T>::define(const Vec3& a_normal, const EdgePtr& a_edge) noexcept {
    m_normal   = a_normal;
    m_halfEdge = a_edge;
  }

  template <class T>
  inline
  void FaceT<T>::reconcile() noexcept {
    this->computeNormal();
    this->normalizeNormalVector();
    this->computeCentroid();
    this->computeArea();
    this->computePolygon2D();
    this->computeAndStoreEdges();
  }

  template <class T>
  inline
  void FaceT<T>::computeAndStoreEdges() noexcept {
    m_edges.resize(0);

    for (EdgeIterator edgeIt(*this); edgeIt.ok(); ++edgeIt){
      m_edges.emplace_back(edgeIt());
    }
  }

  template <class T>
  inline
  void FaceT<T>::setHalfEdge(const EdgePtr& a_halfEdge) noexcept {
    m_halfEdge = a_halfEdge;
  }

  template <class T>
  inline
  void FaceT<T>::normalizeNormalVector() noexcept {
    m_normal = m_normal/m_normal.length();
  }

  template <class T>
  inline
  void FaceT<T>::setInsideOutsideAlgorithm(typename Dcel::Polygon2D<T>::InsideOutsideAlgorithm& a_algorithm) noexcept {
    m_poly2Algorithm = a_algorithm;
  }

  template <class T>
  inline
  void FaceT<T>::computeArea() noexcept {
    m_area = 0.0;

    // This computes the area of any N-side polygon. 
    const auto vertices = this->gatherVertices();

    for (size_t i = 0; i < vertices.size() - 1; i++){
      const auto& v1 = vertices[i]  ->getPosition();
      const auto& v2 = vertices[i+1]->getPosition();
      m_area += m_normal.dot(v2.cross(v1));
    }

    m_area = 0.5*std::abs(m_area);
  }

  template <class T>
  inline
  void FaceT<T>::computeCentroid() noexcept {
    m_centroid = Vec3::zero();

    const auto vertices = this->gatherVertices();
  
    for (const auto& v : vertices){
      m_centroid += v->getPosition();
    }
  
    m_centroid = m_centroid/vertices.size();
  }

  template <class T>  
  inline
  void FaceT<T>::computeNormal() noexcept {
    const auto vertices = this->gatherVertices();
    
    const size_t N = vertices.size();

    // To compute the normal vector we find three vertices in this polygon face. They span a plane, and we just compute the
    // normal vector of that plane.
    for (size_t i = 0; i < N; i++){
      const auto& x0 = vertices[i]      ->getPosition();
      const auto& x1 = vertices[(i+1)%N]->getPosition();
      const auto& x2 = vertices[(i+2)%N]->getPosition();

      m_normal = (x2-x1).cross(x2-x0);
    
      if(m_normal.length() > 0.0) break; // Found one. 
    }

    this->normalizeNormalVector();
  }

  template <class T>
  inline
  void FaceT<T>::computePolygon2D() noexcept{

    // See CD_DcelPoly.H to see how the 2D embedding operates. 
    m_poly2 = std::make_shared<Polygon2D<T> >(m_normal, this->getAllVertexCoordinates());
  }

  template <class T>
  inline
  T& FaceT<T>::getCentroid(const size_t a_dir) noexcept {
    return m_centroid[a_dir];
  }

  template <class T>
  inline
  const T& FaceT<T>::getCentroid(const size_t a_dir) const noexcept {
    return m_centroid[a_dir];
  }

  template <class T>
  inline
  Vec3T<T>& FaceT<T>::getCentroid() noexcept {
    return (m_centroid);
  }

  template <class T>
  inline
  const Vec3T<T>& FaceT<T>::getCentroid() const noexcept {
    return (m_centroid);
  }

  template <class T>
  inline
  Vec3T<T>& FaceT<T>::getNormal() noexcept {
    return (m_normal);
  }

  template <class T>
  inline
  const Vec3T<T>& FaceT<T>::getNormal() const noexcept {
    return (m_normal);
  }

  template <class T>
  inline
  T FaceT<T>::getArea() noexcept {
    return (m_area);
  }

  template <class T>
  inline
  T FaceT<T>::getArea() const noexcept {
    return (m_area);
  }

  template <class T>
  inline
  std::shared_ptr<EdgeT<T> >& FaceT<T>::getHalfEdge() noexcept {
    return (m_halfEdge);
  }

  template <class T>
  inline
  const std::shared_ptr<EdgeT<T> >& FaceT<T>::getHalfEdge() const noexcept {
    return (m_halfEdge);
  }

  template <class T>
  inline
  std::vector<std::shared_ptr<VertexT<T> > > FaceT<T>::gatherVertices() const noexcept {
    std::vector<VertexPtr> vertices;

    for (EdgeIterator iter(*this); iter.ok(); ++iter){
      EdgePtr& edge = iter();
      vertices.emplace_back(edge->getVertex());
    }

    return vertices;
  }

  template <class T>
  inline
  std::vector<Vec3T<T> > FaceT<T>::getAllVertexCoordinates() const noexcept {
    std::vector<Vec3> ret;

    for (EdgeIterator iter(*this); iter.ok(); ++iter){
      EdgePtr& edge = iter();
      ret.emplace_back(edge->getVertex()->getPosition());
    }

    return ret;
  }

  template <class T>
  inline
  Vec3T<T> FaceT<T>::getSmallestCoordinate() const noexcept {
    const auto coords = this->getAllVertexCoordinates();

    auto minCoord = coords.front();

    for (const auto& c : coords){
      minCoord = min(minCoord, c);
    }

    return minCoord;
  }

  template <class T>
  inline
  Vec3T<T> FaceT<T>::getHighestCoordinate() const noexcept {
    const auto coords = this->getAllVertexCoordinates();

    auto maxCoord = coords.front();

    for (const auto& c : coords){
      maxCoord = max(maxCoord, c);
    }

    return maxCoord;
  }

  template <class T>
  inline
  Vec3T<T> FaceT<T>::projectPointIntoFacePlane(const Vec3& a_p) const noexcept {
    return a_p - m_normal * (m_normal.dot(a_p-m_centroid));
  }

  template <class T>
  inline
  bool FaceT<T>::isPointInsideFace(const Vec3& a_p) const noexcept {
    const Vec3 p = this->projectPointIntoFacePlane(a_p);

    return m_poly2->isPointInside(p, m_poly2Algorithm);
  }

  template <class T>
  inline
  T FaceT<T>::signedDistance(const Vec3& a_x0) const noexcept {
    T retval = std::numeric_limits<T>::infinity();

    const bool inside = this->isPointInsideFace(a_x0);

    if(inside){ // Projects to inside so distance and sign are straightforward to compute. 
      retval = m_normal.dot(a_x0 - m_centroid);
    }
    else {
      for (const auto& e : m_edges){ // Projects to outside so edge/vertex are closest. Check that distance. 
	const T curDist = e->signedDistance(a_x0);

	retval = (std::abs(curDist) < std::abs(retval)) ? curDist : retval;
      }
    }

    return retval;
  }

  template <class T>
  inline
  T FaceT<T>::unsignedDistance2(const Vec3& a_x0) const noexcept {
    T retval = std::numeric_limits<T>::infinity();

    const bool inside = this->isPointInsideFace(a_x0);

    if(inside){ // Projects to inside the polygon face so distance is straightforward. 
      const T normDist = m_normal.dot(a_x0-m_centroid);
      
      retval = normDist*normDist;
    }
    else{
      for (const auto& e : m_edges){ // Projects to outside so edge/vertex are closest.
	const T curDist2 = e->unsignedDistance2(a_x0);

	retval = (curDist2 < retval) ? curDist2 : retval;
      }
    }

    return retval;
  }
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
