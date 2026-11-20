// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

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
#include <cstring>
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
#include "EBGeometry_Math.hpp"
#include "EBGeometry_Soup.hpp"

namespace EBGeometry {

template <typename T>
inline bool
Soup::containsDegeneratePolygons(const std::vector<EBGeometry::Vec3T<T>>& a_vertices,
                                 const std::vector<std::vector<size_t>>&  a_facets)
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
              std::string&                             a_reason)
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
inline std::vector<size_t>
Soup::compress(std::vector<EBGeometry::Vec3T<T>>& a_vertices, std::vector<std::vector<size_t>>& a_facets)
{
  static_assert(std::is_floating_point_v<T>, "Soup::compress requires a floating-point T");

  using Vec3 = EBGeometry::Vec3T<T>;

  // TLDR: Polygon soups read from file (STL, OBJ, PLY, VTK, ...) typically contain many duplicate
  //       vertices (each facet lists its own copies of shared vertex positions). We need to remove
  //       those duplicates and also update a_facets such that each facet references the compressed
  //       vertex vector.

  const size_t originalVertexCount = a_vertices.size();

  // Find the distinct positions first, with a hash table, and sort only those: a file such as a binary STL lists
  // every vertex once per facet that uses it, and sorting every copy would cost several times as much. The table, with
  // open addressing (linear probing) and at most half full, holds each distinct position's index in `distinct`.
  // Positions are equal as Vec3T::operator== says, so -0 and +0 are one position and hash alike.
  constexpr size_t Empty = Math::Limits<size_t>::max();

  const auto hashOf = [](const Vec3& a_x) noexcept {
    uint64_t hash = 0x9E3779B97F4A7C15ULL;

    for (size_t dir = 0; dir < 3; dir++) {
      const T coordinate = (a_x[dir] == T(0)) ? T(0) : a_x[dir];

      std::conditional_t<sizeof(T) == sizeof(uint32_t), uint32_t, uint64_t> bits;

      static_assert(sizeof(bits) == sizeof(T), "Soup::compress: T must be a 32-bit or 64-bit floating-point type");

      std::memcpy(&bits, &coordinate, sizeof(T));

      hash = (hash ^ uint64_t(bits)) * 0xC2B2AE3D27D4EB4FULL;
      hash ^= hash >> 31;
    }

    return hash;
  };

  size_t capacity = 16;

  while (capacity < 2 * originalVertexCount) {
    capacity *= 2;
  }

  const size_t mask = capacity - 1;

  std::vector<size_t> table(capacity, Empty);
  std::vector<size_t> distinct;                        // For each distinct position, the first input vertex at it.
  std::vector<size_t> distinctOf(originalVertexCount); // For each input vertex, its distinct position.

  for (size_t i = 0; i < originalVertexCount; i++) {
    const Vec3& x = a_vertices[i];

    size_t slot = size_t(hashOf(x)) & mask;

    while (table[slot] != Empty && a_vertices[distinct[table[slot]]] != x) {
      slot = (slot + 1) & mask;
    }

    if (table[slot] == Empty) {
      table[slot] = distinct.size();
      distinct.push_back(i);
    }

    distinctOf[i] = table[slot];
  }

  // Sort the distinct positions lexicographically. No two are equal, so their order is unique.
  std::vector<std::pair<Vec3, size_t>> sorted;
  sorted.reserve(distinct.size());

  for (size_t d = 0; d < distinct.size(); d++) {
    sorted.emplace_back(a_vertices[distinct[d]], d);
  }

  std::sort(sorted.begin(), sorted.end(), [](const std::pair<Vec3, size_t>& A, const std::pair<Vec3, size_t>& B) {
    return A.first.lessLX(B.first);
  });

  // Compress the vertex vector, and build the old-to-new index map.
  std::vector<size_t> newIndexOfDistinct(distinct.size());

  a_vertices.clear();

  for (size_t i = 0; i < sorted.size(); i++) {
    a_vertices.emplace_back(sorted[i].first);

    newIndexOfDistinct[sorted[i].second] = i;
  }

  std::vector<size_t> indexMap(originalVertexCount);

  for (size_t i = 0; i < originalVertexCount; i++) {
    indexMap[i] = newIndexOfDistinct[distinctOf[i]];
  }

  // With no vertices, no facet can be valid.
  if (originalVertexCount == 0) {
    a_facets.clear();
  }

  // Fix facet indicing. A malformed facet (referencing a vertex index that was never in the original
  // a_vertices) triggers a diagnosable EBGEOMETRY_EXPECT; isValid() rules it out for the file readers.
  for (auto& facet : a_facets) {
    for (size_t& ivert : facet) {
      EBGEOMETRY_EXPECT(ivert < originalVertexCount);

      ivert = indexMap[ivert];
    }
  }

  return indexMap;
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

  // Newell's method: the sum of x_i cross x_(i+1) around the polygon is twice its (vector) area. The positions are
  // taken relative to the polygon's first vertex, as in FaceT::computeNormal(): with absolute positions each term is of
  // the order of |x|^2 while the sum is of the order of the area, so far from the origin the sum cancels, and in single
  // precision a well-shaped face 3e-3 across, 5 from the origin, summed to exactly zero and was removed as degenerate.
  EBGEOMETRY_EXPECT(a_facet[0] < a_vertices.size());

  const Vec3 pivot        = a_vertices[a_facet[0]];
  Vec3       normal       = Vec3::zeros();
  T          longestEdge2 = T(0);

  for (size_t i = 0; i < N; i++) {
    EBGEOMETRY_EXPECT(a_facet[i] < a_vertices.size());
    EBGEOMETRY_EXPECT(a_facet[(i + 1) % N] < a_vertices.size());

    const Vec3 x0 = a_vertices[a_facet[i]] - pivot;
    const Vec3 x1 = a_vertices[a_facet[(i + 1) % N]] - pivot;

    normal += x0.cross(x1);
    longestEdge2 = Math::max(longestEdge2, (x1 - x0).length2());
  }

  return normal.length() <= T(64) * Math::Limits<T>::epsilon() * longestEdge2;
}

