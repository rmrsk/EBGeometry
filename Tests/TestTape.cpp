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

TEMPLATE_TEST_CASE("Tape: Add and Displace sum their operands' values, with the boxes and quality their rules give",
                   "[Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  const SphereSDF<T> sphere(Vec3(T(0.2), T(-0.1), T(0.3)), T(0.8));
  const BoxSDF<T>    box(Vec3(T(-1), T(-0.5), T(-0.25)), Vec3(T(0.5), T(1), T(0.75)));
  const PerlinSDF<T> perlin(T(0.5), T(2) * Vec3::ones(), T(0.5), 4);

  const Expr<T> sum       = Add(sphere, box);
  const Expr<T> displaced = Displace(sphere, perlin, T(0.25));

  const Tape<T> sumTape(pool, sum);
  const Tape<T> displacedTape(pool, displaced);
  const Tape<T> chained(pool, Add(Add(sphere, box), Offset(sphere, T(0.5))));

  for (const auto& p : probes<T>()) {
    CHECK(sumTape.signedDistance(p) == sphere.signedDistance(p) + box.signedDistance(p));
    CHECK(displacedTape.signedDistance(p) == sphere.signedDistance(p) + perlin.signedDistance(p));
    CHECK_THAT(chained.signedDistance(p),
               withinAbsT(sphere.signedDistance(p) + box.signedDistance(p) + sphere.signedDistance(p) - T(0.5),
                          tightMargin<T>()));
  }

  // Add bounds nothing; Displace grows the object's box by the amplitude. Neither is a distance.
  const auto sphereBox = sphere.computeBoundingVolume();
  const auto grownBox  = displaced.computeBoundingVolume();

  CHECK(sum.computeBoundingVolume().getLowCorner() == -Vec3::max());
  CHECK(sum.computeBoundingVolume().getHighCorner() == Vec3::max());
  CHECK(sum.getDistanceQuality() == DistanceQuality::NotADistance);
  CHECK(displaced.getDistanceQuality() == DistanceQuality::NotADistance);

  for (size_t dir = 0; dir < 3; dir++) {
    CHECK_THAT(grownBox.getLowCorner()[dir], withinAbsT(sphereBox.getLowCorner()[dir] - T(0.25), tightMargin<T>()));
    CHECK_THAT(grownBox.getHighCorner()[dir], withinAbsT(sphereBox.getHighCorner()[dir] + T(0.25), tightMargin<T>()));
  }

  // Displacing what is not a distance bounds nothing: its value can be small far from its object.
  const Expr<T> twice = Displace(displaced, perlin, T(0.25));

  CHECK(twice.computeBoundingVolume().getLowCorner() == -Vec3::max());
  CHECK(twice.computeBoundingVolume().getHighCorner() == Vec3::max());

  // Not a distance, so a union of displaced objects is evaluated without a BVH.
  std::vector<Expr<T>> bumpy;

  for (int i = 0; i < 20; i++) {
    bumpy.push_back(Displace(SphereSDF<T>(Vec3(T(i), T(0), T(0)), T(0.4)), perlin, T(0.25)));
  }

  CHECK(Tape<T>(pool, Union(bumpy)).getNumBVHUnions() == 0U);
}

