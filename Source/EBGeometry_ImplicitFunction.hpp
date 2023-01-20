/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_ImplicitFunction.hpp
  @brief  Abstract base class for representing an implicit function.
  @author Robert Marskar
*/

#ifndef EBGeometry_ImplicitFunction
#define EBGeometry_ImplicitFunction

// Our includes
#include "EBGeometry_Vec.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Abstract representation of an implicit function function (not necessarily signed distance). 

  The value function must be implemented by subclasses. 
*/
template <class T>
class ImplicitFunction
{
public:
  /*!
    @brief Disallowed, use the full constructor
  */
  ImplicitFunction() = default;

  /*!
    @brief Destructor (does nothing)
  */
  virtual ~ImplicitFunction() = default;

  /*!
    @brief Value function. Points are outside the object if value > 0.0 and inside
    if value < 0.0
    @param[in] a_point 3D point.
  */
  virtual T
  value(const Vec3T<T>& a_point) const noexcept = 0;
};

#include "EBGeometry_NamespaceFooter.hpp"

#endif
