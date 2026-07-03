// SPDX-FileCopyrightText: 2023 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  @file    EBGeometry_MeshDistanceFunctions.hpp
  @brief   Declaration of signed distance functions for DCEL meshes
  @author  Robert Marskar
*/

#ifndef EBGEOMETRY_MESHDISTANCEFUNCTIONS_HPP
#define EBGEOMETRY_MESHDISTANCEFUNCTIONS_HPP

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_SignedDistanceFunction.hpp"
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_BVH.hpp"
#include "EBGeometry_Triangle.hpp"
#include "EBGeometry_TriangleSoA.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

namespace DCEL {

  /**
    @brief One-liner for turning a DCEL mesh into a full-tree BVH. 
    @param[in] a_dcelMesh Input DCEL mesh.
    @param[in] a_build Build specification for BVH.
    @return Returns a pointer to a full-tree BVH representation of the DCEL faces.
  */
  /**
    @brief Build a full (tree) BVH from a DCEL mesh.
    @tparam T    Floating-point precision type.
    @tparam Meta Triangle metadata type.
    @tparam BV   Bounding-volume type (e.g. AABBT<T>).
    @tparam K    BVH branching factor (number of children per node).
    @param[in] a_dcelMesh Input DCEL mesh.
    @param[in] a_build    Build strategy (TopDown, Morton, or Nested).
    @return Shared pointer to the root of the resulting tree BVH.
  */
  template <class T, class Meta, class BV, size_t K>
  std::shared_ptr<EBGeometry::BVH::TreeBVH<T, FaceT<T, Meta>, BV, K>>
  buildFullBVH(const std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>& a_dcelMesh,
               const BVH::Build                                         a_build = BVH::Build::TopDown) noexcept;
} // namespace DCEL

/**
  @brief Signed distance function for a DCEL mesh. Does not use BVHs.
  @details Iterates over every face of the mesh on every query — O(N) per call.
  Suitable only for very small meshes or debugging.
  @tparam T    Floating-point precision type (float or double).
  @tparam Meta Triangle metadata type stored on each DCEL face.
*/
template <class T, class Meta = DCEL::DefaultMetaData>
class MeshSDF : public SignedDistanceFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "MeshSDF requires a floating-point T");

public:
  /**
    @brief Alias for DCEL mesh type
  */
  using Mesh = EBGeometry::DCEL::MeshT<T, Meta>;

  /**
    @brief Disallowed constructor
  */
  MeshSDF() = delete;

  /**
    @brief Full constructor.
    @param[in] a_mesh Input mesh
  */
  MeshSDF(const std::shared_ptr<Mesh>& a_mesh) noexcept;

  /**
    @brief Destructor
  */
  virtual ~MeshSDF() = default;

  /**
    @brief Compute the signed distance from a_point to the mesh.
    @param[in] a_point Query point.
    @return Signed distance to the nearest face; negative inside the mesh.
  */
  virtual T
  signedDistance(const Vec3T<T>& a_point) const noexcept override;

  /**
    @brief Get the underlying DCEL mesh.
    @return Shared pointer to the mesh.
  */
  const std::shared_ptr<Mesh>
  getMesh() const noexcept;

  /**
    @brief Compute the axis-aligned bounding volume enclosing the mesh.
    @tparam BV Bounding-volume type to construct (e.g. AABBT<T>).
    @return Bounding volume that encloses all mesh vertices.
  */
  template <class BV>
  BV
  computeBoundingVolume() const noexcept;

protected:
  /**
    @brief DCEL mesh
  */
  std::shared_ptr<Mesh> m_mesh;
};

/**
  @brief Signed distance function for a DCEL mesh. This class uses the full BVH representation.
  @tparam T    Floating-point precision type (float or double).
  @tparam Meta Triangle metadata type stored on each DCEL face.
  @tparam BV   Bounding-volume type used by the tree (e.g. AABBT<T>).
  @tparam K    BVH branching factor (number of children per internal node).
*/
template <class T, class Meta, class BV, size_t K>
class FastMeshSDF : public SignedDistanceFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "FastMeshSDF requires a floating-point T");
  static_assert(K >= 2, "FastMeshSDF requires branching factor K >= 2");

