// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Closest-point search over a random point cloud, using PointAoSoA<T, Meta, W> groups as the
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

// Type aliases. PointGroup is the leaf primitive (W points + each point's cloud index as metadata);
// Packed stores groups by value (ValueStorage, no pointer indirection on the hot path); Tree only
// names the LeafPredicate type the top-down partitioners take.
using Vec3       = EBGeometry::Vec3T<T>;
using AABB       = EBGeometry::BoundingVolumes::AABBT<T>;
using PointGroup = EBGeometry::PointAoSoA<T, size_t, W>;
using Packed     = EBGeometry::BVH::PackedBVH<T, PointGroup, K, EBGeometry::BVH::ValueStorage<PointGroup>>;
using Tree       = EBGeometry::BVH::TreeBVH<T, PointGroup, AABB, K>;

// Run configuration. maxLeafGroups is the target groups per leaf; a leaf-size sweep on this
// workload put the query-time knee at ~16-32, so 16 is a good default (try 8 or 32).
constexpr size_t   numPoints     = 50000;
constexpr size_t   numQueries    = 500;
constexpr uint64_t pointSeed     = 123456789ULL;
constexpr uint64_t querySeed     = 987654321ULL;
constexpr size_t   maxLeafGroups = 16;

namespace {

/**
 * @brief A closest-point search result, doubling as the running state of a BVH traversal.
 * @details Both a query's answer (bruteForceClosest / bvhClosest return one) and the running best
 * carried through a pruneTraverse() are just a squared distance plus an index, so they share this
 * type. During a traversal @c index is the PackedBVH group currently holding the best distance;
 * bvhClosest() resolves it to the cloud point index once the traversal is over. Squared distance
 * throughout -- there is no sqrt on the hot path.
 */
struct Closest
{
  T      dist2 = std::numeric_limits<T>::max(); ///< Squared distance to the closest point found so far.
  size_t index = 0;                             ///< Cloud point index (a group index mid-traversal).
};

/**
 * @brief Timing and traversal statistics for one BVH build strategy.
 */
struct StrategyResult
{
  double buildSeconds     = 0.0; ///< Construction time (filled in by the caller, which times it).
  double querySeconds     = 0.0; ///< Total time for all queries.
  double avgLeafVisits    = 0.0; ///< Average leaf nodes visited per query.
  double avgGroupsPerLeaf = 0.0; ///< Average groups per visited leaf (the leaf occupancy achieved).
};

/**
 * @brief Group the cloud into spatially-coherent PointGroup leaves for the BVH.
 * @details Morton-sorts the points, then chunks the sorted order into consecutive runs of W.
 * Compact groups have tight bounding volumes and prune well. Each point's original index rides along
 * as its lane metadata, surviving the reshuffle.
 * @param[in] a_positions The point cloud.
 * @return One (PointGroup, bounding volume) pair per group.
 */
std::vector<std::pair<PointGroup, AABB>>
buildGroups(const std::vector<Vec3>& a_positions)
{
  const std::vector<uint32_t> order = EBGeometry::SFC::order<EBGeometry::SFC::Morton>(a_positions);

  const size_t numGroups = (a_positions.size() + W - 1U) / W;

  std::vector<std::pair<PointGroup, AABB>> groupsAndBVs;
  groupsAndBVs.reserve(numGroups);

  for (size_t g = 0; g < numGroups; g++) {
    const size_t offset = g * W;
    const size_t count  = std::min(W, a_positions.size() - offset);

    std::array<Vec3, W>   posArr;
    std::array<size_t, W> metaArr;
    for (size_t i = 0; i < count; i++) {
      const uint32_t srcIdx = order[offset + i];

      posArr[i]  = a_positions[srcIdx];
      metaArr[i] = srcIdx;
    }

    PointGroup group;
    group.pack(posArr.data(), metaArr.data(), static_cast<uint32_t>(count));

    groupsAndBVs.emplace_back(group, group.template computeBoundingVolume<AABB>());
  }

  return groupsAndBVs;
}

/**
 * @brief Closest point by brute force -- the ground truth the BVH searches are checked against.
 * @param[in] a_positions The point cloud to search.
 * @param[in] a_query     Query point.
 * @return The closest point's squared distance and its index in a_positions.
 */
Closest
bruteForceClosest(const std::vector<Vec3>& a_positions, const Vec3& a_query)
{
  Closest best;

  for (size_t i = 0; i < a_positions.size(); i++) {
    const T d2 = (a_positions[i] - a_query).length2();
    if (d2 < best.dist2) {
      best.dist2 = d2;
      best.index = i;
    }
  }

  return best;
}

/**
 * @brief Closest point via PackedBVH::pruneTraverse().
 * @details The search state is a running *squared* distance -- pruneTraverse() compares its bound
 * against squared box distances, so there is no sqrt on the hot path -- plus the group currently
 * holding it. Only once the traversal is over, and only for the winning group, is getMetaData() read
 * to recover the actual cloud point index.
 * @param[in]     a_bvh        BVH over the PointGroup leaves.
 * @param[in]     a_positions  The point cloud (for resolving the winning group's metadata to a point).
 * @param[in]     a_query      Query point.
 * @param[in,out] a_leafVisits Accumulates the number of leaf nodes visited (a traversal statistic).
 * @param[in,out] a_groupEvals Accumulates the number of group distance evaluations (a statistic).
 * @return The closest point's squared distance and its index in a_positions.
 */
Closest
bvhClosest(const Packed&            a_bvh,
           const std::vector<Vec3>& a_positions,
           const Vec3&              a_query,
           long long&               a_leafVisits,
           long long&               a_groupEvals)
{
  Closest state; // Until the rescan below, state.index is the winning *group*, not a point.

  const auto& groups = a_bvh.getPrimitives();

  const auto evalLeaf =
    [&groups, &a_query, &a_leafVisits, &a_groupEvals](Closest& a_state, size_t a_offset, size_t a_count) noexcept {
      a_leafVisits++;
      a_groupEvals += static_cast<long long>(a_count);
      for (size_t i = 0; i < a_count; i++) {
        const T d2 = groups[a_offset + i].getDistance2(a_query);
        if (d2 < a_state.dist2) {
          a_state.dist2 = d2;
          a_state.index = a_offset + i;
        }
      }
    };
  const auto pruneDist2 = [](const Closest& a_state) noexcept -> T { return a_state.dist2; };

  a_bvh.pruneTraverse(a_query, state, evalLeaf, pruneDist2);

  // The traversal always evaluates at least one group (the initial infinite bound prunes nothing).
  EBGEOMETRY_EXPECT(state.dist2 < std::numeric_limits<T>::max());

  // Resolve the winning group's metadata to the actual cloud point: rescan its lanes, keeping the
  // closest, which turns state's group index into the point index.
  const PointGroup& winner = groups[state.index];

  Closest best;
  for (size_t lane = 0; lane < W; lane++) {
    const size_t idx = winner.getMetaData(lane);
    const T      d2  = (a_positions[idx] - a_query).length2();
    if (d2 < best.dist2) {
      best.dist2 = d2;
      best.index = idx;
    }
  }

  return best;
}

/**
 * @brief Run every query against a_bvh, timing them and accumulating traversal statistics.
 * @details Correctness is checked with EBGEOMETRY_EXPECT (opt-in via EBGEOMETRY_ENABLE_ASSERTIONS):
 * every query's BVH closest squared distance must match brute force's, or the assertion aborts. A
 * volatile sink keeps each result live so the query -- metadata rescan included -- is not optimized
 * away in no-assertion builds.
 * @param[in] a_bvh        BVH to query.
 * @param[in] a_positions  The point cloud.
 * @param[in] a_queries    Query points.
 * @param[in] a_bruteForce Per-query ground truth to check against.
 * @return Query timing and leaf-visit statistics; build time is filled in by the caller.
 */
StrategyResult
benchmarkStrategy(const Packed&               a_bvh,
                  const std::vector<Vec3>&    a_positions,
                  const std::vector<Vec3>&    a_queries,
                  const std::vector<Closest>& a_bruteForce)
{
  constexpr T tolerance = std::is_same_v<T, float> ? T(1.0e-4) : T(1.0e-9);

  StrategyResult result;

  long long               leafVisits = 0;
  long long               groupEvals = 0;
  volatile T              sink       = T(0); // Consume each result so the query (metadata rescan
  EBGeometry::SimpleTimer timer;             // included) is not optimized away in no-assertion builds.

  timer.start();
  for (size_t q = 0; q < a_queries.size(); q++) {
    const Closest bvhResult = bvhClosest(a_bvh, a_positions, a_queries[q], leafVisits, groupEvals);

    EBGEOMETRY_EXPECT(std::abs(bvhResult.dist2 - a_bruteForce[q].dist2) <=
                      tolerance * std::max(a_bruteForce[q].dist2, T(1.0)));

    sink += bvhResult.dist2;
  }
  timer.stop();
  (void)sink;

  result.querySeconds     = timer.seconds();
  result.avgLeafVisits    = double(leafVisits) / double(a_queries.size());
  result.avgGroupsPerLeaf = (leafVisits > 0) ? double(groupEvals) / double(leafVisits) : 0.0;

  return result;
}

/**
 * @brief Print one results-table row for a build strategy.
 * @details Assumes std::cout is already in std::fixed mode (set by the caller before the table).
 * @param[in] a_label        Strategy name for the leftmost column.
 * @param[in] a_result       That strategy's timing and traversal statistics.
 * @param[in] a_bruteSeconds Brute-force baseline total time, used for the speedup column.
 */
void
printRow(const char* a_label, const StrategyResult& a_result, double a_bruteSeconds)
{
  const double queryUs = 1.0e6 * a_result.querySeconds / double(numQueries);
  const double speedup = a_bruteSeconds / a_result.querySeconds;

  std::cout << std::left << std::setw(20) << a_label << std::right << std::setw(13) << std::setprecision(2)
            << 1.0e3 * a_result.buildSeconds << std::setw(15) << std::setprecision(3) << queryUs << std::setw(10)
            << std::setprecision(1) << speedup << "x" << std::setw(14) << std::setprecision(1) << a_result.avgLeafVisits
            << std::setw(14) << std::setprecision(1) << a_result.avgGroupsPerLeaf << '\n';
}

} // namespace

