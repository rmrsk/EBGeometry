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
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_Triangle.hpp"
#include "EBGeometry_TriangleAoSoA.hpp"
#include "EBGeometry_TriangleSoA.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief The face of a mesh closest to a query point, as the mesh SDFs' getClosestFace() returns it.
 * @details A plain, trivially copyable value, so getClosestFace() is callable on a device. When
 * several faces are equally close (a point nearest an edge or vertex they share), the first one found
 * is reported.
 * @tparam T Floating-point precision type.
 */
template <class T>
struct ClosestFace
{
  T        signedDistance = Math::Limits<T>::max(); ///< Signed distance to the closest face; negative inside.
  uint32_t faceId         = UINT32_MAX;             ///< Id of the closest face; UINT32_MAX for an empty mesh.
};

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
 */
template <class T>
class FlatMeshSDF
{
  static_assert(std::is_floating_point_v<T>, "FlatMeshSDF requires a floating-point T");

public:
  /**
   * @brief Alias for DCEL mesh type
   */
  using Mesh = EBGeometry::DCEL::MeshT<T>;

  /**
   * @brief Disallowed constructor
   */
  FlatMeshSDF() = delete;

  /**
   * @brief Full constructor.
   * @details Copies the mesh descriptor. Nothing is frozen or bound: the mesh resolves its storage
   * through a_pool's control block on every access, so this object is queryable at once and stays
   * queryable across a Pool::reserve that grows and moves the block. a_pool is taken to check that
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
   * @brief Find the face closest to a_point, and the signed distance to it.
   * @details Scans every face with the rule of DCEL::MeshT::SearchAlgorithm::Direct2, the mesh's
   * default: the face with the smallest unsigned distance wins.
   * @param[in] a_point Query point. Must be finite.
   * @return The closest face's id (its index in the mesh) and signed distance. The signed distance
   * equals what signedDistance() returns for the same point, unless the mesh's search algorithm was
   * changed (DCEL::MeshT::setSearchAlgorithm()).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline ClosestFace<T>
  getClosestFace(const Vec3T<T>& a_point) const noexcept;

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
   * @param[in] a_pool Pool to rebase onto: the object's own pool or one in its mirror chain.
   * @return A FlatMeshSDF resolving against @p a_pool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline FlatMeshSDF
  rebasedView(const Pool& a_pool) const noexcept;

  /**
   * @brief A copy of this object resolving against @p a_location instead of its own pool location.
   * @details For an object stored inside another object's pool, such as a primitive of a
   * BVHUnionIF; see PoolLocation. Unlike rebasedView() it checks nothing: @p a_location must belong
   * to the pool this object was reserved from, or to a mirror of it.
   * @param[in] a_location Location to resolve against.
   * @return A FlatMeshSDF resolving against @p a_location.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline FlatMeshSDF
  relocatedTo(const PoolLocation& a_location) const noexcept;

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
 * @details The BVH's primitives are face ids: indices into the mesh's face array, four bytes each.
 * Every query resolves a face through the mesh, which MeshSDF holds as well, so the mesh is the only
 * copy of the geometry: flipping the mesh (DCEL::MeshT::flip()) after building flips every distance,
 * and after moving vertices, reconciling the mesh and refitting the BVH (see getRoot()) is enough.
 * SIMD node pruning is used when T and K match an available ISA path.
 *
 * A plain value type, like FlatMeshSDF: it holds the mesh descriptor and the BVH by value, both
 * resolving against the one Pool passed to the constructor, so it is trivially copyable and
 * signedDistance() is callable on the host and on a device. It does not derive from
 * SignedDistanceFunction -- a class with virtual functions can never be passed to a kernel. Copies
 * share the pool memory, and the pool must outlive all of them. To evaluate on a device, freeze and
 * mirror the pool and pass rebasedView() into a kernel.
 * @tparam T    Floating-point precision type (float or double).
 * @tparam K    BVH branching factor (number of children per internal node).
 */
template <class T, size_t K>
class MeshSDF
{
  static_assert(std::is_floating_point_v<T>, "MeshSDF requires a floating-point T");
  static_assert(K >= 2, "MeshSDF requires branching factor K >= 2");

public:
  /**
   * @brief Alias for DCEL face type
   */
  using Face = typename EBGeometry::DCEL::FaceT<T>;

  /**
   * @brief Alias for DCEL mesh type
   */
  using Mesh = typename EBGeometry::DCEL::MeshT<T>;

  /**
   * @brief Alias for the linearized BVH root. Its primitives are face ids.
   */
  using Root = EBGeometry::BVH::PackedBVH<T, uint32_t, K>;

