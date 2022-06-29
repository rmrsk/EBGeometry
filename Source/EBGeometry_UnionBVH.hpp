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
#include <type_traits>

// Our includes
#include "EBGeometry_ImplicitFunction.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Implicit function union using BVHs. 
  @note If the BVH-enabled union is to make sense, the primitives must be 
  distance fields (I think). There's a static_assert to make sure of that. 
*/
template <class T, class P, class BV, size_t K>
class UnionBVH : public ImplicitFunction<T>
{
public:
  static_assert(std::is_base_of<EBGeometry::SignedDistanceFunction<T>, P>::value);

  using BVConstructor = EBGeometry::BVH::BVConstructorT<P, BV>;

  /*!
    @brief Disallowed, use the full constructor
  */
  UnionBVH() = delete;

  /*!
    @brief Full constructor.
    @param[in] a_distanceFunctions Signed distance functions.
    @param[in] a_flipSign          Hook for turning inside to outside
    @param[in] a_bvConstructor     Bounding volume constructor.
  */
  UnionBVH(const std::vector<std::shared_ptr<P>>& a_distanceFunctions,
           const bool                             a_flipSign,
           const BVConstructor&                   a_bvConstructor);
  
  /*!
    @brief Destructor (does nothing)
  */
  virtual ~UnionBVH() = default;

  /*!
    @brief Value function
    @param[in] a_point 3D point.
  */
  T
  value(const Vec3T<T>& a_point) const noexcept override;

  /*!
    @brief Get the bounding volume
  */
  const BV&
  getBoundingVolume() const noexcept;

protected:

  /*!
    @brief Root node for linearized BVH tree
  */
  std::shared_ptr<EBGeometry::BVH::LinearBVH<T, P, BV, K>> m_rootNode;

  /*!
    @brief Hook for turning inside to outside
  */
  bool m_flipSign;

  /*!
    @brief Build BVH tree for the input objects. User must supply a partitioner
    and a BV constructor for the SDF objects.
    @param[in] a_bvConstructor Constructor for building a bounding volume that
    encloses an object.
  */
  inline void
  buildTree(const std::vector<std::shared_ptr<P>>& a_distanceFunctions, const BVConstructor& a_bvConstructor) noexcept; 
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_UnionBVHImplem.hpp"

#endif
