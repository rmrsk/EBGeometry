/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_DCEL_EdgeImplem.hpp
  @brief  Implementation of EBGeometry_DCEL_Edge.hpp
  @author Robert Marskar
  @todo   Include m_face in constructors
*/

#ifndef CD_DCELEdgeImplem
#define CD_DCELEdgeImplem

// Our includes
#include "EBGeometry_DCEL_Vertex.hpp"
#include "EBGeometry_DCEL_Edge.hpp"
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

namespace DCEL {

  template <class T, class Meta>
  inline EdgeT<T, Meta>::EdgeT()
  {
    m_normal   = Vec3::zero();
    m_face     = nullptr;
    m_vertex   = nullptr;
    m_pairEdge = nullptr;
    m_nextEdge = nullptr;
  }

  template <class T, class Meta>
  inline EdgeT<T, Meta>::EdgeT(const VertexPtr& a_vertex) : EdgeT<T, Meta>()
  {
    m_vertex = a_vertex;
  }

  template <class T, class Meta>
  inline EdgeT<T, Meta>::EdgeT(const Edge& a_otherEdge) : EdgeT<T, Meta>()
  {
    m_normal   = a_otherEdge.m_normal;
    m_face     = a_otherEdge.m_face;
    m_vertex   = a_otherEdge.m_vertex;
    m_pairEdge = a_otherEdge.m_pairEdge;
    m_nextEdge = a_otherEdge.m_nextEdge;
  }

  template <class T, class Meta>
  inline EdgeT<T, Meta>::~EdgeT()
  {}

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
  EdgeT<T, Meta>::flip() noexcept
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
  inline Vec3T<T>
  EdgeT<T, Meta>::computeNormal() const noexcept
  {
    Vec3T<T> normal = Vec3T<T>::zero();

    if (m_face) {
      normal += m_face->getNormal();
    }
    if (m_pairEdge) {
      normal += m_pairEdge->getFace()->getNormal();
    }

    return normal / normal.length();
  }

  template <class T, class Meta>
  inline const Vec3T<T>&
  EdgeT<T, Meta>::getNormal() const noexcept
  {
    return (m_normal);
  }

  template <class T, class Meta>
  inline std::shared_ptr<VertexT<T, Meta>>&
  EdgeT<T, Meta>::getVertex() noexcept
  {
    return (m_vertex);
  }

  template <class T, class Meta>
  inline const std::shared_ptr<VertexT<T, Meta>>&
  EdgeT<T, Meta>::getVertex() const noexcept
  {
    return (m_vertex);
  }

  template <class T, class Meta>
  inline std::shared_ptr<VertexT<T, Meta>>&
  EdgeT<T, Meta>::getOtherVertex() noexcept
  {
    return (m_nextEdge->getVertex());
  }

  template <class T, class Meta>
  inline const std::shared_ptr<VertexT<T, Meta>>&
  EdgeT<T, Meta>::getOtherVertex() const noexcept
  {
    return (m_nextEdge->getVertex());
  }

  template <class T, class Meta>
  inline std::shared_ptr<EdgeT<T, Meta>>&
  EdgeT<T, Meta>::getPairEdge() noexcept
  {
    return (m_pairEdge);
  }

  template <class T, class Meta>
  inline const std::shared_ptr<EdgeT<T, Meta>>&
  EdgeT<T, Meta>::getPairEdge() const noexcept
  {
    return (m_pairEdge);
  }

  template <class T, class Meta>
  inline std::shared_ptr<EdgeT<T, Meta>>&
  EdgeT<T, Meta>::getNextEdge() noexcept
  {
    return (m_nextEdge);
  }

  template <class T, class Meta>
  inline const std::shared_ptr<EdgeT<T, Meta>>&
  EdgeT<T, Meta>::getNextEdge() const noexcept
  {
    return (m_nextEdge);
  }

  template <class T, class Meta>
  inline Vec3T<T>
  EdgeT<T, Meta>::getX2X1() const noexcept
  {
    const auto& x1 = this->getVertex()->getPosition();
    const auto& x2 = this->getOtherVertex()->getPosition();

    return x2 - x1;
  }

  template <class T, class Meta>
  inline std::shared_ptr<FaceT<T, Meta>>&
  EdgeT<T, Meta>::getFace() noexcept
  {
    return (m_face);
  }

  template <class T, class Meta>
  inline const std::shared_ptr<FaceT<T, Meta>>&
  EdgeT<T, Meta>::getFace() const noexcept
  {
    return (m_face);
  }

  template <class T, class Meta>  
  inline Meta&
  EdgeT<T, Meta>::getMetaData() noexcept {
    return (m_metaData);
  }

  template <class T, class Meta>  
  inline const Meta&
  EdgeT<T, Meta>::getMetaData() const noexcept {
    return (m_metaData);
  }  

  template <class T, class Meta>
  inline T
  EdgeT<T, Meta>::projectPointToEdge(const Vec3& a_x0) const noexcept
  {
    const auto p = a_x0 - m_vertex->getPosition();

    const auto x2x1 = this->getX2X1();

    return p.dot(x2x1) / (x2x1.dot(x2x1));
  }

  template <class T, class Meta>
  inline T
  EdgeT<T, Meta>::signedDistance(const Vec3& a_x0) const noexcept
  {
    // Project point to edge.
    const T t = this->projectPointToEdge(a_x0);

    T retval;
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
      const Vec3 linePoint = m_vertex->getPosition() + t * x2x1;
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
    constexpr T zero = 0.0;
    constexpr T one  = 1.0;

    // Project point to edge and restrict to edge length.
    const auto t = std::min(std::max(zero, this->projectPointToEdge(a_x0)), one);

    // Compute distance to this edge.
    const Vec3T<T> x2x1      = this->getX2X1();
    const Vec3T<T> linePoint = m_vertex->getPosition() + t * x2x1;
    const Vec3T<T> delta     = a_x0 - linePoint;

    return delta.dot(delta);
  }
} // namespace DCEL

#include "EBGeometry_NamespaceFooter.hpp"

#endif
