// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Unit tests for the ray queries of analytic shapes (EBGeometry_Ray.hpp and the analytic ray forms): the
// ray forms against hand-computed crossings and against the march, a tube's spans against its values along
// the ray, segments, an unresolved march, and device evaluation.

#include "TestRayHelpers.hpp"

using namespace EBGeometry;
using namespace EBGeometryTestRay;

TEMPLATE_TEST_CASE("Ray: the analytic ray forms give the closed-form crossings", "[Ray]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const RayT<T> alongX{Vec3(-T(3), T(0), T(0.3)), Vec3(1, 0, 0)};
  const T       z = T(0.3);

  // Each answer by hand: the ray is x = -3 + t, y = 0, z = 0.3.
  const PlaneSDF<T> plane(Vec3(T(1), T(0), T(0)), Vec3(1, 0, 0));
  const HitT<T>     planeHit = firstHit(plane, alongX, T(0), T(10));

  REQUIRE(planeHit.hit());
  REQUIRE_THAT(planeHit.m_t, withinAbsT(T(4), tightMargin<T>()));
  REQUIRE_FALSE(planeHit.m_entering); // The ray starts on the negative (inside) side.

  const SphereSDF<T> sphere(Vec3::zeros(), T(1));
  const HitT<T>      sphereHit = firstHit(sphere, alongX, T(0), T(10));

  REQUIRE(sphereHit.hit());
  REQUIRE(sphereHit.m_entering);
  REQUIRE_THAT(sphereHit.m_t, withinAbsT(T(3) - std::sqrt(T(1) - z * z), tightMargin<T>()));
  REQUIRE_THAT(dot(sphereHit.m_normal, sphereHit.m_point), withinAbsT(T(1), tightMargin<T>()));

  const BoxSDF<T> box(-Vec3::ones(), Vec3::ones());
  const HitT<T>   boxHit = firstHit(box, alongX, T(0), T(10));

  REQUIRE(boxHit.hit());
  REQUIRE_THAT(boxHit.m_t, withinAbsT(T(2), tightMargin<T>()));
  REQUIRE_THAT(boxHit.m_normal[0], withinAbsT(-T(1), tightMargin<T>()));

  // From inside the box, the crossing is where the ray leaves.
  const HitT<T> boxOut = firstHit(box, alongX, T(2.5), T(10));

  REQUIRE(boxOut.hit());
  REQUIRE_FALSE(boxOut.m_entering);
  REQUIRE_THAT(boxOut.m_t, withinAbsT(T(4), tightMargin<T>()));
  REQUIRE_THAT(boxOut.m_normal[0], withinAbsT(T(1), tightMargin<T>()));

  // Along its axis, a cylinder is entered through a cap; across it, through its side.
  const CylinderSDF<T> cylinder(Vec3(-T(1), T(0), T(0)), Vec3(T(1), T(0), T(0)), T(0.5));
  const HitT<T>        capHit = firstHit(cylinder, alongX, T(0), T(10));

  REQUIRE(capHit.hit());
  REQUIRE_THAT(capHit.m_t, withinAbsT(T(2), tightMargin<T>()));
  REQUIRE_THAT(capHit.m_normal[0], withinAbsT(-T(1), tightMargin<T>()));

  const RayT<T> alongY{Vec3(T(0.2), -T(3), T(0.3)), Vec3(0, 1, 0)};
  const HitT<T> sideHit = firstHit(cylinder, alongY, T(0), T(10));

  REQUIRE(sideHit.hit());
  REQUIRE_THAT(sideHit.m_t, withinAbsT(T(3) - std::sqrt(T(0.25) - z * z), tightMargin<T>()));
  REQUIRE_THAT(sideHit.m_normal[0], withinAbsT(T(0), tightMargin<T>()));

  const InfiniteCylinderSDF<T> tube(Vec3::zeros(), T(0.5), size_t(0));
  const HitT<T>                tubeHit = firstHit(tube, alongY, T(0), T(10));

  REQUIRE(tubeHit.hit());
  REQUIRE_THAT(tubeHit.m_t, withinAbsT(T(3) - std::sqrt(T(0.25) - z * z), tightMargin<T>()));
  REQUIRE(firstHit(tube, alongX, T(0), T(10)).m_status == HitStatus::Miss); // Inside it all along.

  // A capsule along x, with tips at x = -1 and 1 (so its cap centers are at x = -0.5 and 0.5), is entered
  // through its hemispherical cap.
  const CapsuleSDF<T> capsule(Vec3(-T(1), T(0), T(0)), Vec3(T(1), T(0), T(0)), T(0.5));
  const HitT<T>       capsuleHit = firstHit(capsule, alongX, T(0), T(10));

  REQUIRE(capsuleHit.hit());
  REQUIRE_THAT(capsuleHit.m_t, withinAbsT(T(2.5) - std::sqrt(T(0.25) - z * z), tightMargin<T>()));

  for (const HitT<T>& hit : {planeHit, sphereHit, boxHit, capHit, sideHit, tubeHit, capsuleHit}) {
    REQUIRE(hit.m_exact);
    REQUIRE_THAT(hit.m_normal.length(), withinAbsT(T(1), tightMargin<T>()));
  }

  // A miss.
  REQUIRE(firstHit(sphere, RayT<T>{Vec3(-T(3), T(2), T(0)), Vec3(1, 0, 0)}, T(0), T(10)).m_status == HitStatus::Miss);
  REQUIRE(intersects(sphere, alongX, T(0), T(1)) == HitStatus::Miss);
}

