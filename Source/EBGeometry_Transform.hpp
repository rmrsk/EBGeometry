/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_Transform
  @brief  Various transformations for implicit functions and distance fields
  @author Robert Marskar
*/

#ifndef EBGeometry_Transform
#define EBGeometry_Transform

// Our includes
#include "EBGeometry_ImplicitFunction.hpp"
#include "EBGeometry_Vec.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

namespace Transform {
  /*!
    @brief Convenience function for translating an implicit function
    @param[in] a_implicitFunction Input implicit function to be translated
    @param[in] a_shift Distance to shift
  */
  template <class T>
  std::shared_ptr<ImplicitFunction<T>>
  translate(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const Vec3T<T>& a_shift) noexcept;

  /*!
    @brief Convenience function for rotating an implicit function. 
    @param[in] a_implicitFunction Input implicit function to be rotated.
    @param[in] a_shift Distance to shift
  */
  template <class T>
  std::shared_ptr<ImplicitFunction<T>>
  rotate(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_angle, const size_t a_axis) noexcept;

  /*!
    @brief Convenience function for scaling an implicit function. 
    @param[in] a_implicitFunction Input implicit function to be scaled. 
    @param[in] a_scale Scaling factor
  */
  template <class T>
  std::shared_ptr<ImplicitFunction<T>>
  scale(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_scale) noexcept;

  /*!
    @brief Convenience function for offsetting an implicit function
    @param[in] a_implicitFunction Input implicit function to be offset
    @param[in] a_shift Distance to shift
  */
  template <class T>
  std::shared_ptr<ImplicitFunction<T>>
  offset(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_offset) noexcept;

  /*!
    @brief Convenience function for creating a shell out of an implicit function
    @param[in] a_implicitFunction Input implicit function to be shelled.
    @param[in] a_delta Shell thickness
  */
  template <class T>
  std::shared_ptr<ImplicitFunction<T>>
  annular(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_delta) noexcept;

  /*!
    @brief Convenience function for smoothing an implicit function
    @param[in] a_implicitFunction Input implicit function to be smoothed
    @param[in] a_fc Smoothing parameter
    @param[in] a_b  Smoothing parameter
  */
  template <class T>
  std::shared_ptr<ImplicitFunction<T>>
  smooth(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_fc, const T a_b) noexcept;
} // namespace Transform

/*!
  @brief Translated implicit function
*/
template <class T>
class TranslateIF : public ImplicitFunction<T>
{
public:
  /*!
    @brief No weak construction for this one
  */
  TranslateIF() = delete;

  /*!
    @brief Full constructor
    @param[in] a_implicitFunction Input implicit function to be translated
    @param[in] a_shift Distance to shift
  */
  TranslateIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const Vec3T<T>& a_shift) noexcept;

  /*!
    @brief Destructor (does nothing)
  */
  virtual ~TranslateIF();

  /*!
    @brief Value function
  */
  virtual T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /*!
    @brief Underlying implicit function
  */
  std::shared_ptr<ImplicitFunction<T>> m_implicitFunction;

  /*!
    @brief Input point translate. 
  */
  Vec3T<T> m_shift;
};

/*!
  @brief Rotated implicit function. Rotates an implicit function about an axis. 
*/
template <class T>
class RotateIF : public ImplicitFunction<T>
{
public:
  /*!
    @brief No weak construction
  */
  RotateIF() = delete;

  /*!
    @brief Rounded SDF. Rounds the input SDF
    @param[in] a_implicitFunction  Input implicit function. 
    @param[in] a_offset Offset value. 
  */
  RotateIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction,
           const T                                     a_angle,
           const size_t                                a_axis) noexcept;

  /*!
    @brief Destructor
  */
  virtual ~RotateIF();

  /*!
    @brief Implementation of value function with rotation. 
  */
  virtual T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /*!
    @brief Underlying implicit function. 
  */
  std::shared_ptr<ImplicitFunction<T>> m_implicitFunction;

  /*!
    @brief Axis to rotate about
  */
  size_t m_axis;

  /*!
    @brief Angle to rotate. 
  */
  T m_angle;

  /*!
    @brief Parameter in rotation matrix. Stored for efficiency.
  */
  T m_cosAngle;

  /*!
    @brief Parameter in rotation matrix. Stored for efficiency.
  */
  T m_sinAngle;
};

