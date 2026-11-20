// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// Test helpers shared by the BVH tests (TestBVH, TestBVHDevice): the traversal probes and functors that run on the
// host and on a device, the rays that meet a box only along an edge, and a BVH of point ids with the functors that
// move its points and read their boxes.

#ifndef EBGEOMETRY_TESTBVHHELPERS_HPP
#define EBGEOMETRY_TESTBVHHELPERS_HPP

#include "EBGeometry.hpp"

#include <cstddef>
#include <cstdint>
#include <random>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace EBGeometryTestBVH {

using namespace EBGeometry;

inline std::string
dataPath(const std::string& a_filename)
{
  return std::string(EBGEOMETRY_TEST_DATA_DIR) + "/" + a_filename;
}

// Comparisons between a brute-force scan and a BVH-accelerated traversal (or between different
// partitioning/packing/build strategies) chain many more dot/cross products across the mesh's 36
// faces, so tolerate more accumulated rounding error than a simple format round-trip.
template <class T>
double
traversalMargin()
{
  return std::is_same_v<T, float> ? 1.0e-2 : 1.0e-6;
}

// Bare point primitive with no signedDistance() (or any other) member at all -- used below to
// prove that PackedBVH::traverse() imposes no interface requirement on its primitive type,
// unlike a caller-built signed-distance wrapper (e.g. MeshSDF/TriMeshSDF::signedDistance()),
// which requires P::signedDistance(Vec3T<T>). Also reused by the degenerate-axis
// bottomUpSortAndPartition test further below, whose nearest-neighbor query likewise goes
// through traverse().
template <class T>
struct BareTestPoint
{
  Vec3T<T> m_pos;
};

// A handful of query points spanning inside, outside, and near-surface -- enough to catch a BVH
// traversal or partitioning bug without the test suite taking noticeably longer to run.
template <class T>
std::vector<Vec3T<T>>
queryPoints()
{
  using Vec3 = Vec3T<T>;
  return {
    Vec3(0, 0, 0),
    Vec3(0.5, 0.5, 0.5),
    Vec3(1.0, 1.0, 1.0),
    Vec3(1.5, 0.0, 0.0),
    Vec3(0.0, 1.5, 0.0),
    Vec3(0.0, 0.0, 1.5),
    Vec3(-1.5, -1.5, -1.5),
    Vec3(3.0, 3.0, 3.0),
    Vec3(10.0, 0.0, 0.0),
    Vec3(-5.0, 2.0, 1.0),
  };
}

// traverse() is templated on its callables rather than taking std::function, and the CUDA/HIP
// builds do not pass --extended-lambda, so device callers must hand it functors. These are those
// functors. The host suite (TestBVH) runs them too, so it compiles and exercises the exact types the kernel
// (TestBVHDevice) uses -- the kernel launch itself is then the only part it does not check.
template <class T>
struct NearestLeafEval
{
  EBGeometry::PODSpan<const BareTestPoint<T>> m_prims;
  Vec3T<T>                                    m_query;

  EBGEOMETRY_HOST_DEVICE
  void
  operator()(T& a_state, size_t a_offset, size_t a_count) const noexcept
  {
    for (size_t i = 0; i < a_count; i++) {
      const T d2 = (m_prims[a_offset + i].m_pos - m_query).length2();

      if (d2 < a_state) {
        a_state = d2;
      }
    }
  }
};

template <class T>
struct IdentityPruneDist2
{
  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const T& a_state) const noexcept
  {
    return a_state;
  }
};

/**
 * @brief The entire body of the device query functor PackedBvhTraversalQuery (TestBVHDevice.cpp),
 * factored out so the host suite compiles and runs the exact code the device runs -- not merely the
 * same functors.
 *
 * The BVH is taken by value and const, matching the device side exactly. Both halves matter: by value
 * because that is how a descriptor reaches a kernel, and const because a non-const PackedBVH
 * selects getPrimitives()'s mutable overload, whose PODSpan<P> does not convert to the functors'
 * PODSpan<const P>. A test that reaches traverse() only through a const reference exercises a
 * different overload than the kernel does, and so cannot catch that mismatch.
 */
