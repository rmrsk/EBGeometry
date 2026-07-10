// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Head-to-head: EBGeometry PackedBVH vs nanoflann kd-tree on the same all-nearest-neighbor problem
// (for every point in a random cloud, find its single nearest *other* point). Both are built and
// queried single-threaded from the identical point cloud, so the numbers are directly comparable --
// which mirrors the intended deployment, where each core owns its own cloud and its own tree (outer
// parallelism over independent trees, no inner threading). nanoflann is a purpose-built kd-tree for
// nearest-neighbor, so it is the fair apples-to-apples reference for the query side.
//
// This is a throwaway comparison harness living under ThirdParty/ (not CI-tested); nanoflann.hpp is
// fetched by the GNUmakefile, not vendored. See README.md.

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
#include <vector>

#include <nanoflann.hpp>

#include <EBGeometry.hpp>

#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif
using T = EBGEOMETRY_PRECISION;

// SoA width and BVH branching factor, fixed at 4 (matches the Examples/NearestNeighbor* sweet spot).
constexpr size_t W = 4;
constexpr size_t K = 4;

using Vec3       = EBGeometry::Vec3T<T>;
using AABB       = EBGeometry::BoundingVolumes::AABBT<T>;
using PointGroup = EBGeometry::PointAoSoA<T, size_t, W>;
using Packed     = EBGeometry::BVH::PackedBVH<T, PointGroup, K, EBGeometry::BVH::ValueStorage<PointGroup>>;
using Tree       = EBGeometry::BVH::TreeBVH<T, PointGroup, AABB, K>;

// One cloud point as a BVH primitive, for the "SAH over individual points" build path.
struct Point
{
  Vec3   position;
  size_t index;
};
using PointTree = EBGeometry::BVH::TreeBVH<T, Point, AABB, K>;

constexpr size_t   numPoints        = 500000;
constexpr uint64_t pointSeed        = 123456789ULL;
constexpr size_t   maxClusterGroups = 8;  // EBGeometry ClusterSAH bucket size (groups)
constexpr size_t   maxLeafGroups    = 16; // EBGeometry SAH top-down target leaf size (groups)
constexpr size_t   nanoflannLeaf    = 16; // nanoflann leaf_max_size (points)

