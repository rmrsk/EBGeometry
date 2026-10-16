// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Vec.hpp
 * @brief  Declaration of 2D and 3D point/vector classes with templated
 * precision. Used with DCEL tools.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_VEC_HPP
#define EBGEOMETRY_VEC_HPP

// Std includes
#include <algorithm>
#include <cstddef>
#include <type_traits>

// Our includes
#include "EBGeometry_Array.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"

namespace EBGeometry {

/**
 * @brief Two-dimensional vector class with arithmetic operators.
 * @details Like Vec3T, the components are reached through operator[] (index 0 or 1).
 * @note Vec2T is a templated class primarily used with DCEL grids.
 * @tparam T Floating-point precision (float or double).
 */
template <typename T>
class Vec2T
{
public:
  static_assert(std::is_floating_point_v<T>, "Vec2T<T>: T must be a floating-point type");
  /**
   * @brief Default constructor. Leaves the components uninitialized.
   * @details Defaulted, so that Vec2T is trivially default constructible and can be the element type of
   * a CUDA/HIP `__shared__` array, which may not have an initializer. As with a plain T, `Vec2T<T> v;`
   * leaves v uninitialized, while value initialization (`Vec2T<T> v{};`, `Vec2T<T>()`) zeroes it, as does
   * zeros().
   */
  Vec2T() noexcept = default;

  /**
   * @brief Copy constructor
   * @param[in] u Other vector
   * @details Sets *this = u
   */
  constexpr Vec2T(const Vec2T& u) noexcept = default;

  /**
   * @brief Full constructor
   * @param[in] a_x First vector component
   * @param[in] a_y Second vector component
   * @details Sets X[0] = a_x and X[1] = a_y.
   */
  EBGEOMETRY_HOST_DEVICE
  constexpr Vec2T(const T& a_x, const T& a_y) noexcept;

  /**
   * @brief Destructor (does nothing)
   */
  ~Vec2T() = default;

  /**
   * @brief Mutable element access.
   * @param[in] i Component index (0 = x, 1 = y). Must be < 2.
   * @return Reference to the i-th component.
   */
  EBGEOMETRY_HOST_DEVICE
  inline constexpr T&
  operator[](size_t i) noexcept;

  /**
   * @brief Const element access.
   * @param[in] i Component index (0 = x, 1 = y). Must be < 2.
   * @return Const reference to the i-th component.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline constexpr const T&
  operator[](size_t i) const noexcept;

  /**
   * @brief Return av vector with x = y = 0.
   * @return Zero vector (0, 0).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline static constexpr Vec2T<T>
  zeros() noexcept;

  /**
   * @brief Return a vector with x = y = 1.
   * @return Unit vector (1, 1).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline static constexpr Vec2T<T>
  ones() noexcept;

  /**
   * @brief Return the most-negative representable vector.
   * @return Vector with each component equal to -Math::Limits<T>::max().
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline static constexpr Vec2T<T>
  min() noexcept;

  /**
   * @brief Return the most-positive representable vector.
   * @return Vector with each component equal to Math::Limits<T>::max().
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline static constexpr Vec2T<T>
  max() noexcept;

  /**
   * @brief Return a vector with infinite components.
   * @return Vector with each component equal to Math::Limits<T>::infinity().
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline static constexpr Vec2T<T>
  infinity() noexcept;

  /**
   * @brief Assignment operator.
   * @param[in] a_other Other vector.
   * @return Reference to *this after assignment.
   */
  constexpr Vec2T<T>&
  operator=(const Vec2T& a_other) noexcept = default;

