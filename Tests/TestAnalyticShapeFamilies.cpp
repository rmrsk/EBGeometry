// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// The shape families: ConvexPolyhedronSDF (with the regular solids and the pyramid), BoxFrameSDF, QuadricSDF,
// SuperellipsoidSDF, SolidAngleSDF, VesicaSDF, HelixSDF and LatticeSDF. Each is checked against an independent
// reference: a hull of hand-written vertices, a union of boxes, a finely sampled profile or center line, or a closed
// form. The device checks are in TestAnalyticShapeFamiliesDevice.cpp.

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestExactSDF.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestUtilities.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <random>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

namespace {

using V = Vec3T<double>;

// The distance from a point to a triangle (Ericson, Real-Time Collision Detection, 5.1.5).
double
triangleDistance(const V& a_p, const V& a_a, const V& a_b, const V& a_c)
{
  const V ab = a_b - a_a;
  const V ac = a_c - a_a;
  const V ap = a_p - a_a;

  const double d1 = dot(ab, ap);
  const double d2 = dot(ac, ap);

  if (d1 <= 0 && d2 <= 0) {
    return (a_p - a_a).length();
  }

  const V      bp = a_p - a_b;
  const double d3 = dot(ab, bp);
  const double d4 = dot(ac, bp);

  if (d3 >= 0 && d4 <= d3) {
    return (a_p - a_b).length();
  }

  const double vc = d1 * d4 - d3 * d2;

  if (vc <= 0 && d1 >= 0 && d3 <= 0) {
    return (a_p - (a_a + d1 / (d1 - d3) * ab)).length();
  }

  const V      cp = a_p - a_c;
  const double d5 = dot(ab, cp);
  const double d6 = dot(ac, cp);

  if (d6 >= 0 && d5 <= d6) {
    return (a_p - a_c).length();
  }

  const double vb = d5 * d2 - d1 * d6;

  if (vb <= 0 && d2 >= 0 && d6 <= 0) {
    return (a_p - (a_a + d2 / (d2 - d6) * ac)).length();
  }

  const double va = d3 * d6 - d5 * d4;

  if (va <= 0 && (d4 - d3) >= 0 && (d5 - d6) >= 0) {
    return (a_p - (a_b + (d4 - d3) / ((d4 - d3) + (d5 - d6)) * (a_c - a_b))).length();
  }

  const double denom = 1.0 / (va + vb + vc);

  return (a_p - (a_a + ab * (vb * denom) + ac * (vc * denom))).length();
}

// The convex hull of a set of points, as every triangle of three of them with all the points on one side, and the
// planes of those triangles: the reference for a polyhedron given by its vertices, independent of the shape's code.
struct Hull
{
  std::vector<std::array<V, 3>>     m_triangles;
  std::vector<std::pair<V, double>> m_planes;

  explicit Hull(const std::vector<V>& a_points)
  {
    const size_t n = a_points.size();

    for (size_t i = 0; i < n; i++) {
      for (size_t j = i + 1; j < n; j++) {
        for (size_t k = j + 1; k < n; k++) {
          V normal = cross(a_points[j] - a_points[i], a_points[k] - a_points[i]);

          if (normal.length() < 1e-9) {
            continue;
          }

          normal = normal / normal.length();

          double offset = dot(normal, a_points[i]);
          bool   above  = false;
          bool   below  = false;

          for (const V& p : a_points) {
            above = above || dot(normal, p) > offset + 1e-9;
            below = below || dot(normal, p) < offset - 1e-9;
          }

          if (above && below) {
            continue;
          }
          if (above) {
            normal = -normal;
            offset = -offset;
          }

          m_triangles.push_back({a_points[i], a_points[j], a_points[k]});
          m_planes.emplace_back(normal, offset);
        }
      }
    }
  }

