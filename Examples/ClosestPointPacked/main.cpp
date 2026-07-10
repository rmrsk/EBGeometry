// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Closest-point search over a random point cloud, using PointAoSoA<T, Meta, W> groups as the
// leaves of a PackedBVH. Unlike Examples/ClosestPointSFC -- which forms the groups up front from
// the cloud's Morton order and builds a PackedBVH directly over them -- this example builds a
// TreeBVH over the *individual* points first (SAH), then materialises the PointAoSoA groups from
// each leaf via TreeBVH::packWith(). This is the same pattern the library's own TriMeshSDF uses.
// Letting SAH partition all N points, rather than a coarser set of pre-formed groups, tends to
// build a higher-quality tree -- fewer leaf visits per query. See README.md.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
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

namespace {

/**
 * @brief The BVH primitive during construction: a single cloud point plus its cloud index.
 * @details The TreeBVH is built over these individual points -- one primitive per point -- so SAH
 * partitions the full cloud. packWith() then coalesces each leaf's points into PointAoSoA groups,
 * carrying @c index along as each point's lane metadata.
 */
struct Point
{
  EBGeometry::Vec3T<T> position; ///< The point's position in the cloud.
  size_t               index;    ///< The point's index in the original cloud (becomes lane metadata).
};

} // namespace

// Type aliases. PointGroup is the leaf primitive of the *packed* tree (W points + each point's
// cloud index as metadata); PointTree is the intermediate per-point tree that packWith() converts
// into the by-value (ValueStorage) PackedBVH the queries run against.
using Vec3       = EBGeometry::Vec3T<T>;
using AABB       = EBGeometry::BoundingVolumes::AABBT<T>;
using PointGroup = EBGeometry::PointAoSoA<T, size_t, W>;
using PointTree  = EBGeometry::BVH::TreeBVH<T, Point, AABB, K>;
using Packed     = EBGeometry::BVH::PackedBVH<T, PointGroup, K, EBGeometry::BVH::ValueStorage<PointGroup>>;

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
 * @brief Timing and traversal statistics for the packed BVH.
 */
struct StrategyResult
{
  double buildSeconds     = 0.0; ///< Construction time (filled in by the caller, which times it).
  double querySeconds     = 0.0; ///< Total time for all queries.
  double avgLeafVisits    = 0.0; ///< Average leaf nodes visited per query.
  double avgGroupsPerLeaf = 0.0; ///< Average groups per visited leaf (the leaf occupancy achieved).
};

/**
 * @brief Leaf-conversion callback for TreeBVH::packWith(): coalesce one leaf's points into groups.
 * @details Called once per TreeBVH leaf. Chunks the leaf's contiguous run of points into
 * consecutive PointAoSoA groups of up to W each; because a leaf holds up to maxLeafGroups*W points,
 * only the final group of a leaf is ever partially filled, so the groups stay nearly full. Each
 * point's cloud index rides along as its lane metadata. This mirrors TriMeshSDF::groupTrianglesIntoSoA.
 * @param[in] a_points The full primitive list of the tree; only [a_offset, a_offset+a_count) is this leaf.
 * @param[in] a_offset Index of this leaf's first point in a_points.
 * @param[in] a_count  Number of points in this leaf.
 * @return The PointAoSoA groups covering this leaf's points.
 */