namespace {

// Pack the cloud into Morton-sorted PointAoSoA groups (the EBGeometry leaf primitive), the same way
// Examples/NearestNeighborSFCPacked does.
std::vector<std::pair<PointGroup, AABB>>
buildGroups(const std::vector<Vec3>& a_positions)
{
  const std::vector<uint32_t> order     = EBGeometry::SFC::order<EBGeometry::SFC::Morton>(a_positions);
  const size_t                numGroups = (a_positions.size() + W - 1U) / W;

  std::vector<std::pair<PointGroup, AABB>> groups;
  groups.reserve(numGroups);
  for (size_t g = 0; g < numGroups; g++) {
    const size_t offset = g * W;
    const size_t count  = std::min(W, a_positions.size() - offset);

    std::array<Vec3, W>   posArr;
    std::array<size_t, W> metaArr;
    for (size_t i = 0; i < count; i++) {
      const uint32_t src = order[offset + i];
      posArr[i]          = a_positions[src];
      metaArr[i]         = src;
    }
    PointGroup group;
    group.pack(posArr.data(), metaArr.data(), static_cast<uint32_t>(count));
    groups.emplace_back(group, group.template computeBoundingVolume<AABB>());
  }
  return groups;
}

// Run the all-nearest-neighbor query over a built PackedBVH: for every point, its single nearest
// *other* point via pruneTraverse(), processing points in the given order. Processing order does not
// change the result, only the query time (via cache locality). Returns the nearest index per point
// and the total query time.
std::pair<std::vector<uint32_t>, double>
runEbQuery(const Packed& a_bvh, const std::vector<Vec3>& a_positions, const std::vector<uint32_t>& a_order)
{
  std::vector<uint32_t> nearest(a_positions.size());
  const auto&           groups = a_bvh.getPrimitives();

  EBGeometry::SimpleTimer timer;
  timer.start();
  for (const uint32_t p : a_order) {
    const Vec3 q        = a_positions[p];
    const auto evalLeaf = [&groups, &q, p](std::pair<T, uint32_t>& st, size_t off, size_t cnt) noexcept {
      for (size_t g = 0; g < cnt; g++) {
        const PointGroup&      grp = groups[off + g];
        const std::array<T, W> d2  = grp.getDistances2(q);
        for (size_t lane = 0; lane < W; lane++) {
          const size_t c = grp.getMetaData(lane);
          if (c == p) {
            continue; // skip self
          }
          if (d2[lane] < st.first) {
            st.first  = d2[lane];
            st.second = static_cast<uint32_t>(c);
          }
        }
      }
    };
    const auto             pruneDist2 = [](const std::pair<T, uint32_t>& st) noexcept -> T { return st.first; };
    std::pair<T, uint32_t> st{std::numeric_limits<T>::max(), 0};
    a_bvh.pruneTraverse(q, st, evalLeaf, pruneDist2);
    nearest[p] = st.second;
  }
  timer.stop();
  return {nearest, timer.seconds()};
}

// TreeBVH::packWith() leaf converter: coalesce a leaf's contiguous run of points into PointAoSoA
// groups (mirrors Examples/NearestNeighborTreePacked).
std::vector<PointGroup>
groupPointsIntoSoA(const std::vector<std::shared_ptr<const Point>>& a_points, uint32_t a_offset, uint32_t a_count)
{
  constexpr uint32_t soaWidth  = static_cast<uint32_t>(W);
  const uint32_t     numGroups = (a_count + soaWidth - 1U) / soaWidth;

  std::vector<PointGroup> groups;
  groups.reserve(numGroups);
  for (uint32_t g = 0; g < numGroups; g++) {
    const uint32_t groupOffset = a_offset + g * soaWidth;
    const uint32_t groupCount  = std::min(soaWidth, a_count - g * soaWidth);

    std::array<Vec3, W>   posArr;
    std::array<size_t, W> metaArr;
    for (uint32_t i = 0; i < groupCount; i++) {
      posArr[i]  = a_points[groupOffset + i]->position;
      metaArr[i] = a_points[groupOffset + i]->index;
    }
    PointGroup group;
    group.pack(posArr.data(), metaArr.data(), groupCount);
    groups.push_back(group);
  }
  return groups;
}

// nanoflann dataset adaptor over the raw point cloud.
struct Cloud
{
  const std::vector<Vec3>& pts;
  inline size_t
  kdtree_get_point_count() const
  {
    return pts.size();
  }
  inline T
  kdtree_get_pt(size_t i, size_t dim) const
  {
    return pts[i][dim];
  }
  template <class BBOX>
  bool
  kdtree_get_bbox(BBOX&) const
  {
    return false;
  }
};
using KDTree = nanoflann::KDTreeSingleIndexAdaptor<nanoflann::L2_Simple_Adaptor<T, Cloud>, Cloud, 3, uint32_t>;

// Run the all-nearest-neighbor query over the nanoflann kd-tree in the given processing order,
// mirroring runEbQuery() so the two are timed identically.
std::pair<std::vector<uint32_t>, double>
runNfQuery(const KDTree& a_tree, const std::vector<Vec3>& a_positions, const std::vector<uint32_t>& a_order)
{
  std::vector<uint32_t> nearest(a_positions.size());

  EBGeometry::SimpleTimer timer;
  timer.start();
  for (const uint32_t p : a_order) {
    const T                 q[3] = {a_positions[p][0], a_positions[p][1], a_positions[p][2]};
    std::array<uint32_t, 2> idx{};
    std::array<T, 2>        dist{};
    (void)a_tree.knnSearch(q, 2, idx.data(), dist.data()); // idx[0] is self (dist 0); idx[1] nearest other
    nearest[p] = idx[1];
  }
  timer.stop();
  return {nearest, timer.seconds()};
}

} // namespace

