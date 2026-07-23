// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestGPU.hpp"

#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

using namespace EBGeometry;

namespace {
struct alignas(16) Align16
{
  double m_x[2];
};

struct alignas(64) Align64
{
  double m_x[8];
};

static_assert(std::is_trivially_copyable_v<Align16>, "Align16 must be trivially copyable");
static_assert(std::is_trivially_copyable_v<Align64>, "Align64 must be trivially copyable");
} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// Grow: exceeding the initial block reallocates but preserves contents
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("Pool: grow enlarges the block and preserves prior contents", "[Pool]")
{
  Pool pool(hostMemoryResource(), 256);

  REQUIRE(pool.capacityBytes() == 256);
  REQUIRE(pool.usedBytes() == 0);

  // First array (64 bytes) fits inside the initial block.
  PODVector<uint64_t> first;
  first.reserveFrom(pool, 8);

  for (uint32_t i = 0; i < 8; i++) {
    first.push_back(pool.base(), 0xDEADBEEF00ULL + i); // sentinel pattern
  }

  const size_t capBefore = pool.capacityBytes();

  // Second array (512 bytes) overflows the 256-byte block and forces a grow.
  PODVector<uint64_t> second;
  second.reserveFrom(pool, 64);

  for (uint32_t i = 0; i < 64; i++) {
    second.push_back(pool.base(), 0xABCD0000ULL + i);
  }

  REQUIRE(pool.capacityBytes() > capBefore);

  // The first array's contents survived the reallocation (offsets resolve against the new base).
  for (uint32_t i = 0; i < 8; i++) {
    REQUIRE(first.at(pool.base(), i) == 0xDEADBEEF00ULL + i);
  }

  // ...and the second array reads back correctly too.
  for (uint32_t i = 0; i < 64; i++) {
    REQUIRE(second.at(pool.base(), i) == 0xABCD0000ULL + i);
  }
}

