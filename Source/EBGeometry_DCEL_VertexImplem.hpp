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
#include <vector>

// Our includes
#include "EBGeometry_DCEL_Edge.hpp"
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_DCEL_Iterator.hpp"
#include "EBGeometry_DCEL_Vertex.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"

namespace EBGeometry {

namespace DCEL {

template <class T>
EBGEOMETRY_HOST_DEVICE
inline VertexT<T>::VertexT(const Vec3& a_position) : VertexT()
{
  EBGEOMETRY_EXPECT(std::isfinite(a_position[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_position[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_position[2]));

  m_position = a_position;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline VertexT<T>::VertexT(const Vec3& a_position, const Vec3& a_normal) : VertexT()
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

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
VertexT<T>::define(const Vec3& a_position, const uint32_t a_edgeIndex, const Vec3& a_normal) noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_position[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_position[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_position[2]));
  EBGEOMETRY_EXPECT(std::isfinite(a_normal[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_normal[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_normal[2]));

  // a_edgeIndex == UINT32_MAX is valid here (e.g. a freshly-created vertex not yet wired into a mesh).
  m_position     = a_position;
  m_outgoingEdge = a_edgeIndex;
  m_normal       = a_normal;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
VertexT<T>::setPosition(const Vec3& a_position) noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_position[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_position[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_position[2]));

  m_position = a_position;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
VertexT<T>::setEdge(const uint32_t a_edgeIndex) noexcept
{
  // a_edgeIndex == UINT32_MAX is valid here; callers that resolve m_outgoingEdge are responsible
  // for checking it first (getOutgoingEdge() EXPECTs it set).
  m_outgoingEdge = a_edgeIndex;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
VertexT<T>::setNormal(const Vec3& a_normal) noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_normal[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_normal[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_normal[2]));

  m_normal = a_normal;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
VertexT<T>::normalizeNormalVector() noexcept
{
  // Zero only when the faces around the vertex cancel out, which folded input causes. The file readers
  // reject that (Soup::findFoldedFeature); a mesh built by hand keeps a zero normal rather than a NaN.
  const T len = m_normal.length();

  if (len > Math::Limits<T>::epsilon()) {
    m_normal = m_normal / len;
  }
}

template <class T>
EBGEOMETRY_HOST
inline void
VertexT<T>::computeVertexNormalAngleWeighted(const uint32_t               a_thisVertexIndex,
                                             const std::vector<uint32_t>& a_faceIndices,
                                             const Mesh&                  a_mesh)
{
  EBGEOMETRY_EXPECT(!a_faceIndices.empty());

  const VertexNormalStatus status = this->computeVertexNormalAngleWeighted(
    a_thisVertexIndex, a_faceIndices.data(), static_cast<uint32_t>(a_faceIndices.size()), a_mesh);

  // Indexing the two neighbours is an out-of-bounds read unless the face is incident on this vertex
  // through exactly two half-edges (one incoming, one outgoing).
  EBGEOMETRY_REQUIRE(status.m_code != VertexNormalStatus::Code::NotVisitedOnce,
                     "DCEL::VertexT::computeVertexNormalAngleWeighted: face %u must visit vertex %u exactly once "
                     "(found %u incident half-edges instead of 2)",
                     unsigned(status.m_face),
                     unsigned(a_thisVertexIndex),
                     unsigned(status.m_found));

  // A zero-length edge has no subtended angle; normalizing it would divide by zero.
  EBGEOMETRY_REQUIRE(status.m_code != VertexNormalStatus::Code::CoincidentPositions,
                     "DCEL::VertexT::computeVertexNormalAngleWeighted: face %u has coincident positions among "
                     "vertices %u, %u and %u",
                     unsigned(status.m_face),
                     unsigned(a_thisVertexIndex),
                     unsigned(status.m_neighbours[0]),
                     unsigned(status.m_neighbours[1]));
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline VertexNormalStatus
VertexT<T>::computeVertexNormalAngleWeighted(const uint32_t  a_thisVertexIndex,
                                             const uint32_t* a_faceIndices,
                                             const uint32_t  a_numFaces,
                                             const Mesh&     a_mesh) noexcept
{
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
  // The caller gives the faces touching the vertex rather than letting it walk around the vertex
  // through pair edges: a flipped input face would make that walk never end. On each face, the two
  // half-edges with this vertex as an endpoint give its two neighbours, and the angle between them is
  // the weight.
  EBGEOMETRY_EXPECT(a_numFaces > 0);

  VertexNormalStatus status;

  m_normal = Vec3::zeros();

  const uint32_t originVertexIndex = a_thisVertexIndex;

  for (uint32_t i = 0; i < a_numFaces; i++) {
    const uint32_t faceIndex = a_faceIndices[i];
    const Face&    f         = a_mesh.getFace(faceIndex);

    // A zero-area face has a zero normal (FaceT::computeNormal) and contributes nothing. Skipping it
    // here also avoids measuring an angle across one of its zero-length edges.
    if (f.getNormal().length2() == T(0)) {
      continue;
    }

    uint32_t neighbours[2] = {0, 0};
    uint32_t found         = 0;

    for (EdgeIterator edgeIt(a_mesh, f); edgeIt.ok(); ++edgeIt) {
      const Edge& e = a_mesh.getEdge(edgeIt());

      const uint32_t v1 = e.getVertexIndex();
      const uint32_t v2 = e.getNextEdge(a_mesh).getVertexIndex();

      if (v1 == originVertexIndex || v2 == originVertexIndex) {
        if (found < 2) {
          neighbours[found] = (v1 == originVertexIndex) ? v2 : v1;
        }

        found++;
      }
    }

    if (found != 2) {
      status.m_code  = VertexNormalStatus::Code::NotVisitedOnce;
      status.m_face  = faceIndex;
      status.m_found = found;

      return status;
    }

    const Vec3& x0 = a_mesh.getVertex(originVertexIndex).getPosition();
    const Vec3& x1 = a_mesh.getVertex(neighbours[0]).getPosition();
    const Vec3& x2 = a_mesh.getVertex(neighbours[1]).getPosition();

    if (!(x0 != x1 && x0 != x2 && x1 != x2)) {
      status.m_code          = VertexNormalStatus::Code::CoincidentPositions;
      status.m_face          = faceIndex;
      status.m_neighbours[0] = neighbours[0];
      status.m_neighbours[1] = neighbours[1];

      return status;
    }

    Vec3 u1 = x1 - x0;
    Vec3 u2 = x2 - x0;

    u1 = u1 / u1.length();
    u2 = u2 / u2.length();

    const Vec3& norm = f.getNormal();

    // Clamp to [-1,1] to guard against std::acos(NaN) from floating-point rounding.
    const T alpha = std::acos(Math::clamp(u1.dot(u2), T(-1), T(1)));

    m_normal += alpha * norm;
  }

  this->normalizeNormalVector();

  return status;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
VertexT<T>::flipNormal() noexcept
{
  m_normal = -m_normal;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline Vec3T<T>&
VertexT<T>::getPosition() noexcept
{
  return m_position;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline const Vec3T<T>&
VertexT<T>::getPosition() const noexcept
{
  return m_position;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline Vec3T<T>&
VertexT<T>::getNormal() noexcept
{
  return m_normal;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline const Vec3T<T>&
VertexT<T>::getNormal() const noexcept
{
  return m_normal;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
VertexT<T>::getOutgoingEdgeIndex() const noexcept
{
  return m_outgoingEdge;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline EdgeT<T>&
VertexT<T>::getOutgoingEdge(Mesh& a_mesh) noexcept
{
  EBGEOMETRY_EXPECT(m_outgoingEdge != UINT32_MAX);

  return a_mesh.getEdge(m_outgoingEdge);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline const EdgeT<T>&
VertexT<T>::getOutgoingEdge(const Mesh& a_mesh) const noexcept
{
  EBGEOMETRY_EXPECT(m_outgoingEdge != UINT32_MAX);

  return a_mesh.getEdge(m_outgoingEdge);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline T
VertexT<T>::signedDistance(const Vec3& a_x0) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[2]));

  const auto delta = a_x0 - m_position;
  const T    dist  = delta.length();
  const T    dot   = m_normal.dot(delta);
  const int  sign  = (dot > T(0.)) ? 1 : -1;

  return dist * sign;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline T
VertexT<T>::unsignedDistance2(const Vec3& a_x0) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[2]));

  const auto d = a_x0 - m_position;

  return d.dot(d);
}
} // namespace DCEL

} // namespace EBGeometry

#endif
