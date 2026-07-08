// SPDX-FileCopyrightText: 2024 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Triangle.hpp
 * @brief  Declaration of a triangle class with signed distance functionality.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_TRIANGLE_HPP
#define EBGEOMETRY_TRIANGLE_HPP

// Std includes
#include <array>
#include <type_traits>

// Our includes
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Triangle class with signed distance functionality.
 * @details This class represents a planar triangle and has a signed distance functionality. It is
 * self-contained: it stores all of its own data and holds no external references. The class contains a
 * triangle face normal vector; three vertex positions, and normal vectors for the three vertices and three edges.
 *
 * This class assumes that the vertices are organized with the right-hand rule. I.e., edges are enumerated
 * as follows:
 *
 * Edge 1 points from vertex 1 to vertex 2
 * Edge 2 points from vertex 2 to vertex 3
 * Edge 3 points from vertex 3 to vertex 0
 *
 * This class can compute its own normal vector from the vertex positions, and the triangle orientation
 * is then implicitly given by the vertex order.
 *
 * To compute the distance from a point to the triangle, one must determine if the point projects to the
 * "inside" or "outside" of the triangle. If the query point projects to the inside of the triangle, the
 * distance is simply the projected distance onto the triangle plane. If it projects to the outside of
 * the triangle, we check the distance against the triangle edges and vertices.
 * @tparam T    Floating-point precision.
 * @tparam Meta User-defined metadata type stored with the triangle.
 */
template <class T, class Meta>
class Triangle
{
  static_assert(std::is_floating_point_v<T>, "T must be a floating-point type");

public:
  /**
   * @brief Alias for 2D vector type.
   */
  using Vec2 = Vec2T<T>;

  /**
   * @brief Alias for 3D vector type.
   */
  using Vec3 = Vec3T<T>;

  /**
   * @brief Default constructor. Does not put the triangle in a usable state.
   */
  Triangle() noexcept = default;

  /**
   * @brief Copy constructor.
   * @param[in] a_otherTriangle Other triangle.
   */
  Triangle(const Triangle& a_otherTriangle) noexcept = default;

  /**
   * @brief Move constructor.
   * @param[in, out] a_otherTriangle Other triangle.
   */
  Triangle(Triangle&& a_otherTriangle) noexcept = default;

  /**
   * @brief Full constructor.
   * @param[in] a_vertexPositions Triangle vertex positions.
   */
  Triangle(const std::array<Vec3, 3>& a_vertexPositions) noexcept;

  /**
   * @brief Destructor (does nothing).
   */
  ~Triangle() noexcept = default;

  /**
   * @brief Copy assignment.
   * @param[in] a_otherTriangle Other triangle.
   * @return Reference to (*this).
   */
  Triangle&
  operator=(const Triangle& a_otherTriangle) noexcept = default;

  /**
   * @brief Move assignment.
   * @param[in, out] a_otherTriangle Other triangle.
   * @return Reference to (*this).
   */
  Triangle&
  operator=(Triangle&& a_otherTriangle) noexcept = default;

  /**
   * @brief Set the triangle normal vector.
   * @param[in] a_normal Normal vector. Must be finite and unit length.
   * @note The input normal should be consistent with the vertex ordering.
   */
  void
  setNormal(const Vec3& a_normal) noexcept;

  /**
   * @brief Set the triangle vertex positions
   * @details Also recomputes the triangle face normal from the new vertex positions (see
   * computeNormal). The three vertices must not be collinear (or coincident), or the
   * face normal is undefined.
   * @param[in] a_vertexPositions Vertex positions. Each component must be finite.
   */
  void
  setVertexPositions(const std::array<Vec3, 3>& a_vertexPositions) noexcept;

  /**
   * @brief Set the triangle vertex normals
   * @param[in] a_vertexNormals Vertex normals. Each must be finite and unit length.
   */
  void
  setVertexNormals(const std::array<Vec3, 3>& a_vertexNormals) noexcept;

  /**
   * @brief Set the triangle edge normals
   * @param[in] a_edgeNormals Edge normals. Each must be finite and unit length.
   */
  void
  setEdgeNormals(const std::array<Vec3, 3>& a_edgeNormals) noexcept;

  /**
   * @brief Set the triangle meta-data
   * @param[in] a_metaData Triangle metadata.
   */
  void
  setMetaData(const Meta& a_metaData) noexcept;

  /**
   * @brief Compute the triangle normal vector.
   * @details This computes the normal vector from two of the triangle edges, using the
   * right-hand rule and vertex ordering to infer the normal vector. The three vertices
   * must not be collinear (or coincident): a zero-area triangle has an undefined face normal.
   */
  void
  computeNormal() noexcept;

  /**
   * @brief Get the triangle normal vector.
   * @details Const-only: mutating this in place would desynchronize it from the vertex
   * positions it was derived from. Use setNormal or setVertexPositions instead.
   * @return m_triangleNormal
   */
  [[nodiscard]] const Vec3&
  getNormal() const noexcept;

  /**
   * @brief Get the vertex positions
   * @details Const-only: mutating positions in place would leave m_triangleNormal stale.
   * Use setVertexPositions instead.
   * @return m_vertexPositions
   */
  [[nodiscard]] const std::array<Vec3, 3>&
  getVertexPositions() const noexcept;

  /**
   * @brief Get the vertex normals
   * @details Const-only; use setVertexNormals to update.
   * @return m_vertexNormals
   */
  [[nodiscard]] const std::array<Vec3, 3>&
  getVertexNormals() const noexcept;

  /**
   * @brief Get the edge normals
   * @details Const-only; use setEdgeNormals to update.
   * @return m_edgeNormals
   */
  [[nodiscard]] const std::array<Vec3, 3>&
  getEdgeNormals() const noexcept;

  /**
   * @brief Get the triangle meta-data
   * @details Const-only; use setMetaData to update.
   * @return m_metaData
   */
  [[nodiscard]] const Meta&
  getMetaData() const noexcept;

  /**
   * @brief Compute the signed distance from the input point x to the triangle.
   * @details Requires the triangle to be fully initialized with a non-degenerate (non-collinear)
   * set of vertex positions; see setVertexPositions.
   * @param[in] a_point Query point. Must be finite.
   * @return Signed distance; negative inside, positive outside.
   */
  [[nodiscard]] T
  signedDistance(const Vec3& a_point) const noexcept;

protected:
  /**
   * @brief Triangle face normal
   */
  Vec3 m_triangleNormal = Vec3::max();

  /**
   * @brief Triangle vertex positions
   */
  std::array<Vec3, 3> m_vertexPositions{Vec3::max(), Vec3::max(), Vec3::max()};

  /**
   * @brief Triangle vertex normals
   */
  std::array<Vec3, 3> m_vertexNormals{Vec3::max(), Vec3::max(), Vec3::max()};

  /**
   * @brief Triangle edge normals
   */
  std::array<Vec3, 3> m_edgeNormals{Vec3::max(), Vec3::max(), Vec3::max()};

  /**
   * @brief Triangle meta-data normals
   */
  Meta m_metaData;
};
} // namespace EBGeometry

#include "EBGeometry_TriangleImplem.hpp" // NOLINT

#endif