  double
  signedDistance(const V& a_p) const
  {
    double distance = 1e300;

    for (const auto& t : m_triangles) {
      distance = std::min(distance, triangleDistance(a_p, t[0], t[1], t[2]));
    }

    bool inside = true;

    for (const auto& plane : m_planes) {
      inside = inside && dot(plane.first, a_p) < plane.second;
    }

    return inside ? -distance : distance;
  }
};

// The vertices of the regular solids, at circumradius a_radius around a_center, written out by hand.
std::vector<V>
scaled(const std::vector<V>& a_directions, const V& a_center, const double a_radius)
{
  std::vector<V> points;

  for (const V& d : a_directions) {
    points.push_back(a_center + a_radius * d / d.length());
  }

  return points;
}

const double phi = (1.0 + std::sqrt(5.0)) / 2.0;

std::vector<V>
cubeCorners()
{
  std::vector<V> corners;

  for (const double x : {1.0, -1.0}) {
    for (const double y : {1.0, -1.0}) {
      for (const double z : {1.0, -1.0}) {
        corners.emplace_back(x, y, z);
      }
    }
  }

  return corners;
}

std::vector<V>
cyclic(const double a_a, const double a_b)
{
  std::vector<V> points;

  for (const double s : {1.0, -1.0}) {
    for (const double t : {1.0, -1.0}) {
      points.emplace_back(0.0, s * a_a, t * a_b);
      points.emplace_back(s * a_a, t * a_b, 0.0);
      points.emplace_back(t * a_b, 0.0, s * a_a);
    }
  }

  return points;
}

// The tolerance against a reference: the precision's loose margin, or what sampling a curve leaves, whichever is
// larger.
template <class T>
double
sampledMargin()
{
  return std::max(double(looseMargin<T>()), 1e-6);
}

// Requires a shape to agree with a reference signed distance at random points.
template <class T, class Shape>
void
requireAgrees(const Shape& a_shape, const std::function<double(const V&)>& a_reference, const T a_extent, const int a_n)
{
  for (const Vec3T<T>& p : randomPoints<T>(a_extent, a_n)) {
    const V q = V(double(p[0]), double(p[1]), double(p[2]));

    INFO("p = " << p);
    REQUIRE_THAT(a_shape.signedDistance(p), withinAbsT<T>(T(a_reference(q)), sampledMargin<T>()));
  }
}

// The distance from (a_r, a_y) to a curve in the cross-section through an axis, sampled finely over [a_t0, a_t1].
double
curveDistance(const std::function<std::pair<double, double>(double)>& a_curve,
              const double                                            a_t0,
              const double                                            a_t1,
              const double                                            a_r,
              const double                                            a_y)
{
  constexpr int N = 200000;

  double best = 1e300;

  for (int i = 0; i <= N; i++) {
    const auto c = a_curve(a_t0 + (a_t1 - a_t0) * double(i) / double(N));

    best = std::min(best, std::hypot(c.first - a_r, c.second - a_y));
  }

  return best;
}

// Requires a ray form's spans to be where the shape's values along the ray are negative, away from the crossings.
template <class T, class Shape>
void
requireSpansMatchValues(const Shape& a_shape, const T a_extent)
{
  std::mt19937                      rng(77);
  std::uniform_real_distribution<T> uniform(-T(1), T(1));

  for (int i = 0; i < 100; i++) {
    const Vec3T<T> origin = a_extent * Vec3T<T>(uniform(rng), uniform(rng), uniform(rng));
    Vec3T<T>       dir(uniform(rng), uniform(rng), uniform(rng));

    dir = dir / dir.length();

    const RayT<T> ray{origin, dir};
    const T       tMax = T(3) * a_extent;

    RaySpansT<T> spans;

    RayForm<Shape>::spans(a_shape, ray, T(0), tMax, RayOptions<T>(), false, spans);

    for (int k = 1; k < 400; k++) {
      const T t     = tMax * T(k) / T(400);
      const T value = a_shape.signedDistance(ray.at(t));

      if (std::abs(value) < T(1e-2) * a_extent) {
        continue;
      }

      bool inside = false;

      for (uint32_t s = 0; s < spans.m_count; s++) {
        inside = inside || (t >= spans.m_spans[s].m_in.m_t && t <= spans.m_spans[s].m_out.m_t);
      }

      INFO("ray " << i << " at t = " << t << ", value " << value);
      REQUIRE(inside == (value < T(0)));
    }
  }
}

} // namespace

