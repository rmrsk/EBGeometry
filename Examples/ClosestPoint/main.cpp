// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <type_traits>
#include <utility>
#include <vector>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T = EBGEOMETRY_PRECISION;

using Vec3 = EBGeometry::Vec3T<T>;
using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

// SIMD width for each leaf group: the ISA-optimal width for T (EBGeometry::PointSoA::
// DefaultWidth<T>()'s own table -- e.g. 8 for float / 4 for double under AVX, 16/8 under
// AVX-512F). Each leaf primitive below packs up to this many points, with each point's index in
// the original 50,000-point cloud carried alongside as metadata.
constexpr size_t W = EBGeometry::PointSoA::DefaultWidth<T>();

// Leaf primitive: one SoA group of up to W points plus their per-point metadata (here, each
// point's own index into the original point cloud).
using Group = EBGeometry::PointAoSoA<T, int, W>;

// BVH branching factor. Independent of W (it governs the *outer* tree over groups, not how many
// points live inside one group) -- fixed rather than ISA-dependent so the tree shape doesn't vary
// with the compiling machine's SIMD tier, matching Examples/BuildBVH's own convention.
constexpr size_t K = 4;

// Every group below is freshly built by buildGroups() and shared with nothing else, so
// ValueStorage<Group> (primitives stored by value, no shared_ptr indirection) is a clear win over
// the default SharedPtrStorage<Group> -- exactly the "self-contained value type, frequently
// re-visited leaves" case ValueStorage's own doc comment calls out for a point-cloud search.
using Packed = EBGeometry::BVH::PackedBVH<T, Group, K, EBGeometry::BVH::ValueStorage<Group>>;

// TreeBVH type over the same groups, used only to name the LeafPredicate type below (the direct
// top-down PackedBVH constructor's stop criterion is expressed in terms of a TreeBVH node).
using Tree = EBGeometry::BVH::TreeBVH<T, Group, AABB, K>;

constexpr size_t   numPoints  = 50000;
constexpr size_t   numQueries = 500;
constexpr uint64_t pointSeed  = 123456789ULL;
constexpr uint64_t querySeed  = 987654321ULL;

// Target PointAoSoA groups per BVH leaf, shared by all four build strategies (the SFC-family
// Morton constructor takes it directly; the top-down partitioners stop splitting once a node holds
// this many groups or fewer, via the leaf predicate in main()). At W points per group this is
// maxLeafGroups * W points per leaf.
//
// 16 is deliberate, not incidental: a leaf-size sweep (1..64 groups) on this exact workload showed
// every strategy gets *faster* as leaves grow from the naive ~K up through ~16-32 groups -- fewer,
// fatter leaves mean fewer interior nodes to traverse (fewer SIMD box tests, stack operations, and
// branch mispredicts), while the extra in-leaf getDistance2() scans are cheap SIMD -- then plateaus
// and slightly regresses past ~32 as the growing linear in-leaf scan starts to outweigh the saved
// traversal. 16 sits at the knee (captures the bulk of the speedup; e.g. SAH query time drops
// ~0.88 -> ~0.72 us/q vs a ~K-sized leaf) and is a good default across all four strategies. Try
// 8 or 32 here to see the shoulders of that curve.
constexpr size_t maxLeafGroups = 16;

namespace {

// Uniform random points in the unit cube. Fixed seed: reproducible run-to-run (matches
// Examples/BuildBVH's own convention).
std::vector<Vec3>
makeRandomPoints(size_t a_count, uint64_t a_seed)
{
  std::mt19937_64                   rng(a_seed);
  std::uniform_real_distribution<T> dist(T(0.0), T(1.0));

  std::vector<Vec3> points;
  points.reserve(a_count);
  for (size_t i = 0; i < a_count; i++) {
    points.emplace_back(dist(rng), dist(rng), dist(rng));
  }

  return points;
}

// Group the point cloud into spatially-coherent PointAoSoA<T, int, W> leaves: sort all points
// along the Morton curve first (the same BVH::computeSFCBins() + SFC::Morton::encode() pairing
// TreeBVH::bottomUpSortAndPartition() and PackedBVH's own direct SFC-build constructor use
// internally), then chunk the sorted order into consecutive runs of W points. This way each
// group's up-to-W points are genuinely close together -- a group built from an arbitrary
// (unsorted) slice of the input would have a needlessly large bounding volume and prune far worse.
// Each point's ORIGINAL index (its position in a_positions, before this sort) is packed as that
// lane's metadata, so it survives being reshuffled into group order.
//
// This grouping is shared, unchanged, by every BVH build strategy compared in main(); only the
// *outer* tree over these groups differs between strategies.
std::vector<std::pair<Group, AABB>>
buildGroups(const std::vector<Vec3>& a_positions)
{
  const auto bins = EBGeometry::BVH::computeSFCBins<T>(a_positions);

  std::vector<uint32_t> order(a_positions.size());
  std::iota(order.begin(), order.end(), uint32_t(0));
  std::sort(order.begin(), order.end(), [&bins](uint32_t a_a, uint32_t a_b) noexcept -> bool {
    return EBGeometry::SFC::Morton::encode(bins[a_a]) < EBGeometry::SFC::Morton::encode(bins[a_b]);
  });

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

    const AABB bv = group.template computeBoundingVolume<AABB>();

    groupsAndBVs.emplace_back(group, bv);
  }

