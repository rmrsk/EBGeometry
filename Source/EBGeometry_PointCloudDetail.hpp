// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_PointCloudDetail.hpp
 * @brief  Internal helpers shared by PointCloudBVH and PointCloudHashGrid, and by their builds.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_POINTCLOUDDETAIL_HPP
#define EBGEOMETRY_POINTCLOUDDETAIL_HPP

// Std includes
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_GPUAlgorithms.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_PointCloud.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Internal helpers shared by the point-cloud search structures; not part of the public API.
 */
namespace PointCloudDetail {

/**
 * @brief Abort with a message unless a point cloud can be indexed.
 * @details Checks, once per construction, that the number of points fits the uint32 cloud indices
 * the structures store and report (with PointCloud::InvalidIndex left over as the "no point"
 * sentinel), and that every coordinate is finite. Always on rather than an EBGEOMETRY_EXPECT: each
 * would otherwise be a silently truncated cloud or a poisoned build in a Release build.
 * @tparam T Floating-point precision.
 * @param[in] a_who       Class being built, for the message.
 * @param[in] a_positions Point positions.
 */
template <class T>
inline void
requireValidCloud(const char* a_who, const std::vector<Vec3T<T>>& a_positions) noexcept
{
  EBGEOMETRY_REQUIRE(a_positions.size() < std::size_t(PointCloud::InvalidIndex),
                     "%s: too many points for 32-bit indices (%zu points, limit %zu)",
                     a_who,
                     a_positions.size(),
                     std::size_t(PointCloud::InvalidIndex) - 1);

  for (std::size_t i = 0; i < a_positions.size(); i++) {
    const Vec3T<T>& p = a_positions[i];

    EBGEOMETRY_REQUIRE(std::isfinite(p[0]) && std::isfinite(p[1]) && std::isfinite(p[2]),
                       "%s: point %zu of %zu has a non-finite coordinate",
                       a_who,
                       i,
                       a_positions.size());
  }
}

/**
 * @brief The smallest power of two at least @p a_n, and 0 for 0: how many keys a build sorts with
 * GPU::bitonicSort().
 * @details Internal helper; not part of the public API.
 * @param[in] a_n Number of points, at most 2^31.
 * @return The key count.
 */
[[nodiscard]] EBGEOMETRY_HOST
inline uint32_t
keyCount(const uint32_t a_n) noexcept
{
  uint32_t n = (a_n == 0) ? 0 : 1;

  while (n < a_n) {
    n *= 2;
  }

  return n;
}

/**
 * @brief The box of a cloud, found where a build runs, and every coordinate checked finite.
 * @details Internal helper; not part of the public API. Through GPU::parallelFor() (GPU::boundingBoxOf()), a
 * non-finite point shows only as a non-finite box, so it is reported as "a point"; on the host it is reported by
 * its index.
 * @tparam T      Floating-point precision.
 * @tparam Source The positions' source.
 * @param[in] a_source    The positions.
 * @param[in] a_numPoints Number of points.
 * @param[in] a_parallel  Whether the build runs through GPU::parallelFor().
 * @param[in] a_scratch   GPU::boundingBoxScratchSize(a_numPoints) elements the launches can write, for a build
 *                        through GPU::parallelFor(); unused otherwise.
 * @param[in] a_who       The calling function, for the message when a point is not finite.
 * @param[in] a_noun      What a point is called in that message ("point", or "sphere" for a box of centers).
 * @return The box; zeros for no points.
 */
template <class T, class Source>
[[nodiscard]] EBGEOMETRY_HOST
inline BoundingVolumes::AABBT<T>
boxOf(const Source&  a_source,
      const uint32_t a_numPoints,
      const bool     a_parallel,
      Vec3T<T>*      a_scratch,
      const char*    a_who,
      const char*    a_noun = "point")
{
  if (a_numPoints == 0) {
    return BoundingVolumes::AABBT<T>(Vec3T<T>::zeros(), Vec3T<T>::zeros());
  }

  if (a_parallel) {
    const BoundingVolumes::AABBT<T> box = GPU::boundingBoxOf<T>(a_source, a_numPoints, a_scratch);

    const Vec3T<T>& lo = box.getLowCorner();
    const Vec3T<T>& hi = box.getHighCorner();

    EBGEOMETRY_REQUIRE(std::isfinite(lo[0]) && std::isfinite(lo[1]) && std::isfinite(lo[2]) && std::isfinite(hi[0]) &&
                         std::isfinite(hi[1]) && std::isfinite(hi[2]),
                       "%s: a %s has a non-finite coordinate",
                       a_who,
                       a_noun);

    return box;
  }

  Vec3T<T> lo = +Vec3T<T>::max();
  Vec3T<T> hi = -Vec3T<T>::max();

  for (uint32_t i = 0; i < a_numPoints; i++) {
    const Vec3T<T> p = a_source(i);

    EBGEOMETRY_REQUIRE(std::isfinite(p[0]) && std::isfinite(p[1]) && std::isfinite(p[2]),
                       "%s: %s %u of %u has a non-finite coordinate",
                       a_who,
                       a_noun,
                       unsigned(i),
                       unsigned(a_numPoints));

    lo = min(lo, p);
    hi = max(hi, p);
  }

  return BoundingVolumes::AABBT<T>(lo, hi);
}

} // namespace PointCloudDetail

} // namespace EBGeometry

#endif
