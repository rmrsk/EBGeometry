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
#include <cmath>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_ImplicitFunction.hpp"
#include "EBGeometry_Macros.hpp"
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
 * @brief Distance-formula trait for the complement transform.
 * @details Single source of truth for the complement's value remapping: the complement negates the
 * wrapped function value. This math is reused verbatim by the tape interpreter, so it must live in the
 * HOST_DEVICE trait static rather than inline in ComplementIF::value.
 * @tparam T Floating-point precision.
 */
template <class T>
struct ComplementOp
{
  /**
   * @brief Stored parameters for the complement transform (none).
   */
  struct Params
  {
  };

  /**
   * @brief The complement of a signed distance function is again a signed distance function.
   */
  static constexpr bool preservesSignedDistance = true;

  /**
   * @brief Remap the child value to its complement.
   * @param[in] a_params     Complement parameters (unused).
   * @param[in] a_childValue Value of the wrapped implicit function.
   * @return The negated child value.
   */
  EBGEOMETRY_HOST_DEVICE static T
  mapValue(const Params& a_params, T a_childValue) noexcept
  {
    (void)a_params;

    return -a_childValue;
  }
};

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
   * @param[in] a_other Other ComplementIF.
   */
  ComplementIF(const ComplementIF& a_other) noexcept = default;

  /**
   * @brief Move constructor.
   * @param[in, out] a_other Other ComplementIF.
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
   * @param[in] a_other Other ComplementIF.
   * @return Reference to (*this).
   */
  ComplementIF&
  operator=(const ComplementIF& a_other) noexcept = default;

  /**
   * @brief Move assignment operator.
   * @param[in, out] a_other Other ComplementIF.
   * @return Reference to (*this).
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

  /**
   * @brief Transform parameters (empty for the complement).
   */
  typename ComplementOp<T>::Params m_params;
};

/**
 * @brief Distance-formula trait for the translation transform.
 * @details Single source of truth for the translation's coordinate remapping: the query point is
 * shifted by -shift before evaluating the wrapped function. Reused verbatim by the tape interpreter.
 * @tparam T Floating-point precision.
 */
template <class T>
struct TranslateOp
{
  /**
   * @brief Stored parameters for the translation transform.
   */
  struct Params
  {
    Vec3T<T> shift = Vec3T<T>::zeros(); ///< Translation amount.
  };

  /**
   * @brief A rigid translation of a signed distance function is again a signed distance function.
   */
  static constexpr bool preservesSignedDistance = true;

  /**
   * @brief Remap the query point by the inverse translation.
   * @param[in] a_params Translation parameters.
   * @param[in] a_point  Query point.
   * @return The query point shifted by -shift.
   */
  EBGEOMETRY_HOST_DEVICE static Vec3T<T>
  mapPoint(const Params& a_params, const Vec3T<T>& a_point) noexcept
  {
    return a_point - a_params.shift;
  }
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
   * @param[in] a_other Other TranslateIF.
   */
  TranslateIF(const TranslateIF& a_other) noexcept = default;

  /**
   * @brief Move constructor.
   * @param[in, out] a_other Other TranslateIF.
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
   * @param[in] a_other Other TranslateIF.
   * @return Reference to (*this).
   */
  TranslateIF&
  operator=(const TranslateIF& a_other) noexcept = default;

  /**
   * @brief Move assignment operator.
   * @param[in, out] a_other Other TranslateIF.
   * @return Reference to (*this).
   */
  TranslateIF&
  operator=(TranslateIF&& a_other) noexcept = default;

  /**
   * @brief Value function
   * @param[in] a_point Query point
   * @return Value of the wrapped implicit function evaluated at a_point shifted by -m_params.shift
   */
  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
   * @brief Underlying implicit function
   */
  std::shared_ptr<ImplicitFunction<T>> m_implicitFunction = nullptr;

  /**
   * @brief Transform parameters (translation shift).
   */
  typename TranslateOp<T>::Params m_params;
};

/**
 * @brief Distance-formula trait for the rotation transform.
 * @details Single source of truth for the rotation's coordinate remapping: the query point is rotated
 * about a Cartesian axis by the pre-computed cos/sin of the rotation angle before evaluating the
 * wrapped function. Reused verbatim by the tape interpreter.
 * @tparam T Floating-point precision.
 */
template <class T>
struct RotateOp
{
  /**
   * @brief Stored parameters for the rotation transform.
   */
  struct Params
  {
    size_t axis     = 0;    ///< Axis to rotate about (0=x, 1=y, 2=z).
    T      cosAngle = T(1); ///< Cosine of the rotation angle.
    T      sinAngle = T(0); ///< Sine of the rotation angle.
  };

