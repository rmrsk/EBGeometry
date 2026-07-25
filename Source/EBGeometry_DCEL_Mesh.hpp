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
#include "EBGeometry_PODVector.hpp"
#include "EBGeometry_Pool.hpp"

namespace EBGeometry {

namespace DCEL {

/**
 * @brief Mesh class which stores a full DCEL mesh (with signed distance
 * functions)
 * @details This encapsulates a full DCEL mesh, and also includes DIRECT signed
 * distance functions. The mesh consists of a set of vertices, half-edges, and
 * polygon faces, stored as PODVectors into a caller-owned, externally-supplied
 * Pool; every cross-reference between them (vertex-to-outgoing-edge,
 * edge-to-vertex/pair edge/next edge/face, face-to-half-edge) is an index into
 * these arrays rather than a pointer -- see the class-level notes on
 * VertexT/EdgeT/FaceT. The signed distance functions DIRECT, which means that
 * they go through ALL of the polygon faces and compute the signed distance to
 * them. This is extremely inefficient, which is why this class is almost
 * always embedded into a bounding volume hierarchy.
 * @note MeshT does not own the Pool it is built into -- see the constructor.
 * This is deliberate: one Pool can back many meshes (built one after another,
 * their arrays laid out contiguously in the same block), which is cheaper than
 * giving every mesh its own Pool. The caller is responsible for keeping the
 * Pool alive for at least as long as any MeshT built into it, and for its
 * lifetime otherwise (see EBGeometry_Pool.hpp -- a Pool is a pure bump
 * allocator; nothing reserved from it is individually freed).
 * @note This class is not for the light of heart -- it will almost always be
 * instantiated through a file parser which reads vertices and edges from file
 * and builds the mesh from that. Do not try to build a MeshT object yourself,
 * use file parsers!
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
   * @brief Disallowed default construction.
   * @details A MeshT always needs a backing Pool to reserve its vertex/edge/face storage from;
   * use the Pool-taking constructor.
   */
  MeshT() = delete;

  /**
   * @brief Full constructor. Associates this mesh with the Pool it will reserve its
   * vertex/edge/face storage from.
   * @details Leaves the mesh empty (no vertices, edges, or faces); use reserveVertices()/
   * reserveEdges()/reserveFaces() followed by addVertex()/addEdge()/addFace() to populate it (a
   * file parser normally does this).
   * @param[in,out] a_pool Backing Pool. Non-owning: the caller must keep it alive for at least as
   * long as this mesh (and any other mesh built into the same Pool).
   */
  explicit MeshT(Pool& a_pool) noexcept;

  /**
   * @brief Disallowed copy construction.
   * @details Copying is deliberately disallowed to keep a single supported way to duplicate a mesh
   * (deepCopy()) rather than two subtly-different ones. Moving is allowed (see the move
   * constructor), since that transfers ownership rather than duplicating it.
   * @param[in] a_otherMesh Other mesh.
   */
  MeshT(const Mesh& a_otherMesh) = delete;

  /**
   * @brief Move constructor.
   * @details Defaulted memberwise move. Every member (the Pool pointer, and the PODVector
   * descriptors) is a plain value, so this is a cheap value-copy of the descriptor rather than a
   * transfer of exclusive ownership -- unlike the old std::vector-backed MeshT, the moved-from
   * mesh is left referencing the exact same Pool data (not emptied), since the underlying storage
   * was never owned by this mesh in the first place (see the class-level note on the Pool).
   * @param[in, out] a_otherMesh Other mesh.
   */
  MeshT(Mesh&& a_otherMesh) noexcept = default;

  /**
   * @brief Destructor (does nothing; the backing Pool is not owned by this mesh)
   */
  ~MeshT() noexcept = default;

  /**
   * @brief Disallowed copy assignment.
   * @details Has the same rationale as the disallowed copy constructor; see its documentation.
   * @param[in] a_otherMesh Other mesh.
   * @return Reference to (*this).
   */
  Mesh&
  operator=(const Mesh& a_otherMesh) = delete;

