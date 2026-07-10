// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// All-k-nearest-neighbors (kNN = 1) over a random point cloud, using PointAoSoA<T, Meta, W> groups as
// the leaves of a PackedBVH. Unlike Examples/ClosestPointTreePacked -- which resolves a batch of
// independent closest-point queries (one nearest point each) -- this example finds, for *every*
// point in the cloud, its kNN nearest neighbors (the classic k-NN graph). It mirrors
// ClosestPointTreePacked's organization: a TreeBVH is built over the individual points and the
// PointAoSoA groups are materialised from its leaves via TreeBVH::packWith(), five build strategies
// deep. The novel piece is *reciprocal distance culling*: when point A's traversal discovers
// neighbor B at distance d, A is recorded as a candidate neighbor of B too (distance is symmetric),
// tightening B's search radius before B is ever processed, so B's later traversal prunes harder.
// See README.md.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
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

// SoA width W (points per group, sizing the leaf SIMD distance kernel) and branching factor K
// (children per node, sizing the box test), both fixed at 4 -- a good sweet spot balancing build
// and query time. Each trailing comment is the ISA-adaptive default it replaces, which instead
// picks the SIMD-optimal value for the compiled precision.
constexpr size_t W = 4; // = EBGeometry::PointSoA::DefaultWidth<T>()
constexpr size_t K = 4; // = EBGeometry::BVH::DefaultBranchingRatio<T>()

