// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_AnalyticDistanceFunctions.hpp
 * @brief  File containing various analytic signed distance fields.
 * @details Each analytic primitive is expressed as a distance-formula trait (an @c XxxOp struct with
 * a nested @c Params bundle and a single-source-of-truth @c eval) and a thin named wrapper class that
 * derives from @c ImplicitFunction<T, XxxOp<T>>. The trait's @c eval holds the distance math in
 * exactly one place; the wrapper only provides friendly constructors and accessors.
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
#include <random>
#include <type_traits>

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_Constants.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_ImplicitFunction.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Distance-formula trait for a plane.
 * @details The SDF evaluates to `dot(a_point - point, normal)`. `normal` must be unit length for the
 * SDF to return a true distance (the PlaneSDF wrapper normalizes on construction).
 * @tparam T Floating-point precision.
 */
template <class T>
struct PlaneOp
{
  /**
   * @brief Stored parameters for a plane.
   */
  struct Params
  {
    Vec3T<T> point  = Vec3T<T>::zeros();          ///< Point on plane.
    Vec3T<T> normal = Vec3T<T>(T(0), T(1), T(0)); ///< Plane normal (unit length).
  };

  /**
   * @brief All planes are true signed distance functions.
   */
  static constexpr bool isSignedDistance = true;

  /**
   * @brief Signed distance to the plane.
   * @param[in] a_params Plane parameters.
   * @param[in] a_point  Query point.
   * @return Signed distance; negative on the side opposite to the normal.
   */
  EBGEOMETRY_HOST_DEVICE static T
  eval(const Params& a_params, const Vec3T<T>& a_point) noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));
    EBGEOMETRY_EXPECT(std::abs(a_params.normal.length() - T(1)) < std::sqrt(std::numeric_limits<T>::epsilon()));

    return dot((a_point - a_params.point), a_params.normal);
  }
};

/**
 * @brief Signed distance function for a plane.
 * @details Thin wrapper around PlaneOp. By default the plane is the y = 0 plane with normal (0,1,0).
 * @tparam T Floating-point precision.
 */
template <class T>
class PlaneSDF : public ImplicitFunction<T, PlaneOp<T>>
{
  static_assert(std::is_floating_point_v<T>, "PlaneSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief Default constructor. Constructs the y = 0 plane with outward normal (0, 1, 0).
   */
  PlaneSDF() noexcept : ImplicitFunction<T, PlaneOp<T>>(typename PlaneOp<T>::Params{})
  {}

  /**
   * @brief Full constructor.
   * @param[in] a_point  Any point on the plane.
   * @param[in] a_normal Plane normal vector (need not be unit length; it is normalized internally).
   */
  PlaneSDF(const Vec3T<T>& a_point, const Vec3T<T>& a_normal) noexcept
    : ImplicitFunction<T, PlaneOp<T>>(typename PlaneOp<T>::Params{a_point, a_normal / a_normal.length()})
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));
    EBGEOMETRY_EXPECT(std::isfinite(a_normal[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_normal[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_normal[2]));
    EBGEOMETRY_EXPECT(a_normal.length() > T(0));
  }
};

/**
 * @brief Distance-formula trait for a sphere.
 * @tparam T Floating-point precision.
 */
template <class T>
struct SphereOp
{
  /**
   * @brief Stored parameters for a sphere.
   */
  struct Params
  {
    Vec3T<T> center = Vec3T<T>::zeros(); ///< Sphere center.
    T        radius = T(1);              ///< Sphere radius.
  };

  /**
   * @brief All spheres are true signed distance functions.
   */
  static constexpr bool isSignedDistance = true;

  /**
   * @brief Signed distance to the sphere.
   * @param[in] a_params Sphere parameters.
   * @param[in] a_point  Query point.
   * @return Signed distance; negative inside the sphere.
   */
  EBGEOMETRY_HOST_DEVICE static T
  eval(const Params& a_params, const Vec3T<T>& a_point) noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    return (a_point - a_params.center).length() - a_params.radius;
  }
};

/**
 * @brief Signed distance field for a sphere.
 * @details Thin wrapper around SphereOp. By default the sphere is centered at the origin with radius 1.
 * @tparam T Floating-point precision.
 */
template <class T>
class SphereSDF : public ImplicitFunction<T, SphereOp<T>>
{
  static_assert(std::is_floating_point_v<T>, "SphereSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief Default constructor. Constructs a unit sphere centered at the origin.
   */
  SphereSDF() noexcept : ImplicitFunction<T, SphereOp<T>>(typename SphereOp<T>::Params{})
  {}

  /**
   * @brief Full constructor.
   * @param[in] a_center Sphere center.
   * @param[in] a_radius Sphere radius.
   */
  SphereSDF(const Vec3T<T>& a_center, const T& a_radius) noexcept
    : ImplicitFunction<T, SphereOp<T>>(typename SphereOp<T>::Params{a_center, a_radius})
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_center[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_center[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_center[2]));
    EBGEOMETRY_EXPECT(std::isfinite(a_radius));
    EBGEOMETRY_EXPECT(a_radius > T(0));
  }

  /**
   * @brief Get sphere center (const).
   * @return Const reference to the center position.
   */
  [[nodiscard]] const Vec3T<T>&
  getCenter() const noexcept
  {
    return this->m_params.center;
  }

