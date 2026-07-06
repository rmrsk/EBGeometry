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
#include <array>
#include <cstddef>
#include <limits>
#include <ostream>
#include <type_traits>

// Our includes
#include "EBGeometry_Macros.hpp"

namespace EBGeometry {

/**
 * @brief Two-dimensional vector class with arithmetic operators.
 * @details The class has a public-only interface. To change it's components one
 * can call the member functions, or set components directly, e.g. vec.x = 5.0
 * @note Vec2T is a templated class primarily used with DCEL grids.
 * @tparam T Floating-point precision (float or double).
 */
template <typename T>
class Vec2T
{
public:
  static_assert(std::is_floating_point_v<T>, "Vec2T<T>: T must be a floating-point type");
  /**
   * @brief Default constructor. Sets the vector to the zero vector.
   */
  Vec2T() noexcept;

  /**
   * @brief Copy constructor
   * @param[in] u Other vector
   * @details Sets *this = u
   */
  Vec2T(const Vec2T& u) noexcept;

  /**
   * @brief Full constructor
   * @param[in] a_x First vector component
   * @param[in] a_y Second vector component
   * @details Sets this->x = a_x and this->y = a_y
   */
  constexpr Vec2T(const T& a_x, const T& a_y) noexcept;

  /**
   * @brief Destructor (does nothing)
   */
  ~Vec2T() = default;

  /**
   * @brief First component in the vector
   */
  T x;

  /**
   * @brief Second component in the vector
   */
  T y;

  /**
   * @brief Return av vector with x = y = 0.
   * @return Zero vector (0, 0).
   */
  [[nodiscard]] inline static constexpr Vec2T<T>
  zero() noexcept;

  /**
   * @brief Return a vector with x = y = 1.
   * @return Unit vector (1, 1).
   */
  [[nodiscard]] inline static constexpr Vec2T<T>
  one() noexcept;

  /**
   * @brief Return the most-negative representable vector.
   * @return Vector with each component equal to -std::numeric_limits<T>::max().
   */
  [[nodiscard]] inline static constexpr Vec2T<T>
  min() noexcept;

  /**
   * @brief Return the most-positive representable vector.
   * @return Vector with each component equal to std::numeric_limits<T>::max().
   */
  [[nodiscard]] inline static constexpr Vec2T<T>
  max() noexcept;

  /**
   * @brief Return a vector with infinite components.
   * @return Vector with each component equal to std::numeric_limits<T>::infinity().
   */
  [[nodiscard]] inline static constexpr Vec2T<T>
  infinity() noexcept;

  /**
   * @brief Assignment operator.
   * @param[in] a_other Other vector.
   * @return Reference to *this after assignment.
   */
  inline constexpr Vec2T<T>&
  operator=(const Vec2T& a_other) noexcept;

  /**
   * @brief Addition operator.
   * @param[in] a_other Other vector.
   * @return New vector with x = this->x + a_other.x, y = this->y + a_other.y.
   */
  [[nodiscard]] inline constexpr Vec2T<T>
  operator+(const Vec2T& a_other) const noexcept;

  /**
   * @brief Subtraction operator.
   * @param[in] a_other Other vector.
   * @return New vector with x = this->x - a_other.x, y = this->y - a_other.y.
   */
  [[nodiscard]] inline constexpr Vec2T<T>
  operator-(const Vec2T& a_other) const noexcept;

  /**
   * @brief Negation operator.
   * @return New Vec2T<T> with negated components.
   */
  [[nodiscard]] inline constexpr Vec2T<T>
  operator-() const noexcept;

  /**
   * @brief Scalar multiplication operator.
   * @param[in] s Scalar.
   * @return New vector with x = s*this->x, y = s*this->y.
   */
  [[nodiscard]] inline constexpr Vec2T<T>
  operator*(const T& s) const noexcept;

  /**
   * @brief Scalar division operator.
   * @param[in] s Scalar divisor.
   * @return New vector with x = this->x/s, y = this->y/s.
   */
  [[nodiscard]] inline constexpr Vec2T<T>
  operator/(const T& s) const noexcept;

  /**
   * @brief In-place addition operator.
   * @param[in] a_other Other vector to add.
   * @return Reference to *this after addition.
   */
  inline constexpr Vec2T<T>&
  operator+=(const Vec2T& a_other) noexcept;

  /**
   * @brief In-place subtraction operator.
   * @param[in] a_other Other vector to subtract.
   * @return Reference to *this after subtraction.
   */
  inline constexpr Vec2T<T>&
  operator-=(const Vec2T& a_other) noexcept;

