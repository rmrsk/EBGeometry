// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// Test suite for refitting BVHs to moved primitives: TreeBVH::refit(), PackedBVH::refit() and BVH::refit() with
// its RefitSchedule.

#include "EBGeometry.hpp"
#include "TestBVHHelpers.hpp"
#include "TestConstructions.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"
#include "TestUtilities.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <limits>
#include <memory>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using namespace EBGeometryTestBVH;
using Catch::Matchers::WithinRel;

TEMPLATE_TEST_CASE("BVH refit: TreeBVH::refit and PackedBVH::refit update bounding volumes for a "
                   "moving geometry, keeping queries correct without a rebuild",
                   "[BVH][refit]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  Pool pool(hostMemoryResource());

  // A modest, non-lattice point cloud -- enough to force multiple leaves/levels through the default
  // partitioner while keeping the brute-force cross-check fast.
  std::vector<Vec3> positions;
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      for (int k = 0; k < 5; k++) {
        positions.emplace_back(T(i) + T(0.3) * T(j), T(j) - T(0.2) * T(k), T(k) + T(0.1) * T(i));
      }
    }
  }
  REQUIRE(positions.size() == 125);

  // Keep mutable handles so the geometry can be *moved* after the BVH is built. TreeBVH stores
  // these very objects as shared_ptr<const Pnt>, so mutating a handle is what a moving geometry does
  // for the tree. The packed BVH is different: packing copies each primitive out, so it owns its own
  // copies and cannot see a handle change. Its counterpart is to move the packed primitives directly
  // through the mutable getPrimitives(); the loop below moves both representations by the same
  // displacement so the two must still agree afterwards.
  std::vector<std::shared_ptr<Pnt>> handles;
  handles.reserve(positions.size());
  for (const auto& pos : positions) {
    handles.emplace_back(std::make_shared<Pnt>(Pnt{pos}));
  }

  BVH::PrimAndBVList<Pnt, AABB> primsAndBVs;
  for (const auto& h : handles) {
    primsAndBVs.emplace_back(h, AABB(h->m_pos, h->m_pos));
  }

  auto tree = std::make_shared<BVH::TreeBVH<T, Pnt, AABB, K>>(primsAndBVs);
  tree->topDownSortAndPartition();

  auto packed = tree->pack(pool);
  REQUIRE(packed.getPrimitives().size() == positions.size());

  // A single primitive's current bounding volume: a zero-extent box at its (possibly moved) position.
  const auto bvConstructor = [](const Pnt& a_p) noexcept -> AABB { return AABB(a_p.m_pos, a_p.m_pos); };

  const auto& prims      = packed.getPrimitives();
  const auto  pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

  // Nearest-neighbor (squared) over the packed BVH via traverse(). If refit left any node volume
  // too small to enclose its moved primitives, pruning would wrongly discard the true nearest and
  // this would disagree with the brute-force scan below.
  const auto nearest2 = [&prims, &packed, &pruneDist2](const Vec3& a_query) noexcept -> T {
    T state = std::numeric_limits<T>::max();

    const auto evalLeaf = [&prims, &a_query](T& a_state, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = 0; i < a_count; i++) {
        const T d2 = (prims[a_offset + i].m_pos - a_query).length2();

        if (d2 < a_state) {
          a_state = d2;
        }
      }
    };

    packed.traverse(BVH::PointQuery{a_query}, state, evalLeaf, pruneDist2);

    return state;
  };

  // Tight bounding box over the current handle positions -- what a correctly-refitted root equals.
  const auto tightRoot = [&handles]() noexcept -> AABB {
    Vec3 lo = +Vec3::max();
    Vec3 hi = -Vec3::max();

    for (const auto& h : handles) {
      lo = min(lo, h->m_pos);
      hi = max(hi, h->m_pos);
    }

    return AABB(lo, hi);
  };

  const auto bruteNearest2 = [&handles](const Vec3& a_query) noexcept -> T {
    T brute2 = std::numeric_limits<T>::max();

    for (const auto& h : handles) {
      brute2 = std::min(brute2, (h->m_pos - a_query).length2());
    }

    return brute2;
  };

  SECTION("refit with unchanged geometry reproduces the original bounding volumes")
  {
    const AABB before = packed.getBoundingVolume();

    tree->refit(bvConstructor);
    packed.refit(bvConstructor);

    for (int d = 0; d < 3; d++) {
      REQUIRE_THAT(packed.getBoundingVolume().getLowCorner()[d],
                   withinAbsT(before.getLowCorner()[d], tightMargin<T>()));
      REQUIRE_THAT(packed.getBoundingVolume().getHighCorner()[d],
                   withinAbsT(before.getHighCorner()[d], tightMargin<T>()));
    }

    for (const auto& q : queryPoints<T>()) {
      REQUIRE_THAT(nearest2(q), withinAbsT(bruteNearest2(q), traversalMargin<T>()));
    }
  }

  SECTION("refit after moving the geometry keeps every query correct")
  {
    // Displace every point by a per-point, non-uniform translation: primitives shift without
    // migrating between leaves -- exactly the refit use case.
    for (size_t i = 0; i < handles.size(); i++) {
      const T s = T(0.05) * T(i);

      handles[i]->m_pos += Vec3(T(2.0) + s, T(-1.0) - s, T(0.5) * s);
    }

    // The packed BVH owns its primitives by value, so move them through its own array. Packing
    // reorders primitives into leaf order, so the i-th packed primitive is not the i-th handle --
    // match them up by position instead, which is well defined here because the cloud has no
    // duplicate points.
    auto packedPrims = packed.getPrimitives();

    REQUIRE(packedPrims.size() == positions.size());

    for (auto& prim : packedPrims) {
      size_t src = positions.size();

      for (size_t i = 0; i < positions.size(); i++) {
        if (prim.m_pos == positions[i]) {
          src = i;

          break;
        }
      }

      REQUIRE(src < positions.size());

      prim.m_pos = handles[src]->m_pos;
    }

    tree->refit(bvConstructor);
    packed.refit(bvConstructor);

    // Both roots must agree with the tight box over the moved cloud.
    const AABB tight = tightRoot();

    for (int d = 0; d < 3; d++) {
      REQUIRE_THAT(tree->getBoundingVolume().getLowCorner()[d], withinAbsT(tight.getLowCorner()[d], tightMargin<T>()));
      REQUIRE_THAT(tree->getBoundingVolume().getHighCorner()[d],
                   withinAbsT(tight.getHighCorner()[d], tightMargin<T>()));
      REQUIRE_THAT(packed.getBoundingVolume().getLowCorner()[d], withinAbsT(tight.getLowCorner()[d], tightMargin<T>()));
      REQUIRE_THAT(packed.getBoundingVolume().getHighCorner()[d],
                   withinAbsT(tight.getHighCorner()[d], tightMargin<T>()));
    }

    for (const auto& q : queryPoints<T>()) {
      REQUIRE_THAT(nearest2(q), withinAbsT(bruteNearest2(q), traversalMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("BVH::refit: refits a BVH in its host pool to the boxes of the moved primitives",
                   "[BVH][refit]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t   K = 4;
  constexpr uint32_t N = 2000;

  Pool pool(hostMemoryResource());

  PODVector<Vec3T<T>> points;

  const auto bvh = pointIdBVH<T, K>(pool, points, N);

  const BVH::RefitSchedule schedule(pool, bvh);

  pool.freeze();

  // Five levels or more: the depth grouping is exercised, not a single leaf.
  REQUIRE(schedule.numLevels() >= 5);

  for (uint32_t i = 0; i < N; i++) {
    MovePoints<T>{points.data(pool.base())}(i);
  }

  BVH::refit(pool, bvh, schedule, PointBoxById<T>{points.data(pool.base())});

  const Vec3T<T>* moved = points.data(pool.base());

  requireTightAndQueryable<T, K>(bvh, std::vector<Vec3T<T>>(moved, moved + N));
}

TEMPLATE_TEST_CASE("BVH::refit: a schedule built on a frozen pool, for another BVH (of another size or the same) or in "
                   "another pool aborts",
                   "[BVH][refit]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;

  using Box = PointBoxById<T>;

  REQUIRE(abortsWith(
    [] {
      Pool                pool(hostMemoryResource());
      PODVector<Vec3T<T>> points;
      const auto          bvh = pointIdBVH<T, K>(pool, points, 100);

      pool.freeze();

      const BVH::RefitSchedule schedule(pool, bvh);

      (void)schedule;
    },
    "BVH::RefitSchedule: the pool is frozen; build the schedule after the BVH and before freezing the pool"));

  REQUIRE(abortsWith(
    [] {
      Pool                pool(hostMemoryResource());
      Pool                other(hostMemoryResource());
      PODVector<Vec3T<T>> points;
      const auto          bvh = pointIdBVH<T, K>(pool, points, 100);

      const BVH::RefitSchedule schedule(other, bvh);

      (void)schedule;
    },
    "BVH::RefitSchedule: the pool must be the one the BVH lives in"));

  REQUIRE(abortsWith(
    [] {
      Pool                pool(hostMemoryResource());
      PODVector<Vec3T<T>> points;
      PODVector<Vec3T<T>> morePoints;
      const auto          bvh   = pointIdBVH<T, K>(pool, points, 100);
      const auto          other = pointIdBVH<T, K>(pool, morePoints, 300);

      const BVH::RefitSchedule schedule(pool, other);

      BVH::refit(pool, bvh, schedule, Box{points.data(pool.base())});
    },
    "BVH::refit: the schedule covers"));

  REQUIRE(abortsWith(
    [] {
      Pool                pool(hostMemoryResource());
      PODVector<Vec3T<T>> points;
      PODVector<Vec3T<T>> samePoints;
      const auto          bvh  = pointIdBVH<T, K>(pool, points, 100);
      const auto          twin = pointIdBVH<T, K>(pool, samePoints, 100);

      const BVH::RefitSchedule schedule(pool, twin);

      BVH::refit(pool, bvh, schedule, Box{points.data(pool.base())});
    },
    "BVH::refit: the schedule was built for another BVH in this pool, or for this one before a rebuild; build the "
    "schedule for this BVH"));

  REQUIRE(abortsWith(
    [] {
      Pool                pool(hostMemoryResource());
      Pool                other(hostMemoryResource());
      PODVector<Vec3T<T>> points;
      PODVector<Vec3T<T>> otherPoints;
      const auto          bvh      = pointIdBVH<T, K>(pool, points, 100);
      const auto          otherBVH = pointIdBVH<T, K>(other, otherPoints, 100);

      const BVH::RefitSchedule schedule(other, otherBVH);

      BVH::refit(pool, bvh, schedule, Box{points.data(pool.base())});
    },
    "BVH::refit: the schedule was built in a pool of another mirror chain; build it in the BVH's pool"));
}
