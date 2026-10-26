// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Unit tests for the ray queries (EBGeometry_Ray.hpp and the ray forms): a TriMeshSDF of the unit cube
// against a slab test written here, the analytic ray forms against hand-computed crossings and against the
// march, the span arithmetic against sets of points, compositions (fused, BVH unions and tapes, with mesh
// SDFs, marched parts and user shapes) against hand-computed crossings and against their values sampled
// along the ray, segments, an unresolved march, and device evaluation.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <string>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;

namespace {

std::string
unitCube()
{
  return std::string(EBGEOMETRY_TEST_DATA_DIR) + "/cube_quads.obj";
}

// The reference: where a ray first crosses the surface of the closed box [0,1]^3 in [tMin, tMax], from
// either side, by the slab test. Returns +infinity for no crossing. A ray lying in a face's plane is
// outside the open slab on that axis and inside the closed one: it counts as inside, so it crosses where it
// enters or leaves along the other axes.
template <class T>
T
slabCrossing(const Vec3T<T>& a_origin, const Vec3T<T>& a_direction, const T a_tMin, const T a_tMax)
{
  T in  = -std::numeric_limits<T>::infinity();
  T out = std::numeric_limits<T>::infinity();

  for (size_t axis = 0; axis < 3; axis++) {
    const T o = a_origin[axis];
    const T d = a_direction[axis];

    if (d == T(0)) {
      if (o < T(0) || o > T(1)) {
        return std::numeric_limits<T>::infinity();
      }
    }
    else {
      const T t0 = (T(0) - o) / d;
      const T t1 = (T(1) - o) / d;

      in  = std::max(in, std::min(t0, t1));
      out = std::min(out, std::max(t0, t1));
    }
  }

  if (in > out) {
    return std::numeric_limits<T>::infinity();
  }
  if (in >= a_tMin && in <= a_tMax) {
    return in;
  }
  if (out >= a_tMin && out <= a_tMax) {
    return out;
  }

  return std::numeric_limits<T>::infinity();
}

// A cube of side a_side at a_corner, each face an a_n x a_n grid of quadrilaterals, counter-clockwise seen
// from outside: a fine mesh far from the origin, where rounding in a ray's crossing point is large next
// to the faces.
template <class T>
PolygonSoup<T>
gridCube(const Vec3T<T>& a_corner, const T a_side, const size_t a_n)
{
  std::vector<Vec3T<T>>            vertices;
  std::vector<std::vector<size_t>> faces;

  // Each face: a fixed axis and side, and two in-plane axes (u, v) with u x v pointing outwards.
  struct Side
  {
    size_t axis;
    T      at;
    size_t u;
    size_t v;
  };

  const Side sides[6] = {
    {0, T(0), 2, 1}, {0, T(1), 1, 2}, {1, T(0), 0, 2}, {1, T(1), 2, 0}, {2, T(0), 1, 0}, {2, T(1), 0, 1}};

  for (const Side& side : sides) {
    const size_t first = vertices.size();

    for (size_t j = 0; j <= a_n; j++) {
      for (size_t i = 0; i <= a_n; i++) {
        Vec3T<T> p;

        p[side.axis] = side.at;
        p[side.u]    = T(i) / T(a_n);
        p[side.v]    = T(j) / T(a_n);

        vertices.push_back(a_corner + a_side * p);
      }
    }

    for (size_t j = 0; j < a_n; j++) {
      for (size_t i = 0; i < a_n; i++) {
        const size_t a = first + j * (a_n + 1) + i;

        faces.push_back({a, a + 1, a + a_n + 2, a + a_n + 1});
      }
    }
  }

  return PolygonSoup<T>("grid cube", vertices, faces);
}

// A shape with no ray form, so the queries march it: what the ray forms are compared with.
template <class T, class S>
struct Marched
{
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  S m_shape;

  EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    return m_shape.signedDistance(a_point);
  }

  EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    return m_shape.computeBoundingVolume();
  }
};

template <class T>
Vec3T<T>
normalized(const Vec3T<T>& a_v)
{
  return a_v / a_v.length();
}

