// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for EBGeometry_Tape.hpp: the compiled form of an expression. Covers what the transform
// and CSG tests do not: every primitive type the tape stores (analytic shapes, the three mesh SDFs,
// user types including a BVH union and another tape), unions of objects of different types with and
// without a BVH, the handle operations (layout, rebasedView onto a mirror, deepCopy), the metadata the
// compiler records, and the limits the constructor enforces.

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"
#include "TestTapeUtils.hpp"

#include <cmath>
#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;

namespace {

std::string
dodecahedron()
{
  return std::string(EBGEOMETRY_TEST_DATA_DIR) + "/dodecahedron.stl";
}

template <class T>
std::vector<Vec3T<T>>
probes()
{
  std::vector<Vec3T<T>> points;

  for (int i = -3; i <= 3; i++) {
    for (int j = -2; j <= 2; j++) {
      points.emplace_back(T(0.7) * T(i) + T(0.05), T(0.6) * T(j) - T(0.1), T(0.3) * T(i * j) + T(0.2));
    }
  }

  return points;
}

// A shape type the tape does not know unless it is listed: a sphere under another name.
template <class T>
struct OtherSphere
{
  SphereSDF<T> m_sphere;

  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    return m_sphere.signedDistance(a_point);
  }

  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    return m_sphere.computeBoundingVolume();
  }
};

// Device functor: a tape's value at one point.
template <class T, class TapeType>
struct TapeQuery
{
  TapeType m_tape;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return m_tape.signedDistance(a_point);
  }
};

} // namespace

TEMPLATE_TEST_CASE("Tape: a trivially copyable pool handle, and a shape itself", "[Tape]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  STATIC_REQUIRE(std::is_trivially_copyable_v<Tape<T>>);
  STATIC_REQUIRE(std::is_trivially_copyable_v<typename Tape<T>::Layout>);
  STATIC_REQUIRE(isPoolHandle<Tape<T>>);
  STATIC_REQUIRE_FALSE(isPoolHandle<typename Tape<T>::Layout>);
  STATIC_REQUIRE(isShape<Tape<T>>);
  STATIC_REQUIRE(sizeof(Tape<T>) == sizeof(typename Tape<T>::Layout) + sizeof(PoolLocation));
  STATIC_REQUIRE_FALSE(isShape<int>);
  STATIC_REQUIRE(isShape<OtherSphere<T>>);
}

TEMPLATE_TEST_CASE("Tape: a single analytic shape evaluates exactly like the shape",
                   "[Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const PlaneSDF<T>            plane(Vec3(T(0.1), T(0.2), T(0.3)), Vec3(T(1), T(2), T(-1)));
  const SphereSDF<T>           sphere(Vec3(T(0.2), T(-0.1), T(0.3)), T(0.8));
  const BoxSDF<T>              box(Vec3(T(-1), T(-0.5), T(-0.25)), Vec3(T(0.5), T(1), T(0.75)));
  const TorusSDF<T>            torus(Vec3(T(0.1), T(0), T(-0.2)), T(1), T(0.25));
  const CylinderSDF<T>         cylinder(Vec3(T(-1), T(0), T(0)), Vec3(T(1), T(0.5), T(0)), T(0.4));
  const InfiniteCylinderSDF<T> infiniteCylinder(Vec3(T(0.1), T(0.2), T(0.3)), T(0.5), 2);
  const CapsuleSDF<T>          capsule(Vec3(T(-1), T(0), T(0)), Vec3(T(1), T(0.5), T(0)), T(0.4));
  const InfiniteConeSDF<T>     infiniteCone(Vec3(T(0), T(1), T(0)), T(30));
  const ConeSDF<T>             cone(Vec3(T(0), T(1), T(0)), T(2), T(30));
  const RoundedBoxSDF<T>       roundedBox(Vec3(T(0.1), T(0), T(0)), Vec3(T(1.2), T(1), T(0.8)), T(0.1));
  const RoundedCylinderSDF<T>  roundedCylinder(Vec3(T(0), T(0.1), T(0)), T(0.5), T(0.1), T(1));
  const PerlinSDF<T>           perlin(T(0.5), T(2) * Vec3::ones(), T(0.5), 4);

  const auto check = [](const auto& a_shape) {
    const TestUtils::Compiled<T> tape(a_shape);

    CHECK(tape.tape().getNumClauses() == 1U);

    for (const auto& p : probes<T>()) {
      CHECK(tape(p) == a_shape.signedDistance(p));
    }
  };

  check(plane);
  check(sphere);
  check(box);
  check(torus);
  check(cylinder);
  check(infiniteCylinder);
  check(capsule);
  check(infiniteCone);
  check(cone);
  check(roundedBox);
  check(roundedCylinder);
  check(perlin);
}

TEMPLATE_TEST_CASE("Tape: a union of a mesh SDF and a sphere is the smaller of the two distances everywhere",
                   "[Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = BVH::DefaultBranchingRatio<T>();
  constexpr size_t W = TriangleSoA::DefaultWidth<T>();

  Pool pool(hostMemoryResource());

  const auto mesh = Parser::readIntoDCEL<T>(dodecahedron(), pool);

  const FlatMeshSDF<T>      flat(mesh, pool);
  const MeshSDF<T, K>       bvhMesh(mesh, pool, BVH::Construction::SAH);
  const TriMeshSDF<T, K, W> triMesh(mesh, pool, BVH::Construction::SAH, 4);
  const SphereSDF<T>        sphere(Vec3(T(0.8), T(0.2), T(-0.1)), T(0.5));

  const Tape<T> flatUnion(pool, Union(flat, sphere));
  const Tape<T> meshUnion(pool, Union(bvhMesh, sphere));
  const Tape<T> triUnion(pool, Union(triMesh, sphere));

  for (const auto& p : probes<T>()) {
    CHECK(flatUnion.signedDistance(p) == Math::min(flat.signedDistance(p), sphere.signedDistance(p)));
    CHECK(meshUnion.signedDistance(p) == Math::min(bvhMesh.signedDistance(p), sphere.signedDistance(p)));
    CHECK(triUnion.signedDistance(p) == Math::min(triMesh.signedDistance(p), sphere.signedDistance(p)));
  }
}

TEMPLATE_TEST_CASE("Tape: a union of many objects of different types matches brute force with and without a BVH",
                   "[Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = BVH::DefaultBranchingRatio<T>();

  Pool pool(hostMemoryResource());

  const auto          mesh = Parser::readIntoDCEL<T>(dodecahedron(), pool);
  const MeshSDF<T, K> meshSDF(mesh, pool, BVH::Construction::SAH);

  std::vector<Expr<T>> objects;

  objects.reserve(18);

  for (int i = 0; i < 6; i++) {
    const Vec3 shift(T(3) * T(i), T(0), T(0));

    objects.emplace_back(SphereSDF<T>(shift + Vec3(T(0), T(2), T(0)), T(0.5)));
    objects.emplace_back(BoxSDF<T>(shift - T(0.4) * Vec3::ones(), shift + T(0.4) * Vec3::ones()));
    objects.emplace_back(Translate(meshSDF, shift + Vec3(T(0), T(-2.5), T(0))));
  }

  const auto bruteForce = [&objects, &pool](const Vec3& a_point) {
    T minimum = Math::Limits<T>::infinity();

    for (const auto& object : objects) {
      minimum = Math::min(minimum, Tape<T>(pool, object).signedDistance(a_point));
    }

    return minimum;
  };

  TapeOptions linearOptions;

  linearOptions.m_bvhUnionThreshold = 1000;

  const Tape<T> withBVH(pool, Union(objects));
  const Tape<T> linear(pool, Union(objects), linearOptions);

  CHECK(withBVH.getNumBVHUnions() == 1U);
  CHECK(linear.getNumBVHUnions() == 0U);
  CHECK(withBVH.getNumPrograms() == 1U + objects.size());
  CHECK(withBVH.getCallDepth() == 1U);
  CHECK(linear.getCallDepth() == 0U);

  for (const auto& p : probes<T>()) {
    const T expected = bruteForce(p);

    CHECK(withBVH.signedDistance(p) == expected);
    CHECK(linear.signedDistance(p) == expected);
  }
}

