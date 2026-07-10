// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// PROTOTYPE (Dev/, delete before merge): does a uniform spatial hash grid beat the PointCloudBVH tree
// for all-nearest-neighbor over a near-uniform particle cloud?
//
// The grid here is the bounded-domain form of spatial hashing: points are counting-sorted into a
// dense CSR bucket array (cellStart[] + pointIdx[]) keyed by integer cell coordinates. The exact same
// query logic works with an unordered_map<cellId, bucket> for unbounded/sparse domains -- that trades
// the dense array's O(1) cache-friendly lookup for a hash probe, so this dense version is the grid's
// best case and the fair thing to race against the tree.
//
// Query is an expanding-shell search with an exact stopping rule (no missed neighbors): after
// searching every cell within Chebyshev radius R of the query's cell, any *unsearched* point is at
// least R*h away, so once the best distance found is <= R*h we can stop. With ~1 point/cell that is
// almost always R=1 or R=2.
//
// Build with: make    (see GNUmakefile). Runs a 500k cloud; prints build+query times for the BVH and
// several grid cell sizes, all verified against brute force on a sample.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <vector>

#include <EBGeometry.hpp>

using T    = double;
using Vec3 = EBGeometry::Vec3T<T>;

constexpr std::size_t   numPoints  = 500000;
constexpr std::size_t   sampleSize = 500; // points verified against brute force
constexpr std::uint64_t pointSeed  = 123456789ULL;

namespace {

// ── Uniform spatial grid (dense CSR bucketing) ──────────────────────────────────────────────────
class UniformGrid
{
public:
  // Build over the cloud with a chosen cell size. Counting-sort the points into cells.
  void
  build(const std::vector<Vec3>& a_pos, T a_cellSize)
  {
    const std::size_t n = a_pos.size();

    m_lo    = +Vec3::max();
    Vec3 hi = -Vec3::max();
    for (const auto& p : a_pos) {
      m_lo = min(m_lo, p);
      hi   = max(hi, p);
    }

    m_h    = a_cellSize;
    m_invH = T(1) / m_h;
    m_nx   = std::max(1, int((hi[0] - m_lo[0]) * m_invH) + 1);
    m_ny   = std::max(1, int((hi[1] - m_lo[1]) * m_invH) + 1);
    m_nz   = std::max(1, int((hi[2] - m_lo[2]) * m_invH) + 1);

    const std::size_t nCells = std::size_t(m_nx) * m_ny * m_nz;

    // Counting sort: histogram -> prefix sum (cellStart) -> scatter point indices.
    m_cellStart.assign(nCells + 1, 0);
    for (std::size_t i = 0; i < n; i++) {
      m_cellStart[cellId(a_pos[i]) + 1]++;
    }
    for (std::size_t c = 0; c < nCells; c++) {
      m_cellStart[c + 1] += m_cellStart[c];
    }
    m_pointIdx.resize(n);
    std::vector<std::uint32_t> cursor(m_cellStart.begin(), m_cellStart.end() - 1);
    for (std::size_t i = 0; i < n; i++) {
      m_pointIdx[cursor[cellId(a_pos[i])]++] = std::uint32_t(i);
    }
  }