  /**
   * @brief Alias for a single linearized node
   */
  using Node = typename Root::Node;

  /**
   * @brief Default disallowed constructor
   */
  MeshSDF() = delete;

  /**
   * @brief The leaf-size settings the constructor without options uses.
   * @details K-1 faces per leaf for the top-down methods (a node with fewer than K faces is a leaf),
   * a target of 1 for the space-filling curves (the deepest balanced tree, at most K faces per
   * leaf), and the default ClusterSpec for ClusterSAH. Start from these to change one method's
   * setting; see BVH::ConstructionOptions.
   * @return The default settings.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static inline BVH::ConstructionOptions
  defaultConstructionOptions() noexcept;

  /**
   * @brief Full constructor. Copies the mesh descriptor and builds the BVH over its faces.
   * @details No default arguments: this is a low-level constructor, and callers working at this
   * level must consciously choose a construction method. Use Parser::readIntoMeshSDF for sensible
   * defaults. The BVH is reserved from a_pool, which must be the pool a_mesh was built in, so that
   * one rebasedView() rebases both. a_pool must outlive this object and every copy of it.
   * @param[in]     a_mesh   Input mesh, built against a_pool.
   * @param[in,out] a_pool   Pool a_mesh's storage was reserved from; the BVH is reserved here too.
   * @param[in]     a_construction  Preset construction method; every BVH::Construction value is supported. SAH
   * (binned Surface Area Heuristic) is recommended. The leaf sizes are defaultConstructionOptions():
   * the top-down methods stop at fewer than K faces per leaf, and ClusterSAH uses the default
   * ClusterSpec, so a leaf holds up to (K-1) clusters of faces.
   */
  EBGEOMETRY_HOST
  inline MeshSDF(const Mesh& a_mesh, Pool& a_pool, const BVH::Construction a_construction);

  /**
   * @brief Full constructor with leaf-size settings.
   * @details As the constructor without options, which uses defaultConstructionOptions(). The
   * chosen method reads only its own field of @p a_options; see BVH::ConstructionOptions.
   * @param[in]     a_mesh         Input mesh, built against a_pool.
   * @param[in,out] a_pool         Pool a_mesh's storage was reserved from; the BVH is reserved here too.
   * @param[in]     a_construction Preset construction method; every BVH::Construction value is supported.
   * @param[in]     a_options      Leaf-size settings, in faces.
   */
  EBGEOMETRY_HOST
  inline MeshSDF(const Mesh&                     a_mesh,
                 Pool&                           a_pool,
                 const BVH::Construction         a_construction,
                 const BVH::ConstructionOptions& a_options);

  /**
   * @brief Compute the signed distance from a_point to the mesh.
   * @param[in] a_point Query point.
   * @return Signed distance to the nearest face; negative inside the mesh.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  signedDistance(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Find the face closest to a_point, and the signed distance to it.
   * @details The same BVH traversal as signedDistance() (PackedBVH::pruneTraverse), keeping the
   * winning face's id as well, so it is callable on a device too.
   * @param[in] a_point Query point. Must be finite.
   * @return The closest face's id (its index in the mesh) and signed distance. The signed distance
   * equals what signedDistance() returns for the same point.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline ClosestFace<T>
  getClosestFace(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Get the PackedBVH enclosing the mesh.
   * @details Mutable, so that PackedBVH::refit() can be called on it. After moving the mesh's
   * vertices, reconcile the mesh (DCEL::MeshT::reconcile()), which recomputes its normals, then
   * refit with each face's new bounding box:
   * @code
   * sdf.getRoot().refit([&mesh](uint32_t a_face) {
   *   return BoundingVolumes::AABBT<T>(mesh.getFace(a_face).getAllVertexCoordinates(mesh));
   * });
   * @endcode
   * This is enough because the BVH stores face ids and reads the faces from the mesh. A deformation
   * large enough to degrade the tree still calls for a rebuild.
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
   * @param[in] a_pool Pool to rebase onto: the object's own pool or one in its mirror chain.
   * @return A MeshSDF resolving against @p a_pool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline MeshSDF
  rebasedView(const Pool& a_pool) const noexcept;

  /**
   * @brief A copy of this object resolving against @p a_location instead of its own pool location.
   * @details For an object stored inside another object's pool, such as a primitive of a
   * BVHUnionIF; see PoolLocation. Unlike rebasedView() it checks nothing: @p a_location must belong
   * to the pool this object was reserved from, or to a mirror of it.
   * @param[in] a_location Location to resolve against.
   * @return A MeshSDF resolving against @p a_location.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline MeshSDF
  relocatedTo(const PoolLocation& a_location) const noexcept;

  /**
   * @brief Duplicate the mesh's and the BVH's storage into @p a_dstPool.
   * @details The copy constructor copies descriptors only. This gives genuinely independent storage.
   * The copied face ids still name the right faces of the copied mesh, since a mesh deep copy
   * preserves every element's index.
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
   * @param[in]     a_construction Preset BVH construction method.
   * @param[in]     a_options      Leaf-size settings.
   * @return The packed BVH.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static inline Root
  buildBVH(const Mesh&                     a_mesh,
           Pool&                           a_pool,
           const BVH::Construction         a_construction,
           const BVH::ConstructionOptions& a_options);

  /**
   * @brief Source DCEL mesh descriptor.
   * @details The BVH's primitives are face ids into this mesh, meaningful only together with it.
   */
  Mesh m_mesh;

