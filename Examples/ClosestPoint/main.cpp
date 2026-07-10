// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Nearest-neighbor search over a random point cloud, using PointAoSoA<T, Meta, W> groups as the
// leaves of a PackedBVH. The same groups are built four ways (Morton, top-down centroid, midpoint,
// SAH) and queried against a brute-force baseline. See README.md.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T = EBGEOMETRY_PRECISION;

// SIMD widths, both derived from the ISA so each SIMD op fills one register: W points per group
// (leaf distance test) and K children per BVH node (box test).
constexpr size_t W = EBGeometry::PointSoA::DefaultWidth<T>();
constexpr size_t K = EBGeometry::BVH::DefaultBranchingRatio<T>();

// Type aliases. Group is the leaf primitive (W points + each point's cloud index as metadata);
// Packed stores groups by value (ValueStorage, no pointer indirection on the hot path); Tree only
// names the LeafPredicate type the top-down partitioners take.
using Vec3   = EBGeometry::Vec3T<T>;
using AABB   = EBGeometry::BoundingVolumes::AABBT<T>;
using Group  = EBGeometry::PointAoSoA<T, int, W>;
using Packed = EBGeometry::BVH::PackedBVH<T, Group, K, EBGeometry::BVH::ValueStorage<Group>>;
using Tree   = EBGeometry::BVH::TreeBVH<T, Group, AABB, K>;

// Run configuration. maxLeafGroups is the target groups per leaf; a leaf-size sweep on this
// workload put the query-time knee at ~16-32, so 16 is a good default (try 8 or 32).
constexpr size_t   numPoints     = 50000;
constexpr size_t   numQueries    = 500;
constexpr uint64_t pointSeed     = 123456789ULL;
constexpr uint64_t querySeed     = 987654321ULL;
constexpr size_t   maxLeafGroups = 16;

namespace {

// Group the cloud into spatially-coherent PointAoSoA leaves: Morton-sort the points, then chunk the
// sorted order into consecutive runs of W. Compact groups have tight bounding volumes and prune
// well. Each point's original index rides along as its lane metadata, surviving the reshuffle.
std::vector<std::pair<Group, AABB>>
buildGroups(const std::vector<Vec3>& a_positions)
{
  const std::vector<uint32_t> order = EBGeometry::SFC::order(a_positions, EBGeometry::SFC::Morton{});

  const size_t numGroups = (a_positions.size() + W - 1U) / W;

  std::vector<std::pair<Group, AABB>> groupsAndBVs;
  groupsAndBVs.reserve(numGroups);

  for (size_t g = 0; g < numGroups; g++) {
    const size_t offset = g * W;
    const size_t count  = std::min(W, a_positions.size() - offset);

    std::array<Vec3, W> posArr;
    std::array<int, W>  metaArr;
    for (size_t i = 0; i < count; i++) {
      const uint32_t srcIdx = order[offset + i];

      posArr[i]  = a_positions[srcIdx];
      metaArr[i] = static_cast<int>(srcIdx);
    }

    Group group;
    group.pack(posArr.data(), metaArr.data(), static_cast<uint32_t>(count));

    groupsAndBVs.emplace_back(group, group.template computeBoundingVolume<AABB>());
  }

  return groupsAndBVs;
}

// Squared distance to the closest point found, and that point's index in the cloud.
struct Nearest
{
  T   dist2;
  int index;
};

// Ground truth, and the baseline the BVH searches are benchmarked against.
Nearest
bruteForceNearest(const std::vector<Vec3>& a_positions, const Vec3& a_query)
{
  Nearest best{std::numeric_limits<T>::max(), -1};

  for (size_t i = 0; i < a_positions.size(); i++) {
    const T d2 = (a_positions[i] - a_query).length2();
    if (d2 < best.dist2) {
      best.dist2 = d2;
      best.index = static_cast<int>(i);
    }
  }

  return best;
}

// Nearest neighbor via PackedBVH::pruneTraverse(). The search state is a running *squared* distance
// -- pruneTraverse compares the bound against squared box distances, so there is no sqrt on the hot
// path -- plus the index of the group currently holding it. getMetaData() is read only once the
// traversal is over, and only for the winning group, to recover the actual point index. a_leafVisits
// and a_groupEvals accumulate traversal statistics for the report.
Nearest
bvhNearest(const Packed&            a_bvh,
           const std::vector<Vec3>& a_positions,
           const Vec3&              a_query,
           long long&               a_leafVisits,
           long long&               a_groupEvals)
{
  struct SearchState
  {
    T      dist2    = std::numeric_limits<T>::max();
    size_t groupIdx = 0;
  };

  SearchState state;

  const auto& groups = a_bvh.getPrimitives();

  const auto evalLeaf =
    [&groups, &a_query, &a_leafVisits, &a_groupEvals](SearchState& a_state, size_t a_offset, size_t a_count) noexcept {
      a_leafVisits++;
      a_groupEvals += static_cast<long long>(a_count);
      for (size_t i = 0; i < a_count; i++) {
        const T d2 = groups[a_offset + i].getDistance2(a_query);
        if (d2 < a_state.dist2) {
          a_state.dist2    = d2;
          a_state.groupIdx = a_offset + i;
        }
      }
    };
  const auto pruneDist2 = [](const SearchState& a_state) noexcept -> T { return a_state.dist2; };

  a_bvh.pruneTraverse(a_query, state, evalLeaf, pruneDist2);

  const Group& winner = groups[state.groupIdx];

  Nearest best{std::numeric_limits<T>::max(), -1};
  for (size_t lane = 0; lane < W; lane++) {
    const int idx = winner.getMetaData(lane);
    const T   d2  = (a_positions[idx] - a_query).length2();
    if (d2 < best.dist2) {
      best.dist2 = d2;
      best.index = idx;
    }
  }

  return best;
}

// Timing/verification result for one BVH build strategy.
struct StrategyResult
{
  double buildSeconds     = 0.0; // Construction time (filled in by the caller, which times it).
  double querySeconds     = 0.0; // Total time for all queries.
  double avgLeafVisits    = 0.0; // Average leaf nodes visited per query.
  double avgGroupsPerLeaf = 0.0; // Average groups per visited leaf (the leaf occupancy achieved).
  int    indexMatches     = 0;   // Queries whose winning index equalled brute force's.
};

// Runs every query against a_bvh, aborting on any nearest-distance mismatch vs brute force, and
// accumulates timing, leaf-visit, and metadata-agreement statistics.
StrategyResult
benchmarkStrategy(const Packed&               a_bvh,
                  const std::vector<Vec3>&    a_positions,
                  const std::vector<Vec3>&    a_queries,
                  const std::vector<Nearest>& a_bruteForce,
                  const char*                 a_label)
{
  constexpr T tolerance = std::is_same_v<T, float> ? T(1.0e-4) : T(1.0e-9);

  StrategyResult result;

  long long               leafVisits = 0;
  long long               groupEvals = 0;
  EBGeometry::SimpleTimer timer;

  timer.start();
  for (size_t q = 0; q < a_queries.size(); q++) {
    const Nearest bvhResult = bvhNearest(a_bvh, a_positions, a_queries[q], leafVisits, groupEvals);

    const Nearest& bruteResult = a_bruteForce[q];

    if (std::abs(bvhResult.dist2 - bruteResult.dist2) > tolerance * std::max(bruteResult.dist2, T(1.0))) {
      std::cerr << a_label << ": nearest-distance mismatch (BVH dist2 = " << bvhResult.dist2
                << ", brute-force dist2 = " << bruteResult.dist2 << ")\n";
      std::exit(1);
    }

    if (bvhResult.index == bruteResult.index) {
      result.indexMatches++;
    }
  }
  timer.stop();

  result.querySeconds     = timer.seconds();
  result.avgLeafVisits    = double(leafVisits) / double(a_queries.size());
  result.avgGroupsPerLeaf = (leafVisits > 0) ? double(groupEvals) / double(leafVisits) : 0.0;

  return result;
}

} // namespace

