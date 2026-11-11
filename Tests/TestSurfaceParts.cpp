// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for the parts of a shape and the face groups of a mesh (EBGeometry_SurfaceRef.hpp,
// EBGeometry_FaceGroups.hpp): each built-in shape's parts at points beside each face, a mesh's part as the face
// a point is beside, the readers' face groups (OBJ groups, STL solids) kept through clean(), SurfaceLabel(shape,
// FaceGroups) in ExprIndex and in a tape's closestSurface() and firstSurface(), the free closestSurface() and
// firstSurface() on any shape, the rejected labels and numbers, and device evaluation.

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;

namespace {

std::string
dataFile(const std::string& a_name)
{
  return std::string(EBGEOMETRY_TEST_DATA_DIR) + "/" + a_name;
}

// Check a shape's part at each point against the part expected there.
template <class T, class S>
void
requireParts(const S& a_shape, const uint32_t a_numParts, const std::vector<std::pair<Vec3T<T>, uint32_t>>& a_probes)
{
  static_assert(hasParts<S, T>);

  CHECK(a_shape.getNumParts() == a_numParts);

  for (const auto& [point, part] : a_probes) {
    INFO("point " << point);
    CHECK(a_shape.closestPart(point) == part);
  }
}

// Device functor: the surface's label and part at a point.
template <class T, class TapeType>
struct PartLabelQuery
{
  TapeType m_tape;

  EBGEOMETRY_HOST_DEVICE
  uint32_t
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    const SurfaceRef<T> s = m_tape.closestSurface(a_point);

    return 1000U * s.m_surfaceLabel.value() + s.m_part;
  }
};

} // namespace

TEMPLATE_TEST_CASE("Parts: a box, a rounded box and the round shapes report the face beside a point",
                   "[SurfaceRef][Parts]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;
  using V = Vec3T<T>;

  // -x, +x, -y, +y, -z, +z, outside and just inside.
  requireParts<T>(BoxSDF<T>(V(-1, -2, -3), V(1, 2, 3)),
                  6,
                  {{V(-1.5, 0, 0), 0},
                   {V(1.5, 0, 0), 1},
                   {V(0, -2.5, 0), 2},
                   {V(0, 2.5, 0), 3},
                   {V(0, 0, -3.5), 4},
                   {V(0, 0, 3.5), 5},
                   {V(0.9, 0, 0), 1},
                   {V(0, -1.9, 0.5), 2},
                   {V(0, 0, 2.95), 5}});

  requireParts<T>(RoundedBoxSDF<T>(V(0, 0, 0), V(2, 4, 6), T(0.2)),
                  6,
                  {{V(-2, 0, 0), 0}, {V(2, 0, 0), 1}, {V(0, 3, 0), 3}, {V(0, 0, -4), 4}, {V(0, 0, 4), 5}});

  // Side, the cap at the first center, the cap at the second.
  requireParts<T>(CylinderSDF<T>(V(0, 0, -1), V(0, 0, 1), T(0.5)),
                  3,
                  {{V(0.8, 0, 0), 0}, {V(0, 0, -1.3), 1}, {V(0, 0, 1.3), 2}, {V(0.45, 0, 0), 0}, {V(0, 0, 0.95), 2}});

  requireParts<T>(
    CapsuleSDF<T>(V(0, 0, -2), V(0, 0, 2), T(0.5)), 3, {{V(0.8, 0, 0), 0}, {V(0, 0, -2.3), 1}, {V(0, 0, 2.3), 2}});

  requireParts<T>(
    RoundedCylinderSDF<T>(V(0, 0, 0), T(1), T(0.1), T(1)), 3, {{V(3, 0, 0), 0}, {V(0, -3, 0), 1}, {V(0, 3, 0), 2}});

  // A cone opening down from its tip: the side, then the base.
  requireParts<T>(ConeSDF<T>(V(0, 0, 0), T(2), T(60)), 2, {{V(0.9, -1, 0), 0}, {V(0, -2.3, 0), 1}});

  requireParts<T>(CappedConeSDF<T>(V(0, 0, 0), V(0, 0, 2), T(1), T(0.5)),
                  3,
                  {{V(1.2, 0, 1), 0}, {V(0.2, 0, -0.3), 1}, {V(0.2, 0, 2.3), 2}});

  requireParts<T>(RoundConeSDF<T>(V(0, 0, 0), V(0, 0, 2), T(1), T(0.5)),
                  3,
                  {{V(1.2, 0, 1), 0}, {V(0, 0, -1.3), 1}, {V(0, 0, 2.8), 2}});

  // Outer wall, inner wall, the end at the first center, the end at the second.
  requireParts<T>(TubeSDF<T>(V(0, 0, 0), V(0, 0, 2), T(0.5), T(1)),
                  4,
                  {{V(1.3, 0, 1), 0}, {V(0.2, 0, 1), 1}, {V(0.75, 0, -0.3), 2}, {V(0.75, 0, 2.3), 3}});

  // A quarter of a torus around +x: the tube, the end at +45 degrees (toward +z), the end at -45.
  const T end = T(55) * pi<T> / T(180);

  requireParts<T>(CappedTorusSDF<T>(V(0, 0, 0), T(2), T(0.3), T(90)),
                  3,
                  {{V(2.4, 0, 0), 0},
                   {V(2 * std::cos(end), 0, 2 * std::sin(end)), 1},
                   {V(2 * std::cos(end), 0, -2 * std::sin(end)), 2}});

  // The sphere above the cut plane at y = 0.3, and the flat face.
  requireParts<T>(CutSphereSDF<T>(V(0, 0, 0), T(1), T(0.3)), 2, {{V(0, 1.3, 0), 0}, {V(0, 0, 0), 1}});

  requireParts<T>(
    CutHollowSphereSDF<T>(V(0, 0, 0), T(1), T(0.3), T(0.1)), 2, {{V(0, 1.3, 0), 0}, {V(0.95, 0.2, 0), 1}});

  // A wedge with its edge along z: the face on the -x side, the one on the +x side.
  requireParts<T>(WedgeSDF<T>(V(0, 0, 0), T(60)), 2, {{V(-1, -0.2, 0), 0}, {V(1, -0.2, 0), 1}});

  static_assert(!hasParts<SphereSDF<T>, T>);
  static_assert(!hasParts<TorusSDF<T>, T>);
}