TEMPLATE_TEST_CASE("Tape: user primitive types, including a BVH union and another tape",
                   "[Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T      = TestType;
  using Vec3   = Vec3T<T>;
  using Union4 = BVHUnion<T, SphereSDF<T>, 4>;

  Pool pool(hostMemoryResource());

  std::vector<SphereSDF<T>>              spheres;
  std::vector<BoundingVolumes::AABBT<T>> boxes;

  for (int i = 0; i < 20; i++) {
    spheres.emplace_back(Vec3(T(i), T(0), T(0)), T(0.3));
    boxes.push_back(spheres.back().computeBoundingVolume());
  }

  const Union4 row(pool, spheres, boxes);

  const OtherSphere<T> other{SphereSDF<T>(Vec3(T(0), T(1.5), T(0)), T(0.5))};

  const Tape<T, Union4, OtherSphere<T>> outer(pool, Union(row, Offset(other, T(0.1))));

  // A tape is a pool-resident shape, so another tape stores it as a layout.
  const Tape<T> inner(pool, Difference(BoxSDF<T>(-Vec3::ones(), Vec3::ones()), SphereSDF<T>(Vec3::zeros(), T(1.2))));
  const Tape<T, Tape<T>>          nested(pool, Translate(inner, Vec3(T(5), T(0), T(0))));
  const Tape<T, Tape<T, Tape<T>>> twice(pool, Scale(nested, T(2)));

  for (const auto& p : probes<T>()) {
    CHECK(outer.signedDistance(p) == Math::min(row.signedDistance(p), other.signedDistance(p) - T(0.1)));
    CHECK(nested.signedDistance(p) == inner.signedDistance(p - Vec3(T(5), T(0), T(0))));
    CHECK_THAT(twice.signedDistance(p), withinAbsT(T(2) * nested.signedDistance(p / T(2)), tightMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("Tape: a handle rebuilt from its layout, a view on a mirror and a deep copy all answer alike",
                   "[Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = BVH::DefaultBranchingRatio<T>();

  Pool pool(hostMemoryResource());

  const auto          mesh = Parser::readIntoDCEL<T>(dodecahedron(), pool);
  const MeshSDF<T, K> meshSDF(mesh, pool, BVH::Construction::SAH);

  std::vector<Expr<T>> objects;

  objects.reserve(11);

  for (int i = 0; i < 10; i++) {
    objects.emplace_back(SphereSDF<T>(Vec3(T(i), T(1.5), T(0)), T(0.4)));
  }

  objects.emplace_back(meshSDF);

  const Tape<T> tape(pool, Blur(SmoothUnion(objects, T(0.2)), T(0.05)));
  const Tape<T> again(tape.layout(), tape.location());

  CHECK(tape.getCallDepth() == 2U);

  std::vector<T> expected;

  for (const auto& p : probes<T>()) {
    expected.push_back(tape.signedDistance(p));

    CHECK(again.signedDistance(p) == expected.back());
  }

  // A deep copy into another pool keeps working after the source pool is gone.
  Pool       copyPool(hostMemoryResource());
  const auto copy = tape.deepCopy(copyPool);

  pool.freeze();

  const Pool mirror = Pool::mirror(pool, hostMemoryResource());
  const auto view   = tape.rebasedView(mirror);

  for (size_t i = 0; i < probes<T>().size(); i++) {
    const Vec3 p = probes<T>()[i];

    CHECK(view.signedDistance(p) == expected[i]);
    CHECK(copy.signedDistance(p) == expected[i]);
  }
}

TEMPLATE_TEST_CASE("Tape: the bounding volume and distance quality are the expression's",
                   "[Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  const SphereSDF<T> sphere(Vec3(T(1), T(0), T(0)), T(0.5));
  const BoxSDF<T>    box(-Vec3::ones(), Vec3::ones());
  const PerlinSDF<T> perlin(T(0.5), T(2) * Vec3::ones(), T(0.5), 4);

  const Expr<T> shifted = Translate(sphere, Vec3(T(0), T(2), T(0)));
  const Tape<T> shiftedTape(pool, shifted);

  CHECK(shiftedTape.getDistanceQuality() == DistanceQuality::Exact);
  CHECK(shiftedTape.computeBoundingVolume().getLowCorner() == shifted.computeBoundingVolume().getLowCorner());
  CHECK(shiftedTape.computeBoundingVolume().getHighCorner() == shifted.computeBoundingVolume().getHighCorner());
  CHECK_THAT(shifted.computeBoundingVolume().getLowCorner()[1], withinAbsT(T(1.5), tightMargin<T>()));
  CHECK_THAT(shifted.computeBoundingVolume().getHighCorner()[1], withinAbsT(T(2.5), tightMargin<T>()));

  CHECK(Tape<T>(pool, Union(sphere, box)).getDistanceQuality() == DistanceQuality::Bound);
  CHECK(Tape<T>(pool, Union(sphere, perlin)).getDistanceQuality() == DistanceQuality::NotADistance);
  CHECK(Tape<T>(pool, Complement(sphere)).getDistanceQuality() == DistanceQuality::Exact);
  CHECK(Tape<T>(pool, Offset(sphere, T(-0.1))).getDistanceQuality() == DistanceQuality::Bound);
  CHECK(Tape<T>(pool, Elongate(sphere, Vec3::ones())).getDistanceQuality() == DistanceQuality::Bound);

  // A union with an object that is not a distance cannot be pruned, so it gets no BVH.
  std::vector<Expr<T>> objects(10, Expr<T>(sphere));

  objects.emplace_back(perlin);

  CHECK(Tape<T>(pool, Union(objects)).getNumBVHUnions() == 0U);

  // The difference keeps the box of the object subtracted from; the intersection that of the overlap.
  const auto difference = Difference(box, sphere).computeBoundingVolume();
  const auto overlap    = Intersection(box, sphere).computeBoundingVolume();

  CHECK(difference.getLowCorner() == box.computeBoundingVolume().getLowCorner());
  CHECK(difference.getHighCorner() == box.computeBoundingVolume().getHighCorner());
  CHECK(overlap.getLowCorner() == Vec3(T(0.5), T(-0.5), T(-0.5)));
  CHECK(overlap.getHighCorner() == Vec3(T(1), T(0.5), T(0.5)));
}

TEMPLATE_TEST_CASE("Tape: an intersection of disjoint objects is empty, and a BVH union of it still evaluates it",
                   "[Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  const SphereSDF<T> left(Vec3::zeros(), T(1));
  const SphereSDF<T> right(Vec3(T(3), T(0), T(0)), T(1));

  const Expr<T> empty = Intersection(left, right);

  // The box is empty, which an inverted box would have been rejected as.
  CHECK(!(empty.computeBoundingVolume().getLowCorner()[0] <= empty.computeBoundingVolume().getHighCorner()[0]));

  std::vector<Expr<T>> objects(1, empty);

  for (int i = 0; i < 10; i++) {
    objects.emplace_back(SphereSDF<T>(Vec3(T(i), T(5), T(0)), T(0.3)));
  }

  TapeOptions linearOptions;

  linearOptions.m_bvhUnionThreshold = 1000;

  const Tape<T> withBVH(pool, Union(objects));
  const Tape<T> linear(pool, Union(objects), linearOptions);

  CHECK(withBVH.getNumBVHUnions() == 1U);

  for (const auto& p : probes<T>()) {
    CHECK(Tape<T>(pool, empty).signedDistance(p) == Math::max(left.signedDistance(p), right.signedDistance(p)));
    CHECK(withBVH.signedDistance(p) == linear.signedDistance(p));
  }
}

TEMPLATE_TEST_CASE("Tape: a BVH union of Bound operands keeps the sign and never undercuts the plain minimum",
                   "[Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  // Elongated spheres are only bounds: inside the elongated core the value underestimates the depth, and
  // a shrinking offset makes every value smaller than the distance outside.
  std::vector<Expr<T>> objects;

  objects.reserve(12);

  for (int i = 0; i < 12; i++) {
    objects.emplace_back(
      Offset(Elongate(SphereSDF<T>(Vec3(T(1.5) * T(i), T(0), T(0)), T(0.5)), Vec3(T(0.3), T(0), T(0))), T(-0.2)));
  }

  TapeOptions linearOptions;

  linearOptions.m_bvhUnionThreshold = 1000;

  const Tape<T> withBVH(pool, Union(objects));
  const Tape<T> linear(pool, Union(objects), linearOptions);

  CHECK(withBVH.getNumBVHUnions() == 1U);
  CHECK(withBVH.getDistanceQuality() == DistanceQuality::Bound);

  for (const auto& p : probes<T>()) {
    const T bvh   = withBVH.signedDistance(p);
    const T plain = linear.signedDistance(p);

    // The BVH evaluates a subset of the operands: never below the plain minimum, and of the same sign.
    CHECK(bvh >= plain);
    CHECK((bvh < T(0)) == (plain < T(0)));
  }
}

TEMPLATE_TEST_CASE("Tape: calls may nest exactly MaxCallDepth deep", "[Tape]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  const SphereSDF<T> sphere(Vec3::zeros(), T(1));

  Expr<T> expr = sphere;

  for (unsigned i = 0; i < Tape<T>::MaxCallDepth; i++) {
    expr = Blur(expr, T(0.01));
  }

  const Tape<T> tape(pool, expr);

  CHECK(tape.getCallDepth() == Tape<T>::MaxCallDepth);
  CHECK(tape.getNumPrograms() == Tape<T>::MaxCallDepth + 1U);
  CHECK(std::isfinite(tape.signedDistance(Vec3(T(2), T(0), T(0)))));
}

TEST_CASE("Tape: rejects an unknown primitive type, a mesh from another pool and an expression that nests too deeply",
          "[Tape][death]")
{
  using T    = double;
  using Vec3 = Vec3T<T>;

  // Expressions one level past each limit, built outside the death-test lambdas.
  Expr<T> deepBlur = SphereSDF<T>(Vec3::zeros(), T(1));

  for (unsigned i = 0; i <= Tape<T>::MaxCallDepth; i++) {
    deepBlur = Blur(deepBlur, T(0.01));
  }

  Expr<T> deepMaps = SphereSDF<T>(Vec3::zeros(), T(1));

  for (unsigned i = 0; i < Tape<T>::PointStackSize; i++) {
    deepMaps = Elongate(deepMaps, T(0.01) * Vec3::ones());
  }

  Expr<T> deepValues = SphereSDF<T>(Vec3::zeros(), T(1));

  for (unsigned i = 0; i < Tape<T>::ValueStackSize; i++) {
    deepValues = Intersection(SphereSDF<T>(Vec3::ones(), T(1)), deepValues);
  }

  REQUIRE(abortsWith(
    [] {
      Pool pool(hostMemoryResource());

      const OtherSphere<T> other{SphereSDF<T>(Vec3::zeros(), T(1))};
      const Tape<T>        tape(pool, Union(other, SphereSDF<T>(Vec3::ones(), T(1))));
    },
    "Tape: a shape's type is not one of the tape's primitive types"));

  REQUIRE(abortsWith(
    [] {
      Pool meshPool(hostMemoryResource());
      Pool tapePool(hostMemoryResource());

      const auto           mesh = Parser::readIntoDCEL<T>(dodecahedron(), meshPool);
      const FlatMeshSDF<T> flat(mesh, meshPool);
      const Tape<T>        tape(tapePool, Union(flat, SphereSDF<T>(Vec3::ones(), T(1))));
    },
    "Tape: a pool-resident shape (a mesh SDF, a BVH union or another tape) lives in another Pool"));

  REQUIRE(abortsWith(
    [&deepBlur] {
      Pool pool(hostMemoryResource());

      const Tape<T> tape(pool, deepBlur);
    },
    "Tape: Blur, Mollify and BVH unions nest 5 deep, but a tape allows at most 4"));

  REQUIRE(abortsWith(
    [&deepMaps] {
      Pool pool(hostMemoryResource());

      const Tape<T> tape(pool, deepMaps);
    },
    "Tape: the expression nests 8 point maps"));

  REQUIRE(abortsWith(
    [&deepValues] {
      Pool pool(hostMemoryResource());

      const Tape<T> tape(pool, deepValues);
    },
    "Tape: the expression needs 17 values at once in one sub-program"));

  REQUIRE(abortsWith(
    [] {
      Pool        pool(hostMemoryResource());
      TapeOptions options;

      options.m_bvhUnionThreshold = 1;

      const Tape<T> tape(pool, SphereSDF<T>(Vec3::zeros(), T(1)), options);
    },
    "Tape: the BVH union threshold must be at least 2 (1)"));
}

TEMPLATE_TEST_CASE("Tape: device evaluation matches the host for every operation",
                   "[Tape][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T      = TestType;
  using Vec3   = Vec3T<T>;
  using Union4 = BVHUnion<T, SphereSDF<T>, 4>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  constexpr size_t K = BVH::DefaultBranchingRatio<T>();
  constexpr size_t W = TriangleSoA::DefaultWidth<T>();

  Pool pool(hostMemoryResource());

  // Every built-in primitive type: the twelve analytic shapes and the three mesh SDFs.
  const auto mesh = Parser::readIntoDCEL<T>(dodecahedron(), pool);

  const FlatMeshSDF<T>      flat(mesh, pool);
  const MeshSDF<T, K>       bvhMesh(mesh, pool, BVH::Construction::SAH);
  const TriMeshSDF<T, K, W> triMesh(mesh, pool, BVH::Construction::SAH, 4);

  const PlaneSDF<T>            plane(Vec3(T(0.1), T(0.2), T(0.3)), Vec3(T(1), T(2), T(-1)));
  const SphereSDF<T>           sphere(Vec3(T(0.2), T(-0.1), T(0.3)), T(0.8));
  const BoxSDF<T>              box(Vec3(T(-1), T(-0.5), T(-0.25)), Vec3(T(0.5), T(1), T(0.75)));
  const TorusSDF<T>            torus(Vec3(T(0.1), T(0), T(-0.2)), T(1), T(0.25));
  const CylinderSDF<T>         cylinder(Vec3(T(-1), T(0), T(0)), Vec3(T(1), T(0.5), T(0)), T(0.4));
  const InfiniteCylinderSDF<T> infiniteCylinder(Vec3(T(0.1), T(0.2), T(0.3)), T(0.5), 2);
  const CapsuleSDF<T>          capsule(Vec3(T(-1), T(0), T(0)), Vec3(T(1), T(0.5), T(0)), T(0.4));
  const InfiniteConeSDF<T>     infiniteCone(Vec3(T(0), T(1), T(0)), T(30));
  const ConeSDF<T>             cone(Vec3(T(0), T(1), T(0)), T(2), T(30));
  const RoundedBoxSDF<T>       roundedBox(Vec3(T(0.1), T(0), T(0)), Vec3(T(1.2), T(1), T(0.8)), T(0.1));
  const RoundedCylinderSDF<T>  roundedCylinder(Vec3(T(0), T(0.1), T(0)), T(0.5), T(0.1), T(1));
  const PerlinSDF<T>           perlin(T(0.5), T(2) * Vec3::ones(), T(0.5), 4);

  std::vector<Expr<T>> primitives{plane,
                                  sphere,
                                  box,
                                  torus,
                                  cylinder,
                                  infiniteCylinder,
                                  capsule,
                                  infiniteCone,
                                  cone,
                                  roundedBox,
                                  roundedCylinder,
                                  Translate(flat, Vec3(T(3), T(0), T(0))),
                                  Translate(bvhMesh, Vec3(T(-3), T(0), T(0))),
                                  Translate(triMesh, Vec3(T(0), T(3), T(0)))};

  // One tape per operation, so a failure names it, and one per primitive.
  std::vector<std::pair<std::string, Expr<T>>> exprs;

  for (size_t i = 0; i < primitives.size(); i++) {
    exprs.emplace_back("primitive " + std::to_string(i), primitives[i]);
  }

  exprs.emplace_back("perlin", Offset(perlin, T(0.45)));

  TapeOptions linear;

  linear.m_bvhUnionThreshold = 1000;

  exprs.emplace_back("union", Union(primitives));

  const size_t unionIndex = exprs.size() - 1;

  exprs.emplace_back("smooth union", SmoothUnion(primitives, T(0.3)));
  exprs.emplace_back("exponential smooth union", SmoothUnion(sphere, box, T(0.3), BlendKind::Exponential));
  exprs.emplace_back("intersection", Intersection(sphere, box));
  exprs.emplace_back("smooth intersection", SmoothIntersection(sphere, torus, T(0.2)));
  exprs.emplace_back("exponential smooth intersection",
                     SmoothIntersection(sphere, torus, T(0.2), BlendKind::Exponential));
  exprs.emplace_back("difference", Difference(box, sphere));
  exprs.emplace_back("smooth difference", SmoothDifference(box, std::vector<Expr<T>>{sphere, capsule}, T(0.2)));
  exprs.emplace_back("complement", Complement(cone));
  exprs.emplace_back("affine chain",
                     Translate(Rotate(Scale(Reflect(roundedBox, 0), T(1.5)), T(30), Vec3(T(1), T(1), T(0))),
                               Vec3(T(0.2), T(0.1), T(0))));
  exprs.emplace_back("offset and annular", Annular(Offset(capsule, T(0.1)), T(0.05)));
  exprs.emplace_back("elongate", Elongate(torus, Vec3(T(0.5), T(0), T(0.25))));
  exprs.emplace_back(
    "finite repetition",
    FiniteRepetition(
      SphereSDF<T>(Vec3::zeros(), T(0.3)), Vec3(T(1), T(1), T(1)), Vec3(T(2), T(1), T(0)), Vec3(T(2), T(1), T(0))));
  exprs.emplace_back("blur", Blur(Difference(box, sphere), T(0.1)));
  exprs.emplace_back("mollify", Mollify(Intersection(box, sphere), T(0.1), 3));

  // Calls nested to the limit: each union of 8 operands is a BVH union whose first operand is the
  // next one in.
  Expr<T> nestedUnions = sphere;

  for (unsigned level = 0; level < Tape<T>::MaxCallDepth; level++) {
    std::vector<Expr<T>> operands{nestedUnions};

    for (int i = 1; i < 8; i++) {
      operands.emplace_back(SphereSDF<T>(Vec3(T(i), T(level), T(0)), T(0.3)));
    }

    nestedUnions = Union(operands);
  }

  const size_t nestedUnionsIndex = exprs.size();

  exprs.emplace_back("unions nested MaxCallDepth deep", nestedUnions);

  // The full value and point stacks.
  Expr<T> deepValues = sphere;
  Expr<T> deepMaps   = torus;

  for (unsigned i = 0; i + 1 < Tape<T>::ValueStackSize; i++) {
    deepValues = Intersection(SphereSDF<T>(T(0.01) * T(i) * Vec3::ones(), T(1)), deepValues);
  }

  for (unsigned i = 0; i + 1 < Tape<T>::PointStackSize; i++) {
    deepMaps = Elongate(deepMaps, T(0.02) * Vec3::ones());
  }

  exprs.emplace_back("full value stack", deepValues);
  exprs.emplace_back("full point stack", deepMaps);

  std::vector<std::pair<std::string, Tape<T>>> tapes;

  tapes.reserve(exprs.size() + 1);

  for (const auto& [name, expr] : exprs) {
    tapes.emplace_back(name, Tape<T>(pool, expr));
  }

  tapes.emplace_back("union without a BVH", Tape<T>(pool, Union(primitives), linear));

  // User primitive types: a BVH union, another shape type, and a tape inside a tape.
  std::vector<SphereSDF<T>>              rowSpheres;
  std::vector<BoundingVolumes::AABBT<T>> rowBoxes;

  for (int i = 0; i < 20; i++) {
    rowSpheres.emplace_back(Vec3(T(0.5) * T(i) - T(5), T(0), T(0)), T(0.3));
    rowBoxes.push_back(rowSpheres.back().computeBoundingVolume());
  }

  const Union4         row(pool, rowSpheres, rowBoxes);
  const OtherSphere<T> other{SphereSDF<T>(Vec3(T(0), T(1.5), T(0)), T(0.5))};

  const Tape<T, Union4, OtherSphere<T>> user(pool, SmoothUnion(row, Offset(other, T(0.1)), T(0.2)));
  const Tape<T>                         inner(pool, Difference(box, sphere));
  const Tape<T, Tape<T>>                nested(pool, Rotate(Translate(inner, Vec3::ones()), T(45), 1));

  CHECK(tapes[0].second.getNumClauses() == 1U);
  CHECK(tapes[unionIndex].second.getNumBVHUnions() == 1U);
  CHECK(tapes[exprs.size()].second.getNumBVHUnions() == 0U);
  CHECK(tapes[nestedUnionsIndex].second.getCallDepth() == Tape<T>::MaxCallDepth);

  pool.freeze();

  const Pool devicePool = Pool::mirror(pool, deviceTestResource());

  const auto points = queryGrid<T>(Vec3(T(-4), T(-4), T(-4)), Vec3(T(4), T(4), T(4)), 12);

  const auto check = [&points, &devicePool](const auto& a_tape) {
    using TapeType = std::decay_t<decltype(a_tape)>;

    const TapeType view = a_tape.rebasedView(devicePool);

    requireSameResults(evaluateOnDevice<T>(TapeQuery<T, TapeType>{view}, points),
                       evaluateOnHost<T>(TapeQuery<T, TapeType>{a_tape}, points));
  };

  for (const auto& [name, tape] : tapes) {
    INFO(name);

    check(tape);
  }

  check(user);
  check(nested);
}

namespace {

// A user shape that is not a renamed built-in: a regular octahedron |x| + |y| + |z| <= s, with an exact
// distance (after Inigo Quilez's formula).
template <class T>
struct Octahedron
{
  T m_size = T(1);

  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    const T x = std::abs(a_point[0]);
    const T y = std::abs(a_point[1]);
    const T z = std::abs(a_point[2]);
    const T m = x + y + z - m_size;

    T q0;
    T q1;
    T q2;

    if (T(3) * x < m) {
      q0 = x;
      q1 = y;
      q2 = z;
    }
    else if (T(3) * y < m) {
      q0 = y;
      q1 = z;
      q2 = x;
    }
    else if (T(3) * z < m) {
      q0 = z;
      q1 = x;
      q2 = y;
    }
    else {
      return m / std::sqrt(T(3));
    }

    const T k  = Math::clamp(T(0.5) * (q2 - q1 + m_size), T(0), m_size);
    const T d1 = q1 - m_size + k;
    const T d2 = q2 - k;

    return std::sqrt(q0 * q0 + d1 * d1 + d2 * d2);
  }

  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    return BoundingVolumes::AABBT<T>(-m_size * Vec3T<T>::ones(), m_size * Vec3T<T>::ones());
  }

  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;
};

// Center of octahedron i of n on a ring of radius r in the xz-plane.
template <class T>
Vec3T<T>
ringCenter(const int a_index, const int a_count, const T a_radius)
{
  const T angle = T(2) * pi<T> * T(a_index) / T(a_count);

  return Vec3T<T>(a_radius * std::cos(angle), T(0), a_radius * std::sin(angle));
}

// Twelve octahedra, each turned about its own vertical axis, on a ring around a sphere: thirteen
// operands, so the union is evaluated through a BVH.
template <class T>
Expr<T>
octahedronRing(const T a_size, const T a_sphereRadius)
{
  std::vector<Expr<T>> objects;

  objects.reserve(13);

  for (int i = 0; i < 12; i++) {
    objects.push_back(Translate(Rotate(Octahedron<T>{a_size}, T(30) * T(i), 1), ringCenter(i, 12, T(1.5))));
  }

  objects.emplace_back(SphereSDF<T>(Vec3T<T>::zeros(), a_sphereRadius));

  return Union(objects);
}

} // namespace

TEMPLATE_TEST_CASE("Tape: a user shape keeps its exact values through rotation, translation and a BVH union",
                   "[Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  static_assert(isShape<Octahedron<T>>);

  const T size   = T(0.25);
  const T radius = T(0.75);
  const T sqrt3  = std::sqrt(T(3));

  Pool pool(hostMemoryResource());

  const Tape<T, Octahedron<T>> tape(pool, octahedronRing(size, radius));

  CHECK(tape.getNumBVHUnions() == 1U);

  // The sphere's center, and every octahedron's center (a distance s / sqrt(3) from each face).
  CHECK_THAT(tape.signedDistance(Vec3::zeros()), withinAbsT(-radius, tightMargin<T>()));

  for (int i = 0; i < 12; i++) {
    CHECK_THAT(tape.signedDistance(ringCenter(i, 12, T(1.5))), withinAbsT(-size / sqrt3, looseMargin<T>()));
  }

  // Octahedron 0 is not rotated: 2s straight above its center is s beyond its top vertex, and along
  // (1, 1, 1) the direction meets a face head on.
  const Vec3 center0 = ringCenter(0, 12, T(1.5));
  const T    t       = T(0.2);

  CHECK_THAT(tape.signedDistance(center0 + Vec3(T(0), T(2) * size, T(0))), withinAbsT(size, looseMargin<T>()));
  CHECK_THAT(tape.signedDistance(center0 + t * Vec3::ones()), withinAbsT((T(3) * t - size) / sqrt3, looseMargin<T>()));
}

TEMPLATE_TEST_CASE("Tape: a tape built directly in managed memory evaluates on the device through its own pool",
                   "[Tape][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  // Managed memory in a GPU build; in a host build, host memory that reports itself device-accessible,
  // which takes the same path: the pool is built in place, frozen, and the tape rebased onto it.
#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  MemoryResource& resource = managedMemoryResource();
#else
  MemoryResource& resource = emulatedDeviceMemoryResource();
#endif

  Pool pool(resource);

  const Tape<T, Octahedron<T>> tape(pool, octahedronRing(T(0.25), T(0.75)));

  pool.freeze();

  const Tape<T, Octahedron<T>> view = tape.rebasedView(pool);

  const auto points = queryGrid<T>(Vec3T<T>(T(-2), T(-0.5), T(-2)), Vec3T<T>(T(2), T(0.5), T(2)), 17);

  requireSameResults(evaluateOnDevice<T>(TapeQuery<T, Tape<T, Octahedron<T>>>{view}, points),
                     evaluateOnHost<T>(TapeQuery<T, Tape<T, Octahedron<T>>>{tape}, points));
}

namespace {

// Spheres of different radii spread over a box, far enough apart that most points have one clearly
// nearest sphere.
template <class T>
std::vector<SphereSDF<T>>
spreadSpheres(const int a_count)
{
  std::vector<SphereSDF<T>> spheres;

  for (int i = 0; i < a_count; i++) {
    const Vec3T<T> center(T(1.3) * T(i % 4) - T(2), T(1.1) * T((i / 4) % 3) - T(1), T(0.4) * T(i % 3));

    spheres.emplace_back(center, T(0.2) + T(0.05) * T(i % 5));
  }

  return spheres;
}

// The index of the extreme value of the leaves at a point (the smallest, or the largest if a_max), or -1
// when the two most extreme values are closer than a_gap, so that rounding could pick either.
template <class T>
int
extremeLeaf(const std::vector<T>& a_values, const bool a_max, const T a_gap)
{
  int best   = 0;
  T   second = a_max ? -Math::Limits<T>::infinity() : Math::Limits<T>::infinity();

  for (size_t i = 1; i < a_values.size(); i++) {
    const T v = a_values[i];
    const T b = a_values[static_cast<size_t>(best)];

    if (a_max ? (v > b) : (v < b)) {
      second = b;
      best   = static_cast<int>(i);
    }
    else if (a_max ? (v > second) : (v < second)) {
      second = v;
    }
  }

  return (std::abs(a_values[static_cast<size_t>(best)] - second) < a_gap) ? -1 : best;
}

// Device functor: the leaf that decides a tape's value at one point.
template <class T, class TapeType>
struct LeafQuery
{
  TapeType m_tape;

  EBGEOMETRY_HOST_DEVICE
  uint32_t
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return m_tape.primitiveAt(a_point);
  }
};

} // namespace

TEMPLATE_TEST_CASE("Tape: primitiveAt names the leaf a union or intersection takes its value from",
                   "[Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto spheres = spreadSpheres<T>(12);

  std::vector<Expr<T>> operands(spheres.begin(), spheres.end());

  TapeOptions linear;

  linear.m_bvhUnionThreshold = 1000;

  const TestUtils::Compiled<T> withBVH(Union(operands));
  const TestUtils::Compiled<T> withoutBVH(Union(operands), linear);
  const TestUtils::Compiled<T> intersection(Intersection(operands));

  CHECK(withBVH.tape().getNumBVHUnions() == 1U);
  CHECK(withoutBVH.tape().getNumBVHUnions() == 0U);
  CHECK(withBVH.tape().getNumPrimitives() == 12U);

  const auto points = EBGeometryTestGPU::queryGrid<T>(Vec3(T(-3), T(-2), T(-1)), Vec3(T(3), T(2), T(2)), 10);

  size_t checked = 0;

  for (const auto& p : points) {
    std::vector<T> values;

    for (const auto& s : spheres) {
      values.push_back(s.signedDistance(p));
    }

    const int nearest  = extremeLeaf(values, false, T(looseMargin<T>()));
    const int farthest = extremeLeaf(values, true, T(looseMargin<T>()));

    if (nearest >= 0) {
      CHECK(withBVH.tape().primitiveAt(p) == uint32_t(nearest));
      CHECK(withoutBVH.tape().primitiveAt(p) == uint32_t(nearest));

      checked++;
    }
    if (farthest >= 0) {
      CHECK(intersection.tape().primitiveAt(p) == uint32_t(farthest));
    }
  }

  CHECK(checked > points.size() / 2);

  // A difference is the first operand intersected with the complement of the second: the box decides
  // where its value is the larger, the sphere where minus its value is.
  const BoxSDF<T>    box(Vec3(T(-1), T(-1), T(-1)), Vec3(T(1), T(1), T(1)));
  const SphereSDF<T> ball(Vec3(T(1), T(0), T(0)), T(0.8));

  const TestUtils::Compiled<T> difference(Difference(box, ball));

  for (const auto& p : points) {
    const T a = box.signedDistance(p);
    const T b = -ball.signedDistance(p);

    if (std::abs(a - b) > T(looseMargin<T>())) {
      CHECK(difference.tape().primitiveAt(p) == ((b > a) ? 1U : 0U));
    }
  }
}

TEMPLATE_TEST_CASE("Tape: leaves are numbered depth first, each occurrence counted, through maps and value "
                   "operations",
                   "[Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // One sphere used three times, so the three leaves differ only in where they were placed. The
  // rotation turns the sphere about its own centre, so it moves nothing.
  const SphereSDF<T> s(Vec3::zeros(), T(0.5));

  const Vec3 a(T(-3), T(0), T(0));
  const Vec3 b(T(0), T(3), T(0));
  const Vec3 c(T(3), T(0), T(0));

  const auto expr = Union(Translate(Offset(s, T(0.1)), a),
                          Union(Translate(Rotate(Complement(Complement(s)), T(90), 2), b),
                                Annular(Scale(Translate(s, c / T(2)), T(2)), T(0.05))));

  const TestUtils::Compiled<T> compiled(expr);

  CHECK(compiled.tape().getNumPrimitives() == 3U);

  CHECK(compiled.tape().primitiveAt(a) == 0U);
  CHECK(compiled.tape().primitiveAt(b) == 1U);
  CHECK(compiled.tape().primitiveAt(c) == 2U);
  CHECK(compiled.tape().primitiveAt(a + Vec3(T(0), T(0), T(0.9))) == 0U);
}

TEMPLATE_TEST_CASE("Tape: primitiveAt names the more extreme operand of a smooth union or intersection",
                   "[Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto few  = spreadSpheres<T>(3);
  const auto many = spreadSpheres<T>(12);

  const std::vector<Expr<T>> fewOperands(few.begin(), few.end());
  const std::vector<Expr<T>> manyOperands(many.begin(), many.end());

  const TestUtils::Compiled<T> smoothUnion(SmoothUnion(fewOperands, T(0.2)));
  const TestUtils::Compiled<T> exponential(SmoothUnion(fewOperands, T(0.2), BlendKind::Exponential));
  const TestUtils::Compiled<T> smoothIntersection(SmoothIntersection(fewOperands, T(0.2)));
  const TestUtils::Compiled<T> smoothUnionBVH(SmoothUnion(manyOperands, T(0.2)));

  CHECK(smoothUnionBVH.tape().getNumBVHUnions() == 1U);

  const auto points = EBGeometryTestGPU::queryGrid<T>(Vec3(T(-3), T(-2), T(-1)), Vec3(T(3), T(2), T(2)), 10);

  for (const auto& p : points) {
    std::vector<T> fewValues;
    std::vector<T> manyValues;

    for (const auto& s : few) {
      fewValues.push_back(s.signedDistance(p));
    }
    for (const auto& s : many) {
      manyValues.push_back(s.signedDistance(p));
    }

    const int nearest  = extremeLeaf(fewValues, false, T(looseMargin<T>()));
    const int farthest = extremeLeaf(fewValues, true, T(looseMargin<T>()));
    const int nearestM = extremeLeaf(manyValues, false, T(looseMargin<T>()));

    if (nearest >= 0) {
      CHECK(smoothUnion.tape().primitiveAt(p) == uint32_t(nearest));
      CHECK(exponential.tape().primitiveAt(p) == uint32_t(nearest));
    }
    if (farthest >= 0) {
      CHECK(smoothIntersection.tape().primitiveAt(p) == uint32_t(farthest));
    }
    if (nearestM >= 0) {
      CHECK(smoothUnionBVH.tape().primitiveAt(p) == uint32_t(nearestM));
    }
  }
}

TEMPLATE_TEST_CASE("Tape: Blur and Mollify take the leaf of their centre tap", "[Tape]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const SphereSDF<T> left(Vec3(T(-1.5), T(0), T(0)), T(0.6));
  const SphereSDF<T> right(Vec3(T(1.5), T(0), T(0)), T(0.6));

  const TestUtils::Compiled<T> blurred(Blur(Union(left, right), T(0.1)));
  const TestUtils::Compiled<T> mollified(Mollify(Union(left, right), T(0.1), 3));
  const TestUtils::Compiled<T> evenMollified(Mollify(Union(left, right), T(0.1), 4));

  // Well away from the plane x = 0 where the two are equally near, every tap has the same nearest
  // sphere, so the centre tap's leaf is also the nearest sphere's.
  for (const T x : {T(-2.5), T(-1.5), T(-0.4), T(0.4), T(1.5), T(2.5)}) {
    const Vec3     p(x, T(0.3), T(-0.2));
    const uint32_t expected = (x < T(0)) ? 0U : 1U;

    CHECK(blurred.tape().primitiveAt(p) == expected);
    CHECK(mollified.tape().primitiveAt(p) == expected);
    CHECK(evenMollified.tape().primitiveAt(p) == expected);
  }
}

TEMPLATE_TEST_CASE("Tape: primitiveAt on the device matches the host", "[Tape][gpu]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  Pool pool(hostMemoryResource());

  const auto few  = spreadSpheres<T>(3);
  const auto many = spreadSpheres<T>(12);

  const std::vector<Expr<T>> fewOperands(few.begin(), few.end());
  const std::vector<Expr<T>> manyOperands(many.begin(), many.end());

  const BoxSDF<T> box(Vec3(T(-1), T(-1), T(-1)), Vec3(T(1), T(1), T(1)));

  std::vector<std::pair<std::string, Tape<T>>> tapes;

  tapes.emplace_back("union", Tape<T>(pool, Union(fewOperands)));
  tapes.emplace_back("BVH union", Tape<T>(pool, Union(manyOperands)));
  tapes.emplace_back("smooth BVH union", Tape<T>(pool, SmoothUnion(manyOperands, T(0.2))));
  tapes.emplace_back("smooth intersection", Tape<T>(pool, SmoothIntersection(fewOperands, T(0.2))));
  tapes.emplace_back("difference", Tape<T>(pool, Difference(box, Union(fewOperands))));
  tapes.emplace_back("blur", Tape<T>(pool, Blur(Union(fewOperands), T(0.1))));
  tapes.emplace_back("mollify", Tape<T>(pool, Mollify(Union(fewOperands), T(0.1), 3)));

  pool.freeze();

  const Pool devicePool = Pool::mirror(pool, deviceTestResource());

  const auto points = queryGrid<T>(Vec3(T(-3), T(-2), T(-1)), Vec3(T(3), T(2), T(2)), 10);

  for (const auto& [name, tape] : tapes) {
    INFO(name);

    const Tape<T> view = tape.rebasedView(devicePool);

    CHECK(evaluateOnDevice<uint32_t>(LeafQuery<T, Tape<T>>{view}, points) ==
          evaluateOnHost<uint32_t>(LeafQuery<T, Tape<T>>{tape}, points));
  }
}

namespace {

// Rays from a ring of origins around a point, aimed at targets spread over a box around it.
template <class T>
std::vector<RayT<T>>
raysAround(const Vec3T<T>& a_center, const T a_radius, const T a_spread)
{
  std::vector<RayT<T>> rays;

  for (int i = 0; i < 16; i++) {
    const T        angle = T(2) * pi<T> * T(i) / T(16);
    const Vec3T<T> origin =
      a_center + Vec3T<T>(a_radius * std::cos(angle), T(0.2) * a_radius * T(i % 3 - 1), a_radius * std::sin(angle));

    for (int j = 0; j < 25; j++) {
      const Vec3T<T> target =
        a_center + a_spread * Vec3T<T>(T(0.5) * T(j % 5) - T(1), T(0.5) * T(j / 5) - T(1), T(0.2) * T(j % 3) - T(0.2));
      const Vec3T<T> d = target - origin;

      rays.push_back(RayT<T>{origin, d / d.length()});
    }
  }

  return rays;
}

// The ray program against sphere tracing the same tape through its scalar evaluator: the same crossings
// (to the march's tolerance, over rays that do not graze the surface), entered or left alike, and the
// leaf the ray program names the one the value comes from just inside or outside the crossing.
template <class T, class TapeType>
void
checkRayProgram(const TapeType& a_tape, const bool a_exact, const T a_radius, const T a_spread)
{
  const RayOptions<T> options;

  const auto box   = a_tape.computeBoundingVolume();
  const T    scale = (box.getHighCorner() - box.getLowCorner()).length();
  const T    eps   = options.m_tolerance * scale;
  const auto c     = box.getCentroid();

  size_t hits = 0;

  for (const auto& ray : raysAround(c, a_radius, a_spread)) {
    const HitT<T> traced  = a_tape.firstHit(ray, T(0), T(100), options);
    const HitT<T> marched = RayDetail::march(a_tape, ray, T(0), T(100), options);

    REQUIRE(marched.m_status != HitStatus::Unresolved);

    if (marched.hit()) {
      // A ray that grazes the surface can go either way.
      const T        h = T(1e-3) * scale;
      const Vec3T<T> p = marched.m_point;

      Vec3T<T> g;

      for (size_t a = 0; a < 3; a++) {
        Vec3T<T> e = Vec3T<T>::zeros();

        e[a] = h;
        g[a] = a_tape.signedDistance(p + e) - a_tape.signedDistance(p - e);
      }

      if (g.length() == T(0) || std::abs(dot(g / g.length(), ray.m_direction)) < T(0.25)) {
        continue;
      }
    }

    // Sphere tracing accepts a point within its tolerance of the surface, so it also "hits" where the
    // solid along the ray is thinner than that (where two surfaces nearly meet), which the exact spans
    // rightly do not.
    if (marched.hit() && !traced.hit()) {
      T deepest = Math::Limits<T>::max();

      for (int k = -20; k <= 20; k++) {
        deepest = Math::min(deepest, a_tape.signedDistance(ray.at(marched.m_t + T(k) * T(0.5) * eps)));
      }

      if (deepest > -T(2) * eps) {
        continue;
      }
    }

    INFO("ray " << ray.m_origin << " -> " << ray.m_direction << ": traced " << int(traced.m_status) << " at "
                << traced.m_t << ", marched " << int(marched.m_status) << " at " << marched.m_t);
    CHECK(traced.hit() == marched.hit());

    if (traced.hit() && marched.hit()) {
      // Exact unless the ray crossed a sphere-traced node.
      if (a_exact) {
        CHECK(traced.m_exact);
      }
      CHECK(traced.m_entering == marched.m_entering);
      CHECK(std::abs(traced.m_t - marched.m_t) <= T(6) * eps);
      CHECK(traced.m_primitive < a_tape.getNumPrimitives());

      // Just past the crossing (inside the solid if the ray enters), the value comes from the named leaf
      // or the surface there is shared.
      const T        past = T(4) * eps;
      const Vec3T<T> q    = ray.at(traced.m_t + (traced.m_entering ? past : -past));

      CHECK((a_tape.primitiveAt(q) == traced.m_primitive || std::abs(a_tape.signedDistance(q)) < T(8) * eps));

      hits++;
    }
  }

  CHECK(hits > 40U);
}

} // namespace

TEMPLATE_TEST_CASE("Tape: firstHit follows unions, intersections, complements and maps exactly",
                   "[Tape][Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const BoxSDF<T>    cube(-Vec3::ones(), Vec3::ones());
  const SphereSDF<T> ball(Vec3::zeros(), T(1.35));

  std::vector<Expr<T>> drills;

  for (size_t axis = 0; axis < 3; axis++) {
    drills.emplace_back(InfiniteCylinderSDF<T>(Vec3::zeros(), T(0.55), axis));
  }

  const auto part = Difference(Intersection(cube, ball), Union(drills));

  const TestUtils::Compiled<T> classic(part);

  checkRayProgram<T>(classic.tape(), true, T(4), T(1.2));

  // Maps: the part turned, moved and scaled, and reflected.
  const TestUtils::Compiled<T> moved(
    Translate(Rotate(Scale(Reflect(part, 0), T(0.7)), T(30), Vec3(T(1), T(1), T(0))), Vec3(T(0.3), T(-0.2), T(0.1))));

  checkRayProgram<T>(moved.tape(), true, T(4), T(1));

  // A union of more objects than the BVH threshold, of different types, some overlapping.
  std::vector<Expr<T>> objects;

  for (int i = 0; i < 12; i++) {
    const Vec3 at(T(0.8) * T(i % 4) - T(1.2), T(0.7) * T(i / 4) - T(0.7), T(0.1) * T(i % 3));

    if (i % 3 == 0) {
      objects.emplace_back(SphereSDF<T>(at, T(0.45)));
    }
    else if (i % 3 == 1) {
      objects.emplace_back(BoxSDF<T>(at - T(0.35) * Vec3::ones(), at + T(0.35) * Vec3::ones()));
    }
    else {
      objects.emplace_back(CapsuleSDF<T>(at - Vec3(T(0.4), T(0), T(0)), at + Vec3(T(0.4), T(0.1), T(0)), T(0.2)));
    }
  }

  const TestUtils::Compiled<T> many(Union(objects));

  CHECK(many.tape().getNumBVHUnions() == 1U);

  checkRayProgram<T>(many.tape(), true, T(5), T(1.5));

  // A hole: everything outside a box, inside a big sphere.
  const TestUtils::Compiled<T> hollow(Intersection(SphereSDF<T>(Vec3::zeros(), T(2)), Complement(cube)));

  checkRayProgram<T>(hollow.tape(), true, T(5), T(1.5));
}

TEMPLATE_TEST_CASE("Tape: firstHit sphere traces the nodes with no ray form, inside an expression it follows",
                   "[Tape][Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const BoxSDF<T>   cube(-Vec3::ones(), Vec3::ones());
  const TorusSDF<T> torus(Vec3(T(0), T(1), T(0)), T(0.9), T(0.25));

  // A torus (no ray form) and a smooth union cut out of a box: the box is followed exactly, the rest
  // sphere traced.
  const auto smooth =
    SmoothUnion(SphereSDF<T>(Vec3(T(1), T(0), T(0)), T(0.5)), SphereSDF<T>(Vec3(T(1), T(0.6), T(0)), T(0.4)), T(0.2));

  const TestUtils::Compiled<T> mixed(Difference(cube, Union(torus, smooth)));

  checkRayProgram<T>(mixed.tape(), false, T(4), T(1.2));

  // Nothing with a ray form: the tape is sphere traced as a whole, as before.
  const TestUtils::Compiled<T> marched(Offset(torus, T(0.05)));

  checkRayProgram<T>(marched.tape(), false, T(4), T(1.2));
}

TEMPLATE_TEST_CASE("Tape: firstHit traces a mesh and a tape inside a tape", "[Tape][Ray]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  constexpr size_t K = BVH::DefaultBranchingRatio<T>();
  constexpr size_t W = TriangleSoA::DefaultWidth<T>();

  const auto                mesh = Parser::readIntoDCEL<T>(dodecahedron(), pool);
  const TriMeshSDF<T, K, W> tri(mesh, pool, BVH::Construction::SAH, 4);

  const Tape<T> withMesh(pool, Difference(tri, SphereSDF<T>(tri.computeBoundingVolume().getCentroid(), T(0.5))));

  checkRayProgram<T>(withMesh, true, T(4), T(1));

  const Tape<T>          inner(pool,
                      Union(SphereSDF<T>(Vec3::zeros(), T(0.6)),
                            BoxSDF<T>(Vec3(T(0.3), T(-0.4), T(-0.4)), Vec3(T(1.1), T(0.4), T(0.4)))));
  const Tape<T, Tape<T>> outer(pool, Intersection(Rotate(inner, T(40), 2), SphereSDF<T>(Vec3::zeros(), T(1))));

  checkRayProgram<T>(outer, true, T(4), T(1));

  // A deep copy and a rebased view trace alike.
  Pool other(hostMemoryResource());

  const Tape<T> copy = withMesh.deepCopy(other);

  CHECK(copy.getNumPrimitives() == withMesh.getNumPrimitives());

  for (const auto& ray : raysAround(Vec3::zeros(), T(4), T(1))) {
    const HitT<T> a = withMesh.firstHit(ray, T(0), T(100));
    const HitT<T> b = copy.firstHit(ray, T(0), T(100));

    CHECK(a.m_status == b.m_status);
    CHECK(a.m_t == b.m_t);
    CHECK(a.m_primitive == b.m_primitive);
  }
}

namespace {

// Device functor: the distance to a tape's first crossing, -1 for a miss and -2 for an unresolved query.
template <class T, class TapeType>
struct TapeHitQuery
{
  TapeType m_tape;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const RayT<T>& a_ray) const noexcept
  {
    const HitT<T> hit = m_tape.firstHit(a_ray, T(0), T(100));

    return hit.hit() ? hit.m_t : ((hit.m_status == HitStatus::Miss) ? T(-1) : T(-2));
  }
};

} // namespace