  /**
   * @brief Get sphere center (mutable).
   * @return Mutable reference to the center position.
   */
  Vec3T<T>&
  getCenter() noexcept
  {
    return this->m_params.center;
  }

  /**
   * @brief Get sphere radius (const).
   * @return Const reference to the radius.
   */
  [[nodiscard]] const T&
  getRadius() const noexcept
  {
    return this->m_params.radius;
  }

  /**
   * @brief Get sphere radius (mutable).
   * @return Mutable reference to the radius.
   */
  T&
  getRadius() noexcept
  {
    return this->m_params.radius;
  }
};

/**
 * @brief Distance-formula trait for an axis-aligned box (AABB).
 * @tparam T Floating-point precision.
 */
template <class T>
struct BoxOp
{
  /**
   * @brief Stored parameters for an axis-aligned box.
   */
  struct Params
  {
    Vec3T<T> loCorner = Vec3T<T>(T(-0.5), T(-0.5), T(-0.5)); ///< Low box corner.
    Vec3T<T> hiCorner = Vec3T<T>(T(0.5), T(0.5), T(0.5));    ///< High box corner.
  };

  /**
   * @brief All boxes are true signed distance functions.
   */
  static constexpr bool isSignedDistance = true;

  /**
   * @brief Signed distance to the axis-aligned box.
   * @param[in] a_params Box parameters.
   * @param[in] a_point  Query point.
   * @return Signed distance; negative inside the box.
   */
  EBGEOMETRY_HOST_DEVICE static T
  eval(const Params& a_params, const Vec3T<T>& a_point) noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    // For each coordinate direction, we have delta[dir] if a_point[dir] falls
    // between xLo and xHi. In this case delta[dir] will be the signed distance
    // to the closest box face in the dir-direction. Otherwise, if a_point[dir]
    // is outside the corner we have delta[dir] > 0.
    const Vec3T<T> delta(std::max(a_params.loCorner[0] - a_point[0], a_point[0] - a_params.hiCorner[0]),
                         std::max(a_params.loCorner[1] - a_point[1], a_point[1] - a_params.hiCorner[1]),
                         std::max(a_params.loCorner[2] - a_point[2], a_point[2] - a_params.hiCorner[2]));

    // Note: max is max(Vec3T<T>, Vec3T<T>) and not std::max. It returns a
    // vector with coordinate-wise largest components. Note that the first part
    // std::min(...) is the signed distance on the inside of the box (delta will
    // have negative components). The other part max(Vec3T<T>::zeros(), ...) is
    // for outside the box.
    const T d = std::min(T(0.0), delta[delta.maxDir(false)]) + max(Vec3T<T>::zeros(), delta).length();

    return d;
  }
};

/**
 * @brief Signed distance field for an axis-aligned box (AABB).
 * @details Thin wrapper around BoxOp. By default the box is the unit cube `[-0.5, 0.5]^3`.
 * @tparam T Floating-point precision.
 */
template <class T>
class BoxSDF : public ImplicitFunction<T, BoxOp<T>>
{
  static_assert(std::is_floating_point_v<T>, "BoxSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief Default constructor. Constructs a unit cube centered at the origin: [-0.5, 0.5]^3.
   */
  BoxSDF() noexcept : ImplicitFunction<T, BoxOp<T>>(typename BoxOp<T>::Params{})
  {}

  /**
   * @brief Full constructor.
   * @param[in] a_loCorner Lower corner (minimum x, y, z).
   * @param[in] a_hiCorner Upper corner (maximum x, y, z). Each component must be strictly greater
   * than the corresponding component of a_loCorner.
   */
  BoxSDF(const Vec3T<T>& a_loCorner, const Vec3T<T>& a_hiCorner) noexcept
    : ImplicitFunction<T, BoxOp<T>>(typename BoxOp<T>::Params{a_loCorner, a_hiCorner})
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
  }

  /**
   * @brief Get lower-left corner
   * @return Low corner.
   */
  [[nodiscard]] const Vec3T<T>&
  getLowCorner() const noexcept
  {
    return this->m_params.loCorner;
  }

  /**
   * @brief Get lower-left corner
   * @return Low corner.
   */
  Vec3T<T>&
  getLowCorner() noexcept
  {
    return this->m_params.loCorner;
  }

  /**
   * @brief Get upper-right corner
   * @return High corner.
   */
  [[nodiscard]] const Vec3T<T>&
  getHighCorner() const noexcept
  {
    return this->m_params.hiCorner;
  }

  /**
   * @brief Get upper-right corner
   * @return High corner.
   */
  Vec3T<T>&
  getHighCorner() noexcept
  {
    return this->m_params.hiCorner;
  }
};

/**
 * @brief Distance-formula trait for a torus.
 * @details The torus ring lies in the xy-plane and is centred at `center`.
 * @tparam T Floating-point precision.
 */
template <class T>
struct TorusOp
{
  /**
   * @brief Stored parameters for a torus.
   */
  struct Params
  {
    Vec3T<T> center      = Vec3T<T>::zeros(); ///< Torus center.
    T        majorRadius = T(1);              ///< Major (ring) radius.
    T        minorRadius = T(0.5);            ///< Minor (tube) radius.
  };

