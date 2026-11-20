// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

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
 *   RoundedCylinder, CappedTorus, CutSphere, CutHollowSphere, Wedge, Quadric and SolidAngle, and the
 *   defaults of the others); Rotate turns it. Cylinder, Capsule, CappedCone, RoundCone, PolygonPrism,
 *   Tube, Vesica, Helix and ConvexPolyhedron's pyramid take two end points, and InfiniteCylinder an
 *   axis index, so they can point anywhere. The regular solids are in their standard orientations.
 * - Outer sizes: a size is that of the finished shape, rounding included.
 * - computeBoundingVolume() returns the shape's axis-aligned bounding box. Along a direction in
 *   which the shape is unbounded, the box extends to plus or minus Math::Limits<T>::max().
 * - distanceQuality says how far signedDistance() can be trusted as a distance (DistanceQuality).
 *   Every shape is Exact except SuperellipsoidSDF and LatticeSDF, which are Bound, and PerlinSDF,
 *   which is NotADistance.
 * - Families: shapes that share their code are one type (ConvexPolyhedronSDF with the regular solids
 *   and pyramids, QuadricSDF, LatticeSDF), since each type a tape can hold enlarges every tape's
 *   evaluator.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_ANALYTICDISTANCEFUNCTIONS_HPP
#define EBGEOMETRY_ANALYTICDISTANCEFUNCTIONS_HPP

// Std includes
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

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
  PlaneSDF(const Vec3T<T>& a_point, const Vec3T<T>& a_normal)
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
  SphereSDF(const Vec3T<T>& a_center, const T& a_radius)
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
  BoxSDF(const Vec3T<T>& a_loCorner, const Vec3T<T>& a_hiCorner)
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
  TorusSDF(const Vec3T<T>& a_center, const T& a_majorRadius, const T& a_minorRadius)
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
  CylinderSDF(const Vec3T<T>& a_center1, const Vec3T<T>& a_center2, const T& a_radius)
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
  InfiniteCylinderSDF(const Vec3T<T>& a_center, const T& a_radius, const size_t a_axis)
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
  CapsuleSDF(const Vec3T<T>& a_tip1, const Vec3T<T>& a_tip2, const T& a_radius)
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
  InfiniteConeSDF(const Vec3T<T>& a_tip, const T& a_angle)
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
  ConeSDF(const Vec3T<T>& a_tip, const T& a_height, const T& a_angle)
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
  RoundedBoxSDF(const Vec3T<T>& a_center, const Vec3T<T>& a_size, const T a_curvature)
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
            const unsigned int a_noiseOctaves)
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
  RoundedCylinderSDF(const Vec3T<T>& a_center, const T a_radius, const T a_curvature, const T a_height)
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
  EllipsoidSDF(const Vec3T<T>& a_center, const Vec3T<T>& a_radii)
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
  CappedConeSDF(const Vec3T<T>& a_center1, const Vec3T<T>& a_center2, const T a_radius1, const T a_radius2)
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
  RoundConeSDF(const Vec3T<T>& a_center1, const Vec3T<T>& a_center2, const T a_radius1, const T a_radius2)
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
                  const Vec3T<T>& a_vertexDirection)
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
  TubeSDF(const Vec3T<T>& a_center1, const Vec3T<T>& a_center2, const T a_innerRadius, const T a_outerRadius)
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
  CappedTorusSDF(const Vec3T<T>& a_center, const T a_majorRadius, const T a_minorRadius, const T a_angle)
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
  CutSphereSDF(const Vec3T<T>& a_center, const T a_radius, const T a_height)
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
  CutHollowSphereSDF(const Vec3T<T>& a_center, const T a_radius, const T a_height, const T a_thickness)
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
  WedgeSDF(const Vec3T<T>& a_tip, const T a_angle)
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
 * @brief Signed distance field for a convex polyhedron: the intersection of the half-spaces behind a set of planes,
 * such as a regular (Platonic) solid or a pyramid.
 * @details The distance is exact. Inside, it is that to the nearest face plane. Outside, it is that to the face
 * whose plane is farthest from the point when the point's projection onto that plane lies on the face, and
 * otherwise that to the nearest edge or vertex: the largest plane distance alone, which is all most formulas take,
 * is a bound at the edges and corners. The faces, edges and vertices are held in the shape itself, in arrays of
 * fixed size, so the polyhedron is trivially copyable and needs no pool; it has at most MaxFaces faces and
 * MaxVertices vertices. A polyhedron larger than that is a mesh (MeshSDF, TriMeshSDF), which has no limit.
 *
 * The regular solids and the pyramid are made by the static functions tetrahedron(), octahedron(), dodecahedron(),
 * icosahedron() and pyramid(), which return a ConvexPolyhedronSDF; any other convex polyhedron is made from its
 * planes, with the constructor. A box is BoxSDF and a regular prism PolygonPrismSDF, both cheaper to evaluate than
 * the same solid as a polyhedron. Round its edges with Offset of a slightly smaller polyhedron. By default the
 * polyhedron is the cube [-0.5, 0.5]^3.
 * @tparam T Floating-point precision.
 */
