/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_Vec.hpp
  @brief  Declaration of 2D and 3D point/vector classes with templated
  precision. Used with DCEL tools.
  @author Robert Marskar
*/

#ifndef EBGeometry_Vec
#define EBGeometry_Vec

// Our includes
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Two-dimensional vector class with arithmetic operators.
  @details The class has a public-only interface. To change it's components one
  can call the member functions, or set components directly, e.g. vec.x = 5.0
  @note Vec2T is a templated class primarily used with DCEL grids.
*/
template <class T>
class Vec2T
{
public:
  /*!
    @brief Default constructor. Sets the vector to the zero vector.
  */
  Vec2T();

  /*!
    @brief Copy constructor
    @param[in] u Other vector
    @details Sets *this = u
  */
  Vec2T(const Vec2T& u) noexcept;

  /*!
    @brief Full constructor
    @param[in] a_x First vector component
    @param[in] a_y Second vector component
    @details Sets this->x = a_x and this->y = a_y
  */
  Vec2T(const T& a_x, const T& a_y);

  /*!
    @brief Destructor (does nothing)
  */
  ~Vec2T() = default;

  /*!
    @brief First component in the vector
  */
  T x;

  /*!
    @brief Second component in the vector
  */
  T y;

  /*!
    @brief Return av vector with x = y = 0
  */
  inline static constexpr Vec2T<T>
  zero() noexcept;

  /*!
    @brief Return av vector with x = y = 1
  */
  inline static constexpr Vec2T<T>
  one() noexcept;

  /*!
    @brief Return minimum possible representative vector.
  */
  inline static constexpr Vec2T<T>
  min() noexcept;

  /*!
    @brief Return maximum possible representative vector.
  */
  inline static constexpr Vec2T<T>
  max() noexcept;

  /*!
    @brief Return a vector with inf components.
  */
  inline static constexpr Vec2T<T>
  infinity() noexcept;

  /*!
    @brief Assignment operator. Sets this.x = a_other.x and this.y = a_other.y
    @param[in] a_other Other vector
  */
  inline Vec2T<T>&
  operator=(const Vec2T& a_other) noexcept;

  /*!
    @brief Addition operator.
    @param[in] a_other Other vector
    @details Returns a new object with component x = this->x + a_other.x (same
    for y-component)
  */
  inline Vec2T<T>
  operator+(const Vec2T& a_other) const noexcept;

  /*!
    @brief Subtraction operator.
    @param[in] a_other Other vector
    @details Returns a new object with component x = this->x - a_other.x (same
    for y-component)
  */
  inline Vec2T<T>
  operator-(const Vec2T& a_other) const noexcept;

  /*!
    @brief Negation operator. Returns a new Vec2T<T> with negated components
  */
  inline Vec2T<T>
  operator-() const noexcept;

  /*!
    @brief Multiplication operator
    @param[in] s Scalar to be multiplied
    @details Returns a new Vec2T<T> with components x = s*this->x (and same for
    y)
  */
  inline Vec2T<T>
  operator*(const T& s) const noexcept;

  /*!
    @brief Division operator
    @param[in] s Scalar to be multiplied
    @details Returns a new Vec2T<T> with components x = (1/s)*this->x (and same
    for y)
  */
  inline Vec2T<T>
  operator/(const T& s) const noexcept;

  /*!
    @brief Addition operator
    @param[in] a_other Other vector to add
    @details Returns (*this) with components this->x = this->x + a_other.x (and
    same for y)
  */
  inline Vec2T<T>&
  operator+=(const Vec2T& a_other) noexcept;

  /*!
    @brief Subtraction operator
    @param[in] a_other Other vector to subtract
    @details Returns (*this) with components this->x = this->x - a_other.x (and
    same for y)
  */
  inline Vec2T<T>&
  operator-=(const Vec2T& a_other) noexcept;

  /*!
    @brief Multiplication operator
    @param[in] s Scalar to multiply by
    @details Returns (*this) with components this->x = s*this->x (and same for
    y)
  */
  inline Vec2T<T>&
  operator*=(const T& s) noexcept;

  /*!
    @brief Division operator operator
    @param[in] s Scalar to divide by
    @details Returns (*this) with components this->x = (1/s)*this->x (and same
    for y)
  */
  inline Vec2T<T>&
  operator/=(const T& s) noexcept;

