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
 * search-algorithm enum, and a resolved base pointer) is a plain value, so
 * MeshT is trivially copyable -- a bytewise copy is meaningful in any address
 * space, since it never needs to dereference anything beyond its own bytes.
 * This is what makes a mesh built and reconciled on the host mirror-safe: its
 * Pool can be mirrored to a device, and the mesh re-bound (see bind())
 * against the mirrored Pool's device base, with zero pointer patching inside
 * the mesh itself.
 * @details Two families of accessors resolve a PODVector offset into an
 * actual vertex/edge/face:
 * - An explicit-base overload (e.g. getVertex(void* a_base, uint32_t)),
 *   which resolves fresh against whatever base is passed in. Valid even
 *   against a Pool that is still being built into (mid-construction), as
 *   long as the base passed in is the Pool's *current* base at the time of
 *   the call -- this is what Soup/Parser use while building and reconciling
 *   a mesh, since the mesh cannot yet be bind()'d (its Pool may still be
 *   open for more meshes, and may not even be frozen).
 * - A no-argument overload (e.g. getVertex(uint32_t)), which resolves
 *   against a base cached once via bind(). This is the ergonomic path for
 *   querying a finished mesh -- see bind() for the contract.
 * Build-phase mutators (reserveVertices/Edges/Faces, addVertex/Edge/Face)
 * always take an explicit Pool&, since reserving can grow (and move) the
 * Pool's block; there is no cached-base convenience form for these, since
 * building only happens on the host, with a live Pool in hand.
 * @note Everything that resolves purely through m_base and the PODVectors (getVertex/getEdge/
 * getFace, numVertices/numEdges/numFaces, boundView) is annotated EBGEOMETRY_HOST_DEVICE.
 * Everything that touches a Pool directly (bind, deepCopy, reserveVertices/Edges/Faces,
 * addVertex/Edge/Face), returns a host container (getAllVertexCoordinates), or delegates to a
 * VertexT/EdgeT/FaceT method (reconcile, flip, sanityCheck, signedDistance, unsignedDistance2 --
 * none of which are themselves device-annotated yet) is EBGEOMETRY_HOST for now; that set becomes
 * EBGEOMETRY_HOST_DEVICE once VertexT/EdgeT/FaceT are annotated in turn.
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
   * @brief Default constructor. Leaves the mesh empty (no vertices, edges, faces) and unbound (see
   * bind()).
   * @details No Pool is needed at construction -- every build-phase method (reserveX/addX) takes
   * its Pool explicitly. Use reserveVertices()/reserveEdges()/reserveFaces() followed by
   * addVertex()/addEdge()/addFace() to populate the mesh (a file parser normally does this), then
   * bind() it to a frozen Pool for ergonomic, no-argument querying.
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
   * @brief Bind this mesh to a frozen Pool's base, enabling the no-argument query overloads
   * (getVertex(i), signedDistance(p), ...).
   * @details The bound base must remain valid (and unchanged) for as long as it is relied on: this
   * requires a_pool to be frozen (EBGEOMETRY_EXPECT-checked), since Pool::reserve() can grow (move)
   * the block, which would silently invalidate a base cached before the pool stopped changing. Safe
   * to call again later (e.g. to re-bind against a mirrored Pool's device base before transporting
   * this mesh to a device) -- rebinding is just overwriting one plain pointer value.
   * @param[in] a_pool Frozen Pool this mesh's vertex/edge/face storage was reserved from (directly,
   * or via the same Pool a source mesh was deepCopy()'d from).
   */
  EBGEOMETRY_HOST
  inline void
  bind(const Pool& a_pool) noexcept;

  /**
   * @brief Create an independent, fully-decoupled copy of this mesh's data in another Pool.
   * @details Reserves fresh storage in a_dstPool and copies every vertex/edge/face by value; since
   * every cross-reference is an index rather than a pointer, the copy is already fully independent
   * and correctly linked -- no relinking pass is needed. Every field is preserved exactly (position,
   * normal vector, centroid, area, meta-data, search algorithm) rather than recomputed, so a mesh
   * that has had flipNormal()/flip() called on it and not yet been re-reconciled is copied
   * faithfully rather than silently "un-flipped". The returned mesh is unbound (see bind()); unlike
   * the copy/move constructors, this creates genuinely separate storage rather than sharing the
   * source's.
   * @param[in]     a_srcBase Base to resolve this mesh's own data against while reading it (this
   * mesh's bound base if already bind()'d, or the building Pool's current base otherwise).
   * @param[in,out] a_dstPool Pool to reserve the copy's storage from. May be the same Pool this
   * mesh's data lives in, or a different one.
   * @return A new mesh, independent of this one.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline std::shared_ptr<Mesh>
  deepCopy(const void* a_srcBase, Pool& a_dstPool) const;

  /**
   * @brief Create an independent, fully-decoupled copy of this mesh's data in another Pool, using
   * this mesh's bound base (see bind()) to read from.
   * @param[in,out] a_dstPool Pool to reserve the copy's storage from.
   * @return A new mesh, independent of this one.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline std::shared_ptr<Mesh>
  deepCopy(Pool& a_dstPool) const;

  /**
   * @brief Perform a sanity check, resolving against an explicitly-supplied base.
   * @details This will provide error messages if vertices are badly linked,
   * faces have no half-edge, and so on. These messages are logged by calling
   * incrementWarning() which identifies types of errors that can occur, and how
   * many of those errors have occurred.
   * @param[in] a_base Base to resolve this mesh's data against.
   * @param[in] a_id   Identifier when printing error messages (can be empty string).
   */
  EBGEOMETRY_HOST
  inline void
  sanityCheck(const void* a_base, const std::string a_id) const;

  /**
   * @brief Perform a sanity check, resolving against this mesh's bound base (see bind()).
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
   * faces, resolving against an explicitly-supplied base.
   * @details Computing the signed distance to faces requires testing if a point
   * projected to a polygo face plane falls inside or outside the polygon face.
   * There are multiple algorithms to use here.
   * @param[in] a_base      Base to resolve this mesh's data against.
   * @param[in] a_algorithm Algorithm to use
   */
  EBGEOMETRY_HOST
  inline void
  setInsideOutsideAlgorithm(void* a_base, InsideOutsideAlgorithm a_algorithm) noexcept;

  /**
   * @brief Set the inside/outside algorithm to use when computing the signed distance to polygon
   * faces, resolving against this mesh's bound base (see bind()).
   * @param[in] a_algorithm Algorithm to use
   */
  EBGEOMETRY_HOST
  inline void
  setInsideOutsideAlgorithm(InsideOutsideAlgorithm a_algorithm) noexcept;

  /**
   * @brief Reconcile function which computes the internal parameters in vertices, edges, and faces
   * for use with signed distance functionality, resolving against an explicitly-supplied base.
   * @param[in] a_base   Base to resolve this mesh's data against.
   * @param[in] a_weight Vertex angle weighting function. Either
   * VertexNormalWeight::None for unweighted vertex normals or
   * VertexNormalWeight::Angle for the pseudonormal
   * @details This will reconcile faces, edges, and vertices, e.g. computing the
   * area and normal vector for faces. This is the overload Soup/Parser use internally while
   * building a mesh, since the mesh is not yet bind()'able at that point (see the class-level note).
   */
  EBGEOMETRY_HOST
  inline void
  reconcile(void* a_base, const DCEL::VertexNormalWeight a_weight = DCEL::VertexNormalWeight::Angle) noexcept;

  /**
   * @brief Reconcile function which computes the internal parameters in vertices, edges, and faces
   * for use with signed distance functionality, resolving against this mesh's bound base (see
   * bind()).
   * @param[in] a_weight Vertex angle weighting function. Either
   * VertexNormalWeight::None for unweighted vertex normals or
   * VertexNormalWeight::Angle for the pseudonormal
   */
  EBGEOMETRY_HOST
  inline void
  reconcile(const DCEL::VertexNormalWeight a_weight = DCEL::VertexNormalWeight::Angle) noexcept;

  /**
   * @brief Flip the mesh, making all the normals change direction, resolving against an
   * explicitly-supplied base.
   * @param[in] a_base Base to resolve this mesh's data against.
   * @note Should be called AFTER all normals have been computed.
   */
  EBGEOMETRY_HOST
  inline void
  flip(void* a_base) noexcept;

  /**
   * @brief Flip the mesh, making all the normals change direction, resolving against this mesh's
   * bound base (see bind()).
   * @note Should be called AFTER all normals have been computed.
   */
  EBGEOMETRY_HOST
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
   * @brief Get modifiable vertex by index, resolving against an explicitly-supplied base.
   * @param[in] a_base  Base to resolve this mesh's data against.
   * @param[in] a_index Vertex index (must be < numVertices()).
   * @return Reference to the vertex.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Vertex&
  getVertex(void* a_base, uint32_t a_index) noexcept;

  /**
   * @brief Get immutable vertex by index, resolving against an explicitly-supplied base.
   * @param[in] a_base  Base to resolve this mesh's data against.
   * @param[in] a_index Vertex index (must be < numVertices()).
   * @return Const reference to the vertex.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const Vertex&
  getVertex(const void* a_base, uint32_t a_index) const noexcept;

  /**
   * @brief Get modifiable vertex by index, resolving against this mesh's bound base (see bind()).
   * @param[in] a_index Vertex index (must be < numVertices()).
   * @return Reference to the vertex.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Vertex&
  getVertex(uint32_t a_index) noexcept;

  /**
   * @brief Get immutable vertex by index, resolving against this mesh's bound base (see bind()).
   * @param[in] a_index Vertex index (must be < numVertices()).
   * @return Const reference to the vertex.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const Vertex&
  getVertex(uint32_t a_index) const noexcept;

  /**
   * @brief Get modifiable half-edge by index, resolving against an explicitly-supplied base.
   * @param[in] a_base  Base to resolve this mesh's data against.
   * @param[in] a_index Half-edge index (must be < numEdges()).
   * @return Reference to the half-edge.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Edge&
  getEdge(void* a_base, uint32_t a_index) noexcept;

  /**
   * @brief Get immutable half-edge by index, resolving against an explicitly-supplied base.
   * @param[in] a_base  Base to resolve this mesh's data against.
   * @param[in] a_index Half-edge index (must be < numEdges()).
   * @return Const reference to the half-edge.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const Edge&
  getEdge(const void* a_base, uint32_t a_index) const noexcept;

  /**
   * @brief Get modifiable half-edge by index, resolving against this mesh's bound base (see
   * bind()).
   * @param[in] a_index Half-edge index (must be < numEdges()).
   * @return Reference to the half-edge.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Edge&
  getEdge(uint32_t a_index) noexcept;

  /**
   * @brief Get immutable half-edge by index, resolving against this mesh's bound base (see bind()).
   * @param[in] a_index Half-edge index (must be < numEdges()).
   * @return Const reference to the half-edge.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const Edge&
  getEdge(uint32_t a_index) const noexcept;

  /**
   * @brief Get modifiable face by index, resolving against an explicitly-supplied base.
   * @param[in] a_base  Base to resolve this mesh's data against.
   * @param[in] a_index Face index (must be < numFaces()).
   * @return Reference to the face.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Face&
  getFace(void* a_base, uint32_t a_index) noexcept;

  /**
   * @brief Get immutable face by index, resolving against an explicitly-supplied base.
   * @param[in] a_base  Base to resolve this mesh's data against.
   * @param[in] a_index Face index (must be < numFaces()).
   * @return Const reference to the face.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const Face&
  getFace(const void* a_base, uint32_t a_index) const noexcept;

  /**
   * @brief Get modifiable face by index, resolving against this mesh's bound base (see bind()).
   * @param[in] a_index Face index (must be < numFaces()).
   * @return Reference to the face.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Face&
  getFace(uint32_t a_index) noexcept;

  /**
   * @brief Get immutable face by index, resolving against this mesh's bound base (see bind()).
   * @param[in] a_index Face index (must be < numFaces()).
   * @return Const reference to the face.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const Face&
  getFace(uint32_t a_index) const noexcept;

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
   * @brief Return all vertex coordinates in the mesh, resolving against an explicitly-supplied
   * base.
   * @param[in] a_base Base to resolve this mesh's data against.
   * @return Vector of 3D coordinates of all vertices.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline std::vector<Vec3T<T>>
  getAllVertexCoordinates(const void* a_base) const noexcept;

  /**
   * @brief Return all vertex coordinates in the mesh, resolving against this mesh's bound base (see
   * bind()).
   * @return Vector of 3D coordinates of all vertices.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline std::vector<Vec3T<T>>
  getAllVertexCoordinates() const noexcept;

  /**
   * @brief Compute the signed distance from a point to this mesh, resolving against an
   * explicitly-supplied base.
   * @param[in] a_base Base to resolve this mesh's data against.
   * @param[in] a_x0   3D point in space.
   * @details This function will iterate through ALL faces in the mesh and return
   * the value with the smallest magnitude. This is horrendously slow, which is
   * why this function is almost never called. Rather, MeshT<T, Meta> can be embedded
   * in a bounding volume hierarchy for faster access.
   * @note This will call the other version with the object's search algorithm.
   * @return Signed distance to the mesh; negative inside, positive outside. Returns +infinity if
   * the mesh has no faces.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline T
  signedDistance(const void* a_base, const Vec3& a_x0) const noexcept;

  /**
   * @brief Compute the signed distance from a point to this mesh, resolving against this mesh's
   * bound base (see bind()).
   * @param[in] a_x0 3D point in space.
   * @return Signed distance to the mesh; negative inside, positive outside. Returns +infinity if
   * the mesh has no faces.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline T
  signedDistance(const Vec3& a_x0) const noexcept;

  /**
   * @brief Compute the signed distance from a point to this mesh, resolving against an
   * explicitly-supplied base.
   * @param[in] a_base      Base to resolve this mesh's data against.
   * @param[in] a_x0        3D point in space.
   * @param[in] a_algorithm Search algorithm
   * @details This function will iterate through ALL faces in the mesh and return
   * the value with the smallest magnitude. This is horrendously slow, which is
   * why this function is almost never called. Rather, MeshT<T, Meta> can be embedded
   * in a bounding volume hierarchy for faster access.
   * @return Signed distance to the mesh; negative inside, positive outside. Returns +infinity if
   * the mesh has no faces.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline T
  signedDistance(const void* a_base, const Vec3& a_x0, SearchAlgorithm a_algorithm) const noexcept;

  /**
   * @brief Compute the signed distance from a point to this mesh, resolving against this mesh's
   * bound base (see bind()).
   * @param[in] a_x0        3D point in space.
   * @param[in] a_algorithm Search algorithm
   * @return Signed distance to the mesh; negative inside, positive outside. Returns +infinity if
   * the mesh has no faces.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline T
  signedDistance(const Vec3& a_x0, SearchAlgorithm a_algorithm) const noexcept;

  /**
   * @brief Compute the unsigned square distance from a point to this mesh, resolving against an
   * explicitly-supplied base.
   * @param[in] a_base Base to resolve this mesh's data against.
   * @param[in] a_x0   3D point in space.
   * @details This function will iterate through ALL faces in the mesh and return
   * the value with the smallest magnitude. This is horrendously slow, which is
   * why this function is almost never called. Rather, MeshT<T, Meta> can be embedded
   * in a bounding volume hierarchy for faster access.
   * @return Squared unsigned distance to the nearest face, or +infinity if the mesh has no faces.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline T
  unsignedDistance2(const void* a_base, const Vec3& a_x0) const noexcept;

  /**
   * @brief Compute the unsigned square distance from a point to this mesh, resolving against this
   * mesh's bound base (see bind()).
   * @param[in] a_x0 3D point in space.
   * @return Squared unsigned distance to the nearest face, or +infinity if the mesh has no faces.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline T
  unsignedDistance2(const Vec3& a_x0) const noexcept;

  /**
   * @brief Build a lightweight, stack-only, read-resolving view of this mesh bound to an
   * explicitly-supplied base.
   * @details VertexT/EdgeT/FaceT's `const Mesh&`-taking methods (reconcile(), gatherVertexIndices(),
   * getNextEdge(), computeVertexNormalAverage(), ...) always resolve indices through the mesh's own
   * no-argument (bound) accessors, so *this cannot be passed to them directly unless it is already
   * bind()'d against the base being resolved with. boundView() bridges this: a disposable copy of
   * this mesh's descriptor (all plain values, so trivially cheap) with m_base set to a_base, safe to
   * pass wherever a bound `const Mesh&` is required before the real mesh can be bound (e.g.
   * Soup/Parser building a mesh whose Pool isn't frozen yet, or TriMeshSDF's mesh-based constructor,
   * which deliberately never binds/freezes -- see EBGeometry_MeshDistanceFunctions.hpp).
   * @param[in] a_base Base to bind the returned view to.
   * @return A mesh view sharing this mesh's data but bound to a_base.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Mesh
  boundView(const void* a_base) const noexcept;

protected:
  /**
   * @brief Resolved base pointer, set via bind(). Null until bound.
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
   * @brief Function which computes internal things for the polygon faces.
   * @param[in] a_base Base to resolve this mesh's data against.
   * @note This calls DCEL::FaceT<T, Meta>::reconcile()
   */
  EBGEOMETRY_HOST
  inline void
  reconcileFaces(void* a_base) noexcept;

  /**
   * @brief Function which computes internal things for the half-edges
   * @param[in] a_base Base to resolve this mesh's data against.
   * @note This calls DCEL::EdgeT<T, Meta>::reconcile()
   */
  EBGEOMETRY_HOST
  inline void
  reconcileEdges(void* a_base) noexcept;

  /**
   * @brief Function which computes internal things for the vertices
   * @param[in] a_base   Base to resolve this mesh's data against.
   * @param[in] a_weight Vertex angle weighting
   * @note This calls DCEL::VertexT<T, Meta>::computeVertexNormalAverage() or
   * DCEL::VertexT<T, Meta>::computeVertexNormalAngleWeighted()
   */
  EBGEOMETRY_HOST
  inline void
  reconcileVertices(void* a_base, const DCEL::VertexNormalWeight a_weight) noexcept;

  /**
   * @brief Flip all face normals
   * @param[in] a_base Base to resolve this mesh's data against.
   */
  EBGEOMETRY_HOST
  inline void
  flipFaceNormals(void* a_base) noexcept;

  /**
   * @brief Flip all edge normals
   * @param[in] a_base Base to resolve this mesh's data against.
   */
  EBGEOMETRY_HOST
  inline void
  flipEdgeNormals(void* a_base) noexcept;

  /**
   * @brief Flip all vertex normals
   * @param[in] a_base Base to resolve this mesh's data against.
   */
  EBGEOMETRY_HOST
  inline void
  flipVertexNormals(void* a_base) noexcept;

  /**
   * @brief Implementation of signed distance function which iterates through all
   * faces
   * @param[in] a_base  Base to resolve this mesh's data against.
   * @param[in] a_point 3D point
   * @return Signed distance to the nearest face.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline T
  DirectSignedDistance(const void* a_base, const Vec3& a_point) const noexcept;

  /**
   * @brief Implementation of squared signed distance function which iterates
   * through all faces.
   * @details This first find the face with the smallest unsigned square
   * distance, and the returns the signed distance to that face (more efficient
   * than the other version).
   * @param[in] a_base  Base to resolve this mesh's data against.
   * @param[in] a_point 3D point
   * @return Signed distance to the nearest face.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline T
  DirectSignedDistance2(const void* a_base, const Vec3& a_point) const noexcept;

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