template <class T>
Vec3T<T>
unit(const T a_x, const T a_y, const T a_z)
{
  const Vec3T<T> v(a_x, a_y, a_z);

  return v / v.length();
}

// The solid of the CSG example: a cube with its corners rounded by a ball, drilled along each axis.
template <class T>
struct Part
{
  BoxSDF<T>      cube{-Vec3T<T>::ones(), Vec3T<T>::ones()};
  SphereSDF<T>   ball{Vec3T<T>::zeros(), T(1.35)};
  CylinderSDF<T> drillX{Vec3T<T>(-T(1.5), T(0), T(0)), Vec3T<T>(T(1.5), T(0), T(0)), T(0.55)};
  CylinderSDF<T> drillY{Vec3T<T>(T(0), -T(1.5), T(0)), Vec3T<T>(T(0), T(1.5), T(0)), T(0.55)};
  CylinderSDF<T> drillZ{Vec3T<T>(T(0), T(0), -T(1.5)), Vec3T<T>(T(0), T(0), T(1.5)), T(0.55)};

  auto
  fused() const
  {
    return Fused::Difference(Fused::Intersection(cube, ball), drillX, drillY, drillZ);
  }

  Expr<T>
  expr() const
  {
    return Difference(Intersection(cube, ball), Union(std::vector<Expr<T>>{drillX, drillY, drillZ}));
  }
};

// The distance to the first crossing, or -1 for none: comparable between host and device.
template <class T, class Shape>
struct FirstCrossing
{
  Shape m_shape;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const RayT<T>& a_ray) const noexcept
  {
    const HitT<T> hit = firstHit(m_shape, a_ray, T(0), T(10));

    return hit.hit() ? hit.m_t : -T(1);
  }
};

// A ball whose ray form has only firstHit(): a composition finds its spans crossing by crossing.
template <class T>
struct FirstHitBall
{
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Exact;

  SphereSDF<T> m_sphere;

  EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    return m_sphere.signedDistance(a_point);
  }

  EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    return m_sphere.computeBoundingVolume();
  }
};

// A ball whose ray form's firstHit() lacks the options argument: it is not a ray form, and the ball is marched.
template <class T>
struct MistypedBall : FirstHitBall<T>
{
};

// Where a ray is inside a line of balls along the x axis, as intervals of x: each ball's [cx - r, cx + r]
// on the axis.
template <class T>
std::vector<std::pair<T, T>>
ballRow(const size_t a_count, const T a_spacing, const T a_radius)
{
  std::vector<std::pair<T, T>> row;

  for (size_t i = 0; i < a_count; i++) {
    const T cx = -T(1) + a_spacing * T(i);

    row.emplace_back(cx - a_radius, cx + a_radius);
  }

  return row;
}

// The reference for a traced crossing: the shape's value sampled along the ray. A crossing is on the
// surface, the ray stays on one side of it before it (the outside if it enters there), and a miss changes
// side nowhere. A_skip is how far before the crossing the samples stop, so that the sample next to the
// crossing does not round to its other side.
template <class T, class S>
void
requireSampledAgreement(
  const S& a_shape, const RayT<T>& a_ray, const T a_tMax, const HitT<T>& a_hit, const size_t a_samples, const T a_skip)
{
  REQUIRE(a_hit.m_status != HitStatus::Unresolved);

  const T end = a_hit.hit() ? a_hit.m_t - a_skip : a_tMax;

  if (a_hit.hit()) {
    REQUIRE_THAT(a_shape.signedDistance(a_hit.m_point), withinAbsT(T(0), 10 * looseMargin<T>()));

    // The normal is the leaf's, found again from the leaf the trace ended on: it must be the gradient of the
    // shape's value there (where the gradient is one: not on an edge between two leaves).
    const T  h = std::is_same_v<T, float> ? T(1e-3) : T(1e-6);
    Vec3T<T> gradient;

    for (size_t dir = 0; dir < 3; dir++) {
      Vec3T<T> step = Vec3T<T>::zeros();

      step[dir] = h;

      gradient[dir] =
        (a_shape.signedDistance(a_hit.m_point + step) - a_shape.signedDistance(a_hit.m_point - step)) / (T(2) * h);
    }

    if (std::abs(gradient.length() - T(1)) < T(1e-2)) {
      REQUIRE(dot(a_hit.m_normal, gradient / gradient.length()) > T(0.99));
    }
  }

  if (!(end > T(0))) {
    return;
  }

  const bool inside = a_shape.signedDistance(a_ray.at(T(0))) < T(0);

  for (size_t k = 1; k <= a_samples; k++) {
    const T t = end * T(k) / T(a_samples);

    REQUIRE((a_shape.signedDistance(a_ray.at(t)) < T(0)) == inside);
  }

  if (a_hit.hit()) {
    REQUIRE(a_hit.m_entering == !inside);
  }
}

