// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_AnalyticDistanceFunctions.hpp
 * @brief  File containing various analytic signed distance fields.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_ANALYTICDISTANCEFUNCTIONS_HPP
#define EBGEOMETRY_ANALYTICDISTANCEFUNCTIONS_HPP

// Std includes
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <memory>
#include <random>
#include <type_traits>

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_Constants.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_SignedDistanceFunction.hpp"
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
class PlaneSDF : public SignedDistanceFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "PlaneSDF<T>: T must be a floating-point type");

public:
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
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));
    EBGEOMETRY_EXPECT(std::isfinite(a_normal[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_normal[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_normal[2]));
    EBGEOMETRY_EXPECT(a_normal.length() > T(0));

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
   * @brief Destructor.
   */
  ~PlaneSDF() override = default;

  /**
   * @brief Signed distance function for the plane.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the plane; negative on the side opposite to the normal.
   */
  [[nodiscard]] T
  signedDistance(const Vec3T<T>& a_point) const noexcept override
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));
    EBGEOMETRY_EXPECT(std::abs(m_normal.length() - T(1)) < std::sqrt(std::numeric_limits<T>::epsilon()));

    return dot((a_point - m_point), m_normal);
  }

protected:
  /**
   * @brief Point on plane.
   */
  Vec3T<T> m_point = Vec3T<T>::zero();

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
class SphereSDF : public SignedDistanceFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "SphereSDF<T>: T must be a floating-point type");

public:
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
    EBGEOMETRY_EXPECT(std::isfinite(a_center[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_center[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_center[2]));
    EBGEOMETRY_EXPECT(std::isfinite(a_radius));
    EBGEOMETRY_EXPECT(a_radius > T(0));

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
   * @brief Destructor.
   */
  ~SphereSDF() override = default;

  /**
   * @brief Get sphere center (const).
   * @return Const reference to the center position.
   */
  [[nodiscard]] const Vec3T<T>&
  getCenter() const noexcept
  {
    return m_center;
  }

  /**
   * @brief Get sphere center (mutable).
   * @return Mutable reference to the center position.
   */
  Vec3T<T>&
  getCenter() noexcept
  {
    return m_center;
  }

  /**
   * @brief Get sphere radius (const).
   * @return Const reference to the radius.
   */
  [[nodiscard]] const T&
  getRadius() const noexcept
  {
    return m_radius;
  }

  /**
   * @brief Get sphere radius (mutable).
   * @return Mutable reference to the radius.
   */
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
  [[nodiscard]] T
  signedDistance(const Vec3T<T>& a_point) const noexcept override
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    return (a_point - m_center).length() - m_radius;
  }

protected:
  /**
   * @brief Sphere center.
   */
  Vec3T<T> m_center = Vec3T<T>::zero();

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
class BoxSDF : public SignedDistanceFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "BoxSDF<T>: T must be a floating-point type");

