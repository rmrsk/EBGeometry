// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <limits>
#include <random>
#include <vector>

using namespace EBGeometry;

// SFC::ValidSpan is uint64_t; cast to unsigned int for use in Index (fits: ValidBits=21).
static constexpr unsigned int kMaxCoord = static_cast<unsigned int>(SFC::ValidSpan);

// ─────────────────────────────────────────────────────────────────────────────
// Morton SFC
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("Morton SFC: encode → decode roundtrip", "[SFC][Morton]")
{
  const std::vector<SFC::Index> points = {
    {0, 0, 0},
    {1, 0, 0},
    {0, 1, 0},
    {0, 0, 1},
    {3, 5, 7},
    {100, 200, 300},
    {kMaxCoord, kMaxCoord, kMaxCoord}, // max valid coordinate
  };

  for (const auto& p : points) {
    const SFC::Code  code    = SFC::Morton::encode(p);
    const SFC::Index decoded = SFC::Morton::decode(code);
    REQUIRE(decoded[0] == p[0]);
    REQUIRE(decoded[1] == p[1]);
    REQUIRE(decoded[2] == p[2]);
  }
}

TEST_CASE("Morton SFC: encode is monotone along x-axis", "[SFC][Morton]")
{
  // Codes for (i,0,0) should be strictly increasing
  SFC::Code prev = SFC::Morton::encode({0, 0, 0});
  for (unsigned int i = 1; i <= 16; ++i) {
    const SFC::Code cur = SFC::Morton::encode({i, 0, 0});
    REQUIRE(cur > prev);
    prev = cur;
  }
}

TEST_CASE("Morton SFC: distinct points produce distinct codes", "[SFC][Morton]")
{
  const SFC::Index a = {1, 2, 3};
  const SFC::Index b = {3, 2, 1};
  REQUIRE(SFC::Morton::encode(a) != SFC::Morton::encode(b));
}

// ─────────────────────────────────────────────────────────────────────────────
// Nested SFC
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("Nested SFC: encode → decode roundtrip", "[SFC][Nested]")
{
  const std::vector<SFC::Index> points = {
    {0, 0, 0},
    {1, 0, 0},
    {0, 1, 0},
    {0, 0, 1},
    {10, 20, 30},
    {500, 700, 300},
    {kMaxCoord, kMaxCoord, kMaxCoord}, // max valid coordinate
  };

  for (const auto& p : points) {
    const SFC::Code  code    = SFC::Nested::encode(p);
    const SFC::Index decoded = SFC::Nested::decode(code);
    REQUIRE(decoded[0] == p[0]);
    REQUIRE(decoded[1] == p[1]);
    REQUIRE(decoded[2] == p[2]);
  }
}

TEST_CASE("Nested SFC: encode is injective for distinct indices", "[SFC][Nested]")
{
  const SFC::Index a = {1, 0, 0};
  const SFC::Index b = {0, 1, 0};
  const SFC::Index c = {0, 0, 1};

  REQUIRE(SFC::Nested::encode(a) != SFC::Nested::encode(b));
  REQUIRE(SFC::Nested::encode(b) != SFC::Nested::encode(c));
  REQUIRE(SFC::Nested::encode(a) != SFC::Nested::encode(c));
}

TEST_CASE("Nested SFC: ValidSpan boundary is handled correctly", "[SFC][Nested]")
{
  // A coordinate equal to ValidSpan used to decode incorrectly (base was ValidSpan
  // instead of ValidSpan+1). This test guards against regression.
  const SFC::Index boundary = {kMaxCoord, 0, 0};
  const SFC::Code  code     = SFC::Nested::encode(boundary);
  const SFC::Index decoded  = SFC::Nested::decode(code);

  REQUIRE(decoded[0] == SFC::ValidSpan);
  REQUIRE(decoded[1] == 0u);
  REQUIRE(decoded[2] == 0u);
}

TEST_CASE("Nested SFC: encode(0,0,0) == 0", "[SFC][Nested]")
{
  REQUIRE(SFC::Nested::encode({0, 0, 0}) == 0u);
}

// ─────────────────────────────────────────────────────────────────────────────
// Hilbert SFC
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("Hilbert SFC: encode → decode roundtrip", "[SFC][Hilbert]")
{
  std::vector<SFC::Index> points = {
    {0, 0, 0},
    {1, 0, 0},
    {0, 1, 0},
    {0, 0, 1},
    {3, 5, 7},
    {100, 200, 300},
    {kMaxCoord, 0, 0},
    {0, kMaxCoord, 0},
    {kMaxCoord, kMaxCoord, kMaxCoord}, // max valid coordinate
  };

  // A spread of pseudo-random interior points (deterministic; no <random> needed here).
  unsigned int state = 123457u;
  const auto   next  = [&state]() -> unsigned int {
    state = state * 1103515245u + 12345u;
    return (state >> 8) % 2000000u;
  };
  for (int i = 0; i < 2000; i++) {
    points.push_back({next(), next(), next()});
  }

  for (const auto& p : points) {
    const SFC::Code  code    = SFC::Hilbert::encode(p);
    const SFC::Index decoded = SFC::Hilbert::decode(code);
    REQUIRE(decoded[0] == p[0]);
    REQUIRE(decoded[1] == p[1]);
    REQUIRE(decoded[2] == p[2]);
  }
}

