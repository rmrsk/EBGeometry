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
 * @brief Partition a TreeBVH with one of the preset methods.
 * @details Internal helper shared by MeshSDF and TriMeshSDF. ClusterSAH has no TreeBVH form, so
 * callers build it directly as a PackedBVH and never pass it here. Any other value outside
 * BVH::Construction aborts, in every build: carrying on would pack an unpartitioned tree.
 * @tparam T  Floating-point precision.
 * @tparam P  Primitive type.
 * @tparam BV Bounding-volume type.
 * @tparam K  BVH branching factor.
 * @param[in,out] a_tree         Root of the tree to partition.
 * @param[in]     a_construction Preset construction method.
 * @param[in]     a_options      Leaf-size settings: the top-down methods stop at a_options.maxLeafSize
 * primitives per leaf, and the space-filling-curve methods aim for a_options.targetLeafSize.
 * @param[in]     a_who          Class being built, for the message.
 */
template <class T, class P, class BV, size_t K>
inline void
partitionTree(BVH::TreeBVH<T, P, BV, K>&      a_tree,
              const BVH::Construction         a_construction,
              const BVH::ConstructionOptions& a_options,
              const char*                     a_who)
{
  BVH::detail::requireLeafSetting(a_who, a_construction, a_options);

  const size_t maxLeafSize = a_options.maxLeafSize;

  const typename BVH::TreeBVH<T, P, BV, K>::LeafPredicate stopCrit =
    [maxLeafSize](const BVH::TreeBVH<T, P, BV, K>& a_node) noexcept -> bool {
    return a_node.getPrimitives().size() <= maxLeafSize;
  };

  switch (a_construction) {
  case BVH::Construction::CentroidSplit: {
    a_tree.topDownSortAndPartition(BVH::BVCentroidPartitioner<T, P, BV, K>, stopCrit);

    return;
  }
  case BVH::Construction::MidpointSplit: {
    a_tree.topDownSortAndPartition(BVH::MidpointPartitioner<T, P, BV, K>, stopCrit);

    return;
  }
  case BVH::Construction::SAH: {
    a_tree.topDownSortAndPartition(BVH::BinnedSAHPartitioner<T, P, BV, K>, stopCrit);

    return;
  }
  case BVH::Construction::Morton: {
    a_tree.template bottomUpSortAndPartition<SFC::Morton>(a_options.targetLeafSize);

    return;
  }
  case BVH::Construction::Nested: {
    a_tree.template bottomUpSortAndPartition<SFC::Nested>(a_options.targetLeafSize);

    return;
  }
  case BVH::Construction::Hilbert: {
    a_tree.template bottomUpSortAndPartition<SFC::Hilbert>(a_options.targetLeafSize);

    return;
  }
  case BVH::Construction::ClusterSAH:
  default: {
    break;
  }
  }

  EBGEOMETRY_EXPECT(a_construction != BVH::Construction::ClusterSAH);
  EBGEOMETRY_REQUIRE(false, "%s: unknown BVH::Construction value (%d)", a_who, static_cast<int>(a_construction));
}

/**
 * @brief Pair every face id of a DCEL mesh with the face's bounding volume.
 * @details Internal helper; not part of the public API.
 * @tparam T  Floating-point precision type.
 * @tparam BV Bounding-volume type (e.g. AABBT<T>).
 * @param[in] a_mesh Input DCEL mesh.
 * @return One (face id, bounding volume) pair per face, in face order.
 */
template <class T, class BV>
[[nodiscard]] EBGEOMETRY_HOST
inline std::vector<std::pair<uint32_t, BV>>
faceIdsAndBVs(const EBGeometry::DCEL::MeshT<T>& a_mesh)
{
  std::vector<std::pair<uint32_t, BV>> idsAndBVs;

  idsAndBVs.reserve(a_mesh.numFaces());

  for (uint32_t i = 0; i < a_mesh.numFaces(); i++) {
    idsAndBVs.emplace_back(i, BV(a_mesh.getFace(i).getAllVertexCoordinates(a_mesh)));
  }

  return idsAndBVs;
}

