// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_DCEL_Face.hpp
 * @brief  Declaration of a polygon face class for use in DCEL descriptions of
 * polygon tessellations.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_DCEL_FACE_HPP
#define EBGEOMETRY_DCEL_FACE_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_DCEL.hpp"
#include "EBGeometry_DCEL_Edge.hpp"
#include "EBGeometry_DCEL_Vertex.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

namespace DCEL {

/**
 * @brief Class which represents a polygon face in a double-edge connected list
 * (DCEL).
 * @details This class is a polygon face in a DCEL mesh. It stores the index of
 * one of the half-edges that circulate the inside of the polygon face, as well
 * as having a normal vector, a centroid, and an area. This class supports
 * signed distance computations. These computations require algorithms that
 * compute e.g. the winding number of the polygon, or the number of times a ray
 * cast passes through it. Thus, one of its central features is that it can be
 * embedded in 2D by projecting it along the cardinal direction of its normal
 * vector.
 * @note To compute the distance from a point to the face one must determine if
 * the point projects "inside" or "outside" the polygon. There are several
 * algorithms for this, and by default this class uses a crossing number
 * algorithm.
 * @note m_halfEdge is an index into the owning DCEL::MeshT's own edge array,
 * resolved by passing that mesh to the accessors below (see EdgeT for why
 * indices rather than pointers are used). Every other member is a plain value
 * (Vec3, T, uint32_t, an enum, and Meta), so as long as Meta and T are
 * trivially copyable, FaceT itself is trivially copyable -- it can be memcpy'd
 * or mirrored to a different address space with no pointer patching, as long
 * as it is interpreted against the same mesh's arrays on the other side.
 * @tparam T    Floating-point precision type.
 * @tparam Meta User-defined metadata type.
 */
template <class T, class Meta>
class FaceT
{
  static_assert(std::is_floating_point_v<T>, "FaceT requires a floating-point T");

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
   * @brief Default constructor. Sets the half-edge index to the unset sentinel and the
   * inside/outside algorithm to crossing number algorithm
   */
  FaceT() = default;

  /**
   * @brief Partial constructor. Calls default constructor but associates a
   * half-edge
   * @param[in] a_edgeIndex Index of the half-edge in the owning mesh's edge array.
   */
  FaceT(const uint32_t a_edgeIndex);

  /**
   * @brief Copy constructor.
   * @details Defaulted memberwise copy: every member is a plain value (see the class-level note),
   * so the copy is immediately usable for a point-in-face test against the same mesh the source
   * face belonged to, and carries the same meta-data as the source face.
   * @param[in] a_otherFace Other face to copy from.
   */
  FaceT(const Face& a_otherFace) = default;

  /**
   * @brief Move constructor.
   * @details Defaulted memberwise move; equivalent to the copy constructor since every member is a
   * plain value.
   * @param[in, out] a_otherFace Other face.
   */
  FaceT(Face&& a_otherFace) = default;

  /**
   * @brief Destructor (does nothing)
   */
  ~FaceT() = default;

  /**
   * @brief Copy assignment operator.
   * @details Defaulted memberwise copy; see the copy constructor's documentation.
   * @param[in] a_otherFace Other face.
   * @return Reference to (*this).
   */
  Face&
  operator=(const Face& a_otherFace) = default;

  /**
   * @brief Move assignment operator.
   * @details Defaulted memberwise move; equivalent to copy assignment since every member is a
   * plain value.
   * @param[in, out] a_otherFace Other face.
   * @return Reference to (*this).
   */
  Face&
  operator=(Face&& a_otherFace) = default;

  /**
   * @brief Define function which sets the normal vector and half-edge index
   * @param[in] a_normal    Normal vector
   * @param[in] a_edgeIndex Index of the half-edge in the owning mesh's edge array.
   */
  inline void
  define(const Vec3& a_normal, const uint32_t a_edgeIndex) noexcept;

  /**
   * @brief Reconcile face. This will compute the normal vector, area, centroid,
   * and the 2D embedding of the polygon
   * @param[in] a_mesh Owning mesh, used to resolve the half-edge loop.
   * @note "Everything" must be set before doing this, i.e. the face must be
   * complete with half edges and there can be no dangling edges.
   */
  inline void
  reconcile(const Mesh& a_mesh);