TEMPLATE_TEST_CASE("Parts: a polygon prism numbers its sides around the axis, then its caps",
                   "[SurfaceRef][Parts]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;
  using V = Vec3T<T>;

  const PolygonPrismSDF<T> prism(V(0, 0, 0), V(0, 0, 2), T(1), 5U, V(1, 0, 0));

  CHECK(prism.getNumParts() == 7U);
  CHECK(prism.closestPart(V(0, 0, -0.5)) == 5U);
  CHECK(prism.closestPart(V(0, 0, 2.5)) == 6U);

  // Beside the middle of each side, going around: five different sides, one after the other in one direction.
  std::vector<uint32_t> sides;

  for (int k = 0; k < 5; k++) {
    const T angle = (T(2 * k) + T(1)) * pi<T> / T(5);

    sides.push_back(prism.closestPart(V(T(1.5) * std::cos(angle), T(1.5) * std::sin(angle), T(1))));
  }

  const uint32_t step = (sides[1] + 5U - sides[0]) % 5U;

  CHECK((step == 1U || step == 4U));

  for (int k = 0; k < 5; k++) {
    CHECK(sides[k] < 5U);
    CHECK(sides[(k + 1) % 5] == (sides[k] + step) % 5U);
  }
}

TEMPLATE_TEST_CASE("Parts: a mesh's part is the face a point is beside",
                   "[SurfaceRef][Parts]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;
  using V = Vec3T<T>;

  Pool pool(hostMemoryResource());

  const auto soup = Parser::readIntoPolygonSoup<T>(dataFile("cube_quads.obj"));

  const auto tri  = Parser::readIntoTriMeshSDF<T>(dataFile("cube_quads.obj"), pool);
  const auto dcel = Parser::readIntoMeshSDF<T>(dataFile("cube_quads.obj"), pool);
  const auto flat = Parser::readIntoFlatMeshSDF<T>(dataFile("cube_quads.obj"), pool);

  CHECK(tri.getNumParts() == 6U);
  CHECK(dcel.getNumParts() == 6U);
  CHECK(flat.getNumParts() == 6U);

  // Each face's center, pushed out along its normal: that face is the part.
  for (uint32_t f = 0; f < soup.numFacets(); f++) {
    const auto& facet = soup.getFacets()[f];
    const auto& v     = soup.getVertexCoordinates();

    V center = V::zeros();

    for (const size_t i : facet) {
      center += v[i] / T(facet.size());
    }

    const V normal = cross(v[facet[1]] - v[facet[0]], v[facet[2]] - v[facet[0]]) /
                     cross(v[facet[1]] - v[facet[0]], v[facet[2]] - v[facet[0]]).length();
    const V point = center + T(0.2) * normal;

    CHECK(tri.closestPart(point) == f);
    CHECK(dcel.closestPart(point) == f);
    CHECK(flat.closestPart(point) == f);
  }
}

