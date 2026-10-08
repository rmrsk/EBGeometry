// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_BoundingVolumes.hpp
 * @brief  Declarations of bounding volume types used in bounding volume hierarchies.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_BOUNDINGVOLUMES_HPP
#define EBGEOMETRY_BOUNDINGVOLUMES_HPP

// Std includes
#include <cstddef>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Namespace encapsulating bounding volume types for use with bounding volume hierarchies.
 */
namespace BoundingVolumes {

/**
 * @brief Axis-aligned bounding box (AABB) enclosing a set of 3D points.
 * @tparam T Floating-point precision (e.g., float or double).
 */
template <class T>
class AABBT
{
  static_assert(std::is_floating_point_v<T>, "AABBT<T>: T must be a floating-point type.");

public:
  /**
   * @brief Alias to cut down on typing.
   */
  using Vec3 = Vec3T<T>;

  /**
   * @brief Default constructor. Sets lo = +∞, hi = −∞ (inverted sentinel).
   * @details The inverted state is the identity element for AABB union and also
   * ensures that any geometric query (distance, volume, intersection) fires an
   * @c EBGEOMETRY_EXPECT assertion. Call define() or use the explicit constructor
   * before use.
   */
  AABBT() noexcept = default;

  /**
   * @brief Construct an AABB from explicit low and high corners.
   * @param[in] a_lo Low corner.
   * @param[in] a_hi High corner.
   */
  EBGEOMETRY_HOST_DEVICE
  inline AABBT(const Vec3T<T>& a_lo, const Vec3T<T>& a_hi) noexcept;

  /**
   * @brief Copy constructor.
   * @param[in] a_other Bounding box to copy.
   */
  AABBT(const AABBT& a_other) noexcept = default;

  /**
   * @brief Construct the smallest AABB enclosing all boxes in @p a_others.
   * @param[in] a_others Bounding boxes to enclose.
   */
  EBGEOMETRY_HOST
  inline AABBT(const std::vector<AABBT<T>>& a_others) noexcept;

  /**
   * @brief Construct the smallest AABB enclosing a set of 3D points.
   * @details Mixed floating-point precision is allowed: @p P may differ from @p T.
   * @tparam P Floating-point precision of the input points.
   * @param[in] a_points Set of 3D points.
   */
  template <class P>
  EBGEOMETRY_HOST
  inline AABBT(const std::vector<Vec3T<P>>& a_points) noexcept;

  /**
   * @brief Construct the smallest AABB enclosing a point array.
   * @details Callable from host and device. @p a_points must reference @p a_numPoints points that are
   * accessible from wherever the constructor runs (a device pointer for a device call).
   * @tparam P Floating-point precision of the input points.
   * @param[in] a_points    Pointer to the first of @p a_numPoints points.
   * @param[in] a_numPoints Number of points to enclose (must be > 0).
   */
  template <class P>
  EBGEOMETRY_HOST_DEVICE
  inline AABBT(const Vec3T<P>* a_points, size_t a_numPoints) noexcept;

  /**
   * @brief Destructor.
   */
  ~AABBT() noexcept = default;

  /**
   * @brief Copy assignment.
   * @param[in] a_other Bounding box to copy.
   * @return Reference to (*this).
   */
  AABBT&
  operator=(const AABBT<T>& a_other) = default;

  /**
   * @brief Move constructor.
   * @param[in] a_other Bounding box to move from.
   */
  AABBT(AABBT&& a_other) noexcept = default;

  /**
   * @brief Move assignment operator.
   * @param[in] a_other Bounding box to move from.
   * @return Reference to (*this).
   */
  AABBT&
  operator=(AABBT&& a_other) noexcept = default;

  /**
   * @brief Fit this AABB to the smallest box enclosing @p a_points.
   * @details Mixed floating-point precision is allowed: @p P may differ from @p T.
   * @tparam P Floating-point precision of the input points.
   * @param[in] a_points Set of 3D points.
   */
  template <class P>
  EBGEOMETRY_HOST
  inline void
  define(const std::vector<Vec3T<P>>& a_points) noexcept;

  /**
   * @brief Fit this AABB to the smallest box enclosing a point array.
   * @details Callable from host and device. @p a_points must reference @p a_numPoints points that are
   * accessible from wherever the call runs (a device pointer for a device call).
   * @tparam P Floating-point precision of the input points.
   * @param[in] a_points    Pointer to the first of @p a_numPoints points.
   * @param[in] a_numPoints Number of points to enclose (must be > 0).
   */
  template <class P>
  EBGEOMETRY_HOST_DEVICE
  inline void
  define(const Vec3T<P>* a_points, size_t a_numPoints) noexcept;