/*!
  @brief Offset implicit function. Offsets the implicit function using the input value. 
*/
template <class T>
class OffsetIF : public ImplicitFunction<T>
{
public:
  /*!
    @brief Disallowed weak construction
  */
  OffsetIF() = delete;

  /*!
    @brief Rounded SDF. Rounds the input SDF
    @param[in] a_implicitFunction Input implicit function. 
    @param[in] a_offset Offset value. 
  */
  OffsetIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_offset) noexcept;

  /*!
    @brief Destructor
  */
  virtual ~OffsetIF();

  /*!
    @brief Implementation of value function with offset. 
  */
  virtual T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /*!
    @brief Underlying implicit function. 
  */
  std::shared_ptr<ImplicitFunction<T>> m_implicitFunction;

  /*!
    @brief Offset value.
  */
  T m_offset;
};

/*!
  @brief Scaled implicit function. 
*/
template <class T>
class ScaleIF : public ImplicitFunction<T>
{
public:
  /*!
    @brief Disallowed weak construction
  */
  ScaleIF() = delete;

  /*!
    @brief Scaled implicit function. 
    @param[in] a_implicitFunction Implicit function to be scaled. 
    @param[in] a_scale            Scaling factor.
  */
  ScaleIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_scale) noexcept;

  /*!
    @brief Destructor
  */
  virtual ~ScaleIF() noexcept;

  /*!
    @brief Value function. 
    @param[in] a_point Input point.
  */
  virtual T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /*!
    @brief Original implicit function. 
  */
  std::shared_ptr<ImplicitFunction<T>> m_implicitFunction;

  /*!
    @brief Scaling factor.
  */
  T m_scale;
};

/*!
  @brief Annular implicit function function. Creates a shell out of the implicit function. 
*/
template <class T>
class AnnularIF : public ImplicitFunction<T>
{
public:
  /*!
    @brief Disallowed weak construction
  */
  AnnularIF() = delete;

  /*!
    @brief Full constructor. 
    @param[in] a_implicitFunction  Input implicit function. 
    @param[in] a_delta Shell thickness (at least if the implicit function is also signed distance)
  */
  AnnularIF(const std::shared_ptr<ImplicitFunction<T>> a_implicitFunction, const T a_delta);

  /*!
    @brief Destructor
  */
  virtual ~AnnularIF();

  /*!
    @brief Value function. 
    @param[in] a_point Input point.
  */
  virtual T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /*!
    @brief Original implicit function. 
  */
  std::shared_ptr<const ImplicitFunction<T>> m_implicitFunction;

  /*!
    @brief Shell thickness. 
  */
  T m_delta;
};

/*!
  @brief Smoothed implicit function. 
*/
template <class T>
class SmoothIF : public ImplicitFunction<T>
{
public:
  /*!
    @brief Disallowed weak construction
  */
  SmoothIF() = delete;

  /*!
    @brief Full constructor
    @param[in] a_implicitFunction  Input implicit function. 
    @param[in] a_delta Shell thickness (at least if the implicit function is also signed distance)
  */
  SmoothIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_fc, const T a_b) noexcept;

  /*!
    @brief Destructor
  */
  virtual ~SmoothIF() noexcept;

  /*!
    @brief Value function
    @param[in] a_point Input point
  */
  virtual T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /*!
    @brief Original implicit function. 
  */
  std::shared_ptr<ImplicitFunction<T>> m_implicitFunction;

  /*!
    @brief Smoothing parameter
  */
  T m_fc;

  /*!
    @brief Smoothing parameter
  */
  T m_b;

  /*!
    @brief Smoothstep kernel function using the fc parameter
    @param[in] a_point Input point
  */
  inline T
  smoothStep(const Vec3T<T>& a_point) const noexcept;

  /*!
    @brief Bump kernel
  */
  inline T
  bump(const Vec3T<T>& a_u) const noexcept;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_TransformImplem.hpp"

#endif