template <class T>
class ConvexPolyhedronSDF
{
  static_assert(std::is_floating_point_v<T>, "ConvexPolyhedronSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: the Euclidean distance to the surface.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  /**
   * @brief The most faces a polyhedron can have: those of the icosahedron.
   */
  static constexpr uint32_t MaxFaces = 20U;

  /**
   * @brief The most vertices a polyhedron can have: those of the dodecahedron.
   */
  static constexpr uint32_t MaxVertices = 20U;

  /**
   * @brief The most edges a polyhedron can have, from Euler's formula: MaxFaces + MaxVertices - 2.
   */
  static constexpr uint32_t MaxEdges = MaxFaces + MaxVertices - 2U;

  /**
   * @brief Default constructor. Constructs the cube [-0.5, 0.5]^3.
   */
  ConvexPolyhedronSDF();

  /**
   * @brief Constructs the convex polyhedron bounded by a set of planes: the points behind every plane.
   * @details Runs on the host. The vertices and edges are found from the planes. Face k is the face on plane k,
   * which is also the shape's part k.
   * @param[in] a_planes The planes, each with its normal pointing out of the solid (PlaneSDF's sign: the solid is
   * where every plane's value is negative). There must be from 4 to MaxFaces of them, each must bound a face of the
   * solid (no plane may miss it or only touch it at an edge or a vertex), the solid they bound must be finite and
   * have a volume, and it must have at most MaxVertices vertices.
   */
  explicit ConvexPolyhedronSDF(const std::vector<PlaneSDF<T>>& a_planes);

  /**
   * @brief A regular tetrahedron.
   * @details Its vertices are at four alternate corners of a cube around the center, in the directions (1, 1, 1),
   * (1, -1, -1), (-1, 1, -1) and (-1, -1, 1), in that order; turn it with Rotate. Face k is the one opposite vertex
   * k. The inradius is a third of the circumradius.
   * @param[in] a_center Center.
   * @param[in] a_radius Circumradius: the distance from the center to a vertex, finite and positive.
   * @return The tetrahedron.
   */
  [[nodiscard]] static ConvexPolyhedronSDF
  tetrahedron(const Vec3T<T>& a_center, T a_radius);

  /**
   * @brief A regular octahedron.
   * @details Its six vertices are on the coordinate axes through the center, so that the solid is |x| + |y| + |z|
   * <= radius around it; turn it with Rotate. Face 4 i + 2 j + k is the one whose outward normal is ((-1)^i, (-1)^j,
   * (-1)^k) / sqrt(3). The inradius is the circumradius over sqrt(3).
   * @param[in] a_center Center.
   * @param[in] a_radius Circumradius: the distance from the center to a vertex, finite and positive.
   * @return The octahedron.
   */
  [[nodiscard]] static ConvexPolyhedronSDF
  octahedron(const Vec3T<T>& a_center, T a_radius);

  /**
   * @brief A regular dodecahedron.
   * @details In its standard orientation, in which the three coordinate planes through the center are mirror
   * planes: its vertices are in the directions (+-1, +-1, +-1), (0, +-1/phi, +-phi), (+-1/phi, +-phi, 0) and (+-phi,
   * 0, +-1/phi), with phi the golden ratio; turn it with Rotate. Its twelve faces have their outward normals along
   * (0, +-phi, +-1), (+-phi, +-1, 0) and (+-1, 0, +-phi), numbered in that order, the signs within each group in the
   * order (+, +), (+, -), (-, +), (-, -).
   * @param[in] a_center Center.
   * @param[in] a_radius Circumradius: the distance from the center to a vertex, finite and positive.
   * @return The dodecahedron.
   */
  [[nodiscard]] static ConvexPolyhedronSDF
  dodecahedron(const Vec3T<T>& a_center, T a_radius);

  /**
   * @brief A regular icosahedron.
   * @details In its standard orientation, in which the three coordinate planes through the center are mirror
   * planes: its vertices are in the directions (0, +-1, +-phi), (+-1, +-phi, 0) and (+-phi, 0, +-1), with phi the
   * golden ratio; turn it with Rotate. Its twenty faces have their outward normals along (+-1, +-1, +-1), (0, +-phi,
   * +-1/phi), (+-phi, +-1/phi, 0) and (+-1/phi, 0, +-phi), numbered in that order: the first eight with the signs of
   * x, y and z counted as the bits of 4 i + 2 j + k (a minus sign for a set bit), the others with their two signs in
   * the order (+, +), (+, -), (-, +), (-, -).
   * @param[in] a_center Center.
   * @param[in] a_radius Circumradius: the distance from the center to a vertex, finite and positive.
   * @return The icosahedron.
   */
  [[nodiscard]] static ConvexPolyhedronSDF
  icosahedron(const Vec3T<T>& a_center, T a_radius);

  /**
   * @brief A right pyramid on a regular polygon: a square pyramid with four sides, a tetrahedron-like one with
   * three.
   * @details As for PolygonPrismSDF, the base is a regular polygon of n sides around the axis from the base center
   * to the apex, with one vertex along the given direction. Faces 0 to n - 1 are the sides, counted around the axis
   * from the one that starts at that vertex, as PolygonPrismSDF counts its sides; face n is the base.
   * @param[in] a_baseCenter      Center of the base.
   * @param[in] a_apex            The apex; it must differ from a_baseCenter.
   * @param[in] a_radius          Circumradius of the base: the distance from the axis to a base vertex, finite and
   * positive.
   * @param[in] a_sides           Number of sides, from 3 to MaxVertices - 1.
   * @param[in] a_vertexDirection Direction from the axis toward one base vertex. Only its part across the axis is
   * used, and that must not be zero.
   * @return The pyramid.
   */
  [[nodiscard]] static ConvexPolyhedronSDF
  pyramid(const Vec3T<T>& a_baseCenter,
          const Vec3T<T>& a_apex,
          T               a_radius,
          unsigned        a_sides,
          const Vec3T<T>& a_vertexDirection);

  /**
   * @brief Copy constructor.
   */
  ConvexPolyhedronSDF(const ConvexPolyhedronSDF&) = default;

  /**
   * @brief Move constructor.
   */
  ConvexPolyhedronSDF(ConvexPolyhedronSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  ConvexPolyhedronSDF&
  operator=(const ConvexPolyhedronSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  ConvexPolyhedronSDF&
  operator=(ConvexPolyhedronSDF&&) = default;

  /**
   * @brief Get the number of faces.
   * @return m_numFaces.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  getNumFaces() const noexcept
  {
    return m_numFaces;
  }

  /**
   * @brief Get the unit outward normal of a face.
   * @param[in] a_face The face, below getNumFaces().
   * @return The normal.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getNormal(const uint32_t a_face) const noexcept
  {
    EBGEOMETRY_EXPECT(a_face < m_numFaces);

    return m_normals[a_face];
  }

  /**
   * @brief Get the offset of a face's plane: the plane is the points x with dot(getNormal(a_face), x) equal to it.
   * @param[in] a_face The face, below getNumFaces().
   * @return The offset.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getOffset(const uint32_t a_face) const noexcept
  {
    EBGEOMETRY_EXPECT(a_face < m_numFaces);

    return m_offsets[a_face];
  }

  /**
   * @brief Get the number of vertices.
   * @return m_numVertices.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  getNumVertices() const noexcept
  {
    return m_numVertices;
  }

  /**
   * @brief Get a vertex.
   * @param[in] a_vertex The vertex, below getNumVertices().
   * @return Its position.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getVertex(const uint32_t a_vertex) const noexcept
  {
    EBGEOMETRY_EXPECT(a_vertex < m_numVertices);

    return m_vertices[a_vertex];
  }

  /**
   * @brief Get the number of edges.
   * @return m_numEdges.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  getNumEdges() const noexcept
  {
    return m_numEdges;
  }

  /**
   * @brief Get one end of an edge.
   * @param[in] a_edge The edge, below getNumEdges().
   * @param[in] a_end  0 or 1.
   * @return The index of the vertex at that end.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  getEdgeVertex(const uint32_t a_edge, const uint32_t a_end) const noexcept
  {
    EBGEOMETRY_EXPECT(a_edge < m_numEdges && a_end < 2U);

    return m_edges[2U * a_edge + a_end];
  }

  /**
   * @brief Signed distance function for the polyhedron.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the surface; negative inside.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief How many parts the shape has (see hasParts): its faces, getNumFaces().
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  getNumParts() const noexcept
  {
    return m_numFaces;
  }

  /**
   * @brief The part whose term decides the value at a point (see hasParts): the face whose plane is farthest above
   * the point (the lowest such face on a tie). Inside, that is the nearest face; outside, the face the nearest
   * point is on whenever that point is inside a face.
   * @param[in] a_point Position.
   * @return The part.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  closestPart(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief The shape's axis-aligned bounding box: that of its vertices.
   * @return The bounding box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept;

protected:
  /**
   * @brief The six planes of the cube [-0.5, 0.5]^3, the default polyhedron.
   * @return The planes.
   */
  static std::vector<PlaneSDF<T>>
  cubePlanes();

  /**
   * @brief The twelve directions (0, +-a, +-b), (+-a, +-b, 0) and (+-b, 0, +-a), in that order, the signs within each
   * group in the order (+, +), (+, -), (-, +), (-, -).
   * @param[in] a_small The first magnitude, a.
   * @param[in] a_large The second magnitude, b.
   * @return The directions.
   */
  static std::vector<Vec3T<T>>
  goldenDirections(T a_small, T a_large);

  /**
   * @brief A regular solid from the directions of its vertices and of its faces' normals: each face's plane is as
   * far out along its normal as the farthest vertex, the vertices being at the circumradius from the center.
   * @param[in] a_center           Center.
   * @param[in] a_vertexDirections Directions from the center to the vertices.
   * @param[in] a_normals          The faces' outward normals, in the faces' order.
   * @param[in] a_radius           Circumradius, finite and positive.
   * @param[in] a_who              The factory's name, for the messages.
   * @return The solid.
   */
  static ConvexPolyhedronSDF
  fromNormals(const Vec3T<T>&              a_center,
              const std::vector<Vec3T<T>>& a_vertexDirections,
              const std::vector<Vec3T<T>>& a_normals,
              T                            a_radius,
              const char*                  a_who);

  /**
   * @brief Number of faces.
   */
  uint32_t m_numFaces = 0U;

  /**
   * @brief Number of vertices.
   */
  uint32_t m_numVertices = 0U;

  /**
   * @brief Number of edges.
   */
  uint32_t m_numEdges = 0U;

  /**
   * @brief Unit outward normal of each face.
   */
  Array<Vec3T<T>, MaxFaces> m_normals;

  /**
   * @brief Offset of each face's plane along its normal.
   */
  Array<T, MaxFaces> m_offsets;

  /**
   * @brief The vertices.
   */
  Array<Vec3T<T>, MaxVertices> m_vertices;

  /**
   * @brief The edges, as the indices of their two vertices: edge e from m_edges[2 e] to m_edges[2 e + 1].
   */
  Array<uint8_t, 2U * MaxEdges> m_edges;
};

/**
 * @brief Signed distance field for the frame of a box: its twelve edges, as bars of square cross-section.
 * @details The frame fits the box between the two corners, and each bar is as thick as given, measured across it.
 * The distance is exact. Outside, it is Quilez's box frame: the point is folded into one octant of the box, and the
 * frame there is three bars meeting at a corner, the nearest of which is the distance. Inside, it is the distance
 * to the box's outer faces or to the openings between the bars, which the nearest bar's depth undercuts where bars
 * meet. Round its edges with Offset of a slightly smaller frame. By default the frame
 * is that of the cube [-0.5, 0.5]^3, with bars 0.1 thick.
 * @tparam T Floating-point precision.
 */
template <class T>
class BoxFrameSDF
{
  static_assert(std::is_floating_point_v<T>, "BoxFrameSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: the Euclidean distance to the surface.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  /**
   * @brief Default constructor. Constructs the frame of the cube [-0.5, 0.5]^3, with bars 0.1 thick.
   */
  BoxFrameSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_loCorner  Lower corner of the box the frame fits (minimum x, y, z).
   * @param[in] a_hiCorner  Upper corner. Each component must be strictly greater than that of a_loCorner.
   * @param[in] a_thickness Thickness of a bar, across it: positive, and less than half of the box's shortest side.
   */
  BoxFrameSDF(const Vec3T<T>& a_loCorner, const Vec3T<T>& a_hiCorner, T a_thickness);

  /**
   * @brief Copy constructor.
   */
  BoxFrameSDF(const BoxFrameSDF&) = default;

  /**
   * @brief Move constructor.
   */
  BoxFrameSDF(BoxFrameSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  BoxFrameSDF&
  operator=(const BoxFrameSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  BoxFrameSDF&
  operator=(BoxFrameSDF&&) = default;

  /**
   * @brief Get the lower corner of the box the frame fits.
   * @return m_loCorner.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getLowCorner() const noexcept
  {
    return m_loCorner;
  }

  /**
   * @brief Get the upper corner of the box the frame fits.
   * @return m_hiCorner.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getHighCorner() const noexcept
  {
    return m_hiCorner;
  }

  /**
   * @brief Get the thickness of a bar.
   * @return m_thickness.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getThickness() const noexcept
  {
    return m_thickness;
  }

  /**
   * @brief Signed distance function for the frame.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the surface; negative inside a bar.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief How many parts the shape has (see hasParts): its twelve bars.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  getNumParts() const noexcept
  {
    return 12U;
  }

  /**
   * @brief The part whose term decides the value at a point (see hasParts): the nearest bar, 4 a + 2 i + j for the
   * bar along axis a (x, y, z) on the low (0) or high (1) side of the first (i) and the second (j) of the other two
   * axes, in the order x, y, z. Where two bars meet, the one along the lower axis.
   * @param[in] a_point Position.
   * @return The part.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  closestPart(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief The shape's axis-aligned bounding box: the box the frame fits.
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
   * @brief The three bars that meet at the corner of the octant a point is in: the signed distance to the one along
   * x, along y and along z.
   * @param[in] a_point Position.
   * @return The three distances.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  Vec3T<T>
  barTerms(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Lower corner of the box.
   */
  Vec3T<T> m_loCorner = Vec3T<T>(T(-0.5), T(-0.5), T(-0.5));

  /**
   * @brief Upper corner of the box.
   */
  Vec3T<T> m_hiCorner = Vec3T<T>(T(0.5), T(0.5), T(0.5));

  /**
   * @brief Thickness of a bar.
   */
  T m_thickness = T(0.1);
};

namespace AnalyticDetail {

/**
 * @brief A convex polyhedron's faces, vertices and edges, found on the host from its planes in double precision.
 * @details Internal helper; not part of the public API.
 */
struct PolyhedronTopology
{
  /**
   * @brief The vertices.
   */
  std::vector<Vec3T<double>> m_vertices;

  /**
   * @brief The edges, as pairs of vertex indices.
   */
  std::vector<std::pair<uint32_t, uint32_t>> m_edges;
};

/**
 * @brief Finds the vertices and edges of the convex polyhedron behind a set of planes, and checks that every plane
 * bounds a face and that the solid is finite and has a volume.
 * @details Internal helper; not part of the public API. A vertex is where three planes meet and every other plane
 * is behind it; vertices closer than a tolerance are one (where more than three planes meet, as at an octahedron's
 * or a pyramid's tip). Two vertices are an edge when they share two planes: two vertices on the line where two
 * planes meet are the two ends of the edge there, since a vertex cannot be inside an edge. The polyhedron is finite
 * and has a volume when Euler's formula holds and every plane holds at least three vertices.
 * @param[in] a_normals The planes' unit outward normals.
 * @param[in] a_offsets The planes' offsets: plane k is dot(a_normals[k], x) = a_offsets[k].
 * @param[in] a_epsilon The machine epsilon of the precision the planes were given in, which sets the tolerance.
 * @param[in] a_who     The class name, for the messages.
 * @return The vertices and edges.
 */
inline PolyhedronTopology
polyhedronTopology(const std::vector<Vec3T<double>>& a_normals,
                   const std::vector<double>&        a_offsets,
                   const double                      a_epsilon,
                   const char*                       a_who)
{
  const size_t numPlanes = a_normals.size();

  double scale = 0.0;

  for (const double h : a_offsets) {
    scale = Math::max(scale, std::abs(h));
  }

  // The planes are rounded to the shape's precision, so where more than three meet, their crossings scatter by a
  // few of its units in the last place, amplified by how obliquely they meet.
  const double tol = 256.0 * a_epsilon * Math::max(scale, 1.E-300);

  PolyhedronTopology topology;

  for (size_t i = 0; i < numPlanes; i++) {
    for (size_t j = i + 1; j < numPlanes; j++) {
      for (size_t k = j + 1; k < numPlanes; k++) {
        const Vec3T<double>& a = a_normals[i];
        const Vec3T<double>& b = a_normals[j];
        const Vec3T<double>& c = a_normals[k];

        const Vec3T<double> bc  = cross(b, c);
        const double        det = dot(a, bc);

        if (std::abs(det) < 1.E-12) {
          continue;
        }

        // Cramer's rule for dot(a, x) = ha, dot(b, x) = hb, dot(c, x) = hc.
        const Vec3T<double> x = (a_offsets[i] * bc + a_offsets[j] * cross(c, a) + a_offsets[k] * cross(a, b)) / det;

        bool behind = true;

        for (size_t m = 0; m < numPlanes && behind; m++) {
          behind = dot(a_normals[m], x) - a_offsets[m] <= tol;
        }

        if (!behind) {
          continue;
        }

        bool known = false;

        for (const Vec3T<double>& v : topology.m_vertices) {
          known = known || (v - x).length() <= 10.0 * tol;
        }

        if (!known) {
          topology.m_vertices.push_back(x);
        }
      }
    }
  }

  const size_t numVertices = topology.m_vertices.size();

  // The planes each vertex is on.
  std::vector<std::vector<bool>> on(numVertices, std::vector<bool>(numPlanes, false));

  for (size_t v = 0; v < numVertices; v++) {
    for (size_t m = 0; m < numPlanes; m++) {
      on[v][m] = std::abs(dot(a_normals[m], topology.m_vertices[v]) - a_offsets[m]) <= 10.0 * tol;
    }
  }

  for (size_t m = 0; m < numPlanes; m++) {
    size_t count = 0;

    for (size_t v = 0; v < numVertices; v++) {
      count += on[v][m] ? 1U : 0U;
    }

    EBGEOMETRY_REQUIRE(
      count >= 3U, "%s: plane %zu does not bound a face of the solid, or the solid is not finite", a_who, m);
  }

  for (size_t v = 0; v < numVertices; v++) {
    for (size_t w = v + 1; w < numVertices; w++) {
      size_t shared = 0;

      for (size_t m = 0; m < numPlanes; m++) {
        shared += (on[v][m] && on[w][m]) ? 1U : 0U;
      }

      if (shared >= 2U) {
        topology.m_edges.emplace_back(uint32_t(v), uint32_t(w));
      }
    }
  }

  EBGEOMETRY_REQUIRE(numVertices + numPlanes == topology.m_edges.size() + 2U,
                     "%s: the planes do not bound a finite solid with a volume (%zu vertices, %zu edges, %zu faces)",
                     a_who,
                     numVertices,
                     topology.m_edges.size(),
                     numPlanes);

  return topology;
}

} // namespace AnalyticDetail

template <class T>
inline std::vector<PlaneSDF<T>>
ConvexPolyhedronSDF<T>::cubePlanes()
{
  std::vector<PlaneSDF<T>> planes;

  for (size_t dir = 0; dir < 3; dir++) {
    for (const T side : {T(-1), T(1)}) {
      Vec3T<T> normal = Vec3T<T>::zeros();

      normal[dir] = side;

      planes.emplace_back(T(0.5) * normal, normal);
    }
  }

  return planes;
}

template <class T>
inline ConvexPolyhedronSDF<T>::ConvexPolyhedronSDF() : ConvexPolyhedronSDF(cubePlanes())
{}

template <class T>
inline ConvexPolyhedronSDF<T>::ConvexPolyhedronSDF(const std::vector<PlaneSDF<T>>& a_planes)
{
  EBGEOMETRY_REQUIRE(a_planes.size() >= 4U && a_planes.size() <= MaxFaces,
                     "ConvexPolyhedronSDF: a polyhedron has from 4 to %u planes (%zu)",
                     unsigned(MaxFaces),
                     a_planes.size());

  std::vector<Vec3T<double>> normals;
  std::vector<double>        offsets;

  for (const PlaneSDF<T>& plane : a_planes) {
    const Vec3T<double> point(double(plane.getPoint()[0]), double(plane.getPoint()[1]), double(plane.getPoint()[2]));
    const Vec3T<double> normal(
      double(plane.getNormal()[0]), double(plane.getNormal()[1]), double(plane.getNormal()[2]));

    EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(point) && AnalyticDetail::isFinite(normal) && normal.length() > 0.0,
                       "ConvexPolyhedronSDF: plane %zu must have a finite point and a finite, nonzero normal",
                       normals.size());

    normals.push_back(normal / normal.length());
    offsets.push_back(dot(normals.back(), point));
  }

  const AnalyticDetail::PolyhedronTopology topology =
    AnalyticDetail::polyhedronTopology(normals, offsets, double(Math::Limits<T>::epsilon()), "ConvexPolyhedronSDF");

  EBGEOMETRY_REQUIRE(topology.m_vertices.size() <= MaxVertices,
                     "ConvexPolyhedronSDF: a polyhedron has at most %u vertices (%zu)",
                     unsigned(MaxVertices),
                     topology.m_vertices.size());

  m_numFaces    = uint32_t(normals.size());
  m_numVertices = uint32_t(topology.m_vertices.size());
  m_numEdges    = uint32_t(topology.m_edges.size());

  for (uint32_t f = 0; f < m_numFaces; f++) {
    m_normals[f] = Vec3T<T>(T(normals[f][0]), T(normals[f][1]), T(normals[f][2]));
    m_offsets[f] = T(offsets[f]);
  }

  for (uint32_t v = 0; v < m_numVertices; v++) {
    const Vec3T<double>& x = topology.m_vertices[v];

    m_vertices[v] = Vec3T<T>(T(x[0]), T(x[1]), T(x[2]));
  }

  for (uint32_t e = 0; e < m_numEdges; e++) {
    m_edges[2U * e]      = uint8_t(topology.m_edges[e].first);
    m_edges[2U * e + 1U] = uint8_t(topology.m_edges[e].second);
  }
}

template <class T>
inline ConvexPolyhedronSDF<T>
ConvexPolyhedronSDF<T>::fromNormals(const Vec3T<T>&              a_center,
                                    const std::vector<Vec3T<T>>& a_vertexDirections,
                                    const std::vector<Vec3T<T>>& a_normals,
                                    const T                      a_radius,
                                    const char*                  a_who)
{
  EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_center),
                     "%s: the center must be finite (%g, %g, %g)",
                     a_who,
                     double(a_center[0]),
                     double(a_center[1]),
                     double(a_center[2]));
  EBGEOMETRY_REQUIRE(std::isfinite(a_radius) && a_radius > T(0),
                     "%s: the radius must be finite and positive (%g)",
                     a_who,
                     double(a_radius));

  // Each face's plane is at the largest height of a vertex along its normal (the vertices lie on a sphere of the
  // circumradius), computed in double so that a float solid's faces are as close to its vertices as can be.
  std::vector<PlaneSDF<T>> planes;

  for (const Vec3T<T>& n : a_normals) {
    const Vec3T<double> normal = Vec3T<double>(double(n[0]), double(n[1]), double(n[2])) /
                                 Vec3T<double>(double(n[0]), double(n[1]), double(n[2])).length();

    double height = -1.0;

    for (const Vec3T<T>& d : a_vertexDirections) {
      const Vec3T<double> direction = Vec3T<double>(double(d[0]), double(d[1]), double(d[2])) /
                                      Vec3T<double>(double(d[0]), double(d[1]), double(d[2])).length();

      height = Math::max(height, dot(normal, direction));
    }

    const Vec3T<T> unit = Vec3T<T>(T(normal[0]), T(normal[1]), T(normal[2]));

    planes.emplace_back(a_center + T(height * double(a_radius)) * unit, unit);
  }

  return ConvexPolyhedronSDF<T>(planes);
}

template <class T>
inline ConvexPolyhedronSDF<T>
ConvexPolyhedronSDF<T>::tetrahedron(const Vec3T<T>& a_center, const T a_radius)
{
  const std::vector<Vec3T<T>> vertices{Vec3T<T>(T(1), T(1), T(1)),
                                       Vec3T<T>(T(1), T(-1), T(-1)),
                                       Vec3T<T>(T(-1), T(1), T(-1)),
                                       Vec3T<T>(T(-1), T(-1), T(1))};

  std::vector<Vec3T<T>> normals;

  for (const Vec3T<T>& v : vertices) {
    normals.push_back(-v);
  }

  return fromNormals(a_center, vertices, normals, a_radius, "ConvexPolyhedronSDF::tetrahedron");
}

template <class T>
inline ConvexPolyhedronSDF<T>
ConvexPolyhedronSDF<T>::octahedron(const Vec3T<T>& a_center, const T a_radius)
{
  std::vector<Vec3T<T>> vertices;
  std::vector<Vec3T<T>> normals;

  for (size_t dir = 0; dir < 3; dir++) {
    vertices.push_back(Vec3T<T>::unit(dir));
    vertices.push_back(-Vec3T<T>::unit(dir));
  }

  for (uint32_t f = 0; f < 8U; f++) {
    normals.emplace_back((f & 4U) ? T(-1) : T(1), (f & 2U) ? T(-1) : T(1), (f & 1U) ? T(-1) : T(1));
  }

  return fromNormals(a_center, vertices, normals, a_radius, "ConvexPolyhedronSDF::octahedron");
}

template <class T>
inline std::vector<Vec3T<T>>
ConvexPolyhedronSDF<T>::goldenDirections(const T a_small, const T a_large)
{
  std::vector<Vec3T<T>> directions;

  for (const T s : {T(1), T(-1)}) {
    for (const T t : {T(1), T(-1)}) {
      directions.emplace_back(T(0), s * a_small, t * a_large);
    }
  }

  for (const T s : {T(1), T(-1)}) {
    for (const T t : {T(1), T(-1)}) {
      directions.emplace_back(s * a_small, t * a_large, T(0));
    }
  }

  for (const T s : {T(1), T(-1)}) {
    for (const T t : {T(1), T(-1)}) {
      directions.emplace_back(s * a_large, T(0), t * a_small);
    }
  }

  return directions;
}

template <class T>
inline ConvexPolyhedronSDF<T>
ConvexPolyhedronSDF<T>::dodecahedron(const Vec3T<T>& a_center, const T a_radius)
{
  const T phi = (T(1) + std::sqrt(T(5))) / T(2);

  std::vector<Vec3T<T>> vertices;

  for (uint32_t c = 0; c < 8U; c++) {
    vertices.emplace_back((c & 4U) ? T(-1) : T(1), (c & 2U) ? T(-1) : T(1), (c & 1U) ? T(-1) : T(1));
  }

  for (const Vec3T<T>& v : goldenDirections(T(1) / phi, phi)) {
    vertices.push_back(v);
  }

  return fromNormals(a_center, vertices, goldenDirections(phi, T(1)), a_radius, "ConvexPolyhedronSDF::dodecahedron");
}

template <class T>
inline ConvexPolyhedronSDF<T>
ConvexPolyhedronSDF<T>::icosahedron(const Vec3T<T>& a_center, const T a_radius)
{
  const T phi = (T(1) + std::sqrt(T(5))) / T(2);

  std::vector<Vec3T<T>> normals;

  for (uint32_t c = 0; c < 8U; c++) {
    normals.emplace_back((c & 4U) ? T(-1) : T(1), (c & 2U) ? T(-1) : T(1), (c & 1U) ? T(-1) : T(1));
  }

  for (const Vec3T<T>& n : goldenDirections(phi, T(1) / phi)) {
    normals.push_back(n);
  }

  return fromNormals(a_center, goldenDirections(T(1), phi), normals, a_radius, "ConvexPolyhedronSDF::icosahedron");
}

template <class T>
inline ConvexPolyhedronSDF<T>
ConvexPolyhedronSDF<T>::pyramid(const Vec3T<T>& a_baseCenter,
                                const Vec3T<T>& a_apex,
                                const T         a_radius,
                                const unsigned  a_sides,
                                const Vec3T<T>& a_vertexDirection)
{
  EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_baseCenter),
                     "ConvexPolyhedronSDF::pyramid: the base center must be finite (%g, %g, %g)",
                     double(a_baseCenter[0]),
                     double(a_baseCenter[1]),
                     double(a_baseCenter[2]));
  EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_apex),
                     "ConvexPolyhedronSDF::pyramid: the apex must be finite (%g, %g, %g)",
                     double(a_apex[0]),
                     double(a_apex[1]),
                     double(a_apex[2]));
  EBGEOMETRY_REQUIRE(
    (a_apex - a_baseCenter).length() > T(0),
    "ConvexPolyhedronSDF::pyramid: the apex must differ from the base center, but both are (%g, %g, %g)",
    double(a_apex[0]),
    double(a_apex[1]),
    double(a_apex[2]));
  EBGEOMETRY_REQUIRE(std::isfinite(a_radius) && a_radius > T(0),
                     "ConvexPolyhedronSDF::pyramid: the radius must be finite and positive (%g)",
                     double(a_radius));
  EBGEOMETRY_REQUIRE(a_sides >= 3U && a_sides < MaxVertices,
                     "ConvexPolyhedronSDF::pyramid: the number of sides must be from 3 to %u (%u)",
                     unsigned(MaxVertices - 1U),
                     a_sides);

  // In double: the base vertices, then each side's plane through two of them and the apex.
  const auto toDouble = [](const Vec3T<T>& a_v) {
    return Vec3T<double>(double(a_v[0]), double(a_v[1]), double(a_v[2]));
  };

  const Vec3T<double> base   = toDouble(a_baseCenter);
  const Vec3T<double> apex   = toDouble(a_apex);
  const Vec3T<double> axis   = (apex - base) / (apex - base).length();
  const Vec3T<double> dir    = toDouble(a_vertexDirection);
  const Vec3T<double> across = dir - dot(dir, axis) * axis;

  EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_vertexDirection) && across.length() > 0.0,
                     "ConvexPolyhedronSDF::pyramid: the vertex direction (%g, %g, %g) must have a part across the axis",
                     double(a_vertexDirection[0]),
                     double(a_vertexDirection[1]),
                     double(a_vertexDirection[2]));

  const Vec3T<double> u = across / across.length();
  const Vec3T<double> v = cross(axis, u);

  std::vector<Vec3T<double>> corners;

  for (unsigned k = 0; k < a_sides; k++) {
    const double angle = 2.0 * pi<double> * double(k) / double(a_sides);

    corners.push_back(base + double(a_radius) * (std::cos(angle) * u + std::sin(angle) * v));
  }

  std::vector<PlaneSDF<T>> planes;

  const auto toT = [](const Vec3T<double>& a_v) { return Vec3T<T>(T(a_v[0]), T(a_v[1]), T(a_v[2])); };

  for (unsigned k = 0; k < a_sides; k++) {
    const Vec3T<double>& a = corners[k];
    const Vec3T<double>& b = corners[(k + 1U) % a_sides];

    Vec3T<double> normal = cross(b - a, apex - a);

    // Outward: away from the base center.
    if (dot(normal, a - base) < 0.0) {
      normal = -normal;
    }

    planes.emplace_back(toT(a), toT(normal / normal.length()));
  }

  planes.emplace_back(a_baseCenter, toT(-axis));

  return ConvexPolyhedronSDF<T>(planes);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
