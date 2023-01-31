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

// Std includes
#include <stack>

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
  Node<Meta, Data>::getMetaData() noexcept
  {
    return m_meta;
  }

  template <typename Meta, typename Data>
  inline const Meta&
  Node<Meta, Data>::getMetaData() const noexcept
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
  Node<Meta, Data>::buildDepthFirst(const StopFunction&    a_stopFunction,
                                    const MetaConstructor& a_metaConstructor,
                                    const DataConstructor& a_dataConstructor) noexcept
  {
    if (this->isLeaf()) {
      if (!(a_stopFunction(*this))) {

        // Initialize children.
        for (size_t k = 0; k < 8; k++) {
          m_children[k]                = std::make_shared<Node<Meta, Data>>();
          m_children[k]->getMetaData() = a_metaConstructor(static_cast<OctantIndex>(k), this->getMetaData());
          m_children[k]->getData()     = a_dataConstructor(static_cast<OctantIndex>(k), this->getData());
        }

        // Recurse.
        for (auto& c : m_children) {
          c->buildDepthFirst(a_stopFunction, a_metaConstructor, a_dataConstructor);
        }
      }
    }
    else {
      std::cerr << "Node<Meta, Data>::buildDepthFirst() got called on an interior node\n";
    }
  }

  template <typename Meta, typename Data>
  inline void
  Node<Meta, Data>::buildBreadthFirst(const StopFunction&    a_stopFunction,
                                      const MetaConstructor& a_metaConstructor,
                                      const DataConstructor& a_dataConstructor) noexcept
  {
    if (this->isLeaf()) {
      std::stack<std::shared_ptr<Node<Meta, Data>>> q;

      q.emplace(this->shared_from_this());

      while (!(q.empty())) {
        auto& curNode = *q.top();

        q.pop();

        if (!(a_stopFunction(curNode))) {

          // Initialize children and push them onto the stack.
          auto& children = curNode.getChildren();

          for (size_t k = 0; k < 8; k++) {
            children[k]                = std::make_shared<Node<Meta, Data>>();
            children[k]->getMetaData() = a_metaConstructor(static_cast<OctantIndex>(k), curNode.getMetaData());
            children[k]->getData()     = a_dataConstructor(static_cast<OctantIndex>(k), curNode.getData());

            q.push(children[k]);
          }
        }
      }
    }
    else {
      std::cerr << "Node<Meta, Data>::buildBreadthFirst() got called on an interior node\n";
    }
  }

  template <typename Meta, typename Data>
  inline void
  Node<Meta, Data>::traverse(const Updater& a_updater, const Visiter& a_visiter, const Sorter& a_sorter) const noexcept
  {

    std::array<std::shared_ptr<const Node<Meta, Data>>, 8> children;
    std::stack<std::shared_ptr<const Node<Meta, Data>>>    q;

    q.emplace(this->shared_from_this());

    while (!(q.empty())) {

      const auto& curNode = *q.top();

      q.pop();

      if (a_visiter(curNode)) {
        if (curNode.isLeaf()) {
          a_updater(curNode);
        }
        else {
          for (size_t k = 0; k < 8; k++) {
            children[k] = curNode.getChildren()[k];
          }

          a_sorter(children);

          for (const auto& c : children) {
            q.push(c);
          }
        }
      }
    }
  }
} // namespace Octree
#include "EBGeometry_NamespaceFooter.hpp"

#endif
