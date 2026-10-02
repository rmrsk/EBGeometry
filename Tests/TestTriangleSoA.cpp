// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for EBGeometry_TriangleSoA.hpp (TriangleSoAT). Previously only exercised indirectly
// through TriMeshSDF's internal packing (TestDCEL.cpp/TestBVH.cpp); this tests the SoA group
// directly against the individual Triangle::signedDistance() values it's built from.

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;

namespace {

template <class T>
using Tri = Triangle<T, short>;

constexpr size_t W = 4;

template <class T>
using SoA = TriangleSoAT<T, W>;

template <class T>
using AoSoA = TriangleAoSoA<T, short, W>;

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
    tris.emplace_back(Array<Vec3, 3>{base + Vec3(0, 0, 0), base + Vec3(1, 0, 0), base + Vec3(0, 1, 0)});
  }
  return tris;
}

// The same four triangles, each tagged with a distinct metadata value (10, 11, 12, 13) so a
// closest-triangle query's returned metadata can be checked against the known closest triangle.
template <class T>
std::vector<Tri<T>>
fourTrianglesWithMeta()
{
  auto tris = fourTriangles<T>();
  for (size_t i = 0; i < tris.size(); i++) {
    tris[i].setMetaData(static_cast<short>(10 + i));
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

TEMPLATE_TEST_CASE("TriangleSoAT::signedDistances returns per-lane distances whose min-|value| "
                   "equals signedDistance()",
                   "[TriangleSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto tris = fourTriangles<T>();

  SoA<T> group;
  group.template pack<short>(tris.data(), static_cast<uint32_t>(tris.size()));

  for (const auto& p : queryPoints<T>()) {
    const Array<T, W> perLane = group.signedDistances(p);

    // Each lane must match the corresponding individual triangle, and the min-|lane| must equal the
    // reduced signedDistance().
    T best = std::numeric_limits<T>::max();
    for (size_t i = 0; i < tris.size(); i++) {
      REQUIRE_THAT(perLane[i], withinAbsT(tris[i].signedDistance(p), looseMargin<T>()));

      if (std::abs(perLane[i]) < std::abs(best)) {
        best = perLane[i];
      }
    }

    REQUIRE_THAT(group.signedDistance(p), withinAbsT(best, looseMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("TriangleAoSoA::signedDistance(point, meta) reports the closest triangle's "
                   "metadata and matches the plain overload",
                   "[TriangleSoA][TriangleAoSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto tris = fourTrianglesWithMeta<T>();
  REQUIRE(tris.size() == W);

  AoSoA<T> group;
  group.pack(tris.data(), static_cast<uint32_t>(tris.size()));

  for (const auto& p : queryPoints<T>()) {
    // Brute-force the closest triangle (by |signed distance|) and its metadata.
    T     best         = std::numeric_limits<T>::max();
    short expectedMeta = -1;
    for (const auto& tri : tris) {
      const T d = tri.signedDistance(p);

      if (std::abs(d) < std::abs(best)) {
        best         = d;
        expectedMeta = tri.getMetaData();
      }
    }

    short   meta = -1;
    const T d    = group.signedDistance(p, meta);

    REQUIRE_THAT(d, withinAbsT(best, looseMargin<T>()));
    REQUIRE(meta == expectedMeta);

    // The plain overload (hot path) must agree with the metadata-reporting one.
    REQUIRE_THAT(group.signedDistance(p), withinAbsT(best, looseMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("TriangleAoSoA::getMetaData returns each lane's metadata and pads with the last "
                   "real triangle",
                   "[TriangleSoA][TriangleAoSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  auto tris = fourTrianglesWithMeta<T>();
  tris.resize(2); // Pack only the first 2; lanes 2 and 3 are padded with the last real triangle.

  AoSoA<T> group;
  group.pack(tris.data(), 2);

  REQUIRE(group.getMetaData(0) == short(10));
  REQUIRE(group.getMetaData(1) == short(11));
  REQUIRE(group.getMetaData(2) == short(11)); // padded -> last real triangle's metadata
  REQUIRE(group.getMetaData(3) == short(11)); // padded -> last real triangle's metadata
}

// ─────────────────────────────────────────────────────────────────────────────
// Device: a host-packed TriangleAoSoA is queried in a kernel and matches the host
// ─────────────────────────────────────────────────────────────────────────────

// The signed distance from a query point to the group, which is held by value. With m_withMeta, it
// comes from the overload that also reports the closest triangle's metadata.
template <class T>
struct TriangleAoSoADistanceQuery
{
  AoSoA<T> m_group;
  bool     m_withMeta;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    short closestMeta = 0;

    return m_withMeta ? m_group.signedDistance(a_point, closestMeta) : m_group.signedDistance(a_point);
  }
};

// The closest triangle's metadata, as reported by signedDistance(point, meta).
template <class T>
struct TriangleAoSoAClosestMetaQuery
{
  AoSoA<T> m_group;

  EBGEOMETRY_HOST_DEVICE
  int
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    short closestMeta = 0;

    (void)m_group.signedDistance(a_point, closestMeta);

    return static_cast<int>(closestMeta);
  }
};

// The metadata of lane i % W.
template <class T>
struct TriangleAoSoAMetaDataQuery
{
  AoSoA<T> m_group;

  EBGEOMETRY_HOST_DEVICE
  int
  operator()(const int& a_i) const noexcept
  {
    return static_cast<int>(m_group.getMetaData(static_cast<size_t>(a_i) % W));
  }
};

// Require two integer result lists to be identical, element by element.
inline void
requireSameIntegers(const std::vector<int>& a_device, const std::vector<int>& a_host)
{
  REQUIRE(a_device.size() == a_host.size());

  for (size_t i = 0; i < a_host.size(); i++) {
    INFO("query " << i);
    REQUIRE(a_device[i] == a_host[i]);
  }
}

TEMPLATE_TEST_CASE("TriangleAoSoA: device query surface matches the host",
                   "[TriangleAoSoA][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  const auto tris = fourTrianglesWithMeta<T>();

  AoSoA<T> group;
  group.pack(tris.data(), static_cast<uint32_t>(tris.size()));

  // The triangles lie in the z = 0 plane at x in [0, 10]; the grid surrounds all of them on both
  // sides of the plane, so each one is the closest for some queries and both signs occur.
  const auto points = queryGrid<T>(Vec3T<T>(T(-2), T(-2), T(-1.5)), Vec3T<T>(T(12), T(3), T(1.5)), 10);

  for (const bool withMeta : {false, true}) {
    INFO("with metadata: " << withMeta);
    requireSameResults(evaluateOnDevice<T>(TriangleAoSoADistanceQuery<T>{group, withMeta}, points),
                       evaluateOnHost<T>(TriangleAoSoADistanceQuery<T>{group, withMeta}, points));
  }

  requireSameIntegers(evaluateOnDevice<int>(TriangleAoSoAClosestMetaQuery<T>{group}, points),
                      evaluateOnHost<int>(TriangleAoSoAClosestMetaQuery<T>{group}, points));

  std::vector<int> lanes;

  for (int i = 0; i < 256; i++) {
    lanes.push_back(i);
  }

  requireSameIntegers(evaluateOnDevice<int>(TriangleAoSoAMetaDataQuery<T>{group}, lanes),
                      evaluateOnHost<int>(TriangleAoSoAMetaDataQuery<T>{group}, lanes));
}

TEST_CASE("TriangleSoAT/TriangleAoSoA::pack: reject a count outside [1, W] and a null array",
          "[TriangleSoA][TriangleAoSoA][death]")
{
  using T = double;

  // EBGEOMETRY_REQUIREs, so they abort in every build. The count is checked before any triangle is
  // read, so the fixture needs no more than W triangles even for the count of W + 1.
  const std::vector<Tri<T>> tris = fourTriangles<T>();

  REQUIRE(abortsWith(
    [&tris] {
      SoA<T> soa;
      soa.pack(tris.data(), 0U);
    },
    "TriangleSoAT::pack: the triangle count must be between 1 and 4 (0)"));

  REQUIRE(abortsWith(
    [&tris] {
      SoA<T> soa;
      soa.pack(tris.data(), static_cast<uint32_t>(W + 1));
    },
    "TriangleSoAT::pack: the triangle count must be between 1 and 4 (5)"));

  REQUIRE(abortsWith(
    [] {
      SoA<T> soa;
      soa.pack(static_cast<const Tri<T>*>(nullptr), 1U);
    },
    "TriangleSoAT::pack: the triangle array must not be null"));

  REQUIRE(abortsWith(
    [&tris] {
      AoSoA<T> aosoa;
      aosoa.pack(tris.data(), 0U);
    },
    "TriangleAoSoA::pack: the triangle count must be between 1 and 4 (0)"));
}