TEMPLATE_TEST_CASE("Tape: a point cloud as a leaf is the union of balls around its points",
                   "[Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  std::vector<Vec3> points;

  for (int i = 0; i < 300; i++) {
    const T t = T(0.07) * T(i);

    points.emplace_back(T(0.2) * t * std::cos(t), T(0.2) * t * std::sin(t), T(0.05) * t - T(0.5));
  }

  const PointCloudBVH<T> cloud(pool, points);
  const T                radius = T(0.15);

  const PointCloudSDF<T> balls(cloud, radius);
  const PointCloudSDF<T> unsignedDistance(cloud, T(0));

  const Tape<T, PointCloudSDF<T>> tape(pool, Union(balls, SphereSDF<T>(Vec3(T(2), T(0), T(0)), T(0.5))));
  const Tape<T, PointCloudSDF<T>> moved(pool, Translate(unsignedDistance, Vec3(T(0), T(0), T(1))));

  const auto nearest = [&points](const Vec3& a_p) {
    T best = Math::Limits<T>::max();

    for (const auto& c : points) {
      best = Math::min(best, (a_p - c).length());
    }

    return best;
  };

  for (const auto& p : probes<T>()) {
    const T truth = nearest(p);

    CHECK_THAT(balls.signedDistance(p), withinAbsT(truth - radius, tightMargin<T>()));
    CHECK_THAT(unsignedDistance.signedDistance(p), withinAbsT(truth, tightMargin<T>()));
    CHECK_THAT(tape.signedDistance(p),
               withinAbsT(Math::min(truth - radius, (p - Vec3(T(2), T(0), T(0))).length() - T(0.5)), tightMargin<T>()));
    CHECK_THAT(moved.signedDistance(p), withinAbsT(nearest(p - Vec3(T(0), T(0), T(1))), tightMargin<T>()));
  }

  // The box is the cloud's grown by the radius, and the shape is a lower bound inside.
  const auto cloudBox = cloud.getBVH().computeBoundingVolume();
  const auto ballsBox = balls.computeBoundingVolume();

  for (size_t dir = 0; dir < 3; dir++) {
    CHECK_THAT(ballsBox.getLowCorner()[dir], withinAbsT(cloudBox.getLowCorner()[dir] - radius, tightMargin<T>()));
    CHECK_THAT(ballsBox.getHighCorner()[dir], withinAbsT(cloudBox.getHighCorner()[dir] + radius, tightMargin<T>()));
  }

  CHECK(distanceQualityOf<PointCloudSDF<T>> == DistanceQuality::Bound);

  // A handle rebuilt from its layout, a host view and a deep copy answer alike.
  const PointCloudSDF<T> rebuilt(balls.layout(), balls.location());

  pool.freeze();

  const Pool             mirror = Pool::mirror(pool, hostMemoryResource());
  const PointCloudSDF<T> view   = balls.rebasedView(mirror);

  Pool                   other(hostMemoryResource());
  const PointCloudSDF<T> copy = balls.deepCopy(other);

  for (const auto& p : probes<T>()) {
    CHECK(rebuilt.signedDistance(p) == balls.signedDistance(p));
    CHECK(view.signedDistance(p) == balls.signedDistance(p));
    CHECK(copy.signedDistance(p) == balls.signedDistance(p));
  }
}

TEST_CASE("PointCloudSDF: an empty cloud is empty space, and a negative radius is rejected", "[Tape][death]")
{
  using T = double;

  Pool pool(hostMemoryResource());

  const PointCloudBVH<T> empty(pool, std::vector<Vec3T<T>>{});
  const PointCloudSDF<T> nothing(empty, T(0.5));

  CHECK(nothing.signedDistance(Vec3T<T>::zeros()) == Math::Limits<T>::max());
  CHECK_FALSE(nothing.computeBoundingVolume().getLowCorner()[0] <= nothing.computeBoundingVolume().getHighCorner()[0]);

  REQUIRE(abortsWith(
    [] {
      Pool                   p(hostMemoryResource());
      const PointCloudBVH<T> cloud(p, std::vector<Vec3T<T>>{Vec3T<T>::zeros()});
      const PointCloudSDF<T> balls(cloud, T(-1));
    },
    "PointCloudSDF: the radius must be finite and non-negative (-1)"));

  REQUIRE(abortsWith(
    [] { (void)Displace(SphereSDF<T>(Vec3T<T>::zeros(), T(1)), SphereSDF<T>(Vec3T<T>::zeros(), T(1)), T(-1)); },
    "Displace: the amplitude must be finite and non-negative (-1)"));
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
  exprs.emplace_back("add", Add(sphere, box));
  exprs.emplace_back("displace", Displace(torus, perlin, T(1)));

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

  // A point cloud as a leaf: balls around a spiral of points, unioned with a box.
  std::vector<Vec3> spiral;

  for (int i = 0; i < 200; i++) {
    const T t = T(0.1) * T(i);

    spiral.emplace_back(T(0.15) * t * std::cos(t), T(0.15) * t * std::sin(t), T(0.1) * t - T(1));
  }

  const PointCloudBVH<T>          cloud(pool, spiral);
  const Tape<T, PointCloudSDF<T>> balls(pool, Union(PointCloudSDF<T>(cloud, T(0.2)), box));
  const Tape<T>                   inner(pool, Difference(box, sphere));
  const Tape<T, Tape<T>>          nested(pool, Rotate(Translate(inner, Vec3::ones()), T(45), 1));

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
  check(balls);
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
