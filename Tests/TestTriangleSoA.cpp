// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for EBGeometry_TriangleSoA.hpp (TriangleSoAT). Previously only exercised indirectly
// through TriMeshSDF's internal packing (TestDCEL.cpp/TestBVH.cpp); this tests the SoA group
// directly against the individual Triangle::signedDistance() values it's built from.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;

namespace {

template <class T>
using Tri = Triangle<T, short>;

constexpr size_t W = 4;

template <class T>
using SoA = TriangleSoAT<T, W>;

// Four disjoint, well-separated triangles, spread out along x so each one is unambiguously the
// closest for a query point placed near it.
template <class T>
std::vector<Tri<T>>
fourTriangles()
{
  using Vec3 = Vec3T<T>;

  std::vector<Tri<T>> tris;
  for (int i = 0; i < 4; i++) {
    const Vec3 base(T(3.0 * i), T(0), T(0));
    tris.emplace_back(std::array<Vec3, 3>{base + Vec3(0, 0, 0), base + Vec3(1, 0, 0), base + Vec3(0, 1, 0)});
  }
  return tris;
}

template <class T>
T
minAbsSignedDistance(const std::vector<Tri<T>>& a_tris, const Vec3T<T>& a_point)
{
  T best = std::numeric_limits<T>::max();
  for (const auto& tri : a_tris) {
    const T d = tri.signedDistance(a_point);
    if (std::abs(d) < std::abs(best)) {
      best = d;
    }
  }
  return best;
}

template <class T>
std::vector<Vec3T<T>>
queryPoints()
{
  using Vec3 = Vec3T<T>;
  return {
    Vec3(0.25, 0.25, 0.1),
    Vec3(3.25, 0.25, -0.2),
    Vec3(6.25, 0.25, 0.05),
    Vec3(9.25, 0.25, 0),
    Vec3(-5, -5, -5),
    Vec3(20, 20, 20),
  };
}

} // namespace

TEMPLATE_TEST_CASE("TriangleSoAT::signedDistance matches the minimum-|distance| triangle when "
                   "fully packed (count == W)",
                   "[TriangleSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto tris = fourTriangles<T>();
  REQUIRE(tris.size() == W);

  SoA<T> group;
  group.template pack<short>(tris.data(), static_cast<uint32_t>(tris.size()));

  for (const auto& p : queryPoints<T>()) {
    REQUIRE_THAT(group.signedDistance(p), withinAbsT(minAbsSignedDistance<T>(tris, p), looseMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("TriangleSoAT::signedDistance is unaffected by padding when count < W",
                   "[TriangleSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  auto tris = fourTriangles<T>();
  tris.resize(2); // Pack only the first 2 of the 4 disjoint triangles; lanes 2 and 3 get padded.

  SoA<T> group;
  group.template pack<short>(tris.data(), static_cast<uint32_t>(tris.size()));

  for (const auto& p : queryPoints<T>()) {
    REQUIRE_THAT(group.signedDistance(p), withinAbsT(minAbsSignedDistance<T>(tris, p), looseMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("TriangleSoAT::signedDistance handles a single packed triangle (count == 1)",
                   "[TriangleSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  auto tris = fourTriangles<T>();
  tris.resize(1);

  SoA<T> group;
  group.template pack<short>(tris.data(), 1);

  for (const auto& p : queryPoints<T>()) {
    REQUIRE_THAT(group.signedDistance(p), withinAbsT(tris.front().signedDistance(p), looseMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("TriangleSoAT::computeBoundingVolume matches an AABB built directly from the "
                   "triangles' vertices",
                   "[TriangleSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using AABB = BoundingVolumes::AABBT<T>;

  const auto tris = fourTriangles<T>();

  SoA<T> group;
  group.template pack<short>(tris.data(), static_cast<uint32_t>(tris.size()));

  const auto bv = group.template computeBoundingVolume<AABB>();

  std::vector<Vec3> allVertices;
  for (const auto& tri : tris) {
    for (const auto& v : tri.getVertexPositions()) {
      allVertices.push_back(v);
    }
  }
  const AABB expected(allVertices);

  for (int i = 0; i < 3; i++) {
    REQUIRE_THAT(bv.getLowCorner()[i], withinAbsT(expected.getLowCorner()[i], looseMargin<T>()));
    REQUIRE_THAT(bv.getHighCorner()[i], withinAbsT(expected.getHighCorner()[i], looseMargin<T>()));
  }
}