  // Nearest OTHER point to particle a_self (expanding-shell search with the exact stopping rule).
  std::uint32_t
  nearestNeighbor(std::size_t a_self, const std::vector<Vec3>& a_pos) const
  {
    const Vec3 p  = a_pos[a_self];
    const int  cx = clamp(int((p[0] - m_lo[0]) * m_invH), m_nx);
    const int  cy = clamp(int((p[1] - m_lo[1]) * m_invH), m_ny);
    const int  cz = clamp(int((p[2] - m_lo[2]) * m_invH), m_nz);

    T             best  = std::numeric_limits<T>::max();
    std::uint32_t bestI = std::numeric_limits<std::uint32_t>::max();

    const int rMax = std::max(m_nx, std::max(m_ny, m_nz));
    for (int r = 0; r <= rMax; r++) {
      // Visit only the new shell at Chebyshev radius r (cells with max(|dx|,|dy|,|dz|) == r).
      for (int dz = -r; dz <= r; dz++) {
        for (int dy = -r; dy <= r; dy++) {
          for (int dx = -r; dx <= r; dx++) {
            if (std::max(std::abs(dx), std::max(std::abs(dy), std::abs(dz))) != r) {
              continue;
            }
            const int ix = cx + dx, iy = cy + dy, iz = cz + dz;
            if (ix < 0 || ix >= m_nx || iy < 0 || iy >= m_ny || iz < 0 || iz >= m_nz) {
              continue;
            }
            const std::size_t   cid   = std::size_t(ix) + std::size_t(m_nx) * (iy + std::size_t(m_ny) * iz);
            const std::uint32_t begin = m_cellStart[cid];
            const std::uint32_t end   = m_cellStart[cid + 1];
            for (std::uint32_t k = begin; k < end; k++) {
              const std::uint32_t q = m_pointIdx[k];
              if (q == a_self) {
                continue;
              }
              const T d2 = (a_pos[q] - p).length2();
              if (d2 < best) {
                best  = d2;
                bestI = q;
              }
            }
          }
        }
      }
      // After finishing shell r, any unsearched point is >= r*h away. Stop if best is already inside.
      if (bestI != std::numeric_limits<std::uint32_t>::max() && best <= (T(r) * m_h) * (T(r) * m_h)) {
        break;
      }
    }
    return bestI;
  }

  double
  averagePerCell() const
  {
    return double(m_pointIdx.size()) / double(std::size_t(m_nx) * m_ny * m_nz);
  }

private:
  static int
  clamp(int a_i, int a_n)
  {
    return a_i < 0 ? 0 : (a_i >= a_n ? a_n - 1 : a_i);
  }

  std::size_t
  cellId(const Vec3& a_p) const
  {
    const int ix = clamp(int((a_p[0] - m_lo[0]) * m_invH), m_nx);
    const int iy = clamp(int((a_p[1] - m_lo[1]) * m_invH), m_ny);
    const int iz = clamp(int((a_p[2] - m_lo[2]) * m_invH), m_nz);
    return std::size_t(ix) + std::size_t(m_nx) * (iy + std::size_t(m_ny) * iz);
  }

  Vec3 m_lo;
  T    m_h    = T(1);
  T    m_invH = T(1);
  int  m_nx = 1, m_ny = 1, m_nz = 1;

  std::vector<std::uint32_t> m_cellStart; // size nCells+1 (CSR offsets)
  std::vector<std::uint32_t> m_pointIdx;  // size N, point indices sorted by cell
};

// Brute-force nearest OTHER point (ground truth).
std::uint32_t
bruteForceNN(std::size_t a_self, const std::vector<Vec3>& a_pos)
{
  T             best  = std::numeric_limits<T>::max();
  std::uint32_t bestI = 0;
  for (std::size_t i = 0; i < a_pos.size(); i++) {
    if (i == a_self) {
      continue;
    }
    const T d2 = (a_pos[i] - a_pos[a_self]).length2();
    if (d2 < best) {
      best  = d2;
      bestI = std::uint32_t(i);
    }
  }
  return bestI;
}

} // namespace

