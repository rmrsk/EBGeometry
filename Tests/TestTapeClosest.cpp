// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for Tape::closestLeaf() (EBGeometry_Tape.hpp, EBGeometry_LeafRef.hpp): the deciding
// leaf of unions, intersections and differences, through smooth blends, blurs, maps and repetitions, the
// numbering of repeated leaves, meshes and fused parts as leaves, and device evaluation. Also the labels
// of the leaves (SurfaceLabel(), Tape::getSurfaceLabel()) and the numbering of an expression's nodes (ExprIndex).

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

#include <cmath>
#include <cstdint>
#include <string>
#include <tuple>
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

// Five unit spheres three apart along x, labeled left, right, right, none and left: two names, one shared by
// separate objects, one given to a union.
template <class T>
Expr<T>
labeledRow()
{
  std::vector<SphereSDF<T>> s;

  for (int i = 0; i < 5; i++) {
    s.emplace_back(Vec3T<T>(T(3 * i), T(0), T(0)), T(1));
  }

  return Union(std::vector<Expr<T>>{
    SurfaceLabel(s[0], "left"), SurfaceLabel(Union(s[1], s[2]), "right"), s[3], SurfaceLabel(s[4], "left")});
}

} // namespace

TEMPLATE_TEST_CASE("Tape: closestLeaf names the operand a union, intersection or difference takes its value from",
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

  CHECK(carved.closestLeaf(Vec3(T(0.3), T(0), T(0))).m_leaf == 1U);
  CHECK(carved.closestLeaf(Vec3(T(-1.2), T(0), T(0))).m_leaf == 0U);
  CHECK(carved.template isPrimitive<SphereSDF<T>>(carved.closestLeaf(Vec3(T(0.3), T(0), T(0)))));
  CHECK(carved.template isPrimitive<BoxSDF<T>>(carved.closestLeaf(Vec3(T(-1.2), T(0), T(0)))));

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
      CHECK(unionBVH.closestLeaf(p).m_leaf == smallest);
      CHECK(unionLinear.closestLeaf(p).m_leaf == smallest);
      CHECK(intersection.closestLeaf(p).m_leaf == largest);

      const uint32_t difference = (box.signedDistance(p) >= -ball.signedDistance(p)) ? 0U : 1U;

      CHECK(carved.closestLeaf(p).m_leaf == difference);
    }
  }
}

