/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file    EBGeometry_AnalyticDistanceFunctions.hpp
  @brief   Declaration of various analytic distance functions.
  @details This file contains various analytic signed distance fields. Some of
  these also include member function for fetching parameters, and users are free
  to add such functions also to other shapes.
  @author  Robert Marskar
*/

#ifndef EBGeometry_AnalyticDistanceFunctions
#define EBGeometry_AnalyticDistanceFunctions

// Std includes
#include <algorithm>

// Our includes
#include "EBGeometry_SignedDistanceFunction.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Clamp function. Returns lo if v < lo and hi if v > hi. Otherwise
  returns v.
  @param[in] v  Value to be clamped.
  @param[in] lo Lower clamping value.
  @param[in] hi Higher clamping value.
*/
template <class T>
constexpr const T&
clamp(const T& v, const T& lo, const T& hi)
{
  return v < lo ? lo : hi < v ? hi : v;
}

/*!
  @brief Rounded signed distance function.
  @details Rounded distance function -- returns the original distance function
  displaced by the specified rounding radius.
  @note Users may need to scale their distance functions afterwards.
*/
template <class T>
class RoundedSDF : public SignedDistanceFunction<T>
{
public:
  /*!
    @brief Disallowed weak construction
  */
  RoundedSDF() = delete;

  /*!
    @brief Rounded SDF. Rounds the input SDF
    @param[in] a_sdf  Input signed distance function.
    @param[in] a_curv Rounding radius.
  */
  RoundedSDF(const std::shared_ptr<SignedDistanceFunction<T>> a_sdf, const T a_curv)
  {
    m_sdf  = a_sdf;
    m_curv = a_curv;
  }

  /*!
    @brief Destructor
  */
  virtual ~RoundedSDF() {}

  /*!
    @brief Signed distance field.
  */
  virtual T
  signedDistance(const Vec3T<T>& a_point) const noexcept override
  {
    return m_sdf->signedDistance(a_point) - m_curv;
  }

protected:
  /*!
    @brief Original signed distance function
  */
  std::shared_ptr<const SignedDistanceFunction<T>> m_sdf;

  /*!
    @brief Rounding radius
  */
  T m_curv;
};

/*!
  @brief Annular signed distance function. Creates a shell out of an object.
*/
template <class T>
class AnnularSDF : public SignedDistanceFunction<T>
{
public:
  /*!
    @brief Disallowed weak construction
  */
  AnnularSDF() = delete;

  /*!
    @brief Rounded SDF. Rounds the input SDF
    @param[in] a_sdf  Input signed distance function.
    @param[in] a_curv Rounding radius.
  */
  AnnularSDF(const std::shared_ptr<SignedDistanceFunction<T>> a_sdf, const T a_curv)
  {
    m_sdf  = a_sdf;
    m_curv = a_curv;
  }

  /*!
    @brief Destructor
  */
  virtual ~AnnularSDF() {}

  /*!
    @brief Signed distance field.
  */
  virtual T
  signedDistance(const Vec3T<T>& a_point) const noexcept override
  {
    return std::abs(m_sdf->signedDistance(a_point)) - m_curv;
  }

protected:
  /*!
    @brief Original signed distance function
  */
  std::shared_ptr<const SignedDistanceFunction<T>> m_sdf;

  /*!
    @brief Rounding radius
  */
  T m_curv;
};

/*!
  @brief Scaled signed distance function.
*/
template <class T>
class ScaledSDF : public SignedDistanceFunction<T>
{
public:
  /*!
    @brief Disallowed weak construction
  */
  ScaledSDF() = delete;

  /*!
    @brief Scaled SDF.
    @param[in] a_sdf   Input signed distance function.
    @param[in] a_scale Scaling factor.
  */
  ScaledSDF(const std::shared_ptr<SignedDistanceFunction<T>> a_sdf, const T a_scale)
  {
    m_sdf   = a_sdf;
    m_scale = a_scale;
  }

  /*!
    @brief Destructor
  */
  virtual ~ScaledSDF() {}

  /*!
    @brief Signed distance field.
    @param[in] a_point Input point.
  */
  virtual T
  signedDistance(const Vec3T<T>& a_point) const noexcept override
  {
    return (m_sdf->signedDistance(a_point / m_scale)) * m_scale;
  }

protected:
  /*!
    @brief Original signed distance function
  */
  std::shared_ptr<const SignedDistanceFunction<T>> m_sdf;

