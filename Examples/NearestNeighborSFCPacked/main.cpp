// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// All-k-nearest-neighbors (kNN = 1) over a random point cloud, using PointAoSoA<T, Meta, W> groups as
// the leaves of a PackedBVH. Unlike Examples/ClosestPointSFCPacked -- which resolves a batch of
// independent closest-point queries (one nearest point each) -- this example finds, for *every*
// point in the cloud, its kNN nearest neighbors (the classic k-NN graph). It mirrors
// ClosestPointSFCPacked's organization: the points are SFC-sorted and chunked into groups up front,
// then the same groups are built into a PackedBVH five ways via the direct constructors. The novel
// piece is *reciprocal distance culling*: when point A's traversal discovers neighbor B at distance
// d, A is recorded as a candidate neighbor of B too (distance is symmetric), tightening B's search
// radius before B is ever processed, so B's later traversal prunes harder. See README.md.

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
// (leaf distance kernel) and K children per BVH node (box test).
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

// Run configuration. kNN is the number of nearest neighbors per point (hard-coded to 1). numVerify
// points are checked against a brute-force scan (full all-pairs would be O(N^2)). maxLeafGroups is
// the target groups per leaf.
constexpr size_t   numPoints     = 500000;
constexpr size_t   kNN           = 1;
constexpr size_t   numVerify     = 500;
constexpr uint64_t pointSeed     = 123456789ULL;
constexpr size_t   maxLeafGroups = 16;
// ClusterSAH bucket size (in groups). Its leaves hold up to a few buckets, so this is chosen so its
// leaves are comparable to the other strategies' maxLeafGroups; tune to trade build time vs query.
constexpr size_t maxClusterGroups = 8;

namespace {

/**
 * @brief A point's kNN nearest neighbors so far, doubling as the running state of its BVH traversal.
 * @details Holds up to @c kNN (cloud index, squared distance) pairs, kept sorted by ascending squared
 * distance. worst2() -- the current k-th distance, or +inf until k are found -- is the pruneTraverse
 * bound: it is always a valid *upper* bound on the point's true k-th-nearest distance (every entry
 * is a real point at a real distance), so using it to prune never discards a true neighbor. Squared
 * distance throughout -- no sqrt on the hot path.
 */
struct Knn
{
  std::array<T, kNN>      dist2 = {}; ///< Squared distances to the k best, ascending (valid for lanes < count).
  std::array<size_t, kNN> index = {}; ///< Cloud indices of the k best, parallel to dist2.
  size_t                  count = 0;  ///< Number of valid entries so far (0..kNN).

  /**
   * @brief Current pruning bound: the k-th smallest squared distance, or +inf until k are found.
   */
  [[nodiscard]] T
  worst2() const noexcept
  {
    return (count < kNN) ? std::numeric_limits<T>::max() : dist2[kNN - 1];
  }