// A scene of balls and boxes in [-1, 1]^3, and rays through it from outside and from inside.
template <class T>
struct Scene
{
  std::vector<SphereSDF<T>> balls;
  std::vector<BoxSDF<T>>    boxes;
  std::vector<SphereSDF<T>> holes;
  std::vector<RayT<T>>      rays;

  explicit Scene(const unsigned a_seed)
  {
    std::mt19937                      rng(a_seed);
    std::uniform_real_distribution<T> u(-T(1), T(1));

    for (int i = 0; i < 6; i++) {
      balls.emplace_back(Vec3T<T>(u(rng), u(rng), u(rng)), T(0.25) + T(0.15) * u(rng));
    }

    for (int i = 0; i < 6; i++) {
      const Vec3T<T> c(u(rng), u(rng), u(rng));
      const Vec3T<T> h(T(0.2) + T(0.1) * u(rng), T(0.2) + T(0.1) * u(rng), T(0.2) + T(0.1) * u(rng));

      boxes.emplace_back(c - h, c + h);
    }

    for (int i = 0; i < 4; i++) {
      holes.emplace_back(Vec3T<T>(u(rng), u(rng), u(rng)), T(0.3) + T(0.1) * u(rng));
    }

    for (int i = 0; i < 80; i++) {
      // From outside, at a point near the middle; from inside, in any direction.
      const Vec3T<T> from = (i % 2 == 0) ? T(3) * unit(u(rng), u(rng), u(rng)) : Vec3T<T>(u(rng), u(rng), u(rng));
      const Vec3T<T> to(T(0.5) * u(rng), T(0.5) * u(rng), T(0.5) * u(rng));

      rays.push_back(RayT<T>{from, (i % 2 == 0) ? normalized(to - from) : unit(u(rng), u(rng), u(rng))});
    }
  }

  std::vector<Expr<T>>
  solids() const
  {
    std::vector<Expr<T>> e;

    for (const auto& b : balls) {
      e.emplace_back(b);
    }

    for (const auto& b : boxes) {
      e.emplace_back(b);
    }

    return e;
  }

  std::vector<Expr<T>>
  drills() const
  {
    return std::vector<Expr<T>>(holes.begin(), holes.end());
  }
};
} // namespace

namespace EBGeometry {

// FirstHitBall's ray form: the sphere's first crossing, and nothing else.
template <class T>
struct RayForm<FirstHitBall<T>>
{
  EBGEOMETRY_HOST_DEVICE
  static HitT<T>
  firstHit(
    const FirstHitBall<T>& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax, const RayOptions<T>& a_options) noexcept
  {
    return RayForm<SphereSDF<T>>::firstHit(a_shape.m_sphere, a_ray, a_tMin, a_tMax, a_options);
  }
};

template <class T>
struct RayForm<MistypedBall<T>>
{
  EBGEOMETRY_HOST_DEVICE
  static HitT<T>
  firstHit(const MistypedBall<T>& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax) noexcept
  {
    return RayForm<SphereSDF<T>>::firstHit(a_shape.m_sphere, a_ray, a_tMin, a_tMax, RayOptions<T>());
  }
};

} // namespace EBGeometry

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