  /*!
    @brief Scaling factor.
  */
  T m_scale;
};

template <class T>
class PlaneSDF : public SignedDistanceFunction<T>
{
public:
  /*!
    @brief Disallowed weak construction
  */
  PlaneSDF() = delete;

  /*!
    @brief Full constructor
    @param[in] a_point      Point on the plane
    @param[in] a_normal     Plane normal vector.
    @param[in] a_flipInside Hook for turning inside to outside.
  */
  PlaneSDF(const Vec3T<T>& a_point, const Vec3T<T>& a_normal, const bool a_flipInside)
  {
    m_point      = a_point;
    m_normal     = a_normal;
    m_flipInside = a_flipInside;

    m_normal /= m_normal.length();
  }

  /*!
    @brief Signed distance function for sphere.
    @param[in] a_point Position.
  */
  virtual T
  signedDistance(const Vec3T<T>& a_point) const noexcept override
  {
    const T sign = m_flipInside ? -1.0 : 1.0;

    return sign * dot(a_point - m_point, m_normal);
  }

protected:
  /*!
    @brief Point on plane
  */
  Vec3T<T> m_point;

  /*!
    @brief Plane normal vector
  */
  Vec3T<T> m_normal;

  /*!
    @brief Inside to outside hook
  */
  bool m_flipInside;
};

/*!
  @brief Signed distance field for sphere.
*/
template <class T>
class SphereSDF : public SignedDistanceFunction<T>
{
public:
  /*!
    @brief Disallowed weak construction.
  */
  SphereSDF() = delete;

  /*!
    @brief Default constructor
    @param[in] a_center Sphere center
    @param[in] a_radius Sphere radius
    @param[in] a_flipInside Flip inside or not
  */
  SphereSDF(const Vec3T<T>& a_center, const T& a_radius, const bool a_flipInside)
  {
    this->m_center     = a_center;
    this->m_radius     = a_radius;
    this->m_flipInside = a_flipInside;
  }

  /*!
    @brief Copy constructor
  */
  SphereSDF(const SphereSDF& a_other)
  {
    this->m_center       = a_other.m_center;
    this->m_radius       = a_other.m_radius;
    this->m_flipInside   = a_other.m_flipInside;
    this->m_transformOps = a_other.m_transformOps;
  }

  /*!
    @brief Destructor
  */
  virtual ~SphereSDF() = default;

  /*!
    @brief Get center
  */
  const Vec3T<T>&
  getCenter() const noexcept
  {
    return m_center;
  }

  /*!
    @brief Get center
  */
  Vec3T<T>&
  getCenter() noexcept
  {
    return m_center;
  }

  /*!
    @brief Get radius
  */
  const T&
  getRadius() const noexcept
  {
    return m_radius;
  }

  /*!
    @brief Get radius
  */
  T&
  getRadius() noexcept
  {
    return m_radius;
  }

  /*!
    @brief Signed distance function for sphere.
    @param[in] a_point Position.
  */
  virtual T
  signedDistance(const Vec3T<T>& a_point) const noexcept override
  {
    const T sign = m_flipInside ? -1.0 : 1.0;

    return sign * ((a_point - m_center).length() - m_radius);
  }

protected:
  /*!
    @brief Sphere center
  */
  Vec3T<T> m_center;

  /*!
    @brief Sphere radius
  */
  T m_radius;

  /*!
    @brief For flipping sign.
  */
  bool m_flipInside;
};

/*!
  @brief Signed distance field for an axis-aligned box.
*/
template <class T>
class BoxSDF : public SignedDistanceFunction<T>
{
public:
  /*!
    @brief Disallowed default constructor
  */
  BoxSDF() = delete;

  /*!
    @brief Full constructor. Sets the low and high corner
    @param[in] a_loCorner   Lower left corner
    @param[in] a_hiCorner   Upper right corner
    @param[in] a_flipInside Flip inside/outside.
  */
  BoxSDF(const Vec3T<T>& a_loCorner, const Vec3T<T>& a_hiCorner, const bool a_flipInside)
  {
    this->m_loCorner   = a_loCorner;
    this->m_hiCorner   = a_hiCorner;
    this->m_flipInside = a_flipInside;
  }