  /*!
    @brief Dot product operator
    @param[in] a_other other vector
    @details Returns the dot product, i.e. this->x*a_other.x + this->y+a_other.y
  */
  inline T
  dot(const Vec2T& a_other) const noexcept;

  /*!
    @brief Compute length of vector
    @return Returns length of vector, i.e. sqrt[(this->x)*(this->x) +
    (this->y)*(this->y)]
  */
  inline T
  length() const noexcept;

  /*!
    @brief Compute square of vector
    @return Returns length of vector, i.e. (this->x)*(this->x) +
    (this->y)*(this->y)
  */
  inline T
  length2() const noexcept;
};

/*!
  @brief Multiplication operator in the form s*Vec2T
  @param[in] s Multiplication factor
  @param[in] a_other Other vector
  @return Returns a new vector with components x = s*a_other.x (and same for y)
*/
template <class T>
inline Vec2T<T>
operator*(const T& s, const Vec2T<T>& a_other) noexcept;

/*!
  @brief Division operator in the form s*Vec2T.
  @param[in] s Division factor
  @param[in] a_other Other vector
  @return Returns a new vector with components x = (1/s)*a_other.x (and same for
  y)
*/
template <class T>
inline Vec2T<T>
operator/(const T& s, const Vec2T<T>& a_other) noexcept;

/*!
  @brief Three-dimensional vector class with arithmetic operators.
  @details The class has a public-only interface. To change it's components one
  can call the member functions, or set components directly, e.g. vec.x = 5.0
  @note Vec3T is a templated class primarily used with DCEL grids. It is always
  3D, i.e. independent of Chombo configuration settings. This lets one use DCEL
  functionality even though the simulation might only be 2D.
*/
template <class T>
class Vec3T
{
public:
  /*!
    @brief Default constructor. Sets the vector to the zero vector.
  */
  Vec3T();

  /*!
    @brief Copy constructor
    @param[in] a_u Other vector
    @details Sets *this = u
  */
  Vec3T(const Vec3T<T>& a_u) noexcept;

  /*!
    @brief Full constructor
    @param[in] a_x First vector component
    @param[in] a_y Second vector component
    @param[in] a_z Third vector component
    @details Sets this->x = a_x, this->y = a_y, and this->z = a_z
  */
  Vec3T(const T& a_x, const T& a_y, const T& a_z);

  /*!
    @brief Destructor (does nothing)
  */
  ~Vec3T() = default;

  /*!
    @brief Return av vector with x = y = z = 0
  */
  inline static constexpr Vec3T<T>
  zero() noexcept;

  /*!
    @brief Return av vector with x = y = z = 1
  */
  inline static constexpr Vec3T<T>
  one() noexcept;

  /*!
    @brief Return av vector with x = y = z = 1
    @param[in] a_dir Dircetion
  */
  inline static constexpr Vec3T<T>
  unit(const size_t a_dir) noexcept;

  /*!
    @brief Return a vector with minimum representable components.
  */
  inline static constexpr Vec3T<T>
  min() noexcept;

  /*!
    @brief Return a vector with maximum representable components.
  */
  inline static constexpr Vec3T<T>
  max() noexcept;

  /*!
    @brief Return a vector with inf components.
  */
  inline static constexpr Vec3T<T>
  infinity() noexcept;

  /*!
    @brief Return component in vector. (i=0 => x and so on)
    @param[in] i Index. Must be < 3
  */
  inline T&
  operator[](size_t i) noexcept;

  /*!
    @brief Return non-modifiable component in vector. (i=0 => x and so on)
    @param[in] i Index. Must be < 3
  */
  inline const T&
  operator[](size_t i) const noexcept;

  /*!
    @brief Comparison operator. Returns true if all components are the same
    @param[in] u Other vector
  */
  inline bool
  operator==(const Vec3T<T>& u) const noexcept;

  /*!
    @brief "Smaller than" operator.
    @details Returns true if this->x < u.x AND this->y < u.y AND this->z < u.z
    and false otherwise
    @param[in] u Other vector
  */
  inline bool
  operator<(const Vec3T<T>& u) const noexcept;

  /*!
    @brief "Greater than" operator.
    @details Returns true if this->x > u.x AND this->y > u.y AND this->z > u.z
    @param[in] u Other vector
  */
  inline bool
  operator>(const Vec3T<T>& u) const noexcept;

