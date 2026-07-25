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
buildDCELTreeBVH(const std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>& a_dcelMesh,
                 const BVH::Build                                         a_build = BVH::Build::SAH)
{
  static_assert(std::is_floating_point_v<T>,
                "MeshDistanceFunctionsDetail::buildDCELTreeBVH requires a floating-point type T");
  static_assert(K >= 2, "MeshDistanceFunctionsDetail::buildDCELTreeBVH: branching factor K must be at least 2");

  EBGEOMETRY_EXPECT(a_dcelMesh != nullptr);
  EBGEOMETRY_EXPECT(!a_dcelMesh->getFaces().empty());

  using Prim          = EBGeometry::DCEL::FaceT<T, Meta>;
  using PrimAndBVList = std::vector<std::pair<std::shared_ptr<const Prim>, BV>>;

  // Create a pair-wise list of DCEL faces and their bounding volumes. Each face is copied into a
  // fresh shared_ptr here (rather than aliasing the mesh's own array) since DCEL::FaceT is now a
  // plain, trivially-copyable value -- the copy's topology indices remain valid because they are
  // resolved against the same retained mesh (see MeshSDF::m_mesh) both before and after the copy.
  PrimAndBVList primsAndBVs;

  for (const auto& f : a_dcelMesh->getFaces()) {
    primsAndBVs.emplace_back(
      std::make_pair(std::make_shared<const Prim>(f), BV(f.getAllVertexCoordinates(*a_dcelMesh))));
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
buildTriTreeBVH(const std::vector<std::shared_ptr<EBGeometry::Triangle<T, Meta>>>& a_triangles,
                const BVH::Build                                                   a_build,
                const size_t                                                       a_maxLeafSize)
{
  static_assert(std::is_floating_point_v<T>,
                "MeshDistanceFunctionsDetail::buildTriTreeBVH requires a floating-point type T");
  static_assert(K >= 2, "MeshDistanceFunctionsDetail::buildTriTreeBVH: branching factor K must be at least 2");

  EBGEOMETRY_EXPECT(!a_triangles.empty());

  using Prim          = EBGeometry::Triangle<T, Meta>;
  using PrimAndBVList = std::vector<std::pair<std::shared_ptr<const Prim>, BV>>;

  // Create a pair-wise list of DCEL faces and their bounding volumes.
  PrimAndBVList primsAndBVs;

  for (const auto& tri : a_triangles) {
    EBGEOMETRY_EXPECT(tri != nullptr);

    const auto& vertexPositions = tri->getVertexPositions();

    const std::vector<EBGeometry::Vec3T<T>> vertices{vertexPositions[0], vertexPositions[1], vertexPositions[2]};

    primsAndBVs.emplace_back(std::make_pair(tri, BV(vertices)));
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
FlatMeshSDF<T, Meta>::FlatMeshSDF(const std::shared_ptr<Mesh>& a_mesh) noexcept
{
  EBGEOMETRY_EXPECT(a_mesh != nullptr);

  m_mesh = a_mesh;
}

template <class T, class Meta>
T
FlatMeshSDF<T, Meta>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(m_mesh != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  return m_mesh->signedDistance(a_point);
}

template <class T, class Meta>
const std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>
FlatMeshSDF<T, Meta>::getMesh() const noexcept
{
  EBGEOMETRY_EXPECT(m_mesh != nullptr);

  return m_mesh;
}

template <class T, class Meta>
template <class BV>
BV
FlatMeshSDF<T, Meta>::computeBoundingVolume() const
{
  EBGEOMETRY_EXPECT(m_mesh != nullptr);

  return BV(m_mesh->getAllVertexCoordinates());
};

template <class T, class Meta, size_t K>
MeshSDF<T, Meta, K>::MeshSDF(const std::shared_ptr<Mesh>& a_mesh, const BVH::Build a_build)
{
  EBGEOMETRY_EXPECT(a_mesh != nullptr);

  using AABB     = EBGeometry::BoundingVolumes::AABBT<T>;
  const auto bvh = EBGeometry::MeshDistanceFunctionsDetail::buildDCELTreeBVH<T, Meta, AABB, K>(a_mesh, a_build);

  m_bvh = bvh->pack();

  // Each DCEL::FaceT held by m_bvh only stores a half-edge INDEX, meaningful solely against the
  // mesh's own vertex/edge/face arrays -- it carries no owning reference to them. The source mesh
  // must therefore be retained here too, or those indices would dangle as soon as the caller's
  // mesh shared_ptr goes out of scope.
  m_mesh = a_mesh;
}

template <class T, class Meta, size_t K>
T
MeshSDF<T, Meta, K>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(m_bvh != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  EBGEOMETRY_EXPECT(m_mesh != nullptr);

  T           minDist = std::numeric_limits<T>::max();
  const auto& faces   = m_bvh->getPrimitives();
  const auto& mesh    = *m_mesh;

  const auto evalLeaf = [&faces, &a_point, &mesh](T& a_state, size_t a_offset, size_t a_count) noexcept {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      const T curDist = faces[i]->signedDistance(a_point, mesh);

      EBGEOMETRY_EXPECT(!std::isnan(curDist));

      a_state = std::abs(curDist) < std::abs(a_state) ? curDist : a_state;
    }
  };

  const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state * a_state; };

  m_bvh->pruneTraverse(a_point, minDist, evalLeaf, pruneDist2);

  return minDist;
}

template <class T, class Meta, size_t K>
std::vector<std::pair<std::shared_ptr<const EBGeometry::DCEL::FaceT<T, Meta>>, T>>
MeshSDF<T, Meta, K>::getClosestFaces(const Vec3T<T>& a_point, const bool a_sorted) const
{
  EBGEOMETRY_EXPECT(m_bvh != nullptr);
  EBGEOMETRY_EXPECT(m_mesh != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  const auto& mesh = *m_mesh;

  using FaceAndDist = std::pair<std::shared_ptr<const Face>, T>;

  // List of candidate faces.
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
      const std::vector<std::shared_ptr<const Face>>& a_faces, size_t offset, size_t count) noexcept -> void {
    for (size_t i = offset; i < offset + count; i++) {
      const T distToFace = std::sqrt(a_faces[i]->unsignedDistance2(a_point, mesh));

      EBGEOMETRY_EXPECT(!std::isnan(distToFace));

      if (distToFace <= shortestDistanceSoFar) {
        candidateFaces.emplace_back(a_faces[i], distToFace);
        shortestDistanceSoFar = distToFace;
      }
    }
  };

  m_bvh->traverse(leafEvaluator, prunePredicate, childOrderer, nodeKeyFactory);

  if (a_sorted) {
    std::sort(candidateFaces.begin(), candidateFaces.end(), [](const FaceAndDist& a, const FaceAndDist& b) {
      return a.second < b.second;
    });
  }

  return candidateFaces;
}

template <class T, class Meta, size_t K>
std::shared_ptr<EBGeometry::BVH::PackedBVH<T, EBGeometry::DCEL::FaceT<T, Meta>, K>>&
MeshSDF<T, Meta, K>::getRoot() noexcept
{
  EBGEOMETRY_EXPECT(m_bvh != nullptr);

  return (m_bvh);
}

template <class T, class Meta, size_t K>
const std::shared_ptr<EBGeometry::BVH::PackedBVH<T, EBGeometry::DCEL::FaceT<T, Meta>, K>>&
MeshSDF<T, Meta, K>::getRoot() const noexcept
{
  EBGEOMETRY_EXPECT(m_bvh != nullptr);

  return (m_bvh);
}

template <class T, class Meta, size_t K>
EBGeometry::BoundingVolumes::AABBT<T>
MeshSDF<T, Meta, K>::computeBoundingVolume() const noexcept
{
  EBGEOMETRY_EXPECT(m_bvh != nullptr);

  return m_bvh->getBoundingVolume();
};

template <class T, class Meta, size_t K, size_t W, class StoragePolicy>
std::vector<typename TriMeshSDF<T, Meta, K, W, StoragePolicy>::TriAoSoA>
TriMeshSDF<T, Meta, K, W, StoragePolicy>::groupTrianglesIntoSoA(
  const std::vector<std::shared_ptr<const Tri>>& a_triangles, uint32_t a_offset, uint32_t a_count)
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

template <class T, class Meta, size_t K, size_t W, class StoragePolicy>
TriMeshSDF<T, Meta, K, W, StoragePolicy>::TriMeshSDF(const std::shared_ptr<Mesh>& a_mesh,
                                                     const BVH::Build             a_build,
                                                     const size_t                 a_maxLeafGroups) noexcept
{
  EBGEOMETRY_EXPECT(a_mesh != nullptr);
  EBGEOMETRY_EXPECT(a_maxLeafGroups > 0);

  using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

  std::vector<std::shared_ptr<Tri>> triangles;

  for (const auto& f : a_mesh->getFaces()) {
    const auto normal        = f.getNormal();
    const auto vertexIndices = f.gatherVertexIndices(*a_mesh);
    const auto edgeIndices   = f.gatherEdgeIndices(*a_mesh);

    EBGEOMETRY_EXPECT(vertexIndices.size() == 3);
    EBGEOMETRY_EXPECT(edgeIndices.size() == 3);

    if ((vertexIndices.size() != 3) || (edgeIndices.size() != 3)) {
      std::cerr << "TriMeshSDF -- mesh not triangulated!\n";
    }

    const auto& v0 = a_mesh->getVertices()[vertexIndices[0]];
    const auto& v1 = a_mesh->getVertices()[vertexIndices[1]];
    const auto& v2 = a_mesh->getVertices()[vertexIndices[2]];

    const auto& e0 = a_mesh->getEdges()[edgeIndices[0]];
    const auto& e1 = a_mesh->getEdges()[edgeIndices[1]];
    const auto& e2 = a_mesh->getEdges()[edgeIndices[2]];

    auto tri = std::make_shared<Tri>();

    tri->setNormal(normal);
    tri->setVertexPositions({v0.getPosition(), v1.getPosition(), v2.getPosition()});
    tri->setVertexNormals({v0.getNormal(), v1.getNormal(), v2.getNormal()});
    tri->setEdgeNormals({e0.getNormal(), e1.getNormal(), e2.getNormal()});
    tri->setMetaData(f.getMetaData());

    triangles.emplace_back(tri);
  }

  const size_t maxLeafSize = a_maxLeafGroups * W;

  using Converter = decltype(&TriMeshSDF::groupTrianglesIntoSoA);

  m_bvh = EBGeometry::MeshDistanceFunctionsDetail::buildTriTreeBVH<T, Meta, AABB, K>(triangles, a_build, maxLeafSize)
            ->template packWith<TriAoSoA, Converter, StoragePolicy>(&TriMeshSDF::groupTrianglesIntoSoA);
}

template <class T, class Meta, size_t K, size_t W, class StoragePolicy>
TriMeshSDF<T, Meta, K, W, StoragePolicy>::TriMeshSDF(const std::vector<std::shared_ptr<Tri>>& a_triangles,
                                                     const BVH::Build                         a_build,
                                                     const size_t                             a_maxLeafGroups) noexcept
{
  EBGEOMETRY_EXPECT(!a_triangles.empty());
  EBGEOMETRY_EXPECT(a_maxLeafGroups > 0);

  using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

  const size_t maxLeafSize = a_maxLeafGroups * W;

  using Converter = decltype(&TriMeshSDF::groupTrianglesIntoSoA);

  m_bvh = EBGeometry::MeshDistanceFunctionsDetail::buildTriTreeBVH<T, Meta, AABB, K>(a_triangles, a_build, maxLeafSize)
            ->template packWith<TriAoSoA, Converter, StoragePolicy>(&TriMeshSDF::groupTrianglesIntoSoA);
}

template <class T, class Meta, size_t K, size_t W, class StoragePolicy>
T
TriMeshSDF<T, Meta, K, W, StoragePolicy>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(m_bvh != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  T           minDist = std::numeric_limits<T>::max();
  const auto& groups  = m_bvh->getPrimitives();

  const auto evalLeaf = [&groups, &a_point](T& a_state, size_t a_offset, size_t a_count) noexcept {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      const T d = StoragePolicy::get(groups[i]).signedDistance(a_point);

      EBGEOMETRY_EXPECT(!std::isnan(d));

      if (std::abs(d) < std::abs(a_state)) {
        a_state = d;
      }
    }
  };

  const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state * a_state; };

  m_bvh->pruneTraverse(a_point, minDist, evalLeaf, pruneDist2);

  return minDist;
}