  /**
   * @brief Linearized BVH over the mesh's face ids.
   */
  Root m_bvh;
};

/**
 * @brief Signed distance function for a pure triangle mesh using SoA-grouped primitives
 * in a compact (linearized) BVH.
 * @details Triangles are packed into SoA groups of W triangles each (TriangleAoSoA<T,W>), enabling
 * SIMD evaluation of up to W signed distances simultaneously.
 *
 * No default arguments: this is a low-level constructor, and callers who excavate down to it
 * must consciously choose K and W. BVH::DefaultBranchingRatio<T>() and TriangleSoA::DefaultWidth<T>()
 * (both 4) are the portable choice, and Parser::readIntoTriMeshSDF's defaults.
 *
 * Each leaf primitive is a TriangleAoSoA<T, W>: an SoA triangle block for SIMD signed-distance
 * evaluation, plus a physically separate per-lane array of face ids, the index of the mesh face each
 * triangle was cut from. The hot signedDistance() path never reads the ids; getClosestFace() does,
 * returning the closest triangle's signed distance together with its face id (see issue #105). The
 * groups are self-contained -- no reference back to a mesh -- so the packed BVH is the only thing
 * this class holds, and a mesh changed after the build is not seen.
 *
 * A plain value type, like FlatMeshSDF and MeshSDF: the BVH is held by value in the Pool passed to
 * the constructor, so the class is trivially copyable and every query is callable on the host and
 * on a device. It does not derive from SignedDistanceFunction. Copies share the pool memory, and
 * the pool must outlive all of them. To evaluate on a device, freeze and mirror the pool and pass
 * rebasedView() into a kernel.
 * @tparam T    Floating-point precision type (float or double).
 * @tparam K    BVH branching factor (number of children per internal node). Must be >= 2.
 * @tparam W    SoA width: number of triangles per SIMD group. Must be > 0.
 */
template <class T, size_t K, size_t W>
class TriMeshSDF
{
  static_assert(std::is_floating_point_v<T>, "TriMeshSDF<T,K,W> requires a floating-point T");
  static_assert(K >= 2, "TriMeshSDF requires branching factor K >= 2");
  static_assert(W > 0, "TriMeshSDF requires SoA width W > 0");

public:
  /**
   * @brief Alias for DCEL mesh type
   */
  using Mesh = EBGeometry::DCEL::MeshT<T>;

  /**
   * @brief Alias for the flat triangle type the BVH is built from.
   */
  using Tri = typename EBGeometry::Triangle<T>;

  /**
   * @brief Alias for the face-id-carrying SoA triangle group type (the BVH leaf primitive).
   */
  using TriAoSoA = TriangleAoSoA<T, W>;

  /**
   * @brief Alias for which BVH root node
   */
  using Root = typename EBGeometry::BVH::PackedBVH<T, TriAoSoA, K>;

  /**
   * @brief Default disallowed constructor
   */
  TriMeshSDF() = delete;

