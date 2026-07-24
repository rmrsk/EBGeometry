// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_DCEL_Edge.hpp
 * @brief  Declaration of a half-edge class for use in DCEL descriptions of
 * polygon tessellations.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_DCEL_EDGE_HPP
#define EBGEOMETRY_DCEL_EDGE_HPP

// Std includes
#include <cstdint>
#include <type_traits>

// Our includes
#include "EBGeometry_DCEL.hpp"
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

namespace DCEL {

/**
 * @brief Class which represents a half-edge in a double-edge connected list
 * (DCEL).
 * @details This class is used in DCEL functionality which stores polygonal
 * surfaces in a mesh. The information contain in an EdgeT object contains the
 * necessary object for logically circulating the inside of a polygon face. This
 * means that a polygon face has a double-connected list of half-edges which
 * circulate the interior of the face. The EdgeT object is such a half-edge; it
 * represents the outgoing half-edge from a vertex, located such that it can be
 * logically represented as a half edge on the "inside" of a polygon face. It
 * stores the indices of the polygon face, vertex, and next half edge in the
 * owning mesh's arrays. It also stores the index of the "pair" half edge, i.e.
 * the corresponding half-edge on the other face that shares this edge. Since
 * this class is used with DCEL functionality and signed distance fields, this
 * class also has a signed distance function and thus a "normal vector".
 * @note The normal vector is outgoing, i.e. a point x is "outside" if the dot
 * product between n and (x - x0) is positive.
 * @note All topology indices below (m_vertex, m_pairEdge, m_nextEdge, m_face) are
 * indices into the owning DCEL::MeshT's own vertex/edge/face arrays, resolved by
 * passing that mesh to the accessors below. Using indices rather than pointers
 * (the previous weak_ptr/shared_ptr scheme) keeps EdgeT a plain, relocatable
 * value: the owning mesh's arrays are the sole storage, and every
 * cross-reference is a position within them, not an address -- so an EdgeT is
 * meaningful under a memcpy or a host-to-device mirror with no pointer patching.
 * @tparam T    Floating-point precision.
 * @tparam Meta Meta-data type stored per edge.
 */
template <class T, class Meta>
class EdgeT
{
  static_assert(std::is_floating_point_v<T>, "EdgeT<T,Meta>: T must be a floating-point type");

public:
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
   * @brief Default constructor. Sets all topology indices to the unset sentinel and vectors to
   * zero vectors.
   */
  EdgeT() noexcept = default;

  /**
   * @brief Copy constructor.
   * @details Defaulted memberwise copy of every member -- the normal vector, all four topology
   * indices (vertex, pair edge, next edge, face), and meta-data. Copying every member (rather than
   * the narrow, metadata-excluding copy this used to have) is what lets EdgeT be trivially
   * copyable: `std::is_trivially_copyable` requires the copy constructor to be the implicit/defaulted
   * one, so a user-provided body -- even one that does nothing but a plain memberwise copy --
   * would disqualify it. operator=(const Edge&) has identical semantics.
   * @param[in] a_otherEdge Other edge.
   */
  EdgeT(const Edge& a_otherEdge) = default;

  /**
   * @brief Move constructor.
   * @details Defaulted memberwise move; equivalent to the copy constructor since every member is a
   * plain value.
   * @param[in, out] a_otherEdge Other edge.
   */
  EdgeT(Edge&& a_otherEdge) = default;

  /**
   * @brief Partial constructor. Calls the default constructor but sets the
   * starting vertex.
   * @param[in] a_vertexIndex Index of the starting vertex in the owning mesh's vertex array.
   */
  EdgeT(const uint32_t a_vertexIndex) noexcept;

  /**
   * @brief Destructor (does nothing)
   */
  ~EdgeT() = default;

  /**
   * @brief Copy assignment operator.
   * @details Defaulted memberwise copy; see the copy constructor's documentation.
   * @param[in] a_otherEdge Other edge.
   * @return Reference to (*this).
   */
  Edge&
  operator=(const Edge& a_otherEdge) = default;

