// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// Test suite for SphereCloudBVH and its leaf group SphereAoSoA: the signed distance to a union of spheres of
// differing radii and the closest sphere, checked against a brute-force scan over the spheres, in both precisions;
// the builds from one array of spheres and from centers and radii kept apart; rebuild(); the views; and the checks
// that abort. The device queries, the build in device memory and the rebuild through GPU::parallelFor() are in
// TestSphereCloudBVHDevice.cpp.

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestSphereCloudBVHHelpers.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using namespace EBGeometryTestSphereCloud;

// ─────────────────────────────────────────────────────────────────────────────
// SphereAoSoA: the leaf group
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("SphereAoSoA: signed distances, ids and the box of a packed group",
                   "[SphereAoSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr std::size_t W = 4;

  const std::vector<SphereCloud::Sphere<T>> spheres = {
    {Vec3T<T>(T(0), T(0), T(0)), T(1)}, {Vec3T<T>(T(3), T(0), T(0)), T(0.5)}, {Vec3T<T>(T(0), T(4), T(0)), T(0)}};
  const std::vector<uint32_t> ids = {7, 11, 13};

  SphereAoSoA<T, W> group;

  group.pack(spheres.data(), ids.data(), 3);

  REQUIRE(group.numValid() == 3);

  SECTION("each lane's signed distance is |p - c| - r, and the padded lane repeats the last sphere")
  {
    const Array<T, W> distances = group.getSignedDistances(Vec3T<T>(T(0), T(0), T(0)));

    CHECK_THAT(distances[0], withinAbsT<T>(T(-1), tightMargin<T>()));
    CHECK_THAT(distances[1], withinAbsT<T>(T(2.5), tightMargin<T>()));
    CHECK_THAT(distances[2], withinAbsT<T>(T(4), tightMargin<T>()));
    CHECK_THAT(distances[3], withinAbsT<T>(T(4), tightMargin<T>()));
  }

  SECTION("each lane's separation from a sphere is |c - c_j| - (r + r_j), the same from either side")
  {
    const Array<T, W> separations = group.getSeparations(Vec3T<T>(T(0), T(0), T(0)), T(0.25));

    CHECK_THAT(separations[0], withinAbsT<T>(T(-1.25), tightMargin<T>()));
    CHECK_THAT(separations[1], withinAbsT<T>(T(2.25), tightMargin<T>()));
    CHECK_THAT(separations[2], withinAbsT<T>(T(3.75), tightMargin<T>()));
    CHECK_THAT(separations[3], withinAbsT<T>(T(3.75), tightMargin<T>()));

    // Lane 1 seen from a group holding the query sphere instead.
    const std::vector<SphereCloud::Sphere<T>> other    = {{Vec3T<T>(T(0), T(0), T(0)), T(0.25)}};
    const std::vector<uint32_t>               otherIds = {0};

    SphereAoSoA<T, W> otherGroup;

    otherGroup.pack(other.data(), otherIds.data(), 1);

    CHECK(otherGroup.getSeparations(spheres[1].m_center, spheres[1].m_radius)[0] == separations[1]);
  }

  SECTION("each lane's ray interval is where the ray enters and leaves its sphere, and empty where it misses")
  {
    Array<T, W> entry;
    Array<T, W> exit;

    // Along the x axis from x = -3: through the unit sphere at the origin and the sphere of radius 0.5 at x = 3.
    group.rayIntervals(Vec3T<T>(T(-3), T(0), T(0)), Vec3T<T>(T(1), T(0), T(0)), entry, exit);

    CHECK_THAT(entry[0], withinAbsT<T>(T(2), tightMargin<T>()));
    CHECK_THAT(exit[0], withinAbsT<T>(T(4), tightMargin<T>()));
    CHECK_THAT(entry[1], withinAbsT<T>(T(5.5), tightMargin<T>()));
    CHECK_THAT(exit[1], withinAbsT<T>(T(6.5), tightMargin<T>()));

    // The sphere of radius 0 is missed, and the padded lane is empty although it repeats that sphere.
    CHECK(entry[2] == std::numeric_limits<T>::infinity());
    CHECK(exit[2] == -std::numeric_limits<T>::infinity());
    CHECK(entry[3] == std::numeric_limits<T>::infinity());
    CHECK(exit[3] == -std::numeric_limits<T>::infinity());

    // A ray starting inside the unit sphere enters it behind its origin.
    group.rayIntervals(Vec3T<T>(T(0), T(0), T(0)), Vec3T<T>(T(0), T(0), T(1)), entry, exit);

    CHECK_THAT(entry[0], withinAbsT<T>(T(-1), tightMargin<T>()));
    CHECK_THAT(exit[0], withinAbsT<T>(T(1), tightMargin<T>()));
  }

  SECTION("each lane carries its sphere's id, and the padded lane the last one")
  {
    CHECK(group.getSphereId(0) == 7);
    CHECK(group.getSphereId(1) == 11);
    CHECK(group.getSphereId(2) == 13);
    CHECK(group.getSphereId(3) == 13);
  }

  SECTION("the box holds every sphere whole")
  {
    const auto box = group.template computeBoundingVolume<BoundingVolumes::AABBT<T>>();

    CHECK(box.getLowCorner() == Vec3T<T>(T(-1), T(-1), T(-1)));
    CHECK(box.getHighCorner() == Vec3T<T>(T(3.5), T(4), T(1)));
  }
}

