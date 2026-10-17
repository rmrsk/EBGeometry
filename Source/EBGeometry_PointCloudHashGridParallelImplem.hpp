// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file    EBGeometry_PointCloudHashGridParallelImplem.hpp
 * @brief   Implementation of PointCloudHashGrid::buildParallel(): the constructor's grid, built with
 *          GPU::parallelFor().
 * @author  Robert Marskar
 */

#ifndef EBGEOMETRY_POINTCLOUDHASHGRIDPARALLELIMPLEM_HPP
#define EBGEOMETRY_POINTCLOUDHASHGRIDPARALLELIMPLEM_HPP

// Std includes
#include <cstdint>
#include <vector>

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_GPUAlgorithms.hpp"
#include "EBGeometry_GPUTransform.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_PointCloudDetail.hpp"
#include "EBGeometry_PointCloudHashGrid.hpp"

namespace EBGeometry {

template <class T>
struct PointCloudHashGrid<T>::CellKeys
{
  /// @brief The grid, shaped but with no arrays: only its cell arithmetic is used.
  PointCloudHashGrid m_grid;

  /// @brief Point positions.
  const Vec3T<T>* m_positions;

  /// @brief Keys, a power of two of them.
  uint64_t* m_keys;

  /// @brief Number of points.
  uint32_t m_numPoints;

  /**
   * @brief Key @p a_i.
   * @param[in] a_i Key index.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_i) const noexcept
  {
    const auto i = static_cast<uint32_t>(a_i);

    m_keys[i] = (i < m_numPoints) ? (uint64_t(m_grid.cellIndexOf(m_positions[i])) << 32) | uint64_t(i) : UINT64_MAX;
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
struct PointCloudHashGrid<T>::CellPoints
{
  /// @brief Sorted keys.
  const uint64_t* m_keys;

  /// @brief Cloud indices in cell order.
  uint32_t* m_cellPoints;

  /**
   * @brief Sorted point @p a_s.
   * @param[in] a_s Position in cell order.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_s) const noexcept
  {
    m_cellPoints[a_s] = static_cast<uint32_t>(m_keys[a_s] & uint64_t{0xffffffff});
  }
};

template <class T>
EBGEOMETRY_HOST
inline PointCloudHashGrid<T>
PointCloudHashGrid<T>::buildParallel(Pool& a_pool, const std::vector<Vec3T<T>>& a_positions, T a_targetPerCell)
{
  static_assert(std::is_floating_point_v<T>, "PointCloudHashGrid requires a floating-point type T");

  PointCloudDetail::requireValidCloud("PointCloudHashGrid::buildParallel", a_positions);

  EBGEOMETRY_REQUIRE(a_targetPerCell > T(0),
                     "PointCloudHashGrid::buildParallel: the target points per cell must be positive (%g)",
                     double(a_targetPerCell));

  // The keys are padded to a power of two that is counted in 32 bits.
  EBGEOMETRY_REQUIRE(a_positions.size() <= (std::size_t{1} << 31),
                     "PointCloudHashGrid::buildParallel: at most 2^31 points (%zu)",
                     a_positions.size());

  // The positions are copied in from the host, the box is read back, and kernels write the rest.
  EBGEOMETRY_REQUIRE(a_pool.resource().isHostAccessible() && (!GPU::enabled || a_pool.resource().isDeviceAccessible()),
                     "PointCloudHashGrid::buildParallel: the pool must be host-accessible, and device-accessible "
                     "as well in a %s build (managed memory)",
                     GPU::backendName);

  const auto numPoints = static_cast<uint32_t>(a_positions.size());

  uint32_t numKeys = (numPoints == 0) ? 0 : 1;

  while (numKeys < numPoints) {
    numKeys *= 2;
  }

  // The scratch arrays, all reserved before the scratch pool's base is resolved. The positions go to
  // scratch first: the grid's own arrays cannot be sized before the box gives the cell count.
  Pool scratch(a_pool.resource());

  PODVector<Vec3T<T>> scratchPositions;
  PODVector<Vec3T<T>> boxScratch;
  PODVector<uint64_t> keys;

  scratchPositions.reserveFrom(scratch, numPoints);
  boxScratch.reserveSizedFrom(scratch, GPU::boundingBoxScratchSize(numPoints));
  keys.reserveSizedFrom(scratch, numKeys);

  void* scratchBase = scratch.base();

  scratchPositions.assign(scratchBase, a_positions.data(), numPoints);

  const Vec3T<T>* positions = scratchPositions.data(static_cast<const void*>(scratchBase));

  const BoundingVolumes::AABBT<T> box = GPU::boundingBox(positions, numPoints, boxScratch.data(scratchBase));

  PointCloudHashGrid grid(box.getLowCorner(), box.getHighCorner(), numPoints, a_targetPerCell);

  // setShape() caps the cell count at Limits<int>::max(), so it fits 32 bits with one to spare for
  // the end offset.
  const auto numCells = static_cast<uint32_t>(grid.numCells());

  // The grid's arrays, all reserved before the pool's base is resolved.
  grid.attach(a_pool, "PointCloudHashGrid");

  grid.m_positions.reserveFrom(a_pool, numPoints);
  grid.m_cellStart.reserveSizedFrom(a_pool, numCells + 1);
  grid.m_cellPoints.reserveSizedFrom(a_pool, numPoints);

  void* base = a_pool.base();

  grid.m_positions.assign(base, a_positions.data(), numPoints);

  uint64_t* keyData = keys.data(scratchBase);

  GPU::parallelFor(numKeys, CellKeys{grid, positions, keyData, numPoints});
  GPU::bitonicSort(keyData, numKeys);
  GPU::parallelFor(std::size_t{numCells} + 1, CellStarts{keyData, grid.m_cellStart.data(base), numPoints});
  GPU::parallelFor(numPoints, CellPoints{keyData, grid.m_cellPoints.data(base)});

  return grid;
}

} // namespace EBGeometry

#endif