/**
 * @brief Build a tree BVH over the face ids of a DCEL mesh.
 * @details Internal helper; not part of the public API.
 * @tparam T  Floating-point precision type.
 * @tparam BV Bounding-volume type (e.g. AABBT<T>).
 * @tparam K  BVH branching factor (number of children per node).
 * @param[in] a_dcelMesh      Input DCEL mesh.
 * @param[in] a_construction  Preset construction method, any but ClusterSAH.
 * @param[in] a_options       Leaf-size settings; see partitionTree().
 * @return Shared pointer to the root of the resulting tree BVH.
 */
template <class T, class BV, size_t K>
[[nodiscard]] std::shared_ptr<EBGeometry::BVH::TreeBVH<T, uint32_t, BV, K>>
buildDCELTreeBVH(const EBGeometry::DCEL::MeshT<T>& a_dcelMesh,
                 const BVH::Construction           a_construction,
                 const BVH::ConstructionOptions&   a_options)
{
  static_assert(std::is_floating_point_v<T>,
                "MeshDistanceFunctionsDetail::buildDCELTreeBVH requires a floating-point type T");
  static_assert(K >= 2, "MeshDistanceFunctionsDetail::buildDCELTreeBVH: branching factor K must be at least 2");

  requireNonEmpty("MeshSDF", a_dcelMesh.numFaces());

  using Prim = uint32_t;

  // TreeBVH is the host-side builder and holds its primitives by shared_ptr, so each face id is
  // wrapped in one here.
  std::vector<std::pair<std::shared_ptr<const Prim>, BV>> primsAndBVs;

  primsAndBVs.reserve(a_dcelMesh.numFaces());

  for (auto& [id, bv] : faceIdsAndBVs<T, BV>(a_dcelMesh)) {
    primsAndBVs.emplace_back(std::make_shared<const Prim>(id), bv);
  }

  using Tree = EBGeometry::BVH::TreeBVH<T, Prim, BV, K>;

  auto bvh = std::make_shared<Tree>(primsAndBVs);

  partitionTree<T, Prim, BV, K>(*bvh, a_construction, a_options, "MeshSDF");

  return bvh;
}

/**
 * @brief Extract every face of a DCEL mesh as flat, self-contained Triangles.
 * @details Internal helper shared by TriMeshSDF's mesh constructor and Parser::readIntoTriangles, so
 * both produce identical triangles. Each triangle carries the face normal, its three vertex positions
 * and vertex normals, its three edge normals (edge i runs from vertex i to vertex i+1), and the id
 * of the face it was cut from (its index in the mesh).
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
 * @param[in] a_mesh DCEL mesh.
 * @return The triangles of every face, in face order.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST
inline std::vector<Triangle<T>>
extractTriangles(const DCEL::MeshT<T>& a_mesh)
{
  std::vector<Triangle<T>> triangles;

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

      Triangle<T> tri;

      tri.setVertexPositions({position(i0), position(i1), position(i2)});
      tri.setNormal(faceNormal);
      tri.setVertexNormals({a_mesh.getVertex(vertexIndices[i0]).getNormal(),
                            a_mesh.getVertex(vertexIndices[i1]).getNormal(),
                            a_mesh.getVertex(vertexIndices[i2]).getNormal()});
      tri.setEdgeNormals({n01, n12, n20});
      tri.setFaceId(faceIndex);

      triangles.emplace_back(tri);
    }
  }

  return triangles;
}

/**
 * @brief Build a tree BVH from a flat triangle soup.
 * @details Internal helper; not part of the public API. Creates one BV per triangle from its
 * vertex positions, then builds a K-ary tree BVH according to a_construction and a_options.
 * @tparam T    Floating-point precision type.
 * @tparam BV   Bounding-volume type (e.g. AABBT<T>).
 * @tparam K    BVH branching factor (number of children per internal node).
 * @param[in] a_triangles   Triangle soup to build the BVH over.
 * @param[in] a_construction       Preset construction method, any but ClusterSAH.
 * @param[in] a_options       Leaf-size settings, in triangles; see partitionTree().
 * @return Shared pointer to the root of the resulting tree BVH.
 */
