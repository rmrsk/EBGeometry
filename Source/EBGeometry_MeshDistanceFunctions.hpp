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
#include "EBGeometry_Triangle.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

namespace DCEL {

  /*!
    @brief One-liner for turning a DCEL mesh into a full-tree BVH. 
    @param[in] a_dcelMesh Input DCEL mesh.
    @param[in] a_build Build specification for BVH.
    @return Returns a pointer to a full-tree BVH representation of the DCEL faces.
  */
  template <class T, class Meta, class BV, size_t K>
  std::shared_ptr<EBGeometry::BVH::NodeT<T, FaceT<T, Meta>, BV, K>>
  buildFullBVH(const std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>& a_dcelMesh,
               const BVH::Build                                         a_build = BVH::Build::TopDown) noexcept;
} // namespace DCEL

/*!
  @brief Signed distance function for a DCEL mesh. Does not use BVHs.
*/
template <class T, class Meta = DCEL::DefaultMetaData>
class MeshSDF : public SignedDistanceFunction<T>
{
public:
  /*!
    @brief Alias for DCEL mesh type
  */
  using Mesh = EBGeometry::DCEL::MeshT<T, Meta>;

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

  /*!
    @brief Compute bounding volume for this mesh. 
  */
  template <class BV>
  BV
  computeBoundingVolume() const noexcept;

protected:
  /*!
    @brief DCEL mesh
  */
  std::shared_ptr<Mesh> m_mesh;
};

/*!
  @brief Signed distance function for a DCEL mesh. This class uses the full BVH representation. 
*/
template <class T, class Meta, class BV, size_t K>
class FastMeshSDF : public SignedDistanceFunction<T>
{
public:
  /*!
    @brief Alias for DCEL face type
  */
  using Face = typename EBGeometry::DCEL::FaceT<T, Meta>;

  /*!
    @brief Alias for DCEL mesh type
  */
  using Mesh = typename EBGeometry::DCEL::MeshT<T, Meta>;

  /*!
    @brief Alias for BVH root node 
  */
  using Node = EBGeometry::BVH::NodeT<T, Face, BV, K>;

  /*!
    @brief Default disallowed constructor
  */
  FastMeshSDF() = delete;

  /*!
    @brief Full constructor. Takes the input mesh and creates the BVH.
    @param[in] a_mesh Input mesh
    @param[in] a_build Specification of build method. Must be TopDown, Morton, or Nested.
  */
  FastMeshSDF(const std::shared_ptr<Mesh>& a_mesh, const BVH::Build a_build = BVH::Build::TopDown) noexcept;

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

  /*!
    @brief Get the closest faces to the input point
    @details This returns a list of candidate faces that are close to the input point. The returned
    argment consists of the faces and the unsigned distance to the face.
    @param[in] a_point Input ponit
    @param[in] a_sorted Sort the output vector by distance or not. Closest goes first
    @return List of candidate faces (potentially sorted)
  */
  virtual std::vector<std::pair<std::shared_ptr<const Face>, T>>
  getClosestFaces(const Vec3T<T>& a_point, const bool a_sorted) const noexcept;

  /*!
    @brief Get the bounding volume hierarchy enclosing the mesh
  */
  virtual std::shared_ptr<Node>&
  getBVH() noexcept;

  /*!
    @brief Get the bounding volume hierarchy enclosing the mesh
  */
  virtual const std::shared_ptr<Node>&
  getBVH() const noexcept;

  /*!
    @brief Compute bounding volume for this mesh. 
  */
  BV
  computeBoundingVolume() const noexcept;

protected:
  /*!
    @brief Bounding volume hierarchy
  */
  std::shared_ptr<Node> m_bvh;
};

/*!
  @brief Signed distance function for a DCEL mesh. This class uses the compact BVH representation. 
*/
template <class T, class Meta, class BV, size_t K>
class FastCompactMeshSDF : public SignedDistanceFunction<T>
{
public:
  /*!
    @brief Alias for DCEL face type
  */
  using Face = typename EBGeometry::DCEL::FaceT<T, Meta>;

  /*!
    @brief Alias for DCEL mesh type
  */
  using Mesh = typename EBGeometry::DCEL::MeshT<T, Meta>;

  /*!
    @brief Alias for which BVH root node 
  */
  using Root = typename EBGeometry::BVH::LinearBVH<T, Face, BV, K>;

  /*!
    @brief Alias for linearized BVH
  */
  using Node = typename Root::LinearNode;

  /*!
    @brief Default disallowed constructor
  */
  FastCompactMeshSDF() = delete;