TEMPLATE_TEST_CASE("ConvexPolyhedronSDF: the regular solids have their radii, counts and hulls",
                   "[ConvexPolyhedronSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const V      c(0.1, -0.2, 0.3);
  const Vec3   center(T(0.1), T(-0.2), T(0.3));
  const double r = 0.9;

  // The inradius over the circumradius of each solid.
  const double dodecaIn = std::sqrt((25.0 + 11.0 * std::sqrt(5.0)) / 10.0) / (std::sqrt(3.0) * phi);

  struct Case
  {
    const char*            m_name;
    ConvexPolyhedronSDF<T> m_shape;
    std::vector<V>         m_vertices;
    uint32_t               m_faces;
    uint32_t               m_edges;
    double                 m_inradius;
  };

  std::vector<V> dodecahedron = cubeCorners();

  for (const V& v : cyclic(1.0 / phi, phi)) {
    dodecahedron.push_back(v);
  }

  const std::vector<Case> cases{
    {"tetrahedron",
     ConvexPolyhedronSDF<T>::tetrahedron(center, T(r)),
     scaled({V(1, 1, 1), V(1, -1, -1), V(-1, 1, -1), V(-1, -1, 1)}, c, r),
     4,
     6,
     r / 3.0},
    {"octahedron",
     ConvexPolyhedronSDF<T>::octahedron(center, T(r)),
     scaled({V(1, 0, 0), V(-1, 0, 0), V(0, 1, 0), V(0, -1, 0), V(0, 0, 1), V(0, 0, -1)}, c, r),
     8,
     12,
     r / std::sqrt(3.0)},
    {"dodecahedron",
     ConvexPolyhedronSDF<T>::dodecahedron(center, T(r)),
     scaled(dodecahedron, c, r),
     12,
     30,
     r * dodecaIn},
    {"icosahedron",
     ConvexPolyhedronSDF<T>::icosahedron(center, T(r)),
     scaled(cyclic(1.0, phi), c, r),
     20,
     30,
     r * dodecaIn}};

  for (const Case& test : cases) {
    INFO(test.m_name);

    REQUIRE(test.m_shape.getNumFaces() == test.m_faces);
    REQUIRE(test.m_shape.getNumVertices() == test.m_vertices.size());
    REQUIRE(test.m_shape.getNumEdges() == test.m_edges);
    REQUIRE(test.m_shape.getNumParts() == test.m_faces);

    REQUIRE_THAT(test.m_shape.signedDistance(center), withinAbsT<T>(T(-test.m_inradius), double(looseMargin<T>())));

    for (const V& v : test.m_vertices) {
      REQUIRE_THAT(test.m_shape.signedDistance(Vec3(T(v[0]), T(v[1]), T(v[2]))),
                   WithinAbs(0.0, double(looseMargin<T>())));
    }

    const Hull hull(test.m_vertices);

    requireAgrees<T>(test.m_shape, [&hull](const V& a_p) { return hull.signedDistance(a_p); }, T(2), 300);

    // The bounding box is the vertices' box.
    const auto box = test.m_shape.computeBoundingVolume();

    for (size_t dir = 0; dir < 3; dir++) {
      double lo = 1e300;
      double hi = -1e300;

      for (const V& v : test.m_vertices) {
        lo = std::min(lo, v[dir]);
        hi = std::max(hi, v[dir]);
      }

      REQUIRE_THAT(box.getLowCorner()[dir], withinAbsT<T>(T(lo), double(looseMargin<T>())));
      REQUIRE_THAT(box.getHighCorner()[dir], withinAbsT<T>(T(hi), double(looseMargin<T>())));
    }
  }

  // The faces are numbered as documented: the octahedron's by the signs of their normals, the icosahedron's
  // first eight likewise, then (0, +-phi, +-1/phi).
  const auto octahedron  = ConvexPolyhedronSDF<T>::octahedron(Vec3::zeros(), T(1));
  const auto icosahedron = ConvexPolyhedronSDF<T>::icosahedron(Vec3::zeros(), T(1));
  const auto dodeca      = ConvexPolyhedronSDF<T>::dodecahedron(Vec3::zeros(), T(1));

  REQUIRE(octahedron.closestPart(Vec3(T(1), T(1), T(1))) == 0U);
  REQUIRE(octahedron.closestPart(Vec3(T(-1), T(1), T(-1))) == 5U);
  REQUIRE(octahedron.closestPart(Vec3(T(-1), T(-1), T(-1))) == 7U);
  REQUIRE(icosahedron.closestPart(Vec3(T(1), T(-1), T(1))) == 2U);
  REQUIRE(icosahedron.closestPart(T(2) * Vec3(T(0), T(phi), T(-1 / phi))) == 9U);
  REQUIRE(dodeca.closestPart(T(2) * Vec3(T(0), T(phi), T(1))) == 0U);
  REQUIRE(dodeca.closestPart(T(2) * Vec3(T(-1), T(0), T(-phi))) == 11U);

  // The default is the cube [-0.5, 0.5]^3, the same as BoxSDF's.
  const ConvexPolyhedronSDF<T> cube;
  const BoxSDF<T>              box;

  for (const Vec3& p : randomPoints<T>(T(1.5), 300)) {
    REQUIRE_THAT(cube.signedDistance(p), withinAbsT<T>(box.signedDistance(p), double(tightMargin<T>())));
  }
}

TEMPLATE_TEST_CASE("ConvexPolyhedronSDF: a pyramid and a solid from planes match the hulls of their vertices",
                   "[ConvexPolyhedronSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // A square pyramid along y with a vertex along x: its base corners and apex by hand.
  const auto pyramid = ConvexPolyhedronSDF<T>::pyramid(
    Vec3(T(0), T(-0.5), T(0)), Vec3(T(0), T(0.8), T(0)), T(0.7), 4U, Vec3(T(1), T(0), T(0)));

  REQUIRE(pyramid.getNumFaces() == 5U);
  REQUIRE(pyramid.getNumVertices() == 5U);
  REQUIRE(pyramid.getNumEdges() == 8U);

  const Hull square({V(0.7, -0.5, 0), V(0, -0.5, 0.7), V(-0.7, -0.5, 0), V(0, -0.5, -0.7), V(0, 0.8, 0)});

  requireAgrees<T>(pyramid, [&square](const V& a_p) { return square.signedDistance(a_p); }, T(1.5), 300);

  // Its base is the last face; side 0 starts at the first vertex and runs toward cross(axis, vertex) = -z.
  REQUIRE(pyramid.closestPart(Vec3(T(0), T(-2), T(0))) == 4U);
  REQUIRE(pyramid.closestPart(Vec3(T(1), T(-0.3), T(-1))) == 0U);

  // A pyramid on many sides, tilted.
  requireExactSDF<T>(
    ConvexPolyhedronSDF<T>::pyramid(
      Vec3(T(0.1), T(-0.4), T(0.2)), Vec3(T(-0.2), T(0.7), T(0.1)), T(0.6), 19U, Vec3(T(1), T(1), T(0))),
    T(1.5));

  // A cube [-1, 1]^3 with the corner (1, 1, 1) cut off by x + y + z <= 1.5: seven corners and three cut points.
  std::vector<PlaneSDF<T>> planes;

  for (size_t dir = 0; dir < 3; dir++) {
    planes.emplace_back(Vec3::unit(dir), Vec3::unit(dir));
    planes.emplace_back(-Vec3::unit(dir), -Vec3::unit(dir));
  }

  planes.emplace_back(Vec3(T(0.5), T(0.5), T(0.5)), Vec3::ones());

  const ConvexPolyhedronSDF<T> cut(planes);

  REQUIRE(cut.getNumFaces() == 7U);
  REQUIRE(cut.getNumVertices() == 10U);
  REQUIRE(cut.getNumEdges() == 15U);

  std::vector<V> corners;

  for (const V& v : cubeCorners()) {
    if (v[0] + v[1] + v[2] < 1.5) {
      corners.push_back(v);
    }
  }

  corners.emplace_back(-0.5, 1, 1);
  corners.emplace_back(1, -0.5, 1);
  corners.emplace_back(1, 1, -0.5);

  const Hull hull(corners);

  requireAgrees<T>(cut, [&hull](const V& a_p) { return hull.signedDistance(a_p); }, T(2), 300);

  // The cut face is part 6, the plane's place in the list.
  REQUIRE(cut.closestPart(Vec3(T(2), T(2), T(2))) == 6U);

  // A small solid far from the origin keeps its vertices apart: where planes meet is judged against its own size.
  const Vec3 far(T(100), T(-100), T(100));
  const auto tiny = ConvexPolyhedronSDF<T>::icosahedron(far, T(0.01));

  REQUIRE(tiny.getNumVertices() == 12U);
  REQUIRE(tiny.signedDistance(far + T(0.02) * Vec3::unit(0)) > T(0));
  REQUIRE(tiny.signedDistance(far) < T(0));
}