  /**
   * @brief In-place scalar multiplication operator.
   * @param[in] s Scalar.
   * @return Reference to *this after multiplication.
   */
  inline constexpr Vec2T<T>&
  operator*=(const T& s) noexcept;

  /**
   * @brief In-place scalar division operator.
   * @param[in] s Scalar divisor.
   * @return Reference to *this after division.
   */
  inline constexpr Vec2T<T>&
  operator/=(const T& s) noexcept;

  /**
   * @brief Dot product.
   * @param[in] a_other Other vector.
   * @return Scalar dot product: this->x*a_other.x + this->y*a_other.y.
   */
  [[nodiscard]] inline constexpr T
  dot(const Vec2T& a_other) const noexcept;

  /**
   * @brief Compute length of vector
   * @return Returns length of vector, i.e. sqrt[(this->x)*(this->x) +
   * (this->y)*(this->y)]
   */
  [[nodiscard]] inline constexpr T
  length() const noexcept;

  /**
   * @brief Compute square of vector
   * @return Returns length of vector, i.e. (this->x)*(this->x) +
   * (this->y)*(this->y)
   */
  [[nodiscard]] inline constexpr T
  length2() const noexcept;
};

/**
 * @brief Three-dimensional vector class with arithmetic operators.
 * @details The class has a public-only interface. To change it's components one
 * can call the member functions, or set components directly, e.g. vec.x = 5.0
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
   * @brief Stream insertion operator.
   * @param[in,out] os  Output stream.
   * @param[in]     vec Vector to print.
   * @return Reference to the output stream after insertion.
   */
  friend std::ostream&
  operator<<(std::ostream& os, const Vec3T<T>& vec)
  {
    os << '(' << vec[0] << ',' << vec[1] << ',' << vec[2] << ')';

    return os;
  }

  /**
   * @brief Default constructor. Sets the vector to the zero vector.
   */
  Vec3T() noexcept;

  /**
   * @brief Copy constructor
   * @param[in] a_u Other vector
   * @details Sets *this = u
   */
  Vec3T(const Vec3T<T>& a_u) noexcept;

  /**
   * @brief Full constructor
   * @param[in] a_x First vector component
   * @param[in] a_y Second vector component
   * @param[in] a_z Third vector component
   * @details Sets this->x = a_x, this->y = a_y, and this->z = a_z
   */
  constexpr Vec3T(const T& a_x, const T& a_y, const T& a_z) noexcept;

  /**
   * @brief Destructor (does nothing)
   */
  ~Vec3T() noexcept = default;

  /**
   * @brief Return a vector with x = y = z = 0.
   * @return Zero vector (0, 0, 0).
   */
  [[nodiscard]] inline static constexpr Vec3T<T>
  zero() noexcept;

  /**
   * @brief Return a vector with x = y = z = 1.
   * @return Ones vector (1, 1, 1).
   */
  [[nodiscard]] inline static constexpr Vec3T<T>
  one() noexcept;

  /**
   * @brief Return the unit basis vector for the given coordinate axis.
   * @param[in] a_dir Axis index (0 = x, 1 = y, 2 = z).
   * @return Basis vector e_{a_dir} with a 1 in position a_dir and 0 elsewhere.
   */
  [[nodiscard]] inline static constexpr Vec3T<T>
  unit(const size_t a_dir) noexcept;

  /**
   * @brief Return the most-negative representable vector.
   * @return Vector with each component equal to -std::numeric_limits<T>::max().
   */
  [[nodiscard]] inline static constexpr Vec3T<T>
  min() noexcept;

  /**
   * @brief Return the most-positive representable vector.
   * @return Vector with each component equal to std::numeric_limits<T>::max().
   */
  [[nodiscard]] inline static constexpr Vec3T<T>
  max() noexcept;

  /**
   * @brief Return a vector with infinite components.
   * @return Vector with each component equal to std::numeric_limits<T>::infinity().
   */
  [[nodiscard]] inline static constexpr Vec3T<T>
  infinity() noexcept;

  /**
   * @brief Lexicographic comparison (x first, then y, then z).
   * @details Returns true if this vector is lexicographically less than u,
   * i.e. the first differing component of (*this) is less than the corresponding
   * component of u.
   * @param[in] u Other vector.
   * @return True if (*this) is lexicographically less than u.
   */
  [[nodiscard]] inline constexpr bool
  lessLX(const Vec3T<T>& u) const noexcept;

  /**
   * @brief Mutable element access.
   * @param[in] i Component index (0 = x, 1 = y, 2 = z). Must be < 3.
   * @return Reference to the i-th component.
   */
  inline constexpr T&
  operator[](size_t i) noexcept;

