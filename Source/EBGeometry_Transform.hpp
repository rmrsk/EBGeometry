// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Transform.hpp
 * @brief  Various transformations for implicit functions and distance fields
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_TRANSFORM_HPP
#define EBGEOMETRY_TRANSFORM_HPP

// Std includes
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_ImplicitFunction.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Convenience function for taking the complement of an implicit function
 * @tparam T Floating-point precision
 * @param[in] a_implicitFunction Input implicit function
 * @return New implicit function representing the complement (negated value)
 */
template <class T>
[[nodiscard]] std::shared_ptr<ImplicitFunction<T>>
Complement(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction);

/**
 * @brief Convenience function for translating an implicit function
 * @tparam T Floating-point precision
 * @param[in] a_implicitFunction Input implicit function to be translated
 * @param[in] a_shift Distance to shift
 * @return New implicit function representing the translated input
 */
template <class T>
[[nodiscard]] std::shared_ptr<ImplicitFunction<T>>
Translate(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const Vec3T<T>& a_shift);

/**
 * @brief Convenience function for rotating an implicit function.
 * @tparam T Floating-point precision
 * @param[in] a_implicitFunction Input implicit function to be rotated.
 * @param[in] a_angle Angle to be rotated by (in degrees)
 * @param[in] a_axis Axis to rotate about (0=x, 1=y, 2=z)
 * @return New implicit function representing the rotated input
 */
template <class T>
[[nodiscard]] std::shared_ptr<ImplicitFunction<T>>
Rotate(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_angle, const size_t a_axis);

/**
 * @brief Convenience function for scaling an implicit function.
 * @tparam T Floating-point precision
 * @param[in] a_implicitFunction Input implicit function to be scaled.
 * @param[in] a_scale Scaling factor (must be non-zero)
 * @return New implicit function representing the uniformly scaled input
 */
template <class T>
[[nodiscard]] std::shared_ptr<ImplicitFunction<T>>
Scale(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_scale);

/**
 * @brief Convenience function for offsetting an implicit function
 * @tparam T Floating-point precision
 * @param[in] a_implicitFunction Input implicit function to be offset
 * @param[in] a_offset Offset distance subtracted from the function value
 * @return New implicit function representing the offset (grown/shrunk) input
 */
template <class T>
[[nodiscard]] std::shared_ptr<ImplicitFunction<T>>
Offset(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_offset);

/**
 * @brief Convenience function for creating a shell out of an implicit function
 * @tparam T Floating-point precision
 * @param[in] a_implicitFunction Input implicit function to be shelled.
 * @param[in] a_delta Shell thickness
 * @return New implicit function representing the annular (hollow) version of the input
 */
template <class T>
[[nodiscard]] std::shared_ptr<ImplicitFunction<T>>
Annular(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_delta);

/**
 * @brief Convenience function for blurring an implicit function
 * @tparam T Floating-point precision
 * @param[in] a_implicitFunction Input implicit function to be blurred
 * @param[in] a_blurDistance Smoothing distance
 * @return New implicit function representing the blurred input
 */
template <class T>
[[nodiscard]] std::shared_ptr<ImplicitFunction<T>>
Blur(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_blurDistance);

/**
 * @brief Convenience function for mollification with an input sphere.
 * @tparam T Floating-point precision
 * @param[in] a_implicitFunction Input implicit function to be mollified
 * @param[in] a_dist Mollification distance.
 * @param[in] a_mollifierSamples Number of samples per dimension for the convolution kernel (must be >= 1).
 * @return New implicit function representing the mollified (smoothed) input
 */
template <class T>
[[nodiscard]] std::shared_ptr<ImplicitFunction<T>>
Mollify(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction,
        const T                                     a_dist,
        const size_t                                a_mollifierSamples = 2);

/**
 * @brief Convenience function for elongating (stretching) an implicit function
 * @tparam T Floating-point precision
 * @param[in] a_implicitFunction Implicit function to be elongated
 * @param[in] a_elongation Per-axis elongation amounts
 * @return New implicit function representing the stretched input
 */
template <class T>
[[nodiscard]] std::shared_ptr<ImplicitFunction<T>>
Elongate(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const Vec3T<T>& a_elongation);

/**
 * @brief Convenience function for reflecting an implicit function
 * @tparam T Floating-point precision
 * @param[in] a_implicitFunction Implicit function to be reflected
 * @param[in] a_reflectPlane Plane to reflect across (0=yz-plane, 1=xz-plane, 2=xy-plane).
 * @return New implicit function representing the reflected input
 */
