/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_SignedDistanceDcel.hpp
  @brief  Declaration of a signed-distance object that gets its value function from a DCEL surface tesselation
  @author Robert Marskar
*/

#ifndef EBGeometry_SignedDistanceDcel
#define EBGeometry_SignedDistanceDcel

// Std includes
#include <memory>

// Our includes
#include "EBGeometry_DcelMesh.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Signed distance function from a DCEL mesh. 
  @details This does not use a bounding volume hierarchy for performance. The template parameter is the floating point precision. 
*/
template <class T>
class SignedDistanceDcel {
public:

  /*!
    @brief Alias for DCEl mesh with precision T
  */
  using Mesh = Dcel::MeshT<T>;

  /*!
    @brief Disallowed, use the full constructor
  */
  SignedDistanceDcel() = delete;

  /*!
    @brief Full constructor
    @param[in] a_mesh DCEL mesh
    @param[in] a_flipSign Hook for turning inside to outside
  */
  SignedDistanceDcel(const std::shared_ptr<Mesh>& a_mesh, const bool a_flipSign);

  /*!
    @brief Copy constructor
    @param[in] a_object Other distance function
  */
  SignedDistanceDcel(const SignedDistanceDcel& a_object);

  /*!
    @brief Destructor (does nothing)
  */
  virtual ~SignedDistanceDcel();

  /*!
    @brief Value function
    @param[in] a_point 3D point. 
  */
  T operator()(const Vec3T<T>& a_point) const;
  
protected:

  /*!
    @brief Half-edge mesh structure. 
  */
  std::shared_ptr<Mesh> m_mesh;

  /*!
    @brief Hook for turning inside to outside
  */
  bool m_flipSign;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_SignedDistanceDcelImplem.hpp"

#endif
