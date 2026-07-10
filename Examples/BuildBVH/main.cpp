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

// Build the flat (primitive, bounding volume) pair list PackedBVH's direct constructors want --
// no shared_ptr anywhere, matching what those constructors are actually for.
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

// Build-time results for one strategy: treeBuildTime is TreeBVH construction + partitioning
// alone; packTime is the additional pack() call; directBuildTime is PackedBVH's direct
// constructor (no TreeBVH at all). treeBuildTime + packTime is the fair number to compare against
// directBuildTime, since a bare TreeBVH cannot answer queries on its own.
struct StrategyResult
{
  double treeBuildTime;
  double packTime;
  double directBuildTime;
};

// Times one "partitioner family" strategy (TopDown/SAH/Midpoint): both the TreeBVH path (build +
// a_partitionFn, then pack()) and PackedBVH's direct top-down constructor with the same
// a_partitioner/a_stopCrit, then cross-checks both results against brute force.
template <class PartitionFn, class Partitioner, class LeafPred>
StrategyResult
runPartitionerFamily(const char*              a_label,
                     const std::vector<Vec3>& a_positions,
                     const std::vector<Vec3>& a_queryPoints,
                     PartitionFn&&            a_partitionFn,
                     Partitioner&&            a_partitioner,
                     LeafPred&&               a_stopCrit)
{
  EBGeometry::SimpleTimer timer;

  timer.start();
  auto tree = std::make_shared<Tree>(makeWrappedPrimitives(a_positions));
  a_partitionFn(*tree);
  timer.stop();
  const double treeBuildTime = timer.seconds();

  timer.start();
  auto packed = tree->pack();
  timer.stop();
  const double packTime = timer.seconds();

  timer.start();
  const Packed direct(makeFlatPrimitives(a_positions), a_partitioner, a_stopCrit);
  timer.stop();
  const double directBuildTime = timer.seconds();

  checkAgainstBruteForce(a_label, *packed, a_positions, a_queryPoints);
  checkAgainstBruteForce(a_label, direct, a_positions, a_queryPoints);

  return {treeBuildTime, packTime, directBuildTime};
}

// Times one "SFC family" strategy (Morton/Nested): both the TreeBVH path
// (bottomUpSortAndPartition<S>(), then pack()) and PackedBVH's direct SFC-build constructor with
// the same curve S, then cross-checks both results against brute force.
template <class S>
StrategyResult
runSFCFamily(const char*              a_label,
             const std::vector<Vec3>& a_positions,
             const std::vector<Vec3>& a_queryPoints,
             size_t                   a_targetLeafSize)
{
  EBGeometry::SimpleTimer timer;

  timer.start();
  auto tree = std::make_shared<Tree>(makeWrappedPrimitives(a_positions));
  tree->template bottomUpSortAndPartition<S>();
  timer.stop();
  const double treeBuildTime = timer.seconds();

  timer.start();
  auto packed = tree->pack();
  timer.stop();
  const double packTime = timer.seconds();

  timer.start();
  const Packed direct(makeFlatPrimitives(a_positions), a_targetLeafSize, S{});
  timer.stop();
  const double directBuildTime = timer.seconds();

  checkAgainstBruteForce(a_label, *packed, a_positions, a_queryPoints);
  checkAgainstBruteForce(a_label, direct, a_positions, a_queryPoints);

  return {treeBuildTime, packTime, directBuildTime};
}

// Times ClusterSAH: PackedBVH's direct ClusterSAH constructor (density-adaptive clustering, then SAH
// over the clusters). Direct-only -- there is no TreeBVH path -- so treeBuild/pack are reported as
// "--". Cross-checks the result against brute force.
StrategyResult
runClusterSAH(const char*              a_label,
              const std::vector<Vec3>& a_positions,
              const std::vector<Vec3>& a_queryPoints,
              size_t                   a_maxClusterSize)
{
  EBGeometry::SimpleTimer timer;

  timer.start();
  const Packed direct(makeFlatPrimitives(a_positions), EBGeometry::BVH::ClusterSpec{a_maxClusterSize});
  timer.stop();
  const double directBuildTime = timer.seconds();

  checkAgainstBruteForce(a_label, direct, a_positions, a_queryPoints);

  return {-1.0, -1.0, directBuildTime}; // treeBuild/pack sentinel: direct-only strategy
}

} // namespace