TEMPLATE_TEST_CASE("BoxFrameSDF: the union of its twelve bars, exact, with the bars numbered by axis and side",
                   "[BoxFrameSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Vec3 lo(T(-0.6), T(-0.4), T(-0.5));
  const Vec3 hi(T(0.7), T(0.5), T(0.4));
  const T    t = T(0.15);

  const BoxFrameSDF<T> frame(lo, hi, t);

  // The bars as boxes: outside the frame, the union's distance is the nearest bar's.
  std::vector<BoxSDF<T>> bars;

  for (uint32_t bar = 0; bar < 12U; bar++) {
    const uint32_t axis   = bar / 4U;
    const uint32_t first  = (axis == 0U) ? 1U : 0U;
    const uint32_t second = (axis == 2U) ? 1U : 2U;

    Vec3 barLo = lo;
    Vec3 barHi = hi;

    ((bar & 2U) ? barLo[first] : barHi[first])   = (bar & 2U) ? hi[first] - t : lo[first] + t;
    ((bar & 1U) ? barLo[second] : barHi[second]) = (bar & 1U) ? hi[second] - t : lo[second] + t;

    bars.emplace_back(barLo, barHi);
  }

  for (const Vec3& p : randomPoints<T>(T(1.2), 1000)) {
    T        nearest = Math::Limits<T>::max();
    uint32_t which   = 0;

    for (uint32_t bar = 0; bar < 12U; bar++) {
      if (bars[bar].signedDistance(p) < nearest) {
        nearest = bars[bar].signedDistance(p);
        which   = bar;
      }
    }

    INFO("p = " << p);

    if (nearest > T(0)) {
      REQUIRE_THAT(frame.signedDistance(p), withinAbsT<T>(nearest, double(looseMargin<T>())));
    }
    else {
      REQUIRE(frame.signedDistance(p) <= T(0));
    }

    // The part is the nearest bar, where it is clearly nearer than the others.
    T second = Math::Limits<T>::max();

    for (uint32_t bar = 0; bar < 12U; bar++) {
      if (bar != which) {
        second = std::min(second, bars[bar].signedDistance(p));
      }
    }

    if (second - nearest > T(1e-2)) {
      REQUIRE(frame.closestPart(p) == which);
    }
  }

  // In the middle of a bar, half its thickness deep.
  REQUIRE_THAT(frame.signedDistance(Vec3(T(0), lo[1] + T(0.5) * t, lo[2] + T(0.5) * t)),
               withinAbsT<T>(T(-0.5) * t, double(looseMargin<T>())));

  // In float, the check's tolerance on the gradient's length is too wide to tell a point beside a ridge, where two
  // bars are equally near, from one on neither; the comparison with the bars above covers float.
  if constexpr (std::is_same_v<T, double>) {
    requireExactSDF<T>(frame, T(1.2));
  }
}