template <typename T>
inline std::vector<size_t>
Soup::removeDegeneratePolygons(const std::vector<EBGeometry::Vec3T<T>>& a_vertices,
                               std::vector<std::vector<size_t>>&        a_facets)
{
  static_assert(std::is_floating_point_v<T>, "Soup::removeDegeneratePolygons requires a floating-point T");

  using Edge = std::pair<size_t, size_t>;

  const size_t numFacets = a_facets.size();

  std::vector<bool> removed(numFacets, false);

  // Merge repeated consecutive vertices (compress() gives coincident vertices one index), in place: vertex i
  // is kept unless it equals the next one, and the last is compared with the facet's first as it was read.
  for (size_t f = 0; f < numFacets; f++) {
    std::vector<size_t>& facet = a_facets[f];

    const size_t n = facet.size();

    if (n > 0) {
      const size_t front = facet[0];

      size_t kept = 0;

      for (size_t i = 0; i < n; i++) {
        const size_t next = (i + 1 < n) ? facet[i + 1] : front;

        if (facet[i] != next) {
          facet[kept++] = facet[i];
        }
      }

      facet.resize(kept);
    }

    if (facet.size() < 3) {
      removed[f] = true;
    }
  }

  // Most soups have no zero-area facet left, and then the loop below changes nothing: without one, it needs
  // no table of edges either.
  bool hasZeroArea = false;

  for (size_t f = 0; f < numFacets && !hasZeroArea; f++) {
    hasZeroArea = !removed[f] && Soup::isZeroArea(a_vertices, a_facets[f]);
  }

  // Directed edge (u, v) -> the facet that contains it. Used to find the facet across a
  // T-junction filler's longest edge, which contains that edge reversed.
  std::map<Edge, size_t> facetOfEdge;

  for (size_t f = 0; f < numFacets && hasZeroArea; f++) {
    if (!removed[f]) {
      const std::vector<size_t>& facet = a_facets[f];

      for (size_t i = 0; i < facet.size(); i++) {
        facetOfEdge[Edge(facet[i], facet[(i + 1) % facet.size()])] = f;
      }
    }
  }

  for (size_t f = 0; f < numFacets && hasZeroArea; f++) {
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
  std::vector<size_t>              originalIndex;

  kept.reserve(numFacets);
  originalIndex.reserve(numFacets);

  for (size_t f = 0; f < numFacets; f++) {
    if (!removed[f]) {
      kept.emplace_back(std::move(a_facets[f]));
      originalIndex.push_back(f);
    }
  }

  a_facets = std::move(kept);

  return originalIndex;
}

inline std::string
Soup::findRepeatedVertex(const std::vector<std::vector<size_t>>& a_facets)
{
  // One buffer for every facet, rather than an allocation per facet.
  std::vector<size_t> sorted;

  for (size_t f = 0; f < a_facets.size(); f++) {
    sorted.assign(a_facets[f].begin(), a_facets[f].end());

    std::sort(sorted.begin(), sorted.end());

    if (std::adjacent_find(sorted.begin(), sorted.end()) != sorted.end()) {
      return "face " + std::to_string(f) + " visits the same vertex twice";
    }
  }

  return std::string();
}

inline std::string
Soup::findTopologyDefect(const std::vector<std::vector<size_t>>& a_facets)
{
  const std::string repeated = Soup::findRepeatedVertex(a_facets);

  if (!repeated.empty()) {
    return repeated;
  }

  // Which polygon first traversed each directed edge, in a hash table with open addressing (linear probing),
  // filled in the order of the polygons and their edges: the first edge found already there is the defect
  // reported. At most half full: a node-based map would allocate a node for every edge, at several times the cost.
  struct Slot
  {
    size_t from;
    size_t to;
    size_t facet; // Empty for a free slot.
  };

  constexpr size_t Empty = Math::Limits<size_t>::max();

  size_t numEdges = 0;

  for (const auto& facet : a_facets) {
    numEdges += facet.size();
  }

  size_t capacity = 16;

  while (capacity < 2 * numEdges) {
    capacity *= 2;
  }

  const size_t      mask = capacity - 1;
  std::vector<Slot> slots(capacity, Slot{0, 0, Empty});

  for (size_t f = 0; f < a_facets.size(); f++) {
    const std::vector<size_t>& facet = a_facets[f];

    for (size_t i = 0; i < facet.size(); i++) {
      const size_t from = facet[i];
      const size_t to   = facet[(i + 1) % facet.size()];

      uint64_t hash = uint64_t(from) * 0x9E3779B97F4A7C15ULL + uint64_t(to) * 0xC2B2AE3D27D4EB4FULL;

      hash ^= hash >> 29;

      size_t slot = size_t(hash) & mask;

      while (slots[slot].facet != Empty && (slots[slot].from != from || slots[slot].to != to)) {
        slot = (slot + 1) & mask;
      }

      if (slots[slot].facet != Empty) {
        return "faces " + std::to_string(slots[slot].facet) + " and " + std::to_string(f) + " both run from vertex " +
               std::to_string(from) + " to vertex " + std::to_string(to) +
               ": they are oriented inconsistently, or more than two faces share that edge";
      }

      slots[slot] = Slot{from, to, f};
    }
  }

  return std::string();
}

template <typename T>
inline std::string
Soup::findFoldedFeature(const EBGeometry::DCEL::MeshT<T>& a_mesh)
{
  for (uint32_t e = 0; e < a_mesh.numEdges(); e++) {
    const auto& edge = a_mesh.getEdge(e);

    if (edge.getNormal().length2() == T(0)) {
      return "the faces on either side of the edge from vertex " + std::to_string(edge.getVertexIndex()) +
             " to vertex " + std::to_string(edge.getNextEdge(a_mesh).getVertexIndex()) + " fold back onto each other";
    }
  }

  for (uint32_t v = 0; v < a_mesh.numVertices(); v++) {
    const auto& vertex = a_mesh.getVertex(v);

    if (vertex.getOutgoingEdgeIndex() != UINT32_MAX && vertex.getNormal().length2() == T(0)) {
      return "the faces around vertex " + std::to_string(v) + " fold back onto each other";
    }
  }

  return std::string();
}

template <typename T>
inline void
Soup::soupToDCEL(EBGeometry::DCEL::MeshT<T>&              a_mesh,
                 Pool&                                    a_pool,
                 const std::vector<EBGeometry::Vec3T<T>>& a_vertices,
                 const std::vector<std::vector<size_t>>&  a_facets,
                 const std::string&                       a_id)
{
  static_assert(std::is_floating_point_v<T>, "Soup::soupToDCEL requires a floating-point T");

  using Vec3   = EBGeometry::Vec3T<T>;
  using Vertex = EBGeometry::DCEL::VertexT<T>;
  using Edge   = EBGeometry::DCEL::EdgeT<T>;
  using Face   = EBGeometry::DCEL::FaceT<T>;

  // Upper bound on the half-edge count: every facet contributes one half-edge per vertex, even
  // ones later skipped below for having fewer than 3 vertices. PODVector capacity need not be
  // used exactly, so reserving this upper bound (rather than a second pass to compute the exact
  // count) is fine.
  size_t numEdgesUpperBound = 0;
  for (const auto& curFacet : a_facets) {
    numEdgesUpperBound += curFacet.size();
  }

  // The mesh numbers its vertices, half-edges and faces with uint32_t, and UINT32_MAX means "unset".
  EBGEOMETRY_REQUIRE(a_vertices.size() < UINT32_MAX && numEdgesUpperBound < UINT32_MAX && a_facets.size() < UINT32_MAX,
                     "Soup::soupToDCEL: '%s' has too many vertices (%zu), half-edges (%zu) or faces (%zu) for "
                     "32-bit indices",
                     a_id.c_str(),
                     a_vertices.size(),
                     numEdgesUpperBound,
                     a_facets.size());

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
  for (size_t ifacet = 0; ifacet < a_facets.size(); ifacet++) {
    const auto& curFacet = a_facets[ifacet];

    // The file readers remove such faces (Soup::removeDegeneratePolygons) before they get here.
    EBGEOMETRY_REQUIRE(curFacet.size() >= 3,
                       "Soup::soupToDCEL: face %zu of '%s' has %zu vertices, but a face needs at least 3",
                       ifacet,
                       a_id.c_str(),
                       curFacet.size());

    const uint32_t firstEdgeIndex = a_mesh.numEdges();
    const uint32_t numFaceEdges   = static_cast<uint32_t>(curFacet.size());

    // Build the half-edges for this polygon: one per vertex, appended contiguously.
    for (uint32_t i = 0; i < numFaceEdges; i++) {
      EBGEOMETRY_REQUIRE(curFacet[i] < a_vertices.size(),
                         "Soup::soupToDCEL: face %zu of '%s' names vertex %zu, but there are only %zu vertices",
                         ifacet,
                         a_id.c_str(),
                         curFacet[i],
                         a_vertices.size());

      const uint32_t vertexIndex = static_cast<uint32_t>(curFacet[i]);

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

  a_mesh.reconcile();
}

template <typename T>
inline void
Soup::reconcilePairEdgesDCEL(EBGeometry::DCEL::MeshT<T>& a_mesh)
{
  static_assert(std::is_floating_point_v<T>, "Soup::reconcilePairEdgesDCEL requires a floating-point T");

  using Edge = EBGeometry::DCEL::EdgeT<T>;

  const uint32_t numEdges    = a_mesh.numEdges();
  const uint32_t numVertices = a_mesh.numVertices();

  // Local, transient bookkeeping (NOT stored on VertexT, which keeps only a single outgoing-edge
  // index -- see its class-level note): which half-edges start at each vertex, so the search below
  // stays O(V + E) instead of an O(E^2) scan over every edge pair. Each edge's origin vertex and next
  // edge are checked in every build, since both loops index with them; soupToDCEL never breaks
  // either, a mesh assembled by hand can.
  // The lists are stored one after another, vertex v's in edgesStartingAtVertex[first[v]] to
  // edgesStartingAtVertex[first[v + 1] - 1] in increasing edge order, counted in one pass and filled in a
  // second, rather than a vector per vertex, which would allocate as it grows.
  std::vector<uint32_t> first(size_t(numVertices) + 1, 0);

  for (uint32_t i = 0; i < numEdges; i++) {
    const uint32_t v    = a_mesh.getEdge(i).getVertexIndex();
    const uint32_t next = a_mesh.getEdge(i).getNextEdgeIndex();

    EBGEOMETRY_REQUIRE(v < numVertices,
                       "Soup::reconcilePairEdgesDCEL: half-edge %u starts at vertex %u, but there are only %u vertices",
                       unsigned(i),
                       unsigned(v),
                       unsigned(numVertices));
    EBGEOMETRY_REQUIRE(next < numEdges,
                       "Soup::reconcilePairEdgesDCEL: half-edge %u has next edge %u, but there are only %u half-edges",
                       unsigned(i),
                       unsigned(next),
                       unsigned(numEdges));

    first[size_t(v) + 1]++;
  }

  for (uint32_t v = 0; v < numVertices; v++) {
    first[size_t(v) + 1] += first[v];
  }

  std::vector<uint32_t> edgesStartingAtVertex(numEdges);
  std::vector<uint32_t> fill(first.begin(), first.end() - 1);

  for (uint32_t i = 0; i < numEdges; i++) {
    edgesStartingAtVertex[fill[a_mesh.getEdge(i).getVertexIndex()]++] = i;
  }

  for (uint32_t curIndex = 0; curIndex < numEdges; curIndex++) {
    Edge& curEdge = a_mesh.getEdge(curIndex);

    const uint32_t nextIndex = curEdge.getNextEdgeIndex();

    const uint32_t vertexStart = curEdge.getVertexIndex();
    const uint32_t vertexEnd   = a_mesh.getEdge(nextIndex).getVertexIndex();

    // The pair edge starts where this edge ends, and ends where this edge starts.
    for (uint32_t cand = first[vertexEnd]; cand < first[size_t(vertexEnd) + 1]; cand++) {
      const uint32_t candIndex     = edgesStartingAtVertex[cand];
      Edge&          candEdge      = a_mesh.getEdge(candIndex);
      const uint32_t candNextIndex = candEdge.getNextEdgeIndex();

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