TEMPLATE_TEST_CASE("Ray: a small ball and a thin cylinder far from the ray's start are hit or missed by the exact "
                   "distance of the ray",
                   "[Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // Rays from the origin that pass a ball (and a cylinder) of radius 0.03, about 3 away, at 1 +- 3e-4 and
  // 1 +- 1e-3 times the radius from its center (axis). The reference decides from that distance, computed in
  // double from the ray as stored: float's |oc|^2 - r^2 alone loses about 1e-3 of the radius here.
  const T    radius = T(0.03);
  const Vec3 center(T(2.4), T(1.3), -T(1.1));

  const SphereSDF<T>           ball(center, radius);
  const InfiniteCylinderSDF<T> tube(Vec3(center[0], center[1], T(0)), radius, 2);

  const double offsets[] = {-1e-3, -3e-4, 3e-4, 1e-3};

  const auto exactFrom = [](const Vec3& a_v) {
    return std::array<double, 3>{double(a_v[0]), double(a_v[1]), double(a_v[2])};
  };
  const auto crossOf = [](const std::array<double, 3>& a_u, const std::array<double, 3>& a_v) {
    return std::array<double, 3>{
      a_u[1] * a_v[2] - a_u[2] * a_v[1], a_u[2] * a_v[0] - a_u[0] * a_v[2], a_u[0] * a_v[1] - a_u[1] * a_v[0]};
  };
  const auto norm = [](const std::array<double, 3>& a_v) {
    return std::sqrt(a_v[0] * a_v[0] + a_v[1] * a_v[1] + a_v[2] * a_v[2]);
  };

  const std::array<double, 3> c = exactFrom(center);

  for (int k = 0; k < 64; k++) {
    const double angle = 6.283185307179586 * double(k) / 64.0;

    for (const double offset : offsets) {
      const double reach = 0.03 * (1.0 + offset);

      // The ball: a unit vector perpendicular to the center, turned by the angle.
      {
        const std::array<double, 3> u = {
          1.3 / std::sqrt(2.4 * 2.4 + 1.3 * 1.3), -2.4 / std::sqrt(2.4 * 2.4 + 1.3 * 1.3), 0.0};
        const std::array<double, 3> w      = crossOf(c, u);
        const double                wn     = norm(w);
        const std::array<double, 3> target = {c[0] + reach * (std::cos(angle) * u[0] + std::sin(angle) * w[0] / wn),
                                              c[1] + reach * (std::cos(angle) * u[1] + std::sin(angle) * w[1] / wn),
                                              c[2] + reach * (std::cos(angle) * u[2] + std::sin(angle) * w[2] / wn)};
        const double                tn     = norm(target);

        const RayT<T> ray{Vec3::zeros(), Vec3(T(target[0] / tn), T(target[1] / tn), T(target[2] / tn))};

        const std::array<double, 3> d    = exactFrom(ray.m_direction);
        const double                dist = norm(crossOf(c, d)) / norm(d);

        INFO("ball, angle " << angle << ", offset " << offset << ", distance/radius " << dist / 0.03);
        REQUIRE(firstHit(ball, ray, T(0), T(10)).hit() == (dist < double(radius)));
      }

      // The cylinder along z: the ray rises as it goes, and passes the axis at the reach.
      {
        const std::array<double, 3> u = {
          -1.3 / std::sqrt(2.4 * 2.4 + 1.3 * 1.3), 2.4 / std::sqrt(2.4 * 2.4 + 1.3 * 1.3), 0.0};
        const double                side   = (k % 2 == 0) ? 1.0 : -1.0;
        const std::array<double, 3> target = {
          c[0] + side * reach * u[0], c[1] + side * reach * u[1], 0.5 * std::sin(angle)};
        const double tn = norm(target);

        const RayT<T> ray{Vec3::zeros(), Vec3(T(target[0] / tn), T(target[1] / tn), T(target[2] / tn))};

        // The distance between the ray's line and the axis: the axis point's offset along their common normal.
        const std::array<double, 3> d    = exactFrom(ray.m_direction);
        const std::array<double, 3> n    = crossOf(d, {0.0, 0.0, 1.0});
        const double                dist = std::abs(c[0] * n[0] + c[1] * n[1]) / norm(n);

        INFO("cylinder, angle " << angle << ", offset " << offset << ", distance/radius " << dist / 0.03);
        REQUIRE(firstHit(tube, ray, T(0), T(10)).hit() == (dist < double(radius)));
      }
    }
  }
}