  /**
   * @brief Const element access.
   * @param[in] i Component index (0 = x, 1 = y, 2 = z). Must be < 3.
   * @return Const reference to the i-th component.
   */
  [[nodiscard]] inline constexpr const T&
  operator[](size_t i) const noexcept;

  /**
   * @brief Equality comparison.
   * @param[in] u Other vector.
   * @return True if all three components are equal.
   */
  [[nodiscard]] inline constexpr bool
  operator==(const Vec3T<T>& u) const noexcept;

  /**
   * @brief Inequality comparison.
   * @param[in] u Other vector.
   * @return True if any component differs.
   */
  [[nodiscard]] inline constexpr bool
  operator!=(const Vec3T<T>& u) const noexcept;

  /**
   * @brief "Smaller than" operator.
   * @details Returns true if this->x < u.x AND this->y < u.y AND this->z < u.z
   * and false otherwise
   * @param[in] u Other vector
   * @return True if all three components of *this are strictly less than the
   * corresponding components of u, false otherwise.
   */
  [[nodiscard]] inline constexpr bool
  operator<(const Vec3T<T>& u) const noexcept;

  /**
   * @brief "Greater than" operator.
   * @details Returns true if this->x > u.x AND this->y > u.y AND this->z > u.z
   * @param[in] u Other vector
   * @return True if all three components of *this are strictly greater than the
   * corresponding components of u, false otherwise.
   */
  [[nodiscard]] inline constexpr bool
  operator>(const Vec3T<T>& u) const noexcept;

  /**
   * @brief "Smaller or equal to" operator.
   * @details Returns true if this->x <= u.x AND this->y <= u.y AND this->z <=
   * u.z
   * @param[in] u Other vector
   * @return True if all three components of *this are less than or equal to the
   * corresponding components of u, false otherwise.
   */
  [[nodiscard]] inline constexpr bool
  operator<=(const Vec3T<T>& u) const noexcept;

  /**
   * @brief "Greater or equal to" operator.
   * @details Returns true if this->x >= u.x AND this->y >= u.y AND this->z >=
   * u.z
   * @param[in] u Other vector
   * @return True if all three components of *this are greater than or equal to
   * the corresponding components of u, false otherwise.
   */
  [[nodiscard]] inline constexpr bool
  operator>=(const Vec3T<T>& u) const noexcept;

  /**
   * @brief Assignment operator.
   * @param[in] u Other vector.
   * @return Reference to *this after assignment.
   */
  inline constexpr Vec3T<T>&
  operator=(const Vec3T<T>& u) noexcept;

  /**
   * @brief Addition operator.
   * @param[in] u Other vector.
   * @return New vector with X[i] = this->X[i] + u[i] for each component.
   */
  [[nodiscard]] inline constexpr Vec3T<T>
  operator+(const Vec3T<T>& u) const noexcept;

  /**
   * @brief Subtraction operator. Returns a new vector with subtracted components
   * @return Returns a new vector with x = this->x - u.x and so on.
   * @param[in] u Other vector
   */
  [[nodiscard]] inline constexpr Vec3T<T>
  operator-(const Vec3T<T>& u) const noexcept;

  /**
   * @brief Identity operator.
   * @return Copy of this vector (unary plus, component-wise identity).
   */
  [[nodiscard]] inline constexpr Vec3T<T>
  operator+() const noexcept;

  /**
   * @brief Negation operator.
   * @return New vector with each component negated.
   */
  [[nodiscard]] inline constexpr Vec3T<T>
  operator-() const noexcept;

  /**
   * @brief Multiplication operator. Returns a vector with scalar multiplied
   * components
   * @param[in] s Scalar to multiply by
   * @return Returns a new vector with X[i] = this->X[i] * s
   */
  [[nodiscard]] inline constexpr Vec3T<T>
  operator*(const T& s) const noexcept;

  /**
   * @brief Component-wise multiplication operator
   * @param[in] s Scalar to multiply by
   * @return Returns a new vector with X[i] = this->X[i] * s[i] for each
   * component.
   */
  [[nodiscard]] inline constexpr Vec3T<T>
  operator*(const Vec3T<T>& s) const noexcept;

  /**
   * @brief Division operator. Returns a vector with scalar divided components
   * @param[in] s Scalar to divided by
   * @return Returns a new vector with X[i] = this->X[i] / s
   */
  [[nodiscard]] inline constexpr Vec3T<T>
  operator/(const T& s) const noexcept;

