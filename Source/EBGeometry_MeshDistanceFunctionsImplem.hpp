/* EBGeometry
 * Copyright © 2023 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file    EBGeometry_MeshDistanceFunctionsImplem.hpp
  @brief   Implementation of EBGeometry_MeshDistanceFunctions.hpp
  @author  Robert Marskar
*/

#ifndef EBGeometry_MeshDistanceFunctionsImplem
#define EBGeometry_MeshDistanceFunctionsImplem

// Our includes
#include "EBGeometry_MeshDistanceFunctions.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

template <class T, class Meta, class BV, size_t K>
std::shared_ptr<EBGeometry::BVH::NodeT<T, EBGeometry::DCEL::FaceT<T, Meta>, BV, K>>
DCEL::buildFullBVH(const std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>& a_dcelMesh,
                   const BVH::Build                                         a_build) noexcept
{
  using Prim          = EBGeometry::DCEL::FaceT<T, Meta>;
  using PrimAndBVList = std::vector<std::pair<std::shared_ptr<const Prim>, BV>>;

  // Create a pair-wise list of DCEL faces and their bounding volumes.
  PrimAndBVList primsAndBVs;

  for (const auto& f : a_dcelMesh->getFaces()) {
    primsAndBVs.emplace_back(std::make_pair(f, BV(f->getAllVertexCoordinates())));
  }

  // Partition the BVH using the default input arguments.
  auto bvh = std::make_shared<EBGeometry::BVH::NodeT<T, Prim, BV, K>>(primsAndBVs);

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
  default: {
    std::cerr << "EBGeometry::DCEL::buildFullBVH - unsupported build method requested" << std::endl;

    break;
  }
  }

  return bvh;
}

template <class T, class Meta, class BV, size_t K>
std::shared_ptr<EBGeometry::BVH::NodeT<T, Triangle<T, Meta>, BV, K>>
buildTriMeshFullBVH(const std::vector<std::shared_ptr<EBGeometry::Triangle<T, Meta>>>& a_triangles,
                    const BVH::Build a_build       = BVH::Build::TopDown,
                    const size_t     a_maxLeafSize = 4U) noexcept
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
  auto bvh = std::make_shared<EBGeometry::BVH::NodeT<T, Prim, BV, K>>(primsAndBVs);

  switch (a_build) {
  case BVH::Build::TopDown: {
    using Node     = EBGeometry::BVH::NodeT<T, Prim, BV, K>;
    using StopFunc = typename Node::StopFunction;
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
  default: {
    std::cerr << "EBGeometry::DCEL::buildFullBVH - unsupported build method requested" << std::endl;

    break;
  }
  }

  return bvh;
}

template <class T, class Meta>
MeshSDF<T, Meta>::MeshSDF(const std::shared_ptr<Mesh>& a_mesh) noexcept
{
  m_mesh = a_mesh;
}

template <class T, class Meta>
T
MeshSDF<T, Meta>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  return m_mesh->signedDistance(a_point);
}

template <class T, class Meta>
const std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>
MeshSDF<T, Meta>::getMesh() const noexcept
{
  return m_mesh;
}

template <class T, class Meta>
template <class BV>
BV
MeshSDF<T, Meta>::computeBoundingVolume() const noexcept
{
  return BV(m_mesh->getAllVertexCoordinates());
};

template <class T, class Meta, class BV, size_t K>
FastMeshSDF<T, Meta, BV, K>::FastMeshSDF(const std::shared_ptr<Mesh>& a_mesh, const BVH::Build a_build) noexcept
{
  m_bvh = EBGeometry::DCEL::buildFullBVH<T, Meta, BV, K>(a_mesh, a_build);
}

template <class T, class Meta, class BV, size_t K>
T
FastMeshSDF<T, Meta, BV, K>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  T minDist = std::numeric_limits<T>::max();

  BVH::Updater<Face> updater = [&minDist,
                                &a_point](const std::vector<std::shared_ptr<const Face>>& faces) noexcept -> void {
    for (const auto& f : faces) {
      const T curDist = f->signedDistance(a_point);

      minDist = std::abs(curDist) < std::abs(minDist) ? curDist : minDist;
    }
  };

  BVH::Visiter<Node, T> visiter = [&minDist](const Node& a_node, const T& a_bvDist) noexcept -> bool {
    return a_bvDist <= std::abs(minDist);
  };

  BVH::Sorter<Node, T, K> sorter =
    [](std::array<std::pair<std::shared_ptr<const Node>, T>, K>& a_leaves) noexcept -> void {
    std::sort(a_leaves.begin(),
              a_leaves.end(),
              [](const std::pair<std::shared_ptr<const Node>, T>& n1,
                 const std::pair<std::shared_ptr<const Node>, T>& n2) -> bool { return n1.second > n2.second; });
  };

  BVH::MetaUpdater<Node, T> metaUpdater = [&a_point](const Node& a_node) noexcept -> T {
    return a_node.getDistanceToBoundingVolume(a_point);
  };

  // Traverse the tree.
  m_bvh->traverse(updater, visiter, sorter, metaUpdater);

  return minDist;
}

