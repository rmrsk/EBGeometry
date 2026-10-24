// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for closestPrimitive(), the deciding leaf of a tape at a point: the classic CSG solid,
// a scene of fused CSG parts (each one leaf of the tape) and a mesh drilled through by a cylinder, in float
// and double, at random points around each object. Each query returns the leaf and the leaf's own value at
// its local point (through visitPrimitive()), so that the checksum depends on both. A host build runs the
// host cases; a CUDA or HIP build runs them as host/device pairs, which agree when they name the same leaf
// or two leaves whose values tie. In a probe of its own, so that adding it changes no other probe's
// translation unit (see Performance/Fused). See ../README.md and PerformanceTesting.rst.

#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// Queries per case, at full and at smoke size; the mesh cases' queries traverse a BVH.
constexpr std::size_t HostQueries     = 400000;
constexpr std::size_t PairQueries     = 1048576;
constexpr std::size_t SmokeQueries    = 1000;
constexpr std::size_t MeshHostQueries = 20000;
constexpr std::size_t MeshPairQueries = 262144;

// The scene of fused parts: ScenePartsPerAxis^3 copies of the classic CSG solid, ScenePitch apart.
constexpr std::size_t ScenePartsPerAxis = 2;
constexpr double      ScenePitch        = 3.0;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

// A query's answer: the deciding leaf and its own value at its local point.
template <class T>
struct LeafHit
{
  uint32_t m_leaf;
  T        m_value;
};

// A composition's deciding leaf at a point, with the leaf's value there.
template <class T, class Shape>
struct Leaf
{
  Shape m_shape;

  EBGEOMETRY_HOST_DEVICE
  LeafHit<T>
  operator()(const Vec3<T>& a_point) const noexcept
  {
    const EBGeometry::PrimitiveRef<T> ref = m_shape.closestPrimitive(a_point);

    T value = T(0);

    m_shape.visitPrimitive(
      ref, [&value, &ref](const auto& a_leaf) noexcept { value = a_leaf.signedDistance(ref.m_localPoint); });

    return LeafHit<T>{ref.m_leaf, value};
  }
};

// The sum of the leaves' numbers and values.
template <class T>
double
checksumOf(const std::vector<LeafHit<T>>& a_hits)
{
  double sum = 0.0;

  for (const auto& h : a_hits) {
    sum += static_cast<double>(h.m_leaf) + static_cast<double>(h.m_value);
  }

  return sum;
}

// A host and a device answer agree when their leaves' values are within Perf::agreementTolerance():
// the same leaf, or two whose values tie.
template <class T>
std::pair<double, bool>
agreeLeaf(const LeafHit<T>& a_host, const LeafHit<T>& a_device)
{
  const double vh   = static_cast<double>(a_host.m_value);
  const double diff = std::abs(vh - static_cast<double>(a_device.m_value));

  return {diff, diff > Perf::agreementTolerance<T>(std::abs(vh))};
}

// The classic CSG solid of the Tape probe: a cube with its corners cut by a ball, drilled through along x, y
// and z (five leaves).
template <class T>
EBGeometry::Expr<T>
classicCSG()
{
  const EBGeometry::Expr<T> cube = EBGeometry::BoxSDF<T>(Vec3<T>(T(-1), T(-1), T(-1)), Vec3<T>(T(1), T(1), T(1)));
  const EBGeometry::Expr<T> ball = EBGeometry::SphereSDF<T>(Vec3<T>::zeros(), T(1.35));

  std::vector<EBGeometry::Expr<T>> drills;

  for (std::size_t axis = 0; axis < 3; axis++) {
    drills.push_back(EBGeometry::InfiniteCylinderSDF<T>(Vec3<T>::zeros(), T(0.55), axis));
  }

  return EBGeometry::Difference(EBGeometry::Intersection(cube, ball), EBGeometry::Union(drills));
}

// The same solid, fused.
template <class T>
auto
fusedCSG()
{
  const EBGeometry::BoxSDF<T>    cube(Vec3<T>(T(-1), T(-1), T(-1)), Vec3<T>(T(1), T(1), T(1)));
  const EBGeometry::SphereSDF<T> ball(Vec3<T>::zeros(), T(1.35));

  const EBGeometry::InfiniteCylinderSDF<T> drill0(Vec3<T>::zeros(), T(0.55), 0);
  const EBGeometry::InfiniteCylinderSDF<T> drill1(Vec3<T>::zeros(), T(0.55), 1);
  const EBGeometry::InfiniteCylinderSDF<T> drill2(Vec3<T>::zeros(), T(0.55), 2);

  return EBGeometry::Fused::Difference(EBGeometry::Fused::Intersection(cube, ball), drill0, drill1, drill2);
}

// Random points in a box grown by a quarter of its size on every side.
template <class T>
std::vector<Vec3<T>>
pointsAround(const EBGeometry::BoundingVolumes::AABBT<T>& a_box, std::size_t a_n)
{
  const Vec3<T> grow = T(0.25) * (a_box.getHighCorner() - a_box.getLowCorner());

  return Perf::pointsInBox<T>(
    EBGeometry::BoundingVolumes::AABBT<T>(a_box.getLowCorner() - grow, a_box.getHighCorner() + grow),
    a_n,
    Perf::Seed::TapeQueries);
}

