// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// A benchmark of EBGeometry's BVH build strategies. The same random point cloud is built into a BVH
// with every BVH::Strategy (and, for Strategy::SpaceFillingCurve, every BVH::Curve), and for each one
// the example times
//
//   1. BVH::buildTopology() alone -- the tree's shape, before any primitive is stored,
//   2. the full PackedBVH construction -- the shape plus the primitives copied into leaf order, and
//   3. a fixed query workload -- the closest cloud point to each of a few thousand random query
//      points, found with PackedBVH::pruneTraverse(),
//
// and prints them as a table, together with the depth and node count of each tree. Every strategy
// must find the same closest points; the example checks that, and a sample of the queries against a
// brute-force scan, and fails if any disagree. See README.md.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>
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

// The library's default branching factor (BVH::DefaultBranchingRatio<T>() is 4 on every machine).
constexpr size_t K = EBGeometry::BVH::DefaultBranchingRatio<T>();

// A minimal point primitive -- deliberately example-local, not a library type. A PackedBVH asks
// nothing of its primitive but that it be trivially copyable; the builder only sees its bounding box.
struct Point
{
  Vec3 m_pos;
};

using Packed = EBGeometry::BVH::PackedBVH<T, Point, K>;

// Run configuration: large enough for the build times to mean something, small enough that the whole
// benchmark finishes in seconds even in a Debug build.
constexpr size_t        numPoints    = 500'000;
constexpr size_t        numQueries   = 4'000;
constexpr size_t        numVerified  = 50;
constexpr std::uint64_t pointSeed    = 123456789ULL;
constexpr std::uint64_t querySeed    = 987654321ULL;
constexpr uint32_t      leafCapacity = 4;

namespace {

// One row of the benchmark: a build specification and a label for it.
struct Candidate
{
  const char*                label;
  EBGeometry::BVH::BuildSpec spec;
};

// What one strategy measured.
struct Result
{
  double topologyTime; // BVH::buildTopology() alone.
  double packedTime;   // The full PackedBVH construction (which runs buildTopology() itself).
  double queryTime;    // All closest-point queries.
  size_t depth;        // Node levels.
  size_t nodes;        // Number of wide nodes.

  std::vector<T> closest; // Per query: the squared distance to the closest cloud point.
};

// The closest-point search state carried through PackedBVH::pruneTraverse().
struct Nearest
{
  T        dist2 = std::numeric_limits<T>::infinity();
  uint32_t index = std::numeric_limits<uint32_t>::max();
};

// The closest primitive to a_query, as an index into the BVH's own (leaf-ordered) primitive array.
Nearest
closestPoint(const Packed& a_bvh, const Vec3& a_query) noexcept
{
  const auto primitives = a_bvh.getPrimitives();

  Nearest state;

  // Leaf visit: scan the leaf's primitives and keep the closest. Pruning bound: the best squared
  // distance so far, so every slot whose box is farther away than that is skipped.
  a_bvh.pruneTraverse(
    a_query,
    state,
    [&primitives, &a_query](Nearest& a_state, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = a_offset; i < a_offset + a_count; i++) {
        const T d2 = (primitives[uint32_t(i)].m_pos - a_query).length2();

        if (d2 < a_state.dist2) {
          a_state.dist2 = d2;
          a_state.index = uint32_t(i);
        }
      }
    },
    [](const Nearest& a_state) noexcept -> T { return a_state.dist2; });

  return state;
}

// Build, time and query one strategy.
Result
runStrategy(const EBGeometry::BVH::BuildSpec& a_spec,
            const std::vector<Vec3>&          a_positions,
            const std::vector<AABB>&          a_boxes,
            const std::vector<Vec3>&          a_queries)
{
  EBGeometry::SimpleTimer timer;

  Result result;

  // 1. The shape of the tree alone.
  timer.start();
  const auto topology = EBGeometry::BVH::buildTopology<T, K>(a_boxes, a_spec);
  timer.stop();

  result.topologyTime = timer.seconds();

  // 2. The full construction. The (primitive, box) list is set up outside the timer: it is the input,
  // and the same for every strategy.
  std::vector<std::pair<Point, AABB>> primsAndBVs;

  primsAndBVs.reserve(a_positions.size());

  for (size_t i = 0; i < a_positions.size(); i++) {
    primsAndBVs.emplace_back(Point{a_positions[i]}, a_boxes[i]);
  }

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  timer.start();
  const Packed bvh(pool, std::move(primsAndBVs), a_spec);
  timer.stop();

  result.packedTime = timer.seconds();

  const auto nodes = bvh.getNodes();

  result.nodes = nodes.size();
  result.depth = EBGeometry::BVH::treeDepth(nodes.begin(), nodes.size());

  // 3. The query workload.
  result.closest.resize(a_queries.size());

  timer.start();

  for (size_t q = 0; q < a_queries.size(); q++) {
    result.closest[q] = closestPoint(bvh, a_queries[q]).dist2;
  }

  timer.stop();

  result.queryTime = timer.seconds();

  // The topology was built only to be timed.
  (void)topology;

  return result;
}

} // namespace

