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
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
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

      if (Soup::isZeroArea(a_vertices, facet)) {
        return true;
      }
    }
    else {
      return true;
    }
  }

  return false;
}

template <typename T>
inline bool
Soup::isValid(const std::vector<EBGeometry::Vec3T<T>>& a_vertices,
              const std::vector<std::vector<size_t>>&  a_facets,
              std::string&                             a_reason) noexcept
{
  static_assert(std::is_floating_point_v<T>, "Soup::isValid requires a floating-point T");

  for (size_t v = 0; v < a_vertices.size(); v++) {
    const EBGeometry::Vec3T<T>& x = a_vertices[v];

    if (!(std::isfinite(x[0]) && std::isfinite(x[1]) && std::isfinite(x[2]))) {
      a_reason = "vertex " + std::to_string(v) + " has a non-finite coordinate";

      return false;
    }
  }

  for (size_t f = 0; f < a_facets.size(); f++) {
    for (const size_t v : a_facets[f]) {
      if (v >= a_vertices.size()) {
        a_reason = "face " + std::to_string(f) + " refers to vertex " + std::to_string(v) + ", but there are only " +
                   std::to_string(a_vertices.size()) + " vertices";

        return false;
      }
    }
  }

  return true;
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

template <typename T>
inline bool
Soup::isZeroArea(const std::vector<EBGeometry::Vec3T<T>>& a_vertices, const std::vector<size_t>& a_facet) noexcept
{
  static_assert(std::is_floating_point_v<T>, "Soup::isZeroArea requires a floating-point T");

  using Vec3 = EBGeometry::Vec3T<T>;

  const size_t N = a_facet.size();

  if (N < 3) {
    return true;
  }

  // Newell's method: the sum of x_i cross x_(i+1) around the polygon is twice its (vector) area.
  Vec3 normal       = Vec3::zeros();
  T    longestEdge2 = T(0);

  for (size_t i = 0; i < N; i++) {
    EBGEOMETRY_EXPECT(a_facet[i] < a_vertices.size());
    EBGEOMETRY_EXPECT(a_facet[(i + 1) % N] < a_vertices.size());

    const Vec3& x0 = a_vertices[a_facet[i]];
    const Vec3& x1 = a_vertices[a_facet[(i + 1) % N]];

    normal += x0.cross(x1);
    longestEdge2 = std::max(longestEdge2, (x1 - x0).length2());
  }

  return normal.length() <= T(64) * std::numeric_limits<T>::epsilon() * longestEdge2;
}

template <typename T>
inline size_t
Soup::removeDegeneratePolygons(const std::vector<EBGeometry::Vec3T<T>>& a_vertices,
                               std::vector<std::vector<size_t>>&        a_facets) noexcept
{
  static_assert(std::is_floating_point_v<T>, "Soup::removeDegeneratePolygons requires a floating-point T");

  using Edge = std::pair<size_t, size_t>;

  const size_t numFacets = a_facets.size();

  std::vector<bool> removed(numFacets, false);

  // Merge repeated consecutive vertices (compress() gives coincident vertices one index).
  for (size_t f = 0; f < numFacets; f++) {
    std::vector<size_t>& facet = a_facets[f];

    std::vector<size_t> merged;
    merged.reserve(facet.size());

    for (size_t i = 0; i < facet.size(); i++) {
      if (facet[i] != facet[(i + 1) % facet.size()]) {
        merged.push_back(facet[i]);
      }
    }

    facet = std::move(merged);

    if (facet.size() < 3) {
      removed[f] = true;
    }
  }

  // Directed edge (u, v) -> the facet that contains it. Used to find the facet across a
  // T-junction filler's longest edge, which contains that edge reversed.
  std::map<Edge, size_t> facetOfEdge;

  for (size_t f = 0; f < numFacets; f++) {
    if (!removed[f]) {
      const std::vector<size_t>& facet = a_facets[f];

      for (size_t i = 0; i < facet.size(); i++) {
        facetOfEdge[Edge(facet[i], facet[(i + 1) % facet.size()])] = f;
      }
    }
  }

  for (size_t f = 0; f < numFacets; f++) {
    if (removed[f] || !Soup::isZeroArea(a_vertices, a_facets[f])) {
      continue;
    }

    const std::vector<size_t> facet = a_facets[f];

    removed[f] = true;

    for (size_t i = 0; i < facet.size(); i++) {
      const auto it = facetOfEdge.find(Edge(facet[i], facet[(i + 1) % facet.size()]));

      if (it != facetOfEdge.end() && it->second == f) {
        facetOfEdge.erase(it);
      }
    }

    if (facet.size() != 3) {
      continue;
    }

    // The middle vertex of three collinear ones is the one opposite the longest edge.
    size_t longest       = 0;
    T      longestLength = T(-1);

    for (size_t i = 0; i < 3; i++) {
      const T length2 = (a_vertices[facet[(i + 1) % 3]] - a_vertices[facet[i]]).length2();

      if (length2 > longestLength) {
        longest       = i;
        longestLength = length2;
      }
    }

    const size_t p = facet[longest];
    const size_t q = facet[(longest + 1) % 3];
    const size_t m = facet[(longest + 2) % 3];

    // The facet across the longest edge contains it as q -> p. Insert m between them.
    const auto across = facetOfEdge.find(Edge(q, p));

    if (across == facetOfEdge.end()) {
      continue;
    }

    const size_t         g        = across->second;
    std::vector<size_t>& neighbor = a_facets[g];

    for (size_t i = 0; i < neighbor.size(); i++) {
      if (neighbor[i] == q && neighbor[(i + 1) % neighbor.size()] == p) {
        neighbor.insert(neighbor.begin() + static_cast<std::ptrdiff_t>(i + 1), m);

        break;
      }
    }

    facetOfEdge.erase(across);
    facetOfEdge[Edge(q, m)] = g;
    facetOfEdge[Edge(m, p)] = g;
  }

  std::vector<std::vector<size_t>> kept;
  kept.reserve(numFacets);

  for (size_t f = 0; f < numFacets; f++) {
    if (!removed[f]) {
      kept.emplace_back(std::move(a_facets[f]));
    }
  }

  const size_t numRemoved = numFacets - kept.size();

  a_facets = std::move(kept);

  return numRemoved;
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
  // arrays and wiring them by index rather than by pointer. The mesh re-resolves its base through
  // a_pool's control block on every access, so a grow that moves the block mid-loop is invisible
  // here -- but a reference obtained from getVertex/getEdge must still not be held across one.
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
      a_mesh.getVertex(vertexIndex).setEdge(firstEdgeIndex + i);
    }

    for (uint32_t i = 0; i < numFaceEdges; i++) {
      a_mesh.getEdge(firstEdgeIndex + i).setNextEdge(firstEdgeIndex + (i + 1) % numFaceEdges);
    }

    const uint32_t faceIndex = a_mesh.numFaces();
    a_mesh.addFace(a_pool, Face(firstEdgeIndex));

    for (uint32_t i = 0; i < numFaceEdges; i++) {
      a_mesh.getEdge(firstEdgeIndex + i).setFace(faceIndex);
    }
  }

  // Reconcile the pair edges, run a sanity check, and compute the normals. The mesh is queryable
  // from its first reserve onwards, so these need no base of their own and a_pool need not be frozen.
  Soup::reconcilePairEdgesDCEL(a_mesh);

  a_mesh.sanityCheck(a_id);

  a_mesh.reconcile(EBGeometry::DCEL::VertexNormalWeight::Angle);
}

