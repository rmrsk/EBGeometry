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

template <class T, class Meta, class BV, size_t K>
std::shared_ptr<EBGeometry::BVH::TreeBVH<T, EBGeometry::DCEL::FaceT<T, Meta>, BV, K>>
DCEL::buildTreeBVH(const std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>& a_dcelMesh, const BVH::Build a_build)
{
  static_assert(std::is_floating_point_v<T>, "DCEL::buildTreeBVH requires a floating-point type T");
  static_assert(K >= 2, "DCEL::buildTreeBVH: branching factor K must be at least 2");

  EBGEOMETRY_EXPECT(a_dcelMesh != nullptr);
  EBGEOMETRY_EXPECT(!a_dcelMesh->getFaces().empty());

  using Prim          = EBGeometry::DCEL::FaceT<T, Meta>;
  using PrimAndBVList = std::vector<std::pair<std::shared_ptr<const Prim>, BV>>;

  // Create a pair-wise list of DCEL faces and their bounding volumes.
  PrimAndBVList primsAndBVs;

  for (const auto& f : a_dcelMesh->getFaces()) {
    EBGEOMETRY_EXPECT(f != nullptr);

    primsAndBVs.emplace_back(std::make_pair(f, BV(f->getAllVertexCoordinates())));
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
    using StopFunc = typename Node::StopFunction;

    const StopFunc stopCrit = [](const Node& n) noexcept -> bool { return n.getPrimitives().size() < K; };

    bvh->topDownSortAndPartition(EBGeometry::BVH::BinnedSAHPartitioner<T, Prim, BV, K>, stopCrit);

    break;
  }
  default: {
    std::cerr << "EBGeometry::DCEL::buildTreeBVH - unsupported build method requested" << '\n';

    break;
  }
  }

  return bvh;
}

/**
 * @brief Build a tree BVH from a flat triangle soup.
 * @details Creates one BV per triangle from its vertex positions, then
 * builds a K-ary tree BVH according to a_build.  For TopDown builds the
 * tree is partitioned until each leaf holds at most a_maxLeafSize triangles.
 * @tparam T    Floating-point precision type.
 * @tparam Meta Triangle metadata type.
 * @tparam BV   Bounding-volume type (e.g. AABBT<T>).
 * @tparam K    BVH branching factor (number of children per internal node).
 * @param[in] a_triangles   Triangle soup to build the BVH over.
 * @param[in] a_build       Build strategy (TopDown, Morton, Nested, or SAH). SAH is the default.
 * @param[in] a_maxLeafSize Maximum number of triangles per BVH leaf node
 * (ignored for Morton and Nested builds).
 * @return Shared pointer to the root of the resulting tree BVH.
 */
template <class T, class Meta, class BV, size_t K>
std::shared_ptr<EBGeometry::BVH::TreeBVH<T, Triangle<T, Meta>, BV, K>>
buildTriMeshTreeBVH(const std::vector<std::shared_ptr<EBGeometry::Triangle<T, Meta>>>& a_triangles,
                    const BVH::Build                                                   a_build       = BVH::Build::SAH,
                    const size_t                                                       a_maxLeafSize = 4U)
{
  static_assert(std::is_floating_point_v<T>, "buildTriMeshTreeBVH requires a floating-point type T");
  static_assert(K >= 2, "buildTriMeshTreeBVH: branching factor K must be at least 2");

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
    using StopFunc          = typename Node::StopFunction;
    const StopFunc stopCrit = [a_maxLeafSize](const Node& n) noexcept -> bool {
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
    using StopFunc = typename Node::StopFunction;

    const StopFunc stopCrit = [a_maxLeafSize](const Node& n) noexcept -> bool {
      return n.getPrimitives().size() <= a_maxLeafSize;
    };

    bvh->topDownSortAndPartition(EBGeometry::BVH::BinnedSAHPartitioner<T, Prim, BV, K>, stopCrit);

    break;
  }
  default: {
    std::cerr << "EBGeometry::buildTriMeshTreeBVH - unsupported build method requested" << '\n';

    break;
  }
  }

  return bvh;
}

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
  const auto bvh = EBGeometry::DCEL::buildTreeBVH<T, Meta, AABB, K>(a_mesh, a_build);

  m_bvh = bvh->pack();
}