int
main()
{
  std::cout << "BuildBVH: every BVH build strategy over a " << numPoints << "-point cloud in the unit cube\n";
  std::cout << "  Precision T      = " << (std::is_same_v<T, float> ? "float" : "double") << '\n';
  std::cout << "  Branching K      = " << K << '\n';
  std::cout << "  Max leaf size    = " << leafCapacity << '\n';
  std::cout << "  Closest-point    = " << numQueries << " queries\n\n";

  const std::vector<Vec3> positions = EBGeometry::Random::samplePoints<T>(numPoints, pointSeed);
  const std::vector<Vec3> queries   = EBGeometry::Random::samplePoints<T>(numQueries, querySeed);

  // A point's bounding box is the degenerate box at the point.
  std::vector<AABB> boxes;

  boxes.reserve(positions.size());

  for (const auto& p : positions) {
    boxes.emplace_back(p, p);
  }

  using EBGeometry::BVH::Curve;
  using EBGeometry::BVH::Strategy;

  const std::vector<Candidate> candidates = {
    {"SAH", {Strategy::SAH, Curve::Morton, leafCapacity}},
    {"Centroid", {Strategy::Centroid, Curve::Morton, leafCapacity}},
    {"Midpoint", {Strategy::Midpoint, Curve::Morton, leafCapacity}},
    {"ClusterSAH", {Strategy::ClusterSAH, Curve::Morton, leafCapacity}},
    {"SFC/Morton", {Strategy::SpaceFillingCurve, Curve::Morton, leafCapacity}},
    {"SFC/Nested", {Strategy::SpaceFillingCurve, Curve::Nested, leafCapacity}},
    {"SFC/Hilbert", {Strategy::SpaceFillingCurve, Curve::Hilbert, leafCapacity}},
  };

  std::cout << std::left << std::setw(14) << "Strategy" << std::right << std::setw(16) << "Topology (s)"
            << std::setw(16) << "PackedBVH (s)" << std::setw(8) << "Depth" << std::setw(10) << "Nodes" << std::setw(14)
            << "Queries (s)" << '\n';

  std::cout << std::fixed << std::setprecision(6);

  std::vector<Result> results;

  for (const auto& candidate : candidates) {
    results.push_back(runStrategy(candidate.spec, positions, boxes, queries));

    const Result& r = results.back();

    std::cout << std::left << std::setw(14) << candidate.label << std::right << std::setw(16) << r.topologyTime
              << std::setw(16) << r.packedTime << std::setw(8) << r.depth << std::setw(10) << r.nodes << std::setw(14)
              << r.queryTime << '\n';
  }

  // Check the answers. Every strategy must find a point at the same distance as a brute-force scan
  // (for a sample of the queries) and as every other strategy (for all of them). Distances are
  // compared rather than indices, since two cloud points may be equally close, and exactly, since
  // every path computes them with the same arithmetic.
  bool ok = true;

  for (size_t q = 0; q < numVerified; q++) {
    T best = std::numeric_limits<T>::infinity();

    for (size_t i = 0; i < positions.size(); i++) {
      best = std::min(best, (positions[i] - queries[q]).length2());
    }

    for (size_t s = 0; s < results.size(); s++) {
      if (results[s].closest[q] != best) {
        std::cerr << "Mismatch: " << candidates[s].label << " disagrees with brute force on query " << q << '\n';
        ok = false;
      }
    }
  }

  for (size_t q = 0; q < numQueries; q++) {
    for (size_t s = 1; s < results.size(); s++) {
      if (results[s].closest[q] != results[0].closest[q]) {
        std::cerr << "Mismatch: " << candidates[s].label << " disagrees with " << candidates[0].label << " on query "
                  << q << '\n';
        ok = false;
      }
    }
  }

  std::cout << '\n'
            << (ok ? "All strategies agree with each other, and with brute force on the first "
                   : "FAILED: the strategies disagree (see above); brute force checked the first ")
            << numVerified << " queries.\n";

  return ok ? 0 : 1;
}
