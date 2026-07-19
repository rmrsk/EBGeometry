// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_DCEL_Vertex.hpp
 * @brief  Declaration of a vertex class for use in DCEL descriptions of polygon
 * tessellations.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_DCEL_VERTEX_HPP
#define EBGEOMETRY_DCEL_VERTEX_HPP

// Std includes
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_DCEL.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

namespace DCEL {

/**
 * @brief Class which represents a vertex node in a double-edge connected list
 * (DCEL).
 * @details This class is used in DCEL functionality which stores polygonal
 * surfaces in a mesh. In the flat, index-based DCEL representation the vertex is a pure data
 * holder: it stores a position, a normal vector, the index of one of the outgoing half-edges from
 * the vertex, and the indices of the polygon faces that share this vertex (used only for the
 * pseudonormal computation). All topological links are @c DCELIndex indices into the owning
 * MeshT's flat arrays; @c InvalidIndex means "no link".
 * @note The normal vector is outgoing, i.e. a point x is "outside" the vertex if
 * the dot product between n and (x - x0) is positive.
 * @tparam T    Floating-point precision.
 * @tparam Meta Meta-data type stored per vertex.
 */
template <class T, class Meta>
class VertexT
{
  static_assert(std::is_floating_point_v<T>, "VertexT<T,Meta>: T must be a floating-point type");

public:
  /**
   * @brief Alias for vector type
   */
  using Vec3 = Vec3T<T>;

  /**
   * @brief Alias for vertex type
   */
  using Vertex = VertexT<T, Meta>;

  /**
   * @brief Default constructor.
   * @details Defaulted: the position and normal vectors zero-initialize via Vec3T's own default
   * constructor, the outgoing edge index defaults to InvalidIndex, the face list defaults to empty,
   * and the meta-data value-initializes. Not marked noexcept since Meta is an unconstrained template
   * parameter whose default constructor is not guaranteed to be noexcept.
   */
  VertexT() = default;

  /**
   * @brief Partial constructor.
   * @param[in] a_position Vertex position
   * @details This initializes the position to a_position and the normal vector
   * to the zero vector. The polygon face list is empty.
   */
  VertexT(const Vec3& a_position);

  /**
   * @brief Constructor.
   * @param[in] a_position    Vertex position
   * @param[in] a_normal Vertex normal vector
   * @details This initializes the position to a_position and the normal vector
   * to a_normal. The polygon face list is empty.
   */
  VertexT(const Vec3& a_position, const Vec3& a_normal);

  /**
   * @brief Copy constructor (defaulted memberwise copy).
   * @details The vertex is a flat data holder whose topological links are plain array indices, so
   * an ordinary memberwise copy is well-defined and independent -- the indices remain valid in a
   * copied MeshT array. Copies every field (position, normal, outgoing edge index, incident face
   * indices, meta-data).
   * @param[in] a_otherVertex Other vertex.
   */
  VertexT(const Vertex& a_otherVertex) = default;

  /**
   * @brief Move constructor (defaulted memberwise move).
   * @param[in, out] a_otherVertex Other vertex.
   */
  VertexT(Vertex&& a_otherVertex) = default;

  /**
   * @brief Destructor (does nothing)
   */
  ~VertexT() = default;

  /**
   * @brief Copy assignment operator (defaulted memberwise copy).
   * @param[in] a_otherVertex Other vertex.
   * @return Reference to (*this).
   */
  Vertex&
  operator=(const Vertex& a_otherVertex) = default;

  /**
   * @brief Move assignment operator (defaulted memberwise move).
   * @param[in, out] a_otherVertex Other vertex.
   * @return Reference to (*this).
   */
  Vertex&
  operator=(Vertex&& a_otherVertex) = default;

  /**
   * @brief Define function
   * @param[in] a_position Vertex position
   * @param[in] a_edge     Index of the outgoing edge (InvalidIndex if none yet)
   * @param[in] a_normal   Vertex normal vector
   * @details This sets the position, normal vector, and outgoing-edge index.
   */
  inline void
  define(const Vec3& a_position, const DCELIndex a_edge, const Vec3& a_normal) noexcept;