  /**
   * @brief Move assignment operator.
   * @details Has the same semantics as the move constructor; see its documentation.
   * @param[in, out] a_otherMesh Other mesh.
   * @return Reference to (*this).
   */
  Mesh&
  operator=(Mesh&& a_otherMesh) noexcept = default;

  /**
   * @brief Create an independent, fully-decoupled copy of this mesh in another Pool.
   * @details Since copy construction/assignment are disallowed (see their documentation), this is
   * the supported way to duplicate a mesh. Reserves fresh storage in a_dstPool and copies every
   * vertex/edge/face by value; since every cross-reference is an index rather than a pointer, the
   * copy is already fully independent and correctly linked -- no relinking pass is needed. Every
   * field is preserved exactly (position, normal vector, centroid, area, meta-data, search
   * algorithm) rather than recomputed, so a mesh that has had flipNormal()/flip() called on it and
   * not yet been re-reconciled is copied faithfully rather than silently "un-flipped".
   * @param[in,out] a_dstPool Pool to reserve the copy's storage from. May be the same Pool this
   * mesh is built into, or a different one.
   * @return A new mesh, independent of this one.
   */
  [[nodiscard]] inline std::shared_ptr<Mesh>
  deepCopy(Pool& a_dstPool) const;

  /**
   * @brief Perform a sanity check.
   * @details This will provide error messages if vertices are badly linked,
   * faces have no half-edge, and so on. These messages are logged by calling
   * incrementWarning() which identifies types of errors that can occur, and how
   * many of those errors have occurred.
   * @param[in] a_id Identifier when printing error messages (can be empty string).
   */
  inline void
  sanityCheck(const std::string a_id) const;

  /**
   * @brief Search algorithm for direct signed distance computations
   * @param[in] a_algorithm Algorithm to use
   */
  inline void
  setSearchAlgorithm(const SearchAlgorithm a_algorithm) noexcept;

  /**
   * @brief Set the inside/outside algorithm to use when computing the signed
   * distance to polygon faces.
   * @details Computing the signed distance to faces requires testing if a point
   * projected to a polygo face plane falls inside or outside the polygon face.
   * There are multiple algorithms to use here.
   * @param[in] a_algorithm Algorithm to use
   */
  inline void
  setInsideOutsideAlgorithm(InsideOutsideAlgorithm a_algorithm) noexcept;

  /**
   * @brief Reconcile function which computes the internal parameters in
   * vertices, edges, and faces for use with signed distance functionality
   * @param[in] a_weight Vertex angle weighting function. Either
   * VertexNormalWeight::None for unweighted vertex normals or
   * VertexNormalWeight::Angle for the pseudonormal
   * @details This will reconcile faces, edges, and vertices, e.g. computing the
   * area and normal vector for faces
   */
  inline void
  reconcile(const DCEL::VertexNormalWeight a_weight = DCEL::VertexNormalWeight::Angle) noexcept;

  /**
   * @brief Flip the mesh, making all the normals change direction.
   * @note Should be called AFTER all normals have been computed.
   */
  inline void
  flip() noexcept;

  /**
   * @brief Reserve storage for a_capacity vertices.
   * @details Must be called before any addVertex() call, and only once -- PODVector never
   * reallocates once reserved (see EBGeometry_PODVector.hpp).
   * @param[in] a_capacity Number of vertex slots to reserve.
   */
  inline void
  reserveVertices(uint32_t a_capacity);

  /**
   * @brief Reserve storage for a_capacity half-edges.
   * @details Must be called before any addEdge() call, and only once -- PODVector never
   * reallocates once reserved (see EBGeometry_PODVector.hpp).
   * @param[in] a_capacity Number of half-edge slots to reserve.
   */
  inline void
  reserveEdges(uint32_t a_capacity);

