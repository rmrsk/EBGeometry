// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestGPU.hpp"

#include <cstdint>
#include <cstring>
#include <string>
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

// The checks below are EBGEOMETRY_REQUIREs, so they abort in every build: a release build would
// otherwise silently grow a frozen (possibly already mirrored) block, mask with a bad alignment, or
// mirror a block that can still change.

TEST_CASE("Pool: reserve after freeze aborts", "[Pool][death]")
{
  REQUIRE(abortsWith(
    [] {
      Pool pool(hostMemoryResource());

      pool.freeze();

      (void)pool.reserve(1, sizeof(double), alignof(double)); // must abort
    },
    "Pool::reserve: cannot reserve from a frozen pool"));
}

TEST_CASE("Pool: reserve with an invalid alignment aborts", "[Pool][death]")
{
  const char* const message = "Pool::reserve: the alignment must be a power of two no larger than 256";

  // Zero (would underflow the alignment mask).
  REQUIRE(abortsWith(
    [] {
      Pool pool(hostMemoryResource());

      (void)pool.reserve(1, sizeof(double), 0);
    },
    message));

  // Not a power of two.
  REQUIRE(abortsWith(
    [] {
      Pool pool(hostMemoryResource());

      (void)pool.reserve(1, sizeof(double), 24);
    },
    std::string(message) + " (24)"));

  // Larger than the base alignment, which the block cannot honour.
  REQUIRE(abortsWith(
    [] {
      Pool pool(hostMemoryResource());

      (void)pool.reserve(1, sizeof(double), 2 * PoolBaseAlign);
    },
    std::string(message) + " (512)"));
}

TEST_CASE("Pool: mirror of a non-frozen pool aborts", "[Pool][death]")
{
  REQUIRE(abortsWith(
    [] {
      Pool pool(hostMemoryResource());

      PODVector<double> vec;
      vec.reserveFrom(pool, 4);

      // Not frozen -- mirror must abort.
      Pool mirror = Pool::mirror(pool, hostMemoryResource());

      (void)mirror.base();
    },
    "Pool::mirror: the source pool must be frozen before it is mirrored"));
}

TEST_CASE("Pool: reserving from a moved-from pool aborts", "[Pool][death]")
{
  // A moved-from pool owns no control block, so there is nowhere to publish a base. The check is an
  // EBGEOMETRY_REQUIRE, so it aborts in every build (a release build would otherwise dereference
  // null).
  REQUIRE(abortsWith(
    [] {
      Pool source(hostMemoryResource(), 128);
      Pool moved(std::move(source));

      PODVector<double> vec;
      vec.reserveFrom(source, 4); // must abort
    },
    "Pool::reserve: cannot reserve from a moved-from pool"));
}

// ─────────────────────────────────────────────────────────────────────────────
// Move semantics
// ─────────────────────────────────────────────────────────────────────────────

namespace {

// A stand-in for a device resource: host memory that reports itself as device-only, and counts the
// copies routed through it. Lets a host-only build check that Pool::mirror hands the copy to the
// non-host resource, as it must for a real device resource.
class FakeDeviceResource final : public MemoryResource
{
public:
  void*
  allocate(size_t a_bytes, size_t a_alignment) override
  {
    return hostMemoryResource().allocate(a_bytes, a_alignment);
  }

  void
  deallocate(void* a_ptr, size_t a_bytes, size_t a_alignment) noexcept override
  {
    hostMemoryResource().deallocate(a_ptr, a_bytes, a_alignment);
  }

  bool
  isHostAccessible() const noexcept override
  {
    return false;
  }

  bool
  isDeviceAccessible() const noexcept override
  {
    return true;
  }

  void
  copy(void* a_dst, const MemoryResource&, const void* a_src, const MemoryResource&, size_t a_bytes)
    const noexcept override
  {
    std::memcpy(a_dst, a_src, a_bytes);

    m_copies++;
  }

  mutable int m_copies = 0;
};

} // namespace

TEST_CASE("Pool: mirror hands the copy to the non-host resource", "[Pool]")
{
  Pool host(hostMemoryResource());

  const size_t offset = host.reserve(1, sizeof(int), alignof(int));

  *static_cast<int*>(static_cast<void*>(static_cast<char*>(host.base()) + offset)) = 42;

  host.freeze();

  FakeDeviceResource device;

  const Pool mirror = Pool::mirror(host, device);

  REQUIRE(device.m_copies == 1);
  REQUIRE(*static_cast<const int*>(static_cast<const void*>(static_cast<const char*>(mirror.base()) + offset)) == 42);

  // Host to host goes through the base implementation, not the fake.
  const Pool hostMirror = Pool::mirror(host, hostMemoryResource());

  REQUIRE(device.m_copies == 1);
  REQUIRE(hostMirror.usedBytes() == host.usedBytes());
}

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

