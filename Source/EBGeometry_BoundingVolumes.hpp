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
#include <ostream>
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
 * @brief Bounding sphere — an approximation to the smallest sphere enclosing a set of 3D points.
 * @tparam T Floating-point precision (e.g., float or double).
 */
template <class T>
class SphereT
{
  static_assert(std::is_floating_point_v<T>, "SphereT<T>: T must be a floating-point type.");

public:
  /**
   * @brief Output a bounding sphere to a stream in the form "(centroid, radius)".
   * @param[in,out] os     Output stream.
   * @param[in]     sphere Bounding sphere to print.
   * @return Reference to @p os to allow chaining.
   */
  friend std::ostream&
  operator<<(std::ostream& os, const SphereT<T>& sphere)
  {
    os << '(' << sphere.getCentroid() << ", " << sphere.getRadius() << ')';

    return os;
  }

  /**
   * @brief Algorithm used to compute a bounding sphere for a set of 3D points.
   */
  enum class BuildAlgorithm
  {
    Ritter, ///< Ritter's simple two-pass approximation. Fast but may overestimate the radius slightly.
  };

  /**
   * @brief Alias to cut down on typing.
   */
  using Vec3 = Vec3T<T>;

  /**
   * @brief Default constructor. Sets radius to -1 (invalid sentinel).
   * @details Any geometric method called on a default-constructed sphere will fire
   * an @c EBGEOMETRY_EXPECT assertion. Call define() or use the explicit constructor
   * before use.
   */
  SphereT() noexcept = default;

  /**
   * @brief Construct a sphere with an explicit centre and radius.
   * @param[in] a_center Sphere centre.
   * @param[in] a_radius Sphere radius.
   */
  EBGEOMETRY_HOST_DEVICE
  inline SphereT(const Vec3T<T>& a_center, const T& a_radius) noexcept;

  /**
   * @brief Construct the smallest sphere enclosing all @p a_otherSpheres.
   * @param[in] a_otherSpheres Spheres to enclose.
   */
  EBGEOMETRY_HOST
  inline SphereT(const std::vector<SphereT<T>>& a_otherSpheres) noexcept;

  /**
   * @brief Copy constructor.
   * @param[in] a_other Sphere to copy.
   */
  SphereT(const SphereT& a_other) noexcept = default;

  /**
   * @brief Construct a bounding sphere enclosing a set of 3D points.
   * @details Mixed floating-point precision is allowed: @p P may differ from @p T.
   * @tparam P Floating-point precision of the input points.
   * @param[in] a_points Set of 3D points.
   * @param[in] a_alg    Algorithm to use (default: Ritter).
   */
  template <class P>
  EBGEOMETRY_HOST
  inline SphereT(const std::vector<Vec3T<P>>& a_points, const BuildAlgorithm& a_alg = BuildAlgorithm::Ritter) noexcept;

  /**
   * @brief Construct a bounding sphere enclosing a point array, using Ritter's algorithm.
   * @details Callable from host and device. @p a_points must reference @p a_numPoints points that are
   * accessible from wherever the constructor runs (a device pointer for a device call).
   * @tparam P Floating-point precision of the input points.
   * @param[in] a_points    Pointer to the first of @p a_numPoints points.
   * @param[in] a_numPoints Number of points to enclose (must be > 0).
   */
  template <class P>
  EBGEOMETRY_HOST_DEVICE
  inline SphereT(const Vec3T<P>* a_points, size_t a_numPoints) noexcept;

  /**
   * @brief Destructor.
   */
  ~SphereT() noexcept = default;

  /**
   * @brief Copy assignment operator.
   * @param[in] a_other Sphere to copy.
   * @return Reference to (*this).
   */
  SphereT&
  operator=(const SphereT& a_other) = default;

  /**
   * @brief Move constructor.
   * @param[in] a_other Sphere to move from.
   */
  SphereT(SphereT&& a_other) noexcept = default;

  /**
   * @brief Move assignment operator.
   * @param[in] a_other Sphere to move from.
   * @return Reference to (*this).
   */
  SphereT&
  operator=(SphereT&& a_other) noexcept = default;

  /**
   * @brief Fit this sphere to a set of 3D points using the specified algorithm.
   * @details Mixed floating-point precision is allowed: @p P may differ from @p T.
   * @tparam P Floating-point precision of the input points.
   * @param[in] a_points Set of 3D points.
   * @param[in] a_alg    Algorithm to use.
   */
  template <class P>
  EBGEOMETRY_HOST
  inline void
  define(const std::vector<Vec3T<P>>& a_points, const BuildAlgorithm& a_alg) noexcept;

  /**
   * @brief Test whether this bounding sphere intersects @p a_other.
   * @param[in] a_other Other bounding sphere.
   * @return True if the two spheres overlap, false otherwise.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  intersects(const SphereT& a_other) const noexcept;

  /**
   * @brief Get a modifiable reference to the sphere radius.
   * @return Reference to the sphere radius.
   */
  EBGEOMETRY_HOST_DEVICE
  inline T&
  getRadius() noexcept;

