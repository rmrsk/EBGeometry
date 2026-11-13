// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for the .ebg language (EBGeometry_Language.hpp): every shape and operation against the same expression
// built in C++, arithmetic, params and overrides, regions and materials, domains with their subdomains, properties and
// surface groups, couplings, includes and setups, meshes and their face groups, errors at their line and
// column (none aborts), the printer's canonical form, a domain's completeness, and tables read on a device.

#include "Source/EBGeometry_Language.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <random>
#include <regex>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

using namespace EBGeometry;

namespace {

// Points on a grid around the origin, off the axes.
template <class T>
std::vector<Vec3T<T>>
gridPoints(const T a_half, const int a_n)
{
  std::vector<Vec3T<T>> points;

  for (int i = 0; i < a_n; i++) {
    for (int j = 0; j < a_n; j++) {
      for (int k = 0; k < a_n; k++) {
        const auto at = [&](const int a_i) { return -a_half + T(2) * a_half * (T(a_i) + T(0.37)) / T(a_n); };

        points.emplace_back(at(i), at(j), at(k));
      }
    }
  }

  return points;
}

// The first error of a text, or "" when it reads.
template <class T>
Language::Diagnostic
firstError(const std::string& a_text)
{
  Pool pool(hostMemoryResource());

  try {
    (void)Language::parse<T>(a_text, "test.ebg", pool);
  } catch (const Language::Error& e) {
    return e.diagnostics().front();
  }

  return Language::Diagnostic{Language::Diagnostic::Severity::Warning, {}, ""};
}

// Whether two selectors hold the same conditions.
bool
sameSelector(const SurfaceSelector& a_first, const SurfaceSelector& a_second)
{
  return a_first.m_surface == a_second.m_surface && a_first.m_part == a_second.m_part &&
         a_first.m_regionA == a_second.m_regionA && a_first.m_regionB == a_second.m_regionB &&
         a_first.m_label == a_second.m_label && a_first.m_materialA == a_second.m_materialA &&
         a_first.m_materialB == a_second.m_materialB && a_first.m_rest == a_second.m_rest;
}

std::string
dataFile(const char* a_name)
{
  return std::string(EBGEOMETRY_TEST_DATA_DIR) + "/" + a_name;
}

} // namespace

TEMPLATE_TEST_CASE("Language: every shape and operation gives the expression the C++ builder gives",
                   "[Language]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;
  using V = Vec3T<T>;

  Pool pool(hostMemoryResource());

  // Each line: the language's text, and the same expression in C++.
  const std::vector<std::pair<std::string, Expr<T>>> cases{
    {"plane((0, 0, 0.1), (0, 0, 1))", PlaneSDF<T>(V(0, 0, T(0.1)), V(0, 0, 1))},
    {"sphere((0.1, 0, 0), 0.7)", SphereSDF<T>(V(T(0.1), 0, 0), T(0.7))},
    {"box((-0.5, -0.4, -0.3), (0.5, 0.4, 0.3))", BoxSDF<T>(V(T(-0.5), T(-0.4), T(-0.3)), V(T(0.5), T(0.4), T(0.3)))},
    {"torus((0, 0, 0), 0.6, 0.2)", TorusSDF<T>(V(0, 0, 0), T(0.6), T(0.2))},
    {"cylinder((0, 0, -0.5), (0, 0, 0.5), 0.4)", CylinderSDF<T>(V(0, 0, T(-0.5)), V(0, 0, T(0.5)), T(0.4))},
    {"infinite_cylinder((0, 0, 0), 0.3, \"y\")", InfiniteCylinderSDF<T>(V(0, 0, 0), T(0.3), 1)},
    {"capsule((0, 0, -0.5), (0, 0, 0.5), 0.3)", CapsuleSDF<T>(V(0, 0, T(-0.5)), V(0, 0, T(0.5)), T(0.3))},
    {"infinite_cone((0, 0, 0.5), 30)", InfiniteConeSDF<T>(V(0, 0, T(0.5)), T(30))},
    {"cone((0, 0, 0.5), 1, 30)", ConeSDF<T>(V(0, 0, T(0.5)), T(1), T(30))},
    {"rounded_box((0, 0, 0), (1, 0.8, 0.6), 0.1)", RoundedBoxSDF<T>(V(0, 0, 0), V(1, T(0.8), T(0.6)), T(0.1))},
    {"perlin(0.5, (1, 1, 1), 0.5, 2)", PerlinSDF<T>(T(0.5), V(1, 1, 1), T(0.5), 2U)},
    {"rounded_cylinder((0, 0, 0), 0.4, 0.1, 1)", RoundedCylinderSDF<T>(V(0, 0, 0), T(0.4), T(0.1), T(1))},
    {"ellipsoid((0, 0, 0), (0.6, 0.4, 0.3))", EllipsoidSDF<T>(V(0, 0, 0), V(T(0.6), T(0.4), T(0.3)))},
    {"capped_cone((0, 0, -0.5), (0, 0, 0.5), 0.5, 0.2)",
     CappedConeSDF<T>(V(0, 0, T(-0.5)), V(0, 0, T(0.5)), T(0.5), T(0.2))},
    {"round_cone((0, 0, -0.5), (0, 0, 0.5), 0.4, 0.2)",
     RoundConeSDF<T>(V(0, 0, T(-0.5)), V(0, 0, T(0.5)), T(0.4), T(0.2))},
    {"polygon_prism((0, 0, -0.5), (0, 0, 0.5), 0.5, 6, (1, 0, 0))",
     PolygonPrismSDF<T>(V(0, 0, T(-0.5)), V(0, 0, T(0.5)), T(0.5), 6U, V(1, 0, 0))},
    {"tube((0, 0, -0.5), (0, 0, 0.5), 0.2, 0.5)", TubeSDF<T>(V(0, 0, T(-0.5)), V(0, 0, T(0.5)), T(0.2), T(0.5))},
    {"capped_torus((0, 0, 0), 0.6, 0.2, 120)", CappedTorusSDF<T>(V(0, 0, 0), T(0.6), T(0.2), T(120))},
    {"cut_sphere((0, 0, 0), 0.6, 0.2)", CutSphereSDF<T>(V(0, 0, 0), T(0.6), T(0.2))},
    {"cut_hollow_sphere((0, 0, 0), 0.6, 0.2, 0.05)", CutHollowSphereSDF<T>(V(0, 0, 0), T(0.6), T(0.2), T(0.05))},
    {"wedge((0, 0, 0), 60)", WedgeSDF<T>(V(0, 0, 0), T(60))}};

  const SphereSDF<T> a(V(T(-0.2), 0, 0), T(0.5));
  const BoxSDF<T>    b(V(0, T(-0.3), T(-0.3)), V(T(0.6), T(0.3), T(0.3)));

  const std::string ab = "a = sphere((-0.2, 0, 0), 0.5)\nb = box((0, -0.3, -0.3), (0.6, 0.3, 0.3))\n";

  // Operations on a and b; keyword and positional forms, and the method form, mixed.
  const std::vector<std::pair<std::string, Expr<T>>> operations{
    {"union(a, b)", Union(std::vector<Expr<T>>{a, b})},
    {"bvh_union(a, b)", Union(std::vector<Expr<T>>{a, b}, Accelerate::BVH)},
    {"smooth_union(a, b, smooth = 0.1)", SmoothUnion(std::vector<Expr<T>>{a, b}, T(0.1))},
    {"a.smooth_union(b, smooth = 0.1, blend = \"exponential\", bvh = true)",
     SmoothUnion(std::vector<Expr<T>>{a, b}, T(0.1), BlendKind::Exponential, Accelerate::BVH)},
    {"intersection(a, b)", Intersection(std::vector<Expr<T>>{a, b})},
    {"smooth_intersection(a, b, smooth = 0.1)", SmoothIntersection(std::vector<Expr<T>>{a, b}, T(0.1))},
    {"difference(a, b)", Difference(toExpr(a), std::vector<Expr<T>>{b})},
    {"bvh_difference(a, b)", Difference(toExpr(a), std::vector<Expr<T>>{b}, Accelerate::BVH)},
    {"smooth_difference(a, b, smooth = 0.1)", SmoothDifference(toExpr(a), std::vector<Expr<T>>{b}, T(0.1))},
    {"complement(a)", Complement(a)},
    {"a.translate((0.1, 0.2, 0.3))", Translate(a, V(T(0.1), T(0.2), T(0.3)))},
    {"rotate(b, 30, \"z\")", Rotate(b, T(30), size_t(2))},
    {"b.rotate(degrees = 30, axis = (1, 1, 0))", Rotate(b, T(30), V(1, 1, 0))},
    {"scale(b, 1.5)", Scale(b, T(1.5))},
    {"reflect(a, \"x\")", Reflect(a, size_t(0))},
    {"offset(b, 0.1)", Offset(b, T(0.1))},
    {"annular(a, 0.05)", Annular(a, T(0.05))},
    {"elongate(a, (0.2, 0, 0))", Elongate(a, V(T(0.2), 0, 0))},
    {"blur(b, 0.1)", Blur(b, T(0.1))},
    {"mollify(b, 0.1, 3)", Mollify(b, T(0.1), 3)},
    {"repeat(a, (2, 2, 2), (1, 1, 1), (1, 1, 1))", FiniteRepetition(a, V(2, 2, 2), V(1, 1, 1), V(1, 1, 1))}};

  const auto points = gridPoints<T>(T(1.2), 7);

  const auto compare = [&](const std::string& a_text, const Expr<T>& a_expected) {
    INFO(a_text);

    const auto model = Language::parse<T>(a_text, "test.ebg", pool);

    const Tape<T> read(pool, model.object());
    const Tape<T> expected(pool, a_expected);

    for (const auto& p : points) {
      CHECK(read.signedDistance(p) == expected.signedDistance(p));
    }
  };

  for (const auto& [text, expected] : cases) {
    compare("ebg 0\nobject " + text + "\n", expected);
  }

  for (const auto& [text, expected] : operations) {
    // a and b are defined only where they are used.
    const bool usesA = std::regex_search(text, std::regex("\\ba\\b"));
    const bool usesB = std::regex_search(text, std::regex("\\bb\\b"));

    const std::string defineA = ab.substr(0, ab.find('\n') + 1);
    const std::string defineB = ab.substr(ab.find('\n') + 1);

    compare("ebg 0\n" + (usesA ? defineA : "") + (usesB ? defineB : "") + "object " + text + "\n", expected);
  }
}

