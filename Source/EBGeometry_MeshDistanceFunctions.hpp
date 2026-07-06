// SPDX-FileCopyrightText: 2023 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file    EBGeometry_MeshDistanceFunctions.hpp
 * @brief   Declaration of signed distance functions for DCEL meshes
 * @author  Robert Marskar
 */

#ifndef EBGEOMETRY_MESHDISTANCEFUNCTIONS_HPP
#define EBGEOMETRY_MESHDISTANCEFUNCTIONS_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_BVH.hpp"
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_SignedDistanceFunction.hpp"
#include "EBGeometry_Triangle.hpp"
#include "EBGeometry_TriangleSoA.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

namespace DCEL {

/**
 * @brief Build a tree BVH from a DCEL mesh.
 * @tparam T    Floating-point precision type.
 * @tparam Meta Face and vertex metadata type.
 * @tparam BV   Bounding-volume type (e.g. AABBT<T>).
 * @tparam K    BVH branching factor (number of children per node).
 * @param[in] a_dcelMesh Input DCEL mesh.
 * @param[in] a_build    Build strategy (TopDown, Morton, Nested, or SAH). SAH is the default.
 * @return Shared pointer to the root of the resulting tree BVH.
 */
template <class T, class Meta, class BV, size_t K>
[[nodiscard]] std::shared_ptr<EBGeometry::BVH::TreeBVH<T, FaceT<T, Meta>, BV, K>>
buildTreeBVH(const std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>& a_dcelMesh,
             const BVH::Build                                         a_build = BVH::Build::SAH);
} // namespace DCEL

/**
 * @brief Signed distance function for a DCEL mesh. Does not use BVHs.
 * @details Iterates over every face of the mesh on every query — O(N) per call.
 * Suitable only for very small meshes or debugging.
 * @tparam T    Floating-point precision type (float or double).
 * @tparam Meta Triangle metadata type stored on each DCEL face.
 */
template <class T, class Meta = DCEL::DefaultMetaData>
class FlatMeshSDF : public SignedDistanceFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "FlatMeshSDF requires a floating-point T");

public:
  /**
   * @brief Alias for DCEL mesh type
   */
  using Mesh = EBGeometry::DCEL::MeshT<T, Meta>;

  /**
   * @brief Disallowed constructor
   */
  FlatMeshSDF() = delete;

  /**
   * @brief Full constructor.
   * @param[in] a_mesh Input mesh
   */
  FlatMeshSDF(const std::shared_ptr<Mesh>& a_mesh) noexcept;

  /**
   * @brief Destructor
   */
  ~FlatMeshSDF() override = default;

  /**
   * @brief Compute the signed distance from a_point to the mesh.
   * @param[in] a_point Query point.
   * @return Signed distance to the nearest face; negative inside the mesh.
   */
  [[nodiscard]] T
  signedDistance(const Vec3T<T>& a_point) const noexcept override;

  /**
   * @brief Get the underlying DCEL mesh.
   * @return Shared pointer to the mesh.
   */
  [[nodiscard]] const std::shared_ptr<Mesh>
  getMesh() const noexcept;

  /**
   * @brief Compute the axis-aligned bounding volume enclosing the mesh.
   * @tparam BV Bounding-volume type to construct (e.g. AABBT<T>).
   * @return Bounding volume that encloses all mesh vertices.
   */
  template <class BV>
  [[nodiscard]] BV
  computeBoundingVolume() const;

protected:
  /**
   * @brief DCEL mesh
   */
  std::shared_ptr<Mesh> m_mesh;
};

/**
 * @brief Signed distance function for a DCEL mesh. Stores the mesh in a PackedBVH for
 * SIMD-accelerated traversal. Accepts any polygon, not just triangles.
 * @details The mesh faces are packed into a flat-array PackedBVH. SIMD traversal
 * is used when T and K match an available ISA path.
 * @tparam T    Floating-point precision type (float or double).
 * @tparam Meta Triangle metadata type stored on each DCEL face.
 * @tparam K    BVH branching factor (number of children per internal node).
 */
template <class T, class Meta, size_t K>
class MeshSDF : public SignedDistanceFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "MeshSDF requires a floating-point T");
  static_assert(K >= 2, "MeshSDF requires branching factor K >= 2");