  /**
   * @brief All tori are true signed distance functions.
   */
  static constexpr bool isSignedDistance = true;

  /**
   * @brief Signed distance to the torus.
   * @param[in] a_params Torus parameters.
   * @param[in] a_point  Query point.
   * @return Signed distance; negative inside the torus tube.
   */
  EBGEOMETRY_HOST_DEVICE static T
  eval(const Params& a_params, const Vec3T<T>& a_point) noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    const Vec3T<T> p   = a_point - a_params.center;
    const T        rho = std::sqrt(p[0] * p[0] + p[1] * p[1]) - a_params.majorRadius;
    const T        d   = std::sqrt(rho * rho + p[2] * p[2]) - a_params.minorRadius;

    return d;
  }
};

/**
 * @brief Signed distance field for a torus.
 * @details Thin wrapper around TorusOp. By default the torus is centred at the origin with major
 * radius 1 and minor radius 0.5.
 * @tparam T Floating-point precision.
 */
template <class T>
class TorusSDF : public ImplicitFunction<T, TorusOp<T>>
{
  static_assert(std::is_floating_point_v<T>, "TorusSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief Default constructor. Constructs a torus centered at the origin with major radius 1 and minor radius 0.5.
   */
  TorusSDF() noexcept : ImplicitFunction<T, TorusOp<T>>(typename TorusOp<T>::Params{})
  {}

  /**
   * @brief Full constructor.
   * @param[in] a_center      Torus center.
   * @param[in] a_majorRadius Major (ring) radius.
   * @param[in] a_minorRadius Minor (tube) radius.
   */
  TorusSDF(const Vec3T<T>& a_center, const T& a_majorRadius, const T& a_minorRadius) noexcept
    : ImplicitFunction<T, TorusOp<T>>(typename TorusOp<T>::Params{a_center, a_majorRadius, a_minorRadius})
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_center[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_center[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_center[2]));
    EBGEOMETRY_EXPECT(std::isfinite(a_majorRadius));
    EBGEOMETRY_EXPECT(std::isfinite(a_minorRadius));
    EBGEOMETRY_EXPECT(a_majorRadius > T(0));
    EBGEOMETRY_EXPECT(a_minorRadius > T(0));
    EBGEOMETRY_EXPECT(a_minorRadius < a_majorRadius);
  }

  /**
   * @brief Get torus center.
   * @return Torus center.
   */
  [[nodiscard]] const Vec3T<T>&
  getCenter() const noexcept
  {
    return this->m_params.center;
  }

  /**
   * @brief Get torus center.
   * @return Torus center.
   */
  Vec3T<T>&
  getCenter() noexcept
  {
    return this->m_params.center;
  }

  /**
   * @brief Get major radius.
   * @return Major radius.
   */
  [[nodiscard]] const T&
  getMajorRadius() const noexcept
  {
    return this->m_params.majorRadius;
  }

  /**
   * @brief Get major radius.
   * @return Major radius.
   */
  T&
  getMajorRadius() noexcept
  {
    return this->m_params.majorRadius;
  }

  /**
   * @brief Get minor radius.
   * @return Minor radius.
   */
  [[nodiscard]] const T&
  getMinorRadius() const noexcept
  {
    return this->m_params.minorRadius;
  }

  /**
   * @brief Get minor radius.
   * @return Minor radius.
   */
  T&
  getMinorRadius() noexcept
  {
    return this->m_params.minorRadius;
  }
};

/**
 * @brief Distance-formula trait for a finite, flat-capped cylinder.
 * @details The cylinder axis is the unit vector from `center1` to `center2`; `center`, `axis`, and
 * `length` are precomputed by the CylinderSDF wrapper.
 * @tparam T Floating-point precision.
 */
template <class T>
struct CylinderOp
{
  /**
   * @brief Stored parameters for a finite cylinder.
   */
  struct Params
  {
    Vec3T<T> center1 = Vec3T<T>(T(0), T(-0.5), T(0)); ///< One endpoint (cap center).
    Vec3T<T> center2 = Vec3T<T>(T(0), T(0.5), T(0));  ///< Other endpoint (cap center).
    Vec3T<T> center  = Vec3T<T>::zeros();             ///< Midpoint of the two endpoints.
    Vec3T<T> axis    = Vec3T<T>(T(0), T(1), T(0));    ///< Unit axis from center1 to center2.
    T        length  = T(1);                          ///< Distance between the two endpoints.
    T        radius  = T(1);                          ///< Cylinder radius.
  };

  /**
   * @brief All cylinders are true signed distance functions.
   */
  static constexpr bool isSignedDistance = true;

  /**
   * @brief Signed distance to the cylinder.
   * @param[in] a_params Cylinder parameters.
   * @param[in] a_point  Query point.
   * @return Signed distance; negative inside.
   */
  EBGEOMETRY_HOST_DEVICE static T
  eval(const Params& a_params, const Vec3T<T>& a_point) noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    T d = std::numeric_limits<T>::infinity();

