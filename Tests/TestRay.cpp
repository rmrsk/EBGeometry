// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"
#include "TestTapeUtils.hpp"

#include <cmath>
#include <cstdint>
#include <string>
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

// Rays from a ring of origins around the origin, aimed at points spread over a box around it: some
// hit a unit-sized object there, some miss it.
template <class T>
std::vector<RayT<T>>
fanOfRays(const T a_radius)
{
  std::vector<RayT<T>> rays;

  for (int i = 0; i < 12; i++) {
    const T        angle = T(2) * pi<T> * T(i) / T(12);
    const Vec3T<T> origin(a_radius * std::cos(angle), T(0.3) * T(i % 3) - T(0.3), a_radius * std::sin(angle));

    for (int j = 0; j < 25; j++) {
      const Vec3T<T> target(T(0.35) * T(j % 5) - T(0.7), T(0.35) * T(j / 5) - T(0.7), T(0.1) * T(j % 3));
      const Vec3T<T> d = target - origin;

      rays.push_back(RayT<T>{origin, d / d.length()});
    }
  }

  return rays;
}

// Where a ray first meets a sphere: the smaller root of |o + t d - c|^2 = r^2 at or after a_tMin, or
// -1 if none. a_cos is set to the cosine between the ray and the normal there.
template <class T>
T
sphereHit(const RayT<T>& a_ray, const Vec3T<T>& a_center, const T a_radius, const T a_tMin, T& a_cos)
{
  const Vec3T<T> oc = a_ray.m_origin - a_center;

  const T b    = dot(oc, a_ray.m_direction);
  const T c    = dot(oc, oc) - a_radius * a_radius;
  const T disc = b * b - c;

  if (disc < T(0)) {
    return T(-1);
  }

  for (const T t : {-b - std::sqrt(disc), -b + std::sqrt(disc)}) {
    if (t >= a_tMin) {
      const Vec3T<T> n = (a_ray.at(t) - a_center) / a_radius;

      a_cos = std::abs(dot(n, a_ray.m_direction));

      return t;
    }
  }

  return T(-1);
}

// A sphere whose value has the right sign but is not a distance: ten times |p - c|^2 - r^2.
template <class T>
struct SquaredSphere
{
  Vec3T<T> m_center;
  T        m_radius;

  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    return T(10) * ((a_point - m_center).length2() - m_radius * m_radius);
  }

  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    return BoundingVolumes::AABBT<T>(m_center - m_radius * Vec3T<T>::ones(), m_center + m_radius * Vec3T<T>::ones());
  }

  static constexpr DistanceQuality distanceQuality = DistanceQuality::NotADistance;
};

// Device functor: the distance to the first crossing along a ray, or -1 for a miss and -2 for an
// unresolved query.
template <class T, class Shape>
struct HitQuery
{
  Shape m_shape;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const RayT<T>& a_ray) const noexcept
  {
    const HitT<T> hit = firstHit(m_shape, a_ray, T(0), T(100));

    return hit.hit() ? hit.m_t : ((hit.m_status == HitStatus::Miss) ? T(-1) : T(-2));
  }
};

} // namespace

