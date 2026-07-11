// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
  @file    EBGeometry_PointCloudHashGridImplem.hpp
  @brief   Implementation of EBGeometry_PointCloudHashGrid.hpp
  @author  Robert Marskar
*/

#ifndef EBGeometry_PointCloudHashGridImplem
#define EBGeometry_PointCloudHashGridImplem

// Std includes
#include <algorithm>
#include <cmath>

// Our includes
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_PointCloudHashGrid.hpp"

namespace EBGeometry {

template <class T, class Meta>
inline PointCloudHashGrid<T, Meta>::PointCloudHashGrid(const std::vector<Vec3T<T>>& a_positions,
                                                       const std::vector<Meta>&     a_metadata,
                                                       T                            a_targetPerCell)
  : m_positions(a_positions), m_metadata(a_metadata)
{
  EBGEOMETRY_EXPECT(a_positions.size() == a_metadata.size());
  EBGEOMETRY_EXPECT(a_targetPerCell > T(0));

  const std::size_t n = m_positions.size();

  // Bounding box of the cloud.
  m_lo        = +Vec3T<T>::max();
  Vec3T<T> hi = -Vec3T<T>::max();
  for (const auto& p : m_positions) {
    m_lo = min(m_lo, p);
    hi   = max(hi, p);
  }
  if (n == 0) {
    m_lo = Vec3T<T>::zeros();
    hi   = Vec3T<T>::zeros();
  }

  // Cell size from the target average occupancy: h = cbrt(bboxVolume / N * target). Fall back to a
  // characteristic length when the cloud is degenerate (zero-volume box or empty).
  const Vec3T<T> ext = hi - m_lo;
  const T        vol = ext[0] * ext[1] * ext[2];
  if (vol > T(0) && n > 0) {
    m_h = std::cbrt(vol / T(n) * a_targetPerCell);
  }
  else {
    const T maxExt = std::max(ext[0], std::max(ext[1], ext[2]));
    m_h            = (maxExt > T(0) && n > 0) ? maxExt / std::cbrt(T(n)) : T(1);
  }
  if (!(m_h > T(0))) {
    m_h = T(1);
  }
  m_invH = T(1) / m_h;

  m_nx = std::max(1, int((hi[0] - m_lo[0]) * m_invH) + 1);
  m_ny = std::max(1, int((hi[1] - m_lo[1]) * m_invH) + 1);
  m_nz = std::max(1, int((hi[2] - m_lo[2]) * m_invH) + 1);

  const std::size_t nCells = std::size_t(m_nx) * std::size_t(m_ny) * std::size_t(m_nz);

  // Counting sort: histogram -> prefix sum (cellStart) -> scatter cloud indices into cell order.
  m_cellStart.assign(nCells + 1, 0);
  for (std::size_t i = 0; i < n; i++) {
    m_cellStart[cellIndexOf(m_positions[i]) + 1]++;
  }
  for (std::size_t c = 0; c < nCells; c++) {
    m_cellStart[c + 1] += m_cellStart[c];
  }
  m_cellPoints.resize(n);
  std::vector<std::uint32_t> cursor(m_cellStart.begin(), m_cellStart.end() - 1);
  for (std::size_t i = 0; i < n; i++) {
    m_cellPoints[cursor[cellIndexOf(m_positions[i])]++] = std::uint32_t(i);
  }
}

template <class T, class Meta>
inline int
PointCloudHashGrid<T, Meta>::cellCoord(T a_x, T a_lo, int a_n) const noexcept
{
  const int c = int((a_x - a_lo) * m_invH);
  return c < 0 ? 0 : (c >= a_n ? a_n - 1 : c);
}

template <class T, class Meta>
inline std::size_t
PointCloudHashGrid<T, Meta>::cellIndex(int a_ix, int a_iy, int a_iz) const noexcept
{
  return std::size_t(a_ix) + std::size_t(m_nx) * (std::size_t(a_iy) + std::size_t(m_ny) * std::size_t(a_iz));
}

template <class T, class Meta>
inline std::size_t
PointCloudHashGrid<T, Meta>::cellIndexOf(const Vec3T<T>& a_p) const noexcept
{
  return cellIndex(
    cellCoord(a_p[0], m_lo[0], m_nx), cellCoord(a_p[1], m_lo[1], m_ny), cellCoord(a_p[2], m_lo[2], m_nz));
}

template <class T, class Meta>
inline void
PointCloudHashGrid<T, Meta>::query(
  const Vec3T<T>& a_query, std::size_t a_k, Hit* a_out, std::size_t& a_found, std::size_t a_exclude) const noexcept
{
  a_found = 0;
  if (a_k == 0 || m_positions.empty()) {
    return;
  }

  const int cx = cellCoord(a_query[0], m_lo[0], m_nx);
  const int cy = cellCoord(a_query[1], m_lo[1], m_ny);
  const int cz = cellCoord(a_query[2], m_lo[2], m_nz);

  // Sorted insert into a_out (kept ascending, capacity a_k). Each particle lives in exactly one cell
  // and each cell is visited in exactly one shell, so no de-duplication is needed.
  const auto insert = [&](std::uint32_t a_idx, T a_d2) noexcept {
    if (a_found == a_k) {
      if (a_d2 >= a_out[a_k - 1].distanceSquared) {
        return;
      }
    }
    std::size_t i = (a_found < a_k) ? a_found++ : (a_k - 1);
    for (; i > 0 && a_out[i - 1].distanceSquared > a_d2; i--) {
      a_out[i] = a_out[i - 1];
    }
    a_out[i] = Hit{std::size_t(a_idx), a_d2};
  };

  // Largest shell radius that still adds cells (beyond it the whole grid is searched).
  const int rMax =
    std::max(std::max(cx, m_nx - 1 - cx), std::max(std::max(cy, m_ny - 1 - cy), std::max(cz, m_nz - 1 - cz)));

  for (int r = 0; r <= rMax; r++) {
    // Visit only the new shell at Chebyshev radius r (cells with max(|dx|,|dy|,|dz|) == r).
    const int xlo = std::max(0, cx - r), xhi = std::min(m_nx - 1, cx + r);
    const int ylo = std::max(0, cy - r), yhi = std::min(m_ny - 1, cy + r);
    const int zlo = std::max(0, cz - r), zhi = std::min(m_nz - 1, cz + r);
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
          const std::uint32_t begin = m_cellStart[cid];
          const std::uint32_t end   = m_cellStart[cid + 1];
          for (std::uint32_t s = begin; s < end; s++) {
            const std::uint32_t p = m_cellPoints[s];
            if (std::size_t(p) == a_exclude) {
              continue;
            }
            const T d2 = (m_positions[p] - a_query).length2();
            insert(p, d2);
          }
        }
      }
    }

    // Exact stopping rule. After searching the cell box [cx-r, cx+r] x ... (clamped), the world region
    // guaranteed fully searched is [loCov, hiCov] per axis, with the box extending to +/-infinity on
    // any axis whose clamped side reached the grid edge (no particles exist beyond the grid). Any
    // unvisited particle is outside that world box, so its distance to the query is at least the
    // distance from the query to the nearest covered face. Stop once we hold a_k and that bound is
    // not closer than our current worst.
    if (a_found == a_k) {
      const T inf   = std::numeric_limits<T>::max();
      T       bound = inf;
      for (int axis = 0; axis < 3; axis++) {
        const int c     = (axis == 0) ? cx : (axis == 1) ? cy : cz;
        const int nAxis = (axis == 0) ? m_nx : (axis == 1) ? m_ny : m_nz;
        if (c - r > 0) {
          bound = std::min(bound, a_query[axis] - (m_lo[axis] + T(c - r) * m_h));
        }
        if (c + r < nAxis - 1) {
          bound = std::min(bound, (m_lo[axis] + T(c + r + 1) * m_h) - a_query[axis]);
        }
      }
      if (bound == inf || a_out[a_k - 1].distanceSquared <= bound * bound) {
        break;
      }
    }
  }
}