public:
  /**
    @brief Alias for DCEL face type
  */
  using Face = typename EBGeometry::DCEL::FaceT<T, Meta>;

  /**
    @brief Alias for DCEL mesh type
  */
  using Mesh = typename EBGeometry::DCEL::MeshT<T, Meta>;

  /**
    @brief Alias for BVH root node
  */
  using Node = EBGeometry::BVH::TreeBVH<T, Face, BV, K>;

  /**
    @brief Default disallowed constructor
  */
  FastMeshSDF() = delete;

  /**
    @brief Full constructor. Takes the input mesh and creates the BVH.
    @param[in] a_mesh Input mesh
    @param[in] a_build Specification of build method. Must be TopDown, Morton, or Nested.
  */
  FastMeshSDF(const std::shared_ptr<Mesh>& a_mesh, const BVH::Build a_build = BVH::Build::TopDown) noexcept;

  /**
    @brief Destructor
  */
  virtual ~FastMeshSDF() = default;

  /**
    @brief Compute the signed distance from a_point to the mesh.
    @param[in] a_point Query point.
    @return Signed distance to the nearest face; negative inside the mesh.
  */
  virtual T
  signedDistance(const Vec3T<T>& a_point) const noexcept override;

  /**
    @brief Return faces within BVH-pruned candidate distance of a_point.
    @details Traverses the tree BVH and collects all faces whose leaf
    bounding volume is within the current best unsigned distance.  The
    result pairs each candidate face with its unsigned distance to a_point.
    @param[in] a_point  Query point.
    @param[in] a_sorted If true, the returned vector is sorted by ascending
                        unsigned distance (closest face first).
    @return Vector of (face, unsigned_distance) pairs, optionally sorted.
  */
  virtual std::vector<std::pair<std::shared_ptr<const Face>, T>>
  getClosestFaces(const Vec3T<T>& a_point, const bool a_sorted) const noexcept;

  /**
    @brief Get the bounding volume hierarchy enclosing the mesh.
    @return Mutable reference to the shared-pointer owning the BVH root.
  */
  virtual std::shared_ptr<Node>&
  getBVH() noexcept;

  /**
    @brief Get the bounding volume hierarchy enclosing the mesh (const overload).
    @return Const reference to the shared-pointer owning the BVH root.
  */
  virtual const std::shared_ptr<Node>&
  getBVH() const noexcept;

  /**
    @brief Compute the bounding volume of the root node.
    @return Bounding volume enclosing the entire mesh.
  */
  BV
  computeBoundingVolume() const noexcept;

protected:
  /**
    @brief Bounding volume hierarchy
  */
  std::shared_ptr<Node> m_bvh;
};

/**
  @brief Signed distance function for a DCEL mesh backed by a compact (linearized) BVH.
  @details The mesh is stored in a flat-array PackedBVH for better cache behaviour
  than the tree BVH. SIMD traversal is used when T and K match an available ISA path.
  @tparam T    Floating-point precision type (float or double).
  @tparam Meta Triangle metadata type stored on each DCEL face.
  @tparam K    BVH branching factor (number of children per internal node).
*/
template <class T, class Meta, size_t K>
class FastCompactMeshSDF : public SignedDistanceFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "FastCompactMeshSDF requires a floating-point T");
  static_assert(K >= 2, "FastCompactMeshSDF requires branching factor K >= 2");

public:
  /**
    @brief Alias for DCEL face type
  */
  using Face = typename EBGeometry::DCEL::FaceT<T, Meta>;

  /**
    @brief Alias for DCEL mesh type
  */
  using Mesh = typename EBGeometry::DCEL::MeshT<T, Meta>;

  /**
    @brief Alias for the linearized BVH root
  */
  using Root = EBGeometry::BVH::PackedBVH<T, Face, K>;

  /**
    @brief Alias for a single linearized node
  */
  using Node = typename Root::Node;

  /**
    @brief Default disallowed constructor
  */
  FastCompactMeshSDF() = delete;

  /**
    @brief Full constructor. Takes the input mesh and creates the BVH.
    @param[in] a_mesh Input mesh
    @param[in] a_build Build specification. Either top-down, Morton, or Nested.
  */
  FastCompactMeshSDF(const std::shared_ptr<Mesh>& a_mesh, const BVH::Build a_build = BVH::Build::TopDown) noexcept;

  /**
    @brief Destructor
  */
  virtual ~FastCompactMeshSDF() = default;

  /**
    @brief Compute the signed distance from a_point to the mesh.
    @param[in] a_point Query point.
    @return Signed distance to the nearest face; negative inside the mesh.
  */
  virtual T
  signedDistance(const Vec3T<T>& a_point) const noexcept override;

  /**
    @brief Return faces within BVH-pruned candidate distance of a_point.
    @details Traverses the compact BVH and collects candidate faces.  The
    result pairs each face with its unsigned distance to a_point.
    @param[in] a_point  Query point.
    @param[in] a_sorted If true, the returned vector is sorted by ascending
                        unsigned distance (closest face first).
    @return Vector of (face, unsigned_distance) pairs, optionally sorted.
  */
  virtual std::vector<std::pair<std::shared_ptr<const Face>, T>>
  getClosestFaces(const Vec3T<T>& a_point, const bool a_sorted) const noexcept;

  /**
    @brief Get the compact BVH enclosing the mesh.
    @return Mutable reference to the shared-pointer owning the packed BVH root.
  */
  virtual std::shared_ptr<Root>&
  getRoot() noexcept;

  /**
    @brief Get the compact BVH enclosing the mesh (const overload).
    @return Const reference to the shared-pointer owning the packed BVH root.
  */
  virtual const std::shared_ptr<Root>&
  getRoot() const noexcept;

  /**
    @brief Compute the AABB enclosing the entire mesh.
    @return Axis-aligned bounding box of the mesh.
  */
  EBGeometry::BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept;