  /**
   * @brief A rigid rotation of a signed distance function is again a signed distance function.
   */
  static constexpr bool preservesSignedDistance = true;

  /**
   * @brief Remap the query point by the rotation.
   * @param[in] a_params Rotation parameters.
   * @param[in] a_point  Query point.
   * @return The rotated query point.
   */
  EBGEOMETRY_HOST_DEVICE static Vec3T<T>
  mapPoint(const Params& a_params, const Vec3T<T>& a_point) noexcept
  {
    const T& x = a_point[0];
    const T& y = a_point[1];
    const T& z = a_point[2];

    Vec3T<T> rotatedPoint = a_point;

    switch (a_params.axis) {
    case 0: {
      rotatedPoint[1] = y * a_params.cosAngle + z * a_params.sinAngle;
      rotatedPoint[2] = -y * a_params.sinAngle + z * a_params.cosAngle;

      break;
    }
    case 1: {
      rotatedPoint[0] = x * a_params.cosAngle - z * a_params.sinAngle;
      rotatedPoint[2] = x * a_params.sinAngle + z * a_params.cosAngle;

      break;
    }
    case 2: {
      rotatedPoint[0] = x * a_params.cosAngle + y * a_params.sinAngle;
      rotatedPoint[1] = -x * a_params.sinAngle + y * a_params.cosAngle;

      break;
    }
    default: {
      EBGEOMETRY_EXPECT(false && "RotateOp: axis must be 0, 1, or 2");

      break;
    }
    }

    return rotatedPoint;
  }
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
   * @param[in] a_other Other RotateIF.
   */
  RotateIF(const RotateIF& a_other) noexcept = default;

  /**
   * @brief Move constructor.
   * @param[in, out] a_other Other RotateIF.
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
   * @param[in] a_other Other RotateIF.
   * @return Reference to (*this).
   */
  RotateIF&
  operator=(const RotateIF& a_other) noexcept = default;

  /**
   * @brief Move assignment operator.
   * @param[in, out] a_other Other RotateIF.
   * @return Reference to (*this).
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
   * @brief Transform parameters (rotation axis and pre-computed cos/sin of the angle).
   */
  typename RotateOp<T>::Params m_params;
};

/**
 * @brief Distance-formula trait for the offset transform.
 * @details Single source of truth for the offset's value remapping: a constant offset is subtracted
 * from the wrapped function value. Reused verbatim by the tape interpreter.
 * @tparam T Floating-point precision.
 */
template <class T>
struct OffsetOp
{
  /**
   * @brief Stored parameters for the offset transform.
   */
  struct Params
  {
    T offset = T(0); ///< Offset subtracted from the wrapped value.
  };

  /**
   * @brief Offsetting a signed distance function by a constant preserves the signed distance property.
   */
  static constexpr bool preservesSignedDistance = true;

  /**
   * @brief Remap the child value by subtracting the offset.
   * @param[in] a_params     Offset parameters.
   * @param[in] a_childValue Value of the wrapped implicit function.
   * @return The child value minus the offset.
   */
  EBGEOMETRY_HOST_DEVICE static T
  mapValue(const Params& a_params, T a_childValue) noexcept
  {
    return a_childValue - a_params.offset;
  }
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
   * @param[in] a_other Other OffsetIF.
   */
  OffsetIF(const OffsetIF& a_other) noexcept = default;

  /**
   * @brief Move constructor.
   * @param[in, out] a_other Other OffsetIF.
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
   * @param[in] a_other Other OffsetIF.
   * @return Reference to (*this).
   */
  OffsetIF&
  operator=(const OffsetIF& a_other) noexcept = default;

  /**
   * @brief Move assignment operator.
   * @param[in, out] a_other Other OffsetIF.
   * @return Reference to (*this).
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
   * @brief Transform parameters (offset value).
   */
  typename OffsetOp<T>::Params m_params;
};

/**
 * @brief Distance-formula trait for the uniform-scaling transform.
 * @details Single source of truth for the scale's remapping: the query point is divided by the scale
 * factor before evaluating the wrapped function and the result is multiplied back by the scale factor.
 * Reused verbatim by the tape interpreter.
 * @tparam T Floating-point precision.
 */
template <class T>
struct ScaleOp
{
  /**
   * @brief Stored parameters for the scaling transform.
   */
  struct Params
  {
    T scale = T(1); ///< Uniform scaling factor (non-zero).
  };

  /**
   * @brief A uniform scaling of a signed distance function is again a signed distance function.
   */
  static constexpr bool preservesSignedDistance = true;