template <class T, class Meta, size_t K, size_t W, class StoragePolicy>
typename TriMeshSDF<T, Meta, K, W, StoragePolicy>::ClosestTriangle
TriMeshSDF<T, Meta, K, W, StoragePolicy>::getClosestTriangle(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(m_bvh != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  // Same SIMD-pruned traversal as signedDistance(), but the running state also carries the winning
  // triangle's metadata: each visited leaf group reports both its closest signed distance and that
  // triangle's Meta via TriangleAoSoA::signedDistance(point, Meta&). The pruning bound is still the
  // squared running distance, so node pruning is identical to signedDistance()'s.
  ClosestTriangle closest;

  const auto& groups = m_bvh->getPrimitives();

  const auto evalLeaf = [&groups, &a_point](ClosestTriangle& a_state, size_t a_offset, size_t a_count) noexcept {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      Meta    groupMeta{};
      const T d = StoragePolicy::get(groups[i]).signedDistance(a_point, groupMeta);

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

  m_bvh->pruneTraverse(a_point, closest, evalLeaf, pruneDist2);

  return closest;
}

template <class T, class Meta, size_t K, size_t W, class StoragePolicy>
std::shared_ptr<EBGeometry::BVH::PackedBVH<T, EBGeometry::TriangleAoSoA<T, Meta, W>, K, StoragePolicy>>&
TriMeshSDF<T, Meta, K, W, StoragePolicy>::getRoot() noexcept
{
  EBGEOMETRY_EXPECT(m_bvh != nullptr);

  return m_bvh;
}

template <class T, class Meta, size_t K, size_t W, class StoragePolicy>
const std::shared_ptr<EBGeometry::BVH::PackedBVH<T, EBGeometry::TriangleAoSoA<T, Meta, W>, K, StoragePolicy>>&
TriMeshSDF<T, Meta, K, W, StoragePolicy>::getRoot() const noexcept
{
  EBGEOMETRY_EXPECT(m_bvh != nullptr);

  return m_bvh;
}

template <class T, class Meta, size_t K, size_t W, class StoragePolicy>
EBGeometry::BoundingVolumes::AABBT<T>
TriMeshSDF<T, Meta, K, W, StoragePolicy>::computeBoundingVolume() const noexcept
{
  EBGEOMETRY_EXPECT(m_bvh != nullptr);

  return m_bvh->getBoundingVolume();
};

} // namespace EBGeometry

#endif