TEMPLATE_TEST_CASE("Parser: OBJ groups and STL solids are kept as face groups",
                   "[Parser][FaceGroups]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto cube = Parser::readOBJ<T>(dataFile("cube_groups.obj"));

  const FaceGroups& groups = cube.getFaceGroups();

  REQUIRE(groups.m_names == std::vector<std::string>{"bottom", "top", "walls"});
  REQUIRE(groups.m_group == std::vector<uint32_t>{0, 1, 2, 2, 2, FaceGroups::None});

  // Every solid of an STL file is read, each named one a group.
  const auto tetrahedron = Parser::readSTL<T>(dataFile("tetrahedron_solids.stl"));

  CHECK(tetrahedron.numFacets() == 4U);
  CHECK(tetrahedron.getFaceGroups().m_names == std::vector<std::string>{"base", "sides"});
  CHECK(tetrahedron.getFaceGroups().m_group == std::vector<uint32_t>{0, 1, 1, 1});

  // A solid that starts before the previous one has ended ends it.
  const auto open = Parser::readSTL<T>(dataFile("tetrahedron_solids_open.stl"));

  CHECK(open.getFaceGroups().m_group == std::vector<uint32_t>{0, 1, 1, 1});

  // A file without groups has none.
  CHECK(Parser::readOBJ<T>(dataFile("cube_quads.obj")).getFaceGroups().empty());
  CHECK(Parser::readSTL<T>(dataFile("tetrahedron.stl")).getFaceGroups().m_names ==
        std::vector<std::string>{"tetrahedron"});
}