template <class T, class Meta, class BV, size_t K>
std::vector<std::pair<std::shared_ptr<const EBGeometry::DCEL::FaceT<T, Meta>>, T>>
FastMeshSDF<T, Meta, BV, K>::getClosestFaces(const Vec3T<T>& a_point, const bool a_sorted) const noexcept
{
  using FaceAndDist = std::pair<std::shared_ptr<const Face>, T>;

  // List of candidate faces.
  std::vector<FaceAndDist> candidateFaces;

  // Declaration of the BVH metadata attached to each node - this will be the distance to the node itself.
  using BVHMeta = T;

  // Shortest distance so far.
  BVHMeta shortestDistanceSoFar = std::numeric_limits<T>::max();

  // Visitation pattern - go into the node if the point is inside or the distance to the BV is shorter than
  // the shortest distance so far.
  EBGeometry::BVH::Visiter<Node, T> visiter = [&shortestDistanceSoFar](const Node&    a_node,
                                                                       const BVHMeta& a_bvDist) noexcept -> bool {
    return a_bvDist <= 0.0 || a_bvDist <= shortestDistanceSoFar;
  };

  // Sorter for BVH nodes, visit closest nodes first
  EBGeometry::BVH::Sorter<Node, T, K> sorter =
    [](std::array<std::pair<std::shared_ptr<const Node>, T>, K>& a_leaves) noexcept -> void {
    std::sort(a_leaves.begin(),
              a_leaves.end(),
              [](const std::pair<std::shared_ptr<const Node>, T>& n1,
                 const std::pair<std::shared_ptr<const Node>, T>& n2) -> bool { return n1.second > n2.second; });
  };

  // Meta-data updater - this meta-data enters into the visitor pattern.
  EBGeometry::BVH::MetaUpdater<Node, BVHMeta> metaUpdater = [&a_point](const Node& a_node) noexcept -> BVHMeta {
    return a_node.getDistanceToBoundingVolume(a_point);
  };

  // Update rule for the BVH. Go through the faces and check
  EBGeometry::BVH::Updater<Face> updater = [&shortestDistanceSoFar, &a_point, &candidateFaces](
                                             const std::vector<std::shared_ptr<const Face>>& a_faces) noexcept -> void {
    // Calculate the distance to each face in the leaf node. If it is shorter than the shortest distance so far, add this face
    // to the list of faces and update the shortest distance.
    for (const auto& f : a_faces) {
      const T distToFace = sqrt(f->unsignedDistance2(a_point));

      if (distToFace <= shortestDistanceSoFar) {
        candidateFaces.emplace_back(f, distToFace);

        shortestDistanceSoFar = distToFace;
      }
    }
  };

  // Traverse the tree
  m_bvh->traverse(updater, visiter, sorter, metaUpdater);

  // Sort if the user asks for it
  if (a_sorted) {
    std::sort(candidateFaces.begin(), candidateFaces.end(), [](const FaceAndDist& a, const FaceAndDist& b) {
      return a.second < b.second;
    });
  }

  return candidateFaces;
}

template <class T, class Meta, class BV, size_t K>
std::shared_ptr<EBGeometry::BVH::NodeT<T, EBGeometry::DCEL::FaceT<T, Meta>, BV, K>>&
FastMeshSDF<T, Meta, BV, K>::getBVH() noexcept
{
  return (m_bvh);
}

template <class T, class Meta, class BV, size_t K>
const std::shared_ptr<EBGeometry::BVH::NodeT<T, EBGeometry::DCEL::FaceT<T, Meta>, BV, K>>&
FastMeshSDF<T, Meta, BV, K>::getBVH() const noexcept
{
  return (m_bvh);
}