TEMPLATE_TEST_CASE("Ray: every mesh SDF of the unit cube crosses where the slab test says",
                   "[Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // The cube as triangles (12) and as its six quadrilateral faces, through a BVH and without one.
  Pool       pool(hostMemoryResource());
  const auto triangles = Parser::readIntoTriMeshSDF<T, 4, 4>(unitCube(), pool);
  const auto polygons  = Parser::readIntoMeshSDF<T, 4>(unitCube(), pool);
  const auto flat      = Parser::readIntoFlatMeshSDF<T>(unitCube(), pool);

  const T inf = std::numeric_limits<T>::infinity();

  struct Case
  {
    const char* name;
    Vec3        origin;
    Vec3        direction;
    T           tMin;
    T           tMax;
    T           t;        // Expected crossing, inf for none.
    bool        entering; // Expected for a crossing.
  };

  const T h = T(0.5);

  const std::vector<Case> cases{
    {"axis-aligned", Vec3(-T(1), T(0.3), T(0.4)), Vec3(1, 0, 0), T(0), T(10), T(1), true},
    {"axis-aligned, from above", Vec3(T(0.3), T(0.4), T(3)), Vec3(0, 0, -1), T(0), T(10), T(2), true},
    {"diagonal", Vec3(-T(1), -T(1), -T(1)), unit(T(1), T(1), T(1)), T(0), T(10), std::sqrt(T(3)), true},
    {"through a face diagonal", Vec3(h, h, -T(1)), Vec3(0, 0, 1), T(0), T(10), T(1), true},
    {"through an edge", Vec3(-T(1), -T(1), h), unit(T(1), T(1), T(0)), T(0), T(10), std::sqrt(T(2)), true},
    {"through a corner", Vec3(T(2), T(2), T(2)), unit(-T(1), -T(1), -T(1)), T(0), T(10), std::sqrt(T(3)), true},
    {"from inside", Vec3(h, T(0.3), T(0.4)), Vec3(1, 0, 0), T(0), T(10), h, false},
    {"in a face's plane", Vec3(-T(1), h, T(0)), Vec3(1, 0, 0), T(0), T(10), T(1), true},
    {"beside", Vec3(-T(1), T(2), h), Vec3(1, 0, 0), T(0), T(10), inf, false},
    {"away", Vec3(-T(1), h, h), Vec3(-1, 0, 0), T(0), T(10), inf, false},
    {"stopping short", Vec3(-T(1), h, h), Vec3(1, 0, 0), T(0), T(0.9), inf, false},
    {"axis-aligned, to infinity", Vec3(-T(1), T(0.3), T(0.4)), Vec3(1, 0, 0), T(0), inf, T(1), true},
    {"on the surface, t = 0", Vec3(T(0), h, T(0.4)), Vec3(1, 0, 0), T(0), T(10), T(0), true},
    {"stepping off the surface", Vec3(T(0), h, T(0.4)), Vec3(1, 0, 0), T(1e-3), T(10), T(1), false},
  };

  const auto check = [&cases, inf](const auto& cube, const char* a_kind) {
    for (const Case& c : cases) {
      INFO(a_kind << ": " << c.name);

      const T reference = slabCrossing(c.origin, c.direction, c.tMin, c.tMax);

      // The reference agrees with the hand-written answer, so the cases are what they say.
      REQUIRE_THAT(std::min(reference, T(100)), withinAbsT(std::min(c.t, T(100)), tightMargin<T>()));

      const RayT<T> ray{c.origin, c.direction};
      const HitT<T> hit = firstHit(cube, ray, c.tMin, c.tMax);

      if (c.t == inf) {
        REQUIRE(hit.m_status == HitStatus::Miss);
        REQUIRE(intersects(cube, ray, c.tMin, c.tMax) == HitStatus::Miss);
      }
      else {
        REQUIRE(hit.m_status == HitStatus::Hit);
        REQUIRE(hit.m_exact);
        REQUIRE_THAT(hit.m_t, withinAbsT(c.t, tightMargin<T>()));
        REQUIRE_THAT((hit.m_point - ray.at(c.t)).length(), withinAbsT(T(0), tightMargin<T>()));
        REQUIRE(intersects(cube, ray, c.tMin, c.tMax) == HitStatus::Hit);

        // The normal is a face normal of the cube, its face holds the point, and the face id is that face's
        // (the file numbers the faces -z, +z, -y, +x, +y, -x).
        REQUIRE_THAT(hit.m_normal.length(), withinAbsT(T(1), tightMargin<T>()));

        const size_t axis = hit.m_normal.maxDir(true);
        const T      side = hit.m_normal[axis] > T(0) ? T(1) : T(0);

        REQUIRE_THAT(hit.m_point[axis], withinAbsT(side, tightMargin<T>()));

        const uint32_t faceOf[3][2] = {{5U, 3U}, {2U, 4U}, {0U, 1U}};

        REQUIRE(hit.m_primitive == faceOf[axis][size_t(side)]);

        // A ray starting on the surface has no "before"; only check the rest.
        if (c.t > T(0)) {
          REQUIRE(hit.m_entering == c.entering);
        }
      }
    }
  };

  check(triangles, "TriMeshSDF");
  check(polygons, "MeshSDF");
  check(flat, "FlatMeshSDF");
}