TEMPLATE_TEST_CASE("Ray: the analytic ray forms agree with the march on random rays",
                   "[Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  std::mt19937                      rng(5);
  std::uniform_real_distribution<T> uniform(-T(1), T(1));

  const auto compare = [&rng, &uniform](const auto& a_shape) {
    const Marched<T, std::decay_t<decltype(a_shape)>> marched{a_shape};

    int compared = 0;

    for (int i = 0; i < 300; i++) {
      const Vec3 origin    = T(3) * normalized(Vec3(uniform(rng), uniform(rng), uniform(rng)));
      const Vec3 target    = T(0.5) * Vec3(uniform(rng), uniform(rng), uniform(rng));
      const Vec3 direction = normalized(target - origin);

      const RayT<T> ray{origin, direction};
      const HitT<T> exact = firstHit(a_shape, ray, T(0), T(10));
      const HitT<T> march = firstHit(marched, ray, T(0), T(10));

      // A grazing ray is where marching is slow and its crossing ill-conditioned; compare the rest. A ray that
      // passes just outside an edge, within the march's tolerance, is a hit to the march and a miss to the ray form.
      if (exact.hit() && std::abs(dot(exact.m_normal, direction)) < T(0.2)) {
        continue;
      }
      if (!exact.hit() && march.hit() && a_shape.signedDistance(march.m_point) > T(0)) {
        continue;
      }

      // A march that runs out of steps (along a grazing ray) has not decided; there is nothing to compare.
      if (march.m_status == HitStatus::Unresolved) {
        continue;
      }

      INFO("ray " << i << " from " << origin << " along " << direction);

      REQUIRE(march.m_status == exact.m_status);

      if (exact.hit()) {
        REQUIRE_FALSE(march.m_exact);
        REQUIRE(march.m_entering == exact.m_entering);
        REQUIRE_THAT(march.m_t, withinAbsT(exact.m_t, 0.01));

        // At an edge the march's normal blends the faces' normals; the exact one is then a face's, and a step along
        // it leaves the surface by that step.
        const T step = T(1e-2);

        if (std::abs(dot(march.m_normal, exact.m_normal) - T(1)) > T(0.01)) {
          REQUIRE_THAT(a_shape.signedDistance(exact.m_point + step * exact.m_normal), withinAbsT(step, 1e-3));
        }
      }

      compared++;
    }

    REQUIRE(compared > 200);
  };

  compare(SphereSDF<T>(Vec3(T(0.1), T(0), -T(0.2)), T(0.8)));
  compare(BoxSDF<T>(Vec3(-T(0.7), -T(0.5), -T(0.9)), Vec3(T(0.6), T(0.8), T(0.4))));
  compare(CylinderSDF<T>(Vec3(-T(0.6), -T(0.3), T(0)), Vec3(T(0.5), T(0.4), T(0.2)), T(0.45)));
  compare(CapsuleSDF<T>(Vec3(-T(0.6), -T(0.3), T(0)), Vec3(T(0.5), T(0.4), T(0.2)), T(0.45)));
  compare(EllipsoidSDF<T>(Vec3(T(0.1), T(0), -T(0.2)), Vec3(T(0.9), T(0.4), T(0.6))));
  compare(CappedConeSDF<T>(Vec3(-T(0.6), -T(0.3), T(0)), Vec3(T(0.5), T(0.4), T(0.2)), T(0.6), T(0.2)));
  compare(CappedConeSDF<T>(Vec3(-T(0.6), -T(0.3), T(0)), Vec3(T(0.5), T(0.4), T(0.2)), T(0), T(0.7)));
  compare(PolygonPrismSDF<T>(Vec3(-T(0.6), -T(0.3), T(0)), Vec3(T(0.5), T(0.4), T(0.2)), T(0.6), 5U, Vec3::ones()));
  compare(TubeSDF<T>(Vec3(-T(0.6), -T(0.3), T(0)), Vec3(T(0.5), T(0.4), T(0.2)), T(0.3), T(0.6)));
  compare(CutSphereSDF<T>(Vec3(T(0.1), T(0), -T(0.2)), T(0.8), T(0.3)));
  compare(CutSphereSDF<T>(Vec3(T(0.1), T(0), -T(0.2)), T(0.8), -T(0.4)));
  compare(WedgeSDF<T>(Vec3(T(0.1), T(0.2), T(0)), T(70)));
}

