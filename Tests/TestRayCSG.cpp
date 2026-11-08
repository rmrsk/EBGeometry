// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Unit tests for the ray queries of compositions: the ray-form traits, the span arithmetic against sets of
// points, compositions (fused, BVH unions and tapes, with mesh SDFs, marched parts and user shapes) against
// hand-computed crossings and against their values sampled along the ray, firstCrossing() on every kind
// of shape, smooth unions and intersections (fused, and marched through a tape), full span lists, and
// device evaluation.

#include <array>

#include "TestRayHelpers.hpp"

using namespace EBGeometry;
using namespace EBGeometryTestRay;

TEMPLATE_TEST_CASE("Ray: hasRayForm and hasRaySpans tell a ray form by its signatures",
                   "[Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  STATIC_REQUIRE(hasRayForm<SphereSDF<T>, T>);
  STATIC_REQUIRE(hasRaySpans<SphereSDF<T>, T>);
  STATIC_REQUIRE(hasRayForm<FirstHitBall<T>, T>);
  STATIC_REQUIRE_FALSE(hasRaySpans<FirstHitBall<T>, T>);
  STATIC_REQUIRE_FALSE(hasRayForm<MistypedBall<T>, T>);
  STATIC_REQUIRE_FALSE(hasRayForm<TorusSDF<T>, T>);
  STATIC_REQUIRE(hasRayForm<Tape<T>, T>);
  STATIC_REQUIRE(hasRaySpans<decltype(Fused::Union(SphereSDF<T>(), BoxSDF<T>())), T>);
  STATIC_REQUIRE_FALSE(hasRayForm<decltype(Fused::Blur(SphereSDF<T>(), T(0.1))), T>);
}

TEMPLATE_TEST_CASE("Ray: compositions of the shapes with exact ray forms are traced exactly, in a tape and fused",
                   "[Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const EllipsoidSDF<T>    ellipsoid(Vec3(T(0.1), T(0), -T(0.2)), Vec3(T(0.5), T(0.3), T(0.4)));
  const CappedConeSDF<T>   cone(Vec3(-T(0.8), -T(0.5), T(0)), Vec3(-T(0.2), T(0.6), T(0.1)), T(0.5), T(0.1));
  const PolygonPrismSDF<T> prism(Vec3(T(0.2), -T(0.8), T(0)), Vec3(T(0.3), T(0.8), T(0.2)), T(0.4), 6U, Vec3::unit(0));
  const TubeSDF<T>         tube(Vec3(-T(0.9), T(0), -T(0.6)), Vec3(T(0.9), T(0), -T(0.5)), T(0.15), T(0.3));
  const CutSphereSDF<T>    dome(Vec3(T(0), T(0.4), T(0.5)), T(0.5), T(0.1));
  const WedgeSDF<T>        wedge(Vec3(T(0), T(0.9), T(0)), T(100));

  const auto fused = Fused::Intersection(Fused::Difference(Fused::Union(cone, prism, tube, dome), ellipsoid), wedge);

  Pool          pool(hostMemoryResource());
  const Tape<T> tape(pool,
                     Intersection(Difference(Union(std::vector<Expr<T>>{cone, prism, tube, dome}), ellipsoid), wedge));

  std::mt19937                      rng(17);
  std::uniform_real_distribution<T> uniform(-T(1), T(1));

  int hits = 0;

  for (int i = 0; i < 400; i++) {
    const Vec3    origin = T(3) * normalized(Vec3(uniform(rng), uniform(rng), uniform(rng)));
    const Vec3    target = T(0.6) * Vec3(uniform(rng), uniform(rng), uniform(rng));
    const RayT<T> ray{origin, normalized(target - origin)};

    const HitT<T> viaFused = firstHit(fused, ray, T(0), T(10));
    const HitT<T> viaTape  = tape.firstHit(ray, T(0), T(10));

    INFO("ray " << i << " from " << origin);
    REQUIRE(viaTape.m_status == viaFused.m_status);

    if (viaFused.hit()) {
      REQUIRE(viaFused.m_exact);
      REQUIRE(viaTape.m_exact);
      REQUIRE_THAT(viaTape.m_t, withinAbsT(viaFused.m_t, looseMargin<T>()));
      REQUIRE_THAT(std::abs(fused.signedDistance(viaFused.m_point)), withinAbsT(T(0), 1e-3));

      hits++;
    }
  }

  REQUIRE(hits > 50);
}

TEMPLATE_TEST_CASE("Ray: fused and taped compositions are traced to the closed-form crossings, and marched on request",
                   "[Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Part<T> part;
  const auto    fused = part.fused();

  Pool          pool(hostMemoryResource());
  const Tape<T> tape(pool, part.expr());

  // Traced: exact, to rounding. Marched (RayOptions::m_march): to the march tolerance, 1e-4 of the part's
  // box diagonal (about 3.5e-4).
  const double exactMargin = looseMargin<T>();
  const double margin      = 1e-3;

  RayOptions<T> marchOptions;

  marchOptions.m_march = true;

  struct Case
  {
    const char* name;
    Vec3        from;
    Vec3        to;
    T           t;        // Expected crossing, negative for none.
    bool        entering; // Expected for a crossing.
    Vec3        normal;   // Expected outward normal.
    uint32_t    leaf;     // Expected deciding leaf: cube 0, ball 1, drills 2 (x), 3 (y), 4 (z).
  };

  const Vec3 diagonal = unit(T(1), T(1), T(1));

  const std::vector<Case> cases{
    // Into a cube face, clear of the ball and the drills.
    {"cube face", Vec3(-T(3), T(0.7), T(0.1)), Vec3(T(3), T(0.7), T(0.1)), T(2), true, Vec3(-1, 0, 0), 0U},
    // Into the ball, which rounds the cube's corner.
    {"ball", T(3) * diagonal, -T(3) * diagonal, T(3) - T(1.35), true, diagonal, 1U},
    // From inside the solid out into the z drill.
    {"drill wall", Vec3(-T(0.8), T(0), T(0.7)), Vec3(T(0.8), T(0), T(0.7)), T(0.25), false, Vec3(1, 0, 0), 4U},
    // Down the x drill, all the way through: no crossing.
    {"through the drill", Vec3(-T(3), T(0.1), T(0.1)), Vec3(T(3), T(0.1), T(0.1)), -T(1), false, Vec3::zeros(), 0U},
    // Stopping short of the cube.
    {"short", Vec3(-T(3), T(0.7), T(0.1)), Vec3(-T(1.5), T(0.7), T(0.1)), -T(1), false, Vec3::zeros(), 0U},
  };

  for (const Case& c : cases) {
    INFO(c.name);

    const HitT<T> viaFused = firstHit(fused, c.from, c.to);
    const HitT<T> viaTape  = tape.firstHit(c.from, c.to);

    if (c.t < T(0)) {
      REQUIRE(viaFused.m_status == HitStatus::Miss);
      REQUIRE(viaTape.m_status == HitStatus::Miss);
      REQUIRE(tape.intersects(c.from, c.to) == HitStatus::Miss);
      REQUIRE(intersects(fused, c.from, c.to) == HitStatus::Miss);

      continue;
    }

    for (const HitT<T>& hit : {viaFused, viaTape}) {
      REQUIRE(hit.hit());
      REQUIRE(hit.m_exact);
      REQUIRE(hit.m_entering == c.entering);
      REQUIRE_THAT(hit.m_t, withinAbsT(c.t, exactMargin));
      REQUIRE_THAT(dot(hit.m_normal, c.normal), withinAbsT(T(1), exactMargin));
    }

    const HitT<T> marchedFused = firstHit(fused, c.from, c.to, T(0), marchOptions);
    const HitT<T> marchedTape  = tape.firstHit(c.from, c.to, T(0), marchOptions);

    for (const HitT<T>& hit : {marchedFused, marchedTape}) {
      REQUIRE(hit.hit());
      REQUIRE_FALSE(hit.m_exact);
      REQUIRE(hit.m_entering == c.entering);
      REQUIRE_THAT(hit.m_t, withinAbsT(c.t, margin));
      REQUIRE_THAT(dot(hit.m_normal, c.normal), withinAbsT(T(1), margin));
    }

    REQUIRE(viaTape.m_primitive == c.leaf);
    REQUIRE(marchedTape.m_primitive == c.leaf);
    REQUIRE(tape.intersects(c.from, c.to) == HitStatus::Hit);
    REQUIRE(intersects(fused, c.from, c.to) == HitStatus::Hit);

    // The ray overloads agree with the segment ones.
    const Vec3    direction = normalized(c.to - c.from);
    const HitT<T> viaRay    = tape.firstHit(RayT<T>{c.from, direction}, T(0), (c.to - c.from).length());

    REQUIRE(viaRay.hit());
    REQUIRE(viaRay.m_t == viaTape.m_t);
  }
}