  /**
   * @brief The leaf-size settings the constructors taking a_maxLeafGroups use.
   * @details At most a_maxLeafGroups * W triangles per leaf for the top-down methods, the same as the
   * target for the space-filling curves, and for ClusterSAH a cluster size that keeps a leaf of K-1
   * clusters within that bound (or within K-1 triangles, if the bound is smaller, since a cluster
   * holds at least one). Start from these to change one method's setting; see
   * BVH::ConstructionOptions.
   * @param[in] a_maxLeafGroups Maximum number of full W-sized TriangleSoA groups per leaf. Must be > 0.
   * @return The settings, in triangles.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static inline BVH::ConstructionOptions
  defaultConstructionOptions(const size_t a_maxLeafGroups) noexcept;

  /**
   * @brief Full constructor. Extracts flat triangles from a DCEL mesh, then builds the BVH.
   * @details No default arguments: this is a low-level constructor, and callers who excavate down
   * to it must consciously choose every parameter. Use Parser::readIntoTriMeshSDF for sensible
   * defaults. The mesh is not retained: its triangles are copied into the BVH's SoA groups, each
   * carrying the id of the face it was cut from.
   * @param[in]     a_mesh          DCEL mesh. Faces with more than three vertices are fan-triangulated.
   * @param[in,out] a_pool          Pool the packed BVH is reserved from; must outlive this object.
   * @param[in]     a_construction         Preset construction method; every BVH::Construction value is supported.
   * SAH (binned Surface Area Heuristic) produces near-optimal traversal cost; CentroidSplit and
   * MidpointSplit are faster to build but yield deeper trees. Every method honours a_maxLeafGroups
   * through defaultConstructionOptions(): the top-down methods as a bound, the space-filling curves as
   * a target, and ClusterSAH through its cluster size.
   * @param[in]     a_maxLeafGroups Maximum number of full W-sized TriangleSoA groups per BVH leaf; the
   * actual raw-triangle leaf-size bound used is a_maxLeafGroups * W. This bounds the pre-packing
   * tree's leaf size, not the packed representation directly: each leaf's triangles become their
   * own TriangleSoA group(s) during packing, with no batching across leaves, so a leaf smaller
   * than W wastes some of its group's SIMD lanes on padding. It is an upper bound, not a target —
   * the top-down partitioner still splits down to tighter, more selective leaves wherever the
   * geometry warrants it. Expressing this as a count of W-sized groups (rather than a raw triangle
   * count) makes it impossible to accidentally pick a leaf size that isn't a multiple of W. Must
   * be > 0.
   */
  EBGEOMETRY_HOST
  inline TriMeshSDF(const Mesh&             a_mesh,
                    Pool&                   a_pool,
                    const BVH::Construction a_construction,
                    const size_t            a_maxLeafGroups);

  /**
   * @brief Full constructor. Takes the input triangles and creates the BVH.
   * @param[in]     a_triangles     Input triangle soup; copied into the BVH's SoA groups.
   * getClosestFace() reports the face id each triangle carries (Triangle::getFaceId()).
   * @param[in,out] a_pool          Pool the packed BVH is reserved from; must outlive this object.
   * @param[in]     a_construction         Preset BVH construction method (see the mesh-based constructor for details).
   * @param[in]     a_maxLeafGroups Maximum number of full W-sized TriangleSoA groups per BVH leaf (see
   * the mesh-based constructor for the tree-quality/SIMD-occupancy trade-off). Must be > 0.
   */
  EBGEOMETRY_HOST
  inline TriMeshSDF(const std::vector<Tri>& a_triangles,
                    Pool&                   a_pool,
                    const BVH::Construction a_construction,
                    const size_t            a_maxLeafGroups);

  /**
   * @brief Full constructor from a DCEL mesh, with leaf-size settings.
   * @details As the constructor taking a_maxLeafGroups, which uses
   * defaultConstructionOptions(a_maxLeafGroups). The chosen method reads only its own field of
   * @p a_options, counted in triangles; see BVH::ConstructionOptions. A leaf's triangles are packed
   * into ceil(n / W) groups.
   * @param[in]     a_mesh         DCEL mesh. Faces with more than three vertices are fan-triangulated.
   * @param[in,out] a_pool         Pool the packed BVH is reserved from; must outlive this object.
   * @param[in]     a_construction Preset construction method; every BVH::Construction value is supported.
   * @param[in]     a_options      Leaf-size settings, in triangles.
   */
  EBGEOMETRY_HOST
  inline TriMeshSDF(const Mesh&                     a_mesh,
                    Pool&                           a_pool,
                    const BVH::Construction         a_construction,
                    const BVH::ConstructionOptions& a_options);

  /**
   * @brief Full constructor from a triangle soup, with leaf-size settings.
   * @param[in]     a_triangles    Input triangle soup; copied into the BVH's SoA groups.
   * @param[in,out] a_pool         Pool the packed BVH is reserved from; must outlive this object.
   * @param[in]     a_construction Preset construction method; every BVH::Construction value is supported.
   * @param[in]     a_options      Leaf-size settings, in triangles; see the mesh-based constructor.
   */
  EBGEOMETRY_HOST
  inline TriMeshSDF(const std::vector<Tri>&         a_triangles,
                    Pool&                           a_pool,
                    const BVH::Construction         a_construction,
                    const BVH::ConstructionOptions& a_options);

