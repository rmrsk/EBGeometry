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

template <class T, class BV, size_t K>
FastMeshSDF<T, BV, K>::FastMeshSDF(const std::shared_ptr<Mesh>& a_mesh) noexcept
{
  m_bvh = EBGeometry::DCEL::buildFullBVH<T, BV, K>(a_mesh);
}

template <class T, class BV, size_t K>
T
FastMeshSDF<T, BV, K>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  return m_bvh->signedDistance(a_point);
}

template <class T, class BV, size_t K>
FastCompactMeshSDF<T, BV, K>::FastCompactMeshSDF(const std::shared_ptr<Mesh>& a_mesh) noexcept
{
  const auto bvh = EBGeometry::DCEL::buildFullBVH<T, BV, K>(a_mesh);

  m_bvh = bvh->flattenTree();
}

template <class T, class BV, size_t K>
T
FastCompactMeshSDF<T, BV, K>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
#if 1
  T minDist = std::numeric_limits<T>::infinity();

  // Lambda for updating the shortest distance.
  BVH::Updater<Face> updater = [&minDist, &a_point](const std::vector<std::shared_ptr<const Face>>& faces) -> void {
    for (const auto& f : faces) {
      const T curDist = f->signedDistance(a_point);

      minDist = std::abs(curDist) < std::abs(minDist) ? curDist : minDist;
    }
  };

  // Lambda for pruning nodes. Returns true if we should visit the node.
  BVH::Visiter<Node> visiter = [&minDist, &a_point](const Node& a_node) -> bool {
    return a_node.getDistanceToBoundingVolume(a_point) <= std::abs(minDist);
  };

  // Sort criterion.
  BVH::Sorter<Node, K> sorter = [&a_point](std::array<std::shared_ptr<const Node>, K>& a_leaves) -> void {
    std::sort(a_leaves.begin(),
              a_leaves.end(),
              [&a_point](const std::shared_ptr<const Node>& n1, const std::shared_ptr<const Node>& n2) -> bool {
                return n1->getDistanceToBoundingVolume(a_point) > n2->getDistanceToBoundingVolume(a_point);
              });
  };

  // Traverse the tree.
  //  return m_bvh->signedDistance(a_point);
  m_bvh->traverse(updater, visiter, sorter);

  return minDist;

#else
  return m_bvh->signedDistance(a_point);

#endif
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
