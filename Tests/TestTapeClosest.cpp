// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for Tape::closestPrimitive() (EBGeometry_Tape.hpp, EBGeometry_PrimitiveRef.hpp): the deciding
// leaf of unions, intersections and differences, through smooth blends, blurs, maps and repetitions, the
// numbering of repeated leaves, meshes and fused parts as leaves, and device evaluation.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

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

// A visitor for visitPrimitive(): the leaf's value at a point. A struct with an EBGEOMETRY_HOST_DEVICE
// call operator, as a CUDA or HIP translation unit needs, rather than a lambda.
template <class T>
struct ValueAt
{
  Vec3T<T> m_point;
  T        m_value = T(0);

  template <class P>
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const P& a_primitive) noexcept
  {
    m_value = a_primitive.signedDistance(m_point);
  }
};

// Device functor: the deciding leaf at a point, and the leaf's own value at its local point.
template <class T, class TapeType>
struct LeafQuery
{
  TapeType m_tape;

  EBGEOMETRY_HOST_DEVICE
  PrimitiveRef<T>
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return m_tape.closestPrimitive(a_point);
  }
};

} // namespace

TEMPLATE_TEST_CASE("Tape: closestPrimitive names the operand a union, intersection or difference takes its value from",
                   "[Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const BoxSDF<T>    box(Vec3(T(-1), T(-1), T(-1)), Vec3(T(1), T(1), T(1)));
  const SphereSDF<T> ball(Vec3(T(1), T(0), T(0)), T(0.6));

  Pool pool(hostMemoryResource());

  // The carved box: near the cavity the sphere decides, near the outside the box (leaves 0 and 1).
  const Tape<T> carved(pool, Difference(box, ball));

  CHECK(carved.closestPrimitive(Vec3(T(0.3), T(0), T(0))).m_leaf == 1U);
  CHECK(carved.closestPrimitive(Vec3(T(-1.2), T(0), T(0))).m_leaf == 0U);
  CHECK(carved.template isPrimitive<SphereSDF<T>>(carved.closestPrimitive(Vec3(T(0.3), T(0), T(0)))));
  CHECK(carved.template isPrimitive<BoxSDF<T>>(carved.closestPrimitive(Vec3(T(-1.2), T(0), T(0)))));

  // Against the brute-force choice: difference max(a, -b), union argmin and intersection argmax, the
  // union through the tape's BVH and operand by operand.
  std::vector<SphereSDF<T>> spheres;
  std::vector<Expr<T>>      operands;

  for (int i = 0; i < 12; i++) {
    spheres.emplace_back(Vec3(T(0.7) * T(i % 4) - T(1), T(0.6) * T(i / 4) - T(0.6), T(0.1) * T(i % 3)), T(0.45));
    operands.emplace_back(spheres.back());
  }

  const Tape<T> unionBVH(pool, Union(operands, Accelerate::BVH));
  const Tape<T> unionLinear(pool, Union(operands));
  const Tape<T> intersection(pool, Intersection(operands));

  REQUIRE(unionBVH.getNumBVHUnions() == 1U);

  for (int i = -10; i <= 10; i++) {
    for (int j = -6; j <= 6; j++) {
      const Vec3 p(T(0.23) * T(i) + T(0.01), T(0.19) * T(j) - T(0.02), T(0.17));

      uint32_t smallest = 0;
      uint32_t largest  = 0;

      for (uint32_t k = 1; k < spheres.size(); k++) {
        smallest = (spheres[k].signedDistance(p) < spheres[smallest].signedDistance(p)) ? k : smallest;
        largest  = (spheres[largest].signedDistance(p) < spheres[k].signedDistance(p)) ? k : largest;
      }

      INFO("p = " << p[0] << ", " << p[1] << ", " << p[2]);
      CHECK(unionBVH.closestPrimitive(p).m_leaf == smallest);
      CHECK(unionLinear.closestPrimitive(p).m_leaf == smallest);
      CHECK(intersection.closestPrimitive(p).m_leaf == largest);

      const uint32_t difference = (box.signedDistance(p) >= -ball.signedDistance(p)) ? 0U : 1U;

      CHECK(carved.closestPrimitive(p).m_leaf == difference);
    }
  }
}

