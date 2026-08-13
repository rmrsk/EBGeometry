// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_DCEL_Mesh.hpp
 * @brief  Declaration of a mesh class which stores a DCEL mesh (with signed
 * distance functions)
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_DCEL_MESH_HPP
#define EBGEOMETRY_DCEL_MESH_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_DCEL.hpp"
#include "EBGeometry_DCEL_Edge.hpp"
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_DCEL_Vertex.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_PODVector.hpp"
#include "EBGeometry_Pool.hpp"

namespace EBGeometry {

namespace DCEL {

/**
 * @brief Mesh class which stores a full DCEL mesh (with signed distance
 * functions)
 * @details This encapsulates a full DCEL mesh, and also includes DIRECT signed
 * distance functions. The mesh consists of a set of vertices, half-edges, and
 * polygon faces, stored as PODVectors; every cross-reference between them
 * (vertex-to-outgoing-edge, edge-to-vertex/pair edge/next edge/face,
 * face-to-half-edge) is an index into these arrays rather than a pointer --
 * see the class-level notes on VertexT/EdgeT/FaceT. The signed distance
 * functions DIRECT, which means that they go through ALL of the polygon faces
 * and compute the signed distance to them. This is extremely inefficient,
 * which is why this class is almost always embedded into a bounding volume
 * hierarchy.
 * @note MeshT owns no resources: every member (three PODVectors, the
 * search-algorithm enum, a control-block pointer and a base pointer) is a
 * plain value, so MeshT is trivially copyable -- a bytewise copy is
 * meaningful in any address space, since it never needs to dereference
 * anything beyond its own bytes. This is what makes a mesh built and
 * reconciled on the host mirror-safe: its Pool can be mirrored to a device
 * and the mesh rebased onto the mirror (see rebasedView()), with zero
 * pointer patching inside the mesh itself.
 * @details A mesh resolves its PODVector offsets through whichever of two
 * pointers is set, and exactly one of them always is:
 * - m_control, the stable control block of the Pool the mesh was reserved
 *   from. Set on the first reserveVertices/Edges/Faces() call. Because the
 *   base is re-read from the control block on every access, a mesh is
 *   queryable at any point during the build, including across a
 *   Pool::reserve that grows and moves the block -- there is no freeze,
 *   and nothing to re-bind.
 * - m_base, a plain base address, set only by rebasedView() and only for a
 *   device-accessible target, since a kernel cannot follow a control block.
 * A device view therefore has m_control == nullptr and nothing else does,
 * which is what lets base() assert exactly, in both directions: a non-null
 * m_control on device means a host descriptor was copied into a kernel
 * without rebasedView(), and a null m_control on the host means either a
 * device view being dereferenced on the host or a mesh nobody ever
 * reserved into.
 * @warning A reference returned by getVertex()/getEdge()/getFace() is a raw
 * address resolved at the moment of the call, and Pool::reserve may
 * deallocate the block it points into (see the warning on Pool::reserve).
 * Resolve, use, discard: to carry an element across a reserve, copy it by
 * value and write it back through a fresh accessor call.
 * @details Build-phase mutators (reserveVertices/Edges/Faces,
 * addVertex/Edge/Face) take an explicit Pool&, since reserving can grow
 * (and move) the Pool's block; building only happens on the host, with a
 * live Pool in hand.
 * @note Everything that resolves purely through base(), the PODVectors, and VertexT/EdgeT/FaceT
 * methods that are themselves EBGEOMETRY_HOST_DEVICE (getVertex/getEdge/getFace, numVertices/
 * numEdges/numFaces, setSearchAlgorithm, setInsideOutsideAlgorithm, reconcileEdges,
 * flipFaceNormals/flipEdgeNormals/flipVertexNormals, flip, DirectSignedDistance/
 * DirectSignedDistance2, every signedDistance overload, unsignedDistance2) is annotated
 * EBGEOMETRY_HOST_DEVICE -- signedDistance's algorithm-selecting switch deliberately relies on
 * EBGEOMETRY_EXPECT() alone (not std::cerr) to flag a corrupted SearchAlgorithm, specifically so it
 * stays device-callable; see the implementation. The rest stays EBGEOMETRY_HOST because it touches a
 * Pool directly (rebasedView, deepCopy, reserveVertices/Edges/Faces, addVertex/Edge/Face), returns a
 * host container (getAllVertexCoordinates), logs diagnostics to std::cerr (sanityCheck and its
 * incrementWarning/printWarnings helpers), or delegates to a VertexT/EdgeT/FaceT method that itself
 * allocates a std::vector or can log a diagnostic the same way (reconcileFaces calls FaceT::reconcile;
 * reconcileVertices builds a transient per-vertex face list and can warn on a corrupted
 * VertexNormalWeight) -- reconcile/reconcileFaces/reconcileVertices therefore remain EBGEOMETRY_HOST.
 * @tparam T    Floating-point precision type.
 * @tparam Meta User-defined metadata type.
 */
template <class T, class Meta>
class MeshT
{
  static_assert(std::is_floating_point_v<T>, "MeshT requires a floating-point T");

public:
  /**
   * @brief Possible search algorithms for DCEL::MeshT
   */
  enum class SearchAlgorithm
  {
    Direct, ///< Computes the signed distance to every face and returns the one with the smallest
            ///< magnitude. See DirectSignedDistance().
    Direct2 ///< Computes the squared unsigned distance to every face to find the closest one, then
            ///< returns the signed distance to only that face. Equivalent to Direct but usually
            ///< faster since it avoids a square root per face. See DirectSignedDistance2().
  };