    if (a_params.length > T(0.0) && a_params.radius > T(0.0)) {
      EBGEOMETRY_EXPECT(std::abs(a_params.axis.length() - T(1)) < std::sqrt(std::numeric_limits<T>::epsilon()));

      const Vec3T<T> point = a_point - a_params.center;
      const T        para  = dot(point, a_params.axis);
      const Vec3T<T> ortho = point - para * a_params.axis; // Distance from cylinder axis.

      // w: Distance from cylinder wall. < 0 on inside and > 0 on outside.
      // h: Distance from cylinder top.  < 0 on inside and > 0 on outside.
      const T w = ortho.length() - a_params.radius;
      const T h = std::abs(para) - T(0.5) * a_params.length;

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
};

/**
 * @brief Signed distance field for a finite, flat-capped cylinder.
 * @details Thin wrapper around CylinderOp. By default the cylinder has radius 1 and height 1, centred
 * at the origin with its axis along y (cap centres at (0,-0.5,0) and (0,0.5,0)).
 * @tparam T Floating-point precision.
 */
template <class T>
class CylinderSDF : public ImplicitFunction<T, CylinderOp<T>>
{
  static_assert(std::is_floating_point_v<T>, "CylinderSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief Default constructor. Constructs a unit cylinder of radius 1 and height 1, centred at the
   * origin with its axis along y (cap centres at (0,-0.5,0) and (0,0.5,0)).
   */
  CylinderSDF() noexcept : ImplicitFunction<T, CylinderOp<T>>(typename CylinderOp<T>::Params{})
  {}

  /**
   * @brief Full constructor.
   * @param[in] a_center1 One endpoint (cap center).
   * @param[in] a_center2 Other endpoint (cap center).
   * @param[in] a_radius  Cylinder radius.
   */
  CylinderSDF(const Vec3T<T>& a_center1, const Vec3T<T>& a_center2, const T& a_radius) noexcept
    : ImplicitFunction<T, CylinderOp<T>>(makeParams(a_center1, a_center2, a_radius))
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
  }

  /**
   * @brief Get one endpoint
   * @return One cap center.
   */
  [[nodiscard]] const Vec3T<T>&
  getCenter1() const noexcept
  {
    return this->m_params.center1;
  }

  /**
   * @brief Get the other endpoint
   * @return Other cap center.
   */
  [[nodiscard]] const Vec3T<T>&
  getCenter2() const noexcept
  {
    return this->m_params.center2;
  }

  /**
   * @brief Get radius.
   * @return Cylinder radius.
   */
  [[nodiscard]] const T&
  getRadius() const noexcept
  {
    return this->m_params.radius;
  }

protected:
  /**
   * @brief Build the parameter bundle from the two cap centers and radius.
   * @param[in] a_center1 One endpoint.
   * @param[in] a_center2 Other endpoint.
   * @param[in] a_radius  Cylinder radius.
   * @return Populated cylinder parameters (with precomputed center, axis, and length).
   */
  [[nodiscard]] static typename CylinderOp<T>::Params
  makeParams(const Vec3T<T>& a_center1, const Vec3T<T>& a_center2, const T& a_radius) noexcept
  {
    typename CylinderOp<T>::Params p;

    p.center1 = a_center1;
    p.center2 = a_center2;
    p.radius  = a_radius;
    p.center  = (a_center2 + a_center1) * T(0.5);
    p.length  = (a_center2 - a_center1).length();
    p.axis    = (a_center2 - a_center1) / p.length;

    return p;
  }
};

/**
 * @brief Distance-formula trait for an infinitely long cylinder.
 * @details The cylinder is infinite along the coordinate axis selected by `axis` (0 = x, 1 = y,
 * 2 = z) with a circular cross-section of radius `radius` centred at `center`.
 * @tparam T Floating-point precision.
 */
template <class T>
struct InfiniteCylinderOp
{
  /**
   * @brief Stored parameters for an infinite cylinder.
   */
  struct Params
  {
    Vec3T<T> center = Vec3T<T>::zeros(); ///< Center point on the cylinder axis.
    T        radius = T(1);              ///< Cylinder radius.
    size_t   axis   = 1U;                ///< Coordinate axis index (0 = x, 1 = y, 2 = z).
  };

  /**
   * @brief All infinite cylinders are true signed distance functions.
   */
  static constexpr bool isSignedDistance = true;

  /**
   * @brief Signed distance to the infinite cylinder.
   * @param[in] a_params Cylinder parameters.
   * @param[in] a_point  Query point.
   * @return Signed distance; negative inside.
   */
  EBGEOMETRY_HOST_DEVICE static T
  eval(const Params& a_params, const Vec3T<T>& a_point) noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    Vec3T<T> delta       = a_point - a_params.center;
    delta[a_params.axis] = 0.0;

    const T d = delta.length() - a_params.radius;

    return d;
  }
};

/**
 * @brief Signed distance field for an infinitely long cylinder.
 * @details Thin wrapper around InfiniteCylinderOp. By default the cylinder has radius 1, passes
 * through the origin, and extends along y.
 * @tparam T Floating-point precision.
 */