  /*!
    @brief Full constructor. Takes the input mesh and creates the BVH.
    @param[in] a_mesh Input mesh
    @param[in] a_build Build specification. Either top-down, Morton, or Nested.
  */
  FastCompactMeshSDF(const std::shared_ptr<Mesh>& a_mesh, const BVH::Build a_build = BVH::Build::TopDown) noexcept;

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

  /*!
    @brief Get the closest faces to the input point
    @details This returns a list of candidate faces that are close to the input point. The returned
    argment consists of the faces and the unsigned distance to the face.
    @param[in] a_point Input point
    @param[in] a_sorted Sort the output vector by distance or not. Closest go first. 
    @return List of candidate faces (potentially sorted)
  */
  virtual std::vector<std::pair<std::shared_ptr<const Face>, T>>
  getClosestFaces(const Vec3T<T>& a_point, const bool a_sorted) const noexcept;

  /*!
    @brief Get the bounding volume hierarchy enclosing the mesh
  */
  virtual std::shared_ptr<Root>&
  getRoot() noexcept;

  /*!
    @brief Get the bounding volume hierarchy enclosing the mesh
  */
  virtual const std::shared_ptr<Root>&
  getRoot() const noexcept;

  /*!
    @brief Compute bounding volume for this mesh. 
  */
  BV
  computeBoundingVolume() const noexcept;

protected:
  /*!
    @brief Bounding volume hierarchy
  */
  std::shared_ptr<Root> m_bvh;
};

/*!
  @brief Signed distance function for a triangle mesh. This class uses the full BVH representation. 
*/
template <class T, class Meta, class BV, size_t K>
class FastTriMeshSDF : public SignedDistanceFunction<T>
{
public:
  /*!
    @brief Alias for DCEL mesh type
  */
  using Mesh = EBGeometry::DCEL::MeshT<T, Meta>;

  /*!
    @brief Alias for DCEL face type
  */
  using Tri = typename EBGeometry::Triangle<T, Meta>;

  /*!
    @brief Alias for which BVH root node 
  */
  using Root = typename EBGeometry::BVH::LinearBVH<T, Tri, BV, K>;

  /*!
    @brief Alias for linearized BVH
  */
  using Node = typename Root::LinearNode;

  /*!
    @brief Default disallowed constructor
  */
  FastTriMeshSDF() = delete;

  /*!
    @brief Full constructor. Takes a DCEL mesh and creates the input triangles. Then creates the BVH.
    @param[in] a_mesh DCEL mesh
    @param[in] a_build Specification of build method. Must be TopDown, Morton, or Nested.
  */
  FastTriMeshSDF(const std::shared_ptr<Mesh>& a_mesh, const BVH::Build a_build = BVH::Build::TopDown) noexcept;

  /*!
    @brief Full constructor. Takes the input triangles and creates the BVH.
    @param[in] a_triangles Input triangle soup
    @param[in] a_build Specification of build method. Must be TopDown, Morton, or Nested.
  */
  FastTriMeshSDF(const std::vector<std::shared_ptr<Tri>>& a_triangles,
                 const BVH::Build                         a_build = BVH::Build::TopDown) noexcept;

  /*!
    @brief Destructor
  */
  virtual ~FastTriMeshSDF() = default;

  /*!
    @brief Value function
    @param[in] a_point Input point. 
  */
  virtual T
  signedDistance(const Vec3T<T>& a_point) const noexcept override;

  /*!
    @brief Get the closest triangles to the input point
    @details This returns a list of candidate triangles that are close to the input point. The returned
    argment consists of the triangles and the unsigned distance to the triangles.
    @param[in] a_point Input point
    @param[in] a_sorted Sort the output vector by distance or not. Closest go first. 
    @return List of candidate triangles (potentially sorted)
  */
  virtual std::vector<std::pair<std::shared_ptr<const Tri>, T>>
  getClosestTriangles(const Vec3T<T>& a_point, const bool a_sorted) const noexcept;

  /*!
    @brief Get the bounding volume hierarchy enclosing the mesh
  */
  virtual std::shared_ptr<Root>&
  getRoot() noexcept;

  /*!
    @brief Get the bounding volume hierarchy enclosing the mesh
  */
  virtual const std::shared_ptr<Root>&
  getRoot() const noexcept;

  /*!
    @brief Compute bounding volume for this mesh. 
  */
  BV
  computeBoundingVolume() const noexcept;

protected:
  /*!
    @brief Bounding volume hierarchy
  */
  std::shared_ptr<Root> m_bvh;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_MeshDistanceFunctionsImplem.hpp"

#endif