template <class T, class Meta, class BV, size_t K>
BV
FastMeshSDF<T, Meta, BV, K>::computeBoundingVolume() const noexcept
{
  return m_bvh->getBoundingVolume();
};

template <class T, class Meta, class BV, size_t K>
FastCompactMeshSDF<T, Meta, BV, K>::FastCompactMeshSDF(const std::shared_ptr<Mesh>& a_mesh,
                                                       const BVH::Build             a_build) noexcept
{
  const auto bvh = EBGeometry::DCEL::buildFullBVH<T, Meta, BV, K>(a_mesh, a_build);

  m_bvh = bvh->flattenTree();
}

template <class T, class Meta, class BV, size_t K>
T
FastCompactMeshSDF<T, Meta, BV, K>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  T minDist = std::numeric_limits<T>::max();

  BVH::LinearUpdater<Face> updater =
    [&minDist, &a_point](const std::vector<std::shared_ptr<const Face>>& faces,
                         size_t                                           offset,
                         size_t                                           count) noexcept -> void {
#pragma GCC ivdep
    for (size_t i = offset; i < offset + count; i++) {
      const T curDist = faces[i]->signedDistance(a_point);
      minDist         = std::abs(curDist) < std::abs(minDist) ? curDist : minDist;
    }
  };

  BVH::Visiter<Node, T> visiter = [&minDist](const Node& a_node, const T& a_bvDist) noexcept -> bool {
    return a_bvDist <= std::abs(minDist);
  };

  BVH::LinearSorter<T, K> sorter =
    [](std::array<std::pair<uint32_t, T>, K>& a_leaves) noexcept -> void {
    std::sort(a_leaves.begin(),
              a_leaves.end(),
              [](const std::pair<uint32_t, T>& n1, const std::pair<uint32_t, T>& n2) -> bool {
                return n1.second > n2.second;
              });
  };

  BVH::MetaUpdater<Node, T> metaUpdater = [&a_point](const Node& a_node) noexcept -> T {
    return a_node.getDistanceToBoundingVolume(a_point);
  };

  m_bvh->traverse(updater, visiter, sorter, metaUpdater);

  return minDist;
}

