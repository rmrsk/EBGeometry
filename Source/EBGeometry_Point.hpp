// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Point.hpp
 * @brief  Declaration of a point class with unsigned-distance functionality.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_POINT_HPP
#define EBGEOMETRY_POINT_HPP

// Std includes
#include <type_traits>

// Our includes
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Point class with unsigned-distance functionality.
 * @details This class represents a single 3D point (e.g. one particle in a point cloud) and has
 * no orientation of its own -- unlike Triangle<T, Meta>, which has a face normal and can report a
 * signed distance, a bare point has no inside/outside notion, so this class only ever reports
 * unsigned distance. It is self-contained: it stores all of its own data and holds no external
 * references.
 * @tparam T    Floating-point precision.
 * @tparam Meta User-defined metadata type stored with the point.
 */
template <class T, class Meta>
class Point
{
  static_assert(std::is_floating_point_v<T>, "T must be a floating-point type");

public:
  /**
   * @brief Alias for 3D vector type.
   */
  using Vec3 = Vec3T<T>;

  /**
   * @brief Default constructor. Does not put the point in a usable state.
   */
  Point() noexcept = default;

  /**
   * @brief Copy constructor.
   * @param[in] a_otherPoint Other point.
   */
  Point(const Point& a_otherPoint) noexcept = default;

  /**
   * @brief Move constructor.
   * @param[in, out] a_otherPoint Other point.
   */
  Point(Point&& a_otherPoint) noexcept = default;

  /**
   * @brief Full constructor.
   * @param[in] a_position Point position.
   * @param[in] a_metaData Point metadata.
   */
  Point(const Vec3& a_position, const Meta& a_metaData) noexcept;

  /**
   * @brief Destructor (does nothing).
   */
  ~Point() noexcept = default;

  /**
   * @brief Copy assignment.
   * @param[in] a_otherPoint Other point.
   * @return Reference to (*this).
   */
  Point&
  operator=(const Point& a_otherPoint) noexcept = default;

  /**
   * @brief Move assignment.
   * @param[in, out] a_otherPoint Other point.
   * @return Reference to (*this).
   */
  Point&
  operator=(Point&& a_otherPoint) noexcept = default;

  /**
   * @brief Set the point position.
   * @param[in] a_position Point position. Each component must be finite.
   */
  void
  setPosition(const Vec3& a_position) noexcept;

  /**
   * @brief Set the point meta-data.
   * @param[in] a_metaData Point metadata.
   */
  void
  setMetaData(const Meta& a_metaData) noexcept;

  /**
   * @brief Get the point position.
   * @details Const-only; use setPosition to update.
   * @return m_position
   */
  [[nodiscard]] const Vec3&
  getPosition() const noexcept;

  /**
   * @brief Get the point meta-data.
   * @details Const-only; use setMetaData to update.
   * @return m_metaData
   */
  [[nodiscard]] const Meta&
  getMetaData() const noexcept;

  /**
   * @brief Compute the unsigned distance from the input point to this point.
   * @param[in] a_point Query point. Must be finite.
   * @return Euclidean distance between a_point and this point's position.
   */
  [[nodiscard]] T
  getDistance(const Vec3& a_point) const noexcept;

  /**
   * @brief Compute the squared unsigned distance from the input point to this point.
   * @details Avoids the sqrt that getDistance() pays -- prefer this whenever the caller only
   * needs the distance for comparison (e.g. nearest-neighbor search), not its actual magnitude.
   * @param[in] a_point Query point. Must be finite.
   * @return Squared Euclidean distance between a_point and this point's position.
   */
  [[nodiscard]] T
  getDistance2(const Vec3& a_point) const noexcept;

protected:
  /**
   * @brief Point position.
   */
  Vec3 m_position = Vec3::max();

  /**
   * @brief Point meta-data.
   */
  Meta m_metaData;
};
} // namespace EBGeometry

#include "EBGeometry_PointImplem.hpp" // NOLINT

#endif
