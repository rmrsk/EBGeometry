// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// Test suite for the mesh SDFs built on the BVHs (MeshSDF, FlatMeshSDF, TriMeshSDF): signed distances, signs,
// closest faces, points and gradients, construction options, copies and views, and the parsers that build them.
// Uses a regular dodecahedron (20 vertices, 36 triangulated faces, watertight and orientable) read from disk in
// all four supported formats (STL/PLY/OBJ/VTK).

#include "EBGeometry.hpp"
#include "TestBVHHelpers.hpp"
#include "TestConstructions.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"
#include "TestUtilities.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <limits>
#include <memory>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using namespace EBGeometryTestBVH;
using Catch::Matchers::WithinRel;

namespace {

// Comparisons across independently-parsed file formats of literally the same geometry should
// agree almost exactly; the residual gap is down to how each format's ASCII/binary encoding
// round-trips floating-point coordinates, not accumulated BVH arithmetic.
template <class T>
double
formatMargin()
{
  return std::is_same_v<T, float> ? 1.0e-4 : 1.0e-9;
}

// A soup of 512 small, differently oriented triangles on a jittered grid: enough for many levels,
// and no two triangles at the same distance from a query.
template <class T>
std::vector<Triangle<T>>
jitteredTriangleSoup(std::mt19937& a_rng)
{
  using Vec3 = Vec3T<T>;

  std::uniform_real_distribution<T> jitter(T(-0.2), T(0.2));

  std::vector<Triangle<T>> tris;

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      for (int k = 0; k < 8; k++) {
        const Vec3 base(T(i) + jitter(a_rng), T(j) + jitter(a_rng), T(k) + jitter(a_rng));
        const Vec3 a = base + Vec3(T(0.3), jitter(a_rng), jitter(a_rng));
        const Vec3 b = base + Vec3(jitter(a_rng), T(0.3), jitter(a_rng));
        const Vec3 n = (a - base).cross(b - base) / (a - base).cross(b - base).length();

        Triangle<T> tri;
        tri.setVertexPositions({base, a, b});
        tri.setNormal(n);
        tri.setVertexNormals({n, n, n});
        tri.setEdgeNormals({n, n, n});

        tris.emplace_back(tri);
      }
    }
  }

  return tris;
}

// 200 query points inside the soup's grid.
template <class T>
std::vector<Vec3T<T>>
jitteredSoupQueries(std::mt19937& a_rng)
{
  std::uniform_real_distribution<T> jitter(T(-0.2), T(0.2));

  std::vector<Vec3T<T>> queries;

  for (int q = 0; q < 200; q++) {
    queries.emplace_back(T(9) * (jitter(a_rng) + T(0.5)) - T(0.5),
                         T(9) * (jitter(a_rng) + T(0.5)) - T(0.5),
                         T(9) * (jitter(a_rng) + T(0.5)) - T(0.5));
  }

  return queries;
}

// The distance from a_point to the nearest triangle of a_tris, by brute force.
template <class T>
T
bruteNearestTriangle(const std::vector<Triangle<T>>& a_tris, const Vec3T<T>& a_point)
{
  T nearest = std::numeric_limits<T>::max();

  for (const auto& tri : a_tris) {
    nearest = std::min(nearest, std::abs(tri.signedDistance(a_point)));
  }

  return nearest;
}

bool
isTopDown(const BVH::Construction a_construction)
{
  return a_construction == BVH::Construction::CentroidSplit || a_construction == BVH::Construction::MidpointSplit ||
         a_construction == BVH::Construction::SAH;
}

bool
isSpaceFillingCurve(const BVH::Construction a_construction)
{
  return a_construction == BVH::Construction::Morton || a_construction == BVH::Construction::Nested ||
         a_construction == BVH::Construction::Hilbert;
}

// The largest leaf of a packed BVH, in its own primitives.
template <class Root>
uint32_t
largestLeaf(const Root& a_root)
{
  const auto nodes = a_root.getNodes();

  uint32_t largest = 0;

  for (uint32_t i = 0; i < nodes.size(); i++) {
    if (nodes[i].isLeaf()) {
      largest = std::max(largest, nodes[i].getNumPrimitives());
    }
  }

  return largest;
}

} // namespace