  /**
   * @brief Compute the signed distance from a_point to the triangle mesh.
   * @param[in] a_point Query point.
   * @return Signed distance to the nearest triangle; negative inside the mesh.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  signedDistance(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Find the face closest to a_point, and the signed distance to it.
   * @details The face-id-retrieving companion to signedDistance(): it drives the same SIMD-pruned
   * BVH traversal (PackedBVH::pruneTraverse), but each visited leaf group reports the winning
   * triangle's face id via TriangleAoSoA::signedDistance(point, uint32_t&) (see issue #105).
   * @param[in] a_point Query point. Must be finite.
   * @return The face id of the closest triangle and the signed distance to it. The signed distance
   * agrees with what signedDistance() returns for the same point up to rounding: signedDistance()
   * reduces each leaf group with SIMD instructions, this function lane by lane.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline ClosestFace<T>
  getClosestFace(const Vec3T<T>& a_point) const noexcept;

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
   * @param[in] a_pool Pool to rebase onto: the object's own pool or one in its mirror chain.
   * @return A TriMeshSDF resolving against @p a_pool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline TriMeshSDF
  rebasedView(const Pool& a_pool) const noexcept;

  /**
   * @brief A copy of this object resolving against @p a_location instead of its own pool location.
   * @details For an object stored inside another object's pool, such as a primitive of a
   * BVHUnionIF; see PoolLocation. Unlike rebasedView() it checks nothing: @p a_location must belong
   * to the pool this object was reserved from, or to a mirror of it.
   * @param[in] a_location Location to resolve against.
   * @return A TriMeshSDF resolving against @p a_location.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline TriMeshSDF
  relocatedTo(const PoolLocation& a_location) const noexcept;

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
   * @brief Extract every face of a DCEL mesh as flat Triangles, fan-triangulating polygons.
   * @param[in] a_mesh DCEL mesh.
   * @return The triangles of every face, in face order.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static inline std::vector<Tri>
  extractTriangles(const Mesh& a_mesh);

  /**
   * @brief Build and pack the BVH over a triangle soup.
   * @param[in]     a_triangles     Triangles to index.
   * @param[in,out] a_pool          Pool to reserve the packed BVH from.
   * @param[in]     a_construction  Preset BVH construction method.
   * @param[in]     a_options       Leaf-size settings, in triangles.
   * @return The packed BVH.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static inline Root
  buildBVH(const std::vector<Tri>&         a_triangles,
           Pool&                           a_pool,
           const BVH::Construction         a_construction,
           const BVH::ConstructionOptions& a_options);

  /**
   * @brief Build the BVH with ClusterSAH, which has no TreeBVH form.
   * @details Builds a ClusterSAH PackedBVH over the triangle indices in a scratch pool, then copies
   * it node for node with each leaf's triangles regrouped into W-wide SoA groups.
   * @param[in]     a_triangles   Triangles to index.
   * @param[in,out] a_pool        Pool to reserve the packed BVH from.
   * @param[in]     a_spec        Cluster size; a leaf holds 1 to K-1 clusters.
   * @return The packed BVH.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static inline Root
  buildClusterSAH(const std::vector<Tri>& a_triangles, Pool& a_pool, const BVH::ClusterSpec& a_spec);

  /**
   * @brief Pack a run of triangles into W-wide SoA groups; the last group may be partly filled.
   * @tparam GetTriangle Callable taking an index in [0, a_count) and returning a const Tri&.
   * @param[in] a_getTriangle Accessor for the run's triangles.
   * @param[in] a_count       Number of triangles in the run.
   * @return ceil(a_count / W) groups, in run order.
   */
  template <class GetTriangle>
  [[nodiscard]] EBGEOMETRY_HOST
  static inline std::vector<TriAoSoA>
  groupTriangles(const GetTriangle& a_getTriangle, uint32_t a_count);

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
static_assert(std::is_trivially_copyable_v<MeshSDF<float, 4>>, "MeshSDF<float, 4> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<MeshSDF<double, 4>>, "MeshSDF<double, 4> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<TriMeshSDF<float, 4, 4>>,
              "TriMeshSDF<float, 4, 4> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<TriMeshSDF<double, 4, 4>>,
              "TriMeshSDF<double, 4, 4> must be trivially copyable");

} // namespace EBGeometry

#include "EBGeometry_MeshDistanceFunctionsImplem.hpp"

#endif
