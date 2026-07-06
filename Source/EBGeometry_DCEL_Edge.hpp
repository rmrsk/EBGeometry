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
#include <memory>
#include <type_traits>

// Our includes
#include "EBGeometry_DCEL.hpp"
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

namespace DCEL {

/**
 * @brief Class which represents a half-edge in a double-edge connected list
 * (DCEL).
 * @details This class is used in DCEL functionality which stores polygonal
 * surfaces in a mesh. The information contain in an EdgeT object contains the
 * necessary object for logically circulating the inside of a polygon face. This
 * means that a polygon face has a double-connected list of half-edges which
 * circulate the interior of the face. The EdgeT object is such a half-edge; it
 * represents the outgoing half-edge from a vertex, located such that it can be
 * logically represented as a half edge on the "inside" of a polygon face. It
 * contains pointers to the polygon face, vertex, and next half edge It also contains
 * a pointer to the "pair" half edge, i.e. the
 * corresponding half-edge on the other face that shares this edge. Since this
 * class is used with DCEL functionality and signed distance fields, this class
 * also has a signed distance function and thus a "normal vector".
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
   * @brief Alias for vertex type
   */
  using Vertex = VertexT<T, Meta>;

  /**
   * @brief Alias for edge type
   */
  using Edge = EdgeT<T, Meta>;

  /**
   * @brief Alias for face type
   */
  using Face = FaceT<T, Meta>;

  /**
   * @brief Alias for vertex pointer type
   */
  using VertexPtr = std::shared_ptr<Vertex>;

  /**
   * @brief Alias for edge pointer type
   */
  using EdgePtr = std::shared_ptr<Edge>;

  /**
   * @brief Alias for face pointer type
   */
  using FacePtr = std::shared_ptr<Face>;

  /**
   * @brief Default constructor. Sets all pointers to zero and vectors to zero
   * vectors
   */
  EdgeT() noexcept;

  /**
   * @brief Copy constructor. Copies all information from the other half-edge.
   * @param[in] a_otherEdge Other edge
   */
  EdgeT(const Edge& a_otherEdge) noexcept;

  /**
   * @brief Partial constructor. Calls the default constructor but sets the
   * starting vertex.
   * @param[in] a_vertex Starting vertex.
   */
  EdgeT(const VertexPtr& a_vertex) noexcept;

  /**
   * @brief Destructor (does nothing)
   */
  virtual ~EdgeT();

  /**
   * @brief Get size (in bytes) of this object.
   * @return Size in bytes of this edge object.
   */
  [[nodiscard]] inline size_t
  size() const noexcept;

  /**
   * @brief Define function. Sets the starting vertex, edges, and normal vectors
   * @param[in] a_vertex       Starting vertex
   * @param[in] a_pairEdge     Pair half-edge
   * @param[in] a_nextEdge     Next half-edge
   */
  inline void
  define(const VertexPtr& a_vertex, const EdgePtr& a_pairEdge, const EdgePtr& a_nextEdge) noexcept;

  /**
   * @brief Reconcile internal logic
   * @details Computes normal.
   */
  inline void
  reconcile() noexcept;

  /**
   * @brief Flip surface normal
   */
  inline void
  flip() noexcept;

  /**
   * @brief Set the starting vertex
   * @param[in] a_vertex Starting vertex
   */
  inline void
  setVertex(const VertexPtr& a_vertex) noexcept;

  /**
   * @brief Set the pair edge
   * @param[in] a_pairEdge Pair edge
   */
  inline void
  setPairEdge(const EdgePtr& a_pairEdge) noexcept;

  /**
   * @brief Set the next edge
   * @param[in] a_nextEdge Next edge
   */
  inline void
  setNextEdge(const EdgePtr& a_nextEdge) noexcept;

  /**
   * @brief Set the pointer to this half-edge's face.
   * @param[in] a_face Face to associate with this half-edge.
   */
  inline void
  setFace(const FacePtr& a_face) noexcept;

  /**
   * @brief Get modifiable starting vertex
   * @return Returns m_vertex
   */
  [[nodiscard]] inline VertexPtr&
  getVertex() noexcept;

  /**
   * @brief Get immutable starting vertex
   * @return Returns m_vertex
   */
  [[nodiscard]] inline const VertexPtr&
  getVertex() const noexcept;

