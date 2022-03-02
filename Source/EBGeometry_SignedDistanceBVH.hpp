/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_SignedDistanceBVH.hpp
  @brief  Declaration of a signed-distance object that uses bounding volume hiearchices for accelerating signed queries from DCEL tesselations. 
  @author Robert Marskar
*/

#ifndef EBGeometry_SignedDistanceBVH
#define EBGeometry_SignedDistanceBVH

// Std includes
#include <memory>

// Our includes
#include "EBGeometry_DcelMesh.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Signed distance function from a DCEL mesh, using BVHs for accelerating the query. 
  @details This class stores a reference to the node in the BVH tree. For that reason, the class is templated using the precision (T), the bounding volume type
  that encloses the primitives (BV), and the tree degree (K. 
*/
template <class T, class BV, int K>
class SignedDistanceBVH {
public:

  /*!
    @brief Alias for DCEL faces
  */
  using Face = EBGeometry::Dcel::FaceT<T>;

  /*!
    @brief Alias for BVH node
  */
  using Node = EBGeometry::BVH::NodeT<T, Face, BV, K>;

  /*!
    @brief Alias for DCEl mesh with precision T
  */
  using Mesh = Dcel::MeshT<T>;

  /*!
    @brief Disallowed, use the full constructor
  */
  SignedDistanceBVH() = delete;

  /*!
    @brief Full constructor
    @param[in] a_rootNode Reference to root node in BVH. 
    @param[in] a_flipSign Hook for turning inside to outside
  */
  SignedDistanceBVH(const std::shared_ptr<Node>& a_rootNode, const bool a_flipSign);

  /*!
    @brief Copy constructor
    @param[in] a_object Other distance function
  */
  SignedDistanceBVH(const SignedDistanceBVH& a_object);

  /*!
    @brief Destructor (does nothing)
  */
  virtual ~SignedDistanceBVH();

  /*!
    @brief Value function
    @param[in] a_point 3D point. 
  */
  T operator()(const Vec3T<T>& a_point) const;
  
protected:

  /*!
    @brief Reference to root node in DCEL mesh. 
  */
  std::shared_ptr<Node> m_rootNode;

  /*!
    @brief Hook for turning inside to outside
  */
  bool m_flipSign;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_SignedDistanceBVHImplem.hpp"

#endif