  /**
   * @brief Alias for vector type
   */
  using Vec3 = Vec3T<T>;

  /**
   * @brief Alias for vertex type
   */
  using Vertex = VertexT<T, Meta>;

  /**
   * @brief Alias for edge type
   */
  using Edge = EdgeT<T, Meta>;

  /**
   * @brief Alias for face type
   */
  using Face = FaceT<T, Meta>;

  /**
   * @brief Alias for mesh type
   */
  using Mesh = MeshT<T, Meta>;

  /**
   * @brief Default constructor. Leaves the mesh empty (no vertices, edges, faces) and attached to
   * no Pool, so it cannot yet resolve anything.
   * @details No Pool is needed at construction -- every build-phase method (reserveX/addX) takes
   * its Pool explicitly. Use reserveVertices()/reserveEdges()/reserveFaces() followed by
   * addVertex()/addEdge()/addFace() to populate the mesh (a file parser normally does this); the
   * first reserve attaches the mesh to that Pool, after which it is queryable immediately -- there
   * is no separate binding or freezing step.
   */
  MeshT() noexcept = default;

  /**
   * @brief Copy constructor.
   * @details Defaulted memberwise copy. Every member is a plain value (three PODVectors, the
   * search-algorithm enum, and a resolved base pointer), so this is a cheap, always-safe descriptor
   * copy -- it does NOT duplicate the underlying vertex/edge/face data, which stays shared between
   * the original and the copy (both still resolve against the same Pool/base). Use deepCopy() when
   * independent, separately-owned storage is required (e.g. in a different Pool).
   * @param[in] a_otherMesh Other mesh.
   */
  MeshT(const Mesh& a_otherMesh) noexcept = default;

  /**
   * @brief Move constructor.
   * @details Defaulted memberwise move. Since every member is a plain value, this is identical to
   * the copy constructor -- the moved-from mesh is left referencing the exact same data (not
   * emptied), since MeshT never owned that data in the first place.
   * @param[in, out] a_otherMesh Other mesh.
   */
  MeshT(Mesh&& a_otherMesh) noexcept = default;

  /**
   * @brief Destructor (does nothing; MeshT owns no resources).
   */
  ~MeshT() noexcept = default;

  /**
   * @brief Copy assignment operator.
   * @details Has the same semantics as the copy constructor; see its documentation.
   * @param[in] a_otherMesh Other mesh.
   * @return Reference to (*this).
   */
  Mesh&
  operator=(const Mesh& a_otherMesh) noexcept = default;

  /**
   * @brief Move assignment operator.
   * @details Has the same semantics as the move constructor; see its documentation.
   * @param[in, out] a_otherMesh Other mesh.
   * @return Reference to (*this).
   */
  Mesh&
  operator=(Mesh&& a_otherMesh) noexcept = default;

