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
   * @brief Compute the AABB enclosing the entire mesh.
   * @details A componentwise min/max over the mesh vertices, so it is callable on host and device
   * alike. An empty mesh yields an inverted box (low corner +max, high corner -max).
   * @return Axis-aligned bounding box of the mesh.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline EBGeometry::BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept;

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
 * @brief Signed distance function for a DCEL mesh, accelerated by a PackedBVH over its faces.
 * Accepts any polygon, not just triangles.
 * @details The mesh faces are packed into a flat-array PackedBVH; SIMD node pruning is used when T
 * and K match an available ISA path. Each packed face is a copy of the corresponding DCEL face,
 * whose half-edge is an index into the mesh's own edge array, so MeshSDF holds the mesh as well and
 * passes it to every face query that resolves topology.
 *
 * A plain value type, like FlatMeshSDF: it holds the mesh descriptor and the BVH by value, both
 * resolving against the one Pool passed to the constructor, so it is trivially copyable and
 * signedDistance() is callable on the host and on a device. It does not derive from
 * SignedDistanceFunction -- a class with virtual functions can never be passed to a kernel. Copies
 * share the pool memory, and the pool must outlive all of them. To evaluate on a device, freeze and
 * mirror the pool and pass rebasedView() into a kernel.
 * @tparam T    Floating-point precision type (float or double).
 * @tparam Meta Triangle metadata type stored on each DCEL face.
 * @tparam K    BVH branching factor (number of children per internal node).
 */