template <class T, size_t K>
EBGEOMETRY_HOST_DEVICE
T
packedBvhTraversalProbe(const EBGeometry::BVH::PackedBVH<T, BareTestPoint<T>, K> a_bvh, const Vec3T<T> a_query) noexcept
{
  T state = EBGeometry::Math::Limits<T>::max();

  const NearestLeafEval<T>    evalLeaf{a_bvh.getPrimitives(), a_query};
  const IdentityPruneDist2<T> pruneDist2{};

  // Spelled out, not BVH::PointQuery{a_query}: HIP treats a deduction guide as host-only, and this
  // function also runs on a device.
  a_bvh.traverse(BVH::PointQuery<T>{a_query}, state, evalLeaf, pruneDist2);

  // Also exercise the plain accessors so a broken base() shows up even if traversal were to pass.
  return state + T(a_bvh.getPrimitives().size()) + a_bvh.getBoundingVolume().getLowCorner().length();
}

// Functors for a nearest-point search over PackedBVH::traverse(), written as structs so the same code
// runs on host and device: the leaf scan, the bound (the best squared distance so far) and a visit
// callback that descends everywhere and counts the nodes entered.
template <class T>
struct TraverseNearestLeaf
{
  EBGeometry::PODSpan<const BareTestPoint<T>> m_prims;
  Vec3T<T>                                    m_query;

  EBGEOMETRY_HOST_DEVICE
  void
  operator()(T& a_best, size_t a_offset, size_t a_count) const noexcept
  {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      const T d2 = (m_prims[static_cast<uint32_t>(i)].m_pos - m_query).length2();

      if (d2 < a_best) {
        a_best = d2;
      }
    }
  }
};

template <class T>
struct TraverseCountNodes
{
  uint32_t* m_count;

  template <class Node>
  EBGEOMETRY_HOST_DEVICE
  bool
  operator()(T&, uint32_t, const Node&, const T&) const noexcept
  {
    (*m_count)++;

    return true;
  }
};

/**
 * @brief Squared distance from a_query to the nearest point, found with PackedBVH::traverse() using a
 * visit callback and the FarthestFirst order (the non-default branches of the walk).
 * @details The body of the device query functor CustomTraversalQuery (TestBVHDevice.cpp), factored out so the host suite
 * runs exactly the code the device runs. The BVH is taken by value and const, as a kernel receives it.
 */
template <class T, size_t K>
EBGEOMETRY_HOST_DEVICE
T
customTraversalProbe(const EBGeometry::BVH::PackedBVH<T, BareTestPoint<T>, K> a_bvh, const Vec3T<T> a_query) noexcept
{
  T        best  = EBGeometry::Math::Limits<T>::max();
  uint32_t nodes = 0;

  a_bvh.traverse(EBGeometry::BVH::PointQuery<T>{a_query},
                 best,
                 TraverseNearestLeaf<T>{a_bvh.getPrimitives(), a_query},
                 IdentityPruneDist2<T>{},
                 TraverseCountNodes<T>{&nodes},
                 EBGeometry::BVH::FarthestFirst{});

  // Every walk enters at least the root.
  return (nodes > 0) ? best : T(-1);
}

// Random boxes, half of them on a grid of cells 0.125 wide so that many share faces.
template <class T>
std::vector<std::pair<BoundingVolumes::AABBT<T>, BoundingVolumes::AABBT<T>>>
rayTestBoxes(const int a_count)
{
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;

  std::mt19937                      rng(11);
  std::uniform_real_distribution<T> coord(T(-1), T(1));
  std::uniform_real_distribution<T> half(T(0.01), T(0.08));

  std::vector<std::pair<AABB, AABB>> boxes;

  for (int i = 0; i < a_count; i++) {
    const bool onGrid = (i % 2 == 0);
    const Vec3 c      = onGrid ? Vec3(T(0.125) * T(i % 8), T(0.125) * T((i / 8) % 8), T(0.125) * T((i / 64) % 8))
                               : Vec3(coord(rng), coord(rng), coord(rng));
    const Vec3 h      = onGrid ? T(0.0625) * Vec3::ones() : Vec3(half(rng), half(rng), half(rng));

    boxes.emplace_back(AABB(c - h, c + h), AABB(c - h, c + h));
  }

  return boxes;
}

// The nearest box entered along a ray, by the walk: the state is the nearest entry so far, which is also
// the bound.
template <class T, size_t K>
struct NearestBoxEntered
{
  EBGeometry::PODSpan<const BoundingVolumes::AABBT<T>> m_boxes;
  EBGeometry::BVH::RayQuery<T>                         m_ray;

  EBGEOMETRY_HOST_DEVICE
  void
  operator()(T& a_best, size_t a_offset, size_t a_count) const noexcept
  {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      const T t = m_ray.key(m_boxes[static_cast<uint32_t>(i)]);

      if (t < a_best) {
        a_best = t;
      }
    }
  }
};

