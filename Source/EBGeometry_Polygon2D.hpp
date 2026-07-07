// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Polygon2D.hpp
 * @brief  Declaration of a two-dimensional polygon class for embedding 3D
 * polygon faces
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_POLYGON2D_HPP
#define EBGEOMETRY_POLYGON2D_HPP

// Std includes
#include <cstddef>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Class for embedding a polygon face into 2D.
 * @details This class is required for determining whether or not a 3D point
 * projected to the plane of an N-sided polygon lies inside or outside the
 * polygon face. To do this we compute the 2D embedding of the polygon face,
 * reducing the problem to a tractable dimension where we can use well-tested
 * algorithm. The 2D embedding of a polygon occurs by taking a set of 3D points
 * and a corresponding normal vector, and projecting those points along one of
 * the 3D Cartesian axes such that the polygon has the largest area. In essence,
 * we simply find the direction with the smallest normal vector component and
 * ignore that. Once the 2D embedding is computed, we can use well-known
 * algorithms for checking if a point lies inside or outside. The supported
 * algorithms are 1) The winding number algorithm (computing the winding number),
 * 2) Computing the subtended angle of the point with the edges of the polygon
 * (sums to 360 degrees if the point is inside), or computing the crossing number
 * which checks how many times a ray cast from the point crosses the edges of the
 * polygon.
 * @tparam T Floating-point precision.
 */
template <class T>
class Polygon2D
{
  static_assert(std::is_floating_point_v<T>, "Polygon2D requires a floating-point T");

public:
  /**
   * @brief Supported algorithms for performing inside/outside tests when
   * checking if a point projects to the inside or outside of a polygon face.
   */
  enum class InsideOutsideAlgorithm
  {
    SubtendedAngle, ///< Sums the subtended angle of the point with each polygon edge; the point is
                    ///< inside if the sum is +-2*pi (360 degrees) and outside if it is 0. See
                    ///< isPointInsidePolygonSubtend() and computeSubtendedAngle().
    CrossingNumber, ///< Casts a ray from the point along +x and counts how many times it crosses a
                    ///< polygon edge; the point is inside if the crossing count is odd (even-odd
                    ///< rule). See isPointInsidePolygonCrossingNumber() and computeCrossingNumber().
    WindingNumber   ///< Computes the winding number of the polygon boundary around the point; the
                    ///< point is inside if the winding number is non-zero. See
                    ///< isPointInsidePolygonWindingNumber() and computeWindingNumber().
  };

  /**
   * @brief Alias for the 2D vector type used for the projected polygon points.
   */
  using Vec2 = Vec2T<T>;

  /**
   * @brief Alias for the 3D vector type used for the polygon's original (unprojected) points.
   */
  using Vec3 = Vec3T<T>;

  /**
   * @brief Disallowed constructor, use the one with the normal vector and points
   */
  Polygon2D() = delete;

  /**
   * @brief Full constructor
   * @param[in] a_normal Normal vector of the 3D polygon face. Must be finite and non-degenerate
   * (length > 0).
   * @param[in] a_points Vertex coordinates of the 3D polygon face. Must have at least 3 points.
   */
  Polygon2D(const Vec3& a_normal, const std::vector<Vec3>& a_points);

  /**
   * @brief Copy constructor.
   * @param[in] a_other Other polygon.
   */
  Polygon2D(const Polygon2D& a_other) = default;

  /**
   * @brief Move constructor.
   * @param[in, out] a_other Other polygon.
   */
  Polygon2D(Polygon2D&& a_other) noexcept = default;

  /**
   * @brief Destructor.
   */
  ~Polygon2D() = default;

  /**
   * @brief Copy assignment operator.
   * @param[in] a_other Other polygon.
   * @return Reference to (*this).
   */
  Polygon2D&
  operator=(const Polygon2D& a_other) = default;

  /**
   * @brief Move assignment operator.
   * @param[in, out] a_other Other polygon.
   * @return Reference to (*this).
   */
  Polygon2D&
  operator=(Polygon2D&& a_other) noexcept = default;

  /**
   * @brief Check if a point is inside or outside the 2D polygon.
   * @param[in] a_point     3D point coordinates.
   * @param[in] a_algorithm Inside/outside algorithm to use.
   * @details This will call the function corresponding to a_algorithm.
   * @return True if the projected point lies inside the 2D polygon.
   */
  [[nodiscard]] inline bool
  isPointInside(const Vec3& a_point, const InsideOutsideAlgorithm a_algorithm) const noexcept;

  /**
   * @brief Check if a point is inside a 2D polygon, using the winding number
   * algorithm
   * @param[in] a_point 3D point coordinates
   * @return Returns true if the 3D point projects to the inside of the 2D
   * polygon
   */
  [[nodiscard]] inline bool
  isPointInsidePolygonWindingNumber(const Vec3& a_point) const noexcept;

  /**
   * @brief Check if a point is inside a 2D polygon, using the subtended angles
   * @param[in] a_point 3D point coordinates
   * @return Returns true if the 3D point projects to the inside of the 2D
   * polygon
   */
  [[nodiscard]] inline bool
  isPointInsidePolygonSubtend(const Vec3& a_point) const noexcept;

  /**
   * @brief Check if a point is inside a 2D polygon, by computing the number of
   * times a ray crosses the polygon edges.
   * @param[in] a_point 3D point coordinates
   * @return Returns true if the 3D point projects to the inside of the 2D
   * polygon
   */
  [[nodiscard]] inline bool
  isPointInsidePolygonCrossingNumber(const Vec3& a_point) const noexcept;

private:
  /**
   * @brief The corresponding 2D x-direction (one direction is ignored)
   */
  size_t m_xDir = 0;

  /**
   * @brief The corresponding 2D y-direction (one direction is ignored)
   */
  size_t m_yDir = 0;

  /**
   * @brief Projected set of points in 2D
   */
  std::vector<Vec2> m_points;

  /**
   * @brief Project a 3D point onto the 2D polygon plane (this ignores one of the
   * vector components)
   * @param[in] a_point 3D point
   * @return 2D point, ignoring one of the coordinate directions.
   */
  [[nodiscard]] inline Vec2
  projectPoint(const Vec3& a_point) const noexcept;

  /**
   * @brief Define function. This find the direction to ignore and then computes
   * the 2D points.
   * @param[in] a_normal Normal vector for polygon face
   * @param[in] a_points Vertex coordinates for polygon face.
   */
  inline void
  define(const Vec3& a_normal, const std::vector<Vec3>& a_points);

  /**
   * @brief Compute the winding number for a point P with the 2D polygon
   * @param[in] P 2D point
   * @return Returns winding number.
   */
  [[nodiscard]] inline int
  computeWindingNumber(const Vec2& P) const noexcept;

  /**
   * @brief Compute the crossing number for a point P with the 2D polygon
   * @param[in] P 2D point
   * @return Returns crossing number.
   */
  [[nodiscard]] inline size_t
  computeCrossingNumber(const Vec2& P) const noexcept;

  /**
   * @brief Compute the subtended angle for a point P with the 2D polygon
   * @param[in] P 2D point
   * @return Returns subtended angle.
   */
  [[nodiscard]] inline T
  computeSubtendedAngle(const Vec2& P) const noexcept;
};

} // namespace EBGeometry

#include "EBGeometry_Polygon2DImplem.hpp"

#endif