  /**
   * @brief Remap the query point by the inverse scaling.
   * @param[in] a_params Scaling parameters.
   * @param[in] a_point  Query point.
   * @return The query point divided by the scale factor.
   */
  EBGEOMETRY_HOST_DEVICE static Vec3T<T>
  mapPoint(const Params& a_params, const Vec3T<T>& a_point) noexcept
  {
    return a_point / a_params.scale;
  }

  /**
   * @brief Post-scale the child value.
   * @param[in] a_params     Scaling parameters.
   * @param[in] a_childValue Value of the wrapped implicit function.
   * @return The child value multiplied by the scale factor.
   */
  EBGEOMETRY_HOST_DEVICE static T
  mapValue(const Params& a_params, T a_childValue) noexcept
  {
    return a_childValue * a_params.scale;
  }
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
   * @param[in] a_other Other ScaleIF.
   */
  ScaleIF(const ScaleIF& a_other) noexcept = default;

  /**
   * @brief Move constructor.
   * @param[in, out] a_other Other ScaleIF.
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
   * @param[in] a_other Other ScaleIF.
   * @return Reference to (*this).
   */
  ScaleIF&
  operator=(const ScaleIF& a_other) noexcept = default;

  /**
   * @brief Move assignment operator.
   * @param[in, out] a_other Other ScaleIF.
   * @return Reference to (*this).
   */
  ScaleIF&
  operator=(ScaleIF&& a_other) noexcept = default;

  /**
   * @brief Value function.
   * @param[in] a_point Query point
   * @return Signed distance of the wrapped function evaluated at a_point/m_params.scale, multiplied by m_params.scale
   */
  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
   * @brief Original implicit function.
   */
  std::shared_ptr<ImplicitFunction<T>> m_implicitFunction = nullptr;

  /**
   * @brief Transform parameters (scaling factor).
   */
  typename ScaleOp<T>::Params m_params;
};

/**
 * @brief Distance-formula trait for the annular (shell) transform.
 * @details Single source of truth for the annular value remapping: the absolute value of the wrapped
 * function is taken and the shell thickness is subtracted, hollowing the shape into a shell. Reused
 * verbatim by the tape interpreter.
 * @tparam T Floating-point precision.
 */
template <class T>
struct AnnularOp
{
  /**
   * @brief Stored parameters for the annular transform.
   */
  struct Params
  {
    T delta = T(0); ///< Shell thickness.
  };

  /**
   * @brief The annular map of a signed distance function is again a signed distance function.
   */
  static constexpr bool preservesSignedDistance = true;

  /**
   * @brief Remap the child value into a shell.
   * @param[in] a_params     Annular parameters.
   * @param[in] a_childValue Value of the wrapped implicit function.
   * @return std::abs(a_childValue) - delta.
   */
  EBGEOMETRY_HOST_DEVICE static T
  mapValue(const Params& a_params, T a_childValue) noexcept
  {
    return std::abs(a_childValue) - a_params.delta;
  }
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
   * @param[in] a_other Other AnnularIF.
   */
  AnnularIF(const AnnularIF& a_other) noexcept = default;

  /**
   * @brief Move constructor.
   * @param[in, out] a_other Other AnnularIF.
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
   * @param[in] a_other Other AnnularIF.
   * @return Reference to (*this).
   */
  AnnularIF&
  operator=(const AnnularIF& a_other) noexcept = default;

  /**
   * @brief Move assignment operator.
   * @param[in, out] a_other Other AnnularIF.
   * @return Reference to (*this).
   */
  AnnularIF&
  operator=(AnnularIF&& a_other) noexcept = default;

  /**
   * @brief Value function.
   * @param[in] a_point Query point
   * @return std::abs(f(a_point)) - m_params.delta, hollowing out the shape at distance m_params.delta from the surface
   */
  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
   * @brief Original implicit function.
   */
  std::shared_ptr<const ImplicitFunction<T>> m_implicitFunction = nullptr;

  /**
   * @brief Transform parameters (shell thickness).
   */
  typename AnnularOp<T>::Params m_params;
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
   * @param[in] a_other Other BlurIF.
   */
  BlurIF(const BlurIF& a_other) noexcept = default;

  /**
   * @brief Move constructor.
   * @param[in, out] a_other Other BlurIF.
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
   * @param[in] a_other Other BlurIF.
   * @return Reference to (*this).
   */
  BlurIF&
  operator=(const BlurIF& a_other) noexcept = default;

  /**
   * @brief Move assignment operator.
   * @param[in, out] a_other Other BlurIF.
   * @return Reference to (*this).
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
   * @param[in] a_other Other MollifyIF.
   */
  MollifyIF(const MollifyIF& a_other) = default;

