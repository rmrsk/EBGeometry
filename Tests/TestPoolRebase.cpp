// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// The offset/rebase proof. A structure of PODVectors is built into a host "source" pool, then
// mirrored (host-to-host std::memcpy path) into a second pool at a different address. The same POD
// values must resolve to identical elements against the new base with zero pointer patching --
// exactly the host-to-device invariant, exercised without a GPU.

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"

#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>

using namespace EBGeometry;

namespace {
// A small scene: three PODVector arrays of mixed element type. Being all-PODVector, the struct
// is itself trivially copyable and placement-independent -- the whole point of the foundation.
template <class T>
struct Scene
{
  PODVector<T>        m_scalars;
  PODVector<uint32_t> m_indices;
  PODVector<Vec3T<T>> m_points;
};

static_assert(std::is_trivially_copyable_v<Scene<float>>, "Scene<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<Scene<double>>, "Scene<double> must be trivially copyable");

// Build the scene into a_pool with deterministic known values, then freeze the pool.
template <class T>
Scene<T>
buildScene(Pool& a_pool, uint32_t a_n)
{
  Scene<T> scene;

  scene.m_scalars.reserveFrom(a_pool, a_n);
  scene.m_indices.reserveFrom(a_pool, a_n);
  scene.m_points.reserveFrom(a_pool, a_n);

  for (uint32_t i = 0; i < a_n; i++) {
    scene.m_scalars.push_back(a_pool.base(), T(i) * T(1.5) - T(2));
    scene.m_indices.push_back(a_pool.base(), i * 7u + 3u);
    scene.m_points.push_back(a_pool.base(), Vec3T<T>(T(i), T(2 * i), T(3 * i)));
  }

  a_pool.freeze();

  return scene;
}
} // namespace

TEMPLATE_TEST_CASE("Pool::mirror: offsets rebase against a different base with identical reads",
                   "[PoolRebase]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr uint32_t n = 24;

  Pool source(hostMemoryResource());

  const Scene<T> scene = buildScene<T>(source, n);

  // Mirror into a second host pool (host-to-host std::memcpy path). The scene POD value is shared
  // unchanged -- only the base differs.
  Pool mirrored = Pool::mirror(source, hostMemoryResource());

  // Different physical address...
  REQUIRE(mirrored.base() != source.base());
  REQUIRE(mirrored.isFrozen());
  REQUIRE(mirrored.usedBytes() == source.usedBytes());

  // ...but every element resolves identically against the new base.
  for (uint32_t i = 0; i < n; i++) {
    REQUIRE(scene.m_scalars.at(mirrored.base(), i) == scene.m_scalars.at(source.base(), i));
    REQUIRE(scene.m_indices.at(mirrored.base(), i) == scene.m_indices.at(source.base(), i));

    const Vec3T<T> ps = scene.m_points.at(source.base(), i);
    const Vec3T<T> pm = scene.m_points.at(mirrored.base(), i);

    REQUIRE(pm[0] == ps[0]);
    REQUIRE(pm[1] == ps[1]);
    REQUIRE(pm[2] == ps[2]);

    // And against the originally-known values.
    REQUIRE(scene.m_scalars.at(mirrored.base(), i) == T(i) * T(1.5) - T(2));
    REQUIRE(scene.m_indices.at(mirrored.base(), i) == i * 7u + 3u);
  }
}

TEMPLATE_TEST_CASE("Pool::mirror: the copy is independent of the source", "[PoolRebase]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr uint32_t n = 16;

  Pool source(hostMemoryResource());

  const Scene<T> scene = buildScene<T>(source, n);

  Pool mirrored = Pool::mirror(source, hostMemoryResource());

  // Corrupt the entire source block: the mirrored copy must be unaffected.
  std::memset(source.base(), 0, source.usedBytes());

  for (uint32_t i = 0; i < n; i++) {
    REQUIRE(scene.m_scalars.at(mirrored.base(), i) == T(i) * T(1.5) - T(2));
    REQUIRE(scene.m_indices.at(mirrored.base(), i) == i * 7u + 3u);

    const Vec3T<T> pm = scene.m_points.at(mirrored.base(), i);

    REQUIRE(pm[0] == T(i));
    REQUIRE(pm[1] == T(2 * i));
    REQUIRE(pm[2] == T(3 * i));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Where a pool-resident object resolves its arrays (PoolLocation)
// ─────────────────────────────────────────────────────────────────────────────

namespace {

// Stands in for CUDA/HIP managed or mapped memory without a GPU: memory both the host and a device
// can address. (Mirroring into it takes the resource's host-to-host copy path.)
class FakeManagedResource final : public MemoryResource
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
    return true;
  }

  bool
  isDeviceAccessible() const noexcept override
  {
    return true;
  }
};

using Meta = DCEL::DefaultMetaData;

template <class T>
using TestSDF = MeshSDF<T, Meta, 4>;

std::string
dodecahedron()
{
  return std::string(EBGEOMETRY_TEST_DATA_DIR) + "/dodecahedron.stl";
}

template <class T>
std::vector<Vec3T<T>>
probes()
{
  return {Vec3T<T>(T(0), T(0), T(0)), Vec3T<T>(T(2), T(0.5), T(-0.25)), Vec3T<T>(T(-1.2), T(1.1), T(0.9))};
}

} // namespace

TEMPLATE_TEST_CASE("PoolLocation: a view onto a managed mirror answers queries on the host",
                   "[PoolRebase][PoolLocation]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool       host(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T, Meta>(dodecahedron(), host);
  const auto sdf  = TestSDF<T>(mesh, host, BVH::Construction::SAH);

  host.freeze();

  FakeManagedResource managed;
  const Pool          mirror = Pool::mirror(host, managed);

  // A snapshot of memory the host can reach: before PoolLocation decided this by value, the host
  // pass dereferenced the (null) control block of any device-accessible view.
  const auto view = sdf.rebasedView(mirror);

  REQUIRE(view.getRoot().location().m_control == nullptr);
  REQUIRE(view.getRoot().location().m_hostAccessible);

  for (const auto& p : probes<T>()) {
    REQUIRE(view.signedDistance(p) == sdf.signedDistance(p));
  }
}

TEMPLATE_TEST_CASE("PoolLocation: an object built directly in managed memory can be rebased onto its own pool",
                   "[PoolRebase][PoolLocation]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  FakeManagedResource managed;
  Pool                pool(managed);

  const auto mesh = Parser::readIntoDCEL<T, Meta>(dodecahedron(), pool);
  const auto sdf  = TestSDF<T>(mesh, pool, BVH::Construction::SAH);

  pool.freeze();

  // No mirror needed: the pool is already device-accessible. The view is what a kernel would take.
  const auto view = sdf.rebasedView(pool);

  REQUIRE(view.getRoot().location().m_control == nullptr);

  for (const auto& p : probes<T>()) {
    REQUIRE(view.signedDistance(p) == sdf.signedDistance(p));
  }
}

TEMPLATE_TEST_CASE("PoolLocation: a view rebased onto a staging mirror can be rebased again onto its mirror",
                   "[PoolRebase][PoolLocation]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool       host(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T, Meta>(dodecahedron(), host);
  const auto sdf  = TestSDF<T>(mesh, host, BVH::Construction::SAH);

  host.freeze();

  // host -> staging (host memory, as pinned memory would be) -> final (managed). The staging view
  // follows the staging pool, whose identity is not the root's; lineage is checked by root.
  const Pool staging = Pool::mirror(host, hostMemoryResource());

  FakeManagedResource managed;
  const Pool          final = Pool::mirror(staging, managed);

  const auto stagingView = sdf.rebasedView(staging);
  const auto finalView   = stagingView.rebasedView(final);

  REQUIRE(stagingView.getRoot().location().isAttachedTo(staging));
  REQUIRE(finalView.getRoot().location().m_control == nullptr);

  for (const auto& p : probes<T>()) {
    REQUIRE(stagingView.signedDistance(p) == sdf.signedDistance(p));
    REQUIRE(finalView.signedDistance(p) == sdf.signedDistance(p));
  }
}

TEST_CASE("PoolLocation: rebasing onto an unfrozen device-accessible pool aborts", "[PoolRebase][PoolLocation][death]")
{
  using T = double;

  REQUIRE(abortsWith(
    [] {
      FakeManagedResource managed;
      Pool                pool(managed);

      const auto mesh = Parser::readIntoDCEL<T, Meta>(dodecahedron(), pool);
      const auto sdf  = TestSDF<T>(mesh, pool, BVH::Construction::SAH);

      // Not frozen: the pool could still grow and move, so a snapshot of its base would go stale.
      [[maybe_unused]] const auto view = sdf.rebasedView(pool);
    },
    "::rebasedView: a device-accessible pool must be frozen"));
}

#if defined(EBGEOMETRY_ENABLE_ASSERTIONS)
namespace {

// Device memory the host cannot reach, without a GPU.
class FakeDeviceOnlyResource final : public MemoryResource
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
  }
};

} // namespace

TEST_CASE("PoolLocation: a view of device-only memory used on the host fails an assertion",
          "[PoolRebase][PoolLocation][death]")
{
  using T = double;

  REQUIRE(abortsWith(
    [] {
      Pool       host(hostMemoryResource());
      const auto mesh = Parser::readIntoDCEL<T, Meta>(dodecahedron(), host);
      const auto sdf  = TestSDF<T>(mesh, host, BVH::Construction::SAH);

      host.freeze();

      FakeDeviceOnlyResource device;
      const Pool             mirror = Pool::mirror(host, device);
      const auto             view   = sdf.rebasedView(mirror);

      // The view is meant for a kernel. On the host its base would be a device address.
      [[maybe_unused]] const T d = view.signedDistance(Vec3T<T>::zeros());
    },
    "EBGeometry assertion failed: (m_hostAccessible)"));
}
#endif
