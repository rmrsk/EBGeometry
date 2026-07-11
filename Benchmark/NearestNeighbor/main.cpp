// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Benchmark: EBGeometry PointCloudBVH vs picoflann vs nanoflann, all-nearest-neighbor.
// 500k random 3D points in the unit cube (double). Every point's nearest OTHER point. All three
// verified against a brute-force sample. Both KD-trees used vanilla; distances are squared throughout.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

#include <EBGeometry.hpp>

#include "nanoflann.hpp"
#include "picoflann.h"

using T    = double;
using Vec3 = EBGeometry::Vec3T<T>;

constexpr std::size_t   numPoints  = 500000;
constexpr std::size_t   sampleSize = 500;
constexpr std::uint64_t pointSeed  = 123456789ULL;

namespace {

// picoflann adapter.
struct Vec3Adapter
{
  inline T
  operator()(const Vec3& a_p, int a_dim) const
  {
    return a_p[a_dim];
  }
};

// nanoflann dataset adaptor over std::vector<Vec3>.
struct NanoCloud
{
  const std::vector<Vec3>& pts;
  inline std::size_t
  kdtree_get_point_count() const
  {
    return pts.size();
  }
  inline T
  kdtree_get_pt(const std::size_t a_idx, const std::size_t a_dim) const
  {
    return pts[a_idx][a_dim];
  }
  template <class BBOX>
  bool
  kdtree_get_bbox(BBOX&) const
  {
    return false;
  }
};
using NanoTree = nanoflann::KDTreeSingleIndexAdaptor<nanoflann::L2_Simple_Adaptor<T, NanoCloud>, NanoCloud, 3>;

T
bruteForceNN2(std::size_t a_self, const std::vector<Vec3>& a_pos)
{
  T best = std::numeric_limits<T>::max();
  for (std::size_t i = 0; i < a_pos.size(); i++) {
    if (i != a_self) {
      best = std::min(best, (a_pos[i] - a_pos[a_self]).length2());
    }
  }
  return best;
}

} // namespace