template <class T, class BV, size_t K>
std::shared_ptr<EBGeometry::BVH::TreeBVH<T, Triangle<T>, BV, K>>
buildTriTreeBVH(const std::vector<EBGeometry::Triangle<T>>& a_triangles,
                const BVH::Construction                     a_construction,
                const BVH::ConstructionOptions&             a_options)
{
  static_assert(std::is_floating_point_v<T>,
                "MeshDistanceFunctionsDetail::buildTriTreeBVH requires a floating-point type T");
  static_assert(K >= 2, "MeshDistanceFunctionsDetail::buildTriTreeBVH: branching factor K must be at least 2");

  requireNonEmpty("TriMeshSDF", a_triangles.size());

  using Prim          = EBGeometry::Triangle<T>;
  using PrimAndBVList = std::vector<std::pair<std::shared_ptr<const Prim>, BV>>;

  // Create a pair-wise list of triangles and their bounding volumes. TreeBVH is the host-side
  // builder and holds its primitives by shared_ptr, so each triangle is copied into one here.
  PrimAndBVList primsAndBVs;

  primsAndBVs.reserve(a_triangles.size());

  for (const auto& tri : a_triangles) {
    const auto& vertexPositions = tri.getVertexPositions();

    const std::vector<EBGeometry::Vec3T<T>> vertices{vertexPositions[0], vertexPositions[1], vertexPositions[2]};

    primsAndBVs.emplace_back(std::make_pair(std::make_shared<const Prim>(tri), BV(vertices)));
  }

  using Tree = EBGeometry::BVH::TreeBVH<T, Prim, BV, K>;

  auto bvh = std::make_shared<Tree>(primsAndBVs);

  partitionTree<T, Prim, BV, K>(*bvh, a_construction, a_options, "TriMeshSDF");

  return bvh;
}

} // namespace MeshDistanceFunctionsDetail