TEMPLATE_TEST_CASE("Language: arithmetic, vectors, params and overrides", "[Language]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  const std::string text = R"(ebg 0
param r = 0.5
param c = (1, 2, 3)
d = 2 * r - -r / 2 + (3 - 1) * 0.25   # 1.75
v = (c + (1, 1, 1)) * 2 / 4 - c * 0    # (1, 1.5, 2)
w = -(1, 2, 3) + 2 * pi * (0, 0, 0)
object sphere(v, d)
)";

  const auto model = Language::parse<T>(text, "test.ebg", pool);

  REQUIRE(model.params().size() == 2U);
  CHECK(model.params()[0].m_name == "r");
  CHECK(model.params()[0].m_value == "0.5");
  CHECK(model.params()[0].m_type == "number");
  CHECK(model.params()[1].m_type == "vector");
  CHECK_FALSE(model.params()[0].m_overridden);

  const Tape<T> tape(pool, model.object());

  CHECK_THAT(tape.signedDistance(Vec3T<T>(1, T(1.5), 2)), withinAbsT<T>(T(-1.75), tightMargin<T>()));

  // An override replaces a param's value, written as in a file.
  const auto overridden = Language::parse<T>(text, "test.ebg", pool, {{"r", "1"}, {"c", "(0, 0, 0)"}});

  CHECK(overridden.params()[0].m_overridden);
  CHECK(overridden.params()[0].m_value == "1");

  const Tape<T> other(pool, overridden.object());

  // d = 2 - -0.5 + 0.5 = 3; v = (0.5, 0.5, 0.5).
  CHECK_THAT(other.signedDistance(Vec3T<T>(T(0.5), T(0.5), T(0.5))), withinAbsT<T>(T(-3), tightMargin<T>()));

  // Overriding a plain value, or a name the model does not have, is an error.
  CHECK_THROWS_WITH(Language::parse<T>(text, "test.ebg", pool, {{"d", "1"}}),
                    Catch::Matchers::ContainsSubstring("'d' is overridden, but it is not a param"));
  CHECK_THROWS_WITH(Language::parse<T>(text, "test.ebg", pool, {{"rr", "1"}}),
                    Catch::Matchers::ContainsSubstring("no param 'rr' (did you mean 'r'?)"));
}

TEMPLATE_TEST_CASE("Language: regions, materials and labels are named in the object's index",
                   "[Language]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  const std::string text = R"(ebg 0
air_box = box((-3, -2, -3), (5, 2, 3)).label(xmin = "inlet", xmax = "outlet", ymin = "far", ymax = "far")
cable = overlay {
  region jacket = tube((0, 0, -3), (0, 0, 3), 0.5, 0.8) material PTFE
  region core   = cylinder((0, 0, -3), (0, 0, 3), 0.5) material copper
}
object overlay {
  region air   = air_box material air
  region wires = cable
}
material copper { conductivity = 5.8e7 }
material PTFE { permittivity = 2.1 }
material air { permittivity = 1, density = 1.2, kind = "gas", flowing = true, velocity = (1, 0, 0) }
)";

  const auto model = Language::parse<T>(text, "test.ebg", pool);

  const ExprIndex<T>& index = model.index();

  REQUIRE(index.getNumRegions() == 4U);
  CHECK(index.findRegion("air") == 1U);
  CHECK(index.findRegion("jacket") == 2U);
  CHECK(index.findRegion("core") == 3U);
  CHECK(index.getRegionsOfRegionGroup(1) == std::vector<uint32_t>{2, 3});
  CHECK(index.getRegionGroup(1).m_name == "wires");

  REQUIRE(model.getNumMaterials() == 3U);

  const Language::Properties& air = model.materialProperties(index.getMaterialOf(1).value());

  CHECK(air.number("density") == 1.2);
  CHECK(air.string("kind") == "gas");
  CHECK(air.vector<T>("velocity") == Vec3T<T>(1, 0, 0));
  CHECK(air.flag("flowing"));
  CHECK_FALSE(air.flag("frozen", false));
  CHECK_THROWS_AS(air.flag("density"), Language::Error);
  CHECK_THROWS_AS(air.flag("frozen"), Language::Error);
  CHECK(air.number("viscosity", 1.8e-5) == 1.8e-5);
  CHECK(air.code("kind", {"liquid", "gas"}) == 1U);
  CHECK(air.spanOf("density").m_line == 13U);
  CHECK_THROWS_WITH(air.number("viscosity"),
                    Catch::Matchers::ContainsSubstring("material 'air' has no property 'viscosity'"));
  CHECK_THROWS_WITH(air.number("kind"), Catch::Matchers::ContainsSubstring("'kind' is a string, not a number"));
  CHECK_THROWS_WITH(air.code("kind", {"liquid", "solid"}),
                    Catch::Matchers::ContainsSubstring("\"gas\", which is none of: liquid, solid"));

  CHECK(index.findSurfaceLabel("inlet").value() == 0U);
  CHECK(index.findSurfaceLabel("far").value() == 2U);

  // The box's parts: xmin and xmax named, ymin and ymax both "far", zmin and zmax unnamed.
  REQUIRE(model.shapes().size() == 3U);
  CHECK(model.shapes()[0].m_shape == "box");
  CHECK(model.shapes()[0].m_partLabels == std::vector<std::string>{"inlet", "outlet", "far", "far", "", ""});

  // Each built-in shape's part names are as many as its parts.
  CHECK(Language::partNames("box").size() == BoxSDF<T>(Vec3T<T>::zeros(), Vec3T<T>::ones()).getNumParts());
  CHECK(Language::partNames("tube").size() ==
        TubeSDF<T>(Vec3T<T>::zeros(), Vec3T<T>::ones(), T(0.1), T(0.2)).getNumParts());
  CHECK(Language::partNames("polygon_prism", 5).size() ==
        PolygonPrismSDF<T>(Vec3T<T>::zeros(), Vec3T<T>::ones(), T(1), 5U, Vec3T<T>(1, 0, 0)).getNumParts());
  CHECK(Language::partNames("sphere").empty());
}