int
main()
{
  std::cout << "ClosestPoint: nearest-neighbor search over a " << numPoints << "-point cloud in the unit cube\n";
  std::cout << "  Precision T             = " << (std::is_same_v<T, float> ? "float" : "double") << '\n';
  std::cout << "  SoA width W             = " << W << " (PointSoA::DefaultWidth<T>())\n";
  std::cout << "  BVH branching factor K  = " << K << " (BVH::DefaultBranchingRatio<T>())\n";
  std::cout << "  Points                  = " << numPoints << '\n';
  std::cout << "  Queries                 = " << numQueries << "\n\n";

  const std::vector<Vec3> positions  = EBGeometry::Random::samplePoints<T>(numPoints, pointSeed);
  const std::vector<Vec3> rawQueries = EBGeometry::Random::samplePoints<T>(numQueries, querySeed);

  // Shared across every build strategy: Morton-sort the points and pack them into spatially-coherent
  // PointAoSoA groups. Only the outer tree over these groups varies between strategies.
  const std::vector<std::pair<Group, AABB>> groups = buildGroups(positions);

  // Build the same groups four ways, timing each. Each constructor takes its primitive list by
  // value, so passing `groups` copies it in -- an identical, fair cost for all four.
  const EBGeometry::BVH::LeafPredicate<T, Group, AABB, K> stopCrit = [](const Tree& a_node) noexcept -> bool {
    return a_node.getPrimitives().size() <= maxLeafGroups;
  };

  EBGeometry::SimpleTimer timer;

  timer.start();
  const Packed mortonBVH(groups, maxLeafGroups, EBGeometry::SFC::Morton{});
  timer.stop();
  const double mortonBuild = timer.seconds();

  timer.start();
  const Packed topDownBVH(groups, EBGeometry::BVH::BVCentroidPartitioner<T, Group, AABB, K>, stopCrit);
  timer.stop();
  const double topDownBuild = timer.seconds();

  timer.start();
  const Packed midpointBVH(groups, EBGeometry::BVH::MidpointPartitioner<T, Group, AABB, K>, stopCrit);
  timer.stop();
  const double midpointBuild = timer.seconds();

  timer.start();
  const Packed sahBVH(groups, EBGeometry::BVH::BinnedSAHPartitioner<T, Group, AABB, K>, stopCrit);
  timer.stop();
  const double sahBuild = timer.seconds();

  // Morton-sort the query batch so every strategy visits queries in the same spatially-coherent
  // order: consecutive queries descend through nearly the same nodes and reuse warm cache (the query
  // is memory-latency bound). A one-time cost that applies to any batch of queries free to reorder.
  std::vector<Vec3> queries;
  queries.reserve(numQueries);
  for (const uint32_t idx : EBGeometry::SFC::order(rawQueries, EBGeometry::SFC::Morton{})) {
    queries.push_back(rawQueries[idx]);
  }

  // Brute-force ground truth for every query, plus its own timing (the baseline to beat).
  std::vector<Nearest> bruteForce;
  bruteForce.reserve(numQueries);
  timer.start();
  for (const auto& q : queries) {
    bruteForce.push_back(bruteForceNearest(positions, q));
  }
  timer.stop();
  const double bruteSeconds = timer.seconds();

  StrategyResult morton   = benchmarkStrategy(mortonBVH, positions, queries, bruteForce, "Morton (SFC)");
  StrategyResult topDown  = benchmarkStrategy(topDownBVH, positions, queries, bruteForce, "TopDown centroid");
  StrategyResult midpoint = benchmarkStrategy(midpointBVH, positions, queries, bruteForce, "Midpoint");
  StrategyResult sah      = benchmarkStrategy(sahBVH, positions, queries, bruteForce, "SAH");

  morton.buildSeconds   = mortonBuild;
  topDown.buildSeconds  = topDownBuild;
  midpoint.buildSeconds = midpointBuild;
  sah.buildSeconds      = sahBuild;

  // Every strategy passed the per-query distance check inside benchmarkStrategy; the metadata index
  // can differ from brute force only on an exact equidistant tie.
  const int worstIndexMatches =
    std::min({morton.indexMatches, topDown.indexMatches, midpoint.indexMatches, sah.indexMatches});

  T avgDist = T(0.0);
  for (const auto& b : bruteForce) {
    avgDist += std::sqrt(b.dist2);
  }
  avgDist /= T(numQueries);

  std::cout << "All four BVH strategies agree with brute force on every query's nearest distance.\n";
  std::cout << "Nearest-point metadata (index) matched brute force in at least " << worstIndexMatches << "/"
            << numQueries << " queries per strategy";
  if (worstIndexMatches != int(numQueries)) {
    std::cout << " (the rest were exact equidistant ties between two points)";
  }
  std::cout << ".\n";
  std::cout << std::setprecision(9);
  std::cout << "Average nearest distance = " << avgDist << '\n';
  std::cout << "Target leaf size         = " << maxLeafGroups << " groups (" << maxLeafGroups * W
            << " points) per leaf\n\n";

  // Query time is the headline; build time, leaf visits, and groups-per-leaf are shown alongside
  // because they explain it (a tighter tree visits fewer leaves and queries faster, usually at the
  // cost of a slower build).
  std::cout << std::left << std::setw(20) << "Strategy" << std::right << std::setw(13) << "Build (ms)" << std::setw(15)
            << "Query (us/q)" << std::setw(11) << "Speedup" << std::setw(14) << "Leaf visits" << std::setw(14)
            << "Groups/leaf" << '\n';
  std::cout << std::string(87, '-') << '\n';

  std::cout << std::fixed;
  std::cout << std::left << std::setw(20) << "Brute force" << std::right << std::setw(13) << std::setprecision(2)
            << "--" << std::setw(15) << std::setprecision(3) << 1.0e6 * bruteSeconds / double(numQueries)
            << std::setw(11) << std::setprecision(1) << "1.0x" << std::setw(14) << "--" << std::setw(14) << "--"
            << '\n';

  const auto printRow = [](const char* a_label, const StrategyResult& a_result, double a_bruteSeconds) {
    const double queryUs = 1.0e6 * a_result.querySeconds / double(numQueries);
    const double speedup = a_bruteSeconds / a_result.querySeconds;

    std::cout << std::left << std::setw(20) << a_label << std::right << std::setw(13) << std::setprecision(2)
              << 1.0e3 * a_result.buildSeconds << std::setw(15) << std::setprecision(3) << queryUs << std::setw(10)
              << std::setprecision(1) << speedup << "x" << std::setw(14) << std::setprecision(1)
              << a_result.avgLeafVisits << std::setw(14) << std::setprecision(1) << a_result.avgGroupsPerLeaf << '\n';
  };

  printRow("Morton (SFC)", morton, bruteSeconds);
  printRow("TopDown centroid", topDown, bruteSeconds);
  printRow("Midpoint", midpoint, bruteSeconds);
  printRow("SAH", sah, bruteSeconds);

  return 0;
}