TEMPLATE_TEST_CASE("SurfaceLabel: face groups label a mesh's faces in a tape, over the leaf's label",
                   "[SurfaceRef][FaceGroups][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using V    = Vec3T<T>;
  using Mesh = TriMeshSDF<T, BVH::DefaultBranchingRatio<T>(), TriangleSoA::DefaultWidth<T>()>;

  Pool pool(hostMemoryResource());

  const auto soup = Parser::readIntoPolygonSoup<T>(dataFile("cube_groups.obj"));
  const Mesh cube = Parser::readIntoTriMeshSDF<T>(dataFile("cube_groups.obj"), pool);

  // The cube labeled "part" as a whole and its faces by group; a ball far away labeled "ball".
  const Expr<T> expr = Union(SurfaceLabel(SurfaceLabel(cube, soup.getFaceGroups()), "part"),
                             SurfaceLabel(SphereSDF<T>(V(5, 0.5, 0.5), T(0.5)), "ball"));

  const Tape<T>      tape(pool, expr);
  const ExprIndex<T> index(expr);

  // "part" first (the node above the leaf), then the groups, then "ball".
  REQUIRE(tape.getNumSurfaceLabels() == 5U);
  CHECK(index.getSurfaceLabelName(SurfaceLabelId(0)) == "part");
  CHECK(index.findSurfaceLabel("bottom").value() == 1U);
  CHECK(index.findSurfaceLabel("top").value() == 2U);
  CHECK(index.findSurfaceLabel("walls").value() == 3U);
  CHECK(index.findSurfaceLabel("ball").value() == 4U);
  CHECK(index.getPartSurfaceLabel(0, 5).value() == 0U); // the ungrouped face keeps the leaf's label
  CHECK(index.getPartSurfaceLabel(1, 0).value() == 4U); // a leaf without groups: its label

  const std::vector<std::pair<V, uint32_t>> probes{{V(0.5, 0.5, -0.2), 1},
                                                   {V(0.5, 0.5, 1.2), 2},
                                                   {V(0.5, -0.2, 0.5), 3},
                                                   {V(1.2, 0.5, 0.5), 3},
                                                   {V(-0.2, 0.5, 0.5), 0},
                                                   {V(5, 0.5, 1.2), 4}};

  for (const auto& [point, label] : probes) {
    INFO("point " << point);

    const SurfaceRef<T> s = tape.closestSurface(point);

    CHECK(s.m_surfaceLabel.value() == label);
    CHECK(s.m_surfaceLabel == index.getPartSurfaceLabel(s.m_leaf, s.m_part));
  }

  // Along a ray: the bottom from below, the walls from the side, through firstSurface().
  const SurfaceHitT<T> fromBelow = tape.firstSurface(RayT<T>{V(0.5, 0.5, -2), V(0, 0, 1)}, T(0), T(10));
  const SurfaceHitT<T> fromSide  = tape.firstSurface(V(3, 0.5, 0.5), V(0.5, 0.5, 0.5));
  const SurfaceHitT<T> missing   = tape.firstSurface(RayT<T>{V(0.5, 0.5, -2), V(0, 0, -1)}, T(0), T(10));

  REQUIRE(fromBelow.m_hit.hit());
  REQUIRE(fromSide.m_hit.hit());
  CHECK(fromBelow.m_surface.m_surfaceLabel.value() == 1U);
  CHECK(fromSide.m_surface.m_surfaceLabel.value() == 3U);
  CHECK(!missing.m_hit.hit());
  CHECK(missing.m_surface.m_surface == SurfaceRef<T>::None);

  // A copy keeps the part labels.
  Pool       other(hostMemoryResource());
  const auto copy = tape.deepCopy(other);

  CHECK(copy.closestSurface(V(0.5, 0.5, 1.2)).m_surfaceLabel.value() == 2U);
}

TEMPLATE_TEST_CASE("closestSurface and firstSurface: any shape reports its part, and a union its member's",
                   "[SurfaceRef][Parts]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;
  using V = Vec3T<T>;

  const BoxSDF<T> box(V(-1, -1, -1), V(1, 1, 1));

  const SurfaceRef<T> s = closestSurface(box, V(0, 0, 1.5));

  CHECK(s.m_surface == 0U);
  CHECK(s.m_part == 5U);
  CHECK(closestSurface(SphereSDF<T>(V::zeros(), T(1)), V(2, 0, 0)).m_part == 0U);

  const SurfaceHitT<T> h = firstSurface(box, RayT<T>{V(5, 0, 0), V(-1, 0, 0)}, T(0), T(10));

  REQUIRE(h.m_hit.hit());
  CHECK(h.m_surface.m_part == 1U);

  // A union of boxes reports the member and its face; a tape over it, the same, after the leaves before it.
  Pool pool(hostMemoryResource());

  const std::vector<BoxSDF<T>> boxes{BoxSDF<T>(V(4, -1, -1), V(6, 1, 1)), box};

  const BVHUnion<T, 4, BoxSDF<T>> scene(pool, boxes);

  const SurfaceRef<T> u = scene.closestSurface(V(0, -1.5, 0));

  CHECK(u.m_surface == 1U);
  CHECK(u.m_part == 2U);

  const Tape<T, BVHUnion<T, 4, BoxSDF<T>>> tape(pool, Union(SphereSDF<T>(V(0, 10, 0), T(1)), toExpr(scene)));

  const SurfaceRef<T> t = tape.closestSurface(V(5, 0, 1.5));

  CHECK(t.m_surface == 1U);
  CHECK(t.m_part == 5U);
}