  /*!
    @brief Destructor (does nothing).
  */
  virtual ~BoxSDF() {}

  /*!
    @brief Get lower-left corner
    @return m_loCorner
  */
  const Vec3T<T>&
  getLowCorner() const noexcept
  {
    return m_loCorner;
  }

  /*!
    @brief Get lower-left corner
    @return m_loCorner
  */
  Vec3T<T>&
  getLowCorner() noexcept
  {
    return m_loCorner;
  }

  /*!
    @brief Get upper-right corner
    @return m_hiCorner
  */
  const Vec3T<T>&
  getHighCorner() const noexcept
  {
    return m_hiCorner;
  }

  /*!
    @brief Get upper-right corner
    @return m_hiCorner
  */
  Vec3T<T>&
  getHighCorner() noexcept
  {
    return m_hiCorner;
  }

  /*!
    @brief Signed distance function for sphere.
    @param[in] a_point Position.
  */
  virtual T
  signedDistance(const Vec3T<T>& a_point) const noexcept override
  {
    // For each coordinate direction, we have delta[dir] if a_point[dir] falls
    // between xLo and xHi. In this case delta[dir] will be the signed distance
    // to the closest box face in the dir-direction. Otherwise, if a_point[dir]
    // is outside the corner we have delta[dir] > 0.
    const Vec3T<T> delta(std::max(m_loCorner[0] - a_point[0],
                                  a_point[0] - m_hiCorner[0]), // < 0 if point falls between xLo and xHi.
                         std::max(m_loCorner[1] - a_point[1],
                                  a_point[1] - m_hiCorner[1]), // < 0 if point falls between yLo and yHi.
                         std::max(m_loCorner[2] - a_point[2],
                                  a_point[2] - m_hiCorner[2])); // < 0 if point falls between zLo and zHi.

    // Note: max is max(Vec3T<T>, Vec3T<T>) and not std::max. It returns a
    // vector with coordinate-wise largest components. Note that the first part
    // std::min(...) is the signed distance on the inside of the box (delta will
    // have negative components). The other part max(Vec3T<T>::zero(), ...) is
    // for outside the box.
    const T d = std::min(T(0.0), delta[delta.maxDir(false)]) + max(Vec3T<T>::zero(), delta).length();

    const T sign = m_flipInside ? -1.0 : 1.0;

    return sign * d;
  }

protected:
  /*!
    @brief Low box corner
  */
  Vec3T<T> m_loCorner;

  /*!
    @brief High box corner
  */
  Vec3T<T> m_hiCorner;

  /*!
    @brief Hook for making outside -> inside.
  */
  bool m_flipInside;
};

/*!
  @brief Signed distance field for a torus.
  @details This is constructed such that the donut lies in the xy-plane.
*/
template <class T>
class TorusSDF : public SignedDistanceFunction<T>
{
public:
  /*!
    @brief Disallowed weak construction.
  */
  TorusSDF() = delete;

  /*!
    @brief Full constructor.
    @param[in] a_center      Torus center.
    @param[in] a_majorRadius Major torus radius.
    @param[in] a_minorRadius Minor torus radius.
    @param[in] a_flipInside  Flip inside/outside.
  */
  TorusSDF(const Vec3T<T>& a_center, const T& a_majorRadius, const T& a_minorRadius, const bool a_flipInside)
  {
    this->m_center      = a_center;
    this->m_majorRadius = a_majorRadius;
    this->m_minorRadius = a_minorRadius;
    this->m_flipInside  = a_flipInside;
  }

  /*!
    @brief Destructor (does nothing).
  */
  virtual ~TorusSDF() {}

  /*!
    @brief Get torus center.
    @return m_center
  */
  const Vec3T<T>&
  getCenter() const noexcept
  {
    return m_center;
  }

  /*!
    @brief Get torus center.
    @return m_center
  */
  Vec3T<T>&
  getCenter() noexcept
  {
    return m_center;
  }

  /*!
    @brief Get major radius.
    @return m_majorRadius
  */
  const T&
  getMajorRadius() const noexcept
  {
    return m_majorRadius;
  }

  /*!
    @brief Get major radius.
    @return m_majorRadius
  */
  T&
  getMajorRadius() noexcept
  {
    return m_majorRadius;
  }

  /*!
    @brief Get minor radius.
    @return m_minorRadius
  */
  const T&
  getMinorRadius() const noexcept
  {
    return m_minorRadius;
  }

