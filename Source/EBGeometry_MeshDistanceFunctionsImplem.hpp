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

template <class T>
MeshSDF<T>::MeshSDF(const std::shared_ptr<Mesh>& a_mesh) noexcept
{
  m_mesh = a_mesh;
}

template <class T>
T
MeshSDF<T>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  return m_mesh->signedDistance(a_point);
}

template <class T>
const std::shared_ptr<EBGeometry::DCEL::MeshT<T>>
MeshSDF<T>::getMesh() const noexcept
{
  return m_mesh;
}

template <class T, class BV, size_t K>
FastMeshSDF<T, BV, K>::FastMeshSDF(const std::shared_ptr<Mesh>& a_mesh) noexcept
{
  m_bvh = EBGeometry::DCEL::buildFullBVH<T, BV, K>(a_mesh);
}

template <class T, class BV, size_t K>
T
FastMeshSDF<T, BV, K>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  T minDist = std::numeric_limits<T>::infinity();

  BVH::Updater<Face> updater = [&minDist, &a_point](const std::vector<std::shared_ptr<const Face>>& faces) -> void {
    for (const auto& f : faces) {
      const T curDist = f->signedDistance(a_point);

      minDist = std::abs(curDist) < std::abs(minDist) ? curDist : minDist;
    }
  };

  BVH::Visiter<Node, T> visiter = [&minDist, &a_point](const Node& a_node, const T& a_bvDist) -> bool {
    return a_bvDist <= std::abs(minDist);
  };

  BVH::Sorter<Node, T, K> sorter =
    [&a_point](std::array<std::pair<std::shared_ptr<const Node>, T>, K>& a_leaves) -> void {
    std::sort(
      a_leaves.begin(),
      a_leaves.end(),
      [&a_point](const std::pair<std::shared_ptr<const Node>, T>& n1,
                 const std::pair<std::shared_ptr<const Node>, T>& n2) -> bool { return n1.second > n2.second; });
  };

  BVH::MetaUpdater<Node, T> metaUpdater = [&a_point](const Node& a_node) -> T {
    return a_node.getDistanceToBoundingVolume(a_point);
  };

  // Traverse the tree.
  m_bvh->traverse(updater, visiter, sorter, metaUpdater);

  return minDist;
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
  T minDist = std::numeric_limits<T>::infinity();

  BVH::Updater<Face> updater = [&minDist, &a_point](const std::vector<std::shared_ptr<const Face>>& faces) -> void {
    for (const auto& f : faces) {
      const T curDist = f->signedDistance(a_point);

      minDist = std::abs(curDist) < std::abs(minDist) ? curDist : minDist;
    }
  };

  BVH::Visiter<Node, T> visiter = [&minDist, &a_point](const Node& a_node, const T& a_bvDist) -> bool {
    return a_bvDist <= std::abs(minDist);
  };

  BVH::Sorter<Node, T, K> sorter =
    [&a_point](std::array<std::pair<std::shared_ptr<const Node>, T>, K>& a_leaves) -> void {
    std::sort(
      a_leaves.begin(),
      a_leaves.end(),
      [&a_point](const std::pair<std::shared_ptr<const Node>, T>& n1,
                 const std::pair<std::shared_ptr<const Node>, T>& n2) -> bool { return n1.second > n2.second; });
  };

  BVH::MetaUpdater<Node, T> metaUpdater = [&a_point](const Node& a_node) -> T {
    return a_node.getDistanceToBoundingVolume(a_point);
  };

  m_bvh->traverse(updater, visiter, sorter, metaUpdater);

  return minDist;
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