TEST_CASE("Pool: repeated growth across several doublings preserves every region", "[Pool]")
{
  Pool pool(hostMemoryResource()); // empty; first reserve allocates

  std::vector<PODVector<uint32_t>> arrays;

  constexpr uint32_t numArrays = 20;
  constexpr uint32_t perArray  = 32;

  for (uint32_t a = 0; a < numArrays; a++) {
    PODVector<uint32_t> vec;
    vec.reserveFrom(pool, perArray);

    for (uint32_t i = 0; i < perArray; i++) {
      vec.push_back(pool.base(), a * 1000u + i);
    }

    arrays.push_back(vec);
  }

  pool.freeze();

  for (uint32_t a = 0; a < numArrays; a++) {
    for (uint32_t i = 0; i < perArray; i++) {
      REQUIRE(arrays[a].at(pool.base(), i) == a * 1000u + i);
    }
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Alignment of interleaved reservations
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("Pool: interleaved reservations honour each type's alignment", "[Pool]")
{
  Pool pool(hostMemoryResource());

  const uint64_t o0 = pool.reserve(1, sizeof(uint32_t), alignof(uint32_t));
  const uint64_t o1 = pool.reserve(3, sizeof(Vec3T<double>), alignof(Vec3T<double>));
  const uint64_t o2 = pool.reserve(1, sizeof(Align16), alignof(Align16));
  const uint64_t o3 = pool.reserve(2, sizeof(uint32_t), alignof(uint32_t));
  const uint64_t o4 = pool.reserve(1, sizeof(Align64), alignof(Align64));
  const uint64_t o5 = pool.reserve(4, sizeof(Vec3T<double>), alignof(Vec3T<double>));

  REQUIRE(o0 % alignof(uint32_t) == 0);
  REQUIRE(o1 % alignof(Vec3T<double>) == 0);
  REQUIRE(o2 % alignof(Align16) == 0);
  REQUIRE(o3 % alignof(uint32_t) == 0);
  REQUIRE(o4 % alignof(Align64) == 0);
  REQUIRE(o5 % alignof(Vec3T<double>) == 0);

  pool.freeze();

  // The base is PoolBaseAlign-aligned, so offset alignment lifts to absolute pointer alignment.
  REQUIRE(reinterpret_cast<uintptr_t>(pool.base()) % PoolBaseAlign == 0);

  auto* base = static_cast<unsigned char*>(pool.base());

  REQUIRE(reinterpret_cast<uintptr_t>(base + o4) % alignof(Align64) == 0);
  REQUIRE(reinterpret_cast<uintptr_t>(base + o2) % alignof(Align16) == 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Freeze contract
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("Pool: freeze stabilises base and is idempotent", "[Pool]")
{
  Pool pool(hostMemoryResource(), 4096); // large enough that reserving does not grow

  PODVector<double> vec;
  vec.reserveFrom(pool, 16);

  void* beforeFreeze = pool.base();

  REQUIRE_FALSE(pool.isFrozen());

  pool.freeze();

  REQUIRE(pool.isFrozen());
  REQUIRE(pool.base() == beforeFreeze); // no grow occurred, so base is unchanged

  pool.freeze(); // idempotent

  REQUIRE(pool.isFrozen());
  REQUIRE(pool.base() == beforeFreeze);
}

#if defined(EBGEOMETRY_ENABLE_ASSERTIONS)

TEST_CASE("Pool: reserve after freeze aborts", "[Pool][death]")
{
  REQUIRE(abortsUnderAssertions([] {
    Pool pool(hostMemoryResource());

    pool.freeze();

    (void)pool.reserve(1, sizeof(double), alignof(double)); // must abort
  }));
}

TEST_CASE("Pool: mirror of a non-frozen pool aborts", "[Pool][death]")
{
  REQUIRE(abortsUnderAssertions([] {
    Pool pool(hostMemoryResource());

    PODVector<double> vec;
    vec.reserveFrom(pool, 4);

    // Not frozen -- mirror must abort.
    Pool mirror = Pool::mirror(pool, hostMemoryResource());

    (void)mirror.base();
  }));
}

#endif // EBGEOMETRY_ENABLE_ASSERTIONS

// ─────────────────────────────────────────────────────────────────────────────
// Move semantics
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("Pool: move construction transfers ownership and empties the source", "[Pool]")
{
  Pool source(hostMemoryResource(), 512);

  PODVector<double> vec;
  vec.reserveFrom(source, 8);

  for (uint32_t i = 0; i < 8; i++) {
    vec.push_back(source.base(), double(i));
  }

  void* stolenBase = source.base();

  Pool moved(std::move(source));

  // Source is emptied (null base, zero sizes) so its destructor frees nothing.
  REQUIRE(source.base() == nullptr);
  REQUIRE(source.usedBytes() == 0);
  REQUIRE(source.capacityBytes() == 0);

  // Destination owns the original block.
  REQUIRE(moved.base() == stolenBase);

  for (uint32_t i = 0; i < 8; i++) {
    REQUIRE(vec.at(moved.base(), i) == double(i));
  }
}

TEST_CASE("Pool: move assignment frees the target's old block and steals the source's", "[Pool]")
{
  Pool target(hostMemoryResource(), 256); // has its own block, to be freed on assignment
  Pool source(hostMemoryResource(), 1024);

  PODVector<double> vec;
  vec.reserveFrom(source, 4);

  for (uint32_t i = 0; i < 4; i++) {
    vec.push_back(source.base(), double(i) + 0.5);
  }

  void* stolenBase = source.base();

  target = std::move(source); // frees target's old 256-byte block exactly once (sanitizer-checked)

  REQUIRE(source.base() == nullptr);
  REQUIRE(target.base() == stolenBase);

  for (uint32_t i = 0; i < 4; i++) {
    REQUIRE(vec.at(target.base(), i) == double(i) + 0.5);
  }
}

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
// ─────────────────────────────────────────────────────────────────────────────
// Device: a PODVector built into a host Pool, mirrored to the device, reads back
// correctly through base + offset with no pointer patching
// ─────────────────────────────────────────────────────────────────────────────

namespace {
struct PoolSmokeElement
{
  double m_a;
  double m_b;
};

static_assert(std::is_trivially_copyable_v<PoolSmokeElement>, "PoolSmokeElement must be trivially copyable");
} // namespace

EBGEOMETRY_GLOBAL
void
poolDeviceKernel(PODVector<PoolSmokeElement> a_vec, void* a_base, double* a_out)
{
  double sum = 0.0;

  for (uint32_t i = 0; i < a_vec.size(); i++) {
    const PoolSmokeElement element = a_vec.at(a_base, i);

    sum += element.m_a + element.m_b;
  }

  a_out[0] = sum;
}

TEST_CASE("Pool: a mirrored PODVector reads back correctly on the device", "[Pool][PODVector][gpu]")
{
  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  Pool hostPool(hostMemoryResource());

  PODVector<PoolSmokeElement> vec;
  vec.reserveFrom(hostPool, 8);

  double hostSum = 0.0;

  for (uint32_t i = 0; i < 8; i++) {
    const PoolSmokeElement element{double(i), double(2 * i)};

    vec.push_back(hostPool.base(), element);
    hostSum += element.m_a + element.m_b;
  }

  hostPool.freeze();

  Pool devicePool = Pool::mirror(hostPool, deviceMemoryResource());

  double* deviceOut = deviceScalar();

  poolDeviceKernel<<<1, 1>>>(vec, devicePool.base(), deviceOut);
  (void)GPU::deviceSynchronize();

  // The reduction sums small integer-valued doubles in the same order on host and device, so the
  // result is bit-exact -- no floating-point matcher (or its header) is needed here.
  REQUIRE(readScalar(deviceOut) == hostSum);

  deviceFree(deviceOut);
}
#endif