TEMPLATE_TEST_CASE("Language: domains, their subdomains with properties, surface groups and couplings",
                   "[Language]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  const std::string text = R"(ebg 0
air_box = box((-3, -2, -3), (5, 2, 3)).label(xmin = "inlet", xmax = "outlet")
object overlay {
  region air    = air_box material air
  region jacket = tube((0, 0, -3), (0, 0, 3), 0.5, 0.8) material PTFE
  region core   = cylinder((0, 0, -3), (0, 0, 3), 0.5) material copper
}
material copper { conductivity = 5.8e7 }
material PTFE { permittivity = 2.1, density = 2200 }
material air { permittivity = 1, density = 1.2 }

domain electrostatics {
  subdomain insulation = jacket
  subdomain gas = air { permittivity = 1.0006 }
  group conductor = interface(core, jacket) { potential = 1000 }
  group dielectric = interface(jacket, air)
  group ground = label("inlet"), label("outlet") { potential = 0 }
  group rest = remaining()
}
domain heat {
  subdomain solid = jacket, core { density = 5000 }
  group outer = materials(air, PTFE), region(outside)
}
domain cfd {
  subdomain fluid = air
  group walls = interface(air, jacket)
  group inflow = label("inlet", between = (air, outside))
}
couple body_force = volume(electrostatics.gas, cfd.fluid)
couple charge = surface(electrostatics.dielectric, cfd.walls) { transfer = "charge" }
)";

  const auto model = Language::parse<T>(text, "test.ebg", pool);

  REQUIRE(model.getNumDomains() == 3U);

  const Language::ModelDomain& es = model.domain("electrostatics");

  REQUIRE(es.getNumSubdomains() == 2U);
  CHECK(es.subdomainName(1) == "gas");
  CHECK(es.findSubdomain("insulation") == 0U);
  CHECK(es.setup().getRegions(0) == std::vector<uint32_t>{2});
  CHECK(es.setup().getRegions(1) == std::vector<uint32_t>{1});

  // A subdomain's properties: its own over its regions' material's.
  CHECK(es.subdomainProperties(1).number("permittivity") == 1.0006);
  CHECK(es.subdomainProperties(1).number("density") == 1.2);
  CHECK(es.subdomainProperties(0).number("density") == 2200);

  // Two materials agreeing on nothing the subdomain sets: the subdomain's own wins, the others merge.
  const Language::ModelDomain& heat = model.domain("heat");

  CHECK(heat.subdomainProperties(0).number("density") == 5000);
  CHECK(heat.subdomainProperties(0).number("conductivity") == 5.8e7);
  CHECK(heat.subdomainProperties(0).number("permittivity") == 2.1);

  // Surface groups: their selectors, in order, against the C++ selectors.
  REQUIRE(es.getNumSurfaceGroups() == 4U);
  CHECK(es.findSurfaceGroup("ground") == 2U);
  CHECK(es.surfaceGroupProperties(0).number("potential") == 1000);
  REQUIRE(es.setup().getNumSelectors() == 5U);
  CHECK(sameSelector(es.setup().getSelector(0), SurfaceSelector::interface(3, 2)));
  CHECK(sameSelector(es.setup().getSelector(1), SurfaceSelector::interface(2, 1)));
  CHECK(sameSelector(es.setup().getSelector(2), SurfaceSelector::label(SurfaceLabelId(0))));
  CHECK(sameSelector(es.setup().getSelector(3), SurfaceSelector::label(SurfaceLabelId(1))));
  CHECK(sameSelector(es.setup().getSelector(4), SurfaceSelector::rest()));
  CHECK(es.setup().getSurfaceGroupOfSelector(3) == 2U);
  CHECK(sameSelector(
    heat.setup().getSelector(0),
    SurfaceSelector::materials(model.index().findMaterialLabel("air"), model.index().findMaterialLabel("PTFE"))));
  CHECK(sameSelector(heat.setup().getSelector(1), SurfaceSelector::region(0)));

  // A label narrowed to the surfaces between two regions.
  SurfaceSelector inflow = SurfaceSelector::label(SurfaceLabelId(0));

  inflow.m_regionA = 1;
  inflow.m_regionB = 0;

  CHECK(sameSelector(model.domain("cfd").setup().getSelector(1), inflow));

  // Each leaf of the object knows the shape call it comes from.
  REQUIRE(model.shapes().size() == 3U);
  CHECK(model.siteOfLeaf(0)->m_shape == "box");
  CHECK(model.siteOfLeaf(1)->m_shape == "tube");
  CHECK(model.siteOfLeaf(2)->m_shape == "cylinder");
  CHECK(model.siteOfLeaf(2)->m_span.m_line == 6U);
  CHECK(model.siteOfLeaf(3) == nullptr);

  REQUIRE(model.couplings().size() == 2U);
  CHECK(model.couplings()[0].m_kind == Language::Coupling::Kind::Volume);
  CHECK(model.couplings()[1].m_partB == "walls");
  CHECK(model.couplings()[1].m_properties.string("transfer") == "charge");

  CHECK_THROWS_WITH(model.domain("flow"), Catch::Matchers::ContainsSubstring("no domain 'flow'"));

  // A domain built from the setup for the model's tape answers as the setup does.
  const Tape<T> tape(pool, model.object());
  const Domain  domain(pool, tape, es.setup());

  CHECK(domain.subdomainAt(Vec3T<T>(T(0.65), 0, 0)) == 0U);
  CHECK(domain.subdomainAt(Vec3T<T>(2, 0, 0)) == 1U);
  CHECK(domain.subdomainAt(Vec3T<T>(0, 0, 0)) == Domain<T>::Excluded);

  // The names a viewer shows.
  const RegionNames names = model.names();

  CHECK(names.getNumDomains() == 3U);
  CHECK(names.domainName(2) == "cfd");
  CHECK(names.surface(0, false) == "box, test.ebg:2");
  CHECK(names.surface(2, false) == "cylinder, test.ebg:6");
}

TEMPLATE_TEST_CASE("Language: what review and fuzzing found reads as it should",
                   "[Language]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  // An overlay of several regions inside parentheses: its regions are still lines.
  const auto clipped = Language::parse<T>(R"(ebg 0
object intersection(overlay {
  region a = sphere((0, 0, 0), 1)
  region b = sphere((0.5, 0, 0), 0.5)
}, box((-2, -2, -2), (0, 2, 2)))
)",
                                          "test.ebg",
                                          pool);

  CHECK(clipped.index().getNumRegions() == 3U);

  // Spans end at what they hold, not at a line end skipped inside parentheses.
  const auto spans = Language::parse<T>("ebg 0\nr = 1\nobject sphere((0, 0, 0),\n  r\n)\n", "test.ebg", pool);

  const Language::Syntax::Node& radius = spans.sources()[0].m_statements[1].m_expr.m_children[1];

  CHECK(radius.m_span.m_end - radius.m_span.m_begin == 1U);

  // Numbers too large to be what they count.
  CHECK_THAT(firstError<T>("ebg 4294967296\n").m_message, Catch::Matchers::ContainsSubstring("is unknown"));
  CHECK_THAT(firstError<T>("ebg 0\nobject polygon_prism((0, 0, 0), (0, 0, 1), 1, 4294967299, (1, 0, 0))\n").m_message,
             Catch::Matchers::ContainsSubstring("a whole number from 0 to 1000000"));
  CHECK_THAT(firstError<T>("ebg 0\nobject sphere((0, 0, 0), 1e300 * 1e300)\n").m_message,
             Catch::Matchers::ContainsSubstring("the result of '*' is too large"));
  CHECK_THAT(firstError<T>("ebg 0\nmaterial m { k = (0, 1e300, 0) * 1e300 }\nobject sphere((0, 0, 0), 1)\n").m_message,
             Catch::Matchers::ContainsSubstring("the result of '*' is too large"));

  for (const char* text : {"ebg 0\nobject sphere((0, 0, 0), 1e39)\n",
                           "ebg 0\nobject sphere((0, 1e39, 0), 1)\n",
                           "ebg 0\nobject rotate(sphere((0, 0, 0), 1), 30, (1e39, 0, 0))\n"}) {
    CAPTURE(text);

    if (std::is_same<T, float>::value) {
      CHECK_THAT(firstError<T>(text).m_message,
                 Catch::Matchers::ContainsSubstring("is too large for a model read in single precision"));
    }
    else {
      CHECK(firstError<T>(text).m_message.empty());
    }
  }

  // Line ends where the reference continues a statement: after '=' and a leading '-', before a method of a label
  // statement, and between a keyword and its '=' inside parentheses.
  const auto continued = Language::parse<T>("ebg 0\nr =\n  -\n  1\nball = sphere((0, 0, 0), -r)\nball\n  "
                                            ".label(\"skin\")\nobject smooth_union(ball, sphere((1, 0, 0), 1), smooth\n"
                                            "  = 0.1)\n",
                                            "test.ebg",
                                            pool);

  CHECK(continued.index().getSurfaceLabelName(continued.index().getSurfaceLabel(0)) == "skin");

  // What the grammar does not allow, and a number no double holds.
  CHECK_THAT(firstError<T>("ebg 0 r = 1\n").m_message, Catch::Matchers::ContainsSubstring("the version only"));
  CHECK_THAT(firstError<T>("ebg 0\nr = 1e999\n").m_message, Catch::Matchers::ContainsSubstring("too large"));
  CHECK_THAT(firstError<T>("ebg 0\nr = --1\n").m_message, Catch::Matchers::ContainsSubstring("two '-' in a row"));
  CHECK_THAT(firstError<T>("ebg 0\nobject sphere((0, 0, 0), 1,)\n").m_message,
             Catch::Matchers::ContainsSubstring("expected an argument after ','"));

  // A byte-order mark is skipped; a byte outside ASCII elsewhere is named by its value.
  CHECK(firstError<T>("\xEF\xBB\xBF"
                      "ebg 0\nobject sphere((0, 0, 0), 1)\n")
          .m_message.empty());
  CHECK_THAT(firstError<T>("ebg 0\nr\xC3\xA9 = 1\n").m_message, Catch::Matchers::ContainsSubstring("byte 0xC3"));
  CHECK_THAT(firstError<T>(std::string("ebg 0\nr = 1\0\n", 13)).m_message,
             Catch::Matchers::ContainsSubstring("control character 0x00"));
  CHECK_THAT(firstError<T>("ebg 0\nr = 1\x01\n").m_message,
             Catch::Matchers::ContainsSubstring("control character 0x01"));

  // Every error is found when the statements are indented, and after an error inside parentheses.
  for (const char* text : {"ebg 0\n  a = 1 1\n  b = 2 2\n  c = 3 3\n", "ebg 0\na = (1,\n2 2)\nb = 2 2\nc = 3 3\n"}) {
    INFO(text);

    size_t errors = 0;

    try {
      (void)Language::parse<T>(text, "test.ebg", pool);
    } catch (const Language::Error& e) {
      errors = e.diagnostics().size();
    }

    CHECK(errors == 3U);
  }

  // A mesh's face groups whose names are not names of the language: its parts are named so that they can be.
  const auto mesh = Language::read<T>(dataFile("language/odd_groups/model.ebg"), pool);

  CHECK(mesh.shapes()[0].m_parts == std::vector<std::string>{"bottom", "_2top", "side_wall"});
  CHECK(mesh.shapes()[0].m_partLabels == std::vector<std::string>{"bottom", "2top", "outer"});

  // Every comment is kept, those inside what is printed on one line after it.
  const std::string commented =
    "ebg 0  # a\ns = sphere((0, 0, 0),  # b\n  1)\nmaterial m {\n  # c\n  d = 1  # e\n}\nobject "
    "overlay {\n  region r = s material m\n}\n# f\n";

  const std::string printed = Language::print(Language::parse<T>(commented, "test.ebg", pool));

  for (const char* comment : {"# a", "# b", "# c", "# e", "# f"}) {
    CHECK_THAT(printed, Catch::Matchers::ContainsSubstring(comment));
  }
}

