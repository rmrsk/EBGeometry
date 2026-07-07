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
#include <unordered_map>
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
inline MeshT<T, Meta>::MeshT(std::vector<FacePtr>&   a_faces,
                             std::vector<EdgePtr>&   a_edges,
                             std::vector<VertexPtr>& a_vertices) noexcept
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
  EBGEOMETRY_EXPECT(m_vertices.size() > 0 || m_edges.size() > 0 || m_faces.size() > 0);

  // Pass 0: allocate a fresh, default-constructed object for every existing vertex/edge/face, and
  // record the old->new correspondence. This must happen before any relinking below, since a
  // vertex/edge/face's pointers can reference any other vertex/edge/face in the mesh.
  std::vector<VertexPtr> newVertices(m_vertices.size());
  std::vector<EdgePtr>   newEdges(m_edges.size());
  std::vector<FacePtr>   newFaces(m_faces.size());

  std::unordered_map<VertexPtr, VertexPtr> vertexMap;
  std::unordered_map<EdgePtr, EdgePtr>     edgeMap;
  std::unordered_map<FacePtr, FacePtr>     faceMap;

  for (size_t i = 0; i < m_vertices.size(); i++) {
    EBGEOMETRY_EXPECT(m_vertices[i] != nullptr);

    newVertices[i]           = std::make_shared<Vertex>();
    vertexMap[m_vertices[i]] = newVertices[i];
  }
  for (size_t i = 0; i < m_edges.size(); i++) {
    EBGEOMETRY_EXPECT(m_edges[i] != nullptr);

    newEdges[i]         = std::make_shared<Edge>();
    edgeMap[m_edges[i]] = newEdges[i];
  }
  for (size_t i = 0; i < m_faces.size(); i++) {
    EBGEOMETRY_EXPECT(m_faces[i] != nullptr);

    newFaces[i]         = std::make_shared<Face>();
    faceMap[m_faces[i]] = newFaces[i];
  }

  // Remap a possibly-null old pointer to its corresponding new one. A non-null pointer that is not
  // in the map means it refers to a vertex/edge/face outside of this mesh's own vectors, which is a
  // corrupted/inconsistent mesh (the same condition sanityCheck() is meant to catch beforehand).
  auto remapVertex = [&](const VertexPtr& a_v) -> VertexPtr {
    if (a_v == nullptr) {
      return nullptr;
    }
    const auto it = vertexMap.find(a_v);
    EBGEOMETRY_EXPECT(it != vertexMap.end());

    return it->second;
  };
  auto remapEdge = [&](const EdgePtr& a_e) -> EdgePtr {
    if (a_e == nullptr) {
      return nullptr;
    }
    const auto it = edgeMap.find(a_e);
    EBGEOMETRY_EXPECT(it != edgeMap.end());

    return it->second;
  };
  auto remapFace = [&](const FacePtr& a_f) -> FacePtr {
    if (a_f == nullptr) {
      return nullptr;
    }
    const auto it = faceMap.find(a_f);
    EBGEOMETRY_EXPECT(it != faceMap.end());

    return it->second;
  };

  // Pass 1: vertices. Copy value data (position, normal, meta-data) exactly, and relink the
  // outgoing edge and face list to the new objects.
  for (size_t i = 0; i < m_vertices.size(); i++) {
    const VertexPtr& oldV = m_vertices[i];
    const VertexPtr& newV = newVertices[i];

    newV->setPosition(oldV->getPosition());
    newV->setNormal(oldV->getNormal());
    newV->setMetaData(oldV->getMetaData());
    newV->setEdge(remapEdge(oldV->getOutgoingEdge()));

    for (const auto& f : oldV->getFaces()) {
      newV->addFace(remapFace(f));
    }
  }

  // Pass 2: edges. Copy the normal vector and meta-data exactly, and relink the vertex, pair edge,
  // next edge, and face to the new objects.
  for (size_t i = 0; i < m_edges.size(); i++) {
    const EdgePtr& oldE = m_edges[i];
    const EdgePtr& newE = newEdges[i];

    newE->setVertex(remapVertex(oldE->getVertex()));
    newE->setPairEdge(remapEdge(oldE->getPairEdge()));
    newE->setNextEdge(remapEdge(oldE->getNextEdge()));
    newE->setFace(remapFace(oldE->getFace()));
    newE->getNormal() = oldE->getNormal();
    newE->setMetaData(oldE->getMetaData());
  }

  // Pass 3: faces. Copy the normal vector, centroid, area, and meta-data exactly, relink the half
  // edge to the new object, and rebuild the 2D embedding from the (already-relinked) topology and
  // the (already-restored) normal vector. This deliberately does NOT call reconcile(), which would
  // recompute the normal vector from vertex winding and silently discard a prior flipNormal().
  for (size_t i = 0; i < m_faces.size(); i++) {
    const FacePtr& oldF = m_faces[i];
    const FacePtr& newF = newFaces[i];

    newF->setHalfEdge(remapEdge(oldF->getHalfEdge()));
    newF->getNormal()   = oldF->getNormal();
    newF->getCentroid() = oldF->getCentroid();
    newF->getArea()     = oldF->getArea();
    newF->setMetaData(oldF->getMetaData());
    newF->computePolygon2D();
  }

  auto newMesh = std::make_shared<Mesh>();

  newMesh->getVertices() = std::move(newVertices);
  newMesh->getEdges()    = std::move(newEdges);
  newMesh->getFaces()    = std::move(newFaces);
  newMesh->setSearchAlgorithm(m_algorithm);

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
  const std::string f_null       = "nullptr face";
  const std::string f_noEdge     = "face with no edge";
  const std::string f_degenerate = "degenerate face";

  const std::string e_null       = "nullptr edges";
  const std::string e_degenerate = "degenerate edge";
  const std::string e_noPairEdge = "no pair edge (not watertight)";
  const std::string e_noNextEdge = "no next edge (badly linked dcel)";
  const std::string e_noOrigVert = "no origin vertex found for half edge (badly linked dcel)";
  const std::string e_noFace     = "no face found for half edge (badly linked dcel)";

  const std::string v_null   = "nullptr vertex";
  const std::string v_noEdge = "no referenced edge for vertex (unreferenced vertex)";

  std::map<std::string, size_t> warnings = {{f_null, 0},
                                            {f_noEdge, 0},
                                            {f_degenerate, 0},
                                            {e_null, 0},
                                            {e_degenerate, 0},
                                            {e_noPairEdge, 0},
                                            {e_noNextEdge, 0},
                                            {e_noOrigVert, 0},
                                            {e_noFace, 0},
                                            {v_null, 0},
                                            {v_noEdge, 0}};

  for (const auto& f : m_faces) {
    if (f == nullptr) {
      this->incrementWarning(warnings, f_null);
      continue;
    }

    const auto& halfEdge = f->getHalfEdge();
    if (halfEdge == nullptr) {
      this->incrementWarning(warnings, f_noEdge);
    }

    auto vertices = f->gatherVertices();
    std::sort(vertices.begin(), vertices.end());
    auto       it           = std::unique(vertices.begin(), vertices.end());
    const bool noDuplicates = (it == vertices.end());
    if (!noDuplicates) {
      this->incrementWarning(warnings, f_degenerate);
    }
  }

  for (const auto& e : m_edges) {
    if (e == nullptr) {
      this->incrementWarning(warnings, e_null);
      continue;
    }

    if (e->getVertex() == e->getOtherVertex()) {
      this->incrementWarning(warnings, e_degenerate);
    }
    if (e->getPairEdge() == nullptr) {
      this->incrementWarning(warnings, e_noPairEdge);
    }
    if (e->getNextEdge() == nullptr) {
      this->incrementWarning(warnings, e_noNextEdge);
    }
    if (e->getVertex() == nullptr) {
      this->incrementWarning(warnings, e_noOrigVert);
    }
    if (e->getFace() == nullptr) {
      this->incrementWarning(warnings, e_noFace);
    }
  }

  // Vertex check
  for (const auto& v : m_vertices) {
    if (v == nullptr) {
      this->incrementWarning(warnings, v_null);
    }
    else if (v->getOutgoingEdge() == nullptr) {
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
    EBGEOMETRY_EXPECT(f != nullptr);

    f->setInsideOutsideAlgorithm(a_algorithm);
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
MeshT<T, Meta>::reconcileFaces() noexcept
{
  for (auto& f : m_faces) {
    EBGEOMETRY_EXPECT(f != nullptr);

    f->reconcile();
  }
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::reconcileEdges() noexcept
{
  for (auto& e : m_edges) {
    EBGEOMETRY_EXPECT(e != nullptr);

    e->reconcile();
  }
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::reconcileVertices(const DCEL::VertexNormalWeight a_weight) noexcept
{
  for (auto& v : m_vertices) {
    EBGEOMETRY_EXPECT(v != nullptr);

    switch (a_weight) {
    case DCEL::VertexNormalWeight::None: {
      v->computeVertexNormalAverage();

      break;
    }
    case DCEL::VertexNormalWeight::Angle: {
      v->computeVertexNormalAngleWeighted();

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
    EBGEOMETRY_EXPECT(f != nullptr);

    f->flipNormal();
  }
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::flipEdgeNormals() noexcept
{
  for (auto& e : m_edges) {
    EBGEOMETRY_EXPECT(e != nullptr);

    e->flipNormal();
  }
}

template <class T, class Meta>
inline void
MeshT<T, Meta>::flipVertexNormals() noexcept
{
  for (auto& v : m_vertices) {
    EBGEOMETRY_EXPECT(v != nullptr);

    v->flipNormal();
  }
}

template <class T, class Meta>
inline std::vector<std::shared_ptr<VertexT<T, Meta>>>&
MeshT<T, Meta>::getVertices() noexcept
{
  return m_vertices;
}

template <class T, class Meta>
inline const std::vector<std::shared_ptr<VertexT<T, Meta>>>&
MeshT<T, Meta>::getVertices() const noexcept
{
  return m_vertices;
}

template <class T, class Meta>
inline std::vector<std::shared_ptr<EdgeT<T, Meta>>>&
MeshT<T, Meta>::getEdges() noexcept
{
  return m_edges;
}

template <class T, class Meta>
inline const std::vector<std::shared_ptr<EdgeT<T, Meta>>>&
MeshT<T, Meta>::getEdges() const noexcept
{
  return m_edges;
}

template <class T, class Meta>
inline std::vector<std::shared_ptr<FaceT<T, Meta>>>&
MeshT<T, Meta>::getFaces() noexcept
{
  return m_faces;
}

template <class T, class Meta>
inline const std::vector<std::shared_ptr<FaceT<T, Meta>>>&
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
    EBGEOMETRY_EXPECT(v != nullptr);

    vertexCoordinates.emplace_back(v->getPosition());
  }

  return vertexCoordinates;
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

  for (const auto& f : m_faces) {
    EBGEOMETRY_EXPECT(f != nullptr);

    const T curDist2 = f->unsignedDistance2(a_point);

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

  EBGEOMETRY_EXPECT(m_faces.front() != nullptr);

  T minDist  = m_faces.front()->signedDistance(a_point);
  T minDist2 = minDist * minDist;

  for (const auto& f : m_faces) {
    EBGEOMETRY_EXPECT(f != nullptr);

    const T curDist  = f->signedDistance(a_point);
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

  EBGEOMETRY_EXPECT(m_faces.front() != nullptr);

  FacePtr closest  = m_faces.front();
  T       minDist2 = closest->unsignedDistance2(a_point);

  for (const auto& f : m_faces) {
    EBGEOMETRY_EXPECT(f != nullptr);

    const T curDist2 = f->unsignedDistance2(a_point);

    if (curDist2 < minDist2) {
      closest  = f;
      minDist2 = curDist2;
    }
  }

  return closest->signedDistance(a_point);
}
} // namespace DCEL

} // namespace EBGeometry

#endif