template <class T>
class InfiniteCylinderSDF : public ImplicitFunction<T, InfiniteCylinderOp<T>>
{
  static_assert(std::is_floating_point_v<T>, "InfiniteCylinderSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief Default constructor. Constructs an infinite cylinder of radius 1 centred at the origin
   * with its axis along y (axis index 1).
   */
  InfiniteCylinderSDF() noexcept : ImplicitFunction<T, InfiniteCylinderOp<T>>(typename InfiniteCylinderOp<T>::Params{})
  {}

  /**
   * @brief Full constructor.
   * @param[in] a_center Center point on the cylinder axis.
   * @param[in] a_radius Cylinder radius.
   * @param[in] a_axis   Coordinate axis index (0 = x, 1 = y, 2 = z).
   */
  InfiniteCylinderSDF(const Vec3T<T>& a_center, const T& a_radius, const size_t a_axis) noexcept
    : ImplicitFunction<T, InfiniteCylinderOp<T>>(typename InfiniteCylinderOp<T>::Params{a_center, a_radius, a_axis})
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_center[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_center[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_center[2]));
    EBGEOMETRY_EXPECT(std::isfinite(a_radius));
    EBGEOMETRY_EXPECT(a_radius > T(0));
    EBGEOMETRY_EXPECT(a_axis < 3U);
  }
};

/**
 * @brief Distance-formula trait for a capsule (pill shape): a cylinder capped with hemispheres.
 * @details The hemisphere centres `center1`/`center2` are inset from the tips by the radius along the
 * capsule axis; the CapsuleSDF wrapper computes them from the outer tip positions.
 * @tparam T Floating-point precision.
 */
template <class T>
struct CapsuleOp
{
  /**
   * @brief Stored parameters for a capsule.
   */
  struct Params
  {
    Vec3T<T> center1 = Vec3T<T>(T(0), T(-0.5), T(0)); ///< Center of one hemispherical cap.
    Vec3T<T> center2 = Vec3T<T>(T(0), T(0.5), T(0));  ///< Center of the other hemispherical cap.
    T        radius  = T(0.5);                        ///< Capsule radius.
  };

  /**
   * @brief All capsules are true signed distance functions.
   */
  static constexpr bool isSignedDistance = true;

  /**
   * @brief Signed distance to the capsule.
   * @param[in] a_params Capsule parameters.
   * @param[in] a_point  Query point.
   * @return Signed distance; negative inside.
   */
  EBGEOMETRY_HOST_DEVICE static T
  eval(const Params& a_params, const Vec3T<T>& a_point) noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));
    EBGEOMETRY_EXPECT((a_params.center2 - a_params.center1).length() > T(0));

    const Vec3T<T> v1 = a_point - a_params.center1;
    const Vec3T<T> v2 = a_params.center2 - a_params.center1;

    const T h = std::clamp(dot(v1, v2) / dot(v2, v2), T(0.0), T(1.0));
    const T d = length(v1 - h * v2) - a_params.radius;

    return d;
  }
};

/**
 * @brief Signed distance field for a capsule (pill shape).
 * @details Thin wrapper around CapsuleOp. By default the capsule is centred at the origin, aligned
 * along y, with tips at (0,-1,0) and (0,1,0) and radius 0.5.
 * @tparam T Floating-point precision.
 */
template <class T>
class CapsuleSDF : public ImplicitFunction<T, CapsuleOp<T>>
{
  static_assert(std::is_floating_point_v<T>, "CapsuleSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief Default constructor. Constructs a capsule with tips at (0,-1,0) and (0,1,0), radius 0.5.
   */
  CapsuleSDF() noexcept : ImplicitFunction<T, CapsuleOp<T>>(typename CapsuleOp<T>::Params{})
  {}

  /**
   * @brief Full constructor.
   * @details The hemisphere centres stored internally are derived from the tips:
   * `center1 = a_tip1 + a_radius * axis` and `center2 = a_tip2 - a_radius * axis`.
   * @param[in] a_tip1   Outer tip of one hemispherical cap (outermost point in that direction).
   * @param[in] a_tip2   Outer tip of the other hemispherical cap.
   * @param[in] a_radius Capsule radius (applied to both the tube and the hemispherical caps).
   */
  CapsuleSDF(const Vec3T<T>& a_tip1, const Vec3T<T>& a_tip2, const T& a_radius) noexcept
    : ImplicitFunction<T, CapsuleOp<T>>(makeParams(a_tip1, a_tip2, a_radius))
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
  }

protected:
  /**
   * @brief Build the parameter bundle from the two outer tips and radius.
   * @param[in] a_tip1   Outer tip of one hemispherical cap.
   * @param[in] a_tip2   Outer tip of the other hemispherical cap.
   * @param[in] a_radius Capsule radius.
   * @return Populated capsule parameters (with hemisphere centres inset from the tips).
   */
  [[nodiscard]] static typename CapsuleOp<T>::Params
  makeParams(const Vec3T<T>& a_tip1, const Vec3T<T>& a_tip2, const T& a_radius) noexcept
  {
    const Vec3T<T> axis = (a_tip2 - a_tip1) / length(a_tip2 - a_tip1);

    typename CapsuleOp<T>::Params p;

    p.center1 = a_tip1 + a_radius * axis;
    p.center2 = a_tip2 - a_radius * axis;
    p.radius  = a_radius;

    return p;
  }
};

/**
 * @brief Distance-formula trait for an infinite cone.
 * @details The cone tip is at `tip` and the body opens in the -z direction; `c` encodes
 * `(sin, cos)` of the half opening-angle.
 * @tparam T Floating-point precision.
 */
