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
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>

// Our includes
#include "EBGeometry_DCEL_Edge.hpp"
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_DCEL_Iterator.hpp"
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
inline VertexT<T, Meta>::VertexT(const VertexT<T, Meta>& a_otherVertex)
{
  m_position     = a_otherVertex.m_position;
  m_normal       = a_otherVertex.m_normal;
  m_outgoingEdge = a_otherVertex.m_outgoingEdge;
}

template <class T, class Meta>
inline VertexT<T, Meta>&
VertexT<T, Meta>::operator=(const VertexT<T, Meta>& a_otherVertex)
{
  if (this != &a_otherVertex) {
    m_position     = a_otherVertex.m_position;
    m_normal       = a_otherVertex.m_normal;
    m_outgoingEdge = a_otherVertex.m_outgoingEdge;
  }

  return *this;
}

template <class T, class Meta>
inline void
VertexT<T, Meta>::define(const Vec3& a_position, const EdgePtr& a_edge, const Vec3& a_normal) noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_position[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_position[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_position[2]));
  EBGEOMETRY_EXPECT(std::isfinite(a_normal[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_normal[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_normal[2]));

  // a_edge == nullptr is valid here (e.g. a freshly-created vertex not yet wired into the mesh).
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
VertexT<T, Meta>::setEdge(const EdgePtr& a_edge) noexcept
{
  // a_edge == nullptr is valid here; callers that dereference m_outgoingEdge are responsible for
  // checking it first (see e.g. computeVertexNormalAngleWeighted).
  m_outgoingEdge = a_edge;
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
VertexT<T, Meta>::addFace(const FacePtr& a_face)
{
  EBGEOMETRY_EXPECT(a_face != nullptr);

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
VertexT<T, Meta>::computeVertexNormalAverage() noexcept
{
  this->computeVertexNormalAverage(m_faces);
}

template <class T, class Meta>
inline void
VertexT<T, Meta>::computeVertexNormalAverage(const std::vector<FacePtr>& a_faces) noexcept
{
  EBGEOMETRY_EXPECT(!a_faces.empty());

  m_normal = Vec3::zeros();

  // TLDR: We simply compute the sum of the normal vectors for each face in
  // a_faces and then normalize. This
  //       will yield an "average" of the normal vectors of the faces
  //       circulating this vertex.
  for (const auto& f : a_faces) {
    EBGEOMETRY_EXPECT(f != nullptr);

    m_normal += f->getNormal();
  }

  this->normalizeNormalVector();
}

template <class T, class Meta>
inline void
VertexT<T, Meta>::computeVertexNormalAngleWeighted()
{
  this->computeVertexNormalAngleWeighted(m_faces);
}

template <class T, class Meta>
inline void
VertexT<T, Meta>::computeVertexNormalAngleWeighted(const std::vector<FacePtr>& a_faces)
{
  m_normal = Vec3::zeros();

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

  EBGEOMETRY_EXPECT(!a_faces.empty());
  EBGEOMETRY_EXPECT(m_outgoingEdge != nullptr);
  const VertexPtr& originVertex = m_outgoingEdge->getVertex(); // AKA 'this'

  for (const auto& f : a_faces) {
    EBGEOMETRY_EXPECT(f != nullptr);

    std::vector<VertexPtr> inoutVertices(0);
    for (EdgeIterator edgeIt(f->getHalfEdge()); edgeIt.ok(); ++edgeIt) {
      const auto& e = edgeIt();

      const auto& v1 = e->getVertex();
      const auto& v2 = e->getOtherVertex();

      EBGEOMETRY_EXPECT(v1 != nullptr);
      EBGEOMETRY_EXPECT(v2 != nullptr);

      if (v1 == originVertex || v2 == originVertex) {
        if (v1 == originVertex) {
          inoutVertices.emplace_back(v2);
        }
        else if (v2 == originVertex) {
          inoutVertices.emplace_back(v1);
        }
        else {
          std::cerr << "VertexT<T, Meta>::computeVertexNormalAngleWeighted(): unreachable branch "
                       "hit -- a half-edge of face f was found to have originVertex as one of its "
                       "two endpoints, but neither v1 nor v2 compares equal to it. This points to "
                       "a corrupted or inconsistent half-edge/vertex topology (e.g. a stale vertex "
                       "pointer) rather than a normal mesh-quality issue.\n";
        }
      }
    }

    if (inoutVertices.size() != 2) {
      std::cerr << "VertexT<T, Meta>::computeVertexNormalAngleWeighted(): face f should be incident "
                   "on the origin vertex through exactly 2 half-edges (one incoming, one "
                   "outgoing), but "
                << inoutVertices.size()
                << " were found. This means f is not a well-formed triangle sharing this vertex "
                   "(e.g. the origin vertex appears more than once on f's boundary, or not at "
                   "all) -- check the mesh for degenerate or non-manifold faces.\n";
    }

    // The cerr above only warns; this used to fall through to indexing inoutVertices[0]/[1]
    // unconditionally, which is undefined behaviour (out-of-bounds read) if the face isn't a
    // well-formed triangle incident on exactly two edges at this vertex.
    EBGEOMETRY_EXPECT(inoutVertices.size() == 2);

    const Vec3& x0 = originVertex->getPosition();
    const Vec3& x1 = inoutVertices[0]->getPosition();
    const Vec3& x2 = inoutVertices[1]->getPosition();

    if (x0 == x1 || x0 == x2 || x1 == x2) {
      std::cerr << "VertexT<T, Meta>::computeVertexNormalAngleWeighted(): degenerate face f -- two "
                   "of the origin vertex position ("
                << x0 << ") and its two neighboring vertex positions (" << x1 << ", " << x2
                << ") coincide. This produces a zero-length edge, which has no well-defined "
                   "subtended angle -- check the mesh for duplicate/collapsed vertices.\n";
    }

    // Likewise, the cerr above only warns; this used to fall through to dividing by
    // v1.length()/v2.length() unconditionally, which is a division by zero if x0 coincides with
    // x1 or x2 (a degenerate/zero-length edge).
    EBGEOMETRY_EXPECT(x0 != x1);
    EBGEOMETRY_EXPECT(x0 != x2);
    EBGEOMETRY_EXPECT(x1 != x2);

    Vec3 v1 = x1 - x0;
    Vec3 v2 = x2 - x0;

    v1 = v1 / v1.length();
    v2 = v2 / v2.length();

    const Vec3& norm = f->getNormal();

    // Clamp to [-1,1] to guard against acos(NaN) from floating-point rounding.
    const T alpha = std::acos(std::clamp(v1.dot(v2), T(-1), T(1)));

    m_normal += alpha * norm;
  }

  this->normalizeNormalVector();
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
inline std::shared_ptr<EdgeT<T, Meta>>&
VertexT<T, Meta>::getOutgoingEdge() noexcept
{
  return m_outgoingEdge;
}

template <class T, class Meta>
inline const std::shared_ptr<EdgeT<T, Meta>>&
VertexT<T, Meta>::getOutgoingEdge() const noexcept
{
  return m_outgoingEdge;
}

template <class T, class Meta>
inline std::vector<std::shared_ptr<FaceT<T, Meta>>>&
VertexT<T, Meta>::getFaces() noexcept
{
  return m_faces;
}

template <class T, class Meta>
inline const std::vector<std::shared_ptr<FaceT<T, Meta>>>&
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
  const int  sign  = (dot > 0.) ? 1 : -1;

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