public:
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
    EBGEOMETRY_EXPECT(std::isfinite(a_loCorner[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_loCorner[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_loCorner[2]));
    EBGEOMETRY_EXPECT(std::isfinite(a_hiCorner[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_hiCorner[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_hiCorner[2]));
    EBGEOMETRY_EXPECT(a_loCorner[0] < a_hiCorner[0]);
    EBGEOMETRY_EXPECT(a_loCorner[1] < a_hiCorner[1]);
    EBGEOMETRY_EXPECT(a_loCorner[2] < a_hiCorner[2]);

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
   * @brief Destructor.
   */
  ~BoxSDF() override = default;

  /**
   * @brief Get lower-left corner
   * @return m_loCorner
   */
  [[nodiscard]] const Vec3T<T>&
  getLowCorner() const noexcept
  {
    return m_loCorner;
  }

  /**
   * @brief Get lower-left corner
   * @return m_loCorner
   */
  Vec3T<T>&
  getLowCorner() noexcept
  {
    return m_loCorner;
  }

  /**
   * @brief Get upper-right corner
   * @return m_hiCorner
   */
  [[nodiscard]] const Vec3T<T>&
  getHighCorner() const noexcept
  {
    return m_hiCorner;
  }

  /**
   * @brief Get upper-right corner
   * @return m_hiCorner
   */
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
  [[nodiscard]] T
  signedDistance(const Vec3T<T>& a_point) const noexcept override
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    // For each coordinate direction, we have delta[dir] if a_point[dir] falls
    // between xLo and xHi. In this case delta[dir] will be the signed distance
    // to the closest box face in the dir-direction. Otherwise, if a_point[dir]
    // is outside the corner we have delta[dir] > 0.
    const Vec3T<T> delta(std::max(m_loCorner[0] - a_point[0], a_point[0] - m_hiCorner[0]),
                         std::max(m_loCorner[1] - a_point[1], a_point[1] - m_hiCorner[1]),
                         std::max(m_loCorner[2] - a_point[2], a_point[2] - m_hiCorner[2]));

    // Note: max is max(Vec3T<T>, Vec3T<T>) and not std::max. It returns a
    // vector with coordinate-wise largest components. Note that the first part
    // std::min(...) is the signed distance on the inside of the box (delta will
    // have negative components). The other part max(Vec3T<T>::zero(), ...) is
    // for outside the box.
    const T d = std::min(T(0.0), delta[delta.maxDir(false)]) + max(Vec3T<T>::zero(), delta).length();

    return d;
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
 * @details The torus ring lies in the xy-plane and is centred at `m_center`. The major radius
 * `m_majorRadius` is the distance from the torus centre to the centre-line of the tube; the minor
 * radius `m_minorRadius` is the tube radius. The bounding box spans
 * `[center ± (majorRadius + minorRadius)]` in x and y, and `[center ± minorRadius]` in z.
 * The SDF is negative inside the tube and positive outside. The minor radius must be strictly
 * less than the major radius; otherwise the tube would self-intersect at the torus centre.
 * By default the torus is centred at the origin with major radius 1 and minor radius 0.5.
 * @tparam T Floating-point precision.
 */
template <class T>
class TorusSDF : public SignedDistanceFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "TorusSDF<T>: T must be a floating-point type");

public:
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
    EBGEOMETRY_EXPECT(std::isfinite(a_center[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_center[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_center[2]));
    EBGEOMETRY_EXPECT(std::isfinite(a_majorRadius));
    EBGEOMETRY_EXPECT(std::isfinite(a_minorRadius));
    EBGEOMETRY_EXPECT(a_majorRadius > T(0));
    EBGEOMETRY_EXPECT(a_minorRadius > T(0));
    EBGEOMETRY_EXPECT(a_minorRadius < a_majorRadius);

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
   * @brief Destructor.
   */
  ~TorusSDF() override = default;

  /**
   * @brief Get torus center.
   * @return m_center
   */
  [[nodiscard]] const Vec3T<T>&
  getCenter() const noexcept
  {
    return m_center;
  }

  /**
   * @brief Get torus center.
   * @return m_center
   */
  Vec3T<T>&
  getCenter() noexcept
  {
    return m_center;
  }

  /**
   * @brief Get major radius.
   * @return m_majorRadius
   */
  [[nodiscard]] const T&
  getMajorRadius() const noexcept
  {
    return m_majorRadius;
  }

  /**
   * @brief Get major radius.
   * @return m_majorRadius
   */
  T&
  getMajorRadius() noexcept
  {
    return m_majorRadius;
  }

  /**
   * @brief Get minor radius.
   * @return m_minorRadius
   */
  [[nodiscard]] const T&
  getMinorRadius() const noexcept
  {
    return m_minorRadius;
  }

  /**
   * @brief Get minor radius.
   * @return m_minorRadius
   */
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
  [[nodiscard]] T
  signedDistance(const Vec3T<T>& a_point) const noexcept override
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    const Vec3T<T> p   = a_point - m_center;
    const T        rho = sqrt(p[0] * p[0] + p[1] * p[1]) - m_majorRadius;
    const T        d   = sqrt(rho * rho + p[2] * p[2]) - m_minorRadius;

    return d;
  }

protected:
  /**
   * @brief Torus center.
   */
  Vec3T<T> m_center = Vec3T<T>::zero();

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
 * @details The cylinder is defined by two cap-centre positions `m_center1` and `m_center2` and a
 * radius `m_radius`. The cylinder axis is the unit vector from `m_center1` to `m_center2`, and its
 * height (distance between the two flat caps) equals `distance(m_center1, m_center2)`. The SDF is
 * negative inside the cylinder and positive outside. By default the cylinder has radius 1 and
 * height 1, centred at the origin with its axis along y (cap centres at (0,-0.5,0) and (0,0.5,0)).
 * @tparam T Floating-point precision.
 */
template <class T>
class CylinderSDF : public SignedDistanceFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "CylinderSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief Default constructor. Constructs a unit cylinder of radius 1 and height 1, centred at the
   * origin with its axis along y (cap centres at (0,-0.5,0) and (0,0.5,0)).
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
    EBGEOMETRY_EXPECT(std::isfinite(a_center1[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_center1[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_center1[2]));
    EBGEOMETRY_EXPECT(std::isfinite(a_center2[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_center2[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_center2[2]));
    EBGEOMETRY_EXPECT(std::isfinite(a_radius));
    EBGEOMETRY_EXPECT(a_radius > T(0));
    EBGEOMETRY_EXPECT((a_center2 - a_center1).length() > T(0));

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
   * @brief Destructor.
   */
  ~CylinderSDF() override = default;

  /**
   * @brief Get one endpoint
   * @return m_center1
   */
  [[nodiscard]] const Vec3T<T>&
  getCenter1() const noexcept
  {
    return m_center1;
  }

  /**
   * @brief Get the other endpoint
   * @return m_center2
   */
  [[nodiscard]] const Vec3T<T>&
  getCenter2() const noexcept
  {
    return m_center2;
  }

  /**
   * @brief Get radius.
   * @return m_radius.
   */
  [[nodiscard]] const T&
  getRadius() const noexcept
  {
    return m_radius;
  }

  /**
   * @brief Signed distance function for the cylinder.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the cylinder surface; negative inside.
   */
  [[nodiscard]] T
  signedDistance(const Vec3T<T>& a_point) const noexcept override
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    T d = std::numeric_limits<T>::infinity();

    if (m_length > 0.0 && m_radius > 0.0) {
      EBGEOMETRY_EXPECT(std::abs(m_axis.length() - T(1)) < std::sqrt(std::numeric_limits<T>::epsilon()));

      const Vec3T<T> point = a_point - m_center;
      const T        para  = dot(point, m_axis);
      const Vec3T<T> ortho = point - para * m_axis; // Distance from cylinder axis.

      // w: Distance from cylinder wall. < 0 on inside and > 0 on outside.
      // h: Distance from cylinder top.  < 0 on inside and > 0 on outside.
      const T w = ortho.length() - m_radius;
      const T h = std::abs(para) - 0.5 * m_length;

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
        d = sqrt(w * w + h * h);
      }
    }

    return d;
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
  Vec3T<T> m_center = Vec3T<T>::zero();

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
 * 1 = y, 2 = z) and has a circular cross-section of radius `m_radius` centred at `m_center`
 * in the plane perpendicular to that axis. The SDF is the radial distance from the axis minus
 * the radius: negative inside the cylinder and positive outside. By default the cylinder has
 * radius 1, passes through the origin, and extends along y.
 * @tparam T Floating-point precision.
 */
template <class T>
class InfiniteCylinderSDF : public SignedDistanceFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "InfiniteCylinderSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief Default constructor. Constructs an infinite cylinder of radius 1 centred at the origin
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
    EBGEOMETRY_EXPECT(std::isfinite(a_center[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_center[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_center[2]));
    EBGEOMETRY_EXPECT(std::isfinite(a_radius));
    EBGEOMETRY_EXPECT(a_radius > T(0));
    EBGEOMETRY_EXPECT(a_axis < 3U);

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
   * @brief Destructor.
   */
  ~InfiniteCylinderSDF() override = default;

  /**
   * @brief Signed distance function for the infinite cylinder.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the cylinder surface; negative inside.
   */
  [[nodiscard]] T
  signedDistance(const Vec3T<T>& a_point) const noexcept override
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    Vec3T<T> delta = a_point - m_center;
    delta[m_axis]  = 0.0;

    const T d = delta.length() - m_radius;

    return d;
  }

protected:
  /**
   * @brief Center point on the cylinder axis.
   */
  Vec3T<T> m_center = Vec3T<T>::zero();

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
 * Internally the hemisphere centres are stored as `m_center1` and `m_center2` (inset from the
 * tips by `radius` along the capsule axis). These are **not** the same as the tip positions
 * passed to the constructor.
 *
 * The SDF is negative inside the capsule and positive outside. By default the capsule is
 * centred at the origin, aligned along y, with tips at (0,-1,0) and (0,1,0) and radius 0.5,
 * giving a total height of 2 and a cylindrical body of length 1.
 * @tparam T Floating-point precision.
 */
template <class T>
class CapsuleSDF : public SignedDistanceFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "CapsuleSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief Default constructor. Constructs a capsule with tips at (0,-1,0) and (0,1,0), radius 0.5.
   * @details Total height = 2, cylindrical body length = 1, aligned along y, centred at origin.
   * The stored hemisphere centres are at (0,-0.5,0) and (0,0.5,0).
   */
  CapsuleSDF() = default;

  /**
   * @brief Full constructor.
   * @details The hemisphere centres stored internally are derived from the tips:
   * `m_center1 = a_tip1 + a_radius * axis` and `m_center2 = a_tip2 - a_radius * axis`,
   * where `axis = (a_tip2 - a_tip1) / distance(a_tip1, a_tip2)`.
   * The cylindrical body length is `distance(a_tip1, a_tip2) - 2 * a_radius`; for a valid
   * non-degenerate shape this must be positive, i.e. `distance(a_tip1, a_tip2) > 2 * a_radius`.
   * @param[in] a_tip1   Outer tip of one hemispherical cap (outermost point in that direction).
   * @param[in] a_tip2   Outer tip of the other hemispherical cap.
   * @param[in] a_radius Capsule radius (applied to both the tube and the hemispherical caps).
   */
  CapsuleSDF(const Vec3T<T>& a_tip1, const Vec3T<T>& a_tip2, const T& a_radius) noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_tip1[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_tip1[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_tip1[2]));
    EBGEOMETRY_EXPECT(std::isfinite(a_tip2[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_tip2[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_tip2[2]));
    EBGEOMETRY_EXPECT(std::isfinite(a_radius));
    EBGEOMETRY_EXPECT(a_radius > T(0));
    EBGEOMETRY_EXPECT((a_tip2 - a_tip1).length() > T(0));

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
   * @brief Destructor.
   */
  ~CapsuleSDF() override = default;

  /**
   * @brief Signed distance function for the capsule.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the capsule surface; negative inside.
   */
  [[nodiscard]] T
  signedDistance(const Vec3T<T>& a_point) const noexcept override
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));
    EBGEOMETRY_EXPECT((m_center2 - m_center1).length() > T(0));

    const Vec3T<T> v1 = a_point - m_center1;
    const Vec3T<T> v2 = m_center2 - m_center1;

    const T h = std::clamp(dot(v1, v2) / dot(v2, v2), T(0.0), T(1.0));
    const T d = length(v1 - h * v2) - m_radius;

    return d;
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
 * @details The cone tip is at `m_tip`. The cone body opens in the **-z** direction from the tip
 * (the interior, where SDF < 0, is the solid region extending downward along -z within the
 * cone surface). The full opening angle `a_angle` (tip-to-tip across the cone) is halved
 * internally; the half-angle is stored as `m_c = (sin(half_angle), cos(half_angle))`.
 *
 * For a 45° full opening angle the half-angle is 22.5°; a point directly below the tip at
 * depth `d` sits on the cone surface when its radial distance from the axis equals
 * `d * tan(half_angle)`.
 *
 * The cone is infinite: it has no base plane. By default the tip is at the origin and the full
 * opening angle is 45°, with the body extending along -z.
 * @tparam T Floating-point precision.
 */
template <class T>
class InfiniteConeSDF : public SignedDistanceFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "InfiniteConeSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief Default constructor. Constructs an infinite cone with tip at the origin, 45° full
   * opening angle, and body extending in the -z direction.
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
    EBGEOMETRY_EXPECT(std::isfinite(a_tip[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_tip[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_tip[2]));
    EBGEOMETRY_EXPECT(std::isfinite(a_angle));
    EBGEOMETRY_EXPECT(a_angle > T(0));
    EBGEOMETRY_EXPECT(a_angle < T(180));

    m_tip = a_tip;
    m_c.x = std::sin(T(0.5) * a_angle * pi<T> / T(180));
    m_c.y = std::cos(T(0.5) * a_angle * pi<T> / T(180));
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
   * @brief Destructor.
   */
  ~InfiniteConeSDF() override = default;

  /**
   * @brief Signed distance function for the infinite cone.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the cone surface; negative inside the cone.
   */
  [[nodiscard]] T
  signedDistance(const Vec3T<T>& a_point) const noexcept override
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));
    EBGEOMETRY_EXPECT(std::abs(length(m_c) - T(1)) < std::sqrt(std::numeric_limits<T>::epsilon()));

    const Vec3T<T> delta = a_point - m_tip;
    const Vec2T<T> q(sqrt(delta[0] * delta[0] + delta[1] * delta[1]), -delta[2]);

    const T d1 = length(q - m_c * std::max(dot(q, m_c), T(0.0)));
    const T d2 = d1 * ((q.x * m_c.y - q.y * m_c.x < 0.0) ? -1.0 : 1.0);

    return d2;
  }

protected:
  /**
   * @brief Tip position.
   */
  Vec3T<T> m_tip = Vec3T<T>::zero();

  /**
   * @brief (sin, cos) of the half opening-angle. Default: 45° full angle → half-angle 22.5°.
   */
  Vec2T<T> m_c = Vec2T<T>(std::sin(pi<T> / T(8)), std::cos(pi<T> / T(8)));
};

/**
 * @brief Signed distance field for a finite cone.
 * @details The cone tip is at `m_tip` and the body opens in the **-z** direction, so the base
 * disc lies at `m_tip + (0, 0, -m_height)`. The full opening angle `a_angle` is the tip-to-tip
 * apex angle; internally the half-angle is encoded as `m_c = (sin(half_angle), cos(half_angle))`.
 * The base radius equals `m_height * tan(half_angle) = m_height * m_c.x / m_c.y`.
 *
 * The SDF is negative inside the solid cone (including the base disc) and positive outside.
 * By default the tip is at the origin, the height is 1, and the full opening angle is 45°,
 * giving a base radius of `tan(22.5°) ≈ 0.414`.
 * @tparam T Floating-point precision.
 */
template <class T>
class ConeSDF : public SignedDistanceFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "ConeSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief Default constructor. Constructs a finite cone with its tip at the origin, height 1,
   * and a 45-degree opening angle, body extending in the -z direction.
   */
  ConeSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_tip    Cone tip position.
   * @param[in] a_height Cone height (tip to base).
   * @param[in] a_angle  Full opening angle in degrees.
   */
  ConeSDF(const Vec3T<T>& a_tip, const T& a_height, const T& a_angle) noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_tip[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_tip[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_tip[2]));
    EBGEOMETRY_EXPECT(std::isfinite(a_height));
    EBGEOMETRY_EXPECT(std::isfinite(a_angle));
    EBGEOMETRY_EXPECT(a_height > T(0));
    EBGEOMETRY_EXPECT(a_angle > T(0));
    EBGEOMETRY_EXPECT(a_angle < T(180));

    m_tip    = a_tip;
    m_height = a_height;
    m_c.x    = std::sin(T(0.5) * a_angle * pi<T> / T(180));
    m_c.y    = std::cos(T(0.5) * a_angle * pi<T> / T(180));
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
   * @brief Destructor.
   */
  ~ConeSDF() override = default;

  /**
   * @brief Signed distance function for the finite cone.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the cone surface; negative inside the cone.
   */
  [[nodiscard]] T
  signedDistance(const Vec3T<T>& a_point) const noexcept override
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));
    EBGEOMETRY_EXPECT(std::abs(length(m_c) - T(1)) < std::sqrt(std::numeric_limits<T>::epsilon()));
    EBGEOMETRY_EXPECT(m_c.y > T(0));

    const Vec3T<T> delta = a_point - m_tip;
    const T        dr    = sqrt(delta[0] * delta[0] + delta[1] * delta[1]);
    const T        dz    = delta[2];

    constexpr T zero = T(0.0);
    constexpr T one  = T(1.0);

    const Vec2T<T> q = m_height * Vec2T<T>(m_c.x / m_c.y, -1.0);
    const Vec2T<T> w = Vec2T<T>(dr, dz);
    const Vec2T<T> a = w - std::clamp(dot(w, q) / dot(q, q), zero, one) * q;
    const Vec2T<T> b = w - Vec2T<T>(q.x * std::clamp(w.x / q.x, zero, one), q.y);

    auto sign = [](const T& x) -> int { return (x > zero) - (x < zero); };

    const T k = sign(q.y);
    const T d = std::min(dot(a, a), dot(b, b));
    const T s = std::max(k * (w.x * q.y - w.y * q.x), k * (w.y - q.y));

    return sqrt(d) * sign(s);
  }

protected:
  /**
   * @brief Tip position.
   */
  Vec3T<T> m_tip = Vec3T<T>::zero();

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
 * @brief Signed distance field for an axis-aligned box with rounded corners.
 * @details The box is centred at the origin. The constructor takes the full side lengths along
 * each axis and a corner curvature radius. Internally, half-extents `m_dimensions = 0.5 *
 * a_dimensions` are stored. The actual bounding half-extent in each direction is
 * `m_dimensions[i] + curvature`, so the total bounding box has full side lengths
 * `a_dimensions[i] + 2 * curvature`. The corners are rounded by spheres of radius `curvature`.
 * The SDF is negative inside and positive outside.
 * @tparam T Floating-point precision.
 */
template <class T>
class RoundedBoxSDF : public SignedDistanceFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "RoundedBoxSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief Default constructor. Constructs a rounded unit cube centered at the origin
   * ([-0.5, 0.5]^3 before rounding) with corner curvature 0.1.
   * @details Total half-extent in each direction is 0.5 + 0.1 = 0.6.
   */
  RoundedBoxSDF() = default;

  /**
   * @brief Full constructor.
   * @details The total half-extent in direction @p i is @c 0.5*a_dimensions[i] + a_curvature.
   * @param[in] a_dimensions Box dimensions (full side lengths before rounding).
   * @param[in] a_curvature  Corner sphere radius. Must be > 0.
   */
  RoundedBoxSDF(const Vec3T<T>& a_dimensions, const T a_curvature) noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_dimensions[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_dimensions[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_dimensions[2]));
    EBGEOMETRY_EXPECT(std::isfinite(a_curvature));
    EBGEOMETRY_EXPECT(a_dimensions[0] > T(0));
    EBGEOMETRY_EXPECT(a_dimensions[1] > T(0));
    EBGEOMETRY_EXPECT(a_dimensions[2] > T(0));
    EBGEOMETRY_EXPECT(a_curvature > T(0));

    m_dimensions = T(0.5) * a_dimensions;
    m_sphere     = std::make_shared<SphereSDF<T>>(Vec3T<T>::zero(), a_curvature);
  }

  /**
   * @brief Copy constructor. Shares the internal sphere object.
   */
  RoundedBoxSDF(const RoundedBoxSDF&) = default;

  /**
   * @brief Move constructor.
   */
  RoundedBoxSDF(RoundedBoxSDF&&) = default;

  /**
   * @brief Copy assignment. Shares the internal sphere object.
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
   * @brief Destructor.
   */
  ~RoundedBoxSDF() override = default;

  /**
   * @brief Signed distance function for the rounded box.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the rounded-box surface; negative inside.
   */
  [[nodiscard]] T
  signedDistance(const Vec3T<T>& a_point) const noexcept override
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    return m_sphere->signedDistance(a_point - clamp(a_point, -m_dimensions, m_dimensions));
  }