  /*!
    @brief Get minor radius.
    @return m_minorRadius
  */
  T&
  getMinorRadius() noexcept
  {
    return m_minorRadius;
  }
  /*!
    @brief Signed distance function for a torus.
    @param[in] a_point Position.
  */
  virtual T
  signedDistance(const Vec3T<T>& a_point) const noexcept override
  {
    const Vec3T<T> p   = a_point - m_center;
    const T        rho = sqrt(p[0] * p[0] + p[1] * p[1]) - m_majorRadius;
    const T        d   = sqrt(rho * rho + p[2] * p[2]) - m_minorRadius;

    const T sign = m_flipInside ? -1.0 : 1.0;

    return sign * d;
  }

protected:
  /*!
    @brief Torus center.
  */
  Vec3T<T> m_center;

  /*!
    @brief Major torus radius.
  */
  T m_majorRadius;

  /*!
    @brief Minor torus radius.
  */
  T m_minorRadius;

  /*!
    @brief Hook for making outside -> inside.
  */
  bool m_flipInside;
};

/*!
  @brief Signed distance field for a cylinder.
*/
template <class T>
class CylinderSDF : public SignedDistanceFunction<T>
{
public:
  /*!
    @brief Disallowed weak construction. Use one of the full constructors.
  */
  CylinderSDF() = delete;

  /*!
    @brief Full constructor.
    @param[in] a_center1    One endpoint.
    @param[in] a_center2    Other endpoint.
    @param[in] a_radius     Cylinder radius.
    @param[in] a_flipInside Flip inside/outside.
  */
  CylinderSDF(const Vec3T<T>& a_center1, const Vec3T<T>& a_center2, const T& a_radius, const bool a_flipInside)
  {
    this->m_center1    = a_center1;
    this->m_center2    = a_center2;
    this->m_radius     = a_radius;
    this->m_flipInside = a_flipInside;

    // Some derived quantities that are needed for SDF computations.
    m_center = (m_center2 + m_center1) * 0.5;
    m_length = (m_center2 - m_center1).length();
    m_axis   = (m_center2 - m_center1) / m_length;
  }

  /*!
    @brief Destructor (does nothing).
  */
  virtual ~CylinderSDF() {}

  /*!
    @brief Get one endpoint
    @return m_center1
  */
  const Vec3T<T>&
  getCenter1() const noexcept
  {
    return m_center1;
  }

  /*!
    @brief Get the other endpoint
    @return m_center2
  */
  const Vec3T<T>&
  getCenter2() const noexcept
  {
    return m_center2;
  }

  /*!
    @brief Get radius.
    @return m_radius.
  */
  const T&
  getRadius() const noexcept
  {
    return m_radius;
  }

  /*!
    @brief Signed distance function for a torus.
    @param[in] a_point Position.
  */
  virtual T
  signedDistance(const Vec3T<T>& a_point) const noexcept override
  {
    T d = std::numeric_limits<T>::infinity();

    if (m_length > 0.0 && m_radius > 0.0) {
      const Vec3T<T> point = a_point - m_center;
      const T        para  = dot(point, m_axis);
      const Vec3T<T> ortho = point - para * m_axis; // Distance from cylinder axis.

      const T w = ortho.length() - m_radius;       // Distance from cylinder wall. < 0
                                                   // on inside and > 0 on outside.
      const T h = std::abs(para) - 0.5 * m_length; // Distance from cylinder top.  < 0 on
                                                   // inside and > 0 on outside.

      constexpr T zero = T(0.0);

      if (w <= zero && h <= zero) { // Inside cylinder
        d = (std::abs(w) < std::abs(h)) ? w : h;
      }
      else if (w <= zero && h > zero) { // Above one of the endcaps.
        d = h;
      }
      else if (w > zero && h < zero) { // Outside radius but between the
                                       // endcaps.
        d = w;
      }
      else {
        d = sqrt(w * w + h * h);
      }
    }

    const T sign = m_flipInside ? -1.0 : 1.0;

    return sign * d;
  }

protected:
  /*!
    @brief One endpoint.
  */
  Vec3T<T> m_center1;

  /*!
    @brief The other endpoint.
  */
  Vec3T<T> m_center2;

