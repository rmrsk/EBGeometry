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
#include <iomanip>
#include <iostream>
#include <limits>
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

constexpr size_t   numPoints        = 500000;
constexpr uint64_t pointSeed        = 123456789ULL;
constexpr size_t   maxClusterGroups = 8;  // EBGeometry ClusterSAH bucket size (groups)
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

  // ---- EBGeometry: build (SFC pack + ClusterSAH PackedBVH), then all-nearest-neighbor query ----
  timer.start();
  const auto groups = buildGroups(positions);
  timer.stop();
  const double ebPack = timer.seconds();

  timer.start();
  const Packed bvh(groups, EBGeometry::BVH::ClusterSpec{maxClusterGroups});
  timer.stop();
  const double ebBvh   = timer.seconds();
  const double ebBuild = ebPack + ebBvh;

  std::vector<uint32_t> ebNearest(numPoints);
  const auto&           bvhGroups = bvh.getPrimitives();
  timer.start();
  for (size_t p = 0; p < numPoints; p++) {
    const Vec3 q         = positions[p];
    T          bestDist2 = std::numeric_limits<T>::max();
    uint32_t   bestIdx   = 0;
    const auto evalLeaf  = [&](std::pair<T, uint32_t>& st, size_t off, size_t cnt) noexcept {
      for (size_t g = 0; g < cnt; g++) {
        const PointGroup&      grp = bvhGroups[off + g];
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
    std::pair<T, uint32_t> st{bestDist2, bestIdx};
    bvh.pruneTraverse(q, st, evalLeaf, pruneDist2);
    ebNearest[p] = st.second;
  }
  timer.stop();
  const double ebQuery = timer.seconds();

  // ---- nanoflann: build kd-tree, then all-nearest-neighbor query ----
  const Cloud                               cloud{positions};
  nanoflann::KDTreeSingleIndexAdaptorParams params(
    nanoflannLeaf, nanoflann::KDTreeSingleIndexAdaptorFlags::SkipInitialBuildIndex, 1u);
  KDTree tree(3, cloud, params);
  timer.start();
  tree.buildIndex();
  timer.stop();
  const double nfBuild = timer.seconds();

  std::vector<uint32_t> nfNearest(numPoints);
  timer.start();
  for (size_t p = 0; p < numPoints; p++) {
    const T                 q[3] = {positions[p][0], positions[p][1], positions[p][2]};
    std::array<uint32_t, 2> idx{};
    std::array<T, 2>        dist{};
    (void)tree.knnSearch(q, 2, idx.data(), dist.data()); // idx[0] is self (dist 0); idx[1] is nearest other
    nfNearest[p] = idx[1];
  }
  timer.stop();
  const double nfQuery = timer.seconds();

  // ---- cross-check: both must agree on the nearest distance (indices may differ on ties) ----
  size_t  mismatches = 0;
  const T tol        = std::is_same_v<T, float> ? T(1.0e-4) : T(1.0e-9);
  for (size_t p = 0; p < numPoints; p++) {
    const T de = (positions[ebNearest[p]] - positions[p]).length2();
    const T dn = (positions[nfNearest[p]] - positions[p]).length2();
    if (std::abs(de - dn) > tol * std::max(std::max(de, dn), T(1.0))) {
      mismatches++;
    }
  }

  // ---- report ----
  const double us = 1.0e6 / double(numPoints);
  std::cout << std::left << std::setw(16) << "Library" << std::right << std::setw(14) << "Build (ms)" << std::setw(16)
            << "Query (us/pt)" << '\n';
  std::cout << std::string(46, '-') << '\n';
  std::cout << std::fixed << std::setprecision(2);
  std::cout << std::left << std::setw(16) << "EBGeometry" << std::right << std::setw(14) << 1.0e3 * ebBuild
            << std::setw(16) << std::setprecision(3) << ebQuery * us << '\n';
  std::cout << std::setprecision(2);
  std::cout << std::left << std::setw(16) << "nanoflann" << std::right << std::setw(14) << 1.0e3 * nfBuild
            << std::setw(16) << std::setprecision(3) << nfQuery * us << '\n';
  std::cout << std::string(46, '-') << '\n';
  std::cout << "EBGeometry build breakdown: " << std::setprecision(2) << 1.0e3 * ebPack << " ms SFC-pack + "
            << 1.0e3 * ebBvh << " ms ClusterSAH\n";
  std::cout << "Build ratio  (nanoflann / EBGeometry): " << std::setprecision(2) << (nfBuild / ebBuild) << "x\n";
  std::cout << "Query ratio  (nanoflann / EBGeometry): " << std::setprecision(2) << (nfQuery / ebQuery) << "x\n";
  std::cout << "Nearest-distance mismatches: " << mismatches << " / " << numPoints
            << (mismatches == 0 ? "  (identical results)\n" : "  (see note on ties)\n");

  return 0;
}