  /**
   * @brief Component-wise division operator.
   * @param[in] v Other vector
   * @return Returns a new vector with X[i] = this->X[i]/v[i] for each component.
   */
  [[nodiscard]] inline constexpr Vec3T<T>
  operator/(const Vec3T<T>& v) const noexcept;

  /**
   * @brief Vector addition operator.
   * @param[in] u Vector to add
   * @return Returns (*this) with incremented components, e.g. this->X[0] =
   * this->X[0] + u.X[0]
   */
  inline constexpr Vec3T<T>&
  operator+=(const Vec3T<T>& u) noexcept;

  /**
   * @brief Vector subtraction operator.
   * @param[in] u Vector to subtraction
   * @return Returns (*this) with subtracted components, e.g. this->X[0] =
   * this->X[0] - u.X[0]
   */
  inline constexpr Vec3T<T>&
  operator-=(const Vec3T<T>& u) noexcept;

  /**
   * @brief Vector multiplication operator.
   * @param[in] s Scalar to multiply by
   * @return Returns (*this) with multiplied components, e.g. this->X[0] =
   * this->X[0] * s
   */
  inline constexpr Vec3T<T>&
  operator*=(const T& s) noexcept;

  /**
   * @brief Vector division operator.
   * @param[in] s Scalar to divide by
   * @return Returns (*this) with multiplied components, e.g. this->X[0] =
   * this->X[0] / s
   */
  inline constexpr Vec3T<T>&
  operator/=(const T& s) noexcept;

  /**
   * @brief In-place component-wise minimum.
   * @details Updates *this so that each component becomes the minimum of itself
   * and the corresponding component of u, then returns a copy of *this.
   * @param[in] u Other vector.
   * @return Copy of *this after the in-place update.
   */
  inline constexpr Vec3T<T>
  min(const Vec3T<T>& u) noexcept;

  /**
   * @brief In-place component-wise maximum.
   * @details Updates *this so that each component becomes the maximum of itself
   * and the corresponding component of u, then returns a copy of *this.
   * @param[in] u Other vector.
   * @return Copy of *this after the in-place update.
   */
  inline constexpr Vec3T<T>
  max(const Vec3T<T>& u) noexcept;

  /**
   * @brief Vector cross product.
   * @param[in] u Other vector.
   * @return Cross product (*this) × u.
   */
  [[nodiscard]] inline constexpr Vec3T<T>
  cross(const Vec3T<T>& u) const noexcept;

  /**
   * @brief Vector dot product.
   * @param[in] u Other vector.
   * @return Scalar dot product (*this) · u.
   */
  [[nodiscard]] inline constexpr T
  dot(const Vec3T<T>& u) const noexcept;

  /**
   * @brief Return the index of the smallest component (optionally by magnitude).
   * @param[in] a_doAbs If true, compare |X[i]| instead of X[i].
   * @return Index (0, 1, or 2) of the component with the smallest value (or magnitude).
   */
  [[nodiscard]] inline size_t
  minDir(const bool a_doAbs) const noexcept;

  /**
   * @brief Return the direction which has the largest component (can be
   * absolute)
   * @param[in] a_doAbs If true, evaluate component magnitudes rather than
   * values.
   * @return Direction with the biggest component
   */
  [[nodiscard]] inline size_t
  maxDir(const bool a_doAbs) const noexcept;

  /**
   * @brief Compute vector length.
   * @return Euclidean length: sqrt(X[0]*X[0] + X[1]*X[1] + X[2]*X[2]).
   */
  [[nodiscard]] inline constexpr T
  length() const noexcept;

  /**
   * @brief Compute squared vector length.
   * @return Squared Euclidean length: X[0]*X[0] + X[1]*X[1] + X[2]*X[2].
   */
  [[nodiscard]] inline constexpr T
  length2() const noexcept;

protected:
  /**
   * @brief Vector components
   */
  std::array<T, 3> m_X;
};

/**
 * @brief Scalar-left multiplication: s * vec.
 * @tparam T Floating-point precision.
 * @param[in] s       Scalar multiplier.
 * @param[in] a_other Vector.
 * @return New vector with x = s*a_other.x, y = s*a_other.y.
 */
template <typename T>
[[nodiscard]] inline constexpr Vec2T<T>
operator*(const T& s, const Vec2T<T>& a_other) noexcept;

/**
 * @brief Component-wise scalar-left division: s / vec.
 * @details Returns a new vector where each component i is s / a_other[i].
 * @tparam T Floating-point precision.
 * @param[in] s       Scalar numerator.
 * @param[in] a_other Vector whose components are the denominators.
 * @return New vector with x = s/a_other.x, y = s/a_other.y.
 */
