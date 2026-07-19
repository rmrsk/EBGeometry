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
#include <iostream>
#include <map>
#include <memory>
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

  const size_t originalVertexCount = a_vertices.size();

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
                 const std::vector<EBGeometry::Vec3T<T>>& a_vertices,
                 const std::vector<std::vector<size_t>>&  a_facets,
                 const std::string&                       a_id) noexcept
{
  static_assert(std::is_floating_point_v<T>, "Soup::soupToDCEL requires a floating-point T");

  using Vec3      = EBGeometry::Vec3T<T>;
  using Vertex    = EBGeometry::DCEL::VertexT<T, Meta>;
  using Edge      = EBGeometry::DCEL::EdgeT<T, Meta>;
  using Face      = EBGeometry::DCEL::FaceT<T, Meta>;
  using DCELIndex = EBGeometry::DCEL::DCELIndex;

  std::vector<Vertex>& vertices = a_mesh.getVertices();
  std::vector<Edge>&   edges    = a_mesh.getEdges();
  std::vector<Face>&   faces    = a_mesh.getFaces();

  // Build the vertex array from the input vertices.
  vertices.reserve(a_vertices.size());
  for (const auto& v : a_vertices) {
    vertices.emplace_back(v, Vec3::zeros());
  }

  // Now build the faces and their half-edges directly into the flat arrays.
  for (const auto& curFacet : a_facets) {
    if (curFacet.size() < 3) {
      std::cerr << "Soup::soupToDCEL -- not enough vertices in face, skipping it\n";

      // The cerr above only warns; falling through would create a malformed face below.
      EBGEOMETRY_EXPECT(curFacet.size() >= 3);

      continue;
    }

    const DCELIndex firstEdge = static_cast<DCELIndex>(edges.size());
    const DCELIndex faceIdx   = static_cast<DCELIndex>(faces.size());
    const size_t    nEdges    = curFacet.size();

    // Create one half-edge per corner, with the corner's vertex as its origin.
    for (size_t i = 0; i < nEdges; i++) {
      EBGEOMETRY_EXPECT(curFacet[i] < vertices.size());

      const DCELIndex vertexIdx = static_cast<DCELIndex>(curFacet[i]);
      const DCELIndex edgeIdx   = firstEdge + static_cast<DCELIndex>(i);

      edges.emplace_back(vertexIdx);

      // Wire up next-edge ring cyclically and the owning face.
      edges[edgeIdx].setNextEdge(firstEdge + static_cast<DCELIndex>((i + 1) % nEdges));
      edges[edgeIdx].setFace(faceIdx);

      // Give the vertex an outgoing edge and register the incident face.
      vertices[vertexIdx].setEdge(edgeIdx);
      vertices[vertexIdx].addFace(faceIdx);
    }

    faces.emplace_back(firstEdge);
  }

  // Reconcile the pair edges and run a sanity check.
  Soup::reconcilePairEdgesDCEL(edges);

  a_mesh.sanityCheck(a_id);

  a_mesh.reconcile(EBGeometry::DCEL::VertexNormalWeight::Angle);
}

template <typename T, typename Meta>
inline void
Soup::reconcilePairEdgesDCEL(std::vector<EBGeometry::DCEL::EdgeT<T, Meta>>& a_edges) noexcept
{
  static_assert(std::is_floating_point_v<T>, "Soup::reconcilePairEdgesDCEL requires a floating-point T");

  using DCELIndex = EBGeometry::DCEL::DCELIndex;

  // Map each undirected vertex pair (min,max) to the first half-edge seen with that pair. When a
  // second half-edge with the same undirected pair appears, the two are each other's twins.
  std::map<std::pair<DCELIndex, DCELIndex>, DCELIndex> edgeMap;

  for (size_t i = 0; i < a_edges.size(); i++) {
    const DCELIndex edgeIdx  = static_cast<DCELIndex>(i);
    const DCELIndex nextEdge = a_edges[edgeIdx].getNextEdge();

    EBGEOMETRY_EXPECT(nextEdge != EBGeometry::DCEL::InvalidIndex);

    const DCELIndex u = a_edges[edgeIdx].getVertex();
    const DCELIndex w = a_edges[nextEdge].getVertex();

    EBGEOMETRY_EXPECT(u != EBGeometry::DCEL::InvalidIndex);
    EBGEOMETRY_EXPECT(w != EBGeometry::DCEL::InvalidIndex);

    const std::pair<DCELIndex, DCELIndex> key = std::minmax(u, w);

    const auto it = edgeMap.find(key);
    if (it == edgeMap.end()) {
      edgeMap.emplace(key, edgeIdx);
    }
    else {
      const DCELIndex otherEdge = it->second;

      a_edges[edgeIdx].setPairEdge(otherEdge);
      a_edges[otherEdge].setPairEdge(edgeIdx);
    }
  }
}

} // namespace EBGeometry

#endif
