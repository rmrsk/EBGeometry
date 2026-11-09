// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_AnalyticDistanceFunctions.hpp
 * @brief  File containing various analytic signed distance fields.
 * @details Every shape here is a plain, trivially copyable value type with no base class and no
 * virtual functions. Its signedDistance() and accessors are callable on the host and on a device, so
 * a shape can be copied into a kernel by value and evaluated there. Constructors run on the host.
 * Every shape is a shape in the sense of isShape, so it can be composed with the expression builder
 * (Union, Translate, ...; see EBGeometry_Expr.hpp) and compiled into a Tape.
 *
 * Conventions shared by every shape:
 * - Placement first: each constructor takes the shape's position (a center, corner, tip or point)
 *   before its sizes. PerlinSDF, which fills all of space, has none.
 * - One axis: a shape with a fixed axis has it along y (Torus, InfiniteCone, Cone,
 *   RoundedCylinder, CappedTorus, CutSphere, CutHollowSphere and Wedge, and the defaults of the
 *   others); Rotate turns it. Cylinder, Capsule, CappedCone, RoundCone, PolygonPrism and Tube take
 *   two end points, and InfiniteCylinder an axis index, so they can point anywhere.
 * - Outer sizes: a size is that of the finished shape, rounding included.
 * - computeBoundingVolume() returns the shape's axis-aligned bounding box. Along a direction in
 *   which the shape is unbounded, the box extends to plus or minus Math::Limits<T>::max().
 * - distanceQuality says how far signedDistance() can be trusted as a distance (DistanceQuality).
 *   Every shape is Exact except PerlinSDF, which is NotADistance.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_ANALYTICDISTANCEFUNCTIONS_HPP
#define EBGEOMETRY_ANALYTICDISTANCEFUNCTIONS_HPP

// Std includes
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <random>
#include <type_traits>

// Our includes
#include "EBGeometry_Array.hpp"
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_Constants.hpp"
#include "EBGeometry_DistanceQuality.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Signed distance function for a plane.
 * @details The SDF evaluates to `dot(a_point - m_point, m_normal)`: positive on the half-space
 * the normal points into, negative on the opposite side. The plane itself is the zero level-set.
 * `m_normal` must be unit length for the SDF to return a true distance; the full constructor
 * normalizes the input automatically. By default the plane is the y = 0 plane with normal (0,1,0).
 * @tparam T Floating-point precision.
 */