TEMPLATE_TEST_CASE("Ray: an intersection marches an operand only where its other operands leave it open",
                   "[Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  int count = 0;

  // The unit ball, marched, counting its evaluations at x in (a_lo, a_hi).
  const auto ball = [&](const T a_lo, const T a_hi) {
    return ProbedBall<T>{SphereSDF<T>(Vec3::zeros(), T(1)), a_lo, a_hi, &count};
  };

  // The tape also names the deciding leaf, numbered as written, though it marches the ball last (a fused
  // composition is one shape, with no leaves).
  const auto check = [&](const auto&    a_fused,
                         const Expr<T>& a_expr,
                         const RayT<T>& a_ray,
                         const T        a_t,
                         const bool     a_exact,
                         const uint32_t a_leaf) {
    const Tape<T, ProbedBall<T>> tape(pool, a_expr);

    for (int k = 0; k < 2; k++) {
      count = 0;

      const HitT<T> hit = (k == 0) ? firstHit(a_fused, a_ray, T(0), T(10)) : tape.firstHit(a_ray, T(0), T(10));

      REQUIRE(hit.hit());
      REQUIRE(hit.m_exact == a_exact);
      REQUIRE_THAT(hit.m_t, withinAbsT(a_t, 1e-3));
      REQUIRE(count == 0);

      if (k == 1) {
        REQUIRE(hit.m_primitive == a_leaf);
      }
    }
  };

  // The ball less a box that holds x < -0.5 near the x axis.
  const BoxSDF<T> cut(Vec3(-T(4), -T(0.5), -T(0.5)), Vec3(-T(0.5), T(0.5), T(0.5)));

  // Along the x axis from x = -3: the box decides where the ray enters, at x = -0.5, and the ball is marched
  // only past it, never inside the box (x in (-1, -0.5), where the ray is inside the ball).
  const RayT<T> axis{Vec3(-T(3), T(0), T(0)), Vec3(1, 0, 0)};

  check(
    Fused::Difference(ball(-T(1.9), -T(0.55)), cut), Difference(ball(-T(1.9), -T(0.55)), cut), axis, T(2.5), true, 1U);

  // Past the box, at y = 0.8: the ray enters the ball first, at x = -0.6, and it is marched no further than
  // that, not on to where the ray leaves it, at x = 0.6.
  const RayT<T> above{Vec3(-T(3), T(0.8), T(0)), Vec3(1, 0, 0)};

  check(
    Fused::Difference(ball(-T(0.4), T(0.4)), cut), Difference(ball(-T(0.4), T(0.4)), cut), above, T(2.4), false, 0U);

  // A box less the ball, the box holding x > 0.5: the ball is marched only inside the box, where the ray
  // leaves it at x = 1.
  const BoxSDF<T> slab(Vec3(T(0.5), -T(2), -T(2)), Vec3(T(4), T(2), T(2)));

  check(
    Fused::Difference(slab, ball(-T(1.9), T(0.45))), Difference(slab, ball(-T(1.9), T(0.45))), axis, T(4), false, 1U);

  // Two marched balls in an intersection with two boxes, x in (-2, -1) and (1, 2): the big ball's first span
  // within the boxes, in the first box, lies outside the small ball, which holds only the second box's
  // entry at x = 1. The trace goes on past the first box, in either order of the balls, and neither ball is
  // evaluated between the boxes.
  const BoxSDF<T> left(Vec3(-T(2), -T(0.5), -T(0.5)), Vec3(-T(1), T(0.5), T(0.5)));
  const BoxSDF<T> right(Vec3(T(1), -T(0.5), -T(0.5)), Vec3(T(2), T(0.5), T(0.5)));
  const auto      big   = ProbedBall<T>{SphereSDF<T>(Vec3::zeros(), T(3)), -T(1), T(1), &count};
  const auto      small = ProbedBall<T>{SphereSDF<T>(Vec3(T(1.5), T(0), T(0)), T(0.8)), -T(1), T(1), &count};
  const RayT<T>   along{Vec3(-T(5), T(0), T(0)), Vec3(1, 0, 0)};

  check(Fused::Intersection(Fused::Union(left, right), big, small),
        Intersection(std::vector<Expr<T>>{Union(left, right), big, small}),
        along,
        T(6),
        true,
        1U);
  check(Fused::Intersection(Fused::Union(left, right), small, big),
        Intersection(std::vector<Expr<T>>{Union(left, right), small, big}),
        along,
        T(6),
        true,
        1U);
}

