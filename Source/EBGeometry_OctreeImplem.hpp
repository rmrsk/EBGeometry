/* EBGeometry
 * Copyright © 2023 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_OctreeImplem.hpp
  @brief  Implementation of EBGeometry_Octree.hpp
  @author Robert Marskar
*/

#ifndef EBGeometry_OctreeImplem
#define EBGeometry_OctreeImplem

// Our includes
#include "EBGeometry_Octree.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

template <typename Meta, typename Data>
Octree<Meta, Data>::Octree()
{
  for (auto& c : m_children) {
    c = nullptr;
  }
}

template <typename Meta, typename Data>
Octree<Meta, Data>::~Octree()
{}

template <typename Meta, typename Data>
inline const std::array<std::shared_ptr<Octree<Meta, Data>>, 8>&
Octree<Meta, Data>::getChildren() const noexcept
{
  return m_children;
}

template <typename Meta, typename Data>
inline std::array<std::shared_ptr<Octree<Meta, Data>>, 8>&
Octree<Meta, Data>::getChildren() noexcept
{
  return m_children;
}

template <typename Meta, typename Data>
inline bool
Octree<Meta, Data>::isLeaf() const noexcept
{
  return m_children[0] == nullptr;
}

template <typename Meta, typename Data>
inline void
Octree<Meta, Data>::traverse() const noexcept
{}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