  /*!
    @brief "Smaller or equal to" operator.
    @details Returns true if this->x <= u.x AND this->y <= u.y AND this->z <=
    u.z
    @param[in] u Other vector
  */
  inline bool
  operator<=(const Vec3T<T>& u) const noexcept;

  /*!
    @brief "Greater or equal to" operator.
    @details Returns true if this->x >= u.x AND this->y >= u.y AND this->z >=
    u.z
    @param[in] u Other vector
  */
  inline bool
  operator>=(const Vec3T<T>& u) const noexcept;

  /*!
    @brief Assignment operator. Sets components equal to the argument vector's
    components
    @param[in] u Other vector
  */
  inline Vec3T<T>&
  operator=(const Vec3T<T>& u) noexcept;

  /*!
    @brief Addition operator. Returns a new vector with added components
    @return Returns a new vector with x = this->x - u.x and so on.
    @param[in] u Other vector
  */
  inline Vec3T<T>
  operator+(const Vec3T<T>& u) const noexcept;

  /*!
    @brief Subtraction operator. Returns a new vector with subtracted components
    @return Returns a new vector with x = this->x - u.x and so on.
    @param[in] u Other vector
  */
  inline Vec3T<T>
  operator-(const Vec3T<T>& u) const noexcept;

  /*!
    @brief Negation operator. Returns a vector with negated components
  */
  inline Vec3T<T>
  operator-() const noexcept;

  /*!
    @brief Multiplication operator. Returns a vector with scalar multiplied
    components
    @param[in] s Scalar to multiply by
    @return Returns a new vector with X[i] = this->X[i] * s
  */
  inline Vec3T<T>
  operator*(const T& s) const noexcept;

  /*!
    @brief Component-wise multiplication operator
    @param[in] s Scalar to multiply by
    @return Returns a new vector with X[i] = this->X[i] * s[i] for each
    component.
  */
  inline Vec3T<T>
  operator*(const Vec3T<T>& s) const noexcept;

  /*!
    @brief Division operator. Returns a vector with scalar divided components
    @param[in] s Scalar to divided by
    @return Returns a new vector with X[i] = this->X[i] / s
  */
  inline Vec3T<T>
  operator/(const T& s) const noexcept;

  /*!
    @brief Component-wise division operator.
    @param[in] v Other vector
    @return Returns a new vector with X[i] = this->X[i]/v[i] for each component.
  */
  inline Vec3T<T>
  operator/(const Vec3T<T>& v) const noexcept;

  /*!
    @brief Vector addition operator.
    @param[in] u Vector to add
    @return Returns (*this) with incremented components, e.g. this->X[0] =
    this->X[0] + u.X[0]
  */
  inline Vec3T<T>&
  operator+=(const Vec3T<T>& u) noexcept;

  /*!
    @brief Vector subtraction operator.
    @param[in] u Vector to subtraction
    @return Returns (*this) with subtracted components, e.g. this->X[0] =
    this->X[0] - u.X[0]
  */
  inline Vec3T<T>&
  operator-=(const Vec3T<T>& u) noexcept;

  /*!
    @brief Vector multiplication operator.
    @param[in] s Scalar to multiply by
    @return Returns (*this) with multiplied components, e.g. this->X[0] =
    this->X[0] * s
  */
  inline Vec3T<T>&
  operator*=(const T& s) noexcept;

  /*!
    @brief Vector division operator.
    @param[in] s Scalar to divide by
    @return Returns (*this) with multiplied components, e.g. this->X[0] =
    this->X[0] / s
  */
  inline Vec3T<T>&
  operator/=(const T& s) noexcept;

  /*!
    @brief Vector minimum function. Returns a new vector with componentwise
    minimums.
    @param[in] u Other vector
    @return Returns a new vector with X[0] = std::min(this->X[0], u.X[0]) (and
    same for the other components)
  */
  inline Vec3T<T>
  min(const Vec3T<T>& u) noexcept;

  /*!
    @brief Vector maximum function. Returns a new vector with componentwise
    maximums.
    @param[in] u Other vector
    @return Returns a new vector with X[0] = std::minmax->X[0], u.X[0]) (and
    same for the other components)
  */
  inline Vec3T<T>
  max(const Vec3T<T>& u) noexcept;

