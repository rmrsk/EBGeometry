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

// Our includes
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Abstract representation of a signed distance function. 
*/
template <class T>
class SignedDistanceFunction {
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
    @brief Signed distance function. 
    @param[in] a_point 3D point. 
  */
  virtual T signedDistance(const Vec3T<T>& a_point) const noexcept = 0;

  /*!
    @brief Unsigned distance function. Must return the distance^2. 
    @param[in] a_point 3D point. 
  */
  virtual T unsignedDistance2(const Vec3T<T>& a_point) const noexcept {
    return std::pow(this->signedDistance(a_point), 2);
  }
};

#include "EBGeometry_NamespaceFooter.hpp"

#endif
