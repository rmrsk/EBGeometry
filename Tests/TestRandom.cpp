// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for EBGeometry_Random.hpp (Random::samplePoints).

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"

#include <catch2/catch_template_test_macros.hpp>

#include <cstddef>
#include <vector>

using namespace EBGeometry;

TEMPLATE_TEST_CASE("Random::samplePoints: count, unit-cube range, and reproducibility",
                   "[Random]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto points = Random::samplePoints<T>(1000, 12345ULL);

  REQUIRE(points.size() == 1000);

  for (const auto& p : points) {
    for (int i = 0; i < 3; i++) {
      REQUIRE(p[i] >= T(0));
      REQUIRE(p[i] < T(1));
    }
  }

  // Deterministic: the same seed reproduces the exact same points.
  const auto same = Random::samplePoints<T>(1000, 12345ULL);
  REQUIRE(same.size() == points.size());
  for (size_t i = 0; i < points.size(); i++) {
    REQUIRE(same[i][0] == points[i][0]);
    REQUIRE(same[i][1] == points[i][1]);
    REQUIRE(same[i][2] == points[i][2]);
  }

  // A different seed produces a different sequence (overwhelmingly likely for 1000 points).
  const auto other   = Random::samplePoints<T>(1000, 67890ULL);
  bool       differs = false;
  for (size_t i = 0; i < points.size() && !differs; i++) {
    differs = (other[i][0] != points[i][0]) || (other[i][1] != points[i][1]) || (other[i][2] != points[i][2]);
  }
  REQUIRE(differs);
}

TEST_CASE("Random::samplePoints: zero count yields an empty vector", "[Random]")
{
  REQUIRE(Random::samplePoints<double>(0, 1ULL).empty());
}