  /**
   * @brief Create an independent, fully-decoupled copy of this mesh's data in another Pool.
   * @details Reserves fresh storage in a_dstPool and copies every vertex/edge/face by value; since
   * every cross-reference is an index rather than a pointer, the copy is already fully independent
   * and correctly linked -- no relinking pass is needed. Every field is preserved exactly (position,
   * normal vector, centroid, area, meta-data, search algorithm) rather than recomputed, so a mesh
   * that has had flipNormal()/flip() called on it and not yet been re-reconciled is copied
   * faithfully rather than silently "un-flipped". Unlike the copy/move constructors, this creates
   * genuinely separate storage rather than sharing the source's, and the returned mesh is attached
   * to a_dstPool.
   * @note a_dstPool may be the same Pool this mesh's data already lives in. That is safe even
   * though the reserves below can grow (and move) the block, because both meshes re-resolve through
   * the Pool's control block on every access rather than caching an address.
   * @param[in,out] a_dstPool Pool to reserve the copy's storage from.
   * @return A new mesh, independent of this one.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline std::shared_ptr<Mesh>
  deepCopy(Pool& a_dstPool) const;

  /**
   * @brief Perform a sanity check.
   * @details This will provide error messages if vertices are badly linked,
   * faces have no half-edge, and so on. These messages are logged by calling
   * incrementWarning() which identifies types of errors that can occur, and how
   * many of those errors have occurred.
   * @param[in] a_id Identifier when printing error messages (can be empty string).
   */
  EBGEOMETRY_HOST
  inline void
  sanityCheck(const std::string a_id) const;

  /**
   * @brief Search algorithm for direct signed distance computations
   * @param[in] a_algorithm Algorithm to use
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  setSearchAlgorithm(const SearchAlgorithm a_algorithm) noexcept;

  /**
   * @brief Set the inside/outside algorithm to use when computing the signed distance to polygon
   * faces.
   * @details Computing the signed distance to faces requires testing if a point
   * projected to a polygo face plane falls inside or outside the polygon face.
   * There are multiple algorithms to use here.
   * @param[in] a_algorithm Algorithm to use
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  setInsideOutsideAlgorithm(InsideOutsideAlgorithm a_algorithm) noexcept;

  /**
   * @brief Reconcile function which computes the internal parameters in vertices, edges, and faces
   * for use with signed distance functionality.
   * @param[in] a_weight Vertex angle weighting function. Either
   * VertexNormalWeight::None for unweighted vertex normals or
   * VertexNormalWeight::Angle for the pseudonormal
   * @details This will reconcile faces, edges, and vertices, e.g. computing the
   * area and normal vector for faces.
   */
  EBGEOMETRY_HOST
  inline void
  reconcile(const DCEL::VertexNormalWeight a_weight = DCEL::VertexNormalWeight::Angle) noexcept;

  /**
   * @brief Flip the mesh, making all the normals change direction.
   * @note Should be called AFTER all normals have been computed.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  flip() noexcept;

  /**
   * @brief Reserve storage for a_capacity vertices.
   * @details Must be called before any addVertex() call, and only once -- PODVector never
   * reallocates once reserved (see EBGeometry_PODVector.hpp).
   * @param[in,out] a_pool     Pool to reserve from (must not be frozen).
   * @param[in]     a_capacity Number of vertex slots to reserve.
   */
  EBGEOMETRY_HOST
  inline void
  reserveVertices(Pool& a_pool, uint32_t a_capacity);

  /**
   * @brief Reserve storage for a_capacity half-edges.
   * @details Must be called before any addEdge() call, and only once -- PODVector never
   * reallocates once reserved (see EBGeometry_PODVector.hpp).
   * @param[in,out] a_pool     Pool to reserve from (must not be frozen).
   * @param[in]     a_capacity Number of half-edge slots to reserve.
   */
  EBGEOMETRY_HOST
  inline void
  reserveEdges(Pool& a_pool, uint32_t a_capacity);

  /**
   * @brief Reserve storage for a_capacity faces.
   * @details Must be called before any addFace() call, and only once -- PODVector never
   * reallocates once reserved (see EBGeometry_PODVector.hpp).
   * @param[in,out] a_pool     Pool to reserve from (must not be frozen).
   * @param[in]     a_capacity Number of face slots to reserve.
   */
  EBGEOMETRY_HOST
  inline void
  reserveFaces(Pool& a_pool, uint32_t a_capacity);

