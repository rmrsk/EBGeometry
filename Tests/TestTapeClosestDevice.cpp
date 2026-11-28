// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// The device tests of Tape::closestLeaf() and Tape::getSurfaceLabel() (TestTapeClosest.cpp): the deciding leaf and
// its label on a device against the host. They are apart from TestTapeClosest.cpp so that a GPU build compiles only
// them with the device compiler.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"
#include "TestTapeUtils.hpp"
#include "TestUtilities.hpp"

#include <cstdint>
#include <string>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using EBGeometry::TestUtils::labeledRow;

namespace {

// Device functor: the deciding leaf at a point, and the leaf's own value at its local point.
template <class T, class TapeType>
struct LeafQuery
{
  TapeType m_tape;

  EBGEOMETRY_HOST_DEVICE
  LeafRef<T>
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return m_tape.closestLeaf(a_point);
  }
};

// Device functor: the label of the deciding leaf at a point.
template <class T, class TapeType>
struct LabelQuery
{
  TapeType m_tape;

  EBGEOMETRY_HOST_DEVICE
  uint32_t
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return m_tape.getSurfaceLabel(m_tape.closestLeaf(a_point)).value();
  }
};

} // namespace

TEMPLATE_TEST_CASE("Tape: closestLeaf on a device matches the host", "[Tape][gpu]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  constexpr size_t K = BVH::DefaultBranchingRatio<T>();
  constexpr size_t W = TriangleSoA::DefaultWidth<T>();

  Pool pool(hostMemoryResource());

  const auto                mesh = Parser::readIntoDCEL<T>(dodecahedron(), pool);
  const TriMeshSDF<T, K, W> tri(mesh, pool, BVH::Construction::SAH, 4);

  std::vector<Expr<T>> operands{Translate(tri, Vec3(T(1.2), T(0), T(0)))};

  for (int i = 0; i < 10; i++) {
    operands.emplace_back(SphereSDF<T>(Vec3(T(0.6) * T(i % 5) - T(1.5), T(0.7) * T(i / 5), T(0)), T(0.35)));
  }

  // A union through a BVH, a smooth blend, a blur and a repetition, with a mesh leaf.
  const Tape<T> tape(
    pool,
    SmoothDifference(
      Union(operands),
      Blur(FiniteRepetition(SphereSDF<T>(Vec3::zeros(), T(0.2)), Vec3(T(1), T(1), T(1)), Vec3::ones(), Vec3::ones()),
           T(0.05)),
      T(0.1)));

  pool.freeze();

  const Pool device = Pool::mirror(pool, deviceTestResource());

  const auto points = queryGrid<T>(Vec3(T(-2), T(-0.5), T(-0.5)), Vec3(T(2.5), T(1.5), T(0.5)), 9);

  const auto onDevice = evaluateOnDevice<LeafRef<T>>(LeafQuery<T, Tape<T>>{tape.rebasedView(device)}, points);
  const auto onHost   = evaluateOnHost<LeafRef<T>>(LeafQuery<T, Tape<T>>{tape}, points);

  size_t differ = 0;

  for (size_t i = 0; i < points.size(); i++) {
    differ += (onDevice[i].m_leaf != onHost[i].m_leaf) ? 1U : 0U;
  }

  // The device rounds differently (fused multiply-add), so a point where two leaves are nearly as close can
  // name the other; anything more is a fault.
  CHECK(differ <= points.size() / 100);
}

TEMPLATE_TEST_CASE("Tape: getSurfaceLabel on a device matches the host", "[Tape][gpu]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  Pool pool(hostMemoryResource());

  const Tape<T> tape(pool, labeledRow<T>());

  pool.freeze();

  const Pool device = Pool::mirror(pool, deviceTestResource());

  // Points near each sphere, and between them.
  std::vector<Vec3> points;

  for (int i = 0; i < 60; i++) {
    points.emplace_back(T(0.25) * T(i) - T(1), T(1.1), T(0));
  }

  const auto onDevice = evaluateOnDevice<uint32_t>(LabelQuery<T, Tape<T>>{tape.rebasedView(device)}, points);
  const auto onHost   = evaluateOnHost<uint32_t>(LabelQuery<T, Tape<T>>{tape}, points);

  size_t differ = 0;

  for (size_t i = 0; i < points.size(); i++) {
    differ += (onDevice[i] != onHost[i]) ? 1U : 0U;
  }

  // Midway between two spheres both are as close; the device's rounding may pick the other.
  CHECK(differ <= 2U);
}
