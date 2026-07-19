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
#include <type_traits>

// Our includes
#include "EBGeometry_DCEL.hpp"
#include "EBGeometry_Polygon2D.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

namespace DCEL {

/**
 * @brief Class which represents a polygon face in a double-edge connected list
 * (DCEL).
 * @details This class is a polygon face in a DCEL mesh. In the flat, index-based DCEL
 * representation it is a data holder: it stores a normal vector, a centroid, an area, the index of
 * one of the half-edges circulating the inside of the polygon (@c DCELIndex into the owning
 * MeshT's edge array; @c InvalidIndex if unset), and -- for signed-distance evaluation -- the 2D
 * embedding of the polygon (stored by value) together with the inside/outside test algorithm. All
 * traversal of the face's half-edge loop and all geometric queries are performed by MeshT, which
 * owns the arrays the half-edge index refers to.
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
   * @brief Alias for face type
   */
  using Face = FaceT<T, Meta>;

  /**
   * @brief Alias for the inside/outside test algorithm type.
   */
  using InsideOutsideAlgorithm = typename Polygon2D<T>::InsideOutsideAlgorithm;

  /**
   * @brief Default constructor. Leaves the half-edge index unset and the inside/outside algorithm
   * at the crossing-number algorithm.
   */
  FaceT() = default;

  /**
   * @brief Partial constructor. Calls default constructor but associates a
   * half-edge index.
   * @param[in] a_edge Index of the half-edge.
   */
  FaceT(const DCELIndex a_edge);

  /**
   * @brief Copy constructor (defaulted memberwise copy).
   * @details The face is a flat data holder: its half-edge link is a plain array index, and the 2D
   * embedding (m_poly2) is stored by value, so an ordinary memberwise copy carries everything --
   * normal, centroid, area, half-edge index, meta-data, the 2D embedding, and the inside/outside
   * algorithm. This removes the old "copy silently drops m_poly2" hazard that the previous
   * shared_ptr-based storage had.
   * @param[in] a_otherFace Other face to copy from.
   */
  FaceT(const Face& a_otherFace) = default;

  /**
   * @brief Move constructor (defaulted memberwise move).
   * @param[in, out] a_otherFace Other face.
   */
  FaceT(Face&& a_otherFace) = default;

  /**
   * @brief Destructor (does nothing)
   */
  ~FaceT() = default;

  /**
   * @brief Copy assignment operator (defaulted memberwise copy).
   * @param[in] a_otherFace Other face.
   * @return Reference to (*this).
   */
  Face&
  operator=(const Face& a_otherFace) = default;

  /**
   * @brief Move assignment operator (defaulted memberwise move).
   * @param[in, out] a_otherFace Other face.
   * @return Reference to (*this).
   */
  Face&
  operator=(Face&& a_otherFace) = default;

  /**
   * @brief Define function which sets the normal vector and half-edge index
   * @param[in] a_normal Normal vector
   * @param[in] a_edge   Index of the half-edge
   */
  inline void
  define(const Vec3& a_normal, const DCELIndex a_edge) noexcept;

  /**
   * @brief Flip the normal vector
   */
  inline void
  flipNormal() noexcept;

  /**
   * @brief Normalize the normal vector, ensuring it has a length of 1
   */
  inline void
  normalizeNormalVector() noexcept;

  /**
   * @brief Set the half-edge index
   * @param[in] a_halfEdge Index of the half-edge
   */
  inline void
  setHalfEdge(const DCELIndex a_halfEdge) noexcept;

  /**
   * @brief Set the meta-data.
   * @param[in] a_metaData Meta-data.
   */
  inline void
  setMetaData(const Meta& a_metaData) noexcept;

  /**
   * @brief Set the 2D embedding of this polygon.
   * @param[in] a_poly2 2D embedding.
   */
  inline void
  setPolygon2D(const Polygon2D<T>& a_poly2) noexcept;

  /**
   * @brief Set the inside/outside algorithm when determining if a point projects
   * to the inside or outside of the polygon.
   * @param[in] a_algorithm Desired algorithm
   */
  inline void
  setInsideOutsideAlgorithm(const InsideOutsideAlgorithm a_algorithm) noexcept;

  /**
   * @brief Get the index of the starting half-edge.
   * @return Half-edge index, or InvalidIndex.
   */
  [[nodiscard]] inline DCELIndex
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
   * @brief Get modifiable polygon area (computed during MeshT::reconcile()).
   * @return Reference to m_area.
   */
  [[nodiscard]] inline T&
  getArea() noexcept;

  /**
   * @brief Get immutable polygon area (computed during MeshT::reconcile()).
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
   * @brief Get the 2D embedding of this polygon.
   * @return Const reference to the 2D embedding.
   */
  [[nodiscard]] inline const Polygon2D<T>&
  getPolygon2D() const noexcept;

  /**
   * @brief Get the inside/outside test algorithm for this face.
   * @return Inside/outside algorithm.
   */
  [[nodiscard]] inline InsideOutsideAlgorithm
  getInsideOutsideAlgorithm() const noexcept;

protected:
  /**
   * @brief Polygon face normal vector
   */
  Vec3 m_normal = Vec3::zeros();

  /**
   * @brief Polygon face centroid position
   */
  Vec3 m_centroid = Vec3::zeros();

  /**
   * @brief Cached polygon face area (stored by MeshT::reconcile()).
   */
  T m_area{T(0)};

  /**
   * @brief Index of one of this polygon's half-edges. A valid face always has this set.
   */
  DCELIndex m_halfEdge = InvalidIndex;

  /**
   * @brief Meta-data attached to this face
   * @details Value-initialized so that every constructor leaves it in a defined state.
   */
  Meta m_metaData{};

  /**
   * @brief 2D embedding of this polygon, stored by value. This is the 2D view of the current
   * object projected along its normal vector cardinal. Populated by MeshT during reconcile().
   */
  Polygon2D<T> m_poly2;

  /**
   * @brief Algorithm for inside/outside tests
   */
  InsideOutsideAlgorithm m_poly2Algorithm = Polygon2D<T>::InsideOutsideAlgorithm::CrossingNumber;
};
} // namespace DCEL

} // namespace EBGeometry

#include "EBGeometry_DCEL_FaceImplem.hpp"

#endif
