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
#include <random>

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
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
constexpr const T
clamp(const T& v, const T& lo, const T& hi)
{
  return (v < lo) ? lo : (v > hi) ? hi : v;
}

/*!
  @brief Clamp function. Returns lo if v < lo and hi if v > hi. Otherwise
  returns v.
  @param[in] v  Value to be clamped.
  @param[in] lo Lower clamping value.
  @param[in] hi Higher clamping value.
*/
template <class T>
constexpr const Vec3T<T>
clamp(const Vec3T<T>& v, const Vec3T<T>& lo, const Vec3T<T>& hi)
{
  return Vec3T<T>(clamp(v[0], lo[0], hi[0]), clamp(v[1], lo[1], hi[1]), clamp(v[2], lo[2], hi[2]));
}

/*!
  @brief Ken Perlin's original permutation array. Use for initializing PerlinSDF
*/
constexpr static std::array<int, 256> s_perlinPermutationTable = {
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

/*!
  @brief Signed distance function for a plane
*/
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
  */
  PlaneSDF(const Vec3T<T>& a_point, const Vec3T<T>& a_normal)
  {
    m_point  = a_point;
    m_normal = a_normal / a_normal.length();
  }

  /*!
    @brief Signed distance function for sphere.
    @param[in] a_point Position.
  */
  virtual T
  signedDistance(const Vec3T<T>& a_point) const noexcept override
  {
    return dot((a_point - m_point), m_normal);
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
  */
  SphereSDF(const Vec3T<T>& a_center, const T& a_radius)
  {
    this->m_center = a_center;
    this->m_radius = a_radius;
  }

  /*!
    @brief Copy constructor
  */
  SphereSDF(const SphereSDF& a_other)
  {
    this->m_center = a_other.m_center;
    this->m_radius = a_other.m_radius;
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
    return (a_point - m_center).length() - m_radius;
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
  */
  BoxSDF(const Vec3T<T>& a_loCorner, const Vec3T<T>& a_hiCorner)
  {
    this->m_loCorner = a_loCorner;
    this->m_hiCorner = a_hiCorner;
  }

  /*!
    @brief Destructor (does nothing).
  */
  virtual ~BoxSDF()
  {}

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
  /*!
    @brief Low box corner
  */
  Vec3T<T> m_loCorner;

  /*!
    @brief High box corner
  */
  Vec3T<T> m_hiCorner;
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
  */
  TorusSDF(const Vec3T<T>& a_center, const T& a_majorRadius, const T& a_minorRadius)
  {
    this->m_center      = a_center;
    this->m_majorRadius = a_majorRadius;
    this->m_minorRadius = a_minorRadius;
  }

  /*!
    @brief Destructor (does nothing).
  */
  virtual ~TorusSDF()
  {}

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

    return d;
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
  */
  CylinderSDF(const Vec3T<T>& a_center1, const Vec3T<T>& a_center2, const T& a_radius)
  {
    this->m_center1 = a_center1;
    this->m_center2 = a_center2;
    this->m_radius  = a_radius;

    // Some derived quantities that are needed for SDF computations.
    m_center = (m_center2 + m_center1) * 0.5;
    m_length = (m_center2 - m_center1).length();
    m_axis   = (m_center2 - m_center1) / m_length;
  }

  /*!
    @brief Destructor (does nothing).
  */
  virtual ~CylinderSDF()
  {}

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

    return d;
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
    @brief Halfway between center1 and m_center2
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
  */
  InfiniteCylinderSDF(const Vec3T<T>& a_center, const T& a_radius, const size_t a_axis)
  {
    m_center = a_center;
    m_radius = a_radius;
    m_axis   = a_axis;
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

    const T d = delta.length() - m_radius;

    return d;
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
  */
  CapsuleSDF(const Vec3T<T>& a_tip1, const Vec3T<T> a_tip2, const T& a_radius)
  {
    const Vec3T<T> axis = (a_tip2 - a_tip1) / length(a_tip2 - a_tip1);
    m_center1           = a_tip1 + a_radius * axis;
    m_center2           = a_tip2 - a_radius * axis;
    m_radius            = a_radius;
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

    const T h = clamp(dot(v1, v2) / dot(v2, v2), T(0.0), T(1.0));
    const T d = length(v1 - h * v2) - m_radius;

    return d;
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
  */
  InfiniteConeSDF(const Vec3T<T>& a_tip, const T& a_angle)
  {
    constexpr T pi = 3.14159265358979323846;

    m_tip = a_tip;
    m_c.x = std::sin(0.5 * a_angle * pi / 180.0);
    m_c.y = std::cos(0.5 * a_angle * pi / 180.0);
  }

  /*!
    @brief Destructor -- does nothing
  */
  virtual ~InfiniteConeSDF()
  {}

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

    return d2;
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
  */
  ConeSDF(const Vec3T<T>& a_tip, const T& a_height, const T& a_angle)
  {
    constexpr T pi = 3.14159265358979323846;

    m_tip    = a_tip;
    m_height = a_height;
    m_c.x    = std::sin(0.5 * a_angle * pi / 180.0);
    m_c.y    = std::cos(0.5 * a_angle * pi / 180.0);
  }

  /*!
    @brief Destructor -- does nothing
  */
  virtual ~ConeSDF()
  {}

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

    const T k = sign(q.y);
    const T d = std::min(dot(a, a), dot(b, b));
    const T s = std::max(k * (w.x * q.y - w.y * q.x), k * (w.y - q.y));

    return sqrt(d) * sign(s);
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
};

/*!
  @brief Box of arbitrary dimensions centered at the origin, with rounded corners. 
*/
template <class T>
class RoundedBoxSDF : public SignedDistanceFunction<T>
{
public:
  /*!
    @brief Disallowed default constructor
  */
  RoundedBoxSDF() = delete;

  /*!
    @brief Full constructor. User inputs dimensions and corner curvature. 
    @note The extensions of the box in each direction 'dir' is a_dimensions[dir] + a_curvature. 
    @param[in] a_dimensions Box dimensions (width, length, height)
    @param[in] a_curvature Corner curvature. 
    @note Curvature must be > 0.0
  */
  RoundedBoxSDF(const Vec3T<T>& a_dimensions, const T a_curvature)
  {
    this->m_dimensions = 0.5 * a_dimensions;

    m_sphere = std::make_shared<SphereSDF<T>>(Vec3T<T>::zero(), a_curvature);
  }

  /*!
    @brief Destructor (does nothing).
  */
  virtual ~RoundedBoxSDF() noexcept
  {}

  /*!
    @brief Signed distance function for the rounded box
  */
  virtual T
  signedDistance(const Vec3T<T>& a_point) const noexcept override
  {
    return m_sphere->value(a_point - clamp(a_point, -m_dimensions, m_dimensions));
  }

protected:
  /*!
    @brief Sphere at origin with radius a_curvature
  */
  std::shared_ptr<SphereSDF<T>> m_sphere;

  /*!
    @brief Box dimensions
  */
  Vec3T<T> m_dimensions;
};

/*!
  @brief Ken Perlins gradient noise function
*/
template <class T>
class PerlinSDF : public SignedDistanceFunction<T>
{
public:
  /*!
    @brief Full constructor. 
    @param[in] a_noiseAmplitude Noise amplitude
    @param[in] a_noiseFrequency Spatial noise frequency along the three Cartesian axes
    @param[in] a_noisePersistence Noise amplitude drop-off and frequency increase for octave noise. Should be < 1
    @param[in] a_noiseOctaves Number of octaves. Should be > 0
  */
  PerlinSDF(const T            a_noiseAmplitude,
            const Vec3T<T>     a_noiseFrequency,
            const T            a_noisePersistence,
            const unsigned int a_noiseOctaves) noexcept
  {
    m_noiseAmplitude   = a_noiseAmplitude;
    m_noiseFrequency   = a_noiseFrequency;
    m_noisePersistence = std::min(1.0, a_noisePersistence);
    m_noiseOctaves     = std::max((unsigned int)1, a_noiseOctaves);

    // By default, use Ken Perlin's original permutation table
    for (int i = 0; i < 256; i++) {
      m_permutationTable[i]       = s_perlinPermutationTable[i];
      m_permutationTable[i + 256] = s_perlinPermutationTable[i];
    }

#if 1 // Development code
    std::random_device rd;
    std::mt19937       g(0);

    this->shuffle(g);
#endif
  }

  /*!
    @brief Destructor (does nothing)
  */
  virtual ~PerlinSDF() noexcept
  {}

  /*!
    @brief Signed distance function. Generates a distance on [0,curAmplitude]
    @param[in] a_point Input point
  */
  virtual T
  signedDistance(const Vec3T<T>& a_point) const noexcept override
  {
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

  /*!
    @brief Shuffle the permutation with the input RNG. 
    @details URNG should be a uniform random number generator, e.g. 
    @param[in] g Uniform random number generator (e.g., std::mt19937)
    @note When using parallel calculations it is exceptionally important that the input RNG is the same across all threads/ranks. Otherwise, the
    user must manually ensure that the permutation table is the same. Failure to do so implies that each thread/rank generates it's own gradient noise
    and there is correspondingly no single geometry. 
  */
  template <class URNG>
  void
  shuffle(URNG& g) noexcept
  {

    for (unsigned int i = 0; i < 256; i++) {
      m_permutationTable[i] = i;
    }

    std::shuffle(m_permutationTable.begin(), m_permutationTable.begin() + 256, g);

    for (int i = 0; i < 256; i++) {
      m_permutationTable[i + 256] = m_permutationTable[i];
    }
  }

  /*!
    @brief Get the internal permutation table
    @return m_permutationTable.
  */
  std::array<int, 512>&
  getPermutationTable() noexcept
  {
    return m_permutationTable;
  }

protected:
  /*!
    @brief Noise frequency
  */
  Vec3T<T> m_noiseFrequency;

  /*!
    @brief Noise amplitude
  */
  T m_noiseAmplitude;

  /*!
    @brief Noise persistence
  */
  T m_noisePersistence;

  /*!
    @brief Permutation table
  */
  std::array<T, 512> m_permutationTable;

  /*!
    @brief Number of noise octaves
  */
  unsigned int m_noiseOctaves;

  /*!
    @brief Ken Perlin's lerp function
    @param[in] t Input parameter
    @param[in] a Input parameter
    @param[in] b Input parameter
  */
  virtual T
  lerp(const T t, const T a, const T b) const noexcept
  {
    return a + t * (b - a);
  };

  /*!
    @brief Ken Perlin's fade function
    @param[in] t Input parameter
  */
  virtual T
  fade(const T t) const noexcept
  {
    return t * t * t * (t * (t * 6 - 15) + 10);
  };

  /*!
    @brief Ken Perlins grad function
    @param[in] hash Input parameter
    @param[in] x Input parameter
    @param[in] y Input parameter
    @param[in] z Input parameter
  */
  T
  grad(const int hash, const T x, const T y, const T z) const noexcept
  {
    const int h = hash & 15;
    const T   u = h < 8 ? x : y;
    const T   v = h < 4 ? y : h == 12 || h == 14 ? x : y;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
  }

  /*!
    @brief Octave noise function
    @param[in] a_point Input point
  */
  T
  noise(const Vec3T<T>& a_point) const noexcept
  {
    // Lower cube corner
    const int X = (int)std::floor(a_point[0]) & 255;
    const int Y = (int)std::floor(a_point[1]) & 255;
    const int Z = (int)std::floor(a_point[2]) & 255;

    // Relative distance wrt lower cube corner
    const double x = a_point[0] - std::floor(a_point[0]);
    const double y = a_point[1] - std::floor(a_point[1]);
    const double z = a_point[2] - std::floor(a_point[2]);

    // Fade curves
    const double u = fade(x);
    const double v = fade(y);
    const double w = fade(z);

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

#include "EBGeometry_NamespaceFooter.hpp"

#endif
