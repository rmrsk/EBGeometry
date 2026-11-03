// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file    EBGeometry_PointCloudHashGridImplem.hpp
 * @brief   Implementation of EBGeometry_PointCloudHashGrid.hpp
 * @author  Robert Marskar
 */

#ifndef EBGEOMETRY_POINTCLOUDHASHGRIDIMPLEM_HPP
#define EBGEOMETRY_POINTCLOUDHASHGRIDIMPLEM_HPP

// Std includes
#include <algorithm>
#include <cmath>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_PointCloud.hpp"
#include "EBGeometry_PointCloudDetail.hpp"
#include "EBGeometry_PointCloudHashGrid.hpp"

namespace EBGeometry {

template <class T>
EBGEOMETRY_HOST
inline void
PointCloudHashGrid<T>::setShape(const Vec3T<T>&   a_lo,
                                const Vec3T<T>&   a_hi,
                                const std::size_t a_numPoints,
                                const T           a_targetPerCell,
                                const std::size_t a_cellCap) noexcept
{
  const std::size_t numPoints = a_numPoints;

  m_lo        = a_lo;
  Vec3T<T> hi = a_hi;

  if (numPoints == 0) {
    m_lo = Vec3T<T>::zeros();
    hi   = Vec3T<T>::zeros();
  }

  // Cell size from the target average occupancy: h = cbrt(bboxVolume / N * target). Fall back to a
  // characteristic length when the cloud is degenerate (zero-volume box or empty).
  const Vec3T<T> ext = hi - m_lo;
  const T        vol = ext[0] * ext[1] * ext[2];

  if (vol > T(0) && numPoints > 0) {
    m_h = std::cbrt(vol / T(numPoints) * a_targetPerCell);
  }
  else {
    const T maxExt = Math::max(ext[0], Math::max(ext[1], ext[2]));

    m_h = (maxExt > T(0) && numPoints > 0) ? maxExt / std::cbrt(T(numPoints)) : T(1);
  }

  if (!(m_h > T(0))) {
    m_h = T(1);
  }

  m_invH = T(1) / m_h;

  // Cap the dense grid size. m_h is sized to the cloud's *average* density over the whole bounding
  // box, so a small a_targetPerCell -- or an anisotropic box (a tight cluster plus a single distant
  // outlier stretches the box along one axis while the local spacing stays tiny) -- can drive the
  // per-axis cell counts, and their product, arbitrarily large: an unbounded m_cellStart allocation,
  // or undefined behavior from the int() cast below overflowing. Coarsen m_h until the *actual*
  // per-axis cell counts multiply to within a budget proportional to the point count. The counts are
  // evaluated in double so the int casts below cannot overflow, and each cube-root step shrinks the
  // product geometrically so the loop converges in a few iterations (the bound is a pure safety net).
  // The budget is also capped to INT_MAX so no single axis dimension can overflow int. Coarsening
  // changes only grid resolution, never query results (the query is exact for any cell size).
  // A rebuild also stays within the cells it has room for.
  const std::size_t maxCells = Math::min(
    Math::min(std::size_t(64) + std::size_t(8) * numPoints, std::size_t(Math::Limits<int>::max())), a_cellCap);

  const auto predictedCells = [&]() noexcept -> double {
    const double dx = std::floor(double(hi[0] - m_lo[0]) * double(m_invH)) + 1.0;
    const double dy = std::floor(double(hi[1] - m_lo[1]) * double(m_invH)) + 1.0;
    const double dz = std::floor(double(hi[2] - m_lo[2]) * double(m_invH)) + 1.0;

    return dx * dy * dz;
  };

  for (int guard = 0; guard < 100; guard++) {
    const double predicted = predictedCells();

    if (predicted <= double(maxCells)) {
      break;
    }

    m_h *= static_cast<T>(std::cbrt(predicted / double(maxCells)));
    m_invH = T(1) / m_h;
  }

  EBGEOMETRY_EXPECT(m_h > T(0));
  EBGEOMETRY_EXPECT(predictedCells() <= double(maxCells));

  // Derive the integer dimensions from the same double-valued counts used for the cap, so the product
  // is guaranteed to match the budget check above and the int() casts cannot overflow.
  m_nx = Math::max(1, int(std::floor(double(hi[0] - m_lo[0]) * double(m_invH)) + 1.0));
  m_ny = Math::max(1, int(std::floor(double(hi[1] - m_lo[1]) * double(m_invH)) + 1.0));
  m_nz = Math::max(1, int(std::floor(double(hi[2] - m_lo[2]) * double(m_invH)) + 1.0));

  EBGEOMETRY_EXPECT(m_nx >= 1 && m_ny >= 1 && m_nz >= 1);
  EBGEOMETRY_EXPECT(this->numCells() <= maxCells);
}

template <class T>
EBGEOMETRY_HOST
inline PointCloudHashGrid<T>
PointCloudHashGrid<T>::rebasedView(const Pool& a_pool) const noexcept
{
  const uint64_t endByte = Math::max(Math::max(Math::max(m_positions.endByte(), m_cellStart.endByte()),
                                               Math::max(m_cellPoints.endByte(), m_slotOf.endByte())),
                                     Math::max(m_keys.endByte(), m_boxScratch.endByte()));

  PointCloudHashGrid view = *this;

  view.setLocation(this->rebasedLocation(a_pool, endByte, "PointCloudHashGrid"));

  return view;
}

template <class T>
EBGEOMETRY_HOST
inline PointCloudHashGrid<T>
PointCloudHashGrid<T>::deepCopy(Pool& a_dstPool) const
{
  // Read everything out before reserving anything: a_dstPool may be this object's own pool, and a
  // reserve there can move the block under any address taken from it. The cells are read by slot, so
  // the copy holds the same arrays in the same order.
  const void* srcBase = this->base();

  const PODSpan<const Vec3T<T>>      positions  = m_positions.bind(srcBase);
  const PODSpan<const std::uint32_t> cellStart  = m_cellStart.bind(srcBase);
  const PODSpan<const std::uint32_t> cellPoints = m_cellPoints.bind(srcBase);
  const PODSpan<const std::uint32_t> slotOf     = m_slotOf.bind(srcBase);

  const std::vector<Vec3T<T>>      hostPositions(positions.begin(), positions.end());
  const std::vector<std::uint32_t> hostCellStart(cellStart.begin(), cellStart.end());
  const std::vector<std::uint32_t> hostCellPoints(cellPoints.begin(), cellPoints.end());
  const std::vector<std::uint32_t> hostSlotOf(slotOf.begin(), slotOf.end());

  PointCloudHashGrid copy = *this;

  copy.setLocation(PoolLocation{});

  copy.m_positions  = PODVector<Vec3T<T>>{};
  copy.m_cellStart  = PODVector<std::uint32_t>{};
  copy.m_cellPoints = PODVector<std::uint32_t>{};
  copy.m_slotOf     = PODVector<std::uint32_t>{};
  copy.m_keys       = PODVector<std::uint64_t>{};
  copy.m_boxScratch = PODVector<Vec3T<T>>{};
  copy.m_parallel   = false;

  copy.reserveArrays(
    a_dstPool, static_cast<uint32_t>(hostPositions.size()), static_cast<uint32_t>(hostCellStart.size()) - 1U);

  void* dstBase = copy.base();

  copy.m_positions.assign(dstBase, hostPositions.data(), static_cast<uint32_t>(hostPositions.size()));
  copy.m_cellStart.assign(dstBase, hostCellStart.data(), static_cast<uint32_t>(hostCellStart.size()));
  copy.m_cellPoints.assign(dstBase, hostCellPoints.data(), static_cast<uint32_t>(hostCellPoints.size()));
  copy.m_slotOf.assign(dstBase, hostSlotOf.data(), static_cast<uint32_t>(hostSlotOf.size()));

  return copy;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline int
PointCloudHashGrid<T>::cellCoord(T a_x, T a_lo, int a_n) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_x));
  EBGEOMETRY_EXPECT(a_n >= 1);

  // Clamp in floating point before converting: a query far outside the grid would otherwise
  // overflow the int conversion (undefined behavior, and in practice a cell on the wrong side of
  // the grid). A NaN coordinate fails both comparisons and lands in cell 0.
  const T t = std::floor((a_x - a_lo) * m_invH);

  if (!(t > T(0))) {
    return 0;
  }

  return (t >= T(a_n - 1)) ? a_n - 1 : int(t);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline std::size_t
