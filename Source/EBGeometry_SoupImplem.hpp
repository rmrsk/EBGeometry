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
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_Soup.hpp"

namespace EBGeometry {

template <typename T>
inline bool
Soup::containsDegeneratePolygons(const std::vector<EBGeometry::Vec3T<T>>& a_vertices,
                                 const std::vector<std::vector<size_t>>&  a_facets) noexcept
{
  using Vec3 = EBGeometry::Vec3T<T>;

  for (const auto& facet : a_facets) {

    if (facet.size() >= 3) {

      // Build the vertex vector.
      std::vector<Vec3> vertices;
      vertices.reserve(facet.size());
      for (const auto& ind : facet) {
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
  using Vec3 = EBGeometry::Vec3T<T>;

  // TLDR: Because it's an STL file, a_vertices contains many duplicate vertices. We need to remove
  //       them and also update a_facets such that each facet references the compressed vertex vector.

  // Create a "map" of the vertices, storing their original indices. Then sort
  // the map lexicographically.
  std::vector<std::pair<Vec3, size_t>> vertexMap;
  for (size_t i = 0; i < a_vertices.size(); i++) {
    vertexMap.emplace_back(a_vertices[i], i);
  }

  std::sort(vertexMap.begin(), vertexMap.end(), [](const std::pair<Vec3, size_t> A, const std::pair<Vec3, size_t> B) {
    const Vec3& a = A.first;
    const Vec3& b = B.first;

    return a.lessLX(b);
  });

  // Compress the vertex vector. While doing so we should build up the old-to-new index map
  a_vertices.resize(0);

  if (vertexMap.empty()) {
    a_facets.clear();
    return;
  }
  EBGEOMETRY_EXPECT(!vertexMap.empty());

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

  // Fix facet indicing.
  for (auto& facet : a_facets) {
    for (unsigned long& ivert : facet) {
      ivert = indexMap.at(ivert);
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

  using Vec3   = EBGeometry::Vec3T<T>;
  using Vertex = EBGeometry::DCEL::VertexT<T, Meta>;
  using Edge   = EBGeometry::DCEL::EdgeT<T, Meta>;
  using Face   = EBGeometry::DCEL::FaceT<T, Meta>;

  std::vector<std::shared_ptr<Vertex>>& vertices = a_mesh.getVertices();
  std::vector<std::shared_ptr<Edge>>&   edges    = a_mesh.getEdges();
  std::vector<std::shared_ptr<Face>>&   faces    = a_mesh.getFaces();

  // Build the vertex vectors from the input vertices.
  for (const auto& v : a_vertices) {
    vertices.emplace_back(std::make_shared<Vertex>(v, Vec3::zero()));
  }

  // Now build the faces.
  for (const auto& curFacet : a_facets) {
    if (curFacet.size() < 3) {
      std::cerr << "Parser::soupToDCEL -- not enough vertices in face\n";
    }

    // Figure out which vertices are involved here.
    std::vector<std::shared_ptr<Vertex>> faceVertices;
    faceVertices.reserve(curFacet.size());
    for (const unsigned long i : curFacet) {
      faceVertices.emplace_back(vertices[i]);
    }

    // Build the half-edges for this polygon.
    std::vector<std::shared_ptr<Edge>> halfEdges;
    for (const auto& v : faceVertices) {
      halfEdges.emplace_back(std::make_shared<Edge>(v));
      v->setEdge(halfEdges.back());
    }

    for (size_t i = 0; i < halfEdges.size(); i++) {
      auto& curEdge  = halfEdges[i];
      auto& nextEdge = halfEdges[(i + 1) % halfEdges.size()];

      curEdge->setNextEdge(nextEdge);
    }

    edges.insert(edges.end(), halfEdges.begin(), halfEdges.end());

    faces.emplace_back(std::make_shared<Face>(halfEdges.front()));
    auto& curFace = faces.back();

    for (auto& e : halfEdges) {
      e->setFace(curFace);
    }

    // Must give vertices access to all faces associated them.
    for (const auto& v : faceVertices) {
      v->addFace(curFace);
    }
  }

  // Reconcile the pair edges and run a sanity check.
  Soup::reconcilePairEdgesDCEL(edges);

  a_mesh.sanityCheck(a_id);

  a_mesh.reconcile(EBGeometry::DCEL::VertexNormalWeight::Angle);
}

template <typename T, typename Meta>
inline void
Soup::reconcilePairEdgesDCEL(std::vector<std::shared_ptr<EBGeometry::DCEL::EdgeT<T, Meta>>>& a_edges) noexcept
{
  for (auto& curEdge : a_edges) {
    const auto& nextEdge = curEdge->getNextEdge();

    const auto& vertexStart = curEdge->getVertex();
    const auto& vertexEnd   = nextEdge->getVertex();

    for (const auto& p : vertexStart->getFaces()) {
      for (EBGeometry::DCEL::EdgeIteratorT<T, Meta> edgeIt(*p); edgeIt.ok(); ++edgeIt) {
        const auto& polyCurEdge  = edgeIt();
        const auto& polyNextEdge = polyCurEdge->getNextEdge();

        const auto& polyVertexStart = polyCurEdge->getVertex();
        const auto& polyVertexEnd   = polyNextEdge->getVertex();

        if (vertexStart == polyVertexEnd && polyVertexStart == vertexEnd) { // Found the pair edge
          curEdge->setPairEdge(polyCurEdge);
          polyCurEdge->setPairEdge(curEdge);
        }
      }
    }
  }
}

} // namespace EBGeometry

#endif