  /**
   * @brief Append a vertex into the pre-reserved capacity (see reserveVertices()).
   * @param[in,out] a_pool   Pool this mesh's vertices were reserved from.
   * @param[in]     a_vertex Vertex to append.
   * @return Index of the newly-appended vertex in this mesh's vertex array.
   */
  EBGEOMETRY_HOST
  inline uint32_t
  addVertex(Pool& a_pool, const Vertex& a_vertex);

  /**
   * @brief Append a half-edge into the pre-reserved capacity (see reserveEdges()).
   * @param[in,out] a_pool Pool this mesh's half-edges were reserved from.
   * @param[in]     a_edge Half-edge to append.
   * @return Index of the newly-appended half-edge in this mesh's edge array.
   */
  EBGEOMETRY_HOST
  inline uint32_t
  addEdge(Pool& a_pool, const Edge& a_edge);

  /**
   * @brief Append a face into the pre-reserved capacity (see reserveFaces()).
   * @param[in,out] a_pool Pool this mesh's faces were reserved from.
   * @param[in]     a_face Face to append.
   * @return Index of the newly-appended face in this mesh's face array.
   */
  EBGEOMETRY_HOST
  inline uint32_t
  addFace(Pool& a_pool, const Face& a_face);

  /**
   * @brief Get modifiable vertex by index.
   * @warning The returned reference is invalidated by any Pool::reserve on this mesh's Pool; see
   * the class-level warning.
   * @param[in] a_index Vertex index (must be < numVertices()).
   * @return Reference to the vertex.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Vertex&
  getVertex(uint32_t a_index) noexcept;

  /**
   * @brief Get immutable vertex by index.
   * @warning The returned reference is invalidated by any Pool::reserve on this mesh's Pool; see
   * the class-level warning.
   * @param[in] a_index Vertex index (must be < numVertices()).
   * @return Const reference to the vertex.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const Vertex&
  getVertex(uint32_t a_index) const noexcept;

  /**
   * @brief Get modifiable half-edge by index.
   * @warning The returned reference is invalidated by any Pool::reserve on this mesh's Pool; see
   * the class-level warning.
   * @param[in] a_index Half-edge index (must be < numEdges()).
   * @return Reference to the half-edge.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Edge&
  getEdge(uint32_t a_index) noexcept;

  /**
   * @brief Get immutable half-edge by index.
   * @warning The returned reference is invalidated by any Pool::reserve on this mesh's Pool; see
   * the class-level warning.
   * @param[in] a_index Half-edge index (must be < numEdges()).
   * @return Const reference to the half-edge.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const Edge&
  getEdge(uint32_t a_index) const noexcept;

  /**
   * @brief Get modifiable face by index.
   * @warning The returned reference is invalidated by any Pool::reserve on this mesh's Pool; see
   * the class-level warning.
   * @param[in] a_index Face index (must be < numFaces()).
   * @return Reference to the face.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Face&
  getFace(uint32_t a_index) noexcept;

  /**
   * @brief Get immutable face by index.
   * @warning The returned reference is invalidated by any Pool::reserve on this mesh's Pool; see
   * the class-level warning.
   * @param[in] a_index Face index (must be < numFaces()).
   * @return Const reference to the face.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const Face&
  getFace(uint32_t a_index) const noexcept;

  /**
   * @brief Whether this mesh's storage was reserved from a_pool.
   * @details A mesh attaches to a Pool on its first reserveVertices/Edges/Faces() call and resolves
   * everything through that Pool thereafter, so the Pool must outlive the mesh. This is the check a
   * caller that holds both can make to confirm they belong together.
   * @param[in] a_pool Pool to test against.
   * @return True if this mesh resolves through a_pool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline bool
  isAttachedTo(const Pool& a_pool) const noexcept;

  /**
   * @brief Number of vertices in this mesh.
   * @return Vertex count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  numVertices() const noexcept;

  /**
   * @brief Number of half-edges in this mesh.
   * @return Half-edge count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  numEdges() const noexcept;

  /**
   * @brief Number of faces in this mesh.
   * @return Face count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  numFaces() const noexcept;

  /**
   * @brief Return all vertex coordinates in the mesh.
   * @return Vector of 3D coordinates of all vertices.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline std::vector<Vec3T<T>>
  getAllVertexCoordinates() const noexcept;

  /**
   * @brief Compute the signed distance from a point to this mesh.
   * @param[in] a_x0 3D point in space.
   * @details This function will iterate through ALL faces in the mesh and return
   * the value with the smallest magnitude. This is horrendously slow, which is
   * why this function is almost never called. Rather, MeshT<T, Meta> can be embedded
   * in a bounding volume hierarchy for faster access.
   * @note This will call the other version with the object's search algorithm.
   * @return Signed distance to the mesh; negative inside, positive outside. Returns +infinity if
   * the mesh has no faces.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  signedDistance(const Vec3& a_x0) const noexcept;

  /**
   * @brief Compute the signed distance from a point to this mesh, using an explicit search
   * algorithm.
   * @param[in] a_x0        3D point in space.
   * @param[in] a_algorithm Search algorithm
   * @details This function will iterate through ALL faces in the mesh and return
   * the value with the smallest magnitude. This is horrendously slow, which is
   * why this function is almost never called. Rather, MeshT<T, Meta> can be embedded
   * in a bounding volume hierarchy for faster access.
   * @return Signed distance to the mesh; negative inside, positive outside. Returns +infinity if
   * the mesh has no faces.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  signedDistance(const Vec3& a_x0, SearchAlgorithm a_algorithm) const noexcept;

  /**
   * @brief Compute the unsigned square distance from a point to this mesh.
   * @param[in] a_x0 3D point in space.
   * @details This function will iterate through ALL faces in the mesh and return
   * the value with the smallest magnitude. This is horrendously slow, which is
   * why this function is almost never called. Rather, MeshT<T, Meta> can be embedded
   * in a bounding volume hierarchy for faster access.
   * @return Squared unsigned distance to the nearest face, or +infinity if the mesh has no faces.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  unsignedDistance2(const Vec3& a_x0) const noexcept;

  /**
   * @brief Produce a copy of this mesh's descriptor that resolves against a mirror of this mesh's
   * Pool, rather than against the Pool the mesh was built in.
   * @details This is the one sanctioned way to move a mesh between address spaces. Only the
   * descriptor is copied -- the vertex/edge/face data is whatever a_pool already contains, which
   * Pool::mirror put there. Since every cross-reference is a byte offset rather than a pointer, no
   * patching is needed on either side:
   *
   * @code
   * hostPool.freeze();
   * Pool       devicePool = Pool::mirror(hostPool, deviceMemoryResource());
   * const Mesh deviceMesh = mesh->rebasedView(devicePool);   // rebase on the host ...
   * myKernel<<<blocks, threads>>>(deviceMesh, ...);          // ... then copy by value
   * @endcode
   *
   * How the returned view resolves depends on a_pool, not on a separate choice by the caller:
   * - a device-accessible target (Device, Managed, Mapped) yields a view holding a plain base
   *   address, since a kernel cannot follow a host control block. Such a view must not be
   *   dereferenced on the host, which base() asserts.
   * - a host-only target (Host, Pinned) yields a view holding that Pool's control block, so it is
   *   growth-immune exactly like the original mesh.
   * @note a_pool must be a mirror of the Pool this mesh was built in -- directly, or through any
   * number of intermediate mirrors, since Pool::mirrorOf() names the root of the chain (so
   * host -> pinned staging -> device works). It must also be large enough to contain this mesh's
   * arrays, which catches a Pool mirrored before the mesh's last reserve. Both are
   * EBGEOMETRY_EXPECT-checked.
   * @param[in] a_pool Pool to rebase onto; a mirror of this mesh's own Pool.
   * @return A mesh descriptor resolving against a_pool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline Mesh
  rebasedView(const Pool& a_pool) const noexcept;

protected:
  /**
   * @brief Control block of the Pool this mesh was reserved from; null if, and only if, this is a
   * device view produced by rebasedView().
   * @details Host bookkeeping. Never mirrored, never dereferenced from device code. Re-reading the
   * base through it on every access is what makes a mesh immune to a Pool::reserve that grows and
   * moves the block.
   */
  const PoolControl* m_control = nullptr;

