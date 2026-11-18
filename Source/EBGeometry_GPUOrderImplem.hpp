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
 * @brief A point's sort key: its Morton code, then its index, so that no two keys are equal and every sort gives
 * the one order.
 */
struct Key
{
  /// @brief The Morton code of the point's cell.
  uint64_t m_code;

  /// @brief The point's index.
  uint32_t m_index;

  /**
   * @brief Whether this key sorts before @p a_other: by code, then by index.
   * @param[in] a_other The other key.
   * @return True if this key comes first.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  operator<(const Key& a_other) const noexcept
  {
    return m_code < a_other.m_code || (m_code == a_other.m_code && m_index < a_other.m_index);
  }
};

/**
 * @brief The Morton grid over a box: a point's cell is (p / 2 - lo / 2) * scale, clamped to [0, SFC::ValidSpan].
 * @details The coordinates are halved before subtracting, so that a box wider than the largest value of T still
 * has a finite extent; halving is exact, so the host and a device compute the same cells.
 * @tparam T Precision.
 */
template <class T>
struct Grid
{
  /// @brief Half the box's lower corner.
  Vec3T<T> m_halfLo;

  /// @brief Cells per unit of halved length (0 for a box of one point).
  T m_scale;

  /**
   * @brief The grid over a box: cubic cells, spanning its longest side.
   * @param[in] a_lo The box's lower corner.
   * @param[in] a_hi The box's upper corner.
   * @return The grid.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static Grid
  over(const Vec3T<T>& a_lo, const Vec3T<T>& a_hi) noexcept
  {
    const Vec3T<T> halfLo = T(0.5) * a_lo;
    const Vec3T<T> extent = T(0.5) * a_hi - halfLo;
    const T        side   = Math::max(extent[0], Math::max(extent[1], extent[2]));

    return Grid{halfLo, (side > T(0)) ? T(SFC::ValidSpan) / side : T(0)};
  }

  /**
   * @brief A point's key.
   * @param[in] a_point The point.
   * @param[in] a_index Its index.
   * @return The key.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  Key
  key(const Vec3T<T>& a_point, const uint32_t a_index) const noexcept
  {
    SFC::Index cell;

    for (int dir = 0; dir < 3; dir++) {
      const T scaled = (T(0.5) * a_point[dir] - m_halfLo[dir]) * m_scale;

      cell[dir] = static_cast<unsigned int>(Math::clamp(scaled, T(0), T(SFC::ValidSpan)));
    }

    return Key{SFC::Morton::encode(cell), a_index};
  }
};

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
  Grid<T> m_grid;

  /// @brief The keys.
  Key* m_keys;

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

    m_keys[i] = (i < m_count) ? m_grid.key(m_source(i), i) : Key{UINT64_MAX, UINT32_MAX};
  }
};

/**
 * @brief Launch: the order, read off the sorted keys.
 */
struct ReadOrder
{
  /// @brief The sorted keys.
  const Key* m_keys;

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

  const Grid<T> grid = Grid<T>::over(lo, hi);

  std::vector<Key> keys(a_count);

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

  if (!a_pool.resource().isDeviceAccessible()) {
    OrderDetail::onHost<T>(a_source, a_count, order);

    return;
  }

  // Where the launches run: the box, the keys padded to a power of two, a bitonic sort, the order read off them.
  uint32_t padded = 1;

  while (padded < a_count) {
    padded *= 2;
  }

  Pool scratch(a_pool.resource());

  PODVector<OrderDetail::Key> keys;
  PODVector<Vec3T<T>>         corners;

  keys.reserveSizedFrom(scratch, padded);
  corners.reserveSizedFrom(scratch, boundingBoxScratchSize(a_count));

  void* base = scratch.base();

  const BoundingVolumes::AABBT<T> box = boundingBoxOf<T>(a_source, a_count, corners.data(base));

  OrderDetail::requireFinite(box.getLowCorner(), box.getHighCorner());

  const OrderDetail::Grid<T> grid = OrderDetail::Grid<T>::over(box.getLowCorner(), box.getHighCorner());

  OrderDetail::Key* sorted = keys.data(base);

  parallelFor(padded, OrderDetail::MakeKeys<T, Source>{a_source, grid, sorted, a_count});
  bitonicSort(sorted, padded);
  parallelFor(a_count, OrderDetail::ReadOrder{sorted, order});
}

template <class T>
inline std::vector<uint32_t>
mortonOrder(const std::vector<Vec3T<T>>& a_points)
{
  EBGEOMETRY_REQUIRE(
    a_points.size() <= (std::size_t(1) << 31), "GPU::mortonOrder: at most 2^31 points (%zu)", a_points.size());

  const auto count = static_cast<uint32_t>(a_points.size());

  std::vector<uint32_t> order(count);

  if (count > 0) {
    OrderDetail::onHost<T>(AlgorithmsDetail::FromPointer<T>{a_points.data()}, count, order.data());
  }

  return order;
}

} // namespace GPU
} // namespace EBGeometry

#endif
