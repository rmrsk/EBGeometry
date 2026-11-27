// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// The device tests of the fused CSG operations (TestFused.cpp): an analytic composition and a drilled mesh evaluated
// on a device against the host. They are apart from TestFused.cpp so that a GPU build compiles only them with the
// device compiler.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestFusedHelpers.hpp"
#include "TestGPU.hpp"
#include "TestUtilities.hpp"

#include <type_traits>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>

using namespace EBGeometry;
using namespace EBGeometryTestFused;

namespace {

// Device functor: a shape's value at one point.
template <class T, class Shape>
struct Query
{
  Shape m_shape;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return m_shape.signedDistance(a_point);
  }
};

} // namespace

TEMPLATE_TEST_CASE("Fused: device evaluation matches the host", "[Fused][gpu]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  constexpr size_t K = BVH::DefaultBranchingRatio<T>();
  constexpr size_t W = TriangleSoA::DefaultWidth<T>();

  const Shapes<T> s;

  const std::vector<Vec3> points = probes<T>();

  // Every operation, without a pool.
  const auto analytic = Fused::SmoothUnion(
    Fused::operands(
      Fused::Difference(Fused::Intersection(s.box, s.ball), s.drill),
      Fused::Translate(Fused::Rotate(Fused::Scale(Fused::Reflect(s.torus, size_t(0)), T(1.5)), T(30), size_t(2)),
                       Vec3(T(1), T(0), T(0))),
      Fused::Annular(Fused::Offset(Fused::Complement(s.small), T(0.1)), T(0.05)),
      Fused::Elongate(s.small, Vec3(T(0.2), T(0), T(0))),
      Fused::FiniteRepetition(s.small, Vec3(T(1.3), T(2), T(1.1)), Vec3::ones(), Vec3::ones()),
      Fused::Blur(s.box, T(0.1)),
      Fused::Mollify(s.ball, T(0.1), size_t(3))),
    T(0.2),
    BlendKind::Exponential);

  using A = std::decay_t<decltype(analytic)>;

  requireSameResults(evaluateOnDevice<T>(Query<T, A>{analytic}, points),
                     evaluateOnHost<T>(Query<T, A>{analytic}, points));

  // A mesh SDF inside: the composition is evaluated against a device mirror of its pool.
  Pool pool(hostMemoryResource());

  const auto                mesh = Parser::readIntoDCEL<T>(dodecahedron(), pool);
  const TriMeshSDF<T, K, W> tri(mesh, pool, BVH::Construction::SAH, 4);

  const auto drilled = Fused::Translate(Fused::Difference(tri, s.drill), Vec3(T(0.3), T(0), T(0)));

  using D = std::decay_t<decltype(drilled)>;

  pool.freeze();

  const Pool device = Pool::mirror(pool, deviceTestResource());

  requireSameResults(evaluateOnDevice<T>(Query<T, D>{drilled.rebasedView(device)}, points),
                     evaluateOnHost<T>(Query<T, D>{drilled}, points));
}

TEMPLATE_TEST_CASE("Fused: a sphere cloud operand is rebuilt from its layout on a device",
                   "[Fused][SphereCloudBVH][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  const Shapes<T> s;

  const std::vector<Vec3> points = probes<T>();

  // A ball with grains cut out of it: the node rebuilds the cloud from its stored layout and the node's location at
  // every evaluation (SphereCloudBVH's host-device (Layout, PoolLocation) constructor), which a CUDA build compiles
  // with warnings as errors.
  Pool pool(hostMemoryResource());

  std::vector<Vec3> centers;
  std::vector<T>    radii;

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      centers.emplace_back(T(0.4) * T(i) - T(0.6), T(0.4) * T(j) - T(0.6), T(0.1) * T(i + j) - T(0.3));
      radii.push_back(T(0.1) + T(0.02) * T((i + 2 * j) % 3));
    }
  }

  const SphereCloudBVH<T> cloud(pool, centers, radii);

  const auto grained = Fused::Difference(s.ball, cloud);

  using G = std::decay_t<decltype(grained)>;

  pool.freeze();

  const Pool device = Pool::mirror(pool, deviceTestResource());

  requireSameResults(evaluateOnDevice<T>(Query<T, G>{grained.rebasedView(device)}, points),
                     evaluateOnHost<T>(Query<T, G>{grained}, points));
}