TEMPLATE_TEST_CASE("Tape: firstHit on the device matches the host", "[Tape][Ray][gpu]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  Pool pool(hostMemoryResource());

  std::vector<Expr<T>> objects;

  for (int i = 0; i < 10; i++) {
    objects.emplace_back(SphereSDF<T>(Vec3(T(0.6) * T(i % 5) - T(1.2), T(0.8) * T(i / 5) - T(0.4), T(0)), T(0.35)));
  }

  std::vector<std::pair<std::string, Tape<T>>> tapes;

  tapes.emplace_back(
    "classic",
    Tape<T>(pool,
            Difference(Intersection(BoxSDF<T>(-Vec3::ones(), Vec3::ones()), SphereSDF<T>(Vec3::zeros(), T(1.35))),
                       InfiniteCylinderSDF<T>(Vec3::zeros(), T(0.55), 1))));
  tapes.emplace_back("BVH union", Tape<T>(pool, Rotate(Union(objects), T(20), 1)));
  tapes.emplace_back(
    "with a fallback",
    Tape<T>(pool, Union(BoxSDF<T>(-Vec3::ones(), Vec3::zeros()), TorusSDF<T>(Vec3::zeros(), T(1), T(0.2)))));

  pool.freeze();

  const Pool devicePool = Pool::mirror(pool, deviceTestResource());
  const auto rays       = raysAround(Vec3::zeros(), T(4), T(1));

  for (const auto& [name, tape] : tapes) {
    INFO(name);

    const Tape<T> view = tape.rebasedView(devicePool);

    requireSameResults(evaluateOnDevice<T>(TapeHitQuery<T, Tape<T>>{view}, rays),
                       evaluateOnHost<T>(TapeHitQuery<T, Tape<T>>{tape}, rays),
                       T(1e-3));
  }
}