TEST_CASE("SphereAoSoA: pack rejects a negative radius and a count out of range", "[SphereAoSoA][death]")
{
  using T = double;

  const std::vector<SphereCloud::Sphere<T>> spheres = {{Vec3T<T>(T(0), T(0), T(0)), T(1)},
                                                       {Vec3T<T>(T(1), T(0), T(0)), T(-0.5)}};
  const std::vector<uint32_t>               ids     = {0, 1};

  REQUIRE(abortsWith(
    [&spheres, &ids] {
      SphereAoSoA<T, 4> group;

      group.pack(spheres.data(), ids.data(), 2);
    },
    "SphereAoSoA::pack: sphere 1 needs a finite center and a finite radius of at least 0"));

  REQUIRE(abortsWith(
    [&spheres, &ids] {
      SphereAoSoA<T, 4> group;

      group.pack(spheres.data(), ids.data(), 5);
    },
    "SphereAoSoA::pack: the sphere count must be between 1 and 4 (5)"));
}

// ─────────────────────────────────────────────────────────────────────────────
// SphereCloudBVH: queries against brute force
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("SphereCloudBVH: the signed distance and the closest sphere match brute force",
                   "[SphereCloudBVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  static_assert(isShape<SphereCloudBVH<T>>, "SphereCloudBVH is a shape");

  constexpr std::size_t n = 2000;

  const std::vector<SphereCloud::Sphere<T>> spheres = makeSpheres<T>(n, 20261104u);
  const std::vector<Vec3T<T>>               queries = makeQueries<T>(500, 17u);

  Pool pool(hostMemoryResource());

  const SphereCloudBVH<T> cloud(pool, spheres);

  REQUIRE(cloud.numSpheres() == n);

  SECTION("signedDistance is the smallest |p - c| - r over the spheres")
  {
    std::size_t inside = 0;

    for (const auto& query : queries) {
      const T truth = bruteForceDistance(spheres, query);

      CHECK_THAT(cloud.signedDistance(query), withinAbsT<T>(truth, tightMargin<T>()));

      inside += (truth < T(0)) ? 1 : 0;
    }

    // Some queries are inside a sphere, so the negative side is covered.
    CHECK(inside > 0);
  }

  SECTION("closestSphere names a sphere at that distance")
  {
    for (const auto& query : queries) {
      const auto hit   = cloud.closestSphere(query);
      const T    truth = bruteForceDistance(spheres, query);

      REQUIRE(hit.valid());
      REQUIRE(hit.index < n);
      CHECK_THAT(hit.distance, withinAbsT<T>(truth, tightMargin<T>()));
      CHECK_THAT(sphereDistance(spheres[hit.index], query), withinAbsT<T>(truth, tightMargin<T>()));
    }
  }

  SECTION("at a sphere's center, the distance is minus its radius, unless a bigger sphere holds it deeper")
  {
    for (uint32_t i = 0; i < n; i += 97) {
      CHECK_THAT(cloud.signedDistance(spheres[i].m_center),
                 withinAbsT<T>(bruteForceDistance(spheres, spheres[i].m_center), tightMargin<T>()));
      CHECK(cloud.signedDistance(spheres[i].m_center) <= -spheres[i].m_radius);
    }
  }

  SECTION("the box is the box of every sphere grown by its radius")
  {
    Vec3T<T> lo = +Vec3T<T>::max();
    Vec3T<T> hi = -Vec3T<T>::max();

    for (const auto& sphere : spheres) {
      const Vec3T<T> radius = sphere.m_radius * Vec3T<T>::ones();

      lo = min(lo, sphere.m_center - radius);
      hi = max(hi, sphere.m_center + radius);
    }

    const auto box = cloud.computeBoundingVolume();

    CHECK(box.getLowCorner() == lo);
    CHECK(box.getHighCorner() == hi);
  }

  SECTION("sphere() returns each sphere by its cloud index")
  {
    for (uint32_t i = 0; i < n; i += 131) {
      CHECK(cloud.sphere(i).m_center == spheres[i].m_center);
      CHECK(cloud.sphere(i).m_radius == spheres[i].m_radius);
    }
  }
}

TEMPLATE_TEST_CASE("SphereCloudBVH: centers and radii kept apart build the same tree as one array of spheres",
                   "[SphereCloudBVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const std::vector<SphereCloud::Sphere<T>> spheres = makeSpheres<T>(700, 5u);

  std::vector<Vec3T<T>> centers;
  std::vector<T>        radii;

  for (const auto& sphere : spheres) {
    centers.push_back(sphere.m_center);
    radii.push_back(sphere.m_radius);
  }

  Pool pool(hostMemoryResource());

  const SphereCloudBVH<T> fromSpheres(pool, spheres);
  const SphereCloudBVH<T> fromVectors(pool, centers, radii);

  using Source = SphereCloud::FromCentersAndRadii<T, PointCloud::FromArray<T>, SphereCloud::FromRadii<T>>;

  const SphereCloudBVH<T> fromSource = SphereCloudBVH<T>::build(
    pool, Source{PointCloud::FromArray<T>{centers.data()}, SphereCloud::FromRadii<T>{radii.data()}}, 700);

  requireSameTree(fromVectors, fromSpheres);
  requireSameTree(fromSource, fromSpheres);
}

