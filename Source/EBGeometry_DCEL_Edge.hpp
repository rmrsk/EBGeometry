// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_DCEL_Edge.hpp
 * @brief  Declaration of a half-edge class for use in DCEL descriptions of
 * polygon tessellations.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_DCEL_EDGE_HPP
#define EBGEOMETRY_DCEL_EDGE_HPP

// Std includes
#include <cstddef>
#include <type_traits>

// Our includes
#include "EBGeometry_DCEL.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

namespace DCEL {

/**
 * @brief Class which represents a half-edge in a double-edge connected list
 * (DCEL).
 * @details This class is used in DCEL functionality which stores polygonal
 * surfaces in a mesh. In the flat, index-based DCEL representation the half-edge is a pure data
 * holder: it stores a normal vector plus four topological links, each a @c DCELIndex index into
 * the owning MeshT's flat arrays -- the origin vertex, the pair (twin) half-edge, the next
 * half-edge around the face, and the owning face. @c InvalidIndex means "no link" (e.g. the pair
 * of a boundary half-edge). There is no stored previous-edge link. Walking a face's edge loop, and
 * all geometric queries (signed distance, projection, the end vertex), are performed by MeshT,
 * which owns the arrays these indices refer to.
 * @note The normal vector is outgoing, i.e. a point x is "outside" if the dot
 * product between n and (x - x0) is positive.
 * @tparam T    Floating-point precision.
 * @tparam Meta Meta-data type stored per edge.
 */
template <class T, class Meta>
class EdgeT
{
  static_assert(std::is_floating_point_v<T>, "EdgeT<T,Meta>: T must be a floating-point type");

public:
  /**
   * @brief Alias for vector type
   */
  using Vec3 = Vec3T<T>;

  /**
   * @brief Alias for edge type
   */
  using Edge = EdgeT<T, Meta>;

  /**
   * @brief Default constructor. Sets all indices to InvalidIndex and the normal to the zero vector.
   */
  EdgeT() noexcept = default;

  /**
   * @brief Copy constructor (defaulted memberwise copy).
   * @details The half-edge is a flat data holder whose topological links are plain array indices,
   * so an ordinary memberwise copy is well-defined -- the indices remain valid in a copied MeshT
   * array. Copies the normal vector, all four topology indices, and the meta-data.
   * @param[in] a_otherEdge Other edge.
   */
  EdgeT(const Edge& a_otherEdge) noexcept = default;

  /**
   * @brief Move constructor (defaulted memberwise move).
   * @param[in, out] a_otherEdge Other edge.
   */
  EdgeT(Edge&& a_otherEdge) noexcept = default;

  /**
   * @brief Partial constructor. Sets the starting-vertex index.
   * @param[in] a_vertex Index of the starting (origin) vertex.
   */
  EdgeT(const DCELIndex a_vertex) noexcept;

  /**
   * @brief Destructor (does nothing)
   */
  ~EdgeT() = default;

  /**
   * @brief Copy assignment operator (defaulted memberwise copy).
   * @param[in] a_otherEdge Other edge.
   * @return Reference to (*this).
   */
  Edge&
  operator=(const Edge& a_otherEdge) noexcept = default;

  /**
   * @brief Move assignment operator (defaulted memberwise move).
   * @param[in, out] a_otherEdge Other edge.
   * @return Reference to (*this).
   */
  Edge&
  operator=(Edge&& a_otherEdge) noexcept = default;

  /**
   * @brief Get size (in bytes) of this object.
   * @return Size in bytes of this edge object.
   */
  [[nodiscard]] inline size_t
  size() const noexcept;

  /**
   * @brief Define function. Sets the origin vertex, pair edge, and next edge indices.
   * @param[in] a_vertex   Index of the origin vertex
   * @param[in] a_pairEdge Index of the pair (twin) half-edge
   * @param[in] a_nextEdge Index of the next half-edge around the face
   */
  inline void
  define(const DCELIndex a_vertex, const DCELIndex a_pairEdge, const DCELIndex a_nextEdge) noexcept;

  /**
   * @brief Flip surface normal
   */
  inline void
  flipNormal() noexcept;

  /**
   * @brief Set the origin-vertex index
   * @param[in] a_vertex Index of the origin vertex
   */
  inline void
  setVertex(const DCELIndex a_vertex) noexcept;

  /**
   * @brief Set the pair-edge index
   * @param[in] a_pairEdge Index of the pair edge
   */
  inline void
  setPairEdge(const DCELIndex a_pairEdge) noexcept;

  /**
   * @brief Set the next-edge index
   * @param[in] a_nextEdge Index of the next edge
   */
  inline void
  setNextEdge(const DCELIndex a_nextEdge) noexcept;

  /**
   * @brief Set the index of this half-edge's face.
   * @param[in] a_face Index of the face to associate with this half-edge.
   */
  inline void
  setFace(const DCELIndex a_face) noexcept;

  /**
   * @brief Set the meta-data.
   * @param[in] a_metaData Meta-data.
   */
  inline void
  setMetaData(const Meta& a_metaData) noexcept;

  /**
   * @brief Get the index of the origin vertex.
   * @return Origin-vertex index, or InvalidIndex.
   */
  [[nodiscard]] inline DCELIndex
  getVertex() const noexcept;

  /**
   * @brief Get the index of the pair (twin) edge.
   * @return Pair-edge index, or InvalidIndex.
   */
  [[nodiscard]] inline DCELIndex
  getPairEdge() const noexcept;

  /**
   * @brief Get the index of the next edge around the face.
   * @return Next-edge index, or InvalidIndex.
   */
  [[nodiscard]] inline DCELIndex
  getNextEdge() const noexcept;

  /**
   * @brief Get the index of this half-edge's face.
   * @return Owning-face index, or InvalidIndex.
   */
  [[nodiscard]] inline DCELIndex
  getFace() const noexcept;

  /**
   * @brief Get modifiable normal vector.
   * @return Reference to m_normal.
   */
  [[nodiscard]] inline Vec3T<T>&
  getNormal() noexcept;

  /**
   * @brief Get the stored normal vector.
   * @return Const reference to m_normal.
   */
  [[nodiscard]] inline const Vec3T<T>&
  getNormal() const noexcept;

  /**
   * @brief Get meta-data
   * @return m_metaData
   */
  [[nodiscard]] inline Meta&
  getMetaData() noexcept;

  /**
   * @brief Get meta-data
   * @return m_metaData
   */
  [[nodiscard]] inline const Meta&
  getMetaData() const noexcept;

protected:
  /**
   * @brief Normal vector
   */
  Vec3 m_normal = Vec3::zeros();

  /**
   * @brief Index of the origin (starting) vertex (InvalidIndex if unset).
   */
  DCELIndex m_vertex = InvalidIndex;

  /**
   * @brief Index of the pair (twin) half-edge (InvalidIndex for a boundary edge).
   */
  DCELIndex m_pairEdge = InvalidIndex;

  /**
   * @brief Index of the next half-edge around the face (InvalidIndex if unset).
   */
  DCELIndex m_nextEdge = InvalidIndex;

  /**
   * @brief Index of the enclosing polygon face (InvalidIndex if unset).
   */
  DCELIndex m_face = InvalidIndex;

  /**
   * @brief Meta-data attached to this edge
   * @details Value-initialized so that every constructor leaves it in a defined state.
   */
  Meta m_metaData{};
};
} // namespace DCEL

} // namespace EBGeometry

#include "EBGeometry_DCEL_EdgeImplem.hpp"

#endif