TEMPLATE_TEST_CASE("Language: the view block warns about a key a viewer passes over",
                   "[Language]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  const std::string model = "ebg 0\nmaterial red\nmaterial blu\nobject overlay {\n  region r1 = sphere((0, 0, 0), 1) "
                            "material red\n  region r2 = sphere((2, 0, 0), 1) material blu\n}\n";

  // Every key a viewer reads, and a color for each kind of name: no warning.
  const auto clean = Language::parse<T>(model + "view { camera = (7, 5, 9), target = (1, 0, 0), up = (0, 1, 0), "
                                                "field_of_view = 30, orthographic = false, background = (1, 1, 1), "
                                                "sky = false, floor = false, color.red = (1, 0, 1), "
                                                "color.r2 = (0, 0, 1) }\n",
                                        "test.ebg",
                                        pool);

  CHECK(clean.warnings().empty());

  // Each mistake a warning at its line, pointing to the key meant; the model still reads.
  const auto wrong = Language::parse<T>(model + "view {\n  red.colorr = (1, 0, 1)\n  blue.color = (0, 0, 1)\n"
                                                "  colr.blu = (0, 0, 1)\n  color.green = (0, 1, 0)\n"
                                                "  camra = (7, 5, 9)\n}\n",
                                        "test.ebg",
                                        pool);

  const auto& warnings = wrong.warnings();

  REQUIRE(warnings.size() == 5U);

  CHECK(warnings[0].m_span.m_line == 9U);
  CHECK_THAT(warnings[0].m_message,
             Catch::Matchers::ContainsSubstring("'red.colorr' is no view key (did you mean 'color.red'?)"));
  CHECK_THAT(warnings[0].m_message, !Catch::Matchers::ContainsSubstring("nothing is named"));

  CHECK(warnings[1].m_span.m_line == 10U);
  CHECK_THAT(warnings[1].m_message, Catch::Matchers::ContainsSubstring("did you mean 'color.blue'?"));
  CHECK_THAT(warnings[1].m_message,
             Catch::Matchers::ContainsSubstring("nothing is named 'blue' (did you mean 'blu'?)"));

  CHECK_THAT(warnings[2].m_message,
             Catch::Matchers::ContainsSubstring("'colr.blu' is no view key (did you mean 'color.blu'?)"));

  CHECK_THAT(warnings[3].m_message, Catch::Matchers::ContainsSubstring("'color.green' colors nothing"));

  CHECK_THAT(warnings[4].m_message,
             Catch::Matchers::ContainsSubstring("'camra' is no view key (did you mean 'camera'?)"));
}

TEMPLATE_TEST_CASE("Language: deep models, files read twice, meshes used twice and empty selectors",
                   "[Language]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  // A shape built deeper through named values than an index or a tape can recurse: an error, not a crash.
  {
    std::string text = "ebg 0\ns0 = sphere((0, 0, 0), 1)\n";

    for (int i = 1; i <= 5000; i++) {
      text += "s" + std::to_string(i) + " = union(s" + std::to_string(i - 1) + ", sphere((0, 0, 0), 1))\n";
    }

    text += "object s5000\n";

    CHECK_THAT(firstError<T>(text).m_message, Catch::Matchers::ContainsSubstring("operations deep"));
  }

  // Calls nested deeper than a tape allows, through named values: reported at the call, when the model is read.
  {
    std::string text = "ebg 0\nb0 = sphere((0, 0, 0), 1)\n";

    for (unsigned i = 1; i <= Tape<T>::MaxCallDepth + 1; i++) {
      text += "b" + std::to_string(i) + " = blur(b" + std::to_string(i - 1) + ", 0.01)\n";
    }

    text += "object b" + std::to_string(Tape<T>::MaxCallDepth + 1) + "\n";

    const Language::Diagnostic d = firstError<T>(text);

    CHECK_THAT(d.m_message, Catch::Matchers::ContainsSubstring("a tape allows at most"));
    CHECK(d.m_span.m_line == Tape<T>::MaxCallDepth + 3U);
  }

  // A label statement's arguments are read where the shape is defined, and its error says so.
  CHECK_THAT(
    firstError<T>("ebg 0\nair = box((0, 0, 0), (1, 1, 1))\nname = \"in\"\nair.label(xmin = name)\nobject air\n")
      .m_message,
    Catch::Matchers::ContainsSubstring("arguments are read where 'air' is defined"));

  // Selectors: an interface of a region with itself selects nothing (a warning); remaining() is one group's.
  const std::string regions = "ebg 0\nobject overlay {\n  region a = box((-1, -1, -1), (1, 1, 1))\n  region b = "
                              "sphere((0, 0, 0), 0.5)\n}\n";

  const auto empty =
    Language::parse<T>(regions + "domain d {\n  subdomain s = a\n  group g = interface(a, a)\n}\n", "test.ebg", pool);

  REQUIRE(empty.warnings().size() == 1U);
  CHECK_THAT(empty.warnings()[0].m_message, Catch::Matchers::ContainsSubstring("selects nothing"));
  CHECK_THAT(
    firstError<T>(regions + "domain d {\n  subdomain s = a\n  group g = remaining()\n  group h = remaining()\n}\n")
      .m_message,
    Catch::Matchers::ContainsSubstring("remaining() is already in a group of domain 'd'"));

  const auto between =
    Language::parse<T>("ebg 0\nobject overlay {\n  region a = box((-1, -1, -1), (1, 1, 1)).label(\"skin\")\n"
                       "  region b = sphere((0, 0, 0), 0.5)\n}\ndomain d {\n  subdomain s = a\n"
                       "  group g = label(\"skin\", between = (a, a))\n}\n",
                       "test.ebg",
                       pool);

  REQUIRE(between.warnings().size() == 1U);
  CHECK_THAT(between.warnings()[0].m_message, Catch::Matchers::ContainsSubstring("selects nothing"));

  // A polygon has at most 4096 sides, each a named part.
  CHECK_THAT(firstError<T>("ebg 0\nobject polygon_prism((0, 0, 0), (0, 0, 1), 1, 4097, (1, 0, 0))\n").m_message,
             Catch::Matchers::ContainsSubstring("from 3 to 4096 sides"));

  // One shape labeled in two values: its site lists the labels it is used with, the same part's two labels
  // included; a labeled value never used labels nothing.
  const auto sites = [&](const std::string& a_values, const std::string& a_object) {
    return Language::parse<T>(
             "ebg 0\na = box((0, 0, 0), (1, 1, 1))\n" + a_values + "object " + a_object + "\n", "test.ebg", pool)
      .shapes();
  };
  const auto partOf = [](const Language::ShapeSite& a_site, const std::string& a_part) {
    return size_t(std::find(a_site.m_parts.begin(), a_site.m_parts.end(), a_part) - a_site.m_parts.begin());
  };

  const auto two = sites("b = a.label(xmin = \"p\")\nc = a.label(xmax = \"q\")\n", "union(b, c)");

  REQUIRE(two.size() == 1U);
  CHECK(two[0].m_partLabeled[partOf(two[0], "xmin")]);
  CHECK(two[0].m_partLabels[partOf(two[0], "xmin")] == "p");
  CHECK(two[0].m_partLabels[partOf(two[0], "xmax")] == "q");

  const auto same = sites("b = a.label(xmin = \"p\")\nc = a.label(xmin = \"q\")\n", "union(b, c)");

  CHECK(same[0].m_partLabels[partOf(same[0], "xmin")] == "p,q");

  const auto unused = sites("b = a.label(\"p\", xmin = \"q\")\n", "a");

  CHECK(unused[0].m_label.empty());
  CHECK(!unused[0].m_partLabeled[partOf(unused[0], "xmin")]);

  // A property too large for the precision it is read in.
  const auto large =
    Language::parse<T>("ebg 0\nmaterial m { v = (0, 1e300, 0) }\nobject overlay {\n  region r = sphere((0, 0, 0), 1) "
                       "material m\n}\n",
                       "test.ebg",
                       pool);

  if (std::is_same<T, float>::value) {
    CHECK_THROWS_AS(large.materialProperties(0).template vector<T>("v"), Language::Error);
  }
  else {
    CHECK(large.materialProperties(0).template vector<T>("v")[1] == T(1e300));
  }

  // Files on disk: a folder of their own.
  namespace fs = std::filesystem;

  const fs::path folder = fs::temp_directory_path() / ("ebgeometry-language-" + std::to_string(sizeof(T)));

  fs::remove_all(folder);
  fs::create_directories(folder / "parts");
  fs::copy_file(dataFile("language/cube_odd_groups.obj"), folder / "parts" / "cube.obj");

  const auto write = [&](const fs::path& a_file, const std::string& a_text) {
    std::ofstream(a_file, std::ios::binary) << a_text;
  };

  // A file is read once, however often and by whatever path it is included: a second include is skipped, not a second
  // (or exponential) read, and its names are defined once.
  write(folder / "values.ebg", "ebg 0\nr = 1\n");
  write(folder / "twice.ebg",
        "ebg 0\ninclude(\"values.ebg\")\ninclude(\"./values.ebg\")\nobject sphere((0, 0, 0), r)\n");

  CHECK_NOTHROW(Language::read<T>((folder / "twice.ebg").string(), pool));

  // Setups sharing one geometry, combined: each includes the geometry and the materials, and a third includes both
  // setups and couples their domains.
  write(folder / "geometry.ebg",
        "ebg 0\nobject overlay {\n  region solid = box((0, 0, 0), (2, 1, 1)) material steel\n  region water = "
        "cylinder((0, 0.5, 0.5), (2, 0.5, 0.5), 0.2) material water\n}\n");
  write(folder / "materials.ebg", "ebg 0\nmaterial steel { density = 7850 }\nmaterial water { density = 1000 }\n");
  write(
    folder / "flow.ebg",
    "ebg 0\ninclude(\"geometry.ebg\")\ninclude(\"materials.ebg\")\ndomain flow {\n  subdomain fluid = water\n  group "
    "wall = interface(water, solid)\n  group ends = remaining()\n}\n");
  write(
    folder / "heat.ebg",
    "ebg 0\ninclude(\"geometry.ebg\")\ninclude(\"materials.ebg\")\ndomain heat {\n  subdomain metal = solid\n  group "
    "cooled = interface(solid, water)\n  group rest = remaining()\n}\n");
  write(folder / "coupled.ebg",
        "ebg 0\ninclude(\"flow.ebg\")\ninclude(\"heat.ebg\")\ncouple wall = surface(heat.cooled, flow.wall)\n");

  const auto coupled = Language::read<T>((folder / "coupled.ebg").string(), pool);

  CHECK(coupled.getNumDomains() == 2U);
  CHECK(coupled.getNumMaterials() == 2U);
  CHECK(coupled.couplings().size() == 1U);
  CHECK(coupled.index().getNumRegions() == 3U);

  // A file including itself, through another, is still an error.
  write(folder / "loop_a.ebg", "ebg 0\ninclude(\"loop_b.ebg\")\nobject sphere((0, 0, 0), 1)\n");
  write(folder / "loop_b.ebg", "ebg 0\ninclude(\"loop_a.ebg\")\n");

  try {
    (void)Language::read<T>((folder / "loop_a.ebg").string(), pool);

    FAIL("an include loop reads");
  } catch (const Language::Error& e) {
    CHECK_THAT(e.diagnostics().front().m_message, Catch::Matchers::ContainsSubstring("includes itself"));
  }

  // A mesh used by two calls: each call is its leaf's site. Flattened, an included file's mesh path still reads.
  write(folder / "parts" / "cubes.ebg",
        "ebg 0\nfirst = mesh(\"cube.obj\", groups = false)\nsecond = mesh(\"./cube.obj\", groups = false)\n");
  write(folder / "main.ebg",
        "ebg 0\ninclude(\"parts/cubes.ebg\")\nobject overlay {\n  region a = first\n  region b = "
        "second.translate((3, 0, 0))\n}\n");

  const auto meshes = Language::read<T>((folder / "main.ebg").string(), pool);

  REQUIRE(meshes.siteOfLeaf(0) != nullptr);
  REQUIRE(meshes.siteOfLeaf(1) != nullptr);
  CHECK(meshes.siteOfLeaf(0)->m_span.m_line == 2U);
  CHECK(meshes.siteOfLeaf(1)->m_span.m_line == 3U);

  const std::string flattened = Language::print(meshes, true);

  CHECK_THAT(flattened, Catch::Matchers::ContainsSubstring("mesh(\"parts/cube.obj\""));
  CHECK(Language::parse<T>(flattened, (folder / "main.ebg").string(), pool).index().getNumRegions() == 3U);

  // A mesh with a topology defect (two tetrahedra sharing an edge: four faces on it) reads with a warning by default,
  // and is an error when the model asks for one.
  write(folder / "bad.obj",
        "v 0 0 0\nv 1 0 0\nv 0 1 0\nv 0 0 1\nv 0 -1 0\nv 0 0 -1\n"
        "f 1 3 2\nf 1 2 4\nf 1 4 3\nf 2 3 4\nf 1 2 5\nf 1 6 2\nf 1 5 6\nf 2 6 5\n");
  write(folder / "defect.ebg", "ebg 0\nobject mesh(\"bad.obj\")\n");
  write(folder / "strict.ebg", "ebg 0\nobject mesh(\"bad.obj\", on_defect = \"throw\")\n");

  CHECK_NOTHROW(Language::read<T>((folder / "defect.ebg").string(), pool));

  try {
    (void)Language::read<T>((folder / "strict.ebg").string(), pool);

    FAIL("a defect is an error with on_defect = \"throw\"");
  } catch (const Language::Error& e) {
    CHECK_THAT(e.diagnostics().front().m_message, Catch::Matchers::ContainsSubstring("more than two faces share"));
  }

  fs::remove_all(folder);
}

TEMPLATE_TEST_CASE("Language: a label statement labels a named shape wherever it is written",
                   "[Language]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  // Labeled before its definition, after it, and in a second statement; the shape used before the last.
  const auto model = Language::parse<T>(R"(ebg 0
air_box.label(xmin = "inlet")
air_box = box((-1, -1, -1), (1, 1, 1))
moved = air_box.translate((0, 0, 0))
air_box.label(xmax = "outlet", ymin = "far")
air_box.label("walls")
object moved
)",
                                        "test.ebg",
                                        pool);

  CHECK(model.shapes()[0].m_partLabels == std::vector<std::string>{"inlet", "outlet", "far", "", "", ""});

  const ExprIndex<T>& index = model.index();

  CHECK(index.getSurfaceLabelName(index.getPartSurfaceLabel(0, 0)) == "inlet");
  CHECK(index.getSurfaceLabelName(index.getPartSurfaceLabel(0, 1)) == "outlet");
  CHECK(index.getSurfaceLabelName(index.getSurfaceLabel(0)) == "walls");

  // It prints as written.
  CHECK_THAT(Language::print(model),
             Catch::Matchers::ContainsSubstring("\nair_box.label(xmax = \"outlet\", ymin = \"far\")\n"));

  // What it cannot label.
  const std::vector<std::pair<std::string, std::string>> errors{
    {"ebg 0\nb = box((0, 0, 0), (1, 1, 1)).label(xmin = \"a\")\nb.label(xmin = \"c\")\nobject b\n",
     "part 'xmin' is already labeled \"a\" at test.ebg:2"},
    {"ebg 0\nb = box((0, 0, 0), (1, 1, 1))\nb.label(\"a\")\nb.label(\"c\")\nobject b\n",
     "the shape is already labeled \"a\" at test.ebg:3"},
    {"ebg 0\nb = box((0, 0, 0), (1, 1, 1)).translate((1, 0, 0))\nb.label(xmin = \"a\")\nobject b\n",
     "'b' is not one shape made by a call, whose parts a label statement can name (it is a composition)"},
    {"ebg 0\nr = 1\nr.label(\"a\")\nobject sphere((0, 0, 0), r)\n", "(it is a number)"},
    {"ebg 0\nobject sphere((0, 0, 0), 1)\nball.label(\"a\")\n", "no value 'ball' to label"},
    {"ebg 0\nb = box((0, 0, 0), (1, 1, 1))\nb.translate((1, 0, 0))\nobject b\n",
     "a statement that starts with a name is 'name = value', or 'name.label(...)'"}};

  for (const auto& [text, message] : errors) {
    INFO(text);

    CHECK_THAT(firstError<T>(text).m_message, Catch::Matchers::ContainsSubstring(message));
  }
}