// The cases for one composition: closestPrimitive() on the host and as a host/device pair. a_build(case,
// pool) returns the tape and the box its points are drawn around.
template <class T, class Build>
void
addComposition(
  Perf::Suite& a_suite, const std::string& a_input, const std::size_t a_host, const std::size_t a_pair, Build a_build)
{
  using Shape = typename std::invoke_result_t<Build, Perf::Case&, EBGeometry::Pool&>::first_type;

  const std::string prec = Perf::precisionName<T>();

  a_suite.add(Perf::caseName({"ClosestPrimitive", a_input, "closestPrimitive", prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const auto              made   = a_build(c, pool);
    const auto              points = pointsAround(made.second, c.size(a_host, SmokeQueries));
    std::vector<LeafHit<T>> hits;

    Perf::measureHostQuery(c, Leaf<T, Shape>{made.first}, points, hits);

    c.checksum(checksumOf(hits));
    c.poolBytes(pool.usedBytes());
  });

  a_suite.add(Perf::caseName({"ClosestPrimitive", a_input, "closestPrimitive", prec, "hostdevice"}),
              [=](Perf::Case& c) {
                EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

                const auto made   = a_build(c, pool);
                const auto points = pointsAround(made.second, c.size(a_pair, SmokeQueries));

                const auto hits = Perf::measureHostAndDevice<LeafHit<T>>(
                  c,
                  pool,
                  points,
                  [&](const EBGeometry::Pool& a_pool) { return Leaf<T, Shape>{made.first.rebasedView(a_pool)}; },
                  agreeLeaf<T>);

                c.checksum(checksumOf(hits));
              });
}

template <class T>
void
addPrecision(Perf::Suite& a_suite)
{
  using AABB = EBGeometry::BoundingVolumes::AABBT<T>;
  using Mesh = EBGeometry::TriMeshSDF<T, 4, 4>;
  using Part = decltype(fusedCSG<T>());

  const AABB cube(Vec3<T>(T(-1), T(-1), T(-1)), Vec3<T>(T(1), T(1), T(1)));

  // The classic CSG solid through a tape: five leaves.
  addComposition<T>(a_suite, "csg", HostQueries, PairQueries, [cube](Perf::Case& c, EBGeometry::Pool& a_pool) {
    const EBGeometry::Tape<T> tape(a_pool, classicCSG<T>());

    c.param("clauses", tape.getNumClauses());

    return std::make_pair(tape, cube);
  });

  // A scene of ScenePartsPerAxis^3 copies of the classic solid, each fused: one leaf per part, so that the
  // tape interprets only the union of the parts and each part is evaluated as compiled code.
  const T    half = T(0.5) * T(ScenePitch) * T(ScenePartsPerAxis);
  const AABB sceneBox(-half * Vec3<T>::ones(), half * Vec3<T>::ones());

  addComposition<T>(a_suite, "parts", HostQueries, PairQueries, [sceneBox](Perf::Case& c, EBGeometry::Pool& a_pool) {
    const T                          centre = T(0.5) * T(ScenePartsPerAxis - 1);
    std::vector<EBGeometry::Expr<T>> parts;

    for (std::size_t i = 0; i < ScenePartsPerAxis; i++) {
      for (std::size_t j = 0; j < ScenePartsPerAxis; j++) {
        for (std::size_t k = 0; k < ScenePartsPerAxis; k++) {
          const Vec3<T> at = T(ScenePitch) * (Vec3<T>(T(i), T(j), T(k)) - centre * Vec3<T>::ones());

          parts.push_back(EBGeometry::Translate(EBGeometry::Expr<T>(fusedCSG<T>()), at));
        }
      }
    }

    const EBGeometry::Tape<T, Part> tape(a_pool, EBGeometry::Union(parts));

    c.param("parts", parts.size());
    c.param("clauses", tape.getNumClauses());

    return std::make_pair(tape, sceneBox);
  });

  // The armadillo (the dodecahedron under --smoke) drilled through along x, through a tape.
  addComposition<T>(
    a_suite, "drilled-mesh", MeshHostQueries, MeshPairQueries, [](Perf::Case& c, EBGeometry::Pool& a_pool) {
      const Mesh    mesh = EBGeometry::Parser::readIntoTriMeshSDF<T, 4, 4>(Perf::meshFile(c.smoke()), a_pool);
      const AABB    box  = mesh.computeBoundingVolume();
      const Vec3<T> size = box.getHighCorner() - box.getLowCorner();

      const EBGeometry::InfiniteCylinderSDF<T> drill(
        T(0.5) * (box.getLowCorner() + box.getHighCorner()), T(0.12) * size[1], 0);

      c.param("K", 4);
      c.param("W", 4);

      return std::make_pair(EBGeometry::Tape<T, Mesh>(a_pool, EBGeometry::Difference(EBGeometry::Expr<T>(mesh), drill)),
                            box);
    });
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "ClosestPrimitive");

  addPrecision<float>(suite);
  addPrecision<double>(suite);

  return suite.run();
}