TEMPLATE_TEST_CASE("Ray: a tube's spans are its wall before and after the hollow, as its values along the ray say",
                   "[Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const TubeSDF<T> tube(Vec3(T(0), T(0), -T(1)), Vec3(T(0), T(0), T(1)), T(0.5), T(1));

  // Across the axis: the near wall from 1 to 1.5, the far wall from 2.5 to 3.
  const RayT<T> across{Vec3(-T(2), T(0), T(0)), Vec3::unit(0)};

  RaySpansT<T> spans;

  RayForm<TubeSDF<T>>::spans(tube, across, T(0), T(10), RayOptions<T>(), false, spans);

  REQUIRE(spans.m_count == 2);
  REQUIRE_THAT(spans.m_spans[0].m_in.m_t, withinAbsT(T(1), looseMargin<T>()));
  REQUIRE_THAT(spans.m_spans[0].m_out.m_t, withinAbsT(T(1.5), looseMargin<T>()));
  REQUIRE_THAT(spans.m_spans[1].m_in.m_t, withinAbsT(T(2.5), looseMargin<T>()));
  REQUIRE_THAT(spans.m_spans[1].m_out.m_t, withinAbsT(T(3), looseMargin<T>()));

  // From inside the hollow, the first crossing enters the wall from the hollow, its normal toward the axis.
  const HitT<T> fromHollow = firstHit(tube, RayT<T>{Vec3::zeros(), Vec3::unit(1)}, T(0), T(10));

  REQUIRE(fromHollow.hit());
  REQUIRE(fromHollow.m_entering);
  REQUIRE_THAT(fromHollow.m_t, withinAbsT(T(0.5), looseMargin<T>()));
  REQUIRE_THAT(fromHollow.m_normal[1], withinAbsT(-T(1), looseMargin<T>()));

  // Along the axis inside the hollow, nothing is crossed; along the wall, the end face is.
  REQUIRE_FALSE(firstHit(tube, RayT<T>{Vec3(T(0), T(0), -T(3)), Vec3::unit(2)}, T(0), T(10)).hit());
  REQUIRE_THAT(firstHit(tube, RayT<T>{Vec3(T(0.75), T(0), -T(3)), Vec3::unit(2)}, T(0), T(10)).m_t,
               withinAbsT(T(2), looseMargin<T>()));

  // On random rays, a point is in a span exactly where the tube's value is negative.
  std::mt19937                      rng(9);
  std::uniform_real_distribution<T> uniform(-T(1), T(1));

  for (int i = 0; i < 200; i++) {
    const Vec3    origin = T(3) * normalized(Vec3(uniform(rng), uniform(rng), uniform(rng)));
    const Vec3    target = T(0.5) * Vec3(uniform(rng), uniform(rng), uniform(rng));
    const RayT<T> ray{origin, normalized(target - origin)};

    RayForm<TubeSDF<T>>::spans(tube, ray, T(0), T(6), RayOptions<T>(), false, spans);

    for (int k = 0; k <= 600; k++) {
      const T t     = T(0.01) * T(k);
      const T value = tube.signedDistance(ray.at(t));

      // Close to the surface the two may disagree by rounding.
      if (std::abs(value) < T(1e-3)) {
        continue;
      }

      bool inside = false;

      for (uint32_t j = 0; j < spans.m_count; j++) {
        inside = inside || (t >= spans.m_spans[j].m_in.m_t && t <= spans.m_spans[j].m_out.m_t);
      }

      INFO("ray " << i << ", t = " << t << ", value " << value);
      REQUIRE(inside == (value < T(0)));
    }
  }
}