  /**
   * @brief Offer one candidate neighbor; keep it iff it is among the kNN closest and not already held.
   * @details De-duplicates by cloud index -- so a PointAoSoA group's padded lanes (which repeat the
   * last real index) are no-ops, and a neighbor found both directly and via the reciprocal update is
   * listed once. Keeps the arrays sorted by ascending distance.
   * @param[in] a_dist2 Squared distance to the candidate.
   * @param[in] a_index Candidate's cloud index.
   * @return True iff the candidate was newly accepted into the kNN best (worth reciprocating).
   */
  bool
  tryInsert(T a_dist2, size_t a_index) noexcept
  {
    for (size_t i = 0; i < count; i++) {
      if (index[i] == a_index) {
        return false; // already held -- no new information
      }
    }
    if (count == kNN && a_dist2 >= dist2[kNN - 1]) {
      return false; // not among the kNN closest
    }

    size_t pos = (count < kNN) ? count++ : (kNN - 1); // grow, or overwrite the current worst
    dist2[pos] = a_dist2;
    index[pos] = a_index;

    while (pos > 0 && dist2[pos] < dist2[pos - 1]) { // bubble toward the front to stay ascending
      std::swap(dist2[pos], dist2[pos - 1]);
      std::swap(index[pos], index[pos - 1]);
      pos--;
    }

    return true;
  }
};

/**
 * @brief Timing and traversal statistics for one build strategy's all-k-NN pass.
 */
struct StrategyResult
{
  double buildSeconds     = 0.0; ///< Construction time (filled in by the caller, which times it).
  double knnSeconds       = 0.0; ///< Total time for the whole-cloud all-k-NN pass.
  double avgLeafVisits    = 0.0; ///< Average leaf nodes visited per point.
  double avgGroupsPerLeaf = 0.0; ///< Average groups per visited leaf (the leaf occupancy achieved).
};

/**
 * @brief Group the cloud into spatially-coherent PointGroup leaves for the BVH.
 * @details Morton-sorts the points, then chunks the sorted order into consecutive runs of W. Compact
 * groups have tight bounding volumes and prune well. Each point's original index rides along as its
 * lane metadata, surviving the reshuffle.
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
 * @brief The kNN nearest neighbors of one point by brute force -- the ground truth for verification.
 * @param[in] a_positions The point cloud.
 * @param[in] a_query     Cloud index of the point whose neighbors are wanted (excluded from its own).
 * @return The kNN smallest squared distances to other points, ascending.
 */
std::array<T, kNN>
bruteForceKnn(const std::vector<Vec3>& a_positions, size_t a_query)
{
  Knn best;
  for (size_t i = 0; i < a_positions.size(); i++) {
    if (i == a_query) {
      continue;
    }
    best.tryInsert((a_positions[i] - a_positions[a_query]).length2(), i);
  }

  return best.dist2; // kNN filled since numPoints > kNN, and already sorted ascending
}

/**
 * @brief Run the all-k-NN pass over the whole cloud via pruneTraverse(), with reciprocal culling.
 * @details Processes the points in @p a_order (a spatial ordering) so that a point's near neighbors
 * are visited close together in time -- which is what makes reciprocal culling pay off. For each
 * processed point P, the traversal state is P's own Knn; at every leaf, each candidate C (self and
 * padded-lane duplicates skipped by the Knn de-dupe) is offered to P's Knn, and -- when
 * @p a_reciprocal is set -- P is offered right back to C's Knn, tightening C's bound before C is
 * processed. All distance work stays squared: PointAoSoA::getDistances2() computes all W lane
 * distances to the query in one SIMD kernel, and getMetaData() names each lane's cloud index
 * (getMinimumDistance2() would only give the group's single closest, no use for k-NN).
 * @param[in]     a_bvh        BVH over the PointGroup leaves.
 * @param[in]     a_positions  The point cloud.
 * @param[in]     a_order      Processing order (cloud indices); spatial order maximises culling.
 * @param[in]     a_reciprocal Whether to push each discovered distance to the neighbor's Knn too.
 * @param[in,out] a_leafVisits Accumulates leaf nodes visited (a traversal statistic).
 * @param[in,out] a_groupEvals Accumulates groups scanned (a traversal statistic).
 * @return One Knn per cloud point (its kNN nearest neighbors, sorted by ascending squared distance).
 */
std::vector<Knn>
knnPass(const Packed&                a_bvh,
        const std::vector<Vec3>&     a_positions,
        const std::vector<uint32_t>& a_order,
        bool                         a_reciprocal,
        long long&                   a_leafVisits,
        long long&                   a_groupEvals)
{
  std::vector<Knn> knn(a_positions.size());

  const auto& groups = a_bvh.getPrimitives();

  for (const uint32_t p : a_order) {
    const auto evalLeaf = [&groups, &knn, &a_positions, &a_leafVisits, &a_groupEvals, p, a_reciprocal](
                            Knn& a_state, size_t a_offset, size_t a_count) noexcept {
      a_leafVisits++;
      a_groupEvals += static_cast<long long>(a_count);
      for (size_t g = 0; g < a_count; g++) {
        const PointGroup& group = groups[a_offset + g];

        // One SIMD kernel gives all W lane distances at once; getMetaData(lane) names each lane's
        // cloud index (getMinimumDistance2() would only give the group's single closest).
        const std::array<T, W> distances = group.getDistances2(a_positions[p]);

        for (size_t lane = 0; lane < W; lane++) {
          const size_t c = group.getMetaData(lane);
          if (c == p) {
            continue; // skip self (P finds itself at distance 0); padded lanes de-dupe by index in tryInsert
          }
          const T d2 = distances[lane];
          // Reciprocate only when C is actually accepted as one of P's neighbors: distances are
          // symmetric, so P is then a strong candidate for C too, and offering it now tightens C's
          // bound before C is processed. Reciprocating every far candidate instead (they get
          // rejected anyway) is pure overhead -- see README.
          if (a_state.tryInsert(d2, c) && a_reciprocal) { // a_state is knn[p]
            knn[c].tryInsert(d2, p);
          }
        }
      }
    };
    const auto pruneDist2 = [](const Knn& a_state) noexcept -> T { return a_state.worst2(); };

    a_bvh.pruneTraverse(a_positions[p], knn[p], evalLeaf, pruneDist2);
  }

  return knn;
}

/**
 * @brief Run the all-k-NN pass on a_bvh, timing it, gathering statistics, and verifying a sample.
 * @details Correctness is checked with EBGEOMETRY_EXPECT (opt-in via EBGEOMETRY_ENABLE_ASSERTIONS):
 * for every sampled point, the BVH's three squared neighbor distances must match brute force's
 * (compared as sorted distances, so ties are handled -- the *set* of nearest neighbors is unique
 * even when the choice of indices at the k-th distance is not). The leaf-visit count depends on the
 * Knn contents through the pruning bound, so the traversal is never optimized away even when the
 * assertions compile out.
 * @param[in] a_bvh        BVH to query.
 * @param[in] a_positions  The point cloud.
 * @param[in] a_order      Processing order (cloud indices).
 * @param[in] a_reciprocal Whether reciprocal culling is enabled.
 * @param[in] a_verifyIdx  Cloud indices of the verification sample.
 * @param[in] a_bruteKnn   Brute-force ground truth for a_verifyIdx (parallel arrays).
 * @return Timing and leaf-visit statistics; build time is filled in by the caller.
 */
StrategyResult
benchmarkStrategy(const Packed&                          a_bvh,
                  const std::vector<Vec3>&               a_positions,
                  const std::vector<uint32_t>&           a_order,
                  bool                                   a_reciprocal,
                  const std::vector<size_t>&             a_verifyIdx,
                  const std::vector<std::array<T, kNN>>& a_bruteKnn)
{
  constexpr T tolerance = std::is_same_v<T, float> ? T(1.0e-4) : T(1.0e-9);

  long long leafVisits = 0;
  long long groupEvals = 0;

  EBGeometry::SimpleTimer timer;
  timer.start();
  const std::vector<Knn> knn = knnPass(a_bvh, a_positions, a_order, a_reciprocal, leafVisits, groupEvals);
  timer.stop();

  for (size_t s = 0; s < a_verifyIdx.size(); s++) {
    const Knn& result = knn[a_verifyIdx[s]];
    EBGEOMETRY_EXPECT(result.count == kNN);
    for (size_t j = 0; j < kNN; j++) {
      EBGEOMETRY_EXPECT(std::abs(result.dist2[j] - a_bruteKnn[s][j]) <= tolerance * std::max(a_bruteKnn[s][j], T(1.0)));
    }
  }

  StrategyResult result;
  result.knnSeconds       = timer.seconds();
  result.avgLeafVisits    = double(leafVisits) / double(numPoints);
  result.avgGroupsPerLeaf = (leafVisits > 0) ? double(groupEvals) / double(leafVisits) : 0.0;

  return result;
}

/**
 * @brief Print one results-table row for a build strategy.
 * @details Assumes std::cout is already in std::fixed mode (set by the caller before the table).
 * @param[in] a_label            Strategy name for the leftmost column.
 * @param[in] a_result           That strategy's timing and traversal statistics.
 * @param[in] a_brutePerPointSec Estimated brute-force time per point, for the speedup column.
 */
void
printRow(const char* a_label, const StrategyResult& a_result, double a_brutePerPointSec)
{
  const double knnUs   = 1.0e6 * a_result.knnSeconds / double(numPoints);
  const double speedup = a_brutePerPointSec / (a_result.knnSeconds / double(numPoints));

  std::cout << std::left << std::setw(24) << a_label << std::right << std::setw(13) << std::setprecision(2)
            << 1.0e3 * a_result.buildSeconds << std::setw(15) << std::setprecision(3) << knnUs << std::setw(10)
            << std::setprecision(1) << speedup << "x" << std::setw(14) << std::setprecision(1) << a_result.avgLeafVisits
            << std::setw(14) << std::setprecision(1) << a_result.avgGroupsPerLeaf << '\n';
}

} // namespace