TEMPLATE_TEST_CASE(
  "Language: a domain is complete when every subdomain has properties and every bounding piece a group",
  "[Language]",
  EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  const std::string geometry = R"(ebg 0
object overlay {
  region air   = box((-2, -2, -2), (2, 2, 2)).label(xmin = "inlet") material air
  region solid = sphere((0, 0, 0), 1) material steel
}
material air { density = 1.2 }
)";

  // Complete: the fluid's boundary is the box (inlet, the rest of it) and the sphere.
  const auto complete = Language::parse<T>(geometry + R"(
domain cfd {
  subdomain fluid = air
  group inlet = label("inlet")
  group walls = interface(air, solid)
  group far = remaining()
}
)",
                                           "test.ebg",
                                           pool);

  const Tape<T> tape(pool, complete.object());

  const Language::Completeness yes = Language::completeness(complete, tape, "cfd");

  CHECK(yes.m_complete);
  CHECK(yes.m_findings.empty());

  // Incomplete: the box's other faces are in no group, and the solid's subdomain has no properties (steel is not
  // declared).
  const auto incomplete = Language::parse<T>(geometry + R"(
domain heat {
  subdomain fluid = air
  subdomain solid = solid
  group inlet = label("inlet")
}
)",
                                             "test.ebg",
                                             pool);

  const Tape<T> other(pool, incomplete.object());

  const Language::Completeness no = Language::completeness(incomplete, other, "heat");

  CHECK_FALSE(no.m_complete);

  bool properties = false;
  bool box        = false;

  for (const Language::Diagnostic& d : no.m_findings) {
    properties =
      properties || d.m_message.find("subdomain 'solid' of domain 'heat' has no properties") != std::string::npos;
    box = box || (d.m_message.find("the box at test.ebg:3 between 'air' and outside is partly in no surface group") !=
                    std::string::npos &&
                  d.m_severity == Language::Diagnostic::Severity::Error);
  }

  CHECK(properties);
  CHECK(box);

  // Steel was never declared: a warning, the model still reads.
  REQUIRE(incomplete.warnings().size() == 1U);
  CHECK_THAT(incomplete.warnings()[0].m_message,
             Catch::Matchers::ContainsSubstring("material 'steel' is not declared"));
}

