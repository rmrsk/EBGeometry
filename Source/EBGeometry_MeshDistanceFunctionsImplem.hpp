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
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_DCEL_Edge.hpp"
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_DCEL_Vertex.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_MeshDistanceFunctions.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Internal helpers shared by MeshSDF and TriMeshSDF
 */
namespace MeshDistanceFunctionsDetail {

/**
 * @brief Build a tree BVH from a DCEL mesh.
 * @details Internal helper; not part of the public API.
 * @tparam T    Floating-point precision type.
 * @tparam Meta Face and vertex metadata type.
 * @tparam BV   Bounding-volume type (e.g. AABBT<T>).
 * @tparam K    BVH branching factor (number of children per node).
 * @param[in] a_dcelMesh Input DCEL mesh.
 * @param[in] a_build    Build strategy (TopDown, Morton, Nested, or SAH). SAH is the default.
 * @return Shared pointer to the root of the resulting tree BVH.
 */
template <class T, class Meta, class BV, size_t K>
[[nodiscard]] std::shared_ptr<EBGeometry::BVH::TreeBVH<T, DCEL::FaceT<T, Meta>, BV, K>>
buildDCELTreeBVH(const EBGeometry::DCEL::MeshT<T, Meta>& a_dcelMesh, const BVH::Build a_build = BVH::Build::SAH)
{
  static_assert(std::is_floating_point_v<T>,
                "MeshDistanceFunctionsDetail::buildDCELTreeBVH requires a floating-point type T");
  static_assert(K >= 2, "MeshDistanceFunctionsDetail::buildDCELTreeBVH: branching factor K must be at least 2");

  EBGEOMETRY_EXPECT(a_dcelMesh.numFaces() > 0);

  using Prim          = EBGeometry::DCEL::FaceT<T, Meta>;
  using PrimAndBVList = std::vector<std::pair<std::shared_ptr<const Prim>, BV>>;

  // Create a pair-wise list of DCEL faces and their bounding volumes. Each face is copied into a
  // fresh shared_ptr here (rather than aliasing the mesh's own array) since DCEL::FaceT is now a
  // plain, trivially-copyable value -- the copy's topology indices remain valid because they are
  // resolved against the same retained mesh (see MeshSDF::m_mesh) both before and after the copy.
  PrimAndBVList primsAndBVs;

  for (uint32_t i = 0; i < a_dcelMesh.numFaces(); i++) {
    const auto& f = a_dcelMesh.getFace(i);

    primsAndBVs.emplace_back(
      std::make_pair(std::make_shared<const Prim>(f), BV(f.getAllVertexCoordinates(a_dcelMesh))));
  }

  // Partition the BVH using the default input arguments.
  auto bvh = std::make_shared<EBGeometry::BVH::TreeBVH<T, Prim, BV, K>>(primsAndBVs);

  switch (a_build) {
  case BVH::Build::TopDown: {
    bvh->topDownSortAndPartition();

    break;
  }
  case BVH::Build::Morton: {
    bvh->template bottomUpSortAndPartition<SFC::Morton>();

    break;
  }
  case BVH::Build::Nested: {
    bvh->template bottomUpSortAndPartition<SFC::Nested>();

    break;
  }
  case BVH::Build::SAH: {
    using Node     = EBGeometry::BVH::TreeBVH<T, Prim, BV, K>;
    using LeafPred = typename Node::LeafPredicate;

    const LeafPred stopCrit = [](const Node& n) noexcept -> bool { return n.getPrimitives().size() < K; };

    bvh->topDownSortAndPartition(EBGeometry::BVH::BinnedSAHPartitioner<T, Prim, BV, K>, stopCrit);

    break;
  }
  default: {
    std::cerr << "EBGeometry::MeshDistanceFunctionsDetail::buildDCELTreeBVH - unsupported build method requested"
              << '\n';

    break;
  }
  }

  return bvh;
}

/**
 * @brief Extract every face of a DCEL mesh as a flat, self-contained Triangle.
 * @details Internal helper shared by TriMeshSDF's mesh constructor and Parser::readIntoTriangles, so
 * both produce identical triangles: the face normal, the three vertex positions and vertex normals,
 * the three half-edge normals (edge i runs from vertex i to vertex i+1, the order both
 * FaceT::gatherVertexIndices and FaceT::gatherEdgeIndices walk the face), and the face metadata.
 * A face with more than three vertices contributes its first three, and a_onlyTriangles is cleared;
 * callers decide how to report that.
 * @tparam T    Floating-point precision type.
 * @tparam Meta Face metadata type.
 * @param[in]  a_mesh          DCEL mesh.
 * @param[out] a_onlyTriangles Set to true if every face is a triangle, false otherwise.
 * @return One Triangle per face, in face order.
 */
template <class T, class Meta>
[[nodiscard]] EBGEOMETRY_HOST
inline std::vector<Triangle<T, Meta>>
extractTriangles(const DCEL::MeshT<T, Meta>& a_mesh, bool& a_onlyTriangles)
{
  std::vector<Triangle<T, Meta>> triangles;

  triangles.reserve(a_mesh.numFaces());

  a_onlyTriangles = true;

  for (uint32_t faceIndex = 0; faceIndex < a_mesh.numFaces(); faceIndex++) {
    const auto& f             = a_mesh.getFace(faceIndex);
    const auto  vertexIndices = f.gatherVertexIndices(a_mesh);
    const auto  edgeIndices   = f.gatherEdgeIndices(a_mesh);

    EBGEOMETRY_EXPECT(vertexIndices.size() >= 3);
    EBGEOMETRY_EXPECT(edgeIndices.size() == vertexIndices.size());

    if (vertexIndices.size() != 3) {
      a_onlyTriangles = false;
    }

    const auto& v0 = a_mesh.getVertex(vertexIndices[0]);
    const auto& v1 = a_mesh.getVertex(vertexIndices[1]);
    const auto& v2 = a_mesh.getVertex(vertexIndices[2]);

    const auto& e0 = a_mesh.getEdge(edgeIndices[0]);
    const auto& e1 = a_mesh.getEdge(edgeIndices[1]);
    const auto& e2 = a_mesh.getEdge(edgeIndices[2]);

    Triangle<T, Meta> tri;

    tri.setNormal(f.getNormal());
    tri.setVertexPositions({v0.getPosition(), v1.getPosition(), v2.getPosition()});
    tri.setVertexNormals({v0.getNormal(), v1.getNormal(), v2.getNormal()});
    tri.setEdgeNormals({e0.getNormal(), e1.getNormal(), e2.getNormal()});
    tri.setMetaData(f.getMetaData());

    triangles.emplace_back(tri);
  }

  return triangles;
}

/**
 * @brief Build a tree BVH from a flat triangle soup.
 * @details Internal helper; not part of the public API. Creates one BV per triangle from its
 * vertex positions, then builds a K-ary tree BVH according to a_build.  For TopDown builds the
 * tree is partitioned until each leaf holds at most a_maxLeafSize triangles.
 * @tparam T    Floating-point precision type.
 * @tparam Meta Triangle metadata type.
 * @tparam BV   Bounding-volume type (e.g. AABBT<T>).
 * @tparam K    BVH branching factor (number of children per internal node).
 * @param[in] a_triangles   Triangle soup to build the BVH over.
 * @param[in] a_build       Build strategy (TopDown, Morton, Nested, or SAH).
 * @param[in] a_maxLeafSize Maximum number of triangles per BVH leaf node
 * (ignored for Morton and Nested builds).
 * @return Shared pointer to the root of the resulting tree BVH.
 */
template <class T, class Meta, class BV, size_t K>
std::shared_ptr<EBGeometry::BVH::TreeBVH<T, Triangle<T, Meta>, BV, K>>
buildTriTreeBVH(const std::vector<EBGeometry::Triangle<T, Meta>>& a_triangles,
                const BVH::Build                                  a_build,
                const size_t                                      a_maxLeafSize)
{
  static_assert(std::is_floating_point_v<T>,
                "MeshDistanceFunctionsDetail::buildTriTreeBVH requires a floating-point type T");
  static_assert(K >= 2, "MeshDistanceFunctionsDetail::buildTriTreeBVH: branching factor K must be at least 2");

  EBGEOMETRY_EXPECT(!a_triangles.empty());

  using Prim          = EBGeometry::Triangle<T, Meta>;
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

  // Partition the BVH using the default input arguments.
  auto bvh = std::make_shared<EBGeometry::BVH::TreeBVH<T, Prim, BV, K>>(primsAndBVs);

  switch (a_build) {
  case BVH::Build::TopDown: {
    using Node              = EBGeometry::BVH::TreeBVH<T, Prim, BV, K>;
    using LeafPred          = typename Node::LeafPredicate;
    const LeafPred stopCrit = [a_maxLeafSize](const Node& n) noexcept -> bool {
      return n.getPrimitives().size() <= a_maxLeafSize;
    };
    bvh->topDownSortAndPartition(EBGeometry::BVH::BVCentroidPartitioner<T, Prim, BV, K>, stopCrit);

    break;
  }
  case BVH::Build::Morton: {
    bvh->template bottomUpSortAndPartition<SFC::Morton>();

    break;
  }
  case BVH::Build::Nested: {
    bvh->template bottomUpSortAndPartition<SFC::Nested>();

    break;
  }
  case BVH::Build::SAH: {
    using Node     = EBGeometry::BVH::TreeBVH<T, Prim, BV, K>;
    using LeafPred = typename Node::LeafPredicate;

    const LeafPred stopCrit = [a_maxLeafSize](const Node& n) noexcept -> bool {
      return n.getPrimitives().size() <= a_maxLeafSize;
    };

    bvh->topDownSortAndPartition(EBGeometry::BVH::BinnedSAHPartitioner<T, Prim, BV, K>, stopCrit);

    break;
  }
  default: {
    std::cerr << "EBGeometry::MeshDistanceFunctionsDetail::buildTriTreeBVH - unsupported build method requested"
              << '\n';

    break;
  }
  }

  return bvh;
}

} // namespace MeshDistanceFunctionsDetail