  /**
   * @brief Addition operator.
   * @param[in] a_other Other vector.
   * @return New vector with x = X[0] + a_other[0], y = X[1] + a_other[1].
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline constexpr Vec2T<T>
  operator+(const Vec2T& a_other) const noexcept;

  /**
   * @brief Subtraction operator.
   * @param[in] a_other Other vector.
   * @return New vector with x = X[0] - a_other[0], y = X[1] - a_other[1].
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline constexpr Vec2T<T>
  operator-(const Vec2T& a_other) const noexcept;

  /**
   * @brief Negation operator.
   * @return New Vec2T<T> with negated components.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline constexpr Vec2T<T>
  operator-() const noexcept;

  /**
   * @brief Scalar multiplication operator.
   * @param[in] s Scalar.
   * @return New vector with x = s*X[0], y = s*X[1].
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline constexpr Vec2T<T>
  operator*(const T& s) const noexcept;

  /**
   * @brief Scalar division operator.
   * @param[in] s Scalar divisor.
   * @return New vector with x = X[0]/s, y = X[1]/s.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline constexpr Vec2T<T>
  operator/(const T& s) const noexcept;

  /**
   * @brief In-place addition operator.
   * @param[in] a_other Other vector to add.
   * @return Reference to *this after addition.
   */
  EBGEOMETRY_HOST_DEVICE
  inline constexpr Vec2T<T>&
  operator+=(const Vec2T& a_other) noexcept;

  /**
   * @brief In-place subtraction operator.
   * @param[in] a_other Other vector to subtract.
   * @return Reference to *this after subtraction.
   */
  EBGEOMETRY_HOST_DEVICE
  inline constexpr Vec2T<T>&
  operator-=(const Vec2T& a_other) noexcept;

  /**
   * @brief In-place scalar multiplication operator.
   * @param[in] s Scalar.
   * @return Reference to *this after multiplication.
   */
  EBGEOMETRY_HOST_DEVICE
  inline constexpr Vec2T<T>&
  operator*=(const T& s) noexcept;

  /**
   * @brief In-place scalar division operator.
   * @param[in] s Scalar divisor.
   * @return Reference to *this after division.
   */
  EBGEOMETRY_HOST_DEVICE
  inline constexpr Vec2T<T>&
  operator/=(const T& s) noexcept;

  /**
   * @brief Dot product.
   * @param[in] a_other Other vector.
   * @return Scalar dot product: X[0]*a_other[0] + X[1]*a_other[1].
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline constexpr T
  dot(const Vec2T& a_other) const noexcept;

  /**
   * @brief Compute length of vector
   * @details Not constexpr, since std::sqrt is not.
   * @return Euclidean length: std::sqrt(X[0]*X[0] + X[1]*X[1]).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  length() const noexcept;

  /**
   * @brief Compute square of vector
   * @return Returns length of vector, i.e. X[0]*X[0] +
   * X[1]*X[1]
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline constexpr T
  length2() const noexcept;

protected:
  /**
   * @brief Vector components
   */
  Array<T, 2> m_X;
};

/**
 * @brief Three-dimensional vector class with arithmetic operators.
 * @details The components are reached through operator[] (index 0, 1 or 2).
 * @note Vec3T is a templated class primarily used with DCEL grids. It is always
 * 3D, i.e. independent of Chombo configuration settings. This lets one use DCEL
 * functionality even though the simulation might only be 2D.
 * @tparam T Floating-point precision (float or double).
 */
template <typename T>
class Vec3T
{
public:
  static_assert(std::is_floating_point_v<T>, "Vec3T<T>: T must be a floating-point type");

  /**
   * @brief Default constructor. Leaves the components uninitialized.
   * @details Defaulted, so that Vec3T is trivially default constructible and can be the element type of
   * a CUDA/HIP `__shared__` array, which may not have an initializer. As with a plain T, `Vec3T<T> v;`
   * leaves v uninitialized, while value initialization (`Vec3T<T> v{};`, `Vec3T<T>()`) zeroes it, as does
   * zeros().
   */
  Vec3T() noexcept = default;

  /**
   * @brief Copy constructor
   * @param[in] a_u Other vector
   * @details Sets *this = u
   */
  constexpr Vec3T(const Vec3T<T>& a_u) noexcept = default;

