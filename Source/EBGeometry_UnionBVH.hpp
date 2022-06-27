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
#include "EBGeometry_ImplicitFunction.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Implicit function union using BVHs. 
*/
template <class T, class P, class BV, size_t K>
class UnionBVH : public ImplicitFunction<T>
{
public:

  using BVConstructor = EBGeometry::BVH::BVConstructorT<P, BV>;

  /*!
    @brief Disallowed, use the full constructor
  */
  UnionBVH() = delete;

  /*!
    @brief Partial constructor. Associates distance functions but does not build
    BVH tree.
    @param[in] a_distanceFunctions Signed distance functions.
    @param[in] a_flipSign          Hook for turning inside to outside
  */
  UnionBVH(const std::vector<std::shared_ptr<P>>& a_distanceFunctions, const bool a_flipSign);

  /*!
    @brief Full constructor.
    @param[in] a_distanceFunctions Signed distance functions.
    @param[in] a_flipSign          Hook for turning inside to outside
    @param[in] a_bvConstructor     Bounding volume constructor.
  */
  UnionBVH(const std::vector<std::shared_ptr<P>>& a_distanceFunctions,
           const bool                               a_flipSign,
           const BVConstructor&                     a_bvConstructor);

  /*!
    @brief Build BVH tree for the input objects. User must supply a partitioner
    and a BV constructor for the SDF objects.
    @param[in] a_bvConstructor Constructor for building a bounding volume that
    encloses an object.
  */
  void
  buildTree(const BVConstructor& a_bvConstructor);

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

protected:

  using PrimList = std::vector<std::shared_ptr<const P>>;
  using BuilderNode = EBGeometry::BVH::NodeT<T, P, BV, K>;
  using RootNode = EBGeometry::BVH::LinearBVH<T, P, BV, K>;

  /*!
    @brief List of primitive functions. 
  */
  std::vector<std::shared_ptr<const P>> m_distanceFunctions;

  /*!
    @brief Root node for BVH tree
  */
  std::shared_ptr<RootNode> m_rootNode;

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