template <typename T>
[[nodiscard]] inline constexpr Vec2T<T>
operator/(const T& s, const Vec2T<T>& a_other) noexcept;

/**
 * @brief Component-wise minimum of two vectors.
 * @tparam T Floating-point precision.
 * @param[in] u First vector.
 * @param[in] v Second vector.
 * @return New vector with x = std::min(u.x, v.x), y = std::min(u.y, v.y).
 */
template <typename T>
[[nodiscard]] inline Vec2T<T>
min(const Vec2T<T>& u, const Vec2T<T>& v) noexcept;

/**
 * @brief Component-wise maximum of two vectors.
 * @tparam T Floating-point precision.
 * @param[in] u First vector.
 * @param[in] v Second vector.
 * @return New vector with x = std::max(u.x, v.x), y = std::max(u.y, v.y).
 */
template <typename T>
[[nodiscard]] inline Vec2T<T>
max(const Vec2T<T>& u, const Vec2T<T>& v) noexcept;

/**
 * @brief Dot product of two 2D vectors.
 * @tparam T Floating-point precision.
 * @param[in] u First vector.
 * @param[in] v Second vector.
 * @return Scalar dot product u · v.
 */
template <typename T>
[[nodiscard]] inline T
dot(const Vec2T<T>& u, const Vec2T<T>& v) noexcept;

/**
 * @brief Euclidean length of a 2D vector.
 * @tparam T Floating-point precision.
 * @param[in] v Vector.
 * @return sqrt(v.x*v.x + v.y*v.y).
 */
template <typename T>
[[nodiscard]] inline T
length(const Vec2T<T>& v) noexcept;

/**
 * @brief Scalar-left multiplication: s * vec.
 * @tparam R Type of scalar (may differ from T, e.g. int * Vec3T<double>).
 * @tparam T Floating-point precision of the vector.
 * @param[in] s Scalar multiplier.
 * @param[in] u Vector.
 * @return New vector with X[i] = s * u[i].
 */
template <class R, typename T>
[[nodiscard]] inline constexpr Vec3T<T>
operator*(const R& s, const Vec3T<T>& u) noexcept;

/**
 * @brief Component-wise multiplication of two vectors.
 * @tparam T Floating-point precision.
 * @param[in] u First vector.
 * @param[in] v Second vector.
 * @return New vector with X[i] = u[i] * v[i].
 */
template <typename T>
[[nodiscard]] inline constexpr Vec3T<T>
operator*(const Vec3T<T>& u, const Vec3T<T>& v) noexcept;

/**
 * @brief Component-wise scalar-left division: s / vec.
 * @details Returns a new vector where each component i is s / u[i].
 * @tparam R Type of scalar numerator.
 * @tparam T Floating-point precision of the vector.
 * @param[in] s Scalar numerator.
 * @param[in] u Vector whose components are the denominators.
 * @return New vector with X[i] = s / u[i].
 */
template <class R, typename T>
[[nodiscard]] inline constexpr Vec3T<T>
operator/(const R& s, const Vec3T<T>& u) noexcept;

/**
 * @brief Component-wise minimum of two 3D vectors.
 * @tparam T Floating-point precision.
 * @param[in] u First vector.
 * @param[in] v Second vector.
 * @return New vector with X[i] = std::min(u[i], v[i]).
 */
template <typename T>
[[nodiscard]] inline constexpr Vec3T<T>
min(const Vec3T<T>& u, const Vec3T<T>& v) noexcept;

/**
 * @brief Component-wise maximum of two 3D vectors.
 * @tparam T Floating-point precision.
 * @param[in] u First vector.
 * @param[in] v Second vector.
 * @return New vector with X[i] = std::max(u[i], v[i]).
 */
template <typename T>
[[nodiscard]] inline constexpr Vec3T<T>
max(const Vec3T<T>& u, const Vec3T<T>& v) noexcept;

/**
 * @brief Dot product of two 3D vectors.
 * @tparam T Floating-point precision.
 * @param[in] u First vector.
 * @param[in] v Second vector.
 * @return Scalar dot product u · v.
 */
template <typename T>
[[nodiscard]] inline constexpr T
dot(const Vec3T<T>& u, const Vec3T<T>& v) noexcept;

/**
 * @brief Euclidean length of a 3D vector.
 * @tparam T Floating-point precision.
 * @param[in] v Vector.
 * @return sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]).
 */
template <typename T>
[[nodiscard]] inline constexpr T
length(const Vec3T<T>& v) noexcept;

} // namespace EBGeometry

#include "EBGeometry_VecImplem.hpp"

#endif
