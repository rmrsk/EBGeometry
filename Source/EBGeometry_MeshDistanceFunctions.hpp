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
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_BVH.hpp"
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_SignedDistanceFunction.hpp"
#include "EBGeometry_Triangle.hpp"
#include "EBGeometry_TriangleAoSoA.hpp"
#include "EBGeometry_TriangleSoA.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Signed distance function for a DCEL mesh. Does not use BVHs.
 * @details Iterates over every face of the mesh on every query -- O(N) per call. Suitable only for
 * very small meshes, debugging, and as a brute-force reference for the BVH-accelerated mesh SDFs.
 *
 * A plain value type: it holds the mesh descriptor by value (offsets into a Pool plus the pool's
 * control block or base address) and nothing else, so it is trivially copyable and every query is
 * callable on the host and on a device. It does not derive from SignedDistanceFunction -- a class
 * with virtual functions can never be passed to a kernel.
 *
 * Copying a FlatMeshSDF copies descriptors only: every copy resolves against the same pool memory,
 * and the pool must outlive all of them. A FlatMeshSDF sees the mesh as it was when it was
 * constructed; build a new one after changing the mesh's size. To evaluate on a device, freeze and
 * mirror the pool, then pass rebasedView() into a kernel:
 *
 * @code
 * hostPool.freeze();
 * Pool       devicePool = Pool::mirror(hostPool, deviceMemoryResource());
 * const auto deviceSDF  = flatSDF.rebasedView(devicePool);
 * myKernel<<<blocks, threads>>>(deviceSDF, ...);
 * @endcode
 * @tparam T    Floating-point precision type (float or double).
 * @tparam Meta Triangle metadata type stored on each DCEL face.
 */
template <class T, class Meta = DCEL::DefaultMetaData>
class FlatMeshSDF
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
   * @details Copies the mesh descriptor. Nothing is frozen or bound: the mesh resolves its storage
   * through a_pool's control block on every access, so this object is queryable at once and stays
   * queryable across a Pool::reserve that grows and moves the block. a_pool is taken to assert that
   * a_mesh really was reserved from it, and to make visible at the call site that it must outlive
   * this object and every copy of it.
   * @param[in]     a_mesh Input mesh, built against a_pool.
   * @param[in,out] a_pool Pool a_mesh's storage was reserved from. Must outlive this object.
   */
  EBGEOMETRY_HOST
  inline FlatMeshSDF(const Mesh& a_mesh, Pool& a_pool) noexcept;

  /**
   * @brief Compute the signed distance from a_point to the mesh.
   * @param[in] a_point Query point.
   * @return Signed distance to the nearest face; negative inside the mesh.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  signedDistance(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Get the underlying DCEL mesh descriptor.
   * @return The mesh.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const Mesh&
  getMesh() const noexcept
  {
    return m_mesh;
  }

  /**
   * @brief Produce a copy of this object that resolves against @p a_pool.
   * @details Rebases the mesh descriptor; see DCEL::MeshT::rebasedView() for the contract. This is
   * the one sanctioned crossing to a device.
   * @param[in] a_pool Pool to rebase onto; must be a mirror of the mesh's own pool.
   * @return A FlatMeshSDF resolving against @p a_pool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline FlatMeshSDF
  rebasedView(const Pool& a_pool) const noexcept;

  /**
   * @brief Duplicate the mesh's storage into @p a_dstPool.
   * @details The copy constructor copies descriptors only, leaving both objects resolving against
   * the same pool memory. This is the operation that gives genuinely independent storage.
   * @param[in,out] a_dstPool Pool to reserve the copy's mesh from; may be this object's own pool.
   * @return A FlatMeshSDF over an independent copy of the mesh, attached to @p a_dstPool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline FlatMeshSDF
  deepCopy(Pool& a_dstPool) const;

  /**
   * @brief Check whether the mesh was reserved from @p a_pool.
   * @param[in] a_pool Pool to test against.
   * @return True if the mesh is attached to @p a_pool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline bool
  isAttachedTo(const Pool& a_pool) const noexcept
  {
    return m_mesh.isAttachedTo(a_pool);
  }

  /**
   * @brief Compute the axis-aligned bounding volume enclosing the mesh.
   * @tparam BV Bounding-volume type to construct (e.g. AABBT<T>).
   * @return Bounding volume that encloses all mesh vertices.
   */
  template <class BV>
  [[nodiscard]] EBGEOMETRY_HOST
  inline BV
  computeBoundingVolume() const;