protected:
  /**
    @brief Linearized BVH
  */
  std::shared_ptr<Root> m_bvh;
};

/**
  @brief Signed distance function for a pure triangle mesh using SoA-grouped primitives
         in a compact (linearized) BVH.
  @details Triangles are packed into SoA groups of W triangles each
  (TriangleSoAT<T,W>), enabling SIMD evaluation of up to W signed distances
  simultaneously.  W defaults to EBGEOMETRY_SOA_DEFAULT_WIDTH, which is
  set to 8 on AVX targets and 4 on SSE4.1-only targets.
  @tparam T    Floating-point precision type (float or double).
  @tparam Meta Triangle metadata type.
  @tparam K    BVH branching factor (number of children per internal node).
  @tparam W    SoA width: number of triangles per SIMD group.
               Defaults to EBGEOMETRY_SOA_DEFAULT_WIDTH (ISA-optimal).
*/
template <class T, class Meta, size_t K, size_t W = EBGEOMETRY_SOA_DEFAULT_WIDTH>
class FastTriMeshSDF : public SignedDistanceFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "FastTriMeshSDF requires a floating-point T");
  static_assert(K >= 2, "FastTriMeshSDF requires branching factor K >= 2");
  static_assert(W > 0, "FastTriMeshSDF requires SoA width W > 0");

public:
  /**
    @brief Alias for DCEL mesh type
  */
  using Mesh = EBGeometry::DCEL::MeshT<T, Meta>;

  /**
    @brief Alias for DCEL face type
  */
  using Tri = typename EBGeometry::Triangle<T, Meta>;

  /**
    @brief Alias for the SoA triangle group type
  */
  using TriSoA = TriangleSoAT<T, W>;

  /**
    @brief Alias for which BVH root node
  */
  using Root = typename EBGeometry::BVH::PackedBVH<T, TriSoA, K>;

  /**
    @brief Alias for linearized BVH
  */
  using Node = typename Root::Node;

  /**
    @brief Default disallowed constructor
  */
  FastTriMeshSDF() = delete;

  /**
    @brief Full constructor. Takes a DCEL mesh and creates the input triangles. Then creates the BVH.
    @param[in] a_mesh DCEL mesh
    @param[in] a_build Specification of build method. Must be TopDown, Morton, or Nested.
    @param[in] a_maxLeafSize Maximum number of primitives per BVH leaf. Larger values yield coarser
    BVH culling but better SIMD throughput at leaf evaluation. Default is 8.
  */
  FastTriMeshSDF(const std::shared_ptr<Mesh>& a_mesh,
                 const BVH::Build             a_build       = BVH::Build::TopDown,
                 const size_t                 a_maxLeafSize = 8U) noexcept;

  /**
    @brief Full constructor. Takes the input triangles and creates the BVH.
    @param[in] a_triangles Input triangle soup
    @param[in] a_build Specification of build method. Must be TopDown, Morton, or Nested.
    @param[in] a_maxLeafSize Maximum number of primitives per BVH leaf.
  */
  FastTriMeshSDF(const std::vector<std::shared_ptr<Tri>>& a_triangles,
                 const BVH::Build                         a_build       = BVH::Build::TopDown,
                 const size_t                             a_maxLeafSize = 8U) noexcept;

  /**
    @brief Destructor
  */
  virtual ~FastTriMeshSDF() = default;

  /**
    @brief Compute the signed distance from a_point to the triangle mesh.
    @param[in] a_point Query point.
    @return Signed distance to the nearest triangle; negative inside the mesh.
  */
  virtual T
  signedDistance(const Vec3T<T>& a_point) const noexcept override;

  /**
    @brief Get the compact BVH storing SoA triangle groups.
    @return Mutable reference to the shared-pointer owning the packed BVH root.
  */
  virtual std::shared_ptr<Root>&
  getRoot() noexcept;

  /**
    @brief Get the compact BVH storing SoA triangle groups (const overload).
    @return Const reference to the shared-pointer owning the packed BVH root.
  */
  virtual const std::shared_ptr<Root>&
  getRoot() const noexcept;

  /**
    @brief Compute the AABB enclosing the entire triangle mesh.
    @return Axis-aligned bounding box of the mesh.
  */
  EBGeometry::BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept;

protected:
  /**
    @brief Bounding volume hierarchy storing SoA triangle groups.
  */
  std::shared_ptr<Root> m_bvh;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_MeshDistanceFunctionsImplem.hpp"

#endif
