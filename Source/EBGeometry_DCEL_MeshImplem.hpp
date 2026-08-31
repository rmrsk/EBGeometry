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
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
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
EBGEOMETRY_HOST_DEVICE
inline void*
MeshT<T, Meta>::base() const noexcept
{
#if defined(EBGEOMETRY_DEVICE_COMPILE)
  // A non-null control block here means a host descriptor was copied into a kernel directly,
  // instead of going through rebasedView(). The pointer it holds is a host address.
  EBGEOMETRY_EXPECT(m_control == nullptr);

  return m_base;
#else
  // Null here means either a device view being dereferenced on the host, or a mesh nobody ever
  // reserved into. rebasedView() is the only producer of a null control block, and only for a
  // device-accessible target, so this test is exact rather than heuristic.
  EBGEOMETRY_EXPECT(m_control != nullptr);

  return m_control->m_base;
#endif
}

template <class T, class Meta>
EBGEOMETRY_HOST
inline void
MeshT<T, Meta>::attachTo(const Pool& a_pool) noexcept
{
  // A mesh's three arrays must all live in the same Pool: they are resolved against a single base.
  EBGEOMETRY_EXPECT(m_control == nullptr || m_control == a_pool.control());

  m_control = a_pool.control();
}

template <class T, class Meta>
EBGEOMETRY_HOST
inline MeshT<T, Meta>
MeshT<T, Meta>::rebasedView(const Pool& a_pool) const noexcept
{
  EBGEOMETRY_EXPECT(m_control != nullptr);                       // not already a view
  EBGEOMETRY_EXPECT(a_pool.mirrorOf() == m_control->m_id);       // a mirror of *our* pool
  EBGEOMETRY_EXPECT(m_vertices.endByte() <= a_pool.usedBytes()); // our arrays fit inside it
  EBGEOMETRY_EXPECT(m_edges.endByte() <= a_pool.usedBytes());
  EBGEOMETRY_EXPECT(m_faces.endByte() <= a_pool.usedBytes());

  Mesh view = *this;

  if (a_pool.resource().isDeviceAccessible()) {
    // A kernel cannot follow a host control block, so the base has to be captured by value. That is
    // safe precisely here: a device-accessible pool can only come from Pool::mirror, which freezes
    // it, and Pool::grow refuses a non-host-accessible resource outright -- the base cannot move.
    EBGEOMETRY_EXPECT(a_pool.isFrozen());

    view.m_control = nullptr;
    view.m_base    = a_pool.base();
  }
  else {
    // Host target: follow the destination's control block instead of snapshotting its base, so the
    // rebased view is growth-immune exactly like the original mesh -- and so that a null control
    // block keeps meaning "device view" and nothing else.
    view.m_control = a_pool.control();
    view.m_base    = nullptr;
  }

  return view;
}

template <class T, class Meta>
EBGEOMETRY_HOST
inline std::shared_ptr<MeshT<T, Meta>>
MeshT<T, Meta>::deepCopy(Pool& a_dstPool) const
{
  EBGEOMETRY_EXPECT(this->numVertices() > 0 || this->numEdges() > 0 || this->numFaces() > 0);

  // Every VertexT/EdgeT/FaceT cross-reference is an index into the owning mesh's arrays, not a
  // pointer, so copying each element by value into freshly-reserved storage is already an
  // independent, correctly-linked mesh -- no relinking pass is needed.
  auto newMesh = std::make_shared<Mesh>();

  // Note that a_dstPool may be this mesh's own Pool. The reserves below can then grow (and move)
  // the block out from under this mesh, which is harmless: every read here re-resolves through the
  // control block rather than against an address captured before the reserves.
  newMesh->reserveVertices(a_dstPool, this->numVertices());
  newMesh->reserveEdges(a_dstPool, this->numEdges());
  newMesh->reserveFaces(a_dstPool, this->numFaces());

  for (uint32_t i = 0; i < this->numVertices(); i++) {
    newMesh->addVertex(a_dstPool, this->getVertex(i));
  }

  for (uint32_t i = 0; i < this->numEdges(); i++) {
    newMesh->addEdge(a_dstPool, this->getEdge(i));
  }

  for (uint32_t i = 0; i < this->numFaces(); i++) {
    newMesh->addFace(a_dstPool, this->getFace(i));
  }

  newMesh->setSearchAlgorithm(m_algorithm);

  return newMesh;
}