template <class T, class Meta, size_t K>
class MeshSDF
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
   * @brief Full constructor. Copies the mesh descriptor and builds the BVH over its faces.
   * @details No default arguments: this is a low-level constructor, and callers working at this
   * level must consciously choose a build strategy. Use Parser::readIntoPackedBVH for sensible
   * defaults. The BVH is reserved from a_pool, which must be the pool a_mesh was built in, so that
   * one rebasedView() rebases both. a_pool must outlive this object and every copy of it.
   * @param[in]     a_mesh   Input mesh, built against a_pool.
   * @param[in,out] a_pool   Pool a_mesh's storage was reserved from; the BVH is reserved here too.
   * @param[in]     a_build  BVH build strategy. SAH (binned Surface Area Heuristic) is recommended.
   */
  EBGEOMETRY_HOST
  inline MeshSDF(const Mesh& a_mesh, Pool& a_pool, const BVH::Build a_build);

  /**
   * @brief Compute the signed distance from a_point to the mesh.
   * @param[in] a_point Query point.
   * @return Signed distance to the nearest face; negative inside the mesh.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  signedDistance(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Return faces within BVH-pruned candidate distance of a_point.
   * @details Traverses the PackedBVH and collects candidate faces, pairing each with its unsigned
   * distance to @p a_point. Host-only: it runs on PackedBVH::traverse(), whose callbacks are
   * std::functions, and returns a std::vector.
   *
   * Faces are named by their index into this object's own BVH primitive array -- the array
   * getRoot().getPrimitives() returns -- and *not* by an index into the source mesh's face array.
   * Packing reorders primitives into leaf order and stores them by value, so nothing records which
   * mesh face a packed face came from. Resolve an index with getRoot().getPrimitives()[index].
   * @param[in] a_point  Query point.
   * @param[in] a_sorted If true, the returned vector is sorted by ascending
   * unsigned distance (closest face first).
   * @return Vector of (BVH primitive index, unsigned_distance) pairs, optionally sorted.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline std::vector<std::pair<uint32_t, T>>
  getClosestFaces(const Vec3T<T>& a_point, const bool a_sorted) const;

  /**
   * @brief Get the PackedBVH enclosing the mesh.
   * @details Mutable, so that a caller who moves the mesh's vertices can refit() the BVH in place.
   * @return The packed BVH.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Root&
  getRoot() noexcept
  {
    return m_bvh;
  }

  /**
   * @brief Get the PackedBVH enclosing the mesh (const overload).
   * @return The packed BVH.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const Root&
  getRoot() const noexcept
  {
    return m_bvh;
  }

  /**
   * @brief Get the DCEL mesh descriptor the BVH's faces resolve against.
   * @return The mesh.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const Mesh&
  getMesh() const noexcept
  {
    return m_mesh;
  }

  /**
   * @brief Compute the AABB enclosing the entire mesh.
   * @return Axis-aligned bounding box of the mesh.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline EBGeometry::BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    return m_bvh.getBoundingVolume();
  }

  /**
   * @brief Produce a copy of this object that resolves against @p a_pool.
   * @details Rebases the mesh descriptor and the BVH together; see DCEL::MeshT::rebasedView() and
   * BVH::PackedBVH::rebasedView() for the contract. This is the one sanctioned crossing to a device.
   * @param[in] a_pool Pool to rebase onto; must be a mirror of this object's own pool.
   * @return A MeshSDF resolving against @p a_pool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline MeshSDF
  rebasedView(const Pool& a_pool) const noexcept;

  /**
   * @brief Duplicate the mesh's and the BVH's storage into @p a_dstPool.
   * @details The copy constructor copies descriptors only. This gives genuinely independent storage.
   * The copied faces still index the copied mesh correctly, since a mesh deep copy preserves every
   * element's index.
   * @param[in,out] a_dstPool Pool to reserve the copy from; may be this object's own pool.
   * @return A MeshSDF over independent copies of the mesh and BVH, attached to @p a_dstPool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline MeshSDF
  deepCopy(Pool& a_dstPool) const;

  /**
   * @brief Check whether the mesh and the BVH were both reserved from @p a_pool.
   * @param[in] a_pool Pool to test against.
   * @return True if both are attached to @p a_pool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline bool
  isAttachedTo(const Pool& a_pool) const noexcept
  {
    return m_mesh.isAttachedTo(a_pool) && m_bvh.isAttachedTo(a_pool);
  }

private:
  /**
   * @brief Build and pack the BVH over a mesh's faces.
   * @param[in]     a_mesh  Mesh whose faces to index.
   * @param[in,out] a_pool  Pool to reserve the packed BVH from.
   * @param[in]     a_build BVH build strategy.
   * @return The packed BVH.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static inline Root
  buildBVH(const Mesh& a_mesh, Pool& a_pool, const BVH::Build a_build);

  /**
   * @brief Source DCEL mesh descriptor.
   * @details Held because the faces in m_bvh store their half-edge as an index into this mesh's
   * edge array, meaningful only together with it.
   */
  Mesh m_mesh;

  /**
   * @brief Linearized BVH over copies of the mesh's faces.
   */
  Root m_bvh;
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
 *
 * Each leaf primitive is a TriangleAoSoA<T, Meta, W>: an SoA triangle block for SIMD signed-distance
 * evaluation, plus a physically-separate per-lane metadata array. The hot signedDistance() path
 * never reads the metadata; getClosestTriangle() does, returning the closest triangle's signed
 * distance together with its Meta (see issue #105). The groups are self-contained -- no reference
 * back to a mesh -- so the packed BVH is the only thing this class holds.
 *
 * A plain value type, like FlatMeshSDF and MeshSDF: the BVH is held by value in the Pool passed to
 * the constructor, so the class is trivially copyable and every query is callable on the host and
 * on a device. It does not derive from SignedDistanceFunction. Copies share the pool memory, and
 * the pool must outlive all of them. To evaluate on a device, freeze and mirror the pool and pass
 * rebasedView() into a kernel.
 * @tparam T    Floating-point precision type (float or double).
 * @tparam Meta Triangle metadata type.
 * @tparam K    BVH branching factor (number of children per internal node). Must be >= 2.
 * @tparam W    SoA width: number of triangles per SIMD group. Must be > 0.
 */
template <class T, class Meta, size_t K, size_t W>
class TriMeshSDF
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
   * @brief Alias for the flat triangle type the BVH is built from.
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
   * @brief Full constructor. Extracts flat triangles from a DCEL mesh, then builds the BVH.
   * @details No default arguments: this is a low-level constructor, and callers who excavate down
   * to it must consciously choose every parameter. Use Parser::readIntoTriangleBVH for sensible
   * defaults. The mesh is not retained: its triangles are copied into the BVH's SoA groups.
   * @param[in]     a_mesh          DCEL mesh; every face must be a triangle.
   * @param[in,out] a_pool          Pool the packed BVH is reserved from; must outlive this object.
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
  EBGEOMETRY_HOST
  inline TriMeshSDF(const Mesh& a_mesh, Pool& a_pool, const BVH::Build a_build, const size_t a_maxLeafGroups);

  /**
   * @brief Full constructor. Takes the input triangles and creates the BVH.
   * @param[in]     a_triangles     Input triangle soup; copied into the BVH's SoA groups.
   * @param[in,out] a_pool          Pool the packed BVH is reserved from; must outlive this object.
   * @param[in]     a_build         BVH build strategy (see the mesh-based constructor for details).
   * @param[in]     a_maxLeafGroups Maximum number of full W-sized TriangleSoA groups per BVH leaf (see
   * the mesh-based constructor for the tree-quality/SIMD-occupancy trade-off). Must be > 0.
   */
  EBGEOMETRY_HOST
  inline TriMeshSDF(const std::vector<Tri>& a_triangles,
                    Pool&                   a_pool,
                    const BVH::Build        a_build,
                    const size_t            a_maxLeafGroups);

  /**
   * @brief Compute the signed distance from a_point to the triangle mesh.
   * @param[in] a_point Query point.
   * @return Signed distance to the nearest triangle; negative inside the mesh.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  signedDistance(const Vec3T<T>& a_point) const noexcept;

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
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline ClosestTriangle
  getClosestTriangle(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Get the PackedBVH storing SoA triangle groups.
   * @return The packed BVH.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Root&
  getRoot() noexcept
  {
    return m_bvh;
  }

  /**
   * @brief Get the PackedBVH storing SoA triangle groups (const overload).
   * @return The packed BVH.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const Root&
  getRoot() const noexcept
  {
    return m_bvh;
  }

  /**
   * @brief Compute the AABB enclosing the entire triangle mesh.
   * @return Axis-aligned bounding box of the mesh.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline EBGeometry::BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    return m_bvh.getBoundingVolume();
  }

  /**
   * @brief Produce a copy of this object that resolves against @p a_pool.
   * @details Rebases the BVH; see BVH::PackedBVH::rebasedView() for the contract. This is the one
   * sanctioned crossing to a device.
   * @param[in] a_pool Pool to rebase onto; must be a mirror of this object's own pool.
   * @return A TriMeshSDF resolving against @p a_pool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline TriMeshSDF
  rebasedView(const Pool& a_pool) const noexcept;

  /**
   * @brief Duplicate the BVH's storage into @p a_dstPool.
   * @details The copy constructor copies descriptors only. This gives genuinely independent storage.
   * @param[in,out] a_dstPool Pool to reserve the copy from; may be this object's own pool.
   * @return A TriMeshSDF over an independent copy of the BVH, attached to @p a_dstPool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline TriMeshSDF
  deepCopy(Pool& a_dstPool) const;

  /**
   * @brief Check whether the BVH was reserved from @p a_pool.
   * @param[in] a_pool Pool to test against.
   * @return True if the BVH is attached to @p a_pool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline bool
  isAttachedTo(const Pool& a_pool) const noexcept
  {
    return m_bvh.isAttachedTo(a_pool);
  }

private:
  /**
   * @brief Extract every face of a triangulated DCEL mesh as a flat Triangle.
   * @param[in] a_mesh DCEL mesh; every face must be a triangle.
   * @return One Triangle per face, in face order.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static inline std::vector<Tri>
  extractTriangles(const Mesh& a_mesh);

  /**
   * @brief Build and pack the BVH over a triangle soup.
   * @param[in]     a_triangles     Triangles to index.
   * @param[in,out] a_pool          Pool to reserve the packed BVH from.
   * @param[in]     a_build         BVH build strategy.
   * @param[in]     a_maxLeafGroups Maximum number of W-sized groups per leaf.
   * @return The packed BVH.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static inline Root
  buildBVH(const std::vector<Tri>& a_triangles, Pool& a_pool, const BVH::Build a_build, const size_t a_maxLeafGroups);

  /**
   * @brief Leaf-conversion callback for TreeBVH::packWith: groups a BVH leaf's triangles
   * into SoA blocks of width W.
   * @details Stateless (captures nothing), so it is a static member rather than a lambda.
   * @param[in] a_triangles Leaf's triangle list.
   * @param[in] a_offset    Index of the first triangle in this leaf to convert.
   * @param[in] a_count     Number of triangles in this leaf to convert.
   * @return SoA-packed triangle groups covering [a_offset, a_offset + a_count).
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static std::vector<TriAoSoA>
  groupTrianglesIntoSoA(const std::vector<std::shared_ptr<const Tri>>& a_triangles,
                        uint32_t                                       a_offset,
                        uint32_t                                       a_count);

  /**
   * @brief Bounding volume hierarchy storing SoA triangle groups.
   */
  Root m_bvh;
};

/**
 * @brief MeshSDF and TriMeshSDF must be trivially copyable: that is what lets a rebasedView() be
 * byte-copied into a device address space with no pointer patching.
 */
static_assert(std::is_trivially_copyable_v<MeshSDF<float, DCEL::DefaultMetaData, 4>>,
              "MeshSDF<float, ...> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<MeshSDF<double, DCEL::DefaultMetaData, 4>>,
              "MeshSDF<double, ...> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<TriMeshSDF<float, DCEL::DefaultMetaData, 4, 4>>,
              "TriMeshSDF<float, ...> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<TriMeshSDF<double, DCEL::DefaultMetaData, 4, 4>>,
              "TriMeshSDF<double, ...> must be trivially copyable");

} // namespace EBGeometry

#include "EBGeometry_MeshDistanceFunctionsImplem.hpp"

#endif