namespace {

/**
 * @brief The BVH primitive during construction: a single cloud point plus its cloud index.
 * @details The TreeBVH is built over these individual points -- one primitive per point -- so the
 * partitioner sees the full cloud. packWith() then coalesces each leaf's points into PointAoSoA
 * groups, carrying @c index along as each point's lane metadata.
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

// Run configuration. kNN is the number of nearest neighbors per point (hard-coded to 1). numVerify
// points are checked against a brute-force scan (full all-pairs would be O(N^2)). maxLeafGroups is
// the target groups per leaf for the top-down partitioners.
constexpr size_t   numPoints     = 500000;
constexpr size_t   kNN           = 1;
constexpr size_t   numVerify     = 500;
constexpr uint64_t pointSeed     = 123456789ULL;
constexpr size_t   maxLeafGroups = 16;

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
   * @details Tests the cheap distance bound before the de-dupe scan: most candidates in a leaf are
   * farther than the current worst, so rejecting them by distance first lets them skip the index
   * scan entirely. The de-dupe -- which stops a PointAoSoA group's padded lanes (repeating the last
   * real index) and a neighbor found both directly and via the reciprocal update from being listed
   * twice -- is only reached by a candidate that would actually be inserted. For kNN == 1 it is
   * redundant (a repeat is either the current best, already rejected by a_dist2 >= dist2[0], or
   * farther, same) and is compiled out. Keeps the arrays sorted by ascending distance.
   * @param[in] a_dist2 Squared distance to the candidate.
   * @param[in] a_index Candidate's cloud index.
   * @return True iff the candidate was newly accepted into the kNN best (worth reciprocating).
   */
  bool
  tryInsert(T a_dist2, size_t a_index) noexcept
  {
    if (count == kNN && a_dist2 >= dist2[kNN - 1]) {
      return false; // not among the kNN closest -- cheap reject first, before the de-dupe scan
    }

    if constexpr (kNN > 1) {
      for (size_t i = 0; i < count; i++) {
        if (index[i] == a_index) {
          return false; // already held -- no new information
        }
      }
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
 * @brief Leaf-conversion callback for TreeBVH::packWith(): coalesce one leaf's points into groups.
 * @details Called once per TreeBVH leaf. Chunks the leaf's contiguous run of points into consecutive
 * PointAoSoA groups of up to W each; each point's cloud index rides along as its lane metadata. This
 * mirrors TriMeshSDF::groupTrianglesIntoSoA.
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
 * @brief Build one PackedBVH via the packWith path, timing the whole construction.
 * @details Constructs a fresh TreeBVH over the shared per-point primitive list, partitions it with
 * a_partition (the only thing that varies between strategies), then packWith()s it into a by-value
 * PackedBVH. The shared primitive list is built once by the caller and excluded from the timing.
 * @param[in] a_primsAndBVs Shared per-point (primitive, bounding volume) list.
 * @param[in] a_partition   Partitions the tree (e.g. top-down SAH, or bottom-up Hilbert).
 * @return The packed BVH and the construction time in seconds.
 */
std::pair<std::shared_ptr<Packed>, double>
buildPacked(const EBGeometry::BVH::PrimAndBVList<Point, AABB>& a_primsAndBVs,
            const std::function<void(PointTree&)>&             a_partition)
{
  using Converter = decltype(&groupPointsIntoSoA);

  EBGeometry::SimpleTimer timer;
  timer.start();

  auto tree = std::make_shared<PointTree>(a_primsAndBVs);
  a_partition(*tree);
  std::shared_ptr<Packed> packed =
    tree->packWith<PointGroup, Converter, EBGeometry::BVH::ValueStorage<PointGroup>>(&groupPointsIntoSoA);

  timer.stop();

  return {packed, timer.seconds()};
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
  std::cout << "NearestNeighborTreePacked: " << kNN << " nearest neighbors of every point in a " << numPoints
            << "-point cloud\n";
  std::cout << "  Precision T             = " << (std::is_same_v<T, float> ? "float" : "double") << '\n';
  std::cout << "  SoA width W             = " << W << " (fixed; DefaultWidth<T>() picks the ISA width)\n";
  std::cout << "  BVH branching factor K  = " << K << " (fixed; DefaultBranchingRatio<T>() picks the ISA value)\n";
  std::cout << "  Points                  = " << numPoints << '\n';
  std::cout << "  Neighbors per point kNN = " << kNN << '\n';
  std::cout << "  Target leaf size        = " << maxLeafGroups << " groups (" << maxLeafGroups * W << " points)\n\n";

  const std::vector<Vec3> positions = EBGeometry::Random::samplePoints<T>(numPoints, pointSeed);

  // Process points in Hilbert order so a point's neighbors are visited close together -- the spatial
  // locality that makes reciprocal culling pay off. Ordering affects only efficiency, never results.
  const std::vector<uint32_t> order = EBGeometry::SFC::order<EBGeometry::SFC::Hilbert>(positions);

  // The per-point primitive list: one primitive per cloud point, its bounding volume the degenerate
  // box at the point. Built once and reused across strategies (each TreeBVH constructor copies it
  // in), so this shared setup is excluded from the per-strategy build timing below.
  EBGeometry::BVH::PrimAndBVList<Point, AABB> primsAndBVs;
  primsAndBVs.reserve(numPoints);
  for (size_t i = 0; i < positions.size(); i++) {
    primsAndBVs.emplace_back(std::make_shared<Point>(Point{positions[i], i}), AABB(positions[i], positions[i]));
  }

  // The top-down partitioners stop splitting once a node holds <= maxLeafGroups*W points. (The
  // bottom-up SFC builds ignore this and produce fixed K-primitive leaves instead.)
  const size_t                                            maxLeafSize = maxLeafGroups * W;
  const EBGeometry::BVH::LeafPredicate<T, Point, AABB, K> stopCrit =
    // maxLeafSize is a constant expression (maxLeafGroups and W are constexpr), so a named capture
    // ([maxLeafSize]) is "unused" to clang (-Wunused-lambda-capture) yet required by g++; the [=]
    // capture-default satisfies both.
    [=](const PointTree& a_node) noexcept -> bool { return a_node.getPrimitives().size() <= maxLeafSize; };

  // Build the same points into a PackedBVH via the packWith path five ways -- matching
  // NearestNeighborSFCPacked's strategies -- timing each. Only the partition step differs.
  const auto morton =
    buildPacked(primsAndBVs, [](PointTree& a_tree) { a_tree.bottomUpSortAndPartition<EBGeometry::SFC::Morton>(); });
  const auto hilbert =
    buildPacked(primsAndBVs, [](PointTree& a_tree) { a_tree.bottomUpSortAndPartition<EBGeometry::SFC::Hilbert>(); });
  const auto topDown  = buildPacked(primsAndBVs, [&stopCrit](PointTree& a_tree) {
    a_tree.topDownSortAndPartition(EBGeometry::BVH::BVCentroidPartitioner<T, Point, AABB, K>, stopCrit);
  });
  const auto midpoint = buildPacked(primsAndBVs, [&stopCrit](PointTree& a_tree) {
    a_tree.topDownSortAndPartition(EBGeometry::BVH::MidpointPartitioner<T, Point, AABB, K>, stopCrit);
  });
  const auto sah      = buildPacked(primsAndBVs, [&stopCrit](PointTree& a_tree) {
    a_tree.topDownSortAndPartition(EBGeometry::BVH::BinnedSAHPartitioner<T, Point, AABB, K>, stopCrit);
  });

  // Brute-force ground truth for a spread-out sample of points, timed so its per-point cost is the
  // baseline for the speedup column (full all-pairs brute force would be O(N^2)).
  std::vector<size_t>             verifyIdx(numVerify);
  std::vector<std::array<T, kNN>> bruteKnn(numVerify);
  const size_t                    stride = numPoints / numVerify;
  for (size_t s = 0; s < numVerify; s++) {
    verifyIdx[s] = s * stride;
  }
  EBGeometry::SimpleTimer timer;
  timer.start();
  for (size_t s = 0; s < numVerify; s++) {
    bruteKnn[s] = bruteForceKnn(positions, verifyIdx[s]);
  }
  timer.stop();
  const double brutePerPointSec = timer.seconds() / double(numVerify);

  // All-k-NN with reciprocal culling on, for each strategy.
  StrategyResult mortonRes   = benchmarkStrategy(*morton.first, positions, order, true, verifyIdx, bruteKnn);
  StrategyResult hilbertRes  = benchmarkStrategy(*hilbert.first, positions, order, true, verifyIdx, bruteKnn);
  StrategyResult topDownRes  = benchmarkStrategy(*topDown.first, positions, order, true, verifyIdx, bruteKnn);
  StrategyResult midpointRes = benchmarkStrategy(*midpoint.first, positions, order, true, verifyIdx, bruteKnn);
  StrategyResult sahRes      = benchmarkStrategy(*sah.first, positions, order, true, verifyIdx, bruteKnn);

  mortonRes.buildSeconds   = morton.second;
  hilbertRes.buildSeconds  = hilbert.second;
  topDownRes.buildSeconds  = topDown.second;
  midpointRes.buildSeconds = midpoint.second;
  sahRes.buildSeconds      = sah.second;

  // The same BVHs, but with reciprocal culling OFF -- the contrast that shows where culling helps.
  // Culling reduces leaf visits in proportion to how loose the baseline traversal is. Morton/Hilbert
  // are bottom-up over individual points (small K-primitive leaves), so the search stays loose and
  // culling cuts leaf visits sharply. SAH over individual points builds a very tight tree that
  // already visits close to the k-leaf minimum, so there is almost nothing left to cull -- culling
  // on/off are nearly indistinguishable. (Contrast NearestNeighborSFCPacked, whose looser
  // trees-over-groups let culling help every strategy, SAH included.)
  StrategyResult mortonNoCull  = benchmarkStrategy(*morton.first, positions, order, false, verifyIdx, bruteKnn);
  StrategyResult hilbertNoCull = benchmarkStrategy(*hilbert.first, positions, order, false, verifyIdx, bruteKnn);
  StrategyResult sahNoCull     = benchmarkStrategy(*sah.first, positions, order, false, verifyIdx, bruteKnn);
  mortonNoCull.buildSeconds    = morton.second;
  hilbertNoCull.buildSeconds   = hilbert.second;
  sahNoCull.buildSeconds       = sah.second;

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
  std::cout << std::string(91, '-') << '\n';
  std::cout << "Reciprocal culling OFF (compare against the rows above):\n";
  printRow("Morton (no cull)", mortonNoCull, brutePerPointSec);
  printRow("Hilbert (no cull)", hilbertNoCull, brutePerPointSec);
  printRow("SAH (no cull)", sahNoCull, brutePerPointSec);

  return 0;
}
