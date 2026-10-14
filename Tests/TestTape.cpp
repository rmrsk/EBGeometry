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
#include "TestTapeUtils.hpp"

#include <cstdint>
#include <string>
#include <type_traits>
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