template <class T>
struct InfiniteConeOp
{
  /**
   * @brief Stored parameters for an infinite cone.
   */
  struct Params
  {
    Vec3T<T> tip = Vec3T<T>::zeros();                                        ///< Tip position.
    Vec2T<T> c   = Vec2T<T>(std::sin(pi<T> / T(8)), std::cos(pi<T> / T(8))); ///< (sin, cos) of half opening-angle.
  };

  /**
   * @brief All infinite cones are true signed distance functions.
   */
  static constexpr bool isSignedDistance = true;

  /**
   * @brief Signed distance to the infinite cone.
   * @param[in] a_params Cone parameters.
   * @param[in] a_point  Query point.
   * @return Signed distance; negative inside the cone.
   */
  EBGEOMETRY_HOST_DEVICE static T
  eval(const Params& a_params, const Vec3T<T>& a_point) noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));
    EBGEOMETRY_EXPECT(std::abs(length(a_params.c) - T(1)) < std::sqrt(std::numeric_limits<T>::epsilon()));

    const Vec3T<T> delta = a_point - a_params.tip;
    const Vec2T<T> q(std::sqrt(delta[0] * delta[0] + delta[1] * delta[1]), -delta[2]);

    const T d1 = length(q - a_params.c * std::max(dot(q, a_params.c), T(0.0)));
    const T d2 = d1 * ((q.x * a_params.c.y - q.y * a_params.c.x < T(0.0)) ? T(-1.0) : T(1.0));

    return d2;
  }
};

/**
 * @brief Signed distance field for an infinite cone.
 * @details Thin wrapper around InfiniteConeOp. By default the tip is at the origin and the full
 * opening angle is 45°, with the body extending along -z.
 * @tparam T Floating-point precision.
 */
template <class T>
class InfiniteConeSDF : public ImplicitFunction<T, InfiniteConeOp<T>>
{
  static_assert(std::is_floating_point_v<T>, "InfiniteConeSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief Default constructor. Constructs an infinite cone with tip at the origin, 45° full
   * opening angle, and body extending in the -z direction.
   */
  InfiniteConeSDF() noexcept : ImplicitFunction<T, InfiniteConeOp<T>>(typename InfiniteConeOp<T>::Params{})
  {}

  /**
   * @brief Full constructor.
   * @param[in] a_tip   Cone tip position.
   * @param[in] a_angle Full opening angle in degrees (must be in (0°, 180°)).
   */
  InfiniteConeSDF(const Vec3T<T>& a_tip, const T& a_angle) noexcept
    : ImplicitFunction<T, InfiniteConeOp<T>>(typename InfiniteConeOp<T>::Params{
        a_tip, Vec2T<T>(std::sin(T(0.5) * a_angle * pi<T> / T(180)), std::cos(T(0.5) * a_angle * pi<T> / T(180)))})
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_tip[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_tip[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_tip[2]));
    EBGEOMETRY_EXPECT(std::isfinite(a_angle));
    EBGEOMETRY_EXPECT(a_angle > T(0));
    EBGEOMETRY_EXPECT(a_angle < T(180));
  }
};

/**
 * @brief Distance-formula trait for a finite cone.
 * @details The cone tip is at `tip` and the body opens in the -z direction with height `height`;
 * `c` encodes `(sin, cos)` of the half opening-angle.
 * @tparam T Floating-point precision.
 */
template <class T>
struct ConeOp
{
  /**
   * @brief Stored parameters for a finite cone.
   */
  struct Params
  {
    Vec3T<T> tip    = Vec3T<T>::zeros();                                        ///< Tip position.
    Vec2T<T> c      = Vec2T<T>(std::sin(pi<T> / T(8)), std::cos(pi<T> / T(8))); ///< (sin, cos) of half opening-angle.
    T        height = T(1);                                                     ///< Cone height (tip to base).
  };

  /**
   * @brief All finite cones are true signed distance functions.
   */
  static constexpr bool isSignedDistance = true;

  /**
   * @brief Signed distance to the finite cone.
   * @param[in] a_params Cone parameters.
   * @param[in] a_point  Query point.
   * @return Signed distance; negative inside the cone.
   */
  EBGEOMETRY_HOST_DEVICE static T
  eval(const Params& a_params, const Vec3T<T>& a_point) noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));
    EBGEOMETRY_EXPECT(std::abs(length(a_params.c) - T(1)) < std::sqrt(std::numeric_limits<T>::epsilon()));
    EBGEOMETRY_EXPECT(a_params.c.y > T(0));

    const Vec3T<T> delta = a_point - a_params.tip;
    const T        dr    = std::sqrt(delta[0] * delta[0] + delta[1] * delta[1]);
    const T        dz    = delta[2];

    constexpr T zero = T(0.0);
    constexpr T one  = T(1.0);

    const Vec2T<T> q = a_params.height * Vec2T<T>(a_params.c.x / a_params.c.y, -1.0);
    const Vec2T<T> w = Vec2T<T>(dr, dz);
    const Vec2T<T> a = w - std::clamp(dot(w, q) / dot(q, q), zero, one) * q;
    const Vec2T<T> b = w - Vec2T<T>(q.x * std::clamp(w.x / q.x, zero, one), q.y);

    auto sign = [](const T& x) -> int { return (x > zero) - (x < zero); };

    const T k = sign(q.y);
    const T d = std::min(dot(a, a), dot(b, b));
    const T s = std::max(k * (w.x * q.y - w.y * q.x), k * (w.y - q.y));

    return std::sqrt(d) * sign(s);
  }
};