template <class T, class Meta, class BV, size_t K>
std::vector<std::pair<std::shared_ptr<const EBGeometry::DCEL::FaceT<T, Meta>>, T>>
FastCompactMeshSDF<T, Meta, BV, K>::getClosestFaces(const Vec3T<T>& a_point, const bool a_sorted) const noexcept
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

  EBGeometry::BVH::LinearSorter<T, K> sorter =
    [](std::array<std::pair<uint32_t, T>, K>& a_leaves) noexcept -> void {
    std::sort(a_leaves.begin(),
              a_leaves.end(),
              [](const std::pair<uint32_t, T>& n1, const std::pair<uint32_t, T>& n2) -> bool {
                return n1.second > n2.second;
              });
  };

  EBGeometry::BVH::MetaUpdater<Node, BVHMeta> metaUpdater = [&a_point](const Node& a_node) noexcept -> BVHMeta {
    return a_node.getDistanceToBoundingVolume(a_point);
  };

  EBGeometry::BVH::LinearUpdater<Face> updater =
    [&shortestDistanceSoFar, &a_point, &candidateFaces](const std::vector<std::shared_ptr<const Face>>& a_faces,
                                                         size_t                                          offset,
                                                         size_t                                          count) noexcept -> void {
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

template <class T, class Meta, class BV, size_t K>
std::shared_ptr<EBGeometry::BVH::LinearBVH<T, EBGeometry::DCEL::FaceT<T, Meta>, BV, K>>&
FastCompactMeshSDF<T, Meta, BV, K>::getRoot() noexcept
{
  return (m_bvh);
}

template <class T, class Meta, class BV, size_t K>
const std::shared_ptr<EBGeometry::BVH::LinearBVH<T, EBGeometry::DCEL::FaceT<T, Meta>, BV, K>>&
FastCompactMeshSDF<T, Meta, BV, K>::getRoot() const noexcept
{
  return (m_bvh);
}

template <class T, class Meta, class BV, size_t K>
BV
FastCompactMeshSDF<T, Meta, BV, K>::computeBoundingVolume() const noexcept
{
  return m_bvh->getBoundingVolume();
};

template <class T, class Meta, class BV, size_t K>
FastTriMeshSDF<T, Meta, BV, K>::FastTriMeshSDF(const std::shared_ptr<Mesh>& a_mesh,
                                               const BVH::Build             a_build,
                                               const size_t                 a_maxLeafSize) noexcept
{
  // Turn the input mesh into triangles
  std::vector<std::shared_ptr<Triangle<T, Meta>>> triangles;

  for (const auto& f : a_mesh->getFaces()) {
    const auto normal   = f->getNormal();
    const auto vertices = f->gatherVertices();
    const auto edges    = f->gatherEdges();

    if ((vertices.size() != 3) || (edges.size() != 3)) {
      std::cerr << "FastTriMeshSDF::readIntoTriangles -- DCEL mesh not composed of only triangles!" << "\n";
    }

    auto tri = std::make_shared<Triangle<T, Meta>>();
    tri->setNormal(normal);
    tri->setVertexPositions({vertices[0]->getPosition(), vertices[1]->getPosition(), vertices[2]->getPosition()});
    tri->setVertexNormals({vertices[0]->getNormal(), vertices[1]->getNormal(), vertices[2]->getNormal()});
    tri->setEdgeNormals({edges[0]->getNormal(), edges[1]->getNormal(), edges[2]->getNormal()});
    tri->setMetaData(f->getMetaData());
    triangles.emplace_back(tri);
  }

  auto bvh = EBGeometry::buildTriMeshFullBVH<T, Meta, BV, K>(triangles, a_build, a_maxLeafSize);
  m_bvh    = bvh->flattenTree();

  const auto& prims = m_bvh->getPrimitives();
  m_triangleValues.reserve(prims.size());
  for (const auto& p : prims) {
    m_triangleValues.emplace_back(*p);
  }

  // Build flat SoA array: ceil(count/4) groups per leaf, packed contiguously.
  constexpr uint32_t W  = 4;
  const auto&        nds = m_bvh->getLinearNodes();
  m_leafSoaStart.resize(m_triangleValues.size(), UINT32_MAX);
  m_leafSoaCount.resize(m_triangleValues.size(), 0U);
  for (const auto& node : nds) {
    if (node.isLeaf()) {
      const uint32_t off       = node.getPrimitivesOffset();
      const uint32_t cnt       = node.getNumPrimitives();
      const uint32_t numGroups = (cnt + W - 1U) / W;
      m_leafSoaStart[off]      = static_cast<uint32_t>(m_soaFlat.size());
      m_leafSoaCount[off]      = numGroups;
      for (uint32_t g = 0; g < numGroups; g++) {
        const uint32_t bOff   = off + g * W;
        const uint32_t bCount = std::min(W, cnt - g * W);
        TriSoA         soa;
        soa.pack(m_triangleValues.data() + bOff, bCount);
        m_soaFlat.emplace_back(std::move(soa));
      }
    }
  }
}

template <class T, class Meta, class BV, size_t K>
FastTriMeshSDF<T, Meta, BV, K>::FastTriMeshSDF(const std::vector<std::shared_ptr<Tri>>& a_triangles,
                                               const BVH::Build                         a_build,
                                               const size_t                             a_maxLeafSize) noexcept
{
  auto bvh = EBGeometry::buildTriMeshFullBVH<T, Meta, BV, K>(a_triangles, a_build, a_maxLeafSize);
  m_bvh    = bvh->flattenTree();

  const auto& prims = m_bvh->getPrimitives();
  m_triangleValues.reserve(prims.size());
  for (const auto& p : prims) {
    m_triangleValues.emplace_back(*p);
  }

  constexpr uint32_t W   = 4;
  const auto&        nds = m_bvh->getLinearNodes();
  m_leafSoaStart.resize(m_triangleValues.size(), UINT32_MAX);
  m_leafSoaCount.resize(m_triangleValues.size(), 0U);
  for (const auto& node : nds) {
    if (node.isLeaf()) {
      const uint32_t off       = node.getPrimitivesOffset();
      const uint32_t cnt       = node.getNumPrimitives();
      const uint32_t numGroups = (cnt + W - 1U) / W;
      m_leafSoaStart[off]      = static_cast<uint32_t>(m_soaFlat.size());
      m_leafSoaCount[off]      = numGroups;
      for (uint32_t g = 0; g < numGroups; g++) {
        const uint32_t bOff   = off + g * W;
        const uint32_t bCount = std::min(W, cnt - g * W);
        TriSoA         soa;
        soa.pack(m_triangleValues.data() + bOff, bCount);
        m_soaFlat.emplace_back(std::move(soa));
      }
    }
  }
}

template <class T, class Meta, class BV, size_t K>
T
FastTriMeshSDF<T, Meta, BV, K>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  T minDist = std::numeric_limits<T>::max();

  BVH::LinearUpdater<Tri> updater =
    [this, &minDist, &a_point](const std::vector<std::shared_ptr<const Tri>>&,
                               size_t                                           offset,
                               size_t) noexcept -> void {
    const uint32_t soaStart = m_leafSoaStart[offset];
    const uint32_t soaEnd   = soaStart + m_leafSoaCount[offset];
    for (uint32_t g = soaStart; g < soaEnd; g++) {
      const T curDist = m_soaFlat[g].signedDistance(a_point);
      if (std::abs(curDist) < std::abs(minDist)) minDist = curDist;
    }
  };

  m_bvh->traverseSimd(updater, minDist, a_point);

  return minDist;
}