  /**
   * @brief Move constructor.
   * @param[in, out] a_other Other MollifyIF.
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
   * @param[in] a_other Other MollifyIF.
   * @return Reference to (*this).
   */
  MollifyIF&
  operator=(const MollifyIF& a_other) = default;

  /**
   * @brief Move assignment operator.
   * @param[in, out] a_other Other MollifyIF.
   * @return Reference to (*this).
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
 * @brief Distance-formula trait for the elongation transform.
 * @details Single source of truth for the elongation's coordinate remapping: the query point is
 * clamped component-wise to [-elongation, +elongation] and the clamped value is subtracted before
 * evaluating the wrapped function. Reused verbatim by the tape interpreter.
 * @tparam T Floating-point precision.
 */
template <class T>
struct ElongateOp
{
  /**
   * @brief Stored parameters for the elongation transform.
   */
  struct Params
  {
    Vec3T<T> elongation = Vec3T<T>::zeros(); ///< Per-axis elongation amounts.
  };

  /**
   * @brief Elongation does not preserve the signed distance property (the swept region is not metric).
   */
  static constexpr bool preservesSignedDistance = false;

  /**
   * @brief Remap the query point by the elongation.
   * @param[in] a_params Elongation parameters.
   * @param[in] a_point  Query point.
   * @return a_point minus its component-wise clamp to [-elongation, +elongation].
   */
  EBGEOMETRY_HOST_DEVICE static Vec3T<T>
  mapPoint(const Params& a_params, const Vec3T<T>& a_point) noexcept
  {
    return a_point - clamp(a_point, -a_params.elongation, a_params.elongation);
  }
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
   * @param[in] a_other Other ElongateIF.
   */
  ElongateIF(const ElongateIF& a_other) noexcept = default;

  /**
   * @brief Move constructor.
   * @param[in, out] a_other Other ElongateIF.
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
   * @param[in] a_other Other ElongateIF.
   * @return Reference to (*this).
   */
  ElongateIF&
  operator=(const ElongateIF& a_other) noexcept = default;

  /**
   * @brief Move assignment operator.
   * @param[in, out] a_other Other ElongateIF.
   * @return Reference to (*this).
   */
  ElongateIF&
  operator=(ElongateIF&& a_other) noexcept = default;

  /**
   * @brief Value function
   * @param[in] a_point Query point
   * @return Value of the wrapped function evaluated at a_point clamped by m_params.elongation
   */
  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
   * @brief Underlying implicit function to be elongated
   */
  std::shared_ptr<const ImplicitFunction<T>> m_implicitFunction = nullptr;

  /**
   * @brief Transform parameters (per-axis elongation).
   */
  typename ElongateOp<T>::Params m_params;
};

/**
 * @brief Distance-formula trait for the reflection transform.
 * @details Single source of truth for the reflection's coordinate remapping: the query point is
 * multiplied component-wise by the reflection parameters (ones, with -1 in the reflected axis) before
 * evaluating the wrapped function. Reused verbatim by the tape interpreter.
 * @tparam T Floating-point precision.
 */
template <class T>
struct ReflectOp
{
  /**
   * @brief Stored parameters for the reflection transform.
   */
  struct Params
  {
    Vec3T<T> reflectParams = Vec3T<T>::ones(); ///< Per-axis reflection multipliers (-1 in reflected axis).
  };

  /**
   * @brief A reflection of a signed distance function is again a signed distance function.
   */
  static constexpr bool preservesSignedDistance = true;

  /**
   * @brief Remap the query point by the reflection.
   * @param[in] a_params Reflection parameters.
   * @param[in] a_point  Query point.
   * @return The query point multiplied component-wise by the reflection parameters.
   */
  EBGEOMETRY_HOST_DEVICE static Vec3T<T>
  mapPoint(const Params& a_params, const Vec3T<T>& a_point) noexcept
  {
    return a_point * a_params.reflectParams;
  }
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
   * @param[in] a_other Other ReflectIF.
   */
  ReflectIF(const ReflectIF& a_other) noexcept = default;

  /**
   * @brief Move constructor.
   * @param[in, out] a_other Other ReflectIF.
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
   * @param[in] a_other Other ReflectIF.
   * @return Reference to (*this).
   */
  ReflectIF&
  operator=(const ReflectIF& a_other) noexcept = default;

  /**
   * @brief Move assignment operator.
   * @param[in, out] a_other Other ReflectIF.
   * @return Reference to (*this).
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
   * @brief Transform parameters (per-axis reflection multipliers).
   */
  typename ReflectOp<T>::Params m_params;
};

} // namespace EBGeometry

#include "EBGeometry_TransformImplem.hpp"

#endif
