// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

/**
 * @file   EBGeometry_GPUOrderImplem.hpp
 * @brief  Implementation of EBGeometry_GPUOrder.hpp.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_GPUORDERIMPLEM_HPP
#define EBGEOMETRY_GPUORDERIMPLEM_HPP

// Std includes
#include <algorithm>
#include <cmath>
#include <vector>

// Our includes
#include "EBGeometry_GPUAlgorithms.hpp"
#include "EBGeometry_GPUOrder.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_SFC.hpp"

namespace EBGeometry {
namespace GPU {

/**
 * @brief Implementation details of GPU::mortonOrder().
 */
namespace OrderDetail {

/**
 * @brief The precision of a source's points.
 * @tparam V The type a source returns.
 */
template <class V>
struct PrecisionOf;

/**
 * @brief The precision of a Vec3T.
 * @tparam T Precision.
 */
template <class T>
struct PrecisionOf<Vec3T<T>>
{
  /// @brief The precision.
  using type = T;
};

/**
 * @brief The precision of a source's points.
 * @tparam Source Source type.
 */
template <class Source>
using SourcePrecision = typename PrecisionOf<std::decay_t<std::invoke_result_t<const Source&, uint32_t>>>::type;

/**
 * @brief Launch: the keys of the points, and keys that sort last past the last point (padding to a power of two).
 * @tparam T      Precision.
 * @tparam Source Source type.
 */
template <class T, class Source>
struct MakeKeys
{
  /// @brief The points.
  Source m_source;

  /// @brief The grid.
  SFCDetail::MortonGrid<T> m_grid;

  /// @brief The keys.
  SFCDetail::MortonKey* m_keys;

  /// @brief Number of points.
  uint32_t m_count;

  /**
   * @brief Write key i.
   * @param[in] a_i Key index.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_i) const
  {
    const auto i = static_cast<uint32_t>(a_i);

    m_keys[i] = (i < m_count) ? m_grid.key(m_source(i), i) : SFCDetail::MortonKey{UINT64_MAX, UINT32_MAX};
  }
};

/**
 * @brief Launch: the order, read off the sorted keys.
 */
struct ReadOrder
{
  /// @brief The sorted keys.
  const SFCDetail::MortonKey* m_keys;

  /// @brief The order.
  uint32_t* m_order;

  /**
   * @brief Write entry i of the order.
   * @param[in] a_i Position in the order.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_i) const
  {
    m_order[a_i] = m_keys[a_i].m_index;
  }
};

/**
 * @brief Abort unless a box's corners are finite, as they are when every point is.
 * @tparam T Precision.
 * @param[in] a_lo Lower corner.
 * @param[in] a_hi Upper corner.
 */
template <class T>
EBGEOMETRY_HOST
inline void
requireFinite(const Vec3T<T>& a_lo, const Vec3T<T>& a_hi)
{
  bool finite = true;

  for (int dir = 0; dir < 3; dir++) {
    finite = finite && std::isfinite(a_lo[dir]) && std::isfinite(a_hi[dir]);
  }

  EBGEOMETRY_REQUIRE(finite, "GPU::mortonOrder: every point must be finite");
}

/**
 * @brief The order on the host, into a_order.
 * @tparam T      Precision.
 * @tparam Source Source type.
 * @param[in]  a_source The points.
 * @param[in]  a_count  Number of points.
 * @param[out] a_order  The order: a_count entries.
 */
template <class T, class Source>
EBGEOMETRY_HOST
inline void
onHost(const Source& a_source, const uint32_t a_count, uint32_t* a_order)
{
  Vec3T<T> lo = a_source(0);
  Vec3T<T> hi = lo;

  for (uint32_t i = 1; i < a_count; i++) {
    const Vec3T<T> p = a_source(i);

    lo = min(lo, p);
    hi = max(hi, p);
  }

  // min() and max() may drop a NaN, so each point is checked too.
  for (uint32_t i = 0; i < a_count; i++) {
    const Vec3T<T> p = a_source(i);

    requireFinite(p, p);
  }

  requireFinite(lo, hi);

  const SFCDetail::MortonGrid<T> grid = SFCDetail::MortonGrid<T>::over(lo, hi);

  std::vector<SFCDetail::MortonKey> keys(a_count);

  for (uint32_t i = 0; i < a_count; i++) {
    keys[i] = grid.key(a_source(i), i);
  }

  std::sort(keys.begin(), keys.end());

  for (uint32_t i = 0; i < a_count; i++) {
    a_order[i] = keys[i].m_index;
  }
}

} // namespace OrderDetail

template <class Source>
inline void
mortonOrder(const Pool& a_pool, const Source& a_source, const uint32_t a_count, const PODVector<uint32_t>& a_order)
{
  using T = OrderDetail::SourcePrecision<Source>;

  static_assert(std::is_trivially_copyable_v<Source>, "GPU::mortonOrder: the source must be trivially copyable");

  EBGEOMETRY_REQUIRE(a_count <= (uint32_t(1) << 31), "GPU::mortonOrder: at most 2^31 points (%u)", unsigned(a_count));
  EBGEOMETRY_REQUIRE(a_order.size() == a_count && a_order.endByte() <= a_pool.usedBytes(),
                     "GPU::mortonOrder: the order must have one entry per point (%u points, %u entries) and lie in "
                     "the pool",
                     unsigned(a_count),
                     unsigned(a_order.size()));

  if (a_count == 0) {
    return;
  }

  uint32_t* order = a_order.data(a_pool.base());

  // A host build's launches are host loops, where a sort of the keys beats a bitonic network.
  if (!enabled || !a_pool.resource().isDeviceAccessible()) {
    OrderDetail::onHost<T>(a_source, a_count, order);

    return;
  }

  // Where the launches run: the box, the keys padded to a power of two, a bitonic sort, the order read off them.
  uint32_t padded = 1;

  while (padded < a_count) {
    padded *= 2;
  }

  Pool scratch(a_pool.resource());

  PODVector<SFCDetail::MortonKey> keys;
  PODVector<Vec3T<T>>             corners;

  keys.reserveSizedFrom(scratch, padded);
  corners.reserveSizedFrom(scratch, boundingBoxScratchSize(a_count));

  void* base = scratch.base();

  const BoundingVolumes::AABBT<T> box = boundingBoxOf<T>(a_source, a_count, corners.data(base));

  // boundingBoxOf() gives a box with a corner at -infinity when any point is not finite, so this checks every point.
  OrderDetail::requireFinite(box.getLowCorner(), box.getHighCorner());

  const SFCDetail::MortonGrid<T> grid = SFCDetail::MortonGrid<T>::over(box.getLowCorner(), box.getHighCorner());

  SFCDetail::MortonKey* sorted = keys.data(base);

  parallelFor(padded, OrderDetail::MakeKeys<T, Source>{a_source, grid, sorted, a_count});
  bitonicSort(sorted, padded);
  parallelFor(a_count, OrderDetail::ReadOrder{sorted, order});
}

} // namespace GPU
} // namespace EBGeometry

#endif
