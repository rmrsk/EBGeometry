// SPDX-FileCopyrightText: 2023 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file    EBGeometry_MeshDistanceFunctionsImplem.hpp
 * @brief   Implementation of EBGeometry_MeshDistanceFunctions.hpp
 * @author  Robert Marskar
 */

#ifndef EBGEOMETRY_MESHDISTANCEFUNCTIONSIMPLEM_HPP
#define EBGEOMETRY_MESHDISTANCEFUNCTIONSIMPLEM_HPP

// Std includes
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_Array.hpp"
#include "EBGeometry_DCEL_Edge.hpp"
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_DCEL_Vertex.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_MeshDistanceFunctions.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Internal helpers shared by MeshSDF and TriMeshSDF
 */
namespace MeshDistanceFunctionsDetail {

/**
 * @brief Abort with a message if a mesh distance function would be built from nothing.
 * @details Internal helper. Always on (EBGEOMETRY_REQUIRE): a Release build would otherwise
 * dereference a null bounding-volume list. The file readers throw rather than return an empty mesh,
 * so the usual cause is a mesh built by hand.
 * @param[in] a_who   Class being built, for the message.
 * @param[in] a_count Number of faces or triangles it was given.
 */
inline void
requireNonEmpty(const char* a_who, const size_t a_count) noexcept
{
  EBGEOMETRY_REQUIRE(
    a_count > 0, "%s: the mesh has no faces, so there is nothing to build a distance function from", a_who);
}

/**
 * @brief Extract every face of a DCEL mesh as flat, self-contained Triangles.
 * @details Internal helper shared by TriMeshSDF's mesh constructor and Parser::readIntoTriangles, so
 * both produce identical triangles. Each triangle carries the face normal, its three vertex positions
 * and vertex normals, its three edge normals (edge i runs from vertex i to vertex i+1), and the face
 * metadata.
 *
 * A face with more than three vertices is fan-triangulated. For a planar convex face this is exact:
 * a fan diagonal lies in the face plane, so its pseudonormal is the face normal, and the angles the
 * fan triangles subtend at each vertex add up to the polygon's angle there. The apex is the vertex
 * whose fan has the largest smallest triangle, so a face with straight-angle vertices (for example
 * one that received a T-junction vertex in Soup::removeDegeneratePolygons) yields no zero-area
 * triangles. A zero-area face has no interior and yields no triangles; its edges belong to its
 * neighbours as well.
 *
 * The face normal is set after the vertex positions, so a mesh flipped with MeshT::flip() keeps its
 * flipped normals (setting the positions recomputes the normal from vertex order).
 * @tparam T    Floating-point precision type.
 * @tparam Meta Face metadata type.
 * @param[in] a_mesh DCEL mesh.
 * @return The triangles of every face, in face order.
 */
template <class T, class Meta>
[[nodiscard]] EBGEOMETRY_HOST
inline std::vector<Triangle<T, Meta>>
extractTriangles(const DCEL::MeshT<T, Meta>& a_mesh)
{
  std::vector<Triangle<T, Meta>> triangles;

  triangles.reserve(a_mesh.numFaces());

  for (uint32_t faceIndex = 0; faceIndex < a_mesh.numFaces(); faceIndex++) {
    const auto& f             = a_mesh.getFace(faceIndex);
    const auto  vertexIndices = f.gatherVertexIndices(a_mesh);
    const auto  edgeIndices   = f.gatherEdgeIndices(a_mesh);
    const auto& faceNormal    = f.getNormal();
    const auto  N             = vertexIndices.size();

    EBGEOMETRY_EXPECT(N >= 3);
    EBGEOMETRY_EXPECT(edgeIndices.size() == N);

    if (faceNormal.length2() == T(0)) {
      continue;
    }

    const auto position = [&](size_t a_i) -> const Vec3T<T>& {
      return a_mesh.getVertex(vertexIndices[a_i % N]).getPosition();
    };

    // Pick the fan apex whose smallest fan triangle is largest (twice its area, via the cross
    // product). Any apex works for a strictly convex face.
    size_t apex = 0;

    if (N > 3) {
      T bestSmallest = T(-1);

      for (size_t a = 0; a < N; a++) {
        T smallest = Math::Limits<T>::max();

        for (size_t j = 1; j + 1 < N; j++) {
          const T area2 = (position(a + j) - position(a)).cross(position(a + j + 1) - position(a)).length();

          smallest = Math::min(smallest, area2);
        }

        if (smallest > bestSmallest) {
          bestSmallest = smallest;
          apex         = a;
        }
      }
    }

    for (size_t j = 1; j + 1 < N; j++) {
      const size_t i0 = apex;
      const size_t i1 = (apex + j) % N;
      const size_t i2 = (apex + j + 1) % N;

      // Polygon boundary edges keep their half-edge normals; fan diagonals lie in the face plane.
      const Vec3T<T> n01 = (j == 1) ? a_mesh.getEdge(edgeIndices[i0]).getNormal() : faceNormal;
      const Vec3T<T> n12 = a_mesh.getEdge(edgeIndices[i1]).getNormal();
      const Vec3T<T> n20 = (j + 2 == N) ? a_mesh.getEdge(edgeIndices[i2]).getNormal() : faceNormal;

      Triangle<T, Meta> tri;

      tri.setVertexPositions({position(i0), position(i1), position(i2)});
      tri.setNormal(faceNormal);
      tri.setVertexNormals({a_mesh.getVertex(vertexIndices[i0]).getNormal(),
                            a_mesh.getVertex(vertexIndices[i1]).getNormal(),
                            a_mesh.getVertex(vertexIndices[i2]).getNormal()});
      tri.setEdgeNormals({n01, n12, n20});
      tri.setMetaData(f.getMetaData());

      triangles.emplace_back(tri);
    }
  }

  return triangles;
}

} // namespace MeshDistanceFunctionsDetail