int
main()
{
  // TLDR: this program builds the same random point cloud into a BVH via all the construction
  //       strategies EBGeometry offers -- top-down with the default centroid partitioner, SAH,
  //       and the sort-less midpoint-split partitioner; bottom-up along the Morton, Nested, and
  //       Hilbert space-filling curves; and ClusterSAH (density-adaptive clustering, then SAH over
  //       the clusters -- direct-only). For the first six it times both ways of reaching a queryable
  //       PackedBVH: the traditional TreeBVH-then-pack() path, and PackedBVH's direct constructor
  //       (no TreeBVH at all); ClusterSAH has only a direct path. It exists to give a reproducible,
  //       versioned answer to a question
  //       raised in EBGeometry issue #92 (rather than "standalone scratch benchmarks, not
  //       committed anywhere"): whether SFC-based construction still beats top-down construction
  //       once shared_ptr allocation overhead is removed from consideration.
  //
  // For the TreeBVH path, two numbers are reported: the time to build and partition the TreeBVH
  // alone, and the additional time to pack() it into a queryable PackedBVH -- since a bare
  // TreeBVH cannot answer queries, their sum ("Total") is the fair number to compare directly
  // against the direct constructor's single "Direct build" number, which already produces a
  // queryable PackedBVH.
  //
  // Every resulting PackedBVH (all strategies, both paths where applicable) is cross-checked against a
  // brute-force scan for a handful of random query points, so a build strategy that produced a
  // wrong tree would be caught here, not just timed.

  const std::vector<size_t> sizes = {500'000};

  // Target leaf size for the SFC-family direct constructors. Chosen to match K, the leaf
  // occupancy every strategy naturally converges to (topDownSortAndPartition()'s default
  // LeafPredicate stops once a node holds fewer than K primitives; bottomUpSortAndPartition()'s
  // leaf count likewise divides the primitives into groups of roughly K), so the comparison isn't
  // skewed by one strategy simply building shallower or deeper trees than the others.
  constexpr size_t targetLeafSize = K;

  constexpr int numQueryPoints = 500;

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

    using LeafPred          = typename Tree::LeafPredicate;
    const LeafPred stopCrit = [](const Tree& a_node) noexcept -> bool { return a_node.getPrimitives().size() < K; };

    const auto topDown = runPartitionerFamily(
      "TopDown",
      positions,
      queryPoints,
      [](Tree& a_tree) { a_tree.topDownSortAndPartition(); },
      EBGeometry::BVH::BVCentroidPartitioner<T, Point, AABB, K>,
      stopCrit);

    const auto sah = runPartitionerFamily(
      "SAH",
      positions,
      queryPoints,
      [stopCrit](Tree& a_tree) {
        a_tree.topDownSortAndPartition(EBGeometry::BVH::BinnedSAHPartitioner<T, Point, AABB, K>, stopCrit);
      },
      EBGeometry::BVH::BinnedSAHPartitioner<T, Point, AABB, K>,
      stopCrit);

    const auto midpoint = runPartitionerFamily(
      "Midpoint",
      positions,
      queryPoints,
      [stopCrit](Tree& a_tree) {
        a_tree.topDownSortAndPartition(EBGeometry::BVH::MidpointPartitioner<T, Point, AABB, K>, stopCrit);
      },
      EBGeometry::BVH::MidpointPartitioner<T, Point, AABB, K>,
      stopCrit);

    const auto morton = runSFCFamily<EBGeometry::SFC::Morton>("Morton", positions, queryPoints, targetLeafSize);

    const auto nested = runSFCFamily<EBGeometry::SFC::Nested>("Nested", positions, queryPoints, targetLeafSize);

    const auto hilbert = runSFCFamily<EBGeometry::SFC::Hilbert>("Hilbert", positions, queryPoints, targetLeafSize);

    // ClusterSAH is direct-only: cluster to <= maxClusterSize primitives, then SAH over the clusters.
    const auto clusterSah = runClusterSAH("ClusterSAH", positions, queryPoints, /*maxClusterSize=*/8);

    std::cout << std::left << std::setw(12) << "Strategy" << std::right << std::setw(16) << "TreeBVH (s)"
              << std::setw(16) << "+ pack() (s)" << std::setw(16) << "Total (s)" << std::setw(18) << "Direct build (s)"
              << "\n";

    auto printRow = [](const char* a_label, const StrategyResult& a_result) {
      std::cout << std::left << std::setw(12) << a_label << std::right;
      if (a_result.treeBuildTime < 0.0) { // direct-only strategy: no TreeBVH path
        std::cout << std::setw(16) << "--" << std::setw(16) << "--" << std::setw(16) << "--";
      }
      else {
        std::cout << std::setw(16) << a_result.treeBuildTime << std::setw(16) << a_result.packTime << std::setw(16)
                  << (a_result.treeBuildTime + a_result.packTime);
      }
      std::cout << std::setw(18) << a_result.directBuildTime << "\n";
    };

    printRow("TopDown", topDown);
    printRow("SAH", sah);
    printRow("Midpoint", midpoint);
    printRow("Morton", morton);
    printRow("Nested", nested);
    printRow("Hilbert", hilbert);
    printRow("ClusterSAH", clusterSah);
  }

  return 0;
}
