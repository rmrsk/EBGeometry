// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file    EBGeometry_PointCloudSDF.hpp
 * @brief   PointCloudSDF: a union of equal spheres around the points of a PointCloudBVH, as a shape.
 * @author  Robert Marskar
 */

#ifndef EBGEOMETRY_POINTCLOUDSDF_HPP
#define EBGEOMETRY_POINTCLOUDSDF_HPP

// Std includes
#include <cmath>
#include <cstddef>
#include <type_traits>

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_DistanceQuality.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_PointCloudBVH.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief The union of the balls of one radius r around every point of a point cloud:
 * |p - c| - r for the closest cloud point c.
 * @details One closest-point query of the cloud's BVH, which is much cheaper than a BVH union of N
 * sphere primitives: the cloud's leaves are SIMD groups of points, and it needs no sub-program per
 * sphere. With r = 0 the value is the unsigned distance to the points. It is a shape, so it can be a
 * leaf of an expression: a Tape lists it among its user primitives, and it must live in the tape's
 * pool, as the mesh SDFs do.
 *
 * The value is the distance to the union outside it and at most that distance inside it, where two
 * balls overlap, so the distance quality is Bound, as for any union. An empty cloud is empty space:
 * the value is Math::Limits<T>::max() everywhere, which a union or intersection passes through, and
 * which a sum (Add, Displace, Offset by a negative amount) may round to infinity.
 *
 * Like the cloud, this is a handle into the cloud's pool, which must outlive it and every copy and view
 * of it. Mirror the pool and pass rebasedView() into a kernel; signedDistance() is device-callable.
 * @tparam T Floating-point precision.
 * @tparam K BVH branching factor.
 * @tparam W Points per SoA leaf lane group.
 */
template <class T, size_t K = BVH::DefaultBranchingRatio<T>(), size_t W = PointSoA::DefaultWidth<T>()>
class PointCloudSDF : public PoolHandle
{
public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: a lower bound, exact outside.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Bound;

  /**
   * @brief The point cloud type.
   */
  using Cloud = PointCloudBVH<T, K, W>;

  /**
   * @brief The object's arrays as offsets into its pool, and the radius, with no location (see
   * PoolHandle).
   */
  struct Layout
  {
    /**
     * @brief The cloud.
     */
    typename Cloud::Layout m_cloud;

    /**
     * @brief The ball radius.
     */
    T m_radius;
  };

  /**
   * @brief The balls of radius @p a_radius around every point of @p a_cloud.
   * @details Copies the cloud's descriptors, not its arrays: the cloud's pool must outlive this object.
   * Aborts, in every build, on a negative or non-finite radius.
   * @param[in] a_cloud  The cloud.
   * @param[in] a_radius Ball radius (finite, >= 0); 0 gives the unsigned distance to the points.
   */
  EBGEOMETRY_HOST
  PointCloudSDF(const Cloud& a_cloud, const T a_radius)
    : PoolHandle(a_cloud.location()), m_cloud(a_cloud.layout()), m_radius(a_radius)
  {
    EBGEOMETRY_REQUIRE(std::isfinite(a_radius) && a_radius >= T(0),
                       "PointCloudSDF: the radius must be finite and non-negative (%g)",
                       double(a_radius));
  }

  /**
   * @brief Rebuild a handle from a stored layout and the location of the pool that holds it.
   * @details How a Tape evaluates a PointCloudSDF it stores as a layout. @p a_location must belong to
   * the pool the layout was reserved from, or to a mirror of it; nothing is checked, since a device has
   * no pool to check against.
   * @param[in] a_layout   The arrays and the radius.
   * @param[in] a_location Where the arrays live.
   */
  EBGEOMETRY_HOST_DEVICE
  PointCloudSDF(const Layout& a_layout, const PoolLocation& a_location) noexcept
    : PoolHandle(a_location), m_cloud(a_layout.m_cloud), m_radius(a_layout.m_radius)
  {}

  /**
   * @brief The object's arrays and radius without its location, for storing it inside another
   * pool-resident object.
   * @return The layout.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  Layout
  layout() const noexcept
  {
    return Layout{m_cloud, m_radius};
  }

  /**
   * @brief The cloud.
   * @return A handle to it, at this object's location.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  Cloud
  getCloud() const noexcept
  {
    return Cloud(m_cloud, this->location());
  }

  /**
   * @brief The ball radius.
   * @return The radius.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  getRadius() const noexcept
  {
    return m_radius;
  }

  /**
   * @brief Signed distance to the union of the balls: |p - c| - r for the closest cloud point c.
   * @param[in] a_point Query point.
   * @return The value; Math::Limits<T>::max() for an empty cloud.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    const auto hit = this->getCloud().closestPoint(a_point);

    return hit.valid() ? std::sqrt(hit.distanceSquared) - m_radius : Math::Limits<T>::max();
  }

  /**
   * @brief A box enclosing every ball: the cloud's box grown by the radius.
   * @return The box; empty for an empty cloud.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    const BoundingVolumes::AABBT<T> box = this->getCloud().getBVH().computeBoundingVolume();

    const Vec3T<T>& lo = box.getLowCorner();
    const Vec3T<T>& hi = box.getHighCorner();

    if (!(lo[0] <= hi[0] && lo[1] <= hi[1] && lo[2] <= hi[2])) {
      return box;
    }

    return BoundingVolumes::AABBT<T>(lo - m_radius * Vec3T<T>::ones(), hi + m_radius * Vec3T<T>::ones());
  }

  /**
   * @brief A copy of this object that resolves against @p a_pool; see PointCloudBVH::rebasedView().
   * @param[in] a_pool The cloud's pool or one in its mirror chain.
   * @return The view.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  PointCloudSDF
  rebasedView(const Pool& a_pool) const noexcept
  {
    return PointCloudSDF(this->layout(), this->getCloud().rebasedView(a_pool).location());
  }

  /**
   * @brief Copy the cloud's arrays into @p a_dstPool.
   * @param[in,out] a_dstPool Pool to copy into; must outlive the copy.
   * @return The copy.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  PointCloudSDF
  deepCopy(Pool& a_dstPool) const
  {
    return PointCloudSDF(this->getCloud().deepCopy(a_dstPool), m_radius);
  }

private:
  /**
   * @brief The cloud, as a layout: it shares this object's location.
   */
  typename Cloud::Layout m_cloud;

  /**
   * @brief The ball radius.
   */
  T m_radius;
};

static_assert(std::is_trivially_copyable_v<PointCloudSDF<float>>, "PointCloudSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<PointCloudSDF<double>>, "PointCloudSDF<double> must be trivially copyable");

} // namespace EBGeometry

#endif