template <class T>
[[nodiscard]] std::shared_ptr<ImplicitFunction<T>>
Reflect(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const size_t& a_reflectPlane);

/**
 * @brief Complemented implicit function
 * @tparam T Floating-point precision
 */
template <class T>
class ComplementIF : public ImplicitFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "ComplementIF requires a floating-point T");

public:
  /**
   * @brief No weak construction for this one
   */
  ComplementIF() = delete;

  /**
   * @brief Copy constructor.
   */
  ComplementIF(const ComplementIF& a_other) noexcept = default;

  /**
   * @brief Move constructor.
   */
  ComplementIF(ComplementIF&& a_other) noexcept = default;

  /**
   * @brief Full constructor
   * @param[in] a_implicitFunction Input implicit function
   */
  ComplementIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction) noexcept;

  /**
   * @brief Destructor (does nothing)
   */
  ~ComplementIF() override = default;

  /**
   * @brief Copy assignment operator.
   */
  ComplementIF&
  operator=(const ComplementIF& a_other) noexcept = default;

  /**
   * @brief Move assignment operator.
   */
  ComplementIF&
  operator=(ComplementIF&& a_other) noexcept = default;

  /**
   * @brief Value function
   * @param[in] a_point Query point
   * @return Negated value of the wrapped implicit function at a_point
   */
  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
   * @brief Implicit function
   */
  std::shared_ptr<ImplicitFunction<T>> m_implicitFunction = nullptr;
};

/**
 * @brief Translated implicit function
 * @tparam T Floating-point precision
 */
template <class T>
class TranslateIF : public ImplicitFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "TranslateIF requires a floating-point T");

public:
  /**
   * @brief No weak construction for this one
   */
  TranslateIF() = delete;

  /**
   * @brief Copy constructor.
   */
  TranslateIF(const TranslateIF& a_other) noexcept = default;

  /**
   * @brief Move constructor.
   */
  TranslateIF(TranslateIF&& a_other) noexcept = default;

  /**
   * @brief Full constructor
   * @param[in] a_implicitFunction Input implicit function to be translated
   * @param[in] a_shift Distance to shift
   */
  TranslateIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const Vec3T<T>& a_shift) noexcept;

  /**
   * @brief Destructor (does nothing)
   */
  ~TranslateIF() override = default;

  /**
   * @brief Copy assignment operator.
   */
  TranslateIF&
  operator=(const TranslateIF& a_other) noexcept = default;

  /**
   * @brief Move assignment operator.
   */
  TranslateIF&
  operator=(TranslateIF&& a_other) noexcept = default;

  /**
   * @brief Value function
   * @param[in] a_point Query point
   * @return Value of the wrapped implicit function evaluated at a_point shifted by -m_shift
   */
  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
   * @brief Underlying implicit function
   */
  std::shared_ptr<ImplicitFunction<T>> m_implicitFunction = nullptr;

  /**
   * @brief Input point translate.
   */
  Vec3T<T> m_shift;
};

/**
 * @brief Rotated implicit function. Rotates an implicit function about an axis.
 * @tparam T Floating-point precision
 */
template <class T>
class RotateIF : public ImplicitFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "RotateIF requires a floating-point T");

public:
  /**
   * @brief No weak construction
   */
  RotateIF() = delete;

  /**
   * @brief Copy constructor.
   */
  RotateIF(const RotateIF& a_other) noexcept = default;

  /**
   * @brief Move constructor.
   */
  RotateIF(RotateIF&& a_other) noexcept = default;

  /**
   * @brief Full constructor. Rotates the wrapped implicit function by a_angle degrees about a_axis.
   * @param[in] a_implicitFunction  Input implicit function.
   * @param[in] a_angle Angle to rotate (in degrees)
   * @param[in] a_axis Axis to rotate about (0=x, 1=y, 2=z)
   */
  RotateIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction,
           const T                                     a_angle,
           const size_t                                a_axis) noexcept;

  /**
   * @brief Destructor
   */
  ~RotateIF() override = default;

  /**
   * @brief Copy assignment operator.
   */
  RotateIF&
  operator=(const RotateIF& a_other) noexcept = default;

  /**
   * @brief Move assignment operator.
   */
  RotateIF&
  operator=(RotateIF&& a_other) noexcept = default;

  /**
   * @brief Value function with rotation applied to the query point.
   * @param[in] a_point Query point
   * @return Value of the wrapped implicit function evaluated at the inversely rotated point
   */
  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
   * @brief Underlying implicit function.
   */
  std::shared_ptr<ImplicitFunction<T>> m_implicitFunction = nullptr;

  /**
   * @brief Axis to rotate about
   */
  size_t m_axis;

  /**
   * @brief Angle to rotate.
   */
  T m_angle;

  /**
   * @brief Parameter in rotation matrix. Stored for efficiency.
   */
  T m_cosAngle;

  /**
   * @brief Parameter in rotation matrix. Stored for efficiency.
   */
  T m_sinAngle;
};

