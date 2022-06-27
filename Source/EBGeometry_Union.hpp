/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_Union.hpp
  @brief  Declaration of a union operator for creating multi-object scenes.
  @author Robert Marskar
*/

#ifndef EBGeometry_Union
#define EBGeometry_Union

// Std includes
#include <vector>

// Our includes
#include "EBGeometry_ImplicitFunction.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief CSG union. Computes the minimum of all input primitives. This
  is defined also when objects in the scene overlap one another. It will also
*/
template <class P, class T>
class Union : public ImplicitFunction<T>
{
public:

  /*!
    @brief Disallowed, use the full constructor
  */
  Union() = delete;

  /*!
    @brief Full constructor. Computes the CSG union
    @param[in] a_primitives List of primitives
    @param[in] a_flipSign   Hook for turning inside to outside
  */
  Union(const std::vector<std::shared_ptr<P>>& a_primitives, const bool a_flipSign);

  /*!
    @brief Destructor (does nothing)
  */
  virtual ~Union() = default;

  /*!
    @brief Value function
    @param[in] a_point 3D point.
  */
  T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /*!
    @brief List of primitives
  */
  std::vector<std::shared_ptr<const P>> m_primitives;

  /*!
    @brief Hook for turning inside to outside
  */
  bool m_flipSign;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_UnionImplem.hpp"

#endif