int
main()
{
  std::cout << "All-nearest-neighbor: PointCloudBVH vs picoflann vs nanoflann\n";
  std::cout << "  Points = " << numPoints << " (double, unit cube)\n\n";

  const std::vector<Vec3>        positions = EBGeometry::Random::samplePoints<T>(numPoints, pointSeed);
  const std::vector<std::size_t> meta(numPoints);

  EBGeometry::SimpleTimer timer;

  // Query the flann trees in a spatially-coherent (Hilbert) order too, so their node cache is as warm
  // as EBGeometry's leaf-order batch. EBGeometry gets its order free from the build; the flann libs
  // must sort -- time that once so it can be folded in if desired.
  timer.start();
  const std::vector<std::uint32_t> order = EBGeometry::SFC::order<EBGeometry::SFC::Hilbert>(positions);
  timer.stop();
  const double sortUsPerPt = 1.0e6 * timer.seconds() / double(numPoints);

  const std::size_t stride = numPoints / sampleSize;
  std::vector<T>    truth(sampleSize);
  timer.start();
  for (std::size_t s = 0; s < sampleSize; s++) {
    truth[s] = bruteForceNN2(s * stride, positions);
  }
  timer.stop();
  const double bruteUsPerPt = 1.0e6 * timer.seconds() / double(sampleSize);

  auto ok = [&](T a_got, std::size_t a_s) {
    return std::abs(a_got - truth[a_s]) <= 1.0e-9 * std::max(truth[a_s], T(1));
  };

  std::cout << std::left << std::setw(22) << "Method" << std::right << std::setw(12) << "Build(ms)" << std::setw(14)
            << "Query(us/pt)" << std::setw(12) << "vs brute" << '\n';
  std::cout << std::string(60, '-') << '\n';
  std::cout << std::fixed;
  std::cout << std::left << std::setw(22) << "Brute force" << std::right << std::setw(12) << "--" << std::setw(14)
            << std::setprecision(3) << bruteUsPerPt << std::setw(12) << "1.0x" << '\n';

  auto row = [&](const char* a_name, double a_buildMs, double a_queryUs, std::size_t a_bad) {
    std::cout << std::left << std::setw(22) << a_name << std::right << std::setw(12) << std::setprecision(1)
              << a_buildMs << std::setw(14) << std::setprecision(3) << a_queryUs << std::setw(11)
              << std::setprecision(1) << bruteUsPerPt / a_queryUs << "x" << (a_bad ? "  MISMATCH!" : "") << '\n';
  };

  // ── EBGeometry PointCloudBVH (batched all-NN) ──
  {
    timer.start();
    const EBGeometry::PointCloudBVH<T, std::size_t> bvh(positions, meta);
    timer.stop();
    const double buildMs = 1.0e3 * timer.seconds();

    timer.start();
    const auto graph = bvh.allNearestNeighbors(1);
    timer.stop();
    const double queryUs = 1.0e6 * timer.seconds() / double(numPoints);

    std::size_t bad = 0;
    for (std::size_t s = 0; s < sampleSize; s++) {
      bad += !ok(graph[s * stride].distanceSquared, s);
    }
    row("PointCloudBVH", buildMs, queryUs, bad);
  }

  // ── picoflann (per-point searchKnn) ──
  {
    picoflann::KdTreeIndex<3, Vec3Adapter> kdtree;
    timer.start();
    kdtree.build(positions);
    timer.stop();
    const double buildMs = 1.0e3 * timer.seconds();

    volatile T sink = T(0);
    timer.start();
    for (const std::uint32_t p : order) {
      const auto res = kdtree.searchKnn(positions, positions[p], 2);
      for (const auto& pr : res) {
        if (pr.first != p) {
          sink += pr.second;
          break;
        }
      }
    }
    timer.stop();
    (void)sink;
    const double queryUs = 1.0e6 * timer.seconds() / double(numPoints);

    std::size_t bad = 0;
    for (std::size_t s = 0; s < sampleSize; s++) {
      const auto res = kdtree.searchKnn(positions, positions[s * stride], 2);
      T          got = std::numeric_limits<T>::max();
      for (const auto& pr : res) {
        if (pr.first != s * stride) {
          got = pr.second;
          break;
        }
      }
      bad += !ok(got, s);
    }
    row("picoflann", buildMs, queryUs, bad);
  }

  // ── nanoflann (per-point findNeighbors, k=2) ──
  {
    NanoCloud cloud{positions};
    timer.start();
    NanoTree index(3, cloud, nanoflann::KDTreeSingleIndexAdaptorParams(10 /* leaf_max_size */));
    index.buildIndex();
    timer.stop();
    const double buildMs = 1.0e3 * timer.seconds();

    auto nnOther = [&](std::size_t i) {
      std::size_t                idx[2];
      T                          d2[2];
      nanoflann::KNNResultSet<T> rs(2);
      rs.init(idx, d2);
      const T qp[3] = {positions[i][0], positions[i][1], positions[i][2]};
      index.findNeighbors(rs, qp);
      // idx[0] is the point itself (distance 0); take the first neighbor that is not itself.
      return (idx[0] != i) ? d2[0] : d2[1];
    };

    volatile T sink = T(0);
    timer.start();
    for (const std::uint32_t p : order) {
      sink += nnOther(p);
    }
    timer.stop();
    (void)sink;
    const double queryUs = 1.0e6 * timer.seconds() / double(numPoints);

    std::size_t bad = 0;
    for (std::size_t s = 0; s < sampleSize; s++) {
      bad += !ok(nnOther(s * stride), s);
    }
    row("nanoflann", buildMs, queryUs, bad);
  }

  std::cout << "\n  flann query loops iterate in Hilbert order (warm cache, like EBGeometry's leaf order).\n";
  std::cout << "  One-time Hilbert sort the flann libs need for that order: " << std::setprecision(3) << sortUsPerPt
            << " us/pt (add to their query if counted;\n  EBGeometry reuses its build order for free).\n";

  return 0;
}
