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
#include <memory>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_DCEL.hpp"
#include "EBGeometry_DCEL_Edge.hpp"
#include "EBGeometry_DCEL_Vertex.hpp"
#include "EBGeometry_Polygon2D.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

namespace DCEL {

/**
 * @brief Class which represents a polygon face in a double-edge connected list
 * (DCEL).
 * @details This class is a polygon face in a DCEL mesh. It contains pointer
 * storage to one of the half-edges that circulate the inside of the polygon
 * face, as well as having a normal vector, a centroid, and an area. This class
 * supports signed distance computations. These computations require algorithms
 * that compute e.g. the winding number of the polygon, or the number of times a
 * ray cast passes through it. Thus, one of its central features is that it can
 * be embedded in 2D by projecting it along the cardinal direction of its normal
 * vector. To be fully consistent with a DCEL structure the class stores a
 * reference to one of its half edges, but for performance reasons it also stores
 * references to the other half edges.
 * @note To compute the distance from a point to the face one must determine if
 * the point projects "inside" or "outside" the polygon. There are several
 * algorithms for this, and by default this class uses a crossing number
 * algorithm.
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
   * @brief Default constructor. Sets the half-edge to zero and the
   * inside/outside algorithm to crossing number algorithm
   */
  FaceT() = default;

  /**
   * @brief Partial constructor. Calls default constructor but associates a
   * half-edge
   * @param[in] a_edge Half-edge
   */
  FaceT(const EdgePtr& a_edge);

  /**
   * @brief Copy constructor.
   * @details Copies the normal vector, half-edge pointer, centroid, and area from the other face.
   * Meta-data (m_metaData), the 2D embedding (m_poly2), and the inside/outside algorithm
   * (m_poly2Algorithm) are deliberately NOT copied -- they default-construct instead; use
   * setMetaData() and reconcile() afterwards if they need to be (re)populated. Rationale: m_halfEdge
   * is a back-reference into a specific mesh's topology (same as VertexT/EdgeT's copy constructors),
   * so copying it wholesale is not topologically meaningful in general, but is occasionally useful.
   * m_normal, m_centroid, and m_area are plain geometric/cached values with no such concern, so they
   * are copied like any other value. operator=(const Face&) has identical semantics.
   * @param[in] a_otherFace Other face to copy from.
   */
  FaceT(const Face& a_otherFace);

  /**
   * @brief Move constructor.
   * @details Unlike the copy constructor, this transfers the entire state (including m_metaData,
   * m_poly2, and m_poly2Algorithm) from a_otherFace, since moving relocates a single object's
   * identity. Defaulted (memberwise move) rather than hand-written: explicitly defaulting is
   * required here since the presence of a user-declared copy constructor would otherwise suppress
   * the implicitly-generated move constructor. Not marked noexcept since Meta is an unconstrained
   * template parameter whose move constructor is not guaranteed to be noexcept.
   * @param[in, out] a_otherFace Other face.
   */
  FaceT(Face&& a_otherFace) = default;

  /**
   * @brief Destructor (does nothing)
   */
  ~FaceT() = default;

  /**
   * @brief Copy assignment operator.
   * @details Has the same semantics as the copy constructor; see its documentation.
   * @param[in] a_otherFace Other face.
   * @return Reference to (*this).
   */
  Face&
  operator=(const Face& a_otherFace) noexcept;

  /**
   * @brief Move assignment operator.
   * @details Has the same full-state-transfer semantics as the move constructor (defaulted
   * memberwise move; see its documentation for why this must be defaulted explicitly).
   * @param[in, out] a_otherFace Other face.
   * @return Reference to (*this).
   */
  Face&
  operator=(Face&& a_otherFace) = default;

  /**
   * @brief Define function which sets the normal vector and half-edge
   * @param[in] a_normal Normal vector
   * @param[in] a_edge   Half edge
   */
  inline void
  define(const Vec3& a_normal, const EdgePtr& a_edge) noexcept;

  /**
   * @brief Reconcile face. This will compute the normal vector, area, centroid,
   * and the 2D embedding of the polygon
   * @note "Everything" must be set before doing this, i.e. the face must be
   * complete with half edges and there can be no dangling edges.
   */
  inline void
  reconcile();

  /**
   * @brief Compute the 2D embedding of this polygon from the current normal vector and topology,
   * without touching the normal vector, centroid, or area.
   * @details Unlike reconcile(), this does not recompute the normal vector, so it is safe to call
   * after manually restoring a normal vector (e.g. when deep-copying a face and preserving whatever
   * normal it had, including one set via flipNormal()) without having that normal silently
   * overwritten by a geometrically-derived one.
   * @note The face must be complete with half edges and there can be no dangling edges.
   */
  inline void
  computePolygon2D();

