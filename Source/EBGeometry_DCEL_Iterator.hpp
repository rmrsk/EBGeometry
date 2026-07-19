// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_DCEL_Iterator.hpp
 * @brief  Declaration of iterators for DCEL surface tessellations
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_DCEL_ITERATOR_HPP
#define EBGEOMETRY_DCEL_ITERATOR_HPP

// Std includes
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_DCEL.hpp"
#include "EBGeometry_DCEL_Edge.hpp"

namespace EBGeometry {

namespace DCEL {

/**
 * @brief Class which makes it easier to iterate through DCEL edges
 * @details Iterates the half-edge loop bounding a polygon face (or starting from an arbitrary
 * half-edge index), following EdgeT::getNextEdge() until either the loop returns to its starting
 * edge (a well-formed, closed face) or an InvalidIndex next-edge is reached (an incomplete/open
 * half-edge chain). This is an index cursor over a MeshT's flat edge array: it holds a pointer to
 * that array plus the starting and current half-edge indices, and dereferences to the current
 * @c DCELIndex.
 * @code{.cpp}
 * for (EdgeIterator it(mesh.getEdges(), startEdgeIndex); it.ok(); ++it) {
 *   const DCELIndex edge = it();
 *   // ... use edge ...
 * }
 * @endcode
 * @tparam T    Floating-point precision type.
 * @tparam Meta User-defined metadata type.
 */
template <class T, class Meta>
class EdgeIteratorT
{
  static_assert(std::is_floating_point_v<T>, "T must be a floating-point type");

public:
  /**
   * @brief Alias for DCEL edge type
   */
  using Edge = EdgeT<T, Meta>;

  /**
   * @brief Default construction is not allowed. Use the full constructor.
   */
  EdgeIteratorT() = delete;

  /**
   * @brief Constructor, taking the mesh's edge array and a starting half-edge index.
   * @param[in] a_edges     Reference to the mesh's flat edge array. Must outlive the iterator.
   * @param[in] a_startEdge Starting half-edge index. May be InvalidIndex (ok() will then
   * immediately return false).
   */
  EdgeIteratorT(const std::vector<Edge>& a_edges, const DCELIndex a_startEdge) noexcept;

  /**
   * @brief Copy constructor.
   * @param[in] a_other Other iterator.
   */
  EdgeIteratorT(const EdgeIteratorT& a_other) noexcept = default;

  /**
   * @brief Move constructor.
   * @param[in, out] a_other Other iterator.
   */
  EdgeIteratorT(EdgeIteratorT&& a_other) noexcept = default;

  /**
   * @brief Destructor.
   */
  ~EdgeIteratorT() noexcept = default;

  /**
   * @brief Copy assignment operator.
   * @param[in] a_other Other iterator.
   * @return Reference to (*this).
   */
  EdgeIteratorT&
  operator=(const EdgeIteratorT& a_other) noexcept = default;

  /**
   * @brief Move assignment operator.
   * @param[in, out] a_other Other iterator.
   * @return Reference to (*this).
   */
  EdgeIteratorT&
  operator=(EdgeIteratorT&& a_other) noexcept = default;

  /**
   * @brief Operator returning the current half-edge index.
   * @return Index of the current half-edge.
   */
  [[nodiscard]] inline DCELIndex
  operator()() const noexcept;

  /**
   * @brief Reset function for the iterator. This resets the iterator so that it
   * begins from the starting half-edge index.
   */
  inline void
  reset() noexcept;

  /**
   * @brief Incrementation operator, bringing the iterator to the next half-edge
   */
  inline void
  operator++() noexcept;

  /**
   * @brief Function which checks if the iteration can be continued.
   * @return True if the iterator has not completed a full loop and the current edge index is valid.
   */
  [[nodiscard]] inline bool
  ok() const noexcept;

protected:
  /**
   * @brief If true, a full loop has been made around the polygon face
   */
  bool m_fullLoop = false;

  /**
   * @brief Starting half-edge index
   */
  DCELIndex m_startEdge = InvalidIndex;

  /**
   * @brief Current half-edge index
   */
  DCELIndex m_curEdge = InvalidIndex;

  /**
   * @brief Pointer to the mesh's flat edge array (not owned).
   */
  const std::vector<Edge>* m_edges = nullptr;
};
} // namespace DCEL

} // namespace EBGeometry

#include "EBGeometry_DCEL_IteratorImplem.hpp"

#endif
