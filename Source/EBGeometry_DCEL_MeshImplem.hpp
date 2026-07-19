// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_DCEL_MeshImplem.hpp
 * @brief  Implementation of EBGeometry_DCEL_Mesh.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_DCEL_MESHIMPLEM_HPP
#define EBGEOMETRY_DCEL_MESHIMPLEM_HPP

// Std includes
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

// Our includes
#include "EBGeometry_DCEL_Edge.hpp"
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_DCEL_Iterator.hpp"
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_DCEL_Vertex.hpp"
#include "EBGeometry_Macros.hpp"

namespace EBGeometry {

namespace DCEL {

template <class T, class Meta>
inline MeshT<T, Meta>::MeshT(std::vector<Face>&   a_faces,
                             std::vector<Edge>&   a_edges,
                             std::vector<Vertex>& a_vertices) noexcept
  : MeshT()
{
  m_faces    = a_faces;
  m_edges    = a_edges;
  m_vertices = a_vertices;
}

template <class T, class Meta>
inline std::shared_ptr<MeshT<T, Meta>>
MeshT<T, Meta>::deepCopy() const
{
  // The flat representation stores topology as plain array indices and every element (including a
  // face's cached 2D embedding) by value, so an independent copy is simply a by-value copy of the
  // three arrays plus the search algorithm. The indices remain valid, and every geometric field
  // (including flipped normals) is preserved exactly. This deliberately does NOT call reconcile(),
  // which would recompute normals from vertex winding and silently discard a prior flip().
  auto newMesh = std::make_shared<Mesh>();

  newMesh->m_vertices  = m_vertices;
  newMesh->m_edges     = m_edges;
  newMesh->m_faces     = m_faces;
  newMesh->m_algorithm = m_algorithm;

  return newMesh;
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::incrementWarning(std::map<std::string, size_t>& a_warnings, const std::string& a_warn) const
{
  a_warnings[a_warn] += 1;
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::printWarnings(const std::map<std::string, size_t>& a_warnings, const std::string& a_id) const
{
  std::string baseError = "MeshT<T, Meta>::sanityCheck(...)";

  if (a_id != "") {
    baseError += " for '" + a_id + "'";
  }

  baseError += " - warnings about error '";

  for (const auto& warn : a_warnings) {
    if (warn.second > 0) {
      std::cerr << baseError << warn.first << "' = " << warn.second << "\n";
    }
  }
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::sanityCheck(const std::string a_id) const
{
  const std::string f_noEdge     = "face with no edge";
  const std::string f_degenerate = "degenerate face";

  const std::string e_degenerate = "degenerate edge";
  const std::string e_noPairEdge = "no pair edge (not watertight)";
  const std::string e_noNextEdge = "no next edge (badly linked dcel)";
  const std::string e_noOrigVert = "no origin vertex found for half edge (badly linked dcel)";
  const std::string e_noFace     = "no face found for half edge (badly linked dcel)";

  const std::string v_noEdge = "no referenced edge for vertex (unreferenced vertex)";

  std::map<std::string, size_t> warnings = {{f_noEdge, 0},
                                            {f_degenerate, 0},
                                            {e_degenerate, 0},
                                            {e_noPairEdge, 0},
                                            {e_noNextEdge, 0},
                                            {e_noOrigVert, 0},
                                            {e_noFace, 0},
                                            {v_noEdge, 0}};

  for (size_t iFace = 0; iFace < m_faces.size(); iFace++) {
    const DCELIndex halfEdge = m_faces[iFace].getHalfEdge();
    if (halfEdge == InvalidIndex) {
      this->incrementWarning(warnings, f_noEdge);

      continue;
    }

    std::vector<DCELIndex> vertices;

    for (EdgeIteratorT<T, Meta> edgeIt(m_edges, halfEdge); edgeIt.ok(); ++edgeIt) {
      vertices.emplace_back(m_edges[edgeIt()].getVertex());
    }

    std::sort(vertices.begin(), vertices.end());
    const auto it           = std::unique(vertices.begin(), vertices.end());
    const bool noDuplicates = (it == vertices.end());
    if (!noDuplicates) {
      this->incrementWarning(warnings, f_degenerate);
    }
  }

  for (size_t iEdge = 0; iEdge < m_edges.size(); iEdge++) {
    const Edge& e = m_edges[iEdge];

    if (e.getNextEdge() != InvalidIndex && e.getVertex() == this->getOtherVertex(static_cast<DCELIndex>(iEdge))) {
      this->incrementWarning(warnings, e_degenerate);
    }
    if (e.getPairEdge() == InvalidIndex) {
      this->incrementWarning(warnings, e_noPairEdge);
    }
    if (e.getNextEdge() == InvalidIndex) {
      this->incrementWarning(warnings, e_noNextEdge);
    }
    if (e.getVertex() == InvalidIndex) {
      this->incrementWarning(warnings, e_noOrigVert);
    }
    if (e.getFace() == InvalidIndex) {
      this->incrementWarning(warnings, e_noFace);
    }
  }

  for (const auto& v : m_vertices) {
    if (v.getOutgoingEdge() == InvalidIndex) {
      this->incrementWarning(warnings, v_noEdge);
    }
  }

  this->printWarnings(warnings, a_id);
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::setSearchAlgorithm(const SearchAlgorithm a_algorithm) noexcept
{
  m_algorithm = a_algorithm;
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::setInsideOutsideAlgorithm(typename Polygon2D<T>::InsideOutsideAlgorithm a_algorithm) noexcept
{
  for (auto& f : m_faces) {
    f.setInsideOutsideAlgorithm(a_algorithm);
  }
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::reconcile(const DCEL::VertexNormalWeight a_weight) noexcept
{
  this->reconcileFaces();
  this->reconcileEdges();
  this->reconcileVertices(a_weight);
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::flip() noexcept
{
  this->flipFaceNormals();
  this->flipEdgeNormals();
  this->flipVertexNormals();
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::reconcileFace(const DCELIndex a_faceIdx) noexcept
{
  EBGEOMETRY_EXPECT(a_faceIdx >= 0 && a_faceIdx < static_cast<DCELIndex>(m_faces.size()));

  Face& face = m_faces[a_faceIdx];

  const std::vector<Vec3> vertices = this->getFaceVertexCoordinates(a_faceIdx);
  const size_t            N        = vertices.size();

  // A polygon face needs at least 3 vertices to span a plane.
  EBGEOMETRY_EXPECT(N >= 3);

  // Compute the normal vector from three non-degenerate vertices spanning the face plane.
  Vec3 normal = Vec3::zeros();

  for (size_t i = 0; i < N; i++) {
    const Vec3& x0 = vertices[i];
    const Vec3& x1 = vertices[(i + 1) % N];
    const Vec3& x2 = vertices[(i + 2) % N];

    normal = (x2 - x0).cross(x2 - x1);

    if (normal.length() > T(0.0)) {
      break; // Found one.
    }
  }

  EBGEOMETRY_EXPECT(normal.length() > std::numeric_limits<T>::epsilon());

  face.getNormal() = normal;
  face.normalizeNormalVector();

  // Compute the centroid.
  Vec3 centroid = Vec3::zeros();

  for (const auto& v : vertices) {
    centroid += v;
  }

  centroid = centroid / static_cast<T>(N);

  face.getCentroid() = centroid;

  // Compute the area using the cross-product/shoelace formula over ALL N edges, including the
  // wraparound edge from vertices[N-1] back to vertices[0].
  T area = T(0.0);

  for (size_t i = 0; i < N; i++) {
    const Vec3& v1 = vertices[i];
    const Vec3& v2 = vertices[(i + 1) % N];

    area += face.getNormal().dot(v2.cross(v1));
  }

  face.getArea() = T(0.5) * std::abs(area);

  // Compute the 2D embedding from the (now-normalized) normal vector and the vertex coordinates.
  face.setPolygon2D(Polygon2D<T>(face.getNormal(), vertices));
}

template <class T, class Meta>
inline Vec3T<T>
MeshT<T, Meta>::computeEdgeNormal(const DCELIndex a_edgeIdx) const noexcept
{
  EBGEOMETRY_EXPECT(a_edgeIdx >= 0 && a_edgeIdx < static_cast<DCELIndex>(m_edges.size()));

  const Edge& edge = m_edges[a_edgeIdx];

  // Every half-edge belongs to exactly one face by DCEL invariant. The pair edge (and its face)
  // may legitimately be missing for a boundary edge on an open mesh -- handled gracefully below.
  EBGEOMETRY_EXPECT(edge.getFace() != InvalidIndex);

  Vec3 normal = Vec3::zeros();

  if (edge.getFace() != InvalidIndex) {
    const Vec3& faceNormal = m_faces[edge.getFace()].getNormal();

    EBGEOMETRY_EXPECT(std::isfinite(faceNormal[0]));
    EBGEOMETRY_EXPECT(std::isfinite(faceNormal[1]));
    EBGEOMETRY_EXPECT(std::isfinite(faceNormal[2]));

    normal += faceNormal;
  }
  if (edge.getPairEdge() != InvalidIndex) {
    const DCELIndex pairFaceIdx = m_edges[edge.getPairEdge()].getFace();

    if (pairFaceIdx != InvalidIndex) {
      const Vec3& pairFaceNormal = m_faces[pairFaceIdx].getNormal();

      EBGEOMETRY_EXPECT(std::isfinite(pairFaceNormal[0]));
      EBGEOMETRY_EXPECT(std::isfinite(pairFaceNormal[1]));
      EBGEOMETRY_EXPECT(std::isfinite(pairFaceNormal[2]));

      normal += pairFaceNormal;
    }
  }

  const T len = normal.length();

  EBGEOMETRY_EXPECT(len > T(0));

  return (len > std::numeric_limits<T>::epsilon()) ? normal / len : Vec3::zeros();
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::computeVertexNormalAverage(const DCELIndex a_vertexIdx) noexcept
{
  EBGEOMETRY_EXPECT(a_vertexIdx >= 0 && a_vertexIdx < static_cast<DCELIndex>(m_vertices.size()));

  Vertex& vertex = m_vertices[a_vertexIdx];

  const std::vector<DCELIndex>& faces = vertex.getFaces();

  EBGEOMETRY_EXPECT(!faces.empty());

  Vec3 normal = Vec3::zeros();

  // We simply compute the sum of the normal vectors for each face incident on this vertex and then
  // normalize -- an "average" of the incident faces' normal vectors.
  for (const auto& faceIdx : faces) {
    EBGEOMETRY_EXPECT(faceIdx >= 0 && faceIdx < static_cast<DCELIndex>(m_faces.size()));

    normal += m_faces[faceIdx].getNormal();
  }

  vertex.getNormal() = normal;
  vertex.normalizeNormalVector();
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::computeVertexNormalAngleWeighted(const DCELIndex a_vertexIdx)
{
  EBGEOMETRY_EXPECT(a_vertexIdx >= 0 && a_vertexIdx < static_cast<DCELIndex>(m_vertices.size()));

  Vertex& vertex = m_vertices[a_vertexIdx];

  const std::vector<DCELIndex>& faces = vertex.getFaces();

  // This routine computes the pseudonormal from Baerentzen and Aanes in "Signed distance
  // computation using the angle weighted pseudonormal" (DOI: 10.1109/TVCG.2005.49). The normal is
  // an average of the incident faces' normals weighted by the angle each face subtends at this
  // vertex (the angle spanned by the incoming/outgoing edges of the face that pass through this
  // vertex). We look through each incident face to find the two edge endpoints that neighbor this
  // vertex on that face, so a flipped input face does not cause infinite iteration.
  EBGEOMETRY_EXPECT(!faces.empty());
  EBGEOMETRY_EXPECT(vertex.getOutgoingEdge() != InvalidIndex);

  Vec3 normal = Vec3::zeros();

  for (const auto& faceIdx : faces) {
    EBGEOMETRY_EXPECT(faceIdx >= 0 && faceIdx < static_cast<DCELIndex>(m_faces.size()));

    std::vector<DCELIndex> inoutVertices(0);

    for (EdgeIteratorT<T, Meta> edgeIt(m_edges, m_faces[faceIdx].getHalfEdge()); edgeIt.ok(); ++edgeIt) {
      const DCELIndex v1 = m_edges[edgeIt()].getVertex();
      const DCELIndex v2 = this->getOtherVertex(edgeIt());

      if (v1 == a_vertexIdx || v2 == a_vertexIdx) {
        if (v1 == a_vertexIdx) {
          inoutVertices.emplace_back(v2);
        }
        else if (v2 == a_vertexIdx) {
          inoutVertices.emplace_back(v1);
        }
        else {
          std::cerr << "MeshT<T, Meta>::computeVertexNormalAngleWeighted(): unreachable branch hit "
                       "-- a half-edge of the face was found to have the origin vertex as one of "
                       "its two endpoints, but neither v1 nor v2 compares equal to it. This points "
                       "to a corrupted or inconsistent half-edge/vertex topology.\n";
        }
      }
    }

    if (inoutVertices.size() != 2) {
      std::cerr << "MeshT<T, Meta>::computeVertexNormalAngleWeighted(): face should be incident on "
                   "the origin vertex through exactly 2 half-edges (one incoming, one outgoing), "
                   "but "
                << inoutVertices.size()
                << " were found. This means the face is not a well-formed triangle sharing this "
                   "vertex -- check the mesh for degenerate or non-manifold faces.\n";
    }

    EBGEOMETRY_EXPECT(inoutVertices.size() == 2);

    const Vec3& x0 = vertex.getPosition();
    const Vec3& x1 = m_vertices[inoutVertices[0]].getPosition();
    const Vec3& x2 = m_vertices[inoutVertices[1]].getPosition();

    if (x0 == x1 || x0 == x2 || x1 == x2) {
      std::cerr << "MeshT<T, Meta>::computeVertexNormalAngleWeighted(): degenerate face -- two of "
                   "the origin vertex position ("
                << x0 << ") and its two neighboring vertex positions (" << x1 << ", " << x2
                << ") coincide. This produces a zero-length edge, which has no well-defined "
                   "subtended angle -- check the mesh for duplicate/collapsed vertices.\n";
    }

    EBGEOMETRY_EXPECT(x0 != x1);
    EBGEOMETRY_EXPECT(x0 != x2);
    EBGEOMETRY_EXPECT(x1 != x2);

    Vec3 v1 = x1 - x0;
    Vec3 v2 = x2 - x0;

    v1 = v1 / v1.length();
    v2 = v2 / v2.length();

    const Vec3& norm = m_faces[faceIdx].getNormal();

    // Clamp to [-1,1] to guard against std::acos(NaN) from floating-point rounding.
    const T alpha = std::acos(std::clamp(v1.dot(v2), T(-1), T(1)));

    normal += alpha * norm;
  }

  vertex.getNormal() = normal;
  vertex.normalizeNormalVector();
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::reconcileFaces() noexcept
{
  for (size_t i = 0; i < m_faces.size(); i++) {
    this->reconcileFace(static_cast<DCELIndex>(i));
  }
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::reconcileEdges() noexcept
{
  for (size_t i = 0; i < m_edges.size(); i++) {
    m_edges[i].getNormal() = this->computeEdgeNormal(static_cast<DCELIndex>(i));
  }
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::reconcileVertices(const DCEL::VertexNormalWeight a_weight) noexcept
{
  for (size_t i = 0; i < m_vertices.size(); i++) {
    switch (a_weight) {
    case DCEL::VertexNormalWeight::None: {
      this->computeVertexNormalAverage(static_cast<DCELIndex>(i));

      break;
    }
    case DCEL::VertexNormalWeight::Angle: {
      this->computeVertexNormalAngleWeighted(static_cast<DCELIndex>(i));

      break;
    }
    default: {
      std::cerr << "In file 'EBGeometry_DCEL_MeshImplem.hpp' function "
                   "DCEL::MeshT<T, Meta>::reconcileVertices(VertexNormalWeighting) - a_weight does "
                   "not match any of the known VertexNormalWeight enumerators; this indicates a "
                   "corrupted or out-of-range enum value rather than a normal runtime condition.\n";
      EBGEOMETRY_EXPECT(false);

      break;
    }
    }
  }
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::flipFaceNormals() noexcept
{
  for (auto& f : m_faces) {
    f.flipNormal();
  }
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::flipEdgeNormals() noexcept
{
  for (auto& e : m_edges) {
    e.flipNormal();
  }
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::flipVertexNormals() noexcept
{
  for (auto& v : m_vertices) {
    v.flipNormal();
  }
}

template <class T, class Meta>
inline std::vector<VertexT<T, Meta>>&
MeshT<T, Meta>::getVertices() noexcept
{
  return m_vertices;
}

template <class T, class Meta>
inline const std::vector<VertexT<T, Meta>>&
MeshT<T, Meta>::getVertices() const noexcept
{
  return m_vertices;
}

template <class T, class Meta>
inline std::vector<EdgeT<T, Meta>>&
MeshT<T, Meta>::getEdges() noexcept
{
  return m_edges;
}

template <class T, class Meta>
inline const std::vector<EdgeT<T, Meta>>&
MeshT<T, Meta>::getEdges() const noexcept
{
  return m_edges;
}

template <class T, class Meta>
inline std::vector<FaceT<T, Meta>>&
MeshT<T, Meta>::getFaces() noexcept
{
  return m_faces;
}

template <class T, class Meta>
inline const std::vector<FaceT<T, Meta>>&
MeshT<T, Meta>::getFaces() const noexcept
{
  return m_faces;
}

template <class T, class Meta>
inline std::vector<Vec3T<T>>
MeshT<T, Meta>::getAllVertexCoordinates() const noexcept
{
  std::vector<Vec3> vertexCoordinates;
  vertexCoordinates.reserve(m_vertices.size());

  for (const auto& v : m_vertices) {
    vertexCoordinates.emplace_back(v.getPosition());
  }

  return vertexCoordinates;
}

template <class T, class Meta>
inline std::vector<Vec3T<T>>
MeshT<T, Meta>::getFaceVertexCoordinates(const DCELIndex a_faceIdx) const noexcept
{
  EBGEOMETRY_EXPECT(a_faceIdx >= 0 && a_faceIdx < static_cast<DCELIndex>(m_faces.size()));

  std::vector<Vec3> coordinates;
  coordinates.reserve(3);

  for (EdgeIteratorT<T, Meta> edgeIt(m_edges, m_faces[a_faceIdx].getHalfEdge()); edgeIt.ok(); ++edgeIt) {
    coordinates.emplace_back(m_vertices[m_edges[edgeIt()].getVertex()].getPosition());
  }

  return coordinates;
}

template <class T, class Meta>
inline DCELIndex
MeshT<T, Meta>::getOtherVertex(const DCELIndex a_edgeIdx) const noexcept
{
  EBGEOMETRY_EXPECT(a_edgeIdx >= 0 && a_edgeIdx < static_cast<DCELIndex>(m_edges.size()));

  const DCELIndex nextEdge = m_edges[a_edgeIdx].getNextEdge();

  EBGEOMETRY_EXPECT(nextEdge != InvalidIndex);

  return m_edges[nextEdge].getVertex();
}

template <class T, class Meta>
inline Vec3T<T>
MeshT<T, Meta>::projectPointIntoFacePlane(const DCELIndex a_faceIdx, const Vec3& a_p) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_p[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_p[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_p[2]));

  const Face& face = m_faces[a_faceIdx];

  return a_p - face.getNormal() * (face.getNormal().dot(a_p - face.getCentroid()));
}

template <class T, class Meta>
inline bool
MeshT<T, Meta>::isPointInsideFace(const DCELIndex a_faceIdx, const Vec3& a_p) const noexcept
{
  const Face& face = m_faces[a_faceIdx];

  const Vec3 p = this->projectPointIntoFacePlane(a_faceIdx, a_p);

  return face.getPolygon2D().isPointInside(p, face.getInsideOutsideAlgorithm());
}

template <class T, class Meta>
inline T
MeshT<T, Meta>::signedDistanceToVertex(const DCELIndex a_vertexIdx, const Vec3& a_x0) const noexcept
{
  EBGEOMETRY_EXPECT(a_vertexIdx >= 0 && a_vertexIdx < static_cast<DCELIndex>(m_vertices.size()));

  return m_vertices[a_vertexIdx].signedDistance(a_x0);
}

template <class T, class Meta>
inline T
MeshT<T, Meta>::unsignedDistance2ToVertex(const DCELIndex a_vertexIdx, const Vec3& a_x0) const noexcept
{
  EBGEOMETRY_EXPECT(a_vertexIdx >= 0 && a_vertexIdx < static_cast<DCELIndex>(m_vertices.size()));

  return m_vertices[a_vertexIdx].unsignedDistance2(a_x0);
}

template <class T, class Meta>
inline T
MeshT<T, Meta>::signedDistanceToEdge(const DCELIndex a_edgeIdx, const Vec3& a_x0) const noexcept
{
  EBGEOMETRY_EXPECT(a_edgeIdx >= 0 && a_edgeIdx < static_cast<DCELIndex>(m_edges.size()));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[2]));

  const DCELIndex v1Idx = m_edges[a_edgeIdx].getVertex();
  const DCELIndex v2Idx = this->getOtherVertex(a_edgeIdx);

  const Vec3& x1   = m_vertices[v1Idx].getPosition();
  const Vec3& x2   = m_vertices[v2Idx].getPosition();
  const Vec3  x2x1 = x2 - x1;

  EBGEOMETRY_EXPECT(x2x1.dot(x2x1) > T(0));

  const T t = (a_x0 - x1).dot(x2x1) / x2x1.dot(x2x1);

  T retval = std::numeric_limits<T>::max();

  if (t <= T(0.0)) {
    // Closest point is the starting vertex.
    retval = this->signedDistanceToVertex(v1Idx, a_x0);
  }
  else if (t >= T(1.0)) {
    // Closest point is the end vertex.
    retval = this->signedDistanceToVertex(v2Idx, a_x0);
  }
  else {
    // Closest point is the edge itself.
    const Vec3 linePoint = x1 + t * x2x1;
    const Vec3 delta     = a_x0 - linePoint;
    const T    dot       = m_edges[a_edgeIdx].getNormal().dot(delta);

    const int sgn = (dot > T(0.0)) ? 1 : -1;

    retval = sgn * delta.length();
  }

  return retval;
}

template <class T, class Meta>
inline T
MeshT<T, Meta>::unsignedDistance2ToEdge(const DCELIndex a_edgeIdx, const Vec3& a_x0) const noexcept
{
  EBGEOMETRY_EXPECT(a_edgeIdx >= 0 && a_edgeIdx < static_cast<DCELIndex>(m_edges.size()));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[2]));

  const DCELIndex v1Idx = m_edges[a_edgeIdx].getVertex();
  const DCELIndex v2Idx = this->getOtherVertex(a_edgeIdx);

  const Vec3& x1   = m_vertices[v1Idx].getPosition();
  const Vec3& x2   = m_vertices[v2Idx].getPosition();
  const Vec3  x2x1 = x2 - x1;

  EBGEOMETRY_EXPECT(x2x1.dot(x2x1) > T(0));

  const T t = std::clamp((a_x0 - x1).dot(x2x1) / x2x1.dot(x2x1), T(0.0), T(1.0));

  const Vec3 linePoint = x1 + t * x2x1;
  const Vec3 delta     = a_x0 - linePoint;

  return delta.dot(delta);
}

template <class T, class Meta>
inline T
MeshT<T, Meta>::signedDistanceToFace(const DCELIndex a_faceIdx, const Vec3& a_x0) const noexcept
{
  EBGEOMETRY_EXPECT(a_faceIdx >= 0 && a_faceIdx < static_cast<DCELIndex>(m_faces.size()));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[2]));

  const Face& face = m_faces[a_faceIdx];

  EBGEOMETRY_EXPECT(face.getHalfEdge() != InvalidIndex);

  T retval = std::numeric_limits<T>::infinity();

  if (this->isPointInsideFace(a_faceIdx, a_x0)) {
    retval = face.getNormal().dot(a_x0 - face.getCentroid());
  }
  else {
    const DCELIndex startEdge = face.getHalfEdge();
    DCELIndex       cur       = startEdge;

    while (true) {
      const T curDist = this->signedDistanceToEdge(cur, a_x0);

      retval = (std::abs(curDist) < std::abs(retval)) ? curDist : retval;

      cur = m_edges[cur].getNextEdge();

      if (cur == InvalidIndex || cur == startEdge) {
        break;
      }
    }
  }

  return retval;
}

template <class T, class Meta>
inline T
MeshT<T, Meta>::unsignedDistance2ToFace(const DCELIndex a_faceIdx, const Vec3& a_x0) const noexcept
{
  EBGEOMETRY_EXPECT(a_faceIdx >= 0 && a_faceIdx < static_cast<DCELIndex>(m_faces.size()));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_x0[2]));

  const Face& face = m_faces[a_faceIdx];

  EBGEOMETRY_EXPECT(face.getHalfEdge() != InvalidIndex);

  T retval = std::numeric_limits<T>::infinity();

  if (this->isPointInsideFace(a_faceIdx, a_x0)) {
    const T normDist = face.getNormal().dot(a_x0 - face.getCentroid());

    retval = normDist * normDist;
  }
  else {
    const DCELIndex startEdge = face.getHalfEdge();
    DCELIndex       cur       = startEdge;

    while (true) {
      const T curDist2 = this->unsignedDistance2ToEdge(cur, a_x0);

      retval = (curDist2 < retval) ? curDist2 : retval;

      cur = m_edges[cur].getNextEdge();

      if (cur == InvalidIndex || cur == startEdge) {
        break;
      }
    }
  }

  return retval;
}

template <class T, class Meta>
inline T
MeshT<T, Meta>::signedDistance(const Vec3& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  return this->signedDistance(a_point, m_algorithm);
}

template <class T, class Meta>
inline T
MeshT<T, Meta>::unsignedDistance2(const Vec3& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  if (m_faces.empty()) {
    return std::numeric_limits<T>::infinity();
  }

  T minDist2 = std::numeric_limits<T>::max();

  for (size_t i = 0; i < m_faces.size(); i++) {
    const T curDist2 = this->unsignedDistance2ToFace(static_cast<DCELIndex>(i), a_point);

    minDist2 = std::min(minDist2, curDist2);
  }

  return minDist2;
}

template <class T, class Meta>
inline T
MeshT<T, Meta>::signedDistance(const Vec3& a_point, SearchAlgorithm a_algorithm) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  T minDist = std::numeric_limits<T>::max();

  switch (a_algorithm) {
  case SearchAlgorithm::Direct: {
    minDist = this->DirectSignedDistance(a_point);

    break;
  }
  case SearchAlgorithm::Direct2: {
    minDist = this->DirectSignedDistance2(a_point);

    break;
  }
  default: {
    std::cerr << "Error in file 'EBGeometry_DCEL_MeshImplem.hpp' MeshT<T, Meta>::signedDistance - "
                 "a_algorithm does not match any of the known SearchAlgorithm enumerators; this "
                 "indicates a corrupted or out-of-range enum value rather than a normal runtime "
                 "condition.\n";
    EBGEOMETRY_EXPECT(false);

    break;
  }
  }

  return minDist;
}

template <class T, class Meta>
inline T
MeshT<T, Meta>::DirectSignedDistance(const Vec3& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  if (m_faces.empty()) {
    return std::numeric_limits<T>::infinity();
  }

  T minDist  = this->signedDistanceToFace(0, a_point);
  T minDist2 = minDist * minDist;

  for (size_t i = 0; i < m_faces.size(); i++) {
    const T curDist  = this->signedDistanceToFace(static_cast<DCELIndex>(i), a_point);
    const T curDist2 = curDist * curDist;

    if (curDist2 < minDist2) {
      minDist  = curDist;
      minDist2 = curDist2;
    }
  }

  return minDist;
}

template <class T, class Meta>
inline T
MeshT<T, Meta>::DirectSignedDistance2(const Vec3& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  if (m_faces.empty()) {
    return std::numeric_limits<T>::infinity();
  }

  DCELIndex closest  = 0;
  T         minDist2 = this->unsignedDistance2ToFace(0, a_point);

  for (size_t i = 0; i < m_faces.size(); i++) {
    const T curDist2 = this->unsignedDistance2ToFace(static_cast<DCELIndex>(i), a_point);

    if (curDist2 < minDist2) {
      closest  = static_cast<DCELIndex>(i);
      minDist2 = curDist2;
    }
  }

  return this->signedDistanceToFace(closest, a_point);
}
} // namespace DCEL

} // namespace EBGeometry

#endif