  /*!
    @brief Vector cross product
    @param[in] u Other vector
    @returns Returns the cross product between (*this) and u
  */
  inline Vec3T<T>
  cross(const Vec3T<T>& u) const noexcept;

  /*!
    @brief Vector dot product
    @param[in] u Other vector
    @returns Returns the dot product between (*this) and u
  */
  inline T
  dot(const Vec3T<T>& u) const noexcept;

  /*!
    @brief Return the direction which has the smallest component (can be
    absolute)
    @param[in] a_doAbs If true, evaluate component magnitudes rather than
    values.
    @return Direction with the biggest component
  */
  inline size_t
  minDir(const bool a_doAbs) const noexcept;

  /*!
    @brief Return the direction which has the largest component (can be
    absolute)
    @param[in] a_doAbs If true, evaluate component magnitudes rather than
    values.
    @return Direction with the biggest component
  */
  inline size_t
  maxDir(const bool a_doAbs) const noexcept;

  /*!
    @brief Compute vector length
    @return Returns the vector length, i.e. sqrt(X[0]*X[0] + X[1]*X[1] +
    Y[0]*Y[0])
  */
  inline T
  length() const noexcept;

  /*!
    @brief Compute vector length squared
    @return Returns the vector length squared, i.e. (X[0]*X[0] + X[1]*X[1] +
    Y[0]*Y[0])
  */
  inline T
  length2() const noexcept;

protected:
  /*!
    @brief Vector components
  */
  T X[3];
};

/*!
  @brief Multiplication operator.
  @param[in] s Multiplication scalar
  @param[in] u Vector
  @return Returns new vector with components X[0] = s*X[0] and so on.
*/
template <class R, class T>
inline Vec3T<T>
operator*(const R& s, const Vec3T<T>& u) noexcept;

/*!
  @brief Division operator.
  @param[in] s Division scalar
  @param[in] u Vector
  @return Returns new vector with components X[0] = X[0]/s and so on.
*/
template <class R, class T>
inline Vec3T<T>
operator/(const R& s, const Vec3T<T>& u) noexcept;

/*!
  @brief Minimum function. Returns new vector with component-wise minimums.
  @param[in] u Vector
  @param[in] v Other vector
  @return Returns new vector with components X[0] = std::min(u.X[0], v.X[0]) and
  so on
*/
template <class T>
inline Vec3T<T>
min(const Vec3T<T>& u, const Vec3T<T>& v) noexcept;

/*!
  @brief Maximum function. Returns new vector with component-wise minimums.
  @param[in] u Vector
  @param[in] v Other vector
  @return Returns new vector with components X[0] = std::max(u.X[0], v.X[0]) and
  so on
*/
template <class T>
inline Vec3T<T>
max(const Vec3T<T>& u, const Vec3T<T>& v) noexcept;

/*!
  @brief Dot product function.
  @param[in] u Vector
  @param[in] v Other vector
*/
template <class T>
inline T
dot(const Vec3T<T>& u, const Vec3T<T>& v) noexcept;

/*!
  @brief Length function
  @param[in] v Vector.
*/
template <class T>
inline T
length(const Vec3T<T>& v) noexcept;

/*!
  @brief Minimum function. Returns new vector with component-wise minimums.
  @param[in] u Vector
  @param[in] v Other vector
  @return Returns new vector with components x = std::min(u.x, v.x).
*/
template <class T>
inline Vec2T<T>
min(const Vec2T<T>& u, const Vec2T<T>& v) noexcept;

/*!
  @brief Maximum function. Returns new vector with component-wise minimums.
  @param[in] u Vector
  @param[in] v Other vector
  @return Returns new vector with components x = std::max(u.x, v.x).
*/
template <class T>
inline Vec2T<T>
max(const Vec2T<T>& u, const Vec2T<T>& v) noexcept;

/*!
  @brief Dot product function.
  @param[in] u Vector
  @param[in] v Other vector
*/
template <class T>
inline T
dot(const Vec2T<T>& u, const Vec2T<T>& v) noexcept;

/*!
  @brief Length function
  @param[in] v Vector.
*/
template <class T>
inline T
length(const Vec2T<T>& v) noexcept;

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_VecImplem.hpp"

#endif