  /**
   * @brief Move assignment operator.
   * @details Has the same full-state-transfer semantics as the move constructor (defaulted
   * memberwise move; see its documentation for why this must be defaulted explicitly).
   * @param[in, out] a_otherEdge Other edge.
   * @return Reference to (*this).
   */
  Edge&
  operator=(Edge&& a_otherEdge) = default;

  /**
   * @brief Get size (in bytes) of this object.
   * @return Size in bytes of this edge object.
   */
  [[nodiscard]] inline size_t
  size() const noexcept;

  /**
   * @brief Define function. Sets the starting vertex, pair/next edge indices.
   * @param[in] a_vertexIndex   Index of the starting vertex in the owning mesh's vertex array.
   * @param[in] a_pairEdgeIndex Index of the pair half-edge in the owning mesh's edge array, or
   * UINT32_MAX if unset.
   * @param[in] a_nextEdgeIndex Index of the next half-edge in the owning mesh's edge array, or
   * UINT32_MAX if unset.
   */
  inline void
  define(const uint32_t a_vertexIndex, const uint32_t a_pairEdgeIndex, const uint32_t a_nextEdgeIndex) noexcept;

  /**
   * @brief Reconcile internal logic
   * @details Computes the normal vector.
   * @param[in] a_mesh Owning mesh, used to resolve the face/pair-edge indices.
   */
  inline void
  reconcile(const Mesh& a_mesh) noexcept;

  /**
   * @brief Flip surface normal
   */
  inline void
  flipNormal() noexcept;

  /**
   * @brief Set the index of the starting vertex
   * @param[in] a_vertexIndex Index of the starting vertex in the owning mesh's vertex array.
   */
  inline void
  setVertex(const uint32_t a_vertexIndex) noexcept;

  /**
   * @brief Set the index of the pair edge
   * @param[in] a_pairEdgeIndex Index of the pair edge in the owning mesh's edge array, or
   * UINT32_MAX to mark it unset.
   */
  inline void
  setPairEdge(const uint32_t a_pairEdgeIndex) noexcept;

  /**
   * @brief Set the index of the next edge
   * @param[in] a_nextEdgeIndex Index of the next edge in the owning mesh's edge array, or
   * UINT32_MAX to mark it unset.
   */
  inline void
  setNextEdge(const uint32_t a_nextEdgeIndex) noexcept;

  /**
   * @brief Set the index of this half-edge's face.
   * @param[in] a_faceIndex Index of the face in the owning mesh's face array, or UINT32_MAX to
   * mark it unset.
   */
  inline void
  setFace(const uint32_t a_faceIndex) noexcept;

  /**
   * @brief Set the meta-data.
   * @param[in] a_metaData Meta-data.
   */
  inline void
  setMetaData(const Meta& a_metaData) noexcept;

  /**
   * @brief Get the index of the starting vertex.
   * @return Index of the starting vertex in the owning mesh's vertex array, or UINT32_MAX if
   * unset.
   */
  [[nodiscard]] inline uint32_t
  getVertexIndex() const noexcept;

  /**
   * @brief Get the index of the pair edge.
   * @return Index of the pair edge in the owning mesh's edge array, or UINT32_MAX if unset.
   */
  [[nodiscard]] inline uint32_t
  getPairEdgeIndex() const noexcept;

  /**
   * @brief Get the index of the next edge.
   * @return Index of the next edge in the owning mesh's edge array, or UINT32_MAX if unset.
   */
  [[nodiscard]] inline uint32_t
  getNextEdgeIndex() const noexcept;

  /**
   * @brief Get the index of this half-edge's face.
   * @return Index of the face in the owning mesh's face array, or UINT32_MAX if unset.
   */
  [[nodiscard]] inline uint32_t
  getFaceIndex() const noexcept;

  /**
   * @brief Get the starting vertex.
   * @param[in] a_mesh Owning mesh, used to resolve the vertex index.
   * @return Reference to the starting vertex. m_vertex must be set (see getVertexIndex()).
   */
  [[nodiscard]] inline Vertex&
  getVertex(Mesh& a_mesh) noexcept;