PointCloudHashGrid<T>::cellIndex(int a_ix, int a_iy, int a_iz) const noexcept
{
  EBGEOMETRY_EXPECT(a_ix >= 0 && a_ix < m_nx);
  EBGEOMETRY_EXPECT(a_iy >= 0 && a_iy < m_ny);
  EBGEOMETRY_EXPECT(a_iz >= 0 && a_iz < m_nz);

  return std::size_t(a_ix) + std::size_t(m_nx) * (std::size_t(a_iy) + std::size_t(m_ny) * std::size_t(a_iz));
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline std::size_t
PointCloudHashGrid<T>::cellIndexOf(const Vec3T<T>& a_p) const noexcept
{
  return cellIndex(
    cellCoord(a_p[0], m_lo[0], m_nx), cellCoord(a_p[1], m_lo[1], m_ny), cellCoord(a_p[2], m_lo[2], m_nz));
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline std::size_t
PointCloudHashGrid<T>::query(const Vec3T<T>& a_query, std::size_t a_k, Hit* a_out, uint32_t a_exclude) const noexcept
{
  EBGEOMETRY_EXPECT(a_k >= 1);
  EBGEOMETRY_EXPECT(a_out != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_query[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_query[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_query[2]));
  EBGEOMETRY_EXPECT(a_exclude == PointCloud::InvalidIndex || a_exclude < m_positions.size());

  // Each point lives in exactly one cell and each cell is visited in exactly one shell, so every
  // point is offered once and the set needs no de-duplication.
  PointCloud::KBest<T> best(a_out, a_k, a_exclude);

  if (m_positions.empty()) {
    return 0;
  }

  // Resolved once per query: a query reserves nothing, so the pool block cannot move under them.
  const void*                        poolBase   = this->base();
  const PODSpan<const Vec3T<T>>      positions  = m_positions.bind(poolBase);
  const PODSpan<const std::uint32_t> cellStart  = m_cellStart.bind(poolBase);
  const PODSpan<const std::uint32_t> cellPoints = m_cellPoints.bind(poolBase);

  const int cx = cellCoord(a_query[0], m_lo[0], m_nx);
  const int cy = cellCoord(a_query[1], m_lo[1], m_ny);
  const int cz = cellCoord(a_query[2], m_lo[2], m_nz);

  // Largest shell radius that still adds cells (beyond it the whole grid is searched).
  const int rMax =
    Math::max(Math::max(cx, m_nx - 1 - cx), Math::max(Math::max(cy, m_ny - 1 - cy), Math::max(cz, m_nz - 1 - cz)));

  // Rounding slack for the stopping rule below: enough ulps of the largest magnitude involved in
  // locating a cell face to cover the rounding in cellCoord() and in the face itself.
  const T eps = Math::Limits<T>::epsilon();

  for (int r = 0; r <= rMax; r++) {
    // Visit only the new shell at Chebyshev radius r (cells with max(|dx|,|dy|,|dz|) == r).
    const int xlo = Math::max(0, cx - r), xhi = Math::min(m_nx - 1, cx + r);
    const int ylo = Math::max(0, cy - r), yhi = Math::min(m_ny - 1, cy + r);
    const int zlo = Math::max(0, cz - r), zhi = Math::min(m_nz - 1, cz + r);

    for (int iz = zlo; iz <= zhi; iz++) {
      const bool zEdge = (iz == cz - r) || (iz == cz + r);

      for (int iy = ylo; iy <= yhi; iy++) {
        const bool yEdge = (iy == cy - r) || (iy == cy + r);

        // On the interior of the shell only the two x-faces are new; on the shell's own faces the
        // whole x-row is new.
        const bool wholeRow = zEdge || yEdge;

        for (int ix = xlo; ix <= xhi; ix++) {
          if (!wholeRow && ix != cx - r && ix != cx + r) {
            continue;
          }

          const std::size_t   cid   = cellIndex(ix, iy, iz);
          const std::uint32_t begin = cellStart[static_cast<std::uint32_t>(cid)];
          const std::uint32_t end   = cellStart[static_cast<std::uint32_t>(cid + 1)];

          // A cell's points sit in consecutive slots, positions and cloud indices alike.
          for (std::uint32_t s = begin; s < end; s++) {
            best.insert((positions[s] - a_query).length2(), cellPoints[s]);
          }
        }
      }
    }

    // Exact stopping rule. After searching the cell box [cx-r, cx+r] x ... (clamped), every point
    // still unvisited lies in a cell outside it, so along some axis it is beyond one of the box's
    // faces on a side where cells remain (a side whose clamped extent reached the grid edge has none:
    // no points are bucketed beyond the grid). Its distance to the query is therefore at least the
    // distance from the query to the nearest such face. Stop once k points are held and that bound
    // is not closer than the k-th.
    //
    // The faces are the true cell boundaries. Cell membership is decided in cellCoord() by rounding
    // (x - lo) * (1/h), so a point can sit a few ulps on the wrong side of a reconstructed face; the
    // slack subtracted below covers that, and the final comparison leaves room for the rounding in
    // the squared distances. Both are far below a cell for any realistic grid, so the search stops
    // at the first shell whose faces clear the k-th distance -- often r = 0.
    if (best.found() == a_k) {
      const T inf   = Math::Limits<T>::max();
      T       bound = inf;

      for (int axis = 0; axis < 3; axis++) {
        const int c     = (axis == 0) ? cx : (axis == 1) ? cy : cz;
        const int nAxis = (axis == 0) ? m_nx : (axis == 1) ? m_ny : m_nz;
        const T   lo    = m_lo[axis];
        const T   q     = a_query[axis];
        const T   slack = T(8) * eps * (std::abs(lo) + std::abs(q) + T(nAxis) * m_h);

        // Lower side: unvisited cells exist iff c-r > 0; the searched box starts at cell c-r.
        if (c - r > 0) {
          bound = Math::min(bound, q - (lo + T(c - r) * m_h) - slack);
        }

        // Upper side: unvisited cells exist iff c+r < nAxis-1; the searched box ends at the top of
        // cell c+r.
        if (c + r < nAxis - 1) {
          bound = Math::min(bound, (lo + T(c + r + 1) * m_h) - q - slack);
        }
      }

      if (bound == inf || (bound > T(0) && best.bound() <= bound * bound * (T(1) - T(16) * eps))) {
        break;
      }
    }
  }

  return best.found();
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline typename PointCloudHashGrid<T>::Hit
PointCloudHashGrid<T>::closestPoint(const Vec3T<T>& a_query) const noexcept
{
  Hit hit;

  this->query(a_query, 1, &hit, PointCloud::InvalidIndex);

  return hit;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline std::size_t
PointCloudHashGrid<T>::closestPoints(const Vec3T<T>& a_query, std::size_t a_k, Hit* a_out) const noexcept
{
  EBGEOMETRY_EXPECT(a_k >= 1);
  EBGEOMETRY_EXPECT(a_out != nullptr);

  return this->query(a_query, a_k, a_out, PointCloud::InvalidIndex);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline typename PointCloudHashGrid<T>::Hit
PointCloudHashGrid<T>::nearestNeighbor(uint32_t a_point) const noexcept
{
  EBGEOMETRY_EXPECT(a_point < m_positions.size());

  Hit hit;

  this->query(this->position(a_point), 1, &hit, a_point);

  return hit;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline std::size_t
PointCloudHashGrid<T>::nearestNeighbors(uint32_t a_point, std::size_t a_k, Hit* a_out) const noexcept
{
  EBGEOMETRY_EXPECT(a_point < m_positions.size());
  EBGEOMETRY_EXPECT(a_k >= 1);
  EBGEOMETRY_EXPECT(a_out != nullptr);

  return this->query(this->position(a_point), a_k, a_out, a_point);
}

template <class T>
EBGEOMETRY_HOST
inline std::vector<typename PointCloudHashGrid<T>::Hit>
PointCloudHashGrid<T>::allNearestNeighbors(std::size_t a_k) const
{
  EBGEOMETRY_EXPECT(a_k >= 1);

  const std::size_t numPoints = m_positions.size();
  const void*       poolBase  = this->base();

  std::vector<Hit> result(numPoints * a_k);

  // Process points in cell (spatial) order -- consecutive queries touch nearby cells, staying hot in
  // cache. The slots are in cell order.
  const PODSpan<const Vec3T<T>>      positions  = m_positions.bind(poolBase);
  const PODSpan<const std::uint32_t> cellPoints = m_cellPoints.bind(poolBase);

  for (std::uint32_t s = 0; s < static_cast<std::uint32_t>(numPoints); s++) {
    const std::uint32_t p = cellPoints[s];

    EBGEOMETRY_EXPECT(std::size_t(p) < numPoints);

    this->query(positions[s], a_k, &result[std::size_t(p) * a_k], p);
  }

  return result;
}

} // namespace EBGeometry

#endif