  /**
   * @brief Full constructor
   * @param[in] a_x First vector component
   * @param[in] a_y Second vector component
   * @param[in] a_z Third vector component
   * @details Sets X[0] = a_x, X[1] = a_y and X[2] = a_z.
   */
  EBGEOMETRY_HOST_DEVICE
  constexpr Vec3T(const T& a_x, const T& a_y, const T& a_z) noexcept;

  /**
   * @brief Destructor (does nothing)
   */
  ~Vec3T() noexcept = default;

  /**
   * @brief Return a vector with x = y = z = 0.
   * @return Zero vector (0, 0, 0).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline static constexpr Vec3T<T>
  zeros() noexcept;

  /**
   * @brief Return a vector with x = y = z = 1.
   * @return Ones vector (1, 1, 1).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline static constexpr Vec3T<T>
  ones() noexcept;

  /**
   * @brief Return the unit basis vector for the given coordinate axis.
   * @param[in] a_dir Axis index (0 = x, 1 = y, 2 = z).
   * @return Basis vector e_{a_dir} with a 1 in position a_dir and 0 elsewhere.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline static constexpr Vec3T<T>
  unit(const size_t a_dir) noexcept;

  /**
   * @brief Return the most-negative representable vector.
   * @return Vector with each component equal to -Math::Limits<T>::max().
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline static constexpr Vec3T<T>
  min() noexcept;

  /**
   * @brief Return the most-positive representable vector.
   * @return Vector with each component equal to Math::Limits<T>::max().
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline static constexpr Vec3T<T>
  max() noexcept;

  /**
   * @brief Return a vector with infinite components.
   * @return Vector with each component equal to Math::Limits<T>::infinity().
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline static constexpr Vec3T<T>
  infinity() noexcept;

  /**
   * @brief Lexicographic comparison (x first, then y, then z).
   * @details Returns true if this vector is lexicographically less than u,
   * i.e. the first differing component of (*this) is less than the corresponding
   * component of u.
   * @param[in] u Other vector.
   * @return True if (*this) is lexicographically less than u.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline constexpr bool
  lessLX(const Vec3T<T>& u) const noexcept;

  /**
   * @brief Mutable element access.
   * @param[in] i Component index (0 = x, 1 = y, 2 = z). Must be < 3.
   * @return Reference to the i-th component.
   */
  EBGEOMETRY_HOST_DEVICE
  inline constexpr T&
  operator[](size_t i) noexcept;

  /**
   * @brief Const element access.
   * @param[in] i Component index (0 = x, 1 = y, 2 = z). Must be < 3.
   * @return Const reference to the i-th component.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline constexpr const T&
  operator[](size_t i) const noexcept;

  /**
   * @brief Equality comparison.
   * @param[in] u Other vector.
   * @return True if all three components are equal.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline constexpr bool
  operator==(const Vec3T<T>& u) const noexcept;

  /**
   * @brief Inequality comparison.
   * @param[in] u Other vector.
   * @return True if any component differs.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline constexpr bool
  operator!=(const Vec3T<T>& u) const noexcept;

  /**
   * @name Deleted ordering operators
   * @brief Vec3T has no ordering operators.
   * @details These once meant "every component less (greater, ...) than", which is not a strict weak
   * ordering: two different vectors can each fail to be less than the other, so std::set, std::map and
   * std::sort built on them silently merge or misorder distinct vectors. Compare components
   * explicitly, or use lessLX() for a lexicographic ordering.
   */
  ///@{
  bool
  operator<(const Vec3T<T>& u) const noexcept = delete;
  bool
  operator>(const Vec3T<T>& u) const noexcept = delete;
  bool
  operator<=(const Vec3T<T>& u) const noexcept = delete;
  bool
  operator>=(const Vec3T<T>& u) const noexcept = delete;
  ///@}

  /**
   * @brief Assignment operator.
   * @param[in] u Other vector.
   * @return Reference to *this after assignment.
   */
  constexpr Vec3T<T>&
  operator=(const Vec3T<T>& u) noexcept = default;

