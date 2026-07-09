// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

// Declare our precision.
using T = EBGEOMETRY_PRECISION;

// Aliases for cutting down on typing.
using Vec3 = EBGeometry::Vec3T<T>;
using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

// Fixed (not BVH::DefaultBranchingRatio<T>()) so build times are comparable across machines/ISAs
// rather than varying with whatever K the compiling machine's SIMD tier happens to prefer.
constexpr size_t K = 4;

// A minimal point primitive -- deliberately example-local, not a library type. Whether a
// point/particle SoA primitive belongs in Source/ as a first-class type, or stays illustrative
// like this, is still an open question (see EBGeometry issue #92); this example only needs
// *some* small, cheaply-copyable primitive to benchmark build strategies against, and a bare
// position is enough (no getCentroid() method needed: every partitioner and SFC bottom-up build
// used below works off each primitive's *bounding volume* centroid, never the primitive itself).
struct Point
{
  Vec3 m_pos;
};

using Tree   = EBGeometry::BVH::TreeBVH<T, Point, AABB, K>;
using Packed = EBGeometry::BVH::PackedBVH<T, Point, K>;

namespace {

// Build a fresh (primitive, bounding volume) pair list, wrapping each point in a shared_ptr --
// this is the traditional TreeBVH input, and the wrapping is a real, timed part of that path's
// cost, not incidental setup.
EBGeometry::BVH::PrimAndBVList<Point, AABB>
makeWrappedPrimitives(const std::vector<Vec3>& a_positions)
{
  EBGeometry::BVH::PrimAndBVList<Point, AABB> primsAndBVs;
  primsAndBVs.reserve(a_positions.size());

  for (const auto& pos : a_positions) {
    primsAndBVs.emplace_back(std::make_shared<Point>(Point{pos}), AABB(pos, pos));
  }

  return primsAndBVs;
}

// Build the flat (primitive, bounding volume) pair list PackedBVH's direct constructor wants --
// no shared_ptr anywhere, matching what that constructor is actually for.
std::vector<std::pair<Point, AABB>>
makeFlatPrimitives(const std::vector<Vec3>& a_positions)
{
  std::vector<std::pair<Point, AABB>> primsAndBVs;
  primsAndBVs.reserve(a_positions.size());

  for (const auto& pos : a_positions) {
    primsAndBVs.emplace_back(Point{pos}, AABB(pos, pos));
  }

  return primsAndBVs;
}

// Brute-force nearest-neighbor squared distance, used to check every build strategy's result
// against ground truth (not just against each other).
T
bruteForceNearest2(const std::vector<Vec3>& a_positions, const Vec3& a_query)
{
  T best2 = std::numeric_limits<T>::max();
  for (const auto& pos : a_positions) {
    best2 = std::min(best2, (pos - a_query).length2());
  }
  return best2;
}

// Nearest-neighbor squared distance via PackedBVH::pruneTraverse(). State is a running squared
// distance directly -- no abs(), no extra squaring for the prune bound -- exactly the shape a
// point-cloud nearest-neighbor search wants (see Tests/TestBVH.cpp for the same pattern).
T
nearest2(const Packed& a_packed, const Vec3& a_query)
{
  T state = std::numeric_limits<T>::max();

  const auto& prims = a_packed.getPrimitives();

  const auto evalLeaf = [&prims, &a_query](T& a_state, size_t a_offset, size_t a_count) noexcept {
    for (size_t i = 0; i < a_count; i++) {
      const T d2 = (prims[a_offset + i]->m_pos - a_query).length2();
      if (d2 < a_state) {
        a_state = d2;
      }
    }
  };
  const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

  a_packed.pruneTraverse(a_query, state, evalLeaf, pruneDist2);

  return state;
}

// Checks a_packed against a brute-force scan for every point in a_queryPoints. Aborts the program
// on the first mismatch -- a build strategy that produced a wrong tree is a bug, not a benchmark
// footnote.
void
checkAgainstBruteForce(const char*              a_label,
                       const Packed&            a_packed,
                       const std::vector<Vec3>& a_positions,
                       const std::vector<Vec3>& a_queryPoints)
{
  constexpr T tolerance = std::is_same_v<T, float> ? T(1.0e-4) : T(1.0e-9);

  for (const auto& q : a_queryPoints) {
    const T fromBVH    = nearest2(a_packed, q);
    const T bruteForce = bruteForceNearest2(a_positions, q);

    if (std::abs(fromBVH - bruteForce) > tolerance * std::max(bruteForce, T(1.0))) {
      std::cerr << a_label << ": nearest-neighbor mismatch (BVH = " << fromBVH << ", brute-force = " << bruteForce
                << ")\n";
      std::exit(1);
    }
  }
}

// One measured TreeBVH-based build: buildTime is topDownSortAndPartition()/
// bottomUpSortAndPartition() alone; packTime is the additional pack() call. Reported separately
// (see main()'s comment for why), with their sum being the fair comparison against
// PackedBVH's direct constructor.
struct TreeBuildResult
{
  double                  buildTime;
  double                  packTime;
  std::shared_ptr<Packed> packed;
};

template <class PartitionFn>
TreeBuildResult
timeTreeBuild(const std::vector<Vec3>& a_positions, PartitionFn&& a_partitionFn)
{
  EBGeometry::SimpleTimer timer;

  timer.start();
  auto tree = std::make_shared<Tree>(makeWrappedPrimitives(a_positions));
  a_partitionFn(*tree);
  timer.stop();
  const double buildTime = timer.seconds();

  timer.start();
  auto packed = tree->pack();
  timer.stop();
  const double packTime = timer.seconds();

  return {buildTime, packTime, packed};
}

} // namespace

