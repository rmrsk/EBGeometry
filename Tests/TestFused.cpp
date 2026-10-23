// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Unit tests for the fused CSG operations (EBGeometry_Fused.hpp): each against an independent reference,
// each against a Tape of the same expression (both take their arithmetic from EBGeometry_CSGOps.hpp,
// so they agree exactly on the host), the boxes and distance qualities, pool-resident operands, use
// inside a Tape and a BVH union, device evaluation, and every rejected argument.

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"
#include "TestTapeUtils.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>
#include <type_traits>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using EBGeometry::TestUtils::Compiled;

namespace {

std::string
dodecahedron()
{
  return std::string(EBGEOMETRY_TEST_DATA_DIR) + "/dodecahedron.stl";
}

// Query points spread over a few units around the origin, none on a symmetry plane.
template <class T>
std::vector<Vec3T<T>>
probes()
{
  std::vector<Vec3T<T>> points;

  for (int i = -4; i <= 4; i++) {
    for (int j = -3; j <= 3; j++) {
      for (int k = -2; k <= 2; k++) {
        points.emplace_back(T(0.45) * T(i) + T(0.05), T(0.5) * T(j) - T(0.1), T(0.6) * T(k) + T(0.2));
      }
    }
  }

  return points;
}

// A tape compiled without BVH unions, so that it evaluates unions operand by operand, as fused does.
inline TapeOptions
linear()
{
  TapeOptions options;

  options.m_bvhUnionThreshold = std::numeric_limits<uint32_t>::max();

  return options;
}

// The analytic shapes the tests compose.
template <class T>
struct Shapes
{
  BoxSDF<T>              box{Vec3T<T>(T(-1), T(-0.8), T(-0.6)), Vec3T<T>(T(0.9), T(1), T(0.7))};
  SphereSDF<T>           ball{Vec3T<T>(T(0.1), T(-0.1), T(0)), T(1.2)};
  SphereSDF<T>           small{Vec3T<T>(T(0.8), T(0.3), T(-0.2)), T(0.5)};
  InfiniteCylinderSDF<T> drill{Vec3T<T>(T(0), T(0.1), T(0)), T(0.4), 0};
  TorusSDF<T>            torus{Vec3T<T>(T(-0.3), T(0), T(0.2)), T(0.9), T(0.2)};
  PlaneSDF<T>            plane{Vec3T<T>(T(0.1), T(0.2), T(0.3)), Vec3T<T>(T(1), T(2), T(-1))};
};

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

TEMPLATE_TEST_CASE("Fused: every operation is a trivially copyable shape, a pool handle only with a pool-resident "
                   "operand",
                   "[Fused]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Shapes<T> s;

  const auto check = [](const auto& a_shape) {
    using S = std::decay_t<decltype(a_shape)>;

    STATIC_REQUIRE(isShape<S>);
    STATIC_REQUIRE(std::is_trivially_copyable_v<S>);
    STATIC_REQUIRE(!S::isHandle);
  };

  check(Fused::Union(s.box, s.ball, s.drill));
  check(Fused::Intersection(s.box, s.ball));
  check(Fused::Difference(s.box, s.ball, s.drill));
  check(Fused::Complement(s.box));
  check(Fused::SmoothUnion(s.box, s.ball, T(0.1)));
  check(Fused::SmoothIntersection(Fused::operands(s.box, s.ball, s.small), T(0.1)));
  check(Fused::SmoothDifference(s.box, Fused::operands(s.ball, s.drill), T(0.1)));
  check(Fused::Translate(s.box, Vec3(T(1), T(2), T(3))));
  check(Fused::Rotate(s.box, T(30), size_t(1)));
  check(Fused::Rotate(s.box, T(30), Vec3(T(1), T(1), T(0))));
  check(Fused::Scale(s.box, T(2)));
  check(Fused::Reflect(s.box, size_t(2)));
  check(Fused::Offset(s.box, T(0.1)));
  check(Fused::Annular(s.box, T(0.1)));
  check(Fused::Elongate(s.box, Vec3(T(0.1), T(0), T(0.2))));
  check(Fused::FiniteRepetition(s.ball, Vec3(T(3), T(3), T(3)), Vec3::ones(), Vec3::ones()));
  check(Fused::Blur(s.box, T(0.1)));
  check(Fused::Mollify(s.box, T(0.1), size_t(3)));

  // One operand is returned as it is.
  STATIC_REQUIRE(std::is_same_v<std::decay_t<decltype(Fused::Union(s.box))>, BoxSDF<T>>);
  STATIC_REQUIRE(std::is_same_v<std::decay_t<decltype(Fused::SmoothUnion(Fused::operands(s.box), T(1)))>, BoxSDF<T>>);

  // A map of a map is one map.
  const auto twice = Fused::Translate(Fused::Rotate(s.box, T(30), size_t(1)), Vec3(T(1), T(0), T(0)));

  STATIC_REQUIRE(std::is_same_v<std::decay_t<decltype(twice)>, Fused::Node<Fused::AffineOp<T>, BoxSDF<T>>>);

  // A mesh SDF inside makes the composition a pool handle, all the way up.
  Pool pool(hostMemoryResource());

  const auto mesh = Parser::readIntoTriMeshSDF<T>(dodecahedron(), pool, BVH::Construction::SAH);
  const auto hole = Fused::Translate(Fused::Difference(mesh, s.drill), Vec3(T(1), T(0), T(0)));

  using H = std::decay_t<decltype(hole)>;

  STATIC_REQUIRE(isShape<H>);
  STATIC_REQUIRE(std::is_trivially_copyable_v<H>);
  STATIC_REQUIRE(H::isHandle);
  STATIC_REQUIRE(isPoolHandle<H>);
}

TEMPLATE_TEST_CASE("Fused: each operation matches its formula applied by hand", "[Fused]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Shapes<T> s;

  const T sm = T(0.3);

  for (const Vec3& p : probes<T>()) {
    const T box   = s.box.signedDistance(p);
    const T ball  = s.ball.signedDistance(p);
    const T small = s.small.signedDistance(p);
    const T drill = s.drill.signedDistance(p);

    INFO("p = " << p[0] << ", " << p[1] << ", " << p[2]);

    REQUIRE(Fused::Union(s.box, s.ball, s.drill).signedDistance(p) == std::min({box, ball, drill}));
    REQUIRE(Fused::Intersection(s.box, s.ball).signedDistance(p) == std::max(box, ball));
    REQUIRE(Fused::Difference(s.box, s.ball, s.drill).signedDistance(p) == std::max(box, -std::min(ball, drill)));
    REQUIRE(Fused::Complement(s.box).signedDistance(p) == -box);

    // A smooth union of two blends both; of three, only the two smallest.
    REQUIRE_THAT(Fused::SmoothUnion(s.box, s.ball, sm).signedDistance(p),
                 withinAbsT(SmoothMinOp<T>{}(box, ball, sm), tightMargin<T>()));
    REQUIRE_THAT(Fused::SmoothUnion(s.box, s.ball, sm, BlendKind::Exponential).signedDistance(p),
                 withinAbsT(ExpMinOp<T>{}(box, ball, sm), tightMargin<T>()));

    std::vector<T> three{box, ball, small};

    std::sort(three.begin(), three.end());

    REQUIRE_THAT(Fused::SmoothUnion(Fused::operands(s.box, s.ball, s.small), sm).signedDistance(p),
                 withinAbsT(SmoothMinOp<T>{}(three[0], three[1], sm), tightMargin<T>()));
    REQUIRE_THAT(Fused::SmoothIntersection(Fused::operands(s.box, s.ball, s.small), sm).signedDistance(p),
                 withinAbsT(SmoothMaxOp<T>{}(three[2], three[1], sm), tightMargin<T>()));
    REQUIRE_THAT(Fused::SmoothDifference(s.box, s.ball, sm).signedDistance(p),
                 withinAbsT(SmoothMaxOp<T>{}(box, -ball, sm), tightMargin<T>()));

    // The maps: the operand at the mapped point, a scaling times |s|.
    const Vec3 shift(T(0.4), T(-0.3), T(0.2));

    REQUIRE_THAT(Fused::Translate(s.box, shift).signedDistance(p),
                 withinAbsT(s.box.signedDistance(p - shift), tightMargin<T>()));
    REQUIRE_THAT(Fused::Scale(s.box, T(-2)).signedDistance(p),
                 withinAbsT(T(2) * s.box.signedDistance(p / T(-2)), tightMargin<T>()));
    REQUIRE_THAT(Fused::Reflect(s.box, size_t(1)).signedDistance(p),
                 withinAbsT(s.box.signedDistance(Vec3(p[0], -p[1], p[2])), tightMargin<T>()));

    // A quarter turn about z sends (x, y, z) to (-y, x, z), so the operand sees (y, -x, z).
    REQUIRE_THAT(Fused::Rotate(s.box, T(90), size_t(2)).signedDistance(p),
                 withinAbsT(s.box.signedDistance(Vec3(p[1], -p[0], p[2])), looseMargin<T>()));
    REQUIRE_THAT(Fused::Rotate(s.box, T(90), Vec3(T(0), T(0), T(2))).signedDistance(p),
                 withinAbsT(s.box.signedDistance(Vec3(p[1], -p[0], p[2])), looseMargin<T>()));

    // The value transforms.
    REQUIRE(Fused::Offset(s.box, T(0.25)).signedDistance(p) == box - T(0.25));
    REQUIRE(Fused::Annular(s.box, T(0.25)).signedDistance(p) == std::abs(box) - T(0.25));

    const Vec3 h(T(0.3), T(0), T(0.1));
    const Vec3 q(
      p[0] - std::clamp(p[0], -h[0], h[0]), p[1] - std::clamp(p[1], -h[1], h[1]), p[2] - std::clamp(p[2], -h[2], h[2]));

    REQUIRE(Fused::Elongate(s.box, h).signedDistance(p) == s.box.signedDistance(q));
  }

  // A blur or a mollification averages with weights that sum to one over taps symmetric about the
  // point, so it leaves a linear field (a plane) as it is.
  for (const Vec3& p : probes<T>()) {
    const T plane = s.plane.signedDistance(p);

    REQUIRE_THAT(Fused::Blur(s.plane, T(0.2), T(0.3)).signedDistance(p), withinAbsT(plane, looseMargin<T>()));
    REQUIRE_THAT(Fused::Mollify(s.plane, T(0.2), size_t(4)).signedDistance(p), withinAbsT(plane, looseMargin<T>()));
  }

  // Nothing to average: the operand itself.
  for (const Vec3& p : probes<T>()) {
    REQUIRE(Fused::Mollify(s.box, T(0), size_t(5)).signedDistance(p) == s.box.signedDistance(p));
    REQUIRE(Fused::Mollify(s.box, T(0.3), size_t(1)).signedDistance(p) == s.box.signedDistance(p));
  }
}

TEMPLATE_TEST_CASE(
  "Fused: a finite repetition is the minimum over all copies, off-centre and overlapping ones included",
  "[Fused]",
  EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Vec3 counts(2, 1, 2);

  const SphereSDF<T> offCentre(Vec3(T(0.4), T(0), T(0)), T(0.1));
  const SphereSDF<T> overlapping(Vec3::zeros(), T(1));

  for (const auto& [sphere, period] : {std::pair<SphereSDF<T>, Vec3>{offCentre, Vec3::ones()},
                                       std::pair<SphereSDF<T>, Vec3>{overlapping, Vec3(T(1.5), T(1.5), T(1.5))}}) {
    const auto tiled = Fused::FiniteRepetition(sphere, period, counts, counts);

    for (int i = -12; i <= 12; i++) {
      for (int j = -5; j <= 5; j++) {
        const Vec3 p(T(0.27) * T(i) - T(0.05), T(0.31) * T(j), T(0.13));

        T brute = std::numeric_limits<T>::max();

        for (int a = -int(counts[0]); a <= int(counts[0]); a++) {
          for (int b = -int(counts[1]); b <= int(counts[1]); b++) {
            for (int c = -int(counts[2]); c <= int(counts[2]); c++) {
              brute = std::min(brute, sphere.signedDistance(p - period * Vec3(T(a), T(b), T(c))));
            }
          }
        }

        INFO("p = " << p[0] << ", " << p[1] << ", " << p[2]);
        REQUIRE(tiled.signedDistance(p) == brute);
      }
    }
  }
}

TEMPLATE_TEST_CASE("Fused: equals a Tape of the same expression exactly", "[Fused]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Shapes<T> s;

  const Vec3 shift(T(0.4), T(-0.3), T(0.2));
  const Vec3 axis(T(1), T(-2), T(0.5));
  const Vec3 h(T(0.3), T(0), T(0.1));

  // Each fused composition beside the same expression built with the builder.
  const auto compare = [](const auto& a_fused, const Expr<T>& a_expr) {
    const Compiled<T> tape(a_expr, linear());

    for (const Vec3& p : probes<T>()) {
      INFO("p = " << p[0] << ", " << p[1] << ", " << p[2]);
      REQUIRE(a_fused.signedDistance(p) == tape(p));
    }
  };

  compare(Fused::Difference(Fused::Intersection(s.box, s.ball), s.drill, s.small),
          Difference(Intersection(s.box, s.ball), std::vector<Expr<T>>{s.drill, s.small}));
  compare(Fused::Union(s.box, s.ball, s.small, s.torus, s.drill),
          Union(std::vector<Expr<T>>{s.box, s.ball, s.small, s.torus, s.drill}));
  compare(Fused::SmoothUnion(Fused::operands(s.box, s.ball, s.small), T(0.3), BlendKind::Exponential),
          SmoothUnion(std::vector<Expr<T>>{s.box, s.ball, s.small}, T(0.3), BlendKind::Exponential));
  compare(Fused::SmoothIntersection(s.box, s.ball, T(0.2)), SmoothIntersection(s.box, s.ball, T(0.2)));
  compare(Fused::SmoothDifference(s.box, Fused::operands(s.small, s.drill), T(0.2)),
          SmoothDifference(s.box, std::vector<Expr<T>>{s.small, s.drill}, T(0.2)));
  compare(
    Fused::Scale(Fused::Translate(Fused::Rotate(Fused::Reflect(s.torus, size_t(0)), T(25), axis), shift), T(-1.5)),
    Scale(Translate(Rotate(Reflect(s.torus, size_t(0)), T(25), axis), shift), T(-1.5)));
  compare(Fused::Rotate(s.box, T(40), size_t(1)), Rotate(s.box, T(40), size_t(1)));
  compare(Fused::Annular(Fused::Offset(Fused::Complement(s.ball), T(-0.2)), T(0.1)),
          Annular(Offset(Complement(s.ball), T(-0.2)), T(0.1)));
  compare(Fused::Elongate(s.torus, h), Elongate(s.torus, h));
  compare(Fused::FiniteRepetition(s.small, Vec3(T(1.3), T(2), T(1.1)), Vec3(T(2), T(1), T(0)), Vec3(T(1), T(1), T(2))),
          FiniteRepetition(s.small, Vec3(T(1.3), T(2), T(1.1)), Vec3(T(2), T(1), T(0)), Vec3(T(1), T(1), T(2))));
  compare(Fused::Blur(s.box, T(0.15), T(0.4)), Blur(s.box, T(0.15), T(0.4)));
  compare(Fused::Mollify(s.torus, T(0.1), size_t(3)), Mollify(s.torus, T(0.1), size_t(3)));
  compare(Fused::Blur(
            Fused::FiniteRepetition(Fused::Union(s.small, s.drill), Vec3(T(2), T(2), T(2)), Vec3::ones(), Vec3::ones()),
            T(0.1)),
          Blur(FiniteRepetition(Union(s.small, s.drill), Vec3(T(2), T(2), T(2)), Vec3::ones(), Vec3::ones()), T(0.1)));
}

TEMPLATE_TEST_CASE("Fused: the bounding volumes and distance qualities are the expression builder's",
                   "[Fused]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Shapes<T> s;

  const auto compare = [](const auto& a_fused, const Expr<T>& a_expr) {
    using S = std::decay_t<decltype(a_fused)>;

    const BoundingVolumes::AABBT<T> fused = a_fused.computeBoundingVolume();
    const BoundingVolumes::AABBT<T> expr  = a_expr.computeBoundingVolume();

    for (size_t dir = 0; dir < 3; dir++) {
      REQUIRE(fused.getLowCorner()[dir] == expr.getLowCorner()[dir]);
      REQUIRE(fused.getHighCorner()[dir] == expr.getHighCorner()[dir]);
    }

    // The type's quality is decided without the run-time values, so it may be more cautious than the
    // builder's, never less.
    REQUIRE(static_cast<int>(S::distanceQuality) >= static_cast<int>(a_expr.getDistanceQuality()));
  };

  compare(Fused::Union(s.box, s.small), Union(s.box, s.small));
  compare(Fused::Intersection(s.box, s.ball), Intersection(s.box, s.ball));
  compare(Fused::Difference(s.box, s.small), Difference(s.box, s.small));
  compare(Fused::Complement(s.box), Complement(s.box));
  compare(Fused::SmoothUnion(s.box, s.small, T(0.4)), SmoothUnion(s.box, s.small, T(0.4)));
  compare(Fused::SmoothUnion(s.box, s.small, T(0.4), BlendKind::Exponential),
          SmoothUnion(s.box, s.small, T(0.4), BlendKind::Exponential));
  compare(Fused::SmoothIntersection(s.box, s.ball, T(0.4)), SmoothIntersection(s.box, s.ball, T(0.4)));
  compare(Fused::Translate(Fused::Rotate(s.box, T(30), size_t(2)), Vec3(T(1), T(2), T(3))),
          Translate(Rotate(s.box, T(30), size_t(2)), Vec3(T(1), T(2), T(3))));
  compare(Fused::Scale(s.box, T(-3)), Scale(s.box, T(-3)));
  compare(Fused::Offset(s.box, T(0.3)), Offset(s.box, T(0.3)));
  compare(Fused::Offset(s.box, T(-0.3)), Offset(s.box, T(-0.3)));
  compare(Fused::Annular(s.box, T(0.3)), Annular(s.box, T(0.3)));
  compare(Fused::Elongate(s.box, Vec3(T(0.5), T(0), T(1))), Elongate(s.box, Vec3(T(0.5), T(0), T(1))));
  compare(Fused::FiniteRepetition(s.small, Vec3(T(3), T(3), T(3)), Vec3(T(1), T(2), T(0)), Vec3(T(2), T(0), T(1))),
          FiniteRepetition(s.small, Vec3(T(3), T(3), T(3)), Vec3(T(1), T(2), T(0)), Vec3(T(2), T(0), T(1))));
  compare(Fused::Blur(s.box, T(0.2)), Blur(s.box, T(0.2)));
  compare(Fused::Mollify(s.box, T(0.2), size_t(3)), Mollify(s.box, T(0.2), size_t(3)));

  STATIC_REQUIRE(decltype(Fused::Complement(s.ball))::distanceQuality == DistanceQuality::Exact);
  STATIC_REQUIRE(decltype(Fused::Translate(s.ball, Vec3::ones()))::distanceQuality == DistanceQuality::Exact);
  STATIC_REQUIRE(decltype(Fused::Union(s.ball, s.small))::distanceQuality == DistanceQuality::Bound);
  STATIC_REQUIRE(decltype(Fused::FiniteRepetition(s.ball, Vec3::ones(), Vec3::ones(), Vec3::ones()))::distanceQuality ==
                 DistanceQuality::Bound);
}

TEMPLATE_TEST_CASE("Fused: a composition with a mesh SDF resolves through its pool, as a view, a deep copy and a "
                   "rebuilt layout",
                   "[Fused]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = BVH::DefaultBranchingRatio<T>();
  constexpr size_t W = TriangleSoA::DefaultWidth<T>();

  const Shapes<T> s;

  Pool pool(hostMemoryResource());

  const auto                mesh = Parser::readIntoDCEL<T>(dodecahedron(), pool);
  const TriMeshSDF<T, K, W> tri(mesh, pool, BVH::Construction::SAH, 4);
  const MeshSDF<T, K>       bvhMesh(mesh, pool, BVH::Construction::SAH);

  // A drilled mesh, and a smooth union of two copies of it with a sphere.
  const auto hole   = Fused::Difference(tri, s.drill);
  const auto pieces = Fused::SmoothUnion(Fused::operands(Fused::Translate(hole, Vec3(T(0.5), T(0), T(0))),
                                                         Fused::Rotate(bvhMesh, T(20), size_t(1)),
                                                         s.small),
                                         T(0.2));

  using P = std::decay_t<decltype(pieces)>;

  for (const Vec3& p : probes<T>()) {
    REQUIRE(hole.signedDistance(p) == std::max(tri.signedDistance(p), -s.drill.signedDistance(p)));
  }

  // The same expression through a Tape, built in the mesh's pool, gives the same values.
  const Tape<T> tape(pool,
                     SmoothUnion(std::vector<Expr<T>>{Translate(Difference(tri, s.drill), Vec3(T(0.5), T(0), T(0))),
                                                      Rotate(bvhMesh, T(20), size_t(1)),
                                                      s.small},
                                 T(0.2)),
                     linear());

  for (const Vec3& p : probes<T>()) {
    REQUIRE(pieces.signedDistance(p) == tape.signedDistance(p));
  }

  // A host mirror of the pool, a deep copy into another pool, and a node rebuilt from the layout.
  pool.freeze();

  const Pool mirror = Pool::mirror(pool, hostMemoryResource());
  Pool       other(hostMemoryResource());

  const P view = pieces.rebasedView(mirror);
  const P copy = pieces.deepCopy(other);
  const P same(pieces.layout(), pieces.location());

  for (const Vec3& p : probes<T>()) {
    const T value = pieces.signedDistance(p);

    REQUIRE(view.signedDistance(p) == value);
    REQUIRE(copy.signedDistance(p) == value);
    REQUIRE(same.signedDistance(p) == value);
  }
}

TEMPLATE_TEST_CASE("Fused: a fused shape is a Tape primitive and a BVH union's primitive",
                   "[Fused]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Shapes<T> s;

  const auto block = Fused::Difference(Fused::Intersection(s.box, s.ball), s.drill);

  using F = std::decay_t<decltype(block)>;

  // Listed as a user primitive, the fused shape is one clause, inside an interpreted expression.
  Pool pool(hostMemoryResource());

  const Tape<T, F> tape(pool, Translate(Expr<T>(block), Vec3(T(0.5), T(0), T(0))));

  for (const Vec3& p : probes<T>()) {
    REQUIRE(tape.signedDistance(p) == Fused::Translate(block, Vec3(T(0.5), T(0), T(0))).signedDistance(p));
  }

  // Many placements of it in a BVH union, against the plain minimum.
  using Placed = std::decay_t<decltype(Fused::Translate(block, Vec3::zeros()))>;

  std::vector<Placed>                    placed;
  std::vector<BoundingVolumes::AABBT<T>> boxes;

  for (int i = 0; i < 8; i++) {
    placed.push_back(Fused::Translate(block, Vec3(T(3) * T(i % 2), T(3) * T((i / 2) % 2), T(3) * T(i / 4))));
    boxes.push_back(placed.back().computeBoundingVolume());
  }

  const BVHUnion<T, Placed, 4> manyBlocks(pool, placed, boxes);

  for (const Vec3& p : probes<T>()) {
    T brute = std::numeric_limits<T>::max();

    for (const Placed& piece : placed) {
      brute = std::min(brute, piece.signedDistance(p));
    }

    REQUIRE(manyBlocks.signedDistance(p) == brute);
  }
}

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

TEST_CASE("Fused: rejects invalid arguments and operands from different pools", "[Fused]")
{
  using T    = double;
  using Vec3 = Vec3T<T>;

  const Shapes<T> s;

  const T inf = std::numeric_limits<T>::infinity();

  REQUIRE(abortsWith([&] { (void)Fused::SmoothUnion(s.box, s.ball, T(0)); },
                     "Fused::SmoothUnion: the smoothing length must be positive and finite"));
  REQUIRE(abortsWith([&] { (void)Fused::SmoothIntersection(Fused::operands(s.box, s.ball), -T(1)); },
                     "Fused::SmoothIntersection: the smoothing length must be positive and finite"));
  REQUIRE(abortsWith([&] { (void)Fused::SmoothDifference(s.box, s.ball, inf); },
                     "Fused::SmoothDifference: the smoothing length must be positive and finite"));
  REQUIRE(abortsWith([&] { (void)Fused::Translate(s.box, Vec3(inf, T(0), T(0))); },
                     "Fused::Translate: the shift must be finite"));
  REQUIRE(abortsWith([&] { (void)Fused::Rotate(s.box, inf, size_t(0)); },
                     "Fused::Rotate: the rotation angle must be finite"));
  REQUIRE(abortsWith([&] { (void)Fused::Rotate(s.box, T(10), size_t(3)); },
                     "Fused::Rotate: the rotation axis must be 0, 1 or 2"));
  REQUIRE(abortsWith([&] { (void)Fused::Rotate(s.box, T(10), Vec3::zeros()); },
                     "Fused::Rotate: the rotation axis must be finite and non-zero"));
  REQUIRE(
    abortsWith([&] { (void)Fused::Scale(s.box, T(0)); }, "Fused::Scale: the scale factor must be finite and non-zero"));
  REQUIRE(abortsWith([&] { (void)Fused::Reflect(s.box, size_t(3)); },
                     "Fused::Reflect: the reflection plane must be 0, 1 or 2"));
  REQUIRE(abortsWith([&] { (void)Fused::Offset(s.box, inf); }, "Fused::Offset: the offset must be finite"));
  REQUIRE(abortsWith([&] { (void)Fused::Annular(s.box, inf); }, "Fused::Annular: the half-thickness must be finite"));
  REQUIRE(abortsWith([&] { (void)Fused::Elongate(s.box, Vec3(-T(1), T(0), T(0))); },
                     "Fused::Elongate: the elongation must be finite and non-negative"));
  REQUIRE(abortsWith([&] { (void)Fused::FiniteRepetition(s.box, Vec3(T(0), T(1), T(1)), Vec3::ones(), Vec3::ones()); },
                     "Fused::FiniteRepetition: the period must be positive and finite in every direction"));
  REQUIRE(abortsWith([&] { (void)Fused::FiniteRepetition(s.box, Vec3::ones(), -Vec3::ones(), Vec3::ones()); },
                     "Fused::FiniteRepetition: the low repetition counts must be finite and not negative"));
  REQUIRE(abortsWith([&] { (void)Fused::FiniteRepetition(s.box, Vec3::ones(), Vec3::ones(), Vec3(inf, T(0), T(0))); },
                     "Fused::FiniteRepetition: the high repetition counts must be finite and not negative"));
  REQUIRE(abortsWith([&] { (void)Fused::Blur(s.box, -T(1)); },
                     "Fused::Blur: the blur distance must be finite and non-negative"));
  REQUIRE(abortsWith([&] { (void)Fused::Blur(s.box, T(1), T(2)); },
                     "Fused::Blur: the blending weight alpha must lie in [0, 1]"));
  REQUIRE(abortsWith([&] { (void)Fused::Mollify(s.box, inf, size_t(3)); },
                     "Fused::Mollify: the mollification distance must be finite"));
  REQUIRE(abortsWith([&] { (void)Fused::Mollify(s.box, T(1), size_t(0)); },
                     "Fused::Mollify: the number of samples per axis must be between 1 and 64"));

  REQUIRE(abortsWith(
    [] {
      constexpr size_t K = BVH::DefaultBranchingRatio<T>();

      Pool first(hostMemoryResource());
      Pool second(hostMemoryResource());

      const auto          mesh = Parser::readIntoDCEL<T>(dodecahedron(), first);
      const auto          copy = Parser::readIntoDCEL<T>(dodecahedron(), second);
      const MeshSDF<T, K> a(mesh, first, BVH::Construction::SAH);
      const MeshSDF<T, K> b(copy, second, BVH::Construction::SAH);

      (void)Fused::Union(a, b);
    },
    "Fused::Union: every pool-resident operand must live in the same pool"));
}