  /**
   * @brief Compute the 2D embedding of this polygon from the current normal vector and topology,
   * without touching the normal vector, centroid, or area.
   * @details Unlike reconcile(), this does not recompute the normal vector, so it is safe to call
   * after manually restoring a normal vector (e.g. when deep-copying a face and preserving whatever
   * normal it had, including one set via flipNormal()) without having that normal silently
   * overwritten by a geometrically-derived one.
   */
  inline void
  computeProjectionDirections() noexcept;

  /**
   * @brief Flip the normal vector
   */
  inline void
  flipNormal() noexcept;

  /**
   * @brief Set the half edge index
   * @param[in] a_halfEdgeIndex Index of the half-edge in the owning mesh's edge array, or
   * UINT32_MAX to mark it unset.
   */
  inline void
  setHalfEdge(const uint32_t a_halfEdgeIndex) noexcept;

  /**
   * @brief Set the meta-data.
   * @param[in] a_metaData Meta-data.
   */
  inline void
  setMetaData(const Meta& a_metaData) noexcept;

  /**
   * @brief Set the inside/outside algorithm when determining if a point projects
   * to the inside or outside of the polygon.
   * @param[in] a_algorithm Desired algorithm
   */
  inline void
  setInsideOutsideAlgorithm(InsideOutsideAlgorithm a_algorithm) noexcept;

  /**
   * @brief Get the index of the starting half-edge.
   * @return Index of the half-edge in the owning mesh's edge array, or UINT32_MAX if unset.
   */
  [[nodiscard]] inline uint32_t
  getHalfEdgeIndex() const noexcept;

  /**
   * @brief Get modifiable centroid
   * @return Reference to the centroid vector.
   */
  [[nodiscard]] inline Vec3T<T>&
  getCentroid() noexcept;

  /**
   * @brief Get immutable centroid
   * @return Const reference to the centroid vector.
   */
  [[nodiscard]] inline const Vec3T<T>&
  getCentroid() const noexcept;

  /**
   * @brief Get modifiable centroid position in specified coordinate direction
   * @param[in] a_dir Coordinate direction
   * @return Reference to the a_dir-th coordinate of the centroid.
   */
  [[nodiscard]] inline T&
  getCentroid(const size_t a_dir) noexcept;

  /**
   * @brief Get immutable centroid position in specified coordinate direction
   * @param[in] a_dir Coordinate direction
   * @return Const reference to the a_dir-th coordinate of the centroid.
   */
  [[nodiscard]] inline const T&
  getCentroid(const size_t a_dir) const noexcept;

  /**
   * @brief Get modifiable normal vector
   * @return Reference to the normal vector.
   */
  [[nodiscard]] inline Vec3T<T>&
  getNormal() noexcept;

  /**
   * @brief Get immutable normal vector
   * @return Const reference to the normal vector.
   */
  [[nodiscard]] inline const Vec3T<T>&
  getNormal() const noexcept;

  /**
   * @brief Get modifiable polygon area (computed during reconcile()).
   * @return Reference to m_area.
   */
  [[nodiscard]] inline T&
  getArea() noexcept;

  /**
   * @brief Get immutable polygon area (computed during reconcile()).
   * @return Const reference to m_area.
   */
  [[nodiscard]] inline const T&
  getArea() const noexcept;

  /**
   * @brief Get meta-data
   * @return Reference to the metadata.
   */
  [[nodiscard]] inline Meta&
  getMetaData() noexcept;

  /**
   * @brief Get meta-data (const overload)
   * @return Const reference to the metadata.
   */
  [[nodiscard]] inline const Meta&
  getMetaData() const noexcept;

  /**
   * @brief Compute the signed distance to a point.
   * @param[in] a_x0   Point in space
   * @param[in] a_mesh Owning mesh, used to resolve the half-edge loop.
   * @details This algorithm operates by checking if the input point projects to
   * the inside of the polygon. If it does then the distance is just the
   * projected distance onto the polygon plane and the sign is well-defined.
   * Otherwise, we check the distance to the edges of the polygon.
   * @return Signed distance to the face; sign determined by normal direction.
   */
  [[nodiscard]] inline T
  signedDistance(const Vec3& a_x0, const Mesh& a_mesh) const noexcept;

