// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_DCEL_Vertex.hpp
 * @brief  Declaration of a vertex class for use in DCEL descriptions of polygon
 * tessellations.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_DCEL_VERTEX_HPP
#define EBGEOMETRY_DCEL_VERTEX_HPP

// Std includes
#include <cstdint>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_DCEL.hpp"
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

namespace DCEL {

/**
 * @brief Class which represents a vertex node in a double-edge connected list
 * (DCEL).
 * @details This class is used in DCEL functionality which stores polygonal
 * surfaces in a mesh. The VertexT class has a position, a normal vector, and the
 * index of one of the outgoing edges from the vertex.
 * @note The normal vector is outgoing, i.e. a point x is "outside" the vertex if
 * the dot product between n and (x - x0) is positive.
 * @note m_outgoingEdge is an index into the owning DCEL::MeshT's own edge array,
 * resolved by passing that mesh to the accessors below (see EdgeT for why
 * indices rather than pointers are used). It is the sole topology this class
 * stores: which faces touch this vertex is not cached here -- storing a
 * face-index list per vertex was a since-removed convenience, not a
 * requirement, and it would have been the one member keeping VertexT from being
 * trivially copyable (per-vertex variable-length lists cannot be plain values).
 * Every remaining member is a plain value (Vec3, uint32_t, Meta), so as long as
 * Meta and T are trivially copyable, VertexT itself is trivially copyable.
 * computeVertexNormalAverage()/computeVertexNormalAngleWeighted() take the
 * touching-faces list as an explicit parameter instead: MeshT::reconcileVertices()
 * rebuilds it transiently by walking every face's own boundary loop (no pair
 * edges needed), which is exactly as robust to non-watertight/"dirty" meshes as
 * the old per-vertex cache was, and discards it once normals are computed.
 * m_outgoingEdge itself is unrelated to this -- it is not read by either normal
 * computation -- and remains purely a convenience for callers who want O(1)
 * access to some edge leaving this vertex.
 * @tparam T    Floating-point precision.
 * @tparam Meta Meta-data type stored per vertex.
 */
template <class T, class Meta>
class VertexT
{
  static_assert(std::is_floating_point_v<T>, "VertexT<T,Meta>: T must be a floating-point type");

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
   * @brief Alias for edge iterator
   */
  using EdgeIterator = EdgeIteratorT<T, Meta>;

  /**
   * @brief Default constructor.
   * @details Defaulted: the position and normal vectors zero-initialize via Vec3T's own default
   * constructor, the outgoing edge index defaults to the unset sentinel (see m_outgoingEdge), and
   * the meta-data value-initializes (see m_metaData). Not marked noexcept since Meta is an
   * unconstrained template parameter whose default constructor is not guaranteed to be noexcept.
   */
  VertexT() = default;

  /**
   * @brief Partial constructor.
   * @param[in] a_position Vertex position
   * @details This initializes the position to a_position and the normal vector
   * to the zero vector.
   */
  VertexT(const Vec3& a_position);

  /**
   * @brief Constructor.
   * @param[in] a_position    Vertex position
   * @param[in] a_normal Vertex normal vector
   * @details This initializes the position to a_position and the normal vector
   * to a_normal.
   */
  VertexT(const Vec3& a_position, const Vec3& a_normal);

  /**
   * @brief Copy constructor.
   * @param[in] a_otherVertex Other vertex.
   * @details Defaulted memberwise copy of every member -- position, normal vector, outgoing edge
   * index, and meta-data. Copying every member (rather than the narrow, metadata-excluding copy
   * this used to have) is what lets VertexT be trivially copyable: `std::is_trivially_copyable`
   * requires the copy constructor to be the implicit/defaulted one, so a user-provided body --
   * even one that does nothing but a plain memberwise copy -- would disqualify it.
   * operator=(const Vertex&) has identical semantics.
   */
  VertexT(const Vertex& a_otherVertex) = default;

  /**
   * @brief Move constructor.
   * @details Defaulted memberwise move; equivalent to the copy constructor since every member is a
   * plain value.
   * @param[in, out] a_otherVertex Other vertex.
   */
  VertexT(Vertex&& a_otherVertex) = default;