template <class T>
EBGEOMETRY_HOST
inline FlatMeshSDF<T>::FlatMeshSDF(const Mesh& a_mesh, Pool& a_pool) noexcept : m_mesh(a_mesh)
{
  EBGEOMETRY_REQUIRE(a_mesh.isAttachedTo(a_pool), "FlatMeshSDF: the mesh must live in the pool passed in");

  // The mesh descriptor copied above resolves through a_pool, so a_pool must outlive this object.
  // Taking it by reference is what makes that requirement visible at the call site (and checkable
  // above); nothing else is done with it.
  (void)a_pool;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline T
FlatMeshSDF<T>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  return m_mesh.signedDistance(a_point);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline ClosestFace<T>
FlatMeshSDF<T>::getClosestFace(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  // The same search as the mesh's default signedDistance() (SearchAlgorithm::Direct2): the face with
  // the smallest unsigned distance wins, and its signed distance is the result, so the two agree
  // exactly.
  ClosestFace<T> closest;

  T minDist2 = Math::Limits<T>::max();

  for (uint32_t i = 0; i < m_mesh.numFaces(); i++) {
    const T curDist2 = m_mesh.getFace(i).unsignedDistance2(a_point, m_mesh);

    if (curDist2 < minDist2 || closest.faceId == UINT32_MAX) {
      minDist2       = curDist2;
      closest.faceId = i;
    }
  }

  if (closest.faceId != UINT32_MAX) {
    closest.signedDistance = m_mesh.getFace(closest.faceId).signedDistance(a_point, m_mesh);
  }

  return closest;
}

template <class T>
EBGEOMETRY_HOST
inline FlatMeshSDF<T>
FlatMeshSDF<T>::rebasedView(const Pool& a_pool) const noexcept
{
  FlatMeshSDF view = *this;

  view.m_mesh = m_mesh.rebasedView(a_pool);

  return view;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline FlatMeshSDF<T>
FlatMeshSDF<T>::relocatedTo(const PoolLocation& a_location) const noexcept
{
  FlatMeshSDF view = *this;

  view.m_mesh = m_mesh.relocatedTo(a_location);

  return view;
}

template <class T>
EBGEOMETRY_HOST
inline FlatMeshSDF<T>
FlatMeshSDF<T>::deepCopy(Pool& a_dstPool) const
{
  FlatMeshSDF copy = *this;

  copy.m_mesh = m_mesh.deepCopy(a_dstPool);

  return copy;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline EBGeometry::BoundingVolumes::AABBT<T>
FlatMeshSDF<T>::computeBoundingVolume() const noexcept
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

template <class T, size_t K>
EBGEOMETRY_HOST
inline BVH::ConstructionOptions
MeshSDF<T, K>::defaultConstructionOptions() noexcept
{
  return BVH::ConstructionOptions{K - 1, 1, BVH::ClusterSpec{}};
}

template <class T, size_t K>
EBGEOMETRY_HOST
inline MeshSDF<T, K>::MeshSDF(const Mesh& a_mesh, Pool& a_pool, const BVH::Construction a_construction)
  : MeshSDF(a_mesh, a_pool, a_construction, MeshSDF::defaultConstructionOptions())
{}

template <class T, size_t K>
EBGEOMETRY_HOST
inline MeshSDF<T, K>::MeshSDF(const Mesh&                     a_mesh,
                              Pool&                           a_pool,
                              const BVH::Construction         a_construction,
                              const BVH::ConstructionOptions& a_options)
  : m_mesh(a_mesh), m_bvh(MeshSDF::buildBVH(a_mesh, a_pool, a_construction, a_options))
{
  // The mesh and the BVH must share one pool: rebasedView() rebases both onto the same mirror, and
  // the BVH's faces index the mesh's arrays. a_pool must outlive this object and every copy of it.
  EBGEOMETRY_REQUIRE(a_mesh.isAttachedTo(a_pool), "MeshSDF: the mesh must live in the pool passed in");
  EBGEOMETRY_EXPECT(m_bvh.isAttachedTo(a_pool));
}

template <class T, size_t K>
EBGEOMETRY_HOST
inline typename MeshSDF<T, K>::Root
MeshSDF<T, K>::buildBVH(const Mesh&                     a_mesh,
                        Pool&                           a_pool,
                        const BVH::Construction         a_construction,
                        const BVH::ConstructionOptions& a_options)
{
  using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

  // ClusterSAH builds a PackedBVH directly, with no TreeBVH.
  if (a_construction == BVH::Construction::ClusterSAH) {
    MeshDistanceFunctionsDetail::requireNonEmpty("MeshSDF", a_mesh.numFaces());

    return Root(a_pool, MeshDistanceFunctionsDetail::faceIdsAndBVs<T, AABB>(a_mesh), a_options.cluster);
  }

  // pack() still returns a shared_ptr; the PackedBVH it points to is a descriptor into a_pool, so
  // copying it out is all that is needed.
  return *EBGeometry::MeshDistanceFunctionsDetail::buildDCELTreeBVH<T, AABB, K>(a_mesh, a_construction, a_options)
            ->pack(a_pool);
}

template <class T, size_t K>
EBGEOMETRY_HOST_DEVICE
inline T
MeshSDF<T, K>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  T           minDist = Math::Limits<T>::max();
  const auto  faceIds = m_bvh.getPrimitives();
  const auto& mesh    = m_mesh;

  const auto evalLeaf = [&faceIds, &a_point, &mesh](T& a_state, size_t a_offset, size_t a_count) noexcept {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      const T curDist = mesh.getFace(faceIds[static_cast<uint32_t>(i)]).signedDistance(a_point, mesh);

      EBGEOMETRY_EXPECT(!std::isnan(curDist));

      a_state = std::abs(curDist) < std::abs(a_state) ? curDist : a_state;
    }
  };

  const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state * a_state; };

  m_bvh.pruneTraverse(a_point, minDist, evalLeaf, pruneDist2);

  return minDist;
}