TEMPLATE_TEST_CASE("QuadricSDF: the hyperboloids and the paraboloid match their sampled profiles",
                   "[QuadricSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Vec3 center(T(0.1), T(-0.2), T(0.3));
  const V    c(0.1, -0.2, 0.3);

  // A waist of 0.6 and an asymptotic cone of 70 degrees: a = 0.6, c = a / tan(35 degrees).
  const auto   one = QuadricSDF<T>::hyperboloid(center, T(0.6), T(70));
  const double a1  = 0.6;
  const double c1  = 0.6 / std::tan(35.0 * pi<double> / 180.0);

  REQUIRE(one.getKind() == QuadricSDF<T>::Kind::HyperboloidOneSheet);
  REQUIRE_THAT(one.signedDistance(center), withinAbsT<T>(T(-0.6), double(looseMargin<T>())));

  requireAgrees<T>(
    one,
    [&](const V& a_p) {
      const V      w = a_p - c;
      const double r = std::hypot(w[0], w[2]);
      const double d = curveDistance(
        [&](const double a_t) { return std::make_pair(a1 * std::cosh(a_t), c1 * std::sinh(a_t)); }, -4, 4, r, w[1]);

      return (r * r / (a1 * a1) - w[1] * w[1] / (c1 * c1) < 1) ? -d : d;
    },
    T(2),
    100);

  // Tips at 0.5 from the center and a cone of 80 degrees: a = c tan(40 degrees). The center is a tip away.
  const auto   two = QuadricSDF<T>::hyperboloidTwoSheets(center, T(0.5), T(80));
  const double c2  = 0.5;
  const double a2  = 0.5 * std::tan(40.0 * pi<double> / 180.0);

  REQUIRE_THAT(two.signedDistance(center), withinAbsT<T>(T(0.5), double(looseMargin<T>())));

  requireAgrees<T>(
    two,
    [&](const V& a_p) {
      const V      w = a_p - c;
      const double r = std::hypot(w[0], w[2]);
      const double d =
        curveDistance([&](const double a_t) { return std::make_pair(a2 * std::sinh(a_t), c2 * std::cosh(a_t)); },
                      0,
                      4,
                      r,
                      std::abs(w[1]));

      return (w[1] * w[1] / (c2 * c2) - r * r / (a2 * a2) > 1) ? -d : d;
    },
    T(2),
    100);

  // A tip radius of 0.7: y = -r^2 / 1.4 below the tip. Just below the tip on the axis, the tip is the closest point.
  const auto para = QuadricSDF<T>::paraboloid(center, T(0.7));

  REQUIRE_THAT(para.signedDistance(center - T(0.3) * Vec3::unit(1)), withinAbsT<T>(T(-0.3), double(looseMargin<T>())));
  REQUIRE(para.computeBoundingVolume().getHighCorner()[1] == center[1]);

  requireAgrees<T>(
    para,
    [&](const V& a_p) {
      const V      w = a_p - c;
      const double r = std::hypot(w[0], w[2]);
      const double d =
        curveDistance([](const double a_t) { return std::make_pair(a_t, -a_t * a_t / 1.4); }, 0, 5, r, w[1]);

      return (w[1] + r * r / 1.4 < 0) ? -d : d;
    },
    T(2),
    100);

  requireExactSDF<T>(one, T(2));
  requireExactSDF<T>(two, T(2));
  requireExactSDF<T>(para, T(2));
}

TEMPLATE_TEST_CASE("SuperellipsoidSDF: a sphere for exponents 1, and a bound with the right sign otherwise",
                   "[SuperellipsoidSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Vec3 center(T(0.1), T(-0.2), T(0.3));

  const SuperellipsoidSDF<T> ball(center, T(0.8) * Vec3::ones(), T(1), T(1));
  const SphereSDF<T>         sphere(center, T(0.8));

  for (const Vec3& p : randomPoints<T>(T(2), 300)) {
    REQUIRE_THAT(ball.signedDistance(p), withinAbsT<T>(sphere.signedDistance(p), double(looseMargin<T>())));
  }

  // Against the nearest of many points on the surface, from its parametric form.
  for (const auto& e : {std::make_pair(0.2, 0.6), std::make_pair(1.0, 0.3), std::make_pair(1.8, 2.0)}) {
    const Vec3                 radii(T(1), T(0.5), T(0.7));
    const SuperellipsoidSDF<T> shape(center, radii, T(e.first), T(e.second));

    const auto power = [](const double a_x, const double a_e) {
      return (a_x < 0 ? -1.0 : 1.0) * std::pow(std::abs(a_x), a_e);
    };

    std::vector<V> surface;

    for (int i = 0; i <= 300; i++) {
      for (int j = 0; j < 600; j++) {
        const double v = -0.5 * pi<double> + pi<double> * double(i) / 300.0;
        const double u = 2.0 * pi<double> * double(j) / 600.0;

        surface.emplace_back(double(center[0]) + 1.0 * power(std::cos(v), e.first) * power(std::cos(u), e.second),
                             double(center[1]) + 0.5 * power(std::sin(v), e.first),
                             double(center[2]) + 0.7 * power(std::cos(v), e.first) * power(std::sin(u), e.second));
      }
    }

    for (const Vec3& p : randomPoints<T>(T(1.5), 60)) {
      double nearest = 1e300;

      for (const V& s : surface) {
        nearest = std::min(nearest, (s - V(double(p[0]), double(p[1]), double(p[2]))).length());
      }

      const T value = shape.signedDistance(p);

      INFO("p = " << p << ", value " << value << ", nearest " << nearest);
      REQUIRE(std::abs(double(value)) <= nearest + 2e-2);
      REQUIRE((shape.gauge(p) < T(1)) == (value < T(0)));
    }
  }
}

