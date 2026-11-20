// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// The device tests of the .ebg language (EBGeometry_Language.hpp): tables of properties read on a device by region.
// They are apart from TestLanguage.cpp so that a GPU build compiles only them with the device compiler.

#include "Source/EBGeometry_Language.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

#include <cstdint>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>

using namespace EBGeometry;

namespace {

// A row of a solver's table.
template <class T>
struct Gas
{
  T        m_density;
  uint32_t m_kind;
};

// Reads a region's row on the device by the region the tape finds.
template <class T>
struct TableQuery
{
  Tape<T>                 m_tape;
  Language::Table<Gas<T>> m_table;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    const Gas<T>& row = m_table[m_tape.region(a_point)];

    return row.m_density + T(10) * T(row.m_kind);
  }
};

} // namespace

TEMPLATE_TEST_CASE("Language: tables of properties in the pool, read on a device by region",
                   "[Language][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  Pool pool(hostMemoryResource());

  const auto model = Language::parse<T>(R"(ebg 0
object overlay {
  region air   = box((-2, -2, -2), (2, 2, 2)) material air
  region solid = sphere((0, 0, 0), 1) material steel
}
material air { density = 1.2, kind = "gas" }
material steel { density = 7800, kind = "solid" }
domain heat {
  subdomain s = solid
}
)",
                                        "test.ebg",
                                        pool);

  const Tape<T> tape(pool, model.object());

  const auto fill = [](const Language::Properties& a_p) {
    return Gas<T>{T(a_p.number("density", 0)), a_p.has("kind") ? a_p.code("kind", {"gas", "solid"}) : 2U};
  };

  const Language::Table<Gas<T>> regions = model.template regions<Gas<T>>(pool, fill);
  const Language::Table<Gas<T>> volumes = model.domain("heat").template volumes<Gas<T>>(pool, fill);

  REQUIRE(regions.size() == 3U);
  CHECK(regions[0].m_kind == 2U);
  CHECK(regions[1].m_density == T(1.2));
  REQUIRE(volumes.size() == 1U);
  CHECK(volumes[0].m_density == T(7800));

  pool.freeze();

  const Pool device = Pool::mirror(pool, deviceTestResource());

  const std::vector<Vec3T<T>> points{Vec3T<T>(0, 0, 0), Vec3T<T>(T(1.5), 0, 0), Vec3T<T>(3, 0, 0)};

  const auto onDevice =
    evaluateOnDevice<T>(TableQuery<T>{tape.rebasedView(device), regions.rebasedView(device)}, points);

  CHECK(onDevice[0] == T(7800) + T(10));
  CHECK(onDevice[1] == T(1.2));
  CHECK(onDevice[2] == T(20));
}
