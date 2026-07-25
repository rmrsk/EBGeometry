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

namespace EBGeometry {

namespace DCEL {

/**
 * @brief Mesh class which stores a full DCEL mesh (with signed distance
 * functions)
 * @details This encapsulates a full DCEL mesh, and also includes DIRECT signed
 * distance functions. The mesh consists of a set of vertices, half-edges, and
 * polygon faces, stored by value in this class's own arrays; every
 * cross-reference between them (vertex-to-outgoing-edge, edge-to-vertex/pair
 * edge/next edge/face, face-to-half-edge) is an index into these arrays rather
 * than a pointer -- see the class-level notes on VertexT/EdgeT/FaceT. The
 * signed distance functions DIRECT, which means that they go through ALL of the
 * polygon faces and compute the signed distance to them. This is extremely
 * inefficient, which is why this class is almost always embedded into a
 * bounding volume hierarchy.
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
   * @brief Default constructor.
   * @details Leaves the mesh empty (no vertices, edges, or faces) with the default search
   * algorithm; use define() or the mutable getVertices()/getEdges()/getFaces() to populate it.
   */
  MeshT() noexcept = default;

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
   * @param[in, out] a_otherMesh Other mesh.
   */
  MeshT(Mesh&& a_otherMesh) noexcept = default;

  /**
   * @brief Full constructor. This provides the faces, edges, and vertices to the
   * mesh.
   * @details Copies the three vectors directly into m_faces/m_edges/m_vertices. This only
   * associates the index-based topology already recorded on the faces/edges/vertices; internal
   * parameters like face area and normal are computed by reconcile().
   * @param[in] a_faces    Polygon faces
   * @param[in] a_edges    Half-edges
   * @param[in] a_vertices Vertices
   * @note The constructor arguments should provide a complete DCEL mesh
   * description. This is usually done through a file parser which reads a mesh
   * file format and creates the DCEL mesh structure
   */
  MeshT(std::vector<Face>& a_faces, std::vector<Edge>& a_edges, std::vector<Vertex>& a_vertices) noexcept;

  /**
   * @brief Destructor (does nothing)
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
   * @param[in, out] a_otherMesh Other mesh.
   * @return Reference to (*this).
   */
  Mesh&
  operator=(Mesh&& a_otherMesh) noexcept = default;

  /**
   * @brief Create an independent, fully-decoupled copy of this mesh.
   * @details Since copy construction/assignment are disallowed (see their documentation), this is
   * the supported way to duplicate a mesh. Because every VertexT/EdgeT/FaceT cross-reference is an
   * index into this mesh's own arrays rather than a pointer, a plain copy of the three arrays is
   * already a fully independent, correctly-linked mesh -- every index remains meaningful against
   * the copied arrays with no relinking pass required. Every field is preserved exactly (position,
   * normal vector, centroid, area, meta-data, search algorithm) rather than recomputed, so a mesh
   * that has had flipNormal()/flip() called on it and not yet been re-reconciled is copied
   * faithfully rather than silently "un-flipped".
   * @return A new mesh, independent of this one.
   */
  [[nodiscard]] inline std::shared_ptr<Mesh>
  deepCopy() const;

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
   * @brief Get modifiable vertices in this mesh
   * @return Reference to the vector of vertices.
   */
  [[nodiscard]] inline std::vector<Vertex>&
  getVertices() noexcept;

  /**
   * @brief Get immutable vertices in this mesh
   * @return Const reference to the vector of vertices.
   */
  [[nodiscard]] inline const std::vector<Vertex>&
  getVertices() const noexcept;

  /**
   * @brief Return all vertex coordinates in the mesh.
   * @return Vector of 3D coordinates of all vertices.
   */
  [[nodiscard]] inline std::vector<Vec3T<T>>
  getAllVertexCoordinates() const noexcept;

  /**
   * @brief Get modifiable half-edges in this mesh
   * @return Reference to the vector of half-edges.
   */
  [[nodiscard]] inline std::vector<Edge>&
  getEdges() noexcept;

  /**
   * @brief Get immutable half-edges in this mesh
   * @return Const reference to the vector of half-edges.
   */
  [[nodiscard]] inline const std::vector<Edge>&
  getEdges() const noexcept;

  /**
   * @brief Get modifiable faces in this mesh
   * @return Reference to the vector of polygon faces.
   */
  [[nodiscard]] inline std::vector<Face>&
  getFaces() noexcept;

  /**
   * @brief Get immutable faces in this mesh
   * @return Const reference to the vector of polygon faces.
   */
  [[nodiscard]] inline const std::vector<Face>&
  getFaces() const noexcept;

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
   * @brief Search algorithm. Only used in signed distance functions.
   */
  SearchAlgorithm m_algorithm = SearchAlgorithm::Direct2;

  /**
   * @brief Mesh vertices
   */
  std::vector<Vertex> m_vertices;

  /**
   * @brief Mesh half-edges
   */
  std::vector<Edge> m_edges;

  /**
   * @brief Mesh faces
   */
  std::vector<Face> m_faces;

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