template <class T, class Meta>
EBGEOMETRY_HOST
inline void
MeshT<T, Meta>::incrementWarning(std::map<std::string, size_t>& a_warnings, const std::string& a_warn) const
{
  a_warnings[a_warn] += 1;
}

template <class T, class Meta>
EBGEOMETRY_HOST
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
EBGEOMETRY_HOST
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

  for (uint32_t i = 0; i < this->numFaces(); i++) {
    const auto& f = this->getFace(i);

    if (f.getHalfEdgeIndex() == UINT32_MAX) {
      this->incrementWarning(warnings, f_noEdge);
      continue;
    }

    auto vertices = f.gatherVertexIndices(*this);
    std::sort(vertices.begin(), vertices.end());
    auto       it           = std::unique(vertices.begin(), vertices.end());
    const bool noDuplicates = (it == vertices.end());
    if (!noDuplicates) {
      this->incrementWarning(warnings, f_degenerate);
    }
  }

  for (uint32_t i = 0; i < this->numEdges(); i++) {
    const auto& e = this->getEdge(i);

    if (e.getVertexIndex() == UINT32_MAX) {
      this->incrementWarning(warnings, e_noOrigVert);
    }
    else if (e.getNextEdgeIndex() != UINT32_MAX && e.getVertexIndex() == e.getNextEdge(*this).getVertexIndex()) {
      this->incrementWarning(warnings, e_degenerate);
    }

    if (e.getPairEdgeIndex() == UINT32_MAX) {
      this->incrementWarning(warnings, e_noPairEdge);
    }
    if (e.getNextEdgeIndex() == UINT32_MAX) {
      this->incrementWarning(warnings, e_noNextEdge);
    }
    if (e.getFaceIndex() == UINT32_MAX) {
      this->incrementWarning(warnings, e_noFace);
    }
  }

  // Vertex check
  for (uint32_t i = 0; i < this->numVertices(); i++) {
    if (this->getVertex(i).getOutgoingEdgeIndex() == UINT32_MAX) {
      this->incrementWarning(warnings, v_noEdge);
    }
  }

  this->printWarnings(warnings, a_id);
}

template <class T, class Meta>
EBGEOMETRY_HOST_DEVICE
inline void
MeshT<T, Meta>::setSearchAlgorithm(const SearchAlgorithm a_algorithm) noexcept
{
  m_algorithm = a_algorithm;
}

template <class T, class Meta>
EBGEOMETRY_HOST_DEVICE
inline void
MeshT<T, Meta>::setInsideOutsideAlgorithm(InsideOutsideAlgorithm a_algorithm) noexcept
{
  for (uint32_t i = 0; i < this->numFaces(); i++) {
    this->getFace(i).setInsideOutsideAlgorithm(a_algorithm);
  }
}

template <class T, class Meta>
EBGEOMETRY_HOST
inline void
MeshT<T, Meta>::reconcile(const DCEL::VertexNormalWeight a_weight) noexcept
{
  this->reconcileFaces();
  this->reconcileEdges();
  this->reconcileVertices(a_weight);
}

template <class T, class Meta>
EBGEOMETRY_HOST_DEVICE
inline void
MeshT<T, Meta>::flip() noexcept
{
  this->flipFaceNormals();
  this->flipEdgeNormals();
  this->flipVertexNormals();
}

template <class T, class Meta>
EBGEOMETRY_HOST
inline void
MeshT<T, Meta>::reserveVertices(Pool& a_pool, uint32_t a_capacity)
{
  this->attachTo(a_pool);

  m_vertices.reserveFrom(a_pool, a_capacity);
}

template <class T, class Meta>
EBGEOMETRY_HOST
inline void
MeshT<T, Meta>::reserveEdges(Pool& a_pool, uint32_t a_capacity)
{
  this->attachTo(a_pool);

  m_edges.reserveFrom(a_pool, a_capacity);
}

template <class T, class Meta>
EBGEOMETRY_HOST
inline void
MeshT<T, Meta>::reserveFaces(Pool& a_pool, uint32_t a_capacity)
{
  this->attachTo(a_pool);

  m_faces.reserveFrom(a_pool, a_capacity);
}

template <class T, class Meta>
EBGEOMETRY_HOST
inline uint32_t
MeshT<T, Meta>::addVertex(Pool& a_pool, const Vertex& a_vertex)
{
  const uint32_t index = m_vertices.size();

  m_vertices.push_back(a_pool.base(), a_vertex);

  return index;
}