template <class T, class Meta, class BV, size_t K>
std::vector<std::pair<std::shared_ptr<const Triangle<T, Meta>>, T>>
FastTriMeshSDF<T, Meta, BV, K>::getClosestTriangles(const Vec3T<T>& a_point, const bool a_sorted) const noexcept
{
  using TriAndDist = std::pair<std::shared_ptr<const Tri>, T>;

  // List of candidate faces.
  std::vector<TriAndDist> candidateTriangles;

  // Declaration of the BVH metadata attached to each node - this will be the distance to the node itself.
  using BVHMeta = T;

  // Shortest distance so far.
  BVHMeta shortestDistanceSoFar = std::numeric_limits<T>::max();

  EBGeometry::BVH::Visiter<Node, T> visiter = [&shortestDistanceSoFar](const Node&    a_node,
                                                                       const BVHMeta& a_bvDist) noexcept -> bool {
    return a_bvDist <= 0.0 || a_bvDist <= shortestDistanceSoFar;
  };

  EBGeometry::BVH::LinearSorter<T, K> sorter =
    [](std::array<std::pair<uint32_t, T>, K>& a_leaves) noexcept -> void {
    std::sort(a_leaves.begin(),
              a_leaves.end(),
              [](const std::pair<uint32_t, T>& n1, const std::pair<uint32_t, T>& n2) -> bool {
                return n1.second > n2.second;
              });
  };

  EBGeometry::BVH::MetaUpdater<Node, BVHMeta> metaUpdater = [&a_point](const Node& a_node) noexcept -> BVHMeta {
    return a_node.getDistanceToBoundingVolume(a_point);
  };

  EBGeometry::BVH::LinearUpdater<Tri> updater =
    [&shortestDistanceSoFar, &a_point, &candidateTriangles](const std::vector<std::shared_ptr<const Tri>>& a_tris,
                                                             size_t                                         offset,
                                                             size_t                                         count) noexcept -> void {
#pragma GCC ivdep
    for (size_t i = offset; i < offset + count; i++) {
      const T distToTri = std::abs(a_tris[i]->signedDistance(a_point));

      if (distToTri <= shortestDistanceSoFar) {
        candidateTriangles.emplace_back(a_tris[i], distToTri);
        shortestDistanceSoFar = distToTri;
      }
    }
  };

  m_bvh->traverse(updater, visiter, sorter, metaUpdater);

  if (a_sorted) {
    std::sort(candidateTriangles.begin(), candidateTriangles.end(), [](const TriAndDist& a, const TriAndDist& b) {
      return a.second < b.second;
    });
  }

  return candidateTriangles;
}

template <class T, class Meta, class BV, size_t K>
std::shared_ptr<EBGeometry::BVH::LinearBVH<T, EBGeometry::Triangle<T, Meta>, BV, K>>&
FastTriMeshSDF<T, Meta, BV, K>::getRoot() noexcept
{
  return (m_bvh);
}

template <class T, class Meta, class BV, size_t K>
const std::shared_ptr<EBGeometry::BVH::LinearBVH<T, EBGeometry::Triangle<T, Meta>, BV, K>>&
FastTriMeshSDF<T, Meta, BV, K>::getRoot() const noexcept
{
  return (m_bvh);
}

template <class T, class Meta, class BV, size_t K>
BV
FastTriMeshSDF<T, Meta, BV, K>::computeBoundingVolume() const noexcept
{
  return m_bvh->getBoundingVolume();
};

#include "EBGeometry_NamespaceFooter.hpp"

#endif