template <class T>
class PlaneSDF
{
  static_assert(std::is_floating_point_v<T>, "PlaneSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: the Euclidean distance to the surface.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  /**
   * @brief Default constructor. Constructs the y = 0 plane with outward normal (0, 1, 0).
   * @details The default plane passes through the origin and separates y < 0 (negative SDF) from
   * y > 0 (positive SDF).
   */
  PlaneSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_point  Any point on the plane.
   * @param[in] a_normal Plane normal vector (need not be unit length; it is normalized internally).
   */
  PlaneSDF(const Vec3T<T>& a_point, const Vec3T<T>& a_normal) noexcept
  {
    EBGEOMETRY_REQUIRE(std::isfinite(a_point[0]) && std::isfinite(a_point[1]) && std::isfinite(a_point[2]),
                       "PlaneSDF: the point must be finite (%g, %g, %g)",
                       double(a_point[0]),
                       double(a_point[1]),
                       double(a_point[2]));
    EBGEOMETRY_REQUIRE(std::isfinite(a_normal[0]) && std::isfinite(a_normal[1]) && std::isfinite(a_normal[2]),
                       "PlaneSDF: the normal must be finite (%g, %g, %g)",
                       double(a_normal[0]),
                       double(a_normal[1]),
                       double(a_normal[2]));
    EBGEOMETRY_REQUIRE(a_normal.length() > T(0),
                       "PlaneSDF: the normal must be nonzero (%g, %g, %g)",
                       double(a_normal[0]),
                       double(a_normal[1]),
                       double(a_normal[2]));

    m_point  = a_point;
    m_normal = a_normal / a_normal.length();
  }

  /**
   * @brief Copy constructor.
   */
  PlaneSDF(const PlaneSDF&) = default;

  /**
   * @brief Move constructor.
   */
  PlaneSDF(PlaneSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  PlaneSDF&
  operator=(const PlaneSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  PlaneSDF&
  operator=(PlaneSDF&&) = default;

  /**
   * @brief Signed distance function for the plane.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the plane; negative on the side opposite to the normal.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));
    EBGEOMETRY_EXPECT(std::abs(m_normal.length() - T(1)) < std::sqrt(Math::Limits<T>::epsilon()));

    return dot((a_point - m_point), m_normal);
  }

  /**
   * @brief The shape's axis-aligned bounding box.
   * @details A plane is unbounded in every direction a box can describe, so the box is the whole space.
   * @return The bounding box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    return BoundingVolumes::AABBT<T>(Vec3T<T>::min(), Vec3T<T>::max());
  }

  /**
   * @brief Get the point on the plane.
   * @return The point on the plane.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getPoint() const noexcept
  {
    return m_point;
  }

  /**
   * @brief Get the unit normal of the plane.
   * @return The unit normal of the plane.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getNormal() const noexcept
  {
    return m_normal;
  }

protected:
  /**
   * @brief Point on plane.
   */
  Vec3T<T> m_point = Vec3T<T>::zeros();

  /**
   * @brief Plane normal vector (unit length).
   */
  Vec3T<T> m_normal = Vec3T<T>(T(0), T(1), T(0));
};

/**
 * @brief Signed distance field for a sphere.
 * @details The sphere is placed at `m_center` with radius `m_radius`. The SDF is negative
 * inside the sphere and positive outside. By default the sphere is centered at the origin
 * with radius 1.
 * @tparam T Floating-point precision.
 */
template <class T>
class SphereSDF
{
  static_assert(std::is_floating_point_v<T>, "SphereSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: the Euclidean distance to the surface.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  /**
   * @brief Default constructor. Constructs a unit sphere centered at the origin.
   */
  SphereSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_center Sphere center.
   * @param[in] a_radius Sphere radius.
   */
  SphereSDF(const Vec3T<T>& a_center, const T& a_radius) noexcept
  {
    EBGEOMETRY_REQUIRE(std::isfinite(a_center[0]) && std::isfinite(a_center[1]) && std::isfinite(a_center[2]),
                       "SphereSDF: the center must be finite (%g, %g, %g)",
                       double(a_center[0]),
                       double(a_center[1]),
                       double(a_center[2]));
    EBGEOMETRY_REQUIRE(std::isfinite(a_radius) && a_radius > T(0),
                       "SphereSDF: the radius must be finite and positive (%g)",
                       double(a_radius));

    m_center = a_center;
    m_radius = a_radius;
  }

  /**
   * @brief Copy constructor.
   */
  SphereSDF(const SphereSDF&) = default;

  /**
   * @brief Move constructor.
   */
  SphereSDF(SphereSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  SphereSDF&
  operator=(const SphereSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  SphereSDF&
  operator=(SphereSDF&&) = default;

  /**
   * @brief Get sphere center (const).
   * @return Const reference to the center position.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getCenter() const noexcept
  {
    return m_center;
  }

  /**
   * @brief Get sphere center (mutable).
   * @return Mutable reference to the center position.
   */
  EBGEOMETRY_HOST_DEVICE
  Vec3T<T>&
  getCenter() noexcept
  {
    return m_center;
  }

  /**
   * @brief Get sphere radius (const).
   * @return Const reference to the radius.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getRadius() const noexcept
  {
    return m_radius;
  }

  /**
   * @brief Get sphere radius (mutable).
   * @return Mutable reference to the radius.
   */
  EBGEOMETRY_HOST_DEVICE
  T&
  getRadius() noexcept
  {
    return m_radius;
  }

  /**
   * @brief Signed distance function for the sphere.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the sphere surface; negative inside the sphere.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    return (a_point - m_center).length() - m_radius;
  }

  /**
   * @brief The shape's axis-aligned bounding box.
   * @details The center plus and minus the radius along each axis.
   * @return The bounding box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    return BoundingVolumes::AABBT<T>(m_center - m_radius * Vec3T<T>::ones(), m_center + m_radius * Vec3T<T>::ones());
  }

protected:
  /**
   * @brief Sphere center.
   */
  Vec3T<T> m_center = Vec3T<T>::zeros();

  /**
   * @brief Sphere radius.
   */
  T m_radius = T(1);
};

/**
 * @brief Signed distance field for an axis-aligned box (AABB).
 * @details The box is defined by its lower and upper corners `m_loCorner` and `m_hiCorner`.
 * It is axis-aligned (faces are perpendicular to the coordinate axes). The SDF is negative
 * inside and positive outside. Side lengths are `m_hiCorner[i] - m_loCorner[i]` per axis.
 * By default the box is the unit cube `[-0.5, 0.5]^3` centered at the origin.
 * @tparam T Floating-point precision.
 */
template <class T>
class BoxSDF
{
  static_assert(std::is_floating_point_v<T>, "BoxSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: the Euclidean distance to the surface.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  /**
   * @brief Default constructor. Constructs a unit cube centered at the origin: [-0.5, 0.5]^3.
   */
  BoxSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_loCorner Lower corner (minimum x, y, z).
   * @param[in] a_hiCorner Upper corner (maximum x, y, z). Each component must be strictly greater
   * than the corresponding component of a_loCorner.
   */
  BoxSDF(const Vec3T<T>& a_loCorner, const Vec3T<T>& a_hiCorner) noexcept
  {
    EBGEOMETRY_REQUIRE(std::isfinite(a_loCorner[0]) && std::isfinite(a_loCorner[1]) && std::isfinite(a_loCorner[2]),
                       "BoxSDF: the low corner must be finite (%g, %g, %g)",
                       double(a_loCorner[0]),
                       double(a_loCorner[1]),
                       double(a_loCorner[2]));
    EBGEOMETRY_REQUIRE(std::isfinite(a_hiCorner[0]) && std::isfinite(a_hiCorner[1]) && std::isfinite(a_hiCorner[2]),
                       "BoxSDF: the high corner must be finite (%g, %g, %g)",
                       double(a_hiCorner[0]),
                       double(a_hiCorner[1]),
                       double(a_hiCorner[2]));
    EBGEOMETRY_REQUIRE(
      a_loCorner[0] < a_hiCorner[0] && a_loCorner[1] < a_hiCorner[1] && a_loCorner[2] < a_hiCorner[2],
      "BoxSDF: the low corner (%g, %g, %g) must be below the high corner (%g, %g, %g) in every direction",
      double(a_loCorner[0]),
      double(a_loCorner[1]),
      double(a_loCorner[2]),
      double(a_hiCorner[0]),
      double(a_hiCorner[1]),
      double(a_hiCorner[2]));

    m_loCorner = a_loCorner;
    m_hiCorner = a_hiCorner;
  }

  /**
   * @brief Copy constructor.
   */
  BoxSDF(const BoxSDF&) = default;

  /**
   * @brief Move constructor.
   */
  BoxSDF(BoxSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  BoxSDF&
  operator=(const BoxSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  BoxSDF&
  operator=(BoxSDF&&) = default;

  /**
   * @brief Get lower-left corner
   * @return m_loCorner
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getLowCorner() const noexcept
  {
    return m_loCorner;
  }

  /**
   * @brief Get lower-left corner
   * @return m_loCorner
   */
  EBGEOMETRY_HOST_DEVICE
  Vec3T<T>&
  getLowCorner() noexcept
  {
    return m_loCorner;
  }

  /**
   * @brief Get upper-right corner
   * @return m_hiCorner
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getHighCorner() const noexcept
  {
    return m_hiCorner;
  }

  /**
   * @brief Get upper-right corner
   * @return m_hiCorner
   */
  EBGEOMETRY_HOST_DEVICE
  Vec3T<T>&
  getHighCorner() noexcept
  {
    return m_hiCorner;
  }

  /**
   * @brief Signed distance function for the axis-aligned box.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the box surface; negative inside the box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    // For each coordinate direction, we have delta[dir] if a_point[dir] falls
    // between xLo and xHi. In this case delta[dir] will be the signed distance
    // to the closest box face in the dir-direction. Otherwise, if a_point[dir]
    // is outside the corner we have delta[dir] > 0.
    const Vec3T<T> delta(Math::max(m_loCorner[0] - a_point[0], a_point[0] - m_hiCorner[0]),
                         Math::max(m_loCorner[1] - a_point[1], a_point[1] - m_hiCorner[1]),
                         Math::max(m_loCorner[2] - a_point[2], a_point[2] - m_hiCorner[2]));

    // Note: max is max(Vec3T<T>, Vec3T<T>) and not std::max. It returns a
    // vector with coordinate-wise largest components. Note that the first part
    // Math::min(...) is the signed distance on the inside of the box (delta will
    // have negative components). The other part max(Vec3T<T>::zeros(), ...) is
    // for outside the box.
    const T d = Math::min(T(0.0), delta[delta.maxDir(false)]) + max(Vec3T<T>::zeros(), delta).length();

    return d;
  }

  /**
   * @brief How many parts the shape has (see hasParts): 6.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  getNumParts() const noexcept
  {
    return 6U;
  }

  /**
   * @brief The part whose term decides the value at a point (see hasParts): 2 axis + 0 for the face at the low corner, 2 axis + 1 for the one at the high corner (-x, +x, -y, +y, -z, +z), the axis whose term is largest, as in signedDistance().
   * @param[in] a_point Position.
   * @return The part.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  closestPart(const Vec3T<T>& a_point) const noexcept
  {
    const Vec3T<T> lo = m_loCorner - a_point;
    const Vec3T<T> hi = a_point - m_hiCorner;
    const Vec3T<T> delta(Math::max(lo[0], hi[0]), Math::max(lo[1], hi[1]), Math::max(lo[2], hi[2]));

    const size_t axis = delta.maxDir(false);

    return 2U * uint32_t(axis) + ((hi[axis] > lo[axis]) ? 1U : 0U);
  }

  /**
   * @brief The shape's axis-aligned bounding box.
   * @details The box itself.
   * @return The bounding box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    return BoundingVolumes::AABBT<T>(m_loCorner, m_hiCorner);
  }

protected:
  /**
   * @brief Low box corner.
   */
  Vec3T<T> m_loCorner = Vec3T<T>(T(-0.5), T(-0.5), T(-0.5));

  /**
   * @brief High box corner.
   */
  Vec3T<T> m_hiCorner = Vec3T<T>(T(0.5), T(0.5), T(0.5));
};

/**
 * @brief Signed distance field for a torus.
 * @details The torus ring lies in the xz-plane, around an axis along y, and is centered at
 * `m_center`. The major radius `m_majorRadius` is the distance from the torus center to the
 * center-line of the tube; the minor radius `m_minorRadius` is the tube radius. The bounding box
 * spans `[center ± (majorRadius + minorRadius)]` in x and z, and `[center ± minorRadius]` in y.
 * The SDF is negative inside the tube and positive outside. The minor radius must be strictly
 * less than the major radius; otherwise the tube would self-intersect at the torus center.
 * By default the torus is centered at the origin with major radius 1 and minor radius 0.5.
 * @tparam T Floating-point precision.
 */
template <class T>
class TorusSDF
{
  static_assert(std::is_floating_point_v<T>, "TorusSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: the Euclidean distance to the surface.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  /**
   * @brief Default constructor. Constructs a torus centered at the origin with major radius 1 and minor radius 0.5.
   */
  TorusSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_center      Torus center.
   * @param[in] a_majorRadius Major (ring) radius.
   * @param[in] a_minorRadius Minor (tube) radius.
   */
  TorusSDF(const Vec3T<T>& a_center, const T& a_majorRadius, const T& a_minorRadius) noexcept
  {
    EBGEOMETRY_REQUIRE(std::isfinite(a_center[0]) && std::isfinite(a_center[1]) && std::isfinite(a_center[2]),
                       "TorusSDF: the center must be finite (%g, %g, %g)",
                       double(a_center[0]),
                       double(a_center[1]),
                       double(a_center[2]));
    EBGEOMETRY_REQUIRE(std::isfinite(a_majorRadius) && a_majorRadius > T(0),
                       "TorusSDF: the major radius must be finite and positive (%g)",
                       double(a_majorRadius));
    EBGEOMETRY_REQUIRE(std::isfinite(a_minorRadius) && a_minorRadius > T(0),
                       "TorusSDF: the minor radius must be finite and positive (%g)",
                       double(a_minorRadius));
    EBGEOMETRY_REQUIRE(a_minorRadius < a_majorRadius,
                       "TorusSDF: the minor radius (%g) must be less than the major radius (%g)",
                       double(a_minorRadius),
                       double(a_majorRadius));

    m_center      = a_center;
    m_majorRadius = a_majorRadius;
    m_minorRadius = a_minorRadius;
  }

  /**
   * @brief Copy constructor.
   */
  TorusSDF(const TorusSDF&) = default;

  /**
   * @brief Move constructor.
   */
  TorusSDF(TorusSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  TorusSDF&
  operator=(const TorusSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  TorusSDF&
  operator=(TorusSDF&&) = default;

  /**
   * @brief Get torus center.
   * @return m_center
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getCenter() const noexcept
  {
    return m_center;
  }

  /**
   * @brief Get torus center.
   * @return m_center
   */
  EBGEOMETRY_HOST_DEVICE
  Vec3T<T>&
  getCenter() noexcept
  {
    return m_center;
  }

  /**
   * @brief Get major radius.
   * @return m_majorRadius
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getMajorRadius() const noexcept
  {
    return m_majorRadius;
  }

  /**
   * @brief Get major radius.
   * @return m_majorRadius
   */
  EBGEOMETRY_HOST_DEVICE
  T&
  getMajorRadius() noexcept
  {
    return m_majorRadius;
  }

  /**
   * @brief Get minor radius.
   * @return m_minorRadius
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getMinorRadius() const noexcept
  {
    return m_minorRadius;
  }

  /**
   * @brief Get minor radius.
   * @return m_minorRadius
   */
  EBGEOMETRY_HOST_DEVICE
  T&
  getMinorRadius() noexcept
  {
    return m_minorRadius;
  }
  /**
   * @brief Signed distance function for the torus.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the torus surface; negative inside the torus tube.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    const Vec3T<T> p   = a_point - m_center;
    const T        rho = std::sqrt(p[0] * p[0] + p[2] * p[2]) - m_majorRadius;
    const T        d   = std::sqrt(rho * rho + p[1] * p[1]) - m_minorRadius;

    return d;
  }

  /**
   * @brief The shape's axis-aligned bounding box.
   * @details The ring plus the tube radius in x and z, the tube radius alone in y.
   * @return The bounding box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    const T        outer = m_majorRadius + m_minorRadius;
    const Vec3T<T> half(outer, m_minorRadius, outer);

    return BoundingVolumes::AABBT<T>(m_center - half, m_center + half);
  }

protected:
  /**
   * @brief Torus center.
   */
  Vec3T<T> m_center = Vec3T<T>::zeros();

  /**
   * @brief Major (ring) radius.
   */
  T m_majorRadius = T(1);

  /**
   * @brief Minor (tube) radius.
   */
  T m_minorRadius = T(0.5);
};

/**
 * @brief Signed distance field for a finite, flat-capped cylinder.
 * @details The cylinder is defined by two cap-center positions `m_center1` and `m_center2` and a
 * radius `m_radius`. The cylinder axis is the unit vector from `m_center1` to `m_center2`, and its
 * height (distance between the two flat caps) equals `distance(m_center1, m_center2)`. The SDF is
 * negative inside the cylinder and positive outside. By default the cylinder has radius 1 and
 * height 1, centered at the origin with its axis along y (cap centers at (0,-0.5,0) and (0,0.5,0)).
 * @tparam T Floating-point precision.
 */
template <class T>
class CylinderSDF
{
  static_assert(std::is_floating_point_v<T>, "CylinderSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: the Euclidean distance to the surface.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  /**
   * @brief Default constructor. Constructs a unit cylinder of radius 1 and height 1, centered at the
   * origin with its axis along y (cap centers at (0,-0.5,0) and (0,0.5,0)).
   */
  CylinderSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_center1 One endpoint (cap center).
   * @param[in] a_center2 Other endpoint (cap center).
   * @param[in] a_radius  Cylinder radius.
   */
  CylinderSDF(const Vec3T<T>& a_center1, const Vec3T<T>& a_center2, const T& a_radius) noexcept
  {
    EBGEOMETRY_REQUIRE(std::isfinite(a_center1[0]) && std::isfinite(a_center1[1]) && std::isfinite(a_center1[2]),
                       "CylinderSDF: the first center must be finite (%g, %g, %g)",
                       double(a_center1[0]),
                       double(a_center1[1]),
                       double(a_center1[2]));
    EBGEOMETRY_REQUIRE(std::isfinite(a_center2[0]) && std::isfinite(a_center2[1]) && std::isfinite(a_center2[2]),
                       "CylinderSDF: the second center must be finite (%g, %g, %g)",
                       double(a_center2[0]),
                       double(a_center2[1]),
                       double(a_center2[2]));
    EBGEOMETRY_REQUIRE(std::isfinite(a_radius) && a_radius > T(0),
                       "CylinderSDF: the radius must be finite and positive (%g)",
                       double(a_radius));
    EBGEOMETRY_REQUIRE((a_center2 - a_center1).length() > T(0),
                       "CylinderSDF: the two centers must differ, but both are (%g, %g, %g)",
                       double(a_center1[0]),
                       double(a_center1[1]),
                       double(a_center1[2]));

    m_center1 = a_center1;
    m_center2 = a_center2;
    m_radius  = a_radius;

    m_center = (m_center2 + m_center1) * T(0.5);
    m_length = (m_center2 - m_center1).length();
    m_axis   = (m_center2 - m_center1) / m_length;
  }

  /**
   * @brief Copy constructor.
   */
  CylinderSDF(const CylinderSDF&) = default;

  /**
   * @brief Move constructor.
   */
  CylinderSDF(CylinderSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  CylinderSDF&
  operator=(const CylinderSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  CylinderSDF&
  operator=(CylinderSDF&&) = default;

  /**
   * @brief Get one endpoint
   * @return m_center1
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getCenter1() const noexcept
  {
    return m_center1;
  }

  /**
   * @brief Get the other endpoint
   * @return m_center2
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getCenter2() const noexcept
  {
    return m_center2;
  }

  /**
   * @brief Get radius.
   * @return m_radius.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getRadius() const noexcept
  {
    return m_radius;
  }

  /**
   * @brief Signed distance function for the cylinder.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the cylinder surface; negative inside.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    T d = Math::Limits<T>::infinity();

    if (m_length > T(0.0) && m_radius > T(0.0)) {
      EBGEOMETRY_EXPECT(std::abs(m_axis.length() - T(1)) < std::sqrt(Math::Limits<T>::epsilon()));

      const Vec3T<T> point = a_point - m_center;
      const T        para  = dot(point, m_axis);
      const Vec3T<T> ortho = point - para * m_axis; // Distance from cylinder axis.

      // w: Distance from cylinder wall. < 0 on inside and > 0 on outside.
      // h: Distance from cylinder top.  < 0 on inside and > 0 on outside.
      const T w = ortho.length() - m_radius;
      const T h = std::abs(para) - T(0.5) * m_length;

      constexpr T zero = T(0.0);

      if (w <= zero && h <= zero) {
        // Inside cylinder
        d = (std::abs(w) < std::abs(h)) ? w : h;
      }
      else if (w <= zero && h > zero) {
        // Above one of the endcaps.
        d = h;
      }
      else if (w > zero && h < zero) {
        // Outside radius but between the endcaps.
        d = w;
      }
      else {
        d = std::sqrt(w * w + h * h);
      }
    }

    return d;
  }

  /**
   * @brief How many parts the shape has (see hasParts): 3.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  getNumParts() const noexcept
  {
    return 3U;
  }

  /**
   * @brief The part whose term decides the value at a point (see hasParts): 0 the side, 1 the cap at the first center, 2 the cap at the second.
   * @param[in] a_point Position.
   * @return The part.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  closestPart(const Vec3T<T>& a_point) const noexcept
  {
    const Vec3T<T> point = a_point - m_center;
    const T        para  = dot(point, m_axis);
    const T        w     = (point - para * m_axis).length() - m_radius;
    const T        h     = std::abs(para) - T(0.5) * m_length;

    // The side where its term decides (inside, the nearer of the two; outside, the larger).
    const bool side = (w <= T(0) && h <= T(0)) ? (std::abs(w) < std::abs(h)) : (w > h);

    return side ? 0U : ((para < T(0)) ? 1U : 2U);
  }

  /**
   * @brief The shape's axis-aligned bounding box.
   * @details The box around the two cap discs: along axis i, each disc reaches the radius times
   * sqrt(1 - axis_i^2) beyond its center.
   * @return The bounding box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    Vec3T<T> reach;

    for (size_t dir = 0; dir < 3; dir++) {
      reach[dir] = m_radius * std::sqrt(Math::max(T(0), T(1) - m_axis[dir] * m_axis[dir]));
    }

    return BoundingVolumes::AABBT<T>(min(m_center1, m_center2) - reach, max(m_center1, m_center2) + reach);
  }

protected:
  /**
   * @brief One endpoint (cap center).
   */
  Vec3T<T> m_center1 = Vec3T<T>(T(0), T(-0.5), T(0));

  /**
   * @brief Other endpoint (cap center).
   */
  Vec3T<T> m_center2 = Vec3T<T>(T(0), T(0.5), T(0));

  /**
   * @brief Midpoint of m_center1 and m_center2.
   */
  Vec3T<T> m_center = Vec3T<T>::zeros();

  /**
   * @brief Unit axis pointing from m_center1 to m_center2.
   */
  Vec3T<T> m_axis = Vec3T<T>(T(0), T(1), T(0));

  /**
   * @brief Distance between the two endpoints.
   */
  T m_length = T(1);

  /**
   * @brief Cylinder radius.
   */
  T m_radius = T(1);
};

/**
 * @brief Signed distance field for an infinitely long cylinder.
 * @details The cylinder is infinite in one coordinate direction (selected by `m_axis`: 0 = x,
 * 1 = y, 2 = z) and has a circular cross-section of radius `m_radius` centered at `m_center`
 * in the plane perpendicular to that axis. The SDF is the radial distance from the axis minus
 * the radius: negative inside the cylinder and positive outside. By default the cylinder has
 * radius 1, passes through the origin, and extends along y.
 * @tparam T Floating-point precision.
 */
template <class T>
class InfiniteCylinderSDF
{
  static_assert(std::is_floating_point_v<T>, "InfiniteCylinderSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: the Euclidean distance to the surface.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  /**
   * @brief Default constructor. Constructs an infinite cylinder of radius 1 centered at the origin
   * with its axis along y (axis index 1).
   */
  InfiniteCylinderSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_center Center point on the cylinder axis.
   * @param[in] a_radius Cylinder radius.
   * @param[in] a_axis   Coordinate axis index (0 = x, 1 = y, 2 = z).
   */
  InfiniteCylinderSDF(const Vec3T<T>& a_center, const T& a_radius, const size_t a_axis) noexcept
  {
    EBGEOMETRY_REQUIRE(std::isfinite(a_center[0]) && std::isfinite(a_center[1]) && std::isfinite(a_center[2]),
                       "InfiniteCylinderSDF: the center must be finite (%g, %g, %g)",
                       double(a_center[0]),
                       double(a_center[1]),
                       double(a_center[2]));
    EBGEOMETRY_REQUIRE(std::isfinite(a_radius) && a_radius > T(0),
                       "InfiniteCylinderSDF: the radius must be finite and positive (%g)",
                       double(a_radius));
    EBGEOMETRY_REQUIRE(a_axis < 3U, "InfiniteCylinderSDF: the axis must be 0, 1 or 2 (%zu)", a_axis);

    m_center = a_center;
    m_radius = a_radius;
    m_axis   = a_axis;
  }

  /**
   * @brief Copy constructor.
   */
  InfiniteCylinderSDF(const InfiniteCylinderSDF&) = default;

  /**
   * @brief Move constructor.
   */
  InfiniteCylinderSDF(InfiniteCylinderSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  InfiniteCylinderSDF&
  operator=(const InfiniteCylinderSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  InfiniteCylinderSDF&
  operator=(InfiniteCylinderSDF&&) = default;

  /**
   * @brief Signed distance function for the infinite cylinder.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the cylinder surface; negative inside.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    Vec3T<T> delta = a_point - m_center;
    delta[m_axis]  = 0.0;

    const T d = delta.length() - m_radius;

    return d;
  }

  /**
   * @brief The shape's axis-aligned bounding box.
   * @details The center plus and minus the radius across the axis, and unbounded along it.
   * @return The bounding box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    Vec3T<T> lo = m_center - m_radius * Vec3T<T>::ones();
    Vec3T<T> hi = m_center + m_radius * Vec3T<T>::ones();

    lo[m_axis] = -Math::Limits<T>::max();
    hi[m_axis] = Math::Limits<T>::max();

    return BoundingVolumes::AABBT<T>(lo, hi);
  }

  /**
   * @brief Get the point on the axis.
   * @return The point on the axis.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getCenter() const noexcept
  {
    return m_center;
  }

  /**
   * @brief Get the radius.
   * @return The radius.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getRadius() const noexcept
  {
    return m_radius;
  }

  /**
   * @brief Get the coordinate axis the cylinder runs along.
   * @return The coordinate axis the cylinder runs along.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const size_t&
  getAxis() const noexcept
  {
    return m_axis;
  }

protected:
  /**
   * @brief Center point on the cylinder axis.
   */
  Vec3T<T> m_center = Vec3T<T>::zeros();

  /**
   * @brief Cylinder radius.
   */
  T m_radius = T(1);

  /**
   * @brief Coordinate axis index (0 = x, 1 = y, 2 = z).
   */
  size_t m_axis = 1U;
};

/**
 * @brief Signed distance field for a capsule (pill shape): a cylinder capped with hemispheres.
 * @details A capsule is defined by two outer tip positions and a radius. The tips are the
 * outermost points of each hemispherical endcap. The total length (tip to tip) equals
 * `distance(tip1, tip2)`. The internal cylindrical body has length
 * `distance(tip1, tip2) - 2 * radius`, and the two hemispheres each contribute a further
 * `radius` to each end.
 *
 * Internally the hemisphere centers are stored as `m_center1` and `m_center2` (inset from the
 * tips by `radius` along the capsule axis). These are **not** the same as the tip positions
 * passed to the constructor.
 *
 * The SDF is negative inside the capsule and positive outside. By default the capsule is
 * centered at the origin, aligned along y, with tips at (0,-1,0) and (0,1,0) and radius 0.5,
 * giving a total height of 2 and a cylindrical body of length 1.
 * @tparam T Floating-point precision.
 */
template <class T>
class CapsuleSDF
{
  static_assert(std::is_floating_point_v<T>, "CapsuleSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: the Euclidean distance to the surface.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  /**
   * @brief Default constructor. Constructs a capsule with tips at (0,-1,0) and (0,1,0), radius 0.5.
   * @details Total height = 2, cylindrical body length = 1, aligned along y, centered at origin.
   * The stored hemisphere centers are at (0,-0.5,0) and (0,0.5,0).
   */
  CapsuleSDF() = default;

  /**
   * @brief Full constructor.
   * @details The hemisphere centers stored internally are derived from the tips:
   * `m_center1 = a_tip1 + a_radius * axis` and `m_center2 = a_tip2 - a_radius * axis`,
   * where `axis = (a_tip2 - a_tip1) / distance(a_tip1, a_tip2)`.
   * The cylindrical body length is `distance(a_tip1, a_tip2) - 2 * a_radius`, which must not be
   * negative: `distance(a_tip1, a_tip2) >= 2 * a_radius`. At equality the capsule is a sphere.
   * @param[in] a_tip1   Outer tip of one hemispherical cap (outermost point in that direction).
   * @param[in] a_tip2   Outer tip of the other hemispherical cap.
   * @param[in] a_radius Capsule radius (applied to both the tube and the hemispherical caps).
   */
  CapsuleSDF(const Vec3T<T>& a_tip1, const Vec3T<T>& a_tip2, const T& a_radius) noexcept
  {
    EBGEOMETRY_REQUIRE(std::isfinite(a_tip1[0]) && std::isfinite(a_tip1[1]) && std::isfinite(a_tip1[2]),
                       "CapsuleSDF: the first tip must be finite (%g, %g, %g)",
                       double(a_tip1[0]),
                       double(a_tip1[1]),
                       double(a_tip1[2]));
    EBGEOMETRY_REQUIRE(std::isfinite(a_tip2[0]) && std::isfinite(a_tip2[1]) && std::isfinite(a_tip2[2]),
                       "CapsuleSDF: the second tip must be finite (%g, %g, %g)",
                       double(a_tip2[0]),
                       double(a_tip2[1]),
                       double(a_tip2[2]));
    EBGEOMETRY_REQUIRE(std::isfinite(a_radius) && a_radius > T(0),
                       "CapsuleSDF: the radius must be finite and positive (%g)",
                       double(a_radius));
    EBGEOMETRY_REQUIRE((a_tip2 - a_tip1).length() >= T(2) * a_radius,
                       "CapsuleSDF: the tips must be at least two radii apart (distance %g, radius %g)",
                       double((a_tip2 - a_tip1).length()),
                       double(a_radius));

    const Vec3T<T> axis = (a_tip2 - a_tip1) / length(a_tip2 - a_tip1);

    m_center1 = a_tip1 + a_radius * axis;
    m_center2 = a_tip2 - a_radius * axis;
    m_radius  = a_radius;
  }

  /**
   * @brief Copy constructor.
   */
  CapsuleSDF(const CapsuleSDF&) = default;

  /**
   * @brief Move constructor.
   */
  CapsuleSDF(CapsuleSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  CapsuleSDF&
  operator=(const CapsuleSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  CapsuleSDF&
  operator=(CapsuleSDF&&) = default;

  /**
   * @brief Signed distance function for the capsule.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the capsule surface; negative inside.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    const Vec3T<T> v1 = a_point - m_center1;
    const Vec3T<T> v2 = m_center2 - m_center1;

    // Tips exactly two radii apart put both hemisphere centers at the same point: a sphere. Guard the
    // projection, which would otherwise divide zero by zero.
    const T len2 = dot(v2, v2);

    const T h = (len2 > T(0)) ? Math::clamp(dot(v1, v2) / len2, T(0.0), T(1.0)) : T(0.0);
    const T d = length(v1 - h * v2) - m_radius;

    return d;
  }

  /**
   * @brief How many parts the shape has (see hasParts): 3.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  getNumParts() const noexcept
  {
    return 3U;
  }

  /**
   * @brief The part whose term decides the value at a point (see hasParts): 0 the side, 1 the hemisphere at the first tip, 2 the one at the second.
   * @param[in] a_point Position.
   * @return The part.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  closestPart(const Vec3T<T>& a_point) const noexcept
  {
    const Vec3T<T> v1   = a_point - m_center1;
    const Vec3T<T> v2   = m_center2 - m_center1;
    const T        len2 = dot(v2, v2);
    const T        t    = (len2 > T(0)) ? dot(v1, v2) / len2 : T(0);

    return (t <= T(0)) ? 1U : ((t >= T(1)) ? 2U : 0U);
  }

  /**
   * @brief The shape's axis-aligned bounding box.
   * @details The two hemisphere centers plus and minus the radius along each axis.
   * @return The bounding box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    return BoundingVolumes::AABBT<T>(min(m_center1, m_center2) - m_radius * Vec3T<T>::ones(),
                                     max(m_center1, m_center2) + m_radius * Vec3T<T>::ones());
  }

  /**
   * @brief Get the center of the first hemispherical cap.
   * @return The center of the first hemispherical cap.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getCenter1() const noexcept
  {
    return m_center1;
  }

  /**
   * @brief Get the center of the second hemispherical cap.
   * @return The center of the second hemispherical cap.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getCenter2() const noexcept
  {
    return m_center2;
  }

  /**
   * @brief Get the radius.
   * @return The radius.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getRadius() const noexcept
  {
    return m_radius;
  }

protected:
  /**
   * @brief Center of one hemispherical cap.
   */
  Vec3T<T> m_center1 = Vec3T<T>(T(0), T(-0.5), T(0));

  /**
   * @brief Center of the other hemispherical cap.
   */
  Vec3T<T> m_center2 = Vec3T<T>(T(0), T(0.5), T(0));

  /**
   * @brief Capsule radius.
   */
  T m_radius = T(0.5);
};

/**
 * @brief Signed distance field for an infinite cone.
 * @details The cone tip is at `m_tip`. The cone body opens in the **-y** direction from the tip
 * (the interior, where SDF < 0, is the solid region extending downward along -y within the
 * cone surface). The full opening angle `a_angle` (tip-to-tip across the cone) is halved
 * internally; the half-angle is stored as `m_c = (std::sin(half_angle), std::cos(half_angle))`.
 *
 * For a 45° full opening angle the half-angle is 22.5°; a point directly below the tip at
 * depth `d` sits on the cone surface when its radial distance from the axis equals
 * `d * std::tan(half_angle)`.
 *
 * The cone is infinite: it has no base plane. By default the tip is at the origin and the full
 * opening angle is 45°, with the body extending along -y.
 * @tparam T Floating-point precision.
 */
template <class T>
class InfiniteConeSDF
{
  static_assert(std::is_floating_point_v<T>, "InfiniteConeSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: the Euclidean distance to the surface.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  /**
   * @brief Default constructor. Constructs an infinite cone with tip at the origin, 45° full
   * opening angle, and body extending in the -y direction.
   */
  InfiniteConeSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_tip   Cone tip position.
   * @param[in] a_angle Full opening angle in degrees (must be in (0°, 180°)). The half-angle
   * `a_angle / 2` is used internally and encoded as `m_c = (sin, cos)` of that half-angle.
   */
  InfiniteConeSDF(const Vec3T<T>& a_tip, const T& a_angle) noexcept
  {
    EBGEOMETRY_REQUIRE(std::isfinite(a_tip[0]) && std::isfinite(a_tip[1]) && std::isfinite(a_tip[2]),
                       "InfiniteConeSDF: the tip must be finite (%g, %g, %g)",
                       double(a_tip[0]),
                       double(a_tip[1]),
                       double(a_tip[2]));
    EBGEOMETRY_REQUIRE(a_angle > T(0) && a_angle < T(180),
                       "InfiniteConeSDF: the angle must be in (0, 180) degrees (%g)",
                       double(a_angle));

    m_tip  = a_tip;
    m_c[0] = std::sin(T(0.5) * a_angle * pi<T> / T(180));
    m_c[1] = std::cos(T(0.5) * a_angle * pi<T> / T(180));
  }

  /**
   * @brief Copy constructor.
   */
  InfiniteConeSDF(const InfiniteConeSDF&) = default;

  /**
   * @brief Move constructor.
   */
  InfiniteConeSDF(InfiniteConeSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  InfiniteConeSDF&
  operator=(const InfiniteConeSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  InfiniteConeSDF&
  operator=(InfiniteConeSDF&&) = default;

  /**
   * @brief Signed distance function for the infinite cone.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the cone surface; negative inside the cone.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));
    EBGEOMETRY_EXPECT(std::abs(length(m_c) - T(1)) < std::sqrt(Math::Limits<T>::epsilon()));

    const Vec3T<T> delta = a_point - m_tip;
    const Vec2T<T> q(std::sqrt(delta[0] * delta[0] + delta[2] * delta[2]), -delta[1]);

    const T d1 = length(q - m_c * Math::max(dot(q, m_c), T(0.0)));
    const T d2 = d1 * ((q[0] * m_c[1] - q[1] * m_c[0] < T(0.0)) ? T(-1.0) : T(1.0));

    return d2;
  }

  /**
   * @brief The shape's axis-aligned bounding box.
   * @details Bounded above by the tip in y, and unbounded below it and in x and z.
   * @return The bounding box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    Vec3T<T> hi = Vec3T<T>::max();

    hi[1] = m_tip[1];

    return BoundingVolumes::AABBT<T>(Vec3T<T>::min(), hi);
  }

protected:
  /**
   * @brief Tip position.
   */
  Vec3T<T> m_tip = Vec3T<T>::zeros();

  /**
   * @brief (sin, cos) of the half opening-angle. Default: 45° full angle → half-angle 22.5°.
   */
  Vec2T<T> m_c = Vec2T<T>(std::sin(pi<T> / T(8)), std::cos(pi<T> / T(8)));
};

/**
 * @brief Signed distance field for a finite cone.
 * @details The cone tip is at `m_tip` and the body opens in the **-y** direction, so the base
 * disc lies at `m_tip + (0, -m_height, 0)`. The full opening angle `a_angle` is the tip-to-tip
 * apex angle; internally the half-angle is encoded as `m_c = (std::sin(half_angle), std::cos(half_angle))`.
 * The base radius equals `m_height * std::tan(half_angle) = m_height * m_c[0] / m_c[1]`.
 *
 * The SDF is negative inside the solid cone (including the base disc) and positive outside.
 * By default the tip is at the origin, the height is 1, and the full opening angle is 45°,
 * giving a base radius of `std::tan(22.5°) ≈ 0.414`.
 * @tparam T Floating-point precision.
 */
template <class T>
class ConeSDF
{
  static_assert(std::is_floating_point_v<T>, "ConeSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: the Euclidean distance to the surface.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  /**
   * @brief Default constructor. Constructs a finite cone with its tip at the origin, height 1,
   * and a 45-degree opening angle, body extending in the -y direction.
   */
  ConeSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_tip    Cone tip position.
   * @param[in] a_height Cone height (tip to base).
   * @param[in] a_angle  Full opening angle in degrees (must be in (0°, 180°)).
   */
  ConeSDF(const Vec3T<T>& a_tip, const T& a_height, const T& a_angle) noexcept
  {
    EBGEOMETRY_REQUIRE(std::isfinite(a_tip[0]) && std::isfinite(a_tip[1]) && std::isfinite(a_tip[2]),
                       "ConeSDF: the tip must be finite (%g, %g, %g)",
                       double(a_tip[0]),
                       double(a_tip[1]),
                       double(a_tip[2]));
    EBGEOMETRY_REQUIRE(std::isfinite(a_height) && a_height > T(0),
                       "ConeSDF: the height must be finite and positive (%g)",
                       double(a_height));
    EBGEOMETRY_REQUIRE(
      a_angle > T(0) && a_angle < T(180), "ConeSDF: the angle must be in (0, 180) degrees (%g)", double(a_angle));

    m_tip    = a_tip;
    m_height = a_height;
    m_c[0]   = std::sin(T(0.5) * a_angle * pi<T> / T(180));
    m_c[1]   = std::cos(T(0.5) * a_angle * pi<T> / T(180));
  }

  /**
   * @brief Copy constructor.
   */
  ConeSDF(const ConeSDF&) = default;

  /**
   * @brief Move constructor.
   */
  ConeSDF(ConeSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  ConeSDF&
  operator=(const ConeSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  ConeSDF&
  operator=(ConeSDF&&) = default;

  /**
   * @brief Signed distance function for the finite cone.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the cone surface; negative inside the cone.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));
    EBGEOMETRY_EXPECT(std::abs(length(m_c) - T(1)) < std::sqrt(Math::Limits<T>::epsilon()));
    EBGEOMETRY_EXPECT(m_c[1] > T(0));

    const Vec3T<T> delta = a_point - m_tip;
    const T        dr    = std::sqrt(delta[0] * delta[0] + delta[2] * delta[2]);
    const T        dz    = delta[1];

    constexpr T zero = T(0.0);
    constexpr T one  = T(1.0);

    const Vec2T<T> q = m_height * Vec2T<T>(m_c[0] / m_c[1], -1.0);
    const Vec2T<T> w = Vec2T<T>(dr, dz);
    const Vec2T<T> a = w - Math::clamp(dot(w, q) / dot(q, q), zero, one) * q;
    const Vec2T<T> b = w - Vec2T<T>(q[0] * Math::clamp(w[0] / q[0], zero, one), q[1]);

    auto sign = [](const T& x) -> int { return (x > zero) - (x < zero); };

    const T k = sign(q[1]);
    const T d = Math::min(dot(a, a), dot(b, b));
    const T s = Math::max(k * (w[0] * q[1] - w[1] * q[0]), k * (w[1] - q[1]));

    return std::sqrt(d) * sign(s);
  }

  /**
   * @brief How many parts the shape has (see hasParts): 2.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  getNumParts() const noexcept
  {
    return 2U;
  }

  /**
   * @brief The part whose term decides the value at a point (see hasParts): 0 the slanted side, 1 the base.
   * @param[in] a_point Position.
   * @return The part.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  closestPart(const Vec3T<T>& a_point) const noexcept
  {
    const Vec3T<T> delta = a_point - m_tip;
    const Vec2T<T> q     = m_height * Vec2T<T>(m_c[0] / m_c[1], -1.0);
    const Vec2T<T> w     = Vec2T<T>(std::sqrt(delta[0] * delta[0] + delta[2] * delta[2]), delta[1]);
    const Vec2T<T> a     = w - Math::clamp(dot(w, q) / dot(q, q), T(0), T(1)) * q;
    const Vec2T<T> b     = w - Vec2T<T>(q[0] * Math::clamp(w[0] / q[0], T(0), T(1)), q[1]);

    return (dot(a, a) <= dot(b, b)) ? 0U : 1U;
  }

  /**
   * @brief The shape's axis-aligned bounding box.
   * @details In y, from the base disc a height below the tip up to the tip; in x and z, the base
   * radius (the height times the tangent of the half-angle) either side of the axis.
   * @return The bounding box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    const T baseRadius = m_height * m_c[0] / m_c[1];

    return BoundingVolumes::AABBT<T>(m_tip - Vec3T<T>(baseRadius, m_height, baseRadius),
                                     m_tip + Vec3T<T>(baseRadius, T(0), baseRadius));
  }

protected:
  /**
   * @brief Tip position.
   */
  Vec3T<T> m_tip = Vec3T<T>::zeros();

  /**
   * @brief (sin, cos) of the half opening-angle. Default: 45° full angle → half-angle 22.5°.
   */
  Vec2T<T> m_c = Vec2T<T>(std::sin(pi<T> / T(8)), std::cos(pi<T> / T(8)));

  /**
   * @brief Cone height (tip to base).
   */
  T m_height = T(1);
};

/**
 * @brief Signed distance field for an axis-aligned box with rounded edges and corners.
 * @details The constructor takes the box's center, its outer size (the full side lengths of the
 * finished, rounded box, which is also its bounding box) and the rounding radius. The rounding is
 * a sphere of that radius swept over an inner box whose half-extents, `m_dimensions = 0.5 * size -
 * curvature`, are stored. The SDF is negative inside and positive outside.
 * @tparam T Floating-point precision.
 */
template <class T>
class RoundedBoxSDF
{
  static_assert(std::is_floating_point_v<T>, "RoundedBoxSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: the Euclidean distance to the surface.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  /**
   * @brief Default constructor. Constructs a rounded unit cube, [-0.5, 0.5]^3, centered at the
   * origin, with rounding radius 0.1.
   */
  RoundedBoxSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_center    Box center.
   * @param[in] a_size      Outer size: the full side lengths of the rounded box. Each must exceed
   * twice the rounding radius.
   * @param[in] a_curvature Rounding radius of the edges and corners. Must be > 0.
   */
  RoundedBoxSDF(const Vec3T<T>& a_center, const Vec3T<T>& a_size, const T a_curvature) noexcept
  {
    EBGEOMETRY_REQUIRE(std::isfinite(a_center[0]) && std::isfinite(a_center[1]) && std::isfinite(a_center[2]),
                       "RoundedBoxSDF: the center must be finite (%g, %g, %g)",
                       double(a_center[0]),
                       double(a_center[1]),
                       double(a_center[2]));
    EBGEOMETRY_REQUIRE(std::isfinite(a_curvature) && a_curvature > T(0),
                       "RoundedBoxSDF: the curvature must be finite and positive (%g)",
                       double(a_curvature));
    EBGEOMETRY_REQUIRE(std::isfinite(a_size[0]) && std::isfinite(a_size[1]) && std::isfinite(a_size[2]) &&
                         a_size[0] > T(2) * a_curvature && a_size[1] > T(2) * a_curvature &&
                         a_size[2] > T(2) * a_curvature,
                       "RoundedBoxSDF: the size (%g, %g, %g) must be finite and exceed twice the curvature (%g)",
                       double(a_size[0]),
                       double(a_size[1]),
                       double(a_size[2]),
                       double(a_curvature));

    m_center     = a_center;
    m_dimensions = T(0.5) * a_size - a_curvature * Vec3T<T>::ones();
    m_curvature  = a_curvature;
  }

  /**
   * @brief Copy constructor.
   */
  RoundedBoxSDF(const RoundedBoxSDF&) = default;

  /**
   * @brief Move constructor.
   */
  RoundedBoxSDF(RoundedBoxSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  RoundedBoxSDF&
  operator=(const RoundedBoxSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  RoundedBoxSDF&
  operator=(RoundedBoxSDF&&) = default;

  /**
   * @brief Signed distance function for the rounded box.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the rounded-box surface; negative inside.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    // q is the point's per-axis distance beyond the inner box (negative inside it). Outside, the
    // distance to the inner box is the length of q's positive part; inside, it is the distance to
    // the nearest face, max(q) <= 0. Either way the rounding moves the surface out by m_curvature.
    const Vec3T<T> p = a_point - m_center;
    const Vec3T<T> q(
      std::abs(p[0]) - m_dimensions[0], std::abs(p[1]) - m_dimensions[1], std::abs(p[2]) - m_dimensions[2]);

    const T outside = length(max(q, Vec3T<T>::zeros()));
    const T inside  = Math::min(Math::max(q[0], Math::max(q[1], q[2])), T(0));

    return outside + inside - m_curvature;
  }

  /**
   * @brief How many parts the shape has (see hasParts): 6.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  getNumParts() const noexcept
  {
    return 6U;
  }

  /**
   * @brief The part whose term decides the value at a point (see hasParts): 2 axis + 0 for the face on the negative side, 2 axis + 1 for the one on the positive side (-x, +x, -y, +y, -z, +z), the axis whose term is largest; a rounded edge or corner goes to that face.
   * @param[in] a_point Position.
   * @return The part.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  closestPart(const Vec3T<T>& a_point) const noexcept
  {
    const Vec3T<T> p = a_point - m_center;
    const Vec3T<T> q(
      std::abs(p[0]) - m_dimensions[0], std::abs(p[1]) - m_dimensions[1], std::abs(p[2]) - m_dimensions[2]);

    const size_t axis = q.maxDir(false);

    return 2U * uint32_t(axis) + ((p[axis] > T(0)) ? 1U : 0U);
  }

  /**
   * @brief The shape's axis-aligned bounding box.
   * @details The center plus and minus half the outer size.
   * @return The bounding box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    const Vec3T<T> half = m_dimensions + m_curvature * Vec3T<T>::ones();

    return BoundingVolumes::AABBT<T>(m_center - half, m_center + half);
  }

protected:
  /**
   * @brief Corner rounding radius.
   */
  T m_curvature = T(0.1);

  /**
   * @brief Box center.
   */
  Vec3T<T> m_center = Vec3T<T>::zeros();

  /**
   * @brief Half-extents of the inner box (= 0.5 * outer size - curvature).
   */
  Vec3T<T> m_dimensions = Vec3T<T>(T(0.4), T(0.4), T(0.4));
};

/**
 * @brief Ken Perlin's gradient-noise implicit function.
 * @details This class evaluates multi-octave Perlin noise at a 3-D point and returns the result
 * as a scalar. It is **not** a true signed distance function (the gradient magnitude is not
 * guaranteed to equal 1), but it can serve as a procedural displacement field when added to or
 * subtracted from a conventional SDF.
 *
 * The output of `signedDistance` is on the range `[0, m_noiseAmplitude]` for a single octave.
 * Multiple octaves sum scaled contributions: each successive octave divides the spatial frequency by
 * `m_noisePersistence` (doubling it at the default 0.5) and multiplies the amplitude by
 * `m_noisePersistence`.
 *
 * The full constructor shuffles the permutation table with a std::mt19937 seeded with 0, so every
 * PerlinSDF built with the same parameters is the same noise field; call shuffle() for another.
 *
 * @tparam T Floating-point precision.
 */
template <class T>
class PerlinSDF
{
  static_assert(std::is_floating_point_v<T>, "PerlinSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: not at all. The value is noise, so
   * only its sign means anything.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::NotADistance;

  /**
   * @brief Default constructor. Constructs a single-octave Perlin noise field with unit amplitude,
   * unit frequency along all axes, and persistence 0.5: the full constructor with those values.
   */
  PerlinSDF() noexcept : PerlinSDF(T(1), Vec3T<T>::ones(), T(0.5), 1U)
  {}

  /**
   * @brief Full constructor.
   * @param[in] a_noiseAmplitude   Noise amplitude (output scale).
   * @param[in] a_noiseFrequency   Spatial frequency along each Cartesian axis.
   * @param[in] a_noisePersistence Per-octave amplitude decay factor, at most 1. A value of zero or
   * less keeps only the first octave, since every later one would have zero amplitude.
   * @param[in] a_noiseOctaves     Number of noise octaves. Clamped to >= 1.
   */
  PerlinSDF(const T            a_noiseAmplitude,
            const Vec3T<T>     a_noiseFrequency,
            const T            a_noisePersistence,
            const unsigned int a_noiseOctaves) noexcept
  {
    EBGEOMETRY_REQUIRE(
      std::isfinite(a_noiseAmplitude), "PerlinSDF: the noise amplitude must be finite (%g)", double(a_noiseAmplitude));
    EBGEOMETRY_REQUIRE(std::isfinite(a_noiseFrequency[0]) && std::isfinite(a_noiseFrequency[1]) &&
                         std::isfinite(a_noiseFrequency[2]),
                       "PerlinSDF: the noise frequency must be finite (%g, %g, %g)",
                       double(a_noiseFrequency[0]),
                       double(a_noiseFrequency[1]),
                       double(a_noiseFrequency[2]));
    EBGEOMETRY_REQUIRE(std::isfinite(a_noisePersistence),
                       "PerlinSDF: the noise persistence must be finite (%g)",
                       double(a_noisePersistence));

    m_noiseAmplitude   = a_noiseAmplitude;
    m_noiseFrequency   = a_noiseFrequency;
    m_noisePersistence = Math::min(T(1), a_noisePersistence);
    m_noiseOctaves     = Math::max(1U, a_noiseOctaves);

    // Each octave divides the frequency by the persistence, so zero would give an infinite frequency
    // (and NaN noise) for an octave that contributes nothing anyway.
    if (!(m_noisePersistence > T(0))) {
      m_noisePersistence = T(1);
      m_noiseOctaves     = 1U;
    }

    std::mt19937 g(0);
    this->shuffle(g);
  }

  /**
   * @brief Copy constructor.
   */
  PerlinSDF(const PerlinSDF&) = default;

  /**
   * @brief Move constructor.
   */
  PerlinSDF(PerlinSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  PerlinSDF&
  operator=(const PerlinSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  PerlinSDF&
  operator=(PerlinSDF&&) = default;

  /**
   * @brief Signed distance function. Generates a noise value on [0, m_noiseAmplitude] for one octave.
   * @param[in] a_point Input point.
   * @return Octave-summed Perlin noise value scaled by m_noiseAmplitude.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    T ret = 0.0;

    Vec3T<T> curFreq = m_noiseFrequency;
    T        curAmp  = 1.0;

    for (unsigned int curOctave = 0; curOctave < m_noiseOctaves; curOctave++) {
      ret += T(0.5) * curAmp * (T(1) + this->noise(a_point * curFreq));

      curFreq = curFreq / m_noisePersistence;
      curAmp  = curAmp * m_noisePersistence;
    }

    return ret * m_noiseAmplitude;
  };

  /**
   * @brief Shuffle the permutation table with the provided uniform random number generator.
   * @details Resets the first 256 entries of the permutation table to 0, ..., 255, permutes them with
   * std::shuffle and then mirrors them into the upper 256 entries. The URNG type must satisfy the C++ UniformRandomBitGenerator
   * concept (e.g., std::mt19937).
   * @tparam URNG Uniform random number generator type (e.g., std::mt19937).
   * @param[in] g Uniform random number generator instance to drive the shuffle.
   * @note When using parallel calculations it is exceptionally important that the input RNG
   * produces the same sequence across all threads/ranks. Otherwise, each thread/rank will
   * generate a different permutation table and therefore a different noise field, resulting
   * in no single coherent geometry.
   */
  template <class URNG>
  void
  shuffle(URNG& g) noexcept
  {

    for (unsigned int i = 0; i < 256; i++) {
      m_permutationTable[i] = static_cast<int>(i);
    }

    std::shuffle(m_permutationTable.begin(), m_permutationTable.begin() + 256, g);

    for (int i = 0; i < 256; i++) {
      m_permutationTable[i + 256] = m_permutationTable[i];
    }
  }

  /**
   * @brief Get the internal permutation table
   * @return m_permutationTable.
   */
  Array<int, 512>&
  getPermutationTable() noexcept
  {
    return m_permutationTable;
  }

  /**
   * @brief The shape's axis-aligned bounding box.
   * @details Noise fills all of space, so the box is the whole space.
   * @return The bounding box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    return BoundingVolumes::AABBT<T>(Vec3T<T>::min(), Vec3T<T>::max());
  }

protected:
  /**
   * @brief Spatial noise frequency along each Cartesian axis.
   */
  Vec3T<T> m_noiseFrequency = Vec3T<T>(T(1), T(1), T(1));

  /**
   * @brief Noise amplitude (output scale).
   */
  T m_noiseAmplitude = T(1);

  /**
   * @brief Per-octave amplitude decay factor.
   */
  T m_noisePersistence = T(0.5);

  /**
   * @brief Integer hash permutation table (512 entries: [0..255] mirrored).
   * @details Filled by every constructor (the default constructor delegates to the full one) and
   * refilled by shuffle().
   */
  Array<int, 512> m_permutationTable = {};

  /**
   * @brief Number of noise octaves.
   */
  unsigned int m_noiseOctaves = 1U;

  /**
   * @brief Ken Perlin's linear interpolation helper.
   * @param[in] t Interpolation weight in [0,1].
   * @param[in] a Start value.
   * @param[in] b End value.
   * @return a + t*(b - a).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  lerp(const T t, const T a, const T b) const noexcept
  {
    return a + t * (b - a);
  };

  /**
   * @brief Ken Perlin's fade (smoothstep) function: 6t^5 - 15t^4 + 10t^3.
   * @param[in] t Input in [0,1].
   * @return Smoothed value in [0,1].
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  fade(const T t) const noexcept
  {
    return t * t * t * (t * (t * 6 - 15) + 10);
  };

  /**
   * @brief Ken Perlin's gradient hash function.
   * @param[in] hash Hash value selecting one of 16 gradient directions.
   * @param[in] x    x-component of the relative position.
   * @param[in] y    y-component of the relative position.
   * @param[in] z    z-component of the relative position.
   * @return Dot product of the selected pseudo-random gradient with (x, y, z).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  grad(const int hash, const T x, const T y, const T z) const noexcept
  {
    const int h = hash & 15;
    const T   u = h < 8 ? x : y;
    const T   v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
  }

  /**
   * @brief Single-octave Perlin noise evaluation.
   * @param[in] a_point Input point in noise space.
   * @return Noise value in [-1, 1].
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  noise(const Vec3T<T>& a_point) const noexcept
  {
    // Lower cube corner
    const int X = static_cast<int>(std::floor(a_point[0])) & 255;
    const int Y = static_cast<int>(std::floor(a_point[1])) & 255;
    const int Z = static_cast<int>(std::floor(a_point[2])) & 255;

    // Relative distance wrt lower cube corner
    const T x = a_point[0] - std::floor(a_point[0]);
    const T y = a_point[1] - std::floor(a_point[1]);
    const T z = a_point[2] - std::floor(a_point[2]);

    // Fade curves
    const T u = fade(x);
    const T v = fade(y);
    const T w = fade(z);

    // Hash coordinates of 8 cube corners
    const int A  = m_permutationTable[X] + Y;
    const int AA = m_permutationTable[A] + Z;
    const int AB = m_permutationTable[A + 1] + Z;
    const int B  = m_permutationTable[X + 1] + Y;
    const int BA = m_permutationTable[B] + Z;
    const int BB = m_permutationTable[B + 1] + Z;

    // Add blended results from 8 corners of cube
    return lerp(
      w,
      lerp(v,
           lerp(u, grad(m_permutationTable[AA], x, y, z), grad(m_permutationTable[BA], x - 1, y, z)),
           lerp(u, grad(m_permutationTable[AB], x, y - 1, z), grad(m_permutationTable[BB], x - 1, y - 1, z))),
      lerp(v,
           lerp(u, grad(m_permutationTable[AA + 1], x, y, z - 1), grad(m_permutationTable[BA + 1], x - 1, y, z - 1)),
           lerp(u,
                grad(m_permutationTable[AB + 1], x, y - 1, z - 1),
                grad(m_permutationTable[BB + 1], x - 1, y - 1, z - 1))));
  };
};

/**
 * @brief Signed distance field for a cylinder with rounded (toroidal) edges.
 * @details The cylinder is centered at `m_center` with its axis along **y**. The constructor
 * takes the center, the outer radius, the edge curvature radius and the total height. The curvature rounds
 * the sharp edges where the flat caps meet the cylindrical wall; the resulting shape is sometimes
 * called a "stadium of revolution" or a "pill cylinder".
 *
 * Internally three derived parameters are stored:
 * - `m_majorRadius = a_radius - a_curvature`: radius of the inner cylindrical core (before edge
 * rounding).
 * - `m_minorRadius = a_curvature`: radius of the torus-like edge rounding.
 * - `m_height = 0.5 * a_height - a_curvature`: half-height of the inner cylindrical core (before
 * edge rounding).
 *
 * The outer bounding cylinder has radius `m_majorRadius + m_minorRadius = a_radius` and
 * half-height `m_height + m_minorRadius = 0.5 * a_height`. For the geometry to be valid,
 * `a_curvature` must be strictly less than `a_radius` and less than `0.5 * a_height`.
 *
 * The SDF is negative inside and positive outside. By default the cylinder has outer radius 1,
 * height 1, and edge curvature 0.1 (stored as m_majorRadius=0.9, m_minorRadius=0.1, m_height=0.4).
 * @tparam T Floating-point precision.
 */
template <class T>
class RoundedCylinderSDF
{
  static_assert(std::is_floating_point_v<T>, "RoundedCylinderSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: the Euclidean distance to the surface.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  /**
   * @brief Default constructor. Constructs a rounded cylinder centered at the origin, axis along y,
   * with outer radius 1, total height 1, and edge curvature 0.1.
   */
  RoundedCylinderSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_center    Cylinder center.
   * @param[in] a_radius    Outer cylinder radius. Must be > 0 and > a_curvature.
   * @param[in] a_curvature Edge rounding radius. Must be > 0 and satisfy
   * a_curvature < a_radius and 2 * a_curvature < a_height.
   * @param[in] a_height    Total cylinder height. Must be > 0 and > 2 * a_curvature.
   */
  RoundedCylinderSDF(const Vec3T<T>& a_center, const T a_radius, const T a_curvature, const T a_height) noexcept
  {
    EBGEOMETRY_REQUIRE(std::isfinite(a_center[0]) && std::isfinite(a_center[1]) && std::isfinite(a_center[2]),
                       "RoundedCylinderSDF: the center must be finite (%g, %g, %g)",
                       double(a_center[0]),
                       double(a_center[1]),
                       double(a_center[2]));
    EBGEOMETRY_REQUIRE(std::isfinite(a_radius) && a_radius > T(0),
                       "RoundedCylinderSDF: the radius must be finite and positive (%g)",
                       double(a_radius));
    EBGEOMETRY_REQUIRE(std::isfinite(a_curvature) && a_curvature > T(0),
                       "RoundedCylinderSDF: the curvature must be finite and positive (%g)",
                       double(a_curvature));
    EBGEOMETRY_REQUIRE(std::isfinite(a_height) && a_height > T(0),
                       "RoundedCylinderSDF: the height must be finite and positive (%g)",
                       double(a_height));
    EBGEOMETRY_REQUIRE(a_curvature < a_radius,
                       "RoundedCylinderSDF: the curvature (%g) must be less than the radius (%g)",
                       double(a_curvature),
                       double(a_radius));
    EBGEOMETRY_REQUIRE(T(2) * a_curvature < a_height,
                       "RoundedCylinderSDF: twice the curvature (%g) must be less than the height (%g)",
                       double(a_curvature),
                       double(a_height));

    m_center      = a_center;
    m_majorRadius = a_radius - a_curvature;
    m_minorRadius = a_curvature;
    m_height      = T(0.5) * a_height - a_curvature;
  }

  /**
   * @brief Copy constructor.
   */
  RoundedCylinderSDF(const RoundedCylinderSDF&) = default;

  /**
   * @brief Move constructor.
   */
  RoundedCylinderSDF(RoundedCylinderSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  RoundedCylinderSDF&
  operator=(const RoundedCylinderSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  RoundedCylinderSDF&
  operator=(RoundedCylinderSDF&&) = default;

  /**
   * @brief Signed distance function for the rounded cylinder.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the rounded-cylinder surface; negative inside.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    const Vec3T<T> p  = a_point - m_center;
    const T        xz = std::sqrt(p[2] * p[2] + p[0] * p[0]);
    const auto     d1 = Vec2T<T>(xz - m_majorRadius, std::abs(p[1]) - m_height);
    const auto     d2 = Vec2T<T>(Math::max(d1[0], T(0)), Math::max(d1[1], T(0)));

    return Math::min(Math::max(d1[0], d1[1]), T(0)) + std::sqrt(d2[0] * d2[0] + d2[1] * d2[1]) - m_minorRadius;
  }

  /**
   * @brief How many parts the shape has (see hasParts): 3.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  getNumParts() const noexcept
  {
    return 3U;
  }

  /**
   * @brief The part whose term decides the value at a point (see hasParts): 0 the side, 1 the cap at -y, 2 the cap at +y; a rounded edge goes to the term that is larger there.
   * @param[in] a_point Position.
   * @return The part.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  closestPart(const Vec3T<T>& a_point) const noexcept
  {
    const Vec3T<T> p  = a_point - m_center;
    const T        xz = std::sqrt(p[2] * p[2] + p[0] * p[0]);
    const T        r  = xz - m_majorRadius;
    const T        y  = std::abs(p[1]) - m_height;

    return (r >= y) ? 0U : ((p[1] < T(0)) ? 1U : 2U);
  }

  /**
   * @brief The shape's axis-aligned bounding box.
   * @details The outer radius either side of the axis in x and z, and half the height in y.
   * @return The bounding box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    const T        radius = m_majorRadius + m_minorRadius;
    const Vec3T<T> half(radius, m_height + m_minorRadius, radius);

    return BoundingVolumes::AABBT<T>(m_center - half, m_center + half);
  }

protected:
  /**
   * @brief Cylinder center.
   */
  Vec3T<T> m_center = Vec3T<T>::zeros();

  /**
   * @brief Inner cylinder radius (= outer radius - curvature).
   */
  T m_majorRadius = T(0.9);

  /**
   * @brief Edge rounding radius (= curvature).
   */
  T m_minorRadius = T(0.1);

  /**
   * @brief Inner half-height (= 0.5*height - curvature).
   */
  T m_height = T(0.4);
};

namespace AnalyticDetail {

/**
 * @brief A point's coordinates around an axis: its distance from the axis line, and its position along it.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 */
template <class T>
struct AxialCoordinates
{
  /**
   * @brief Distance from the axis line (never negative).
   */
  T m_radial;

  /**
   * @brief Position along the axis, from the axis' start point.
   */
  T m_axial;
};

/**
 * @brief A point's coordinates around the axis that starts at a_start and runs along the unit a_axis.
 * @details Internal helper; not part of the public API. The radial distance is the length of the point's
 * offset across the axis, not sqrt(|p|^2 - axial^2), which cancels near the axis.
 * @tparam T Floating-point precision.
 * @param[in] a_point Point.
 * @param[in] a_start Start of the axis.
 * @param[in] a_axis  Unit axis.
 * @return The coordinates.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline AxialCoordinates<T>
axialCoordinates(const Vec3T<T>& a_point, const Vec3T<T>& a_start, const Vec3T<T>& a_axis) noexcept
{
  const Vec3T<T> w     = a_point - a_start;
  const T        along = dot(w, a_axis);

  return AxialCoordinates<T>{(w - along * a_axis).length(), along};
}

/**
 * @brief The signed distance to a 2D profile extruded between two parallel planes: the exact distance,
 * from the profile's signed distance and the signed distance to the slab between the planes.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_profile Signed distance to the profile, in the cross-section.
 * @param[in] a_slab    Signed distance to the slab, along the axis.
 * @return The signed distance to the extruded solid.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline T
extrusion(const T a_profile, const T a_slab) noexcept
{
  const T outsideProfile = Math::max(a_profile, T(0));
  const T outsideSlab    = Math::max(a_slab, T(0));

  return Math::min(Math::max(a_profile, a_slab), T(0)) +
         std::sqrt(outsideProfile * outsideProfile + outsideSlab * outsideSlab);
}

/**
 * @brief Checks that a point given to a constructor is finite.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_point The point.
 * @return Whether every coordinate is finite.
 */
template <class T>
[[nodiscard]] inline bool
isFinite(const Vec3T<T>& a_point) noexcept
{
  return std::isfinite(a_point[0]) && std::isfinite(a_point[1]) && std::isfinite(a_point[2]);
}

/**
 * @brief The root of Eberly's function F(s) = sum_i (a_n[i] / (s + a_r[i]))^2 - 1 over the components with
 * a_n[i] > 0, for the closest point on an ellipse or ellipsoid scaled so that its smallest semi-axis is 1.
 * @details Internal helper; not part of the public API. F is decreasing and convex for s > -min(a_r), and
 * its root lies in [a_lo, a_hi], where F(a_lo) >= 0 (Eberly, "Distance from a point to an ellipse, an
 * ellipsoid, or a hyperellipsoid", 2013). Newton's method from a_lo therefore increases monotonically to
 * the root and never overshoots it; a step that would leave the bracket (by rounding) is replaced by
 * bisection. It stops when F is no longer positive, when a step makes no progress or changes s by no more
 * than a few units in the last place, or after MaxIterations.
 * @tparam T Floating-point precision.
 * @tparam N Number of components (2 or 3).
 * @param[in] a_r  The squared ratios of the semi-axes to the smallest one.
 * @param[in] a_n  The scaled coordinates of the point, a_r[i] times the point's coordinate over the semi-axis.
 * @param[in] a_lo Lower end of the bracket, with F(a_lo) >= 0.
 * @param[in] a_hi Upper end of the bracket, with F(a_hi) <= 0.
 * @return The root.
 */
template <class T, size_t N>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline T
ellipsoidRoot(const T (&a_r)[N], const T (&a_n)[N], const T a_lo, const T a_hi) noexcept
{
  constexpr int MaxIterations = 64;

  const T eps = Math::Limits<T>::epsilon();

  // Only s moves: hi stays a_hi, a fixed safeguard for a step that rounding pushes past the root, not a
  // bracket that shrinks.
  T s  = a_lo;
  T hi = a_hi;

  for (int i = 0; i < MaxIterations; i++) {
    T f  = T(-1);
    T df = T(0);

    for (size_t k = 0; k < N; k++) {
      const T ratio = a_n[k] / (s + a_r[k]);

      f += ratio * ratio;
      df -= T(2) * ratio * ratio / (s + a_r[k]);
    }

    if (f <= T(0)) {
      break;
    }

    T next = (df < T(0)) ? s - f / df : hi;

    // No progress: s is the root to rounding. (Bisecting from here would step past it.)
    if (!(next > s)) {
      break;
    }
    if (next > hi) {
      next = T(0.5) * (s + hi);
    }

    const bool converged = next - s <= T(4) * eps * Math::max(T(1), std::abs(s));

    s = next;

    if (converged) {
      break;
    }
  }

  return s;
}

/**
 * @brief The distance from a point to an ellipse with semi-axes a_e0 >= a_e1 > 0, for a point with
 * coordinates a_y0, a_y1 >= 0 (Eberly's algorithm).
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_e0 Larger semi-axis.
 * @param[in] a_e1 Smaller semi-axis.
 * @param[in] a_y0 Coordinate along a_e0.
 * @param[in] a_y1 Coordinate along a_e1.
 * @return The unsigned distance to the ellipse curve.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline T
ellipseDistance(const T a_e0, const T a_e1, const T a_y0, const T a_y1) noexcept
{
  if (a_y1 > T(0)) {
    if (a_y0 > T(0)) {
      const T z0 = a_y0 / a_e0;
      const T z1 = a_y1 / a_e1;
      const T g  = z0 * z0 + z1 * z1 - T(1);

      if (g == T(0)) {
        return T(0);
      }

      const T r0 = (a_e0 / a_e1) * (a_e0 / a_e1);
      const T r[2]{r0, T(1)};
      const T n[2]{r0 * z0, z1};
      const T s = ellipsoidRoot<T, 2>(r, n, z1 - T(1), (g < T(0)) ? T(0) : std::sqrt(n[0] * n[0] + z1 * z1) - T(1));

      const T x0 = r0 * a_y0 / (s + r0);
      const T x1 = a_y1 / (s + T(1));

      return std::sqrt((x0 - a_y0) * (x0 - a_y0) + (x1 - a_y1) * (x1 - a_y1));
    }

    return std::abs(a_y1 - a_e1);
  }

  const T numer0 = a_e0 * a_y0;
  const T denom0 = a_e0 * a_e0 - a_e1 * a_e1;

  if (numer0 < denom0) {
    const T xde0 = numer0 / denom0;
    const T x0   = a_e0 * xde0;
    const T x1   = a_e1 * std::sqrt(Math::max(T(0), T(1) - xde0 * xde0));

    return std::sqrt((x0 - a_y0) * (x0 - a_y0) + x1 * x1);
  }

  return std::abs(a_y0 - a_e0);
}

/**
 * @brief The distance from a point to an ellipsoid with semi-axes a_e0 >= a_e1 >= a_e2 > 0, for a point with
 * coordinates a_y0, a_y1, a_y2 >= 0 (Eberly's algorithm).
 * @details Internal helper; not part of the public API. A point on a coordinate plane is handled by the
 * planar cases of the algorithm: its closest point is on the ellipse in that plane, or, for a point inside
 * near the plane of the two larger axes, off it.
 * @tparam T Floating-point precision.
 * @param[in] a_e0 Largest semi-axis.
 * @param[in] a_e1 Middle semi-axis.
 * @param[in] a_e2 Smallest semi-axis.
 * @param[in] a_y0 Coordinate along a_e0.
 * @param[in] a_y1 Coordinate along a_e1.
 * @param[in] a_y2 Coordinate along a_e2.
 * @return The unsigned distance to the ellipsoid surface.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline T
ellipsoidDistance(const T a_e0, const T a_e1, const T a_e2, const T a_y0, const T a_y1, const T a_y2) noexcept
{
  if (a_y2 > T(0)) {
    if (a_y1 > T(0)) {
      if (a_y0 > T(0)) {
        const T z0 = a_y0 / a_e0;
        const T z1 = a_y1 / a_e1;
        const T z2 = a_y2 / a_e2;
        const T g  = z0 * z0 + z1 * z1 + z2 * z2 - T(1);

        if (g == T(0)) {
          return T(0);
        }

        const T r0 = (a_e0 / a_e2) * (a_e0 / a_e2);
        const T r1 = (a_e1 / a_e2) * (a_e1 / a_e2);
        const T r[3]{r0, r1, T(1)};
        const T n[3]{r0 * z0, r1 * z1, z2};
        const T hi = (g < T(0)) ? T(0) : std::sqrt(n[0] * n[0] + n[1] * n[1] + z2 * z2) - T(1);
        const T s  = ellipsoidRoot<T, 3>(r, n, z2 - T(1), hi);

        const T x0 = r0 * a_y0 / (s + r0);
        const T x1 = r1 * a_y1 / (s + r1);
        const T x2 = a_y2 / (s + T(1));

        return std::sqrt((x0 - a_y0) * (x0 - a_y0) + (x1 - a_y1) * (x1 - a_y1) + (x2 - a_y2) * (x2 - a_y2));
      }

      return ellipseDistance(a_e1, a_e2, a_y1, a_y2);
    }

    if (a_y0 > T(0)) {
      return ellipseDistance(a_e0, a_e2, a_y0, a_y2);
    }

    return std::abs(a_y2 - a_e2);
  }

  const T denom0 = a_e0 * a_e0 - a_e2 * a_e2;
  const T denom1 = a_e1 * a_e1 - a_e2 * a_e2;
  const T numer0 = a_e0 * a_y0;
  const T numer1 = a_e1 * a_y1;

  if (numer0 < denom0 && numer1 < denom1) {
    const T xde0  = numer0 / denom0;
    const T xde1  = numer1 / denom1;
    const T discr = T(1) - xde0 * xde0 - xde1 * xde1;

    if (discr > T(0)) {
      const T x0 = a_e0 * xde0;
      const T x1 = a_e1 * xde1;
      const T x2 = a_e2 * std::sqrt(discr);

      return std::sqrt((x0 - a_y0) * (x0 - a_y0) + (x1 - a_y1) * (x1 - a_y1) + x2 * x2);
    }
  }

  return ellipseDistance(a_e0, a_e1, a_y0, a_y1);
}

} // namespace AnalyticDetail

/**
 * @brief Signed distance field for an ellipsoid with its semi-axes along x, y and z.
 * @details The value is the exact signed distance. There is no closed form for it: the closest point on the
 * surface is the root of a scalar equation (Eberly, "Distance from a point to an ellipse, an ellipsoid, or a
 * hyperellipsoid", 2013), found by safeguarded Newton steps from a bracket, a few iterations for most points and
 * at most 64. An ellipsoid therefore costs several times as much to evaluate as the other shapes here. The
 * common formula |p / r| (|p / r| - 1) / |p / r^2| is cheaper, but it can overestimate the distance, which a
 * DistanceQuality::Bound may not. Turn the ellipsoid with Rotate. By default the ellipsoid is the unit sphere
 * at the origin.
 * @tparam T Floating-point precision.
 */
template <class T>
class EllipsoidSDF
{
  static_assert(std::is_floating_point_v<T>, "EllipsoidSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: the Euclidean distance to the surface.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  /**
   * @brief Default constructor. Constructs the unit sphere at the origin.
   */
  EllipsoidSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_center Center.
   * @param[in] a_radii  The semi-axes along x, y and z, each finite and positive.
   */
  EllipsoidSDF(const Vec3T<T>& a_center, const Vec3T<T>& a_radii) noexcept
  {
    EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_center),
                       "EllipsoidSDF: the center must be finite (%g, %g, %g)",
                       double(a_center[0]),
                       double(a_center[1]),
                       double(a_center[2]));
    EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_radii) && a_radii[0] > T(0) && a_radii[1] > T(0) && a_radii[2] > T(0),
                       "EllipsoidSDF: the radii must be finite and positive (%g, %g, %g)",
                       double(a_radii[0]),
                       double(a_radii[1]),
                       double(a_radii[2]));

    m_center = a_center;
    m_radii  = a_radii;
  }

  /**
   * @brief Copy constructor.
   */
  EllipsoidSDF(const EllipsoidSDF&) = default;

  /**
   * @brief Move constructor.
   */
  EllipsoidSDF(EllipsoidSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  EllipsoidSDF&
  operator=(const EllipsoidSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  EllipsoidSDF&
  operator=(EllipsoidSDF&&) = default;

  /**
   * @brief Get the center.
   * @return m_center.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getCenter() const noexcept
  {
    return m_center;
  }

  /**
   * @brief Get the semi-axes.
   * @return m_radii.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getRadii() const noexcept
  {
    return m_radii;
  }

  /**
   * @brief Signed distance function for the ellipsoid.
   * @details The point is folded into the first octant and the axes sorted from largest to smallest, as
   * Eberly's algorithm takes them.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the ellipsoid surface; negative inside.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    T e[3]{m_radii[0], m_radii[1], m_radii[2]};
    T y[3]{std::abs(a_point[0] - m_center[0]), std::abs(a_point[1] - m_center[1]), std::abs(a_point[2] - m_center[2])};

    const T inside = (y[0] / e[0]) * (y[0] / e[0]) + (y[1] / e[1]) * (y[1] / e[1]) + (y[2] / e[2]) * (y[2] / e[2]);

    // Sort the axes from largest to smallest, carrying the coordinates along.
    const auto order = [&e, &y](const size_t a_i, const size_t a_j) noexcept {
      if (e[a_i] < e[a_j]) {
        const T axis  = e[a_i];
        const T coord = y[a_i];

        e[a_i] = e[a_j];
        y[a_i] = y[a_j];
        e[a_j] = axis;
        y[a_j] = coord;
      }
    };

    order(0, 1);
    order(1, 2);
    order(0, 1);

    const T distance = AnalyticDetail::ellipsoidDistance(e[0], e[1], e[2], y[0], y[1], y[2]);

    return (inside < T(1)) ? -distance : distance;
  }

  /**
   * @brief The shape's axis-aligned bounding box.
   * @return The bounding box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    return BoundingVolumes::AABBT<T>(m_center - m_radii, m_center + m_radii);
  }

protected:
  /**
   * @brief Center.
   */
  Vec3T<T> m_center = Vec3T<T>::zeros();

  /**
   * @brief Semi-axes along x, y and z.
   */
  Vec3T<T> m_radii = Vec3T<T>::ones();
};

/**
 * @brief Signed distance field for a capped cone (a frustum): a cone cut flat at both ends, between two end
 * centers, with a radius at each.
 * @details One radius may be zero, which makes a cone with its tip at that end, pointing anywhere (ConeSDF
 * points along y). The distance is exact (Quilez's capped cone): in the plane through the axis, the nearer of
 * the distances to the end discs and to the slanted side. By default the frustum runs from (0,-0.5,0) with
 * radius 1 to (0,0.5,0) with radius 0.5.
 * @tparam T Floating-point precision.
 */
template <class T>
class CappedConeSDF
{
  static_assert(std::is_floating_point_v<T>, "CappedConeSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: the Euclidean distance to the surface.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  /**
   * @brief Default constructor. Constructs a frustum from (0,-0.5,0) with radius 1 to (0,0.5,0) with radius 0.5.
   */
  CappedConeSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_center1 Center of one end.
   * @param[in] a_center2 Center of the other end; it must differ from a_center1.
   * @param[in] a_radius1 Radius at a_center1, at least 0.
   * @param[in] a_radius2 Radius at a_center2, at least 0; not both radii may be 0.
   */
  CappedConeSDF(const Vec3T<T>& a_center1, const Vec3T<T>& a_center2, const T a_radius1, const T a_radius2) noexcept
  {
    EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_center1),
                       "CappedConeSDF: the first center must be finite (%g, %g, %g)",
                       double(a_center1[0]),
                       double(a_center1[1]),
                       double(a_center1[2]));
    EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_center2),
                       "CappedConeSDF: the second center must be finite (%g, %g, %g)",
                       double(a_center2[0]),
                       double(a_center2[1]),
                       double(a_center2[2]));
    EBGEOMETRY_REQUIRE((a_center2 - a_center1).length() > T(0),
                       "CappedConeSDF: the two centers must differ, but both are (%g, %g, %g)",
                       double(a_center1[0]),
                       double(a_center1[1]),
                       double(a_center1[2]));
    EBGEOMETRY_REQUIRE(std::isfinite(a_radius1) && std::isfinite(a_radius2) && a_radius1 >= T(0) && a_radius2 >= T(0) &&
                         a_radius1 + a_radius2 > T(0),
                       "CappedConeSDF: the radii must be finite, at least 0 and not both 0 (%g, %g)",
                       double(a_radius1),
                       double(a_radius2));

    m_center1 = a_center1;
    m_center2 = a_center2;
    m_radius1 = a_radius1;
    m_radius2 = a_radius2;
    m_length  = (a_center2 - a_center1).length();
    m_axis    = (a_center2 - a_center1) / m_length;
  }

  /**
   * @brief Copy constructor.
   */
  CappedConeSDF(const CappedConeSDF&) = default;

  /**
   * @brief Move constructor.
   */
  CappedConeSDF(CappedConeSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  CappedConeSDF&
  operator=(const CappedConeSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  CappedConeSDF&
  operator=(CappedConeSDF&&) = default;

  /**
   * @brief Get the center of one end.
   * @return m_center1.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getCenter1() const noexcept
  {
    return m_center1;
  }

  /**
   * @brief Get the center of the other end.
   * @return m_center2.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getCenter2() const noexcept
  {
    return m_center2;
  }

  /**
   * @brief Get the radius at m_center1.
   * @return m_radius1.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getRadius1() const noexcept
  {
    return m_radius1;
  }

  /**
   * @brief Get the radius at m_center2.
   * @return m_radius2.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getRadius2() const noexcept
  {
    return m_radius2;
  }

  /**
   * @brief Signed distance function for the frustum.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the frustum surface; negative inside.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    const AnalyticDetail::AxialCoordinates<T> q = AnalyticDetail::axialCoordinates(a_point, m_center1, m_axis);

    const T x = q.m_radial;
    const T y = q.m_axial;
    const T h = m_length;

    // Distance to the nearer end disc: radially beyond its radius, and axially beyond the slab.
    const T capX = Math::max(T(0), x - ((y < T(0.5) * h) ? m_radius1 : m_radius2));
    const T capY = std::abs(y - T(0.5) * h) - T(0.5) * h;

    // Distance to the slanted side, the segment from (radius1, 0) to (radius2, h).
    const T dr    = m_radius2 - m_radius1;
    const T f     = Math::clamp((dr * (x - m_radius1) + y * h) / (dr * dr + h * h), T(0), T(1));
    const T sideX = x - m_radius1 - f * dr;
    const T sideY = y - f * h;

    const T sign = (sideX < T(0) && capY < T(0)) ? T(-1) : T(1);

    return sign * std::sqrt(Math::min(capX * capX + capY * capY, sideX * sideX + sideY * sideY));
  }

  /**
   * @brief How many parts the shape has (see hasParts): 3.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  getNumParts() const noexcept
  {
    return 3U;
  }

  /**
   * @brief The part whose term decides the value at a point (see hasParts): 0 the slanted side, 1 the end disc at the first center, 2 the one at the second.
   * @param[in] a_point Position.
   * @return The part.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  closestPart(const Vec3T<T>& a_point) const noexcept
  {
    const AnalyticDetail::AxialCoordinates<T> q = AnalyticDetail::axialCoordinates(a_point, m_center1, m_axis);

    const T x = q.m_radial;
    const T y = q.m_axial;
    const T h = m_length;

    const T capX = Math::max(T(0), x - ((y < T(0.5) * h) ? m_radius1 : m_radius2));
    const T capY = std::abs(y - T(0.5) * h) - T(0.5) * h;

    const T dr    = m_radius2 - m_radius1;
    const T f     = Math::clamp((dr * (x - m_radius1) + y * h) / (dr * dr + h * h), T(0), T(1));
    const T sideX = x - m_radius1 - f * dr;
    const T sideY = y - f * h;

    if (sideX * sideX + sideY * sideY <= capX * capX + capY * capY) {
      return 0U;
    }

    return (y < T(0.5) * h) ? 1U : 2U;
  }

  /**
   * @brief The shape's axis-aligned bounding box.
   * @details The box around the two end discs: along axis i, each disc reaches its radius times
   * sqrt(1 - axis_i^2) beyond its center.
   * @return The bounding box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    Vec3T<T> lo;
    Vec3T<T> hi;

    for (size_t dir = 0; dir < 3; dir++) {
      const T across = std::sqrt(Math::max(T(0), T(1) - m_axis[dir] * m_axis[dir]));

      lo[dir] = Math::min(m_center1[dir] - m_radius1 * across, m_center2[dir] - m_radius2 * across);
      hi[dir] = Math::max(m_center1[dir] + m_radius1 * across, m_center2[dir] + m_radius2 * across);
    }

    return BoundingVolumes::AABBT<T>(lo, hi);
  }

protected:
  /**
   * @brief Center of one end.
   */
  Vec3T<T> m_center1 = Vec3T<T>(T(0), T(-0.5), T(0));

  /**
   * @brief Center of the other end.
   */
  Vec3T<T> m_center2 = Vec3T<T>(T(0), T(0.5), T(0));

  /**
   * @brief Unit axis from m_center1 to m_center2.
   */
  Vec3T<T> m_axis = Vec3T<T>(T(0), T(1), T(0));

  /**
   * @brief Distance between the end centers.
   */
  T m_length = T(1);

  /**
   * @brief Radius at m_center1.
   */
  T m_radius1 = T(1);

  /**
   * @brief Radius at m_center2.
   */
  T m_radius2 = T(0.5);
};

/**
 * @brief Signed distance field for a round cone: two spheres, of different radii, and the cone tangent to
 * both, like a capsule with a radius at each end.
 * @details The distance is exact (Quilez's round cone). The spheres are centered at the two end centers;
 * the smaller one must not lie inside the larger, so the difference of the radii must be less than the
 * distance between the centers. By default the round cone runs from (0,-0.5,0) with radius 0.5 to (0,0.5,0)
 * with radius 0.25.
 * @tparam T Floating-point precision.
 */
template <class T>
class RoundConeSDF
{
  static_assert(std::is_floating_point_v<T>, "RoundConeSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: the Euclidean distance to the surface.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  /**
   * @brief Default constructor. Constructs a round cone from (0,-0.5,0) with radius 0.5 to (0,0.5,0) with
   * radius 0.25.
   */
  RoundConeSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_center1 Center of one sphere.
   * @param[in] a_center2 Center of the other sphere.
   * @param[in] a_radius1 Radius of the sphere at a_center1, at least 0.
   * @param[in] a_radius2 Radius of the sphere at a_center2, at least 0; not both radii may be 0, and their
   * difference must be less than the distance between the centers.
   */
  RoundConeSDF(const Vec3T<T>& a_center1, const Vec3T<T>& a_center2, const T a_radius1, const T a_radius2) noexcept
  {
    EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_center1),
                       "RoundConeSDF: the first center must be finite (%g, %g, %g)",
                       double(a_center1[0]),
                       double(a_center1[1]),
                       double(a_center1[2]));
    EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_center2),
                       "RoundConeSDF: the second center must be finite (%g, %g, %g)",
                       double(a_center2[0]),
                       double(a_center2[1]),
                       double(a_center2[2]));
    EBGEOMETRY_REQUIRE(std::isfinite(a_radius1) && std::isfinite(a_radius2) && a_radius1 >= T(0) && a_radius2 >= T(0) &&
                         a_radius1 + a_radius2 > T(0),
                       "RoundConeSDF: the radii must be finite, at least 0 and not both 0 (%g, %g)",
                       double(a_radius1),
                       double(a_radius2));
    EBGEOMETRY_REQUIRE(std::abs(a_radius1 - a_radius2) < (a_center2 - a_center1).length(),
                       "RoundConeSDF: the difference of the radii (%g, %g) must be less than the distance between "
                       "the centers (%g)",
                       double(a_radius1),
                       double(a_radius2),
                       double((a_center2 - a_center1).length()));

    m_center1 = a_center1;
    m_center2 = a_center2;
    m_radius1 = a_radius1;
    m_radius2 = a_radius2;
    m_length  = (a_center2 - a_center1).length();
    m_axis    = (a_center2 - a_center1) / m_length;
    m_sin     = (a_radius1 - a_radius2) / m_length;
    m_cos     = std::sqrt(T(1) - m_sin * m_sin);
  }

  /**
   * @brief Copy constructor.
   */
  RoundConeSDF(const RoundConeSDF&) = default;

  /**
   * @brief Move constructor.
   */
  RoundConeSDF(RoundConeSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  RoundConeSDF&
  operator=(const RoundConeSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  RoundConeSDF&
  operator=(RoundConeSDF&&) = default;

  /**
   * @brief Get the center of one sphere.
   * @return m_center1.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getCenter1() const noexcept
  {
    return m_center1;
  }

  /**
   * @brief Get the center of the other sphere.
   * @return m_center2.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getCenter2() const noexcept
  {
    return m_center2;
  }

  /**
   * @brief Get the radius of the sphere at m_center1.
   * @return m_radius1.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getRadius1() const noexcept
  {
    return m_radius1;
  }

  /**
   * @brief Get the radius of the sphere at m_center2.
   * @return m_radius2.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getRadius2() const noexcept
  {
    return m_radius2;
  }

  /**
   * @brief Signed distance function for the round cone.
   * @details In the plane through the axis, the point is nearest the first sphere, the second sphere or the
   * tangent line between them, as its position along the line's direction says.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the round cone surface; negative inside.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    const AnalyticDetail::AxialCoordinates<T> q = AnalyticDetail::axialCoordinates(a_point, m_center1, m_axis);

    const T x = q.m_radial;
    const T y = q.m_axial;

    // Position along the tangent line's direction (-sin, cos) in the (radial, axial) plane.
    const T k = -m_sin * x + m_cos * y;

    if (k < T(0)) {
      return std::sqrt(x * x + y * y) - m_radius1;
    }
    if (k > m_cos * m_length) {
      return std::sqrt(x * x + (y - m_length) * (y - m_length)) - m_radius2;
    }

    return m_cos * x + m_sin * y - m_radius1;
  }

  /**
   * @brief How many parts the shape has (see hasParts): 3.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  getNumParts() const noexcept
  {
    return 3U;
  }

  /**
   * @brief The part whose term decides the value at a point (see hasParts): 0 the slanted side, 1 the sphere at the first center, 2 the one at the second.
   * @param[in] a_point Position.
   * @return The part.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  closestPart(const Vec3T<T>& a_point) const noexcept
  {
    const AnalyticDetail::AxialCoordinates<T> q = AnalyticDetail::axialCoordinates(a_point, m_center1, m_axis);

    const T k = -m_sin * q.m_radial + m_cos * q.m_axial;

    return (k < T(0)) ? 1U : ((k > m_cos * m_length) ? 2U : 0U);
  }

  /**
   * @brief The shape's axis-aligned bounding box: the box around the two spheres.
   * @return The bounding box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    return BoundingVolumes::AABBT<T>(
      min(m_center1 - m_radius1 * Vec3T<T>::ones(), m_center2 - m_radius2 * Vec3T<T>::ones()),
      max(m_center1 + m_radius1 * Vec3T<T>::ones(), m_center2 + m_radius2 * Vec3T<T>::ones()));
  }

protected:
  /**
   * @brief Center of one sphere.
   */
  Vec3T<T> m_center1 = Vec3T<T>(T(0), T(-0.5), T(0));

  /**
   * @brief Center of the other sphere.
   */
  Vec3T<T> m_center2 = Vec3T<T>(T(0), T(0.5), T(0));

  /**
   * @brief Unit axis from m_center1 to m_center2.
   */
  Vec3T<T> m_axis = Vec3T<T>(T(0), T(1), T(0));

  /**
   * @brief Distance between the centers.
   */
  T m_length = T(1);

  /**
   * @brief Radius of the sphere at m_center1.
   */
  T m_radius1 = T(0.5);

  /**
   * @brief Radius of the sphere at m_center2.
   */
  T m_radius2 = T(0.25);

  /**
   * @brief Sine of the cone's half-angle, (m_radius1 - m_radius2) / m_length.
   */
  T m_sin = T(0.25);

  /**
   * @brief Cosine of the cone's half-angle.
   */
  T m_cos = T(0.96824583655185422);
};

/**
 * @brief Signed distance field for a regular prism: a regular polygon of n sides, extruded between two end
 * centers.
 * @details The polygon's circumradius (the distance from the axis to a vertex) is the radius given, and one
 * vertex points along the given direction, across the axis. The distance is exact: in the cross-section, the
 * point is turned into the polygon's first sector, where the distance to the regular polygon is that to one
 * edge (Quilez's regular polygon), and the extrusion between the end planes is exact too. Three sides make a
 * triangular prism, four a square one, six a hexagonal rod. Round its edges with Offset of a slightly smaller
 * prism. By default the prism is a hexagonal one from (0,-0.5,0) to (0,0.5,0), of circumradius 1, with a
 * vertex along x.
 * @tparam T Floating-point precision.
 */
template <class T>
class PolygonPrismSDF
{
  static_assert(std::is_floating_point_v<T>, "PolygonPrismSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: the Euclidean distance to the surface.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  /**
   * @brief Default constructor. Constructs a hexagonal prism from (0,-0.5,0) to (0,0.5,0), of circumradius 1,
   * with a vertex along x.
   */
  PolygonPrismSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_center1         Center of one end.
   * @param[in] a_center2         Center of the other end; it must differ from a_center1.
   * @param[in] a_radius          Circumradius: the distance from the axis to a vertex, finite and positive.
   * @param[in] a_sides           Number of sides, at least 3.
   * @param[in] a_vertexDirection Direction from the axis toward one vertex. Only its part across the axis is
   * used, and that must not be zero.
   */
  PolygonPrismSDF(const Vec3T<T>& a_center1,
                  const Vec3T<T>& a_center2,
                  const T         a_radius,
                  const unsigned  a_sides,
                  const Vec3T<T>& a_vertexDirection) noexcept
  {
    EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_center1),
                       "PolygonPrismSDF: the first center must be finite (%g, %g, %g)",
                       double(a_center1[0]),
                       double(a_center1[1]),
                       double(a_center1[2]));
    EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_center2),
                       "PolygonPrismSDF: the second center must be finite (%g, %g, %g)",
                       double(a_center2[0]),
                       double(a_center2[1]),
                       double(a_center2[2]));
    EBGEOMETRY_REQUIRE((a_center2 - a_center1).length() > T(0),
                       "PolygonPrismSDF: the two centers must differ, but both are (%g, %g, %g)",
                       double(a_center1[0]),
                       double(a_center1[1]),
                       double(a_center1[2]));
    EBGEOMETRY_REQUIRE(std::isfinite(a_radius) && a_radius > T(0),
                       "PolygonPrismSDF: the radius must be finite and positive (%g)",
                       double(a_radius));
    EBGEOMETRY_REQUIRE(a_sides >= 3U, "PolygonPrismSDF: the number of sides must be at least 3 (%u)", a_sides);

    const T        length = (a_center2 - a_center1).length();
    const Vec3T<T> axis   = (a_center2 - a_center1) / length;
    const Vec3T<T> across = a_vertexDirection - dot(a_vertexDirection, axis) * axis;

    EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_vertexDirection) && across.length() > T(0),
                       "PolygonPrismSDF: the vertex direction (%g, %g, %g) must have a part across the axis",
                       double(a_vertexDirection[0]),
                       double(a_vertexDirection[1]),
                       double(a_vertexDirection[2]));

    m_center1 = a_center1;
    m_center2 = a_center2;
    m_axis    = axis;
    m_length  = length;
    m_radius  = a_radius;
    m_sides   = a_sides;
    m_vertex  = across / across.length();
    m_side    = cross(m_axis, m_vertex);
  }

  /**
   * @brief Copy constructor.
   */
  PolygonPrismSDF(const PolygonPrismSDF&) = default;

  /**
   * @brief Move constructor.
   */
  PolygonPrismSDF(PolygonPrismSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  PolygonPrismSDF&
  operator=(const PolygonPrismSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  PolygonPrismSDF&
  operator=(PolygonPrismSDF&&) = default;

  /**
   * @brief Get the center of one end.
   * @return m_center1.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getCenter1() const noexcept
  {
    return m_center1;
  }

  /**
   * @brief Get the center of the other end.
   * @return m_center2.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getCenter2() const noexcept
  {
    return m_center2;
  }

  /**
   * @brief Get the circumradius.
   * @return m_radius.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getRadius() const noexcept
  {
    return m_radius;
  }

  /**
   * @brief Get the number of sides.
   * @return m_sides.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  unsigned
  getSides() const noexcept
  {
    return m_sides;
  }

  /**
   * @brief Get the unit direction from the axis toward the first vertex, across the axis.
   * @return m_vertex.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getVertexDirection() const noexcept
  {
    return m_vertex;
  }

  /**
   * @brief The signed distance to the polygon in the cross-section, at coordinates a_u along the vertex
   * direction and a_v across it.
   * @param[in] a_u Coordinate along the direction toward the first vertex.
   * @param[in] a_v Coordinate along the other direction across the axis.
   * @return The signed distance to the polygon.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  polygonDistance(const T a_u, const T a_v) const noexcept
  {
    const T half = pi<T> / T(m_sides);

    // The angle from the first vertex, turned into [-half, half) around the middle of the first edge.
    const T angle = std::atan2(a_v, a_u);
    const T turn  = angle - T(2) * half * std::floor(angle / (T(2) * half)) - half;
    const T r     = std::sqrt(a_u * a_u + a_v * a_v);

    // The point in the first sector: x toward the edge's middle, y along the edge.
    const T x = r * std::cos(turn) - m_radius * std::cos(half);
    T       y = r * std::abs(std::sin(turn)) - m_radius * std::sin(half);

    y += Math::clamp(-y, T(0), m_radius * std::sin(half));

    const T length = std::sqrt(x * x + y * y);

    return (x < T(0)) ? -length : length;
  }

  /**
   * @brief Signed distance function for the prism.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the prism surface; negative inside.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    const Vec3T<T> w = a_point - m_center1;
    const T        y = dot(w, m_axis);

    return AnalyticDetail::extrusion(polygonDistance(dot(w, m_vertex), dot(w, m_side)),
                                     std::abs(y - T(0.5) * m_length) - T(0.5) * m_length);
  }

  /**
   * @brief How many parts the shape has (see hasParts): the sides and the two caps, getSides() + 2.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  getNumParts() const noexcept
  {
    return m_sides + 2U;
  }

  /**
   * @brief The part whose term decides the value at a point (see hasParts): the side k from 0 to getSides() - 1, counted from the first vertex around the axis; then getSides() for the cap at the first center and getSides() + 1 for the one at the second.
   * @param[in] a_point Position.
   * @return The part.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  closestPart(const Vec3T<T>& a_point) const noexcept
  {
    const Vec3T<T> w = a_point - m_center1;
    const T        y = dot(w, m_axis);
    const T        u = dot(w, m_vertex);
    const T        v = dot(w, m_side);

    const T profile = this->polygonDistance(u, v);
    const T slab    = std::abs(y - T(0.5) * m_length) - T(0.5) * m_length;

    if (profile >= slab) {
      const T sector = T(2) * pi<T> / T(m_sides);
      T       angle  = std::atan2(v, u);

      if (angle < T(0)) {
        angle += T(2) * pi<T>;
      }

      return Math::min(uint32_t(angle / sector), m_sides - 1U);
    }

    return (y < T(0.5) * m_length) ? m_sides : m_sides + 1U;
  }

  /**
   * @brief The shape's axis-aligned bounding box.
   * @details The box around the circumscribed cylinder: along axis i, each end reaches the circumradius times
   * sqrt(1 - axis_i^2) beyond its center.
   * @return The bounding box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    Vec3T<T> reach;

    for (size_t dir = 0; dir < 3; dir++) {
      reach[dir] = m_radius * std::sqrt(Math::max(T(0), T(1) - m_axis[dir] * m_axis[dir]));
    }

    return BoundingVolumes::AABBT<T>(min(m_center1, m_center2) - reach, max(m_center1, m_center2) + reach);
  }

protected:
  /**
   * @brief Center of one end.
   */
  Vec3T<T> m_center1 = Vec3T<T>(T(0), T(-0.5), T(0));

  /**
   * @brief Center of the other end.
   */
  Vec3T<T> m_center2 = Vec3T<T>(T(0), T(0.5), T(0));

  /**
   * @brief Unit axis from m_center1 to m_center2.
   */
  Vec3T<T> m_axis = Vec3T<T>(T(0), T(1), T(0));

  /**
   * @brief Unit direction from the axis toward the first vertex.
   */
  Vec3T<T> m_vertex = Vec3T<T>(T(1), T(0), T(0));

  /**
   * @brief Unit direction across the axis and m_vertex, cross(m_axis, m_vertex).
   */
  Vec3T<T> m_side = Vec3T<T>(T(0), T(0), T(-1));

  /**
   * @brief Distance between the end centers.
   */
  T m_length = T(1);

  /**
   * @brief Circumradius.
   */
  T m_radius = T(1);

  /**
   * @brief Number of sides.
   */
  unsigned m_sides = 6U;
};

/**
 * @brief Signed distance field for a tube (a pipe): the space between two coaxial cylinders, cut flat at two
 * end centers.
 * @details The distance is exact: an annulus in the cross-section, extruded between the end planes. An inner
 * radius of 0 would be a cylinder (CylinderSDF). Round its edges with Offset of a slightly smaller tube. By
 * default the tube runs from (0,-0.5,0) to (0,0.5,0), with inner radius 0.5 and outer radius 1.
 * @tparam T Floating-point precision.
 */
template <class T>
class TubeSDF
{
  static_assert(std::is_floating_point_v<T>, "TubeSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: the Euclidean distance to the surface.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  /**
   * @brief Default constructor. Constructs a tube from (0,-0.5,0) to (0,0.5,0), with inner radius 0.5 and outer
   * radius 1.
   */
  TubeSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_center1     Center of one end.
   * @param[in] a_center2     Center of the other end; it must differ from a_center1.
   * @param[in] a_innerRadius Inner radius, positive.
   * @param[in] a_outerRadius Outer radius, finite and larger than the inner radius.
   */
  TubeSDF(const Vec3T<T>& a_center1, const Vec3T<T>& a_center2, const T a_innerRadius, const T a_outerRadius) noexcept
  {
    EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_center1),
                       "TubeSDF: the first center must be finite (%g, %g, %g)",
                       double(a_center1[0]),
                       double(a_center1[1]),
                       double(a_center1[2]));
    EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_center2),
                       "TubeSDF: the second center must be finite (%g, %g, %g)",
                       double(a_center2[0]),
                       double(a_center2[1]),
                       double(a_center2[2]));
    EBGEOMETRY_REQUIRE((a_center2 - a_center1).length() > T(0),
                       "TubeSDF: the two centers must differ, but both are (%g, %g, %g)",
                       double(a_center1[0]),
                       double(a_center1[1]),
                       double(a_center1[2]));
    EBGEOMETRY_REQUIRE(std::isfinite(a_outerRadius) && a_innerRadius > T(0) && a_innerRadius < a_outerRadius,
                       "TubeSDF: the radii must satisfy 0 < inner (%g) < outer (%g)",
                       double(a_innerRadius),
                       double(a_outerRadius));

    m_center1     = a_center1;
    m_center2     = a_center2;
    m_innerRadius = a_innerRadius;
    m_outerRadius = a_outerRadius;
    m_length      = (a_center2 - a_center1).length();
    m_axis        = (a_center2 - a_center1) / m_length;
  }

  /**
   * @brief Copy constructor.
   */
  TubeSDF(const TubeSDF&) = default;

  /**
   * @brief Move constructor.
   */
  TubeSDF(TubeSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  TubeSDF&
  operator=(const TubeSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  TubeSDF&
  operator=(TubeSDF&&) = default;

  /**
   * @brief Get the center of one end.
   * @return m_center1.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getCenter1() const noexcept
  {
    return m_center1;
  }

  /**
   * @brief Get the center of the other end.
   * @return m_center2.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getCenter2() const noexcept
  {
    return m_center2;
  }

  /**
   * @brief Get the inner radius.
   * @return m_innerRadius.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getInnerRadius() const noexcept
  {
    return m_innerRadius;
  }

  /**
   * @brief Get the outer radius.
   * @return m_outerRadius.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getOuterRadius() const noexcept
  {
    return m_outerRadius;
  }

  /**
   * @brief Signed distance function for the tube.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the tube surface; negative inside the wall.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    const AnalyticDetail::AxialCoordinates<T> q = AnalyticDetail::axialCoordinates(a_point, m_center1, m_axis);

    const T annulus =
      std::abs(q.m_radial - T(0.5) * (m_innerRadius + m_outerRadius)) - T(0.5) * (m_outerRadius - m_innerRadius);

    return AnalyticDetail::extrusion(annulus, std::abs(q.m_axial - T(0.5) * m_length) - T(0.5) * m_length);
  }

  /**
   * @brief How many parts the shape has (see hasParts): 4.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  getNumParts() const noexcept
  {
    return 4U;
  }

  /**
   * @brief The part whose term decides the value at a point (see hasParts): 0 the outer wall, 1 the inner wall, 2 the end at the first center, 3 the end at the second.
   * @param[in] a_point Position.
   * @return The part.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  closestPart(const Vec3T<T>& a_point) const noexcept
  {
    const AnalyticDetail::AxialCoordinates<T> q = AnalyticDetail::axialCoordinates(a_point, m_center1, m_axis);

    const T middle  = T(0.5) * (m_innerRadius + m_outerRadius);
    const T annulus = std::abs(q.m_radial - middle) - T(0.5) * (m_outerRadius - m_innerRadius);
    const T slab    = std::abs(q.m_axial - T(0.5) * m_length) - T(0.5) * m_length;

    if (annulus >= slab) {
      return (q.m_radial >= middle) ? 0U : 1U;
    }

    return (q.m_axial < T(0.5) * m_length) ? 2U : 3U;
  }

  /**
   * @brief The shape's axis-aligned bounding box: the box of the outer cylinder.
   * @return The bounding box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    Vec3T<T> reach;

    for (size_t dir = 0; dir < 3; dir++) {
      reach[dir] = m_outerRadius * std::sqrt(Math::max(T(0), T(1) - m_axis[dir] * m_axis[dir]));
    }

    return BoundingVolumes::AABBT<T>(min(m_center1, m_center2) - reach, max(m_center1, m_center2) + reach);
  }

protected:
  /**
   * @brief Center of one end.
   */
  Vec3T<T> m_center1 = Vec3T<T>(T(0), T(-0.5), T(0));

  /**
   * @brief Center of the other end.
   */
  Vec3T<T> m_center2 = Vec3T<T>(T(0), T(0.5), T(0));

  /**
   * @brief Unit axis from m_center1 to m_center2.
   */
  Vec3T<T> m_axis = Vec3T<T>(T(0), T(1), T(0));

  /**
   * @brief Distance between the end centers.
   */
  T m_length = T(1);

  /**
   * @brief Inner radius.
   */
  T m_innerRadius = T(0.5);

  /**
   * @brief Outer radius.
   */
  T m_outerRadius = T(1);
};

/**
 * @brief Signed distance field for a capped torus: an arc of a torus with flat ends, such as a pipe elbow.
 * @details The torus' ring lies in the xz-plane around an axis along y through the center, like TorusSDF's.
 * The arc sweeps the given angle, symmetric about +x, and each end is the disc where the tube is cut by the
 * half-plane through the axis at that angle. The distance is exact. Inside the arc's angle, the nearest point
 * of the solid is that of the whole torus, unless the point is inside and an end disc is nearer. Outside the
 * angle, it is on an end disc: any point of the solid turned around the axis toward the point stays in the
 * solid, and is nearer, until it reaches an end. An angle of 360 degrees is the whole torus. By default the arc
 * sweeps 90 degrees, with major radius 1 and minor radius 0.25.
 * @tparam T Floating-point precision.
 */
template <class T>
class CappedTorusSDF
{
  static_assert(std::is_floating_point_v<T>, "CappedTorusSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: the Euclidean distance to the surface.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  /**
   * @brief Default constructor. Constructs an arc of 90 degrees, centered at the origin, with major radius 1 and
   * minor radius 0.25.
   */
  CappedTorusSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_center      Center of the torus.
   * @param[in] a_majorRadius Major (ring) radius.
   * @param[in] a_minorRadius Minor (tube) radius, positive and less than the major radius.
   * @param[in] a_angle       The angle the arc sweeps, in degrees, in (0, 360].
   */
  CappedTorusSDF(const Vec3T<T>& a_center, const T a_majorRadius, const T a_minorRadius, const T a_angle) noexcept
  {
    EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_center),
                       "CappedTorusSDF: the center must be finite (%g, %g, %g)",
                       double(a_center[0]),
                       double(a_center[1]),
                       double(a_center[2]));
    EBGEOMETRY_REQUIRE(std::isfinite(a_majorRadius) && a_minorRadius > T(0) && a_minorRadius < a_majorRadius,
                       "CappedTorusSDF: the radii must satisfy 0 < minor (%g) < major (%g)",
                       double(a_minorRadius),
                       double(a_majorRadius));
    EBGEOMETRY_REQUIRE(a_angle > T(0) && a_angle <= T(360),
                       "CappedTorusSDF: the angle must be in (0, 360] degrees (%g)",
                       double(a_angle));

    m_center      = a_center;
    m_majorRadius = a_majorRadius;
    m_minorRadius = a_minorRadius;
    m_angle       = a_angle;
    m_whole       = !(a_angle < T(360));
    m_cos         = std::cos(T(0.5) * a_angle * pi<T> / T(180));
    m_sin         = std::sin(T(0.5) * a_angle * pi<T> / T(180));
  }

  /**
   * @brief Copy constructor.
   */
  CappedTorusSDF(const CappedTorusSDF&) = default;

  /**
   * @brief Move constructor.
   */
  CappedTorusSDF(CappedTorusSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  CappedTorusSDF&
  operator=(const CappedTorusSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  CappedTorusSDF&
  operator=(CappedTorusSDF&&) = default;

  /**
   * @brief Get the center.
   * @return m_center.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getCenter() const noexcept
  {
    return m_center;
  }

  /**
   * @brief Get the major (ring) radius.
   * @return m_majorRadius.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getMajorRadius() const noexcept
  {
    return m_majorRadius;
  }

  /**
   * @brief Get the minor (tube) radius.
   * @return m_minorRadius.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getMinorRadius() const noexcept
  {
    return m_minorRadius;
  }

  /**
   * @brief Get the angle the arc sweeps, in degrees.
   * @return m_angle.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getAngle() const noexcept
  {
    return m_angle;
  }

  /**
   * @brief Signed distance function for the capped torus.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the surface; negative inside.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    const Vec3T<T> p   = a_point - m_center;
    const T        rho = std::sqrt(p[0] * p[0] + p[2] * p[2]);

    // The distance to the whole torus.
    const T torus = std::sqrt((rho - m_majorRadius) * (rho - m_majorRadius) + p[1] * p[1]) - m_minorRadius;

    if (m_whole) {
      return torus;
    }

    // The distance to the end disc in the half-plane at angle a_sin, a_cos from +x: the part across the
    // half-plane, and in it, how far beyond the disc's rim the point lies.
    const auto disc = [&p, this](const T a_sin) noexcept {
      const T across = p[2] * m_cos - p[0] * a_sin;
      const T radial = p[0] * m_cos + p[2] * a_sin;
      const T rim =
        Math::max(T(0), std::sqrt((radial - m_majorRadius) * (radial - m_majorRadius) + p[1] * p[1]) - m_minorRadius);

      return std::sqrt(across * across + rim * rim);
    };

    const T ends = Math::min(disc(m_sin), disc(-m_sin));

    // Inside the arc's angle: cos(angle to the point) >= cos(half the arc).
    if (p[0] >= rho * m_cos) {
      return (torus >= T(0)) ? torus : -Math::min(-torus, ends);
    }

    return ends;
  }

  /**
   * @brief How many parts the shape has (see hasParts): 3.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  getNumParts() const noexcept
  {
    return 3U;
  }

  /**
   * @brief The part whose term decides the value at a point (see hasParts): 0 the tube, 1 the end disc at the positive angle, 2 the one at the negative; a whole torus is all tube.
   * @param[in] a_point Position.
   * @return The part.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  closestPart(const Vec3T<T>& a_point) const noexcept
  {
    if (m_whole) {
      return 0U;
    }

    const Vec3T<T> p     = a_point - m_center;
    const T        rho   = std::sqrt(p[0] * p[0] + p[2] * p[2]);
    const T        torus = std::sqrt((rho - m_majorRadius) * (rho - m_majorRadius) + p[1] * p[1]) - m_minorRadius;

    const auto disc = [&p, this](const T a_sin) noexcept {
      const T across = p[2] * m_cos - p[0] * a_sin;
      const T radial = p[0] * m_cos + p[2] * a_sin;
      const T rim =
        Math::max(T(0), std::sqrt((radial - m_majorRadius) * (radial - m_majorRadius) + p[1] * p[1]) - m_minorRadius);

      return std::sqrt(across * across + rim * rim);
    };

    const T        positive = disc(m_sin);
    const T        negative = disc(-m_sin);
    const uint32_t end      = (positive <= negative) ? 1U : 2U;

    if (p[0] >= rho * m_cos) {
      return (torus >= T(0) || -torus <= Math::min(positive, negative)) ? 0U : end;
    }

    return end;
  }

  /**
   * @brief The shape's axis-aligned bounding box.
   * @details Every point of the solid lies in a half-plane through the axis within the arc, at a distance from
   * the axis between the major radius less the minor one and their sum.
   * @return The bounding box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    const T outer = m_majorRadius + m_minorRadius;
    const T inner = m_majorRadius - m_minorRadius;

    // The arc reaches angle 180 (-x) only when whole; it reaches +-90 (z) when it sweeps at least 180.
    const T xLow  = m_whole ? -outer : ((m_cos >= T(0)) ? inner : outer) * m_cos;
    const T zHigh = (m_cos <= T(0)) ? outer : outer * m_sin;

    return BoundingVolumes::AABBT<T>(m_center + Vec3T<T>(xLow, -m_minorRadius, -zHigh),
                                     m_center + Vec3T<T>(outer, m_minorRadius, zHigh));
  }

protected:
  /**
   * @brief Center of the torus.
   */
  Vec3T<T> m_center = Vec3T<T>::zeros();

  /**
   * @brief Major (ring) radius.
   */
  T m_majorRadius = T(1);

  /**
   * @brief Minor (tube) radius.
   */
  T m_minorRadius = T(0.25);

  /**
   * @brief The angle the arc sweeps, in degrees.
   */
  T m_angle = T(90);

  /**
   * @brief Cosine of half the arc's angle.
   */
  T m_cos = T(0.70710678118654752);

  /**
   * @brief Sine of half the arc's angle.
   */
  T m_sin = T(0.70710678118654752);

  /**
   * @brief Whether the arc is the whole torus, with no ends.
   */
  bool m_whole = false;
};

/**
 * @brief Signed distance field for a cut sphere: the part of a sphere above a horizontal plane, a dome or a
 * sphere with a flat bottom.
 * @details The solid is the part of the sphere at y >= center.y + height: a dome (a spherical cap) for a
 * positive height, more than a hemisphere for a negative one. The distance is exact (Quilez's cut sphere): in
 * the plane through the y axis, the point is nearest the sphere, the flat face or the rim between them. Turn it
 * with Rotate. By default it is the upper half of the unit sphere at the origin.
 * @tparam T Floating-point precision.
 */
template <class T>
class CutSphereSDF
{
  static_assert(std::is_floating_point_v<T>, "CutSphereSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: the Euclidean distance to the surface.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  /**
   * @brief Default constructor. Constructs the upper half of the unit sphere at the origin.
   */
  CutSphereSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_center Center of the sphere.
   * @param[in] a_radius Radius, finite and positive.
   * @param[in] a_height Height of the cut plane above the center, between -a_radius and a_radius.
   */
  CutSphereSDF(const Vec3T<T>& a_center, const T a_radius, const T a_height) noexcept
  {
    EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_center),
                       "CutSphereSDF: the center must be finite (%g, %g, %g)",
                       double(a_center[0]),
                       double(a_center[1]),
                       double(a_center[2]));
    EBGEOMETRY_REQUIRE(std::isfinite(a_radius) && a_radius > T(0),
                       "CutSphereSDF: the radius must be finite and positive (%g)",
                       double(a_radius));
    EBGEOMETRY_REQUIRE(std::abs(a_height) < a_radius,
                       "CutSphereSDF: the height (%g) must be between minus and plus the radius (%g)",
                       double(a_height),
                       double(a_radius));

    m_center = a_center;
    m_radius = a_radius;
    m_height = a_height;
    m_rim    = std::sqrt(a_radius * a_radius - a_height * a_height);
  }

  /**
   * @brief Copy constructor.
   */
  CutSphereSDF(const CutSphereSDF&) = default;

  /**
   * @brief Move constructor.
   */
  CutSphereSDF(CutSphereSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  CutSphereSDF&
  operator=(const CutSphereSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  CutSphereSDF&
  operator=(CutSphereSDF&&) = default;

  /**
   * @brief Get the center of the sphere.
   * @return m_center.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getCenter() const noexcept
  {
    return m_center;
  }

  /**
   * @brief Get the radius.
   * @return m_radius.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getRadius() const noexcept
  {
    return m_radius;
  }

  /**
   * @brief Get the height of the cut plane above the center.
   * @return m_height.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getHeight() const noexcept
  {
    return m_height;
  }

  /**
   * @brief Signed distance function for the cut sphere.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the surface; negative inside.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    const Vec3T<T> p = a_point - m_center;
    const T        x = std::sqrt(p[0] * p[0] + p[2] * p[2]);
    const T        y = p[1];
    const T        r = m_radius;
    const T        h = m_height;
    const T        w = m_rim;

    // s < 0 where the nearest point is on the sphere; otherwise on the flat face (x < w) or the rim.
    const T s = Math::max((h - r) * x * x + w * w * (h + r - T(2) * y), h * x - w * y);

    if (s < T(0)) {
      return std::sqrt(x * x + y * y) - r;
    }
    if (x < w) {
      return h - y;
    }

    return std::sqrt((x - w) * (x - w) + (y - h) * (y - h));
  }

  /**
   * @brief How many parts the shape has (see hasParts): 2.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  getNumParts() const noexcept
  {
    return 2U;
  }

  /**
   * @brief The part whose term decides the value at a point (see hasParts): 0 the sphere, 1 the flat cut face (its rim included).
   * @param[in] a_point Position.
   * @return The part.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  closestPart(const Vec3T<T>& a_point) const noexcept
  {
    const Vec3T<T> p = a_point - m_center;
    const T        x = std::sqrt(p[0] * p[0] + p[2] * p[2]);
    const T        y = p[1];

    const T s = Math::max((m_height - m_radius) * x * x + m_rim * m_rim * (m_height + m_radius - T(2) * y),
                          m_height * x - m_rim * y);

    return (s < T(0)) ? 0U : 1U;
  }

  /**
   * @brief The shape's axis-aligned bounding box.
   * @return The bounding box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    const T across = (m_height <= T(0)) ? m_radius : m_rim;

    return BoundingVolumes::AABBT<T>(m_center + Vec3T<T>(-across, m_height, -across),
                                     m_center + Vec3T<T>(across, m_radius, across));
  }

protected:
  /**
   * @brief Center of the sphere.
   */
  Vec3T<T> m_center = Vec3T<T>::zeros();

  /**
   * @brief Radius.
   */
  T m_radius = T(1);

  /**
   * @brief Height of the cut plane above the center.
   */
  T m_height = T(0);

  /**
   * @brief Radius of the rim, sqrt(radius^2 - height^2).
   */
  T m_rim = T(1);
};

/**
 * @brief Signed distance field for a cut hollow sphere: a spherical shell of some thickness, the part above a
 * horizontal plane, such as a dome or a radome.
 * @details The shell is the set of points within half the thickness of the sphere, and the part kept is the
 * one above the cone from the center through the rim, where the sphere meets the plane y = center.y + height.
 * The shell's rims are therefore rounded, with radius half the thickness. The distance is exact (Quilez's cut
 * hollow sphere): the distance to the sphere's arc above the rim, or to the rim, less half the thickness. Turn
 * it with Rotate. By default it is the upper half of a shell of thickness 0.1 around the unit sphere at the
 * origin.
 * @tparam T Floating-point precision.
 */
template <class T>
class CutHollowSphereSDF
{
  static_assert(std::is_floating_point_v<T>, "CutHollowSphereSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: the Euclidean distance to the surface.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  /**
   * @brief Default constructor. Constructs the upper half of a shell of thickness 0.1 around the unit sphere
   * at the origin.
   */
  CutHollowSphereSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_center    Center of the sphere.
   * @param[in] a_radius    Radius of the shell's middle surface, finite and positive.
   * @param[in] a_height    Height of the cut plane above the center, between -a_radius and a_radius.
   * @param[in] a_thickness The shell's thickness, positive and less than twice the radius.
   */
  CutHollowSphereSDF(const Vec3T<T>& a_center, const T a_radius, const T a_height, const T a_thickness) noexcept
  {
    EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_center),
                       "CutHollowSphereSDF: the center must be finite (%g, %g, %g)",
                       double(a_center[0]),
                       double(a_center[1]),
                       double(a_center[2]));
    EBGEOMETRY_REQUIRE(std::isfinite(a_radius) && a_radius > T(0),
                       "CutHollowSphereSDF: the radius must be finite and positive (%g)",
                       double(a_radius));
    EBGEOMETRY_REQUIRE(std::abs(a_height) < a_radius,
                       "CutHollowSphereSDF: the height (%g) must be between minus and plus the radius (%g)",
                       double(a_height),
                       double(a_radius));
    EBGEOMETRY_REQUIRE(a_thickness > T(0) && a_thickness < T(2) * a_radius,
                       "CutHollowSphereSDF: the thickness (%g) must be positive and less than twice the radius (%g)",
                       double(a_thickness),
                       double(a_radius));

    m_center    = a_center;
    m_radius    = a_radius;
    m_height    = a_height;
    m_thickness = a_thickness;
    m_rim       = std::sqrt(a_radius * a_radius - a_height * a_height);
  }

  /**
   * @brief Copy constructor.
   */
  CutHollowSphereSDF(const CutHollowSphereSDF&) = default;

  /**
   * @brief Move constructor.
   */
  CutHollowSphereSDF(CutHollowSphereSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  CutHollowSphereSDF&
  operator=(const CutHollowSphereSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  CutHollowSphereSDF&
  operator=(CutHollowSphereSDF&&) = default;

  /**
   * @brief Get the center of the sphere.
   * @return m_center.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getCenter() const noexcept
  {
    return m_center;
  }

  /**
   * @brief Get the radius of the shell's middle surface.
   * @return m_radius.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getRadius() const noexcept
  {
    return m_radius;
  }

  /**
   * @brief Get the height of the cut plane above the center.
   * @return m_height.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getHeight() const noexcept
  {
    return m_height;
  }

  /**
   * @brief Get the shell's thickness.
   * @return m_thickness.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getThickness() const noexcept
  {
    return m_thickness;
  }

  /**
   * @brief Signed distance function for the cut hollow sphere.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the surface; negative inside the shell.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    const Vec3T<T> p = a_point - m_center;
    const T        x = std::sqrt(p[0] * p[0] + p[2] * p[2]);
    const T        y = p[1];

    // Below the cone through the rim, the nearest point of the arc is the rim; above it, on the sphere.
    const T arc = (m_rim * y < m_height * x) ? std::sqrt((x - m_rim) * (x - m_rim) + (y - m_height) * (y - m_height))
                                             : std::abs(std::sqrt(x * x + y * y) - m_radius);

    return arc - T(0.5) * m_thickness;
  }

  /**
   * @brief How many parts the shape has (see hasParts): 2.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  getNumParts() const noexcept
  {
    return 2U;
  }

  /**
   * @brief The part whose term decides the value at a point (see hasParts): 0 the shell, 1 its rounded rim.
   * @param[in] a_point Position.
   * @return The part.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  closestPart(const Vec3T<T>& a_point) const noexcept
  {
    const Vec3T<T> p = a_point - m_center;
    const T        x = std::sqrt(p[0] * p[0] + p[2] * p[2]);

    return (m_rim * p[1] < m_height * x) ? 1U : 0U;
  }

  /**
   * @brief The shape's axis-aligned bounding box.
   * @return The bounding box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    const T half   = T(0.5) * m_thickness;
    const T across = ((m_height <= T(0)) ? m_radius : m_rim) + half;

    return BoundingVolumes::AABBT<T>(m_center + Vec3T<T>(-across, m_height - half, -across),
                                     m_center + Vec3T<T>(across, m_radius + half, across));
  }

protected:
  /**
   * @brief Center of the sphere.
   */
  Vec3T<T> m_center = Vec3T<T>::zeros();

  /**
   * @brief Radius of the shell's middle surface.
   */
  T m_radius = T(1);

  /**
   * @brief Height of the cut plane above the center.
   */
  T m_height = T(0);

  /**
   * @brief The shell's thickness.
   */
  T m_thickness = T(0.1);

  /**
   * @brief Radius of the rim, sqrt(radius^2 - height^2).
   */
  T m_rim = T(1);
};

/**
 * @brief Signed distance field for an infinite wedge: a knife edge.
 * @details The wedge's edge runs along z through the tip, and the wedge points toward +y: it is the space below
 * two planes through the edge, each at half the opening angle from -y. It is unbounded along z, and downward and
 * sideways from the edge. The distance is exact: inside, that to the nearer plane; outside, to the nearer face,
 * or to the edge where the point is beyond both faces. Round the edge with Offset of a wedge whose tip is set
 * back. Turn it with Rotate. By default the tip is at the origin and the opening is 60 degrees.
 * @tparam T Floating-point precision.
 */
template <class T>
class WedgeSDF
{
  static_assert(std::is_floating_point_v<T>, "WedgeSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: the Euclidean distance to the surface.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  /**
   * @brief Default constructor. Constructs a wedge with its tip at the origin and an opening of 60 degrees.
   */
  WedgeSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_tip   A point on the edge.
   * @param[in] a_angle The opening angle, in degrees, in (0, 180).
   */
  WedgeSDF(const Vec3T<T>& a_tip, const T a_angle) noexcept
  {
    EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_tip),
                       "WedgeSDF: the tip must be finite (%g, %g, %g)",
                       double(a_tip[0]),
                       double(a_tip[1]),
                       double(a_tip[2]));
    EBGEOMETRY_REQUIRE(
      a_angle > T(0) && a_angle < T(180), "WedgeSDF: the angle must be in (0, 180) degrees (%g)", double(a_angle));

    m_tip   = a_tip;
    m_angle = a_angle;
    m_sin   = std::sin(T(0.5) * a_angle * pi<T> / T(180));
    m_cos   = std::cos(T(0.5) * a_angle * pi<T> / T(180));
  }

  /**
   * @brief Copy constructor.
   */
  WedgeSDF(const WedgeSDF&) = default;

  /**
   * @brief Move constructor.
   */
  WedgeSDF(WedgeSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  WedgeSDF&
  operator=(const WedgeSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  WedgeSDF&
  operator=(WedgeSDF&&) = default;

  /**
   * @brief Get the tip, a point on the edge.
   * @return m_tip.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getTip() const noexcept
  {
    return m_tip;
  }

  /**
   * @brief Get the opening angle, in degrees.
   * @return m_angle.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getAngle() const noexcept
  {
    return m_angle;
  }

  /**
   * @brief The unit outward normal of the face on the +x side; the other face's is mirrored in x.
   * @return (cos, sin) of half the opening angle, in x and y.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  Vec3T<T>
  getFaceNormal() const noexcept
  {
    return Vec3T<T>(m_cos, m_sin, T(0));
  }

  /**
   * @brief Signed distance function for the wedge.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the surface; negative inside.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    // In the cross-section, folded onto the +x face: x >= 0.
    const T x = std::abs(a_point[0] - m_tip[0]);
    const T y = a_point[1] - m_tip[1];

    // The face's outward normal is (cos, sin), its direction away from the tip (sin, -cos).
    const T plane = m_cos * x + m_sin * y;
    const T along = m_sin * x - m_cos * y;

    if (plane <= T(0) || along >= T(0)) {
      return plane;
    }

    return std::sqrt(x * x + y * y);
  }

  /**
   * @brief How many parts the shape has (see hasParts): 2.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  getNumParts() const noexcept
  {
    return 2U;
  }

  /**
   * @brief The part whose term decides the value at a point (see hasParts): 0 the face on the -x side of the tip, 1 the one on the +x side; the tip edge goes to the side the point is on.
   * @param[in] a_point Position.
   * @return The part.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  closestPart(const Vec3T<T>& a_point) const noexcept
  {
    return (a_point[0] >= m_tip[0]) ? 1U : 0U;
  }

  /**
   * @brief The shape's axis-aligned bounding box: unbounded but above the tip.
   * @return The bounding box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    const T big = Math::Limits<T>::max();

    return BoundingVolumes::AABBT<T>(Vec3T<T>(-big, -big, -big), Vec3T<T>(big, m_tip[1], big));
  }

protected:
  /**
   * @brief A point on the edge.
   */
  Vec3T<T> m_tip = Vec3T<T>::zeros();

  /**
   * @brief The opening angle, in degrees.
   */
  T m_angle = T(60);

  /**
   * @brief Sine of half the opening angle.
   */
  T m_sin = T(0.5);

  /**
   * @brief Cosine of half the opening angle.
   */
  T m_cos = T(0.86602540378443865);
};

/**
 * @brief Every analytic shape must be trivially copyable: that is what lets it be byte-copied into a
 * device address space, or into a pool-backed array, with no pointer patching.
 */
static_assert(std::is_trivially_copyable_v<PlaneSDF<float>>, "PlaneSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<PlaneSDF<double>>, "PlaneSDF<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<SphereSDF<float>>, "SphereSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<SphereSDF<double>>, "SphereSDF<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<BoxSDF<float>>, "BoxSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<BoxSDF<double>>, "BoxSDF<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<TorusSDF<float>>, "TorusSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<TorusSDF<double>>, "TorusSDF<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<CylinderSDF<float>>, "CylinderSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<CylinderSDF<double>>, "CylinderSDF<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<InfiniteCylinderSDF<float>>,
              "InfiniteCylinderSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<InfiniteCylinderSDF<double>>,
              "InfiniteCylinderSDF<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<CapsuleSDF<float>>, "CapsuleSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<CapsuleSDF<double>>, "CapsuleSDF<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<InfiniteConeSDF<float>>,
              "InfiniteConeSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<InfiniteConeSDF<double>>,
              "InfiniteConeSDF<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<ConeSDF<float>>, "ConeSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<ConeSDF<double>>, "ConeSDF<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<RoundedBoxSDF<float>>, "RoundedBoxSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<RoundedBoxSDF<double>>, "RoundedBoxSDF<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<PerlinSDF<float>>, "PerlinSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<PerlinSDF<double>>, "PerlinSDF<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<RoundedCylinderSDF<float>>,
              "RoundedCylinderSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<RoundedCylinderSDF<double>>,
              "RoundedCylinderSDF<double> must be trivially copyable");

static_assert(std::is_trivially_copyable_v<EllipsoidSDF<float>>, "EllipsoidSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<EllipsoidSDF<double>>, "EllipsoidSDF<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<CappedConeSDF<float>>, "CappedConeSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<CappedConeSDF<double>>, "CappedConeSDF<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<RoundConeSDF<float>>, "RoundConeSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<RoundConeSDF<double>>, "RoundConeSDF<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<PolygonPrismSDF<float>>,
              "PolygonPrismSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<PolygonPrismSDF<double>>,
              "PolygonPrismSDF<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<TubeSDF<float>>, "TubeSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<TubeSDF<double>>, "TubeSDF<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<CappedTorusSDF<float>>, "CappedTorusSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<CappedTorusSDF<double>>,
              "CappedTorusSDF<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<CutSphereSDF<float>>, "CutSphereSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<CutSphereSDF<double>>, "CutSphereSDF<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<CutHollowSphereSDF<float>>,
              "CutHollowSphereSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<CutHollowSphereSDF<double>>,
              "CutHollowSphereSDF<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<WedgeSDF<float>>, "WedgeSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<WedgeSDF<double>>, "WedgeSDF<double> must be trivially copyable");

} // namespace EBGeometry

// Every translation unit that sees these shapes sees their ray forms too, so firstHit() on one of them
// is the same function everywhere.
#include "EBGeometry_AnalyticRayForms.hpp"

#endif