  /**
   * @brief Base address for a device view, set by rebasedView() and unused otherwise.
   * @note Write-only on the host: nothing reads this until the descriptor has been byte-copied into
   * a device address space, so it will look dead to a host-only reader.
   */
  void* m_base = nullptr;

  /**
   * @brief Search algorithm. Only used in signed distance functions.
   */
  SearchAlgorithm m_algorithm = SearchAlgorithm::Direct2;

  /**
   * @brief Mesh vertices
   */
  PODVector<Vertex> m_vertices;

  /**
   * @brief Mesh half-edges
   */
  PODVector<Edge> m_edges;

  /**
   * @brief Mesh faces
   */
  PODVector<Face> m_faces;

  /**
   * @brief Resolve this mesh's current base address.
   * @details Reads through m_control on the host (so a Pool::reserve that moves the block is
   * invisible) and through m_base on device. Exactly one of the two is set; which one is asserted,
   * since a mismatch means a descriptor crossed an address-space boundary the wrong way.
   * @return Base address to resolve this mesh's PODVector offsets against.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline void*
  base() const noexcept;

  /**
   * @brief Attach this mesh to a_pool, or check that it is already attached to it.
   * @details Called by every reserveX(); a mesh's storage must come from exactly one Pool.
   * @param[in] a_pool Pool being reserved from.
   */
  EBGEOMETRY_HOST
  inline void
  attachTo(const Pool& a_pool) noexcept;