TEMPLATE_TEST_CASE("Language: a domain and a coupling judge a shape by the parts of it between the regions",
                   "[Language]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  // A shape whose parts lie between different regions: a channel bored through a block, flush with its ends, its
  // side against the block and its caps (the inlet and the outlet) against the outside. Each domain covers what the
  // channel touches, and the two agree on its wall: complete, and the coupling holds, though a cap labeled "inlet"
  // could lie against the block and the side against the outside.
  const auto channel = Language::parse<T>(R"(ebg 0
block = box((0, 0, 0), (4, 1, 2))
channel = cylinder((0, 0.5, 1), (4, 0.5, 1), 0.25).label(cap1 = "inlet", cap2 = "outlet", side = "bore")
object overlay {
  region solid = block material steel
  region water = channel material water
}
material steel { conductivity = 45 }
material water { viscosity = 1e-3 }
domain flow {
  subdomain fluid = water
  group inlet = label("inlet")
  group outlet = label("outlet")
  group wall = interface(water, solid)
}
domain heat {
  subdomain metal = solid
  group cooled = interface(solid, water)
  group insulated = remaining()
}
couple wall_heat = surface(heat.cooled, flow.wall)
)",
                                          "test.ebg",
                                          pool);

  const Tape<T> bored(pool, channel.object());

  for (const char* domain : {"flow", "heat"}) {
    const Language::Completeness result = Language::completeness(channel, bored, domain);

    CAPTURE(domain);
    CHECK(result.m_complete);

    for (const Language::Diagnostic& d : result.m_findings) {
      CAPTURE(d.m_message);
      CHECK(false);
    }
  }
}

TEST_CASE("Language: every built-in shape and operation is used in the gallery model and has its line in the docs",
          "[Language]")
{
  namespace fs = std::filesystem;

  // The repository, from the test data in it.
  const fs::path repository = fs::path(EBGEOMETRY_TEST_DATA_DIR) / ".." / "..";

  const auto text = [](const fs::path& a_file) {
    std::ifstream     in(a_file);
    std::stringstream all;

    all << in.rdbuf();

    return all.str();
  };

  const std::string gallery    = text(repository / "Studio" / "Models" / "gallery" / "model.ebg");
  const std::string shapes     = text(repository / "Docs" / "Sphinx" / "source" / "LanguageShapes.rst");
  const std::string operations = text(repository / "Docs" / "Sphinx" / "source" / "LanguageOperations.rst");

  REQUIRE_FALSE(gallery.empty());
  REQUIRE_FALSE(shapes.empty());
  REQUIRE_FALSE(operations.empty());

  for (const std::string& name : Language::LanguageDetail::builtinNames()) {
    CAPTURE(name);

    // Called in the gallery (as name(...), or a.name(...) for a method), and named in a table of the docs.
    const std::regex called("(^|[^A-Za-z0-9_])" + name + "\\(");

    CHECK(std::regex_search(gallery, called));
    CHECK((shapes.find("``" + name + "``") != std::string::npos ||
           operations.find("``" + name + "``") != std::string::npos));
  }
}

TEMPLATE_TEST_CASE("Language: a setup file including its model and more; a mesh and its face groups",
                   "[Language]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  const auto model = Language::read<T>(dataFile("language/cubes/setup.ebg"), pool, {{"shift", "(0, 4, 0)"}});

  // setup.ebg, the model it includes and the file that includes (every file is read before any is evaluated), and the
  // mesh, once however often it is used.
  REQUIRE(model.files().size() == 4U);
  CHECK_THAT(model.files()[0], Catch::Matchers::EndsWith("setup.ebg"));
  CHECK_THAT(model.files()[1], Catch::Matchers::EndsWith("model.ebg"));
  CHECK_THAT(model.files()[2], Catch::Matchers::EndsWith("shapes.ebg"));
  CHECK_THAT(model.files()[3], Catch::Matchers::EndsWith("cube_groups.obj"));

  // A folder is no model: the file is named.
  try {
    (void)Language::read<T>(dataFile("language/cubes"), pool);

    FAIL("a folder reads");
  } catch (const Language::Error& e) {
    CHECK_THAT(e.diagnostics().front().m_message, Catch::Matchers::ContainsSubstring("is a folder"));
  }

  // The mesh's face groups are labels, one renamed; the second copy is unlabeled.
  const ExprIndex<T>& index = model.index();

  CHECK_FALSE(index.findSurfaceLabel("bottom").isNone());
  CHECK_FALSE(index.findSurfaceLabel("side").isNone());
  CHECK(index.findSurfaceLabel("walls").isNone());

  const Tape<T> tape(pool, model.object());

  // The second cube moved by the override.
  CHECK(tape.region(Vec3T<T>(T(0.5), T(4.5), T(0.5))) == 2U);
  CHECK(tape.region(Vec3T<T>(T(3.5), T(0.5), T(0.5))) == 0U);

  // The view block is the viewer's: colors by name.
  CHECK(model.view().template vector<double>("color.steel") == Vec3T<double>(0.6, 0.6, 0.7));
}

