// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_SoupImplem.hpp
 * @brief  Implementation of EBGeometry_Soup.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_SOUPIMPLEM_HPP
#define EBGEOMETRY_SOUPIMPLEM_HPP

// Std includes
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_DCEL_Edge.hpp"
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_DCEL_Vertex.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Soup.hpp"

namespace EBGeometry {

template <typename T>
inline bool
Soup::containsDegeneratePolygons(const std::vector<EBGeometry::Vec3T<T>>& a_vertices,
                                 const std::vector<std::vector<size_t>>&  a_facets) noexcept
{
  static_assert(std::is_floating_point_v<T>, "Soup::containsDegeneratePolygons requires a floating-point T");

  using Vec3 = EBGeometry::Vec3T<T>;

  for (const auto& facet : a_facets) {

    if (facet.size() >= 3) {

      // Build the vertex vector.
      std::vector<Vec3> vertices;
      vertices.reserve(facet.size());
      for (const auto& ind : facet) {
        EBGEOMETRY_EXPECT(ind < a_vertices.size());

        vertices.emplace_back(a_vertices[ind]);
      }

      std::sort(vertices.begin(), vertices.end(), [](const Vec3& a, const Vec3& b) { return a.lessLX(b); });

      for (size_t i = 1; i < vertices.size(); i++) {
        const Vec3 cur = vertices[i];
        const Vec3 pre = vertices[i - 1];

        if (cur == pre) {
          return true;
        }
      }
    }
    else {
      return true;
    }
  }

  return false;
}

template <typename T>
inline void
Soup::compress(std::vector<EBGeometry::Vec3T<T>>& a_vertices, std::vector<std::vector<size_t>>& a_facets) noexcept
{
  static_assert(std::is_floating_point_v<T>, "Soup::compress requires a floating-point T");

  using Vec3 = EBGeometry::Vec3T<T>;

  // TLDR: Polygon soups read from file (STL, OBJ, PLY, VTK, ...) typically contain many duplicate
  //       vertices (each facet lists its own copies of shared vertex positions). We need to remove
  //       those duplicates and also update a_facets such that each facet references the compressed
  //       vertex vector.

  [[maybe_unused]] const size_t originalVertexCount = a_vertices.size();

  // Create a "map" of the vertices, storing their original indices. Then sort
  // the map lexicographically.
  std::vector<std::pair<Vec3, size_t>> vertexMap;
  for (size_t i = 0; i < a_vertices.size(); i++) {
    vertexMap.emplace_back(a_vertices[i], i);
  }

  std::sort(vertexMap.begin(), vertexMap.end(), [](const std::pair<Vec3, size_t>& A, const std::pair<Vec3, size_t>& B) {
    const Vec3& a = A.first;
    const Vec3& b = B.first;

    return a.lessLX(b);
  });

  // Compress the vertex vector. While doing so we should build up the old-to-new index map
  a_vertices.clear();

  if (vertexMap.empty()) {
    a_facets.clear();
    return;
  }

  std::map<size_t, size_t> indexMap;

  a_vertices.emplace_back(vertexMap.front().first);
  indexMap.emplace(vertexMap.front().second, 0);

  for (size_t i = 1; i < vertexMap.size(); i++) {
    const size_t oldIndex = vertexMap[i].second;

    const auto& cur  = vertexMap[i].first;
    const auto& prev = vertexMap[i - 1].first;

    if (cur != prev) {
      a_vertices.emplace_back(cur);
    }

    indexMap.emplace(oldIndex, a_vertices.size() - 1);
  }

  // Fix facet indicing. Use find() rather than at() so that a malformed facet (referencing a
  // vertex index that was never in the original a_vertices) triggers a diagnosable
  // EBGEOMETRY_EXPECT rather than an uncontrolled std::terminate() from throwing inside this
  // noexcept function.
  for (auto& facet : a_facets) {
    for (size_t& ivert : facet) {
      EBGEOMETRY_EXPECT(ivert < originalVertexCount);

      const auto it = indexMap.find(ivert);
      EBGEOMETRY_EXPECT(it != indexMap.end());

      ivert = it->second;
    }
  }
}

template <typename T, typename Meta>
inline void
Soup::soupToDCEL(EBGeometry::DCEL::MeshT<T, Meta>&        a_mesh,
                 Pool&                                    a_pool,
                 const std::vector<EBGeometry::Vec3T<T>>& a_vertices,
                 const std::vector<std::vector<size_t>>&  a_facets,
                 const std::string&                       a_id) noexcept
{
  static_assert(std::is_floating_point_v<T>, "Soup::soupToDCEL requires a floating-point T");

  using Vec3   = EBGeometry::Vec3T<T>;
  using Vertex = EBGeometry::DCEL::VertexT<T, Meta>;
  using Edge   = EBGeometry::DCEL::EdgeT<T, Meta>;
  using Face   = EBGeometry::DCEL::FaceT<T, Meta>;

  // Upper bound on the half-edge count: every facet contributes one half-edge per vertex, even
  // ones later skipped below for having fewer than 3 vertices. PODVector capacity need not be
  // used exactly, so reserving this upper bound (rather than a second pass to compute the exact
  // count) is fine.
  size_t numEdgesUpperBound = 0;
  for (const auto& curFacet : a_facets) {
    numEdgesUpperBound += curFacet.size();
  }

  a_mesh.reserveVertices(a_pool, static_cast<uint32_t>(a_vertices.size()));
  a_mesh.reserveEdges(a_pool, static_cast<uint32_t>(numEdgesUpperBound));
  a_mesh.reserveFaces(a_pool, static_cast<uint32_t>(a_facets.size()));

  // Build the vertex array from the input vertices; index i here becomes vertex index i in the
  // mesh, matching how a_facets already indexes into a_vertices.
  for (const auto& v : a_vertices) {
    a_mesh.addVertex(a_pool, Vertex(v, Vec3::zeros()));
  }

  // Now build the faces, appending each facet's half-edges directly into the mesh's own edge/face
  // arrays and wiring them by index rather than by pointer. a_pool.base() is re-read on every use
  // rather than cached across these calls, since addVertex/addEdge/addFace can grow (and move) the
  // pool's block.
  for (const auto& curFacet : a_facets) {
    if (curFacet.size() < 3) {
      std::cerr << "Parser::soupToDCEL -- not enough vertices in face, skipping it\n";

      // The cerr above only warns; falling through here would reach getEdge(firstEdgeIndex) below
      // on a 0-vertex facet, which is a bounds violation.
      EBGEOMETRY_EXPECT(curFacet.size() >= 3);

      continue;
    }

    const uint32_t firstEdgeIndex = a_mesh.numEdges();
    const uint32_t numFaceEdges   = static_cast<uint32_t>(curFacet.size());

    // Build the half-edges for this polygon: one per vertex, appended contiguously.
    for (uint32_t i = 0; i < numFaceEdges; i++) {
      const uint32_t vertexIndex = static_cast<uint32_t>(curFacet[i]);
      EBGEOMETRY_EXPECT(vertexIndex < a_mesh.numVertices());

      a_mesh.addEdge(a_pool, Edge(vertexIndex));
      a_mesh.getVertex(a_pool.base(), vertexIndex).setEdge(firstEdgeIndex + i);
    }

    for (uint32_t i = 0; i < numFaceEdges; i++) {
      a_mesh.getEdge(a_pool.base(), firstEdgeIndex + i).setNextEdge(firstEdgeIndex + (i + 1) % numFaceEdges);
    }

    const uint32_t faceIndex = a_mesh.numFaces();
    a_mesh.addFace(a_pool, Face(firstEdgeIndex));

    for (uint32_t i = 0; i < numFaceEdges; i++) {
      a_mesh.getEdge(a_pool.base(), firstEdgeIndex + i).setFace(faceIndex);
    }
  }

  // Reconcile the pair edges and run a sanity check. Both run before a_mesh can be bind()'d (see
  // soupToDCEL's own doc), so they resolve against a_pool's current base explicitly.
  Soup::reconcilePairEdgesDCEL(a_mesh, a_pool);

  a_mesh.sanityCheck(a_pool.base(), a_id);

  a_mesh.reconcile(a_pool.base(), EBGeometry::DCEL::VertexNormalWeight::Angle);
}

template <typename T, typename Meta>
inline void
Soup::reconcilePairEdgesDCEL(EBGeometry::DCEL::MeshT<T, Meta>& a_mesh, Pool& a_pool) noexcept
{
  static_assert(std::is_floating_point_v<T>, "Soup::reconcilePairEdgesDCEL requires a floating-point T");

  using Edge = EBGeometry::DCEL::EdgeT<T, Meta>;

  void* const base = a_pool.base();

  const uint32_t numEdges    = a_mesh.numEdges();
  const uint32_t numVertices = a_mesh.numVertices();

  // Local, transient bookkeeping (NOT stored on VertexT, which keeps only a single outgoing-edge
  // index -- see its class-level note): which half-edges start at each vertex, so the search below
  // stays O(V + E) instead of an O(E^2) scan over every edge pair.
  std::vector<std::vector<uint32_t>> edgesStartingAtVertex(numVertices);
  for (uint32_t i = 0; i < numEdges; i++) {
    const uint32_t v = a_mesh.getEdge(base, i).getVertexIndex();

    EBGEOMETRY_EXPECT(v < numVertices);

    edgesStartingAtVertex[v].push_back(i);
  }

  for (uint32_t curIndex = 0; curIndex < numEdges; curIndex++) {
    Edge& curEdge = a_mesh.getEdge(base, curIndex);

    const uint32_t nextIndex = curEdge.getNextEdgeIndex();
    EBGEOMETRY_EXPECT(nextIndex != UINT32_MAX);

    const uint32_t vertexStart = curEdge.getVertexIndex();
    const uint32_t vertexEnd   = a_mesh.getEdge(base, nextIndex).getVertexIndex();

    // The pair edge starts where this edge ends, and ends where this edge starts.
    for (const uint32_t candIndex : edgesStartingAtVertex[vertexEnd]) {
      Edge&          candEdge      = a_mesh.getEdge(base, candIndex);
      const uint32_t candNextIndex = candEdge.getNextEdgeIndex();

      EBGEOMETRY_EXPECT(candNextIndex != UINT32_MAX);

      if (a_mesh.getEdge(base, candNextIndex).getVertexIndex() == vertexStart) { // Found the pair edge
        curEdge.setPairEdge(candIndex);
        candEdge.setPairEdge(curIndex);

        break;
      }
    }
  }
}

} // namespace EBGeometry

#endif