TEMPLATE_TEST_CASE(
  "SurfaceLabel: face groups reject a composition, a wrong count and an unnamed group; ExprIndex a part out of range",
  "[FaceGroups]",
  EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;
  using V = Vec3T<T>;

  FaceGroups six;

  six.m_names = {"a"};
  six.m_group = std::vector<uint32_t>(6, 0U);

  REQUIRE(abortsWith([six] { (void)SurfaceLabel(Translate(BoxSDF<T>(V(-1, -1, -1), V(1, 1, 1)), V(1, 0, 0)), six); },
                     "SurfaceLabel: face groups label the parts of one shape, not of a composition (label it first)"));
  REQUIRE(abortsWith(
    [] {
      Pool                            pool(hostMemoryResource());
      const BVHUnion<T, 4, BoxSDF<T>> boxes(pool, std::vector<BoxSDF<T>>{BoxSDF<T>(V(-1, -1, -1), V(1, 1, 1))});

      FaceGroups one;

      one.m_names = {"a"};
      one.m_group = {0U};

      (void)SurfaceLabel(boxes, one);
    },
    "SurfaceLabel: face groups label the parts of one shape, not of a collection (a union, a cloud, a tape)"));
  REQUIRE(abortsWith([six] { (void)SurfaceLabel(CylinderSDF<T>(V(0, 0, 0), V(0, 0, 1), T(1)), six); },
                     "SurfaceLabel: the face groups name 6 parts of a shape with 3"));

  FaceGroups wrong = six;

  wrong.m_group[2] = 4U;

  REQUIRE(abortsWith([wrong] { (void)SurfaceLabel(BoxSDF<T>(V(-1, -1, -1), V(1, 1, 1)), wrong); },
                     "SurfaceLabel: part 2 is in group 4 of 1"));
  REQUIRE(abortsWith(
    [six] {
      const ExprIndex<T> index(SurfaceLabel(BoxSDF<T>(V(-1, -1, -1), V(1, 1, 1)), six));

      (void)index.getPartSurfaceLabel(0, 6);
    },
    "ExprIndex: part 6 of 6"));
}

TEMPLATE_TEST_CASE("Tape: part labels on a device match the host", "[Tape][gpu]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using V    = Vec3T<T>;
  using Mesh = TriMeshSDF<T, BVH::DefaultBranchingRatio<T>(), TriangleSoA::DefaultWidth<T>()>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  Pool pool(hostMemoryResource());

  const auto soup = Parser::readIntoPolygonSoup<T>(dataFile("cube_groups.obj"));
  const Mesh cube = Parser::readIntoTriMeshSDF<T>(dataFile("cube_groups.obj"), pool);

  const Tape<T> tape(
    pool,
    Difference(SurfaceLabel(cube, soup.getFaceGroups()), CylinderSDF<T>(V(0.5, 0.5, -1), V(0.5, 0.5, 2), T(0.2))));

  pool.freeze();

  const Pool device = Pool::mirror(pool, deviceTestResource());

  std::vector<V> points;

  for (int i = 0; i < 40; i++) {
    for (int j = 0; j < 10; j++) {
      points.emplace_back(T(-0.3) + T(0.04) * T(i), T(0.5), T(-0.3) + T(0.16) * T(j));
    }
  }

  const auto onDevice = evaluateOnDevice<uint32_t>(PartLabelQuery<T, Tape<T>>{tape.rebasedView(device)}, points);
  const auto onHost   = evaluateOnHost<uint32_t>(PartLabelQuery<T, Tape<T>>{tape}, points);

  size_t differ = 0;

  for (size_t i = 0; i < points.size(); i++) {
    differ += (onDevice[i] != onHost[i]) ? 1U : 0U;
  }

  // Where two faces are as close, the device's rounding may pick the other.
  CHECK(differ <= 8U);
}