template <class T, size_t K>
EBGEOMETRY_HOST_DEVICE
inline ClosestFace<T>
MeshSDF<T, K>::getClosestFace(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  // Same traversal as signedDistance(); the running state also carries the winning face's id, and
  // the pruning bound is still the squared running distance.
  ClosestFace<T> closest;

  const auto  faceIds = m_bvh.getPrimitives();
  const auto& mesh    = m_mesh;

  const auto evalLeaf = [&faceIds, &a_point, &mesh](ClosestFace<T>& a_state, size_t a_offset, size_t a_count) noexcept {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      const uint32_t faceId = faceIds[static_cast<uint32_t>(i)];
      const T        d      = mesh.getFace(faceId).signedDistance(a_point, mesh);

      EBGEOMETRY_EXPECT(!std::isnan(d));

      if (std::abs(d) < std::abs(a_state.signedDistance)) {
        a_state.signedDistance = d;
        a_state.faceId         = faceId;
      }
    }
  };

  const auto pruneDist2 = [](const ClosestFace<T>& a_state) noexcept -> T {
    return a_state.signedDistance * a_state.signedDistance;
  };

  m_bvh.pruneTraverse(a_point, closest, evalLeaf, pruneDist2);

  return closest;
}

template <class T, size_t K>
EBGEOMETRY_HOST
inline MeshSDF<T, K>
MeshSDF<T, K>::rebasedView(const Pool& a_pool) const noexcept
{
  MeshSDF view = *this;

  view.m_mesh = m_mesh.rebasedView(a_pool);
  view.m_bvh  = m_bvh.rebasedView(a_pool);

  return view;
}

template <class T, size_t K>
EBGEOMETRY_HOST_DEVICE
inline MeshSDF<T, K>
MeshSDF<T, K>::relocatedTo(const PoolLocation& a_location) const noexcept
{
  MeshSDF view = *this;

  view.m_mesh = m_mesh.relocatedTo(a_location);
  view.m_bvh  = m_bvh.relocatedTo(a_location);

  return view;
}

template <class T, size_t K>
EBGEOMETRY_HOST
inline MeshSDF<T, K>
MeshSDF<T, K>::deepCopy(Pool& a_dstPool) const
{
  MeshSDF copy = *this;

  copy.m_mesh = m_mesh.deepCopy(a_dstPool);
  copy.m_bvh  = m_bvh.deepCopy(a_dstPool);

  return copy;
}

