// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file    EBGeometry_PointCloudHashGridBuildImplem.hpp
 * @brief   Implementation of PointCloudHashGrid::build() and rebuild(): a counting sort on the host, or a sort of
 *          cell keys through GPU::parallelFor().
 * @author  Robert Marskar
 */

#ifndef EBGEOMETRY_POINTCLOUDHASHGRIDBUILDIMPLEM_HPP
#define EBGEOMETRY_POINTCLOUDHASHGRIDBUILDIMPLEM_HPP

// Std includes
#include <cmath>
#include <cstdint>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_GPUAlgorithms.hpp"
#include "EBGeometry_GPUTransform.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_MemoryResource.hpp"
#include "EBGeometry_PointCloudDetail.hpp"
#include "EBGeometry_PointCloudHashGrid.hpp"

namespace EBGeometry {

template <class T>
template <class Source>
struct PointCloudHashGrid<T>::CellKeys
{
  /// @brief The grid, shaped: only its cell arithmetic is used.
  PointCloudHashGrid m_grid;

  /// @brief The positions.
  Source m_source;

  /// @brief Keys, a power of two of them.
  uint64_t* m_keys;

  /// @brief Number of points.
  uint32_t m_numPoints;

  /**
   * @brief Key @p a_i: the point's cell and cloud index, or a padding key past the last point.
   * @param[in] a_i Key index.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_i) const noexcept
  {
    const auto i = static_cast<uint32_t>(a_i);

    m_keys[i] = (i < m_numPoints) ? (uint64_t(m_grid.cellIndexOf(m_source(i))) << 32) | uint64_t(i)
                                  : Math::Limits<uint64_t>::max();
  }
};

template <class T>
struct PointCloudHashGrid<T>::CellStarts
{
  /// @brief Sorted keys; the first m_numPoints are the points'.
  const uint64_t* m_keys;

  /// @brief CSR offsets, one per cell plus one.
  uint32_t* m_cellStart;

  /// @brief Number of points.
  uint32_t m_numPoints;

  /**
   * @brief The start of cell @p a_cell: the first key whose cell is at least @p a_cell.
   * @param[in] a_cell Cell, 0 to the cell count (whose start is the point count).
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_cell) const noexcept
  {
    const uint64_t first = uint64_t(a_cell) << 32;

    uint32_t lo = 0;
    uint32_t hi = m_numPoints;

    while (lo < hi) {
      const uint32_t mid = lo + (hi - lo) / 2;

      if (m_keys[mid] < first) {
        lo = mid + 1;
      }
      else {
        hi = mid;
      }
    }

    m_cellStart[a_cell] = lo;
  }
};

template <class T>
template <class Source>
struct PointCloudHashGrid<T>::CellGather
{
  /// @brief Sorted keys.
  const uint64_t* m_keys;

  /// @brief The positions.
  Source m_source;

  /// @brief Positions in cell order.
  Vec3T<T>* m_positions;

  /// @brief Each slot's cloud index.
  uint32_t* m_cellPoints;

  /// @brief Each point's slot.
  uint32_t* m_slotOf;

  /**
   * @brief Slot @p a_s: its point, the point's position there, and the point's slot.
   * @param[in] a_s Slot, in cell order.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_s) const noexcept
  {
    const auto slot  = static_cast<uint32_t>(a_s);
    const auto point = static_cast<uint32_t>(m_keys[slot] & uint64_t{0xffffffff});

    m_cellPoints[slot] = point;
    m_slotOf[point]    = slot;
    m_positions[slot]  = m_source(point);
  }
};

namespace PointCloudDetail {

/**
 * @brief The most cells PointCloudHashGrid's cell-size rule gives a cloud of @p a_numPoints points.
 * @details Internal helper; not part of the public API. setShape() caps its cells at this budget.
 * @param[in] a_numPoints Number of points.
 * @return The cell budget.
 */
[[nodiscard]] EBGEOMETRY_HOST
inline std::size_t
cellBudget(const std::size_t a_numPoints) noexcept
{
  return Math::min(std::size_t(64) + std::size_t(8) * a_numPoints, std::size_t(Math::Limits<int>::max()));
}

} // namespace PointCloudDetail

template <class T>
EBGEOMETRY_HOST
inline PointCloudHashGrid<T>::PointCloudHashGrid(Pool&                        a_pool,
                                                 const std::vector<Vec3T<T>>& a_positions,
                                                 T                            a_targetPerCell)
{
  *this = build(a_pool, a_positions, a_targetPerCell);
}

template <class T>
template <class Source>
EBGEOMETRY_HOST
inline PointCloudHashGrid<T>
PointCloudHashGrid<T>::build(
  Pool& a_pool, const Source& a_source, const uint32_t a_numPoints, const T a_targetPerCell, const uint32_t a_maxPoints)
{
  static_assert(std::is_floating_point_v<T>, "PointCloudHashGrid requires a floating-point type T");
  static_assert(std::is_trivially_copyable_v<Source>,
                "PointCloudHashGrid::build: the source of positions must be trivially copyable");

  EBGEOMETRY_REQUIRE(a_targetPerCell > T(0),
                     "PointCloudHashGrid::build: the target points per cell must be positive (%g)",
                     double(a_targetPerCell));

  // The keys are padded to a power of two that is counted in 32 bits.
  const uint32_t room = Math::max(a_numPoints, a_maxPoints);

  EBGEOMETRY_REQUIRE(
    room <= (uint32_t{1} << 31), "PointCloudHashGrid::build: at most 2^31 points (%u)", unsigned(room));

  PointCloudHashGrid grid;

  grid.m_parallel = a_pool.resource().isDeviceAccessible();

  // A build through GPU::parallelFor() needs its scratch for the box, and the cells need the box, so the scratch
  // is reserved first and the grid's arrays once the cell count is known.
  if (grid.m_parallel) {
    grid.attach(a_pool, "PointCloudHashGrid", true);
    grid.m_keys.reserveSizedFrom(a_pool, PointCloudDetail::keyCount(room));
    grid.m_boxScratch.reserveSizedFrom(a_pool, GPU::boundingBoxScratchSize(room));
  }

  const BoundingVolumes::AABBT<T> box = grid.boxOf(a_source, a_numPoints, "PointCloudHashGrid::build");

  grid.setShape(box.getLowCorner(), box.getHighCorner(), a_numPoints, a_targetPerCell);

  // Room for a rebuild of up to a_maxPoints points: the most cells the rule can give them.
  const std::size_t cells = (room > a_numPoints) ? PointCloudDetail::cellBudget(room) : grid.numCells();

  grid.reserveArrays(a_pool, room, static_cast<uint32_t>(cells));
  grid.fill(a_source, a_numPoints);

  return grid;
}

template <class T>
EBGEOMETRY_HOST
inline PointCloudHashGrid<T>
PointCloudHashGrid<T>::build(Pool&                        a_pool,
                             const std::vector<Vec3T<T>>& a_positions,
                             const T                      a_targetPerCell,
                             const uint32_t               a_maxPoints)
{
  EBGEOMETRY_REQUIRE(a_positions.size() < std::size_t(PointCloud::InvalidIndex),
                     "PointCloudHashGrid::build: too many points for 32-bit indices (%zu points, limit %zu)",
                     a_positions.size(),
                     std::size_t(PointCloud::InvalidIndex) - 1);

  const auto numPoints = static_cast<uint32_t>(a_positions.size());

  // A build on the host checks every point is finite as it finds the box. A build through
  // GPU::parallelFor() checks nothing as it goes, so the points are checked here first.
  if (a_pool.resource().isDeviceAccessible()) {
    PointCloudDetail::requireValidCloud("PointCloudHashGrid::build", a_positions);
  }

  // So does one through GPU::parallelFor() in a host build (in emulated device memory). A kernel cannot read the
  // vector: the positions go to scratch in the pool's memory first, released on return.
  if (!a_pool.resource().isDeviceAccessible() || !GPU::enabled) {
    return build(a_pool, PointCloud::FromArray<T>{a_positions.data()}, numPoints, a_targetPerCell, a_maxPoints);
  }

  Pool scratch(a_pool.resource());

  PODVector<Vec3T<T>> positions;

  positions.reserveSizedFrom(scratch, numPoints);

  Vec3T<T>* data = positions.data(scratch.base());

  if (numPoints > 0) {
    a_pool.resource().copy(
      data, a_pool.resource(), a_positions.data(), hostMemoryResource(), sizeof(Vec3T<T>) * std::size_t(numPoints));
  }

  return build(a_pool, PointCloud::FromArray<T>{data}, numPoints, a_targetPerCell, a_maxPoints);
}

template <class T>
template <class Source>
EBGEOMETRY_HOST
inline void
PointCloudHashGrid<T>::rebuild(const Source& a_source, const uint32_t a_numPoints, const T a_targetPerCell)
{
  static_assert(std::is_trivially_copyable_v<Source>,
                "PointCloudHashGrid::rebuild: the source of positions must be trivially copyable");

  EBGEOMETRY_REQUIRE(a_targetPerCell > T(0),
                     "PointCloudHashGrid::rebuild: the target points per cell must be positive (%g)",
                     double(a_targetPerCell));
  EBGEOMETRY_REQUIRE(a_numPoints <= this->maxPoints(),
                     "PointCloudHashGrid::rebuild: %u points, but the grid has room for %u (see build()'s a_maxPoints)",
                     unsigned(a_numPoints),
                     unsigned(this->maxPoints()));

  const BoundingVolumes::AABBT<T> box = this->boxOf(a_source, a_numPoints, "PointCloudHashGrid::rebuild");

  this->setShape(box.getLowCorner(), box.getHighCorner(), a_numPoints, a_targetPerCell, m_cellStart.m_capacity - 1U);
  this->fill(a_source, a_numPoints);
}

template <class T>
EBGEOMETRY_HOST
inline void
PointCloudHashGrid<T>::reserveArrays(Pool& a_pool, const uint32_t a_maxPoints, const uint32_t a_maxCells)
{
  this->attach(a_pool, "PointCloudHashGrid", m_parallel);

  m_positions.reserveFrom(a_pool, a_maxPoints);
  m_cellStart.reserveFrom(a_pool, a_maxCells + 1U);
  m_cellPoints.reserveFrom(a_pool, a_maxPoints);
  m_slotOf.reserveFrom(a_pool, a_maxPoints);
}

template <class T>
template <class Source>
EBGEOMETRY_HOST
inline BoundingVolumes::AABBT<T>
PointCloudHashGrid<T>::boxOf(const Source& a_source, const uint32_t a_numPoints, const char* a_who)
{
  Vec3T<T>* scratch = m_parallel ? m_boxScratch.data(PoolHandle::base()) : nullptr;

  return PointCloudDetail::boxOf<T>(a_source, a_numPoints, m_parallel, scratch, a_who);
}

template <class T>
template <class Source>
EBGEOMETRY_HOST
inline void
PointCloudHashGrid<T>::fill(const Source& a_source, const uint32_t a_numPoints)
{
  const auto numCells = static_cast<uint32_t>(this->numCells());

  EBGEOMETRY_EXPECT(numCells < m_cellStart.m_capacity);
  EBGEOMETRY_EXPECT(a_numPoints <= m_positions.m_capacity);

  m_positions.m_size  = a_numPoints;
  m_cellStart.m_size  = numCells + 1U;
  m_cellPoints.m_size = a_numPoints;
  m_slotOf.m_size     = a_numPoints;

  void* base = PoolHandle::base();

  if (m_parallel) {
    // Every point's key (cell << 32 | cloud index), sorted: within a cell the points come in cloud-index order,
    // as the counting sort's stable scatter puts them. Then each cell finds its start, and each slot its point.
    uint64_t*      keys    = m_keys.data(base);
    const uint32_t numKeys = PointCloudDetail::keyCount(a_numPoints);

    GPU::parallelFor(numKeys, CellKeys<Source>{*this, a_source, keys, a_numPoints});
    GPU::bitonicSort(keys, numKeys);
    GPU::parallelFor(std::size_t(numCells) + 1, CellStarts{keys, m_cellStart.data(base), a_numPoints});
    GPU::parallelFor(
      a_numPoints,
      CellGather<Source>{keys, a_source, m_positions.data(base), m_cellPoints.data(base), m_slotOf.data(base)});

    return;
  }

  // Counting sort, allocating nothing: a histogram one cell up, the prefix sum (each cell's start), a stable
  // scatter in cloud-index order that uses the starts as cursors (leaving each cell's start at the next one's),
  // and a shift back by one cell.
  uint32_t* cellStart = m_cellStart.data(base);

  for (uint32_t cell = 0; cell <= numCells; cell++) {
    cellStart[cell] = 0;
  }

  for (uint32_t i = 0; i < a_numPoints; i++) {
    cellStart[this->cellIndexOf(a_source(i)) + 1]++;
  }

  for (uint32_t cell = 0; cell < numCells; cell++) {
    cellStart[cell + 1] += cellStart[cell];
  }

  // The total must equal the point count if every point was put in exactly one (in-range) cell.
  EBGEOMETRY_EXPECT(cellStart[numCells] == a_numPoints);

  Vec3T<T>* positions  = m_positions.data(base);
  uint32_t* cellPoints = m_cellPoints.data(base);
  uint32_t* slotOf     = m_slotOf.data(base);

  for (uint32_t i = 0; i < a_numPoints; i++) {
    const Vec3T<T> p    = a_source(i);
    const uint32_t slot = cellStart[this->cellIndexOf(p)]++;

    cellPoints[slot] = i;
    slotOf[i]        = slot;
    positions[slot]  = p;
  }

  for (uint32_t cell = numCells; cell > 0; cell--) {
    cellStart[cell] = cellStart[cell - 1];
  }

  cellStart[0] = 0;
}

} // namespace EBGeometry

#endif