template <class T, class Meta>
EBGEOMETRY_HOST
inline FlatMeshSDF<T, Meta>::FlatMeshSDF(const Mesh& a_mesh, Pool& a_pool) noexcept : m_mesh(a_mesh)
{
  EBGEOMETRY_REQUIRE(a_mesh.isAttachedTo(a_pool), "FlatMeshSDF: the mesh must live in the pool passed in");

  // The mesh descriptor copied above resolves through a_pool, so a_pool must outlive this object.
  // Taking it by reference is what makes that requirement visible at the call site (and checkable
  // above); nothing else is done with it.
  (void)a_pool;
}

template <class T, class Meta>
EBGEOMETRY_HOST_DEVICE
inline T
FlatMeshSDF<T, Meta>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  return m_mesh.signedDistance(a_point);
}

template <class T, class Meta>
EBGEOMETRY_HOST
inline FlatMeshSDF<T, Meta>
FlatMeshSDF<T, Meta>::rebasedView(const Pool& a_pool) const noexcept
{
  FlatMeshSDF view = *this;

  view.m_mesh = m_mesh.rebasedView(a_pool);

  return view;
}

template <class T, class Meta>
EBGEOMETRY_HOST_DEVICE
inline FlatMeshSDF<T, Meta>
FlatMeshSDF<T, Meta>::relocatedTo(const PoolLocation& a_location) const noexcept
{
  FlatMeshSDF view = *this;

  view.m_mesh = m_mesh.relocatedTo(a_location);

  return view;
}

template <class T, class Meta>
EBGEOMETRY_HOST
inline FlatMeshSDF<T, Meta>
FlatMeshSDF<T, Meta>::deepCopy(Pool& a_dstPool) const
{
  FlatMeshSDF copy = *this;

  copy.m_mesh = m_mesh.deepCopy(a_dstPool);

  return copy;
}