  /**
   * @brief Get modifiable end vertex
   * @return Returns the next half-edge's starting vertex
   */
  [[nodiscard]] inline VertexPtr&
  getOtherVertex() noexcept;

  /**
   * @brief Get immutable end vertex
   * @return Returns the next half-edge's starting vertex
   */
  [[nodiscard]] inline const VertexPtr&
  getOtherVertex() const noexcept;

  /**
   * @brief Get modifiable pair edge
   * @return Returns the pair edge
   */
  [[nodiscard]] inline EdgePtr&
  getPairEdge() noexcept;

  /**
   * @brief Get immutable pair edge
   * @return Returns the pair edge
   */
  [[nodiscard]] inline const EdgePtr&
  getPairEdge() const noexcept;

  /**
   * @brief Get modifiable next edge
   * @return Returns the next edge
   */
  [[nodiscard]] inline EdgePtr&
  getNextEdge() noexcept;

  /**
   * @brief Get immutable next edge
   * @return Returns the next edge
   */
  [[nodiscard]] inline const EdgePtr&
  getNextEdge() const noexcept;

  /**
   * @brief Compute the normal vector as the average of the face normals on
   * both sides of this edge.
   * @return Unit normal vector for this edge.
   */
  [[nodiscard]] inline Vec3T<T>
  computeNormal() const noexcept;

  /**
   * @brief Get the stored normal vector.
   * @return Const reference to m_normal.
   */
  [[nodiscard]] inline const Vec3T<T>&
  getNormal() const noexcept;

  /**
   * @brief Get modifiable half-edge face.
   * @return Reference to m_face.
   */
  [[nodiscard]] inline FacePtr&
  getFace() noexcept;

  /**
   * @brief Get immutable half-edge face.
   * @return Const reference to m_face.
   */
  [[nodiscard]] inline const FacePtr&
  getFace() const noexcept;

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

  /**
   * @brief Get the signed distance from a_x0 to this half-edge.
   * @details Checks whether a_x0 projects onto the edge segment or past one of
   * the end vertices. If it projects to a vertex, the signed distance to that
   * vertex is returned. Otherwise the sign is determined from the edge normal.
   * @param[in] a_x0 Query point.
   * @return Signed distance; positive on the normal side of the edge.
   */
  [[nodiscard]] inline T
  signedDistance(const Vec3& a_x0) const noexcept;

  /**
   * @brief Get the squared unsigned distance from a_x0 to this half-edge.
   * @details Clamps the projection parameter to [0,1] so the closest point is
   * always on the segment, then returns the squared distance. Faster than
   * signedDistance().
   * @param[in] a_x0 Query point.
   * @return Squared Euclidean distance to the closest point on the edge.
   */
  [[nodiscard]] inline T
  unsignedDistance2(const Vec3& a_x0) const noexcept;

protected:
  /**
   * @brief Normal vector
   */
  Vec3 m_normal;

  /**
   * @brief Starting vertex
   */
  VertexPtr m_vertex;

  /**
   * @brief Pair edge
   */
  EdgePtr m_pairEdge;

  /**
   * @brief Next edge
   */
  EdgePtr m_nextEdge;

  /**
   * @brief Enclosing polygon face
   */
  FacePtr m_face;

  /**
   * @brief Meta-data attached to this edge
   */
  Meta m_metaData;

  /**
   * @brief Returns the parametric projection of a_x0 onto this edge.
   * @details Parametrizes the edge as x(t) = x1 + (x2-x1)*t and computes t
   * such that x(t) is the closest point to a_x0. Returns t in (-inf, +inf);
   * the point is on the segment when t in [0,1].
   * @param[in] a_x0 Query point.
   * @return Projection parameter t.
   */
  [[nodiscard]] inline T
  projectPointToEdge(const Vec3& a_x0) const noexcept;

  /**
   * @brief Get the vector pointing along this edge (from start to end vertex).
   * @return x2 - x1, where x1 = getVertex()->getPosition() and x2 =
   * getOtherVertex()->getPosition().
   */
  [[nodiscard]] inline Vec3T<T>
  getX2X1() const noexcept;
};
} // namespace DCEL

} // namespace EBGeometry

#include "EBGeometry_DCEL_EdgeImplem.hpp"

#endif
