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
#include <cstdint>
#include <type_traits>

// Our includes
#include "EBGeometry_DCEL.hpp"

namespace EBGeometry {

namespace DCEL {

/**
 * @brief Class which makes it easier to iterate through DCEL edges
 * @details Iterates the half-edge loop bounding a polygon face (or starting from an arbitrary
 * half-edge index), following EdgeT::getNextEdgeIndex() until either the loop returns to its
 * starting edge (a well-formed, closed face) or an unset next-edge index is reached (an
 * incomplete/open half-edge chain). Resolves indices against the mesh passed at construction, so
 * the iterator itself is a lightweight, short-lived (stack-only) helper -- it is not stored inside
 * any DCEL object and does not need to be trivially copyable or portable. Typical usage:
 * @code{.cpp}
 * for (EdgeIterator it(mesh, someFace); it.ok(); ++it) {
 *   const uint32_t edgeIndex = it();
 *   // ... use mesh.getEdges()[edgeIndex] ...
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
   * @brief Alias for DCEL mesh type
   */
  using Mesh = MeshT<T, Meta>;

  /**
   * @brief Default construction is not allowed. Use one of the full constructors
   */
  EdgeIteratorT() = delete;

  /**
   * @brief Constructor, taking a mesh and a face as argument. The iterator begins at the
   * half-edge index contained in the face.
   * @param[in] a_mesh Owning mesh, used to resolve edge indices.
   * @param[in] a_face DCEL polygon face
   * @note This constructor will iterate through the half-edges in the polygon face.
   */
  EdgeIteratorT(const Mesh& a_mesh, const Face& a_face) noexcept;

  /**
   * @brief Constructor, taking a mesh and an arbitrary starting half-edge index directly (rather
   * than a face's own half-edge). Useful for iterating a half-edge loop when only an edge index --
   * not its face -- is available.
   * @param[in] a_mesh          Owning mesh, used to resolve edge indices.
   * @param[in] a_startEdgeIndex Starting half-edge index. May be UINT32_MAX (ok() will then
   * immediately return false).
   */
  EdgeIteratorT(const Mesh& a_mesh, const uint32_t a_startEdgeIndex) noexcept;

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
   * @brief Operator returning the index of the current half-edge
   * @return Index of the current half-edge in the owning mesh's edge array.
   */
  [[nodiscard]] inline uint32_t
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
   * @return True if the iterator has not completed a full loop and the current edge index is set.
   */
  [[nodiscard]] inline bool
  ok() const noexcept;

protected:
  /**
   * @brief Owning mesh, used to resolve edge indices. Non-owning; must outlive the iterator.
   */
  const Mesh* m_mesh = nullptr;

  /**
   * @brief If true, a full loop has been made around the polygon face
   */
  bool m_fullLoop = false;

  /**
   * @brief Starting half-edge index, or UINT32_MAX if unset.
   */
  uint32_t m_startEdge = UINT32_MAX;

  /**
   * @brief Current half-edge index, or UINT32_MAX if unset.
   */
  uint32_t m_curEdge = UINT32_MAX;
};
} // namespace DCEL

} // namespace EBGeometry

#include "EBGeometry_DCEL_IteratorImplem.hpp"

#endif
