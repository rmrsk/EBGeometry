// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_SphereCloud.hpp
 * @brief  The sphere, query result and sphere sources of SphereCloudBVH.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_SPHERECLOUD_HPP
#define EBGEOMETRY_SPHERECLOUD_HPP

// Std includes
#include <cstdint>
#include <type_traits>

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief The sphere, query result and sphere sources of SphereCloudBVH.
 * @details A sphere is identified by its **cloud index**: the index its source is called with, or its
 * position in the array the BVH was built from. Per-sphere user data lives in the caller's own array,
 * indexed by it.
 */
namespace SphereCloud {

/**
 * @brief The cloud index of "no sphere": a miss.
 * @details A cloud holds fewer spheres than this (the builds check it), so it never names a real sphere.
 */
inline constexpr uint32_t InvalidIndex = Math::Limits<uint32_t>::max();

/**
 * @brief A sphere of a sphere cloud: a center and a radius.
 * @details A sphere of radius 0 is a point. The builds reject a non-finite center or radius, and a
 * negative radius.
 * @tparam T Floating-point precision.
 */
template <class T>
struct Sphere
{
  Vec3T<T> m_center; ///< The center.
  T        m_radius; ///< The radius, at least 0.
};

/**
 * @brief One query result: the cloud index of a sphere and the signed distance to its surface.
 * @details The distance is |p - c| - r, negative inside the sphere. A default-constructed Hit is a miss:
 * @c index == InvalidIndex and @c distance == Math::Limits<T>::max(), which a query on an empty cloud
 * returns.
 * @tparam T Floating-point precision.
 */
template <class T>
struct Hit
{
  uint32_t index    = InvalidIndex;           ///< Cloud index of the sphere.
  T        distance = Math::Limits<T>::max(); ///< Signed distance from the query to its surface.

  /**
   * @brief Whether this is a match rather than a miss.
   * @return True if @c index refers to a sphere.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  bool
  valid() const noexcept
  {
    return index != InvalidIndex;
  }
};

/**
 * @brief A source of spheres reading one array of Sphere<T>, indexed by cloud index.
 * @details Trivially copyable, so a build can read it on a device; the array must then be in memory the
 * device can read.
 * @tparam T Floating-point precision.
 */
template <class T>
struct FromArray
{
  /**
   * @brief The spheres, indexed by cloud index.
   */
  const Sphere<T>* m_spheres = nullptr;

  /**
   * @brief The sphere with a cloud index.
   * @param[in] a_index Cloud index.
   * @return The sphere.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  Sphere<T>
  operator()(const uint32_t a_index) const noexcept
  {
    return m_spheres[a_index];
  }
};

/**
 * @brief A source of radii reading one array of T, indexed by cloud index.
 * @tparam T Floating-point precision.
 */
template <class T>
struct FromRadii
{
  /**
   * @brief The radii, indexed by cloud index.
   */
  const T* m_radii = nullptr;

  /**
   * @brief The radius of the sphere with a cloud index.
   * @param[in] a_index Cloud index.
   * @return The radius.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  operator()(const uint32_t a_index) const noexcept
  {
    return m_radii[a_index];
  }
};

/**
 * @brief A source of spheres combining a source of centers and a source of radii, for a caller who keeps
 * them apart.
 * @details With PointCloud::FromArray and FromRadii, it reads two arrays; any other trivially copyable
 * sources work too (radii computed from a particle's mass, say).
 * @tparam T       Floating-point precision.
 * @tparam Centers A source of centers: a trivially copyable type whose const call operator takes a cloud
 *                 index (uint32_t) and returns a Vec3T<T>.
 * @tparam Radii   A source of radii: a trivially copyable type whose const call operator takes a cloud
 *                 index (uint32_t) and returns a T.
 */
template <class T, class Centers, class Radii>
struct FromCentersAndRadii
{
  static_assert(std::is_trivially_copyable_v<Centers>,
                "SphereCloud::FromCentersAndRadii: Centers must be trivially copyable");
  static_assert(std::is_trivially_copyable_v<Radii>,
                "SphereCloud::FromCentersAndRadii: Radii must be trivially copyable");

  /**
   * @brief The centers.
   */
  Centers m_centers;

  /**
   * @brief The radii.
   */
  Radii m_radii;

  /**
   * @brief The sphere with a cloud index.
   * @param[in] a_index Cloud index.
   * @return The sphere: its center from the centers' source, its radius from the radii's.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  Sphere<T>
  operator()(const uint32_t a_index) const noexcept
  {
    return Sphere<T>{m_centers(a_index), m_radii(a_index)};
  }
};

} // namespace SphereCloud

} // namespace EBGeometry

#endif