TEMPLATE_TEST_CASE("Ray: firstHit on a sphere matches the closed form, from outside and from inside",
                   "[Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Vec3         center(T(0.1), T(-0.2), T(0.05));
  const T            radius = T(0.8);
  const SphereSDF<T> sphere(center, radius);

  const RayOptions<T> options;

  // The march stops within the tolerance of the surface, measured along the normal.
  const T eps = options.m_tolerance * T(2) * radius * std::sqrt(T(3));

  size_t hits   = 0;
  size_t misses = 0;

  for (const auto& ray : fanOfRays<T>(T(4))) {
    T       cosine = T(1);
    const T tRef   = sphereHit(ray, center, radius, T(0), cosine);

    const HitT<T> hit = firstHit(sphere, ray, T(0), T(100));

    // Rays that only graze the sphere can go either way.
    if (tRef >= T(0) && cosine < T(0.2)) {
      continue;
    }

    if (tRef < T(0)) {
      CHECK(hit.m_status == HitStatus::Miss);

      misses++;
    }
    else {
      REQUIRE(hit.hit());
      CHECK(hit.m_entering);
      CHECK_FALSE(hit.m_exact);
      CHECK(hit.m_primitive == 0U);
      CHECK(std::abs(hit.m_t - tRef) <= eps / cosine + T(looseMargin<T>()));
      CHECK(std::abs(sphere.signedDistance(hit.m_point)) <= eps);

      hits++;
    }
  }

  CHECK(hits > 50U);
  CHECK(misses > 50U);

  // From the centre the ray leaves the sphere at its radius.
  for (const Vec3& d : {Vec3(1, 0, 0), Vec3(0, -1, 0), Vec3(T(0.6), T(0), T(0.8))}) {
    const HitT<T> hit = firstHit(sphere, RayT<T>{center, d}, T(0), T(100));

    REQUIRE(hit.hit());
    CHECK_FALSE(hit.m_entering);
    CHECK(std::abs(hit.m_t - radius) <= eps + T(looseMargin<T>()));
  }

  // The interval is respected: a crossing beyond a_tMax is a miss, and starting past the near crossing
  // finds where the ray leaves.
  const RayT<T> ray{center - Vec3(T(3), T(0), T(0)), Vec3(1, 0, 0)};

  CHECK(firstHit(sphere, ray, T(0), T(2)).m_status == HitStatus::Miss);

  const HitT<T> leaving = firstHit(sphere, ray, T(3), T(100));

  REQUIRE(leaving.hit());
  CHECK_FALSE(leaving.m_entering);
  CHECK(std::abs(leaving.m_t - (T(3) + radius)) <= eps + T(looseMargin<T>()));

  // The interval's ends are not deduced, so double literals work for a float ray too.
  CHECK(firstHit(sphere, ray, 0.0, 100.0).hit());
}

TEMPLATE_TEST_CASE("Ray: firstHit on a box and on an unbounded plane", "[Ray]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const BoxSDF<T> box(Vec3(T(-1), T(-0.5), T(-0.25)), Vec3(T(0.5), T(1), T(0.75)));

  const RayOptions<T> options;

  const T eps = options.m_tolerance * Vec3(T(1.5), T(1.5), T(1)).length();

  // Rays straight at each face from outside meet it head on.
  const Vec3 lo = box.computeBoundingVolume().getLowCorner();
  const Vec3 hi = box.computeBoundingVolume().getHighCorner();

  for (size_t dir = 0; dir < 3; dir++) {
    Vec3 origin = T(0.5) * (lo + hi);
    Vec3 d      = Vec3::zeros();

    origin[dir] = hi[dir] + T(2);
    d[dir]      = T(-1);

    const HitT<T> hit = firstHit(box, RayT<T>{origin, d}, T(0), T(100));

    REQUIRE(hit.hit());
    CHECK(hit.m_entering);
    CHECK(std::abs(hit.m_t - T(2)) <= eps + T(looseMargin<T>()));
  }

  // A ray past the box misses it.
  CHECK(firstHit(box, RayT<T>{Vec3(T(3), T(3), T(3)), Vec3(1, 0, 0)}, T(0), T(100)).m_status == HitStatus::Miss);

  // A plane has an unbounded box, so the march takes its length scale from the options.
  const PlaneSDF<T> plane(Vec3(T(0), T(0.5), T(0)), Vec3(T(0), T(1), T(0)));

  RayOptions<T> planeOptions;

  planeOptions.m_scale = T(1);

  const Vec3 d = Vec3(T(0.3), T(-1), T(0.2)) / Vec3(T(0.3), T(-1), T(0.2)).length();

  const RayT<T> down{Vec3(T(0), T(3), T(0)), d};
  const HitT<T> hit = firstHit(plane, down, T(0), T(100), planeOptions);

  REQUIRE(hit.hit());
  CHECK(std::abs(hit.m_t - T(2.5) / std::abs(d[1])) <= T(2) * planeOptions.m_tolerance / std::abs(d[1]));

  // Moving away from the plane, the march reaches the end of the interval.
  CHECK(firstHit(plane, RayT<T>{Vec3(T(0), T(3), T(0)), Vec3(0, 1, 0)}, T(0), T(100), planeOptions).m_status ==
        HitStatus::Miss);
}

TEMPLATE_TEST_CASE("Ray: a hole seen from inside the solid is a leaving crossing", "[Ray]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const SphereSDF<T> ball(Vec3::zeros(), T(1));

  // The complement is solid everywhere but in the ball; a camera outside the ball is inside the solid,
  // and what it sees first is where its ray leaves the solid, on the ball's near side.
  const TestUtils::Compiled<T> hole(Complement(ball));

  RayOptions<T> options;

  options.m_scale = T(2);

  const HitT<T> hit = firstHit(hole.tape(), RayT<T>{Vec3(T(-3), T(0), T(0)), Vec3(1, 0, 0)}, T(0), T(10), options);

  REQUIRE(hit.hit());
  CHECK_FALSE(hit.m_entering);
  CHECK(std::abs(hit.m_t - T(2)) <= T(2) * options.m_tolerance * options.m_scale);

  // The same ray on the ball enters it there.
  const HitT<T> solid = firstHit(ball, RayT<T>{Vec3(T(-3), T(0), T(0)), Vec3(1, 0, 0)}, T(0), T(10));

  REQUIRE(solid.hit());
  CHECK(solid.m_entering);
}