TEST_CASE("Hilbert SFC: consecutive codes map to adjacent cells (the defining locality property)", "[SFC][Hilbert]")
{
  // The distinguishing property of the Hilbert curve (vs Morton): stepping the code by 1 always
  // moves to a face-adjacent cell -- exactly one coordinate changes, and by exactly 1. This is what
  // gives it better spatial locality than Z-order.
  for (SFC::Code d = 0; d < 100000; d++) {
    const SFC::Index a = SFC::Hilbert::decode(d);
    const SFC::Index b = SFC::Hilbert::decode(d + 1);

    const long manhattan = std::labs(static_cast<long>(a[0]) - static_cast<long>(b[0])) +
                           std::labs(static_cast<long>(a[1]) - static_cast<long>(b[1])) +
                           std::labs(static_cast<long>(a[2]) - static_cast<long>(b[2]));

    REQUIRE(manhattan == 1);
  }
}

TEST_CASE("Hilbert SFC: distinct points produce distinct codes", "[SFC][Hilbert]")
{
  const SFC::Index a = {1, 2, 3};
  const SFC::Index b = {3, 2, 1};
  REQUIRE(SFC::Hilbert::encode(a) != SFC::Hilbert::encode(b));
}

TEST_CASE("Hilbert SFC: encode(0,0,0) == 0", "[SFC][Hilbert]")
{
  REQUIRE(SFC::Hilbert::encode({0, 0, 0}) == 0u);
}

// ─────────────────────────────────────────────────────────────────────────────
// computeBins / order
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("SFC::computeBins: points map into the valid integer grid", "[SFC][bins]")
{
  using Vec3 = Vec3T<double>;

  const std::vector<Vec3> points = {Vec3(0, 0, 0), Vec3(1, 1, 1), Vec3(0.5, 0.25, 0.75)};

  const auto bins = SFC::computeBins<double>(points);

  REQUIRE(bins.size() == points.size());
  REQUIRE(bins[0] == SFC::Index{0, 0, 0}); // the min corner lands at the grid origin
  for (const auto& b : bins) {
    for (int i = 0; i < 3; i++) {
      REQUIRE(b[i] <= kMaxCoord);
    }
  }
}

TEST_CASE("SFC::computeBins: coincident points collapse to bin 0 (no divide-by-zero)", "[SFC][bins]")
{
  using Vec3 = Vec3T<double>;

  const std::vector<Vec3> points(10, Vec3(0.3, 0.7, -0.1));

  for (const auto& b : SFC::computeBins<double>(points)) {
    REQUIRE(b == SFC::Index{0, 0, 0});
  }
}

TEMPLATE_TEST_CASE("SFC::computeBins: the grid cells are cubes, so a flat cloud's thin axis spans few bins",
                   "[SFC][bins]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // A 10 x 10 x 0.001 slab, with two opposite corners pinned so the extents are exact.
  std::mt19937                      rng(11);
  std::uniform_real_distribution<T> unit(T(0), T(1));

  std::vector<Vec3> points = {Vec3(T(0), T(0), T(0)), Vec3(T(10), T(10), T(0.001))};

  for (int i = 0; i < 500; i++) {
    points.emplace_back(T(10) * unit(rng), T(10) * unit(rng), T(0.001) * unit(rng));
  }

  const auto bins = SFC::computeBins<T>(points);

  REQUIRE(bins.size() == points.size());

  unsigned int maxBin[3] = {0, 0, 0};

  for (const auto& b : bins) {
    for (int dir = 0; dir < 3; dir++) {
      REQUIRE(b[dir] <= kMaxCoord);

      maxBin[dir] = std::max(maxBin[dir], b[dir]);
    }
  }

  // The long axes span the whole grid, give or take the rounding of the cell size.
  REQUIRE(maxBin[0] + 1 >= kMaxCoord);
  REQUIRE(maxBin[1] + 1 >= kMaxCoord);

  // The thin axis spans about a ten-thousandth of it (0.001 / 10 of ValidSpan, ~210 cells), not the
  // whole grid as it would with a separate scale per axis -- but more than one cell.
  REQUIRE(maxBin[2] > 0);
  REQUIRE(maxBin[2] <= kMaxCoord / 10000 + 1);
}

TEMPLATE_TEST_CASE("SFC::computeBins: coincident points all take bin 0, in either precision",
                   "[SFC][bins]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const std::vector<Vec3> points(25, Vec3(T(-3.5), T(1e6), T(0.125)));

  for (const auto& b : SFC::computeBins<T>(points)) {
    REQUIRE(b == SFC::Index{0, 0, 0});
  }
}

