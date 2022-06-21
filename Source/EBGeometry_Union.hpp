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
  @brief CSG union. Computes the minimum of all input implicit functions. This
  is defined also when objects in the scene overlap one another. It will also
  be well-defined for signed distance functions.
*/
template <class T>
class Union : public ImplicitFunction<T>
{
public:
  using ImpFunc = ImplicitFunction<T>;

  using SDF = SignedDistanceFunction<T>;

  /*!
    @brief Disallowed, use the full constructor
  */
  Union() = delete;

  /*!
    @brief Full constructor. Computes the CSG union
    @param[in] a_implictFunctions Implicit functions
    @param[in] a_flipSign         Hook for turning inside to outside
  */
  Union(const std::vector<std::shared_ptr<ImpFunc>>& a_implicitFunctions, const bool a_flipSign);

  /*!
    @brief Full constructor. 
    @param[in] a_implictFunctions Implicit functions
    @param[in] a_flipSign         Hook for turning inside to outside
  */
  Union(const std::vector<std::shared_ptr<SDF>>& a_distanceFunctions, const bool a_flipSign);

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
    @brief List of implicit functions
  */
  std::vector<std::shared_ptr<const ImpFunc>> m_implicitFunctions;

  /*!
    @brief Hook for turning inside to outside
  */
  bool m_flipSign;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_UnionImplem.hpp"

#endif