TEMPLATE_TEST_CASE("Ray: every mesh SDF of the unit cube agrees with the slab test on random rays",
                   "[Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // Triangles in two layouts (one leaf group per SIMD register, and with W = 8 several registers per
  // group), and the quadrilateral faces through a BVH and without one.
  Pool       pool(hostMemoryResource());
  const auto cube     = Parser::readIntoTriMeshSDF<T, 4, 4>(unitCube(), pool);
  const auto wide     = Parser::readIntoTriMeshSDF<T, 2, 8>(unitCube(), pool);
  const auto polygons = Parser::readIntoMeshSDF<T, 4>(unitCube(), pool);
  const auto flat     = Parser::readIntoFlatMeshSDF<T>(unitCube(), pool);

  std::mt19937                      rng(17);
  std::uniform_real_distribution<T> uniform(-T(1), T(2));

  for (int i = 0; i < 2000; i++) {
    const Vec3 origin(uniform(rng), uniform(rng), uniform(rng));
    const Vec3 target(uniform(rng), uniform(rng), uniform(rng));
    const Vec3 direction = (target - origin) / (target - origin).length();

    const T reference = slabCrossing(origin, direction, T(0), T(10));

    INFO("ray " << i);

    const RayT<T> ray{origin, direction};

    for (const HitT<T>& hit : {firstHit(cube, ray, T(0), T(10)),
                               firstHit(wide, ray, T(0), T(10)),
                               firstHit(polygons, ray, T(0), T(10)),
                               firstHit(flat, ray, T(0), T(10))}) {
      if (std::isinf(reference)) {
        REQUIRE(hit.m_status == HitStatus::Miss);
      }
      else {
        REQUIRE(hit.m_status == HitStatus::Hit);
        REQUIRE_THAT(hit.m_t, withinAbsT(reference, looseMargin<T>()));
      }
    }
  }
}

TEMPLATE_TEST_CASE("Ray: intersects() agrees with firstHit() on an unbounded ray that enters a leaf box and misses",
                   "[Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // A tetrahedron's faces have boxes with much empty space in them, so many rays enter a leaf box and miss
  // its faces. With tMax infinite, "no crossing" (+infinity) must still not count as a crossing.
  const std::string file = std::string(EBGEOMETRY_TEST_DATA_DIR) + "/tetrahedron.stl";

  Pool       pool(hostMemoryResource());
  const auto triangles = Parser::readIntoTriMeshSDF<T, 4, 4>(file, pool);
  const auto polygons  = Parser::readIntoMeshSDF<T, 4>(file, pool);
  const auto flat      = Parser::readIntoFlatMeshSDF<T>(file, pool);

  const auto box = polygons.computeBoundingVolume();
  const Vec3 lo  = box.getLowCorner();
  const Vec3 hi  = box.getHighCorner();
  const T    inf = std::numeric_limits<T>::infinity();

  std::mt19937                      rng(11);
  std::uniform_real_distribution<T> uniform(T(0), T(1));

  int misses = 0;

  for (int i = 0; i < 500; i++) {
    const Vec3 target = lo + Vec3(uniform(rng), uniform(rng), uniform(rng)) * (hi - lo);
    const Vec3 origin = target + T(5) * Vec3(uniform(rng) - T(0.5), uniform(rng) - T(0.5), uniform(rng) - T(0.5));

    const RayT<T> ray{origin, normalized(target - origin)};
    const HitT<T> hit = firstHit(polygons, ray, T(0), inf);

    INFO("ray " << i);

    REQUIRE(firstHit(triangles, ray, T(0), inf).hit() == hit.hit());
    REQUIRE(firstHit(flat, ray, T(0), inf).hit() == hit.hit());
    REQUIRE((intersects(triangles, ray, T(0), inf) == HitStatus::Hit) == hit.hit());
    REQUIRE((intersects(polygons, ray, T(0), inf) == HitStatus::Hit) == hit.hit());
    REQUIRE((intersects(flat, ray, T(0), inf) == HitStatus::Hit) == hit.hit());

    misses += hit.hit() ? 0 : 1;
  }

  // The rays are aimed into the box, not at the solid: many miss it.
  REQUIRE(misses > 100);
}

