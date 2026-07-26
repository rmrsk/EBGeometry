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
inline void
MeshT<T, Meta>::bind(const Pool& a_pool) noexcept
{
  EBGEOMETRY_EXPECT(a_pool.isFrozen());

  m_base = a_pool.base();
}

template <class T, class Meta>
inline MeshT<T, Meta>
MeshT<T, Meta>::boundView(const void* a_base) const noexcept
{
  Mesh view = *this;

  // Mesh::m_base is void* (Pool::base() never hands back a const void*, and the same field
  // resolves both mutable and immutable accessors), so a caller reaching us through a const-
  // qualified explicit-base overload needs this cast. It is safe: view is a disposable local that
  // is only ever handed onward as a const Mesh&, so its own non-const accessors are never invoked.
  view.m_base = const_cast<void*>(a_base);

  return view;
}

template <class T, class Meta>
inline std::shared_ptr<MeshT<T, Meta>>
MeshT<T, Meta>::deepCopy(const void* a_srcBase, Pool& a_dstPool) const
{
  EBGEOMETRY_EXPECT(this->numVertices() > 0 || this->numEdges() > 0 || this->numFaces() > 0);

  // Every VertexT/EdgeT/FaceT cross-reference is an index into the owning mesh's arrays, not a
  // pointer, so copying each element by value into freshly-reserved storage is already an
  // independent, correctly-linked mesh -- no relinking pass is needed.
  auto newMesh = std::make_shared<Mesh>();

  newMesh->reserveVertices(a_dstPool, this->numVertices());
  newMesh->reserveEdges(a_dstPool, this->numEdges());
  newMesh->reserveFaces(a_dstPool, this->numFaces());

  for (uint32_t i = 0; i < this->numVertices(); i++) {
    newMesh->addVertex(a_dstPool, this->getVertex(a_srcBase, i));
  }

  for (uint32_t i = 0; i < this->numEdges(); i++) {
    newMesh->addEdge(a_dstPool, this->getEdge(a_srcBase, i));
  }

  for (uint32_t i = 0; i < this->numFaces(); i++) {
    newMesh->addFace(a_dstPool, this->getFace(a_srcBase, i));
  }

  newMesh->setSearchAlgorithm(m_algorithm);

  return newMesh;
}