template <typename T, typename Meta>
inline void
Soup::reconcilePairEdgesDCEL(EBGeometry::DCEL::MeshT<T, Meta>& a_mesh) noexcept
{
  static_assert(std::is_floating_point_v<T>, "Soup::reconcilePairEdgesDCEL requires a floating-point T");

  using Edge = EBGeometry::DCEL::EdgeT<T, Meta>;

  const uint32_t numEdges    = a_mesh.numEdges();
  const uint32_t numVertices = a_mesh.numVertices();

  // Local, transient bookkeeping (NOT stored on VertexT, which keeps only a single outgoing-edge
  // index -- see its class-level note): which half-edges start at each vertex, so the search below
  // stays O(V + E) instead of an O(E^2) scan over every edge pair.
  std::vector<std::vector<uint32_t>> edgesStartingAtVertex(numVertices);
  for (uint32_t i = 0; i < numEdges; i++) {
    const uint32_t v = a_mesh.getEdge(i).getVertexIndex();

    EBGEOMETRY_EXPECT(v < numVertices);

    edgesStartingAtVertex[v].push_back(i);
  }

  for (uint32_t curIndex = 0; curIndex < numEdges; curIndex++) {
    Edge& curEdge = a_mesh.getEdge(curIndex);

    const uint32_t nextIndex = curEdge.getNextEdgeIndex();
    EBGEOMETRY_EXPECT(nextIndex != UINT32_MAX);

    const uint32_t vertexStart = curEdge.getVertexIndex();
    const uint32_t vertexEnd   = a_mesh.getEdge(nextIndex).getVertexIndex();

    // The pair edge starts where this edge ends, and ends where this edge starts.
    for (const uint32_t candIndex : edgesStartingAtVertex[vertexEnd]) {
      Edge&          candEdge      = a_mesh.getEdge(candIndex);
      const uint32_t candNextIndex = candEdge.getNextEdgeIndex();

      EBGEOMETRY_EXPECT(candNextIndex != UINT32_MAX);

      if (a_mesh.getEdge(candNextIndex).getVertexIndex() == vertexStart) { // Found the pair edge
        curEdge.setPairEdge(candIndex);
        candEdge.setPairEdge(curIndex);

        break;
      }
    }
  }
}

} // namespace EBGeometry

#endif