TEMPLATE_TEST_CASE("Ray: a sliver face is not crossed beyond its ends", "[Ray]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // One face, 1 long and a thousand ulps wide, in the plane z = 0. A ray from far above sees the face's
  // width as less than the rounding error of its crossing point: it may count as crossing the face beside
  // it (the safe side), but not 4 face lengths beyond its end.
  const T w = T(1000) * std::numeric_limits<T>::epsilon();

  const PolygonSoup<T> soup(
    "sliver", {Vec3(T(10), T(0), T(0)), Vec3(T(11), T(0), T(0)), Vec3(T(11), w, T(0))}, {{0, 1, 2}});

  Pool                      pool(hostMemoryResource());
  const auto                mesh = soup.convertToDCEL(pool, Parser::OnDefect::Warn);
  const FlatMeshSDF<T>      polygon(mesh, pool);
  const TriMeshSDF<T, 4, 4> triangle(soup.convertToTriangles(Parser::OnDefect::Warn), pool, BVH::Construction::SAH, 4);

  const Vec3 down(T(0), T(0), -T(1));

  const auto check = [&down, w](const auto& a_sliver) {
    REQUIRE(firstHit(a_sliver, RayT<T>{Vec3(T(10.9), T(0.45) * w, T(1000)), down}, T(0), T(2000)).hit());
    REQUIRE_FALSE(firstHit(a_sliver, RayT<T>{Vec3(T(15), T(0), T(1000)), down}, T(0), T(2000)).hit());
    REQUIRE_FALSE(firstHit(a_sliver, RayT<T>{Vec3(T(6), T(0), T(1000)), down}, T(0), T(2000)).hit());
  };

  check(polygon);
  check(triangle);
}

