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

namespace Octree {

  template <typename Meta, typename Data>
  Node<Meta, Data>::Node()
  {
    for (auto& c : m_children) {
      c = nullptr;
    }
  }

  template <typename Meta, typename Data>
  Node<Meta, Data>::~Node()
  {}

  template <typename Meta, typename Data>
  inline const std::array<std::shared_ptr<Node<Meta, Data>>, 8>&
  Node<Meta, Data>::getChildren() const noexcept
  {
    return m_children;
  }

  template <typename Meta, typename Data>
  inline std::array<std::shared_ptr<Node<Meta, Data>>, 8>&
  Node<Meta, Data>::getChildren() noexcept
  {
    return m_children;
  }

  template <typename Meta, typename Data>
  inline Meta&
  Node<Meta, Data>::getMeta() noexcept
  {
    return m_meta;
  }

  template <typename Meta, typename Data>
  inline const Meta&
  Node<Meta, Data>::getMeta() const noexcept
  {
    return m_meta;
  }

  template <typename Meta, typename Data>
  inline std::shared_ptr<Data>&
  Node<Meta, Data>::getData() noexcept
  {
    return m_data;
  }

  template <typename Meta, typename Data>
  inline const std::shared_ptr<Data>&
  Node<Meta, Data>::getData() const noexcept
  {
    return m_data;
  }

  template <typename Meta, typename Data>
  inline bool
  Node<Meta, Data>::isLeaf() const noexcept
  {
    return m_children[0] == nullptr;
  }

  template <typename Meta, typename Data>
  inline void
  Node<Meta, Data>::build(const StopFunction&    a_stopFunction,
                          const MetaConstructor& a_metaConstructor,
                          const DataConstructor& a_dataConstructor) noexcept
  {
    if (this->isLeaf()) {
      if (!(a_stopFunction(*this))) {

        // Initialize children.
        for (size_t k = 0; k < 8; k++) {
          m_children[k]            = std::make_shared<Node<Meta, Data>>();
          m_children[k]->getMeta() = a_metaConstructor(static_cast<OctantIndex>(k), this->getMeta());
        }

        // Recurse.
        for (auto& c : m_children) {
          c->build(a_stopFunction, a_metaConstructor, a_dataConstructor);
        }
      }
    }
    else {
      std::cerr << "Node<Meta, Data>::build() got called on an interior node\n";
    }
  }

  template <typename Meta, typename Data>
  inline void
  Node<Meta, Data>::traverse(const Visiter& a_visiter, const Sorter& a_sorter) const noexcept
  {}
} // namespace Octree
#include "EBGeometry_NamespaceFooter.hpp"

#endif