int
main()
{
  std::cout << "ClosestPoint: closest-point search over a " << numPoints << "-point cloud in the unit cube\n";
  std::cout << "  Precision T             = " << (std::is_same_v<T, float> ? "float" : "double") << '\n';
  std::cout << "  SoA width W             = " << W << " (PointSoA::DefaultWidth<T>())\n";
  std::cout << "  BVH branching factor K  = " << K << " (BVH::DefaultBranchingRatio<T>())\n";
  std::cout << "  Points                  = " << numPoints << '\n';
  std::cout << "  Queries                 = " << numQueries << '\n';
  std::cout << "  Target leaf size        = " << maxLeafGroups << " groups (" << maxLeafGroups * W << " points)\n\n";

  const std::vector<Vec3> positions   = EBGeometry::Random::samplePoints<T>(numPoints, pointSeed);
  const std::vector<Vec3> queryPoints = EBGeometry::Random::samplePoints<T>(numQueries, querySeed);

  // Shared across every build strategy: Morton-sort the points and pack them into spatially-coherent
  // PointGroup leaves. Only the outer tree over these groups varies between strategies.
  const std::vector<std::pair<PointGroup, AABB>> groups = buildGroups(positions);

  // Build the same groups four ways, timing each. Each constructor takes its primitive list by
  // value, so passing `groups` copies it in -- an identical, fair cost for all four.
  const EBGeometry::BVH::LeafPredicate<T, PointGroup, AABB, K> stopCrit = [](const Tree& a_node) noexcept -> bool {
    return a_node.getPrimitives().size() <= maxLeafGroups;
  };

  EBGeometry::SimpleTimer timer;

  timer.start();
  const Packed mortonBVH(groups, maxLeafGroups, EBGeometry::SFC::Morton{});
  timer.stop();
  const double mortonBuild = timer.seconds();

  timer.start();
  const Packed topDownBVH(groups, EBGeometry::BVH::BVCentroidPartitioner<T, PointGroup, AABB, K>, stopCrit);
  timer.stop();
  const double topDownBuild = timer.seconds();

  timer.start();
  const Packed midpointBVH(groups, EBGeometry::BVH::MidpointPartitioner<T, PointGroup, AABB, K>, stopCrit);
  timer.stop();
  const double midpointBuild = timer.seconds();

  timer.start();
  const Packed sahBVH(groups, EBGeometry::BVH::BinnedSAHPartitioner<T, PointGroup, AABB, K>, stopCrit);
  timer.stop();
  const double sahBuild = timer.seconds();

  // Brute-force ground truth for every query, plus its own timing (the baseline to beat).
  std::vector<Closest> bruteForce;
  bruteForce.reserve(numQueries);
  timer.start();
  for (const auto& q : queryPoints) {
    bruteForce.push_back(bruteForceClosest(positions, q));
  }
  timer.stop();
  const double bruteSeconds = timer.seconds();

  StrategyResult morton   = benchmarkStrategy(mortonBVH, positions, queryPoints, bruteForce);
  StrategyResult topDown  = benchmarkStrategy(topDownBVH, positions, queryPoints, bruteForce);
  StrategyResult midpoint = benchmarkStrategy(midpointBVH, positions, queryPoints, bruteForce);
  StrategyResult sah      = benchmarkStrategy(sahBVH, positions, queryPoints, bruteForce);

  morton.buildSeconds   = mortonBuild;
  topDown.buildSeconds  = topDownBuild;
  midpoint.buildSeconds = midpointBuild;
  sah.buildSeconds      = sahBuild;

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

  printRow("Morton (SFC)", morton, bruteSeconds);
  printRow("TopDown centroid", topDown, bruteSeconds);
  printRow("Midpoint", midpoint, bruteSeconds);
  printRow("SAH", sah, bruteSeconds);

  return 0;
}