  /**
   * @brief Reserve storage for a_capacity faces.
   * @details Must be called before any addFace() call, and only once -- PODVector never
   * reallocates once reserved (see EBGeometry_PODVector.hpp).
   * @param[in] a_capacity Number of face slots to reserve.
   */
  inline void
  reserveFaces(uint32_t a_capacity);

  /**
   * @brief Append a vertex into the pre-reserved capacity (see reserveVertices()).
   * @param[in] a_vertex Vertex to append.
   * @return Index of the newly-appended vertex in this mesh's vertex array.
   */
  inline uint32_t
  addVertex(const Vertex& a_vertex);

  /**
   * @brief Append a half-edge into the pre-reserved capacity (see reserveEdges()).
   * @param[in] a_edge Half-edge to append.
   * @return Index of the newly-appended half-edge in this mesh's edge array.
   */
  inline uint32_t
  addEdge(const Edge& a_edge);

  /**
   * @brief Append a face into the pre-reserved capacity (see reserveFaces()).
   * @param[in] a_face Face to append.
   * @return Index of the newly-appended face in this mesh's face array.
   */
  inline uint32_t
  addFace(const Face& a_face);

  /**
   * @brief Get modifiable vertex by index.
   * @param[in] a_index Vertex index (must be < numVertices()).
   * @return Reference to the vertex.
   */
  [[nodiscard]] inline Vertex&
  getVertex(uint32_t a_index) noexcept;

  /**
   * @brief Get immutable vertex by index.
   * @param[in] a_index Vertex index (must be < numVertices()).
   * @return Const reference to the vertex.
   */
  [[nodiscard]] inline const Vertex&
  getVertex(uint32_t a_index) const noexcept;

  /**
   * @brief Get modifiable half-edge by index.
   * @param[in] a_index Half-edge index (must be < numEdges()).
   * @return Reference to the half-edge.
   */
  [[nodiscard]] inline Edge&
  getEdge(uint32_t a_index) noexcept;

  /**
   * @brief Get immutable half-edge by index.
   * @param[in] a_index Half-edge index (must be < numEdges()).
   * @return Const reference to the half-edge.
   */
  [[nodiscard]] inline const Edge&
  getEdge(uint32_t a_index) const noexcept;

  /**
   * @brief Get modifiable face by index.
   * @param[in] a_index Face index (must be < numFaces()).
   * @return Reference to the face.
   */
  [[nodiscard]] inline Face&
  getFace(uint32_t a_index) noexcept;

  /**
   * @brief Get immutable face by index.
   * @param[in] a_index Face index (must be < numFaces()).
   * @return Const reference to the face.
   */
  [[nodiscard]] inline const Face&
  getFace(uint32_t a_index) const noexcept;

  /**
   * @brief Number of vertices in this mesh.
   * @return Vertex count.
   */
  [[nodiscard]] inline uint32_t
  numVertices() const noexcept;

  /**
   * @brief Number of half-edges in this mesh.
   * @return Half-edge count.
   */
  [[nodiscard]] inline uint32_t
  numEdges() const noexcept;

  /**
   * @brief Number of faces in this mesh.
   * @return Face count.
   */
  [[nodiscard]] inline uint32_t
  numFaces() const noexcept;

  /**
   * @brief Return all vertex coordinates in the mesh.
   * @return Vector of 3D coordinates of all vertices.
   */
  [[nodiscard]] inline std::vector<Vec3T<T>>
  getAllVertexCoordinates() const noexcept;

  /**
   * @brief Compute the signed distance from a point to this mesh
   * @param[in] a_x0 3D point in space.
   * @details This function will iterate through ALL faces in the mesh and return
   * the value with the smallest magnitude. This is horrendously slow, which is
   * why this function is almost never called. Rather, MeshT<T, Meta> can be embedded
   * in a bounding volume hierarchy for faster access.
   * @note This will call the other version with the object's search algorithm.
   * @return Signed distance to the mesh; negative inside, positive outside. Returns +infinity if
   * the mesh has no faces.
   */
  [[nodiscard]] inline T
  signedDistance(const Vec3& a_x0) const noexcept;