  /**
   * @brief Destructor (does nothing)
   */
  ~VertexT() = default;

  /**
   * @brief Copy assignment operator.
   * @details Defaulted memberwise copy; see the copy constructor's documentation.
   * @param[in] a_otherVertex Other vertex.
   * @return Reference to (*this).
   */
  Vertex&
  operator=(const Vertex& a_otherVertex) = default;

  /**
   * @brief Move assignment operator.
   * @details Has the same full-state-transfer semantics as the move constructor (defaulted
   * memberwise move; see its documentation for why this must be defaulted explicitly).
   * @param[in, out] a_otherVertex Other vertex.
   * @return Reference to (*this).
   */
  Vertex&
  operator=(Vertex&& a_otherVertex) = default;

  /**
   * @brief Define function
   * @param[in] a_position    Vertex position
   * @param[in] a_edgeIndex   Index of the outgoing edge in the owning mesh's edge array, or
   * UINT32_MAX if not yet wired into a mesh.
   * @param[in] a_normal Vertex normal vector
   * @details This sets the position, normal vector, and outgoing edge index.
   */
  inline void
  define(const Vec3& a_position, const uint32_t a_edgeIndex, const Vec3& a_normal) noexcept;

  /**
   * @brief Set the vertex position
   * @param[in] a_position Vertex position
   */
  inline void
  setPosition(const Vec3& a_position) noexcept;

  /**
   * @brief Set the vertex normal vector
   * @param[in] a_normal Vertex normal vector
   */
  inline void
  setNormal(const Vec3& a_normal) noexcept;

  /**
   * @brief Set the index of the outgoing edge.
   * @param[in] a_edgeIndex Index of an outgoing edge in the owning mesh's edge array, or
   * UINT32_MAX to mark it unset.
   */
  inline void
  setEdge(const uint32_t a_edgeIndex) noexcept;

  /**
   * @brief Set the meta-data.
   * @param[in] a_metaData Meta-data.
   */
  inline void
  setMetaData(const Meta& a_metaData) noexcept;

  /**
   * @brief Normalize the normal vector, ensuring its length is 1
   * @details The current normal must not be (near-)zero-length; a zero-length normal cannot be
   * normalized and is left unchanged (a no-op) rather than dividing by zero.
   */
  inline void
  normalizeNormalVector() noexcept;

  /**
   * @brief Compute the vertex normal, as an unweighted average of the normal vectors of the faces
   * touching this vertex.
   * @param[in] a_faceIndices Indices, into the owning mesh's face array, of every face touching
   * this vertex. Not cached anywhere -- callers (see MeshT::reconcileVertices()) discover this by
   * walking each candidate face's own boundary loop, which needs no pair edges and so works
   * regardless of whether the mesh is watertight.
   * @param[in] a_mesh Owning mesh, used to resolve a_faceIndices to actual faces.
   * @note This computes the vertex normal as n = sum(normal(face))/num(faces).
   */
  inline void
  computeVertexNormalAverage(const std::vector<uint32_t>& a_faceIndices, const Mesh& a_mesh) noexcept;

  /**
   * @brief Compute the vertex normal, using the pseudonormal algorithm which
   * weights the normal with the subtended angle to each connected face.
   * @param[in] a_thisVertexIndex This vertex's own index in the owning mesh's vertex array, used to
   * identify which of a face's vertices is "this" one while walking that face's boundary.
   * @param[in] a_faceIndices Indices, into the owning mesh's face array, of every face touching
   * this vertex (see computeVertexNormalAverage() for how these are found).
   * @param[in] a_mesh Owning mesh, used to resolve a_faceIndices and a_thisVertexIndex.
   * @note This computes the normal vector using the pseudnormal algorithm from
   * Baerentzen and Aanes in "Signed distance computation using the angle
   * weighted pseudonormal" (DOI: 10.1109/TVCG.2005.49).
   */
  inline void
  computeVertexNormalAngleWeighted(const uint32_t               a_thisVertexIndex,
                                   const std::vector<uint32_t>& a_faceIndices,
                                   const Mesh&                  a_mesh);