template <class T, size_t K>
EBGEOMETRY_HOST_DEVICE
T
nearestBoxEntered(const EBGeometry::BVH::PackedBVH<T, BoundingVolumes::AABBT<T>, K>& a_bvh,
                  const EBGeometry::BVH::RayQuery<T>&                                a_ray) noexcept
{
  T best = EBGeometry::Math::Limits<T>::infinity();

  a_bvh.traverse(a_ray, best, NearestBoxEntered<T, K>{a_bvh.getPrimitives(), a_ray}, IdentityPruneDist2<T>{});

  return best;
}

// Rays that meet a box only along an edge: in exact arithmetic the entry equals the exit up to a small positive
// margin (at least 0.06 of the precision's epsilon, relative, checked in long double against the direction itself),
// and without the slab test's widening the rounded entry lands past the rounded exit, a miss. Found by a search over
// rays aimed at a random point of a box's edge; about one such ray in a hundred is missed without the widening.
template <class T>
struct EdgeRay
{
  T m_lo[3];
  T m_hi[3];
  T m_origin[3];
  T m_direction[3];
};

template <class T>
std::vector<EdgeRay<T>>
edgeRays()
{
  if constexpr (std::is_same_v<T, float>) {
    return {
      {{0x1.9f7fdcp-1f, 0x1.d86e9p-1f, 0x1.45d20ep-2f},
       {0x1.fd764p-1f, 0x1.42eaeep+0f, 0x1.2980a6p-1f},
       {0x1.989594p+0f, 0x1.203872p-1f, 0x1.7854e6p-1f},
       {-0x1.522938p-2f, 0x1.89023ep-2f, -0x1.9862d2p-4f}},
      {{0x1.17735ep-2f, 0x1.e9b50ep-1f, 0x1.0882dap-2f},
       {0x1.bf6c7p-2f, 0x1.22a324p+0f, 0x1.740996p-1f},
       {0x1.00871ep+0f, 0x1.c7308cp-2f, 0x1.32af8ep+0f},
       {-0x1.1d07cep-2f, 0x1.5c6846p-2f, -0x1.2cb944p-2f}},
      {{0x1.4f964cp-2f, 0x1.d4f7acp-1f, 0x1.66b2f4p-1f},
       {0x1.9c9d96p-1f, 0x1.5987a8p+0f, 0x1.bdc1fep-1f},
       {0x1.a41606p+0f, 0x1.2d2a26p-1f, 0x1.6c700ap+0f},
       {-0x1.b9fee4p-2f, 0x1.930ffep-2f, -0x1.36dc3cp-2f}},
      {{0x1.76723ap-4f, 0x1.c84cd4p-2f, 0x1.cb05dap-4f},
       {0x1.2a148cp-3f, 0x1.27eb4cp+0f, 0x1.e82354p-3f},
       {-0x1.a871p-1f, 0x1.11b2e2p+1f, 0x1.a43bd2p-1f},
       {0x1.f5fe9p-2f, -0x1.fa0396p-2f, -0x1.5e1238p-2f}},
    };
  }
  else {
    return {
      {{0x1.6d3312f1106efp-4, 0x1.46964c6bfa399p-1, 0x1.98715d9d8c7f4p-5},
       {0x1.346d96167b9e7p-1, 0x1.126c626907ccap+0, 0x1.20cd778921d8ep-2},
       {0x1.5612e49a2e814p+0, 0x1.8f70f0b2eb4e3p-1, 0x1.615888b9c37a8p-1},
       {-0x1.c3e03d494e82bp-2, 0x1.6760ee63eeabp-3, -0x1.1d47f32e15ca6p-2}},
      {{0x1.c4a21076e3793p-1, 0x1.28e6614b31203p-2, 0x1.9fac2f8bc632cp-1},
       {0x1.a9de9a8f941ccp+0, 0x1.3c2ebd91a5bcp+0, 0x1.757adf9ce6d1ap+0},
       {0x1.cde7aa7b0bcf6p+0, 0x1.e800369fe4eccp-3, 0x1.1b403a8e3cee4p+1},
       {-0x1.0dc62d1800cdcp-4, 0x1.dd98e4f9e5c44p-2, -0x1.7255af46752eep-2}},
      {{0x1.55b0205f36163p-1, 0x1.eb6803fff3f51p-1, 0x1.fef4d56fcdde7p-1},
       {0x1.2d7b4de863b67p+0, 0x1.a9fa11552c186p+0, 0x1.0b1edc5c1765bp+0},
       {0x1.91b244f5bee99p+0, 0x1.7c350944d42e7p-1, 0x1.0840ee3feb4p-6},
       {-0x1.42b886adb31d8p-3, 0x1.7bca62f9ba906p-2, 0x1.9f6a7b5f0d1cep-2}},
      {{0x1.9518162bfd57fp-3, 0x1.503eca2d72242p-2, 0x1.fe08368434cc4p-1},
       {0x1.7fa46c65101d1p-2, 0x1.40b2557edf006p+0, 0x1.1680bd1eb7827p+0},
       {-0x1.734e8e35a00eep-1, 0x1.e8ab07ce5d2a7p+0, 0x1.64029838ac3f4p+0},
       {0x1.dbfb352c0ab44p-2, -0x1.1bf44576c3609p-2, -0x1.199abd068b1a8p-3}},
    };
  }
}