TEMPLATE_TEST_CASE(
  "Tape: closestPrimitive follows smooth blends, blurs, maps and repetitions to the leaf, in its frame",
  "[Tape]",
  EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const SphereSDF<T> a(Vec3(T(-0.4), T(0), T(0)), T(0.5));
  const SphereSDF<T> b(Vec3(T(0.5), T(0.1), T(0)), T(0.4));
  const SphereSDF<T> c(Vec3(T(0), T(0.7), T(0.2)), T(0.3));

  Pool pool(hostMemoryResource());

  const Vec3 shift(T(0.3), T(-0.2), T(0.1));

  const Tape<T> smooth(pool, SmoothUnion(std::vector<Expr<T>>{a, b, c}, T(0.3)));
  const Tape<T> blurred(pool, Blur(Union(a, b), T(0.1)));
  const Tape<T> moved(pool, Translate(Scale(Union(a, b), T(2)), shift));
  const Tape<T> tiled(pool, FiniteRepetition(Union(a, b), Vec3(T(2.5), T(2.5), T(2.5)), Vec3::ones(), Vec3::ones()));

  for (int i = -8; i <= 8; i++) {
    for (int j = -5; j <= 5; j++) {
      const Vec3 p(T(0.29) * T(i) + T(0.02), T(0.23) * T(j) - T(0.03), T(0.11));

      INFO("p = " << p[0] << ", " << p[1] << ", " << p[2]);

      // A smooth union of three: the smallest operand; a blur: the leaf at its center tap, the point.
      const T da = a.signedDistance(p);
      const T db = b.signedDistance(p);
      const T dc = c.signedDistance(p);

      const uint32_t smallest3 = (db < da) ? ((dc < db) ? 2U : 1U) : ((dc < da) ? 2U : 0U);

      CHECK(smooth.closestPrimitive(p).m_leaf == smallest3);
      CHECK(blurred.closestPrimitive(p).m_leaf == ((db < da) ? 1U : 0U));

      // A map: the leaf sees (p - shift) / 2, and the primitive there gives the leaf's own value.
      const Vec3            local = (p - shift) / T(2);
      const PrimitiveRef<T> m     = moved.closestPrimitive(p);

      CHECK(m.m_leaf == ((b.signedDistance(local) < a.signedDistance(local)) ? 1U : 0U));
      CHECK_THAT(m.m_localPoint[0], withinAbsT(local[0], tightMargin<T>()));
      CHECK_THAT(m.m_localPoint[1], withinAbsT(local[1], tightMargin<T>()));
      CHECK_THAT(m.m_localPoint[2], withinAbsT(local[2], tightMargin<T>()));
      CHECK(moved.template primitive<SphereSDF<T>>(m).signedDistance(m.m_localPoint) ==
            (m.m_leaf == 0U ? a : b).signedDistance(m.m_localPoint));

      // A repetition: the member of the nearest copy, at the point in that copy's frame.
      T        best      = std::numeric_limits<T>::max();
      uint32_t bestLeaf  = 0;
      Vec3     bestLocal = Vec3::zeros();

      for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
          for (int z = -1; z <= 1; z++) {
            const Vec3 q = p - T(2.5) * Vec3(T(x), T(y), T(z));

            for (uint32_t leaf = 0; leaf < 2; leaf++) {
              const T d = (leaf == 0U ? a : b).signedDistance(q);

              if (d < best) {
                best      = d;
                bestLeaf  = leaf;
                bestLocal = q;
              }
            }
          }
        }
      }

      const PrimitiveRef<T> r = tiled.closestPrimitive(p);

      CHECK(r.m_leaf == bestLeaf);
      CHECK_THAT(tiled.template primitive<SphereSDF<T>>(r).signedDistance(r.m_localPoint),
                 withinAbsT(best, tightMargin<T>()));
      CHECK_THAT(r.m_localPoint[0], withinAbsT(bestLocal[0], tightMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("Tape: closestPrimitive numbers each occurrence of a leaf, and returns a mesh as its handle",
                   "[Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = BVH::DefaultBranchingRatio<T>();
  constexpr size_t W = TriangleSoA::DefaultWidth<T>();

  Pool pool(hostMemoryResource());

  const auto                mesh = Parser::readIntoDCEL<T>(dodecahedron(), pool);
  const TriMeshSDF<T, K, W> tri(mesh, pool, BVH::Construction::SAH, 4);
  const SphereSDF<T>        ball(Vec3(T(1.5), T(0), T(0)), T(0.4));

  // The same sphere twice, moved apart: two leaves (1 and 2), one primitive in the tape's array.
  const Tape<T> tape(pool, Union(std::vector<Expr<T>>{tri, Translate(ball, Vec3(T(0), T(1.5), T(0))), ball}));

  CHECK(tape.closestPrimitive(Vec3(T(1.5), T(1.6), T(0))).m_leaf == 1U);
  CHECK(tape.closestPrimitive(Vec3(T(1.6), T(0), T(0))).m_leaf == 2U);

  const PrimitiveRef<T> first  = tape.closestPrimitive(Vec3(T(1.5), T(1.6), T(0)));
  const PrimitiveRef<T> second = tape.closestPrimitive(Vec3(T(1.6), T(0), T(0)));

  CHECK(first.m_type == second.m_type);

  // A mesh leaf comes back as a handle that resolves against the tape's pool.
  const Vec3            p(T(0.1), T(0.05), T(-0.2));
  const PrimitiveRef<T> leaf = tape.closestPrimitive(p);

  REQUIRE(leaf.m_leaf == 0U);
  REQUIRE(tape.template isPrimitive<TriMeshSDF<T, K, W>>(leaf));
  CHECK_FALSE(tape.template isPrimitive<SphereSDF<T>>(leaf));

  const auto handle = tape.template primitive<TriMeshSDF<T, K, W>>(leaf);

  CHECK(handle.signedDistance(leaf.m_localPoint) == tri.signedDistance(p));
  CHECK(handle.getClosestFace(leaf.m_localPoint).faceId == tri.getClosestFace(p).faceId);

  // visitPrimitive passes the same primitive, whatever its type.
  ValueAt<T> visit{leaf.m_localPoint};

  tape.visitPrimitive(leaf, visit);

  CHECK(visit.m_value == tri.signedDistance(p));
}

TEMPLATE_TEST_CASE("Tape: closestPrimitive treats a fused part as one leaf, the part a scene is made of",
                   "[Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // A drilled part, fused: one shape with one surface, however it is built.
  const BoxSDF<T>              cube(Vec3(T(-1), T(-1), T(-1)), Vec3(T(1), T(1), T(1)));
  const InfiniteCylinderSDF<T> drill(Vec3::zeros(), T(0.4), 0);
  const auto                   part = Fused::Difference(cube, drill);

  using Part = std::decay_t<decltype(part)>;

  const SphereSDF<T> ball(Vec3(T(2.5), T(0.1), T(-0.2)), T(0.6));

  // The scene: the part (leaf 0) and a ball (leaf 1).
  Pool                pool(hostMemoryResource());
  const Tape<T, Part> scene(pool, Union(Expr<T>(part), Expr<T>(ball)));

  size_t partLeaves = 0;

  for (int i = -6; i <= 8; i++) {
    for (int j = -3; j <= 3; j++) {
      const Vec3 p(T(0.43) * T(i) + T(0.05), T(0.41) * T(j) - T(0.07), T(0.13));
      const T    dPart = part.signedDistance(p);
      const T    dBall = ball.signedDistance(p);

      INFO("p = " << p[0] << ", " << p[1] << ", " << p[2]);

      const PrimitiveRef<T> ref = scene.closestPrimitive(p);

      // The part is decided as a whole: inside its drill hole too, where its value comes from the drill.
      REQUIRE(ref.m_leaf == ((dBall < dPart) ? 1U : 0U));

      if (ref.m_leaf == 0U) {
        partLeaves++;

        REQUIRE(scene.template isPrimitive<Part>(ref));
        CHECK(scene.template primitive<Part>(ref).signedDistance(ref.m_localPoint) == dPart);
      }
    }
  }

  CHECK(partLeaves > 0U);
}

TEMPLATE_TEST_CASE("Tape: closestPrimitive on a device matches the host", "[Tape][gpu]", EBGEOMETRY_TEST_PRECISIONS)
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

  const auto onDevice = evaluateOnDevice<PrimitiveRef<T>>(LeafQuery<T, Tape<T>>{tape.rebasedView(device)}, points);
  const auto onHost   = evaluateOnHost<PrimitiveRef<T>>(LeafQuery<T, Tape<T>>{tape}, points);

  size_t differ = 0;

  for (size_t i = 0; i < points.size(); i++) {
    differ += (onDevice[i].m_leaf != onHost[i].m_leaf) ? 1U : 0U;
  }

  // The device rounds differently (fused multiply-add), so a point where two leaves are nearly as close can
  // name the other; anything more is a fault.
  CHECK(differ <= points.size() / 100);
}