TEMPLATE_TEST_CASE("SFC::computeBins: an extent too small to divide into cells gives bin 0, not garbage",
                   "[SFC][bins]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // Below Limits<T>::min() * ValidSpan the cell size would not be a normal number (and could be
  // zero), so every point takes bin 0 on every axis. Checked for an extent just below that cutoff,
  // for the smallest subnormal separation, and away from the origin.
  const T belowCutoff = std::numeric_limits<T>::min() * T(1000);
  const T subnormal   = std::numeric_limits<T>::denorm_min();

  for (const T offset : {T(0), T(1e-30), -std::numeric_limits<T>::min()}) {
    for (const T gap : {belowCutoff, subnormal}) {
      const std::vector<Vec3> points = {Vec3(offset, offset, offset),
                                        Vec3(offset + gap, offset, offset),
                                        Vec3(offset, offset + gap, offset + gap),
                                        Vec3(offset + gap, offset + gap, offset + gap)};

      for (const auto& b : SFC::computeBins<T>(points)) {
        REQUIRE(b == SFC::Index{0, 0, 0});
      }
    }
  }
}

TEST_CASE("SFC::computeBins: a tiny but representable extent still bins into the valid grid", "[SFC][bins]")
{
  using Vec3 = Vec3T<double>;

  // 1e-300 is tiny, but its ValidSpan-th part is still a normal double, so the points are binned
  // normally: the low point at the origin, the high one at the far corner of the grid.
  const std::vector<Vec3> points = {Vec3(0, 0, 0), Vec3(0.5e-300, 0.25e-300, 0), Vec3(1e-300, 1e-300, 1e-300)};

  const auto bins = SFC::computeBins<double>(points);

  REQUIRE(bins[0] == SFC::Index{0, 0, 0});

  for (const auto& b : bins) {
    for (int dir = 0; dir < 3; dir++) {
      REQUIRE(b[dir] <= kMaxCoord);
    }
  }

  REQUIRE(bins[2][0] + 1 >= kMaxCoord);
  REQUIRE(bins[1][0] > bins[1][1]);
  REQUIRE(bins[1][1] > 0);
}

TEST_CASE("SFC::computeBins: an extent that overflows to infinity still gives in-range bins", "[SFC][bins]")
{
  using Vec3 = Vec3T<double>;

  const double big = std::numeric_limits<double>::max();

  const std::vector<Vec3> points = {Vec3(-big, 0, 0), Vec3(big, 1, 2), Vec3(0, -big, big)};

  for (const auto& b : SFC::computeBins<double>(points)) {
    for (int dir = 0; dir < 3; dir++) {
      REQUIRE(b[dir] <= kMaxCoord);
    }
  }
}

TEST_CASE("SFC::order: is a permutation ordering points by non-decreasing SFC code", "[SFC][order]")
{
  using Vec3 = Vec3T<double>;

  std::vector<Vec3> points;
  points.reserve(50);
  for (int i = 0; i < 50; i++) {
    points.emplace_back(0.017 * ((i * 7) % 50), 0.017 * ((i * 13) % 50), 0.017 * ((i * 29) % 50));
  }

  const auto bins  = SFC::computeBins<double>(points);
  const auto order = SFC::order<SFC::Morton>(points);

  REQUIRE(order.size() == points.size());

  // A valid permutation: every index appears exactly once.
  std::vector<char> seen(points.size(), 0);
  for (const auto idx : order) {
    REQUIRE(idx < points.size());
    REQUIRE(seen[idx] == 0);
    seen[idx] = 1;
  }

  // Codes are non-decreasing along the returned order.
  for (size_t i = 1; i < order.size(); i++) {
    REQUIRE(SFC::Morton::encode(bins[order[i - 1]]) <= SFC::Morton::encode(bins[order[i]]));
  }

  // The default tag selects Morton.
  REQUIRE(SFC::order(points) == order);

  // Nested produces its own valid, code-ordered permutation.
  const auto orderNested = SFC::order<SFC::Nested>(points);
  REQUIRE(orderNested.size() == points.size());
  for (size_t i = 1; i < orderNested.size(); i++) {
    REQUIRE(SFC::Nested::encode(bins[orderNested[i - 1]]) <= SFC::Nested::encode(bins[orderNested[i]]));
  }

  // Hilbert likewise produces a valid, code-ordered permutation.
  const auto orderHilbert = SFC::order<SFC::Hilbert>(points);
  REQUIRE(orderHilbert.size() == points.size());
  for (size_t i = 1; i < orderHilbert.size(); i++) {
    REQUIRE(SFC::Hilbert::encode(bins[orderHilbert[i - 1]]) <= SFC::Hilbert::encode(bins[orderHilbert[i]]));
  }
}

TEST_CASE("SFC::computeBins: rejects a non-finite point", "[SFC][bins][death]")
{
  // An EBGEOMETRY_REQUIRE, so it aborts in every build.
  REQUIRE(abortsWith(
    [] {
      const std::vector<Vec3T<double>> points = {Vec3T<double>(0.0, 0.0, 0.0),
                                                 Vec3T<double>(std::numeric_limits<double>::infinity(), 0.0, 0.0)};

      const auto bins = SFC::computeBins<double>(points);

      (void)bins;
    },
    "SFC::computeBins: point 1 of 2 has a non-finite coordinate"));
}