ConvexPolyhedronSDF<T>::closestPart(const Vec3T<T>& a_point) const noexcept
{
  uint32_t face = 0U;
  T        best = dot(m_normals[0], a_point) - m_offsets[0];

  for (uint32_t f = 1; f < m_numFaces; f++) {
    const T d = dot(m_normals[f], a_point) - m_offsets[f];

    if (d > best) {
      best = d;
      face = f;
    }
  }

  return face;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline T
ConvexPolyhedronSDF<T>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  const uint32_t face = this->closestPart(a_point);
  const T        d    = dot(m_normals[face], a_point) - m_offsets[face];

  // Inside, the nearest face plane is the distance: its foot is on the face, the solid being convex.
  if (d <= T(0)) {
    return d;
  }

  // Outside, the closest point is inside a face only if that face's plane is the farthest from the point, and then
  // it is the point's foot on that plane: the foot is the closest point when every other plane is above it.
  const Vec3T<T> foot   = a_point - d * m_normals[face];
  bool           inside = true;

  for (uint32_t f = 0; f < m_numFaces; f++) {
    if (f != face && dot(m_normals[f], foot) - m_offsets[f] > T(0)) {
      inside = false;
    }
  }

  if (inside) {
    return d;
  }

  // Otherwise the closest point is on an edge, or at a vertex, an end of one.
  T squared = Math::Limits<T>::max();

  for (uint32_t e = 0; e < m_numEdges; e++) {
    const Vec3T<T>& a  = m_vertices[m_edges[2U * e]];
    const Vec3T<T>  ab = m_vertices[m_edges[2U * e + 1U]] - a;
    const Vec3T<T>  ap = a_point - a;
    const T         s  = Math::clamp(dot(ap, ab) / dot(ab, ab), T(0), T(1));
    const Vec3T<T>  w  = ap - s * ab;

    squared = Math::min(squared, dot(w, w));
  }

  return std::sqrt(squared);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline BoundingVolumes::AABBT<T>
ConvexPolyhedronSDF<T>::computeBoundingVolume() const noexcept
{
  Vec3T<T> lo = m_vertices[0];
  Vec3T<T> hi = m_vertices[0];

  for (uint32_t v = 1; v < m_numVertices; v++) {
    lo = min(lo, m_vertices[v]);
    hi = max(hi, m_vertices[v]);
  }

  return BoundingVolumes::AABBT<T>(lo, hi);
}

template <class T>
inline BoxFrameSDF<T>::BoxFrameSDF(const Vec3T<T>& a_loCorner, const Vec3T<T>& a_hiCorner, const T a_thickness)
{
  EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_loCorner),
                     "BoxFrameSDF: the low corner must be finite (%g, %g, %g)",
                     double(a_loCorner[0]),
                     double(a_loCorner[1]),
                     double(a_loCorner[2]));
  EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_hiCorner),
                     "BoxFrameSDF: the high corner must be finite (%g, %g, %g)",
                     double(a_hiCorner[0]),
                     double(a_hiCorner[1]),
                     double(a_hiCorner[2]));
  EBGEOMETRY_REQUIRE(
    a_loCorner[0] < a_hiCorner[0] && a_loCorner[1] < a_hiCorner[1] && a_loCorner[2] < a_hiCorner[2],
    "BoxFrameSDF: the low corner (%g, %g, %g) must be below the high corner (%g, %g, %g) in every direction",
    double(a_loCorner[0]),
    double(a_loCorner[1]),
    double(a_loCorner[2]),
    double(a_hiCorner[0]),
    double(a_hiCorner[1]),
    double(a_hiCorner[2]));

  const Vec3T<T> size = a_hiCorner - a_loCorner;
  const T        side = Math::min(size[0], Math::min(size[1], size[2]));

  EBGEOMETRY_REQUIRE(std::isfinite(a_thickness) && a_thickness > T(0) && T(2) * a_thickness < side,
                     "BoxFrameSDF: the thickness must be positive and less than half the shortest side %g (%g)",
                     double(side),
                     double(a_thickness));

  m_loCorner  = a_loCorner;
  m_hiCorner  = a_hiCorner;
  m_thickness = a_thickness;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline Vec3T<T>