template <class T, class Meta>
inline typename PointCloudHashGrid<T, Meta>::Hit
PointCloudHashGrid<T, Meta>::closestPoint(const Vec3T<T>& a_query) const noexcept
{
  Hit         hit;
  std::size_t found = 0;
  this->query(a_query, 1, &hit, found, s_none);
  return hit;
}

template <class T, class Meta>
inline std::size_t
PointCloudHashGrid<T, Meta>::closestPoints(const Vec3T<T>& a_query, std::size_t a_k, Hit* a_out) const noexcept
{
  std::size_t found = 0;
  this->query(a_query, a_k, a_out, found, s_none);
  return found;
}

template <class T, class Meta>
inline typename PointCloudHashGrid<T, Meta>::Hit
PointCloudHashGrid<T, Meta>::nearestNeighbor(std::size_t a_particle) const noexcept
{
  Hit         hit;
  std::size_t found = 0;
  this->query(m_positions[a_particle], 1, &hit, found, a_particle);
  return hit;
}

template <class T, class Meta>
inline std::size_t
PointCloudHashGrid<T, Meta>::nearestNeighbors(std::size_t a_particle, std::size_t a_k, Hit* a_out) const noexcept
{
  std::size_t found = 0;
  this->query(m_positions[a_particle], a_k, a_out, found, a_particle);
  return found;
}