template <class T, class Meta>
EBGEOMETRY_HOST_DEVICE
inline EBGeometry::BoundingVolumes::AABBT<T>
FlatMeshSDF<T, Meta>::computeBoundingVolume() const noexcept
{
  Vec3T<T> lo = +Vec3T<T>::max();
  Vec3T<T> hi = -Vec3T<T>::max();

  for (uint32_t i = 0; i < m_mesh.numVertices(); i++) {
    const Vec3T<T>& x = m_mesh.getVertex(i).getPosition();

    lo = min(lo, x);
    hi = max(hi, x);
  }

  return EBGeometry::BoundingVolumes::AABBT<T>(lo, hi);
}

template <class T, class Meta, size_t K>
EBGEOMETRY_HOST
inline MeshSDF<T, Meta, K>::MeshSDF(const Mesh& a_mesh, Pool& a_pool, const BVH::BuildSpec& a_spec)
  : m_mesh(a_mesh), m_bvh(MeshSDF::buildBVH(a_mesh, a_pool, a_spec))
{
  // The mesh and the BVH must share one pool: rebasedView() rebases both onto the same mirror, and
  // the BVH's faces index the mesh's arrays. a_pool must outlive this object and every copy of it.
  EBGEOMETRY_REQUIRE(a_mesh.isAttachedTo(a_pool), "MeshSDF: the mesh must live in the pool passed in");
  EBGEOMETRY_EXPECT(m_bvh.isAttachedTo(a_pool));
}

template <class T, class Meta, size_t K>
EBGEOMETRY_HOST
inline typename MeshSDF<T, Meta, K>::Root
MeshSDF<T, Meta, K>::buildBVH(const Mesh& a_mesh, Pool& a_pool, const BVH::BuildSpec& a_spec)
{
  using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

  MeshDistanceFunctionsDetail::requireNonEmpty("MeshSDF", a_mesh.numFaces());

  // Each face is stored by value. Its topology indices stay valid, since they resolve against the
  // same mesh (m_mesh) both before and after the copy.
  std::vector<std::pair<Face, AABB>> facesAndBVs;

  facesAndBVs.reserve(a_mesh.numFaces());

  for (uint32_t i = 0; i < a_mesh.numFaces(); i++) {
    const Face& face = a_mesh.getFace(i);

    facesAndBVs.emplace_back(face, AABB(face.getAllVertexCoordinates(a_mesh)));
  }

  return Root(a_pool, std::move(facesAndBVs), a_spec);
}

template <class T, class Meta, size_t K>
EBGEOMETRY_HOST_DEVICE
inline T
MeshSDF<T, Meta, K>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  T           minDist = Math::Limits<T>::max();
  const auto  faces   = m_bvh.getPrimitives();
  const auto& mesh    = m_mesh;

  const auto evalLeaf = [&faces, &a_point, &mesh](T& a_state, size_t a_offset, size_t a_count) noexcept {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      const T curDist = faces[static_cast<uint32_t>(i)].signedDistance(a_point, mesh);

      EBGEOMETRY_EXPECT(!std::isnan(curDist));

      a_state = std::abs(curDist) < std::abs(a_state) ? curDist : a_state;
    }
  };

  const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state * a_state; };

  m_bvh.pruneTraverse(a_point, minDist, evalLeaf, pruneDist2);

  return minDist;
}