  /**
   * @brief Compute the unsigned squared distance to a point.
   * @param[in] a_x0   Point in space
   * @param[in] a_mesh Owning mesh, used to resolve the half-edge loop.
   * @details This algorithm operates by checking if the input point projects to
   * the inside of the polygon. If it does then the distance is just the
   * projected distance onto the polygon plane. Otherwise, we check the distance
   * to the edges of the polygon.
   * @return Squared unsigned distance to the face.
   */
  [[nodiscard]] inline T
  unsignedDistance2(const Vec3& a_x0, const Mesh& a_mesh) const noexcept;

  /**
   * @brief Return the coordinates of all the vertices on this polygon.
   * @param[in] a_mesh Owning mesh, used to resolve the half-edge loop.
   * @details This builds a list of all the vertex coordinates and returns it.
   * @return Vector of 3D coordinates of all vertices on this polygon.
   */
  [[nodiscard]] inline std::vector<Vec3T<T>>
  getAllVertexCoordinates(const Mesh& a_mesh) const;

  /**
   * @brief Return the indices of all the vertices on this polygon
   * @param[in] a_mesh Owning mesh, used to resolve the half-edge loop.
   * @details This builds a list of all the vertex indices and returns it.
   * @return Vector of indices, into the owning mesh's vertex array, of all vertices on this polygon.
   */
  [[nodiscard]] inline std::vector<uint32_t>
  gatherVertexIndices(const Mesh& a_mesh) const;

  /**
   * @brief Return the indices of all the half-edges on this polygon
   * @param[in] a_mesh Owning mesh, used to resolve the half-edge loop.
   * @details This builds a list of all the edge indices and returns it.
   * @return Vector of indices, into the owning mesh's edge array, of all half-edges on this polygon.
   */
  [[nodiscard]] inline std::vector<uint32_t>
  gatherEdgeIndices(const Mesh& a_mesh) const;

  /**
   * @brief Get the lower-left-most coordinate of this polygon face
   * @param[in] a_mesh Owning mesh, used to resolve the half-edge loop.
   * @return Lower-left-most coordinate of this polygon face.
   */
  [[nodiscard]] inline Vec3T<T>
  getSmallestCoordinate(const Mesh& a_mesh) const;

  /**
   * @brief Get the upper-right-most coordinate of this polygon face
   * @param[in] a_mesh Owning mesh, used to resolve the half-edge loop.
   * @return Upper-right-most coordinate of this polygon face.
   */
  [[nodiscard]] inline Vec3T<T>
  getHighestCoordinate(const Mesh& a_mesh) const;

protected:
  /**
   * @brief This polygon's half-edge index. A valid face will always have this set.
   * @details Index into the owning DCEL::MeshT's edge array, or UINT32_MAX if unset.
   */
  uint32_t m_halfEdge = UINT32_MAX;

  /**
   * @brief Polygon face normal vector
   */
  Vec3 m_normal = Vec3::zeros();

  /**
   * @brief Polygon face centroid position
   */
  Vec3 m_centroid = Vec3::zeros();

  /**
   * @brief Meta-data attached to this face
   * @details Value-initialized so that every constructor leaves it in a defined state: for a
   * fundamental Meta type (e.g. short, int), a member with no initializer and no explicit mention
   * in a constructor's member-initializer list is left indeterminate, not zero.
   */
  Meta m_metaData{};

  /**
   * @brief Cached polygon face area (stored by reconcile()).
   */
  T m_area{T(0)};

  /**
   * @brief Cartesian axis used as the 2D x-direction of this polygon's 2D embedding (stored by
   * reconcile()). The inside/outside test projects points to 2D by keeping (m_xDir, m_yDir) and
   * dropping the coordinate of the largest normal component.
   * @details Defaults to the out-of-range sentinel UINT32_MAX rather than a valid axis (0-2) so that
   * a face used before reconcile()/computeProjectionDirections() has run trips the `m_xDir < 3`
   * assertions in projectPoint() and isPointInsideFace() instead of silently projecting onto axis 0.
   */
  uint32_t m_xDir = UINT32_MAX;