  /**
   * @brief Function which computes internal things for the polygon faces.
   * @note This calls DCEL::FaceT<T, Meta>::reconcile()
   */
  EBGEOMETRY_HOST
  inline void
  reconcileFaces() noexcept;

  /**
   * @brief Function which computes internal things for the half-edges
   * @note This calls DCEL::EdgeT<T, Meta>::reconcile()
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  reconcileEdges() noexcept;

  /**
   * @brief Function which computes internal things for the vertices
   * @param[in] a_weight Vertex angle weighting
   * @note This calls DCEL::VertexT<T, Meta>::computeVertexNormalAverage() or
   * DCEL::VertexT<T, Meta>::computeVertexNormalAngleWeighted()
   */
  EBGEOMETRY_HOST
  inline void
  reconcileVertices(const DCEL::VertexNormalWeight a_weight) noexcept;

  /**
   * @brief Flip all face normals
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  flipFaceNormals() noexcept;

  /**
   * @brief Flip all edge normals
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  flipEdgeNormals() noexcept;

  /**
   * @brief Flip all vertex normals
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  flipVertexNormals() noexcept;

  /**
   * @brief Implementation of signed distance function which iterates through all
   * faces
   * @param[in] a_point 3D point
   * @return Signed distance to the nearest face.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  DirectSignedDistance(const Vec3& a_point) const noexcept;

  /**
   * @brief Implementation of squared signed distance function which iterates
   * through all faces.
   * @details This first find the face with the smallest unsigned square
   * distance, and the returns the signed distance to that face (more efficient
   * than the other version).
   * @param[in] a_point 3D point
   * @return Signed distance to the nearest face.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  DirectSignedDistance2(const Vec3& a_point) const noexcept;

  /**
   * @brief Increment a warning. This is used in sanityCheck() for locating holes
   * or bad inputs in the mesh.
   * @param[in] a_warnings Map of all registered warnings
   * @param[in] a_warn     Current warning to increment by
   */
  EBGEOMETRY_HOST
  inline void
  incrementWarning(std::map<std::string, size_t>& a_warnings, const std::string& a_warn) const;

  /**
   * @brief Print all warnings to std::cerr
   * @param[in] a_warnings List of warnings (generated by sanityCheck)
   * @param[in] a_id Identifier used when printing warnings (can be empty string)
   */
  EBGEOMETRY_HOST
  inline void
  printWarnings(const std::map<std::string, size_t>& a_warnings, const std::string& a_id) const;
};
} // namespace DCEL

} // namespace EBGeometry

#include "EBGeometry_DCEL_MeshImplem.hpp"

#endif