/**
 * @brief Offset implicit function. Offsets (grows or shrinks) the implicit function by a constant.
 * @tparam T Floating-point precision
 */
template <class T>
class OffsetIF : public ImplicitFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "OffsetIF requires a floating-point T");

public:
  /**
   * @brief Disallowed weak construction
   */
  OffsetIF() = delete;

  /**
   * @brief Copy constructor.
   */
  OffsetIF(const OffsetIF& a_other) noexcept = default;

  /**
   * @brief Move constructor.
   */
  OffsetIF(OffsetIF&& a_other) noexcept = default;

  /**
   * @brief Full constructor. Subtracts a_offset from the wrapped function value.
   * @param[in] a_implicitFunction Input implicit function.
   * @param[in] a_offset Offset value.
   */
  OffsetIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_offset) noexcept;

  /**
   * @brief Destructor
   */
  ~OffsetIF() override = default;

  /**
   * @brief Copy assignment operator.
   */
  OffsetIF&
  operator=(const OffsetIF& a_other) noexcept = default;

  /**
   * @brief Move assignment operator.
   */
  OffsetIF&
  operator=(OffsetIF&& a_other) noexcept = default;

  /**
   * @brief Value function with offset applied.
   * @param[in] a_point Query point
   * @return Value of the wrapped implicit function minus the stored offset
   */
  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
   * @brief Underlying implicit function.
   */
  std::shared_ptr<ImplicitFunction<T>> m_implicitFunction = nullptr;

  /**
   * @brief Offset value.
   */
  T m_offset;
};

/**
 * @brief Uniformly scaled implicit function.
 * @tparam T Floating-point precision
 */
template <class T>
class ScaleIF : public ImplicitFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "ScaleIF requires a floating-point T");

public:
  /**
   * @brief Disallowed weak construction
   */
  ScaleIF() = delete;

  /**
   * @brief Copy constructor.
   */
  ScaleIF(const ScaleIF& a_other) noexcept = default;

  /**
   * @brief Move constructor.
   */
  ScaleIF(ScaleIF&& a_other) noexcept = default;

  /**
   * @brief Full constructor.
   * @param[in] a_implicitFunction Implicit function to be scaled.
   * @param[in] a_scale            Scaling factor (must be non-zero).
   */
  ScaleIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_scale) noexcept;

  /**
   * @brief Destructor
   */
  ~ScaleIF() override = default;

  /**
   * @brief Copy assignment operator.
   */
  ScaleIF&
  operator=(const ScaleIF& a_other) noexcept = default;

  /**
   * @brief Move assignment operator.
   */
  ScaleIF&
  operator=(ScaleIF&& a_other) noexcept = default;

  /**
   * @brief Value function.
   * @param[in] a_point Query point
   * @return Signed distance of the wrapped function evaluated at a_point/m_scale, multiplied by m_scale
   */
  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
   * @brief Original implicit function.
   */
  std::shared_ptr<ImplicitFunction<T>> m_implicitFunction = nullptr;

  /**
   * @brief Scaling factor.
   */
  T m_scale;
};

/**
 * @brief Annular implicit function. Creates a shell out of the implicit function.
 * @tparam T Floating-point precision
 */
template <class T>
class AnnularIF : public ImplicitFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "AnnularIF requires a floating-point T");

