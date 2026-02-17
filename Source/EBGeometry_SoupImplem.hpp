/* EBGeometry
 * Copyright © 2026 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_SoupImplem.hpp
  @brief  Implementation of EBGeometry_Soup.hpp
  @author Robert Marskar
*/

#ifndef EBGeometry_SoupImplem
#define EBGeometry_SoupImplem

// Our includes
#include "EBGeometry_Soup.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

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
  for (size_t n = 0; n < a_facets.size(); n++) {
    std::vector<size_t>& facet = a_facets[n];

    for (size_t ivert = 0; ivert < facet.size(); ivert++) {
      facet[ivert] = indexMap.at(facet[ivert]);
    }
  }
}

template <typename T, typename Meta>
inline void
Soup::soupToDCEL(EBGeometry::DCEL::MeshT<T, Meta>&        a_mesh,
                 const std::vector<EBGeometry::Vec3T<T>>& a_vertices,
                 const std::vector<std::vector<size_t>>&  a_facets) noexcept
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
    for (size_t i = 0; i < curFacet.size(); i++) {
      faceVertices.emplace_back(vertices[curFacet[i]]);
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

  a_mesh.sanityCheck();

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

#include "EBGeometry_NamespaceFooter.hpp"

#endif