template <class T, class Meta, size_t K>
EBGEOMETRY_HOST
inline std::vector<std::pair<uint32_t, T>>
MeshSDF<T, Meta, K>::getClosestFaces(const Vec3T<T>& a_point, const bool a_sorted) const
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  const auto& mesh = m_mesh;

  using FaceAndDist = std::pair<uint32_t, T>;

  // Candidate faces, named by their index into this BVH's own primitive array (getPrimitives()),
  // not by an index into the source mesh: packing reorders primitives into leaf order and stores
  // them by value, so nothing records where a given face came from in the mesh.
  std::vector<FaceAndDist> candidateFaces;

  // Keep every face that is at least as close as every face visited before it. The pruning bound is
  // the closest distance so far, so a box farther than that is skipped.
  struct State
  {
    std::vector<FaceAndDist>& candidates;
    T                         shortest;
  };

  State      state{candidateFaces, Math::Limits<T>::max()};
  const auto faces = m_bvh.getPrimitives();

  // Not noexcept: collecting candidates allocates. pruneTraverse itself is noexcept, so an allocation
  // failure here terminates, as it would anywhere else in a query.
  const auto evalLeaf = [&faces, &a_point, &mesh](State& a_state, size_t a_offset, size_t a_count) {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      const T distToFace = std::sqrt(faces[static_cast<uint32_t>(i)].unsignedDistance2(a_point, mesh));

      EBGEOMETRY_EXPECT(!std::isnan(distToFace));

      if (distToFace <= a_state.shortest) {
        a_state.candidates.emplace_back(static_cast<uint32_t>(i), distToFace);
        a_state.shortest = distToFace;
      }
    }
  };

  const auto pruneDist2 = [](const State& a_state) noexcept -> T {
    return (a_state.shortest < Math::Limits<T>::max()) ? a_state.shortest * a_state.shortest
                                                       : Math::Limits<T>::infinity();
  };

  m_bvh.pruneTraverse(a_point, state, evalLeaf, pruneDist2);

  if (a_sorted) {
    std::sort(candidateFaces.begin(), candidateFaces.end(), [](const FaceAndDist& a, const FaceAndDist& b) {
      return a.second < b.second;
    });
  }

  return candidateFaces;
}

template <class T, class Meta, size_t K>
EBGEOMETRY_HOST
inline MeshSDF<T, Meta, K>
MeshSDF<T, Meta, K>::rebasedView(const Pool& a_pool) const noexcept
{
  MeshSDF view = *this;

  view.m_mesh = m_mesh.rebasedView(a_pool);
  view.m_bvh  = m_bvh.rebasedView(a_pool);

  return view;
}

template <class T, class Meta, size_t K>
EBGEOMETRY_HOST_DEVICE
inline MeshSDF<T, Meta, K>
MeshSDF<T, Meta, K>::relocatedTo(const PoolLocation& a_location) const noexcept
{
  MeshSDF view = *this;

  view.m_mesh = m_mesh.relocatedTo(a_location);
  view.m_bvh  = m_bvh.relocatedTo(a_location);

  return view;
}