TEMPLATE_TEST_CASE("Dodecahedron: all four file formats parse into an identical, watertight DCEL mesh",
                   "[BVH][Dodecahedron]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  const auto meshSTL = Parser::readIntoDCEL<T>(dataPath("dodecahedron.stl"), pool);
  const auto meshPLY = Parser::readIntoDCEL<T>(dataPath("dodecahedron.ply"), pool);
  const auto meshOBJ = Parser::readIntoDCEL<T>(dataPath("dodecahedron.obj"), pool);
  const auto meshVTK = Parser::readIntoDCEL<T>(dataPath("dodecahedron.vtk"), pool);

  // Every mesh below is queried directly through its own accessors; each attaches to pool on its
  // first reserve inside readIntoDCEL, so nothing has to be frozen or bound first.
  for (const auto& mesh : {meshSTL, meshPLY, meshOBJ, meshVTK}) {
    REQUIRE(mesh.numVertices() == 20);
    REQUIRE(mesh.numFaces() == 36);
    REQUIRE(mesh.numEdges() == 108); // 54 undirected edges * 2 half-edges each.

    for (uint32_t i = 0; i < mesh.numEdges(); i++) {
      REQUIRE(mesh.getEdge(i).getPairEdgeIndex() != UINT32_MAX); // Watertight: every half-edge has a pair.
    }
  }

  // Same geometry regardless of source format: signed distance must agree at every query point.
  for (const auto& p : queryPoints<T>()) {
    const T d = meshSTL.signedDistance(p);

    REQUIRE_THAT(meshPLY.signedDistance(p), withinAbsT(d, formatMargin<T>()));
    REQUIRE_THAT(meshOBJ.signedDistance(p), withinAbsT(d, formatMargin<T>()));
    REQUIRE_THAT(meshVTK.signedDistance(p), withinAbsT(d, formatMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("MeshSDF: signedDistance agrees with FlatMeshSDF for every BVH::Construction strategy",
                   "[BVH][Dodecahedron]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T>(dataPath("dodecahedron.stl"), pool);

  const FlatMeshSDF<T> flat(mesh, pool);

  for (const auto build : allConstructions) {
    const MeshSDF<T, K> packed(mesh, pool, build);

    for (const auto& p : queryPoints<T>()) {
      REQUIRE_THAT(packed.signedDistance(p), withinAbsT(flat.signedDistance(p), traversalMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE(
  "TriMeshSDF: signedDistance agrees with FlatMeshSDF and MeshSDF for every BVH::Construction strategy",
  "[BVH][Dodecahedron][TriMesh]",
  EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T>(dataPath("dodecahedron.ply"), pool);

  const FlatMeshSDF<T> flat(mesh, pool);
  const MeshSDF<T, K>  packed(mesh, pool, BVH::Construction::SAH);

  for (const auto build : allConstructions) {
    const TriMeshSDF<T, K, W> tri(mesh, pool, build, 2);

    for (const auto& p : queryPoints<T>()) {
      REQUIRE_THAT(tri.signedDistance(p), withinAbsT(flat.signedDistance(p), traversalMargin<T>()));
      REQUIRE_THAT(tri.signedDistance(p), withinAbsT(packed.signedDistance(p), traversalMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("Mesh SDFs: a zero-area sliver face leaves the signed distance unchanged",
                   "[BVH][Tetrahedron][Degenerate]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  // tetrahedron_sliver.stl is tetrahedron.stl with its bottom face split at the hypotenuse midpoint
  // and the resulting T-junction closed by a zero-area triangle lying along the hypotenuse -- the
  // kind of filler CAD exporters write. The sliver has no normal of its own, and it sits exactly on
  // the sharp edge between the bottom and the slanted face, so every pseudonormal near that edge
  // depends on how it is handled.
  Pool       pool(hostMemoryResource());
  const auto clean  = Parser::readIntoDCEL<T>(dataPath("tetrahedron.stl"), pool);
  const auto sliver = Parser::readIntoDCEL<T>(dataPath("tetrahedron_sliver.stl"), pool);

  for (uint32_t f = 0; f < sliver.numFaces(); f++) {
    const Vec3T<T>& n = sliver.getFace(f).getNormal();

    REQUIRE(std::isfinite(n[0]));
    REQUIRE(std::isfinite(n[1]));
    REQUIRE(std::isfinite(n[2]));
    REQUIRE_THAT(n.length(), withinAbsT(T(1), looseMargin<T>()));
  }

  const FlatMeshSDF<T>      reference(clean, pool);
  const FlatMeshSDF<T>      flat(sliver, pool);
  const MeshSDF<T, K>       packed(sliver, pool, BVH::Construction::SAH);
  const TriMeshSDF<T, K, W> tri(sliver, pool, BVH::Construction::SAH, 2);

  for (const T x : sweepValues<T>(T(-0.5), T(1.0), 16)) {
    for (const T y : sweepValues<T>(T(-0.5), T(1.0), 16)) {
      for (const T z : sweepValues<T>(T(-0.5), T(1.0), 16)) {
        const Vec3T<T> p(x, y, z);
        const T        expected = reference.signedDistance(p);

        INFO("p = " << p);
        REQUIRE_THAT(flat.signedDistance(p), withinAbsT(expected, looseMargin<T>()));
        REQUIRE_THAT(packed.signedDistance(p), withinAbsT(expected, looseMargin<T>()));
        REQUIRE_THAT(tri.signedDistance(p), withinAbsT(expected, looseMargin<T>()));
      }
    }
  }
}

TEMPLATE_TEST_CASE("Mesh SDFs: signs around concave edges and vertices match an independent inside test",
                   "[BVH][Concave]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  // lblock.stl is the L-shaped prism ([0,2]x[0,1] U [0,1]x[0,2]) x [0,1]. Its edge along x = y = 1 is
  // concave, which the convex fixtures never exercise. FlatMeshSDF shares the pseudonormal code with the
  // BVH-accelerated SDFs, so the reference here is analytic: inside is the union of two boxes, and
  // outside, the distance is the smaller of the two boxes' distances.
  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T>(dataPath("lblock.stl"), pool);

  REQUIRE(mesh.numFaces() == 20);

  const BoxSDF<T> boxA(Vec3(T(0), T(0), T(0)), Vec3(T(2), T(1), T(1)));
  const BoxSDF<T> boxB(Vec3(T(0), T(0), T(0)), Vec3(T(1), T(2), T(1)));

  const auto inside = [](const Vec3& p) {
    const bool inZ = p[2] > T(0) && p[2] < T(1);
    const bool inA = p[0] > T(0) && p[0] < T(2) && p[1] > T(0) && p[1] < T(1);
    const bool inB = p[0] > T(0) && p[0] < T(1) && p[1] > T(0) && p[1] < T(2);

    return inZ && (inA || inB);
  };

  const FlatMeshSDF<T>      flat(mesh, pool);
  const MeshSDF<T, K>       packed(mesh, pool, BVH::Construction::SAH);
  const TriMeshSDF<T, K, W> tri(mesh, pool, BVH::Construction::SAH, 2);

  // A grid centered on the concave edge's end point (1, 1, 1), where the concave edge, the concave
  // vertex and the faces around them all compete for the closest feature. The grid spacing avoids
  // landing exactly on a face.
  for (const T x : sweepValues<T>(T(0.3), T(1.7), 14)) {
    for (const T y : sweepValues<T>(T(0.3), T(1.7), 14)) {
      for (const T z : sweepValues<T>(T(0.45), T(1.55), 11)) {
        const Vec3 p(x + T(0.013), y + T(0.007), z + T(0.011));

        const bool in       = inside(p);
        const T    outside  = std::min(boxA.signedDistance(p), boxB.signedDistance(p));
        const T    distance = in ? T(-1) : outside;

        INFO("p = " << p);

        for (const T value : {flat.signedDistance(p), packed.signedDistance(p), tri.signedDistance(p)}) {
          REQUIRE((value < T(0)) == in);

          if (!in) {
            REQUIRE_THAT(value, withinAbsT(distance, looseMargin<T>()));
          }
        }
      }
    }
  }
}

TEMPLATE_TEST_CASE("Mesh SDFs: signs behind a sharp concave edge match an independent inside test",
                   "[BVH][Concave]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec2 = std::array<T, 2>;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  // notch.stl is a prism over the polygon below, z in [0,1]: a box with a narrow V-shaped notch cut
  // into its top side. The notch's walls meet at the tip (1, 0.4) in a concave edge along z. Behind
  // that edge, inside the solid, the edge is the closest feature, and only the sum of both walls'
  // normals gives the right sign there: either wall's normal alone points outward for part of that
  // region, because the notch is much narrower than 90 degrees.
  const std::array<Vec2, 7> polygon{
    {{T(0), T(0)}, {T(2), T(0)}, {T(2), T(1)}, {T(1.2), T(1)}, {T(1), T(0.4)}, {T(0.8), T(1)}, {T(0), T(1)}}};

  const auto inside = [&polygon](const Vec3& p) {
    if (!(p[2] > T(0) && p[2] < T(1))) {
      return false;
    }

    // Crossing-number point-in-polygon test.
    bool in = false;

    for (size_t i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
      const Vec2& a = polygon[i];
      const Vec2& b = polygon[j];

      if ((a[1] > p[1]) != (b[1] > p[1]) && p[0] < (b[0] - a[0]) * (p[1] - a[1]) / (b[1] - a[1]) + a[0]) {
        in = !in;
      }
    }

    return in;
  };

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T>(dataPath("notch.stl"), pool);

  REQUIRE(mesh.numFaces() == 24);

  const FlatMeshSDF<T>      flat(mesh, pool);
  const MeshSDF<T, K>       packed(mesh, pool, BVH::Construction::SAH);
  const TriMeshSDF<T, K, W> tri(mesh, pool, BVH::Construction::SAH, 2);

  // A grid around the notch's tip, including the region just behind it. The offsets keep the points
  // off the faces.
  for (const T x : sweepValues<T>(T(0.6), T(1.4), 17)) {
    for (const T y : sweepValues<T>(T(-0.1), T(1.1), 25)) {
      for (const T z : sweepValues<T>(T(-0.2), T(1.2), 8)) {
        const Vec3 p(x + T(0.0013), y + T(0.0007), z + T(0.011));

        const bool in = inside(p);

        INFO("p = " << p);

        for (const T value : {flat.signedDistance(p), packed.signedDistance(p), tri.signedDistance(p)}) {
          REQUIRE((value < T(0)) == in);
        }
      }
    }
  }
}

TEMPLATE_TEST_CASE("TriMeshSDF: polygon faces are fan-triangulated, not truncated to their first three vertices",
                   "[BVH][Degenerate]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  Pool       pool(hostMemoryResource());
  const auto cube = Parser::readIntoDCEL<T>(dataPath("cube_quads.obj"), pool);

  REQUIRE(cube.numFaces() == 6);

  const BoxSDF<T>           box(Vec3T<T>::zeros(), Vec3T<T>::ones());
  const TriMeshSDF<T, K, W> tri(cube, pool, BVH::Construction::SAH, 2);
  const auto                triangles = Parser::readIntoTriangles<T>(dataPath("cube_quads.obj"));

  REQUIRE(triangles.size() == 12);

  for (const T x : sweepValues<T>(T(-0.5), T(1.5), 12)) {
    for (const T y : sweepValues<T>(T(-0.5), T(1.5), 12)) {
      for (const T z : sweepValues<T>(T(-0.5), T(1.5), 12)) {
        const Vec3T<T> p(x, y, z);

        INFO("p = " << p);
        REQUIRE_THAT(tri.signedDistance(p), withinAbsT(box.signedDistance(p), looseMargin<T>()));
      }
    }
  }
}

TEMPLATE_TEST_CASE("Mesh SDFs: getClosestFace names a closest mesh face and matches signedDistance()",
                   "[BVH][MeshSDF][TriMeshSDF][FlatMeshSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  // The dodecahedron is pre-triangulated; the cube has quad faces, which TriMeshSDF fan-triangulates,
  // so there its triangles must report the id of the quad they were cut from.
  for (const char* file : {"dodecahedron.obj", "cube_quads.obj"}) {
    INFO(file);

    Pool       pool(hostMemoryResource());
    const auto mesh = Parser::readIntoDCEL<T>(dataPath(file), pool);

    const FlatMeshSDF<T> flat(mesh, pool);

    // The reported face must be one of the closest: its own distance equals the smallest over all
    // faces (several faces tie at points nearest a shared edge or vertex, so the id itself is not
    // unique).
    const auto requireClosest =
      [&mesh](const ClosestFace<T>& a_closest, const Vec3T<T>& a_point, const double a_margin) {
        REQUIRE(a_closest.faceId < mesh.numFaces());

        T smallest = std::numeric_limits<T>::max();

        for (uint32_t f = 0; f < mesh.numFaces(); f++) {
          smallest = std::min(smallest, std::abs(mesh.getFace(f).signedDistance(a_point, mesh)));
        }

        const T own = mesh.getFace(a_closest.faceId).signedDistance(a_point, mesh);

        REQUIRE_THAT(std::abs(own), withinAbsT(smallest, a_margin));
        REQUIRE_THAT(std::abs(a_closest.signedDistance), withinAbsT(smallest, a_margin));
      };

    for (const auto& p : queryPoints<T>()) {
      const auto closest = flat.getClosestFace(p);

      REQUIRE(closest.signedDistance == flat.signedDistance(p));
      requireClosest(closest, p, 0.0);
    }

    for (const auto build : allConstructions) {
      INFO("construction " << static_cast<int>(build));

      const MeshSDF<T, K>       meshSDF(mesh, pool, build);
      const TriMeshSDF<T, K, W> triSDF(mesh, pool, build, 2);

      for (const auto& p : queryPoints<T>()) {
        const auto fromMesh = meshSDF.getClosestFace(p);
        const auto fromTri  = triSDF.getClosestFace(p);

        // TriMeshSDF's signedDistance() reduces each leaf group with SIMD, getClosestFace() lane by
        // lane, so the two can differ in the last bits.
        REQUIRE(fromMesh.signedDistance == meshSDF.signedDistance(p));
        REQUIRE_THAT(fromTri.signedDistance, withinAbsT(triSDF.signedDistance(p), traversalMargin<T>()));

        requireClosest(fromMesh, p, traversalMargin<T>());
        requireClosest(fromTri, p, traversalMargin<T>());
      }
    }
  }
}

// The unit cube [0,1]^3's closest point and signed-distance gradient in closed form, the reference for
// closestPoint() and gradient() on cube_quads.obj: outside, the point clamped to the cube; inside, the
// projection onto the nearest face, whose outward normal is the gradient. a_unique is false inside where two
// faces are nearly equally near (the medial surface), where the closest point is not unique.
template <class T>
void
unitCubeClosest(const Vec3T<T>& a_p, Vec3T<T>& a_closest, Vec3T<T>& a_gradient, bool& a_unique)
{
  using Vec3 = Vec3T<T>;

  const Vec3 clamped(std::clamp(a_p[0], T(0), T(1)), std::clamp(a_p[1], T(0), T(1)), std::clamp(a_p[2], T(0), T(1)));

  a_unique = true;

  if (!(clamped == a_p)) {
    a_closest  = clamped;
    a_gradient = (a_p - clamped) / (a_p - clamped).length();

    return;
  }

  // Inside: the nearest of the six faces.
  T   best     = std::numeric_limits<T>::max();
  T   second   = std::numeric_limits<T>::max();
  int bestDir  = 0;
  int bestSide = 0;

  for (int dir = 0; dir < 3; dir++) {
    for (int side = 0; side < 2; side++) {
      const T d = side == 0 ? a_p[dir] : T(1) - a_p[dir];

      if (d < best) {
        second   = best;
        best     = d;
        bestDir  = dir;
        bestSide = side;
      }
      else if (d < second) {
        second = d;
      }
    }
  }

  a_unique            = second - best > T(1e-3);
  a_closest           = a_p;
  a_closest[bestDir]  = T(bestSide);
  a_gradient          = Vec3::zeros();
  a_gradient[bestDir] = bestSide == 0 ? T(-1) : T(1);
}

TEMPLATE_TEST_CASE("Mesh SDFs: closestPoint and gradient match the unit cube's closed form",
                   "[BVH][MeshSDF][TriMeshSDF][FlatMeshSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T>(dataPath("cube_quads.obj"), pool);

  const FlatMeshSDF<T>      flat(mesh, pool);
  const MeshSDF<T, 4>       meshSDF(mesh, pool, BVH::Construction::SAH);
  const TriMeshSDF<T, 4, 4> triSDF(mesh, pool, BVH::Construction::SAH, 2);

  const auto check = [](const auto& a_sdf, const Vec3& a_p) {
    Vec3 closest;
    Vec3 gradient;
    bool unique = false;

    unitCubeClosest(a_p, closest, gradient, unique);

    const ClosestPoint<T> cp = a_sdf.closestPoint(a_p);

    // The face it reports is one the point lies on, its distance is the signed distance (to rounding: the
    // compiler may fuse multiplies and adds differently in the two), and the point is on the surface.
    REQUIRE(cp.faceId < 6U);
    REQUIRE_THAT(cp.signedDistance, withinAbsT(a_sdf.signedDistance(a_p), tightMargin<T>()));
    REQUIRE_THAT((a_p - cp.point).length(), withinAbsT(std::abs(cp.signedDistance), tightMargin<T>()));
    REQUIRE_THAT(std::abs(a_sdf.signedDistance(cp.point)), withinAbsT(T(0), tightMargin<T>()));
    REQUIRE_THAT(cp.normal.length(), withinAbsT(T(1), tightMargin<T>()));

    const Vec3 g = a_sdf.gradient(a_p);

    REQUIRE_THAT(g.length(), withinAbsT(T(1), tightMargin<T>()));

    if (unique) {
      for (size_t dir = 0; dir < 3; dir++) {
        REQUIRE_THAT(cp.point[dir], withinAbsT(closest[dir], tightMargin<T>()));
        REQUIRE_THAT(g[dir], withinAbsT(gradient[dir], tightMargin<T>()));
      }
    }
  };

  for (const auto& p : EBGeometryTestGPU::queryGrid<T>(Vec3(T(-1), T(-1), T(-1)), Vec3(T(2), T(2), T(2)), 9)) {
    INFO("point " << p[0] << " " << p[1] << " " << p[2]);

    check(flat, p);
    check(meshSDF, p);
    check(triSDF, p);
  }
}

TEMPLATE_TEST_CASE("Mesh SDFs: gradient on the surface is the closest feature's pseudonormal, never NaN",
                   "[BVH][MeshSDF][TriMeshSDF][FlatMeshSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T>(dataPath("cube_quads.obj"), pool);

  const FlatMeshSDF<T>      flat(mesh, pool);
  const MeshSDF<T, 4>       meshSDF(mesh, pool, BVH::Construction::SAH);
  const TriMeshSDF<T, 4, 4> triSDF(mesh, pool, BVH::Construction::SAH, 2);

  const T    r = T(1) / std::sqrt(T(2));
  const T    q = T(1) / std::sqrt(T(3));
  const auto e = [](const T a_x, const T a_y, const T a_z) { return Vec3(a_x, a_y, a_z); };

  // On a face (also on a fan diagonal of the TriMeshSDF's triangles), on an edge and at a corner: the face
  // normal, and the angle-weighted pseudonormals, which for a cube's right angles are the normalized sums of
  // the adjacent faces' normals.
  const std::vector<std::pair<Vec3, Vec3>> cases = {
    {e(T(0.3), T(0.6), T(1)), e(T(0), T(0), T(1))},
    {e(T(0.5), T(0.5), T(0)), e(T(0), T(0), T(-1))},
    {e(T(1), T(0.4), T(1)), e(r, T(0), r)},
    {e(T(0), T(0), T(0)), e(-q, -q, -q)},
    {e(T(1), T(1), T(1)), e(q, q, q)},
  };

  const auto check = [&cases, &e](const auto& a_sdf) {
    for (const auto& [p, normal] : cases) {
      INFO("point " << p[0] << " " << p[1] << " " << p[2]);

      const Vec3 g = a_sdf.gradient(p);

      for (size_t dir = 0; dir < 3; dir++) {
        REQUIRE_THAT(g[dir], withinAbsT(normal[dir], tightMargin<T>()));
      }
    }

    // The center is on the medial surface: every face is equally near. The gradient is one face's normal.
    const Vec3 center = a_sdf.gradient(e(T(0.5), T(0.5), T(0.5)));

    REQUIRE_THAT(center.length(), withinAbsT(T(1), tightMargin<T>()));
    REQUIRE_THAT(std::abs(center[center.maxDir(true)]), withinAbsT(T(1), tightMargin<T>()));
  };

  check(flat);
  check(meshSDF);
  check(triSDF);
}

TEMPLATE_TEST_CASE("Mesh SDFs: gradient matches finite differences away from the medial surface",
                   "[BVH][MeshSDF][TriMeshSDF][FlatMeshSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T>(dataPath("dodecahedron.obj"), pool);

  const FlatMeshSDF<T>      flat(mesh, pool);
  const MeshSDF<T, 4>       meshSDF(mesh, pool, BVH::Construction::SAH);
  const TriMeshSDF<T, 4, 4> triSDF(mesh, pool, BVH::Construction::SAH, 2);

  // A central difference of step h is accurate to O(h^2) where the distance is smooth; a point within a
  // few steps of the medial surface, where the closest face changes, is skipped (its two one-sided
  // differences disagree).
  const T      h      = std::is_same_v<T, float> ? T(1e-2) : T(1e-4);
  const double margin = std::is_same_v<T, float> ? 2e-2 : 1e-5;

  const auto check = [&](const auto& a_sdf) {
    int compared = 0;

    for (const auto& p : EBGeometryTestGPU::queryGrid<T>(Vec3(T(-3), T(-3), T(-3)), Vec3(T(3), T(3), T(3)), 8)) {
      const uint32_t face = a_sdf.closestPoint(p).faceId;

      bool nearMedial = false;

      for (size_t dir = 0; dir < 3 && !nearMedial; dir++) {
        for (const T sign : {T(-2), T(2)}) {
          Vec3 shifted = p;

          shifted[dir] += sign * h;

          nearMedial = nearMedial || flat.closestPoint(shifted).faceId != face;
        }
      }

      if (nearMedial) {
        continue;
      }

      const Vec3 g  = a_sdf.gradient(p);
      const Vec3 fd = normal(a_sdf, p, h);

      for (size_t dir = 0; dir < 3; dir++) {
        REQUIRE_THAT(g[dir], withinAbsT(fd[dir], margin));
      }

      compared++;
    }

    REQUIRE(compared > 100);
  };

  check(flat);
  check(meshSDF);
  check(triSDF);
}

TEMPLATE_TEST_CASE("MeshSDF reads its faces from the mesh: flipping or moving the mesh after the build is seen",
                   "[BVH][MeshSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;

  constexpr size_t K = 4;

  Pool pool(hostMemoryResource());
  auto mesh = Parser::readIntoDCEL<T>(dataPath("dodecahedron.obj"), pool);

  MeshSDF<T, K> sdf(mesh, pool, BVH::Construction::SAH);

  std::vector<T> before;

  for (const auto& p : queryPoints<T>()) {
    before.push_back(sdf.signedDistance(p));
  }

  SECTION("flip")
  {
    // The BVH holds face ids, not copies of the faces, so the flipped normals are what it reads.
    mesh.flip();

    size_t i = 0;

    for (const auto& p : queryPoints<T>()) {
      REQUIRE(sdf.signedDistance(p) == -before[i]);
      REQUIRE(sdf.getClosestFace(p).signedDistance == -before[i]);

      i++;
    }
  }

  SECTION("move the vertices, reconcile and refit")
  {
    // Scale the mesh by two about the origin, which moves every face and changes every normal's
    // cached derived data (centroid, area).
    for (uint32_t v = 0; v < mesh.numVertices(); v++) {
      mesh.getVertex(v).setPosition(T(2) * mesh.getVertex(v).getPosition());
    }

    mesh.reconcile();

    sdf.getRoot().refit([&mesh](uint32_t a_face) { return AABB(mesh.getFace(a_face).getAllVertexCoordinates(mesh)); });

    const FlatMeshSDF<T> reference(mesh, pool);

    for (const auto& p : queryPoints<T>()) {
      REQUIRE_THAT(sdf.signedDistance(p), withinAbsT(reference.signedDistance(p), traversalMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("TriMeshSDF::getClosestFace reports the closest triangle's face id and a "
                   "distance matching signedDistance()",
                   "[BVH][TriMesh]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  Pool pool(hostMemoryResource());

  // A soup of well-separated triangles (3 apart along x), each tagged with a distinct face id so the
  // closest-triangle query's returned id is unambiguous. Enough triangles to force
  // several BVH leaves/levels.
  constexpr int N = 12;

  std::vector<Triangle<T>> tris;
  for (int i = 0; i < N; i++) {
    const Vec3 base(T(3 * i), T(0), T(0));

    Triangle<T> tri;
    tri.setVertexPositions({base + Vec3(0, 0, 0), base + Vec3(1, 0, 0), base + Vec3(0, 1, 0)});
    tri.setNormal(Vec3(0, 0, 1));
    tri.setVertexNormals({Vec3(0, 0, 1), Vec3(0, 0, 1), Vec3(0, 0, 1)});
    tri.setEdgeNormals({Vec3(0, 0, 1), Vec3(0, 0, 1), Vec3(0, 0, 1)});
    tri.setFaceId(static_cast<uint32_t>(100 + i));

    tris.emplace_back(tri);
  }

  for (const auto build : allConstructions) {
    const TriMeshSDF<T, K, W> tri(tris, pool, build, 2);

    for (int i = 0; i < N; i++) {
      const Vec3 q(T(3 * i) + T(0.25), T(0.25), T(0.2)); // unambiguously nearest to triangle i

      const auto closest = tri.getClosestFace(q);

      REQUIRE(closest.faceId == static_cast<uint32_t>(100 + i));
      REQUIRE_THAT(closest.signedDistance, withinAbsT(tri.signedDistance(q), traversalMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("TriMeshSDF: every BVH::Construction value finds the nearest triangle and keeps each leaf "
                   "within maxLeafGroups",
                   "[BVH][TriMesh][Construction]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  std::mt19937 rng(17);

  const auto tris    = jitteredTriangleSoup<T>(rng);
  const auto queries = jitteredSoupQueries<T>(rng);

  Pool pool(hostMemoryResource());

  // The space-filling curves honour maxLeafGroups too: on 512 triangles their target, maxLeafGroups
  // * W, is always within reach of a power-of-K leaf count.
  for (const size_t maxLeafGroups : {size_t(1), size_t(2), size_t(3)}) {
    for (const auto build : allConstructions) {
      const TriMeshSDF<T, K, W> sdf(tris, pool, build, maxLeafGroups);

      REQUIRE(largestLeaf(sdf.getRoot()) <= maxLeafGroups);

      for (const auto& q : queries) {
        REQUIRE_THAT(std::abs(sdf.signedDistance(q)), withinAbsT(bruteNearestTriangle(tris, q), traversalMargin<T>()));
      }
    }
  }
}

TEMPLATE_TEST_CASE("TriMeshSDF: the signed distance is that of the nearest triangle, signed as every group "
                   "signs it",
                   "[BVH][TriMesh]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  std::mt19937 rng(23);

  const auto tris    = jitteredTriangleSoup<T>(rng);
  const auto queries = jitteredSoupQueries<T>(rng);

  Pool pool(hostMemoryResource());

  const TriMeshSDF<T, K, W> sdf(tris, pool, BVH::Construction::SAH, 2);

  // The reference: every group's signedDistance() (each lane signed), the minimum |value| over all of
  // them. The search compares squared unsigned distances and signs only the winning lane, so it must give
  // the same value, sign included.
  const auto groups = sdf.getRoot().getPrimitives();

  for (const auto& q : queries) {
    T expected = std::numeric_limits<T>::max();

    for (uint32_t g = 0; g < groups.size(); g++) {
      const T d = groups[g].signedDistance(q);

      if (std::abs(d) < std::abs(expected)) {
        expected = d;
      }
    }

    REQUIRE_THAT(sdf.signedDistance(q), withinAbsT(expected, traversalMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("TriMeshSDF: ConstructionOptions set each construction method's leaf size",
                   "[BVH][TriMesh][Construction]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  using SDF = TriMeshSDF<T, K, W>;

  std::mt19937 rng(23);

  const auto tris    = jitteredTriangleSoup<T>(rng);
  const auto queries = jitteredSoupQueries<T>(rng);
  const auto groups  = [](const size_t a_triangles) { return (a_triangles + W - 1) / W; };

  Pool pool(hostMemoryResource());

  // Each method reads only its own field, so the other two are left at 0 to show they are unread.
  // The settings are counted in triangles; a leaf of n triangles packs into ceil(n / W) groups.
  for (const size_t setting : {size_t(1), size_t(5), size_t(16), size_t(33), tris.size()}) {
    for (const auto build : allConstructions) {
      BVH::ConstructionOptions options;

      if (isTopDown(build)) {
        options.maxLeafSize = setting;
      }
      else if (isSpaceFillingCurve(build)) {
        options.targetLeafSize = setting;
      }
      else {
        options.cluster.maxClusterSize = setting;
      }

      const SDF sdf(tris, pool, build, options);

      INFO("setting = " << setting);

      if (isTopDown(build)) {
        // A split makes K non-empty children, so a node of fewer than K triangles is a leaf anyway.
        REQUIRE(largestLeaf(sdf.getRoot()) <= groups(std::max(setting, K - 1)));
      }
      else if (isSpaceFillingCurve(build) && setting >= K) {
        REQUIRE(largestLeaf(sdf.getRoot()) <= groups(setting));
      }
      else if (isSpaceFillingCurve(build)) {
        // Below K, a power-of-K leaf count can fall short of the target; the leaves hold at most K.
        REQUIRE(largestLeaf(sdf.getRoot()) <= groups(K));
      }
      else {
        REQUIRE(largestLeaf(sdf.getRoot()) <= groups((K - 1) * setting));
      }

      // A bound or target of every triangle makes the root a leaf.
      if ((isTopDown(build) || isSpaceFillingCurve(build)) && setting == tris.size()) {
        REQUIRE(sdf.getRoot().getNodes().size() == 1);
      }

      for (const auto& q : queries) {
        REQUIRE_THAT(std::abs(sdf.signedDistance(q)), withinAbsT(bruteNearestTriangle(tris, q), traversalMargin<T>()));
      }
    }
  }

  // Both constructors taking a_maxLeafGroups build the trees of defaultConstructionOptions().
  for (const auto build : allConstructions) {
    for (const size_t maxLeafGroups : {size_t(1), size_t(4)}) {
      const SDF implicit(tris, pool, build, maxLeafGroups);
      const SDF explicitOptions(tris, pool, build, SDF::defaultConstructionOptions(maxLeafGroups));

      REQUIRE(sameNodes(implicit.getRoot(), explicitOptions.getRoot()));
    }
  }
}

TEMPLATE_TEST_CASE("MeshSDF: ConstructionOptions set each construction method's leaf size",
                   "[BVH][Construction]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;

  using SDF = MeshSDF<T, K>;

  Pool       pool(hostMemoryResource());
  const auto mesh     = Parser::readIntoDCEL<T>(dataPath("dodecahedron.stl"), pool);
  const auto numFaces = size_t(mesh.numFaces());

  const FlatMeshSDF<T> flat(mesh, pool);

  for (const size_t setting : {size_t(1), size_t(2), size_t(5), size_t(9), numFaces}) {
    for (const auto build : allConstructions) {
      BVH::ConstructionOptions options;

      if (isTopDown(build)) {
        options.maxLeafSize = setting;
      }
      else if (isSpaceFillingCurve(build)) {
        options.targetLeafSize = setting;
      }
      else {
        options.cluster.maxClusterSize = setting;
      }

      const SDF sdf(mesh, pool, build, options);

      INFO("setting = " << setting);

      if (isTopDown(build)) {
        REQUIRE(largestLeaf(sdf.getRoot()) <= std::max(setting, K - 1));
      }
      else if (isSpaceFillingCurve(build) && setting >= K) {
        REQUIRE(largestLeaf(sdf.getRoot()) <= setting);
      }
      else if (isSpaceFillingCurve(build)) {
        REQUIRE(largestLeaf(sdf.getRoot()) <= K);
      }
      else {
        REQUIRE(largestLeaf(sdf.getRoot()) <= (K - 1) * setting);
      }

      if ((isTopDown(build) || isSpaceFillingCurve(build)) && setting == numFaces) {
        REQUIRE(sdf.getRoot().getNodes().size() == 1);
      }

      for (const auto& p : queryPoints<T>()) {
        REQUIRE_THAT(sdf.signedDistance(p), withinAbsT(flat.signedDistance(p), traversalMargin<T>()));
      }
    }
  }

  // The constructor without options builds the trees of defaultConstructionOptions().
  for (const auto build : allConstructions) {
    const SDF implicit(mesh, pool, build);
    const SDF explicitOptions(mesh, pool, build, SDF::defaultConstructionOptions());

    REQUIRE(sameNodes(implicit.getRoot(), explicitOptions.getRoot()));
  }
}

TEMPLATE_TEST_CASE("Mesh SDFs: a value outside BVH::Construction aborts",
                   "[BVH][Construction]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  // An EBGEOMETRY_REQUIRE, so it aborts in every build rather than packing an unpartitioned tree.
  REQUIRE(abortsWith(
    [] {
      Pool                pool(hostMemoryResource());
      const auto          mesh = Parser::readIntoDCEL<T>(dataPath("dodecahedron.stl"), pool);
      const MeshSDF<T, K> sdf(mesh, pool, invalidConstruction);

      (void)sdf;
    },
    "MeshSDF: unknown BVH::Construction value (99)"));

  REQUIRE(abortsWith(
    [] {
      Pool                      pool(hostMemoryResource());
      const auto                mesh = Parser::readIntoDCEL<T>(dataPath("dodecahedron.stl"), pool);
      const TriMeshSDF<T, K, W> sdf(mesh, pool, invalidConstruction, 2);

      (void)sdf;
    },
    "TriMeshSDF: unknown BVH::Construction value (99)"));
}

TEMPLATE_TEST_CASE("Mesh SDFs: a zero leaf-size setting aborts when the construction method reads it",
                   "[BVH][Construction]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  // EBGEOMETRY_REQUIREs, so they abort in every build.
  REQUIRE(abortsWith(
    [] {
      Pool                pool(hostMemoryResource());
      const auto          mesh = Parser::readIntoDCEL<T>(dataPath("dodecahedron.stl"), pool);
      const MeshSDF<T, K> sdf(mesh, pool, BVH::Construction::SAH, BVH::ConstructionOptions{0, 1, {}});

      (void)sdf;
    },
    "MeshSDF: ConstructionOptions::maxLeafSize must be positive"));

  REQUIRE(abortsWith(
    [] {
      Pool                      pool(hostMemoryResource());
      const auto                mesh = Parser::readIntoDCEL<T>(dataPath("dodecahedron.stl"), pool);
      const TriMeshSDF<T, K, W> sdf(mesh, pool, BVH::Construction::Hilbert, BVH::ConstructionOptions{1, 0, {}});

      (void)sdf;
    },
    "TriMeshSDF: ConstructionOptions::targetLeafSize must be positive"));

  REQUIRE(abortsWith(
    [] {
      Pool                pool(hostMemoryResource());
      const auto          mesh = Parser::readIntoDCEL<T>(dataPath("dodecahedron.stl"), pool);
      const MeshSDF<T, K> sdf(mesh, pool, BVH::Construction::ClusterSAH, BVH::ConstructionOptions{1, 1, {0}});

      (void)sdf;
    },
    "ClusterSpec::maxClusterSize must be positive"));

  REQUIRE(abortsWith(
    [] {
      Pool                      pool(hostMemoryResource());
      const auto                mesh = Parser::readIntoDCEL<T>(dataPath("dodecahedron.stl"), pool);
      const TriMeshSDF<T, K, W> sdf(mesh, pool, BVH::Construction::SAH, 0);

      (void)sdf;
    },
    "TriMeshSDF: the maximum number of leaf groups must be positive (0)"));
}

TEMPLATE_TEST_CASE("Parser::readIntoMeshSDF matches MeshSDF built directly from the same mesh",
                   "[BVH][Parser]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;

  // One pool for both: the direct MeshSDF below and readIntoMeshSDF's own independent build
  // share it, which is only sound because building one mesh no longer closes the pool to the next.
  Pool       pool(hostMemoryResource());
  const auto direct = Parser::readIntoDCEL<T>(dataPath("dodecahedron.stl"), pool);

  const MeshSDF<T, K> expected(direct, pool, BVH::Construction::SAH);
  const auto          fromFile = Parser::readIntoMeshSDF<T, K>(dataPath("dodecahedron.stl"), pool);

  for (const auto& p : queryPoints<T>()) {
    REQUIRE_THAT(fromFile.signedDistance(p), withinAbsT(expected.signedDistance(p), formatMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("Parser: multi-file overloads return one result per file, each matching the "
                   "corresponding single-file call",
                   "[BVH][Parser]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;

  // Two different formats of the *same* underlying geometry -- the point isn't that the two
  // files describe different scenes, only that the multi-file overload is a faithful per-file
  // loop, not some kind of multi-mesh merge.
  const std::vector<std::string> files = {dataPath("dodecahedron.stl"), dataPath("dodecahedron.ply")};

  Pool pool(hostMemoryResource());

  SECTION("readIntoDCEL")
  {
    const auto meshes = Parser::readIntoDCEL<T>(files, pool);
    REQUIRE(meshes.size() == 2);

    std::vector<DCEL::MeshT<T>> singles;
    singles.reserve(files.size());
    for (const auto& file : files) {
      singles.push_back(Parser::readIntoDCEL<T>(file, pool));
    }

    for (size_t i = 0; i < files.size(); i++) {
      const auto& single = singles[i];
      REQUIRE(meshes[i].numVertices() == single.numVertices());
      REQUIRE(meshes[i].numFaces() == single.numFaces());
      for (const auto& p : queryPoints<T>()) {
        REQUIRE_THAT(meshes[i].signedDistance(p), withinAbsT(single.signedDistance(p), formatMargin<T>()));
      }
    }
  }

  SECTION("readIntoFlatMeshSDF")
  {
    const auto flatSDFs = Parser::readIntoFlatMeshSDF<T>(files, pool);
    REQUIRE(flatSDFs.size() == 2);

    for (size_t i = 0; i < files.size(); i++) {
      const auto single = Parser::readIntoFlatMeshSDF<T>(files[i], pool);
      for (const auto& p : queryPoints<T>()) {
        REQUIRE_THAT(flatSDFs[i].signedDistance(p), withinAbsT(single.signedDistance(p), formatMargin<T>()));
      }
    }
  }

  SECTION("readIntoMeshSDF")
  {
    const auto packedSDFs = Parser::readIntoMeshSDF<T, K>(files, pool);
    REQUIRE(packedSDFs.size() == 2);

    for (size_t i = 0; i < files.size(); i++) {
      const auto single = Parser::readIntoMeshSDF<T, K>(files[i], pool);
      for (const auto& p : queryPoints<T>()) {
        REQUIRE_THAT(packedSDFs[i].signedDistance(p), withinAbsT(single.signedDistance(p), formatMargin<T>()));
      }
    }
  }

  SECTION("readIntoTriMeshSDF")
  {
    const auto triSDFs = Parser::readIntoTriMeshSDF<T>(files, pool);
    REQUIRE(triSDFs.size() == 2);

    for (size_t i = 0; i < files.size(); i++) {
      const auto single = Parser::readIntoTriMeshSDF<T>(files[i], pool);
      for (const auto& p : queryPoints<T>()) {
        REQUIRE_THAT(triSDFs[i].signedDistance(p), withinAbsT(single.signedDistance(p), formatMargin<T>()));
      }
    }
  }
}

TEMPLATE_TEST_CASE("MeshSDF/TriMeshSDF: both pack their primitives by value",
                   "[BVH][PrimitiveStorage]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  using TriAoSoA = TriangleAoSoA<T, W>;

  // MeshSDF's BVH stores face ids, and every query reads the face from the mesh MeshSDF holds.
  // TriMeshSDF's SoA groups are self-contained (freshly built by packing, shared with nothing), so
  // they need no such companion. See ImplemBVH.rst's "Primitive storage" section.
  static_assert(std::is_same_v<typename MeshSDF<T, K>::Root, BVH::PackedBVH<T, uint32_t, K>>);
  static_assert(std::is_same_v<typename TriMeshSDF<T, K, W>::Root, BVH::PackedBVH<T, TriAoSoA, K>>);
}

TEMPLATE_TEST_CASE("FlatMeshSDF/MeshSDF/TriMeshSDF: move constructor/assignment are usable (no "
                   "longer implicitly suppressed by the user-declared destructor)",
                   "[BVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  static_assert(std::is_copy_constructible_v<FlatMeshSDF<T>>);
  static_assert(std::is_copy_assignable_v<FlatMeshSDF<T>>);
  static_assert(std::is_move_constructible_v<FlatMeshSDF<T>>);
  static_assert(std::is_move_assignable_v<FlatMeshSDF<T>>);

  static_assert(std::is_copy_constructible_v<MeshSDF<T, K>>);
  static_assert(std::is_copy_assignable_v<MeshSDF<T, K>>);
  static_assert(std::is_move_constructible_v<MeshSDF<T, K>>);
  static_assert(std::is_move_assignable_v<MeshSDF<T, K>>);

  static_assert(std::is_copy_constructible_v<TriMeshSDF<T, K, W>>);
  static_assert(std::is_copy_assignable_v<TriMeshSDF<T, K, W>>);
  static_assert(std::is_move_constructible_v<TriMeshSDF<T, K, W>>);
  static_assert(std::is_move_assignable_v<TriMeshSDF<T, K, W>>);
}

TEMPLATE_TEST_CASE("Parser::readIntoTriangles and TriMeshSDF's mesh constructor extract identical triangles",
                   "[BVH][TriMeshSDF][Parser]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  Pool       pool(hostMemoryResource());
  const auto mesh      = Parser::readIntoDCEL<T>(dataPath("dodecahedron.obj"), pool);
  const auto triangles = Parser::readIntoTriangles<T>(dataPath("dodecahedron.obj"));

  REQUIRE(triangles.size() == mesh.numFaces());

  // Each triangle carries its face's id and its three half-edges' normals -- not the vertex normals,
  // which readIntoTriangles used to substitute for them.
  for (uint32_t i = 0; i < mesh.numFaces(); i++) {
    const auto& face        = mesh.getFace(i);
    const auto  edgeIndices = face.gatherEdgeIndices(mesh);

    REQUIRE(edgeIndices.size() == 3);
    REQUIRE(triangles[i].getFaceId() == i);

    for (size_t e = 0; e < 3; e++) {
      REQUIRE(triangles[i].getEdgeNormals()[e] == mesh.getEdge(edgeIndices[e]).getNormal());
    }
  }

  // Built from identical triangles with the same strategy, the two constructors give identical trees.
  const TriMeshSDF<T, K, W> fromMesh(mesh, pool, BVH::Construction::SAH, 2);
  const TriMeshSDF<T, K, W> fromSoup(triangles, pool, BVH::Construction::SAH, 2);

  for (const auto& p : queryPoints<T>()) {
    REQUIRE(fromSoup.signedDistance(p) == fromMesh.signedDistance(p));
    REQUIRE(fromSoup.getClosestFace(p).faceId == fromMesh.getClosestFace(p).faceId);
  }
}

TEMPLATE_TEST_CASE("FlatMeshSDF::computeBoundingVolume is the vertex AABB, as MeshSDF's root box is",
                   "[BVH][FlatMeshSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;

  constexpr size_t K = 4;

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T>(dataPath("dodecahedron.obj"), pool);

  const FlatMeshSDF<T> flat(mesh, pool);
  const MeshSDF<T, K>  meshSDF(mesh, pool, BVH::Construction::SAH);

  const AABB fromFlat     = flat.computeBoundingVolume();
  const AABB fromVertices = AABB(mesh.getAllVertexCoordinates());
  const AABB fromBVH      = meshSDF.computeBoundingVolume();

  REQUIRE(fromFlat.getLowCorner() == fromVertices.getLowCorner());
  REQUIRE(fromFlat.getHighCorner() == fromVertices.getHighCorner());
  REQUIRE(fromFlat.getLowCorner() == fromBVH.getLowCorner());
  REQUIRE(fromFlat.getHighCorner() == fromBVH.getHighCorner());
}

TEMPLATE_TEST_CASE("FlatMeshSDF/MeshSDF/TriMeshSDF: rebasedView and deepCopy answer exactly as the original does",
                   "[BVH][FlatMeshSDF][MeshSDF][TriMeshSDF][rebase]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  using Flat = FlatMeshSDF<T>;
  using Mesh = MeshSDF<T, K>;
  using Tri  = TriMeshSDF<T, K, W>;

  // Each class is itself what crosses to a device, so it must stay trivially copyable, and both
  // crossings must return the class itself rather than some narrower type.
  static_assert(std::is_trivially_copyable_v<Flat>);
  static_assert(std::is_trivially_copyable_v<Mesh>);
  static_assert(std::is_trivially_copyable_v<Tri>);
  static_assert(std::is_same_v<decltype(std::declval<const Flat&>().rebasedView(std::declval<const Pool&>())), Flat>);
  static_assert(std::is_same_v<decltype(std::declval<const Mesh&>().rebasedView(std::declval<const Pool&>())), Mesh>);
  static_assert(std::is_same_v<decltype(std::declval<const Tri&>().rebasedView(std::declval<const Pool&>())), Tri>);
  static_assert(std::is_same_v<decltype(std::declval<const Flat&>().deepCopy(std::declval<Pool&>())), Flat>);
  static_assert(std::is_same_v<decltype(std::declval<const Mesh&>().deepCopy(std::declval<Pool&>())), Mesh>);
  static_assert(std::is_same_v<decltype(std::declval<const Tri&>().deepCopy(std::declval<Pool&>())), Tri>);

  // The dodecahedron fixture is pre-triangulated, so TriMeshSDF accepts it too.
  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T>(dataPath("dodecahedron.obj"), pool);

  const Flat flat(mesh, pool);
  const Mesh meshSDF(mesh, pool, BVH::Construction::SAH);
  const Tri  triSDF(mesh, pool, BVH::Construction::SAH, 2);

  REQUIRE(flat.isAttachedTo(pool));
  REQUIRE(meshSDF.isAttachedTo(pool));
  REQUIRE(triSDF.isAttachedTo(pool));

  for (const auto& p : queryPoints<T>()) {
    REQUIRE(flat.signedDistance(p) == mesh.signedDistance(p));
  }

  // Same tree, same data: every answer must match the original's exactly, including the face each
  // getClosestFace() reports.
  const auto requireSame = [&flat, &meshSDF, &triSDF](const Flat& a_flat, const Mesh& a_mesh, const Tri& a_tri) {
    for (const auto& p : queryPoints<T>()) {
      REQUIRE(a_flat.signedDistance(p) == flat.signedDistance(p));
      REQUIRE(a_mesh.signedDistance(p) == meshSDF.signedDistance(p));
      REQUIRE(a_tri.signedDistance(p) == triSDF.signedDistance(p));

      REQUIRE(a_flat.getClosestFace(p).faceId == flat.getClosestFace(p).faceId);
      REQUIRE(a_mesh.getClosestFace(p).faceId == meshSDF.getClosestFace(p).faceId);

      const auto closest  = a_tri.getClosestFace(p);
      const auto expected = triSDF.getClosestFace(p);

      REQUIRE(closest.signedDistance == expected.signedDistance);
      REQUIRE(closest.faceId == expected.faceId);
    }
  };

  SECTION("deepCopy into a separate pool is independent storage")
  {
    Pool other(hostMemoryResource());

    const Flat flatCopy = flat.deepCopy(other);
    const Mesh meshCopy = meshSDF.deepCopy(other);
    const Tri  triCopy  = triSDF.deepCopy(other);

    REQUIRE(flatCopy.isAttachedTo(other));
    REQUIRE(meshCopy.isAttachedTo(other));
    REQUIRE(triCopy.isAttachedTo(other));
    REQUIRE_FALSE(flatCopy.isAttachedTo(pool));
    REQUIRE_FALSE(meshCopy.isAttachedTo(pool));
    REQUIRE_FALSE(triCopy.isAttachedTo(pool));

    requireSame(flatCopy, meshCopy, triCopy);
  }

  SECTION("deepCopy into its own pool survives the pool growing under it")
  {
    const Flat flatCopy = flat.deepCopy(pool);
    const Mesh meshCopy = meshSDF.deepCopy(pool);
    const Tri  triCopy  = triSDF.deepCopy(pool);

    REQUIRE(meshCopy.isAttachedTo(pool));

    requireSame(flatCopy, meshCopy, triCopy);
  }

  SECTION("a host-to-host rebasedView resolves against the mirror")
  {
    // The host-side analogue of the device crossing: same offsets, a different block, identical
    // answers.
    pool.freeze();

    Pool mirror = Pool::mirror(pool, hostMemoryResource());

    const Flat flatView = flat.rebasedView(mirror);
    const Mesh meshView = meshSDF.rebasedView(mirror);
    const Tri  triView  = triSDF.rebasedView(mirror);

    REQUIRE(flatView.isAttachedTo(mirror));
    REQUIRE(meshView.isAttachedTo(mirror));
    REQUIRE(triView.isAttachedTo(mirror));
    REQUIRE_FALSE(meshView.isAttachedTo(pool));

    requireSame(flatView, meshView, triView);
  }
}

TEST_CASE("FlatMeshSDF/MeshSDF/TriMeshSDF: the constructors reject a mismatched pool or a zero leaf size",
          "[BVH][FlatMeshSDF][MeshSDF][TriMeshSDF][death]")
{
  using T = double;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  // EBGEOMETRY_REQUIREs, so they abort in every build.
  REQUIRE(abortsWith(
    [] {
      Pool       pool(hostMemoryResource());
      Pool       other(hostMemoryResource());
      const auto mesh = Parser::readIntoDCEL<T>(dataPath("dodecahedron.obj"), pool);

      const FlatMeshSDF<T> sdf(mesh, other);
    },
    "FlatMeshSDF: the mesh must live in the pool passed in"));

  REQUIRE(abortsWith(
    [] {
      Pool       pool(hostMemoryResource());
      Pool       other(hostMemoryResource());
      const auto mesh = Parser::readIntoDCEL<T>(dataPath("dodecahedron.obj"), pool);

      const MeshSDF<T, K> sdf(mesh, other, BVH::Construction::SAH);
    },
    "MeshSDF: the mesh must live in the pool passed in"));

  REQUIRE(abortsWith(
    [] {
      Pool       pool(hostMemoryResource());
      const auto mesh = Parser::readIntoDCEL<T>(dataPath("dodecahedron.obj"), pool);

      const TriMeshSDF<T, K, W> sdf(mesh, pool, BVH::Construction::SAH, 0);
    },
    "TriMeshSDF: the maximum number of leaf groups must be positive (0)"));
}

TEMPLATE_TEST_CASE("Nested BVH: a BVHUnion over several TriMeshSDF objects nests each mesh's inner "
                   "PackedBVH inside the outer union PackedBVH and matches a brute-force min",
                   "[BVH][CSG][BVHUnion][Nested]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using BV   = BoundingVolumes::AABBT<T>;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  using Tri = TriMeshSDF<T, K, W>;

  // Two distinct triangle meshes read from the in-repo fixtures. Each TriMeshSDF owns an inner
  // PackedBVH over SoA triangle groups -- these are the inner BVHs that the outer union BVH nests
  // over. Both are the same C++ type, which is all a BVHUnion needs.
  Pool       pool(hostMemoryResource());
  const auto dodec = Parser::readIntoTriangles<T>(dataPath("dodecahedron.stl"));
  const auto tetra = Parser::readIntoTriangles<T>(dataPath("tetrahedron.stl"));

  // Spread several translated mesh SDFs out so the outer union BVH has real structure to partition
  // and prune, rather than collapsing to a single leaf. Each copy is translated before its
  // TriMeshSDF is built.
  const std::vector<std::pair<std::vector<Triangle<T>>, Vec3>> placements = {
    {dodec, Vec3(0, 0, 0)},
    {dodec, Vec3(4, 0, 0)},
    {tetra, Vec3(0, 4, 0)},
    {tetra, Vec3(-4, -4, 2)},
  };

  std::vector<Tri> primitives;
  std::vector<BV>  boundingVolumes;

  for (const auto& [triangles, shift] : placements) {
    auto shifted = triangles;

    for (auto& triangle : shifted) {
      auto vertices = triangle.getVertexPositions();

      for (auto& v : vertices) {
        v = v + shift;
      }

      triangle.setVertexPositions(vertices);
    }

    primitives.emplace_back(shifted, pool, BVH::Construction::SAH, 2);
    boundingVolumes.push_back(primitives.back().computeBoundingVolume());
  }

  // Outer BVH: a PackedBVH over the TriMeshSDFs by value, each owning an inner PackedBVH in the same
  // pool -- a genuine two-level BVH hierarchy.
  const auto nestedUnion = BVHUnion<T, K, Tri>(pool, primitives, boundingVolumes);

  // The union value is the minimum over all primitives; BVH pruning can never discard the actual
  // nearest one, so the nested traversal must agree exactly with a brute-force min over the same
  // translated primitives.
  for (const auto& p : queryPoints<T>()) {
    T bruteMin = std::numeric_limits<T>::infinity();

    for (const auto& prim : primitives) {
      bruteMin = std::min(bruteMin, prim.signedDistance(p));
    }

    REQUIRE_THAT(nestedUnion.signedDistance(p), withinAbsT(bruteMin, traversalMargin<T>()));
  }
}