  /**
   * @brief Get the sphere radius.
   * @return Const reference to the sphere radius.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const T&
  getRadius() const noexcept;

  /**
   * @brief Get a modifiable reference to the sphere centroid.
   * @return Reference to the sphere centroid.
   */
  EBGEOMETRY_HOST_DEVICE
  inline Vec3&
  getCentroid() noexcept;

  /**
   * @brief Get the sphere centroid.
   * @return Const reference to the sphere centroid.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const Vec3&
  getCentroid() const noexcept;

  /**
   * @brief Compute the overlapping volume between this sphere and @p a_other.
   * @param[in] a_other Other bounding sphere.
   * @return Overlap volume; zero if the spheres do not intersect.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  getOverlappingVolume(const SphereT<T>& a_other) const noexcept;

  /**
   * @brief Compute the unsigned distance from @p a_x0 to this sphere.
   * @param[in] a_x0 3D query point.
   * @return Distance from @p a_x0 to the sphere surface; zero if @p a_x0 is inside.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  getDistance(const Vec3& a_x0) const noexcept;

  /**
   * @brief Compute the squared unsigned distance from @p a_x0 to this sphere.
   * @details Mirrors AABBT::getDistance2(), giving both bounding volumes a uniform squared-distance
   * query (e.g. for a BVH's sqrt-free pruning). Unlike the box, a sphere's surface distance is
   * inherently radial, so this cannot avoid the square root the way AABBT::getDistance2() does -- it
   * is simply getDistance() squared.
   * @param[in] a_x0 3D query point.
   * @return Squared distance from @p a_x0 to the sphere surface; zero if @p a_x0 is inside.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  getDistance2(const Vec3& a_x0) const noexcept;

  /**
   * @brief Compute the sphere volume (4/3 * pi * r^3).
   * @return Sphere volume.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  getVolume() const noexcept;

  /**
   * @brief Compute the sphere surface area (4 * pi * r^2).
   * @return Sphere surface area.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  getArea() const noexcept;

protected:
  /**
   * @brief Sphere radius. Initialised to -1 (invalid sentinel).
   */
  T m_radius = T(-1);

  /**
   * @brief Sphere centre. Initialised to the origin.
   */
  Vec3 m_center = Vec3::zeros();

  /**
   * @brief Fit a bounding sphere to @p a_points using Ritter's algorithm.
   * @tparam P Floating-point precision of the input points.
   * @param[in] a_points Set of 3D points to enclose.
   */
  template <class P>
  EBGEOMETRY_HOST
  inline void
  buildRitter(const std::vector<Vec3T<P>>& a_points) noexcept;

  /**
   * @brief Fit a bounding sphere to a point array using Ritter's algorithm.
   * @tparam P Floating-point precision of the input points.
   * @param[in] a_points    Pointer to the first of @p a_numPoints points.
   * @param[in] a_numPoints Number of points to enclose (must be > 0).
   */
  template <class P>
  EBGEOMETRY_HOST_DEVICE
  inline void
  buildRitter(const Vec3T<P>* a_points, size_t a_numPoints) noexcept;
};

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
   * @brief Output an AABB to a stream in the form "(lo, hi)".
   * @param[in,out] os   Output stream.
   * @param[in]     aabb Bounding box to print.
   * @return Reference to @p os to allow chaining.
   */
  friend std::ostream&
  operator<<(std::ostream& os, const AABBT<T>& aabb)
  {
    os << '(' << aabb.getLowCorner() << ", " << aabb.getHighCorner() << ')';

    return os;
  }

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
 * @brief Test whether two bounding spheres overlap.
 * @tparam T Floating-point precision.
 * @param[in] a_u One bounding sphere.
 * @param[in] a_v The other bounding sphere.
 * @return True if the spheres intersect, false otherwise.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
bool
intersects(const SphereT<T>& a_u, const SphereT<T>& a_v) noexcept;

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
 * @brief Compute the overlapping volume between two bounding spheres.
 * @tparam T Floating-point precision.
 * @param[in] a_u One bounding sphere.
 * @param[in] a_v The other bounding sphere.
 * @return Overlap volume; zero if the spheres do not intersect.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
T
getOverlappingVolume(const SphereT<T>& a_u, const SphereT<T>& a_v) noexcept;

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

static_assert(std::is_trivially_copyable_v<SphereT<float>>, "SphereT<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<SphereT<double>>, "SphereT<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<AABBT<float>>, "AABBT<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<AABBT<double>>, "AABBT<double> must be trivially copyable");
} // namespace BoundingVolumes

} // namespace EBGeometry

#include "EBGeometry_BoundingVolumesImplem.hpp"

#endif
