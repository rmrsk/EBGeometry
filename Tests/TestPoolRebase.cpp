// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// The offset/rebase proof. A structure of PODVectors is built into a host "source" pool, then
// mirrored (host-to-host std::memcpy path) into a second pool at a different address. The same POD
// values must resolve to identical elements against the new base with zero pointer patching --
// exactly the host-to-device invariant, exercised without a GPU.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"

#include <cstdint>
#include <cstring>
#include <type_traits>

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