template <class T, class Meta>
EBGEOMETRY_HOST
inline FlatMeshSDF<T, Meta>::FlatMeshSDF(const Mesh& a_mesh, Pool& a_pool) noexcept : m_mesh(a_mesh)
{
  EBGEOMETRY_EXPECT(a_mesh.isAttachedTo(a_pool));

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
inline MeshSDF<T, Meta, K>::MeshSDF(const Mesh& a_mesh, Pool& a_pool, const BVH::Build a_build)
  : m_mesh(a_mesh), m_bvh(MeshSDF::buildBVH(a_mesh, a_pool, a_build))
{
  // The mesh and the BVH must share one pool: rebasedView() rebases both onto the same mirror, and
  // the BVH's faces index the mesh's arrays. a_pool must outlive this object and every copy of it.
  EBGEOMETRY_EXPECT(a_mesh.isAttachedTo(a_pool));
  EBGEOMETRY_EXPECT(m_bvh.isAttachedTo(a_pool));
}

template <class T, class Meta, size_t K>
EBGEOMETRY_HOST
inline typename MeshSDF<T, Meta, K>::Root
MeshSDF<T, Meta, K>::buildBVH(const Mesh& a_mesh, Pool& a_pool, const BVH::Build a_build)
{
  using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

  // pack() still returns a shared_ptr; the PackedBVH it points to is a descriptor into a_pool, so
  // copying it out is all that is needed.
  return *EBGeometry::MeshDistanceFunctionsDetail::buildDCELTreeBVH<T, Meta, AABB, K>(a_mesh, a_build)->pack(a_pool);
}

template <class T, class Meta, size_t K>
EBGEOMETRY_HOST_DEVICE
inline T
MeshSDF<T, Meta, K>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  T           minDist = std::numeric_limits<T>::max();
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

  // Declaration of the BVH metadata attached to each node - this will be the distance to the node itself.
  using BVHNodeKey = T;

  // Shortest distance so far.
  BVHNodeKey shortestDistanceSoFar = std::numeric_limits<T>::max();

  const EBGeometry::BVH::PrunePredicate<Node, T> prunePredicate =
    [&shortestDistanceSoFar](const Node&, const BVHNodeKey& a_bvDist) noexcept -> bool {
    return a_bvDist <= T(0.0) || a_bvDist <= shortestDistanceSoFar;
  };

  const EBGeometry::BVH::PackedChildOrderer<T, K> childOrderer =
    [](std::array<std::pair<uint32_t, T>, K>& a_leaves) noexcept -> void {
    std::sort(
      a_leaves.begin(), a_leaves.end(), [](const std::pair<uint32_t, T>& n1, const std::pair<uint32_t, T>& n2) -> bool {
        return n1.second > n2.second;
      });
  };

  const EBGeometry::BVH::NodeKeyFactory<Node, BVHNodeKey> nodeKeyFactory =
    [&a_point](const Node& a_node) noexcept -> BVHNodeKey { return a_node.getDistanceToBoundingVolume(a_point); };

  const EBGeometry::BVH::PackedLeafEvaluator<Face> leafEvaluator =
    [&shortestDistanceSoFar, &a_point, &candidateFaces, &mesh](
      PODSpan<const Face> a_faces, size_t offset, size_t count) noexcept -> void {
    for (size_t i = offset; i < offset + count; i++) {
      const T distToFace = std::sqrt(a_faces[i].unsignedDistance2(a_point, mesh));

      EBGEOMETRY_EXPECT(!std::isnan(distToFace));

      if (distToFace <= shortestDistanceSoFar) {
        candidateFaces.emplace_back(static_cast<uint32_t>(i), distToFace);
        shortestDistanceSoFar = distToFace;
      }
    }
  };

  m_bvh.traverse(leafEvaluator, prunePredicate, childOrderer, nodeKeyFactory);

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
std::vector<typename TriMeshSDF<T, Meta, K, W>::TriAoSoA>
TriMeshSDF<T, Meta, K, W>::groupTrianglesIntoSoA(const std::vector<std::shared_ptr<const Tri>>& a_triangles,
                                                 uint32_t                                       a_offset,
                                                 uint32_t                                       a_count)
{
  constexpr uint32_t soaWidth = static_cast<uint32_t>(W);

  std::vector<TriAoSoA> groups;

  const uint32_t numGroups = (a_count + soaWidth - 1U) / soaWidth;
  groups.reserve(numGroups);

  for (uint32_t group = 0; group < numGroups; group++) {
    const uint32_t groupOffset = a_offset + group * soaWidth;
    const uint32_t groupCount  = std::min(soaWidth, a_count - group * soaWidth);

    std::array<Tri, W> trisArr;
    for (uint32_t i = 0; i < groupCount; i++) {
      EBGEOMETRY_EXPECT(a_triangles[groupOffset + i] != nullptr);

      trisArr[i] = *a_triangles[groupOffset + i];
    }

    TriAoSoA soa;
    soa.pack(trisArr.data(), groupCount);
    groups.push_back(std::move(soa));
  }

  return groups;
}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST
inline std::vector<typename TriMeshSDF<T, Meta, K, W>::Tri>
TriMeshSDF<T, Meta, K, W>::extractTriangles(const Mesh& a_mesh)
{
  bool onlyTriangles = true;

  std::vector<Tri> triangles = EBGeometry::MeshDistanceFunctionsDetail::extractTriangles(a_mesh, onlyTriangles);

  EBGEOMETRY_EXPECT(onlyTriangles);

  if (!onlyTriangles) {
    std::cerr << "TriMeshSDF -- mesh not triangulated!\n";
  }

  return triangles;
}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST
inline typename TriMeshSDF<T, Meta, K, W>::Root
TriMeshSDF<T, Meta, K, W>::buildBVH(const std::vector<Tri>& a_triangles,
                                    Pool&                   a_pool,
                                    const BVH::Build        a_build,
                                    const size_t            a_maxLeafGroups)
{
  EBGEOMETRY_EXPECT(!a_triangles.empty());
  EBGEOMETRY_EXPECT(a_maxLeafGroups > 0);

  using AABB      = EBGeometry::BoundingVolumes::AABBT<T>;
  using Converter = decltype(&TriMeshSDF::groupTrianglesIntoSoA);

  const size_t maxLeafSize = a_maxLeafGroups * W;

  // packWith() still returns a shared_ptr; the PackedBVH it points to is a descriptor into a_pool,
  // so copying it out is all that is needed.
  return *EBGeometry::MeshDistanceFunctionsDetail::buildTriTreeBVH<T, Meta, AABB, K>(a_triangles, a_build, maxLeafSize)
            ->template packWith<TriAoSoA, Converter>(a_pool, &TriMeshSDF::groupTrianglesIntoSoA);
}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST
inline TriMeshSDF<T, Meta, K, W>::TriMeshSDF(const Mesh&      a_mesh,
                                             Pool&            a_pool,
                                             const BVH::Build a_build,
                                             const size_t     a_maxLeafGroups)
  : TriMeshSDF(TriMeshSDF::extractTriangles(a_mesh), a_pool, a_build, a_maxLeafGroups)
{}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST
inline TriMeshSDF<T, Meta, K, W>::TriMeshSDF(const std::vector<Tri>& a_triangles,
                                             Pool&                   a_pool,
                                             const BVH::Build        a_build,
                                             const size_t            a_maxLeafGroups)
  : m_bvh(TriMeshSDF::buildBVH(a_triangles, a_pool, a_build, a_maxLeafGroups))
{}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline T
TriMeshSDF<T, Meta, K, W>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  T          minDist = std::numeric_limits<T>::max();
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
