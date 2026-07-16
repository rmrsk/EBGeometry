// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Benchmark: EBGeometry PointCloudBVH and PointCloudHashGrid vs picoflann vs nanoflann vs kd3,
// all-nearest-neighbor. 500k 3D points (double). Every point's nearest OTHER point. All five
// verified against a brute-force sample. The KD-trees are used vanilla; distances are squared throughout.
//
// Two point distributions are benchmarked in turn, since spatial data structures behave very
// differently depending on how the points fill space:
//   1. Uniform in the unit cube      -- points fill a 3D volume evenly (the easy, balanced case).
//   2. On the unit-sphere surface    -- points lie on a 2D manifold: locally dense, globally hollow,
//                                       a harder case for uniform grids (many empty cells inside).
//
// kd3 (https://github.com/KaruroChori/kd3) requires C++23 (std::expected/std::span), so this whole
// benchmark is built with -std=c++23. For a like-for-like comparison it is run here in double and
// single-threaded (compiled without -fopenmp); kd3 is primarily float/SIMD-tuned and can parallelize
// its build with OpenMP, neither of which is exercised here.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

#include <kd3/kd3.hpp>

#include "nanoflann.hpp"
#include "picoflann.h"

using T    = double;
using Vec3 = EBGeometry::Vec3T<T>;

// kd3 tree, in double precision (its distance_t defaults to float; pin it to T for a fair comparison).
using Kd3 = kd3::KdTree<kd3::limits<T, 3, T, std::uint32_t>>;

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

// Uniformly sample the surface of the unit sphere: draw a 3D Gaussian and normalize, which is
// rotationally symmetric and hence uniform over the sphere. Points lie on a 2D manifold embedded in
// 3D -- locally dense, globally hollow.
std::vector<Vec3>
samplePointsOnSphere(std::size_t a_count, std::uint64_t a_seed)
{
  std::mt19937_64             rng(a_seed);
  std::normal_distribution<T> gauss(T(0), T(1));

  std::vector<Vec3> points;
  points.reserve(a_count);

  for (std::size_t i = 0; i < a_count; i++) {
    Vec3 v(gauss(rng), gauss(rng), gauss(rng));
    T    len = v.length();
    if (len < std::numeric_limits<T>::min()) {
      v   = Vec3(T(1), T(0), T(0)); // Degenerate zero draw; nudge onto the sphere.
      len = T(1);
    }
    points.emplace_back(v / len);
  }

  return points;
}

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