TEMPLATE_TEST_CASE("Language: errors are reported at their line and column, all of them, and none aborts",
                   "[Language]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  struct Case
  {
    const char* m_text;
    uint32_t    m_line;
    uint32_t    m_column;
    const char* m_message;
  };

  const std::vector<Case> cases{
    {"object sphere((0, 0, 0), 1)\n", 1, 1, "a file starts with its version line"},
    {"ebg 1\n", 1, 5, "version 1 is unknown"},
    {"ebg 0\nx = (1, 2\nobject x\n", 3, 1, "expected ',' between a vector's components"},
    {"ebg 0\nobject sphere((0, 0, 0), 1) $\n", 2, 29, "unexpected character '$'"},
    {"ebg 0\nobject sphre((0, 0, 0), 1)\n", 2, 8, "no shape or operation 'sphre' (did you mean 'sphere'?)"},
    {"ebg 0\nobject sphere((0, 0, 0))\n", 2, 8, "sphere needs 'radius'"},
    {"ebg 0\nobject sphere((0, 0, 0), 1, 2)\n", 2, 29, "too many arguments"},
    {"ebg 0\nobject sphere((0, 0, 0), radius = 1, radius = 2)\n", 2, 47, "'radius' is given twice"},
    {"ebg 0\nobject sphere((0, 0, 0), size = 1)\n", 2, 33, "sphere has no argument 'size'"},
    {"ebg 0\nobject sphere(1, 1)\n", 2, 15, "'center' is a vector"},
    {"ebg 0\nobject sphere((0, 0, 0), 1 / 0)\n", 2, 30, "division by zero"},
    // The library's own checks of a value: an error at the call, not an abort.
    {"ebg 0\nobject sphere((0, 0, 0), -1)\n", 2, 8, "sphere: SphereSDF: the radius must be finite and positive (-1)"},
    {"ebg 0\nobject sphere((0, 0, 0), 1).repeat((1, 1, 1), (-1, 0, 0), (1, 1, 1))\n",
     2,
     8,
     "repeat: FiniteRepetition: the low repetition counts must be finite and not negative"},
    {"ebg 0\nobject sphere((0, 0, 0), \"a\" + 1)\n", 2, 26, "cannot compute a string + a number"},
    {"ebg 0\nobject sphere((0, 0, 0), r)\n", 2, 26, "no value 'r' defined before this line"},
    {"ebg 0\nr = 1\nr = 2\nobject sphere((0, 0, 0), r)\n", 3, 1, "'r' is already defined at test.ebg:2"},
    {"ebg 0\nobject box((0, 0, 0), (1, 1, 1)).label(xmn = \"a\")\n",
     2,
     46,
     "box has no part 'xmn' (did you mean 'xmin'?)"},
    {"ebg 0\nobject translate(box((0, 0, 0), (1, 1, 1)), (1, 0, 0)).label(xmin = \"a\")\n",
     2,
     69,
     "only one shape's own parts can be named"},
    {"ebg 0\no = overlay {\n  region a = sphere((0, 0, 0), 1)\n}\nobject union(o, sphere((3, 0, 0), 1))\n",
     5,
     14,
     "an overlay (regions) cannot be an operand of union"},
    {"ebg 0\nobject overlay {\n  region a = sphere((0, 0, 0), 1)\n  region a = sphere((3, 0, 0), 1)\n}\n",
     4,
     10,
     "region 'a' is already named at test.ebg:3"},
    {"ebg 0\no = overlay {\n  region a = sphere((0, 0, 0), 1)\n}\nobject overlay {\n  region b = o\n  region c = "
     "translate(o, (3, 0, 0))\n}\n",
     7,
     14,
     "region 'a' (named at test.ebg:3) would be in the object twice"},
    {"ebg 0\nx = 1\n", 0, 0, "the model has no object"},
    {"ebg 0\nobject sphere((0, 0, 0), 1)\nobject sphere((3, 0, 0), 1)\n", 3, 1, "a model has one object"},
    {"ebg 0\nobject sphere((0, 0, 0), 1)\ninclude(\"missing.ebg\")\n", 3, 9, "cannot open"},
    {"ebg 0\nobject overlay {\n  region a = sphere((0, 0, 0), 1)\n}\ndomain d {\n  subdomain s = b\n}\n",
     6,
     17,
     "no region 'b'"},
    {"ebg 0\nobject overlay {\n  region a = sphere((0, 0, 0), 1)\n}\ndomain d {\n  group g = label(\"x\")\n}\n",
     6,
     19,
     "no surface label \"x\""},
    {"ebg 0\nobject overlay {\n  region a = sphere((0, 0, 0), 1)\n}\ndomain d {\n  subdomain s = a\n  "
     "subdomain t = a\n}\n",
     7,
     17,
     "region 'a' is already in subdomain 's' of domain 'd'"},
    {"ebg 0\nobject overlay {\n  region a = sphere((0, 0, 0), 1)\n}\ndomain d {\n  subdomain s = a\n  group s = "
     "remaining()\n}\n",
     7,
     9,
     "'s' is already a subdomain or group of domain 'd'"},
    {"ebg 0\nobject overlay {\n  region a = sphere((0, 0, 0), 1)\n}\ndomain d {\n  subdomain s = a\n}\ncouple c = "
     "volume(d.s, e.s)\n",
     8,
     24,
     "no domain 'e'"},
    {"ebg 0\nobject sphere((0, 0, 0), 1)\nmaterial m { a = 1, a = 2 }\n", 3, 21, "'a' is given twice"},
    {"ebg 0\nobject sphere((0, 0, 0), (1, 2))\n", 2, 26, "a pair (a, b) is a selector's regions only"},
    {"ebg 0\nobject overlay {\n  region a = sphere((0, 0, 0), 1).label(\"s\")\n}\ndomain d {\n  group g = label(\"s\", "
     "near = (a, outside))\n}\n",
     6,
     31,
     "a selector: label(\"name\") or label(\"name\", between = (region, region))"},
  };

  for (const Case& c : cases) {
    INFO(c.m_text);

    const Language::Diagnostic d = firstError<T>(c.m_text);

    CHECK(d.m_severity == Language::Diagnostic::Severity::Error);
    CHECK_THAT(d.m_message, Catch::Matchers::ContainsSubstring(c.m_message));
    CHECK(d.m_span.m_line == c.m_line);
    CHECK(d.m_span.m_column == c.m_column);
  }

  // A named object is also a value: labeled on a line of its own, used by later lines.
  {
    Pool pool(hostMemoryResource());

    const auto model = Language::parse<T>(
      "ebg 0\nobject ball = sphere((0, 0, 0), 1)\nball.label(\"skin\")\ncopy = ball.translate((3, 0, 0))\n",
      "test.ebg",
      pool);

    CHECK(model.index().getSurfaceLabelName(model.index().getSurfaceLabel(0)) == "skin");
    CHECK_THAT(Language::print(model), Catch::Matchers::ContainsSubstring("object ball = sphere((0, 0, 0), 1)\n"));
    CHECK_THAT(firstError<T>("ebg 0\nball = 1\nobject ball = sphere((0, 0, 0), 1)\n").m_message,
               Catch::Matchers::ContainsSubstring("'ball' is already defined at test.ebg:2"));
  }

  // A shape defined and not used is kept out of the object, not an error.
  {
    Pool pool(hostMemoryResource());

    const auto model =
      Language::parse<T>("ebg 0\ns = sphere((0, 0, 0), 1)\nobject sphere((3, 0, 0), 1)\n", "test.ebg", pool);

    CHECK(model.warnings().empty());
    CHECK(model.index().getNumLeaves() == 1U);
  }

  // Every error is reported, not only the first.
  Pool pool(hostMemoryResource());

  try {
    (void)Language::parse<T>(
      "ebg 0\na = sphre((0, 0, 0), 1)\nb = box((0, 0, 0))\nobject sphere((0, 0, 0), c)\n", "test.ebg", pool);

    FAIL("the model has errors");
  } catch (const Language::Error& e) {
    REQUIRE(e.diagnostics().size() == 3U);
    CHECK(e.diagnostics()[0].m_span.m_line == 2U);
    CHECK(e.diagnostics()[1].m_span.m_line == 3U);
    CHECK(e.diagnostics()[2].m_span.m_line == 4U);
    CHECK_THAT(std::string(e.what()),
               Catch::Matchers::StartsWith("test.ebg:2:5: error: no shape or operation 'sphre'"));
  }
}

