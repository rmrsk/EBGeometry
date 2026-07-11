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

// A minimal point primitive -- deliberately example-local, not a library type. This example only
// needs some small, cheaply-copyable primitive to benchmark build strategies against; a bare
// position is enough, since every partitioner and SFC build below works off the primitive's
// bounding-volume centroid, never the primitive itself.
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
// a_partitioner/a_stopCrit.
template <class PartitionFn, class Partitioner, class LeafPred>
StrategyResult
runPartitionerFamily(const std::vector<Vec3>& a_positions,
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

  return {treeBuildTime, packTime, directBuildTime};
}

// Times one "SFC family" strategy (Morton/Nested/Hilbert): both the TreeBVH path
// (bottomUpSortAndPartition<S>(), then pack()) and PackedBVH's direct SFC-build constructor with
// the same curve S.
template <class S>
StrategyResult
runSFCFamily(const std::vector<Vec3>& a_positions, size_t a_targetLeafSize)
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

  return {treeBuildTime, packTime, directBuildTime};
}

// Times ClusterSAH: PackedBVH's direct ClusterSAH constructor (density-adaptive clustering, then SAH
// over the clusters). Direct-only -- there is no TreeBVH path -- so treeBuild/pack are reported as
// "--".
StrategyResult
runClusterSAH(const std::vector<Vec3>& a_positions, size_t a_maxClusterSize)
{
  EBGeometry::SimpleTimer timer;

  timer.start();
  const Packed direct(makeFlatPrimitives(a_positions), EBGeometry::BVH::ClusterSpec{a_maxClusterSize});
  timer.stop();
  const double directBuildTime = timer.seconds();

  return {-1.0, -1.0, directBuildTime}; // treeBuild/pack sentinel: direct-only strategy
}

} // namespace

int
main()
{
  // Benchmark: build the same random point cloud into a BVH with every construction strategy
  // EBGeometry offers, and time each. For the top-down and space-filling-curve strategies, both the
  // traditional TreeBVH-then-pack() path and PackedBVH's direct constructor are timed; ClusterSAH is
  // direct-only.

  const std::vector<size_t> sizes = {500'000};

  // Target leaf size for the SFC-family direct constructors. Chosen to match K, the leaf
  // occupancy every strategy naturally converges to (topDownSortAndPartition()'s default
  // LeafPredicate stops once a node holds fewer than K primitives; bottomUpSortAndPartition()'s
  // leaf count likewise divides the primitives into groups of roughly K), so the comparison isn't
  // skewed by one strategy simply building shallower or deeper trees than the others.
  constexpr size_t targetLeafSize = K;

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

    using LeafPred          = typename Tree::LeafPredicate;
    const LeafPred stopCrit = [](const Tree& a_node) noexcept -> bool { return a_node.getPrimitives().size() < K; };

    const auto topDown = runPartitionerFamily(
      positions,
      [](Tree& a_tree) { a_tree.topDownSortAndPartition(); },
      EBGeometry::BVH::BVCentroidPartitioner<T, Point, AABB, K>,
      stopCrit);

    const auto sah = runPartitionerFamily(
      positions,
      [stopCrit](Tree& a_tree) {
        a_tree.topDownSortAndPartition(EBGeometry::BVH::BinnedSAHPartitioner<T, Point, AABB, K>, stopCrit);
      },
      EBGeometry::BVH::BinnedSAHPartitioner<T, Point, AABB, K>,
      stopCrit);

    const auto midpoint = runPartitionerFamily(
      positions,
      [stopCrit](Tree& a_tree) {
        a_tree.topDownSortAndPartition(EBGeometry::BVH::MidpointPartitioner<T, Point, AABB, K>, stopCrit);
      },
      EBGeometry::BVH::MidpointPartitioner<T, Point, AABB, K>,
      stopCrit);

    const auto morton  = runSFCFamily<EBGeometry::SFC::Morton>(positions, targetLeafSize);
    const auto nested  = runSFCFamily<EBGeometry::SFC::Nested>(positions, targetLeafSize);
    const auto hilbert = runSFCFamily<EBGeometry::SFC::Hilbert>(positions, targetLeafSize);

    // ClusterSAH is direct-only: cluster to <= maxClusterSize primitives, then SAH over the clusters.
    const auto clusterSah = runClusterSAH(positions, /*maxClusterSize=*/8);

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