TEMPLATE_TEST_CASE("Ray: span lists unite, intersect and complement as the sets of points they cover",
                   "[Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  std::mt19937                      rng(17);
  std::uniform_real_distribution<T> u(T(0), T(10));

  const T t0 = T(0);
  const T t1 = T(10);

  // A sorted list of up to four disjoint spans in [t0, t1], ends exact crossings.
  const auto randomList = [&]() {
    std::vector<T> ends;

    const int n = static_cast<int>(u(rng)) % 5;

    for (int i = 0; i < 2 * n; i++) {
      ends.push_back(u(rng));
    }

    std::sort(ends.begin(), ends.end());

    RaySpansT<T> list;

    list.reset(t1);

    for (int i = 0; i + 1 < 2 * n; i += 2) {
      if (ends[i] < ends[i + 1]) {
        list.addClipped(ends[i], ends[i + 1], t0, t1);
      }
    }

    return list;
  };

  // Whether a list covers a point, away from its ends.
  const auto covers = [](const RaySpansT<T>& a_list, const T a_t) {
    for (uint32_t i = 0; i < a_list.m_count; i++) {
      if (a_t > a_list.m_spans[i].m_in.m_t && a_t < a_list.m_spans[i].m_out.m_t) {
        return true;
      }
    }

    return false;
  };

  for (int trial = 0; trial < 200; trial++) {
    const RaySpansT<T> a = randomList();
    const RaySpansT<T> b = randomList();

    RaySpansT<T> both;
    RaySpansT<T> either;
    RaySpansT<T> notA;

    RayDetail::unite(a, b, either);
    RayDetail::intersect(a, b, both);
    RayDetail::complement(a, t0, notA);

    for (int k = 0; k < 200; k++) {
      const T t = u(rng);

      REQUIRE(covers(either, t) == (covers(a, t) || covers(b, t)));
      REQUIRE(covers(both, t) == (covers(a, t) && covers(b, t)));
      REQUIRE(covers(notA, t) == !covers(a, t));
    }

    // Disjoint and in order, so that the first crossing is the first end.
    for (const RaySpansT<T>* list : {&either, &both, &notA}) {
      for (uint32_t i = 0; i < list->m_count; i++) {
        REQUIRE(list->m_spans[i].m_in.m_t <= list->m_spans[i].m_out.m_t);

        if (i > 0) {
          REQUIRE(list->m_spans[i - 1].m_out.m_t < list->m_spans[i].m_in.m_t);
        }
      }
    }
  }
}

TEMPLATE_TEST_CASE("Ray: fused compositions and tapes agree with their values sampled along the ray",
                   "[Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Scene<T> scene(5);

  Pool pool(hostMemoryResource());

  // A union of 12 solids, evaluated through a BVH.
  const Expr<T> solids = Union(scene.solids(), Accelerate::BVH);
  const Expr<T> drills = Union(scene.drills());

  const Tape<T> unionTape(pool, solids);
  const Tape<T> differenceTape(pool, Difference(solids, drills));
  const Tape<T> insideOutTape(pool, Intersection(BoxSDF<T>(-Vec3::ones(), Vec3::ones()), Complement(solids)));
  const Tape<T> mappedTape(pool,
                           Difference(Rotate(Scale(Union(scene.balls[0], scene.boxes[0]), T(1.3)), T(30), 2),
                                      Translate(scene.holes[0], Vec3(T(0.1), T(0.2), T(0)))));

  REQUIRE(unionTape.getNumBVHUnions() == 1U);

  const auto fusedDifference = Fused::Difference(
    Fused::Union(scene.balls[0], scene.balls[1], scene.boxes[0], scene.boxes[1]), scene.holes[0], scene.holes[1]);
  const auto fusedMapped =
    Fused::Difference(Fused::Rotate(Fused::Scale(Fused::Union(scene.balls[0], scene.boxes[0]), T(1.3)), T(30), 2),
                      Fused::Translate(scene.holes[0], Vec3(T(0.1), T(0.2), T(0))));

  const T tMax = T(7);

  const auto check = [&](const auto& a_shape) {
    for (const RayT<T>& ray : scene.rays) {
      const HitT<T> hit = firstHit(a_shape, ray, T(0), tMax);

      REQUIRE(hit.m_exact == hit.hit());

      requireSampledAgreement(a_shape, ray, tMax, hit, 1000, T(1e-3));
    }
  };

  check(unionTape);
  check(differenceTape);
  check(insideOutTape);
  check(mappedTape);
  check(fusedDifference);
  check(fusedMapped);

  // The same through a deep copy and a view.
  Pool          other(hostMemoryResource());
  const Tape<T> copy = differenceTape.deepCopy(other);
  const Tape<T> view = differenceTape.rebasedView(pool);

  for (const RayT<T>& ray : scene.rays) {
    const HitT<T> hit = differenceTape.firstHit(ray, T(0), tMax);

    REQUIRE(copy.firstHit(ray, T(0), tMax).m_t == hit.m_t);
    REQUIRE(view.firstHit(ray, T(0), tMax).m_t == hit.m_t);
  }
}