template <class T, size_t K, size_t W>
template <class GetTriangle>
EBGEOMETRY_HOST
inline std::vector<typename TriMeshSDF<T, K, W>::TriAoSoA>
TriMeshSDF<T, K, W>::groupTriangles(const GetTriangle& a_getTriangle, uint32_t a_count)
{
  constexpr uint32_t soaWidth = static_cast<uint32_t>(W);

  std::vector<TriAoSoA> groups;

  const uint32_t numGroups = (a_count + soaWidth - 1U) / soaWidth;
  groups.reserve(numGroups);

  for (uint32_t group = 0; group < numGroups; group++) {
    const uint32_t groupOffset = group * soaWidth;
    const uint32_t groupCount  = Math::min(soaWidth, a_count - groupOffset);

    Array<Tri, W> trisArr;
    for (uint32_t i = 0; i < groupCount; i++) {
      trisArr[i] = a_getTriangle(groupOffset + i);
    }

    TriAoSoA soa;
    soa.pack(trisArr.data(), groupCount);
    groups.push_back(std::move(soa));
  }

  return groups;
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST
inline std::vector<typename TriMeshSDF<T, K, W>::TriAoSoA>
TriMeshSDF<T, K, W>::groupTrianglesIntoSoA(const std::vector<std::shared_ptr<const Tri>>& a_triangles,
                                           uint32_t                                       a_offset,
                                           uint32_t                                       a_count)
{
  const auto getTriangle = [&a_triangles, a_offset](uint32_t a_i) -> const Tri& {
    EBGEOMETRY_EXPECT(a_triangles[a_offset + a_i] != nullptr);

    return *a_triangles[a_offset + a_i];
  };

  return TriMeshSDF::groupTriangles(getTriangle, a_count);
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST
inline std::vector<typename TriMeshSDF<T, K, W>::Tri>
TriMeshSDF<T, K, W>::extractTriangles(const Mesh& a_mesh)
{
  std::vector<Tri> triangles = EBGeometry::MeshDistanceFunctionsDetail::extractTriangles(a_mesh);

  return triangles;
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST
inline typename TriMeshSDF<T, K, W>::Root
TriMeshSDF<T, K, W>::buildBVH(const std::vector<Tri>&         a_triangles,
                              Pool&                           a_pool,
                              const BVH::Construction         a_construction,
                              const BVH::ConstructionOptions& a_options)
{
  MeshDistanceFunctionsDetail::requireNonEmpty("TriMeshSDF", a_triangles.size());

  using AABB      = EBGeometry::BoundingVolumes::AABBT<T>;
  using Converter = decltype(&TriMeshSDF::groupTrianglesIntoSoA);

  if (a_construction == BVH::Construction::ClusterSAH) {
    return TriMeshSDF::buildClusterSAH(a_triangles, a_pool, a_options.cluster);
  }

  // packWith() still returns a shared_ptr; the PackedBVH it points to is a descriptor into a_pool,
  // so copying it out is all that is needed.
  return *EBGeometry::MeshDistanceFunctionsDetail::buildTriTreeBVH<T, AABB, K>(a_triangles, a_construction, a_options)
            ->template packWith<TriAoSoA, Converter>(a_pool, &TriMeshSDF::groupTrianglesIntoSoA);
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST
inline typename TriMeshSDF<T, K, W>::Root
TriMeshSDF<T, K, W>::buildClusterSAH(const std::vector<Tri>& a_triangles, Pool& a_pool, const BVH::ClusterSpec& a_spec)
{
  using AABB    = EBGeometry::BoundingVolumes::AABBT<T>;
  using ByIndex = EBGeometry::BVH::PackedBVH<T, uint32_t, K>;

  std::vector<std::pair<uint32_t, AABB>> indicesAndBVs;

  indicesAndBVs.reserve(a_triangles.size());

  for (uint32_t i = 0; i < static_cast<uint32_t>(a_triangles.size()); i++) {
    const auto& v = a_triangles[i].getVertexPositions();

    indicesAndBVs.emplace_back(i, AABB(std::vector<Vec3T<T>>{v[0], v[1], v[2]}));
  }

  // The tree is built over triangle indices in a scratch pool, then copied node for node with each
  // leaf's triangles regrouped into SIMD groups. Only the leaf ranges change.
  Pool          scratch(hostMemoryResource());
  const ByIndex byIndex(scratch, std::move(indicesAndBVs), a_spec);

  const auto indexNodes = byIndex.getNodes();
  const auto order      = byIndex.getPrimitives();

  std::vector<typename Root::Node> nodes(indexNodes.size());
  std::vector<TriAoSoA>            groups;

  for (uint32_t i = 0; i < indexNodes.size(); i++) {
    const auto& src = indexNodes[i];
    auto&       dst = nodes[i];

    dst.setBoundingVolume(src.getBoundingVolume());

    if (src.isLeaf()) {
      const uint32_t first = src.getPrimitivesOffset();

      const auto getTriangle = [&a_triangles, &order, first](uint32_t a_i) -> const Tri& {
        return a_triangles[order[first + a_i]];
      };

      const std::vector<TriAoSoA> leafGroups = TriMeshSDF::groupTriangles(getTriangle, src.getNumPrimitives());

      dst.setPrimitivesOffset(static_cast<uint32_t>(groups.size()));
      dst.setNumPrimitives(static_cast<uint32_t>(leafGroups.size()));

      groups.insert(groups.end(), leafGroups.begin(), leafGroups.end());
    }
    else {
      for (size_t k = 0; k < K; k++) {
        dst.setChildOffset(src.getChildOffsets()[k], k);
      }
    }
  }

  return Root(a_pool, nodes, groups);
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST
inline BVH::ConstructionOptions
TriMeshSDF<T, K, W>::defaultConstructionOptions(const size_t a_maxLeafGroups) noexcept
{
  EBGEOMETRY_REQUIRE(
    a_maxLeafGroups > 0, "TriMeshSDF: the maximum number of leaf groups must be positive (%zu)", a_maxLeafGroups);

  const size_t maxLeafSize = a_maxLeafGroups * W;

  // A ClusterSAH leaf holds at most K-1 clusters, so this cluster size keeps a leaf within
  // maxLeafSize triangles, as the top-down methods do (or within K-1, if maxLeafSize is smaller).
  const size_t clusterSize = Math::max(size_t(1), maxLeafSize / (K - 1));

  return BVH::ConstructionOptions{maxLeafSize, maxLeafSize, BVH::ClusterSpec{clusterSize}};
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST
inline TriMeshSDF<T, K, W>::TriMeshSDF(const Mesh&             a_mesh,
                                       Pool&                   a_pool,
                                       const BVH::Construction a_construction,
                                       const size_t            a_maxLeafGroups)
  : TriMeshSDF(a_mesh, a_pool, a_construction, TriMeshSDF::defaultConstructionOptions(a_maxLeafGroups))
{}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST
inline TriMeshSDF<T, K, W>::TriMeshSDF(const std::vector<Tri>& a_triangles,
                                       Pool&                   a_pool,
                                       const BVH::Construction a_construction,
                                       const size_t            a_maxLeafGroups)
  : TriMeshSDF(a_triangles, a_pool, a_construction, TriMeshSDF::defaultConstructionOptions(a_maxLeafGroups))
{}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST
inline TriMeshSDF<T, K, W>::TriMeshSDF(const Mesh&                     a_mesh,
                                       Pool&                           a_pool,
                                       const BVH::Construction         a_construction,
                                       const BVH::ConstructionOptions& a_options)
  : TriMeshSDF(TriMeshSDF::extractTriangles(a_mesh), a_pool, a_construction, a_options)
{}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST
inline TriMeshSDF<T, K, W>::TriMeshSDF(const std::vector<Tri>&         a_triangles,
                                       Pool&                           a_pool,
                                       const BVH::Construction         a_construction,
                                       const BVH::ConstructionOptions& a_options)
  : m_bvh(TriMeshSDF::buildBVH(a_triangles, a_pool, a_construction, a_options))
{}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline T
TriMeshSDF<T, K, W>::signedDistance(const Vec3T<T>& a_point) const noexcept
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

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline ClosestFace<T>
TriMeshSDF<T, K, W>::getClosestFace(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  // Same SIMD-pruned traversal as signedDistance(), but the running state also carries the winning
  // triangle's face id: each visited leaf group reports both its closest signed distance and that
  // triangle's id via TriangleAoSoA::signedDistance(point, uint32_t&). The pruning bound is still the
  // squared running distance, so node pruning is identical to signedDistance()'s.
  ClosestFace<T> closest;

  const auto groups = m_bvh.getPrimitives();

  const auto evalLeaf = [&groups, &a_point](ClosestFace<T>& a_state, size_t a_offset, size_t a_count) noexcept {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      uint32_t groupFaceId = UINT32_MAX;
      const T  d           = groups[static_cast<uint32_t>(i)].signedDistance(a_point, groupFaceId);

      EBGEOMETRY_EXPECT(!std::isnan(d));

      if (std::abs(d) < std::abs(a_state.signedDistance)) {
        a_state.signedDistance = d;
        a_state.faceId         = groupFaceId;
      }
    }
  };

  const auto pruneDist2 = [](const ClosestFace<T>& a_state) noexcept -> T {
    return a_state.signedDistance * a_state.signedDistance;
  };

  m_bvh.pruneTraverse(a_point, closest, evalLeaf, pruneDist2);

  return closest;
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST
inline TriMeshSDF<T, K, W>
TriMeshSDF<T, K, W>::rebasedView(const Pool& a_pool) const noexcept
{
  TriMeshSDF view = *this;

  view.m_bvh = m_bvh.rebasedView(a_pool);

  return view;
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline TriMeshSDF<T, K, W>
TriMeshSDF<T, K, W>::relocatedTo(const PoolLocation& a_location) const noexcept
{
  TriMeshSDF view = *this;

  view.m_bvh = m_bvh.relocatedTo(a_location);

  return view;
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST
inline TriMeshSDF<T, K, W>
TriMeshSDF<T, K, W>::deepCopy(Pool& a_dstPool) const
{
  TriMeshSDF copy = *this;

  copy.m_bvh = m_bvh.deepCopy(a_dstPool);

  return copy;
}

} // namespace EBGeometry

#endif