private:
  /**
   * @brief The mesh descriptor, held by value.
   */
  Mesh m_mesh;
};

/**
 * @brief A FlatMeshSDF must be trivially copyable: that is what lets a rebasedView() be byte-copied
 * into a device address space with no pointer patching.
 */
static_assert(std::is_trivially_copyable_v<FlatMeshSDF<float>>, "FlatMeshSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<FlatMeshSDF<double>>, "FlatMeshSDF<double> must be trivially copyable");

/**
 * @brief Signed distance function for a DCEL mesh. Stores the mesh in a PackedBVH for
 * SIMD-accelerated traversal. Accepts any polygon, not just triangles.
 * @details The mesh faces are packed into a flat-array PackedBVH. SIMD traversal
 * is used when T and K match an available ISA path. Each packed face is a fresh copy of the
 * corresponding DCEL mesh face, stored inline (DCEL::FaceT is a plain, trivially-copyable value, so
 * copying it is cheap and free of aliasing). That copy is only meaningful together with the mesh it
 * was built from, though: a FaceT stores its half-edge as an index into its owning mesh's edge
 * array, not a self-resolving reference, so MeshSDF retains the source mesh (m_mesh) and passes it
 * to every DCEL::FaceT query that needs to resolve topology (point-in-face tests, signed
 * distance).
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
   * @details No default arguments: this is a low-level constructor, and callers working at this
   * level must consciously choose a build strategy. Use Parser::readIntoPackedBVH for sensible
   * defaults. Nothing is frozen or bound; see FlatMeshSDF's constructor. a_pool must outlive this
   * object for the same reason given there, and additionally because the BVH built below holds DCEL
   * faces whose indices are meaningful only against that same storage.
   * @param[in]     a_mesh   Input mesh, built against a_pool.
   * @param[in,out] a_pool   Pool a_mesh's storage was reserved from. Must outlive this object.
   * @param[in]     a_build  BVH build strategy. SAH (binned Surface Area Heuristic) is recommended.
   */
  MeshSDF(const std::shared_ptr<Mesh>& a_mesh, Pool& a_pool, const BVH::Build a_build);

  /**
   * @brief Destructor
   */
  ~MeshSDF() override = default;

  /**
   * @brief Copy constructor.
   * @details Explicitly defaulted for documentation purposes: MeshSDF's members (m_bvh, m_mesh)
   * are both shared_ptr, so the implicitly-generated copy is a cheap, correct handle-copy.
   * @param[in] a_other Other instance to copy.
   */
  MeshSDF(const MeshSDF& a_other) = default;

  /**
   * @brief Copy assignment operator.
   * @param[in] a_other Other instance to copy.
   * @return Reference to *this.
   */
  MeshSDF&
  operator=(const MeshSDF& a_other) = default;

  /**
   * @brief Move constructor.
   * @details Explicitly defaulted: the user-declared destructor above would otherwise suppress
   * the implicitly-generated move constructor.
   * @param[in,out] a_other Other instance to move from.
   */
  MeshSDF(MeshSDF&& a_other) noexcept = default;

  /**
   * @brief Move assignment operator.
   * @param[in,out] a_other Other instance to move from.
   * @return Reference to *this.
   */
  MeshSDF&
  operator=(MeshSDF&& a_other) noexcept = default;

  /**
   * @brief Compute the signed distance from a_point to the mesh.
   * @param[in] a_point Query point.
   * @return Signed distance to the nearest face; negative inside the mesh.
   */
  [[nodiscard]] T
  signedDistance(const Vec3T<T>& a_point) const noexcept override;

  /**
   * @brief Return faces within BVH-pruned candidate distance of a_point.
   * @details Traverses the PackedBVH and collects candidate faces, pairing each with its unsigned
   * distance to @p a_point.
   *
   * Faces are named by their index into this object's own BVH primitive array -- the array
   * getRoot()->getPrimitives() returns -- and *not* by an index into the source mesh's face array.
   * Packing reorders primitives into leaf order and stores them by value, so nothing records which
   * mesh face a packed face came from. Resolve an index with getRoot()->getPrimitives()[index].
   * @param[in] a_point  Query point.
   * @param[in] a_sorted If true, the returned vector is sorted by ascending
   * unsigned distance (closest face first).
   * @return Vector of (BVH primitive index, unsigned_distance) pairs, optionally sorted.
   */
  [[nodiscard]] virtual std::vector<std::pair<uint32_t, T>>
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

  /**
   * @brief Source DCEL mesh.
   * @details Retained so that the faces held by m_bvh -- whose half-edges (and the edges/vertices
   * reachable from them) are only weakly referenced, not owned, see DCEL::FaceT::m_halfEdge -- stay
   * valid. The mesh's own vertex/edge/face vectors are the sole owners of that topology.
   */
  std::shared_ptr<Mesh> m_mesh;
};