int
main()
{
  std::cout << "GridNN prototype: uniform spatial grid vs PointCloudBVH for all-nearest-neighbor\n";
  std::cout << "  Points = " << numPoints << " (double, unit cube)\n\n";

  const std::vector<Vec3>        positions = EBGeometry::Random::samplePoints<T>(numPoints, pointSeed);
  const std::vector<std::size_t> meta(numPoints);

  EBGeometry::SimpleTimer timer;

  // Reference NN for a spread sample, and its own timing (the baseline).
  const std::size_t          stride = numPoints / sampleSize;
  std::vector<std::uint32_t> truth(sampleSize);
  timer.start();
  for (std::size_t s = 0; s < sampleSize; s++) {
    truth[s] = bruteForceNN(s * stride, positions);
  }
  timer.stop();
  const double bruteUsPerPt = 1.0e6 * timer.seconds() / double(sampleSize);

  auto sameDistance = [&](std::uint32_t a_i, std::uint32_t a_j, std::size_t a_self) {
    // Ties are valid: compare distances, not indices.
    const T da = (positions[a_i] - positions[a_self]).length2();
    const T db = (positions[a_j] - positions[a_self]).length2();
    return std::abs(da - db) <= 1.0e-9 * std::max(db, T(1));
  };

  std::cout << std::left << std::setw(26) << "Method" << std::right << std::setw(12) << "Build(ms)" << std::setw(14)
            << "Query(us/pt)" << std::setw(12) << "vs brute" << std::setw(14) << "avg pts/cell" << '\n';
  std::cout << std::string(78, '-') << '\n';
  std::cout << std::fixed;

  std::cout << std::left << std::setw(26) << "Brute force" << std::right << std::setw(12) << "--" << std::setw(14)
            << std::setprecision(3) << bruteUsPerPt << std::setw(12) << "1.0x" << std::setw(14) << "--" << '\n';

  // ── PointCloudBVH ──
  {
    timer.start();
    const EBGeometry::PointCloudBVH<T, std::size_t> bvh(positions, meta);
    timer.stop();
    const double buildMs = 1.0e3 * timer.seconds();

    volatile std::size_t sink = 0;
    timer.start();
    for (std::size_t i = 0; i < numPoints; i++) {
      sink += bvh.nearestNeighbor(i).index;
    }
    timer.stop();
    (void)sink;
    const double queryUs = 1.0e6 * timer.seconds() / double(numPoints);

    std::size_t bad = 0;
    for (std::size_t s = 0; s < sampleSize; s++) {
      if (!sameDistance(std::uint32_t(bvh.nearestNeighbor(s * stride).index), truth[s], s * stride)) {
        bad++;
      }
    }

    std::cout << std::left << std::setw(26) << "PointCloudBVH" << std::right << std::setw(12) << std::setprecision(1)
              << buildMs << std::setw(14) << std::setprecision(3) << queryUs << std::setw(11) << std::setprecision(1)
              << bruteUsPerPt / queryUs << "x" << std::setw(14) << "--" << (bad ? "  MISMATCH!" : "") << '\n';
  }

  // ── Uniform grid, several cell sizes ──
  // Cell size from a target average points-per-cell: h = (bboxVolume / N * target)^(1/3).
  const T bboxVol = T(1); // unit cube (sample is in [0,1]^3)
  for (const double target : {0.5, 1.0, 2.0, 4.0}) {
    const T h = std::cbrt(bboxVol / double(numPoints) * target);

    UniformGrid grid;
    timer.start();
    grid.build(positions, h);
    timer.stop();
    const double buildMs = 1.0e3 * timer.seconds();

    volatile std::size_t sink = 0;
    timer.start();
    for (std::size_t i = 0; i < numPoints; i++) {
      sink += grid.nearestNeighbor(i, positions);
    }
    timer.stop();
    (void)sink;
    const double queryUs = 1.0e6 * timer.seconds() / double(numPoints);

    std::size_t bad = 0;
    for (std::size_t s = 0; s < sampleSize; s++) {
      if (!sameDistance(grid.nearestNeighbor(s * stride, positions), truth[s], s * stride)) {
        bad++;
      }
    }

    std::ostringstream label;
    label << "Grid (target " << target << "/cell)";
    std::cout << std::left << std::setw(26) << label.str() << std::right << std::setw(12) << std::setprecision(1)
              << buildMs << std::setw(14) << std::setprecision(3) << queryUs << std::setw(11) << std::setprecision(1)
              << bruteUsPerPt / queryUs << "x" << std::setw(14) << std::setprecision(2) << grid.averagePerCell()
              << (bad ? "  MISMATCH!" : "") << '\n';
  }

  return 0;
}