public:
  /**
   * @brief Alias for DCEL face type
   */
  using Face = typename EBGeometry::DCEL::FaceT<T, Meta>;

  /**
   * @brief Alias for DCEL mesh type
   */
  using Mesh = typename EBGeometry::DCEL::MeshT<T, Meta>;

  /**
   * @brief Alias for the linearized BVH root
   */
  using Root = EBGeometry::BVH::PackedBVH<T, Face, K>;

  /**
   * @brief Alias for a single linearized node
   */
  using Node = typename Root::Node;

  /**
   * @brief Default disallowed constructor
   */
  MeshSDF() = delete;

  /**
   * @brief Full constructor. Takes the input mesh and creates the BVH.
   * @param[in] a_mesh   Input mesh.
   * @param[in] a_build  BVH build strategy. SAH is the default and recommended choice.
   */
  MeshSDF(const std::shared_ptr<Mesh>& a_mesh, const BVH::Build a_build = BVH::Build::SAH);

  /**
   * @brief Destructor
   */
  ~MeshSDF() override = default;

  /**
   * @brief Compute the signed distance from a_point to the mesh.
   * @param[in] a_point Query point.
   * @return Signed distance to the nearest face; negative inside the mesh.
   */
  [[nodiscard]] T
  signedDistance(const Vec3T<T>& a_point) const noexcept override;

  /**
   * @brief Return faces within BVH-pruned candidate distance of a_point.
   * @details Traverses the PackedBVH and collects candidate faces.  The
   * result pairs each face with its unsigned distance to a_point.
   * @param[in] a_point  Query point.
   * @param[in] a_sorted If true, the returned vector is sorted by ascending
   * unsigned distance (closest face first).
   * @return Vector of (face, unsigned_distance) pairs, optionally sorted.
   */
  [[nodiscard]] virtual std::vector<std::pair<std::shared_ptr<const Face>, T>>
  getClosestFaces(const Vec3T<T>& a_point, const bool a_sorted) const;

  /**
   * @brief Get the PackedBVH enclosing the mesh.
   * @return Mutable reference to the shared-pointer owning the packed BVH root.
   */
  [[nodiscard]] virtual std::shared_ptr<Root>&
  getRoot() noexcept;

  /**
   * @brief Get the PackedBVH enclosing the mesh (const overload).
   * @return Const reference to the shared-pointer owning the packed BVH root.
   */
  [[nodiscard]] virtual const std::shared_ptr<Root>&
  getRoot() const noexcept;

  /**
   * @brief Compute the AABB enclosing the entire mesh.
   * @return Axis-aligned bounding box of the mesh.
   */
  [[nodiscard]] EBGeometry::BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept;

protected:
  /**
   * @brief Linearized BVH
   */
  std::shared_ptr<Root> m_bvh;
};

/**
 * @brief Signed distance function for a pure triangle mesh using SoA-grouped primitives
 * in a compact (linearized) BVH.
 * @details Triangles are packed into SoA groups of W triangles each
 * (TriangleSoAT<T,W>), enabling SIMD evaluation of up to W signed distances
 * simultaneously. W defaults to DefaultSoAWidth<T>(), which is 8 on AVX targets
 * (4 for double) and 4 otherwise.
 * @tparam T    Floating-point precision type (float or double).
 * @tparam Meta Triangle metadata type.
 * @tparam K    BVH branching factor (number of children per internal node).
 * Defaults to BVH::DefaultBranchingRatio<T>() — the SIMD-optimal value for T on the current ISA
 * (K=16/float or K=8/double on AVX-512F; K=8/float or K=4/double on AVX; K=4 otherwise).
 * @tparam W    SoA width: number of triangles per SIMD group.
 * Defaults to DefaultSoAWidth<T>() (8/float or 4/double on AVX; 4 otherwise).
 */
template <class T, class Meta, size_t K = BVH::DefaultBranchingRatio<T>(), size_t W = DefaultSoAWidth<T>()>
class TriMeshSDF : public SignedDistanceFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "TriMeshSDF<T,Meta,K,W> requires a floating-point T");
  static_assert(K >= 2, "TriMeshSDF requires branching factor K >= 2");
  static_assert(W > 0, "TriMeshSDF requires SoA width W > 0");