TEMPLATE_TEST_CASE("Tape: closestLeaf follows smooth blends, blurs, maps and repetitions to the leaf, in its frame",
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

      CHECK(smooth.closestLeaf(p).m_leaf == smallest3);
      CHECK(blurred.closestLeaf(p).m_leaf == ((db < da) ? 1U : 0U));

      // A map: the leaf sees (p - shift) / 2, and the primitive there gives the leaf's own value.
      const Vec3       local = (p - shift) / T(2);
      const LeafRef<T> m     = moved.closestLeaf(p);

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

      const LeafRef<T> r = tiled.closestLeaf(p);

      CHECK(r.m_leaf == bestLeaf);
      CHECK_THAT(tiled.template primitive<SphereSDF<T>>(r).signedDistance(r.m_localPoint),
                 withinAbsT(best, tightMargin<T>()));
      CHECK_THAT(r.m_localPoint[0], withinAbsT(bestLocal[0], tightMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("Tape: closestLeaf numbers each occurrence of a leaf, and returns a mesh as its handle",
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

  CHECK(tape.closestLeaf(Vec3(T(1.5), T(1.6), T(0))).m_leaf == 1U);
  CHECK(tape.closestLeaf(Vec3(T(1.6), T(0), T(0))).m_leaf == 2U);

  const LeafRef<T> first  = tape.closestLeaf(Vec3(T(1.5), T(1.6), T(0)));
  const LeafRef<T> second = tape.closestLeaf(Vec3(T(1.6), T(0), T(0)));

  CHECK(first.m_type == second.m_type);

  // A mesh leaf comes back as a handle that resolves against the tape's pool.
  const Vec3       p(T(0.1), T(0.05), T(-0.2));
  const LeafRef<T> leaf = tape.closestLeaf(p);

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

TEMPLATE_TEST_CASE("Tape: closestLeaf treats a fused part as one leaf, the part a scene is made of",
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

      const LeafRef<T> ref = scene.closestLeaf(p);

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

TEMPLATE_TEST_CASE("ExprIndex: numbers nodes and leaves depth first, each node's leaves a contiguous range",
                   "[ExprIndex]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using N    = uint32_t;

  constexpr N None = ExprIndex<T>::None;

  const SphereSDF<T> sphere(Vec3::zeros(), T(1));
  const BoxSDF<T>    box(-Vec3::ones(), Vec3::ones());

  std::vector<Expr<T>> holes;

  for (int i = 0; i < 3; i++) {
    holes.push_back(Translate(sphere, Vec3(T(i), T(2), T(0))));
  }

  // Difference(a, b) is Intersection(a, Complement(b)).
  const ExprIndex<T> index(
    Difference(Intersection(SurfaceLabel(box, "wall"), sphere), SurfaceLabel(Union(holes, Accelerate::BVH), "bores")));

  // Node: parent, first leaf, leaves, own label, operation.
  const std::vector<std::tuple<N, N, N, N, std::string>> expected{{None, 0, 5, None, "Intersection"},
                                                                  {0, 0, 2, None, "Intersection"},
                                                                  {1, 0, 1, 0, "Primitive"},
                                                                  {1, 1, 1, None, "Primitive"},
                                                                  {0, 2, 3, None, "Complement"},
                                                                  {4, 2, 3, 1, "Union (BVH)"},
                                                                  {5, 2, 1, None, "Transform"},
                                                                  {6, 2, 1, None, "Primitive"},
                                                                  {5, 3, 1, None, "Transform"},
                                                                  {8, 3, 1, None, "Primitive"},
                                                                  {5, 4, 1, None, "Transform"},
                                                                  {10, 4, 1, None, "Primitive"}};

  REQUIRE(index.getNumNodes() == expected.size());

  for (N n = 0; n < index.getNumNodes(); n++) {
    const auto& node = index.getNode(n);

    CHECK(node.m_parent == std::get<0>(expected[n]));
    CHECK(node.m_firstLeaf == std::get<1>(expected[n]));
    CHECK(node.m_numLeaves == std::get<2>(expected[n]));
    CHECK(node.m_surfaceLabel.value() == std::get<3>(expected[n]));
    CHECK(node.m_operation == std::get<4>(expected[n]));
  }

  REQUIRE(index.getNumLeaves() == 5U);

  CHECK(index.getLeafNode(0) == 2U);
  CHECK(index.getLeafNode(4) == 11U);
  CHECK(index.pathOf(0) == std::vector<N>{0, 1, 2});
  CHECK(index.pathOf(3) == std::vector<N>{0, 4, 5, 8, 9});

  REQUIRE(index.getNumSurfaceLabels() == 2U);

  CHECK(index.getSurfaceLabelName(SurfaceLabelId(0)) == "wall");
  CHECK(index.getSurfaceLabelName(SurfaceLabelId(1)) == "bores");
  CHECK(index.findSurfaceLabel("bores").value() == 1U);
  CHECK(index.findSurfaceLabel("inlet").isNone());
  CHECK(index.getSurfaceLabel(0).value() == 0U);
  CHECK(index.getSurfaceLabel(1).isNone());

  for (N leaf = 2; leaf < 5; leaf++) {
    CHECK(index.getSurfaceLabel(leaf).value() == 1U);
  }
}

TEMPLATE_TEST_CASE(
  "ExprIndex: an operand's own label wins over a composition's, a relabel replaces (through maps too), "
  "and shared names share a label",
  "[ExprIndex]",
  EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const SphereSDF<T> a(Vec3::zeros(), T(1));
  const SphereSDF<T> b(Vec3::ones(), T(1));

  const ExprIndex<T> nested(SurfaceLabel(Union(SurfaceLabel(a, "inner"), b), "outer"));

  // Names are numbered as they first appear, depth first: the outer union comes first.
  CHECK(nested.getSurfaceLabelName(SurfaceLabelId(0)) == "outer");
  CHECK(nested.getSurfaceLabel(0) == nested.findSurfaceLabel("inner"));
  CHECK(nested.getSurfaceLabel(1) == nested.findSurfaceLabel("outer"));

  const ExprIndex<T> relabeled(SurfaceLabel(SurfaceLabel(a, "x"), "y"));

  CHECK(relabeled.getNumSurfaceLabels() == 1U);
  CHECK(relabeled.findSurfaceLabel("x").isNone());
  CHECK(relabeled.getSurfaceLabel(0) == relabeled.findSurfaceLabel("y"));

  // A move does not make another object: labeling the moved object relabels it, as MaterialLabel() does.
  const ExprIndex<T> moved(SurfaceLabel(Translate(Rotate(SurfaceLabel(a, "x"), T(30), 2), Vec3::ones()), "y"));

  CHECK(moved.getNumSurfaceLabels() == 1U);
  CHECK(moved.findSurfaceLabel("x").isNone());
  CHECK(moved.getSurfaceLabel(0) == moved.findSurfaceLabel("y"));

  // Below a composition, though, the label is a default again: the operand labeled itself keeps its own.
  const ExprIndex<T> assembly(SurfaceLabel(Translate(Union(SurfaceLabel(a, "x"), b), Vec3::ones()), "y"));

  CHECK(assembly.getSurfaceLabel(0) == assembly.findSurfaceLabel("x"));
  CHECK(assembly.getSurfaceLabel(1) == assembly.findSurfaceLabel("y"));

  // A subexpression used twice is numbered at each use; its label is shared, and the copy SurfaceLabel() makes leaves
  // the unlabeled use alone.
  const Expr<T>      pair = Union(a, b);
  const ExprIndex<T> shared(
    Union(std::vector<Expr<T>>{SurfaceLabel(pair, "p"), Translate(pair, Vec3::ones()), SurfaceLabel(b, "p")}));

  CHECK(shared.getNumLeaves() == 5U);
  CHECK(shared.getNumSurfaceLabels() == 1U);
  CHECK(shared.getSurfaceLabel(0).value() == 0U);
  CHECK(shared.getSurfaceLabel(1).value() == 0U);
  CHECK(shared.getSurfaceLabel(2).isNone());
  CHECK(shared.getSurfaceLabel(3).isNone());
  CHECK(shared.getSurfaceLabel(4).value() == 0U);
}

TEMPLATE_TEST_CASE("SurfaceLabel: a labeled map is not merged into the maps around it, and the value does not change",
                   "[ExprIndex]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const SphereSDF<T> sphere(Vec3::zeros(), T(1));
  const Vec3         v(T(1), T(0), T(0));
  const Vec3         w(T(0), T(2), T(0));

  const Expr<T> plain   = Translate(Translate(sphere, v), w);
  const Expr<T> labeled = Translate(SurfaceLabel(Translate(sphere, v), "moved"), w);

  CHECK(ExprIndex<T>(plain).getNumNodes() == 2U);
  CHECK(ExprIndex<T>(labeled).getNumNodes() == 3U);
  CHECK(ExprIndex<T>(labeled).getNode(1).m_operation == "Transform");

  Pool pool(hostMemoryResource());

  const Tape<T> a(pool, plain);
  const Tape<T> b(pool, labeled);

  for (const Vec3& p : {Vec3::zeros(), Vec3(T(1), T(2), T(0.5)), Vec3(T(-2), T(1), T(3))}) {
    const T reference = (p - v - w).length() - T(1);

    CHECK_THAT(a.signedDistance(p), withinAbsT<T>(reference, looseMargin<T>()));
    CHECK_THAT(b.signedDistance(p), withinAbsT<T>(reference, looseMargin<T>()));
  }

  // Without a map in between, a label adds no clause: the values are the same, bit for bit.
  const Tape<T> unlabeled(pool,
                          Union(std::vector<Expr<T>>{SphereSDF<T>(Vec3::zeros(), T(1)),
                                                     Union(SphereSDF<T>(Vec3(T(3), T(0), T(0)), T(1)),
                                                           SphereSDF<T>(Vec3(T(6), T(0), T(0)), T(1))),
                                                     SphereSDF<T>(Vec3(T(9), T(0), T(0)), T(1)),
                                                     SphereSDF<T>(Vec3(T(12), T(0), T(0)), T(1))}));
  const Tape<T> row(pool, labeledRow<T>());

  for (int i = 0; i < 50; i++) {
    const Vec3 p(T(0.3) * T(i) - T(1), T(0.1) * T(i % 7), T(0.2));

    CHECK(row.signedDistance(p) == unlabeled.signedDistance(p));
  }
}

TEMPLATE_TEST_CASE("Tape: getSurfaceLabel names the label of the leaf closestLeaf and a ray hit report",
                   "[Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  const Tape<T>      tape(pool, labeledRow<T>());
  const ExprIndex<T> index(labeledRow<T>());

  const SurfaceLabelId left  = index.findSurfaceLabel("left");
  const SurfaceLabelId right = index.findSurfaceLabel("right");

  const std::vector<SurfaceLabelId> expected{left, right, right, SurfaceLabelId::none(), left};

  REQUIRE(tape.getNumSurfaceLabels() == 2U);
  REQUIRE(tape.getNumPrimitives() == 5U);

  for (uint32_t i = 0; i < 5; i++) {
    const Vec3 center(T(3 * i), T(0), T(0));

    // Just outside sphere i, and two away from the others.
    const LeafRef<T> ref = tape.closestLeaf(center + Vec3(T(0), T(1.1), T(0)));
    const HitT<T>    hit = tape.firstHit(RayT<T>{center + Vec3(T(0), T(5), T(0)), Vec3(0, -1, 0)}, T(0), T(10));

    REQUIRE(ref.m_leaf == i);
    REQUIRE(hit.hit());
    REQUIRE(hit.m_primitive == i);

    CHECK(tape.getSurfaceLabel(ref) == expected[i]);
    CHECK(tape.getSurfaceLabel(hit.m_primitive) == expected[i]);
    CHECK(index.getSurfaceLabel(i) == expected[i]);
  }

  CHECK(tape.getSurfaceLabel(5U).isNone());

  const Tape<T> copy = tape.deepCopy(pool);

  CHECK(copy.getNumSurfaceLabels() == 2U);

  for (uint32_t i = 0; i < 5; i++) {
    CHECK(copy.getSurfaceLabel(i) == expected[i]);
  }

  // A tape without labels stores none.
  const Tape<T> plain(pool, Union(SphereSDF<T>(Vec3::zeros(), T(1)), SphereSDF<T>(Vec3::ones(), T(1))));

  CHECK(plain.getNumSurfaceLabels() == 0U);
  CHECK(plain.getSurfaceLabel(0U).isNone());
}

TEMPLATE_TEST_CASE("Tape: labels follow the tape's leaf numbering through BVH unions, differences, maps and blurs",
                   "[Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // Every sphere is named by its own center's x, so a leaf's label name says which sphere it is, whatever
  // number the tape gave the leaf. Each sphere sits in its own 4-wide slot along x.
  const auto named = [](const int a_slot, const Vec3& a_center, const T a_radius) {
    return SurfaceLabel(SphereSDF<T>(a_center, a_radius), "s" + std::to_string(a_slot));
  };

  std::vector<Expr<T>> bvhOperands;

  // Rotated a half turn about z to (1, 0, 0), then translated back to the origin.
  bvhOperands.push_back(Translate(Rotate(named(0, Vec3(T(-1), T(0), T(0)), T(1)), T(180), 2), Vec3(T(-1), T(0), T(0))));
  bvhOperands.push_back(named(1, Vec3(T(4), T(0), T(0)), T(1)));
  bvhOperands.push_back(Blur(named(2, Vec3(T(8), T(0), T(0)), T(1)), T(0.05)));
  bvhOperands.push_back(Scale(named(3, Vec3(T(6), T(0), T(0)), T(0.5)), T(2))); // center 12, radius 1

  // A ball with a cavity carved out of it, after the union: the cavity's wall is decided by the carving sphere.
  const Expr<T> holed =
    Difference(named(4, Vec3(T(16), T(0), T(0)), T(1.5)), named(5, Vec3(T(16), T(0), T(0)), T(0.5)));

  const Expr<T>      expr = Union(Union(bvhOperands, Accelerate::BVH), holed);
  const ExprIndex<T> index(expr);

  Pool pool(hostMemoryResource());

  const Tape<T> tape(pool, expr);

  REQUIRE(tape.getNumPrimitives() == index.getNumLeaves());
  REQUIRE(tape.getNumSurfaceLabels() == 6U);

  // A point just outside each sphere (inside the cavity for the carving one), and the sphere it must name.
  const std::vector<std::pair<Vec3, int>> probes{{Vec3(T(0), T(1.2), T(0)), 0},
                                                 {Vec3(T(4), T(1.2), T(0)), 1},
                                                 {Vec3(T(8), T(1.2), T(0)), 2},
                                                 {Vec3(T(12), T(1.2), T(0)), 3},
                                                 {Vec3(T(16), T(1.7), T(0)), 4},
                                                 {Vec3(T(16), T(0.3), T(0)), 5}};

  for (const auto& [point, slot] : probes) {
    const LeafRef<T> ref = tape.closestLeaf(point);

    REQUIRE(ref.m_leaf != LeafRef<T>::None);
    CHECK(index.getSurfaceLabelName(tape.getSurfaceLabel(ref)) == "s" + std::to_string(slot));
  }
}

TEMPLATE_TEST_CASE("Label and ExprIndex: reject an empty name and numbers out of range",
                   "[ExprIndex]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  REQUIRE(abortsWith([] { (void)SurfaceLabel(SphereSDF<T>(Vec3T<T>::zeros(), T(1)), ""); },
                     "SurfaceLabel: the name must not be empty"));
  REQUIRE(abortsWith(
    [] {
      const ExprIndex<T> index(toExpr(SphereSDF<T>(Vec3T<T>::zeros(), T(1))));

      (void)index.getNode(1);
    },
    "ExprIndex: node 1 of 1"));
  REQUIRE(abortsWith(
    [] {
      const ExprIndex<T> index(toExpr(SphereSDF<T>(Vec3T<T>::zeros(), T(1))));

      (void)index.pathOf(1);
    },
    "ExprIndex: leaf 1 of 1"));
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