TEMPLATE_TEST_CASE("SphereCloudBVH: equal spheres tie to the lower index, a radius of 0 is a point, an empty cloud "
                   "misses",
                   "[SphereCloudBVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  SECTION("of two equal spheres, the one with the lower index is the closest")
  {
    std::vector<SphereCloud::Sphere<T>> spheres = makeSpheres<T>(300, 9u);

    spheres[250] = spheres[40];

    const SphereCloudBVH<T> cloud(pool, spheres, 4);

    CHECK(cloud.closestSphere(spheres[40].m_center).index == 40);
    CHECK(cloud.closestSphere(spheres[40].m_center + Vec3T<T>(T(0.001), T(0), T(0))).index == 40);
  }

  SECTION("a tie across leaves goes to the lower index, also when its leaf is visited last")
  {
    // Eight spheres at the corners of a cube around the query point, all at the same distance from it (each
    // coordinate differs by exactly 1/4). The keys tie too, so the leaves are visited in Morton order, and index 0
    // is the corner Morton order puts last.
    const Vec3T<T> query(T(0.5), T(0.5), T(0.5));

    std::vector<SphereCloud::Sphere<T>> spheres(8);

    for (int corner = 0; corner < 8; corner++) {
      const T x = (corner & 4) ? T(0.25) : T(0.75);
      const T y = (corner & 2) ? T(0.25) : T(0.75);
      const T z = (corner & 1) ? T(0.25) : T(0.75);

      spheres[static_cast<std::size_t>(corner)] = {Vec3T<T>(x, y, z), T(0.1)};
    }

    const SphereCloudBVH<T> cloud(pool, spheres, 1);

    CHECK(cloud.closestSphere(query).index == 0);
  }

  SECTION("of many equal spheres in many leaves, the lowest index is the closest")
  {
    const std::vector<SphereCloud::Sphere<T>> spheres(
      64, SphereCloud::Sphere<T>{Vec3T<T>(T(0.3), T(0.6), T(0.2)), T(0.05)});

    const SphereCloudBVH<T> cloud(pool, spheres, 4);

    CHECK(cloud.closestSphere(spheres[0].m_center).index == 0);
    CHECK(cloud.closestSphere(spheres[0].m_center + Vec3T<T>(T(0.01), T(0), T(0))).index == 0);
  }

  SECTION("spheres of radius 0 give the distance to the closest point")
  {
    std::vector<SphereCloud::Sphere<T>> spheres = makeSpheres<T>(400, 3u);
    std::vector<Vec3T<T>>               points;

    for (auto& sphere : spheres) {
      sphere.m_radius = T(0);
      points.push_back(sphere.m_center);
    }

    const SphereCloudBVH<T> cloud(pool, spheres);
    const PointCloudBVH<T>  pointCloud(pool, points);

    for (const auto& query : makeQueries<T>(100, 4u)) {
      CHECK_THAT(cloud.signedDistance(query),
                 withinAbsT<T>(std::sqrt(pointCloud.closestPoint(query).distanceSquared), tightMargin<T>()));
    }
  }

  SECTION("an empty cloud returns a miss, and the largest distance")
  {
    const SphereCloudBVH<T> cloud(pool, std::vector<SphereCloud::Sphere<T>>{});

    CHECK(cloud.numSpheres() == 0);
    CHECK_FALSE(cloud.closestSphere(Vec3T<T>(T(0), T(0), T(0))).valid());
    CHECK(cloud.signedDistance(Vec3T<T>(T(0), T(0), T(0))) == Math::Limits<T>::max());
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// forEachContact(): contacts between spheres
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("SphereCloudBVH: forEachContact finds every sphere within the gap, once, as brute force does",
                   "[SphereCloudBVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr std::size_t n = 600;

  const std::vector<SphereCloud::Sphere<T>> spheres = makeSpheres<T>(n, 77u);

  Pool pool(hostMemoryResource());

  const SphereCloudBVH<T> cloud(pool, spheres, 8);

  // Pairs whose separation is within the margin of the gap may go either way with rounding; every other pair
  // must be found exactly when its separation is below the gap.
  const T margin = looseMargin<T>();

  for (const T gap : {T(0), T(0.02), T(-0.01)}) {
    std::size_t found = 0;

    for (uint32_t i = 0; i < n; i++) {
      std::vector<SphereCloud::Hit<T>> contacts = contactsOf(cloud, i, gap);
      std::vector<bool>                seen(n, false);

      for (const auto& contact : contacts) {
        REQUIRE(contact.index < n);
        REQUIRE(contact.index != i);
        REQUIRE_FALSE(seen[contact.index]);

        seen[contact.index] = true;

        const T truth = separation(spheres[i], spheres[contact.index]);

        CHECK_THAT(contact.distance, withinAbsT<T>(truth, looseMargin<T>()));
        CHECK(contact.distance < gap);
      }

      for (uint32_t j = 0; j < n; j++) {
        if (j != i && separation(spheres[i], spheres[j]) < gap - margin) {
          CHECK(seen[j]);
        }
      }

      found += contacts.size();
    }

    // Each gap finds some contacts, so the comparison is not empty.
    CHECK(found > 0);
  }
}

TEMPLATE_TEST_CASE("SphereCloudBVH: contacts are mutual, with the same separation from either side",
                   "[SphereCloudBVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr std::size_t n = 500;

  const std::vector<SphereCloud::Sphere<T>> spheres = makeSpheres<T>(n, 78u);

  Pool pool(hostMemoryResource());

  const SphereCloudBVH<T> cloud(pool, spheres, 4);

  const T gap = T(0.01);

  std::vector<std::vector<SphereCloud::Hit<T>>> contacts(n);

  for (uint32_t i = 0; i < n; i++) {
    contacts[i] = contactsOf(cloud, i, gap);
  }

  for (uint32_t i = 0; i < n; i++) {
    for (const auto& contact : contacts[i]) {
      std::size_t back = 0;

      for (const auto& other : contacts[contact.index]) {
        if (other.index == i) {
          back++;

          CHECK(other.distance == contact.distance);
        }
      }

      CHECK(back == 1);
    }
  }
}

TEMPLATE_TEST_CASE("SphereCloudBVH: contacts stay mutual when the radii span orders of magnitude",
                   "[SphereCloudBVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr std::size_t n = 600;

  // Centers in [0, 10]^3, radii from 0.001 to 1 spread evenly over their logarithm: a few large spheres among many
  // small ones, where the boxes' rounding scales with the large radii.
  std::mt19937                      rng(83u);
  std::uniform_real_distribution<T> unit(T(0), T(1));

  std::vector<SphereCloud::Sphere<T>> spheres(n);

  for (auto& sphere : spheres) {
    sphere.m_center = T(10) * Vec3T<T>(unit(rng), unit(rng), unit(rng));
    sphere.m_radius = std::pow(T(10), T(-3) + T(3) * unit(rng));
  }

  Pool pool(hostMemoryResource());

  const SphereCloudBVH<T> cloud(pool, spheres, 4);

  for (const T gap : {T(0), T(0.05)}) {
    std::vector<std::vector<SphereCloud::Hit<T>>> contacts(n);
    std::size_t                                   found = 0;

    for (uint32_t i = 0; i < n; i++) {
      contacts[i] = contactsOf(cloud, i, gap);
      found += contacts[i].size();
    }

    CHECK(found > 0);

    // Some contact joins a large sphere to a much smaller one, where the rounding the pad covers is largest.
    bool unequal = false;

    for (uint32_t i = 0; i < n; i++) {
      for (const auto& contact : contacts[i]) {
        unequal = unequal || spheres[i].m_radius > T(100) * spheres[contact.index].m_radius;
      }
    }

    CHECK(unequal);

    for (uint32_t i = 0; i < n; i++) {
      for (const auto& contact : contacts[i]) {
        std::size_t back = 0;

        for (const auto& other : contacts[contact.index]) {
          if (other.index == i) {
            back++;

            CHECK(other.distance == contact.distance);
          }
        }

        CHECK(back == 1);
      }

      for (uint32_t j = 0; j < n; j++) {
        if (j != i && separation(spheres[i], spheres[j]) < gap - static_cast<T>(looseMargin<T>())) {
          bool seen = false;

          for (const auto& contact : contacts[i]) {
            seen = seen || contact.index == j;
          }

          CHECK(seen);
        }
      }
    }
  }
}

TEMPLATE_TEST_CASE("SphereCloudBVH: forEachContact of a sphere outside the cloud, with an index to skip",
                   "[SphereCloudBVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr std::size_t n = 400;

  const std::vector<SphereCloud::Sphere<T>> spheres = makeSpheres<T>(n, 79u);

  Pool pool(hostMemoryResource());

  const SphereCloudBVH<T> cloud(pool, spheres);

  const T margin = looseMargin<T>();

  SECTION("a probe sphere's contacts match brute force")
  {
    for (const auto& center : makeQueries<T>(50, 80u)) {
      const SphereCloud::Sphere<T> probe{center, T(0.03)};

      std::vector<bool> seen(n, false);

      cloud.forEachContact(probe, T(0), [&](const SphereCloud::Hit<T>& a_hit) {
        REQUIRE_FALSE(seen[a_hit.index]);

        seen[a_hit.index] = true;

        CHECK_THAT(a_hit.distance, withinAbsT<T>(separation(probe, spheres[a_hit.index]), looseMargin<T>()));
      });

      for (uint32_t j = 0; j < n; j++) {
        if (separation(probe, spheres[j]) < -margin) {
          CHECK(seen[j]);
        }
      }
    }
  }

  SECTION("the sphere itself, passed as a probe and skipped by index, gives the index form's contacts")
  {
    for (uint32_t i = 0; i < n; i += 37) {
      std::vector<SphereCloud::Hit<T>> asProbe;

      cloud.forEachContact(
        spheres[i], T(0.02), [&asProbe](const SphereCloud::Hit<T>& a_hit) { asProbe.push_back(a_hit); }, i);

      const std::vector<SphereCloud::Hit<T>> byIndex = contactsOf(cloud, i, T(0.02));

      REQUIRE(asProbe.size() == byIndex.size());

      for (std::size_t k = 0; k < asProbe.size(); k++) {
        CHECK(asProbe[k].index == byIndex[k].index);
        CHECK(asProbe[k].distance == byIndex[k].distance);
      }
    }
  }

  SECTION("without the index to skip, a sphere of the cloud is its own contact, at minus twice its radius")
  {
    bool self = false;

    cloud.forEachContact(spheres[5], T(0), [&self, &spheres](const SphereCloud::Hit<T>& a_hit) {
      if (a_hit.index == 5) {
        self = true;

        CHECK_THAT(a_hit.distance, withinAbsT<T>(T(-2) * spheres[5].m_radius, tightMargin<T>()));
      }
    });

    CHECK(self);
  }
}

TEMPLATE_TEST_CASE("SphereCloudBVH: forEachContact in a cloud of one sphere, of touching spheres, and an empty one",
                   "[SphereCloudBVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  SECTION("one sphere has no contacts")
  {
    const SphereCloudBVH<T> cloud(pool, std::vector<SphereCloud::Sphere<T>>{{Vec3T<T>(T(0), T(0), T(0)), T(1)}});

    CHECK(contactsOf(cloud, 0, T(10)).empty());
  }

  SECTION("touching spheres are a contact with a gap above 0, and not with a gap of 0")
  {
    // Exact in both precisions: the centers 3 apart, the radii 1 and 2.
    const SphereCloudBVH<T> cloud(
      pool,
      std::vector<SphereCloud::Sphere<T>>{{Vec3T<T>(T(0), T(0), T(0)), T(1)}, {Vec3T<T>(T(3), T(0), T(0)), T(2)}});

    CHECK(contactsOf(cloud, 0, T(0)).empty());

    const auto contacts = contactsOf(cloud, 0, T(0.5));

    REQUIRE(contacts.size() == 1);
    CHECK(contacts[0].index == 1);
    CHECK(contacts[0].distance == T(0));
  }

  SECTION("an empty cloud visits nothing")
  {
    const SphereCloudBVH<T> cloud(pool, std::vector<SphereCloud::Sphere<T>>{});

    bool visited = false;

    cloud.forEachContact(
      SphereCloud::Sphere<T>{Vec3T<T>(T(0), T(0), T(0)), T(1)}, T(1), [&visited](const auto&) { visited = true; });

    CHECK_FALSE(visited);
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// rebuild(), and the views
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("SphereCloudBVH: a deep copy and a rebased view answer like the original",
                   "[SphereCloudBVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const std::vector<SphereCloud::Sphere<T>> spheres = makeSpheres<T>(500, 8u);

  Pool pool(hostMemoryResource());

  const SphereCloudBVH<T> cloud(pool, spheres);

  pool.freeze();

  Pool                    copyPool(hostMemoryResource());
  const SphereCloudBVH<T> copy = cloud.deepCopy(copyPool);

  Pool                    mirror = Pool::mirror(pool, hostMemoryResource());
  const SphereCloudBVH<T> view   = cloud.rebasedView(mirror);

  REQUIRE(copy.isAttachedTo(copyPool));
  REQUIRE(view.base() == mirror.base());

  requireSameTree(copy, cloud);
  requireSameTree(view, cloud);
}

// ─────────────────────────────────────────────────────────────────────────────
// Device: queries in a kernel, and the build in device memory
// ─────────────────────────────────────────────────────────────────────────────

// ─────────────────────────────────────────────────────────────────────────────
// What aborts
// ─────────────────────────────────────────────────────────────────────────────

// ─────────────────────────────────────────────────────────────────────────────
// Rays, the layout, and the cloud in a tape
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("SphereCloudBVH: firstHit crosses the union of the spheres where brute force does, from outside "
                   "and from inside",
                   "[SphereCloudBVH][Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T     = TestType;
  using Cloud = SphereCloudBVH<T>;

  static_assert(hasRayForm<Cloud, T>, "SphereCloudBVH must be ray traced, not marched");
  static_assert(hasRaySpans<Cloud, T>, "SphereCloudBVH must give its spans to compositions");

  Pool pool(hostMemoryResource());

  const std::vector<SphereCloud::Sphere<T>> spheres = makeSpheres<T>(1500, 31u);
  const Cloud                               cloud(pool, spheres);

  std::size_t hits      = 0;
  std::size_t leaving   = 0;
  std::size_t ambiguous = 0;

  for (const RayT<T>& ray : makeRays(spheres, 600, 32u)) {
    bool       unclear = false;
    const auto spans   = bruteForceSpans(spheres, ray, unclear);

    if (unclear) {
      ambiguous++;

      continue;
    }

    const auto [first, entering] = bruteForceFirstCrossing(spans, T(0), T(10));

    const HitT<T>      hit      = firstHit(cloud, ray, T(0), T(10));
    const CrossingT<T> crossing = firstCrossing(cloud, ray, T(0), T(10));

    if (!(first < std::numeric_limits<T>::infinity())) {
      CHECK_FALSE(hit.hit());
      CHECK_FALSE(crossing.hit());

      continue;
    }

    hits++;
    leaving += entering ? 0U : 1U;

    REQUIRE(hit.hit());
    REQUIRE(hit.m_primitive < spheres.size());

    CHECK(hit.m_exact);
    CHECK(hit.m_entering == entering);
    CHECK_THAT(hit.m_t, withinAbsT<T>(first, looseMargin<T>()));

    // The crossing lies on the surface of the sphere it names, and the normal is that sphere's, outward.
    const SphereCloud::Sphere<T>& sphere = spheres[hit.m_primitive];
    const Vec3T<T>                normal = (hit.m_point - sphere.m_center) / sphere.m_radius;

    CHECK_THAT(sphereDistance(sphere, hit.m_point), withinAbsT<T>(T(0), looseMargin<T>()));

    for (std::size_t d = 0; d < 3; d++) {
      CHECK_THAT(hit.m_normal[d], withinAbsT<T>(normal[d], looseMargin<T>()));
    }

    // firstCrossing() is the same trace, without the sphere.
    REQUIRE(crossing.hit());
    CHECK(crossing.m_t == hit.m_t);
  }

  // Enough rays decided, hitting, and leaving from inside, for the comparison to mean something.
  CHECK(ambiguous < 30U);
  CHECK(hits > 300U);
  CHECK(leaving > 200U);
}

TEMPLATE_TEST_CASE("SphereCloudBVH: spans() lists where a ray is inside the union, over an interval, as brute force "
                   "does",
                   "[SphereCloudBVH][Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T     = TestType;
  using Cloud = SphereCloudBVH<T>;

  Pool pool(hostMemoryResource());

  const std::vector<SphereCloud::Sphere<T>> spheres = makeSpheres<T>(1500, 33u);
  const Cloud                               cloud(pool, spheres);

  const T tMin = T(0.3);
  const T tMax = T(1.6);

  std::size_t checked = 0;

  for (const RayT<T>& ray : makeRays(spheres, 300, 34u)) {
    bool       unclear = false;
    const auto spans   = bruteForceSpans(spheres, ray, unclear);

    if (unclear) {
      continue;
    }

    RaySpansT<T> list;

    RayForm<Cloud>::spans(cloud, ray, tMin, tMax, RayOptions<T>{}, false, list);

    // The brute-force spans in the interval, up to what the list is complete up to.
    std::vector<std::pair<T, T>> expected;

    for (const auto& span : spans) {
      if (span.second >= tMin && span.first <= tMax && span.first <= list.m_horizon) {
        expected.emplace_back(std::max(span.first, tMin), std::min(span.second, tMax));
      }
    }

    // A full list drops spans beyond its horizon.
    REQUIRE(list.m_count <= expected.size());
    REQUIRE(list.m_count + 1U >= expected.size());

    for (uint32_t i = 0; i < list.m_count; i++) {
      const auto& span = list.m_spans[i];

      CHECK_THAT(span.m_in.m_t, withinAbsT<T>(expected[i].first, looseMargin<T>()));
      CHECK_THAT(span.m_out.m_t, withinAbsT<T>(expected[i].second, looseMargin<T>()));

      // An end inside the interval is a crossing, named by a sphere whose surface it lies on.
      CHECK(span.m_in.crossing() == (expected[i].first > tMin));
      CHECK(span.m_out.crossing() == (expected[i].second < tMax));

      const auto onItsSphere = [&spheres, &ray](const SpanEndT<T>& a_end) {
        if (a_end.crossing()) {
          REQUIRE(a_end.m_leaf < spheres.size());
          CHECK_THAT(sphereDistance(spheres[a_end.m_leaf], ray.at(a_end.m_t)), withinAbsT<T>(T(0), looseMargin<T>()));
        }
      };

      onItsSphere(span.m_in);
      onItsSphere(span.m_out);
    }

    checked += list.m_count;
  }

  CHECK(checked > 200U);
}

TEMPLATE_TEST_CASE("SphereCloudBVH: one sphere is crossed where a SphereSDF is, a radius of 0 only touched, and an "
                   "empty cloud never",
                   "[SphereCloudBVH][Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T     = TestType;
  using Vec3  = Vec3T<T>;
  using Cloud = SphereCloudBVH<T>;

  Pool pool(hostMemoryResource());

  const Vec3 center(T(0.2), T(-0.1), T(0.3));
  const T    radius = T(0.7);

  const Cloud        cloud(pool, std::vector<SphereCloud::Sphere<T>>{{center, radius}});
  const SphereSDF<T> sphere(center, radius);

  SECTION("the same crossings, entering and leaving, with the same normals")
  {
    for (const RayT<T>& ray : makeRays(std::vector<SphereCloud::Sphere<T>>{{center, radius}}, 50, 35u)) {
      const HitT<T> got  = firstHit(cloud, ray, T(0), T(10));
      const HitT<T> want = firstHit(sphere, ray, T(0), T(10));

      REQUIRE(got.hit() == want.hit());

      // The same arithmetic, to rounding: an optimized build may fuse multiply-adds differently in the two.
      if (want.hit()) {
        CHECK_THAT(got.m_t, withinAbsT<T>(want.m_t, tightMargin<T>()));
        CHECK(got.m_entering == want.m_entering);
        CHECK(got.m_primitive == 0U);

        for (std::size_t d = 0; d < 3; d++) {
          CHECK_THAT(got.m_normal[d], withinAbsT<T>(want.m_normal[d], tightMargin<T>()));
        }
      }
    }
  }

  SECTION("a crossing outside the interval is no crossing")
  {
    const RayT<T> ray{Vec3(T(-3), T(-0.1), T(0.3)), Vec3(T(1), T(0), T(0))};

    CHECK(firstHit(cloud, ray, T(0), T(10)).hit());
    CHECK_FALSE(firstHit(cloud, ray, T(0), T(2)).hit());
    CHECK_FALSE(firstHit(cloud, ray, T(5), T(10)).hit());
    CHECK_FALSE(firstHit(cloud, ray, T(2), T(1)).hit());
  }

  SECTION("a sphere of radius 0 is touched by a ray through its center, and missed by every other")
  {
    const Cloud point(pool, std::vector<SphereCloud::Sphere<T>>{{Vec3(T(1), T(0), T(0)), T(0)}});

    const HitT<T> through = firstHit(point, RayT<T>{Vec3::zeros(), Vec3(T(1), T(0), T(0))}, T(0), T(10));

    REQUIRE(through.hit());
    CHECK_THAT(through.m_t, withinAbsT<T>(T(1), tightMargin<T>()));
    CHECK_FALSE(firstHit(point, RayT<T>{Vec3(T(0), T(0.01), T(0)), Vec3(T(1), T(0), T(0))}, T(0), T(10)).hit());
  }

  SECTION("an empty cloud is never crossed")
  {
    const Cloud empty(pool, std::vector<SphereCloud::Sphere<T>>{});

    CHECK_FALSE(firstHit(empty, RayT<T>{Vec3::zeros(), Vec3(T(1), T(0), T(0))}, T(0), T(10)).hit());
  }
}

TEMPLATE_TEST_CASE("SphereCloudBVH: a cloud made from its layout answers like the original, and has no room for a "
                   "rebuild",
                   "[SphereCloudBVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T     = TestType;
  using Cloud = SphereCloudBVH<T>;

  Pool pool(hostMemoryResource());

  const std::vector<SphereCloud::Sphere<T>> spheres = makeSpheres<T>(700, 36u);
  const Cloud cloud = Cloud::build(pool, spheres, 16 * PointSoA::DefaultWidth<T>(), uint32_t(spheres.size()));
  const Cloud fromLayout(cloud.layout(), cloud.location());

  CHECK(cloud.maxSpheres() == spheres.size());
  CHECK(fromLayout.maxSpheres() == 0U);
  REQUIRE(fromLayout.numSpheres() == cloud.numSpheres());

  for (const Vec3T<T>& point : makeQueries<T>(200, 37u)) {
    CHECK(fromLayout.signedDistance(point) == cloud.signedDistance(point));
    CHECK(fromLayout.closestSphere(point).index == cloud.closestSphere(point).index);
  }

  for (uint32_t i = 0; i < 50; i++) {
    CHECK(contactsOf(fromLayout, i, T(0.01)).size() == contactsOf(cloud, i, T(0.01)).size());
  }

  for (const RayT<T>& ray : makeRays(spheres, 100, 38u)) {
    CHECK(firstHit(fromLayout, ray, T(0), T(10)).m_t == firstHit(cloud, ray, T(0), T(10)).m_t);
  }

  // A deep copy of it is whole: what a tape's deepCopy() makes of a cloud it holds.
  Pool        other(hostMemoryResource());
  const Cloud copy = fromLayout.deepCopy(other);

  for (const Vec3T<T>& point : makeQueries<T>(50, 39u)) {
    CHECK(copy.signedDistance(point) == cloud.signedDistance(point));
  }
}

TEMPLATE_TEST_CASE("SphereCloudBVH: a tape holding a cloud evaluates it, traces it exactly, and copies it",
                   "[SphereCloudBVH][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T     = TestType;
  using Vec3  = Vec3T<T>;
  using Cloud = SphereCloudBVH<T>;

  Pool pool(hostMemoryResource());

  const std::vector<SphereCloud::Sphere<T>> spheres = makeSpheres<T>(500, 40u);
  const Cloud                               cloud(pool, spheres);

  // A box with the particles cut out of it, and the particles beside a sphere far away.
  const BoxSDF<T>    box(Vec3(T(-0.1), T(-0.1), T(-0.1)), Vec3(T(1.1), T(1.1), T(1.1)));
  const SphereSDF<T> distant(Vec3(T(5), T(5), T(5)), T(0.5));

  const Tape<T, Cloud> cut(pool, Difference(Expr<T>(box), Expr<T>(cloud)));
  const Tape<T, Cloud> beside(pool, Union(Expr<T>(cloud), Expr<T>(distant)));

  for (const Vec3& point : makeQueries<T>(300, 41u)) {
    CHECK_THAT(
      cut.signedDistance(point),
      withinAbsT<T>(std::max(box.signedDistance(point), -bruteForceDistance(spheres, point)), tightMargin<T>()));
  }

  for (const RayT<T>& ray : makeRays(spheres, 200, 42u)) {
    const HitT<T> got  = firstHit(beside, ray, T(0), T(10));
    const HitT<T> want = firstHit(cloud, ray, T(0), T(10));

    REQUIRE(got.hit() == want.hit());

    if (want.hit()) {
      CHECK(got.m_exact);
      CHECK_THAT(got.m_t, withinAbsT<T>(want.m_t, tightMargin<T>()));
    }
  }

  Pool                 other(hostMemoryResource());
  const Tape<T, Cloud> copy = cut.deepCopy(other);

  for (const Vec3& point : makeQueries<T>(50, 43u)) {
    CHECK(copy.signedDistance(point) == cut.signedDistance(point));
  }
}

TEST_CASE("SphereCloudBVH: rejects a sphere it cannot build over, mismatched centers and radii, and a rebuild "
          "without room",
          "[SphereCloudBVH][death]")
{
  using T = double;

  // EBGEOMETRY_REQUIREs, so they abort in every build.
  const std::vector<SphereCloud::Sphere<T>> good = {{Vec3T<T>(T(0), T(0), T(0)), T(1)},
                                                    {Vec3T<T>(T(1), T(0), T(0)), T(0.5)}};

  const std::vector<SphereCloud::Sphere<T>> negative = {{Vec3T<T>(T(0), T(0), T(0)), T(1)},
                                                        {Vec3T<T>(T(1), T(0), T(0)), T(-0.5)}};

  const std::vector<SphereCloud::Sphere<T>> notFinite = {
    {Vec3T<T>(T(0), T(0), T(0)), T(1)}, {Vec3T<T>(T(1), std::numeric_limits<T>::quiet_NaN(), T(0)), T(0.5)}};

  REQUIRE(abortsWith(
    [&negative] {
      Pool                    pool(hostMemoryResource());
      const SphereCloudBVH<T> cloud(pool, negative);
    },
    "SphereCloudBVH::build: sphere 1 of 2 has a non-finite or negative radius"));

  REQUIRE(abortsWith(
    [&notFinite] {
      Pool                    pool(hostMemoryResource());
      const SphereCloudBVH<T> cloud(pool, notFinite);
    },
    "SphereCloudBVH::build: sphere 1 of 2 has a non-finite coordinate"));

  REQUIRE(abortsWith(
    [] {
      Pool                    pool(hostMemoryResource());
      const SphereCloudBVH<T> cloud(pool, std::vector<Vec3T<T>>(3), std::vector<T>(2));
    },
    "SphereCloudBVH::build: 3 centers but 2 radii"));

  REQUIRE(abortsWith(
    [&good] {
      Pool              pool(hostMemoryResource());
      SphereCloudBVH<T> cloud(pool, good);

      cloud.rebuild(SphereCloud::FromArray<T>{good.data()}, 2U);
    },
    "SphereCloudBVH::rebuild: the BVH has no room for a rebuild (see build()'s a_maxSpheres)"));

  REQUIRE(abortsWith(
    [&good] {
      Pool              pool(hostMemoryResource());
      SphereCloudBVH<T> cloud = SphereCloudBVH<T>::build(pool, good, 4, 2);

      cloud.rebuild(SphereCloud::FromArray<T>{good.data()}, 3U);
    },
    "SphereCloudBVH::rebuild: 3 spheres, but the BVH has room for 2 (see build()'s a_maxSpheres)"));

  REQUIRE(abortsWith(
    [&good, &negative] {
      Pool              pool(hostMemoryResource());
      SphereCloudBVH<T> cloud = SphereCloudBVH<T>::build(pool, good, 4, 2);

      cloud.rebuild(SphereCloud::FromArray<T>{negative.data()}, 2U);
    },
    "SphereCloudBVH::rebuild: sphere 1 of 2 has a non-finite or negative radius"));

  // Through GPU::parallelFor() (emulated device memory, in a host build, where the launches can read the host
  // array), a bad sphere is found from the boxes, without its index. A CUDA context does not survive the fork.
  if (!GPU::enabled) {
    REQUIRE(abortsWith(
      [&negative] {
        Pool                    pool(emulatedDeviceMemoryResource());
        const SphereCloudBVH<T> cloud = SphereCloudBVH<T>::build(pool, SphereCloud::FromArray<T>{negative.data()}, 2U);

        (void)cloud;
      },
      "SphereCloudBVH::build: a sphere has a non-finite or negative radius"));

    REQUIRE(abortsWith(
      [&notFinite] {
        Pool                    pool(emulatedDeviceMemoryResource());
        const SphereCloudBVH<T> cloud = SphereCloudBVH<T>::build(pool, SphereCloud::FromArray<T>{notFinite.data()}, 2U);

        (void)cloud;
      },
      "SphereCloudBVH::build: a sphere has a non-finite coordinate"));

    for (const T radius : {std::numeric_limits<T>::infinity(), std::numeric_limits<T>::quiet_NaN()}) {
      const std::vector<SphereCloud::Sphere<T>> badRadius = {{Vec3T<T>(T(0), T(0), T(0)), T(1)},
                                                             {Vec3T<T>(T(1), T(0), T(0)), radius}};

      REQUIRE(abortsWith(
        [&badRadius] {
          Pool                    pool(emulatedDeviceMemoryResource());
          const SphereCloudBVH<T> cloud =
            SphereCloudBVH<T>::build(pool, SphereCloud::FromArray<T>{badRadius.data()}, 2U);

          (void)cloud;
        },
        "SphereCloudBVH::build: a sphere has a non-finite or negative radius"));
    }
  }
}
