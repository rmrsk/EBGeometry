// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"

#include <cstdint>
#include <type_traits>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace EBGeometry;

namespace {
// A small mixed-field trivially-copyable POD, standing in for the device-visible structs
// (e.g. BVH leaves) the foundation is built to carry.
struct MixedPOD
{
  uint32_t m_index;
  float    m_scalar;
  double   m_value;

  bool
  operator==(const MixedPOD& a_other) const
  {
    return m_index == a_other.m_index && m_scalar == a_other.m_scalar && m_value == a_other.m_value;
  }
};

static_assert(std::is_trivially_copyable_v<MixedPOD>, "MixedPOD must be trivially copyable");
} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// Layout guarantees
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("PODVector: layout is 16 bytes and trivially copyable", "[PODVector]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  STATIC_REQUIRE(sizeof(PODVector<T>) == 16);
  STATIC_REQUIRE(std::is_trivially_copyable_v<PODVector<T>>);
  STATIC_REQUIRE(std::is_trivially_copyable_v<PODSpan<T>>);
}

TEST_CASE("PODVector: layout holds for integer and mixed-POD element types", "[PODVector]")
{
  STATIC_REQUIRE(sizeof(PODVector<uint32_t>) == 16);
  STATIC_REQUIRE(sizeof(PODVector<MixedPOD>) == 16);
  STATIC_REQUIRE(std::is_trivially_copyable_v<PODVector<uint32_t>>);
  STATIC_REQUIRE(std::is_trivially_copyable_v<PODVector<MixedPOD>>);
}

// ─────────────────────────────────────────────────────────────────────────────
// reserveFrom / push_back / at
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("PODVector: reserveFrom + push_back fills and reads back identically",
                   "[PODVector]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  PODVector<T> vec;
  vec.reserveFrom(pool, 16);

  REQUIRE(vec.size() == 0);
  REQUIRE(vec.empty());

  for (uint32_t i = 0; i < 16; i++) {
    vec.push_back(pool.base(), T(i) * T(1.5));
  }

  REQUIRE(vec.size() == 16);
  REQUIRE_FALSE(vec.empty());

  pool.freeze();

  for (uint32_t i = 0; i < 16; i++) {
    REQUIRE(vec.at(pool.base(), i) == T(i) * T(1.5));
  }
}

TEST_CASE("PODVector: mixed-POD element round-trips through push_back/at", "[PODVector]")
{
  Pool pool(hostMemoryResource());

  PODVector<MixedPOD> vec;
  vec.reserveFrom(pool, 4);

  const MixedPOD values[4] = {{0u, 1.0F, 2.0}, {1u, 3.0F, 4.0}, {2u, 5.0F, 6.0}, {3u, 7.0F, 8.0}};

  for (const auto& v : values) {
    vec.push_back(pool.base(), v);
  }

  pool.freeze();

  for (uint32_t i = 0; i < 4; i++) {
    REQUIRE(vec.at(pool.base(), i) == values[i]);
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// assign (bulk fill)
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("PODVector: assign matches element-wise fill", "[PODVector]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  PODVector<T> viaPush;
  PODVector<T> viaAssign;

  viaPush.reserveFrom(pool, 32);
  viaAssign.reserveFrom(pool, 32);

  std::vector<T> source(32);

  for (uint32_t i = 0; i < 32; i++) {
    source[i] = T(i) * T(0.25) - T(3);

    viaPush.push_back(pool.base(), source[i]);
  }

  viaAssign.assign(pool.base(), source.data(), 32);

  pool.freeze();

  REQUIRE(viaAssign.size() == viaPush.size());

  for (uint32_t i = 0; i < 32; i++) {
    REQUIRE(viaAssign.at(pool.base(), i) == viaPush.at(pool.base(), i));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// bind (PODSpan)
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("PODVector: bind() PODSpan iterates the same values as at()",
                   "[PODVector]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  PODVector<T> vec;
  vec.reserveFrom(pool, 10);

  for (uint32_t i = 0; i < 10; i++) {
    vec.push_back(pool.base(), T(i) * T(2) + T(1));
  }

  pool.freeze();

  const PODSpan<T> span = vec.bind(pool.base());

  REQUIRE(span.size() == vec.size());

  for (uint32_t i = 0; i < span.size(); i++) {
    REQUIRE(span[i] == vec.at(pool.base(), i));
  }

  // Range-for over the span (begin/end) visits the same sequence.
  uint32_t idx = 0;

  for (const T& value : span) {
    REQUIRE(value == vec.at(pool.base(), idx));

    idx++;
  }

  REQUIRE(idx == vec.size());
}

// ─────────────────────────────────────────────────────────────────────────────
// Alignment of reserved sub-regions
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("PODVector: reserved offsets satisfy the element type's alignment", "[PODVector]")
{
  Pool pool(hostMemoryResource());

  // A single uint32_t (4 bytes) first, so the next reservation is only naturally aligned if
  // reserveFrom pads the cursor up to alignof(T).
  PODVector<uint32_t> ints;
  ints.reserveFrom(pool, 1);

  PODVector<Vec3T<double>> vecs;
  vecs.reserveFrom(pool, 3);

  PODVector<float> floats;
  floats.reserveFrom(pool, 5);

  // Offsets are relative to a PoolBaseAlign(=256)-aligned base, so offset alignment implies
  // absolute alignment.
  REQUIRE(vecs.m_offset % alignof(Vec3T<double>) == 0);
  REQUIRE(vecs.m_offset % 8 == 0);
  REQUIRE(floats.m_offset % alignof(float) == 0);
  REQUIRE(floats.m_offset % 4 == 0);

  pool.freeze();

  REQUIRE(reinterpret_cast<uintptr_t>(vecs.data(pool.base())) % alignof(Vec3T<double>) == 0);
  REQUIRE(reinterpret_cast<uintptr_t>(floats.data(pool.base())) % alignof(float) == 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// No-realloc / bounds invariants (assert-death, assertions build only)
// ─────────────────────────────────────────────────────────────────────────────

#if defined(EBGEOMETRY_ENABLE_ASSERTIONS)

TEST_CASE("PODVector: push_back past capacity aborts (no-realloc invariant)", "[PODVector][death]")
{
  REQUIRE(abortsUnderAssertions([] {
    Pool pool(hostMemoryResource());

    PODVector<double> vec;
    vec.reserveFrom(pool, 2);

    vec.push_back(pool.base(), 1.0);
    vec.push_back(pool.base(), 2.0);
    vec.push_back(pool.base(), 3.0); // capacity is 2 -- must abort
  }));
}

TEST_CASE("PODVector: at() out of range aborts (bounds invariant)", "[PODVector][death]")
{
  REQUIRE(abortsUnderAssertions([] {
    Pool pool(hostMemoryResource());

    PODVector<double> vec;
    vec.reserveFrom(pool, 4);

    vec.push_back(pool.base(), 1.0);

    pool.freeze();

    volatile double sink = vec.at(pool.base(), vec.size()); // index == size -- must abort

    (void)sink;
  }));
}

#endif // EBGEOMETRY_ENABLE_ASSERTIONS
