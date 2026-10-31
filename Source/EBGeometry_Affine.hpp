// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Affine.hpp
 * @brief  Declaration of Affine3T, an affine map of 3D space (a 3x3 matrix and a shift).
 * @details The expression builder and the fused operations fold every chain of Translate, Rotate,
 * Reflect and Scale into one Affine3T, which the Tape or the fused node applies to the query point
 * before evaluating the transformed subtree. Device-callable.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_AFFINE_HPP
#define EBGEOMETRY_AFFINE_HPP

// Std includes
#include <cstddef>
#include <type_traits>

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief An affine map of 3D space, x -> A x + b, with A stored by rows.
 * @details A plain, trivially copyable value type. The factory functions build the maps the transforms
 * need; then() composes two maps. Value-initialized (Affine3T<T>{}), it is the zero map, not the
 * identity: use identity().
 * @tparam T Floating-point precision.
 */
template <class T>
struct Affine3T
{
  static_assert(std::is_floating_point_v<T>, "Affine3T requires a floating-point type T");

  /**
   * @brief First row of A.
   */
  Vec3T<T> m_row0;

  /**
   * @brief Second row of A.
   */
  Vec3T<T> m_row1;

  /**
   * @brief Third row of A.
   */
  Vec3T<T> m_row2;

  /**
   * @brief The shift b.
   */
  Vec3T<T> m_shift;

  /**
   * @brief The identity map.
   * @return x -> x.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline Affine3T
  identity() noexcept;

  /**
   * @brief A translation.
   * @param[in] a_shift Shift.
   * @return x -> x + a_shift.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline Affine3T
  translation(const Vec3T<T>& a_shift) noexcept;

  /**
   * @brief A rotation about a coordinate axis, counterclockwise seen from the positive end of the axis.
   * @param[in] a_degrees Angle in degrees.
   * @param[in] a_axis    Coordinate axis (0, 1 or 2).
   * @return x -> R x.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline Affine3T
  rotation(const T a_degrees, const size_t a_axis) noexcept;

  /**
   * @brief A rotation about an arbitrary axis through the origin, counterclockwise seen from the
   * positive end of the axis (Rodrigues' formula).
   * @param[in] a_degrees Angle in degrees.
   * @param[in] a_axis    Axis direction; need not be normalized, must not be zero.
   * @return x -> R x.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline Affine3T
  rotation(const T a_degrees, const Vec3T<T>& a_axis) noexcept;

  /**
   * @brief A uniform scaling.
   * @param[in] a_factor Scale factor.
   * @return x -> a_factor x.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline Affine3T
  scaling(const T a_factor) noexcept;

  /**
   * @brief A reflection in a coordinate plane through the origin.
   * @param[in] a_axis Axis normal to the plane (0, 1 or 2); that coordinate changes sign.
   * @return x -> x with component a_axis negated.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline Affine3T
  reflection(const size_t a_axis) noexcept;

  /**
   * @brief Apply the map.
   * @param[in] a_point Point.
   * @return A a_point + b.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Vec3T<T>
  operator()(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief The map that applies this one and then @p a_next.
   * @param[in] a_next Map to apply second.
   * @return x -> a_next(this(x)).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Affine3T
  then(const Affine3T& a_next) const noexcept;

  /**
   * @brief Whether every row of A has at most one non-zero entry, so that the map sends axis-aligned
   * boxes to axis-aligned boxes exactly.
   * @return True for translations, scalings, reflections and quarter-turn rotations.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  isAxisAligned() const noexcept;

  /**
   * @brief A box enclosing the image of a box under the map.
   * @details Exact for an axis-aligned map, which also keeps an unbounded box unbounded only along the
   * axes it maps an unbounded axis to. For any other map, a bounded box's image is enclosed by the box
   * around its eight transformed corners, and an unbounded box gives an unbounded box. Coordinates
   * saturate at plus or minus Math::Limits<T>::max(). An empty box stays empty.
   * @param[in] a_box Box.
   * @return Box enclosing the image.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline BoundingVolumes::AABBT<T>
  transform(const BoundingVolumes::AABBT<T>& a_box) const noexcept;
};

static_assert(std::is_trivially_copyable_v<Affine3T<float>>, "Affine3T<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<Affine3T<double>>, "Affine3T<double> must be trivially copyable");

} // namespace EBGeometry

#include "EBGeometry_AffineImplem.hpp"

#endif