public:
  /**
   * @brief Disallowed weak construction
   */
  AnnularIF() = delete;

  /**
   * @brief Copy constructor.
   */
  AnnularIF(const AnnularIF& a_other) noexcept = default;

  /**
   * @brief Move constructor.
   */
  AnnularIF(AnnularIF&& a_other) noexcept = default;

  /**
   * @brief Full constructor.
   * @param[in] a_implicitFunction  Input implicit function.
   * @param[in] a_delta Shell thickness (at least if the implicit function is also signed distance)
   */
  AnnularIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_delta) noexcept;

  /**
   * @brief Destructor
   */
  ~AnnularIF() override = default;

  /**
   * @brief Copy assignment operator.
   */
  AnnularIF&
  operator=(const AnnularIF& a_other) noexcept = default;

  /**
   * @brief Move assignment operator.
   */
  AnnularIF&
  operator=(AnnularIF&& a_other) noexcept = default;

  /**
   * @brief Value function.
   * @param[in] a_point Query point
   * @return abs(f(a_point)) - m_delta, hollowing out the shape at distance m_delta from the surface
   */
  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
   * @brief Original implicit function.
   */
  std::shared_ptr<const ImplicitFunction<T>> m_implicitFunction = nullptr;

  /**
   * @brief Shell thickness.
   */
  T m_delta;
};

/**
 * @brief Blurred/interpolated implicit function — can be used for smoothing.
 * @details Passes the input implicit function through a 3D box filter sampled at
 * face centres, edge centres, and corners of a cube with half-side m_blurDistance.
 * The per-sample weights are products of alpha^(3-n) * ((1-alpha)/2)^n where n is
 * the number of displaced axes.
 * @tparam T Floating-point precision
 */
template <class T>
class BlurIF : public ImplicitFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "BlurIF requires a floating-point T");

public:
  /**
   * @brief Disallowed weak construction
   */
  BlurIF() = delete;

  /**
   * @brief Copy constructor.
   */
  BlurIF(const BlurIF& a_other) noexcept = default;

  /**
   * @brief Move constructor.
   */
  BlurIF(BlurIF&& a_other) noexcept = default;

  /**
   * @brief Full constructor.
   * @param[in] a_implicitFunction Input implicit function
   * @param[in] a_blurDistance     Blur distance
   * @param[in] a_alpha            Blend factor in [0, 1]; 1 = no blur, 0 = full blur.
   */
  BlurIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction,
         const T                                     a_blurDistance,
         const T                                     a_alpha = 0.5) noexcept;

  /**
   * @brief Destructor
   */
  ~BlurIF() override = default;

  /**
   * @brief Copy assignment operator.
   */
  BlurIF&
  operator=(const BlurIF& a_other) noexcept = default;

  /**
   * @brief Move assignment operator.
   */
  BlurIF&
  operator=(BlurIF&& a_other) noexcept = default;

  /**
   * @brief Value function
   * @param[in] a_point Query point
   * @return Blurred implicit function value at a_point
   */
  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
   * @brief Original implicit function
   */
  std::shared_ptr<ImplicitFunction<T>> m_implicitFunction = nullptr;

  /**
   * @brief Blur distance.
   */
  T m_blurDistance;

  /**
   * @brief Alpha factor
   */
  T m_alpha;
};

/**
 * @brief Mollified implicit function.
 * @details Convolves the wrapped implicit function with a pre-sampled mollifier kernel
 * (typically a sphere SDF) over a uniform 3-D grid of a_numPoints^3 sample offsets
 * spanning [-a_maxValue, a_maxValue]^3.  Weights are normalized to sum to 1.
 * @tparam T Floating-point precision
 */
template <class T>
class MollifyIF : public ImplicitFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "MollifyIF requires a floating-point T");

public:
  /**
   * @brief Disallowed weak construction
   */
  MollifyIF() = delete;

  /**
   * @brief Copy constructor.
   */
  MollifyIF(const MollifyIF& a_other) = default;

  /**
   * @brief Move constructor.
   */
  MollifyIF(MollifyIF&& a_other) noexcept = default;

  /**
   * @brief Full constructor
   * @param[in] a_implicitFunction  Input implicit function.
   * @param[in] a_mollifier         Mollifier implicit function (e.g. a sphere SDF)
   * @param[in] a_maxValue          Half-width of the sampling region; must be > 0 for any smoothing to occur.
   * @param[in] a_numPoints         Number of sample points per axis; must be >= 2 for any smoothing to occur.
   */
  MollifyIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction,
            const std::shared_ptr<ImplicitFunction<T>>& a_mollifier,
            const T                                     a_maxValue,
            const size_t                                a_numPoints) noexcept;

  /**
   * @brief Destructor
   */
  ~MollifyIF() override = default;

  /**
   * @brief Copy assignment operator.
   */
  MollifyIF&
  operator=(const MollifyIF& a_other) = default;

  /**
   * @brief Move assignment operator.
   */
  MollifyIF&
  operator=(MollifyIF&& a_other) noexcept = default;

  /**
   * @brief Value function
   * @param[in] a_point Query point
   * @return Weighted convolution of the wrapped function with the pre-sampled mollifier kernel
   */
  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
   * @brief Original implicit function.
   */
  std::shared_ptr<ImplicitFunction<T>> m_implicitFunction = nullptr;

  /**
   * @brief Mollifier
   */
  std::shared_ptr<ImplicitFunction<T>> m_mollifier = nullptr;

  /**
   * @brief Mollifier Weights
   */
  std::vector<std::pair<Vec3T<T>, T>> m_sampledMollifier;
};