TEMPLATE_TEST_CASE("SolidAngleSDF and VesicaSDF: their sampled outlines, and exact",
                   "[SolidAngleSDF][VesicaSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Vec3 center(T(0.1), T(-0.2), T(0.3));
  const V    c(0.1, -0.2, 0.3);

  for (const double angle : {70.0, 250.0}) {
    const SolidAngleSDF<T> solid(center, T(1.2), T(angle));
    const double           h = 0.5 * angle * pi<double> / 180.0;

    INFO("angle " << angle);

    requireAgrees<T>(
      solid,
      [&](const V& a_p) {
        const V      w   = a_p - c;
        const double r   = std::hypot(w[0], w[2]);
        const double arc = curveDistance(
          [](const double a_t) { return std::make_pair(1.2 * std::sin(a_t), 1.2 * std::cos(a_t)); }, 0, h, r, w[1]);
        const double side = curveDistance(
          [&](const double a_t) { return std::make_pair(a_t * std::sin(h), a_t * std::cos(h)); }, 0, 1.2, r, w[1]);
        const bool inside = std::hypot(r, w[1]) < 1.2 && std::atan2(r, w[1]) < h;

        return inside ? -std::min(arc, side) : std::min(arc, side);
      },
      T(2),
      100);

    requireExactSDF<T>(solid, T(2));

    // The cap is part 0 above the center, the side part 1 beside the cone.
    REQUIRE(solid.closestPart(center + T(2) * Vec3::unit(1)) == 0U);
  }

  // Tips 2.4 apart and a radius of 0.5: the arc's circle is centered d = (1.2^2 - 0.5^2) / 1 across the axis.
  const VesicaSDF<T> vesica(center - T(1.2) * Vec3::unit(1), center + T(1.2) * Vec3::unit(1), T(0.5));
  const double       d  = 0.5 * (1.2 * 1.2 - 0.25) / 0.5;
  const double       a0 = std::atan2(1.2, d);

  REQUIRE_THAT(vesica.signedDistance(center), withinAbsT<T>(T(-0.5), double(looseMargin<T>())));

  requireAgrees<T>(
    vesica,
    [&](const V& a_p) {
      const V      w = a_p - c;
      const double r = std::hypot(w[0], w[2]);
      const double e = curveDistance(
        [&](const double a_t) { return std::make_pair((d + 0.5) * std::cos(a_t) - d, (d + 0.5) * std::sin(a_t)); },
        -a0,
        a0,
        r,
        w[1]);

      return ((r + d) * (r + d) + w[1] * w[1] < (d + 0.5) * (d + 0.5)) ? -e : e;
    },
    T(2),
    100);

  requireExactSDF<T>(vesica, T(2));
}

TEMPLATE_TEST_CASE("HelixSDF: the distance to its sampled center line less the tube radius",
                   "[HelixSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // A tilted helix with a fraction of a turn, and a wide flat coil of many turns close together.
  const std::vector<HelixSDF<T>> helices{
    HelixSDF<T>(Vec3(T(0.1), T(-1.5), T(0)), Vec3(T(0.3), T(1.5), T(0.2)), T(0.8), T(5.3), T(0.1), Vec3(1, 0, 1)),
    HelixSDF<T>(Vec3::zeros(), Vec3::unit(1), T(1.5), T(10), T(0.04), Vec3::unit(0))};

  for (const HelixSDF<T>& helix : helices) {
    // The line itself, sampled independently from its definition: a turn of 2 pi around the axis rises one pitch.
    const V c1(double(helix.getCenter1()[0]), double(helix.getCenter1()[1]), double(helix.getCenter1()[2]));
    const V c2(double(helix.getCenter2()[0]), double(helix.getCenter2()[1]), double(helix.getCenter2()[2]));
    const V axis = (c2 - c1) / (c2 - c1).length();
    const V u(
      double(helix.getStartDirection()[0]), double(helix.getStartDirection()[1]), double(helix.getStartDirection()[2]));
    const V      v     = cross(axis, u);
    const double turns = double(helix.getTurns());
    const double R     = double(helix.getHelixRadius());

    std::vector<V> line;

    constexpr int N = 200000;

    for (int i = 0; i <= N; i++) {
      const double s = double(i) / double(N);
      const double a = 2.0 * pi<double> * turns * s;

      line.push_back(c1 + s * (c2 - c1) + R * (std::cos(a) * u + std::sin(a) * v));
    }

    for (const Vec3& p : randomPoints<T>(T(2), 60)) {
      double nearest = 1e300;

      for (const V& q : line) {
        nearest = std::min(nearest, (q - V(double(p[0]), double(p[1]), double(p[2]))).length());
      }

      INFO("p = " << p);
      REQUIRE_THAT(helix.signedDistance(p),
                   withinAbsT<T>(T(nearest - double(helix.getTubeRadius())), sampledMargin<T>()));
    }

    // As for the box frame: in float, the check cannot tell a point beside a ridge between two turns.
    if constexpr (std::is_same_v<T, double>) {
      requireExactSDF<T>(helix, T(2));
    }
  }
}

TEST_CASE("HelixSDF: the closest distance between neighboring turns, against a dense scan", "[HelixSDF]")
{
  using Vec3 = Vec3T<double>;

  for (const auto& rp : {std::make_pair(0.5, 1.0), std::make_pair(1.0, 0.1), std::make_pair(0.7, 0.45)}) {
    const double R    = rp.first;
    const double rate = rp.second / (2.0 * pi<double>);

    // The chord between points an angle apart, past its first maximum, over two turns.
    double scan    = 1e300;
    double prev    = 0.0;
    bool   falling = false;

    for (int i = 1; i <= 400000; i++) {
      const double t = 4.0 * pi<double> * double(i) / 400000.0;
      const double d = std::sqrt(2.0 * R * R * (1.0 - std::cos(t)) + rate * rate * t * t);

      falling = falling || d < prev;
      scan    = falling ? std::min(scan, d) : scan;
      prev    = d;
    }

    const double apart = AnalyticDetail::helixTurnDistance(R, rp.second);

    INFO("R " << R << ", pitch " << rp.second);
    REQUIRE_THAT(apart, WithinAbs(scan, 1e-9));

    // A tube just under half of it constructs.
    const HelixSDF<double> h(Vec3::zeros(), Vec3::unit(1) * (4.0 * rp.second), R, 4.0, 0.499 * apart, Vec3::unit(0));

    REQUIRE(h.getTubeRadius() < 0.5 * apart);
  }

  // A pitch large against the radius: the turns never come back nearer.
  REQUIRE(std::isinf(AnalyticDetail::helixTurnDistance(0.1, 5.0)));
}