TEMPLATE_TEST_CASE("Ray: no ray slips between the faces of a fine mesh far from the origin",
                   "[Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // A cube of side 4 at about 100 from the origin, each face a 64 x 64 grid: faces 1/16 wide, rays from
  // about 150 away aimed at the grid's lines and corners, where two or four faces meet. Rounding in the
  // crossing point is then thousands of ulps of a face's size, and every ray must still hit (with
  // tolerances of a few ulps of the face, 30 of these rays slipped through the triangles).
  const Vec3 corner(T(100), T(98), T(101));
  const T    side = T(4);
  const auto soup = gridCube<T>(corner, side, 64);

  Pool       pool(hostMemoryResource());
  const auto mesh = soup.convertToDCEL(pool);

  const TriMeshSDF<T, 4, 4> triangles(soup.convertToTriangles(), pool, BVH::Construction::SAH, 4);
  const MeshSDF<T, 4>       polygons(mesh, pool, BVH::Construction::SAH);

  std::mt19937                       rng(23);
  std::uniform_real_distribution<T>  uniform(T(0), T(1));
  std::uniform_int_distribution<int> line(0, 64);

  int slippedTriangles = 0;
  int slippedPolygons  = 0;

  for (int i = 0; i < 4000; i++) {
    // A target on a face's grid: one coordinate on a grid line, or both (a corner), the other fixed by the face.
    const size_t axis    = size_t(i % 3);
    const T      at      = (i % 2 == 0) ? T(0) : T(1);
    const size_t u       = (axis + 1) % 3;
    const size_t v       = (axis + 2) % 3;
    const bool   corner2 = (i % 5 == 0);

    Vec3 local;

    local[axis] = at;
    local[u]    = T(line(rng)) / T(64);
    local[v]    = corner2 ? T(line(rng)) / T(64) : uniform(rng);

    const Vec3 target = corner + side * local;

    // From outside that face, a few units out, at an angle.
    Vec3 out = Vec3::zeros();

    out[axis] = (at > T(0)) ? T(1) : -T(1);

    const Vec3 origin    = target + T(150) * out + (uniform(rng) - T(0.5)) * T(100) * Vec3::ones();
    const Vec3 direction = normalized(target - origin);
    const T    distance  = (target - origin).length();

    const RayT<T> ray{origin, direction};

    // The ray may meet another face of the cube first (it comes in at an angle), never none, and never
    // beyond its target.
    const auto slips = [distance, side](const HitT<T>& a_hit) {
      return !(a_hit.hit() && a_hit.m_t <= distance + T(1e-3) * side);
    };

    slippedTriangles += slips(firstHit(triangles, ray, T(0), T(1000))) ? 1 : 0;
    slippedPolygons += slips(firstHit(polygons, ray, T(0), T(1000))) ? 1 : 0;
  }

  CHECK(slippedTriangles == 0);
  CHECK(slippedPolygons == 0);
}

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

  // A capsule along x, with tips at x = -1 and 1 (so its cap centres are at x = -0.5 and 0.5), is entered
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

      // A grazing ray is where marching is slow and its crossing ill-conditioned; compare the rest.
      if (exact.hit() && std::abs(dot(exact.m_normal, direction)) < T(0.2)) {
        continue;
      }

      INFO("ray " << i << " from " << origin << " along " << direction);

      REQUIRE(march.m_status == exact.m_status);

      if (exact.hit()) {
        REQUIRE_FALSE(march.m_exact);
        REQUIRE(march.m_entering == exact.m_entering);
        REQUIRE_THAT(march.m_t, withinAbsT(exact.m_t, 0.01));
        REQUIRE_THAT(dot(march.m_normal, exact.m_normal), withinAbsT(T(1), 0.01));
      }

      compared++;
    }

    REQUIRE(compared > 200);
  };

  compare(SphereSDF<T>(Vec3(T(0.1), T(0), -T(0.2)), T(0.8)));
  compare(BoxSDF<T>(Vec3(-T(0.7), -T(0.5), -T(0.9)), Vec3(T(0.6), T(0.8), T(0.4))));
  compare(CylinderSDF<T>(Vec3(-T(0.6), -T(0.3), T(0)), Vec3(T(0.5), T(0.4), T(0.2)), T(0.45)));
  compare(CapsuleSDF<T>(Vec3(-T(0.6), -T(0.3), T(0)), Vec3(T(0.5), T(0.4), T(0.2)), T(0.45)));
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

  // A union of 12 solids is evaluated with a BVH by default (TapeOptions::m_bvhUnionThreshold).
  const Expr<T> solids = Union(scene.solids());
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

  // The ball as a user shape whose ray form has only firstHit(), and the ball offset, which is marched.
  const BoxSDF<T> box(Vec3::zeros(), Vec3::ones());
  const auto      userDifference   = Fused::Difference(box, FirstHitBall<T>{hole});
  const auto      offsetDifference = Fused::Difference(box, Fused::Offset(SphereSDF<T>(middle, T(0.25)), T(0.05)));

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