  /**
   * @brief Flip the normal vector
   */
  inline void
  flipNormal() noexcept;

  /**
   * @brief Set the half edge
   * @param[in] a_halfEdge Half edge
   */
  inline void
  setHalfEdge(const EdgePtr& a_halfEdge) noexcept;

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
   * @note See CD_DCELAlgorithms.H for options (and CD_DCELPolyImplem.H for how
   * the algorithms operate).
   */
  inline void
  setInsideOutsideAlgorithm(typename Polygon2D<T>::InsideOutsideAlgorithm& a_algorithm) noexcept;

  /**
   * @brief Get modifiable half-edge
   * @return Reference to the shared pointer to the starting half-edge.
   */
  [[nodiscard]] inline EdgePtr&
  getHalfEdge() noexcept;

  /**
   * @brief Get immutable half-edge
   * @return Const reference to the shared pointer to the starting half-edge.
   */
  [[nodiscard]] inline const EdgePtr&
  getHalfEdge() const noexcept;

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
   * @param[in] a_x0 Point in space
   * @details This algorithm operates by checking if the input point projects to
   * the inside of the polygon. If it does then the distance is just the
   * projected distance onto the polygon plane and the sign is well-defined.
   * Otherwise, we check the distance to the edges of the polygon.
   * @return Signed distance to the face; sign determined by normal direction.
   */
  [[nodiscard]] inline T
  signedDistance(const Vec3& a_x0) const noexcept;

  /**
   * @brief Compute the unsigned squared distance to a point.
   * @param[in] a_x0 Point in space
   * @details This algorithm operates by checking if the input point projects to
   * the inside of the polygon. If it does then the distance is just the
   * projected distance onto the polygon plane. Otherwise, we check the distance
   * to the edges of the polygon.
   * @return Squared unsigned distance to the face.
   */
  [[nodiscard]] inline T
  unsignedDistance2(const Vec3& a_x0) const noexcept;

  /**
   * @brief Return the coordinates of all the vertices on this polygon.
   * @details This builds a list of all the vertex coordinates and returns it.
   * @return Vector of 3D coordinates of all vertices on this polygon.
   */
  [[nodiscard]] inline std::vector<Vec3T<T>>
  getAllVertexCoordinates() const;

  /**
   * @brief Return all the vertices on this polygon
   * @details This builds a list of all the vertices and returns it.
   * @return Vector of shared pointers to all vertices on this polygon.
   */
  [[nodiscard]] inline std::vector<VertexPtr>
  gatherVertices() const;

  /**
   * @brief Return all the half-edges on this polygon
   * @details This builds a list of all the edges and returns it.
   * @return Vector of shared pointers to all half-edges on this polygon.
   */
  [[nodiscard]] inline std::vector<EdgePtr>
  gatherEdges() const;

  /**
   * @brief Get the lower-left-most coordinate of this polygon face
   * @return Lower-left-most coordinate of this polygon face.
   */
  [[nodiscard]] inline Vec3T<T>
  getSmallestCoordinate() const;

  /**
   * @brief Get the upper-right-most coordinate of this polygon face
   * @return Upper-right-most coordinate of this polygon face.
   */
  [[nodiscard]] inline Vec3T<T>
  getHighestCoordinate() const;

protected:
  /**
   * @brief This polygon's half-edge. A valid face will always have != nullptr
   */
  EdgePtr m_halfEdge = nullptr;

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
   * @brief 2D embedding of this polygon. This is the 2D view of the current
   * object projected along its normal vector cardinal.
   */
  std::shared_ptr<Polygon2D<T>> m_poly2 = nullptr;

  /**
   * @brief Algorithm for inside/outside tests
   */
  typename Polygon2D<T>::InsideOutsideAlgorithm m_poly2Algorithm = Polygon2D<T>::InsideOutsideAlgorithm::CrossingNumber;

  /**
   * @brief Compute the centroid position of this polygon
   */
  inline void
  computeCentroid();

  /**
   * @brief Compute the normal position of this polygon
   */
  inline void
  computeNormal();

  /**
   * @brief Compute the area of this polygon and cache it in m_area.
   */
  inline void
  computeArea();

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
   * @param[in] a_p Point in space
   * @return Returns true if a_p projects to inside the polygon and false
   * otherwise.
   */
  [[nodiscard]] inline bool
  isPointInsideFace(const Vec3& a_p) const noexcept;
};
} // namespace DCEL

} // namespace EBGeometry

#include "EBGeometry_DCEL_FaceImplem.hpp"

#endif
