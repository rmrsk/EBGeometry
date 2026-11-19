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

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_PODVector.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {
namespace GPU {

/**
 * @brief Write into a_order the order to run queries at a source's points in: the permutation of 0, ..., n - 1
 * (n = a_order.size()) that sorts the points along a Morton curve over their bounding box, ties broken by index.
 * Runs on the device, with a bitonic sort, when a_where's memory is device-accessible in a CUDA or HIP build; on the
 * host, with a sort, otherwise. Both give the same order, element for element.
 * @details Queries that arrive in no spatial order make neighboring threads walk unrelated parts of a BVH or a mesh.
 * Run in this order (the ordered GPU::parallelFor() and GPU::transform()), nearby points run side by side, share
 * cache lines and diverge less, while every result still lands at its own index. That pays for queries that read
 * memory (BVHs, meshes, point clouds), not for an analytic shape, whose distance reads none. The order depends on the
 * points only: compute it once and reuse it while they stay put. Any permutation works with the ordered launches,
 * so a caller with its own space-filling curve or sort can pass its own order instead; this is a convenience.
 *
 * The points come from a source: a trivially copyable functor whose const call operator takes an index (uint32_t)
 * and returns that point's Vec3T (PointCloud::FromArray wraps an array), so a struct of particles needs no copy into
 * points first. The source and a_order are used where a_where's memory is, so they must be readable (and a_order
 * writable) there: device or managed memory for a device-accessible a_where, host memory otherwise. This is not
 * checked.
 *
 * Scratch, a 16-byte key per point (on a device rounded up to a power of two) and a few bytes for the box, is
 * allocated on a_where and released before returning. Aborts with a message unless n is at most 2^31 and every point
 * is finite.
 *
 * @code
 * PODVector<uint32_t> order;
 * order.reserveFrom(pool, n);
 * Pool device = Pool::mirror(pool, deviceResource);
 * const PODSpan<uint32_t> o = order.bind(device.base());
 *
 * GPU::mortonOrder(deviceResource, Positions{particles}, o); // once, while the particles stay put
 * GPU::parallelFor(n, query, o);                             // every time the queries run
 * @endcode
 * @tparam Source Source of the points.
 * @param[in]  a_where  The resource whose memory decides where the order is computed, and which holds the scratch.
 * @param[in]  a_source The points.
 * @param[out] a_order  The order, one entry per point.
 */
template <class Source>
EBGEOMETRY_HOST
inline void
mortonOrder(MemoryResource& a_where, const Source& a_source, PODSpan<uint32_t> a_order);

/**
 * @brief Write into a_order, an array in a_pool, the order to run queries at a source's points in; the span overload
 * on a_pool's resource, for an order reserved in a pool.
 * @details See the span overload for the order, the source and where it runs. a_pool may be frozen (a mirror),
 * since nothing is reserved in it. Aborts with a message unless a_order lies within a_pool's used bytes, and in the
 * cases the span overload aborts in.
 * @tparam Source Source of the points.
 * @param[in]  a_pool   The pool holding a_order; its resource decides where the order is computed.
 * @param[in]  a_source The points.
 * @param[out] a_order  The order, one entry per point, in a_pool.
 */
template <class Source>
EBGEOMETRY_HOST
inline void
mortonOrder(const Pool& a_pool, const Source& a_source, PODVector<uint32_t>& a_order);

} // namespace GPU
} // namespace EBGeometry

#include "EBGeometry_GPUOrderImplem.hpp"

#endif
