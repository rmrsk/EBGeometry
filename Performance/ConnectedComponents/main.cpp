// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

// Performance cases for connectedComponents(): the connected components of every region of a model, at two
// resolutions. The model is a box of air holding a grid of hollow shells, each a region whose hollow is a cavity of the
// outside: the search refines down to h near every surface, and its cost grows with the surfaces' area over h squared
// and with the number of regions each evaluation visits. In float and double, on the host (the search is host only).
// In a probe of its own, so that adding it changes no other probe's translation unit (see Performance/Fused). See
// ../README.md and PerformanceTesting.rst.

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// Shells along each axis of the box, at full and at smoke size.
constexpr std::size_t ShellsPerAxis      = 3;
constexpr std::size_t SmokeShellsPerAxis = 1;

// The resolutions: the box's side over these, at full and at smoke size.
constexpr std::size_t CoarseCells      = 64;
constexpr std::size_t FineCells        = 128;
constexpr std::size_t SmokeCoarseCells = 16;
constexpr std::size_t SmokeFineCells   = 24;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

// The model: an Overlay of the air (region 1, the unit cube), then every shell (2 on), its hollow left to the outside.
template <class T>
EBGeometry::Expr<T>
shells(const std::size_t a_perAxis)
{
  std::vector<EBGeometry::Expr<T>> operands{
    EBGeometry::BoxSDF<T>(Vec3<T>(T(0), T(0), T(0)), Vec3<T>(T(1), T(1), T(1)))};

  const T cell  = T(1) / T(a_perAxis);
  const T outer = T(0.35) * cell;
  const T inner = T(0.2) * cell;

  for (std::size_t i = 0; i < a_perAxis; i++) {
    for (std::size_t j = 0; j < a_perAxis; j++) {
      for (std::size_t k = 0; k < a_perAxis; k++) {
        const Vec3<T> center(cell * (T(i) + T(0.5)), cell * (T(j) + T(0.5)), cell * (T(k) + T(0.5)));

        operands.emplace_back(
          EBGeometry::Difference(EBGeometry::SphereSDF<T>(center, outer), EBGeometry::SphereSDF<T>(center, inner)));
      }
    }
  }

  return EBGeometry::Overlay(operands);
}

template <class T>
void
addResolution(Perf::Suite& a_suite, const std::size_t a_cells, const std::size_t a_smokeCells, const char* a_name)
{
  const std::string prec = Perf::precisionName<T>();

  a_suite.add(Perf::caseName({"ConnectedComponents", "shells", a_name, prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const std::size_t         perAxis = c.size(ShellsPerAxis, SmokeShellsPerAxis);
    const std::size_t         cells   = c.size(a_cells, a_smokeCells);
    const EBGeometry::Tape<T> tape(pool, shells<T>(perAxis));

    const EBGeometry::ComponentOptions<T> options{T(1) / T(cells)};

    EBGeometry::ConnectedComponents<T> components = EBGeometry::connectedComponents(tape, options);

    c.measure([&] { components = EBGeometry::connectedComponents(tape, options); });

    // The checksum: every component's group, volume and whether it reaches the box.
    double sum = 0.0;

    for (uint32_t i = 0; i < components.getNumComponents(); i++) {
      const auto& component = components.component(i);

      sum += double(component.m_group) + double(component.m_volume) + (component.m_reachesBox ? 1.0 : 0.0);
    }

    c.checksum(sum);
    c.param("shells", perAxis * perAxis * perAxis);
    c.param("regions", std::size_t(tape.getNumRegions()));
    c.param("cellsPerSide", cells);
    c.param("components", std::size_t(components.getNumComponents()));
    c.param("octreeCells", std::size_t(components.getNumCells()));
    c.poolBytes(pool.usedBytes());
  });
}

template <class T>
void
addPrecision(Perf::Suite& a_suite)
{
  addResolution<T>(a_suite, CoarseCells, SmokeCoarseCells, "build-coarse");
  addResolution<T>(a_suite, FineCells, SmokeFineCells, "build-fine");
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "ConnectedComponents");

  addPrecision<float>(suite);
  addPrecision<double>(suite);

  return suite.run();
}