template <class T, class Meta>
inline std::vector<typename PointCloudHashGrid<T, Meta>::Hit>
PointCloudHashGrid<T, Meta>::allNearestNeighbors(std::size_t a_k) const
{
  const std::size_t n = m_positions.size();

  std::vector<Hit> result(n * a_k);

  // Process particles in cell (spatial) order -- consecutive queries touch nearby cells, staying hot
  // in cache. m_cellPoints already holds the cloud indices in cell order.
  for (const std::uint32_t p : m_cellPoints) {
    std::size_t found = 0;
    this->query(m_positions[p], a_k, &result[std::size_t(p) * a_k], found, std::size_t(p));
  }

  return result;
}

template <class T, class Meta>
inline typename PointCloudHashGrid<T, Meta>::Hit
PointCloudHashGrid<T, Meta>::bruteForceOne(const Vec3T<T>& a_query, std::size_t a_exclude) const noexcept
{
  Hit best;
  for (std::size_t i = 0; i < m_positions.size(); i++) {
    if (i == a_exclude) {
      continue;
    }
    const T d2 = (m_positions[i] - a_query).length2();
    if (d2 < best.distanceSquared) {
      best.distanceSquared = d2;
      best.index           = i;
    }
  }
  return best;
}

template <class T, class Meta>
inline std::size_t
PointCloudHashGrid<T, Meta>::bruteForceK(const Vec3T<T>& a_query,
                                         std::size_t     a_k,
                                         Hit*            a_out,
                                         std::size_t     a_exclude) const
{
  std::vector<Hit> all;
  all.reserve(m_positions.size());
  for (std::size_t i = 0; i < m_positions.size(); i++) {
    if (i == a_exclude) {
      continue;
    }
    all.push_back(Hit{i, (m_positions[i] - a_query).length2()});
  }

  const std::size_t k = std::min(a_k, all.size());
  std::partial_sort(
    all.begin(),
    all.begin() + static_cast<std::ptrdiff_t>(k),
    all.end(),
    [](const Hit& a_lhs, const Hit& a_rhs) noexcept { return a_lhs.distanceSquared < a_rhs.distanceSquared; });
  for (std::size_t j = 0; j < k; j++) {
    a_out[j] = all[j];
  }
  return k;
}

template <class T, class Meta>
inline typename PointCloudHashGrid<T, Meta>::Hit
PointCloudHashGrid<T, Meta>::closestPointBruteForce(const Vec3T<T>& a_query) const noexcept
{
  return this->bruteForceOne(a_query, s_none);
}

template <class T, class Meta>
inline std::size_t
PointCloudHashGrid<T, Meta>::closestPointsBruteForce(const Vec3T<T>& a_query, std::size_t a_k, Hit* a_out) const
{
  return this->bruteForceK(a_query, a_k, a_out, s_none);
}

template <class T, class Meta>
inline typename PointCloudHashGrid<T, Meta>::Hit
PointCloudHashGrid<T, Meta>::nearestNeighborBruteForce(std::size_t a_particle) const noexcept
{
  return this->bruteForceOne(m_positions[a_particle], a_particle);
}

template <class T, class Meta>
inline std::size_t
PointCloudHashGrid<T, Meta>::nearestNeighborsBruteForce(std::size_t a_particle, std::size_t a_k, Hit* a_out) const
{
  return this->bruteForceK(m_positions[a_particle], a_k, a_out, a_particle);
}

} // namespace EBGeometry

#endif