protected:
  /**
   * @brief Sphere of radius = curvature used to round the corners.
   */
  std::shared_ptr<SphereSDF<T>> m_sphere = std::make_shared<SphereSDF<T>>(Vec3T<T>::zero(), T(0.1));

  /**
   * @brief Half-extents of the inner box (= 0.5 * the user-supplied dimensions).
   */
  Vec3T<T> m_dimensions = Vec3T<T>(T(0.5), T(0.5), T(0.5));
};

/**
 * @brief Ken Perlin's gradient-noise implicit function.
 * @details This class evaluates multi-octave Perlin noise at a 3-D point and returns the result
 * as a scalar. It is **not** a true signed distance function (the gradient magnitude is not
 * guaranteed to equal 1), but it can serve as a procedural displacement field when added to or
 * subtracted from a conventional SDF.
 *
 * The output of `signedDistance` is on the range `[0, m_noiseAmplitude]` for a single octave.
 * Multiple octaves sum scaled contributions: each successive octave doubles the spatial frequency
 * (controlled by `m_noisePersistence` dividing the frequency) and attenuates the amplitude by
 * `m_noisePersistence`.
 *
 * @note The default constructor leaves the permutation table all-zeros, producing constant-zero
 * noise. Use the full constructor (which initialises the table from Ken Perlin's reference
 * permutation and then shuffles it) or call `shuffle()` explicitly before use.
 * @tparam T Floating-point precision.
 */