TEMPLATE_TEST_CASE("Ray: smooth unions and intersections are traced as the plain operation and marched where the "
                   "blend changes it",
                   "[Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  const Vec3 x(1, 0, 0);
  const Vec3 y(0, 1, 0);

  // Each crossing fused, exact where the plain operation decides it, and through a tape, which marches the whole
  // blend.
  const auto check = [&](const auto&    a_fused,
                         const Expr<T>& a_expr,
                         const RayT<T>& a_ray,
                         const T        a_t,
                         const Vec3&    a_normal,
                         const bool     a_exact,
                         const bool     a_entering = true) {
    const Tape<T> plain(pool, a_expr);

    const HitT<T> hits[] = {firstHit(a_fused, a_ray, T(0), T(10)), plain.firstHit(a_ray, T(0), T(10))};

    for (const HitT<T>& hit : hits) {
      REQUIRE(hit.hit());
      REQUIRE(hit.m_exact == (a_exact && &hit == &hits[0]));
      REQUIRE(hit.m_entering == a_entering);
      // A march finds a crossing to within a few times its tolerance, 1e-4 of the shape's box diagonal.
      REQUIRE_THAT(hit.m_t, withinAbsT(a_t, hit.m_exact ? looseMargin<T>() : 3e-3));
      REQUIRE(dot(hit.m_normal, a_normal) > T(0.99));
    }
  };

  // Two balls of radius 0.5, 0.2 apart, blended over 0.6. Midway between them both values are equal, and the blend
  // deepens them by 0.6/4: the neck's surface is where |p - c| = 0.65, at y = 0.25, where the plain union has none.
  const SphereSDF<T> left(-T(0.6) * x, T(0.5));
  const SphereSDF<T> right(T(0.6) * x, T(0.5));
  const T            s = T(0.6);

  check(Fused::SmoothUnion(left, right, s), SmoothUnion(left, right, s), RayT<T>{-T(3) * y, y}, T(2.75), -y, false);

  // Along the axis, the ray meets the left ball where the blend changes nothing.
  check(Fused::SmoothUnion(left, right, s), SmoothUnion(left, right, s), RayT<T>{-T(3) * x, x}, T(1.9), -x, true);

  // From inside the neck, outside both balls: the ray leaves the blend at y = 0.25.
  check(
    Fused::SmoothUnion(left, right, s), SmoothUnion(left, right, s), RayT<T>{T(0.1) * y, y}, T(0.15), y, false, false);

  // From the left ball's center along the axis: the plain union is left at x = -0.1, but the blend fills the gap
  // (equal values 0.1 there, deepened by 0.15), so the ray stays inside across the neck and through the right ball,
  // leaving it at x = 1.1. The plain union's first exit bounds the blend zone's march; the march must go on past it.
  {
    const auto    blended = Fused::SmoothUnion(left, right, s);
    const Tape<T> plain(pool, SmoothUnion(left, right, s));
    const RayT<T> ray{-T(0.6) * x, x};

    for (const HitT<T>& hit : {firstHit(blended, ray, T(0), T(10)), plain.firstHit(ray, T(0), T(10))}) {
      REQUIRE(hit.hit());
      REQUIRE_FALSE(hit.m_entering);
      REQUIRE_THAT(hit.m_t, withinAbsT(T(1.7), hit.m_exact ? looseMargin<T>() : 3e-3));
      REQUIRE(dot(hit.m_normal, x) > T(0.99));
    }
  }

  // A third ball far from the others: only the first pair's boxes overlap, so only their neck is marched, and the
  // third ball is met exactly.
  const SphereSDF<T> far(Vec3(T(0), T(0), T(5)), T(0.5));
  const Vec3         z(0, 0, 1);

  check(Fused::SmoothUnion(Fused::operands(left, right, far), s),
        SmoothUnion(std::vector<Expr<T>>{left, right, far}, s),
        RayT<T>{-T(3) * y, y},
        T(2.75),
        -y,
        false);
  check(Fused::SmoothUnion(Fused::operands(left, right, far), s),
        SmoothUnion(std::vector<Expr<T>>{left, right, far}, s),
        RayT<T>{T(8) * z, -z},
        T(2.5),
        z,
        true);

  // The exponential blend deepens equal values by s ln 2, and changes the plain union everywhere: it is marched
  // whole.
  const T deep = T(0.5) + T(0.2) * std::log(T(2));

  check(Fused::SmoothUnion(left, right, T(0.2), BlendKind::Exponential),
        SmoothUnion(left, right, T(0.2), BlendKind::Exponential),
        RayT<T>{-T(3) * y, y},
        T(3) - std::sqrt(deep * deep - T(0.36)),
        -y,
        false);

  // Two balls of radius 0.5 overlapping by 0.4, smoothly intersected over 0.4: midway the blend raises both values by
  // 0.1, so the surface is where |p - c| = 0.4, at y = sqrt(0.07), inside the plain intersection's y = 0.4.
  const SphereSDF<T> a(-T(0.3) * x, T(0.5));
  const SphereSDF<T> b(T(0.3) * x, T(0.5));

  check(Fused::SmoothIntersection(a, b, T(0.4)),
        SmoothIntersection(a, b, T(0.4)),
        RayT<T>{-T(3) * y, y},
        T(3) - std::sqrt(T(0.07)),
        -y,
        false);

  // Random rays through blends of the scene, against the values sampled along them. A marched crossing is found to
  // the march's tolerance, so its value is small rather than zero.
  const Scene<T> scene(7);

  const auto fusedBalls =
    Fused::SmoothUnion(Fused::operands(scene.balls[0], scene.balls[1], scene.balls[2], scene.balls[3]), T(0.2));
  const auto fusedDrilled =
    Fused::SmoothDifference(Fused::SmoothUnion(Fused::operands(scene.boxes[0], scene.boxes[1], scene.boxes[2]), T(0.2)),
                            Fused::operands(scene.holes[0], scene.holes[1]),
                            T(0.1));
  const auto fusedCut =
    Fused::SmoothIntersection(fusedBalls, Fused::Union(scene.boxes[0], scene.boxes[1], scene.boxes[2]), T(0.1));

  // A ball blended into a half-space below it: the plane's box is unbounded, the blend zone's is the ball's grown.
  const PlaneSDF<T>  floor(-T(0.5) * Vec3(0, 1, 0), Vec3(0, 1, 0));
  const SphereSDF<T> ball(Vec3::zeros(), T(0.4));
  const auto         fusedFloor = Fused::SmoothUnion(floor, ball, T(0.3));

  const T tMax = T(7);

  const auto sampled = [&](const auto& a_shape) {
    for (const RayT<T>& ray : scene.rays) {
      const HitT<T> hit = firstHit(a_shape, ray, T(0), tMax);

      REQUIRE(hit.m_status != HitStatus::Unresolved);

      const T    end    = hit.hit() ? hit.m_t - T(1e-3) : tMax;
      const bool inside = a_shape.signedDistance(ray.at(T(0))) < T(0);

      if (hit.hit()) {
        REQUIRE_THAT(a_shape.signedDistance(hit.m_point), withinAbsT(T(0), 1e-3));
        REQUIRE(hit.m_entering == !inside);
      }

      for (size_t k = 1; k <= 1000 && end > T(0); k++) {
        REQUIRE((a_shape.signedDistance(ray.at(end * T(k) / T(1000))) < T(0)) == inside);
      }
    }
  };

  sampled(fusedBalls);
  sampled(fusedCut);
  sampled(fusedDrilled);
  sampled(fusedFloor);
}

