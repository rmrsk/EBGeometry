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
#include <memory>
#include <type_traits>

// Our includes
#include "EBGeometry_DCEL.hpp"

namespace EBGeometry {

namespace DCEL {

/**
 * @brief Class which makes it easier to iterate through DCEL edges
 * @details Iterates the half-edge loop bounding a polygon face (or starting from an arbitrary
 * half-edge), following EdgeT::getNextEdge() until either the loop returns to its starting edge
 * (a well-formed, closed face) or a null next-edge is reached (an incomplete/open half-edge chain).
 * Typical usage:
 * @code{.cpp}
 * for (EdgeIterator it(someFace); it.ok(); ++it) {
 *   const EdgePtr& edge = it();
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
   * @brief Alias for DCEL vertex type
   */
  using Vertex = VertexT<T, Meta>;

  /**
   * @brief Alias for DCEL edge type
   */
  using Edge = EdgeT<T, Meta>;

  /**
   * @brief Alias for DCEL face type
   */
  using Face = FaceT<T, Meta>;

  /**
   * @brief Alias for vertex pointer
   */
  using VertexPtr = std::shared_ptr<Vertex>;

  /**
   * @brief Alias for edge pointer
   */
  using EdgePtr = std::shared_ptr<Edge>;

  /**
   * @brief Alias for face pointer
   */
  using FacePtr = std::shared_ptr<Face>;

  /**
   * @brief Default construction is not allowed. Use one of the full constructors
   */
  EdgeIteratorT() = delete;

  /**
   * @brief Constructor, taking a face as argument. The iterator begins at the
   * half-edge pointer contained in the face
   * @param[in] a_face DCEL polygon face
   * @note This constructor will will iterate through the half-edges in the
   * polygon face.
   */
  EdgeIteratorT(Face& a_face) noexcept;

  /**
   * @brief Constructor, taking a face as argument. The iterator begins at the
   * half-edge pointer contained in the face
   * @param[in] a_face DCEL polygon face
   * @note This constructor will will iterate through the half-edges in the
   * polygon face.
   */
  EdgeIteratorT(const Face& a_face) noexcept;

  /**
   * @brief Constructor, taking an arbitrary starting half-edge directly (rather than a face's own
   * half-edge). Useful for iterating a half-edge loop when only an edge -- not its face -- is
   * available.
   * @param[in] a_startEdge Starting half-edge. May be nullptr (ok() will then immediately return
   * false).
   */
  EdgeIteratorT(const EdgePtr& a_startEdge) noexcept;

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
   * @brief Operator returning a pointer to the current half-edge
   * @return Reference to the shared pointer to the current half-edge.
   */
  [[nodiscard]] inline EdgePtr&
  operator()() noexcept;

  /**
   * @brief Operator returning a pointer to the current half-edge (const overload)
   * @return Const reference to the shared pointer to the current half-edge.
   */
  [[nodiscard]] inline const EdgePtr&
  operator()() const noexcept;

  /**
   * @brief Reset function for the iterator. This resets the iterator so that it
   * begins from the starting half-edge
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
   * @return True if the iterator has not completed a full loop and the current edge is not null.
   */
  [[nodiscard]] inline bool
  ok() const noexcept;

protected:
  /**
   * @brief If true, a full loop has been made around the polygon face
   */
  bool m_fullLoop = false;

  /**
   * @brief Starting half-edge
   */
  std::shared_ptr<Edge> m_startEdge = nullptr;

  /**
   * @brief Current half-edge
   */
  std::shared_ptr<Edge> m_curEdge = nullptr;
};
} // namespace DCEL

} // namespace EBGeometry

#include "EBGeometry_DCEL_IteratorImplem.hpp"

#endif