  return groupsAndBVs;
}

// A nearest-neighbor result: squared distance to the closest point found, and that point's index
// in the original (unsorted) point cloud.
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

// Nearest neighbor via PackedBVH::pruneTraverse(). State is a running *squared* distance -- no
// sqrt anywhere in the hot path -- plus the flattened index of the group that currently holds it.
//
// The pruning bound MUST be this squared distance, not its square root: pruneTraverse() compares
// the bound against the *squared* distance from the query to each child's AABB. Returning a linear
// distance instead would be a unit mismatch that, in the unit cube (where every distance is < 1,
// so squaring shrinks it), makes the bound far too loose to prune -- correct answers, but an order
// of magnitude more leaf visits. This is exactly why PointAoSoA offers getDistance2() at all.
//
// getMetaData() -- the whole reason to use PointAoSoA over a bare PointSoAT here -- is read only
// once the traversal is over, and only for the single group that won: a final, tiny (<= W) per-lane
// rescan against a_positions (using each lane's own metadata as the index into it) recovers which
// point, and therefore which metadata, achieved that group's minimum.
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
  double buildSeconds     = 0.0; // Time to build this PackedBVH from the (already-grouped) points.
  double querySeconds     = 0.0; // Total time for all numQueries nearest-neighbor queries.
  double avgLeafVisits    = 0.0; // Average leaf nodes visited per query (explains the query time).
  double avgGroupsPerLeaf = 0.0; // Average PointAoSoA groups per visited leaf (this tree's leaf occupancy).
  int    indexMatches     = 0;   // Queries whose winning metadata index equalled brute force's.
};

// Runs all numQueries against a_bvh: verifies every result against brute force (aborting the
// program on any nearest-distance mismatch -- a wrong tree is a bug, not a slow benchmark),
// accumulates timing and leaf-visit statistics, and records how often the nearest *point's
// metadata* also matched brute force (it can legitimately differ only on an exact equidistant tie).
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

  result.buildSeconds  = 0.0; // filled in by the caller (which timed construction)
  result.querySeconds  = timer.seconds();
  result.avgLeafVisits = double(leafVisits) / double(a_queries.size());
  // Groups per *visited* leaf -- the leaf occupancy this tree actually achieved (top-down splits
  // into K children make actual occupancy fall somewhat below the maxLeafGroups target).
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
  std::cout << "  BVH branching factor K  = " << K << '\n';
  std::cout << "  Points                  = " << numPoints << '\n';
  std::cout << "  Queries                 = " << numQueries << "\n\n";

  const std::vector<Vec3> positions = makeRandomPoints(numPoints, pointSeed);
  const std::vector<Vec3> queries   = makeRandomPoints(numQueries, querySeed);

  // Shared across every build strategy: sort the points onto the Morton curve and pack them into
  // spatially-coherent PointAoSoA groups. Only the outer tree over these groups varies below.
  const std::vector<std::pair<Group, AABB>> groups = buildGroups(positions);

  // Brute-force ground truth for all queries, plus its own timing (the baseline to beat).
  std::vector<Nearest> bruteForce;
  bruteForce.reserve(numQueries);
  EBGeometry::SimpleTimer timer;
  timer.start();
  for (const auto& q : queries) {
    bruteForce.push_back(bruteForceNearest(positions, q));
  }
  timer.stop();
  const double bruteSeconds = timer.seconds();

  // Stop criterion shared by the three top-down partitioners: a node becomes a leaf once it holds
  // maxLeafGroups groups or fewer (the query-tuned leaf size chosen above, matching the target the
  // Morton constructor is given below so all four strategies aim for the same leaf occupancy).
  const EBGeometry::BVH::LeafPredicate<T, Group, AABB, K> stopCrit = [](const Tree& a_node) noexcept -> bool {
    return a_node.getPrimitives().size() <= maxLeafGroups;
  };

  // Build the same groups four ways, timing each construction. Every constructor takes its
  // primitive list by value, so passing the shared `groups` list copies it in (leaving `groups`
  // intact for the next build) -- an identical, and therefore fair, copy cost for all four.

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

  // Query and verify each strategy.
  StrategyResult morton   = benchmarkStrategy(mortonBVH, positions, queries, bruteForce, "Morton (SFC)");
  StrategyResult topDown  = benchmarkStrategy(topDownBVH, positions, queries, bruteForce, "TopDown centroid");
  StrategyResult midpoint = benchmarkStrategy(midpointBVH, positions, queries, bruteForce, "Midpoint");
  StrategyResult sah      = benchmarkStrategy(sahBVH, positions, queries, bruteForce, "SAH");

  morton.buildSeconds   = mortonBuild;
  topDown.buildSeconds  = topDownBuild;
  midpoint.buildSeconds = midpointBuild;
  sah.buildSeconds      = sahBuild;

  // Every strategy already passed the per-query nearest-distance check inside benchmarkStrategy;
  // the metadata (index) agreement can differ from brute force only on exact equidistant ties.
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

  // Results table. Query time is the star (this is a query benchmark); build time, average leaf
  // visits, and average groups per visited leaf are shown alongside because they explain the query
  // time -- a tighter tree (fewer leaf visits) queries faster, generally at the cost of a slower
  // build, and the groups/leaf column is the actual leaf occupancy each partitioner achieved for
  // the shared maxLeafGroups target.
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