int
main()
{
  // TLDR: this program builds the same random point cloud into a BVH five different ways --
  //       TreeBVH top-down (default centroid split), TreeBVH top-down with the SAH partitioner,
  //       TreeBVH bottom-up along the Morton and Nested space-filling curves, and PackedBVH's
  //       direct constructor (no TreeBVH at all) -- and reports how long each takes to build, at
  //       a few point-cloud sizes. It exists to give a reproducible, versioned answer to a
  //       question raised in EBGeometry issue #92 (rather than "standalone scratch benchmarks,
  //       not committed anywhere"): whether SFC-based construction still beats top-down
  //       construction once shared_ptr allocation overhead is removed from consideration
  //       (compare the "TreeBVH + pack()" columns against the "PackedBVH direct" column).
  //
  // For each of the four TreeBVH-based strategies, two numbers are reported: the time to build
  // and partition the TreeBVH alone, and the additional time to pack() it into a queryable
  // PackedBVH -- since a TreeBVH by itself cannot answer queries, their sum is the fairer number
  // to compare directly against the direct constructor's single number, which already produces a
  // queryable PackedBVH.
  //
  // Every resulting PackedBVH is cross-checked against a brute-force scan for a handful of random
  // query points, so a build strategy that produced a wrong tree would be caught here, not just
  // timed.

  const std::vector<size_t> sizes = {1'000, 10'000, 100'000};

  // Target leaf size for PackedBVH's direct constructor. Chosen to match K, the leaf occupancy
  // the other four strategies naturally converge to (topDownSortAndPartition()'s default
  // LeafPredicate stops once a node holds fewer than K primitives; bottomUpSortAndPartition()'s
  // leaf count likewise divides the primitives into groups of roughly K), so the comparison
  // isn't skewed by one strategy simply building shallower or deeper trees than the others.
  constexpr size_t targetLeafSize = K;

  constexpr int numQueryPoints = 200;

  std::mt19937_64                   rng(123456789ULL); // Fixed seed: reproducible run-to-run.
  std::uniform_real_distribution<T> udist(T(0.0), T(1.0));

  std::cout << std::fixed << std::setprecision(6);

  for (const size_t N : sizes) {
    std::cout << "\n=== N = " << N << " points, K = " << K << " ===\n";

    std::vector<Vec3> positions;
    positions.reserve(N);
    for (size_t i = 0; i < N; i++) {
      positions.emplace_back(udist(rng), udist(rng), udist(rng));
    }

    std::vector<Vec3> queryPoints;
    queryPoints.reserve(numQueryPoints);
    for (int i = 0; i < numQueryPoints; i++) {
      queryPoints.emplace_back(udist(rng), udist(rng), udist(rng));
    }

    const auto topDown = timeTreeBuild(positions, [](Tree& a_tree) { a_tree.topDownSortAndPartition(); });

    const auto sah = timeTreeBuild(positions, [](Tree& a_tree) {
      using LeafPred          = typename Tree::LeafPredicate;
      const LeafPred stopCrit = [](const Tree& a_node) noexcept -> bool { return a_node.getPrimitives().size() < K; };
      a_tree.topDownSortAndPartition(EBGeometry::BVH::BinnedSAHPartitioner<T, Point, AABB, K>, stopCrit);
    });

    const auto morton = timeTreeBuild(
      positions, [](Tree& a_tree) { a_tree.template bottomUpSortAndPartition<EBGeometry::SFC::Morton>(); });

    const auto nested = timeTreeBuild(
      positions, [](Tree& a_tree) { a_tree.template bottomUpSortAndPartition<EBGeometry::SFC::Nested>(); });

    EBGeometry::SimpleTimer timer;
    timer.start();
    Packed direct(makeFlatPrimitives(positions), targetLeafSize);
    timer.stop();
    const double directBuildTime = timer.seconds();

    checkAgainstBruteForce("TopDown", *topDown.packed, positions, queryPoints);
    checkAgainstBruteForce("SAH", *sah.packed, positions, queryPoints);
    checkAgainstBruteForce("Morton", *morton.packed, positions, queryPoints);
    checkAgainstBruteForce("Nested", *nested.packed, positions, queryPoints);
    checkAgainstBruteForce("Direct", direct, positions, queryPoints);

    std::cout << std::left << std::setw(24) << "Strategy" << std::right << std::setw(16) << "Build (s)" << std::setw(16)
              << "+ pack() (s)" << std::setw(16) << "Total (s)" << "\n";

    auto printRow = [](const char* a_label, double a_buildTime, double a_packTime) {
      std::cout << std::left << std::setw(24) << a_label << std::right << std::setw(16) << a_buildTime << std::setw(16)
                << a_packTime << std::setw(16) << (a_buildTime + a_packTime) << "\n";
    };

    printRow("TreeBVH TopDown", topDown.buildTime, topDown.packTime);
    printRow("TreeBVH SAH", sah.buildTime, sah.packTime);
    printRow("TreeBVH Morton", morton.buildTime, morton.packTime);
    printRow("TreeBVH Nested", nested.buildTime, nested.packTime);
    std::cout << std::left << std::setw(24) << "PackedBVH direct" << std::right << std::setw(16) << directBuildTime
              << std::setw(16) << "--" << std::setw(16) << directBuildTime << "\n";
  }

  return 0;
}