TEMPLATE_TEST_CASE("LatticeSDF: walls on each surface, and a bound that never changes faster than the distance",
                   "[LatticeSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Kind = typename LatticeSDF<T>::Kind;

  const Vec3 cell(T(1), T(1.5), T(0.8));
  const T    t = T(0.1);

  // A point on each surface is in the middle of its wall: Schwarz P at a quarter cell, the gyroid and D at the
  // origin, Neovius at a quarter cell along each axis.
  REQUIRE_THAT(LatticeSDF<T>(Kind::SchwarzP, cell, t).signedDistance(T(0.25) * cell),
               withinAbsT<T>(T(-0.5) * t, double(looseMargin<T>())));
  REQUIRE_THAT(LatticeSDF<T>(Kind::Gyroid, cell, t).signedDistance(Vec3::zeros()),
               withinAbsT<T>(T(-0.5) * t, double(looseMargin<T>())));
  REQUIRE_THAT(LatticeSDF<T>(Kind::SchwarzD, cell, t).signedDistance(Vec3::zeros()),
               withinAbsT<T>(T(-0.5) * t, double(looseMargin<T>())));
  REQUIRE_THAT(LatticeSDF<T>(Kind::Neovius, cell, t).signedDistance(T(0.25) * cell),
               withinAbsT<T>(T(-0.5) * t, double(looseMargin<T>())));

  // A value that never changes faster than the distance, between nearby and far pairs of points, is a bound.
  for (const Kind kind : {Kind::Gyroid, Kind::SchwarzP, Kind::SchwarzD, Kind::Neovius}) {
    const LatticeSDF<T> lattice(kind, cell, t);

    const std::vector<Vec3> points = randomPoints<T>(T(2), 2000);

    for (size_t i = 1; i < points.size(); i++) {
      const Vec3 near = points[i] + T(1e-2) * (points[i - 1] - points[i]);

      REQUIRE(std::abs(lattice.signedDistance(points[i]) - lattice.signedDistance(near)) <=
              (points[i] - near).length() * (T(1) + T(1e-3)) + T(1e-6));
      REQUIRE(std::abs(lattice.signedDistance(points[i]) - lattice.signedDistance(points[i - 1])) <=
              (points[i] - points[i - 1]).length() * (T(1) + T(1e-3)) + T(1e-6));
    }
  }
}

TEMPLATE_TEST_CASE("BoxFrameSDF: a ray from inside a bar has a span that starts open, and leaves where the bar ends",
                   "[BoxFrameSDF][Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const BoxFrameSDF<T> frame(-Vec3::ones(), Vec3::ones(), T(0.2));

  // From inside the bar along x at the low y and z, along +x: inside until the bar's end at x = 1.
  const RayT<T> ray{Vec3(T(0), T(-0.9), T(-0.9)), Vec3::unit(0)};

  RaySpansT<T> spans;

  RayForm<BoxFrameSDF<T>>::spans(frame, ray, T(0), T(5), RayOptions<T>(), false, spans);

  REQUIRE(spans.m_count == 1U);
  REQUIRE_FALSE(spans.m_spans[0].m_in.crossing());
  REQUIRE_THAT(spans.m_spans[0].m_out.m_t, withinAbsT<T>(T(1), double(tightMargin<T>())));

  const HitT<T> hit = firstHit(frame, ray, T(0), T(5));

  REQUIRE(hit.hit());
  REQUIRE_FALSE(hit.m_entering);
  REQUIRE_THAT(hit.m_t, withinAbsT<T>(T(1), double(tightMargin<T>())));
}

TEMPLATE_TEST_CASE("Shape families: a chain link is an elongated torus, with Quilez's exact link distance",
                   "[AnalyticSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  // A ring of radius 0.5 in the xy-plane, its straight sides 2 h = 0.8 long along y, a tube radius of 0.15.
  const T h  = T(0.4);
  const T r1 = T(0.5);
  const T r2 = T(0.15);

  const Tape<T> link(pool, Elongate(Rotate(TorusSDF<T>(Vec3::zeros(), r1, r2), T(90), size_t(0)), Vec3(T(0), h, T(0))));

  for (const Vec3& p : randomPoints<T>(T(1.5), 500)) {
    const T qx = p[0];
    const T qy = std::max(std::abs(p[1]) - h, T(0));
    const T qz = p[2];

    const T reference =
      std::sqrt((std::sqrt(qx * qx + qy * qy) - r1) * (std::sqrt(qx * qx + qy * qy) - r1) + qz * qz) - r2;

    INFO("p = " << p);
    REQUIRE_THAT(link.signedDistance(p), withinAbsT<T>(reference, double(looseMargin<T>())));
  }

  requireExactSDF<T>(link, T(1.5));
}

