// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Unit tests for the ray queries of mesh SDFs: a TriMeshSDF, MeshSDF and FlatMeshSDF of the unit cube
// against a slab test (TestRayHelpers.hpp), sliver faces, a fine mesh far from the origin, a segment through an
// open mesh's hole, and device evaluation.

#include "TestRayHelpers.hpp"

using namespace EBGeometry;
using namespace EBGeometryTestRay;

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

TEMPLATE_TEST_CASE("Ray: device ray queries of mesh SDFs match the host", "[Ray][gpu]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  const std::vector<RayT<T>> rays = deviceRays<T>();

  // The mesh SDFs, against device mirrors of their pool.
  Pool pool(hostMemoryResource());

  const auto cube     = Parser::readIntoTriMeshSDF<T, 4, 4>(unitCube(), pool);
  const auto polygons = Parser::readIntoMeshSDF<T, 4>(unitCube(), pool);
  const auto flat     = Parser::readIntoFlatMeshSDF<T>(unitCube(), pool);

  pool.freeze();

  const Pool device = Pool::mirror(pool, deviceTestResource());

  using M = std::decay_t<decltype(cube)>;
  using P = std::decay_t<decltype(polygons)>;
  using L = std::decay_t<decltype(flat)>;

  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, M>{cube.rebasedView(device)}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, M>{cube}, rays));
  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, P>{polygons.rebasedView(device)}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, P>{polygons}, rays));
  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, L>{flat.rebasedView(device)}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, L>{flat}, rays));

  // The same crossings without their normals.
  requireSameResults(evaluateOnDevice<T>(CrossingDistance<T, M>{cube.rebasedView(device)}, rays),
                     evaluateOnHost<T>(CrossingDistance<T, M>{cube}, rays));
}