TEMPLATE_TEST_CASE("Ray: a segment out of an open mesh through its hole crosses nothing",
                   "[Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // The unit cube without its top face. A segment from inside it out through the missing face (at y = 0.94)
  // to a point beside the rim crosses no triangle, although the value's sign differs at its ends: a mesh's
  // sign is not to be trusted at a hole.
  const std::string file = std::string(EBGEOMETRY_TEST_DATA_DIR) + "/cube_open_top.obj";

  Pool       pool(hostMemoryResource());
  const auto triangles = Parser::readIntoTriMeshSDF<T, 4, 4>(file, pool);
  const auto polygons  = Parser::readIntoMeshSDF<T, 4>(file, pool);
  const auto flat      = Parser::readIntoFlatMeshSDF<T>(file, pool);

  const Vec3 inside(T(0.5), T(0.5), T(0.4));
  const Vec3 above(T(0.5), T(1.3), T(1.5));

  const auto check = [&inside, &above](const auto& box) {
    REQUIRE(box.signedDistance(inside) < T(0));
    REQUIRE(box.signedDistance(above) > T(0));

    REQUIRE(intersects(box, inside, above) == HitStatus::Miss);
    REQUIRE(firstHit(box, inside, above).m_status == HitStatus::Miss);

    // Downwards it leaves through the bottom face.
    REQUIRE(intersects(box, inside, Vec3(T(0.5), T(0.5), -T(1))) == HitStatus::Hit);
  };

  check(triangles);
  check(polygons);
  check(flat);
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

TEMPLATE_TEST_CASE("Ray: device ray queries match the host", "[Ray][gpu]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  // Rays from a sphere of radius 3 towards points near the origin.
  std::vector<RayT<T>> rays;

  for (const Vec3& target : queryGrid<T>(-Vec3::ones(), Vec3::ones(), 5)) {
    const Vec3 origin(T(3) * std::cos(target[0] * T(7)), T(3) * std::sin(target[0] * T(7)), T(1.3) * target[2]);

    rays.push_back(RayT<T>{origin, normalized(target - origin)});
  }

  // An analytic ray form, and the march.
  const SphereSDF<T> sphere(Vec3(T(0.1), T(0), T(0)), T(0.9));

  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, SphereSDF<T>>{sphere}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, SphereSDF<T>>{sphere}, rays));
  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, Marched<T, SphereSDF<T>>>{{sphere}}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, Marched<T, SphereSDF<T>>>{{sphere}}, rays),
                     T(1e-3));

  // A fused composition, traced from its operands' spans.
  const Part<T> part;
  const auto    fused = part.fused();

  using F = std::decay_t<decltype(fused)>;

  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, F>{fused}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, F>{fused}, rays));

  // The mesh SDFs and a tape, against device mirrors of their pool.
  Pool pool(hostMemoryResource());

  const auto    cube     = Parser::readIntoTriMeshSDF<T, 4, 4>(unitCube(), pool);
  const auto    polygons = Parser::readIntoMeshSDF<T, 4>(unitCube(), pool);
  const auto    flat     = Parser::readIntoFlatMeshSDF<T>(unitCube(), pool);
  const Tape<T> tape(pool, part.expr());

  // A tape with a BVH union and a difference, and a mesh in a fused difference.
  const Scene<T> scene(5);
  const Tape<T>  sceneTape(pool, Difference(Union(scene.solids()), Union(scene.drills())));
  const auto     meshDifference = Fused::Difference(polygons, SphereSDF<T>(T(0.5) * Vec3::ones(), T(0.3)));

  pool.freeze();

  const Pool device = Pool::mirror(pool, deviceTestResource());

  using M = std::decay_t<decltype(cube)>;

  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, M>{cube.rebasedView(device)}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, M>{cube}, rays));

  using P = std::decay_t<decltype(polygons)>;
  using L = std::decay_t<decltype(flat)>;

  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, P>{polygons.rebasedView(device)}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, P>{polygons}, rays));
  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, L>{flat.rebasedView(device)}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, L>{flat}, rays));
  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, Tape<T>>{tape.rebasedView(device)}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, Tape<T>>{tape}, rays));
  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, Tape<T>>{sceneTape.rebasedView(device)}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, Tape<T>>{sceneTape}, rays));

  using D = std::decay_t<decltype(meshDifference)>;

  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, D>{meshDifference.rebasedView(device)}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, D>{meshDifference}, rays));
}