template <class T, class Meta, size_t K>
T
MeshSDF<T, Meta, K>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(m_bvh != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  T minDist = std::numeric_limits<T>::max();

  const BVH::PackedUpdater<Face> updater = [&minDist, &a_point](const std::vector<std::shared_ptr<const Face>>& faces,
                                                                size_t                                          offset,
                                                                size_t count) noexcept -> void {
    for (size_t i = offset; i < offset + count; i++) {
      const T curDist = faces[i]->signedDistance(a_point);

      EBGEOMETRY_EXPECT(!std::isnan(curDist));

      minDist = std::abs(curDist) < std::abs(minDist) ? curDist : minDist;
    }
  };

  const BVH::Visiter<Node, T> visiter = [&minDist](const Node& a_node, const T& a_bvDist) noexcept -> bool {
    return a_bvDist <= std::abs(minDist);
  };

  const BVH::PackedSorter<T, K> sorter = [](std::array<std::pair<uint32_t, T>, K>& a_leaves) noexcept -> void {
    std::sort(
      a_leaves.begin(), a_leaves.end(), [](const std::pair<uint32_t, T>& n1, const std::pair<uint32_t, T>& n2) -> bool {
        return n1.second > n2.second;
      });
  };

  const BVH::MetaUpdater<Node, T> metaUpdater = [&a_point](const Node& a_node) noexcept -> T {
    return a_node.getDistanceToBoundingVolume(a_point);
  };

  m_bvh->traverse(updater, visiter, sorter, metaUpdater);

  return minDist;
}