template <class T, class Meta>
EBGEOMETRY_HOST
inline uint32_t
MeshT<T, Meta>::addEdge(Pool& a_pool, const Edge& a_edge)
{
  const uint32_t index = m_edges.size();

  m_edges.push_back(a_pool.base(), a_edge);

  return index;
}

template <class T, class Meta>
EBGEOMETRY_HOST
inline uint32_t
MeshT<T, Meta>::addFace(Pool& a_pool, const Face& a_face)
{
  const uint32_t index = m_faces.size();

  m_faces.push_back(a_pool.base(), a_face);

  return index;
}

template <class T, class Meta>
EBGEOMETRY_HOST_DEVICE
inline VertexT<T, Meta>&
MeshT<T, Meta>::getVertex(uint32_t a_index) noexcept
{
  return m_vertices.at(this->base(), a_index);
}

template <class T, class Meta>
EBGEOMETRY_HOST_DEVICE
inline const VertexT<T, Meta>&
MeshT<T, Meta>::getVertex(uint32_t a_index) const noexcept
{
  return m_vertices.at(this->base(), a_index);
}

template <class T, class Meta>
EBGEOMETRY_HOST_DEVICE
inline EdgeT<T, Meta>&
MeshT<T, Meta>::getEdge(uint32_t a_index) noexcept
{
  return m_edges.at(this->base(), a_index);
}

template <class T, class Meta>
EBGEOMETRY_HOST_DEVICE
inline const EdgeT<T, Meta>&
MeshT<T, Meta>::getEdge(uint32_t a_index) const noexcept
{
  return m_edges.at(this->base(), a_index);
}

template <class T, class Meta>
EBGEOMETRY_HOST_DEVICE
inline FaceT<T, Meta>&
MeshT<T, Meta>::getFace(uint32_t a_index) noexcept
{
  return m_faces.at(this->base(), a_index);
}

template <class T, class Meta>
EBGEOMETRY_HOST_DEVICE
inline const FaceT<T, Meta>&
MeshT<T, Meta>::getFace(uint32_t a_index) const noexcept
{
  return m_faces.at(this->base(), a_index);
}

template <class T, class Meta>
EBGEOMETRY_HOST
inline bool
MeshT<T, Meta>::isAttachedTo(const Pool& a_pool) const noexcept
{
  return m_control != nullptr && m_control == a_pool.control();
}

template <class T, class Meta>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
MeshT<T, Meta>::numVertices() const noexcept
{
  return m_vertices.size();
}

template <class T, class Meta>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
MeshT<T, Meta>::numEdges() const noexcept
{
  return m_edges.size();
}

template <class T, class Meta>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
MeshT<T, Meta>::numFaces() const noexcept
{
  return m_faces.size();
}

template <class T, class Meta>
EBGEOMETRY_HOST
inline void
MeshT<T, Meta>::reconcileFaces() noexcept
{
  for (uint32_t i = 0; i < this->numFaces(); i++) {
    this->getFace(i).reconcile(*this);
  }
}

template <class T, class Meta>
EBGEOMETRY_HOST_DEVICE
inline void
MeshT<T, Meta>::reconcileEdges() noexcept
{
  for (uint32_t i = 0; i < this->numEdges(); i++) {
    this->getEdge(i).reconcile(*this);
  }
}