TEMPLATE_TEST_CASE("Ray: a ray through more spans than a list holds still finds its first crossing",
                   "[Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // 20 balls along the x axis, a slab cut out of the row: under the difference every ball's span is needed,
  // more than a list holds.
  const size_t count   = 20;
  const T      spacing = T(0.1);
  const T      radius  = T(0.04);
  const T      slabLo  = T(0.52);
  const T      slabHi  = T(0.58);

  std::vector<Expr<T>> row;

  for (size_t i = 0; i < count; i++) {
    row.emplace_back(SphereSDF<T>(Vec3(-T(1) + spacing * T(i), T(0), T(0)), radius));
  }

  REQUIRE(count > RaySpansT<T>::Capacity);

  const BoxSDF<T> slab(Vec3(slabLo, -T(1), -T(1)), Vec3(slabHi, T(1), T(1)));

  Pool          pool(hostMemoryResource());
  const Tape<T> tape(pool, Difference(Union(row), slab));

  // The reference, on the axis: the ends of the balls' intervals, less the slab, in order.
  std::vector<std::pair<T, bool>> crossings;

  for (const auto& [lo, hi] : ballRow<T>(count, spacing, radius)) {
    const bool cutLo = lo > slabLo && lo < slabHi;
    const bool cutHi = hi > slabLo && hi < slabHi;

    if (hi < slabLo || lo > slabHi) {
      crossings.emplace_back(lo, true);
      crossings.emplace_back(hi, false);
    }
    else {
      if (!cutLo) {
        crossings.emplace_back(lo, true);
        crossings.emplace_back(slabLo, false);
      }
      if (!cutHi) {
        crossings.emplace_back(slabHi, true);
        crossings.emplace_back(hi, false);
      }
    }
  }

  // From every start along the axis, starting before, between and inside the balls (never on a surface,
  // where rounding decides whether the ray crosses at its start).
  for (int k = 0; k < 300; k++) {
    const T       x0 = -T(1.5) + T(0.01) * T(k) + T(0.003);
    const RayT<T> ray{Vec3(x0, T(0), T(0)), Vec3(1, 0, 0)};
    const HitT<T> hit = tape.firstHit(ray, T(0), T(4));

    INFO("start " << x0);

    const auto next = std::find_if(crossings.begin(), crossings.end(), [x0](const auto& c) { return c.first >= x0; });

    if (next == crossings.end()) {
      REQUIRE(hit.m_status == HitStatus::Miss);
    }
    else {
      REQUIRE(hit.hit());
      REQUIRE(hit.m_exact);
      REQUIRE_THAT(hit.m_t, withinAbsT(next->first - x0, looseMargin<T>()));
      REQUIRE(hit.m_entering == next->second);
    }
  }
}