  /*!
    @brief m_Halfway between center1 and m_center2
  */
  Vec3T<T> m_center;

  /*!
    @brief "Axis", pointing from m_center1 to m_center2.
  */
  Vec3T<T> m_axis;

  /*!
    @brief Cylinder length
  */
  T m_length;

  /*!
    @brief radius.
  */
  T m_radius;

  /*!
    @brief Hook for making outside -> inside.
  */
  bool m_flipInside;
};

/*!
  @brief Inifinitely long cylinder class. Oriented along the y-axis.
*/
template <class T>
class InfiniteCylinderSDF : public SignedDistanceFunction<T>
{
public:
  /*!
    @brief No weak construction.
  */
  InfiniteCylinderSDF() = delete;

  /*!
    @brief Full constructor.
    @param[in] a_center     Center point for the cylinder.
    @param[in] a_radius     Cylinder radius.
    @param[in] a_axis       Cylinder axis.
    @param[in] a_flipInside Hook for flipping inside/outside.
  */
  InfiniteCylinderSDF(const Vec3T<T>& a_center, const T& a_radius, const size_t a_axis, const bool a_flipInside)
  {
    m_center     = a_center;
    m_radius     = a_radius;
    m_axis       = a_axis;
    m_flipInside = a_flipInside;
  }

  /*!
    @brief Signed distance function.
    @param[in] a_point Position.
  */
  virtual T
  signedDistance(const Vec3T<T>& a_point) const noexcept override
  {
    Vec3T<T> delta = a_point - m_center;
    delta[m_axis]  = 0.0;

    const T d    = delta.length() - m_radius;
    const T sign = m_flipInside ? -1.0 : 1.0;

    return sign * d;
  }

protected:
  /*!
    @brief Cylinder center.
  */
  Vec3T<T> m_center;

  /*!
    @brief Cylinder radius
  */
  T m_radius;

  /*!
    @brief Axis
  */
  size_t m_axis;

  /*!
    @brief Hook for making outside -> inside.
  */
  bool m_flipInside;
};

/*!
  @brief Capsulate/pill SDF. Basically a cylinder with spherical endcaps,
  oriented along the specified axis.
*/
template <class T>
class CapsuleSDF : public SignedDistanceFunction<T>
{
public:
  /*!
    @brief No weak construction
  */
  CapsuleSDF() = delete;

  /*!
    @brief Full constructor.
    @param[in] a_tip1       One tip point
    @param[in] a_tip2       Other center point.
    @param[in] a_radius     Radius.
    @param[in] a_flipInside Hook for flipping inside/outside.
  */
  CapsuleSDF(const Vec3T<T>& a_tip1, const Vec3T<T> a_tip2, const T& a_radius, const bool a_flipInside)
  {
    const Vec3T<T> axis = (a_tip2 - a_tip1) / length(a_tip2 - a_tip1);
    m_center1           = a_tip1 + a_radius * axis;
    m_center2           = a_tip2 - a_radius * axis;
    m_radius            = a_radius;
    m_flipInside        = a_flipInside;
  }

  /*!
    @brief Implementation of the signed distance function.
    @param[in] a_point Position.
  */
  virtual T
  signedDistance(const Vec3T<T>& a_point) const noexcept override
  {
    const Vec3T<T> v1 = a_point - m_center1;
    const Vec3T<T> v2 = m_center2 - m_center1;

    const T h    = clamp(dot(v1, v2) / dot(v2, v2), T(0.0), T(1.0));
    const T d    = length(v1 - h * v2) - m_radius;
    const T sign = m_flipInside ? -1.0 : 1.0;

    return sign * d;
  }

protected:
  /*!
    @brief Capsule center1.
  */
  Vec3T<T> m_center1;

  /*!
    @brief Capsule center2.
  */
  Vec3T<T> m_center2;

  /*!
    @brief Capsule radius.
  */
  T m_radius;

  /*!
    @brief Hook for making outside -> inside.
  */
  bool m_flipInside;
};

/*!
  @brief Signed distance field for an infinite cone. Oriented along +z.
*/
template <class T>
class InfiniteConeSDF : public SignedDistanceFunction<T>
{
public:
  /*!
    @brief Disallowed weak construction
  */
  InfiniteConeSDF() = delete;

