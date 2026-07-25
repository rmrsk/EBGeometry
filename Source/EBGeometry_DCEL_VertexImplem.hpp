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
inline void
VertexT<T, Meta>::define(const Vec3& a_position, const uint32_t a_edgeIndex, const Vec3& a_normal) noexcept
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
VertexT<T, Meta>::setEdge(const uint32_t a_edgeIndex) noexcept
{
  // a_edgeIndex == UINT32_MAX is valid here; callers that resolve m_outgoingEdge are responsible
  // for checking it first (see e.g. computeVertexNormalAngleWeighted).
  m_outgoingEdge = a_edgeIndex;
}

template <class T, class Meta>
inline void
VertexT<T, Meta>::setMetaData(const Meta& a_metaData) noexcept
{
  m_metaData = a_metaData;
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
VertexT<T, Meta>::computeVertexNormalAverage(const std::vector<uint32_t>& a_faceIndices, const Mesh& a_mesh) noexcept
{
  EBGEOMETRY_EXPECT(!a_faceIndices.empty());

  m_normal = Vec3::zeros();

  // TLDR: We simply compute the sum of the normal vectors for each face in
  // a_faceIndices and then normalize. This
  //       will yield an "average" of the normal vectors of the faces
  //       circulating this vertex.
  for (const uint32_t faceIndex : a_faceIndices) {
    m_normal += a_mesh.getFaces()[faceIndex].getNormal();
  }

  this->normalizeNormalVector();
}

template <class T, class Meta>
inline void
VertexT<T, Meta>::computeVertexNormalAngleWeighted(const uint32_t               a_thisVertexIndex,
                                                   const std::vector<uint32_t>& a_faceIndices,
                                                   const Mesh&                  a_mesh)
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
  //
  // The below code is more complicated than it looks. It happens because we
  // want the two half edges that has the current vertex as a mutual vertex
  // (i.e. the "incoming" and "outgoing" edges into this vertex). Normally we'd
  // just iterate through edges, but if it happens that an input face is
  // flipped, this will result in infinite iteration. Instead, the caller has
  // given us the indices of each face connected to this vertex. We look through
  // each face to find the endpoints of the edges that have the current vertex
  // as the common vertex, and then compute the subtended angle between those.
  // Sigh...

  EBGEOMETRY_EXPECT(!a_faceIndices.empty());

  m_normal = Vec3::zeros();

  const uint32_t originVertexIndex = a_thisVertexIndex;

  for (const uint32_t faceIndex : a_faceIndices) {
    const Face& f = a_mesh.getFaces()[faceIndex];

    std::vector<uint32_t> inoutVertices(0);
    for (EdgeIterator edgeIt(a_mesh, f); edgeIt.ok(); ++edgeIt) {
      const Edge& e = a_mesh.getEdges()[edgeIt()];

      const uint32_t v1 = e.getVertexIndex();
      const uint32_t v2 = e.getNextEdge(a_mesh).getVertexIndex();

      if (v1 == originVertexIndex || v2 == originVertexIndex) {
        if (v1 == originVertexIndex) {
          inoutVertices.emplace_back(v2);
        }
        else if (v2 == originVertexIndex) {
          inoutVertices.emplace_back(v1);
        }
        else {
          std::cerr << "VertexT<T, Meta>::computeVertexNormalAngleWeighted(): unreachable branch "
                       "hit -- a half-edge of face f was found to have originVertexIndex as one of "
                       "its two endpoints, but neither v1 nor v2 compares equal to it. This points "
                       "to a corrupted or inconsistent half-edge/vertex topology (e.g. a stale "
                       "vertex index) rather than a normal mesh-quality issue.\n";
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

    const Vec3& x0 = a_mesh.getVertices()[originVertexIndex].getPosition();
    const Vec3& x1 = a_mesh.getVertices()[inoutVertices[0]].getPosition();
    const Vec3& x2 = a_mesh.getVertices()[inoutVertices[1]].getPosition();

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

    const Vec3& norm = f.getNormal();

    // Clamp to [-1,1] to guard against std::acos(NaN) from floating-point rounding.
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
inline uint32_t
VertexT<T, Meta>::getOutgoingEdgeIndex() const noexcept
{
  return m_outgoingEdge;
}

template <class T, class Meta>
inline EdgeT<T, Meta>&
VertexT<T, Meta>::getOutgoingEdge(Mesh& a_mesh) noexcept
{
  EBGEOMETRY_EXPECT(m_outgoingEdge != UINT32_MAX);

  return a_mesh.getEdges()[m_outgoingEdge];
}

template <class T, class Meta>
inline const EdgeT<T, Meta>&
VertexT<T, Meta>::getOutgoingEdge(const Mesh& a_mesh) const noexcept
{
  EBGEOMETRY_EXPECT(m_outgoingEdge != UINT32_MAX);

  return a_mesh.getEdges()[m_outgoingEdge];
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
  const int  sign  = (dot > T(0.)) ? 1 : -1;

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