TEST_CASE("Pool: the control block survives a move, including a vector reallocation", "[Pool]")
{
  // The reason a pool-resident object stores a PoolControl* rather than a Pool*: moving a Pool must
  // not disturb anything resolving through it. A std::vector<Pool> reallocation is the sharp case,
  // since it moves *and then destroys* the source object rather than merely emptying it.
  std::vector<Pool> pools;
  pools.reserve(1);
  pools.emplace_back(hostMemoryResource(), 512);

  const PoolControl* control = pools[0].control();
  const uint64_t     id      = pools[0].id();

  REQUIRE(control != nullptr);
  REQUIRE(id != 0);

  PODVector<double> vec;
  vec.reserveFrom(pools[0], 8);

  for (uint32_t i = 0; i < 8; i++) {
    vec.push_back(pools[0].base(), double(i) + 0.25);
  }

  // Force a reallocation: pools[0] is move-constructed into fresh storage, and the original object
  // is destroyed.
  while (pools.size() < 16) {
    pools.emplace_back(hostMemoryResource(), 64);
  }

  // Same control block, same identity, same data -- resolved without ever touching the (now dead)
  // original Pool object.
  REQUIRE(pools[0].control() == control);
  REQUIRE(pools[0].id() == id);

  for (uint32_t i = 0; i < 8; i++) {
    REQUIRE(vec.at(control->m_base, i) == double(i) + 0.25);
  }
}

TEST_CASE("Pool: a grow republishes the base through the control block", "[Pool]")
{
  // The other half of the contract: an object that re-resolves through the control block sees a
  // grow immediately, with no freeze and no rebinding.
  Pool pool(hostMemoryResource(), 64);

  const PoolControl* control = pool.control();

  PODVector<double> vec;
  vec.reserveFrom(pool, 4);

  for (uint32_t i = 0; i < 4; i++) {
    vec.push_back(pool.base(), double(i) + 0.5);
  }

  void* const baseBeforeGrow = control->m_base;

  PODVector<double> filler;
  filler.reserveFrom(pool, 4096); // certain to exceed the 64-byte initial capacity

  REQUIRE(control->m_base != baseBeforeGrow); // the block really did move
  REQUIRE(pool.control() == control);         // but the control block did not

  for (uint32_t i = 0; i < 4; i++) {
    REQUIRE(vec.at(control->m_base, i) == double(i) + 0.5);
  }
}

TEST_CASE("Pool: identities are unique and mirrorOf names the root of a chain", "[Pool]")
{
  Pool a(hostMemoryResource(), 128);
  Pool b(hostMemoryResource(), 128);

  REQUIRE(a.id() != b.id());
  REQUIRE(a.mirrorOf() == 0); // not a mirror
  REQUIRE(b.mirrorOf() == 0);

  PODVector<double> vec;
  vec.reserveFrom(a, 4);

  for (uint32_t i = 0; i < 4; i++) {
    vec.push_back(a.base(), double(i));
  }

  a.freeze();

  const Pool hop1 = Pool::mirror(a, hostMemoryResource());
  const Pool hop2 = Pool::mirror(hop1, hostMemoryResource());

  // A mirror is its own pool, but every hop names the *root*, so an object built in `a` can still
  // recognise `hop2` as a faithful copy of its own storage.
  REQUIRE(hop1.id() != a.id());
  REQUIRE(hop2.id() != hop1.id());
  REQUIRE(hop1.mirrorOf() == a.id());
  REQUIRE(hop2.mirrorOf() == a.id());

  for (uint32_t i = 0; i < 4; i++) {
    REQUIRE(vec.at(hop2.base(), i) == double(i));
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

  DeviceBuffer<double> deviceOut;

  poolDeviceKernel<<<1, 1>>>(vec, devicePool.base(), deviceOut.get());
  (void)GPU::deviceSynchronize();

  // The reduction sums small integer-valued doubles in the same order on host and device, so the
  // result is bit-exact -- no floating-point matcher (or its header) is needed here.
  REQUIRE(readScalar(deviceOut.get()) == hostSum);
}
#endif