  /**
   * @brief Flip the normal vector
   */
  inline void
  flipNormal() noexcept;

  /**
   * @brief Return modifiable vertex position.
   * @return Reference to m_position.
   */
  [[nodiscard]] inline Vec3T<T>&
  getPosition() noexcept;

  /**
   * @brief Return immutable vertex position.
   * @return Const reference to m_position.
   */
  [[nodiscard]] inline const Vec3T<T>&
  getPosition() const noexcept;

  /**
   * @brief Return modifiable vertex normal vector.
   * @return Reference to m_normal.
   */
  [[nodiscard]] inline Vec3T<T>&
  getNormal() noexcept;

  /**
   * @brief Return immutable vertex normal vector.
   * @return Const reference to m_normal.
   */
  [[nodiscard]] inline const Vec3T<T>&
  getNormal() const noexcept;

  /**
   * @brief Get the index of the outgoing edge.
   * @return Index of the outgoing edge in the owning mesh's edge array, or UINT32_MAX if unset.
   */
  [[nodiscard]] inline uint32_t
  getOutgoingEdgeIndex() const noexcept;

  /**
   * @brief Get the outgoing edge.
   * @param[in] a_mesh Owning mesh, used to resolve the outgoing edge index.
   * @return Reference to the outgoing edge. m_outgoingEdge must be set (see getOutgoingEdgeIndex()).
   */
  [[nodiscard]] inline Edge&
  getOutgoingEdge(Mesh& a_mesh) noexcept;

  /**
   * @brief Get the outgoing edge (const overload).
   * @param[in] a_mesh Owning mesh, used to resolve the outgoing edge index.
   * @return Const reference to the outgoing edge. m_outgoingEdge must be set (see
   * getOutgoingEdgeIndex()).
   */
  [[nodiscard]] inline const Edge&
  getOutgoingEdge(const Mesh& a_mesh) const noexcept;

  /**
   * @brief Get the signed distance to this vertex
   * @param[in] a_x0 Position in space.
   * @return The returned distance is |a_x0 - m_position| and the sign is given
   * by the sign of m_normal * |a_x0 - m_position|.
   */
  [[nodiscard]] inline T
  signedDistance(const Vec3& a_x0) const noexcept;

  /**
   * @brief Get the squared unsigned distance to this vertex.
   * @details This is faster to compute than signedDistance, and might be
   * preferred for some algorithms.
   * @param[in] a_x0 Query position.
   * @return Squared Euclidean distance |a_x0 - m_position|^2.
   */
  [[nodiscard]] inline T
  unsignedDistance2(const Vec3& a_x0) const noexcept;

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

protected:
  /**
   * @brief Index of an outgoing edge from this vertex.
   * @details Index into the owning DCEL::MeshT's edge array, or UINT32_MAX if unset. This is also
   * the seed half-edge for circulating the faces touching this vertex (see
   * computeVertexNormalAverage()/computeVertexNormalAngleWeighted()).
   */
  uint32_t m_outgoingEdge = UINT32_MAX;

  /**
   * @brief Vertex position
   */
  Vec3 m_position;

  /**
   * @brief Vertex normal vector
   */
  Vec3 m_normal;

  /**
   * @brief Meta-data for this vertex
   * @details Value-initialized so that every constructor leaves it in a defined state: for a
   * fundamental Meta type (e.g. short, int), a member with no initializer and no explicit
   * mention in a constructor's member-initializer list is left indeterminate, not zero.
   */
  Meta m_metaData{};
};

static_assert(std::is_trivially_copyable_v<VertexT<float, DefaultMetaData>>,
              "VertexT<float,DefaultMetaData> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<VertexT<double, DefaultMetaData>>,
              "VertexT<double,DefaultMetaData> must be trivially copyable");

} // namespace DCEL

} // namespace EBGeometry

#include "EBGeometry_DCEL_VertexImplem.hpp"

#endif