  /**
   * @brief Get the starting vertex (const overload).
   * @param[in] a_mesh Owning mesh, used to resolve the vertex index.
   * @return Const reference to the starting vertex. m_vertex must be set (see getVertexIndex()).
   */
  [[nodiscard]] inline const Vertex&
  getVertex(const Mesh& a_mesh) const noexcept;

  /**
   * @brief Get the end vertex.
   * @param[in] a_mesh Owning mesh, used to resolve the next-edge and vertex indices.
   * @return Reference to the next half-edge's starting vertex. m_nextEdge must be set.
   */
  [[nodiscard]] inline Vertex&
  getOtherVertex(Mesh& a_mesh) noexcept;

  /**
   * @brief Get the end vertex (const overload).
   * @param[in] a_mesh Owning mesh, used to resolve the next-edge and vertex indices.
   * @return Const reference to the next half-edge's starting vertex. m_nextEdge must be set.
   */
  [[nodiscard]] inline const Vertex&
  getOtherVertex(const Mesh& a_mesh) const noexcept;

  /**
   * @brief Get the pair edge.
   * @param[in] a_mesh Owning mesh, used to resolve the pair-edge index.
   * @return Reference to the pair edge. m_pairEdge must be set (see getPairEdgeIndex()).
   */
  [[nodiscard]] inline Edge&
  getPairEdge(Mesh& a_mesh) noexcept;

  /**
   * @brief Get the pair edge (const overload).
   * @param[in] a_mesh Owning mesh, used to resolve the pair-edge index.
   * @return Const reference to the pair edge. m_pairEdge must be set (see getPairEdgeIndex()).
   */
  [[nodiscard]] inline const Edge&
  getPairEdge(const Mesh& a_mesh) const noexcept;

  /**
   * @brief Get the next edge.
   * @param[in] a_mesh Owning mesh, used to resolve the next-edge index.
   * @return Reference to the next edge. m_nextEdge must be set (see getNextEdgeIndex()).
   */
  [[nodiscard]] inline Edge&
  getNextEdge(Mesh& a_mesh) noexcept;

  /**
   * @brief Get the next edge (const overload).
   * @param[in] a_mesh Owning mesh, used to resolve the next-edge index.
   * @return Const reference to the next edge. m_nextEdge must be set (see getNextEdgeIndex()).
   */
  [[nodiscard]] inline const Edge&
  getNextEdge(const Mesh& a_mesh) const noexcept;

  /**
   * @brief Compute the normal vector as the average of the face normals on
   * both sides of this edge.
   * @param[in] a_mesh Owning mesh, used to resolve the face/pair-edge indices.
   * @return Unit normal vector for this edge.
   */
  [[nodiscard]] inline Vec3T<T>
  computeNormal(const Mesh& a_mesh) const noexcept;

  /**
   * @brief Get modifiable normal vector.
   * @return Reference to m_normal.
   */
  [[nodiscard]] inline Vec3T<T>&
  getNormal() noexcept;

  /**
   * @brief Get the stored normal vector.
   * @return Const reference to m_normal.
   */
  [[nodiscard]] inline const Vec3T<T>&
  getNormal() const noexcept;

  /**
   * @brief Get this half-edge's face.
   * @param[in] a_mesh Owning mesh, used to resolve the face index.
   * @return Reference to the owning face. m_face must be set (see getFaceIndex()).
   */
  [[nodiscard]] inline Face&
  getFace(Mesh& a_mesh) noexcept;

  /**
   * @brief Get this half-edge's face (const overload).
   * @param[in] a_mesh Owning mesh, used to resolve the face index.
   * @return Const reference to the owning face. m_face must be set (see getFaceIndex()).
   */
  [[nodiscard]] inline const Face&
  getFace(const Mesh& a_mesh) const noexcept;

