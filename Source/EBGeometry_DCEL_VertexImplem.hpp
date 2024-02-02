/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_DCEL_VertexImplem.hpp
  @brief  Implementation of EBGeometry_DCEL_Vertex.hpp
  @author Robert Marskar
*/

#ifndef EBGeometry_DCEL_VertexImplem
#define EBGeometry_DCEL_VertexImplem

// Our includes
#include "EBGeometry_DCEL_Vertex.hpp"
#include "EBGeometry_DCEL_Edge.hpp"
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_DCEL_Iterator.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

namespace DCEL {

  template <class T, class Meta>
  inline VertexT<T, Meta>::VertexT()
  {
    m_position = Vec3::zero();
    m_normal   = Vec3::zero();

    m_faces.resize(0);
  }

  template <class T, class Meta>
  inline VertexT<T, Meta>::VertexT(const Vec3& a_position) : VertexT()
  {
    m_position = a_position;
  }

  template <class T, class Meta>
  inline VertexT<T, Meta>::VertexT(const Vec3& a_position, const Vec3& a_normal) : VertexT()
  {
    m_position = a_position;
    m_normal   = a_normal;
  }

  template <class T, class Meta>
  inline VertexT<T, Meta>::VertexT(const VertexT<T, Meta>& a_otherVertex)
  {
    m_position     = a_otherVertex.m_position;
    m_normal       = a_otherVertex.m_m_normal;
    m_outgoingEdge = a_otherVertex.m_outgoingEdge;
  }

  template <class T, class Meta>
  inline VertexT<T, Meta>::~VertexT()
  {}

  template <class T, class Meta>
  inline void
  VertexT<T, Meta>::define(const Vec3& a_position, const EdgePtr& a_edge, const Vec3& a_normal) noexcept
  {
    m_position     = a_position;
    m_outgoingEdge = a_edge;
    m_normal       = a_normal;
  }

  template <class T, class Meta>
  inline void
  VertexT<T, Meta>::setPosition(const Vec3& a_position) noexcept
  {
    m_position = a_position;
  }

  template <class T, class Meta>
  inline void
  VertexT<T, Meta>::setEdge(const EdgePtr& a_edge) noexcept
  {
    m_outgoingEdge = a_edge;
  }

  template <class T, class Meta>
  inline void
  VertexT<T, Meta>::setNormal(const Vec3& a_normal) noexcept
  {
    m_normal = a_normal;
  }

  template <class T, class Meta>
  inline void
  VertexT<T, Meta>::addFace(const FacePtr& a_face) noexcept
  {
    m_faces.emplace_back(a_face);
  }

  template <class T, class Meta>
  inline void
  VertexT<T, Meta>::normalizeNormalVector() noexcept
  {
    m_normal = m_normal / m_normal.length();
  }

  template <class T, class Meta>
  inline void
  VertexT<T, Meta>::computeVertexNormalAverage() noexcept
  {
    this->computeVertexNormalAverage(m_faces);
  }

  template <class T, class Meta>
  inline void
  VertexT<T, Meta>::computeVertexNormalAverage(const std::vector<FacePtr>& a_faces) noexcept
  {
    m_normal = Vec3::zero();

    // TLDR: We simply compute the sum of the normal vectors for each face in
    // a_faces and then normalize. This
    //       will yield an "average" of the normal vectors of the faces
    //       circulating this vertex.
    for (const auto& f : a_faces) {
      m_normal += f->getNormal();
    }

    this->normalizeNormalVector();
  }

  template <class T, class Meta>
  inline void
  VertexT<T, Meta>::computeVertexNormalAngleWeighted() noexcept
  {
    this->computeVertexNormalAngleWeighted(m_faces);
  }