template <class T, class Meta, size_t K>
std::vector<std::pair<std::shared_ptr<const EBGeometry::DCEL::FaceT<T, Meta>>, T>>
MeshSDF<T, Meta, K>::getClosestFaces(const Vec3T<T>& a_point, const bool a_sorted) const
{
  EBGEOMETRY_EXPECT(m_bvh != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  using FaceAndDist = std::pair<std::shared_ptr<const Face>, T>;

  // List of candidate faces.
  std::vector<FaceAndDist> candidateFaces;

  // Declaration of the BVH metadata attached to each node - this will be the distance to the node itself.
  using BVHMeta = T;

  // Shortest distance so far.
  BVHMeta shortestDistanceSoFar = std::numeric_limits<T>::max();

  const EBGeometry::BVH::Visiter<Node, T> visiter = [&shortestDistanceSoFar](const Node&    a_node,
                                                                             const BVHMeta& a_bvDist) noexcept -> bool {
    return a_bvDist <= 0.0 || a_bvDist <= shortestDistanceSoFar;
  };

  const EBGeometry::BVH::PackedSorter<T, K> sorter =
    [](std::array<std::pair<uint32_t, T>, K>& a_leaves) noexcept -> void {
    std::sort(
      a_leaves.begin(), a_leaves.end(), [](const std::pair<uint32_t, T>& n1, const std::pair<uint32_t, T>& n2) -> bool {
        return n1.second > n2.second;
      });
  };

  const EBGeometry::BVH::MetaUpdater<Node, BVHMeta> metaUpdater = [&a_point](const Node& a_node) noexcept -> BVHMeta {
    return a_node.getDistanceToBoundingVolume(a_point);
  };

  const EBGeometry::BVH::PackedUpdater<Face> updater =
    [&shortestDistanceSoFar, &a_point, &candidateFaces](
      const std::vector<std::shared_ptr<const Face>>& a_faces, size_t offset, size_t count) noexcept -> void {
    for (size_t i = offset; i < offset + count; i++) {
      const T distToFace = sqrt(a_faces[i]->unsignedDistance2(a_point));

      EBGEOMETRY_EXPECT(!std::isnan(distToFace));

      if (distToFace <= shortestDistanceSoFar) {
        candidateFaces.emplace_back(a_faces[i], distToFace);
        shortestDistanceSoFar = distToFace;
      }
    }
  };

  m_bvh->traverse(updater, visiter, sorter, metaUpdater);

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

template <class T, class Meta, size_t K, size_t W>
std::vector<typename TriMeshSDF<T, Meta, K, W>::TriSoA>
TriMeshSDF<T, Meta, K, W>::groupTrianglesIntoSoA(const std::vector<std::shared_ptr<const Tri>>& a_triangles,
                                                 uint32_t                                       a_offset,
                                                 uint32_t                                       a_count)
{
  constexpr uint32_t soaWidth = static_cast<uint32_t>(W);

  std::vector<TriSoA> groups;

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

    TriSoA soa;
    soa.pack(trisArr.data(), groupCount);
    groups.push_back(std::move(soa));
  }

  return groups;
}

template <class T, class Meta, size_t K, size_t W>
TriMeshSDF<T, Meta, K, W>::TriMeshSDF(const std::shared_ptr<Mesh>& a_mesh,
                                      const BVH::Build             a_build,
                                      const size_t                 a_maxLeafSize) noexcept
{
  EBGEOMETRY_EXPECT(a_mesh != nullptr);

  using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

  std::vector<std::shared_ptr<Tri>> triangles;

  for (const auto& f : a_mesh->getFaces()) {
    EBGEOMETRY_EXPECT(f != nullptr);

    const auto normal   = f->getNormal();
    const auto vertices = f->gatherVertices();
    const auto edges    = f->gatherEdges();

    EBGEOMETRY_EXPECT(vertices.size() == 3);
    EBGEOMETRY_EXPECT(edges.size() == 3);

    if ((vertices.size() != 3) || (edges.size() != 3)) {
      std::cerr << "TriMeshSDF -- mesh not triangulated!\n";
    }

    auto tri = std::make_shared<Tri>();

    tri->setNormal(normal);
    tri->setVertexPositions({vertices[0]->getPosition(), vertices[1]->getPosition(), vertices[2]->getPosition()});
    tri->setVertexNormals({vertices[0]->getNormal(), vertices[1]->getNormal(), vertices[2]->getNormal()});
    tri->setEdgeNormals({edges[0]->getNormal(), edges[1]->getNormal(), edges[2]->getNormal()});
    tri->setMetaData(f->getMetaData());

    triangles.emplace_back(tri);
  }

  m_bvh = EBGeometry::buildTriMeshTreeBVH<T, Meta, AABB, K>(triangles, a_build, a_maxLeafSize)
            ->template packWith<TriSoA>(&TriMeshSDF::groupTrianglesIntoSoA);
}

template <class T, class Meta, size_t K, size_t W>
TriMeshSDF<T, Meta, K, W>::TriMeshSDF(const std::vector<std::shared_ptr<Tri>>& a_triangles,
                                      const BVH::Build                         a_build,
                                      const size_t                             a_maxLeafSize) noexcept
{
  EBGEOMETRY_EXPECT(!a_triangles.empty());

  using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

  m_bvh = EBGeometry::buildTriMeshTreeBVH<T, Meta, AABB, K>(a_triangles, a_build, a_maxLeafSize)
            ->template packWith<TriSoA>(&TriMeshSDF::groupTrianglesIntoSoA);
}

template <class T, class Meta, size_t K, size_t W>
T
TriMeshSDF<T, Meta, K, W>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(m_bvh != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  return m_bvh->signedDistance(a_point);
}

template <class T, class Meta, size_t K, size_t W>
std::shared_ptr<EBGeometry::BVH::PackedBVH<T, EBGeometry::TriangleSoAT<T, W>, K>>&
TriMeshSDF<T, Meta, K, W>::getRoot() noexcept
{
  EBGEOMETRY_EXPECT(m_bvh != nullptr);

  return m_bvh;
}

template <class T, class Meta, size_t K, size_t W>
const std::shared_ptr<EBGeometry::BVH::PackedBVH<T, EBGeometry::TriangleSoAT<T, W>, K>>&
TriMeshSDF<T, Meta, K, W>::getRoot() const noexcept
{
  EBGEOMETRY_EXPECT(m_bvh != nullptr);

  return m_bvh;
}

template <class T, class Meta, size_t K, size_t W>
EBGeometry::BoundingVolumes::AABBT<T>
TriMeshSDF<T, Meta, K, W>::computeBoundingVolume() const noexcept
{
  EBGEOMETRY_EXPECT(m_bvh != nullptr);

  return m_bvh->getBoundingVolume();
};

} // namespace EBGeometry

#endif