  /**
   * @brief Addition operator.
   * @param[in] u Other vector.
   * @return New vector with X[i] = this->X[i] + u[i] for each component.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline constexpr Vec3T<T>
  operator+(const Vec3T<T>& u) const noexcept;

  /**
   * @brief Subtraction operator. Returns a new vector with subtracted components
   * @return New vector with X[i] = this->X[i] - u[i] for each component.
   * @param[in] u Other vector
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline constexpr Vec3T<T>
  operator-(const Vec3T<T>& u) const noexcept;

  /**
   * @brief Identity operator.
   * @return Copy of this vector (unary plus, component-wise identity).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline constexpr Vec3T<T>
  operator+() const noexcept;

  /**
   * @brief Negation operator.
   * @return New vector with each component negated.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline constexpr Vec3T<T>
  operator-() const noexcept;

  /**
   * @brief Multiplication operator. Returns a vector with scalar multiplied
   * components
   * @param[in] s Scalar to multiply by
   * @return Returns a new vector with X[i] = this->X[i] * s
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline constexpr Vec3T<T>
  operator*(const T& s) const noexcept;

  /**
   * @brief Component-wise multiplication operator
   * @param[in] s Scalar to multiply by
   * @return Returns a new vector with X[i] = this->X[i] * s[i] for each
   * component.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline constexpr Vec3T<T>
  operator*(const Vec3T<T>& s) const noexcept;

  /**
   * @brief Division operator. Returns a vector with scalar divided components
   * @param[in] s Scalar to divided by
   * @return Returns a new vector with X[i] = this->X[i] / s
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline constexpr Vec3T<T>
  operator/(const T& s) const noexcept;

  /**
   * @brief Component-wise division operator.
   * @param[in] v Other vector
   * @return Returns a new vector with X[i] = this->X[i]/v[i] for each component.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline constexpr Vec3T<T>
  operator/(const Vec3T<T>& v) const noexcept;

  /**
   * @brief Vector addition operator.
   * @param[in] u Vector to add
   * @return (*this), with X[i] += u[i] for each component.
   */
  EBGEOMETRY_HOST_DEVICE
  inline constexpr Vec3T<T>&
  operator+=(const Vec3T<T>& u) noexcept;

  /**
   * @brief Vector subtraction operator.
   * @param[in] u Vector to subtraction
   * @return (*this), with X[i] -= u[i] for each component.
   */
  EBGEOMETRY_HOST_DEVICE
  inline constexpr Vec3T<T>&
  operator-=(const Vec3T<T>& u) noexcept;

  /**
   * @brief Vector multiplication operator.
   * @param[in] s Scalar to multiply by
   * @return (*this), with X[i] *= s for each component.
   */
  EBGEOMETRY_HOST_DEVICE
  inline constexpr Vec3T<T>&
  operator*=(const T& s) noexcept;

  /**
   * @brief In-place scalar division operator.
   * @param[in] s Scalar to divide by
   * @return (*this), with X[i] /= s for each component.
   */
  EBGEOMETRY_HOST_DEVICE
  inline constexpr Vec3T<T>&
  operator/=(const T& s) noexcept;