  /**
   * @brief Compute the signed distance from a point to this mesh
   * @param[in] a_x0        3D point in space.
   * @param[in] a_algorithm Search algorithm
   * @details This function will iterate through ALL faces in the mesh and return
   * the value with the smallest magnitude. This is horrendously slow, which is
   * why this function is almost never called. Rather, MeshT<T, Meta> can be embedded
   * in a bounding volume hierarchy for faster access.
   * @return Signed distance to the mesh; negative inside, positive outside. Returns +infinity if
   * the mesh has no faces.
   */
  [[nodiscard]] inline T
  signedDistance(const Vec3& a_x0, SearchAlgorithm a_algorithm) const noexcept;

  /**
   * @brief Compute the unsigned square distance from a point to this mesh
   * @param[in] a_x0 3D point in space.
   * @details This function will iterate through ALL faces in the mesh and return
   * the value with the smallest magnitude. This is horrendously slow, which is
   * why this function is almost never called. Rather, MeshT<T, Meta> can be embedded
   * in a bounding volume hierarchy for faster access.
   * @return Squared unsigned distance to the nearest face, or +infinity if the mesh has no faces.
   */
  [[nodiscard]] inline T
  unsignedDistance2(const Vec3& a_x0) const noexcept;

protected:
  /**
   * @brief Backing Pool. Non-owning -- see the class-level note and the constructor.
   */
  Pool* m_pool = nullptr;

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
   * @note This calls DCEL::FaceT<T, Meta>::reconcile()
   */
  inline void
  reconcileFaces() noexcept;

  /**
   * @brief Function which computes internal things for the half-edges
   * @note This calls DCEL::EdgeT<T, Meta>::reconcile()
   */
  inline void
  reconcileEdges() noexcept;

  /**
   * @brief Function which computes internal things for the vertices
   * @param[in] a_weight Vertex angle weighting
   * @note This calls DCEL::VertexT<T, Meta>::computeVertexNormalAverage() or
   * DCEL::VertexT<T, Meta>::computeVertexNormalAngleWeighted()
   */
  inline void
  reconcileVertices(const DCEL::VertexNormalWeight a_weight) noexcept;

  /**
   * @brief Flip all face normals
   */
  inline void
  flipFaceNormals() noexcept;

  /**
   * @brief Flip all edge normals
   */
  inline void
  flipEdgeNormals() noexcept;

  /**
   * @brief Flip all vertex normals
   */
  inline void
  flipVertexNormals() noexcept;

  /**
   * @brief Implementation of signed distance function which iterates through all
   * faces
   * @param[in] a_point 3D point
   * @return Signed distance to the nearest face.
   */
  [[nodiscard]] inline T
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
  [[nodiscard]] inline T
  DirectSignedDistance2(const Vec3& a_point) const noexcept;

  /**
   * @brief Increment a warning. This is used in sanityCheck() for locating holes
   * or bad inputs in the mesh.
   * @param[in] a_warnings Map of all registered warnings
   * @param[in] a_warn     Current warning to increment by
   */
  inline void
  incrementWarning(std::map<std::string, size_t>& a_warnings, const std::string& a_warn) const;

  /**
   * @brief Print all warnings to std::cerr
   * @param[in] a_warnings List of warnings (generated by sanityCheck)
   * @param[in] a_id Identifier used when printing warnings (can be empty string)
   */
  inline void
  printWarnings(const std::map<std::string, size_t>& a_warnings, const std::string& a_id) const;
};
} // namespace DCEL

} // namespace EBGeometry

#include "EBGeometry_DCEL_MeshImplem.hpp"

#endif
