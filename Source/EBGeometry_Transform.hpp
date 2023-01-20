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
    @brief Convenience function for taking the complement of an implicit function
    @param[in] a_implicitFunction Input implicit function
  */
  template <class T>
  std::shared_ptr<ImplicitFunction<T>>
  complement(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction) noexcept;

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
    @brief Convenience function for blurring an implicit function
    @param[in] a_implicitFunction Input implicit function to be blurred
    @param[in] a_blurDistance Smoothing distance
  */
  template <class T>
  std::shared_ptr<ImplicitFunction<T>>
  blur(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_blur) noexcept;

  /*!
    @brief Convenience function for mollification with an input sphere. 
    @param[in] a_implicitFunction Input implicit function to be mollifier
    @param[in] a_dist Mollification distance. 
  */
  template <class T>
  std::shared_ptr<ImplicitFunction<T>>
  mollify(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_dist) noexcept;
} // namespace Transform

/*!
  @brief Complemented implicit function
*/
template <class T>
class ComplementIF
{
public:
  /*!
    @brief No weak construction for this one
  */
  ComplementIF() = delete;

  /*!
    @brief Full constructor
    @param[in] a_implicitFunction Input implicit function
  */
  ComplementIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction) noexcept;

  /*!
    @brief Destructor (does nothing)
  */
  virtual ~ComplementIF();

  /*!
    @brief Value function
  */
  virtual T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /*!
    @brief Implicit function
  */
  std::shared_ptr<ImplicitFunction<T>> m_implicitFunction;
};

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
  @brief Blurred/interpolated implicit function - can be used for smoothing.
  @details This passed the input implicit function through a trilinear filter with specified distance. We blur the function 
  using f = alpha * f + (1-alpha)/2 * [f(x+d) + f(x-d)]
*/

template <class T>
class BlurIF : public ImplicitFunction<T>
{
public:
  /*!
    @brief Disallowed weak constructino
  */
  BlurIF() = delete;

  /*!
    @brief Full constructor.
    @param[in] a_implicitFunction Input implicit function
    @param[in] a_blurDistance     Blur distance
    @param[in] a_alpha            Blur factor. 
  */
  BlurIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction,
         const T                                     a_blurDistance,
         const T                                     a_alpha = 0.5) noexcept;

  /*!
   @brief Destructor
  */
  virtual ~BlurIF() noexcept;

  /*!
    @brief Value function
    @param[in] a_point Input point
  */
  virtual T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /*!
    @brief Original implicit function
  */
  std::shared_ptr<ImplicitFunction<T>> m_implicitFunction;

  /*!
    @brief Blur distance.
  */
  T m_blurDistance;

  /*!
    @brief Alpha factor
  */
  T m_alpha;
};

/*!
  @brief Mollified implicit function. 
*/
template <class T>
class MollifyIF : public ImplicitFunction<T>
{
public:
  /*!
    @brief Disallowed weak construction
  */
  MollifyIF() = delete;

  /*!
    @brief Full constructor
    @param[in] a_implicitFunction  Input implicit function. 
    @param[in] a_mollifier         Mollifier
    @param[in] a_maxVal            Max/min val where mollifier is applied
    @param[in] a_numPoints         Number of points to use in mollification convolution kernel. Must be >= 1.
  */
  MollifyIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction,
            const std::shared_ptr<ImplicitFunction<T>>& a_mollifier,
            const T                                     a_maxValue,
            const size_t                                a_numPoints) noexcept;

  /*!
    @brief Destructor
  */
  virtual ~MollifyIF() noexcept;

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
    @brief Mollifier
  */
  std::shared_ptr<ImplicitFunction<T>> a_mollifier;

  /*!
    @brief Mollifier Weights
  */
  std::vector<std::pair<Vec3T<T>, T>> m_sampledMollifier;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_TransformImplem.hpp"

#endif