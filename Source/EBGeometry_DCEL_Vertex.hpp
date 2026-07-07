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
#include <memory>
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
 * surfaces in a mesh. The VertexT class has a position, a normal vector, and a
 * pointer to one of the outgoing edges from the vertex. For performance reasons
 * we also include pointers to all the polygon faces that share this vertex.
 * @note The normal vector is outgoing, i.e. a point x is "outside" the vertex if
 * the dot product between n and (x - x0) is positive.
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
   * @brief Alias for vertex pointer type
   */
  using VertexPtr = std::shared_ptr<Vertex>;

  /**
   * @brief Alias for edge pointer type
   */
  using EdgePtr = std::shared_ptr<Edge>;

  /**
   * @brief Alias for face pointer type
   */
  using FacePtr = std::shared_ptr<Face>;

  /**
   * @brief Alias for edge iterator
   */
  using EdgeIterator = EdgeIteratorT<T, Meta>;

  /**
   * @brief Default constructor.
   * @details Defaulted: the position and normal vectors zero-initialize via Vec3T's own default
   * constructor, the outgoing edge pointer defaults to null, the face list defaults to empty, and
   * the meta-data value-initializes (see m_metaData). Not marked noexcept since Meta is an
   * unconstrained template parameter whose default constructor is not guaranteed to be noexcept.
   */
  VertexT() = default;

  /**
   * @brief Partial constructor.
   * @param[in] a_position Vertex position
   * @details This initializes the position to a_position and the normal vector
   * to the zero vector. The polygon face list is empty.
   */
  VertexT(const Vec3& a_position);

  /**
   * @brief Constructor.
   * @param[in] a_position    Vertex position
   * @param[in] a_normal Vertex normal vector
   * @details This initializes the position to a_position and the normal vector
   * to a_normal. The polygon face list is empty.
   */
  VertexT(const Vec3& a_position, const Vec3& a_normal);

  /**
   * @brief Copy constructor.
   * @param[in] a_otherVertex Other vertex.
   * @details Copies only the position, normal vector, and outgoing edge pointer from the other
   * vertex. The polygon face list (m_faces) and meta-data (m_metaData) are deliberately NOT
   * copied -- they default-construct empty/default instead; use addFace()/setMetaData()
   * afterwards if they need to be populated. Rationale: m_faces and m_outgoingEdge are
   * back-references into a specific mesh's topology (the faces/edges they point to still
   * reference the original vertex, not this copy), so copying them wholesale would not be
   * topologically meaningful; only the geometric value (position, normal) survives a copy.
   * operator=(const Vertex&) has identical semantics.
   */
  VertexT(const Vertex& a_otherVertex);

  /**
   * @brief Move constructor.
   * @details Unlike the copy constructor, this transfers the entire state (including m_faces and
   * m_metaData) from a_otherVertex, since moving relocates a single object's identity rather than
   * grafting a copy into a different mesh's topology. Defaulted (memberwise move) rather than
   * hand-written: explicitly defaulting is required here since the presence of a user-declared
   * copy constructor would otherwise suppress the implicitly-generated move constructor. Not
   * marked noexcept since Meta is an unconstrained template parameter whose move constructor is
   * not guaranteed to be noexcept.
   * @param[in, out] a_otherVertex Other vertex.
   */
  VertexT(Vertex&& a_otherVertex) = default;

  /**
   * @brief Destructor (does nothing)
   */
  ~VertexT() = default;

  /**
   * @brief Copy assignment operator.
   * @details Has the same narrow-copy semantics as the copy constructor (including that m_faces
   * and m_metaData are not copied; use addFace()/setMetaData() afterwards if they need to be
   * populated). See the copy constructor's documentation for details.
   * @param[in] a_otherVertex Other vertex.
   * @return Reference to (*this).
   */
  Vertex&
  operator=(const Vertex& a_otherVertex);

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
   * @param[in] a_edge   Pointer to outgoing edge
   * @param[in] a_normal Vertex normal vector
   * @details This sets the position, normal vector, and edge pointer.
   */
  inline void
  define(const Vec3& a_position, const EdgePtr& a_edge, const Vec3& a_normal) noexcept;

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
   * @brief Set the reference to the outgoing edge
   * @param[in] a_edge Pointer to an outgoing edge
   */
  inline void
  setEdge(const EdgePtr& a_edge) noexcept;

  /**
   * @brief Set the meta-data.
   * @param[in] a_metaData Meta-data.
   */
  inline void
  setMetaData(const Meta& a_metaData) noexcept;

  /**
   * @brief Add a face to the polygon face list.
   * @param[in] a_face Pointer to face.
   */
  inline void
  addFace(const FacePtr& a_face);

  /**
   * @brief Normalize the normal vector, ensuring its length is 1
   * @details The current normal must not be (near-)zero-length; a zero-length normal cannot be
   * normalized and is left unchanged (a no-op) rather than dividing by zero.
   */
  inline void
  normalizeNormalVector() noexcept;

  /**
   * @brief Compute the vertex normal, using an average the normal vector in this
   * vertex's face list (m_faces)
   */
  inline void
  computeVertexNormalAverage() noexcept;

  /**
   * @brief Compute the vertex normal, using an average of the normal vectors in
   * the input face list
   * @param[in] a_faces Faces
   * @note This computes the vertex normal as n = sum(normal(face))/num(faces)
   */
  inline void
  computeVertexNormalAverage(const std::vector<FacePtr>& a_faces) noexcept;

  /**
   * @brief Compute the vertex normal, using the pseudonormal algorithm which
   * weights the normal with the subtended angle to each connected face.
   * @details This calls the other version with a_faces = m_faces
   * @note This computes the normal vector using the pseudnormal algorithm from
   * Baerentzen and Aanes in "Signed distance computation using the angle
   * weighted pseudonormal" (DOI: 10.1109/TVCG.2005.49)
   */
  inline void
  computeVertexNormalAngleWeighted();

  /**
   * @brief Compute the vertex normal, using the pseudonormal algorithm which
   * weights the normal with the subtended angle to each connected face.
   * @param[in] a_faces Faces to use for computation.
   * @note This computes the normal vector using the pseudnormal algorithm from
   * Baerentzen and Aanes in "Signed distance computation using the angle
   * weighted pseudonormal" (DOI: 10.1109/TVCG.2005.49)
   */
  inline void
  computeVertexNormalAngleWeighted(const std::vector<FacePtr>& a_faces);

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
   * @brief Get the outgoing edge.
   * @details Returns a shared_ptr obtained by locking the internal weak_ptr (see the class-level
   * note on ownership near m_outgoingEdge). Returns nullptr if the edge has been destroyed, which
   * should not happen while the owning mesh is alive.
   * @return Outgoing edge, or nullptr.
   */
  [[nodiscard]] inline EdgePtr
  getOutgoingEdge() noexcept;

  /**
   * @brief Get the outgoing edge (const overload).
   * @return Outgoing edge, or nullptr.
   */
  [[nodiscard]] inline EdgePtr
  getOutgoingEdge() const noexcept;

  /**
   * @brief Get modifiable polygon face list for this vertex.
   * @return Reference to m_faces.
   */
  [[nodiscard]] inline std::vector<FacePtr>&
  getFaces() noexcept;

  /**
   * @brief Get immutable polygon face list for this vertex.
   * @return Const reference to m_faces.
   */
  [[nodiscard]] inline const std::vector<FacePtr>&
  getFaces() const noexcept;

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
   * @brief Pointer to an outgoing edge from this vertex.
   * @details Stored as a weak_ptr: the edge is owned by the mesh's own edge list, and a plain
   * shared_ptr here would form a reference cycle with EdgeT::m_vertex (and, transitively, with
   * EdgeT::m_nextEdge/m_pairEdge/m_face and FaceT::m_halfEdge) that shared_ptr's reference
   * counting can never collect.
   */
  std::weak_ptr<Edge> m_outgoingEdge;

  /**
   * @brief Vertex position
   */
  Vec3 m_position;

  /**
   * @brief Vertex normal vector
   */
  Vec3 m_normal;

  /**
   * @brief List of faces connected to this vertex.
   * @details These must be added by addFace(), and is used when computing the vertex
   * normal vector.
   */
  std::vector<FacePtr> m_faces;

  /**
   * @brief Meta-data for this vertex
   * @details Value-initialized so that every constructor leaves it in a defined state: for a
   * fundamental Meta type (e.g. short, int), a member with no initializer and no explicit
   * mention in a constructor's member-initializer list is left indeterminate, not zero.
   */
  Meta m_metaData{};
};
} // namespace DCEL

} // namespace EBGeometry

#include "EBGeometry_DCEL_VertexImplem.hpp"

#endif