/**
 * @brief Signed distance function for a pure triangle mesh using SoA-grouped primitives
 * in a compact (linearized) BVH.
 * @details Triangles are packed into metadata-carrying SoA groups of W triangles each
 * (TriangleAoSoA<T,Meta,W>), enabling SIMD evaluation of up to W signed distances simultaneously.
 *
 * No default arguments: this is a low-level constructor, and callers who excavate down to it
 * must consciously choose K and W. Use Parser::readIntoTriangleBVH for sensible ISA-tuned
 * defaults (BVH::DefaultBranchingRatio<T>() for K, TriangleSoA::DefaultWidth<T>() for W).
 * @tparam T    Floating-point precision type (float or double).
 * @tparam Meta Triangle metadata type.
 * @tparam K    BVH branching factor (number of children per internal node). Must be >= 2.
 * @tparam W    SoA width: number of triangles per SIMD group. Must be > 0.
 * Each leaf primitive is a TriangleAoSoA<T, Meta, W>: an SoA triangle block for SIMD signed-distance
 * evaluation, plus a physically-separate per-lane metadata array. The hot signedDistance() path
 * never reads the metadata; getClosestTriangle() does, returning the closest triangle's signed
 * distance together with its Meta (see issue #105).
 * The SoA groups are built by groupTrianglesIntoSoA() while packing and owned by nothing else, so
 * the packed BVH stores them inline, by value. Instancing the same mesh multiple times (e.g. via
 * Translate/Rotate/Scale or a CSG union) does not duplicate them: those wrappers hold a shared_ptr
 * to the whole TriMeshSDF, so its packed data exists once no matter how many placements refer to
 * it. Making the group array pool-resident in its own right belongs with the de-virtualisation of
 * the SDF wrappers (PORTING.md step 4).
 */