public:
  /**
   * @brief Alias for DCEL mesh type
   */
  using Mesh = EBGeometry::DCEL::MeshT<T, Meta>;

  /**
   * @brief Alias for DCEL face type
   */
  using Tri = typename EBGeometry::Triangle<T, Meta>;

  /**
   * @brief Alias for the SoA triangle group type
   */
  using TriSoA = TriangleSoAT<T, W>;

  /**
   * @brief Alias for which BVH root node
   */
  using Root = typename EBGeometry::BVH::PackedBVH<T, TriSoA, K>;

  /**
   * @brief Default disallowed constructor
   */
  TriMeshSDF() = delete;

  /**
   * @brief Full constructor. Takes a DCEL mesh and creates the input triangles. Then creates the BVH.
   * @param[in] a_mesh         DCEL mesh.
   * @param[in] a_build        BVH build strategy. SAH (binned Surface Area Heuristic) is the default
   * and recommended choice — it produces near-optimal traversal cost.
   * TopDown (centroid median) is faster to build but yields deeper trees.
   * @param[in] a_maxLeafSize  Maximum number of raw triangles per BVH leaf (this is a leaf-size
   * bound on the pre-packing tree, not on the number of TriangleSoA groups). Each leaf's
   * triangles become their own TriangleSoA group(s) during packing, with no batching across
   * leaves, so a leaf smaller than W wastes some of its group's SIMD lanes on padding. This is
   * an upper bound, not a target — the SAH/TopDown partitioner still splits down to tighter,
   * more selective leaves wherever the geometry warrants it. Defaults to 2*W, allowing up to two
   * SoA groups per leaf so tree quality isn't forced to sacrifice everything for exact
   * one-group-per-leaf packing.
   */
  TriMeshSDF(const std::shared_ptr<Mesh>& a_mesh,
             const BVH::Build             a_build       = BVH::Build::SAH,
             const size_t                 a_maxLeafSize = 2 * W) noexcept;

  /**
   * @brief Full constructor. Takes the input triangles and creates the BVH.
   * @param[in] a_triangles   Input triangle soup.
   * @param[in] a_build       BVH build strategy (see the mesh-based constructor for details).
   * @param[in] a_maxLeafSize Maximum number of raw triangles per BVH leaf (see the mesh-based
   * constructor for the tree-quality/SIMD-occupancy trade-off). Defaults to 2*W.
   */
  TriMeshSDF(const std::vector<std::shared_ptr<Tri>>& a_triangles,
             const BVH::Build                         a_build       = BVH::Build::SAH,
             const size_t                             a_maxLeafSize = 2 * W) noexcept;

  /**
   * @brief Destructor
   */
  ~TriMeshSDF() override = default;

  /**
   * @brief Compute the signed distance from a_point to the triangle mesh.
   * @param[in] a_point Query point.
   * @return Signed distance to the nearest triangle; negative inside the mesh.
   */
  [[nodiscard]] T
  signedDistance(const Vec3T<T>& a_point) const noexcept override;

  /**
   * @brief Get the PackedBVH storing SoA triangle groups.
   * @return Mutable reference to the shared-pointer owning the packed BVH root.
   */
  [[nodiscard]] virtual std::shared_ptr<Root>&
  getRoot() noexcept;

  /**
   * @brief Get the PackedBVH storing SoA triangle groups (const overload).
   * @return Const reference to the shared-pointer owning the packed BVH root.
   */
  [[nodiscard]] virtual const std::shared_ptr<Root>&
  getRoot() const noexcept;

  /**
   * @brief Compute the AABB enclosing the entire triangle mesh.
   * @return Axis-aligned bounding box of the mesh.
   */
  [[nodiscard]] EBGeometry::BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept;

protected:
  /**
   * @brief Bounding volume hierarchy storing SoA triangle groups.
   */
  std::shared_ptr<Root> m_bvh;

  /**
   * @brief Leaf-conversion callback for TreeBVH::packWith: groups a BVH leaf's triangles
   * into SoA blocks of width W.
   * @details Shared by both constructors; stateless (captures nothing), so it is a static
   * member rather than a per-constructor lambda.
   * @param[in] a_triangles Leaf's triangle list.
   * @param[in] a_offset    Index of the first triangle in this leaf to convert.
   * @param[in] a_count     Number of triangles in this leaf to convert.
   * @return SoA-packed triangle groups covering [a_offset, a_offset + a_count).
   */
  [[nodiscard]] static std::vector<TriSoA>
  groupTrianglesIntoSoA(const std::vector<std::shared_ptr<const Tri>>& a_triangles,
                        uint32_t                                       a_offset,
                        uint32_t                                       a_count);
};

} // namespace EBGeometry

#include "EBGeometry_MeshDistanceFunctionsImplem.hpp"

#endif
