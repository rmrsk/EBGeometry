// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Unit tests for the ray queries of offsets and annuli: of analytic shapes and of meshes, traced to their
// level sets in a tape and fused, and device evaluation.

#include "TestRayHelpers.hpp"

using namespace EBGeometry;
using namespace EBGeometryTestRay;

TEMPLATE_TEST_CASE("Ray: offsets and annuli of analytic shapes are traced to their closed-form surfaces",
                   "[Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  const Vec3 x(1, 0, 0);
  const Vec3 y(0, 1, 0);

  // Each crossing fused and through a tape with exact offsets: exact, at the closed-form distance, with the
  // closed-form normal.
  const auto check = [&](const auto&    a_fused,
                         const Expr<T>& a_expr,
                         const Vec3&    a_from,
                         const Vec3&    a_direction,
                         const T        a_t,
                         const bool     a_entering,
                         const Vec3&    a_normal) {
    const Tape<T, TapeFeature::ExactOffsets> tape(pool, a_expr);
    const RayT<T>                            ray{a_from, a_direction};

    for (const HitT<T>& hit : {firstHit(a_fused, ray, T(0), T(10)), tape.firstHit(ray, T(0), T(10))}) {
      REQUIRE(hit.hit());
      REQUIRE(hit.m_exact);
      REQUIRE(hit.m_entering == a_entering);
      REQUIRE_THAT(hit.m_t, withinAbsT(a_t, tightMargin<T>()));
      REQUIRE_THAT(dot(hit.m_normal, a_normal), withinAbsT(T(1), tightMargin<T>()));
    }
  };

  // The unit ball's shell 0.2 thick: entered at radius 1.1 from outside, and at radius 0.9 from the hollow.
  const SphereSDF<T> ball(Vec3::zeros(), T(1));

  check(Fused::Annular(ball, T(0.1)), Annular(ball, T(0.1)), -T(3) * x, x, T(1.9), true, -x);
  check(Fused::Annular(ball, T(0.1)), Annular(ball, T(0.1)), Vec3::zeros(), x, T(0.9), true, -x);

  // A box grown by 0.2, its corner rounded, and shrunk by 0.2.
  const BoxSDF<T> box(-Vec3::ones(), Vec3::ones());
  const Vec3      diagonal = Vec3::ones() / std::sqrt(T(3));

  check(Fused::Offset(box, T(0.2)), Offset(box, T(0.2)), -T(3) * x, x, T(1.8), true, -x);
  check(Fused::Offset(box, T(0.2)),
        Offset(box, T(0.2)),
        T(3) * Vec3::ones(),
        -diagonal,
        T(2) * std::sqrt(T(3)) - T(0.2),
        true,
        diagonal);
  check(Fused::Offset(box, -T(0.2)), Offset(box, -T(0.2)), -T(3) * x, x, T(2.2), true, -x);

  // A capsule with its tips at x = -1 and 1, of radius 0.3 (its ends' centers at x = -0.7 and 0.7), grown by 0.2.
  const CapsuleSDF<T> capsule(-x, x, T(0.3));

  check(Fused::Offset(capsule, T(0.2)), Offset(capsule, T(0.2)), -T(3) * y, y, T(2.5), true, -y);
  check(Fused::Offset(capsule, T(0.2)), Offset(capsule, T(0.2)), -T(3) * x, x, T(1.8), true, -x);

  // A plane through the origin across x, as a slab 0.5 thick: entered at x = 0.25, and left there from inside.
  const PlaneSDF<T> plane(Vec3::zeros(), x);

  check(Fused::Annular(plane, T(0.25)), Annular(plane, T(0.25)), T(2) * x, -x, T(1.75), true, x);
  check(Fused::Annular(plane, T(0.25)), Annular(plane, T(0.25)), Vec3::zeros(), x, T(0.25), false, x);

  // An infinite cylinder along z of radius 0.5, its shell 0.2 thick.
  const InfiniteCylinderSDF<T> tube(Vec3::zeros(), T(0.5), 2);

  check(Fused::Annular(tube, T(0.1)), Annular(tube, T(0.1)), -T(3) * x, x, T(2.4), true, -x);
  check(Fused::Annular(tube, T(0.1)), Annular(tube, T(0.1)), Vec3::zeros(), x, T(0.4), true, -x);

  // Through a scale by 2: the annulus of the scaled ball is 0.2 thick about radius 2; the scaled annulus is 0.4
  // thick about it.
  check(Fused::Annular(Fused::Scale(ball, T(2)), T(0.1)),
        Annular(Scale(ball, T(2)), T(0.1)),
        -T(5) * x,
        x,
        T(2.9),
        true,
        -x);
  check(Fused::Scale(Fused::Annular(ball, T(0.1)), T(2)),
        Scale(Annular(ball, T(0.1)), T(2)),
        -T(5) * x,
        x,
        T(2.8),
        true,
        -x);

  // A shell of a shell has no closed form: it is marched.
  const Tape<T, TapeFeature::ExactOffsets> nested(pool, Annular(Annular(ball, T(0.2)), T(0.05)));
  const HitT<T>                            marched = nested.firstHit(RayT<T>{-T(3) * x, x}, T(0), T(10));

  REQUIRE(marched.hit());
  REQUIRE_FALSE(marched.m_exact);
  REQUIRE_THAT(marched.m_t, withinAbsT(T(1.75), 1e-3));

  // A tape without the feature marches a shell it could trace, to the same crossing.
  const Tape<T> plain(pool, Annular(ball, T(0.1)));
  const HitT<T> plainHit = plain.firstHit(RayT<T>{-T(3) * x, x}, T(0), T(10));

  REQUIRE(plainHit.hit());
  REQUIRE_FALSE(plainHit.m_exact);
  REQUIRE_THAT(plainHit.m_t, withinAbsT(T(1.9), 1e-3));
}