// Run the full all-nearest-neighbor comparison over one point distribution and print a table.
void
runCase(const std::string& a_label, const std::vector<Vec3>& a_positions)
{
  const std::size_t              n = a_positions.size();
  const std::vector<std::size_t> meta(n);

  EBGeometry::SimpleTimer timer;

  std::cout << "== " << a_label << " (" << n << " points, double) ==\n";

  // Query the flann trees in a spatially-coherent (Hilbert) order too, so their node cache is as warm
  // as EBGeometry's leaf-order batch. EBGeometry gets its order free from the build; the flann libs
  // must sort -- time that once so it can be folded in if desired.
  timer.start();
  const std::vector<std::uint32_t> order = EBGeometry::SFC::order<EBGeometry::SFC::Hilbert>(a_positions);
  timer.stop();
  const double sortUsPerPt = 1.0e6 * timer.seconds() / double(n);

  const std::size_t stride = n / sampleSize;
  std::vector<T>    truth(sampleSize);
  timer.start();
  for (std::size_t s = 0; s < sampleSize; s++) {
    truth[s] = bruteForceNN2(s * stride, a_positions);
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
    const EBGeometry::PointCloudBVH<T, std::size_t> bvh(a_positions, meta);
    timer.stop();
    const double buildMs = 1.0e3 * timer.seconds();

    timer.start();
    const auto graph = bvh.allNearestNeighbors(1);
    timer.stop();
    const double queryUs = 1.0e6 * timer.seconds() / double(n);

    std::size_t bad = 0;
    for (std::size_t s = 0; s < sampleSize; s++) {
      bad += !ok(graph[s * stride].distanceSquared, s);
    }
    row("PointCloudBVH", buildMs, queryUs, bad);
  }

  // ── EBGeometry PointCloudHashGrid (batched all-NN, uniform grid) ──
  {
    timer.start();
    const EBGeometry::PointCloudHashGrid<T, std::size_t> grid(a_positions, meta);
    timer.stop();
    const double buildMs = 1.0e3 * timer.seconds();

    timer.start();
    const auto graph = grid.allNearestNeighbors(1);
    timer.stop();
    const double queryUs = 1.0e6 * timer.seconds() / double(n);

    std::size_t bad = 0;
    for (std::size_t s = 0; s < sampleSize; s++) {
      bad += !ok(graph[s * stride].distanceSquared, s);
    }
    row("PointCloudHashGrid", buildMs, queryUs, bad);
  }

  // ── picoflann (per-point searchKnn) ──
  {
    picoflann::KdTreeIndex<3, Vec3Adapter> kdtree;
    timer.start();
    kdtree.build(a_positions);
    timer.stop();
    const double buildMs = 1.0e3 * timer.seconds();

    volatile T sink = T(0);
    timer.start();
    for (const std::uint32_t p : order) {
      const auto res = kdtree.searchKnn(a_positions, a_positions[p], 2);
      for (const auto& pr : res) {
        if (pr.first != p) {
          sink += pr.second;
          break;
        }
      }
    }
    timer.stop();
    (void)sink;
    const double queryUs = 1.0e6 * timer.seconds() / double(n);

    std::size_t bad = 0;
    for (std::size_t s = 0; s < sampleSize; s++) {
      const auto res = kdtree.searchKnn(a_positions, a_positions[s * stride], 2);
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
    NanoCloud cloud{a_positions};
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
      const T qp[3] = {a_positions[i][0], a_positions[i][1], a_positions[i][2]};
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
    const double queryUs = 1.0e6 * timer.seconds() / double(n);

    std::size_t bad = 0;
    for (std::size_t s = 0; s < sampleSize; s++) {
      bad += !ok(nnOther(s * stride), s);
    }
    row("nanoflann", buildMs, queryUs, bad);
  }

  // ── kd3 (per-point query_knn, k=2) ──
  {
    // kd3 builds from its own FatPoint {coords, payload} array (payload = cloud index), and sorts it
    // in place. Constructing that array is kd3's required input format, so it is folded into the timed
    // build (mirroring how fcpw's soup construction is timed as part of its build in MeshSDF).
    std::vector<Kd3::FatPoint> fat(n);
    timer.start();
    for (std::size_t i = 0; i < n; i++) {
      fat[i] = Kd3::FatPoint{{a_positions[i][0], a_positions[i][1], a_positions[i][2]}, static_cast<std::uint32_t>(i)};
    }

    auto treeExpected = Kd3::build(fat);
    timer.stop();
    const double buildMs = 1.0e3 * timer.seconds();

    if (!treeExpected) {
      row("kd3", buildMs, std::numeric_limits<double>::infinity(), sampleSize); // build failed -> flag it
    }
    else {
      const Kd3& tree = *treeExpected;

      auto nnOther = [&](std::size_t i) -> T {
        const Kd3::point_t              q = {a_positions[i][0], a_positions[i][1], a_positions[i][2]};
        std::array<Kd3::KnnResult, 2>   buf{};
        const auto                      res = tree.query_knn(q, buf);
        if (res) {
          for (const auto& kr : *res) {
            if (kr.payload_id != static_cast<std::uint32_t>(i)) { // skip the query point itself (dist 0)
              return kr.dist_sq;
            }
          }
        }
        return std::numeric_limits<T>::max();
      };

      volatile T sink = T(0);
      timer.start();
      for (const std::uint32_t p : order) {
        sink += nnOther(p);
      }
      timer.stop();
      (void)sink;
      const double queryUs = 1.0e6 * timer.seconds() / double(n);

      std::size_t bad = 0;
      for (std::size_t s = 0; s < sampleSize; s++) {
        bad += !ok(nnOther(s * stride), s);
      }
      row("kd3", buildMs, queryUs, bad);
    }
  }

  std::cout << "  flann/kd3 query loops iterate in Hilbert order (warm cache, like EBGeometry's leaf order).\n";
  std::cout << "  One-time Hilbert sort the flann libs need for that order: " << std::setprecision(3) << sortUsPerPt
            << " us/pt\n  (add to their query if counted; EBGeometry reuses its build order for free).\n\n";
}

} // namespace

int
main()
{
  std::cout << "All-nearest-neighbor: PointCloudBVH & PointCloudHashGrid vs picoflann vs nanoflann vs kd3\n\n";

  runCase("Uniform in the unit cube", EBGeometry::Random::samplePoints<T>(numPoints, pointSeed));
  runCase("On the unit-sphere surface", samplePointsOnSphere(numPoints, pointSeed));

  return 0;
}
