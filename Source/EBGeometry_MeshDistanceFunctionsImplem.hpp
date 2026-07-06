// SPDX-FileCopyrightText: 2023 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  @file    EBGeometry_MeshDistanceFunctionsImplem.hpp
  @brief   Implementation of EBGeometry_MeshDistanceFunctions.hpp
  @author  Robert Marskar
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
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_MeshDistanceFunctions.hpp"

namespace EBGeometry {

template <class T, class Meta, class BV, size_t K>
std::shared_ptr<EBGeometry::BVH::TreeBVH<T, EBGeometry::DCEL::FaceT<T, Meta>, BV, K>>
DCEL::buildFullBVH(const std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>& a_dcelMesh, const BVH::Build a_build)
{
  using Prim          = EBGeometry::DCEL::FaceT<T, Meta>;
  using PrimAndBVList = std::vector<std::pair<std::shared_ptr<const Prim>, BV>>;

  // Create a pair-wise list of DCEL faces and their bounding volumes.
  PrimAndBVList primsAndBVs;

  for (const auto& f : a_dcelMesh->getFaces()) {
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
    using Node              = EBGeometry::BVH::TreeBVH<T, Prim, BV, K>;
    using StopFunc          = typename Node::StopFunction;
    const StopFunc stopCrit = [](const Node& n) noexcept -> bool { return n.getPrimitives().size() < K; };
    bvh->topDownSortAndPartition(EBGeometry::BVH::BinnedSAHPartitioner<T, Prim, BV, K>, stopCrit);

    break;
  }
  default: {
    std::cerr << "EBGeometry::DCEL::buildFullBVH - unsupported build method requested" << std::endl;

    break;
  }
  }

  return bvh;
}

/**
  @brief Build a full (tree) BVH from a flat triangle soup.
  @details Creates one BV per triangle from its vertex positions, then
  builds a K-ary tree BVH according to a_build.  For TopDown builds the
  tree is partitioned until each leaf holds at most a_maxLeafSize triangles.
  @tparam T    Floating-point precision type.
  @tparam Meta Triangle metadata type.
  @tparam BV   Bounding-volume type (e.g. AABBT<T>).
  @tparam K    BVH branching factor (number of children per internal node).
  @param[in] a_triangles   Triangle soup to build the BVH over.
  @param[in] a_build       Build strategy (TopDown, Morton, Nested, or SAH). SAH is the default.
  @param[in] a_maxLeafSize Maximum number of triangles per BVH leaf node
                           (ignored for Morton and Nested builds).
  @return Shared pointer to the root of the resulting tree BVH.
*/
template <class T, class Meta, class BV, size_t K>
std::shared_ptr<EBGeometry::BVH::TreeBVH<T, Triangle<T, Meta>, BV, K>>
buildTriMeshFullBVH(const std::vector<std::shared_ptr<EBGeometry::Triangle<T, Meta>>>& a_triangles,
                    const BVH::Build                                                   a_build       = BVH::Build::SAH,
                    const size_t                                                       a_maxLeafSize = 4U)
{
  using Prim          = EBGeometry::Triangle<T, Meta>;
  using PrimAndBVList = std::vector<std::pair<std::shared_ptr<const Prim>, BV>>;

  // Create a pair-wise list of DCEL faces and their bounding volumes.
  PrimAndBVList primsAndBVs;

  for (const auto& tri : a_triangles) {
    const auto& vertexPositions = tri->getVertexPositions();

    std::vector<EBGeometry::Vec3T<T>> vertices{vertexPositions[0], vertexPositions[1], vertexPositions[2]};

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
    using Node              = EBGeometry::BVH::TreeBVH<T, Prim, BV, K>;
    using StopFunc          = typename Node::StopFunction;
    const StopFunc stopCrit = [a_maxLeafSize](const Node& n) noexcept -> bool {
      return n.getPrimitives().size() <= a_maxLeafSize;
    };
    bvh->topDownSortAndPartition(EBGeometry::BVH::BinnedSAHPartitioner<T, Prim, BV, K>, stopCrit);

    break;
  }
  default: {
    std::cerr << "EBGeometry::DCEL::buildFullBVH - unsupported build method requested" << std::endl;

    break;
  }
  }

  return bvh;
}

template <class T, class Meta>
FlatMeshSDF<T, Meta>::FlatMeshSDF(const std::shared_ptr<Mesh>& a_mesh) noexcept
{
  m_mesh = a_mesh;
}

template <class T, class Meta>
T
FlatMeshSDF<T, Meta>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  return m_mesh->signedDistance(a_point);
}

template <class T, class Meta>
const std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>
FlatMeshSDF<T, Meta>::getMesh() const noexcept
{
  return m_mesh;
}