  template <class T, class Meta>
  inline void
  VertexT<T, Meta>::computeVertexNormalAngleWeighted(const std::vector<FacePtr>& a_faces) noexcept
  {
    m_normal = Vec3::zero();

    // This routine computes the pseudonormal from pseudnormal algorithm from
    // Baerentzen and Aanes in "Signed distance computation using the angle
    // weighted pseudonormal" (DOI: 10.1109/TVCG.2005.49). This algorithm computes
    // an average normal vector using the normal vectors of each face connected to
    // this vertex, i.e. in the form
    //
    //    n = sum(w * n(face))/sum(w)
    //
    // where w are weights for each face. This weight is given by the subtended
    // angle of the face, which means the angle spanned by the incoming/outgoing
    // edges of the face that pass through this vertex.
    //
    //
    // The below code is more complicated than it looks. It happens because we
    // want the two half edges that has the current vertex as a mutual vertex
    // (i.e. the "incoming" and "outgoing" edges into this vertex). Normally we'd
    // just iterate through edges, but if it happens that an input face is
    // flipped, this will result in infinite iteration. Instead, we have stored
    // the pointers to each face connected to this vertex. We look through each
    // face to find the endpoints of the edges the have the current vertex as the
    // common vertex, and then compute the subtended angle between those. Sigh...

    const VertexPtr& originVertex = m_outgoingEdge->getVertex(); // AKA 'this'

    for (const auto& f : a_faces) {

      std::vector<VertexPtr> inoutVertices(0);
      for (EdgeIterator edgeIt(f->getHalfEdge()); edgeIt.ok(); ++edgeIt) {
        const auto& e = edgeIt();

        const auto& v1 = e->getVertex();
        const auto& v2 = e->getOtherVertex();

        if (v1 == originVertex || v2 == originVertex) {
          if (v1 == originVertex) {
            inoutVertices.emplace_back(v2);
          }
          else if (v2 == originVertex) {
            inoutVertices.emplace_back(v1);
          }
          else {
            std::cerr << "In file 'CD_DCELVertexImplem.H' function "
                         "vertexT<T>::computeVertexNormalAngleWeighted() - logic bust.\n";
          }
        }
      }

      if (inoutVertices.size() != 2) {
        std::cerr << "In file 'CD_DCELVertexImplem.H' function "
                     "vertexT<T>::computeVertexNormalAngleWeighted() - logic bust 2.\n";
      }

      const Vec3& x0 = originVertex->getPosition();
      const Vec3& x1 = inoutVertices[0]->getPosition();
      const Vec3& x2 = inoutVertices[1]->getPosition();

      if (x0 == x1 || x0 == x2 || x1 == x2) {
        std::cerr << "In file 'CD_DCELVertexImplem.H' function "
                     "vertexT<T>::computeVertexNormalAngleWeighted() - logic bust 3.\n";
      }

      Vec3 v1 = x1 - x0;
      Vec3 v2 = x2 - x0;

      v1 = v1 / v1.length();
      v2 = v2 / v2.length();

      const Vec3& norm = f->getNormal();

      const T alpha = acos(v1.dot(v2));

      m_normal += alpha * norm;
    }

    this->normalizeNormalVector();
  }

  template <class T, class Meta>
  inline void
  VertexT<T, Meta>::flip() noexcept
  {
    m_normal = -m_normal;
  }

  template <class T, class Meta>
  inline Vec3T<T>&
  VertexT<T, Meta>::getPosition() noexcept
  {
    return (m_position);
  }

  template <class T, class Meta>
  inline const Vec3T<T>&
  VertexT<T, Meta>::getPosition() const noexcept
  {
    return (m_position);
  }

  template <class T, class Meta>
  inline Vec3T<T>&
  VertexT<T, Meta>::getNormal() noexcept
  {
    return (m_normal);
  }

  template <class T, class Meta>
  inline const Vec3T<T>&
  VertexT<T, Meta>::getNormal() const noexcept
  {
    return (m_normal);
  }

  template <class T, class Meta>
  inline std::shared_ptr<EdgeT<T>>&
  VertexT<T, Meta>::getOutgoingEdge() noexcept
  {
    return (m_outgoingEdge);
  }

  template <class T, class Meta>
  inline const std::shared_ptr<EdgeT<T>>&
  VertexT<T, Meta>::getOutgoingEdge() const noexcept
  {
    return (m_outgoingEdge);
  }

  template <class T, class Meta>
  inline std::vector<std::shared_ptr<FaceT<T>>>&
  VertexT<T, Meta>::getFaces() noexcept
  {
    return (m_faces);
  }

  template <class T, class Meta>
  inline const std::vector<std::shared_ptr<FaceT<T>>>&
  VertexT<T, Meta>::getFaces() const noexcept
  {
    return (m_faces);
  }

  template <class T, class Meta>
  inline T
  VertexT<T, Meta>::signedDistance(const Vec3& a_x0) const noexcept
  {
    const auto delta = a_x0 - m_position;
    const T    dist  = delta.length();
    const T    dot   = m_normal.dot(delta);
    const int  sign  = (dot > 0.) ? 1 : -1;

    return dist * sign;
  }

  template <class T, class Meta>
  inline T
  VertexT<T, Meta>::unsignedDistance2(const Vec3& a_x0) const noexcept
  {
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

#include "EBGeometry_NamespaceFooter.hpp"

#endif