TEMPLATE_TEST_CASE("Shape families: the ray forms' spans are where the values along the ray are negative",
                   "[Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  requireSpansMatchValues<T>(ConvexPolyhedronSDF<T>::icosahedron(Vec3(T(0.1), T(0), T(0)), T(0.8)), T(1.5));
  requireSpansMatchValues<T>(
    ConvexPolyhedronSDF<T>::pyramid(Vec3(T(0), T(-0.5), T(0)), Vec3(T(0.1), T(0.6), T(0)), T(0.7), 5U, Vec3::ones()),
    T(1.5));
  requireSpansMatchValues<T>(BoxFrameSDF<T>(Vec3(T(-0.6), T(-0.5), T(-0.4)), Vec3(T(0.5), T(0.6), T(0.7)), T(0.15)),
                             T(1.5));
  requireSpansMatchValues<T>(QuadricSDF<T>::hyperboloid(Vec3::zeros(), T(0.5), T(60)), T(1.5));
  requireSpansMatchValues<T>(QuadricSDF<T>::hyperboloidTwoSheets(Vec3::zeros(), T(0.4), T(90)), T(1.5));
  requireSpansMatchValues<T>(QuadricSDF<T>::paraboloid(Vec3(T(0), T(0.5), T(0)), T(0.5)), T(1.5));
}

TEST_CASE("Shape families: invalid constructor arguments abort with a message", "[AnalyticSDF][death]")
{
  using T    = double;
  using Vec3 = Vec3T<T>;

  // The six faces of the cube [-1, 1]^3.
  const auto cubePlanes = [] {
    std::vector<PlaneSDF<T>> planes;

    for (size_t dir = 0; dir < 3; dir++) {
      planes.emplace_back(Vec3::unit(dir), Vec3::unit(dir));
      planes.emplace_back(-Vec3::unit(dir), -Vec3::unit(dir));
    }

    return planes;
  };

  requireAbortsWith({
    {[] { (void)ConvexPolyhedronSDF<T>::pyramid(Vec3::zeros(), Vec3::unit(1), T(1), 20U, Vec3::unit(0)); },
     "ConvexPolyhedronSDF::pyramid: the number of sides must be from 3 to 19 (20)"},
    // An open box: five faces leave it unbounded.
    {[&] {
       auto planes = cubePlanes();

       planes.pop_back();

       (void)ConvexPolyhedronSDF<T>(planes);
     },
     "ConvexPolyhedronSDF: plane 0 does not bound a face of the solid, or the solid is not finite"},
    // A plane beyond the cube bounds no face.
    {[&] {
       auto planes = cubePlanes();

       planes.emplace_back(T(2) * Vec3::unit(0), Vec3::unit(0));

       (void)ConvexPolyhedronSDF<T>(planes);
     },
     "ConvexPolyhedronSDF: plane 6 does not bound a face of the solid, or the solid is not finite"},
    {[] { (void)ConvexPolyhedronSDF<T>(std::vector<PlaneSDF<T>>(3)); },
     "ConvexPolyhedronSDF: a polyhedron has from 4 to 20 planes (3)"},
    {[] { (void)BoxFrameSDF<T>(Vec3::zeros(), Vec3::ones(), T(0.5)); },
     "BoxFrameSDF: the thickness must be positive and less than half the shortest side 1 (0.5)"},
    {[] { (void)QuadricSDF<T>::hyperboloid(Vec3::zeros(), T(1), T(180)); },
     "QuadricSDF::hyperboloid: the angle must be in (0, 180) degrees (180)"},
    {[] { (void)SuperellipsoidSDF<T>(Vec3::zeros(), Vec3::ones(), T(3), T(1)); },
     "SuperellipsoidSDF: the exponents must be in [0.1, 2] (3, 1)"},
    {[] { (void)SolidAngleSDF<T>(Vec3::zeros(), T(1), T(360)); },
     "SolidAngleSDF: the angle must be in (0, 360) degrees (360)"},
    {[] { (void)VesicaSDF<T>(Vec3::zeros(), Vec3::unit(1), T(0.5)); },
     "VesicaSDF: the radius must be positive and less than half the distance between the tips 0.5 (0.5)"},
    {[] { (void)HelixSDF<T>(Vec3::zeros(), Vec3::unit(1), T(1), T(10), T(0.06), Vec3::unit(0)); },
     "HelixSDF: the tube radius must be positive and less than the helix radius 1 and half the distance between turns"},
    // A tube under half the pitch that still touches the next turn, the turns being closer than the pitch: the
    // closest approach is not straight across, along the axis, so this tube is short of half the across distance and
    // overlaps.
    {[] { (void)HelixSDF<T>(Vec3::zeros(), Vec3::unit(1), T(0.5), T(1), T(0.4765), Vec3::unit(0)); },
     "HelixSDF: the tube radius must be positive and less than the helix radius 0.5 and half the distance between "
     "turns"},
    {[] { (void)LatticeSDF<T>(LatticeSDF<T>::Kind::Gyroid, Vec3::ones(), T(1)); },
     "LatticeSDF: the thickness must be positive and less than the smallest cell size 1 (1)"},
  });
}