  /**
   * @brief Test whether this AABB intersects @p a_other.
   * @param[in] a_other The other AABB.
   * @return True if the two boxes overlap, false otherwise.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  intersects(const AABBT& a_other) const noexcept;

  /**
   * @brief Get a modifiable reference to the low corner of the AABB.
   * @return Reference to the low corner.
   */
  EBGEOMETRY_HOST_DEVICE
  inline Vec3T<T>&
  getLowCorner() noexcept;

  /**
   * @brief Get the low corner of the AABB.
   * @return Const reference to the low corner.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const Vec3T<T>&
  getLowCorner() const noexcept;

  /**
   * @brief Get a modifiable reference to the high corner of the AABB.
   * @return Reference to the high corner.
   */
  EBGEOMETRY_HOST_DEVICE
  inline Vec3T<T>&
  getHighCorner() noexcept;

  /**
   * @brief Get the high corner of the AABB.
   * @return Const reference to the high corner.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const Vec3T<T>&
  getHighCorner() const noexcept;

  /**
   * @brief Compute the centroid of the AABB.
   * @return Midpoint of the low and high corners.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Vec3
  getCentroid() const noexcept;

  /**
   * @brief Compute the overlapping volume between this AABB and @p a_other.
   * @param[in] a_other The other AABB.
   * @return Overlap volume; zero if the boxes do not intersect.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  getOverlappingVolume(const AABBT<T>& a_other) const noexcept;

  /**
   * @brief Compute the unsigned distance from @p a_x0 to this AABB.
   * @param[in] a_x0 3D query point.
   * @return Distance from @p a_x0 to the nearest box face; zero if @p a_x0 is inside.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  getDistance(const Vec3& a_x0) const noexcept;

  /**
   * @brief Compute the squared unsigned distance from @p a_x0 to this AABB.
   * @details Avoids the sqrt that getDistance() pays -- prefer this whenever the caller only
   * needs the distance for comparison (e.g. BVH pruning), not its actual magnitude.
   * @param[in] a_x0 3D query point.
   * @return Squared distance from @p a_x0 to the nearest box face; zero if @p a_x0 is inside.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  getDistance2(const Vec3& a_x0) const noexcept;

  /**
   * @brief Compute the AABB volume.
   * @return Product of the three side lengths: dx * dy * dz.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  getVolume() const noexcept;

  /**
   * @brief Compute the AABB surface area.
   * @return Sum of the six face areas: 2*(dx*dy + dy*dz + dz*dx).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  getArea() const noexcept;

  /**
   * @brief The smallest AABB enclosing this box and @p a_other.
   * @details Allocation-free, and callable on a device, unlike the constructor from a list of boxes.
   * The default-constructed (inverted) box is its identity: merging it with any box returns that box.
   * @param[in] a_other The other AABB.
   * @return The union of the two boxes.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline AABBT<T>
  merged(const AABBT<T>& a_other) const noexcept;

protected:
  /**
   * @brief Low corner of the bounding box. Initialised to +∞ (inverted sentinel).
   */
  Vec3 m_loCorner = Vec3::infinity();

  /**
   * @brief High corner of the bounding box. Initialised to −∞ (inverted sentinel).
   */
  Vec3 m_hiCorner = -Vec3::infinity();
};

/**
 * @brief Test whether two axis-aligned bounding boxes overlap.
 * @tparam T Floating-point precision.
 * @param[in] a_u One bounding box.
 * @param[in] a_v The other bounding box.
 * @return True if the boxes intersect, false otherwise.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
bool
intersects(const AABBT<T>& a_u, const AABBT<T>& a_v) noexcept;

/**
 * @brief Compute the overlapping volume between two axis-aligned bounding boxes.
 * @tparam T Floating-point precision.
 * @param[in] a_u One bounding box.
 * @param[in] a_v The other bounding box.
 * @return Overlap volume; zero if the boxes do not intersect.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
T
getOverlappingVolume(const AABBT<T>& a_u, const AABBT<T>& a_v) noexcept;

static_assert(std::is_trivially_copyable_v<AABBT<float>>, "AABBT<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<AABBT<double>>, "AABBT<double> must be trivially copyable");
} // namespace BoundingVolumes

} // namespace EBGeometry

#include "EBGeometry_BoundingVolumesImplem.hpp"

#endif