TEMPLATE_TEST_CASE("Ray: offsets and annuli of meshes are traced to their faces offset by the level",
                   "[Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  const auto triangles = Parser::readIntoTriMeshSDF<T, 4, 4>(unitCube(), pool);
  const auto polygons  = Parser::readIntoMeshSDF<T, 4>(unitCube(), pool);
  const auto flat      = Parser::readIntoFlatMeshSDF<T>(unitCube(), pool);

  const Vec3 x(1, 0, 0);
  const Vec3 side(T(-3), T(0.5), T(0.5));
  const Vec3 diagonal = Vec3::ones() / std::sqrt(T(3));

  // The unit cube's crossings, for each mesh SDF fused and through a tape (the triangle mesh's), exact.
  const auto check = [&](const auto&    a_triangles,
                         const auto&    a_polygons,
                         const auto&    a_flat,
                         const Expr<T>& a_expr,
                         const Vec3&    a_from,
                         const Vec3&    a_direction,
                         const T        a_tMin,
                         const T        a_t,
                         const bool     a_entering,
                         const Vec3&    a_normal) {
    const Tape<T, TapeFeature::ExactOffsets, TriMeshSDF<T, 4, 4>> tape(pool, a_expr);
    const RayT<T>                                                 ray{a_from, a_direction};

    for (const HitT<T>& hit : {firstHit(a_triangles, ray, a_tMin, T(10)),
                               firstHit(a_polygons, ray, a_tMin, T(10)),
                               firstHit(a_flat, ray, a_tMin, T(10)),
                               tape.firstHit(ray, a_tMin, T(10))}) {
      REQUIRE(hit.hit());
      REQUIRE(hit.m_exact);
      REQUIRE(hit.m_entering == a_entering);
      REQUIRE_THAT(hit.m_t, withinAbsT(a_t, looseMargin<T>()));
      REQUIRE_THAT(dot(hit.m_normal, a_normal), withinAbsT(T(1), looseMargin<T>()));
    }
  };

  const auto annulus =
    [&](const Vec3& a_from, const Vec3& a_direction, const T a_tMin, const T a_t, const bool a_in, const Vec3& a_n) {
      check(Fused::Annular(triangles, T(0.1)),
            Fused::Annular(polygons, T(0.1)),
            Fused::Annular(flat, T(0.1)),
            Annular(triangles, T(0.1)),
            a_from,
            a_direction,
            a_tMin,
            a_t,
            a_in,
            a_n);
    };

  // The wall 0.2 thick about the face x = 0: entered at x = -0.1; from inside it, left at x = 0.1 into the hollow.
  annulus(side, x, T(0), T(2.9), true, -x);
  annulus(side, x, T(2.95), T(3.1), false, x);

  // Towards the corner at the origin along the diagonal: the ball about the corner.
  annulus(-T(2) * Vec3::ones(), diagonal, T(0), T(2) * std::sqrt(T(3)) - T(0.1), true, -diagonal);

  // Grown by 0.1: entered at x = -0.1; from the center, left at x = 1.1. Shrunk by 0.1: entered at x = 0.1;
  // from the center, left at x = 0.9.
  const auto offset = [&](const T a_r, const Vec3& a_from, const T a_t, const bool a_in, const Vec3& a_n) {
    check(Fused::Offset(triangles, a_r),
          Fused::Offset(polygons, a_r),
          Fused::Offset(flat, a_r),
          Offset(triangles, a_r),
          a_from,
          x,
          T(0),
          a_t,
          a_in,
          a_n);
  };

  offset(T(0.1), side, T(2.9), true, -x);
  offset(T(0.1), T(0.5) * Vec3::ones(), T(0.6), false, x);
  offset(-T(0.1), side, T(3.1), true, -x);
  offset(-T(0.1), T(0.5) * Vec3::ones(), T(0.4), false, x);

  // Less a box past x = 0.5, the wall traced only within the box's complement (an intersection defers a mesh's
  // offset faces to its other operands' spans): still entered at x = -0.1; and so under a scale by 2, the box
  // past x = 1.
  const BoxSDF<T> cut(Vec3(T(0.5), -T(1), -T(1)), Vec3(T(3), T(2), T(2)));
  const BoxSDF<T> scaledCut(Vec3(T(1), -T(1), -T(1)), Vec3(T(3), T(3), T(3)));

  check(Fused::Difference(Fused::Annular(triangles, T(0.1)), cut),
        Fused::Difference(Fused::Annular(polygons, T(0.1)), cut),
        Fused::Difference(Fused::Annular(flat, T(0.1)), cut),
        Difference(Annular(triangles, T(0.1)), cut),
        side,
        x,
        T(0),
        T(2.9),
        true,
        -x);
  check(Fused::Difference(Fused::Annular(Fused::Scale(triangles, T(2)), T(0.1)), scaledCut),
        Fused::Difference(Fused::Annular(Fused::Scale(polygons, T(2)), T(0.1)), scaledCut),
        Fused::Difference(Fused::Annular(Fused::Scale(flat, T(2)), T(0.1)), scaledCut),
        Difference(Annular(Scale(triangles, T(2)), T(0.1)), scaledCut),
        Vec3(-T(3), T(1), T(1)),
        x,
        T(0),
        T(2.9),
        true,
        -x);
}