TEMPLATE_TEST_CASE("Language: the printer writes the canonical form, which reads as the same model",
                   "[Language]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  const std::string text = R"(ebg 0

# A comment of its own.
param   r=0.5      # after code
v = ( 1,2 ,3 )*-(r+1)/2
s = sphere(center=(0,0,0),radius=r).translate(v)
  .label("ball")


b = box((-1, -1, -1), (1, 1, 1)).label(xmin = "a", xmax = "b", ymin = "c", ymax = "d", zmin = "e", zmax = "f")
object overlay {
  # the regions
  region outer = b material air
  region inner = s material "x" + "y"
}
)";

  // The material's name must be a name: the last line is an error; fix it and print.
  const std::string fixed = text.substr(0, text.find("material \"x\"")) + "material steel\n}\n";

  const auto model = Language::parse<T>(fixed, "test.ebg", pool);

  const std::string printed = Language::print(model);

  const std::string expected = R"(ebg 0

# A comment of its own.
param r = 0.5  # after code
v = (1, 2, 3) * -(r + 1) / 2
s = sphere(center = (0, 0, 0), radius = r).translate(v).label("ball")

b = box((-1, -1, -1), (1, 1, 1))
  .label(xmin = "a", xmax = "b", ymin = "c", ymax = "d", zmin = "e", zmax = "f")
object overlay {
  # the regions
  region outer = b material air
  region inner = s material steel
}
)";

  CHECK(printed == expected);

  // Printing is idempotent, and the printed model is the same model.
  const auto again = Language::parse<T>(printed, "test.ebg", pool);

  CHECK(Language::print(again) == printed);

  const Tape<T> a(pool, model.object());
  const Tape<T> b(pool, again.object());

  for (const auto& p : gridPoints<T>(T(1.5), 5)) {
    CHECK(a.signedDistance(p) == b.signedDistance(p));
  }

  // A setup and the model it includes flatten into one text.
  const auto setup     = Language::read<T>(dataFile("language/cubes/setup.ebg"), pool);
  const auto flattened = Language::print(setup, true);

  CHECK_THAT(flattened, Catch::Matchers::ContainsSubstring("cube = mesh("));
  CHECK_THAT(flattened, Catch::Matchers::ContainsSubstring("material steel"));
  CHECK_THAT(flattened, !Catch::Matchers::ContainsSubstring("include("));
  CHECK_THAT(Language::print(setup), Catch::Matchers::ContainsSubstring("include(\"model.ebg\")"));
}

TEMPLATE_TEST_CASE("Language: every model in the repository reads, and its canonical form is itself once printed",
                   "[Language]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // The models the tests, the examples and the docs rely on: a change to the language that breaks one shows here.
  const std::string root = std::string(EBGEOMETRY_TEST_DATA_DIR) + "/../..";

  for (const std::string file : {"Tests/data/language/cubes/setup.ebg",
                                 "Tests/data/language/wire/setup.ebg",
                                 "Examples/Language/cable/setup.ebg"}) {
    INFO(file);

    Pool pool(hostMemoryResource());

    const auto model = Language::read<T>(root + "/" + file, pool);

    // Printed whole, read again: the same object, and printing it again changes nothing.
    const std::string printed = Language::print(model, true);
    const auto        again   = Language::parse<T>(printed, root + "/" + file, pool);

    CHECK(Language::print(again) == printed);
    CHECK(again.getNumDomains() == model.getNumDomains());
    CHECK(again.index().getNumRegions() == model.index().getNumRegions());

    const Tape<T> a(pool, model.object());
    const Tape<T> b(pool, again.object());

    for (const auto& p : gridPoints<T>(T(3), 6)) {
      CHECK(a.signedDistance(p) == b.signedDistance(p));
    }
  }
}

TEST_CASE("Language: mangled models read or report errors, never crash or abort", "[Language]")
{
  using T = double;

  // The repository's models, mangled at random (a fixed seed): characters deleted, inserted, duplicated and swapped,
  // lines dropped, text cut short. Each must read or throw Language::Error; anything else (another exception, a crash,
  // an abort) fails.
  const std::string root = std::string(EBGEOMETRY_TEST_DATA_DIR) + "/../..";

  std::vector<std::string> texts;

  for (const char* file : {"Tests/data/language/wire/model.ebg",
                           "Tests/data/language/wire/setup.ebg",
                           "Examples/Language/cable/model.ebg",
                           "Examples/Language/cable/setup.ebg"}) {
    std::ifstream     in(root + "/" + file);
    std::stringstream text;

    text << in.rdbuf();

    texts.push_back(text.str());
  }

  // The whole cable in one file, so that mangling reaches the domains too.
  std::string setup = texts[3].substr(texts[3].find('\n'));

  setup.erase(setup.find("include(\"model.ebg\")"), std::string("include(\"model.ebg\")").size());

  texts.push_back(texts[2] + setup);

  const std::string alphabet = "(){}[],.=+-*/\"#\\\n abcxyz019_eE";

  std::mt19937 random(20261112U);

  const auto pick = [&random](const size_t a_n) { return a_n == 0 ? size_t(0) : size_t(random() % a_n); };

  size_t errors = 0;
  size_t read   = 0;

  for (int round = 0; round < 3000; round++) {
    std::string text = texts[pick(texts.size())];

    const int edits = 1 + int(pick(4));

    for (int e = 0; e < edits && !text.empty(); e++) {
      const size_t at = pick(text.size());

      switch (pick(6)) {
      case 0: {
        text.erase(at, 1 + pick(8));

        break;
      }
      case 1: {
        text.insert(at, 1, alphabet[pick(alphabet.size())]);

        break;
      }
      case 2: {
        text.insert(at, text.substr(at, 1 + pick(12)));

        break;
      }
      case 3: {
        std::swap(text[at], text[pick(text.size())]);

        break;
      }
      case 4: {
        const size_t line = text.find('\n', at);

        text.erase(at, line == std::string::npos ? std::string::npos : line - at);

        break;
      }
      default: {
        text.resize(at);

        break;
      }
      }
    }

    Pool pool(hostMemoryResource());

    try {
      (void)Language::parse<T>(text, root + "/Examples/Language/cable/fuzz.ebg", pool);

      read++;
    } catch (const Language::Error& e) {
      errors++;

      // Every diagnostic points into the text (or at no line, for the whole model).
      for (const Language::Diagnostic& d : e.diagnostics()) {
        if (d.m_span.m_file == root + "/Examples/Language/cable/fuzz.ebg") {
          CHECK(d.m_span.m_end <= text.size());
          CHECK(d.m_span.m_begin <= d.m_span.m_end);
        }
      }
    } catch (const std::exception& e) {
      INFO(text);
      FAIL("an exception that is not Language::Error: " << e.what());
    }
  }

  CHECK(read + errors == 3000U);
  CHECK(errors > 0U);

  // Deep nesting, long lines, and texts that are almost nothing.
  for (const std::string& text :
       {std::string("ebg 0\nx = ") + std::string(100000, '(') + "1",
        std::string("ebg 0\nx = ") + std::string(100000, '-') + "1\nobject sphere((0, 0, 0), x)\n",
        std::string("ebg 0\nobject ") + std::string(20000, 'a'),
        std::string("ebg"),
        std::string(""),
        std::string("\n\n\n"),
        std::string("ebg 0\n\""),
        std::string("ebg 0\nobject overlay {"),
        std::string("ebg 0\ndomain d {\n  group g = label(\"x\"")}) {
    Pool pool(hostMemoryResource());

    CHECK_THROWS_AS(Language::parse<T>(text, "deep.ebg", pool), Language::Error);
  }
}

namespace {

// A row of a solver's table.
template <class T>
struct Gas
{
  T        m_density;
  uint32_t m_kind;
};

// Reads a region's row on the device by the region the tape finds.
template <class T>
struct TableQuery
{
  Tape<T>                 m_tape;
  Language::Table<Gas<T>> m_table;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    const Gas<T>& row = m_table[m_tape.region(a_point)];

    return row.m_density + T(10) * T(row.m_kind);
  }
};

} // namespace

TEMPLATE_TEST_CASE("Language: tables of properties in the pool, read on a device by region",
                   "[Language][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  Pool pool(hostMemoryResource());

  const auto model = Language::parse<T>(R"(ebg 0
object overlay {
  region air   = box((-2, -2, -2), (2, 2, 2)) material air
  region solid = sphere((0, 0, 0), 1) material steel
}
material air { density = 1.2, kind = "gas" }
material steel { density = 7800, kind = "solid" }
domain heat {
  subdomain s = solid
}
)",
                                        "test.ebg",
                                        pool);

  const Tape<T> tape(pool, model.object());

  const auto fill = [](const Language::Properties& a_p) {
    return Gas<T>{T(a_p.number("density", 0)), a_p.has("kind") ? a_p.code("kind", {"gas", "solid"}) : 2U};
  };

  const Language::Table<Gas<T>> regions = model.template regions<Gas<T>>(pool, fill);
  const Language::Table<Gas<T>> volumes = model.domain("heat").template volumes<Gas<T>>(pool, fill);

  REQUIRE(regions.size() == 3U);
  CHECK(regions[0].m_kind == 2U);
  CHECK(regions[1].m_density == T(1.2));
  REQUIRE(volumes.size() == 1U);
  CHECK(volumes[0].m_density == T(7800));

  pool.freeze();

  const Pool device = Pool::mirror(pool, deviceTestResource());

  const std::vector<Vec3T<T>> points{Vec3T<T>(0, 0, 0), Vec3T<T>(T(1.5), 0, 0), Vec3T<T>(3, 0, 0)};

  const auto onDevice =
    evaluateOnDevice<T>(TableQuery<T>{tape.rebasedView(device), regions.rebasedView(device)}, points);

  CHECK(onDevice[0] == T(7800) + T(10));
  CHECK(onDevice[1] == T(1.2));
  CHECK(onDevice[2] == T(20));
}