/**
 * @brief Signed distance field for a finite cone.
 * @details Thin wrapper around ConeOp. By default the tip is at the origin, the height is 1, and the
 * full opening angle is 45°.
 * @tparam T Floating-point precision.
 */
template <class T>
class ConeSDF : public ImplicitFunction<T, ConeOp<T>>
{
  static_assert(std::is_floating_point_v<T>, "ConeSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief Default constructor. Constructs a finite cone with its tip at the origin, height 1,
   * and a 45-degree opening angle, body extending in the -z direction.
   */
  ConeSDF() noexcept : ImplicitFunction<T, ConeOp<T>>(typename ConeOp<T>::Params{})
  {}

  /**
   * @brief Full constructor.
   * @param[in] a_tip    Cone tip position.
   * @param[in] a_height Cone height (tip to base).
   * @param[in] a_angle  Full opening angle in degrees.
   */
  ConeSDF(const Vec3T<T>& a_tip, const T& a_height, const T& a_angle) noexcept
    : ImplicitFunction<T, ConeOp<T>>(typename ConeOp<T>::Params{
        a_tip,
        Vec2T<T>(std::sin(T(0.5) * a_angle * pi<T> / T(180)), std::cos(T(0.5) * a_angle * pi<T> / T(180))),
        a_height})
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_tip[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_tip[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_tip[2]));
    EBGEOMETRY_EXPECT(std::isfinite(a_height));
    EBGEOMETRY_EXPECT(std::isfinite(a_angle));
    EBGEOMETRY_EXPECT(a_height > T(0));
    EBGEOMETRY_EXPECT(a_angle > T(0));
    EBGEOMETRY_EXPECT(a_angle < T(180));
  }
};

/**
 * @brief Distance-formula trait for an axis-aligned box with rounded corners.
 * @details `dimensions` are the inner box half-extents (= 0.5 * user-supplied full side lengths) and
 * `curvature` is the corner sphere radius. The formula inlines the composed sphere directly.
 * @tparam T Floating-point precision.
 */
template <class T>
struct RoundedBoxOp
{
  /**
   * @brief Stored parameters for a rounded box.
   */
  struct Params
  {
    Vec3T<T> dimensions = Vec3T<T>(T(0.5), T(0.5), T(0.5)); ///< Inner box half-extents.
    T        curvature  = T(0.1);                           ///< Corner sphere radius.
  };

  /**
   * @brief All rounded boxes are true signed distance functions.
   */
  static constexpr bool isSignedDistance = true;

  /**
   * @brief Signed distance to the rounded box.
   * @param[in] a_params Rounded-box parameters.
   * @param[in] a_point  Query point.
   * @return Signed distance; negative inside.
   */
  EBGEOMETRY_HOST_DEVICE static T
  eval(const Params& a_params, const Vec3T<T>& a_point) noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    // Direct rounded-box formula: distance from the query point to the inner (clamped) box, minus the
    // corner curvature. This is exactly SphereSDF(center=0, radius=curvature) evaluated at
    // a_point - clamp(a_point, -dimensions, dimensions).
    return (a_point - clamp(a_point, -a_params.dimensions, a_params.dimensions)).length() - a_params.curvature;
  }
};

/**
 * @brief Signed distance field for an axis-aligned box with rounded corners.
 * @details Thin wrapper around RoundedBoxOp. By default it is a rounded unit cube ([-0.5, 0.5]^3
 * before rounding) with corner curvature 0.1.
 * @tparam T Floating-point precision.
 */
template <class T>
class RoundedBoxSDF : public ImplicitFunction<T, RoundedBoxOp<T>>
{
  static_assert(std::is_floating_point_v<T>, "RoundedBoxSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief Default constructor. Constructs a rounded unit cube centered at the origin
   * ([-0.5, 0.5]^3 before rounding) with corner curvature 0.1.
   */
  RoundedBoxSDF() noexcept : ImplicitFunction<T, RoundedBoxOp<T>>(typename RoundedBoxOp<T>::Params{})
  {}

  /**
   * @brief Full constructor.
   * @details The total half-extent in direction @p i is @c 0.5*a_dimensions[i] + a_curvature.
   * @param[in] a_dimensions Box dimensions (full side lengths before rounding).
   * @param[in] a_curvature  Corner sphere radius. Must be > 0.
   */
  RoundedBoxSDF(const Vec3T<T>& a_dimensions, const T a_curvature) noexcept
    : ImplicitFunction<T, RoundedBoxOp<T>>(typename RoundedBoxOp<T>::Params{T(0.5) * a_dimensions, a_curvature})
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_dimensions[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_dimensions[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_dimensions[2]));
    EBGEOMETRY_EXPECT(std::isfinite(a_curvature));
    EBGEOMETRY_EXPECT(a_dimensions[0] > T(0));
    EBGEOMETRY_EXPECT(a_dimensions[1] > T(0));
    EBGEOMETRY_EXPECT(a_dimensions[2] > T(0));
    EBGEOMETRY_EXPECT(a_curvature > T(0));
  }
};

/**
 * @brief Distance-formula trait for a cylinder with rounded (toroidal) edges.
 * @details The cylinder is centred at the origin with its axis along y. `majorRadius` is the inner
 * core radius, `minorRadius` the edge rounding radius, and `height` the inner half-height.
 * @tparam T Floating-point precision.
 */
