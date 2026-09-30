// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_PointCloudDetail.hpp
 * @brief  Internal helpers shared by PointCloudBVH and PointCloudHashGrid.
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
#include <limits>
#include <vector>

// Our includes
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Internal helpers shared by the point-cloud search structures; not part of the public API.
 */
namespace PointCloudDetail {

/**
 * @brief Abort with a message unless a point cloud can be indexed.
 * @details Checks, once per construction, that there is one metadata entry per point, that the
 * number of points fits the uint32 indices the structures store, and that every coordinate is
 * finite. Always on rather than an EBGEOMETRY_EXPECT: each would otherwise be an out-of-bounds read,
 * a silently truncated cloud, or a poisoned build in a Release build.
 * @tparam T Floating-point precision.
 * @param[in] a_who          Class being built, for the message.
 * @param[in] a_positions    Point positions.
 * @param[in] a_numMetadata  Number of metadata entries supplied.
 */
template <class T>
inline void
requireValidCloud(const char* a_who, const std::vector<Vec3T<T>>& a_positions, const std::size_t a_numMetadata) noexcept
{
  EBGEOMETRY_REQUIRE(a_numMetadata == a_positions.size(),
                     "%s: need one metadata entry per point (%zu metadata entries, %zu points)",
                     a_who,
                     a_numMetadata,
                     a_positions.size());

  EBGEOMETRY_REQUIRE(a_positions.size() <= std::size_t(std::numeric_limits<std::uint32_t>::max()),
                     "%s: too many points for 32-bit indices (%zu points, limit %zu)",
                     a_who,
                     a_positions.size(),
                     std::size_t(std::numeric_limits<std::uint32_t>::max()));

  for (std::size_t i = 0; i < a_positions.size(); i++) {
    const Vec3T<T>& p = a_positions[i];

    EBGEOMETRY_REQUIRE(std::isfinite(p[0]) && std::isfinite(p[1]) && std::isfinite(p[2]),
                       "%s: point %zu of %zu has a non-finite coordinate",
                       a_who,
                       i,
                       a_positions.size());
  }
}

} // namespace PointCloudDetail

} // namespace EBGeometry

#endif