TEMPLATE_TEST_CASE("Ray: a hit names the tape's leaf, and the mesh SDF's face", "[Ray]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const SphereSDF<T> left(Vec3(T(-1.5), T(0), T(0)), T(0.5));
  const SphereSDF<T> right(Vec3(T(1.5), T(0), T(0)), T(0.5));

  const TestUtils::Compiled<T> both(Union(left, right));

  const HitT<T> toLeft  = both.tape().firstHit(RayT<T>{Vec3(T(-1.5), T(3), T(0)), Vec3(0, -1, 0)}, T(0), T(10));
  const HitT<T> toRight = both.tape().firstHit(RayT<T>{Vec3(T(1.5), T(3), T(0)), Vec3(0, -1, 0)}, T(0), T(10));

  REQUIRE(toLeft.hit());
  REQUIRE(toRight.hit());
  CHECK(toLeft.m_primitive == 0U);
  CHECK(toRight.m_primitive == 1U);

  // A mesh SDF names the face the hit is on; the hit is on the surface.
  Pool pool(hostMemoryResource());

  constexpr size_t K = BVH::DefaultBranchingRatio<T>();
  constexpr size_t W = TriangleSoA::DefaultWidth<T>();

  const auto                mesh = Parser::readIntoDCEL<T>(dodecahedron(), pool);
  const MeshSDF<T, K>       meshSDF(mesh, pool, BVH::Construction::SAH);
  const TriMeshSDF<T, K, W> triSDF(mesh, pool, BVH::Construction::SAH, 4);

  const RayOptions<T> options;

  const auto box = meshSDF.computeBoundingVolume();
  const T    eps = options.m_tolerance * (box.getHighCorner() - box.getLowCorner()).length();

  size_t hits = 0;

  for (const auto& ray : fanOfRays<T>(T(5))) {
    const HitT<T> hit = firstHit(meshSDF, ray, T(0), T(100));
    const HitT<T> tri = firstHit(triSDF, ray, T(0), T(100));

    REQUIRE(hit.m_status != HitStatus::Unresolved);

    if (hit.hit()) {
      CHECK(std::abs(meshSDF.signedDistance(hit.m_point)) <= eps);
      CHECK(hit.m_primitive == meshSDF.getClosestFace(hit.m_point).faceId);
      CHECK(hit.m_entering);

      if (tri.hit()) {
        CHECK(std::abs(tri.m_t - hit.m_t) <= T(0.05));
      }

      hits++;
    }
  }

  CHECK(hits > 20U);
}

TEMPLATE_TEST_CASE("Ray: a value that is not a distance is stepped and bisected", "[Ray]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Vec3             center(T(0.1), T(-0.2), T(0.05));
  const T                radius = T(0.8);
  const SquaredSphere<T> squared{center, radius};

  const RayOptions<T> options;

  const T scale = T(2) * radius * std::sqrt(T(3));
  const T eps   = options.m_tolerance * scale;

  size_t hits = 0;

  for (const auto& ray : fanOfRays<T>(T(4))) {
    T       cosine = T(1);
    const T tRef   = sphereHit(ray, center, radius, T(0), cosine);

    // A chord shorter than a step can be stepped over; keep rays that cross well inside.
    if (tRef >= T(0) && cosine < T(0.3)) {
      continue;
    }

    const HitT<T> hit = firstHit(squared, ray, T(0), T(100));

    if (tRef < T(0)) {
      CHECK(hit.m_status == HitStatus::Miss);
    }
    else {
      REQUIRE(hit.hit());
      CHECK(hit.m_entering);
      CHECK(std::abs(hit.m_t - tRef) <= T(2) * eps);

      hits++;
    }
  }

  CHECK(hits > 30U);

  // A tape holding a value that is not a distance marches the same way.
  const TestUtils::Compiled<T> noise(Offset(PerlinSDF<T>(T(0.5), T(2) * Vec3::ones(), T(0.5), 4), T(0.45)));

  CHECK(noise.tape().getDistanceQuality() == DistanceQuality::NotADistance);

  RayOptions<T> noiseOptions;

  noiseOptions.m_scale    = T(1);
  noiseOptions.m_maxSteps = 4000;

  const HitT<T> crossing =
    noise.tape().firstHit(RayT<T>{Vec3(T(-5), T(0.1), T(0.2)), Vec3(1, 0, 0)}, T(0), T(10), noiseOptions);

  if (crossing.hit()) {
    const T before = noise.signedDistance(crossing.m_point - T(4) * eps * Vec3(1, 0, 0));
    const T after  = noise.signedDistance(crossing.m_point + T(4) * eps * Vec3(1, 0, 0));

    CHECK((before < T(0)) != (after < T(0)));
  }
}

