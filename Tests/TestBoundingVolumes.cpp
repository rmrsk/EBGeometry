// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using BoundingVolumes::AABBT;
using BoundingVolumes::BoundingSphereT;
using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

// ─────────────────────────────────────────────────────────────────────────────
// AABBT
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AABBT: construction from lo/hi corners", "[AABBT]")
{
  const Vec3T<double> lo(-1, -2, -3);
  const Vec3T<double> hi(1, 2, 3);
  AABBT<double>       box(lo, hi);

  REQUIRE(box.getLowCorner()[0] == -1.0);
  REQUIRE(box.getHighCorner()[2] == 3.0);

  auto c = box.getCentroid();
  REQUIRE(c[0] == 0.0);
  REQUIRE(c[1] == 0.0);
  REQUIRE(c[2] == 0.0);
}

TEST_CASE("AABBT: construction from point cloud", "[AABBT]")
{
  const std::vector<Vec3T<double>> pts = {{0, 0, 0}, {3, 0, 0}, {0, 4, 0}, {0, 0, 5}};
  AABBT<double>                    box(pts);

  REQUIRE(box.getLowCorner()[0] == 0.0);
  REQUIRE(box.getHighCorner()[0] == 3.0);
  REQUIRE(box.getHighCorner()[1] == 4.0);
  REQUIRE(box.getHighCorner()[2] == 5.0);
}

TEST_CASE("AABBT: volume and surface area", "[AABBT]")
{
  const AABBT<double> unit(Vec3T<double>(0, 0, 0), Vec3T<double>(1, 1, 1));
  REQUIRE_THAT(unit.getVolume(), WithinRel(1.0));
  REQUIRE_THAT(unit.getArea(), WithinRel(6.0));

  const AABBT<double> rect(Vec3T<double>(0, 0, 0), Vec3T<double>(2, 3, 4));
  REQUIRE_THAT(rect.getVolume(), WithinRel(24.0));
}

TEST_CASE("AABBT: distance to point", "[AABBT]")
{
  const AABBT<double> unit(Vec3T<double>(0, 0, 0), Vec3T<double>(1, 1, 1));

  // Inside: distance should be 0
  REQUIRE_THAT(unit.getDistance(Vec3T<double>(0.5, 0.5, 0.5)), WithinAbs(0.0, 1e-12));

  // Outside along one axis
  REQUIRE_THAT(unit.getDistance(Vec3T<double>(3.0, 0.5, 0.5)), WithinRel(2.0));

  // Outside at a corner
  const Vec3T<double> corner_pt(2.0, 0.0, 0.0);
  REQUIRE_THAT(unit.getDistance(corner_pt), WithinRel(1.0));
}

TEST_CASE("AABBT: intersects", "[AABBT]")
{
  const AABBT<double> a(Vec3T<double>(0, 0, 0), Vec3T<double>(2, 2, 2));
  const AABBT<double> b(Vec3T<double>(1, 1, 1), Vec3T<double>(3, 3, 3));
  const AABBT<double> c(Vec3T<double>(5, 5, 5), Vec3T<double>(6, 6, 6));

  REQUIRE(a.intersects(b));
  REQUIRE(b.intersects(a));
  REQUIRE(!a.intersects(c));
  REQUIRE(!c.intersects(a));
}

TEST_CASE("AABBT: overlapping volume", "[AABBT]")
{
  const AABBT<double> a(Vec3T<double>(0, 0, 0), Vec3T<double>(2, 2, 2));
  const AABBT<double> b(Vec3T<double>(1, 1, 1), Vec3T<double>(3, 3, 3));

  // Overlap is a 1×1×1 cube
  REQUIRE_THAT(a.getOverlappingVolume(b), WithinRel(1.0));

  // No overlap
  const AABBT<double> c(Vec3T<double>(5, 5, 5), Vec3T<double>(6, 6, 6));
  REQUIRE_THAT(a.getOverlappingVolume(c), WithinAbs(0.0, 1e-12));

  // Identical boxes
  REQUIRE_THAT(a.getOverlappingVolume(a), WithinRel(8.0));
}

// ─────────────────────────────────────────────────────────────────────────────
// BoundingSphereT
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("BoundingSphereT: construction", "[BoundingSphereT]")
{
  const Vec3T<double>     c(1, 2, 3);
  BoundingSphereT<double> s(c, 5.0);

  REQUIRE(s.getRadius() == 5.0);
  REQUIRE(s.getCentroid()[0] == 1.0);
  REQUIRE(s.getCentroid()[1] == 2.0);
  REQUIRE(s.getCentroid()[2] == 3.0);
}

TEST_CASE("BoundingSphereT: construction from point cloud (Ritter)", "[BoundingSphereT]")
{
  const std::vector<Vec3T<double>> pts = {{-2, 0, 0}, {2, 0, 0}, {0, 2, 0}, {0, -2, 0}};
  BoundingSphereT<double>          s(pts); // uses Ritter algorithm by default
  // Ritter's algorithm is not guaranteed tight, but must enclose all points
  for (const auto& p : pts) {
    const double dist = (p - s.getCentroid()).length();
    REQUIRE(dist <= s.getRadius() + 1e-10);
  }
}

TEST_CASE("BoundingSphereT: intersects", "[BoundingSphereT]")
{
  const BoundingSphereT<double> a(Vec3T<double>(0, 0, 0), 1.0);
  const BoundingSphereT<double> b(Vec3T<double>(1.5, 0, 0), 1.0); // overlap
  const BoundingSphereT<double> c(Vec3T<double>(5, 0, 0), 1.0);   // no overlap

  REQUIRE(a.intersects(b));
  REQUIRE(!a.intersects(c));
}

TEST_CASE("BoundingSphereT: overlapping volume (concentric)", "[BoundingSphereT]")
{
  // Identical spheres: overlap = full volume = 4/3 π r³
  const BoundingSphereT<double> a(Vec3T<double>(0, 0, 0), 1.0);
  constexpr double              pi    = 3.14159265358979323846;
  const double                  exact = (4.0 / 3.0) * pi;
  REQUIRE_THAT(a.getOverlappingVolume(a), WithinRel(exact, 1e-6));

  // Non-overlapping: volume = 0
  const BoundingSphereT<double> b(Vec3T<double>(10, 0, 0), 1.0);
  REQUIRE_THAT(a.getOverlappingVolume(b), WithinAbs(0.0, 1e-12));
}

TEST_CASE("BoundingSphereT: volume and area", "[BoundingSphereT]")
{
  constexpr double              pi = 3.14159265358979323846;
  const BoundingSphereT<double> s(Vec3T<double>(0, 0, 0), 2.0);

  REQUIRE_THAT(s.getVolume(), WithinRel((4.0 / 3.0) * pi * 8.0, 1e-6));
  REQUIRE_THAT(s.getArea(), WithinRel(4.0 * pi * 4.0, 1e-6));
}