TEMPLATE_TEST_CASE("Ray: segments measure from their start and count a crossing at it",
                   "[Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const SphereSDF<T> sphere(Vec3::zeros(), T(1));

  // A zero-length segment crosses nothing.
  REQUIRE(firstHit(sphere, Vec3(T(1), T(0), T(0)), Vec3(T(1), T(0), T(0))).m_status == HitStatus::Miss);
  REQUIRE(intersects(sphere, Vec3(T(1), T(0), T(0)), Vec3(T(1), T(0), T(0))) == HitStatus::Miss);

  // From the surface outwards: the crossing is at the start, unless the search starts past it.
  const Vec3 onSurface(T(1), T(0), T(0));
  const Vec3 outside(T(3), T(0), T(0));

  const HitT<T> atStart = firstHit(sphere, onSurface, outside);

  REQUIRE(atStart.hit());
  REQUIRE(atStart.m_t == T(0));
  REQUIRE(firstHit(sphere, onSurface, outside, T(1e-3)).m_status == HitStatus::Miss);

  // Through the sphere: t is the distance from the start, and a segment ending inside is a crossing found
  // from the signs at its ends.
  const HitT<T> through = firstHit(sphere, Vec3(-T(3), T(0), T(0)), Vec3(T(3), T(0), T(0)));

  REQUIRE(through.hit());
  REQUIRE_THAT(through.m_t, withinAbsT(T(2), tightMargin<T>()));
  REQUIRE(intersects(sphere, Vec3(-T(3), T(0), T(0)), Vec3::zeros()) == HitStatus::Hit);
  REQUIRE(intersects(sphere, Vec3(-T(3), T(0), T(0)), Vec3(-T(2), T(0), T(0))) == HitStatus::Miss);

  // In and out again: the same sign at both ends, so the segment is searched.
  REQUIRE(intersects(sphere, Vec3(-T(3), T(0), T(0)), Vec3(T(3), T(0), T(0))) == HitStatus::Hit);
  REQUIRE(intersects(Marched<T, SphereSDF<T>>{sphere}, Vec3(-T(3), T(0), T(0)), Vec3(T(3), T(0), T(0))) ==
          HitStatus::Hit);
}

