/* EBGeometry
 * Copyright © 2023 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file    EBGeometry_MeshDistanceFunctions.hpp
  @brief   Declaration of signed distance functions for DCEL meshes
  @author  Robert Marskar
*/

#ifndef EBGeometry_MeshDistanceFunctions
#define EBGeometry_MeshDistanceFunctions

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_SignedDistanceFunction.hpp"
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_BVH.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Signed distance function for a DCEL mesh. Does not use BVHs.
*/
template <class T>
class MeshSDF : public SignedDistanceFunction<T>
{
public:
  using Mesh = EBGeometry::DCEL::MeshT<T>;

  /*!
    @brief Disallowed constructor
  */
  MeshSDF() = delete;

  /*!
    @brief Full constructor.
    @param[in] a_mesh Input mesh
  */
  MeshSDF(const std::shared_ptr<Mesh>& a_mesh) noexcept;

  /*!
    @brief Destructor
  */
  virtual ~MeshSDF() = default;

  /*!
    @brief Value function
  */
  virtual T
  signedDistance(const Vec3T<T>& a_point) const noexcept override;

  /*!
    @brief Get the surface mesh
  */
  const std::shared_ptr<Mesh>
  getMesh() const noexcept;

protected:
  /*!
    @brief DCEL mesh
  */
  std::shared_ptr<Mesh> m_mesh;
};

/*!
  @brief Signed distance function for a DCEL mesh. This class uses the full BVH representation. 
*/
template <class T, class BV, size_t K>
class FastMeshSDF : public SignedDistanceFunction<T>
{
public:
  using Face = EBGeometry::DCEL::FaceT<T>;
  using Mesh = EBGeometry::DCEL::MeshT<T>;
  using Node = EBGeometry::BVH::NodeT<T, Face, BV, K>;

  /*!
    @brief Default disallowed constructor
  */
  FastMeshSDF() = delete;

  /*!
    @brief Full constructor. Takes the input mesh and creates the BVH.
  */
  FastMeshSDF(const std::shared_ptr<Mesh>& a_mesh) noexcept;

  /*!
    @brief Destructor
  */
  virtual ~FastMeshSDF() = default;

  /*!
    @brief Value function
    @param[in] a_point Input point. 
  */
  virtual T
  signedDistance(const Vec3T<T>& a_point) const noexcept override;

protected:
  /*!
    @brief Bounding volume hierarchy
  */
  std::shared_ptr<Node> m_bvh;
};

/*!
  @brief Signed distance function for a DCEL mesh. This class uses the compact BVH representation. 
*/
template <class T, class BV, size_t K>
class FastCompactMeshSDF : public SignedDistanceFunction<T>
{
public:
  using Face = typename EBGeometry::DCEL::FaceT<T>;
  using Mesh = typename EBGeometry::DCEL::MeshT<T>;
  using Root = typename EBGeometry::BVH::LinearBVH<T, Face, BV, K>;
  using Node = typename Root::LinearNode;

  /*!
    @brief Default disallowed constructor
  */
  FastCompactMeshSDF() = delete;

  /*!
    @brief Full constructor. Takes the input mesh and creates the BVH.
    @param[in] a_mesh Input mesh
  */
  FastCompactMeshSDF(const std::shared_ptr<Mesh>& a_mesh) noexcept;

  /*!
    @brief Destructor
  */
  virtual ~FastCompactMeshSDF() = default;

  /*!
    @brief Value function
    @param[in] a_point Input point. 
  */
  virtual T
  signedDistance(const Vec3T<T>& a_point) const noexcept override;

protected:
  /*!
    @brief Bounding volume hierarchy
  */
  std::shared_ptr<Root> m_bvh;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_MeshDistanceFunctionsImplem.hpp"

#endif