// The box of point a_id, read from a point array the caller keeps in a pool: what a BVH of ids reads
// in BVH::refit().
template <class T>
struct PointBoxById
{
  const Vec3T<T>* m_points;

  EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  operator()(const uint32_t a_id) const noexcept
  {
    return BoundingVolumes::AABBT<T>(m_points[a_id], m_points[a_id]);
  }
};

// The caller's own kernel: moves every point by its own amount, so that no two leaves move alike.
template <class T>
struct MovePoints
{
  Vec3T<T>* m_points;

  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_i) const noexcept
  {
    const T s = T(0.01) * T(a_i % 17);

    m_points[a_i] += Vec3T<T>(T(1) + s, T(-0.5) - s, T(2) * s);
  }
};

// A BVH of point ids over a_numPoints points of a curve, with the points in a_points, both in a_pool.
template <class T, size_t K>
BVH::PackedBVH<T, uint32_t, K>
pointIdBVH(Pool& a_pool, PODVector<Vec3T<T>>& a_points, const uint32_t a_numPoints)
{
  using AABB = BoundingVolumes::AABBT<T>;

  std::vector<std::pair<uint32_t, AABB>> flat;

  a_points.reserveSizedFrom(a_pool, a_numPoints);

  for (uint32_t i = 0; i < a_numPoints; i++) {
    const T        t = T(0.1) * T(i);
    const Vec3T<T> p(std::sin(t) * t, std::cos(T(1.3) * t) * t, T(0.05) * T(i % 23));

    a_points.at(a_pool.base(), i) = p;

    flat.emplace_back(i, AABB(p, p));
  }

  return BVH::PackedBVH<T, uint32_t, K>::buildBottomUp(a_pool, flat, size_t(4));
}

// Every node's box must be exactly the union of its subtree's point boxes (unions of min and max are
// exact, so the comparison is too), and the nearest point found through the BVH that of a brute-force
// scan.
template <class T, size_t K>
void
requireTightAndQueryable(const BVH::PackedBVH<T, uint32_t, K>& a_bvh, const std::vector<Vec3T<T>>& a_points)
{
  using AABB = BoundingVolumes::AABBT<T>;

  const auto nodes = a_bvh.getNodes();
  const auto prims = a_bvh.getPrimitives();

  std::vector<AABB> expected(nodes.size());

  for (uint32_t i = static_cast<uint32_t>(nodes.size()); i-- > 0;) {
    AABB bv;

    if (nodes[i].isLeaf()) {
      for (uint32_t p = 0; p < nodes[i].getNumPrimitives(); p++) {
        const auto& x = a_points[prims[nodes[i].getPrimitivesOffset() + p]];

        bv = bv.merged(AABB(x, x));
      }
    }
    else {
      for (const uint32_t child : nodes[i].getChildOffsets()) {
        bv = bv.merged(expected[child]);
      }
    }

    expected[i] = bv;

    const AABB got = a_bvh.getBoundingVolume(nodes[i]);

    for (int d = 0; d < 3; d++) {
      REQUIRE(got.getLowCorner()[d] == bv.getLowCorner()[d]);
      REQUIRE(got.getHighCorner()[d] == bv.getHighCorner()[d]);
    }
  }

  for (const auto& q : queryPoints<T>()) {
    T state = std::numeric_limits<T>::max();

    const auto evalLeaf = [&](T& a_state, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = 0; i < a_count; i++) {
        a_state = std::min(a_state, (a_points[prims[a_offset + i]] - q).length2());
      }
    };

    a_bvh.traverse(BVH::PointQuery{q}, state, evalLeaf, [](const T& a_state) noexcept -> T { return a_state; });

    T brute = std::numeric_limits<T>::max();

    for (const auto& x : a_points) {
      brute = std::min(brute, (x - q).length2());
    }

    REQUIRE(state == brute);
  }
}

} // namespace EBGeometryTestBVH

#endif
