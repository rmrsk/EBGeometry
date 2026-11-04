// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_SphereAoSoA.hpp
 * @brief  Declaration of a PointAoSoA group with a radius lane: a group of spheres carrying their ids.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_SPHEREAOSOA_HPP
#define EBGEOMETRY_SPHEREAOSOA_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <type_traits>

// Our includes
#include "EBGeometry_Array.hpp"
#include "EBGeometry_PointAoSoA.hpp"
#include "EBGeometry_SphereCloud.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief A group of up to W spheres carrying their cloud indices: a PointAoSoA<T, W> of the centers, and
 * one more lane array of the radii.
 * @details The leaf primitive of SphereCloudBVH. The centers and ids are a PointAoSoA, so the distance to
 * every center is PointSoAT's SIMD-wide computation, and the radii sit in their own array beside it.
 * @tparam T Floating-point precision.
 * @tparam W SIMD width; see PointSoAT. Defaults to PointSoA::DefaultWidth<T>().
 */
template <class T, size_t W = PointSoA::DefaultWidth<T>()>
struct SphereAoSoA
{
  static_assert(W > 0, "W must be positive");
  static_assert(std::is_floating_point_v<T>, "SphereAoSoA requires a floating-point type T");

public:
  /**
   * @brief Pack a_count (sphere, id) pairs into this group.
   * @details Pads lanes a_count..W-1 by repeating the last real sphere and id, as PointAoSoA::pack() does,
   * so all W lanes hold valid data.
   * @param[in] a_spheres  Source sphere array with at least a_count elements. Must not be null; every center
   *                       and radius finite, every radius at least 0.
   * @param[in] a_sphereIds Source id array (typically cloud indices), same order and length as a_spheres.
   *                       Must not be null.
   * @param[in] a_count    Number of valid (sphere, id) pairs to pack. Must satisfy 1 <= a_count <= W.
   */
  EBGEOMETRY_HOST
  void
  pack(const SphereCloud::Sphere<T>* a_spheres, const uint32_t* a_sphereIds, uint32_t a_count) noexcept;

  /**
   * @brief pack() without its always-on checks, for a build that runs on a device.
   * @details The caller guarantees what pack() checks (checked here only with assertions on).
   * @param[in] a_spheres   Source sphere array with at least a_count elements.
   * @param[in] a_sphereIds Source id array, same order and length as a_spheres.
   * @param[in] a_count     Number of valid (sphere, id) pairs to pack.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  packUnchecked(const SphereCloud::Sphere<T>* a_spheres, const uint32_t* a_sphereIds, uint32_t a_count) noexcept;

  /**
   * @brief Signed distances from a_point to the surface of every one of the W lane spheres, |p - c| - r.
   * @details Negative inside a sphere. Padded lanes repeat the last real sphere's value.
   * @param[in] a_point Query point. Must be finite.
   * @return Per-lane signed distances, one per W lanes.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  Array<T, W>
  getSignedDistances(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Get the id of the sphere in one lane of this group.
   * @param[in] a_lane Lane index. Must satisfy 0 <= a_lane < W (padded lanes return the last real
   * sphere's id; see pack()).
   * @return The id of the sphere at a_lane.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  getSphereId(size_t a_lane) const noexcept;

  /**
   * @brief Number of real (non-padded) spheres in this group.
   * @return The count passed to pack(), 1..W (0 before pack()).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  numValid() const noexcept
  {
    return m_centers.numValid();
  }

  /**
   * @brief Compute the bounding volume enclosing all valid spheres in this group, each grown by its radius.
   * @tparam BV Bounding volume type; must be constructible from a low and a high corner (AABBT<T>, say).
   * @return Bounding volume enclosing every valid sphere.
   */
  template <class BV>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BV
  computeBoundingVolume() const noexcept;

protected:
  /**
   * @brief The centers and ids. It also holds the valid-sphere count.
   */
  PointAoSoA<T, W> m_centers;

  /**
   * @brief Per-lane radii. m_radii[j] = radius of sphere j.
   */
  Array<T, W> m_radii;
};

static_assert(std::is_trivially_copyable_v<SphereAoSoA<float>>, "SphereAoSoA<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<SphereAoSoA<double>>, "SphereAoSoA<double> must be trivially copyable");

} // namespace EBGeometry

#endif