  /**
   * @brief Set the vertex position
   * @param[in] a_position Vertex position
   */
  inline void
  setPosition(const Vec3& a_position) noexcept;

  /**
   * @brief Set the vertex normal vector
   * @param[in] a_normal Vertex normal vector
   */
  inline void
  setNormal(const Vec3& a_normal) noexcept;

  /**
   * @brief Set the index of the outgoing edge
   * @param[in] a_edge Index of an outgoing edge (InvalidIndex if none)
   */
  inline void
  setEdge(const DCELIndex a_edge) noexcept;

  /**
   * @brief Set the meta-data.
   * @param[in] a_metaData Meta-data.
   */
  inline void
  setMetaData(const Meta& a_metaData) noexcept;

  /**
   * @brief Add a face index to the incident-face list.
   * @param[in] a_face Index of the incident face.
   */
  inline void
  addFace(const DCELIndex a_face);

  /**
   * @brief Normalize the normal vector, ensuring its length is 1
   * @details The current normal must not be (near-)zero-length; a zero-length normal cannot be
   * normalized and is left unchanged (a no-op) rather than dividing by zero.
   */
  inline void
  normalizeNormalVector() noexcept;

  /**
   * @brief Flip the normal vector
   */
  inline void
  flipNormal() noexcept;

  /**
   * @brief Return modifiable vertex position.
   * @return Reference to m_position.
   */
  [[nodiscard]] inline Vec3T<T>&
  getPosition() noexcept;

  /**
   * @brief Return immutable vertex position.
   * @return Const reference to m_position.
   */
  [[nodiscard]] inline const Vec3T<T>&
  getPosition() const noexcept;

  /**
   * @brief Return modifiable vertex normal vector.
   * @return Reference to m_normal.
   */
  [[nodiscard]] inline Vec3T<T>&
  getNormal() noexcept;

  /**
   * @brief Return immutable vertex normal vector.
   * @return Const reference to m_normal.
   */
  [[nodiscard]] inline const Vec3T<T>&
  getNormal() const noexcept;

  /**
   * @brief Get the index of the outgoing edge.
   * @return Outgoing-edge index, or InvalidIndex.
   */
  [[nodiscard]] inline DCELIndex
  getOutgoingEdge() const noexcept;

  /**
   * @brief Get modifiable incident-face index list for this vertex.
   * @return Reference to m_faces.
   */
  [[nodiscard]] inline std::vector<DCELIndex>&
  getFaces() noexcept;

  /**
   * @brief Get immutable incident-face index list for this vertex.
   * @return Const reference to m_faces.
   */
  [[nodiscard]] inline const std::vector<DCELIndex>&
  getFaces() const noexcept;

  /**
   * @brief Get the signed distance to this vertex
   * @param[in] a_x0 Position in space.
   * @return The returned distance is |a_x0 - m_position| and the sign is given
   * by the sign of m_normal * (a_x0 - m_position).
   */
  [[nodiscard]] inline T
  signedDistance(const Vec3& a_x0) const noexcept;

  /**
   * @brief Get the squared unsigned distance to this vertex.
   * @details This is faster to compute than signedDistance, and might be
   * preferred for some algorithms.
   * @param[in] a_x0 Query position.
   * @return Squared Euclidean distance |a_x0 - m_position|^2.
   */
  [[nodiscard]] inline T
  unsignedDistance2(const Vec3& a_x0) const noexcept;

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
   * @brief Vertex position
   */
  Vec3 m_position;

  /**
   * @brief Vertex normal vector
   */
  Vec3 m_normal;

  /**
   * @brief Index of an outgoing edge from this vertex (InvalidIndex if none).
   */
  DCELIndex m_outgoingEdge = InvalidIndex;

  /**
   * @brief Indices of the faces incident on this vertex.
   * @details These must be added by addFace(), and are used when computing the vertex
   * normal vector (in particular the angle-weighted pseudonormal).
   */
  std::vector<DCELIndex> m_faces;

  /**
   * @brief Meta-data for this vertex
   * @details Value-initialized so that every constructor leaves it in a defined state.
   */
  Meta m_metaData{};
};
} // namespace DCEL

} // namespace EBGeometry

#include "EBGeometry_DCEL_VertexImplem.hpp"

#endif
