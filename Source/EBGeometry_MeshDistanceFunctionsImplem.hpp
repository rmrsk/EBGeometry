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
  T shortestDistance = std::numeric_limits<T>::infinity();

  // Pruner, for deciding when to visit a branch. If this returns true we visit the branch.
  EBGeometry::BVH::Pruner<T> pruner = [d = &shortestDistance](const T& bvDist, const T& minDist) -> bool {
    return bvDist <= std::abs(minDist);
  };

  // Comparator, for updating the shortest distance.
  EBGeometry::BVH::Comparator<T> comparator = [&D = shortestDistance](const T&              dmin,
                                                                      const std::vector<T>& primDistances) -> T {
    T d = dmin;

    for (const auto& primDist : primDistances) {
      d = std::abs(primDist) < std::abs(d) ? primDist : d;
    }

    D = d;

    return d;
  };

  // Traverse the tree.
  m_bvh->stackPrune(a_point, comparator, pruner);

  return shortestDistance;

#else
  return m_bvh->signedDistance(a_point);

#endif
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