TEMPLATE_TEST_CASE("Ray: a first crossing exactly where a full list stops is found when the ray starts again there",
                   "[Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // 20 balls along the x axis, intersected with a half-space that starts between ball k - 1 and ball k. The
  // row's list fills up and stops where a dropped ball enters, so for k = 8 and k = 16 the first crossing is
  // exactly where a pass ends and the next one starts.
  const size_t count   = 20;
  const T      spacing = T(0.1);
  const T      radius  = T(0.04);

  std::vector<Expr<T>> row;

  for (size_t i = 0; i < count; i++) {
    row.emplace_back(SphereSDF<T>(Vec3(-T(1) + spacing * T(i), T(0), T(0)), radius));
  }

  const auto    balls = ballRow<T>(count, spacing, radius);
  const T       x0    = -T(1.5) + T(0.003);
  const RayT<T> ray{Vec3(x0, T(0), T(0)), Vec3(1, 0, 0)};

  REQUIRE(count > 2 * RaySpansT<T>::Capacity);

  for (size_t k = 1; k < count; k++) {
    const T         cut = T(0.5) * (balls[k - 1].second + balls[k].first);
    const BoxSDF<T> halfSpace(Vec3(cut, -T(1), -T(1)), Vec3(T(10), T(1), T(1)));

    Pool          pool(hostMemoryResource());
    const Tape<T> tape(pool, Intersection(Union(row), halfSpace));
    const HitT<T> hit = tape.firstHit(ray, T(0), T(4));

    INFO("cut before ball " << k);

    REQUIRE(hit.hit());
    REQUIRE(hit.m_exact);
    REQUIRE(hit.m_entering);
    REQUIRE_THAT(hit.m_t, withinAbsT(balls[k].first - x0, looseMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("Ray: a tape with sibling BVH unions, one in a rotated frame, names the leaf each ray hits",
                   "[Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // Two rows of n balls, each row a BVH union, side by side in one program. Row A lies along the x axis. Row
  // B is built along x from translated balls, then turned a quarter turn about z (x goes to y) and lifted
  // to z = 3: its ball k is at (0, k, 3). Leaves are numbered in expression order, A's then B's.
  const size_t n      = 8;
  const T      radius = T(0.3);

  std::vector<Expr<T>> rowA;
  std::vector<Expr<T>> rowB;

  for (size_t k = 0; k < n; k++) {
    rowA.emplace_back(SphereSDF<T>(Vec3(T(k), T(0), T(0)), radius));
    rowB.emplace_back(Translate(SphereSDF<T>(Vec3::zeros(), radius), Vec3(T(k), T(0), T(0))));
  }

  Pool          pool(hostMemoryResource());
  const Tape<T> tape(pool, Union(Union(rowA), Translate(Rotate(Union(rowB), T(90), size_t(2)), Vec3(0, 0, 3))));

  const double margin = looseMargin<T>();

  for (size_t k = 0; k < n; k++) {
    const T x = T(k);

    INFO("ball " << k);

    // Into A's ball k along +x, and into B's ball k along +y and along -y.
    const HitT<T> a = tape.firstHit(RayT<T>{Vec3(x - T(0.5), T(0), T(0)), Vec3(1, 0, 0)}, T(0), T(1));
    const HitT<T> b = tape.firstHit(RayT<T>{Vec3(T(0), x - T(0.5), T(3)), Vec3(0, 1, 0)}, T(0), T(1));
    const HitT<T> c = tape.firstHit(RayT<T>{Vec3(T(0), x + T(0.5), T(3)), Vec3(0, -1, 0)}, T(0), T(1));

    for (const HitT<T>& hit : {a, b, c}) {
      REQUIRE(hit.hit());
      REQUIRE(hit.m_exact);
      REQUIRE(hit.m_entering);
      REQUIRE_THAT(hit.m_t, withinAbsT(T(0.5) - radius, margin));
    }

    REQUIRE(a.m_primitive == k);
    REQUIRE(b.m_primitive == n + k);
    REQUIRE(c.m_primitive == n + k);
    REQUIRE_THAT(dot(a.m_normal, Vec3(-1, 0, 0)), withinAbsT(T(1), margin));
    REQUIRE_THAT(dot(b.m_normal, Vec3(0, -1, 0)), withinAbsT(T(1), margin));
    REQUIRE_THAT(dot(c.m_normal, Vec3(0, 1, 0)), withinAbsT(T(1), margin));
  }
}

TEMPLATE_TEST_CASE("Ray: mesh SDFs, marched parts and user shapes are traced inside compositions",
                   "[Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  // The unit cube as each mesh SDF, with a ball cut out of its middle: from inside the hole, the ray enters
  // the solid at the ball and leaves it at the cube.
  const auto cube     = Parser::readIntoTriMeshSDF<T, 4, 4>(unitCube(), pool);
  const auto polygons = Parser::readIntoMeshSDF<T, 4>(unitCube(), pool);
  const auto flat     = Parser::readIntoFlatMeshSDF<T>(unitCube(), pool);

  const Vec3         middle = T(0.5) * Vec3::ones();
  const SphereSDF<T> hole(middle, T(0.3));

  const auto cubeDifference     = Fused::Difference(cube, hole);
  const auto polygonsDifference = Fused::Difference(polygons, hole);
  const auto flatDifference     = Fused::Difference(flat, hole);

  // The ball as a user shape whose ray form has only firstHit(), and the ball offset as part of a union with a
  // ball far outside the box, which is marched: a union's level sets have no closed form.
  const BoxSDF<T>    box(Vec3::zeros(), Vec3::ones());
  const SphereSDF<T> far(T(10) * Vec3::ones(), T(0.25));
  const auto         userDifference = Fused::Difference(box, FirstHitBall<T>{hole});
  const auto         offsetDifference =
    Fused::Difference(box, Fused::Offset(Fused::Union(SphereSDF<T>(middle, T(0.25)), far), T(0.05)));

  std::mt19937                      rng(3);
  std::uniform_real_distribution<T> u(-T(1), T(1));

  for (int i = 0; i < 100; i++) {
    const Vec3    direction = unit(u(rng), u(rng), u(rng));
    const RayT<T> fromHole{middle, direction};

    // The cube's exit, by the slab test.
    const T exit = slabCrossing(middle, direction, T(0.31), T(10));

    for (const HitT<T>& hit : {firstHit(cubeDifference, fromHole, T(0), T(10)),
                               firstHit(polygonsDifference, fromHole, T(0), T(10)),
                               firstHit(flatDifference, fromHole, T(0), T(10)),
                               firstHit(userDifference, fromHole, T(0), T(10))}) {
      REQUIRE(hit.hit());
      REQUIRE(hit.m_exact);
      REQUIRE(hit.m_entering);
      REQUIRE_THAT(hit.m_t, withinAbsT(T(0.3), looseMargin<T>()));
    }

    const HitT<T> marched = firstHit(offsetDifference, fromHole, T(0), T(10));

    REQUIRE(marched.hit());
    REQUIRE_FALSE(marched.m_exact);
    REQUIRE_THAT(marched.m_t, withinAbsT(T(0.3), 1e-3));

    // Past the ball, the next crossing is the cube's.
    for (const HitT<T>& hit : {firstHit(cubeDifference, fromHole, T(0.31), T(10)),
                               firstHit(polygonsDifference, fromHole, T(0.31), T(10)),
                               firstHit(userDifference, fromHole, T(0.31), T(10))}) {
      REQUIRE(hit.hit());
      REQUIRE_FALSE(hit.m_entering);
      REQUIRE_THAT(hit.m_t, withinAbsT(exit, looseMargin<T>()));
    }
  }

  // A tape of the same, with the mesh as a leaf.
  const Tape<T> tape(pool, Difference(polygons, hole));

  for (int i = 0; i < 20; i++) {
    const HitT<T> hit = tape.firstHit(RayT<T>{middle, unit(u(rng), u(rng), u(rng))}, T(0), T(10));

    REQUIRE(hit.hit());
    REQUIRE(hit.m_exact);
    REQUIRE_THAT(hit.m_t, withinAbsT(T(0.3), looseMargin<T>()));
    REQUIRE(hit.m_primitive == 1U);
  }
}

TEMPLATE_TEST_CASE("Ray: firstCrossing() finds the crossing, its point and side, of every kind of shape",
                   "[Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  const Vec3 x(1, 0, 0);

  // Each crossing at its closed-form distance, with its point and side; exact unless marched.
  const auto check =
    [&](const auto& a_shape, const RayT<T>& a_ray, const T a_t, const bool a_entering, const bool a_exact) {
      const CrossingT<T> c = firstCrossing(a_shape, a_ray, T(0), T(10));

      REQUIRE(c.hit());
      REQUIRE(c.m_exact == a_exact);
      REQUIRE(c.m_entering == a_entering);
      REQUIRE_THAT(c.m_t, withinAbsT(a_t, a_exact ? looseMargin<T>() : 1e-3));
      REQUIRE_THAT((c.m_point - a_ray.at(a_t)).length(), withinAbsT(T(0), a_exact ? looseMargin<T>() : 1e-3));
    };

  // The unit ball: entered at x = -1 from outside, left at x = 1 from its center; traced, and marched.
  const SphereSDF<T> ball(Vec3::zeros(), T(1));

  check(ball, RayT<T>{-T(3) * x, x}, T(2), true, true);
  check(ball, RayT<T>{Vec3::zeros(), x}, T(1), false, true);
  check(Marched<T, SphereSDF<T>>{ball}, RayT<T>{-T(3) * x, x}, T(2), true, false);

  // A box with a ball cut out of its middle, fused, through a tape and as a BVH union of the box alone: from the
  // middle, the ray enters the solid at the ball, x = 0.5.
  const BoxSDF<T>    box(-Vec3::ones(), Vec3::ones());
  const SphereSDF<T> hole(Vec3::zeros(), T(0.5));
  const Tape<T>      tape(pool, Difference(box, hole));

  check(Fused::Difference(box, hole), RayT<T>{Vec3::zeros(), x}, T(0.5), true, true);
  check(tape, RayT<T>{Vec3::zeros(), x}, T(0.5), true, true);
  check(tape, RayT<T>{-T(3) * x, x}, T(2), true, true);

  const std::vector<BoxSDF<T>>                 boxes{box};
  const std::vector<BoundingVolumes::AABBT<T>> boxBoxes{box.computeBoundingVolume()};
  const BVHUnion<T, 4, BoxSDF<T>>              bvhUnion(pool, boxes, boxBoxes);

  check(bvhUnion, RayT<T>{-T(3) * x, x}, T(2), true, true);

  // A smooth union of two balls 0.2 apart, blended over 0.6: its neck, at y = 0.25, where the plain union has no
  // surface, is marched.
  const Vec3 y(0, 1, 0);

  check(Fused::SmoothUnion(SphereSDF<T>(-T(0.6) * x, T(0.5)), SphereSDF<T>(T(0.6) * x, T(0.5)), T(0.6)),
        RayT<T>{-T(3) * y, y},
        T(2.75),
        true,
        false);

  // Segments: measured from their start; one that stops short, and one of zero length, miss.
  const CrossingT<T> through = firstCrossing(ball, -T(3) * x, T(3) * x);

  REQUIRE(through.hit());
  REQUIRE_THAT(through.m_t, withinAbsT(T(2), looseMargin<T>()));
  REQUIRE_FALSE(firstCrossing(ball, -T(3) * x, -T(2) * x).hit());
  REQUIRE_FALSE(firstCrossing(ball, x, x).hit());
  REQUIRE_FALSE(firstCrossing(tape, RayT<T>{T(3) * Vec3::ones(), Vec3::ones() / std::sqrt(T(3))}, T(0), T(10)).hit());

  // The unit cube as a mesh, on random rays: where the slab test says.
  const auto cube = Parser::readIntoTriMeshSDF<T, 4, 4>(unitCube(), pool);

  std::mt19937                      rng(11);
  std::uniform_real_distribution<T> u(-T(1), T(1));

  for (int i = 0; i < 200; i++) {
    const Vec3 from      = T(0.5) * Vec3::ones() + T(2) * Vec3(u(rng), u(rng), u(rng));
    const Vec3 direction = unit(u(rng), u(rng), u(rng));
    const T    expected  = slabCrossing(from, direction, T(0), T(10));
    const auto c         = firstCrossing(cube, RayT<T>{from, direction}, T(0), T(10));

    REQUIRE(c.hit() == (expected < T(10)));

    if (c.hit()) {
      REQUIRE(c.m_exact);
      REQUIRE_THAT(c.m_t, withinAbsT(expected, looseMargin<T>()));
    }
  }

  // Random rays through compositions of the scene, against the values sampled along them.
  const Scene<T> scene(13);
  const Tape<T>  sceneTape(pool, Difference(Union(scene.solids()), Union(scene.drills())));
  const auto     fusedScene = Fused::Difference(
    Fused::Union(scene.balls[0], scene.balls[1], scene.boxes[0], scene.boxes[1]), scene.holes[0], scene.holes[1]);

  const auto sampled = [&](const auto& a_shape) {
    for (const RayT<T>& ray : scene.rays) {
      const CrossingT<T> c = firstCrossing(a_shape, ray, T(0), T(7));

      REQUIRE(c.m_status != HitStatus::Unresolved);

      const T    end    = c.hit() ? c.m_t - T(1e-3) : T(7);
      const bool inside = a_shape.signedDistance(ray.at(T(0))) < T(0);

      if (c.hit()) {
        REQUIRE(c.m_exact);
        REQUIRE_THAT(a_shape.signedDistance(c.m_point), withinAbsT(T(0), 10 * looseMargin<T>()));
        REQUIRE(c.m_entering == !inside);
      }

      for (size_t k = 1; k <= 1000 && end > T(0); k++) {
        REQUIRE((a_shape.signedDistance(ray.at(end * T(k) / T(1000))) < T(0)) == inside);
      }
    }
  };

  sampled(sceneTape);
  sampled(fusedScene);
}

TEMPLATE_TEST_CASE("Ray: a composition whose operand never decides is unresolved after its last pass",
                   "[Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // A ray from the middle of a box, through a ball that is unresolved a step past every start: each pass of
  // the trace gets one step further, and the trace gives up after its 64 passes, 64 steps along the ray.
  const T               step = T(0.01);
  const BoxSDF<T>       box(-T(5) * Vec3::ones(), T(5) * Vec3::ones());
  const CreepingBall<T> ball{{SphereSDF<T>(Vec3::zeros(), T(3))}, step};
  const RayT<T>         ray{Vec3::zeros(), Vec3(1, 0, 0)};

  const auto fused = Fused::Intersection(box, ball);

  Pool                           pool(hostMemoryResource());
  const Tape<T, CreepingBall<T>> tape(pool, Intersection(box, ball));

  for (const HitT<T>& hit : {firstHit(fused, ray, T(0), T(4)), tape.firstHit(ray, T(0), T(4))}) {
    REQUIRE(hit.m_status == HitStatus::Unresolved);
    REQUIRE_THAT(hit.m_t, withinAbsT(T(64) * step, looseMargin<T>()));
  }

  REQUIRE(intersects(fused, ray, T(0), T(4)) == HitStatus::Unresolved);
  REQUIRE(intersects(tape, ray, T(0), T(4)) == HitStatus::Unresolved);
}

TEMPLATE_TEST_CASE("Ray: a BVH union's hit names the member it crossed, in the order given",
                   "[Ray][BVHUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // Eight balls along x, given out of order, so that the BVH stores them in another order than the array's.
  const std::array<int, 8> slots{5, 2, 7, 0, 3, 6, 1, 4};

  std::vector<SphereSDF<T>>              balls;
  std::vector<BoundingVolumes::AABBT<T>> boxes;

  for (const int slot : slots) {
    balls.emplace_back(Vec3(T(3 * slot), T(0), T(0)), T(1));
    boxes.push_back(balls.back().computeBoundingVolume());
  }

  Pool pool(hostMemoryResource());

  const BVHUnion<T, 4, SphereSDF<T>> scene(pool, balls, boxes);

  const auto hitAt = [&scene](const T a_x, const T a_y) {
    return firstHit(scene, RayT<T>{Vec3(a_x, a_y, T(5)), Vec3(T(0), T(0), -T(1))}, T(0), T(10));
  };

  // A member is named by its place in the array the union was built from, whichever point on it is hit.
  for (uint32_t i = 0; i < balls.size(); i++) {
    const T x = T(3 * slots[i]);

    const auto center    = hitAt(x, T(0));
    const auto offCenter = hitAt(x + T(0.3), T(0.2));

    REQUIRE((center.hit() && offCenter.hit()));
    CHECK(center.m_primitive == i);
    CHECK(offCenter.m_primitive == i);
  }
}

TEMPLATE_TEST_CASE("Ray: a BVH union of several types is traced to its nearest member's crossing, through each "
                   "member's ray form",
                   "[Ray][BVHUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  // Balls, boxes and an infinite cylinder (unbounded, kept out of the BVH), and the unit cube as a mesh.
  const std::vector<SphereSDF<T>> balls{SphereSDF<T>(Vec3(-T(0.6), T(0.1), T(0)), T(0.35)),
                                        SphereSDF<T>(Vec3(T(0.5), -T(0.5), T(0.4)), T(0.25))};
  const std::vector<BoxSDF<T>>    boxes{BoxSDF<T>(Vec3(-T(0.2), T(0.4), -T(0.8)), Vec3(T(0.3), T(0.9), -T(0.3)))};
  const std::vector<InfiniteCylinderSDF<T>> rods{InfiniteCylinderSDF<T>(Vec3(T(0.8), T(0.6), T(0)), T(0.15), 2)};
  const std::vector<TriMeshSDF<T, 4, 4>>    meshes{Parser::readIntoTriMeshSDF<T, 4, 4>(unitCube(), pool)};

  const auto scene = BVHUnion(pool, balls, boxes, rods, meshes);

  const T tMax = T(10);

  for (const RayT<T>& ray : deviceRays<T>()) {
    // The reference: each member traced on its own, the nearest crossing kept.
    T nearest = Math::Limits<T>::infinity();

    const auto trace = [&](const auto& a_members) {
      for (const auto& member : a_members) {
        const auto hit = firstHit(member, ray, T(0), tMax);

        if (hit.hit()) {
          nearest = std::min(nearest, hit.m_t);
        }
      }
    };

    trace(balls);
    trace(boxes);
    trace(rods);
    trace(meshes);

    const auto hit = firstHit(scene, ray, T(0), tMax);

    REQUIRE(hit.hit() == (nearest < Math::Limits<T>::infinity()));

    if (hit.hit()) {
      REQUIRE_THAT(hit.m_t, withinAbsT(nearest, looseMargin<T>()));

      // The point is on the union's surface.
      REQUIRE_THAT(scene.signedDistance(hit.m_point), withinAbsT(T(0), looseMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("Ray: a union through a BVH (Accelerate::BVH) is traced to the crossings of the union as "
                   "written",
                   "[Ray][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  const std::vector<Expr<T>> objects{
    SphereSDF<T>(Vec3(-T(0.6), T(0.1), T(0)), T(0.35)),
    Translate(BoxSDF<T>(-T(0.2) * Vec3::ones(), T(0.2) * Vec3::ones()), Vec3(T(0.5), -T(0.4), T(0.3))),
    Scale(SphereSDF<T>(Vec3(T(0.2), T(0.5), -T(0.4)), T(0.1)), T(1.5)),
    Offset(Union(SphereSDF<T>(Vec3(T(0.7), T(0.6), T(0)), T(0.1)), SphereSDF<T>(Vec3(T(0.8), T(0.7), T(0)), T(0.1))),
           T(0.05)),
    Parser::readIntoTriMeshSDF<T>(unitCube(), pool)};

  const Tape<T> withBVH(pool, Union(objects, Accelerate::BVH));
  const Tape<T> asWritten(pool, Union(objects));

  REQUIRE(withBVH.getNumBVHUnions() == 1U);

  for (const RayT<T>& ray : deviceRays<T>()) {
    const auto fast  = firstHit(withBVH, ray, T(0), T(10));
    const auto plain = firstHit(asWritten, ray, T(0), T(10));

    REQUIRE(fast.hit() == plain.hit());

    if (fast.hit()) {
      REQUIRE_THAT(fast.m_t, withinAbsT(plain.m_t, looseMargin<T>()));
      REQUIRE(fast.m_primitive == plain.m_primitive);
    }
  }
}

TEMPLATE_TEST_CASE("Ray: device ray queries of compositions match the host", "[Ray][gpu]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  const std::vector<RayT<T>> rays = deviceRays<T>();

  const SphereSDF<T> sphere(Vec3(T(0.1), T(0), T(0)), T(0.9));

  // A fused composition, traced from its operands' spans, with and without its normals.
  const Part<T> part;
  const auto    fused = part.fused();

  using F = std::decay_t<decltype(fused)>;

  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, F>{fused}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, F>{fused}, rays));
  requireSameResults(evaluateOnDevice<T>(CrossingDistance<T, F>{fused}, rays),
                     evaluateOnHost<T>(CrossingDistance<T, F>{fused}, rays));

  // The same composition in a tape, against device mirrors of its pool.
  Pool pool(hostMemoryResource());

  const auto    polygons = Parser::readIntoMeshSDF<T, 4>(unitCube(), pool);
  const Tape<T> tape(pool, part.expr());

  // A tape with a BVH union and a difference, and a mesh in a fused difference.
  const Scene<T> scene(5);
  const Tape<T>  sceneTape(pool, Difference(Union(scene.solids()), Union(scene.drills())));
  const auto     meshDifference = Fused::Difference(polygons, SphereSDF<T>(T(0.5) * Vec3::ones(), T(0.3)));

  // A marched shell less a box, marched only where the box leaves the difference open: in a tape and fused (the
  // shell of a union, whose level sets have no closed form, with a ball far away).
  const BoxSDF<T>    corner(Vec3::zeros(), T(2) * Vec3::ones());
  const SphereSDF<T> far(T(20) * Vec3::ones(), T(0.5));
  const Tape<T>      shellTape(pool, Difference(Annular(Union(sphere, far), T(0.1)), corner));
  const auto         shellFused = Fused::Difference(Fused::Annular(Fused::Union(sphere, far), T(0.1)), corner);

  // A BVH union of several types: the mesh, balls and an unbounded cylinder.
  const auto mixedUnion =
    BVHUnion(pool,
             std::vector<std::decay_t<decltype(polygons)>>{polygons},
             std::vector<SphereSDF<T>>{sphere, far},
             std::vector<InfiniteCylinderSDF<T>>{InfiniteCylinderSDF<T>(Vec3::zeros(), T(0.2), 1)});

  // A smooth union of the mesh and a ball, traced as the plain union and marched where the blend changes it.
  const auto smoothFused = Fused::SmoothUnion(polygons, SphereSDF<T>(Vec3(T(1.1), T(0.5), T(0.5)), T(0.3)), T(0.2));

  pool.freeze();

  const Pool device = Pool::mirror(pool, deviceTestResource());

  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, Tape<T>>{tape.rebasedView(device)}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, Tape<T>>{tape}, rays));
  requireSameResults(evaluateOnDevice<T>(CrossingDistance<T, Tape<T>>{tape.rebasedView(device)}, rays),
                     evaluateOnHost<T>(CrossingDistance<T, Tape<T>>{tape}, rays));
  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, Tape<T>>{sceneTape.rebasedView(device)}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, Tape<T>>{sceneTape}, rays));

  using D = std::decay_t<decltype(meshDifference)>;

  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, D>{meshDifference.rebasedView(device)}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, D>{meshDifference}, rays));

  using S = std::decay_t<decltype(shellFused)>;

  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, Tape<T>>{shellTape.rebasedView(device)}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, Tape<T>>{shellTape}, rays),
                     T(1e-3));
  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, S>{shellFused}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, S>{shellFused}, rays),
                     T(1e-3));

  using MU = std::decay_t<decltype(mixedUnion)>;

  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, MU>{mixedUnion.rebasedView(device)}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, MU>{mixedUnion}, rays));

  using SF = std::decay_t<decltype(smoothFused)>;

  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, SF>{smoothFused.rebasedView(device)}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, SF>{smoothFused}, rays),
                     T(1e-3));
}
