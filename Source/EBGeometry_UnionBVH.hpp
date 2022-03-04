/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_UnionBVH.hpp
  @brief  Declaration of a union operator for creating multi-object scenes. 
  @author Robert Marskar
*/

#ifndef EBGeometry_UnionBVH
#define EBGeometry_UnionBVH

// Std includes
#include <vector>

// Our includes
#include "EBGeometry_SignedDistanceFunction.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Distance function union using BVHs. Computes the signed distance to the closest object of N non-overlapping objects. 
  @note This class only makes sense if the object do not overlap. 
*/
template <class T>
class UnionBVH : public SignedDistanceFunction<T> {
public:

  /*!
    @brief Alias for cutting down on typing. 
  */
  using SDF = SignedDistanceFunction<T>;

  /*!
    @brief Disallowed, use the full constructor
  */
  UnionBVH() = delete;

  /*!
    @brief Full constructor. Computes the signed distance 
    @param[in] a_distanceFunctions
    @param[in] a_flipSign Hook for turning inside to outside
  */
  UnionBVH(const std::vector<std::shared_ptr<const SDF> >& a_distanceFunctions, const bool a_flipSign);

  /*!
    @brief Destructor (does nothing)
  */
  virtual ~UnionBVH() = default;

  /*!
    @brief Value function
    @param[in] a_point 3D point. 
  */
  T operator()(const Vec3T<T>& a_point) const noexcept override;  
  
protected:

  /*!
    @brief List of distance functions
  */
  std::vector<std::shared_ptr<const SDF> > m_distanceFunctions;

  /*!
    @brief Is good or not
  */
  bool m_isGood;

  /*!
    @brief Hook for turning inside to outside
  */
  bool m_flipSign;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_UnionBVHImplem.hpp"

#endif