  /**
   * @brief Get meta-data
   * @return m_metaData
   */
  [[nodiscard]] inline Meta&
  getMetaData() noexcept;

  /**
   * @brief Get meta-data
   * @return m_metaData
   */
  [[nodiscard]] inline const Meta&
  getMetaData() const noexcept;

  /**
   * @brief Get the signed distance from a_x0 to this half-edge.
   * @details Checks whether a_x0 projects onto the edge segment or past one of
   * the end vertices. If it projects to a vertex, the signed distance to that
   * vertex is returned. Otherwise the sign is determined from the edge normal.
   * @param[in] a_x0   Query point.
   * @param[in] a_mesh Owning mesh, used to resolve the vertex indices.
   * @return Signed distance; positive on the normal side of the edge.
   */
  [[nodiscard]] inline T
  signedDistance(const Vec3& a_x0, const Mesh& a_mesh) const noexcept;

  /**
   * @brief Get the squared unsigned distance from a_x0 to this half-edge.
   * @details Clamps the projection parameter to [0,1] so the closest point is
   * always on the segment, then returns the squared distance. Faster than
   * signedDistance().
   * @param[in] a_x0   Query point.
   * @param[in] a_mesh Owning mesh, used to resolve the vertex indices.
   * @return Squared Euclidean distance to the closest point on the edge.
   */
  [[nodiscard]] inline T
  unsignedDistance2(const Vec3& a_x0, const Mesh& a_mesh) const noexcept;

protected:
  /**
   * @brief Normal vector
   */
  Vec3 m_normal = Vec3::zeros();

  /**
   * @brief Index of the starting vertex.
   * @details Index into the owning DCEL::MeshT's vertex array, or UINT32_MAX if unset.
   */
  uint32_t m_vertex = UINT32_MAX;

  /**
   * @brief Index of the pair edge. See m_vertex for the indexing convention.
   */
  uint32_t m_pairEdge = UINT32_MAX;

  /**
   * @brief Index of the next edge. See m_vertex for the indexing convention.
   */
  uint32_t m_nextEdge = UINT32_MAX;

  /**
   * @brief Index of the enclosing polygon face, into the owning mesh's face array, or
   * UINT32_MAX if unset.
   */
  uint32_t m_face = UINT32_MAX;

  /**
   * @brief Meta-data attached to this edge
   * @details Value-initialized so that every constructor leaves it in a defined state: for a
   * fundamental Meta type (e.g. short, int), a member with no initializer and no explicit mention
   * in a constructor's member-initializer list is left indeterminate, not zero.
   */
  Meta m_metaData{};

  /**
   * @brief Returns the parametric projection of a_x0 onto this edge.
   * @details Parametrizes the edge as x(t) = x1 + (x2-x1)*t and computes t
   * such that x(t) is the closest point to a_x0. Returns t in (-inf, +inf);
   * the point is on the segment when t in [0,1].
   * @param[in] a_x0   Query point.
   * @param[in] a_mesh Owning mesh, used to resolve the vertex indices.
   * @return Projection parameter t.
   */
  [[nodiscard]] inline T
  projectPointToEdge(const Vec3& a_x0, const Mesh& a_mesh) const noexcept;

  /**
   * @brief Get the vector pointing along this edge (from start to end vertex).
   * @param[in] a_mesh Owning mesh, used to resolve the vertex indices.
   * @return x2 - x1, where x1 = getVertex(a_mesh).getPosition() and x2 =
   * getOtherVertex(a_mesh).getPosition().
   */
  [[nodiscard]] inline Vec3T<T>
  getX2X1(const Mesh& a_mesh) const noexcept;
};

static_assert(std::is_trivially_copyable_v<EdgeT<float, DefaultMetaData>>,
              "EdgeT<float,DefaultMetaData> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<EdgeT<double, DefaultMetaData>>,
              "EdgeT<double,DefaultMetaData> must be trivially copyable");

} // namespace DCEL

} // namespace EBGeometry

#include "EBGeometry_DCEL_EdgeImplem.hpp"

#endif