  /**
   * @brief Cartesian axis used as the 2D y-direction of this polygon's 2D embedding (stored by
   * reconcile()).
   * @details Defaults to the out-of-range sentinel UINT32_MAX for the same reason as m_xDir.
   */
  uint32_t m_yDir = UINT32_MAX;

  /**
   * @brief Algorithm for inside/outside tests
   */
  InsideOutsideAlgorithm m_insideOutsideAlgorithm = InsideOutsideAlgorithm::CrossingNumber;

  /**
   * @brief Compute the centroid position of this polygon
   * @param[in] a_mesh Owning mesh, used to resolve the half-edge loop.
   */
  inline void
  computeCentroid(const Mesh& a_mesh);

  /**
   * @brief Compute the normal position of this polygon
   * @param[in] a_mesh Owning mesh, used to resolve the half-edge loop.
   */
  inline void
  computeNormal(const Mesh& a_mesh);

  /**
   * @brief Compute the area of this polygon and cache it in m_area.
   * @param[in] a_mesh Owning mesh, used to resolve the half-edge loop.
   */
  inline void
  computeArea(const Mesh& a_mesh);

  /**
   * @brief Normalize the normal vector, ensuring it has a length of 1
   */
  inline void
  normalizeNormalVector() noexcept;

  /**
   * @brief Compute the projection of a point onto the polygon face plane
   * @param[in] a_p Point in space
   * @return Projected point in the face plane.
   */
  [[nodiscard]] inline Vec3T<T>
  projectPointIntoFacePlane(const Vec3& a_p) const noexcept;

  /**
   * @brief Check if a point projects to inside or outside the polygon face
   * @param[in] a_p    Point in space
   * @param[in] a_mesh Owning mesh, used to resolve the half-edge loop.
   * @return Returns true if a_p projects to inside the polygon and false
   * otherwise.
   */
  [[nodiscard]] inline bool
  isPointInsideFace(const Vec3& a_p, const Mesh& a_mesh) const noexcept;

  /**
   * @brief Project a 3D point to this face's 2D embedding by dropping one coordinate.
   * @param[in] a_point 3D point.
   * @return The 2D point (a_point[m_xDir], a_point[m_yDir]).
   */
  [[nodiscard]] inline Vec2T<T>
  projectPoint(const Vec3& a_point) const noexcept;

  /**
   * @brief Compute the winding number of this polygon's boundary around a projected 2D point.
   * @details Walks the face's half-edge loop, projecting each vertex on the fly.
   * @param[in] a_point Projected 2D query point.
   * @param[in] a_mesh  Owning mesh, used to resolve the half-edge loop.
   * @return The winding number.
   */
  [[nodiscard]] inline int
  computeWindingNumber(const Vec2T<T>& a_point, const Mesh& a_mesh) const noexcept;

  /**
   * @brief Compute the crossing number of a +x ray from a projected 2D point with this polygon.
   * @details Walks the face's half-edge loop, projecting each vertex on the fly.
   * @param[in] a_point Projected 2D query point.
   * @param[in] a_mesh  Owning mesh, used to resolve the half-edge loop.
   * @return The crossing number.
   */
  [[nodiscard]] inline size_t
  computeCrossingNumber(const Vec2T<T>& a_point, const Mesh& a_mesh) const noexcept;

  /**
   * @brief Compute the total subtended angle of this polygon's edges around a projected 2D point.
   * @details Walks the face's half-edge loop, projecting each vertex on the fly.
   * @param[in] a_point Projected 2D query point.
   * @param[in] a_mesh  Owning mesh, used to resolve the half-edge loop.
   * @return The subtended angle (+-2*pi for an interior point, 0 for an exterior one).
   */
  [[nodiscard]] inline T
  computeSubtendedAngle(const Vec2T<T>& a_point, const Mesh& a_mesh) const noexcept;
};

static_assert(std::is_trivially_copyable_v<FaceT<float, DefaultMetaData>>,
              "FaceT<float,DefaultMetaData> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<FaceT<double, DefaultMetaData>>,
              "FaceT<double,DefaultMetaData> must be trivially copyable");

} // namespace DCEL

} // namespace EBGeometry

#include "EBGeometry_DCEL_FaceImplem.hpp"

#endif