int
main()
{
  std::cout << "NearestNeighborSFCPacked: " << kNN << " nearest neighbors of every point in a " << numPoints
            << "-point cloud\n";
  std::cout << "  Precision T             = " << (std::is_same_v<T, float> ? "float" : "double") << '\n';
  std::cout << "  SoA width W             = " << W << " (PointSoA::DefaultWidth<T>())\n";
  std::cout << "  BVH branching factor K  = " << K << " (BVH::DefaultBranchingRatio<T>())\n";
  std::cout << "  Points                  = " << numPoints << '\n';
  std::cout << "  Neighbors per point kNN = " << kNN << '\n';
  std::cout << "  Target leaf size        = " << maxLeafGroups << " groups (" << maxLeafGroups * W << " points)\n\n";

  const std::vector<Vec3> positions = EBGeometry::Random::samplePoints<T>(numPoints, pointSeed);

  // Process points in Hilbert order so a point's neighbors are visited close together -- the spatial
  // locality that makes reciprocal culling pay off. Ordering affects only efficiency, never results.
  const std::vector<uint32_t> order = EBGeometry::SFC::order<EBGeometry::SFC::Hilbert>(positions);

  // Shared across every build strategy: SFC-sort the points and pack them into spatially-coherent
  // PointGroup leaves. Only the outer tree over these groups varies between strategies.
  const std::vector<std::pair<PointGroup, AABB>> groups = buildGroups(positions);

  const EBGeometry::BVH::LeafPredicate<T, PointGroup, AABB, K> stopCrit = [](const Tree& a_node) noexcept -> bool {
    return a_node.getPrimitives().size() <= maxLeafGroups;
  };

  EBGeometry::SimpleTimer timer;

  timer.start();
  const Packed mortonBVH(groups, maxLeafGroups, EBGeometry::SFC::Morton{});
  timer.stop();
  const double mortonBuild = timer.seconds();

  timer.start();
  const Packed hilbertBVH(groups, maxLeafGroups, EBGeometry::SFC::Hilbert{});
  timer.stop();
  const double hilbertBuild = timer.seconds();

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

  // ClusterSAH: density-adaptive clustering of the groups, then SAH over the clusters -- near-SAH tree
  // quality at a fraction of SAH's build cost. Direct-only, disambiguated by the ClusterSpec argument.
  timer.start();
  const Packed clusterSahBVH(groups, EBGeometry::BVH::ClusterSpec{maxClusterGroups});
  timer.stop();
  const double clusterSahBuild = timer.seconds();

  // Brute-force ground truth for a spread-out sample of points, timed so its per-point cost is the
  // baseline for the speedup column (full all-pairs brute force would be O(N^2)).
  std::vector<size_t>             verifyIdx(numVerify);
  std::vector<std::array<T, kNN>> bruteKnn(numVerify);
  const size_t                    stride = numPoints / numVerify;
  for (size_t s = 0; s < numVerify; s++) {
    verifyIdx[s] = s * stride;
  }
  timer.start();
  for (size_t s = 0; s < numVerify; s++) {
    bruteKnn[s] = bruteForceKnn(positions, verifyIdx[s]);
  }
  timer.stop();
  const double brutePerPointSec = timer.seconds() / double(numVerify);

  // All-k-NN with reciprocal culling on, for each strategy.
  StrategyResult mortonRes     = benchmarkStrategy(mortonBVH, positions, order, true, verifyIdx, bruteKnn);
  StrategyResult hilbertRes    = benchmarkStrategy(hilbertBVH, positions, order, true, verifyIdx, bruteKnn);
  StrategyResult topDownRes    = benchmarkStrategy(topDownBVH, positions, order, true, verifyIdx, bruteKnn);
  StrategyResult midpointRes   = benchmarkStrategy(midpointBVH, positions, order, true, verifyIdx, bruteKnn);
  StrategyResult sahRes        = benchmarkStrategy(sahBVH, positions, order, true, verifyIdx, bruteKnn);
  StrategyResult clusterSahRes = benchmarkStrategy(clusterSahBVH, positions, order, true, verifyIdx, bruteKnn);

  mortonRes.buildSeconds     = mortonBuild;
  hilbertRes.buildSeconds    = hilbertBuild;
  topDownRes.buildSeconds    = topDownBuild;
  midpointRes.buildSeconds   = midpointBuild;
  sahRes.buildSeconds        = sahBuild;
  clusterSahRes.buildSeconds = clusterSahBuild;

  // The same BVHs, but with reciprocal culling OFF -- the contrast that shows the culling win.
  // Culling reduces leaf visits in proportion to how loose the baseline traversal is: the SFC-direct
  // builds (Morton/Hilbert) form looser trees over the pre-made groups and visit many leaves whose
  // points aren't actually near the query, so pre-seeding a point's true neighbors prunes them
  // sharply; the tighter SAH build has less to gain. See README.
  StrategyResult mortonNoCull  = benchmarkStrategy(mortonBVH, positions, order, false, verifyIdx, bruteKnn);
  StrategyResult hilbertNoCull = benchmarkStrategy(hilbertBVH, positions, order, false, verifyIdx, bruteKnn);
  StrategyResult sahNoCull     = benchmarkStrategy(sahBVH, positions, order, false, verifyIdx, bruteKnn);
  mortonNoCull.buildSeconds    = mortonBuild;
  hilbertNoCull.buildSeconds   = hilbertBuild;
  sahNoCull.buildSeconds       = sahBuild;

  std::cout << std::left << std::setw(24) << "Strategy" << std::right << std::setw(13) << "Build (ms)" << std::setw(15)
            << "kNN (us/pt)" << std::setw(11) << "Speedup" << std::setw(14) << "Leaf visits" << std::setw(14)
            << "Groups/leaf" << '\n';
  std::cout << std::string(91, '-') << '\n';

  std::cout << std::fixed;
  std::cout << std::left << std::setw(24) << "Brute force" << std::right << std::setw(13) << std::setprecision(2)
            << "--" << std::setw(15) << std::setprecision(3) << 1.0e6 * brutePerPointSec << std::setw(11)
            << std::setprecision(1) << "1.0x" << std::setw(14) << "--" << std::setw(14) << "--" << '\n';

  printRow("Morton (SFC)", mortonRes, brutePerPointSec);
  printRow("Hilbert (SFC)", hilbertRes, brutePerPointSec);
  printRow("TopDown centroid", topDownRes, brutePerPointSec);
  printRow("Midpoint", midpointRes, brutePerPointSec);
  printRow("SAH", sahRes, brutePerPointSec);
  printRow("ClusterSAH", clusterSahRes, brutePerPointSec);
  std::cout << std::string(91, '-') << '\n';
  std::cout << "Reciprocal culling OFF (compare against the rows above):\n";
  printRow("Morton (no cull)", mortonNoCull, brutePerPointSec);
  printRow("Hilbert (no cull)", hilbertNoCull, brutePerPointSec);
  printRow("SAH (no cull)", sahNoCull, brutePerPointSec);

  return 0;
}
