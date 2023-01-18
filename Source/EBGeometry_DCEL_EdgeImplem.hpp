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

  template <class T>
  inline EdgeT<T>::EdgeT()
  {
    m_normal   = Vec3::zero();
    m_face     = nullptr;
    m_vertex   = nullptr;
    m_pairEdge = nullptr;
    m_nextEdge = nullptr;
  }

  template <class T>
  inline EdgeT<T>::EdgeT(const VertexPtr& a_vertex) : EdgeT<T>()
  {
    m_vertex = a_vertex;
  }

  template <class T>
  inline EdgeT<T>::EdgeT(const Edge& a_otherEdge) : EdgeT<T>()
  {
    m_normal   = a_otherEdge.m_normal;
    m_face     = a_otherEdge.m_face;
    m_vertex   = a_otherEdge.m_vertex;
    m_pairEdge = a_otherEdge.m_pairEdge;
    m_nextEdge = a_otherEdge.m_nextEdge;
  }

  template <class T>
  inline EdgeT<T>::~EdgeT()
  {}

  template <class T>
  inline void
  EdgeT<T>::define(const VertexPtr& a_vertex, const EdgePtr& a_pairEdge, const EdgePtr& a_nextEdge) noexcept
  {
    m_vertex   = a_vertex;
    m_pairEdge = a_pairEdge;
    m_nextEdge = a_nextEdge;
  }

  template <class T>
  inline void
  EdgeT<T>::flip() noexcept
  {
    m_normal = -m_normal;
  }

  template <class T>
  inline void
  EdgeT<T>::reconcile() noexcept
  {
    m_normal = this->computeNormal();
  }

  template <class T>
  inline void
  EdgeT<T>::setVertex(const VertexPtr& a_vertex) noexcept
  {
    m_vertex = a_vertex;
  }

  template <class T>
  inline void
  EdgeT<T>::setPairEdge(const EdgePtr& a_pairEdge) noexcept
  {
    m_pairEdge = a_pairEdge;
  }

  template <class T>
  inline void
  EdgeT<T>::setNextEdge(const EdgePtr& a_nextEdge) noexcept
  {
    m_nextEdge = a_nextEdge;
  }

  template <class T>
  inline void
  EdgeT<T>::setFace(const FacePtr& a_face) noexcept
  {
    m_face = a_face;
  }

  template <class T>
  inline Vec3T<T>
  EdgeT<T>::computeNormal() const noexcept
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

  template <class T>
  inline const Vec3T<T>&
  EdgeT<T>::getNormal() const noexcept
  {
    return (m_normal);
  }

  template <class T>
  inline std::shared_ptr<VertexT<T>>&
  EdgeT<T>::getVertex() noexcept
  {
    return (m_vertex);
  }

  template <class T>
  inline const std::shared_ptr<VertexT<T>>&
  EdgeT<T>::getVertex() const noexcept
  {
    return (m_vertex);
  }

  template <class T>
  inline std::shared_ptr<VertexT<T>>&
  EdgeT<T>::getOtherVertex() noexcept
  {
    return (m_nextEdge->getVertex());
  }

  template <class T>
  inline const std::shared_ptr<VertexT<T>>&
  EdgeT<T>::getOtherVertex() const noexcept
  {
    return (m_nextEdge->getVertex());
  }

  template <class T>
  inline std::shared_ptr<EdgeT<T>>&
  EdgeT<T>::getPairEdge() noexcept
  {
    return (m_pairEdge);
  }

  template <class T>
  inline const std::shared_ptr<EdgeT<T>>&
  EdgeT<T>::getPairEdge() const noexcept
  {
    return (m_pairEdge);
  }

  template <class T>
  inline std::shared_ptr<EdgeT<T>>&
  EdgeT<T>::getNextEdge() noexcept
  {
    return (m_nextEdge);
  }

  template <class T>
  inline const std::shared_ptr<EdgeT<T>>&
  EdgeT<T>::getNextEdge() const noexcept
  {
    return (m_nextEdge);
  }

  template <class T>
  inline Vec3T<T>
  EdgeT<T>::getX2X1() const noexcept
  {
    const auto& x1 = this->getVertex()->getPosition();
    const auto& x2 = this->getOtherVertex()->getPosition();

    return x2 - x1;
  }

  template <class T>
  inline std::shared_ptr<FaceT<T>>&
  EdgeT<T>::getFace() noexcept
  {
    return (m_face);
  }

  template <class T>
  inline const std::shared_ptr<FaceT<T>>&
  EdgeT<T>::getFace() const noexcept
  {
    return (m_face);
  }

  template <class T>
  inline T
  EdgeT<T>::projectPointToEdge(const Vec3& a_x0) const noexcept
  {
    const auto p = a_x0 - m_vertex->getPosition();

    const auto x2x1 = this->getX2X1();

    return p.dot(x2x1) / (x2x1.dot(x2x1));
  }

  template <class T>
  inline T
  EdgeT<T>::signedDistance(const Vec3& a_x0) const noexcept
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

  template <class T>
  inline T
  EdgeT<T>::unsignedDistance2(const Vec3& a_x0) const noexcept
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
