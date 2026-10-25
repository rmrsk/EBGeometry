// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Unit tests for the ray queries (EBGeometry_Ray.hpp and the ray forms): a TriMeshSDF of the unit cube
// against a slab test written here, the analytic ray forms against hand-computed crossings and against the
// march, compositions (fused and taped) against hand-computed crossings, segments, an unresolved march, and
// device evaluation.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

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

} // namespace

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

TEMPLATE_TEST_CASE("Ray: fused and taped compositions are marched to the closed-form crossings",
                   "[Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Part<T> part;
  const auto    fused = part.fused();

  Pool          pool(hostMemoryResource());
  const Tape<T> tape(pool, part.expr());

  // March tolerance: 1e-4 of the part's box diagonal (about 3.5e-4), so the crossings are found to that.
  const double margin = 1e-3;

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
      REQUIRE_FALSE(hit.m_exact);
      REQUIRE(hit.m_entering == c.entering);
      REQUIRE_THAT(hit.m_t, withinAbsT(c.t, margin));
      REQUIRE_THAT(dot(hit.m_normal, c.normal), withinAbsT(T(1), margin));
    }

    REQUIRE(viaTape.m_primitive == c.leaf);
    REQUIRE(tape.intersects(c.from, c.to) == HitStatus::Hit);
    REQUIRE(intersects(fused, c.from, c.to) == HitStatus::Hit);

    // The ray overloads agree with the segment ones.
    const Vec3    direction = normalized(c.to - c.from);
    const HitT<T> viaRay    = tape.firstHit(RayT<T>{c.from, direction}, T(0), (c.to - c.from).length());

    REQUIRE(viaRay.hit());
    REQUIRE(viaRay.m_t == viaTape.m_t);
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

  // A fused composition, marched.
  const Part<T> part;
  const auto    fused = part.fused();

  using F = std::decay_t<decltype(fused)>;

  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, F>{fused}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, F>{fused}, rays),
                     T(1e-3));

  // The mesh SDFs and a tape, against device mirrors of their pool.
  Pool pool(hostMemoryResource());

  const auto    cube     = Parser::readIntoTriMeshSDF<T, 4, 4>(unitCube(), pool);
  const auto    polygons = Parser::readIntoMeshSDF<T, 4>(unitCube(), pool);
  const auto    flat     = Parser::readIntoFlatMeshSDF<T>(unitCube(), pool);
  const Tape<T> tape(pool, part.expr());

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
                     evaluateOnHost<T>(FirstCrossing<T, Tape<T>>{tape}, rays),
                     T(1e-3));
}
