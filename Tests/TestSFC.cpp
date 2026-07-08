// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"

#include <catch2/catch_test_macros.hpp>

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