template <class T, class Meta>
inline std::shared_ptr<MeshT<T, Meta>>
MeshT<T, Meta>::deepCopy(Pool& a_dstPool) const
{
  return this->deepCopy(m_base, a_dstPool);
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
MeshT<T, Meta>::sanityCheck(const void* a_base, const std::string a_id) const
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

  const Mesh view = this->boundView(a_base);

  for (uint32_t i = 0; i < this->numFaces(); i++) {
    const auto& f = this->getFace(a_base, i);

    if (f.getHalfEdgeIndex() == UINT32_MAX) {
      this->incrementWarning(warnings, f_noEdge);
      continue;
    }

    auto vertices = f.gatherVertexIndices(view);
    std::sort(vertices.begin(), vertices.end());
    auto       it           = std::unique(vertices.begin(), vertices.end());
    const bool noDuplicates = (it == vertices.end());
    if (!noDuplicates) {
      this->incrementWarning(warnings, f_degenerate);
    }
  }

  for (uint32_t i = 0; i < this->numEdges(); i++) {
    const auto& e = this->getEdge(a_base, i);

    if (e.getVertexIndex() == UINT32_MAX) {
      this->incrementWarning(warnings, e_noOrigVert);
    }
    else if (e.getNextEdgeIndex() != UINT32_MAX && e.getVertexIndex() == e.getNextEdge(view).getVertexIndex()) {
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
    if (this->getVertex(a_base, i).getOutgoingEdgeIndex() == UINT32_MAX) {
      this->incrementWarning(warnings, v_noEdge);
    }
  }

  this->printWarnings(warnings, a_id);
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::sanityCheck(const std::string a_id) const
{
  this->sanityCheck(m_base, a_id);
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::setSearchAlgorithm(const SearchAlgorithm a_algorithm) noexcept
{
  m_algorithm = a_algorithm;
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::setInsideOutsideAlgorithm(void* a_base, InsideOutsideAlgorithm a_algorithm) noexcept
{
  for (uint32_t i = 0; i < this->numFaces(); i++) {
    this->getFace(a_base, i).setInsideOutsideAlgorithm(a_algorithm);
  }
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::setInsideOutsideAlgorithm(InsideOutsideAlgorithm a_algorithm) noexcept
{
  this->setInsideOutsideAlgorithm(m_base, a_algorithm);
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::reconcile(void* a_base, const DCEL::VertexNormalWeight a_weight) noexcept
{
  this->reconcileFaces(a_base);
  this->reconcileEdges(a_base);
  this->reconcileVertices(a_base, a_weight);
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::reconcile(const DCEL::VertexNormalWeight a_weight) noexcept
{
  this->reconcile(m_base, a_weight);
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::flip(void* a_base) noexcept
{
  this->flipFaceNormals(a_base);
  this->flipEdgeNormals(a_base);
  this->flipVertexNormals(a_base);
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::flip() noexcept
{
  this->flip(m_base);
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::reserveVertices(Pool& a_pool, uint32_t a_capacity)
{
  m_vertices.reserveFrom(a_pool, a_capacity);
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::reserveEdges(Pool& a_pool, uint32_t a_capacity)
{
  m_edges.reserveFrom(a_pool, a_capacity);
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::reserveFaces(Pool& a_pool, uint32_t a_capacity)
{
  m_faces.reserveFrom(a_pool, a_capacity);
}

template <class T, class Meta>
inline uint32_t
MeshT<T, Meta>::addVertex(Pool& a_pool, const Vertex& a_vertex)
{
  const uint32_t index = m_vertices.size();

  m_vertices.push_back(a_pool.base(), a_vertex);

  return index;
}

template <class T, class Meta>
inline uint32_t
MeshT<T, Meta>::addEdge(Pool& a_pool, const Edge& a_edge)
{
  const uint32_t index = m_edges.size();

  m_edges.push_back(a_pool.base(), a_edge);

  return index;
}

template <class T, class Meta>
inline uint32_t
MeshT<T, Meta>::addFace(Pool& a_pool, const Face& a_face)
{
  const uint32_t index = m_faces.size();

  m_faces.push_back(a_pool.base(), a_face);

  return index;
}

template <class T, class Meta>
inline VertexT<T, Meta>&
MeshT<T, Meta>::getVertex(void* a_base, uint32_t a_index) noexcept
{
  return m_vertices.at(a_base, a_index);
}

template <class T, class Meta>
inline const VertexT<T, Meta>&
MeshT<T, Meta>::getVertex(const void* a_base, uint32_t a_index) const noexcept
{
  return m_vertices.at(a_base, a_index);
}

template <class T, class Meta>
inline VertexT<T, Meta>&
MeshT<T, Meta>::getVertex(uint32_t a_index) noexcept
{
  return this->getVertex(m_base, a_index);
}

template <class T, class Meta>
inline const VertexT<T, Meta>&
MeshT<T, Meta>::getVertex(uint32_t a_index) const noexcept
{
  return this->getVertex(m_base, a_index);
}

template <class T, class Meta>
inline EdgeT<T, Meta>&
MeshT<T, Meta>::getEdge(void* a_base, uint32_t a_index) noexcept
{
  return m_edges.at(a_base, a_index);
}

template <class T, class Meta>
inline const EdgeT<T, Meta>&
MeshT<T, Meta>::getEdge(const void* a_base, uint32_t a_index) const noexcept
{
  return m_edges.at(a_base, a_index);
}

template <class T, class Meta>
inline EdgeT<T, Meta>&
MeshT<T, Meta>::getEdge(uint32_t a_index) noexcept
{
  return this->getEdge(m_base, a_index);
}

template <class T, class Meta>
inline const EdgeT<T, Meta>&
MeshT<T, Meta>::getEdge(uint32_t a_index) const noexcept
{
  return this->getEdge(m_base, a_index);
}

template <class T, class Meta>
inline FaceT<T, Meta>&
MeshT<T, Meta>::getFace(void* a_base, uint32_t a_index) noexcept
{
  return m_faces.at(a_base, a_index);
}

template <class T, class Meta>
inline const FaceT<T, Meta>&
MeshT<T, Meta>::getFace(const void* a_base, uint32_t a_index) const noexcept
{
  return m_faces.at(a_base, a_index);
}

template <class T, class Meta>
inline FaceT<T, Meta>&
MeshT<T, Meta>::getFace(uint32_t a_index) noexcept
{
  return this->getFace(m_base, a_index);
}

template <class T, class Meta>
inline const FaceT<T, Meta>&
MeshT<T, Meta>::getFace(uint32_t a_index) const noexcept
{
  return this->getFace(m_base, a_index);
}

template <class T, class Meta>
inline uint32_t
MeshT<T, Meta>::numVertices() const noexcept
{
  return m_vertices.size();
}

template <class T, class Meta>
inline uint32_t
MeshT<T, Meta>::numEdges() const noexcept
{
  return m_edges.size();
}

template <class T, class Meta>
inline uint32_t
MeshT<T, Meta>::numFaces() const noexcept
{
  return m_faces.size();
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::reconcileFaces(void* a_base) noexcept
{
  const Mesh view = this->boundView(a_base);

  for (uint32_t i = 0; i < this->numFaces(); i++) {
    this->getFace(a_base, i).reconcile(view);
  }
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::reconcileEdges(void* a_base) noexcept
{
  const Mesh view = this->boundView(a_base);

  for (uint32_t i = 0; i < this->numEdges(); i++) {
    this->getEdge(a_base, i).reconcile(view);
  }
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::reconcileVertices(void* a_base, const DCEL::VertexNormalWeight a_weight) noexcept
{
  const Mesh view = this->boundView(a_base);

  // Transient (not stored) adjacency: for each vertex, every face touching it, found by walking
  // each face's OWN boundary loop (nextEdge only -- no pair edge needed). This is exactly as
  // robust to non-watertight/"dirty" meshes as the per-vertex face list this class used to cache
  // permanently, but is rebuilt here and discarded once vertex normals are computed, so it never
  // threatens VertexT's trivial copyability.
  std::vector<std::vector<uint32_t>> facesTouchingVertex(this->numVertices());

  for (uint32_t faceIndex = 0; faceIndex < this->numFaces(); faceIndex++) {
    for (const uint32_t vertexIndex : this->getFace(a_base, faceIndex).gatherVertexIndices(view)) {
      EBGEOMETRY_EXPECT(vertexIndex < facesTouchingVertex.size());

      facesTouchingVertex[vertexIndex].push_back(faceIndex);
    }
  }

  for (uint32_t vertexIndex = 0; vertexIndex < this->numVertices(); vertexIndex++) {
    auto&       v           = this->getVertex(a_base, vertexIndex);
    const auto& faceIndices = facesTouchingVertex[vertexIndex];

    switch (a_weight) {
    case DCEL::VertexNormalWeight::None: {
      v.computeVertexNormalAverage(faceIndices, view);

      break;
    }
    case DCEL::VertexNormalWeight::Angle: {
      v.computeVertexNormalAngleWeighted(vertexIndex, faceIndices, view);

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
MeshT<T, Meta>::flipFaceNormals(void* a_base) noexcept
{
  for (uint32_t i = 0; i < this->numFaces(); i++) {
    this->getFace(a_base, i).flipNormal();
  }
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::flipEdgeNormals(void* a_base) noexcept
{
  for (uint32_t i = 0; i < this->numEdges(); i++) {
    this->getEdge(a_base, i).flipNormal();
  }
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::flipVertexNormals(void* a_base) noexcept
{
  for (uint32_t i = 0; i < this->numVertices(); i++) {
    this->getVertex(a_base, i).flipNormal();
  }
}

template <class T, class Meta>
inline std::vector<Vec3T<T>>
MeshT<T, Meta>::getAllVertexCoordinates(const void* a_base) const noexcept
{
  std::vector<Vec3> vertexCoordinates;
  vertexCoordinates.reserve(this->numVertices());

  for (uint32_t i = 0; i < this->numVertices(); i++) {
    vertexCoordinates.emplace_back(this->getVertex(a_base, i).getPosition());
  }

  return vertexCoordinates;
}

template <class T, class Meta>
inline std::vector<Vec3T<T>>
MeshT<T, Meta>::getAllVertexCoordinates() const noexcept
{
  return this->getAllVertexCoordinates(m_base);
}

template <class T, class Meta>
inline T
MeshT<T, Meta>::signedDistance(const void* a_base, const Vec3& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  return this->signedDistance(a_base, a_point, m_algorithm);
}

template <class T, class Meta>
inline T
MeshT<T, Meta>::signedDistance(const Vec3& a_point) const noexcept
{
  return this->signedDistance(m_base, a_point);
}

template <class T, class Meta>
inline T
MeshT<T, Meta>::unsignedDistance2(const void* a_base, const Vec3& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  if (this->numFaces() == 0) {
    return std::numeric_limits<T>::infinity();
  }

  const Mesh view = this->boundView(a_base);

  T minDist2 = std::numeric_limits<T>::max();

  for (uint32_t i = 0; i < this->numFaces(); i++) {
    const T curDist2 = this->getFace(a_base, i).unsignedDistance2(a_point, view);

    minDist2 = std::min(minDist2, curDist2);
  }

  return minDist2;
}

template <class T, class Meta>
inline T
MeshT<T, Meta>::unsignedDistance2(const Vec3& a_point) const noexcept
{
  return this->unsignedDistance2(m_base, a_point);
}

template <class T, class Meta>
inline T
MeshT<T, Meta>::signedDistance(const void* a_base, const Vec3& a_point, SearchAlgorithm a_algorithm) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  T minDist = std::numeric_limits<T>::max();

  switch (a_algorithm) {
  case SearchAlgorithm::Direct: {
    minDist = this->DirectSignedDistance(a_base, a_point);

    break;
  }
  case SearchAlgorithm::Direct2: {
    minDist = this->DirectSignedDistance2(a_base, a_point);

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
MeshT<T, Meta>::signedDistance(const Vec3& a_point, SearchAlgorithm a_algorithm) const noexcept
{
  return this->signedDistance(m_base, a_point, a_algorithm);
}

template <class T, class Meta>
inline T
MeshT<T, Meta>::DirectSignedDistance(const void* a_base, const Vec3& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  if (this->numFaces() == 0) {
    return std::numeric_limits<T>::infinity();
  }

  const Mesh view = this->boundView(a_base);

  T minDist  = this->getFace(a_base, 0).signedDistance(a_point, view);
  T minDist2 = minDist * minDist;

  for (uint32_t i = 0; i < this->numFaces(); i++) {
    const T curDist  = this->getFace(a_base, i).signedDistance(a_point, view);
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
MeshT<T, Meta>::DirectSignedDistance2(const void* a_base, const Vec3& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  if (this->numFaces() == 0) {
    return std::numeric_limits<T>::infinity();
  }

  const Mesh view = this->boundView(a_base);

  uint32_t closestIndex = 0;
  T        minDist2     = this->getFace(a_base, closestIndex).unsignedDistance2(a_point, view);

  for (uint32_t i = 0; i < this->numFaces(); i++) {
    const T curDist2 = this->getFace(a_base, i).unsignedDistance2(a_point, view);

    if (curDist2 < minDist2) {
      closestIndex = i;
      minDist2     = curDist2;
    }
  }

  return this->getFace(a_base, closestIndex).signedDistance(a_point, view);
}
} // namespace DCEL

} // namespace EBGeometry

#endif