  /**
   * @brief Vector cross product.
   * @param[in] u Other vector.
   * @return Cross product (*this) × u.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline constexpr Vec3T<T>
  cross(const Vec3T<T>& u) const noexcept;

  /**
   * @brief Vector dot product.
   * @param[in] u Other vector.
   * @return Scalar dot product (*this) · u.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline constexpr T
  dot(const Vec3T<T>& u) const noexcept;

  /**
   * @brief Return the index of the smallest component (optionally by magnitude).
   * @param[in] a_doAbs If true, compare |X[i]| instead of X[i].
   * @return Index (0, 1, or 2) of the component with the smallest value (or magnitude).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline size_t
  minDir(const bool a_doAbs) const noexcept;

  /**
   * @brief Return the direction which has the largest component (can be
   * absolute)
   * @param[in] a_doAbs If true, evaluate component magnitudes rather than
   * values.
   * @return Direction with the biggest component
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline size_t
  maxDir(const bool a_doAbs) const noexcept;

  /**
   * @brief Compute vector length.
   * @details Not constexpr, since std::sqrt is not.
   * @return Euclidean length: std::sqrt(X[0]*X[0] + X[1]*X[1] + X[2]*X[2]).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  length() const noexcept;

  /**
   * @brief Compute squared vector length.
   * @return Squared Euclidean length: X[0]*X[0] + X[1]*X[1] + X[2]*X[2].
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline constexpr T
  length2() const noexcept;

protected:
  /**
   * @brief Vector components
   */
  Array<T, 3> m_X;
};

/**
 * @brief Scalar-left multiplication: s * vec.
 * @tparam R Arithmetic type of the scalar (e.g. 2.0 * Vec2T<float>); it is converted to T first.
 * @tparam T Floating-point precision of the vector.
 * @param[in] s       Scalar multiplier.
 * @param[in] a_other Vector.
 * @return New vector with x = T(s)*a_other[0], y = T(s)*a_other[1].
 */
template <class R, typename T, std::enable_if_t<std::is_arithmetic_v<R>, int> = 0>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline constexpr Vec2T<T>
operator*(const R& s, const Vec2T<T>& a_other) noexcept;

/**
 * @brief Component-wise scalar-left division: s / vec.
 * @details Returns a new vector where each component i is T(s) / a_other[i].
 * @tparam R Arithmetic type of the scalar numerator; it is converted to T first.
 * @tparam T Floating-point precision of the vector.
 * @param[in] s       Scalar numerator.
 * @param[in] a_other Vector whose components are the denominators.
 * @return New vector with x = T(s)/a_other[0], y = T(s)/a_other[1].
 */
template <class R, typename T, std::enable_if_t<std::is_arithmetic_v<R>, int> = 0>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline constexpr Vec2T<T>
operator/(const R& s, const Vec2T<T>& a_other) noexcept;

/**
 * @brief Component-wise minimum of two vectors.
 * @details Each component is Math::min of the two: the first vector's component when they compare
 * equal or either is NaN, like std::min.
 * @tparam T Floating-point precision.
 * @param[in] u First vector.
 * @param[in] v Second vector.
 * @return New vector with x = min(u[0], v[0]), y = min(u[1], v[1]).
 */
template <typename T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline Vec2T<T>
min(const Vec2T<T>& u, const Vec2T<T>& v) noexcept;

/**
 * @brief Component-wise maximum of two vectors.
 * @details Each component is Math::max of the two: the first vector's component when they compare
 * equal or either is NaN, like std::max.
 * @tparam T Floating-point precision.
 * @param[in] u First vector.
 * @param[in] v Second vector.
 * @return New vector with x = max(u[0], v[0]), y = max(u[1], v[1]).
 */
template <typename T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline Vec2T<T>
max(const Vec2T<T>& u, const Vec2T<T>& v) noexcept;

/**
 * @brief Dot product of two 2D vectors.
 * @tparam T Floating-point precision.
 * @param[in] u First vector.
 * @param[in] v Second vector.
 * @return Scalar dot product u · v.
 */
template <typename T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline T
dot(const Vec2T<T>& u, const Vec2T<T>& v) noexcept;

/**
 * @brief Euclidean length of a 2D vector.
 * @tparam T Floating-point precision.
 * @param[in] v Vector.
 * @return std::sqrt(v[0]*v[0] + v[1]*v[1]).
 */
template <typename T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline T
length(const Vec2T<T>& v) noexcept;

/**
 * @brief Scalar-left multiplication: s * vec.
 * @tparam R Arithmetic type of the scalar (e.g. 2 * Vec3T<double>); it is converted to T first.
 * @tparam T Floating-point precision of the vector.
 * @param[in] s Scalar multiplier.
 * @param[in] u Vector.
 * @return New vector with X[i] = T(s) * u[i].
 */
template <class R, typename T, std::enable_if_t<std::is_arithmetic_v<R>, int> = 0>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline constexpr Vec3T<T>
operator*(const R& s, const Vec3T<T>& u) noexcept;

/**
 * @brief Component-wise scalar-left division: s / vec.
 * @details Returns a new vector where each component i is T(s) / u[i].
 * @tparam R Arithmetic type of the scalar numerator; it is converted to T first.
 * @tparam T Floating-point precision of the vector.
 * @param[in] s Scalar numerator.
 * @param[in] u Vector whose components are the denominators.
 * @return New vector with X[i] = T(s) / u[i].
 */
template <class R, typename T, std::enable_if_t<std::is_arithmetic_v<R>, int> = 0>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline constexpr Vec3T<T>
operator/(const R& s, const Vec3T<T>& u) noexcept;

/**
 * @brief Component-wise minimum of two 3D vectors.
 * @details Each component is Math::min of the two: the first vector's component when they compare
 * equal or either is NaN, like std::min.
 * @tparam T Floating-point precision.
 * @param[in] u First vector.
 * @param[in] v Second vector.
 * @return New vector with X[i] = min(u[i], v[i]).
 */
template <typename T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline constexpr Vec3T<T>
min(const Vec3T<T>& u, const Vec3T<T>& v) noexcept;

/**
 * @brief Component-wise maximum of two 3D vectors.
 * @details Each component is Math::max of the two: the first vector's component when they compare
 * equal or either is NaN, like std::max.
 * @tparam T Floating-point precision.
 * @param[in] u First vector.
 * @param[in] v Second vector.
 * @return New vector with X[i] = max(u[i], v[i]).
 */
template <typename T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline constexpr Vec3T<T>
max(const Vec3T<T>& u, const Vec3T<T>& v) noexcept;

/**
 * @brief Component-wise clamp of a 3D vector.
 * @tparam T Floating-point precision.
 * @param[in] v  Vector to be clamped.
 * @param[in] lo Component-wise lower bound.
 * @param[in] hi Component-wise upper bound.
 * @return New vector with X[i] = clamp(v[i], lo[i], hi[i]).
 */
template <typename T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline constexpr Vec3T<T>
clamp(const Vec3T<T>& v, const Vec3T<T>& lo, const Vec3T<T>& hi) noexcept;

/**
 * @brief Dot product of two 3D vectors.
 * @tparam T Floating-point precision.
 * @param[in] u First vector.
 * @param[in] v Second vector.
 * @return Scalar dot product u · v.
 */
template <typename T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline constexpr T
dot(const Vec3T<T>& u, const Vec3T<T>& v) noexcept;

/**
 * @brief Cross product of two 3D vectors.
 * @tparam T Floating-point precision.
 * @param[in] u First vector.
 * @param[in] v Second vector.
 * @return Cross product u × v.
 */
template <typename T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline constexpr Vec3T<T>
cross(const Vec3T<T>& u, const Vec3T<T>& v) noexcept;

/**
 * @brief Euclidean length of a 3D vector.
 * @tparam T Floating-point precision.
 * @param[in] v Vector.
 * @return std::sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]).
 */
template <typename T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline T
length(const Vec3T<T>& v) noexcept;

static_assert(std::is_trivially_copyable_v<Vec2T<float>>, "Vec2T<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<Vec2T<double>>, "Vec2T<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<Vec3T<float>>, "Vec3T<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<Vec3T<double>>, "Vec3T<double> must be trivially copyable");
static_assert(std::is_trivially_default_constructible_v<Vec2T<float>>,
              "Vec2T<float> must be trivially default constructible");
static_assert(std::is_trivially_default_constructible_v<Vec2T<double>>,
              "Vec2T<double> must be trivially default constructible");
static_assert(std::is_trivially_default_constructible_v<Vec3T<float>>,
              "Vec3T<float> must be trivially default constructible");
static_assert(std::is_trivially_default_constructible_v<Vec3T<double>>,
              "Vec3T<double> must be trivially default constructible");

} // namespace EBGeometry

#include "EBGeometry_VecImplem.hpp"

#endif