int
main()
{
  std::cout << "EBGeometry PackedBVH vs nanoflann kd-tree: nearest neighbor of every point in a " << numPoints
            << "-point cloud\n";
  std::cout << "  Precision T = " << (std::is_same_v<T, float> ? "float" : "double") << "  (single-threaded, "
            << "same cloud, kNN=1 excluding self)\n\n";

  const std::vector<Vec3> positions = EBGeometry::Random::samplePoints<T>(numPoints, pointSeed);

  EBGeometry::SimpleTimer timer;

  // ---- EBGeometry: SFC pack (shared), then two build strategies over the same groups ----
  timer.start();
  const auto groups = buildGroups(positions);
  timer.stop();
  const double ebPack = timer.seconds();

  // ClusterSAH: density-adaptive clustering then SAH over the clusters (fast build, balanced).
  timer.start();
  const Packed clusterBvh(groups, EBGeometry::BVH::ClusterSpec{maxClusterGroups});
  timer.stop();
  const double ebClusterBuild = timer.seconds();

  // SAH top-down: full binned-SAH partitioning of the groups (slower build, tighter tree).
  const EBGeometry::BVH::LeafPredicate<T, PointGroup, AABB, K> stopCrit = [](const Tree& a_node) noexcept -> bool {
    return a_node.getPrimitives().size() <= maxLeafGroups;
  };
  timer.start();
  const Packed sahBvh(groups, EBGeometry::BVH::BinnedSAHPartitioner<T, PointGroup, AABB, K>, stopCrit);
  timer.stop();
  const double ebSahBuild = timer.seconds();

  // SAH over individual points: build a TreeBVH over all points, then packWith() into groups. The
  // tree partitions at point granularity (not pre-made groups), giving a much tighter tree -- at a
  // much higher build cost. This build is standalone (it does not reuse the SFC pack above).
  timer.start();
  EBGeometry::BVH::PrimAndBVList<Point, AABB> primsAndBVs;
  primsAndBVs.reserve(numPoints);
  for (size_t i = 0; i < numPoints; i++) {
    primsAndBVs.emplace_back(std::make_shared<Point>(Point{positions[i], i}), AABB(positions[i], positions[i]));
  }
  const EBGeometry::BVH::LeafPredicate<T, Point, AABB, K> stopCritPts = [](const PointTree& a_node) noexcept -> bool {
    return a_node.getPrimitives().size() <= maxLeafGroups * W;
  };
  PointTree pointTree(primsAndBVs);
  pointTree.topDownSortAndPartition(EBGeometry::BVH::BinnedSAHPartitioner<T, Point, AABB, K>, stopCritPts);
  using Converter = decltype(&groupPointsIntoSoA);
  std::shared_ptr<Packed> sahPointsBvh =
    pointTree.packWith<PointGroup, Converter, EBGeometry::BVH::ValueStorage<PointGroup>>(&groupPointsIntoSoA);
  timer.stop();
  const double ebSahPointsBuild = timer.seconds();

  // Processing orders: natural (index 0..N-1) and Hilbert-sorted. Same results either way; only the
  // cache locality -- hence query time -- differs. Building the Hilbert order is a one-time O(N log N),
  // and in a real particle code the particles are often already stored in a spatially coherent order.
  std::vector<uint32_t> naturalOrder(numPoints);
  for (uint32_t i = 0; i < numPoints; i++) {
    naturalOrder[i] = i;
  }
  const std::vector<uint32_t> hilbertOrder = EBGeometry::SFC::order<EBGeometry::SFC::Hilbert>(positions);

  const auto clusterNat = runEbQuery(clusterBvh, positions, naturalOrder);
  const auto clusterSFC = runEbQuery(clusterBvh, positions, hilbertOrder);
  const auto sahGrpNat  = runEbQuery(sahBvh, positions, naturalOrder);
  const auto sahGrpSFC  = runEbQuery(sahBvh, positions, hilbertOrder);
  const auto sahPtsNat  = runEbQuery(*sahPointsBvh, positions, naturalOrder);
  const auto sahPtsSFC  = runEbQuery(*sahPointsBvh, positions, hilbertOrder);

  // ---- nanoflann: build kd-tree, then query it in both orders too (the lever helps any tree) ----
  const Cloud                               cloud{positions};
  nanoflann::KDTreeSingleIndexAdaptorParams params(
    nanoflannLeaf, nanoflann::KDTreeSingleIndexAdaptorFlags::SkipInitialBuildIndex, 1u);
  KDTree tree(3, cloud, params);
  timer.start();
  tree.buildIndex();
  timer.stop();
  const double nfBuild = timer.seconds();

  const auto nfNat = runNfQuery(tree, positions, naturalOrder);
  const auto nfSFC = runNfQuery(tree, positions, hilbertOrder);

  // ---- cross-check: every strategy must agree on each point's nearest distance (ties aside) ----
  const T    tol           = std::is_same_v<T, float> ? T(1.0e-4) : T(1.0e-9);
  const auto countMismatch = [&](const std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {
    size_t n = 0;
    for (size_t p = 0; p < numPoints; p++) {
      const T da = (positions[a[p]] - positions[p]).length2();
      const T db = (positions[b[p]] - positions[p]).length2();
      if (std::abs(da - db) > tol * std::max(std::max(da, db), T(1.0))) {
        n++;
      }
    }
    return n;
  };
  const size_t mmCluster = countMismatch(clusterNat.first, nfNat.first);
  const size_t mmSah     = countMismatch(sahGrpNat.first, nfNat.first);
  const size_t mmSahPts  = countMismatch(sahPtsNat.first, nfNat.first);

  // ---- report ----
  const double us  = 1.0e6 / double(numPoints);
  const auto   row = [&](const char* label, double buildSec, double qNat, double qSFC) {
    const double gain = 100.0 * (1.0 - qSFC / qNat);
    std::cout << std::left << std::setw(24) << label << std::right << std::fixed << std::setw(11)
              << std::setprecision(1) << 1.0e3 * buildSec << std::setw(12) << std::setprecision(3) << qNat * us
              << std::setw(12) << std::setprecision(3) << qSFC * us << std::setw(8) << std::setprecision(0) << gain
              << "%\n";
  };
  std::cout << std::left << std::setw(24) << "Strategy" << std::right << std::setw(11) << "Build ms" << std::setw(12)
            << "Q nat" << std::setw(12) << "Q SFC" << std::setw(9) << "SFC gain" << '\n';
  std::cout << "  (query us/pt; natural index order vs Hilbert-sorted processing order)\n";
  std::cout << std::string(68, '-') << '\n';
  row("EBGeometry SAH (points)", ebSahPointsBuild, sahPtsNat.second, sahPtsSFC.second);
  row("EBGeometry SAH (groups)", ebPack + ebSahBuild, sahGrpNat.second, sahGrpSFC.second);
  row("EBGeometry ClusterSAH", ebPack + ebClusterBuild, clusterNat.second, clusterSFC.second);
  row("nanoflann kd-tree", nfBuild, nfNat.second, nfSFC.second);
  std::cout << std::string(68, '-') << '\n';
  std::cout << std::setprecision(2);
  std::cout << "Best (SFC-ordered) query vs nanoflann(SFC) -- SAH(points): " << (sahPtsSFC.second / nfSFC.second)
            << "x its time  (<1 = faster than nanoflann)\n";
  std::cout << "Nearest-distance mismatches vs nanoflann: SAH(points) " << mmSahPts << ", SAH(groups) " << mmSah
            << ", ClusterSAH " << mmCluster << " / " << numPoints
            << (mmSahPts == 0 && mmSah == 0 && mmCluster == 0 ? "  (all identical)\n" : "  (see note on ties)\n");

  return 0;
}