TEMPLATE_TEST_CASE("Ray: a march that runs out of steps is unresolved, not a miss", "[Ray]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // A ray grazing a sphere: sphere tracing takes ever shorter steps as it passes the surface.
  const Marched<T, SphereSDF<T>> sphere{SphereSDF<T>(Vec3::zeros(), T(1))};
  const RayT<T>                  grazing{Vec3(-T(3), T(1.0001), T(0)), Vec3(1, 0, 0)};

  RayOptions<T> few;
  few.m_maxSteps = 4;

  const HitT<T> hit = firstHit(sphere, grazing, T(0), T(10), few);

  REQUIRE(hit.m_status == HitStatus::Unresolved);
  REQUIRE(hit.m_t > T(0));
  REQUIRE(hit.m_t < T(10));
  REQUIRE(intersects(sphere, grazing, T(0), T(10), few) == HitStatus::Unresolved);

  // With enough steps it is decided.
  REQUIRE(firstHit(sphere, grazing, T(0), T(10)).m_status != HitStatus::Unresolved);
}

TEMPLATE_TEST_CASE("Ray: device ray queries of analytic shapes match the host",
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

  // An analytic ray form, and the march.
  const SphereSDF<T> sphere(Vec3(T(0.1), T(0), T(0)), T(0.9));

  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, SphereSDF<T>>{sphere}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, SphereSDF<T>>{sphere}, rays));
  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, Marched<T, SphereSDF<T>>>{{sphere}}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, Marched<T, SphereSDF<T>>>{{sphere}}, rays),
                     T(1e-3));

  // The ray forms of the other shapes with closed-form crossings.
  const auto checkShape = [&rays](const auto& a_shape) {
    using S = std::decay_t<decltype(a_shape)>;

    requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, S>{a_shape}, rays),
                       evaluateOnHost<T>(FirstCrossing<T, S>{a_shape}, rays));
  };

  checkShape(EllipsoidSDF<T>(Vec3(T(0.1), T(0), -T(0.2)), Vec3(T(0.9), T(0.4), T(0.6))));
  checkShape(CappedConeSDF<T>(Vec3(-T(0.6), -T(0.3), T(0)), Vec3(T(0.5), T(0.4), T(0.2)), T(0.6), T(0.2)));
  checkShape(PolygonPrismSDF<T>(Vec3(-T(0.6), -T(0.3), T(0)), Vec3(T(0.5), T(0.4), T(0.2)), T(0.6), 5U, Vec3::ones()));
  checkShape(TubeSDF<T>(Vec3(-T(0.6), -T(0.3), T(0)), Vec3(T(0.5), T(0.4), T(0.2)), T(0.3), T(0.6)));
  checkShape(CutSphereSDF<T>(Vec3(T(0.1), T(0), -T(0.2)), T(0.8), T(0.3)));
  checkShape(WedgeSDF<T>(Vec3(T(0.1), T(0.2), T(0)), T(70)));

  // The same crossings without their normals.
  requireSameResults(evaluateOnDevice<T>(CrossingDistance<T, SphereSDF<T>>{sphere}, rays),
                     evaluateOnHost<T>(CrossingDistance<T, SphereSDF<T>>{sphere}, rays));
  requireSameResults(evaluateOnDevice<T>(CrossingDistance<T, Marched<T, SphereSDF<T>>>{{sphere}}, rays),
                     evaluateOnHost<T>(CrossingDistance<T, Marched<T, SphereSDF<T>>>{{sphere}}, rays),
                     T(1e-3));
}
