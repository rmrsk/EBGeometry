// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

/**
 * @file   EBGeometry_GPUOrder.hpp
 * @brief  GPU::mortonOrder(): the order to run queries at points in, so that queries at nearby points run side by
 *         side (see the ordered GPU::parallelFor() and GPU::transform()).
 * @details Queries that arrive in no spatial order (particles of a simulation, points sampled at random) make
 * neighboring threads of a device walk unrelated parts of a BVH, and a host loop jump around in memory. Sorting the
 * points along a Morton curve over their bounding box puts nearby points next to each other; the ordered launches
 * then run the queries in that order while every result still lands at its input's index. The order depends on
 * the points only, so it is computed once and reused while they stay put.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_GPUORDER_HPP
#define EBGEOMETRY_GPUORDER_HPP

// Std includes
#include <cstdint>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_PODVector.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {
namespace GPU {

/**
 * @brief The order to run queries at a source's points in: the permutation of 0, ..., a_count - 1 that sorts the
 * points along a Morton curve over their bounding box, ties broken by index. Computed where the pool's memory is.
 * @details The points come from a source: a trivially copyable functor whose const call operator takes an index
 * (uint32_t) and returns that point's Vec3T, as PointCloudBVH::build() takes (PointCloud::FromArray wraps an
 * array). It reads them from the caller's own data, so a struct of particles or an array of coordinates needs no
 * copy into points first. For a pool in device-accessible memory the order is computed by launches (on the device
 * in a CUDA or HIP build, so the source must be callable there and its data device-readable); for any other pool,
 * on the host. Both give the same order, element for element, as does the std::vector overload. The order is
 * written into an array the caller reserved, so a frozen pool (a mirror) will do; the scratch (a key per point,
 * rounded up to a power of two on a device, and the box's) is reserved in a temporary pool on the pool's resource
 * and released before returning. Aborts with a message unless a_order has a_count entries ending inside the pool's
 * used bytes, a_count is at most 2^31, and every point is finite.
 * @tparam Source Source of the points.
 * @param[in] a_pool   The pool holding a_order; its resource decides where the order is computed.
 * @param[in] a_source The points.
 * @param[in] a_count  Number of points.
 * @param[in] a_order  The order's array: a_count entries in a_pool, which are written.
 */
template <class Source>
EBGEOMETRY_HOST
inline void
mortonOrder(const Pool& a_pool, const Source& a_source, uint32_t a_count, const PODVector<uint32_t>& a_order);

/**
 * @brief The order to run queries at these points in, computed on the host; see the pool overload.
 * @tparam T Precision.
 * @param[in] a_points The points; at most 2^31, all finite.
 * @return The order: a permutation of the points' indices.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST
inline std::vector<uint32_t>
mortonOrder(const std::vector<Vec3T<T>>& a_points);

} // namespace GPU
} // namespace EBGeometry

#include "EBGeometry_GPUOrderImplem.hpp"

#endif