TEMPLATE_TEST_CASE("Ray: a query out of steps is unresolved, never a miss", "[Ray]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const SphereSDF<T> sphere(Vec3::zeros(), T(1));

  RayOptions<T> few;

  few.m_maxSteps = 2;

  // The march starts where the ray enters the sphere's box, at t = 49, and approaches the surface
  // more slowly than two steps can cover.
  const RayT<T> ray{Vec3(T(-50), T(0.5), T(0)), Vec3(1, 0, 0)};

  const HitT<T> hit = firstHit(sphere, ray, T(0), T(100), few);

  CHECK(hit.m_status == HitStatus::Unresolved);
  CHECK(hit.m_t >= T(49));
  CHECK(hit.m_t < T(50));

  // With the default budget the same query is decided.
  CHECK(firstHit(sphere, ray, T(0), T(100)).hit());
}

TEMPLATE_TEST_CASE("Ray: intersects tells whether a segment crosses the surface", "[Ray]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const SphereSDF<T> sphere(Vec3::zeros(), T(1));

  const TestUtils::Compiled<T> compiled(sphere);

  const Vec3 inside(T(0.2), T(0.1), T(0));
  const Vec3 outside(T(3), T(0), T(0));
  const Vec3 farSide(T(-3), T(0), T(0));
  const Vec3 above(T(-3), T(2), T(0));

  // Ends on different sides: a crossing, without a march.
  CHECK(intersects(sphere, inside, outside) == HitStatus::Hit);

  // Both ends outside, through the sphere: in and out again.
  CHECK(intersects(sphere, outside, farSide) == HitStatus::Hit);

  // Both ends outside, past it, or both inside: no crossing.
  CHECK(intersects(sphere, outside, above + Vec3(T(6), T(0), T(0))) == HitStatus::Miss);
  CHECK(intersects(sphere, inside, Vec3(T(-0.3), T(0.2), T(0.1))) == HitStatus::Miss);

  // The tape's members are the free functions.
  for (const auto& [a, b] :
       std::vector<std::pair<Vec3, Vec3>>{{inside, outside}, {outside, farSide}, {outside, above}}) {
    CHECK(compiled.tape().intersects(a, b) == intersects(sphere, a, b));
  }

  CHECK(intersects(sphere, outside, outside) == HitStatus::Miss);
}

TEMPLATE_TEST_CASE("Ray: firstHit on the device matches the host", "[Ray][gpu]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  const auto rays = fanOfRays<T>(T(4));

  const SphereSDF<T>     sphere(Vec3(T(0.1), T(-0.2), T(0.05)), T(0.8));
  const SquaredSphere<T> squared{Vec3(T(0.1), T(-0.2), T(0.05)), T(0.8)};

  requireSameResults(evaluateOnDevice<T>(HitQuery<T, SphereSDF<T>>{sphere}, rays),
                     evaluateOnHost<T>(HitQuery<T, SphereSDF<T>>{sphere}, rays));
  requireSameResults(evaluateOnDevice<T>(HitQuery<T, SquaredSphere<T>>{squared}, rays),
                     evaluateOnHost<T>(HitQuery<T, SquaredSphere<T>>{squared}, rays));

  Pool pool(hostMemoryResource());

  const Tape<T> tape(pool, Difference(BoxSDF<T>(-Vec3::ones(), Vec3::ones()), SphereSDF<T>(Vec3::zeros(), T(1.2))));

  pool.freeze();

  const Pool    devicePool = Pool::mirror(pool, deviceTestResource());
  const Tape<T> view       = tape.rebasedView(devicePool);

  requireSameResults(evaluateOnDevice<T>(HitQuery<T, Tape<T>>{view}, rays),
                     evaluateOnHost<T>(HitQuery<T, Tape<T>>{tape}, rays));
}