  /*!
    @brief Infinite cone function
    @param[in] a_tip        Cone tip position
    @param[in] a_angle      Cone opening angle.
    @param[in] a_flipInside Hook for making inside to outside.
  */
  InfiniteConeSDF(const Vec3T<T>& a_tip, const T& a_angle, const bool a_flipInside)
  {
    constexpr T pi = 3.14159265358979323846;

    m_tip        = a_tip;
    m_c.x        = std::sin(0.5 * a_angle * pi / 180.0);
    m_c.y        = std::cos(0.5 * a_angle * pi / 180.0);
    m_flipInside = a_flipInside;
  }

  /*!
    @brief Destructor -- does nothing
  */
  virtual ~InfiniteConeSDF() {}

  /*!
    @brief Implementation of the signed distance function.
    @param[in] a_point Position.
  */
  virtual T
  signedDistance(const Vec3T<T>& a_point) const noexcept override
  {
    const Vec3T<T> delta = a_point - m_tip;
    const Vec2T<T> q(sqrt(delta[0] * delta[0] + delta[1] * delta[1]), -delta[2]);

    const T d1 = length(q - m_c * std::max(dot(q, m_c), T(0.0)));
    const T d2 = d1 * ((q.x * m_c.y - q.y * m_c.x < 0.0) ? -1.0 : 1.0);

    const T sign = m_flipInside ? -1.0 : 1.0;

    return sign * d2;
  }

protected:
  /*!
    @brief Tip position
  */
  Vec3T<T> m_tip;

  /*!
    @brief Sine/cosine of angle
  */
  Vec2T<T> m_c;

  /*!
    @brief Hook for making inside -> outside.
  */
  bool m_flipInside;
};

/*!
  @brief Signed distance field for an finite cone. Oriented along +z.
*/
template <class T>
class ConeSDF : public SignedDistanceFunction<T>
{
public:
  /*!
    @brief Disallowed weak construction
  */
  ConeSDF() = delete;

  /*!
    @brief Finite cone function
    @param[in] a_tip        Cone tip position
    @param[in] a_height     Cone height, measured from top to bottom.
    @param[in] a_angle      Cone opening angle.
    @param[in] a_flipInside Hook for making inside to outside.
  */
  ConeSDF(const Vec3T<T>& a_tip, const T& a_height, const T& a_angle, const bool a_flipInside)
  {
    constexpr T pi = 3.14159265358979323846;

    m_tip        = a_tip;
    m_height     = a_height;
    m_c.x        = std::sin(0.5 * a_angle * pi / 180.0);
    m_c.y        = std::cos(0.5 * a_angle * pi / 180.0);
    m_flipInside = a_flipInside;
  }

  /*!
    @brief Destructor -- does nothing
  */
  virtual ~ConeSDF() {}

  /*!
    @brief Implementation of the signed distance function.
    @param[in] a_point Position.
  */
  virtual T
  signedDistance(const Vec3T<T>& a_point) const noexcept override
  {
    const Vec3T<T> delta = a_point - m_tip;
    const T        dr    = sqrt(delta[0] * delta[0] + delta[1] * delta[1]);
    const T        dz    = delta[2];

    constexpr T zero = T(0.0);
    constexpr T one  = T(1.0);

    const Vec2T<T> q = m_height * Vec2T<T>(m_c.x / m_c.y, -1.0);
    const Vec2T<T> w = Vec2T<T>(dr, dz);
    const Vec2T<T> a = w - clamp(dot(w, q) / dot(q, q), zero, one) * q;
    const Vec2T<T> b = w - Vec2T<T>(q.x * clamp(w.x / q.x, zero, one), q.y);

    auto sign = [](const T& x) { return (x > zero) - (x < zero); };

    const T k    = sign(q.y);
    const T d    = std::min(dot(a, a), dot(b, b));
    const T s    = std::max(k * (w.x * q.y - w.y * q.x), k * (w.y - q.y));
    const T flip = m_flipInside ? -one : one;

    return flip * sqrt(d) * sign(s);
  }

protected:
  /*!
    @brief Tip position
  */
  Vec3T<T> m_tip;

  /*!
    @brief Sine/cosine of angle
  */
  Vec2T<T> m_c;

  /*!
    @brief Cone height
  */
  T m_height;

  /*!
    @brief Hook for making inside -> outside.
  */
  bool m_flipInside;
};

#include "EBGeometry_NamespaceFooter.hpp"

#endif