template <class T, class Meta>
template <class BV>
BV
FlatMeshSDF<T, Meta>::computeBoundingVolume() const
{
  return BV(m_mesh->getAllVertexCoordinates());
};

template <class T, class Meta, size_t K>
MeshSDF<T, Meta, K>::MeshSDF(const std::shared_ptr<Mesh>& a_mesh, const BVH::Build a_build)
{
  using AABB     = EBGeometry::BoundingVolumes::AABBT<T>;
  const auto bvh = EBGeometry::DCEL::buildFullBVH<T, Meta, AABB, K>(a_mesh, a_build);

  m_bvh = bvh->pack();
}

template <class T, class Meta, size_t K>
T
MeshSDF<T, Meta, K>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  T minDist = std::numeric_limits<T>::max();

  BVH::PackedUpdater<Face> updater = [&minDist, &a_point](const std::vector<std::shared_ptr<const Face>>& faces,
                                                          size_t                                          offset,
                                                          size_t count) noexcept -> void {
#pragma GCC ivdep
    for (size_t i = offset; i < offset + count; i++) {
      const T curDist = faces[i]->signedDistance(a_point);
      minDist         = std::abs(curDist) < std::abs(minDist) ? curDist : minDist;
    }
  };

  BVH::Visiter<Node, T> visiter = [&minDist](const Node& a_node, const T& a_bvDist) noexcept -> bool {
    return a_bvDist <= std::abs(minDist);
  };

  BVH::PackedSorter<T, K> sorter = [](std::array<std::pair<uint32_t, T>, K>& a_leaves) noexcept -> void {
    std::sort(
      a_leaves.begin(), a_leaves.end(), [](const std::pair<uint32_t, T>& n1, const std::pair<uint32_t, T>& n2) -> bool {
        return n1.second > n2.second;
      });
  };

  BVH::MetaUpdater<Node, T> metaUpdater = [&a_point](const Node& a_node) noexcept -> T {
    return a_node.getDistanceToBoundingVolume(a_point);
  };

  m_bvh->traverse(updater, visiter, sorter, metaUpdater);

  return minDist;
}