TEMPLATE_TEST_CASE("Ray: device ray queries of offsets and annuli match the host",
                   "[Ray][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  const std::vector<RayT<T>> rays = deviceRays<T>();

  // Offsets and annuli traced from their level sets: of the meshes, in a tape and fused, and of a box under a
  // scale in a tape, against device mirrors of their pool.
  Pool pool(hostMemoryResource());

  const auto cube     = Parser::readIntoTriMeshSDF<T, 4, 4>(unitCube(), pool);
  const auto polygons = Parser::readIntoMeshSDF<T, 4>(unitCube(), pool);
  const auto flat     = Parser::readIntoFlatMeshSDF<T>(unitCube(), pool);

  using M  = std::decay_t<decltype(cube)>;
  using TM = Tape<T, TapeFeature::ExactOffsets, M>;
  using TX = Tape<T, TapeFeature::ExactOffsets>;

  const TM   annulusTape(pool, Annular(cube, T(0.1)));
  const TM   offsetTape(pool, Offset(cube, T(-0.1)));
  const TX   boxTape(pool, Scale(Offset(BoxSDF<T>(-T(0.5) * Vec3::ones(), T(0.4) * Vec3::ones()), T(0.15)), T(1.5)));
  const auto annulusFused = Fused::Annular(polygons, T(0.1));
  const auto annulusFlat  = Fused::Annular(flat, T(0.1));

  pool.freeze();

  const Pool device = Pool::mirror(pool, deviceTestResource());

  using A  = std::decay_t<decltype(annulusFused)>;
  using AF = std::decay_t<decltype(annulusFlat)>;

  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, TM>{annulusTape.rebasedView(device)}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, TM>{annulusTape}, rays));
  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, TM>{offsetTape.rebasedView(device)}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, TM>{offsetTape}, rays));
  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, TX>{boxTape.rebasedView(device)}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, TX>{boxTape}, rays));
  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, A>{annulusFused.rebasedView(device)}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, A>{annulusFused}, rays));
  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, AF>{annulusFlat.rebasedView(device)}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, AF>{annulusFlat}, rays));
}
