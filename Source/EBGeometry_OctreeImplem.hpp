// SPDX-FileCopyrightText: 2023 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_OctreeImplem.hpp
 * @brief  Implementation of EBGeometry_Octree.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_OCTREEIMPLEM_HPP
#define EBGEOMETRY_OCTREEIMPLEM_HPP

// Std includes
#include <array>
#include <cstddef>
#include <iostream>
#include <memory>
#include <queue>
#include <stack>

// Our includes
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Octree.hpp"

namespace EBGeometry {

namespace Octree {

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
  const bool leaf = (m_children[0] == nullptr);

#if defined(EBGEOMETRY_ENABLE_ASSERTIONS)
  // Defensive check for the all-or-nothing children invariant: this only inspects m_children[0],
  // so a partially-populated array (e.g. from mutating getChildren() directly) would otherwise go
  // undetected.
  for (const auto& c : m_children) {
    EBGEOMETRY_EXPECT((c == nullptr) == leaf);
  }
#endif

  return leaf;
}

template <typename Meta, typename Data>
inline void
Node<Meta, Data>::buildDepthFirst(const SplitFunction&   a_splitFunction,
                                  const MetaConstructor& a_metaConstructor,
                                  const DataConstructor& a_dataConstructor) noexcept
{
  EBGEOMETRY_EXPECT(this->isLeaf());
  EBGEOMETRY_EXPECT(a_splitFunction);
  EBGEOMETRY_EXPECT(a_metaConstructor);
  EBGEOMETRY_EXPECT(a_dataConstructor);

  if (this->isLeaf()) {
    if (a_splitFunction(*this)) {

      // Initialize children.
      for (size_t k = 0; k < 8; k++) {
        m_children[k]                = std::make_shared<Node<Meta, Data>>();
        m_children[k]->getMetaData() = a_metaConstructor(static_cast<OctantIndex>(k), this->getMetaData());
        m_children[k]->getData()     = a_dataConstructor(static_cast<OctantIndex>(k), this->getData());
      }

      // Recurse.
      for (auto& c : m_children) {
        EBGEOMETRY_EXPECT(c != nullptr);

        c->buildDepthFirst(a_splitFunction, a_metaConstructor, a_dataConstructor);
      }
    }
  }
  else {
    std::cerr << "Node<Meta, Data>::buildDepthFirst() got called on an interior node\n";
  }
}

template <typename Meta, typename Data>
inline void
Node<Meta, Data>::buildBreadthFirst(const SplitFunction&   a_splitFunction,
                                    const MetaConstructor& a_metaConstructor,
                                    const DataConstructor& a_dataConstructor) noexcept
{
  EBGEOMETRY_EXPECT(this->isLeaf());
  EBGEOMETRY_EXPECT(a_splitFunction);
  EBGEOMETRY_EXPECT(a_metaConstructor);
  EBGEOMETRY_EXPECT(a_dataConstructor);
  EBGEOMETRY_EXPECT(!this->weak_from_this().expired());

  if (this->isLeaf()) {
    std::queue<std::shared_ptr<Node<Meta, Data>>> q;

    q.emplace(this->shared_from_this());

    while (!(q.empty())) {
      EBGEOMETRY_EXPECT(q.front() != nullptr);

      auto& curNode = *q.front();

      q.pop();

      if (a_splitFunction(curNode)) {

        // Initialize children and push them onto the queue.
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
Node<Meta, Data>::traverse(const LeafEvaluator&  a_leafEvaluator,
                           const PrunePredicate& a_prunePredicate,
                           const ChildOrderer&   a_childOrderer) const noexcept
{
  EBGEOMETRY_EXPECT(a_leafEvaluator);
  EBGEOMETRY_EXPECT(a_prunePredicate);
  EBGEOMETRY_EXPECT(a_childOrderer);
  EBGEOMETRY_EXPECT(!this->weak_from_this().expired());

  std::array<std::shared_ptr<const Node<Meta, Data>>, 8> children;
  std::stack<std::shared_ptr<const Node<Meta, Data>>>    q;

  q.emplace(this->shared_from_this());

  while (!(q.empty())) {
    EBGEOMETRY_EXPECT(q.top() != nullptr);

    const auto& curNode = *q.top();

    q.pop();

    if (a_prunePredicate(curNode)) {
      if (curNode.isLeaf()) {
        a_leafEvaluator(curNode);
      }
      else {
        for (size_t k = 0; k < 8; k++) {
          children[k] = curNode.getChildren()[k];

          EBGEOMETRY_EXPECT(children[k] != nullptr);
        }

        a_childOrderer(children);

        for (const auto& c : children) {
          q.push(c);
        }
      }
    }
  }
}
} // namespace Octree
} // namespace EBGeometry

#endif