template <class T, class Meta, size_t K>
std::vector<std::pair<std::shared_ptr<const EBGeometry::DCEL::FaceT<T, Meta>>, T>>
MeshSDF<T, Meta, K>::getClosestFaces(const Vec3T<T>& a_point, const bool a_sorted) const
{
  using FaceAndDist = std::pair<std::shared_ptr<const Face>, T>;

  // List of candidate faces.
  std::vector<FaceAndDist> candidateFaces;

  // Declaration of the BVH metadata attached to each node - this will be the distance to the node itself.
  using BVHMeta = T;

  // Shortest distance so far.
  BVHMeta shortestDistanceSoFar = std::numeric_limits<T>::max();

  EBGeometry::BVH::Visiter<Node, T> visiter = [&shortestDistanceSoFar](const Node&    a_node,
                                                                       const BVHMeta& a_bvDist) noexcept -> bool {
    return a_bvDist <= 0.0 || a_bvDist <= shortestDistanceSoFar;
  };

  EBGeometry::BVH::PackedSorter<T, K> sorter = [](std::array<std::pair<uint32_t, T>, K>& a_leaves) noexcept -> void {
    std::sort(
      a_leaves.begin(), a_leaves.end(), [](const std::pair<uint32_t, T>& n1, const std::pair<uint32_t, T>& n2) -> bool {
        return n1.second > n2.second;
      });
  };

  EBGeometry::BVH::MetaUpdater<Node, BVHMeta> metaUpdater = [&a_point](const Node& a_node) noexcept -> BVHMeta {
    return a_node.getDistanceToBoundingVolume(a_point);
  };

  EBGeometry::BVH::PackedUpdater<Face> updater =
    [&shortestDistanceSoFar, &a_point, &candidateFaces](
      const std::vector<std::shared_ptr<const Face>>& a_faces, size_t offset, size_t count) noexcept -> void {
#pragma GCC ivdep
    for (size_t i = offset; i < offset + count; i++) {
      const T distToFace = sqrt(a_faces[i]->unsignedDistance2(a_point));

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
  return (m_bvh);
}

template <class T, class Meta, size_t K>
const std::shared_ptr<EBGeometry::BVH::PackedBVH<T, EBGeometry::DCEL::FaceT<T, Meta>, K>>&
MeshSDF<T, Meta, K>::getRoot() const noexcept
{
  return (m_bvh);
}

template <class T, class Meta, size_t K>
EBGeometry::BoundingVolumes::AABBT<T>
MeshSDF<T, Meta, K>::computeBoundingVolume() const noexcept
{
  return m_bvh->getBoundingVolume();
};

template <class T, class Meta, size_t K, size_t W>
TriMeshSDF<T, Meta, K, W>::TriMeshSDF(const std::shared_ptr<Mesh>& a_mesh,
                                      const BVH::Build             a_build,
                                      const size_t                 a_maxLeafSize) noexcept
{
  using AABB   = EBGeometry::BoundingVolumes::AABBT<T>;
  using Tri    = Triangle<T, Meta>;
  using TriSoA = TriangleSoAT<T, W>;

  std::vector<std::shared_ptr<Tri>> triangles;

  for (const auto& f : a_mesh->getFaces()) {
    const auto normal   = f->getNormal();
    const auto vertices = f->gatherVertices();
    const auto edges    = f->gatherEdges();

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

  auto soa_grouper = [](const std::vector<std::shared_ptr<const Tri>>& srcPrims,
                        uint32_t                                       offset,
                        uint32_t                                       count) -> std::vector<TriSoA> {
    constexpr uint32_t  soa_w = static_cast<uint32_t>(W);
    std::vector<TriSoA> groups;
    const uint32_t      numGroups = (count + soa_w - 1U) / soa_w;
    groups.reserve(numGroups);
    for (uint32_t g = 0; g < numGroups; g++) {
      const uint32_t bOff   = offset + g * soa_w;
      const uint32_t bCount = std::min(soa_w, count - g * soa_w);
      Tri            trisArr[soa_w];
      for (uint32_t i = 0; i < bCount; i++)
        trisArr[i] = *srcPrims[bOff + i];
      TriSoA soa;
      soa.pack(trisArr, bCount);
      groups.push_back(std::move(soa));
    }
    return groups;
  };

  m_bvh = EBGeometry::buildTriMeshFullBVH<T, Meta, AABB, K>(triangles, a_build, a_maxLeafSize)
            ->template packWith<TriSoA>(soa_grouper);
}

template <class T, class Meta, size_t K, size_t W>
TriMeshSDF<T, Meta, K, W>::TriMeshSDF(const std::vector<std::shared_ptr<Tri>>& a_triangles,
                                      const BVH::Build                         a_build,
                                      const size_t                             a_maxLeafSize) noexcept
{
  using AABB   = EBGeometry::BoundingVolumes::AABBT<T>;
  using TriSoA = TriangleSoAT<T, W>;

  auto soa_grouper = [](const std::vector<std::shared_ptr<const Tri>>& srcPrims,
                        uint32_t                                       offset,
                        uint32_t                                       count) -> std::vector<TriSoA> {
    constexpr uint32_t  soa_w = static_cast<uint32_t>(W);
    std::vector<TriSoA> groups;
    const uint32_t      numGroups = (count + soa_w - 1U) / soa_w;
    groups.reserve(numGroups);
    for (uint32_t g = 0; g < numGroups; g++) {
      const uint32_t bOff   = offset + g * soa_w;
      const uint32_t bCount = std::min(soa_w, count - g * soa_w);
      Tri            trisArr[soa_w];
      for (uint32_t i = 0; i < bCount; i++)
        trisArr[i] = *srcPrims[bOff + i];
      TriSoA soa;
      soa.pack(trisArr, bCount);
      groups.push_back(std::move(soa));
    }
    return groups;
  };

  m_bvh = EBGeometry::buildTriMeshFullBVH<T, Meta, AABB, K>(a_triangles, a_build, a_maxLeafSize)
            ->template packWith<TriSoA>(soa_grouper);
}

template <class T, class Meta, size_t K, size_t W>
T
TriMeshSDF<T, Meta, K, W>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  return m_bvh->signedDistance(a_point);
}

template <class T, class Meta, size_t K, size_t W>
std::shared_ptr<EBGeometry::BVH::PackedBVH<T, EBGeometry::TriangleSoAT<T, W>, K>>&
TriMeshSDF<T, Meta, K, W>::getRoot() noexcept
{
  return (m_bvh);
}

template <class T, class Meta, size_t K, size_t W>
const std::shared_ptr<EBGeometry::BVH::PackedBVH<T, EBGeometry::TriangleSoAT<T, W>, K>>&
TriMeshSDF<T, Meta, K, W>::getRoot() const noexcept
{
  return (m_bvh);
}

template <class T, class Meta, size_t K, size_t W>
EBGeometry::BoundingVolumes::AABBT<T>
TriMeshSDF<T, Meta, K, W>::computeBoundingVolume() const noexcept
{
  return m_bvh->getBoundingVolume();
};

} // namespace EBGeometry

#endif