std::vector<PointGroup>
groupPointsIntoSoA(const std::vector<std::shared_ptr<const Point>>& a_points, uint32_t a_offset, uint32_t a_count)
{
  constexpr uint32_t soaWidth = static_cast<uint32_t>(W);

  const uint32_t numGroups = (a_count + soaWidth - 1U) / soaWidth;

  std::vector<PointGroup> groups;
  groups.reserve(numGroups);

  for (uint32_t g = 0; g < numGroups; g++) {
    const uint32_t groupOffset = a_offset + g * soaWidth;
    const uint32_t groupCount  = std::min(soaWidth, a_count - g * soaWidth);

    std::array<Vec3, W>   posArr;
    std::array<size_t, W> metaArr;
    for (uint32_t i = 0; i < groupCount; i++) {
      EBGEOMETRY_EXPECT(a_points[groupOffset + i] != nullptr);

      posArr[i]  = a_points[groupOffset + i]->position;
      metaArr[i] = a_points[groupOffset + i]->index;
    }

    PointGroup group;
    group.pack(posArr.data(), metaArr.data(), groupCount);
    groups.push_back(group);
  }

  return groups;
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
 * @brief Print one results-table row.
 * @details Assumes std::cout is already in std::fixed mode (set by the caller before the table).
 * @param[in] a_label        Row name for the leftmost column.
 * @param[in] a_result       The timing and traversal statistics.
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
  std::cout << "ClosestPointPacked: closest-point search over a " << numPoints << "-point cloud in the unit cube\n";
  std::cout << "  Precision T             = " << (std::is_same_v<T, float> ? "float" : "double") << '\n';
  std::cout << "  SoA width W             = " << W << " (PointSoA::DefaultWidth<T>())\n";
  std::cout << "  BVH branching factor K  = " << K << " (BVH::DefaultBranchingRatio<T>())\n";
  std::cout << "  Points                  = " << numPoints << '\n';
  std::cout << "  Queries                 = " << numQueries << '\n';
  std::cout << "  Target leaf size        = " << maxLeafGroups << " groups (" << maxLeafGroups * W << " points)\n\n";

  const std::vector<Vec3> positions   = EBGeometry::Random::samplePoints<T>(numPoints, pointSeed);
  const std::vector<Vec3> queryPoints = EBGeometry::Random::samplePoints<T>(numQueries, querySeed);

  // Build a TreeBVH over the individual points (one primitive per point, its bounding volume the
  // degenerate box at the point), SAH-partitioned down to maxLeafGroups*W points per leaf, then
  // packWith() to coalesce each leaf's points into PointAoSoA groups in place. The whole build --
  // per-point tree plus pack -- is timed as one, to compare fairly with ClosestPointSFC's SAH row.
  EBGeometry::SimpleTimer timer;
  timer.start();

  EBGeometry::BVH::PrimAndBVList<Point, AABB> primsAndBVs;
  primsAndBVs.reserve(numPoints);
  for (size_t i = 0; i < positions.size(); i++) {
    primsAndBVs.emplace_back(std::make_shared<Point>(Point{positions[i], i}), AABB(positions[i], positions[i]));
  }

  auto tree = std::make_shared<PointTree>(primsAndBVs);

  const size_t                                            maxLeafSize = maxLeafGroups * W;
  const EBGeometry::BVH::LeafPredicate<T, Point, AABB, K> stopCrit =
    // maxLeafSize is a constant expression (maxLeafGroups and W are constexpr), so a named capture
    // ([maxLeafSize]) is "unused" to clang (-Wunused-lambda-capture) yet required by g++; the [=]
    // capture-default satisfies both.
    [=](const PointTree& a_node) noexcept -> bool { return a_node.getPrimitives().size() <= maxLeafSize; };
  tree->topDownSortAndPartition(EBGeometry::BVH::BinnedSAHPartitioner<T, Point, AABB, K>, stopCrit);

  using Converter = decltype(&groupPointsIntoSoA);
  const std::shared_ptr<Packed> packed =
    tree->packWith<PointGroup, Converter, EBGeometry::BVH::ValueStorage<PointGroup>>(&groupPointsIntoSoA);

  timer.stop();
  const double buildSeconds = timer.seconds();

  // Brute-force ground truth for every query, plus its own timing (the baseline to beat).
  std::vector<Closest> bruteForce;
  bruteForce.reserve(numQueries);
  timer.start();
  for (const auto& q : queryPoints) {
    bruteForce.push_back(bruteForceClosest(positions, q));
  }
  timer.stop();
  const double bruteSeconds = timer.seconds();

  StrategyResult sah = benchmarkStrategy(*packed, positions, queryPoints, bruteForce);
  sah.buildSeconds   = buildSeconds;

  // How full packWith left the groups: building the tree over individual points and only splitting
  // the last group of each leaf keeps groups nearly full, i.e. average occupancy close to W.
  const size_t numGroups = packed->getPrimitives().size();
  const double occupancy = double(numPoints) / double(numGroups);

  std::cout << "Groups formed: " << numGroups << " for " << numPoints << " points (avg " << std::fixed
            << std::setprecision(2) << occupancy << " of " << W << " points per group)\n\n";

  // Query time is the headline; build time, leaf visits, and groups-per-leaf are shown alongside
  // because they explain it (a tighter tree visits fewer leaves and queries faster).
  std::cout << std::left << std::setw(20) << "Strategy" << std::right << std::setw(13) << "Build (ms)" << std::setw(15)
            << "Query (us/q)" << std::setw(11) << "Speedup" << std::setw(14) << "Leaf visits" << std::setw(14)
            << "Groups/leaf" << '\n';
  std::cout << std::string(87, '-') << '\n';

  std::cout << std::fixed;
  std::cout << std::left << std::setw(20) << "Brute force" << std::right << std::setw(13) << std::setprecision(2)
            << "--" << std::setw(15) << std::setprecision(3) << 1.0e6 * bruteSeconds / double(numQueries)
            << std::setw(11) << std::setprecision(1) << "1.0x" << std::setw(14) << "--" << std::setw(14) << "--"
            << '\n';

  printRow("SAH (packWith)", sah, bruteSeconds);

  return 0;
}
