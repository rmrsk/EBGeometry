/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_SignedDistanceFunction.hpp
  @brief  Abstract base class for representing a signed distance function.
  @author Robert Marskar
*/

#ifndef EBGeometry_SignedDistanceFunction
#define EBGeometry_SignedDistanceFunction

#include <memory>
#include <deque>

// Our includes
#include "EBGeometry_ImplicitFunction.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Abstract representation of a signed distance function.
  @details Users can put whatever they like in here, e.g. analytic functions,
  DCEL meshes, or DCEL meshes stored in full or compact BVH trees. The
  signedDistance function must be implemented by the user. When computing it,
  the user can apply transformation operators (rotations, scaling, translations)
  by calling transformPoint on the input coordinate.
*/
template <class T>
class SignedDistanceFunction : public ImplicitFunction<T>
{
public:
  /*!
    @brief Disallowed, use the full constructor
  */
  SignedDistanceFunction() = default;

  /*!
    @brief Destructor (does nothing)
  */
  virtual ~SignedDistanceFunction() = default;

  /*!
    @brief Implementation of ImplicitFunction::value
    @param[in] a_point 3D point.
  */
  virtual T
  value(const Vec3T<T>& a_point) const noexcept override final;

  /*!
    @brief Signed distance function.
    @param[in] a_point 3D point.
  */
  virtual T
  signedDistance(const Vec3T<T>& a_point) const noexcept = 0;

  /*!
    @brief Signed distance normal vector.
    @details Computed using finite differences with step a_delta
    @param[in] a_point 3D point
    @param[in] a_delta Finite difference step
  */
  inline virtual Vec3T<T>
  normal(const Vec3T<T>& a_point, const T& a_delta) const noexcept;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_SignedDistanceFunctionImplem.hpp"

#endif