/**
 * @brief Implicit function which is an elongation of another implicit function along each axis.
 * @details Elongation clamps the query point component-wise to [-elongation, +elongation] and
 * subtracts the clamped value before evaluating the wrapped function — effectively stretching
 * the shape without changing its cross-section profile.
 * @tparam T Floating-point precision
 */
template <class T>
class ElongateIF : public ImplicitFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "ElongateIF requires a floating-point T");

public:
  /**
   * @brief Disallowed weak construction
   */
  ElongateIF() = delete;

  /**
   * @brief Copy constructor.
   */
  ElongateIF(const ElongateIF& a_other) noexcept = default;

  /**
   * @brief Move constructor.
   */
  ElongateIF(ElongateIF&& a_other) noexcept = default;

  /**
   * @brief Full constructor.
   * @param[in] a_implicitFunction Implicit function to be stretched
   * @param[in] a_elongation Per-axis elongation amounts (non-negative values expected)
   */
  ElongateIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const Vec3T<T>& a_elongation) noexcept;

  /**
   * @brief Destructor
   */
  ~ElongateIF() override = default;

  /**
   * @brief Copy assignment operator.
   */
  ElongateIF&
  operator=(const ElongateIF& a_other) noexcept = default;

  /**
   * @brief Move assignment operator.
   */
  ElongateIF&
  operator=(ElongateIF&& a_other) noexcept = default;

  /**
   * @brief Value function
   * @param[in] a_point Query point
   * @return Value of the wrapped function evaluated at a_point clamped by m_elongation
   */
  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
   * @brief Underlying implicit function to be elongated
   */
  std::shared_ptr<const ImplicitFunction<T>> m_implicitFunction = nullptr;

  /**
   * @brief Elongation
   */
  Vec3T<T> m_elongation;
};

/**
 * @brief Implicit function which is a reflection of another implicit function.
 * @tparam T Floating-point precision
 */
template <class T>
class ReflectIF : public ImplicitFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "ReflectIF requires a floating-point T");

public:
  /**
   * @brief Disallowed weak construction
   */
  ReflectIF() = delete;

  /**
   * @brief Copy constructor.
   */
  ReflectIF(const ReflectIF& a_other) noexcept = default;

  /**
   * @brief Move constructor.
   */
  ReflectIF(ReflectIF&& a_other) noexcept = default;

  /**
   * @brief Full constructor. Reflects across the specified plane.
   * @param[in] a_implicitFunction Implicit function to be reflected
   * @param[in] a_reflectPlane Plane to reflect across (0=yz-plane flips x, 1=xz-plane flips y, 2=xy-plane flips z)
   */
  ReflectIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const size_t& a_reflectPlane) noexcept;

  /**
   * @brief Destructor
   */
  ~ReflectIF() override = default;

  /**
   * @brief Copy assignment operator.
   */
  ReflectIF&
  operator=(const ReflectIF& a_other) noexcept = default;

  /**
   * @brief Move assignment operator.
   */
  ReflectIF&
  operator=(ReflectIF&& a_other) noexcept = default;

  /**
   * @brief Value function
   * @param[in] a_point Query point
   * @return Value of the wrapped function evaluated at a_point with the reflected coordinate flipped
   */
  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
   * @brief Underlying implicit function to be reflected
   */
  std::shared_ptr<const ImplicitFunction<T>> m_implicitFunction = nullptr;

  /**
   * @brief Reflection parameters
   */
  Vec3T<T> m_reflectParams;
};

} // namespace EBGeometry

#include "EBGeometry_TransformImplem.hpp"

#endif