template <class T>
class PerlinSDF : public SignedDistanceFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "PerlinSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief Default constructor. Constructs a single-octave Perlin noise field with unit amplitude,
   * unit frequency along all axes, and persistence 0.5.
   */
  PerlinSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_noiseAmplitude   Noise amplitude (output scale).
   * @param[in] a_noiseFrequency   Spatial frequency along each Cartesian axis.
   * @param[in] a_noisePersistence Per-octave amplitude decay factor. Clamped to [0, 1].
   * @param[in] a_noiseOctaves     Number of noise octaves. Clamped to >= 1.
   */
  PerlinSDF(const T            a_noiseAmplitude,
            const Vec3T<T>     a_noiseFrequency,
            const T            a_noisePersistence,
            const unsigned int a_noiseOctaves) noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_noiseAmplitude));
    EBGEOMETRY_EXPECT(std::isfinite(a_noiseFrequency[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_noiseFrequency[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_noiseFrequency[2]));
    EBGEOMETRY_EXPECT(std::isfinite(a_noisePersistence));

    m_noiseAmplitude   = a_noiseAmplitude;
    m_noiseFrequency   = a_noiseFrequency;
    m_noisePersistence = std::min(T(1), a_noisePersistence);
    m_noiseOctaves     = std::max(1U, a_noiseOctaves);

    for (int i = 0; i < 256; i++) {
      m_permutationTable[i]       = s_perlinPermutationTable[i];
      m_permutationTable[i + 256] = s_perlinPermutationTable[i];
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
   * @brief Destructor.
   */
  ~PerlinSDF() override = default;

  /**
   * @brief Signed distance function. Generates a noise value on [0, m_noiseAmplitude].
   * @param[in] a_point Input point.
   * @return Octave-summed Perlin noise value scaled by m_noiseAmplitude.
   */
  [[nodiscard]] T
  signedDistance(const Vec3T<T>& a_point) const noexcept override
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    T ret = 0.0;

    Vec3T<T> curFreq = m_noiseFrequency;
    T        curAmp  = 1.0;

    for (unsigned int curOctave = 0; curOctave < m_noiseOctaves; curOctave++) {
      ret += 0.5 * curAmp * (1 + this->noise(a_point * curFreq));

      curFreq = curFreq / m_noisePersistence;
      curAmp  = curAmp * m_noisePersistence;
    }

    return ret * m_noiseAmplitude;
  };

  /**
   * @brief Shuffle the permutation table with the provided uniform random number generator.
   * @details Reseeds the 256-entry permutation table using std::shuffle and then mirrors it
   * into the upper 256 entries. The URNG type must satisfy the C++ UniformRandomBitGenerator
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
  std::array<int, 512>&
  getPermutationTable() noexcept
  {
    return m_permutationTable;
  }

protected:
  /**
   * @brief Ken Perlin's original 256-entry permutation table.
   * @details Used by the full constructor and shuffle() to seed m_permutationTable. Stored as a
   * static constexpr member so it is available to subclasses without polluting the enclosing
   * namespace.
   */
  static constexpr std::array<int, 256> s_perlinPermutationTable = {
    151, 160, 137, 91,  90,  15,  131, 13,  201, 95,  96,  53,  194, 233, 7,   225, 140, 36,  103, 30,  69,  142,
    8,   99,  37,  240, 21,  10,  23,  190, 6,   148, 247, 120, 234, 75,  0,   26,  197, 62,  94,  252, 219, 203,
    117, 35,  11,  32,  57,  177, 33,  88,  237, 149, 56,  87,  174, 20,  125, 136, 171, 168, 68,  175, 74,  165,
    71,  134, 139, 48,  27,  166, 77,  146, 158, 231, 83,  111, 229, 122, 60,  211, 133, 230, 220, 105, 92,  41,
    55,  46,  245, 40,  244, 102, 143, 54,  65,  25,  63,  161, 1,   216, 80,  73,  209, 76,  132, 187, 208, 89,
    18,  169, 200, 196, 135, 130, 116, 188, 159, 86,  164, 100, 109, 198, 173, 186, 3,   64,  52,  217, 226, 250,
    124, 123, 5,   202, 38,  147, 118, 126, 255, 82,  85,  212, 207, 206, 59,  227, 47,  16,  58,  17,  182, 189,
    28,  42,  223, 183, 170, 213, 119, 248, 152, 2,   44,  154, 163, 70,  221, 153, 101, 155, 167, 43,  172, 9,
    129, 22,  39,  253, 19,  98,  108, 110, 79,  113, 224, 232, 178, 185, 112, 104, 218, 246, 97,  228, 251, 34,
    242, 193, 238, 210, 144, 12,  191, 179, 162, 241, 81,  51,  145, 235, 249, 14,  239, 107, 49,  192, 214, 31,
    181, 199, 106, 157, 184, 84,  204, 176, 115, 121, 50,  45,  127, 4,   150, 254, 138, 236, 205, 93,  222, 114,
    67,  29,  24,  72,  243, 141, 128, 195, 78,  66,  215, 61,  156, 180};

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
   * @details Default-constructed with all zeros (degenerate, constant-zero noise).
   * Use the full constructor or shuffle() to populate with a real permutation.
   */
  std::array<int, 512> m_permutationTable = {};

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
  [[nodiscard]] virtual T
  lerp(const T t, const T a, const T b) const noexcept
  {
    return a + t * (b - a);
  };

  /**
   * @brief Ken Perlin's fade (smoothstep) function: 6t^5 - 15t^4 + 10t^3.
   * @param[in] t Input in [0,1].
   * @return Smoothed value in [0,1].
   */
  [[nodiscard]] virtual T
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
  [[nodiscard]] T
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
  [[nodiscard]] T
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
 * @details The cylinder is centred at the origin with its axis along **y**. The constructor
 * takes the outer radius, the total height, and an edge curvature radius. The curvature rounds
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
class RoundedCylinderSDF : public SignedDistanceFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "RoundedCylinderSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief Default constructor. Constructs a rounded cylinder centred at the origin, axis along y,
   * with outer radius 1, total height 1, and edge curvature 0.1.
   */
  RoundedCylinderSDF() = default;

  /**
   * @brief Full constructor.
   * @param[in] a_radius    Outer cylinder radius. Must be > 0 and > a_curvature.
   * @param[in] a_curvature Edge rounding radius. Must be > 0 and satisfy
   * a_curvature < a_radius and 2 * a_curvature < a_height.
   * @param[in] a_height    Total cylinder height. Must be > 0 and > 2 * a_curvature.
   */
  RoundedCylinderSDF(const T a_radius, const T a_curvature, const T a_height) noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_radius));
    EBGEOMETRY_EXPECT(std::isfinite(a_curvature));
    EBGEOMETRY_EXPECT(std::isfinite(a_height));
    EBGEOMETRY_EXPECT(a_radius > T(0));
    EBGEOMETRY_EXPECT(a_curvature > T(0));
    EBGEOMETRY_EXPECT(a_height > T(0));
    EBGEOMETRY_EXPECT(a_curvature < a_radius);
    EBGEOMETRY_EXPECT(T(2) * a_curvature < a_height);

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
   * @brief Destructor.
   */
  ~RoundedCylinderSDF() override = default;

  /**
   * @brief Signed distance function for the rounded cylinder.
   * @param[in] a_point Position.
   * @return Signed distance from a_point to the rounded-cylinder surface; negative inside.
   */
  [[nodiscard]] T
  signedDistance(const Vec3T<T>& a_point) const noexcept override
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    const T    xz = sqrt(a_point[2] * a_point[2] + a_point[0] * a_point[0]);
    const auto d1 = Vec2T<T>(xz - m_majorRadius, std::abs(a_point[1]) - m_height);
    const auto d2 = Vec2T<T>(std::max(d1.x, T(0)), std::max(d1.y, T(0)));

    return std::min(std::max(d1.x, d1.y), T(0)) + sqrt(d2.x * d2.x + d2.y * d2.y) - m_minorRadius;
  }

protected:
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

} // namespace EBGeometry

#endif