template <class T, class Meta, size_t K>
EBGEOMETRY_HOST
inline MeshSDF<T, Meta, K>
MeshSDF<T, Meta, K>::deepCopy(Pool& a_dstPool) const
{
  MeshSDF copy = *this;

  copy.m_mesh = m_mesh.deepCopy(a_dstPool);
  copy.m_bvh  = m_bvh.deepCopy(a_dstPool);

  return copy;
}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST
inline std::vector<typename TriMeshSDF<T, Meta, K, W>::Tri>
TriMeshSDF<T, Meta, K, W>::extractTriangles(const Mesh& a_mesh)
{
  std::vector<Tri> triangles = EBGeometry::MeshDistanceFunctionsDetail::extractTriangles(a_mesh);

  return triangles;
}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST
inline typename TriMeshSDF<T, Meta, K, W>::Root
TriMeshSDF<T, Meta, K, W>::buildBVH(const std::vector<Tri>& a_triangles, Pool& a_pool, const BVH::BuildSpec& a_spec)
{
  using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

  MeshDistanceFunctionsDetail::requireNonEmpty("TriMeshSDF", a_triangles.size());

  std::vector<AABB> boxes;

  boxes.reserve(a_triangles.size());

  for (const auto& tri : a_triangles) {
    const auto& x = tri.getVertexPositions();

    boxes.emplace_back(min(x[0], min(x[1], x[2])), max(x[0], max(x[1], x[2])));
  }

  // Each leaf's triangles become their own SoA groups of W, the last one padded.
  const auto packLeaf = [&a_triangles](const uint32_t* a_items, uint32_t a_count, std::vector<TriAoSoA>& a_out) {
    constexpr uint32_t width = static_cast<uint32_t>(W);

    for (uint32_t first = 0; first < a_count; first += width) {
      const uint32_t groupCount = Math::min(width, a_count - first);

      Array<Tri, W> group;

      for (uint32_t i = 0; i < groupCount; i++) {
        group[i] = a_triangles[a_items[first + i]];
      }

      TriAoSoA soa;

      soa.pack(group.data(), groupCount);

      a_out.push_back(soa);
    }
  };

  return Root(a_pool, BVH::buildTopology<T, K>(boxes, a_spec), packLeaf);
}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST
inline TriMeshSDF<T, Meta, K, W>::TriMeshSDF(const Mesh& a_mesh, Pool& a_pool, const BVH::BuildSpec& a_spec)
  : TriMeshSDF(TriMeshSDF::extractTriangles(a_mesh), a_pool, a_spec)
{}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST
inline TriMeshSDF<T, Meta, K, W>::TriMeshSDF(const std::vector<Tri>& a_triangles,
                                             Pool&                   a_pool,
                                             const BVH::BuildSpec&   a_spec)
  : m_bvh(TriMeshSDF::buildBVH(a_triangles, a_pool, a_spec))
{}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline T
TriMeshSDF<T, Meta, K, W>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  T          minDist = Math::Limits<T>::max();
  const auto groups  = m_bvh.getPrimitives();

  const auto evalLeaf = [&groups, &a_point](T& a_state, size_t a_offset, size_t a_count) noexcept {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      const T d = groups[static_cast<uint32_t>(i)].signedDistance(a_point);

      EBGEOMETRY_EXPECT(!std::isnan(d));

      if (std::abs(d) < std::abs(a_state)) {
        a_state = d;
      }
    }
  };

  const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state * a_state; };

  m_bvh.pruneTraverse(a_point, minDist, evalLeaf, pruneDist2);

  return minDist;
}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline typename TriMeshSDF<T, Meta, K, W>::ClosestTriangle
TriMeshSDF<T, Meta, K, W>::getClosestTriangle(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  // Same SIMD-pruned traversal as signedDistance(), but the running state also carries the winning
  // triangle's metadata: each visited leaf group reports both its closest signed distance and that
  // triangle's Meta via TriangleAoSoA::signedDistance(point, Meta&). The pruning bound is still the
  // squared running distance, so node pruning is identical to signedDistance()'s.
  ClosestTriangle closest;

  const auto groups = m_bvh.getPrimitives();

  const auto evalLeaf = [&groups, &a_point](ClosestTriangle& a_state, size_t a_offset, size_t a_count) noexcept {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      Meta    groupMeta{};
      const T d = groups[static_cast<uint32_t>(i)].signedDistance(a_point, groupMeta);

      EBGEOMETRY_EXPECT(!std::isnan(d));

      if (std::abs(d) < std::abs(a_state.signedDistance)) {
        a_state.signedDistance = d;
        a_state.metaData       = groupMeta;
      }
    }
  };

  const auto pruneDist2 = [](const ClosestTriangle& a_state) noexcept -> T {
    return a_state.signedDistance * a_state.signedDistance;
  };

  m_bvh.pruneTraverse(a_point, closest, evalLeaf, pruneDist2);

  return closest;
}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST
inline TriMeshSDF<T, Meta, K, W>
TriMeshSDF<T, Meta, K, W>::rebasedView(const Pool& a_pool) const noexcept
{
  TriMeshSDF view = *this;

  view.m_bvh = m_bvh.rebasedView(a_pool);

  return view;
}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline TriMeshSDF<T, Meta, K, W>
TriMeshSDF<T, Meta, K, W>::relocatedTo(const PoolLocation& a_location) const noexcept
{
  TriMeshSDF view = *this;

  view.m_bvh = m_bvh.relocatedTo(a_location);

  return view;
}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST
inline TriMeshSDF<T, Meta, K, W>
TriMeshSDF<T, Meta, K, W>::deepCopy(Pool& a_dstPool) const
{
  TriMeshSDF copy = *this;

  copy.m_bvh = m_bvh.deepCopy(a_dstPool);

  return copy;
}

} // namespace EBGeometry

#endif