template <class T>
struct RoundedCylinderOp
{
  /**
   * @brief Stored parameters for a rounded cylinder.
   */
  struct Params
  {
    T majorRadius = T(0.9); ///< Inner cylinder radius (= outer radius - curvature).
    T minorRadius = T(0.1); ///< Edge rounding radius (= curvature).
    T height      = T(0.4); ///< Inner half-height (= 0.5*height - curvature).
  };

  /**
   * @brief All rounded cylinders are true signed distance functions.
   */
  static constexpr bool isSignedDistance = true;

  /**
   * @brief Signed distance to the rounded cylinder.
   * @param[in] a_params Rounded-cylinder parameters.
   * @param[in] a_point  Query point.
   * @return Signed distance; negative inside.
   */
  EBGEOMETRY_HOST_DEVICE static T
  eval(const Params& a_params, const Vec3T<T>& a_point) noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    const T    xz = std::sqrt(a_point[2] * a_point[2] + a_point[0] * a_point[0]);
    const auto d1 = Vec2T<T>(xz - a_params.majorRadius, std::abs(a_point[1]) - a_params.height);
    const auto d2 = Vec2T<T>(std::max(d1.x, T(0)), std::max(d1.y, T(0)));

    return std::min(std::max(d1.x, d1.y), T(0)) + std::sqrt(d2.x * d2.x + d2.y * d2.y) - a_params.minorRadius;
  }
};

/**
 * @brief Signed distance field for a cylinder with rounded (toroidal) edges.
 * @details Thin wrapper around RoundedCylinderOp. By default the cylinder has outer radius 1, height
 * 1, and edge curvature 0.1 (stored as majorRadius=0.9, minorRadius=0.1, height=0.4).
 * @tparam T Floating-point precision.
 */
template <class T>
class RoundedCylinderSDF : public ImplicitFunction<T, RoundedCylinderOp<T>>
{
  static_assert(std::is_floating_point_v<T>, "RoundedCylinderSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief Default constructor. Constructs a rounded cylinder centred at the origin, axis along y,
   * with outer radius 1, total height 1, and edge curvature 0.1.
   */
  RoundedCylinderSDF() noexcept : ImplicitFunction<T, RoundedCylinderOp<T>>(typename RoundedCylinderOp<T>::Params{})
  {}

  /**
   * @brief Full constructor.
   * @param[in] a_radius    Outer cylinder radius. Must be > 0 and > a_curvature.
   * @param[in] a_curvature Edge rounding radius. Must be > 0 and satisfy
   * a_curvature < a_radius and 2 * a_curvature < a_height.
   * @param[in] a_height    Total cylinder height. Must be > 0 and > 2 * a_curvature.
   */
  RoundedCylinderSDF(const T a_radius, const T a_curvature, const T a_height) noexcept
    : ImplicitFunction<T, RoundedCylinderOp<T>>(
        typename RoundedCylinderOp<T>::Params{a_radius - a_curvature, a_curvature, T(0.5) * a_height - a_curvature})
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_radius));
    EBGEOMETRY_EXPECT(std::isfinite(a_curvature));
    EBGEOMETRY_EXPECT(std::isfinite(a_height));
    EBGEOMETRY_EXPECT(a_radius > T(0));
    EBGEOMETRY_EXPECT(a_curvature > T(0));
    EBGEOMETRY_EXPECT(a_height > T(0));
    EBGEOMETRY_EXPECT(a_curvature < a_radius);
    EBGEOMETRY_EXPECT(T(2) * a_curvature < a_height);
  }
};

/**
 * @brief Ken Perlin's gradient-noise implicit function.
 * @details This class evaluates multi-octave Perlin noise at a 3-D point and returns the result
 * as a scalar. It is **not** a true signed distance function (the gradient magnitude is not
 * guaranteed to equal 1), but it can serve as a procedural displacement field when added to or
 * subtracted from a conventional SDF.
 *
 * Unlike the geometric primitives in this header, PerlinSDF is kept as a direct
 * @c ImplicitFunction<T> subclass that overrides value() rather than a trait-based leaf: it owns a
 * mutable 512-entry permutation table plus stateful helpers (shuffle(), getPermutationTable()) and
 * is explicitly not an SDF, so `isSignedDistance()` reports false.
 *
 * @note The default constructor leaves the permutation table all-zeros, producing constant-zero
 * noise. Use the full constructor (which initialises the table from Ken Perlin's reference
 * permutation and then shuffles it) or call `shuffle()` explicitly before use.
 * @tparam T Floating-point precision.
 */
template <class T>
class PerlinSDF : public ImplicitFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "PerlinSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief Default constructor. Constructs a single-octave Perlin noise field with unit amplitude,
   * unit frequency along all axes, and persistence 0.5.
   */
  PerlinSDF() noexcept
  {
    this->m_sdf = false;
  }

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

    this->m_sdf = false;

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
   * @brief Value function. Generates a noise value on [0, m_noiseAmplitude].
   * @param[in] a_point Input point.
   * @return Octave-summed Perlin noise value scaled by m_noiseAmplitude.
   */
  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override
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

} // namespace EBGeometry

#endif