template <class T, class Meta, size_t K, size_t W>
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
   * @brief Alias for the metadata-carrying SoA triangle group type (the BVH leaf primitive).
   */
  using TriAoSoA = TriangleAoSoA<T, Meta, W>;

  /**
   * @brief Alias for which BVH root node
   */
  using Root = typename EBGeometry::BVH::PackedBVH<T, TriAoSoA, K>;

  /**
   * @brief Result of getClosestTriangle(): the signed distance to the closest triangle and that
   * triangle's metadata.
   * @details Recovers per-triangle metadata for the single nearest triangle through the SIMD SoA
   * path. @c signedDistance equals what signedDistance() returns for the same point.
   */
  struct ClosestTriangle
  {
    T    signedDistance = std::numeric_limits<T>::max(); ///< Signed distance to the closest triangle.
    Meta metaData{};                                     ///< Metadata of the closest triangle.
  };

  /**
   * @brief Default disallowed constructor
   */
  TriMeshSDF() = delete;

  /**
   * @brief Full constructor. Takes a DCEL mesh and creates the input triangles. Then creates the BVH.
   * @details No default arguments: this is a low-level constructor, and callers who excavate down
   * to it must consciously choose every parameter. Use Parser::readIntoTriangleBVH for sensible
   * defaults.
   * @param[in]     a_mesh          DCEL mesh built against a_pool.
   * @param[in,out] a_pool          Pool a_mesh's storage was reserved from. Unlike FlatMeshSDF/
   * MeshSDF, this constructor does not retain a_mesh -- it extracts flat Triangle values from it and
   * discards it -- so nothing here depends on a_pool outliving the returned object.
   * @param[in]     a_build         BVH build strategy. SAH (binned Surface Area Heuristic) produces
   * near-optimal traversal cost; TopDown (centroid median) is faster to build but yields deeper trees.
   * @param[in]     a_maxLeafGroups Maximum number of full W-sized TriangleSoA groups per BVH leaf; the
   * actual raw-triangle leaf-size bound used is a_maxLeafGroups * W. This bounds the pre-packing
   * tree's leaf size, not the packed representation directly: each leaf's triangles become their
   * own TriangleSoA group(s) during packing, with no batching across leaves, so a leaf smaller
   * than W wastes some of its group's SIMD lanes on padding. It is an upper bound, not a target —
   * the SAH/TopDown partitioner still splits down to tighter, more selective leaves wherever the
   * geometry warrants it. Expressing this as a count of W-sized groups (rather than a raw triangle
   * count) makes it impossible to accidentally pick a leaf size that isn't a multiple of W. Must
   * be > 0.
   */
  TriMeshSDF(const std::shared_ptr<Mesh>& a_mesh,
             Pool&                        a_pool,
             const BVH::Build             a_build,
             const size_t                 a_maxLeafGroups) noexcept;

  /**
   * @brief Full constructor. Takes the input triangles and creates the BVH.
   * @param[in] a_triangles     Input triangle soup.
   * @param[in,out] a_pool      Pool the packed BVH's arrays are reserved from; must outlive this object.
   * @param[in] a_build         BVH build strategy (see the mesh-based constructor for details).
   * @param[in] a_maxLeafGroups Maximum number of full W-sized TriangleSoA groups per BVH leaf (see
   * the mesh-based constructor for the tree-quality/SIMD-occupancy trade-off). Must be > 0.
   */
  TriMeshSDF(const std::vector<std::shared_ptr<Tri>>& a_triangles,
             Pool&                                    a_pool,
             const BVH::Build                         a_build,
             const size_t                             a_maxLeafGroups) noexcept;

  /**
   * @brief Destructor
   */
  ~TriMeshSDF() override = default;

  /**
   * @brief Copy constructor.
   * @details Explicitly defaulted for documentation purposes: TriMeshSDF's only member (m_bvh)
   * is a shared_ptr, so the implicitly-generated copy is a cheap, correct handle-copy.
   * @param[in] a_other Other instance to copy.
   */
  TriMeshSDF(const TriMeshSDF& a_other) = default;

  /**
   * @brief Copy assignment operator.
   * @param[in] a_other Other instance to copy.
   * @return Reference to *this.
   */
  TriMeshSDF&
  operator=(const TriMeshSDF& a_other) = default;

  /**
   * @brief Move constructor.
   * @details Explicitly defaulted: the user-declared destructor above would otherwise suppress
   * the implicitly-generated move constructor.
   * @param[in,out] a_other Other instance to move from.
   */
  TriMeshSDF(TriMeshSDF&& a_other) noexcept = default;

  /**
   * @brief Move assignment operator.
   * @param[in,out] a_other Other instance to move from.
   * @return Reference to *this.
   */
  TriMeshSDF&
  operator=(TriMeshSDF&& a_other) noexcept = default;

  /**
   * @brief Compute the signed distance from a_point to the triangle mesh.
   * @param[in] a_point Query point.
   * @return Signed distance to the nearest triangle; negative inside the mesh.
   */
  [[nodiscard]] T
  signedDistance(const Vec3T<T>& a_point) const noexcept override;

  /**
   * @brief Signed distance to the closest triangle, together with that triangle's metadata.
   * @details The metadata-retrieving companion to signedDistance(): it drives the same SIMD-pruned
   * BVH traversal (PackedBVH::pruneTraverse), but each visited leaf group reports the winning
   * triangle's metadata via TriangleAoSoA::signedDistance(point, Meta&), so the result carries both
   * the signed distance and the Meta of the nearest triangle -- the supported path for callers who
   * need both maximum SIMD throughput and per-triangle metadata retrieval (see issue #105). The
   * result's @c signedDistance equals what signedDistance() would return for the same point.
   * @param[in] a_point Query point. Must be finite.
   * @return The closest triangle's signed distance and metadata.
   */
  [[nodiscard]] ClosestTriangle
  getClosestTriangle(const Vec3T<T>& a_point) const noexcept;

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
  [[nodiscard]] static std::vector<TriAoSoA>
  groupTrianglesIntoSoA(const std::vector<std::shared_ptr<const Tri>>& a_triangles,
                        uint32_t                                       a_offset,
                        uint32_t                                       a_count);
};

} // namespace EBGeometry

#include "EBGeometry_MeshDistanceFunctionsImplem.hpp"

#endif