template <class T, class Meta>
EBGEOMETRY_HOST
inline void
MeshT<T, Meta>::reconcileVertices(const DCEL::VertexNormalWeight a_weight) noexcept
{
  // Transient (not stored) adjacency: for each vertex, every face touching it, found by walking
  // each face's own boundary loop (nextEdge only -- no pair edge needed, so this works even on a
  // non-watertight/"dirty" mesh). Rebuilt here and discarded once vertex normals are computed;
  // VertexT itself stores no per-vertex face list.
  std::vector<std::vector<uint32_t>> facesTouchingVertex(this->numVertices());

  for (uint32_t faceIndex = 0; faceIndex < this->numFaces(); faceIndex++) {
    for (const uint32_t vertexIndex : this->getFace(faceIndex).gatherVertexIndices(*this)) {
      EBGEOMETRY_EXPECT(vertexIndex < facesTouchingVertex.size());

      facesTouchingVertex[vertexIndex].push_back(faceIndex);
    }
  }

  for (uint32_t vertexIndex = 0; vertexIndex < this->numVertices(); vertexIndex++) {
    auto&       v           = this->getVertex(vertexIndex);
    const auto& faceIndices = facesTouchingVertex[vertexIndex];

    switch (a_weight) {
    case DCEL::VertexNormalWeight::None: {
      v.computeVertexNormalAverage(faceIndices, *this);

      break;
    }
    case DCEL::VertexNormalWeight::Angle: {
      v.computeVertexNormalAngleWeighted(vertexIndex, faceIndices, *this);

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
EBGEOMETRY_HOST_DEVICE
inline void
MeshT<T, Meta>::flipFaceNormals() noexcept
{
  for (uint32_t i = 0; i < this->numFaces(); i++) {
    this->getFace(i).flipNormal();
  }
}

template <class T, class Meta>
EBGEOMETRY_HOST_DEVICE
inline void
MeshT<T, Meta>::flipEdgeNormals() noexcept
{
  for (uint32_t i = 0; i < this->numEdges(); i++) {
    this->getEdge(i).flipNormal();
  }
}

template <class T, class Meta>
EBGEOMETRY_HOST_DEVICE
inline void
MeshT<T, Meta>::flipVertexNormals() noexcept
{
  for (uint32_t i = 0; i < this->numVertices(); i++) {
    this->getVertex(i).flipNormal();
  }
}

template <class T, class Meta>
EBGEOMETRY_HOST
inline std::vector<Vec3T<T>>
MeshT<T, Meta>::getAllVertexCoordinates() const noexcept
{
  std::vector<Vec3> vertexCoordinates;
  vertexCoordinates.reserve(this->numVertices());

  for (uint32_t i = 0; i < this->numVertices(); i++) {
    vertexCoordinates.emplace_back(this->getVertex(i).getPosition());
  }

  return vertexCoordinates;
}

template <class T, class Meta>
EBGEOMETRY_HOST_DEVICE
inline T
MeshT<T, Meta>::signedDistance(const Vec3& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  return this->signedDistance(a_point, m_algorithm);
}

template <class T, class Meta>
EBGEOMETRY_HOST_DEVICE
inline T
MeshT<T, Meta>::unsignedDistance2(const Vec3& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  if (this->numFaces() == 0) {
    return std::numeric_limits<T>::infinity();
  }

  T minDist2 = std::numeric_limits<T>::max();

  for (uint32_t i = 0; i < this->numFaces(); i++) {
    const T curDist2 = this->getFace(i).unsignedDistance2(a_point, *this);

    minDist2 = std::min(minDist2, curDist2);
  }

  return minDist2;
}

template <class T, class Meta>
EBGEOMETRY_HOST_DEVICE
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
    // a_algorithm does not match any of the known SearchAlgorithm enumerators -- a corrupted or
    // out-of-range enum value rather than a normal runtime condition. EBGEOMETRY_EXPECT() alone
    // (not std::cerr, unlike the equivalent defensive branches elsewhere in this class) is the sole
    // diagnostic here so this function stays EBGEOMETRY_HOST_DEVICE.
    EBGEOMETRY_EXPECT(false);

    break;
  }
  }

  return minDist;
}

template <class T, class Meta>
EBGEOMETRY_HOST_DEVICE
inline T
MeshT<T, Meta>::DirectSignedDistance(const Vec3& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  if (this->numFaces() == 0) {
    return std::numeric_limits<T>::infinity();
  }

  T minDist  = this->getFace(0).signedDistance(a_point, *this);
  T minDist2 = minDist * minDist;

  for (uint32_t i = 0; i < this->numFaces(); i++) {
    const T curDist  = this->getFace(i).signedDistance(a_point, *this);
    const T curDist2 = curDist * curDist;

    if (curDist2 < minDist2) {
      minDist  = curDist;
      minDist2 = curDist2;
    }
  }

  return minDist;
}

template <class T, class Meta>
EBGEOMETRY_HOST_DEVICE
inline T
MeshT<T, Meta>::DirectSignedDistance2(const Vec3& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  if (this->numFaces() == 0) {
    return std::numeric_limits<T>::infinity();
  }

  uint32_t closestIndex = 0;
  T        minDist2     = this->getFace(closestIndex).unsignedDistance2(a_point, *this);

  for (uint32_t i = 0; i < this->numFaces(); i++) {
    const T curDist2 = this->getFace(i).unsignedDistance2(a_point, *this);

    if (curDist2 < minDist2) {
      closestIndex = i;
      minDist2     = curDist2;
    }
  }

  return this->getFace(closestIndex).signedDistance(a_point, *this);
}
} // namespace DCEL

} // namespace EBGeometry

#endif