BoxFrameSDF<T>::barTerms(const Vec3T<T>& a_point) const noexcept
{
  const T        e = T(0.5) * m_thickness;
  const Vec3T<T> c = T(0.5) * (m_loCorner + m_hiCorner);
  const Vec3T<T> b = T(0.5) * (m_hiCorner - m_loCorner);

  // Folded into one octant: p is the signed distance to the box's faces along each axis, q that to the slab two
  // half-thicknesses inside them, which a bar fills across its own axis.
  Vec3T<T> p;
  Vec3T<T> q;

  for (size_t dir = 0; dir < 3; dir++) {
    p[dir] = std::abs(a_point[dir] - c[dir]) - b[dir];
    q[dir] = std::abs(p[dir] + e) - e;
  }

  // A bar along axis a: the box p_a <= 0 crossed with q <= 0 across it.
  const auto bar = [](const T a_x, const T a_y, const T a_z) noexcept {
    const T x = Math::max(a_x, T(0));
    const T y = Math::max(a_y, T(0));
    const T z = Math::max(a_z, T(0));

    return std::sqrt(x * x + y * y + z * z) + Math::min(Math::max(a_x, Math::max(a_y, a_z)), T(0));
  };

  return Vec3T<T>(bar(p[0], q[1], q[2]), bar(q[0], p[1], q[2]), bar(q[0], q[1], p[2]));
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline T
BoxFrameSDF<T>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  const Vec3T<T> terms   = this->barTerms(a_point);
  const T        outside = Math::min(terms[0], Math::min(terms[1], terms[2]));

  if (outside >= T(0)) {
    return outside;
  }

  // Inside, the nearest bar's depth undercuts the distance where bars meet: the boundary is the box's outer faces,
  // or the openings between the bars, where two coordinates are more than a thickness inside the outer faces.
  const Vec3T<T> c = T(0.5) * (m_loCorner + m_hiCorner);
  const Vec3T<T> b = T(0.5) * (m_hiCorner - m_loCorner);

  Vec3T<T> depth;
  Vec3T<T> opening;

  for (size_t dir = 0; dir < 3; dir++) {
    depth[dir]   = b[dir] - std::abs(a_point[dir] - c[dir]);
    opening[dir] = Math::max(m_thickness - depth[dir], T(0));
  }

  T inside = Math::min(depth[0], Math::min(depth[1], depth[2]));

  inside = Math::min(inside, std::sqrt(opening[0] * opening[0] + opening[1] * opening[1]));
  inside = Math::min(inside, std::sqrt(opening[0] * opening[0] + opening[2] * opening[2]));
  inside = Math::min(inside, std::sqrt(opening[1] * opening[1] + opening[2] * opening[2]));

  return -inside;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
BoxFrameSDF<T>::closestPart(const Vec3T<T>& a_point) const noexcept
{
  const Vec3T<T> terms = this->barTerms(a_point);
  const Vec3T<T> c     = T(0.5) * (m_loCorner + m_hiCorner);

  uint32_t axis = 0U;

  if (terms[1] < terms[axis]) {
    axis = 1U;
  }
  if (terms[2] < terms[axis]) {
    axis = 2U;
  }

  const uint32_t first  = (axis == 0U) ? 1U : 0U;
  const uint32_t second = (axis == 2U) ? 1U : 2U;

  return 4U * axis + ((a_point[first] >= c[first]) ? 2U : 0U) + ((a_point[second] >= c[second]) ? 1U : 0U);
}

namespace AnalyticDetail {

/**
 * @brief The distance from a point to the branch X^2 / A^2 - Y^2 / B^2 = 1, X > 0, of a hyperbola, for a point with
 * coordinates a_x, a_y >= 0.
 * @details Internal helper; not part of the public API. The closest point is (a_x A^2 / u, a_y B^2 / (A^2 + B^2 - u))
 * for the root u in (0, A^2 + B^2) of F(u) = (a_x A / u)^2 - (a_y B / (A^2 + B^2 - u))^2 - 1, which decreases from
 * +infinity to -infinity there, so the root is unique, as for an ellipse (Eberly). It is found by Newton steps kept
 * inside a bracket that bisection shrinks. A point on an axis has its closest point in closed form.
 * @tparam T Floating-point precision.
 * @param[in] a_A Semi-axis along X.
 * @param[in] a_B Semi-axis along Y.
 * @param[in] a_x The point's X, at least 0.
 * @param[in] a_y The point's Y, at least 0.
 * @return The distance.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline T
hyperbolaDistance(const T a_A, const T a_B, const T a_x, const T a_y) noexcept
{
  const T A2 = a_A * a_A;
  const T B2 = a_B * a_B;
  const T S  = A2 + B2;

  if (!(a_y > T(0))) {
    const T foot = a_x * A2 / S;

    if (foot > a_A) {
      const T Y = a_B * std::sqrt(foot * foot / A2 - T(1));

      return std::sqrt((foot - a_x) * (foot - a_x) + Y * Y);
    }

    return std::abs(a_x - a_A);
  }

  if (!(a_x > T(0))) {
    const T Y = a_y * B2 / S;
    const T X = a_A * std::sqrt(T(1) + Y * Y / B2);

    return std::sqrt(X * X + (Y - a_y) * (Y - a_y));
  }

  const T xA = a_x * a_A;
  const T yB = a_y * a_B;

  const auto F = [&](const T a_u) noexcept {
    const T first  = xA / a_u;
    const T second = yB / (S - a_u);

    return first * first - second * second - T(1);
  };

  // F is negative at u = x A (there X = A, and Y > 0) and below S.
  T hi = Math::min(xA, S);
  T lo = T(0.5) * hi;

  for (int i = 0; i < 256 && F(lo) <= T(0); i++) {
    hi = lo;
    lo = T(0.5) * lo;
  }

  constexpr int MaxIterations = 128;

  const T eps = Math::Limits<T>::epsilon();

  T u = T(0.5) * (lo + hi);

  for (int i = 0; i < MaxIterations; i++) {
    const T f = F(u);

    if (f == T(0)) {
      break;
    }

    if (f > T(0)) {
      lo = u;
    }
    else {
      hi = u;
    }

    const T first  = xA / u;
    const T second = yB / (S - u);
    const T df     = -T(2) * first * first / u - T(2) * second * second / (S - u);

    T next = u - f / df;

    if (!(next > lo && next < hi)) {
      next = T(0.5) * (lo + hi);
    }

    const bool converged = std::abs(next - u) <= T(4) * eps * u || hi - lo <= T(4) * eps * hi;

    u = next;

    if (converged) {
      break;
    }
  }

  const T X = xA * a_A / u;
  const T Y = yB * a_B / (S - u);

  return std::sqrt((X - a_x) * (X - a_x) + (Y - a_y) * (Y - a_y));
}

/**
 * @brief The distance from a point to the parabola y = -r^2 / (2 rho), for a point with coordinates a_r >= 0 and
 * a_y.
 * @details Internal helper; not part of the public API. The closest point (s, -s^2 / (2 rho)) has s the largest
 * root of the cubic s^3 + P s + Q = 0, P = 2 rho (rho + y), Q = -2 rho^2 r, solved in closed form (Cardano's
 * formula in a form that does not cancel, or the trigonometric one where the cubic has three real roots) and
 * polished by one Newton step.
 * @tparam T Floating-point precision.
 * @param[in] a_rho The radius of curvature at the tip.
 * @param[in] a_r   The point's distance from the axis.
 * @param[in] a_y   The point's height above the tip.
 * @return The distance.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline T
parabolaDistance(const T a_rho, const T a_r, const T a_y) noexcept
{
  const T P     = T(2) * a_rho * (a_rho + a_y);
  const T Q     = -T(2) * a_rho * a_rho * a_r;
  const T third = P / T(3);
  const T disc  = T(0.25) * Q * Q + third * third * third;

  T s = T(0);

  if (disc >= T(0)) {
    const T a = std::cbrt(-T(0.5) * Q + std::sqrt(disc));

    if (a > T(0)) {
      const T b = -third / a;

      // s = a + b; for P > 0 the two cancel, and -Q / (a^2 - a b + b^2) is the same root without cancellation.
      s = (P > T(0)) ? -Q / (a * a + third + b * b) : a + b;
    }
  }
  else {
    const T m      = T(2) * std::sqrt(-third);
    const T cosine = Math::clamp(T(1.5) * Q / P * std::sqrt(-T(3) / P), T(-1), T(1));

    s = m * std::cos(std::acos(cosine) / T(3));
  }

  const T derivative = T(3) * s * s + P;

  if (derivative > T(0)) {
    s -= (s * s * s + P * s + Q) / derivative;
  }

  const T dr = s - a_r;
  const T dy = a_y + s * s / (T(2) * a_rho);

  return std::sqrt(dr * dr + dy * dy);
}

/**
 * @brief The p-norm (|a|^p + |b|^p)^(1/p) of two values at least 0, scaled by the larger so that no power overflows.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_a First value, at least 0.
 * @param[in] a_b Second value, at least 0.
 * @param[in] a_p Exponent, at least 1.
 * @return The norm.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline T
pNorm(const T a_a, const T a_b, const T a_p) noexcept
{
  const T m = Math::max(a_a, a_b);

  if (!(m > T(0))) {
    return T(0);
  }

  return m * std::pow(std::pow(a_a / m, a_p) + std::pow(a_b / m, a_p), T(1) / a_p);
}

/**
 * @brief One over the largest slope of a superellipsoid's gauge: its smallest semi-axis, over the largest ratio of
 * its nested norm to the Euclidean one.
 * @details Internal helper; not part of the public API. With exponents e <= 2, the gauge is a norm of the point
 * scaled by the semi-axes, built of two nested p-norms with p = 2 / e >= 1. A p-norm of two values is at most
 * 2^(1/p - 1/2) times their Euclidean norm when p < 2 and at most that norm otherwise, so the nested norm is at most
 * the product of the two factors times the Euclidean norm of the scaled point, which is at most the point's norm
 * over the smallest semi-axis. By the triangle inequality, the gauge then changes by at most that slope times the
 * distance moved.
 * @tparam T Floating-point precision.
 * @param[in] a_radii     The semi-axes.
 * @param[in] a_exponent1 The exponent through the y axis.
 * @param[in] a_exponent2 The exponent across it.
 * @return The scale.
 */
template <class T>
[[nodiscard]] inline T
superellipsoidScale(const Vec3T<T>& a_radii, const T a_exponent1, const T a_exponent2) noexcept
{
  const auto factor = [](const T a_e) { return std::pow(T(2), Math::max(T(0), T(0.5) * (a_e - T(1)))); };

  return Math::min(a_radii[0], Math::min(a_radii[1], a_radii[2])) / (factor(a_exponent1) * factor(a_exponent2));
}

} // namespace AnalyticDetail

/**
 * @brief Signed distance field for a quadric of revolution around y: a hyperboloid of one sheet, a hyperboloid of
 * two sheets, or a paraboloid.
 * @details The three share their code: in the cross-section through the axis, the surface is a conic (a hyperbola
 * or a parabola), and the distance to it is exact, from the closest point on the conic: the root of a scalar
 * equation found by safeguarded Newton steps for a hyperbola, as for EllipsoidSDF, and a cubic solved in closed form
 * for a parabola. Each is unbounded: cut it with an Intersection, or use it to cut. They are made by the static
 * functions hyperboloid(), hyperboloidTwoSheets() and paraboloid(), which return a QuadricSDF; turn them with
 * Rotate. By default the quadric is the paraboloid with its tip at the origin and a tip radius of 1.
 * @tparam T Floating-point precision.
 */
template <class T>
class QuadricSDF
{
  static_assert(std::is_floating_point_v<T>, "QuadricSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: the Euclidean distance to the surface.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  /**
   * @brief Which quadric, with r the distance from the axis and y the height above the center (or the tip).
   */
  enum class Kind : uint32_t
  {
    HyperboloidOneSheet,  ///< r^2 / a^2 - y^2 / c^2 <= 1: a waisted tower, one piece.
    HyperboloidTwoSheets, ///< y^2 / c^2 - r^2 / a^2 >= 1: two cups facing away from each other.
    Paraboloid            ///< y <= -r^2 / (2 rho): a cup opening toward -y from its tip.
  };

  /**
   * @brief Default constructor. Constructs the paraboloid with its tip at the origin, opening toward -y, with a tip
   * radius of 1.
   */
  QuadricSDF() = default;

  /**
   * @brief A hyperboloid of one sheet around the y axis through the center: a waisted tower, unbounded along y.
   * @param[in] a_center The center, on the axis at the narrowest point.
   * @param[in] a_waist  The radius at the narrowest point, finite and positive.
   * @param[in] a_angle  The full opening angle of the asymptotic cone, in degrees, in (0, 180): the hyperboloid
   * widens toward that cone far from the center.
   * @return The hyperboloid.
   */
  [[nodiscard]] static QuadricSDF
  hyperboloid(const Vec3T<T>& a_center, const T a_waist, const T a_angle)
  {
    QuadricSDF q = QuadricSDF::checked(a_center, a_waist, a_angle, "QuadricSDF::hyperboloid");

    q.m_kind = Kind::HyperboloidOneSheet;
    q.m_a    = a_waist;
    q.m_c    = a_waist / std::tan(T(0.5) * a_angle * pi<T> / T(180));

    return q;
  }

  /**
   * @brief A hyperboloid of two sheets around the y axis through the center: two cups, with their tips on the axis
   * at the given distance above and below the center, each opening away from it, unbounded along y.
   * @details The solid is inside the two cups. The radius of curvature at a tip is a_tipDistance tan^2(angle / 2).
   * @param[in] a_center      The center, midway between the tips.
   * @param[in] a_tipDistance The distance from the center to each tip, finite and positive.
   * @param[in] a_angle       The full opening angle of the asymptotic cone, in degrees, in (0, 180).
   * @return The hyperboloid.
   */
  [[nodiscard]] static QuadricSDF
  hyperboloidTwoSheets(const Vec3T<T>& a_center, const T a_tipDistance, const T a_angle)
  {
    QuadricSDF q = QuadricSDF::checked(a_center, a_tipDistance, a_angle, "QuadricSDF::hyperboloidTwoSheets");

    q.m_kind = Kind::HyperboloidTwoSheets;
    q.m_c    = a_tipDistance;
    q.m_a    = a_tipDistance * std::tan(T(0.5) * a_angle * pi<T> / T(180));

    return q;
  }

  /**
   * @brief A paraboloid around the y axis through its tip, opening toward -y, as the cones do, unbounded below.
   * @param[in] a_tip       The tip.
   * @param[in] a_tipRadius The radius of curvature at the tip, finite and positive: the paraboloid is y = -r^2 / (2
   * a_tipRadius) below the tip.
   * @return The paraboloid.
   */
  [[nodiscard]] static QuadricSDF
  paraboloid(const Vec3T<T>& a_tip, const T a_tipRadius)
  {
    QuadricSDF q = QuadricSDF::checked(a_tip, a_tipRadius, T(90), "QuadricSDF::paraboloid");

    q.m_kind = Kind::Paraboloid;
    q.m_a    = a_tipRadius;
    q.m_c    = a_tipRadius;

    return q;
  }

  /**
   * @brief Copy constructor.
   */
  QuadricSDF(const QuadricSDF&) = default;

  /**
   * @brief Move constructor.
   */
  QuadricSDF(QuadricSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  QuadricSDF&
  operator=(const QuadricSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  QuadricSDF&
  operator=(QuadricSDF&&) = default;

  /**
   * @brief Get which quadric this is.
   * @return m_kind.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  Kind
  getKind() const noexcept
  {
    return m_kind;
  }

  /**
   * @brief Get the center (a hyperboloid) or the tip (a paraboloid).
   * @return m_center.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getCenter() const noexcept
  {
    return m_center;
  }

  /**
   * @brief Get the semi-axis across y, a, of a hyperboloid, or the tip radius of a paraboloid.
   * @return m_a.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getA() const noexcept
  {
    return m_a;
  }

  /**
   * @brief Get the semi-axis along y, c, of a hyperboloid; for a paraboloid, its tip radius.
   * @return m_c.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getC() const noexcept
  {
    return m_c;
  }

  /**
   * @brief Signed distance function for the quadric.
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

    const Vec3T<T> w = a_point - m_center;
    const T        r = std::sqrt(w[0] * w[0] + w[2] * w[2]);
    const T        y = w[1];

    if (m_kind == Kind::Paraboloid) {
      const T d = AnalyticDetail::parabolaDistance(m_a, r, y);

      return (y + r * r / (T(2) * m_a) < T(0)) ? -d : d;
    }

    // Folded into the quadrant of the branch: one sheet's hyperbola opens along r, two sheets' along y.
    const bool oneSheet = m_kind == Kind::HyperboloidOneSheet;
    const T    X        = oneSheet ? r : std::abs(y);
    const T    Y        = oneSheet ? std::abs(y) : r;
    const T    A        = oneSheet ? m_a : m_c;
    const T    B        = oneSheet ? m_c : m_a;

    const T    d      = AnalyticDetail::hyperbolaDistance(A, B, X, Y);
    const T    g      = (X / A) * (X / A) - (Y / B) * (Y / B) - T(1);
    const bool inside = oneSheet ? (g < T(0)) : (g > T(0));

    return inside ? -d : d;
  }

  /**
   * @brief The shape's axis-aligned bounding box: unbounded, but above the tip of a paraboloid.
   * @return The bounding box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    const T big = Math::Limits<T>::max();
    const T top = (m_kind == Kind::Paraboloid) ? m_center[1] : big;

    return BoundingVolumes::AABBT<T>(Vec3T<T>(-big, -big, -big), Vec3T<T>(big, top, big));
  }

protected:
  /**
   * @brief Checks a factory's arguments, and makes a quadric at the center.
   * @param[in] a_center The center or tip.
   * @param[in] a_size   The size, finite and positive.
   * @param[in] a_angle  The angle in degrees, in (0, 180).
   * @param[in] a_who    The factory, for the messages.
   * @return A quadric with its center set.
   */
  static QuadricSDF
  checked(const Vec3T<T>& a_center, const T a_size, const T a_angle, const char* a_who)
  {
    EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_center),
                       "%s: the center must be finite (%g, %g, %g)",
                       a_who,
                       double(a_center[0]),
                       double(a_center[1]),
                       double(a_center[2]));
    EBGEOMETRY_REQUIRE(
      std::isfinite(a_size) && a_size > T(0), "%s: the size must be finite and positive (%g)", a_who, double(a_size));
    EBGEOMETRY_REQUIRE(
      a_angle > T(0) && a_angle < T(180), "%s: the angle must be in (0, 180) degrees (%g)", a_who, double(a_angle));

    QuadricSDF q;

    q.m_center = a_center;

    return q;
  }

  /**
   * @brief Which quadric.
   */
  Kind m_kind = Kind::Paraboloid;

  /**
   * @brief The center (a hyperboloid) or the tip (a paraboloid).
   */
  Vec3T<T> m_center = Vec3T<T>::zeros();

  /**
   * @brief The semi-axis across y, a (a hyperboloid), or the tip radius rho (a paraboloid).
   */
  T m_a = T(1);

  /**
   * @brief The semi-axis along y, c (a hyperboloid); the tip radius for a paraboloid.
   */
  T m_c = T(1);
};

/**
 * @brief Signed distance field for a superellipsoid (Barr's superquadric): from an ellipsoid to a rounded box, and
 * toward a box or an octahedron, with its semi-axes along x, y and z.
 * @details The surface is (|x/a|^(2/e2) + |z/c|^(2/e2))^(e2/e1) + |y/b|^(2/e1) = 1 around the center: e1 shapes the
 * cross-sections through the y axis, e2 those across it. Both exponents are in [0.1, 2], where the solid is convex:
 * 1 gives an ellipsoid, toward 0.1 a box, 2 an octahedron (a double cone where only one is 2). The value is a bound:
 * the superquadric's gauge (the left side above, to the power e1 / 2, which is a norm of the scaled point) less 1,
 * divided by the gauge's largest slope, so that it never exceeds the distance. Far from the surface, and for
 * unequal semi-axes, it is well below it. Turn it with Rotate. By default the superellipsoid is centered at the
 * origin, with semi-axes 1 and both exponents 0.5.
 * @tparam T Floating-point precision.
 */
template <class T>
class SuperellipsoidSDF
{
  static_assert(std::is_floating_point_v<T>, "SuperellipsoidSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: never more than the Euclidean distance.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Bound;

  /**
   * @brief Default constructor. Constructs a superellipsoid at the origin, with semi-axes 1 and both exponents 0.5.
   */
  SuperellipsoidSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_center    Center.
   * @param[in] a_radii     Semi-axes along x, y and z, finite and positive.
   * @param[in] a_exponent1 The exponent e1 of the cross-sections through the y axis, in [0.1, 2].
   * @param[in] a_exponent2 The exponent e2 of the cross-sections across it, in [0.1, 2].
   */
  SuperellipsoidSDF(const Vec3T<T>& a_center, const Vec3T<T>& a_radii, const T a_exponent1, const T a_exponent2)
  {
    EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_center),
                       "SuperellipsoidSDF: the center must be finite (%g, %g, %g)",
                       double(a_center[0]),
                       double(a_center[1]),
                       double(a_center[2]));
    EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_radii) && a_radii[0] > T(0) && a_radii[1] > T(0) && a_radii[2] > T(0),
                       "SuperellipsoidSDF: the semi-axes must be finite and positive (%g, %g, %g)",
                       double(a_radii[0]),
                       double(a_radii[1]),
                       double(a_radii[2]));
    EBGEOMETRY_REQUIRE(a_exponent1 >= T(0.1) && a_exponent1 <= T(2) && a_exponent2 >= T(0.1) && a_exponent2 <= T(2),
                       "SuperellipsoidSDF: the exponents must be in [0.1, 2] (%g, %g)",
                       double(a_exponent1),
                       double(a_exponent2));

    m_center    = a_center;
    m_radii     = a_radii;
    m_exponent1 = a_exponent1;
    m_exponent2 = a_exponent2;
    m_scale     = AnalyticDetail::superellipsoidScale(a_radii, a_exponent1, a_exponent2);
  }

  /**
   * @brief Copy constructor.
   */
  SuperellipsoidSDF(const SuperellipsoidSDF&) = default;

  /**
   * @brief Move constructor.
   */
  SuperellipsoidSDF(SuperellipsoidSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  SuperellipsoidSDF&
  operator=(const SuperellipsoidSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  SuperellipsoidSDF&
  operator=(SuperellipsoidSDF&&) = default;

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
   * @brief The superquadric's gauge at a point: below 1 inside, 1 on the surface, above 1 outside.
   * @param[in] a_point Position.
   * @return The gauge.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  gauge(const Vec3T<T>& a_point) const noexcept
  {
    const Vec3T<T> w = a_point - m_center;
    const T        x = std::abs(w[0] / m_radii[0]);
    const T        y = std::abs(w[1] / m_radii[1]);
    const T        z = std::abs(w[2] / m_radii[2]);

    const T across = AnalyticDetail::pNorm(x, z, T(2) / m_exponent2);

    return AnalyticDetail::pNorm(across, y, T(2) / m_exponent1);
  }

  /**
   * @brief Signed distance function for the superellipsoid: a bound.
   * @param[in] a_point Position.
   * @return A signed value whose magnitude never exceeds the distance from a_point to the surface; negative inside.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    return (this->gauge(a_point) - T(1)) * m_scale;
  }

  /**
   * @brief The shape's axis-aligned bounding box: the center plus and minus the semi-axes.
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
   * @brief Semi-axes.
   */
  Vec3T<T> m_radii = Vec3T<T>::ones();

  /**
   * @brief The exponent through the y axis.
   */
  T m_exponent1 = T(0.5);

  /**
   * @brief The exponent across it.
   */
  T m_exponent2 = T(0.5);

  /**
   * @brief One over the gauge's largest slope.
   */
  T m_scale = T(1);
};

/**
 * @brief Signed distance field for a solid angle: the part of a ball inside a cone from its center, around +y.
 * @details The cone has its tip at the ball's center and opens toward +y with the given full angle; above 180
 * degrees the solid is the ball less a cone opening toward -y. The distance is exact (Quilez's solid angle): in the
 * cross-section through the axis, that to the arc or to the cone's side line. Turn it with Rotate. By default the
 * solid angle is centered at the origin, with radius 1 and an opening of 60 degrees.
 * @tparam T Floating-point precision.
 */
template <class T>
class SolidAngleSDF
{
  static_assert(std::is_floating_point_v<T>, "SolidAngleSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: the Euclidean distance to the surface.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  /**
   * @brief Default constructor. Constructs a solid angle at the origin, with radius 1 and an opening of 60 degrees.
   */
  SolidAngleSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_center The center of the ball, the cone's tip.
   * @param[in] a_radius The radius, finite and positive.
   * @param[in] a_angle  The cone's full opening angle, in degrees, in (0, 360).
   */
  SolidAngleSDF(const Vec3T<T>& a_center, const T a_radius, const T a_angle)
  {
    EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_center),
                       "SolidAngleSDF: the center must be finite (%g, %g, %g)",
                       double(a_center[0]),
                       double(a_center[1]),
                       double(a_center[2]));
    EBGEOMETRY_REQUIRE(std::isfinite(a_radius) && a_radius > T(0),
                       "SolidAngleSDF: the radius must be finite and positive (%g)",
                       double(a_radius));
    EBGEOMETRY_REQUIRE(
      a_angle > T(0) && a_angle < T(360), "SolidAngleSDF: the angle must be in (0, 360) degrees (%g)", double(a_angle));

    m_center = a_center;
    m_radius = a_radius;
    m_angle  = a_angle;
    m_sin    = std::sin(T(0.5) * a_angle * pi<T> / T(180));
    m_cos    = std::cos(T(0.5) * a_angle * pi<T> / T(180));
  }

  /**
   * @brief Copy constructor.
   */
  SolidAngleSDF(const SolidAngleSDF&) = default;

  /**
   * @brief Move constructor.
   */
  SolidAngleSDF(SolidAngleSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  SolidAngleSDF&
  operator=(const SolidAngleSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  SolidAngleSDF&
  operator=(SolidAngleSDF&&) = default;

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
   * @brief Get the full opening angle, in degrees.
   * @return m_angle.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getAngle() const noexcept
  {
    return m_angle;
  }

  /**
   * @brief Signed distance function for the solid angle.
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

    T sphere;
    T side;

    this->terms(a_point, sphere, side);

    return Math::max(sphere, side);
  }

  /**
   * @brief How many parts the shape has (see hasParts): 2, the spherical cap (0) and the cone's side (1).
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  getNumParts() const noexcept
  {
    return 2U;
  }

  /**
   * @brief The part whose term decides the value at a point (see hasParts): 0 the cap, 1 the cone's side.
   * @param[in] a_point Position.
   * @return The part.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  closestPart(const Vec3T<T>& a_point) const noexcept
  {
    T sphere;
    T side;

    this->terms(a_point, sphere, side);

    return (sphere >= side) ? 0U : 1U;
  }

  /**
   * @brief The shape's axis-aligned bounding box: that of the ball.
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
   * @brief The two terms of the distance: to the sphere, and the signed distance to the cone's side segment.
   * @param[in]  a_point  Position.
   * @param[out] a_sphere The signed distance to the sphere.
   * @param[out] a_side   The distance to the side segment, from the center to the rim, negative inside the cone.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  terms(const Vec3T<T>& a_point, T& a_sphere, T& a_side) const noexcept
  {
    const Vec3T<T> w = a_point - m_center;
    const T        r = std::sqrt(w[0] * w[0] + w[2] * w[2]);
    const T        y = w[1];

    const T along = Math::clamp(r * m_sin + y * m_cos, T(0), m_radius);
    const T dr    = r - along * m_sin;
    const T dy    = y - along * m_cos;
    const T cross = m_cos * r - m_sin * y;

    a_sphere = std::sqrt(r * r + y * y) - m_radius;
    a_side   = std::sqrt(dr * dr + dy * dy) * ((cross > T(0)) ? T(1) : ((cross < T(0)) ? T(-1) : T(0)));
  }

  /**
   * @brief Center.
   */
  Vec3T<T> m_center = Vec3T<T>::zeros();

  /**
   * @brief Radius.
   */
  T m_radius = T(1);

  /**
   * @brief Full opening angle, in degrees.
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
 * @brief Signed distance field for a revolved vesica: a lens between two tips, the vesica (the overlap of two discs)
 * in a plane through the tips, turned around the line through them.
 * @details In the cross-section through the axis, the lens' outline is an arc of a circle centered across the axis
 * from it, through both tips; the given radius is the lens' radius at its middle. The distance is exact (Quilez's
 * vesica): to the arc, or to the nearer tip. By default the vesica runs from (0,-1,0) to (0,1,0), with radius 0.5.
 * @tparam T Floating-point precision.
 */
template <class T>
class VesicaSDF
{
  static_assert(std::is_floating_point_v<T>, "VesicaSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: the Euclidean distance to the surface.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  /**
   * @brief Default constructor. Constructs a vesica from (0,-1,0) to (0,1,0), with radius 0.5.
   */
  VesicaSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_tip1   One tip.
   * @param[in] a_tip2   The other tip; it must differ from a_tip1.
   * @param[in] a_radius The lens' radius at its middle, finite, positive and less than half the distance between
   * the tips (a radius of half that distance would make a ball).
   */
  VesicaSDF(const Vec3T<T>& a_tip1, const Vec3T<T>& a_tip2, const T a_radius)
  {
    EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_tip1) && AnalyticDetail::isFinite(a_tip2),
                       "VesicaSDF: the tips must be finite (%g, %g, %g) and (%g, %g, %g)",
                       double(a_tip1[0]),
                       double(a_tip1[1]),
                       double(a_tip1[2]),
                       double(a_tip2[0]),
                       double(a_tip2[1]),
                       double(a_tip2[2]));

    const T half = T(0.5) * (a_tip2 - a_tip1).length();

    EBGEOMETRY_REQUIRE(half > T(0),
                       "VesicaSDF: the two tips must differ, but both are (%g, %g, %g)",
                       double(a_tip1[0]),
                       double(a_tip1[1]),
                       double(a_tip1[2]));
    EBGEOMETRY_REQUIRE(
      std::isfinite(a_radius) && a_radius > T(0) && a_radius < half,
      "VesicaSDF: the radius must be positive and less than half the distance between the tips %g (%g)",
      double(half),
      double(a_radius));

    m_tip1   = a_tip1;
    m_tip2   = a_tip2;
    m_radius = a_radius;
  }

  /**
   * @brief Copy constructor.
   */
  VesicaSDF(const VesicaSDF&) = default;

  /**
   * @brief Move constructor.
   */
  VesicaSDF(VesicaSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  VesicaSDF&
  operator=(const VesicaSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  VesicaSDF&
  operator=(VesicaSDF&&) = default;

  /**
   * @brief Get one tip.
   * @return m_tip1.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getTip1() const noexcept
  {
    return m_tip1;
  }

  /**
   * @brief Get the other tip.
   * @return m_tip2.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getTip2() const noexcept
  {
    return m_tip2;
  }

  /**
   * @brief Get the radius at the middle.
   * @return m_radius.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getRadius() const noexcept
  {
    return m_radius;
  }

  /**
   * @brief Signed distance function for the vesica.
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

    const Vec3T<T> c      = T(0.5) * (m_tip1 + m_tip2);
    const Vec3T<T> axis   = m_tip2 - m_tip1;
    const T        length = axis.length();
    const Vec3T<T> v      = axis / length;
    const Vec3T<T> w      = a_point - c;
    const T        along  = dot(w, v);
    const T        qx     = (w - along * v).length();
    const T        qy     = std::abs(along);
    const T        half   = T(0.5) * length;

    // The arc's circle is centered at -d across the axis, with radius d + radius.
    const T d = T(0.5) * (half * half - m_radius * m_radius) / m_radius;

    // Beyond the line from the circle's center through the tip, the tip is the closest point.
    if (half * qx < d * (qy - half)) {
      return std::sqrt(qx * qx + (qy - half) * (qy - half));
    }

    return std::sqrt((qx + d) * (qx + d) + qy * qy) - (d + m_radius);
  }

  /**
   * @brief The shape's axis-aligned bounding box.
   * @return The bounding box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    return BoundingVolumes::AABBT<T>(min(m_tip1, m_tip2) - m_radius * Vec3T<T>::ones(),
                                     max(m_tip1, m_tip2) + m_radius * Vec3T<T>::ones());
  }

protected:
  /**
   * @brief One tip.
   */
  Vec3T<T> m_tip1 = Vec3T<T>(T(0), T(-1), T(0));

  /**
   * @brief The other tip.
   */
  Vec3T<T> m_tip2 = Vec3T<T>(T(0), T(1), T(0));

  /**
   * @brief Radius at the middle.
   */
  T m_radius = T(0.5);
};

/**
 * @brief Signed distance field for a helical tube: a spring, a coil or a thread, with rounded ends.
 * @details The tube's center line is a right-handed helix around the axis from the first end center to the second,
 * starting from the first in the given direction across the axis; Reflect makes a left-handed one. The value is the
 * distance to the center line less the tube's radius, exact. The squared distance to the line, as a function of the
 * helix angle, is searched turn by turn, in windows of one turn centered on the point's own angle: in each, it has at
 * most one minimum inside, where its derivative increases, which Newton steps kept in a bracket find; the window's
 * ends and the helix' ends are the other candidates. The search starts at the turn nearest the point's height and
 * goes outward while a lower bound on a window's distance (its height difference alone) can still beat the best,
 * which is one to three windows for most points. The tube must not touch itself: its radius is less than the helix
 * radius, and less than half the pitch. By default the helix runs from (0,-1,0) to (0,1,0) in 4 turns of radius 0.5
 * starting along x, with a tube radius of 0.1.
 * @tparam T Floating-point precision.
 */
template <class T>
class HelixSDF
{
  static_assert(std::is_floating_point_v<T>, "HelixSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: the Euclidean distance to the surface.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  /**
   * @brief Default constructor. Constructs a helix from (0,-1,0) to (0,1,0) in 4 turns of radius 0.5, starting
   * along x, with a tube radius of 0.1.
   */
  HelixSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_center1        The axis point at the first end.
   * @param[in] a_center2        The axis point at the second end; it must differ from a_center1.
   * @param[in] a_helixRadius    The distance from the axis to the tube's center line, finite and positive.
   * @param[in] a_turns          The number of turns, finite and positive (not necessarily whole).
   * @param[in] a_tubeRadius     The tube's radius, positive, less than a_helixRadius and than half the pitch.
   * @param[in] a_startDirection Direction from the axis to the center line at the first end. Only its part across
   * the axis is used, and that must not be zero.
   */
  HelixSDF(const Vec3T<T>& a_center1,
           const Vec3T<T>& a_center2,
           const T         a_helixRadius,
           const T         a_turns,
           const T         a_tubeRadius,
           const Vec3T<T>& a_startDirection)
  {
    EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_center1) && AnalyticDetail::isFinite(a_center2),
                       "HelixSDF: the centers must be finite (%g, %g, %g) and (%g, %g, %g)",
                       double(a_center1[0]),
                       double(a_center1[1]),
                       double(a_center1[2]),
                       double(a_center2[0]),
                       double(a_center2[1]),
                       double(a_center2[2]));

    const T length = (a_center2 - a_center1).length();

    EBGEOMETRY_REQUIRE(length > T(0),
                       "HelixSDF: the two centers must differ, but both are (%g, %g, %g)",
                       double(a_center1[0]),
                       double(a_center1[1]),
                       double(a_center1[2]));
    EBGEOMETRY_REQUIRE(std::isfinite(a_helixRadius) && a_helixRadius > T(0),
                       "HelixSDF: the helix radius must be finite and positive (%g)",
                       double(a_helixRadius));
    EBGEOMETRY_REQUIRE(std::isfinite(a_turns) && a_turns > T(0),
                       "HelixSDF: the number of turns must be finite and positive (%g)",
                       double(a_turns));

    const T pitch = length / a_turns;

    EBGEOMETRY_REQUIRE(std::isfinite(a_tubeRadius) && a_tubeRadius > T(0) && a_tubeRadius < a_helixRadius &&
                         T(2) * a_tubeRadius < pitch,
                       "HelixSDF: the tube radius must be positive and less than the helix radius %g and half the "
                       "pitch %g (%g)",
                       double(a_helixRadius),
                       double(T(0.5) * pitch),
                       double(a_tubeRadius));

    const Vec3T<T> axis   = (a_center2 - a_center1) / length;
    const Vec3T<T> across = a_startDirection - dot(a_startDirection, axis) * axis;

    EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_startDirection) && across.length() > T(0),
                       "HelixSDF: the start direction (%g, %g, %g) must have a part across the axis",
                       double(a_startDirection[0]),
                       double(a_startDirection[1]),
                       double(a_startDirection[2]));

    m_center1     = a_center1;
    m_center2     = a_center2;
    m_axis        = axis;
    m_start       = across / across.length();
    m_helixRadius = a_helixRadius;
    m_turns       = a_turns;
    m_tubeRadius  = a_tubeRadius;
  }

  /**
   * @brief Copy constructor.
   */
  HelixSDF(const HelixSDF&) = default;

  /**
   * @brief Move constructor.
   */
  HelixSDF(HelixSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  HelixSDF&
  operator=(const HelixSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  HelixSDF&
  operator=(HelixSDF&&) = default;

  /**
   * @brief Get the axis point at the first end.
   * @return m_center1.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getCenter1() const noexcept
  {
    return m_center1;
  }

  /**
   * @brief Get the axis point at the second end.
   * @return m_center2.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getCenter2() const noexcept
  {
    return m_center2;
  }

  /**
   * @brief Get the helix radius.
   * @return m_helixRadius.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getHelixRadius() const noexcept
  {
    return m_helixRadius;
  }

  /**
   * @brief Get the number of turns.
   * @return m_turns.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getTurns() const noexcept
  {
    return m_turns;
  }

  /**
   * @brief Get the tube radius.
   * @return m_tubeRadius.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getTubeRadius() const noexcept
  {
    return m_tubeRadius;
  }

  /**
   * @brief Get the unit direction from the axis to the center line at the first end.
   * @return m_start.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getStartDirection() const noexcept
  {
    return m_start;
  }

  /**
   * @brief Get the point on the center line at a helix angle.
   * @param[in] a_angle The angle in radians, from 0 at the first end to 2 pi getTurns() at the second.
   * @return The point.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  Vec3T<T>
  centerLine(const T a_angle) const noexcept
  {
    const T rise = a_angle * (m_center2 - m_center1).length() / (T(2) * pi<T> * m_turns);

    return m_center1 + rise * m_axis +
           m_helixRadius * (std::cos(a_angle) * m_start + std::sin(a_angle) * cross(m_axis, m_start));
  }

  /**
   * @brief Signed distance function for the helix.
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

    const T twoPi = T(2) * pi<T>;

    // The point around the axis: its height y, its distance rho from the axis, and its angle phi from the start.
    const Vec3T<T> w   = a_point - m_center1;
    const T        y   = dot(w, m_axis);
    const T        x   = dot(w, m_start);
    const T        z   = dot(w, cross(m_axis, m_start));
    const T        rho = std::sqrt(x * x + z * z);
    const T        phi = std::atan2(z, x);

    // The rise per radian, and the last angle.
    const T rate = (m_center2 - m_center1).length() / (twoPi * m_turns);
    const T last = twoPi * m_turns;

    // Window k covers the angles phi + 2 pi k + s, |s| <= pi.
    const T kLow  = std::ceil((-pi<T> - phi) / twoPi);
    const T kHigh = std::floor((last + pi<T> - phi) / twoPi);
    const T kNear = Math::clamp(std::round((y / rate - phi) / twoPi), kLow, kHigh);

    const T across = (rho - m_helixRadius) * (rho - m_helixRadius);

    T best = this->windowMinimum(kNear, rho, phi, y, rate, last);

    // Outward from the nearest window, while a window's height difference alone does not rule it out.
    for (T step = T(1); step <= kHigh - kLow; step += T(1)) {
      bool further = false;

      for (int side = 0; side < 2; side++) {
        const T k = (side == 0) ? kNear - step : kNear + step;

        if (k < kLow || k > kHigh) {
          continue;
        }

        const T lo   = Math::max(phi + twoPi * k - T(0.5) * twoPi, T(0));
        const T hi   = Math::min(phi + twoPi * k + T(0.5) * twoPi, last);
        const T gap  = Math::max(Math::max(rate * lo - y, y - rate * hi), T(0));
        const T rule = across + gap * gap;

        if (rule < best) {
          further = true;
          best    = Math::min(best, this->windowMinimum(k, rho, phi, y, rate, last));
        }
      }

      if (!further) {
        break;
      }
    }

    return std::sqrt(best) - m_tubeRadius;
  }

  /**
   * @brief The shape's axis-aligned bounding box: that of the cylinder the tube fits in.
   * @return The bounding box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    const T reach = m_helixRadius + m_tubeRadius;

    return BoundingVolumes::AABBT<T>(min(m_center1, m_center2) - reach * Vec3T<T>::ones(),
                                     max(m_center1, m_center2) + reach * Vec3T<T>::ones());
  }

protected:
  /**
   * @brief The smallest squared distance from a point to the center line over one window of helix angles,
   * phi + 2 pi k + s with |s| <= pi, cut to the helix' angles.
   * @details f(s) = rho^2 + R^2 - 2 rho R cos(s) + (Y - rate s)^2, with Y the point's height over the line at s = 0.
   * Its derivative is 2 g(s), g(s) = rho R sin(s) + rate^2 s - rate Y, which increases where rho R cos(s) + rate^2 >
   * 0, on (-s_c, s_c), and decreases outside it. A minimum inside the window is therefore where g crosses zero in
   * (-s_c, s_c), at most once; otherwise f is least at an end.
   * @param[in] a_k    The window, as a whole number.
   * @param[in] a_rho  The point's distance from the axis.
   * @param[in] a_phi  The point's angle around the axis, from the start direction.
   * @param[in] a_y    The point's height along the axis, from the first end.
   * @param[in] a_rate The rise per radian.
   * @param[in] a_last The last angle, 2 pi times the turns.
   * @return The squared distance.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  windowMinimum(const T a_k, const T a_rho, const T a_phi, const T a_y, const T a_rate, const T a_last) const noexcept
  {
    const T halfTurn = pi<T>;
    const T base     = a_phi + T(2) * halfTurn * a_k;
    const T Y        = a_y - a_rate * base;
    const T rR       = a_rho * m_helixRadius;
    const T rate2    = a_rate * a_rate;
    const T A        = a_rho * a_rho + m_helixRadius * m_helixRadius;

    const T sLo = Math::max(-halfTurn, -base);
    const T sHi = Math::min(halfTurn, a_last - base);

    const auto f = [&](const T a_s) noexcept {
      const T v = Y - a_rate * a_s;

      return A - T(2) * rR * std::cos(a_s) + v * v;
    };

    const auto g = [&](const T a_s) noexcept { return rR * std::sin(a_s) + rate2 * a_s - a_rate * Y; };

    T best = Math::min(f(sLo), f(sHi));

    const T sc = (rR > rate2) ? std::acos(-rate2 / rR) : halfTurn;
    T       lo = Math::max(-sc, sLo);
    T       hi = Math::min(sc, sHi);

    if (lo < hi && g(lo) < T(0) && g(hi) > T(0)) {
      constexpr int MaxIterations = 64;

      const T eps = Math::Limits<T>::epsilon();

      T s = Math::clamp(Y / a_rate, lo, hi);

      for (int i = 0; i < MaxIterations; i++) {
        const T value = g(s);

        if (value == T(0)) {
          break;
        }

        if (value < T(0)) {
          lo = s;
        }
        else {
          hi = s;
        }

        const T slope = rR * std::cos(s) + rate2;

        T next = (slope > T(0)) ? s - value / slope : T(0.5) * (lo + hi);

        if (!(next > lo && next < hi)) {
          next = T(0.5) * (lo + hi);
        }

        const bool converged = std::abs(next - s) <= T(4) * eps * Math::max(T(1), std::abs(s)) ||
                               hi - lo <= T(4) * eps * Math::max(T(1), std::abs(s));

        s = next;

        if (converged) {
          break;
        }
      }

      best = Math::min(best, f(s));
    }

    return Math::max(best, T(0));
  }

  /**
   * @brief The axis point at the first end.
   */
  Vec3T<T> m_center1 = Vec3T<T>(T(0), T(-1), T(0));

  /**
   * @brief The axis point at the second end.
   */
  Vec3T<T> m_center2 = Vec3T<T>(T(0), T(1), T(0));

  /**
   * @brief Unit axis from m_center1 to m_center2.
   */
  Vec3T<T> m_axis = Vec3T<T>(T(0), T(1), T(0));

  /**
   * @brief Unit direction to the center line at the first end.
   */
  Vec3T<T> m_start = Vec3T<T>(T(1), T(0), T(0));

  /**
   * @brief The helix radius.
   */
  T m_helixRadius = T(0.5);

  /**
   * @brief The number of turns.
   */
  T m_turns = T(4);

  /**
   * @brief The tube's radius.
   */
  T m_tubeRadius = T(0.1);
};

/**
 * @brief Signed distance field for a lattice of walls along a triply periodic minimal surface: a gyroid, Schwarz's
 * P or D (diamond) surface, or Neovius' surface. Porous media, heat exchangers, scaffolds.
 * @details The four share their code: each is the zero set of a sum of products of sines and cosines f of the point
 * scaled so that a cell is 2 pi, and the solid is the wall around it, |f| / g <= t / 2, with g the largest slope of f
 * in space: G times 2 pi over the smallest cell size, G the largest gradient of f in the scaled coordinates (sqrt(3)
 * for the gyroid, P and D, 7 for Neovius). The value is |f| / g - t / 2, a bound: it never changes faster than the
 * distance. The wall is t thick where f is steepest, and thicker elsewhere. The lattice fills all of space; cut it
 * to a part with an Intersection. By default it is a gyroid of unit cells with walls 0.1 thick.
 * @tparam T Floating-point precision.
 */
template <class T>
class LatticeSDF
{
  static_assert(std::is_floating_point_v<T>, "LatticeSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: never more than the Euclidean distance.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Bound;

  /**
   * @brief Which surface, in coordinates scaled so that a cell is 2 pi.
   */
  enum class Kind : uint32_t
  {
    Gyroid,   ///< sin x cos y + sin y cos z + sin z cos x.
    SchwarzP, ///< cos x + cos y + cos z.
    SchwarzD, ///< The diamond: sin x sin y sin z + sin x cos y cos z + cos x sin y cos z + cos x cos y sin z.
    Neovius   ///< 3 (cos x + cos y + cos z) + 4 cos x cos y cos z.
  };

  /**
   * @brief Default constructor. Constructs a gyroid of unit cells, with walls 0.1 thick.
   */
  LatticeSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_kind      Which surface.
   * @param[in] a_cellSize  The size of a cell along x, y and z, finite and positive.
   * @param[in] a_thickness The walls' thickness where they are thinnest, positive and less than the smallest cell
   * size.
   */
  LatticeSDF(const Kind a_kind, const Vec3T<T>& a_cellSize, const T a_thickness)
  {
    EBGEOMETRY_REQUIRE(AnalyticDetail::isFinite(a_cellSize) && a_cellSize[0] > T(0) && a_cellSize[1] > T(0) &&
                         a_cellSize[2] > T(0),
                       "LatticeSDF: the cell size must be finite and positive (%g, %g, %g)",
                       double(a_cellSize[0]),
                       double(a_cellSize[1]),
                       double(a_cellSize[2]));

    const T smallest = Math::min(a_cellSize[0], Math::min(a_cellSize[1], a_cellSize[2]));

    EBGEOMETRY_REQUIRE(std::isfinite(a_thickness) && a_thickness > T(0) && a_thickness < smallest,
                       "LatticeSDF: the thickness must be positive and less than the smallest cell size %g (%g)",
                       double(smallest),
                       double(a_thickness));
    EBGEOMETRY_REQUIRE(a_kind == Kind::Gyroid || a_kind == Kind::SchwarzP || a_kind == Kind::SchwarzD ||
                         a_kind == Kind::Neovius,
                       "LatticeSDF: unknown kind %u",
                       unsigned(a_kind));

    m_kind      = a_kind;
    m_cellSize  = a_cellSize;
    m_thickness = a_thickness;
    m_scale     = smallest / (T(2) * pi<T> * ((a_kind == Kind::Neovius) ? T(7) : std::sqrt(T(3))));
  }

  /**
   * @brief Copy constructor.
   */
  LatticeSDF(const LatticeSDF&) = default;

  /**
   * @brief Move constructor.
   */
  LatticeSDF(LatticeSDF&&) = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  LatticeSDF&
  operator=(const LatticeSDF&) = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  LatticeSDF&
  operator=(LatticeSDF&&) = default;

  /**
   * @brief Get which surface this is.
   * @return m_kind.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  Kind
  getKind() const noexcept
  {
    return m_kind;
  }

  /**
   * @brief Get the cell size.
   * @return m_cellSize.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const Vec3T<T>&
  getCellSize() const noexcept
  {
    return m_cellSize;
  }

  /**
   * @brief Get the walls' thickness.
   * @return m_thickness.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  getThickness() const noexcept
  {
    return m_thickness;
  }

  /**
   * @brief The surface's field f at a point, zero on the surface.
   * @param[in] a_point Position.
   * @return The field.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  field(const Vec3T<T>& a_point) const noexcept
  {
    const T twoPi = T(2) * pi<T>;

    const T x = twoPi * a_point[0] / m_cellSize[0];
    const T y = twoPi * a_point[1] / m_cellSize[1];
    const T z = twoPi * a_point[2] / m_cellSize[2];

    const T sx = std::sin(x);
    const T sy = std::sin(y);
    const T sz = std::sin(z);
    const T cx = std::cos(x);
    const T cy = std::cos(y);
    const T cz = std::cos(z);

    switch (m_kind) {
    case Kind::SchwarzP: {
      return cx + cy + cz;
    }
    case Kind::SchwarzD: {
      return sx * sy * sz + sx * cy * cz + cx * sy * cz + cx * cy * sz;
    }
    case Kind::Neovius: {
      return T(3) * (cx + cy + cz) + T(4) * cx * cy * cz;
    }
    default: {
      return sx * cy + sy * cz + sz * cx;
    }
    }
  }

  /**
   * @brief Signed distance function for the lattice: a bound.
   * @param[in] a_point Position.
   * @return A signed value whose magnitude never exceeds the distance from a_point to the surface; negative inside.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    return std::abs(this->field(a_point)) * m_scale - T(0.5) * m_thickness;
  }

  /**
   * @brief The shape's axis-aligned bounding box: unbounded.
   * @return The bounding box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    const T big = Math::Limits<T>::max();

    return BoundingVolumes::AABBT<T>(Vec3T<T>(-big, -big, -big), Vec3T<T>(big, big, big));
  }

protected:
  /**
   * @brief Which surface.
   */
  Kind m_kind = Kind::Gyroid;

  /**
   * @brief The cell size.
   */
  Vec3T<T> m_cellSize = Vec3T<T>::ones();

  /**
   * @brief The walls' thickness.
   */
  T m_thickness = T(0.1);

  /**
   * @brief One over the field's largest slope in space.
   */
  T m_scale = T(1) / (T(2) * pi<T> * T(1.7320508075688772));
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

static_assert(std::is_trivially_copyable_v<ConvexPolyhedronSDF<float>>,
              "ConvexPolyhedronSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<ConvexPolyhedronSDF<double>>,
              "ConvexPolyhedronSDF<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<BoxFrameSDF<float>>, "BoxFrameSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<BoxFrameSDF<double>>, "BoxFrameSDF<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<QuadricSDF<float>>, "QuadricSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<QuadricSDF<double>>, "QuadricSDF<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<SuperellipsoidSDF<float>>,
              "SuperellipsoidSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<SuperellipsoidSDF<double>>,
              "SuperellipsoidSDF<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<SolidAngleSDF<float>>, "SolidAngleSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<SolidAngleSDF<double>>, "SolidAngleSDF<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<VesicaSDF<float>>, "VesicaSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<VesicaSDF<double>>, "VesicaSDF<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<HelixSDF<float>>, "HelixSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<HelixSDF<double>>, "HelixSDF<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<LatticeSDF<float>>, "LatticeSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<LatticeSDF<double>>, "LatticeSDF<double> must be trivially copyable");

} // namespace EBGeometry

// Every translation unit that sees these shapes sees their ray forms too, so firstHit() on one of them
// is the same function everywhere.
#include "EBGeometry_AnalyticRayForms.hpp"

#endif
