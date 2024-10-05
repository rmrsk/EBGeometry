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

// Std includes
#include <set>

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
  T minDist = std::numeric_limits<T>::infinity();

  BVH::Updater<Face> updater = [&minDist,
                                &a_point](const std::vector<std::shared_ptr<const Face>>& faces) noexcept -> void {
    for (const auto& f : faces) {
      const T curDist = f->signedDistance(a_point);

      minDist = std::abs(curDist) < std::abs(minDist) ? curDist : minDist;
    }
  };

  BVH::Visiter<Node, T> visiter = [&minDist, &a_point](const Node& a_node, const T& a_bvDist) noexcept -> bool {
    return a_bvDist <= std::abs(minDist);
  };

  BVH::Sorter<Node, T, K> sorter =
    [&a_point](std::array<std::pair<std::shared_ptr<const Node>, T>, K>& a_leaves) noexcept -> void {
    std::sort(
      a_leaves.begin(),
      a_leaves.end(),
      [&a_point](const std::pair<std::shared_ptr<const Node>, T>& n1,
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
  BVHMeta shortestDistanceSoFar = std::numeric_limits<T>::infinity();

  // Visitation pattern - go into the node if the point is inside or the distance to the BV is shorter than
  // the shortest distance so far.
  EBGeometry::BVH::Visiter<Node, T> visiter = [&shortestDistanceSoFar](const Node&    a_node,
                                                                       const BVHMeta& a_bvDist) noexcept -> bool {
    return a_bvDist <= 0.0 || a_bvDist <= shortestDistanceSoFar;
  };

  // Sorter for BVH nodes, visit closest nodes first
  EBGeometry::BVH::Sorter<Node, T, K> sorter =
    [&a_point](std::array<std::pair<std::shared_ptr<const Node>, T>, K>& a_leaves) noexcept -> void {
    std::sort(
      a_leaves.begin(),
      a_leaves.end(),
      [&a_point](const std::pair<std::shared_ptr<const Node>, T>& n1,
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
std::set<std::pair<std::shared_ptr<const DCEL::FaceT<T, Meta>>, std::shared_ptr<const DCEL::FaceT<T, Meta>>>>
FastMeshSDF<T, Meta, BV, K>::getIntersectingFaces(const std::shared_ptr<Mesh>& a_mesh) const noexcept
{
  using FacePtr  = std::shared_ptr<const Face>;
  using FacePair = std::pair<FacePtr, FacePtr>;
  using BVHMeta  = bool;

  auto comparator = [](FacePair A, FacePair B) -> bool {
    if (A.first > A.second) {
      A = std::make_pair(A.second, A.first);
    }
    if (B.first > B.second) {
      B = std::make_pair(B.second, B.first);
    }

    return A.first < B.first;
  };

  std::set<FacePair, decltype(comparator)> uniqueIntersections(comparator);

  // Iterate through the input faces.
  for (const auto& curFace : a_mesh->getFaces()) {

    // Construct the bounding volume for this face.
    const auto faceBV = BV(curFace->getAllVertexCoordinates());

    // Visit pattern -- need to visit the node if it's bounding volume intersects with the face
    // bounding volume. The intersection test is done in the meta-updater.
    EBGeometry::BVH::Visiter<Node, BVHMeta> visiter = [](const Node& a_node, const BVHMeta& a_meta) noexcept -> bool {
      return a_meta;
    };

    // Sorting function for child visit pattern - no need to visit in any particular order in
    // our case
    EBGeometry::BVH::Sorter<Node, BVHMeta, K> sorter =
      [](std::array<std::pair<std::shared_ptr<const Node>, BVHMeta>, K>& a_children) noexcept -> void {

    };

    // Meta-updater - this data is what enters the visitor pattern but it is not required
    // in our case.
    EBGeometry::BVH::MetaUpdater<Node, BVHMeta> metaUpdater = [&faceBV](const Node& a_node) noexcept -> BVHMeta {
      const auto nodeBV = a_node.getBoundingVolume();

      return nodeBV.intersects(faceBV);
    };

    // Update rule for the BVH leaf nodes. This runs through the faces in the node and
    // checks if they intersect the current face.
    EBGeometry::BVH::Updater<Face> updater =
      [&uniqueIntersections, &curFace](const std::vector<std::shared_ptr<const Face>>& a_leafFaces) noexcept -> void {
      for (const auto& leafFace : a_leafFaces) {
        if (leafFace != curFace) {
          if (curFace->intersects(*leafFace)) {
            uniqueIntersections.insert({curFace, leafFace});
          }
        }
      }
    };

    // Traverse the tree.
    m_bvh->traverse(updater, visiter, sorter, metaUpdater);
  }

  // Return set of unique intersecting faces.
  std::set<FacePair> intersectingFaces;

  for (const auto& i : uniqueIntersections) {
    intersectingFaces.insert(i);
  }

  return intersectingFaces;
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
  T minDist = std::numeric_limits<T>::infinity();

  BVH::Updater<Face> updater = [&minDist,
                                &a_point](const std::vector<std::shared_ptr<const Face>>& faces) noexcept -> void {
    for (const auto& f : faces) {
      const T curDist = f->signedDistance(a_point);

      minDist = std::abs(curDist) < std::abs(minDist) ? curDist : minDist;
    }
  };

  BVH::Visiter<Node, T> visiter = [&minDist, &a_point](const Node& a_node, const T& a_bvDist) noexcept -> bool {
    return a_bvDist <= std::abs(minDist);
  };

  BVH::Sorter<Node, T, K> sorter =
    [&a_point](std::array<std::pair<std::shared_ptr<const Node>, T>, K>& a_leaves) noexcept -> void {
    std::sort(
      a_leaves.begin(),
      a_leaves.end(),
      [&a_point](const std::pair<std::shared_ptr<const Node>, T>& n1,
                 const std::pair<std::shared_ptr<const Node>, T>& n2) -> bool { return n1.second > n2.second; });
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
  BVHMeta shortestDistanceSoFar = std::numeric_limits<T>::infinity();

  // Visitation pattern - go into the node if the point is inside or the distance to the BV is shorter than
  // the shortest distance so far.
  EBGeometry::BVH::Visiter<Node, T> visiter = [&shortestDistanceSoFar](const Node&    a_node,
                                                                       const BVHMeta& a_bvDist) noexcept -> bool {
    return a_bvDist <= 0.0 || a_bvDist <= shortestDistanceSoFar;
  };

  // Sorter for BVH nodes, visit closest nodes first
  EBGeometry::BVH::Sorter<Node, T, K> sorter =
    [&a_point](std::array<std::pair<std::shared_ptr<const Node>, T>, K>& a_leaves) noexcept -> void {
    std::sort(
      a_leaves.begin(),
      a_leaves.end(),
      [&a_point](const std::pair<std::shared_ptr<const Node>, T>& n1,
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
std::set<std::pair<std::shared_ptr<const DCEL::FaceT<T, Meta>>, std::shared_ptr<const DCEL::FaceT<T, Meta>>>>
FastCompactMeshSDF<T, Meta, BV, K>::getIntersectingFaces(const std::shared_ptr<Mesh>& a_mesh) const noexcept
{
  using FacePtr  = std::shared_ptr<const Face>;
  using FacePair = std::pair<FacePtr, FacePtr>;
  using BVHMeta  = bool;

  auto comparator = [](FacePair A, FacePair B) -> bool {
    if (A.first > A.second) {
      A = std::make_pair(A.second, A.first);
    }
    if (B.first > B.second) {
      B = std::make_pair(B.second, B.first);
    }

    return A.first < B.first;
  };

  std::set<FacePair, decltype(comparator)> uniqueIntersections(comparator);

  // Iterate through the input faces.
  for (const auto& curFace : a_mesh->getFaces()) {

    // Construct the bounding volume for this face.
    const auto faceBV = BV(curFace->getAllVertexCoordinates());

    // Visit pattern -- need to visit the node if it's bounding volume intersects with the face
    // bounding volume. The intersection test is done in the meta-updater.
    EBGeometry::BVH::Visiter<Node, BVHMeta> visiter = [](const Node& a_node, const BVHMeta& a_meta) noexcept -> bool {
      return a_meta;
    };

    // Sorting function for child visit pattern - no need to visit in any particular order in
    // our case
    EBGeometry::BVH::Sorter<Node, BVHMeta, K> sorter =
      [](std::array<std::pair<std::shared_ptr<const Node>, BVHMeta>, K>& a_children) noexcept -> void {

    };

    // Meta-updater - this data is what enters the visitor pattern but it is not required
    // in our case.
    EBGeometry::BVH::MetaUpdater<Node, BVHMeta> metaUpdater = [&faceBV](const Node& a_node) noexcept -> BVHMeta {
      const auto nodeBV = a_node.getBoundingVolume();

      return nodeBV.intersects(faceBV);
    };

    // Update rule for the BVH leaf nodes. This runs through the faces in the node and
    // checks if they intersect the current face.
    EBGeometry::BVH::Updater<Face> updater =
      [&uniqueIntersections, &curFace](const std::vector<std::shared_ptr<const Face>>& a_leafFaces) noexcept -> void {
      for (const auto& leafFace : a_leafFaces) {
        if (leafFace != curFace) {
          if (curFace->intersects(*leafFace)) {
            uniqueIntersections.insert({curFace, leafFace});
          }
        }
      }
    };

    // Traverse the tree.
    m_bvh->traverse(updater, visiter, sorter, metaUpdater);
  }

  // Return set of unique intersecting faces.
  std::set<FacePair> intersectingFaces;

  for (const auto& i : uniqueIntersections) {
    intersectingFaces.insert(i);
  }

  return intersectingFaces;
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

#include "EBGeometry_NamespaceFooter.hpp"

#endif
