// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// Test suite for regions and materials (Overlay(), MaterialLabel() and ExprIndex's regions, groups and materials):
// the regions of an Overlay against a hand-numbered expression, the background, materials inherited from groups,
// nested Overlays flattened into one priority order, maps and masks kept, a tape's value as the whole object, and
// the rejected placements of a partition and a material.

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestRenderHelpers.hpp"
#include "TestUtilities.hpp"

#include <algorithm>
#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;

namespace {

// A region's material name, or "-" for none.
template <class T>
std::string
materialName(const ExprIndex<T>& a_index, const uint32_t a_region)
{
  const MaterialLabelId material = a_index.getMaterialOf(a_region);

  return material.isNone() ? std::string("-") : a_index.getMaterialLabelName(material);
}

} // namespace

TEMPLATE_TEST_CASE("Overlay: the background is region 0, with no material, then the operands in priority order, with "
                   "their materials",
                   "[Overlay]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr uint32_t None = ExprIndex<T>::None;

  const ExprIndex<T> index(cable<T>());

  // Nodes: 0 Overlay, 1 jacket, 2 core.
  REQUIRE(index.getNumNodes() == 3U);
  CHECK(index.getNode(0).m_operation == "Overlay");

  REQUIRE(index.getNumRegions() == 3U);
  CHECK(index.getRegion(0).m_node == None);
  CHECK(index.getRegion(1).m_node == 1U);
  CHECK(index.getRegion(2).m_node == 2U);
  CHECK(materialName(index, 0) == "-");
  CHECK(materialName(index, 1) == "PTFE");
  CHECK(materialName(index, 2) == "copper");

  // The materials are numbered as they appear.
  REQUIRE(index.getNumMaterialLabels() == 2U);
  CHECK(index.findMaterialLabel("PTFE").value() == 0U);
  CHECK(index.findMaterialLabel("copper").value() == 1U);
  CHECK(index.findMaterialLabel("steel").isNone());

  REQUIRE(index.getNumRegionGroups() == 1U);
  CHECK(index.getRegionGroup(0).m_node == 0U);
  CHECK(index.getRegionGroup(0).m_parent == None);
  CHECK(index.getRegionGroup(0).m_firstRegion == 1U);
  CHECK(index.getRegionGroup(0).m_numRegions == 2U);
  CHECK(index.getRegion(1).m_regionGroup == 0U);

  CHECK(index.getRegionOfLeaf(0) == 1U);
  CHECK(index.getRegionOfLeaf(1) == 2U);

  // Materials and surface labels are numbered apart.
  CHECK(index.getNumSurfaceLabels() == 0U);

  // Operands without a material have none; three or more operands, each on top of the ones before it, are the
  // list's regions. An expression with no Overlay is one region.
  const SphereSDF<T> big(Vec3T<T>::zeros(), T(2));
  const SphereSDF<T> middle(Vec3T<T>::zeros(), T(1));
  const SphereSDF<T> small(Vec3T<T>::zeros(), T(0.5));
  const Vec3T<T>     inMiddle(T(0.8), T(0), T(0));

  const ExprIndex<T> unlabeled(Overlay(big, middle, small));

  // A braced list of shapes and Exprs of different types: the same regions.
  const Expr<T>      bracedMiddle = Translate(middle, Vec3T<T>::zeros());
  const ExprIndex<T> braced(
    Overlay({big, bracedMiddle, BoxSDF<T>(-T(0.3) * Vec3T<T>::ones(), T(0.3) * Vec3T<T>::ones())}));

  CHECK(braced.getNumRegions() == 4U);
  CHECK(braced.getRegion(3).m_node != ExprIndex<T>::None);

  CHECK(unlabeled.getNumRegions() == 4U);

  // An Overlay takes no BVH: however many operands it has, it is no union the large-union warning should flag.
  {
    std::vector<SphereSDF<T>> many;

    for (int i = 0; i < 80; i++) {
      many.emplace_back(Vec3T<T>(T(i), T(0), T(0)), T(0.4));
    }

    Pool        crowd(hostMemoryResource());
    TapeOptions quiet;

    quiet.m_largeUnionWarning = 0U;

    CHECK(Tape<T>(crowd, Overlay(many), quiet).getLargestPlainUnion() == 0U);
    CHECK(Tape<T>(crowd, Union(many), quiet).getLargestPlainUnion() == 80U);
  }
  CHECK(materialName(unlabeled, 0) == "-");
  CHECK(materialName(unlabeled, 1) == "-");

  Pool pool(hostMemoryResource());

  const Tape<T> three(pool, Overlay(big, middle, small));
  const Tape<T> listed(pool, Overlay(std::vector<SphereSDF<T>>{big, middle, small}));

  CHECK(three.region(inMiddle) == 2U);
  CHECK(three.region(Vec3T<T>::zeros()) == 3U);
  CHECK(listed.region(inMiddle) == three.region(inMiddle));
  CHECK(listed.region(Vec3T<T>::zeros()) == three.region(Vec3T<T>::zeros()));

  const ExprIndex<T> plain(MaterialLabel(
    Union(SphereSDF<T>(Vec3T<T>::zeros(), T(1)), BoxSDF<T>(-Vec3T<T>::ones(), Vec3T<T>::ones())), "steel"));

  REQUIRE(plain.getNumRegions() == 2U);
  CHECK(plain.getRegion(1).m_node == 0U);
  CHECK(plain.getRegion(1).m_regionGroup == None);
  CHECK(materialName(plain, 1) == "steel");
  CHECK(plain.getNumRegionGroups() == 0U);
}

TEMPLATE_TEST_CASE("Overlay: nested Overlays flatten into one priority order, kept as groups with default materials",
                   "[Overlay]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  constexpr uint32_t None = ExprIndex<T>::None;

  const BoxSDF<T>    matrix(-T(4) * Vec3::ones(), T(4) * Vec3::ones());
  const SphereSDF<T> shell(Vec3::zeros(), T(1));
  const SphereSDF<T> grain(Vec3::zeros(), T(0.4));

  // A particle (a shell around a grain, of steel unless said otherwise) placed twice in a matrix; the second
  // particle's grain is of carbon.
  const auto particle = MaterialLabel(Overlay(shell, grain), "steel");
  const auto second   = MaterialLabel(Overlay(shell, MaterialLabel(grain, "carbon")), "steel");

  const Expr<T> scene = Overlay(std::vector<Expr<T>>{MaterialLabel(matrix, "resin"),
                                                     Translate(particle, Vec3(T(-2), T(0), T(0))),
                                                     Translate(second, Vec3(T(2), T(0), T(0)))});

  const ExprIndex<T> index(scene);

  // Regions: background, matrix, shell 1, grain 1, shell 2, grain 2.
  REQUIRE(index.getNumRegions() == 6U);
  CHECK(materialName(index, 0) == "-");
  CHECK(materialName(index, 1) == "resin");
  CHECK(materialName(index, 2) == "steel");
  CHECK(materialName(index, 3) == "steel");
  CHECK(materialName(index, 4) == "steel");
  CHECK(materialName(index, 5) == "carbon");

  // Groups, depth first: the scene, then each particle, a contiguous range inside the scene's.
  REQUIRE(index.getNumRegionGroups() == 3U);
  CHECK(index.getRegionGroup(0).m_firstRegion == 1U);
  CHECK(index.getRegionGroup(0).m_numRegions == 5U);
  CHECK(index.getRegionGroup(0).m_parent == None);
  CHECK(index.getRegionGroup(1).m_firstRegion == 2U);
  CHECK(index.getRegionGroup(1).m_numRegions == 2U);
  CHECK(index.getRegionGroup(1).m_parent == 0U);
  CHECK(index.getRegionGroup(2).m_firstRegion == 4U);
  CHECK(index.getRegionGroup(2).m_numRegions == 2U);
  CHECK(index.getRegion(1).m_regionGroup == 0U);
  CHECK(index.getRegion(3).m_regionGroup == 1U);
  CHECK(index.getRegion(5).m_regionGroup == 2U);

  for (uint32_t leaf = 0; leaf < index.getNumLeaves(); leaf++) {
    CHECK(index.getRegionOfLeaf(leaf) == leaf + 1U);
  }
}

TEMPLATE_TEST_CASE("Overlay: maps keep the regions, masks clip every region and belong to none",
                   "[Overlay]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  constexpr uint32_t None = ExprIndex<T>::None;

  const BoxSDF<T> slab(Vec3(T(-0.1), T(-2), T(-2)), Vec3(T(0.1), T(2), T(2)));
  const BoxSDF<T> domain(Vec3(T(-2), T(-2), T(-0.5)), Vec3(T(2), T(2), T(2)));

  // Moved, drilled through by a slab and clipped to a domain: still three regions, the masks' leaves in none.
  const Expr<T> cut = Intersection(Difference(Rotate(Translate(cable<T>(), Vec3::ones()), T(30), 2), slab),
                                   SurfaceLabel(domain, "domain"));

  const ExprIndex<T> index(cut);

  REQUIRE(index.getNumRegions() == 3U);
  CHECK(materialName(index, 1) == "PTFE");
  CHECK(materialName(index, 2) == "copper");
  REQUIRE(index.getNumLeaves() == 4U);
  CHECK(index.getRegionOfLeaf(0) == 1U);
  CHECK(index.getRegionOfLeaf(1) == 2U);
  CHECK(index.getRegionOfLeaf(2) == None);
  CHECK(index.getRegionOfLeaf(3) == None);

  // The tape evaluates an Overlay as the union of its regions: the whole object.
  Pool          pool(hostMemoryResource());
  const Tape<T> overlay(pool, cable<T>());
  const Tape<T> solid(pool, Union(SphereSDF<T>(Vec3::zeros(), T(1)), SphereSDF<T>(Vec3::zeros(), T(0.5))));

  for (const Vec3& p : {Vec3::zeros(), Vec3(T(0.7), T(0), T(0)), Vec3(T(0), T(1.5), T(0.2))}) {
    CHECK(overlay.signedDistance(p) == solid.signedDistance(p));
  }
}

TEMPLATE_TEST_CASE("Overlay and MaterialLabel: reject a partition or a material where its regions would be unclear",
                   "[Overlay]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const char* partition = "an Overlay is a partition into regions, not a solid";
  const char* material  = "MaterialLabel and RegionName name a region";

  REQUIRE(abortsWith([] { (void)Union(cable<T>(), SphereSDF<T>(Vec3::zeros(), T(2))); }, partition));
  REQUIRE(abortsWith([] { (void)Union(cable<T>(), cable<T>()); }, partition));
  REQUIRE(abortsWith([] { (void)Intersection(SphereSDF<T>(Vec3::zeros(), T(2)), cable<T>()); }, partition));
  REQUIRE(abortsWith([] { (void)Difference(SphereSDF<T>(Vec3::zeros(), T(2)), cable<T>()); }, partition));
  REQUIRE(abortsWith([] { (void)Offset(cable<T>(), T(0.1)); }, partition));
  REQUIRE(abortsWith([] { (void)SmoothUnion(cable<T>(), SphereSDF<T>(Vec3::zeros(), T(2)), T(0.1)); }, partition));
  REQUIRE(abortsWith([] { (void)FiniteRepetition(cable<T>(), T(3) * Vec3::ones(), Vec3::zeros(), Vec3::ones()); },
                     partition));

  REQUIRE(abortsWith(
    [] { (void)Union(MaterialLabel(SphereSDF<T>(Vec3::zeros(), T(1)), "steel"), SphereSDF<T>(Vec3::ones(), T(1))); },
    material));
  REQUIRE(abortsWith(
    [] { (void)Offset(Translate(MaterialLabel(SphereSDF<T>(Vec3::zeros(), T(1)), "steel"), Vec3::ones()), T(0.1)); },
    material));
  REQUIRE(abortsWith([] { (void)Intersection(cable<T>(), MaterialLabel(SphereSDF<T>(Vec3::zeros(), T(2)), "steel")); },
                     material));

  REQUIRE(
    abortsWith([] { (void)Overlay(std::vector<SphereSDF<T>>{}); }, "Overlay: the list of regions must not be empty"));
  REQUIRE(abortsWith([] { (void)MaterialLabel(SphereSDF<T>(Vec3::zeros(), T(1)), ""); },
                     "MaterialLabel: the name must not be empty"));
  REQUIRE(abortsWith(
    [] {
      const ExprIndex<T> index(cable<T>());

      (void)index.getRegion(3);
    },
    "ExprIndex: region 3 of 3"));
}

TEMPLATE_TEST_CASE("Tape: a region's surface names the region across it, past a mask's cut and on a shared mask face",
                   "[Overlay][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  // Across a mask's cut: the cable (jacket 1, core 2) drilled by a slab (x > 0, |z| < 0.3). Just inside the core, by
  // the slab's face, the region across is the background the slab leaves, not the jacket the slab cut too.
  {
    const Expr<T> cable = Overlay(SphereSDF<T>(Vec3::zeros(), T(1)), SphereSDF<T>(Vec3::zeros(), T(0.5)));
    const Tape<T> tape(pool, Difference(cable, BoxSDF<T>(Vec3(T(0), T(-2), T(-0.3)), Vec3(T(2), T(2), T(0.3)))));

    const Vec3 p(T(0.1), T(0), T(0.3) + T(1e-3));

    REQUIRE(tape.region(p) == 2U);
    REQUIRE(tape.region(p - T(2e-3) * Vec3(T(0), T(0), T(1))) == 0U);

    const SurfaceRef<T> s = tape.closestSurface(p, 2U);

    CHECK(s.m_ownerRegion == 2U);
    CHECK(s.m_neighborRegion == 0U);

    // A subdomain of the core alone, the jacket another: the cut face is the core's boundary, not an interface.
    DomainSetup subdomains;

    (void)subdomains.addSubdomain({2U});
    (void)subdomains.addSubdomain({1U});

    const Subdomains subdomainsBuilt = subdomains.buildSubdomains(pool, tape.getNumRegions());

    CHECK(subdomains.role(tape.closestSurface(p, subdomainsBuilt, 0U)) == SurfaceRole::Boundary);
  }

  // On a shared mask's face: two spheres overlaid and clipped together (x < 0.3), on a big one. Just outside the face,
  // in the big sphere, the region across is the one on top of the two (3), as a ray through the face finds.
  {
    const Expr<T> pair =
      Overlay(SphereSDF<T>(Vec3(T(-0.5), T(0), T(0)), T(1)), SphereSDF<T>(Vec3(T(0.5), T(0), T(0)), T(1)));
    const Expr<T> clipped = Intersection(pair, BoxSDF<T>(Vec3(T(-3), T(-3), T(-3)), Vec3(T(0.3), T(3), T(3))));
    const Tape<T> tape(pool, Overlay(SphereSDF<T>(Vec3::zeros(), T(2)), clipped));

    const Vec3 out(T(0.3) + T(1e-4), T(-0.3), T(0));
    const Vec3 in(T(0.3) - T(1e-4), T(-0.3), T(0));

    REQUIRE(tape.region(out) == 1U);
    REQUIRE(tape.region(in) == 3U);

    CHECK(tape.closestSurface(out, 1U).m_neighborRegion == 3U);
    CHECK(tape.closestSurface(in, 3U).m_neighborRegion == 1U);
  }

  // On a shared mask's face, but where the one on top does not reach it: two concentric spheres clipped together
  // (x < 0.2) on a big one. Beside the face, outside the inner sphere, the region across is the outer one (2).
  {
    const Expr<T> pair    = Overlay(SphereSDF<T>(Vec3::zeros(), T(1)), SphereSDF<T>(Vec3::zeros(), T(0.5)));
    const Expr<T> clipped = Intersection(pair, BoxSDF<T>(Vec3(T(-5), T(-5), T(-5)), Vec3(T(0.2), T(5), T(5))));
    const Tape<T> tape(pool, Overlay(SphereSDF<T>(Vec3::zeros(), T(3)), clipped));

    const Vec3 p(T(0.214), T(-0.462), T(-0.031));

    REQUIRE(tape.region(p) == 1U);
    REQUIRE(tape.region(Vec3(T(0.1999), T(-0.462), T(-0.031))) == 2U);

    CHECK(tape.closestSurface(p, 1U).m_neighborRegion == 2U);
  }
}

TEMPLATE_TEST_CASE("Tape: a clipped Overlay used twice under different maps clips each copy with a mask of its own",
                   "[Overlay][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // The same clipped pair, and a copy moved by -1 along x: four regions, whose functions must partition space as
  // region() does. With the two copies' masks taken for one, about 2% of the points had no negative function.
  const Expr<T> pair    = Overlay(SphereSDF<T>(Vec3::zeros(), T(1)), SphereSDF<T>(Vec3::zeros(), T(0.5)));
  const Expr<T> clipped = Intersection(pair, BoxSDF<T>(Vec3(T(-2), T(-2), T(-2)), Vec3(T(0.2), T(2), T(2))));

  Pool pool(hostMemoryResource());

  const Tape<T> tape(pool, Overlay(clipped, Translate(clipped, Vec3(T(-1), T(0), T(0)))));

  REQUIRE(tape.getNumRegions() == 5U);

  int bad = 0;

  for (int i = 0; i < 30; i++) {
    for (int j = 0; j < 30; j++) {
      const Vec3     p(T(0.1) * T(i) - T(2.3) + T(0.0071), T(0.08) * T(j) - T(1.2) + T(0.0037), T(0.13));
      const uint32_t region = tape.region(p);

      bad += (tape.signedDistance(p, region) < T(0)) ? 0 : 1;

      tape.regionDistances(p, [&](const uint32_t a_region, const T a_value) {
        bad += (a_value == tape.signedDistance(p, a_region)) ? 0 : 1;
        bad += (a_region != region && a_value < T(0)) ? 1 : 0;
      });
    }
  }

  CHECK(bad == 0);
}

TEMPLATE_TEST_CASE("Tape: a subdomain's surface near a step is its boundary, never a contact between its own regions",
                   "[Overlay][Tape][Subdomains]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // A nozzle-like domain: the jacket and the core together, one subdomain. The core pokes out of the jacket's end, a
  // step where the contact between the two is the nearest surface of one of them at points near the end face.
  const Expr<T> jacket = CylinderSDF<T>(Vec3(T(0), T(0), T(-2)), Vec3(T(0), T(0), T(2)), T(1));
  const Expr<T> core   = CylinderSDF<T>(Vec3(T(0), T(0), T(-2.5)), Vec3(T(0), T(0), T(2.5)), T(0.4));

  Pool pool(hostMemoryResource());

  const Tape<T> tape(pool, Overlay(jacket, core));

  DomainSetup solid;

  (void)solid.addSubdomain({1U, 2U});

  const Subdomains solidBuilt = solid.buildSubdomains(pool, tape.getNumRegions());

  int nearBoundary = 0;

  for (int i = 0; i <= 40; i++) {
    for (int j = 0; j <= 40; j++) {
      const Vec3 p(T(0.05) * T(i), T(0), T(1.6) + T(0.025) * T(j));

      // At the points a cut-cell code asks, near the subdomain's boundary: a boundary, from the subdomain's side.
      if (std::abs(tape.signedDistance(p, solidBuilt, 0U)) > T(0.08)) {
        continue;
      }

      const SurfaceRef<T> s = tape.closestSurface(p, solidBuilt, 0U);

      nearBoundary++;

      CHECK(solid.role(s) == SurfaceRole::Boundary);
      CHECK(solid.subdomainOf(s.m_ownerRegion) == 0U);
      CHECK(s.m_neighborRegion == 0U);
    }
  }

  CHECK(nearBoundary > 100);

  // Just inside the jacket's end face, beside the core: the jacket's own region's nearest surface is the contact with
  // the core; the subdomain's is the jacket's end cap (part 2 of a cylinder: the cap at its second end).
  const Vec3          step(T(0.42), T(0), T(1.95));
  const SurfaceRef<T> cap = tape.closestSurface(step, solidBuilt, 0U);

  REQUIRE(tape.closestSurface(step, 1U).m_neighborRegion == 2U);
  CHECK(cap.m_surface == 0U);
  CHECK(cap.m_part == 2U);
  CHECK(cap.m_ownerRegion == 1U);
  CHECK(cap.m_neighborRegion == 0U);
}

TEMPLATE_TEST_CASE("Tape: an object of one region changes region where a ray crosses it, its sides by the way it goes",
                   "[Overlay][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // An assembly with no Overlay, through a BVH: one region, the object; the outside region 0.
  std::vector<Expr<T>> parts;

  for (int i = 0; i < 6; i++) {
    parts.push_back(SurfaceLabel(SphereSDF<T>(Vec3(T(1.5) * T(i), T(0), T(0)), T(0.8)), "part" + std::to_string(i)));
  }

  parts.push_back(BoxSDF<T>(Vec3(T(-1), T(-2), T(-0.3)), Vec3(T(9), T(-1), T(0.3))));

  Pool pool(hostMemoryResource());

  const Tape<T> tape(pool, Union(parts, Accelerate::BVH));

  REQUIRE(tape.getNumRegions() == 2U);

  int crossings = 0;

  for (int i = 0; i < 30; i++) {
    for (int j = 0; j < 5; j++) {
      // Rays from outside, and from inside a sphere.
      const Vec3    origin = (j == 4) ? Vec3(T(0.3) * T(i % 6) * T(1.5) / T(0.3), T(0.1), T(0))
                                      : Vec3(T(0.3) * T(i) - T(1), T(4), T(0.4) * T(j) - T(0.8));
      const Vec3    toward = Vec3(T(0.37) * T(i % 5) - T(0.7), T(-1), T(0.11) * T(j) - T(0.2));
      const RayT<T> ray{origin, toward / toward.length()};

      const SurfaceHitT<T> change = tape.firstRegionChange(ray, T(0), T(20));
      const HitT<T>        hit    = tape.firstHit(ray, T(0), T(20));

      REQUIRE(change.m_hit.hit() == hit.hit());

      if (!hit.hit()) {
        continue;
      }

      crossings++;

      // Where the tape's own ray query crosses; from outside into the object or back out, its normal pointing into
      // the region entered; the surface closestSurface() names there.
      CHECK_THAT(change.m_hit.m_t, withinAbsT<T>(hit.m_t, looseMargin<T>()));
      CHECK(change.m_surface.m_ownerRegion == (hit.m_entering ? 0U : 1U));
      CHECK(change.m_surface.m_neighborRegion == (hit.m_entering ? 1U : 0U));
      CHECK(dot(change.m_hit.m_normal, ray.m_direction) > T(0));
      CHECK(change.m_surface.m_surface == tape.closestSurface(change.m_hit.m_point).m_surface);
      CHECK(tape.region(change.m_hit.m_point + T(1e-3) * ray.m_direction) == change.m_surface.m_neighborRegion);
    }
  }

  CHECK(crossings > 60);
}

TEMPLATE_TEST_CASE(
  "Tape: firstRegionChange() names the shape a ray crosses, its part and its point in the shape's own frame",
  "[Overlay][Tape]",
  EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // Shapes under maps (the crossing is named by the leaf traced, mapped into its frame), and a repetition (whose
  // copies are a sub-program: named by evaluating the region there). Apart, so that the closest shape to a crossing
  // is the one crossed.
  const Expr<T> box =
    Translate(Rotate(Scale(BoxSDF<T>(Vec3(T(-1), T(-0.5), T(-0.7)), Vec3(T(1), T(0.5), T(0.7))), T(0.8)), T(30), 2),
              Vec3(T(-3), T(0), T(0)));
  const Expr<T> bar =
    Translate(Elongate(SphereSDF<T>(Vec3::zeros(), T(0.5)), Vec3(T(0.6), T(0), T(0))), Vec3(T(0), T(0), T(0.3)));
  const Expr<T> copies = FiniteRepetition(
    SphereSDF<T>(Vec3(T(3), T(0), T(0)), T(0.3)), Vec3(T(1), T(1), T(1)), Vec3::zeros(), Vec3(T(0), T(1), T(0)));

  Pool pool(hostMemoryResource());

  const Tape<T> one(pool, Union(std::vector<Expr<T>>{box, bar, copies}));
  const Tape<T> three(pool, Overlay(box, bar, copies));

  REQUIRE(one.getNumRegions() == 2U);
  REQUIRE(three.getNumRegions() == 4U);

  for (const Tape<T>* tape : {&one, &three}) {
    int crossings = 0;

    for (int i = 0; i < 40; i++) {
      for (int j = 0; j < 8; j++) {
        const Vec3    origin = Vec3(T(0.2) * T(i) - T(4.5), T(4), T(0.25) * T(j) - T(1));
        const Vec3    toward = Vec3(T(0.05) * T(j % 3) - T(0.05), T(-1), T(0.02) * T(i % 4));
        const RayT<T> ray{origin, toward / toward.length()};

        T t = T(0);

        // Every crossing along the ray, out of each shape as well as into it.
        for (int k = 0; k < 8; k++) {
          const SurfaceHitT<T> change = tape->firstRegionChange(ray, t, T(10));

          if (!change.m_hit.hit()) {
            break;
          }

          crossings++;

          // The shape closestSurface() names there, with its part and the point in its frame.
          const SurfaceRef<T> closest = tape->closestSurface(change.m_hit.m_point);

          CHECK(change.m_surface.m_leaf == closest.m_leaf);
          CHECK(change.m_surface.m_part == closest.m_part);

          for (int d = 0; d < 3; d++) {
            CHECK_THAT(change.m_surface.m_localPoint[d], withinAbsT<T>(closest.m_localPoint[d], looseMargin<T>()));
          }

          t = change.m_hit.m_t;
        }
      }
    }

    CHECK(crossings > 200);
  }
}

TEMPLATE_TEST_CASE("Tape: firstRegionChange() names a mesh's part by the face its ray crosses, under maps and beside "
                   "other shapes",
                   "[Overlay][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = BVH::DefaultBranchingRatio<T>();
  constexpr size_t W = TriangleSoA::DefaultWidth<T>();

  Pool pool(hostMemoryResource());

  const auto mesh = Parser::readIntoDCEL<T>(std::string(EBGEOMETRY_TEST_DATA_DIR) + "/dodecahedron.stl", pool);

  const FlatMeshSDF<T>      flat(mesh, pool);
  const MeshSDF<T, K>       bvhMesh(mesh, pool, BVH::Construction::SAH);
  const TriMeshSDF<T, K, W> triMesh(mesh, pool, BVH::Construction::SAH, 4);

  const auto box    = flat.computeBoundingVolume();
  const Vec3 center = T(0.5) * (box.getLowCorner() + box.getHighCorner());
  const T    size   = (box.getHighCorner() - box.getLowCorner()).length();

  // The mesh under maps (its faces are named in its own frame), beside a ball that the rays also cross.
  const Vec3    shift(T(0.2) * size, T(-0.1) * size, T(0.1) * size);
  const Vec3    ballCenter = center + Vec3(size, T(0), T(0));
  const T       ballRadius = T(0.2) * size;
  const Expr<T> ball       = SphereSDF<T>(ballCenter, ballRadius);

  const auto placed = [&](const Expr<T>& a_mesh) { return Union(Translate(Rotate(a_mesh, T(30), 2), shift), ball); };

  const Tape<T> tapes[3] = {
    Tape<T>(pool, placed(flat)), Tape<T>(pool, placed(bvhMesh)), Tape<T>(pool, placed(triMesh))};

  for (const Tape<T>& tape : tapes) {
    int onMesh = 0;
    int agree  = 0;

    for (int i = 0; i < 24; i++) {
      for (int j = 0; j < 12; j++) {
        const Vec3 target = center + shift + T(0.03) * size * Vec3(T(i % 5) - T(2), T(j % 4) - T(1.5), T(i % 3) - T(1));
        const Vec3 origin =
          target + T(2) * size * Vec3(std::cos(T(0.26) * T(i)), std::sin(T(0.26) * T(i)), T(0.15) * T(j) - T(0.8));
        const RayT<T> ray{origin, (target - origin) / (target - origin).length()};

        T t = T(0);

        // Every crossing along the ray: into the mesh and out of it, and the ball's.
        for (int k = 0; k < 6; k++) {
          const SurfaceHitT<T> change = tape.firstRegionChange(ray, t, T(8) * size);

          if (!change.m_hit.hit()) {
            break;
          }

          t = change.m_hit.m_t;

          const SurfaceRef<T> closest = tape.closestSurface(change.m_hit.m_point);

          CHECK(change.m_surface.m_leaf == closest.m_leaf);

          // The ball's crossings: it has one part.
          if (std::abs((change.m_hit.m_point - ballCenter).length() - ballRadius) < T(1e-3) * size) {
            continue;
          }

          onMesh++;

          // A face the crossing lies on: the one closestSurface() names there, but for a crossing on an edge, where
          // the neighbor is as close.
          REQUIRE(change.m_surface.m_part < mesh.numFaces());

          const T onFace = mesh.getFace(change.m_surface.m_part).unsignedDistance2(change.m_surface.m_localPoint, mesh);

          CHECK(std::sqrt(onFace) <= T(1e-3) * size);

          agree += (change.m_surface.m_part == closest.m_part) ? 1 : 0;
        }
      }
    }

    CHECK(onMesh > 300);
    CHECK(agree > (onMesh * 9) / 10);
  }
}

TEMPLATE_TEST_CASE("Tape: regionDistances() gives every region's function in one pass, as signedDistance() does",
                   "[Overlay][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const BoxSDF<T>    matrix(-T(3) * Vec3::ones(), T(3) * Vec3::ones());
  const SphereSDF<T> shell(Vec3(T(-1), T(0), T(0)), T(1));
  const SphereSDF<T> grain(Vec3(T(-1), T(0), T(0)), T(0.4));
  const SphereSDF<T> shell2(Vec3(T(1.2), T(0), T(0)), T(1));
  const SphereSDF<T> grain2(Vec3(T(1.2), T(0), T(0)), T(0.4));
  const BoxSDF<T>    domain(Vec3(T(-4), T(-4), T(-0.5)), Vec3(T(4), T(4), T(4)));
  const BoxSDF<T>    slab(Vec3(T(-0.1), T(-4), T(-4)), Vec3(T(0.1), T(4), T(4)));

  const Expr<T> layers = Overlay(matrix, Overlay(shell, grain), Overlay(shell2, grain2));

  // No masks; the same masks on every region (the shortcut); masks on some regions only (region by region).
  const std::vector<Expr<T>> scenes{layers,
                                    Difference(Intersection(layers, domain), slab),
                                    Overlay(matrix, Difference(Overlay(shell, grain), slab), Overlay(shell2, grain2)),
                                    SphereSDF<T>(Vec3::zeros(), T(1))};

  Pool pool(hostMemoryResource());

  for (const Expr<T>& scene : scenes) {
    const Tape<T> tape(pool, scene);

    for (int i = 0; i < 40; i++) {
      for (int j = 0; j < 9; j++) {
        const Vec3 p(T(0.11) * T(i) - T(2.3), T(0.013) * T(j) - T(0.05), T(0.17) * T(j) - T(0.7));

        std::vector<T>        values(tape.getNumRegions(), Math::Limits<T>::max());
        std::vector<uint32_t> order;

        tape.regionDistances(p, [&](const uint32_t a_region, const T a_value) {
          REQUIRE(a_region < tape.getNumRegions());

          values[a_region] = a_value;

          order.push_back(a_region);
        });

        // Every region once, from the last down to the background, each with signedDistance()'s value.
        REQUIRE(order.size() == tape.getNumRegions());

        for (uint32_t k = 0; k < tape.getNumRegions(); k++) {
          CHECK(order[k] == tape.getNumRegions() - 1U - k);
          CHECK(values[k] == tape.signedDistance(p, k));
        }
      }
    }
  }
}

TEMPLATE_TEST_CASE("Tape: region() and signedDistance(p, region) partition space as the regions' solids say",
                   "[Overlay][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // A scene: a matrix holding two particles (a shell around a grain), the second moved; the whole clipped to a
  // half-space box and drilled by a slab. Each region's solid, by hand: the operand clipped by both masks.
  const BoxSDF<T>    matrix(-T(3) * Vec3::ones(), T(3) * Vec3::ones());
  const SphereSDF<T> shell(Vec3(T(-1), T(0), T(0)), T(1));
  const SphereSDF<T> grain(Vec3(T(-1), T(0), T(0)), T(0.4));
  const SphereSDF<T> shell2(Vec3(T(1.2), T(0), T(0)), T(1));
  const SphereSDF<T> grain2(Vec3(T(1.2), T(0), T(0)), T(0.4));
  const BoxSDF<T>    domain(Vec3(T(-4), T(-4), T(-0.5)), Vec3(T(4), T(4), T(4)));
  const BoxSDF<T>    slab(Vec3(T(-0.1), T(-4), T(-4)), Vec3(T(0.1), T(4), T(4)));

  const Expr<T> scene = Difference(
    Intersection(Overlay(std::vector<Expr<T>>{matrix, Overlay(shell, grain), Overlay(shell2, grain2)}), domain), slab);

  const auto mask = [&](const Vec3& p) { return Math::max(domain.signedDistance(p), -slab.signedDistance(p)); };
  const auto raw  = [&](const uint32_t k, const Vec3& p) {
    const T values[] = {T(0),
                        matrix.signedDistance(p),
                        shell.signedDistance(p),
                        grain.signedDistance(p),
                        shell2.signedDistance(p),
                        grain2.signedDistance(p)};

    return values[k];
  };
  const auto solid = [&](const uint32_t k, const Vec3& p) { return Math::max(raw(k, p), mask(p)); };

  Pool pool(hostMemoryResource());

  const Tape<T>      tape(pool, scene);
  const Tape<T>      plain(pool, scene);
  const ExprIndex<T> index(scene);

  REQUIRE(tape.getNumRegions() == 6U);
  REQUIRE(index.getNumRegions() == 6U);

  // The region programs leave the leaves as the whole expression numbers them.
  CHECK(tape.getNumPrimitives() == plain.getNumPrimitives());

  const Tape<T> copy = tape.deepCopy(pool);

  for (int i = -40; i <= 40; i++) {
    for (int j = -8; j <= 8; j++) {
      const Vec3 p(T(0.1) * T(i) + T(0.013), T(0.1) * T(j) + T(0.007), T(0.3) * T(j % 3) - T(0.4));

      // The reference: the last region whose solid holds the point.
      uint32_t expected = 0;

      for (uint32_t k = 1; k < 6; k++) {
        expected = (solid(k, p) < T(0)) ? k : expected;
      }

      INFO("point " << p);

      CHECK(tape.region(p) == expected);
      CHECK(copy.region(p) == expected);
      CHECK(tape.closestLeaf(p).m_leaf == plain.closestLeaf(p).m_leaf);

      // Exactly one region's function is negative: its own.
      for (uint32_t k = 0; k < 6; k++) {
        const T d = tape.signedDistance(p, k);

        CHECK((d < T(0)) == (k == expected));

        // Region k's function: its solid with the masks, minus the later solids without them (the masks are above
        // every region, so they are applied once); the background's, minus the whole object.
        T reference = -tape.signedDistance(p);

        if (k > 0) {
          T later = Math::Limits<T>::max();

          for (uint32_t m = k + 1; m < 6; m++) {
            later = Math::min(later, raw(m, p));
          }

          reference = Math::max(solid(k, p), -later);
        }

        CHECK_THAT(d, withinAbsT<T>(reference, tightMargin<T>()));
      }
    }
  }

  // An object with no Overlay is one region.
  const Tape<T> one(pool, Union(matrix, shell));

  CHECK(one.getNumRegions() == 2U);
  CHECK(one.region(Vec3::zeros()) == 1U);
  CHECK(one.region(T(5) * Vec3::ones()) == 0U);
  CHECK(one.signedDistance(Vec3::zeros(), 1U) == one.signedDistance(Vec3::zeros()));
  CHECK(one.signedDistance(Vec3::zeros(), 0U) == -one.signedDistance(Vec3::zeros()));
}

TEMPLATE_TEST_CASE("Tape: a mask above two regions leaves no sheet of zeros on its face inside the later one",
                   "[Overlay][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // The cable (concentric spheres) cut by a slab (x > 0, |z| < 0.3): where the slab's face crosses the core, the
  // jacket has no surface.
  const BoxSDF<T> slab(Vec3(T(0), T(-2), T(-0.3)), Vec3(T(2), T(2), T(0.3)));

  Pool pool(hostMemoryResource());

  const Tape<T> tape(pool, Difference(cable<T>(), slab));

  // On the slab's end face z = 0.3, inside the core's radius (0.5): the jacket's function is the distance to the
  // core's surface, not zero (no sheet).
  for (const T x : {T(0.05), T(0.2), T(0.35)}) {
    const Vec3 p(x, T(0), T(0.3));

    INFO("x " << x);

    CHECK_THAT(tape.signedDistance(p, 1), withinAbsT<T>(T(0.5) - p.length(), tightMargin<T>()));
  }

  // A mask on one nested Overlay only: the matrix's function subtracts the particle as clipped, the particle's grain
  // and shell share it.
  const BoxSDF<T>    matrix(-T(3) * Vec3::ones(), T(3) * Vec3::ones());
  const SphereSDF<T> shell(Vec3::zeros(), T(1));
  const SphereSDF<T> grain(Vec3::zeros(), T(0.4));
  const BoxSDF<T>    half(Vec3(T(-2), T(-2), T(-2)), Vec3(T(0), T(2), T(2)));

  const Tape<T> nested(pool, Overlay(matrix, Intersection(Overlay(shell, grain), half)));

  for (const Vec3& p : {Vec3(T(0.5), T(0), T(0)),
                        Vec3(T(-0.5), T(0), T(0)),
                        Vec3(T(0), T(0.2), T(0)),
                        Vec3(T(-0.2), T(0.1), T(0.05))}) {
    INFO("point " << p);

    const T clippedShell = Math::max(shell.signedDistance(p), half.signedDistance(p));
    const T clippedGrain = Math::max(grain.signedDistance(p), half.signedDistance(p));

    // The matrix (region 1): minus the clipped shell and grain. The shell (region 2): clipped, minus the grain without
    // the shared mask.
    CHECK_THAT(
      nested.signedDistance(p, 1),
      withinAbsT<T>(Math::max(matrix.signedDistance(p), -Math::min(clippedShell, clippedGrain)), tightMargin<T>()));
    CHECK_THAT(nested.signedDistance(p, 2),
               withinAbsT<T>(Math::max(clippedShell, -grain.signedDistance(p)), tightMargin<T>()));
    CHECK_THAT(nested.signedDistance(p, 3), withinAbsT<T>(clippedGrain, tightMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("Tape: closestSurface(p, region) names a region's surface and the regions on its two sides there",
                   "[Overlay][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  // The cable (concentric spheres: the jacket of radius 1, region 1, leaf 0; the core of radius 0.5, region 2,
  // leaf 1), in air (region 0).
  const Tape<T> cableTape(pool, cable<T>());

  const Vec3 outer(T(1), T(0), T(0));
  const Vec3 inner(T(0.5), T(0), T(0));

  // The outer sphere: the jacket's own surface, between the jacket and the air.
  const SurfaceRef<T> jacketOuter = cableTape.closestSurface(outer, 1U);

  CHECK(jacketOuter.m_leaf == 0U);
  CHECK(jacketOuter.m_ownerRegion == 1U);
  CHECK(jacketOuter.m_neighborRegion == 0U);

  // The inner sphere, from the jacket: an interface with the core, on the core's surface.
  const SurfaceRef<T> jacketInner = cableTape.closestSurface(inner, 1U);

  CHECK(jacketInner.m_leaf == 1U);
  CHECK(jacketInner.m_ownerRegion == 1U);
  CHECK(jacketInner.m_neighborRegion == 2U);

  // The same interface from the core, and the outer sphere from the air.
  const SurfaceRef<T> core = cableTape.closestSurface(inner, 2U);

  CHECK(core.m_leaf == 1U);
  CHECK(core.m_ownerRegion == 2U);
  CHECK(core.m_neighborRegion == 1U);

  const SurfaceRef<T> air = cableTape.closestSurface(outer, 0U);

  CHECK(air.m_leaf == 0U);
  CHECK(air.m_ownerRegion == 0U);
  CHECK(air.m_neighborRegion == 1U);

  // A triple point: a sphere centered on another's surface faces the air on its outer half and the other sphere on
  // its inner half, so its sides belong to the point.
  const SphereSDF<T> big(Vec3::zeros(), T(1));
  const SphereSDF<T> small(Vec3(T(1), T(0), T(0)), T(0.3));

  const Tape<T> triple(pool, Overlay(big, small));

  CHECK(triple.closestSurface(Vec3(T(1.3), T(0), T(0)), 2U).m_neighborRegion == 0U);
  CHECK(triple.closestSurface(Vec3(T(0.7), T(0), T(0)), 2U).m_neighborRegion == 1U);

  // A mask's cut: the slab's face where it cuts the core is the slab's surface, with its label, the core inside
  // and the air across.
  const BoxSDF<T> slab(Vec3(T(0), T(-2), T(-0.3)), Vec3(T(2), T(2), T(0.3)));

  const Expr<T>      cut = Difference(cable<T>(), SurfaceLabel(slab, "cut"));
  const Tape<T>      cutTape(pool, cut);
  const ExprIndex<T> index(cut);

  const SurfaceRef<T> face = cutTape.closestSurface(Vec3(T(0.1), T(0), T(0.3)), 2U);

  CHECK(face.m_leaf == 2U);
  CHECK(face.m_surfaceLabel == index.findSurfaceLabel("cut"));
  CHECK(face.m_ownerRegion == 2U);
  CHECK(face.m_neighborRegion == 0U);

  // An object with no Overlay: its surface, between it and the background.
  const Tape<T> one(pool, toExpr(big));

  CHECK(one.closestSurface(Vec3(T(1), T(0), T(0)), 1U).m_ownerRegion == 1U);
  CHECK(one.closestSurface(Vec3(T(1), T(0), T(0)), 1U).m_neighborRegion == 0U);
  CHECK(one.getNumRegions() == 2U);
  CHECK(one.region(Vec3::zeros()) == 1U);
  CHECK(one.region(Vec3(T(5), T(0), T(0))) == 0U);
  CHECK(one.getMaterialOf(1).isNone());

  // Its material, on it or on a map above it, numbered as the index numbers it (the first material).
  const Expr<T>      labeled = Translate(MaterialLabel(big, "steel"), Vec3(T(1), T(0), T(0)));
  const Tape<T>      steel(pool, labeled);
  const ExprIndex<T> index2(labeled);

  CHECK(steel.getMaterialOf(1).value() == 0U);
  CHECK(index2.getMaterialOf(1) == steel.getMaterialOf(1));
  CHECK(steel.getMaterialOf(0).isNone());
}

TEMPLATE_TEST_CASE("Subdomains: a solver's regions give each surface a role, and each subdomain a function",
                   "[Overlay][Tape][Subdomains]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  const Tape<T>      tape(pool, cable<T>());
  const ExprIndex<T> index(cable<T>());

  const SurfaceRef<T> outer = tape.closestSurface(Vec3(T(1), T(0), T(0)), 1U);   // the jacket and the outside
  const SurfaceRef<T> inner = tape.closestSurface(Vec3(T(0.5), T(0), T(0)), 1U); // the jacket and the core

  // CFD in the air: the outer sphere is its boundary, the inner one nothing to it.
  DomainSetup cfd;

  CHECK(cfd.addSubdomain({0}) == 0U);
  CHECK(cfd.role(outer) == SurfaceRole::Boundary);
  CHECK(cfd.role(inner) == SurfaceRole::None);

  // Electrostatics in the air and the jacket, the core a conductor: an interface and a boundary.
  DomainSetup electrostatics;

  CHECK(electrostatics.addSubdomain({0}) == 0U);
  CHECK(electrostatics.addSubdomain(index.getRegionsOfMaterial(index.findMaterialLabel("PTFE"))) == 1U);

  const Subdomains electrostaticsBuilt = electrostatics.buildSubdomains(pool, tape.getNumRegions());
  CHECK(electrostatics.getNumSubdomains() == 2U);
  CHECK(electrostatics.subdomainOf(1) == 1U);
  CHECK(electrostatics.subdomainOf(2) == Subdomains::None);
  CHECK(electrostatics.role(outer) == SurfaceRole::Interface);
  CHECK(electrostatics.role(inner) == SurfaceRole::Boundary);

  // Heat in all three: two interfaces. The jacket and the core as one subdomain: their contact is internal.
  DomainSetup heat;

  (void)heat.addSubdomain({0});
  (void)heat.addSubdomain({1});
  (void)heat.addSubdomain({2});

  CHECK(heat.role(outer) == SurfaceRole::Interface);
  CHECK(heat.role(inner) == SurfaceRole::Interface);

  DomainSetup solid;

  (void)solid.addSubdomain(index.getRegionsOfRegionGroup(0));

  const Subdomains solidBuilt = solid.buildSubdomains(pool, tape.getNumRegions());

  CHECK(solid.role(inner) == SurfaceRole::Internal);
  CHECK(solid.role(outer) == SurfaceRole::Boundary);

  // A subdomain's function is negative exactly where one of its regions is; the solid's has the whole object's sign.
  for (int i = -15; i <= 15; i++) {
    const Vec3 p(T(0.1) * T(i) + T(0.013), T(0.02), T(0));

    INFO("point " << p);

    CHECK((tape.signedDistance(p, solidBuilt, 0U) < T(0)) == (tape.signedDistance(p) < T(0)));
    CHECK((tape.signedDistance(p, electrostaticsBuilt, 1U) < T(0)) == (tape.region(p) == 1U));
  }

  // The jacket and the core as one subdomain: on their contact the function is not zero (no sheet inside the subdomain);
  // across the outer sphere it is the distance, from both sides.
  for (const T x : {T(0.5), T(0.45), T(0.55)}) {
    INFO("x " << x);

    CHECK(tape.signedDistance(Vec3(x, T(0), T(0)), solidBuilt, 0U) < -T(0.2));
  }

  CHECK_THAT(tape.signedDistance(Vec3(T(0.95), T(0), T(0)), solidBuilt, 0U), withinAbsT<T>(-T(0.05), tightMargin<T>()));
  CHECK_THAT(tape.signedDistance(Vec3(T(1.05), T(0), T(0)), solidBuilt, 0U), withinAbsT<T>(T(0.05), tightMargin<T>()));

  // The subdomain's surface: from its region whose function is smallest.
  const SurfaceRef<T> jacket = tape.closestSurface(Vec3(T(0.5), T(0), T(0)), electrostaticsBuilt, 1U);

  CHECK(jacket.m_ownerRegion == 1U);
  CHECK(jacket.m_neighborRegion == 2U);
  CHECK(electrostatics.role(jacket) == SurfaceRole::Boundary);

  REQUIRE(
    abortsWith([] { (void)DomainSetup().addSubdomain({}); }, "DomainSetup: a subdomain needs at least one region"));
  REQUIRE(abortsWith(
    [] {
      DomainSetup subdomains;

      (void)subdomains.addSubdomain({1, 2});
      (void)subdomains.addSubdomain({2});
    },
    "DomainSetup: region 2 is already in subdomain 0"));

  // A region listed twice counts once; region numbers are the tape's.
  {
    DomainSetup twice;

    CHECK(twice.addSubdomain({3, 3, 1}) == 0U);
    CHECK(twice.getRegions(0) == std::vector<uint32_t>{1, 3});
  }

  REQUIRE(abortsWith([] { (void)DomainSetup().addSubdomain({Subdomains::None}); },
                     "DomainSetup: no region number 4294967295"));
  REQUIRE(abortsWith([] { DomainSetup().removeSurfaceGroup(0); }, "DomainSetup: surface group 0 of 0"));

  // Tables built directly: a subdomain or a surface group out of range, and selectors without a group each.
  REQUIRE(abortsWith(
    [] {
      Pool tables(hostMemoryResource());

      (void)Subdomains(tables, {2U}, 1U);
    },
    "Subdomains: region 0 is in subdomain 2 of 1"));
  REQUIRE(abortsWith(
    [] {
      Pool tables(hostMemoryResource());

      (void)SurfaceGroups(tables, {SurfaceSelector()}, {3U}, 1U);
    },
    "SurfaceGroups: selector 0 is in surface group 3 of 1"));
  REQUIRE(abortsWith(
    [] {
      Pool tables(hostMemoryResource());

      (void)SurfaceGroups(tables, {SurfaceSelector()}, {}, 1U);
    },
    "SurfaceGroups: 1 selectors and 0 surface groups of them"));

  // A buffer too small for every subdomain's function.
  REQUIRE(abortsWith(
    [&] {
      const Domain domain(pool, tape, electrostatics);

      T values[1];

      domain.signedDistances(Vec3(T(0), T(0), T(0)), values, 1U);
    },
    "Domain::signedDistances: room for 1 values, the domain has 3 functions"));
}

TEMPLATE_TEST_CASE("Domain: its labels divide space, each with a function negative exactly in it",
                   "[Overlay][Tape][Domain]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // The cable (jacket radius 1, region 1; core radius 0.5, region 2; air, region 0) cut by a slab (x > 0, |z| < 0.3),
  // so that the core meets the air and the jacket along triple lines.
  Pool pool(hostMemoryResource());

  const Tape<T> tape(pool, Difference(cable<T>(), BoxSDF<T>(Vec3(T(0), T(-2), T(-0.3)), Vec3(T(2), T(2), T(0.3)))));

  // Electrostatics: the air (0) and the jacket (1); the core, a conductor, excluded (entry 2 of the values).
  DomainSetup setup;

  (void)setup.addSubdomain({0});
  (void)setup.addSubdomain({1});

  const Domain domain(pool, tape, setup);

  REQUIRE(domain.getNumSubdomainFunctions() == 3U);

  // The whole domain as one subdomain: Excluded's function is its function's negative.
  DomainSetup together;

  (void)together.addSubdomain({0, 1});

  const Subdomains togetherBuilt = together.buildSubdomains(pool, tape.getNumRegions());

  const auto valueOf = [&](const Vec3& a_point, const uint32_t a_subdomain) {
    T values[3];

    domain.signedDistances(a_point, values);

    return values[(a_subdomain == Domain<T>::Excluded) ? 2U : a_subdomain];
  };

  // Everywhere: the values are signedDistance()'s, one by one; the subdomain is the region's, and off the
  // surfaces its function alone is negative.
  size_t checked = 0;

  for (int i = 0; i < 25; i++) {
    for (int j = 0; j < 9; j++) {
      const Vec3 p(T(0.1) * T(i) - T(1.2) + T(0.013), T(0.007), T(0.1) * T(j) - T(0.4) + T(0.004));

      INFO("point " << p);

      T values[3];

      domain.signedDistances(p, values);

      const uint32_t subdomain = domain.subdomainAt(p);

      CHECK(values[0] == domain.signedDistance(p, 0U));
      CHECK(values[1] == domain.signedDistance(p, 1U));
      CHECK(values[2] == domain.signedDistance(p, Domain<T>::Excluded));
      CHECK(values[2] == -tape.signedDistance(p, togetherBuilt, 0U));
      CHECK(subdomain == domain.subdomains().subdomainOf(tape.region(p)));

      if (std::abs(values[0]) > T(1e-3) && std::abs(values[1]) > T(1e-3) && std::abs(values[2]) > T(1e-3)) {
        const uint32_t negative = (values[0] < T(0)) ? 0U : (values[1] < T(0)) ? 1U : 2U;

        CHECK(int(values[0] < T(0)) + int(values[1] < T(0)) + int(values[2] < T(0)) == 1);
        CHECK(negative == ((subdomain == Domain<T>::Excluded) ? 2U : subdomain));

        checked++;
      }
    }
  }

  CHECK(checked > 200U);

  // Across the air and the jacket (an interface, away from the cut): the two subdomains' functions change sign, with
  // opposite signs, the excluded regions' not. Across the jacket and the core (the jacket's boundary with the
  // conductor): the jacket's and the excluded regions' change sign, opposite, the air's not.
  for (const T r : {T(0.95), T(1.05)}) {
    const Vec3 p(-r, T(0), T(0));

    INFO("r " << r);

    CHECK(valueOf(p, 0U) == -valueOf(p, 1U));
    CHECK((valueOf(p, 0U) < T(0)) == (r > T(1)));
    CHECK(valueOf(p, Domain<T>::Excluded) > T(0.2));
  }

  for (const T r : {T(0.45), T(0.55)}) {
    const Vec3 p(-r, T(0), T(0));

    INFO("r " << r);

    CHECK(valueOf(p, 1U) == -valueOf(p, Domain<T>::Excluded));
    CHECK((valueOf(p, 1U) < T(0)) == (r > T(0.5)));
    CHECK(valueOf(p, 0U) > T(0.2));
  }

  // Near a triple line (the core, the jacket and the cut's air meeting on the circle of radius 0.4 in the plane
  // z = 0.3): the labels still divide space, the core's side excluded, the jacket's side the jacket's, the cut the
  // air's.
  CHECK(domain.subdomainAt(Vec3(T(0.36), T(0.01), T(0.32))) == Domain<T>::Excluded);
  CHECK(domain.subdomainAt(Vec3(T(0.44), T(0.01), T(0.32))) == 1U);
  CHECK(domain.subdomainAt(Vec3(T(0.4), T(0.01), T(0.28))) == 0U);
}

TEMPLATE_TEST_CASE("Tape: firstRegionChange() tracks a particle through the regions, and firstSurface() a subdomain",
                   "[Overlay][Tape][Ray]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // The cable (concentric spheres: jacket radius 1, region 1; core radius 0.5, region 2) in air, cut by a slab
  // (x > 0, |z| < 0.3) labeled "cut".
  const BoxSDF<T> slab(Vec3(T(0), T(-2), T(-0.3)), Vec3(T(2), T(2), T(0.3)));
  const Expr<T>   cut = Difference(cable<T>(), SurfaceLabel(slab, "cut"));

  Pool pool(hostMemoryResource());

  const Tape<T>      tape(pool, cut);
  const ExprIndex<T> index(cut);

  // Along x at z = 0.4, above the cut: four changes, at the spheres' exact crossings.
  {
    const RayT<T> ray{Vec3(T(-2), T(0), T(0.4)), Vec3(T(1), T(0), T(0))};

    const T        jacket           = std::sqrt(T(1) - T(0.16));
    const T        core             = std::sqrt(T(0.25) - T(0.16));
    const T        ts[]             = {T(2) - jacket, T(2) - core, T(2) + core, T(2) + jacket};
    const uint32_t leftRegions[]    = {0, 1, 2, 1};
    const uint32_t enteredRegions[] = {1, 2, 1, 0};
    const uint32_t leaves[]         = {0, 1, 1, 0};

    T t = T(0);

    for (int i = 0; i < 4; i++) {
      INFO("crossing " << i);

      const SurfaceHitT<T> change = tape.firstRegionChange(ray, t, T(5));

      REQUIRE(change.m_hit.hit());
      CHECK_THAT(change.m_hit.m_t, withinAbsT<T>(ts[i], looseMargin<T>()));
      CHECK(change.m_surface.m_ownerRegion == leftRegions[i]);
      CHECK(change.m_surface.m_neighborRegion == enteredRegions[i]);
      CHECK(change.m_surface.m_leaf == leaves[i]);

      // The normal points from the region left into the region entered: along the ray.
      CHECK(dot(change.m_hit.m_normal, ray.m_direction) > T(0));

      t = change.m_hit.m_t;
    }

    CHECK(!tape.firstRegionChange(ray, t, T(5)).m_hit.hit());
  }

  // Along x at z = 0, through the cut: the particle leaves the core into the air at the cut face.
  {
    const RayT<T> ray{Vec3(T(-2), T(0), T(0)), Vec3(T(1), T(0), T(0))};

    const SurfaceHitT<T> a = tape.firstRegionChange(ray, T(0), T(5));
    const SurfaceHitT<T> b = tape.firstRegionChange(ray, a.m_hit.m_t, T(5));
    const SurfaceHitT<T> c = tape.firstRegionChange(ray, b.m_hit.m_t, T(5));

    CHECK_THAT(c.m_hit.m_t, withinAbsT<T>(T(2), looseMargin<T>()));
    CHECK(c.m_surface.m_ownerRegion == 2U);
    CHECK(c.m_surface.m_neighborRegion == 0U);
    CHECK(c.m_surface.m_surfaceLabel == index.findSurfaceLabel("cut"));
    CHECK(!tape.firstRegionChange(ray, c.m_hit.m_t, T(5)).m_hit.hit());
  }

  // From inside the core, and as a particle's step.
  {
    const SurfaceHitT<T> fromCore = tape.firstRegionChange(Vec3(T(-0.2), T(0), T(0.4)), Vec3(T(2), T(0), T(0.4)));

    REQUIRE(fromCore.m_hit.hit());
    CHECK_THAT(fromCore.m_hit.m_t, withinAbsT<T>(T(0.5), looseMargin<T>()));
    CHECK(fromCore.m_surface.m_ownerRegion == 2U);
    CHECK(fromCore.m_surface.m_neighborRegion == 1U);
  }

  // Rendering: the first entry into the visible regions, the others seen through.
  {
    const RayT<T> ray{Vec3(T(-2), T(0), T(0.4)), Vec3(T(1), T(0), T(0))};

    DomainSetup jacketOnly;
    DomainSetup coreOnly;
    DomainSetup both;

    (void)jacketOnly.addSubdomain({1});
    (void)coreOnly.addSubdomain({2});
    (void)both.addSubdomain({1, 2});

    const Subdomains jacketOnlyBuilt = jacketOnly.buildSubdomains(pool, tape.getNumRegions());
    const Subdomains coreOnlyBuilt   = coreOnly.buildSubdomains(pool, tape.getNumRegions());
    const Subdomains bothBuilt       = both.buildSubdomains(pool, tape.getNumRegions());

    const SurfaceHitT<T> jacket = tape.firstSurface(ray, T(0), T(5), jacketOnlyBuilt, 0U);
    const SurfaceHitT<T> core   = tape.firstSurface(ray, T(0), T(5), coreOnlyBuilt, 0U);
    const SurfaceHitT<T> solid  = tape.firstSurface(ray, T(0), T(5), bothBuilt, 0U);

    CHECK_THAT(jacket.m_hit.m_t, withinAbsT<T>(T(2) - std::sqrt(T(0.84)), looseMargin<T>()));
    CHECK_THAT(core.m_hit.m_t, withinAbsT<T>(T(2) - std::sqrt(T(0.09)), looseMargin<T>()));
    CHECK(core.m_surface.m_neighborRegion == 2U);
    CHECK(solid.m_surface.m_neighborRegion == 1U);

    // Leaving the solid: the contact between the jacket and the core is passed through.
    const SurfaceHitT<T> leave = tape.firstSurface(ray, solid.m_hit.m_t, T(5), bothBuilt, 0U);

    CHECK_THAT(leave.m_hit.m_t, withinAbsT<T>(T(2) + std::sqrt(T(0.84)), looseMargin<T>()));
    CHECK(leave.m_surface.m_neighborRegion == 0U);
  }
}

TEMPLATE_TEST_CASE("ExprIndex: the pairs of regions each surface may separate, the interfaces",
                   "[Overlay][SurfaceGroups]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Pair = std::pair<uint32_t, uint32_t>;

  // The cable (leaves: 0 the jacket, region 1; 1 the core, region 2) cut by a slab (leaf 2, a mask).
  const ExprIndex<T> index(Difference(cable<T>(), BoxSDF<T>(Vec3(T(0), T(-2), T(-0.3)), Vec3(T(2), T(2), T(0.3)))));

  CHECK(index.getSidesOfLeaf(0) == std::vector<Pair>{{0, 1}});
  CHECK(index.getSidesOfLeaf(1) == std::vector<Pair>{{0, 2}, {1, 2}});
  CHECK(index.getSidesOfLeaf(2) == std::vector<Pair>{{0, 1}, {0, 2}});
  CHECK(index.getInterfaces() == std::vector<Pair>{{0, 1}, {0, 2}, {1, 2}});

  // A third region far away, moved: its box meets neither of the others.
  const auto far = Translate(SphereSDF<T>(Vec3::zeros(), T(0.5)), Vec3(T(10), T(0), T(0)));

  const ExprIndex<T> apart(
    Overlay(std::vector<Expr<T>>{SphereSDF<T>(Vec3::zeros(), T(1)), SphereSDF<T>(Vec3::zeros(), T(0.5)), far}));

  CHECK(apart.getSidesOfLeaf(2) == std::vector<Pair>{{0, 3}});

  // An expression with no Overlay: the object and the background.
  CHECK(ExprIndex<T>(toExpr(SphereSDF<T>(Vec3::zeros(), T(1)))).getInterfaces() == std::vector<Pair>{{0, 1}});
}

TEMPLATE_TEST_CASE(
  "SurfaceGroups: a solver's groups of surfaces, from pieces, regions, interfaces, labels and materials",
  "[Overlay][SurfaceGroups]",
  EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Tape = Tape<T>;

  // The cable: the outside 0, the jacket 1 (PTFE), the core 2 (copper), its surface labeled "conductor".
  const auto core   = MaterialLabel(SurfaceLabel(SphereSDF<T>(Vec3::zeros(), T(0.5)), "conductor"), "copper");
  const auto jacket = MaterialLabel(SphereSDF<T>(Vec3::zeros(), T(1)), "PTFE");

  const Expr<T> cable = Overlay(jacket, core);

  Pool pool(hostMemoryResource());

  const Tape         tape(pool, cable);
  const ExprIndex<T> index(cable);

  const SurfaceRef<T> outer = tape.closestSurface(Vec3(T(1), T(0), T(0)), 1U);   // the jacket and the outside
  const SurfaceRef<T> inner = tape.closestSurface(Vec3(T(0.5), T(0), T(0)), 1U); // the jacket and the core

  CHECK(outer.m_ownerMaterial == index.findMaterialLabel("PTFE"));
  CHECK(outer.m_neighborMaterial.isNone());

  // Electrostatics: every surface of the conductor, inside the jacket included (dirichlet); the jacket's outside
  // (a jump).
  DomainSetup electrostatics;

  const uint32_t dirichlet = electrostatics.addSurfaceGroup();
  const uint32_t jump      = electrostatics.addSurfaceGroup();

  electrostatics.add(dirichlet, SurfaceSelector::label(index.findSurfaceLabel("conductor")));
  electrostatics.add(jump, SurfaceSelector::interface(1, 0));

  CHECK(electrostatics.surfaceGroupOf(inner) == dirichlet);
  CHECK(electrostatics.surfaceGroupOf(outer) == jump);

  // CFD outside: every surface facing the outside (no-slip); the inner one is no surface to it.
  DomainSetup cfd;

  const uint32_t noSlip = cfd.addSurfaceGroup();

  cfd.add(noSlip, SurfaceSelector::region(0));

  CHECK(cfd.surfaceGroupOf(outer) == noSlip);
  CHECK(cfd.surfaceGroupOf(inner) == SurfaceGroups::None);

  // The piece clicked matches itself; another part or other sides do not.
  const SurfaceSelector piece = SurfaceSelector::piece(inner);

  CHECK(piece.matches(inner));
  CHECK(!piece.matches(outer));
  CHECK(!piece.withPart(inner.m_part + 1U).matches(inner));
  CHECK(SurfaceSelector::surface(inner.m_surface).matches(inner));

  // By materials, either way round, or one on either side.
  CHECK(SurfaceSelector::materials(index.findMaterialLabel("copper"), index.findMaterialLabel("PTFE")).matches(inner));
  CHECK(SurfaceSelector::materials(index.findMaterialLabel("PTFE"), index.findMaterialLabel("copper")).matches(inner));
  CHECK(!SurfaceSelector::materials(index.findMaterialLabel("PTFE"), index.findMaterialLabel("copper")).matches(outer));
  CHECK(SurfaceSelector::materials(index.findMaterialLabel("copper")).matches(inner));
  CHECK(!SurfaceSelector::materials(index.findMaterialLabel("copper")).matches(outer));

  // A selector is undone; the first that matches decides.
  cfd.remove(noSlip, SurfaceSelector::region(0));

  CHECK(cfd.getNumSelectors() == 0U);
  CHECK(cfd.surfaceGroupOf(outer) == SurfaceGroups::None);

  // Built in a pool, the surface groups answer as the setup does, on the host or a device.
  cfd.add(noSlip, SurfaceSelector::region(0));

  const SurfaceGroups builtElectrostatics = electrostatics.buildSurfaceGroups(pool);
  const SurfaceGroups builtCfd            = cfd.buildSurfaceGroups(pool);

  for (const SurfaceRef<T>& s : {inner, outer}) {
    CHECK(builtElectrostatics.surfaceGroupOf(s) == electrostatics.surfaceGroupOf(s));
    CHECK(builtCfd.surfaceGroupOf(s) == cfd.surfaceGroupOf(s));
  }

  CHECK(SurfaceGroups().surfaceGroupOf(inner) == SurfaceGroups::None);

  // No limit on surface groups or selectors.
  DomainSetup many;

  for (uint32_t g = 0; g < 100U; g++) {
    many.add(many.addSurfaceGroup(), SurfaceSelector::interface(g + 3U, g + 4U));
  }

  many.add(many.addSurfaceGroup(), SurfaceSelector::interface(1, 0));

  const SurfaceGroups builtMany = many.buildSurfaceGroups(pool);

  CHECK(builtMany.getNumSurfaceGroups() == 101U);
  CHECK(builtMany.surfaceGroupOf(outer) == 100U);

  REQUIRE(abortsWith([] { DomainSetup().add(0, SurfaceSelector()); }, "DomainSetup: surface group 0 of 0"));
}

TEMPLATE_TEST_CASE("Domain: an empty domain, one whose tables live apart from its tape, and the setups refused",
                   "[Overlay][Tape][Domain]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // The cable: the outside (0), the jacket (1), the core (2).
  Pool pool(hostMemoryResource());
  Pool apart(hostMemoryResource());

  const Tape<T> tape(pool, cable<T>());
  const Vec3    inJacket(T(0.75), T(0), T(0));

  // No subdomain and no surface group: one label, Excluded, everywhere, its function negative everywhere; no surface
  // in a group.
  const Domain empty(pool, tape, DomainSetup());

  CHECK(empty.getNumSubdomainFunctions() == 1U);
  CHECK(empty.subdomainAt(inJacket) == Domain<T>::Excluded);
  CHECK(empty.surfaceGroupOf(tape.closestSurface(inJacket, 1U)) == SurfaceGroups::None);

  // A subdomain must hold regions the tape has.
  REQUIRE(abortsWith(
    [&] {
      DomainSetup beyond;

      (void)beyond.addSubdomain({5});
      (void)Domain(apart, tape, beyond);
    },
    "DomainSetup: a subdomain holds region 5, the tape has 3 regions"));

  // The tables in a pool of their own: the tape stays where it is, and each moves onto its own pool's mirror.
  DomainSetup outside;

  (void)outside.addSubdomain({0});
  outside.add(outside.addSurfaceGroup(), SurfaceSelector::region(1));

  const Domain kept(apart, tape, outside);

  pool.freeze();
  apart.freeze();

  // A frozen pool takes no domain.
  REQUIRE(abortsWith([&] { (void)Domain(pool, tape, outside); }, "Pool::reserve: cannot reserve from a frozen pool"));

  const Domain emptyView = empty.rebasedView(pool);
  const Domain keptView  = kept.rebasedView(pool, apart);

  T value[1];

  emptyView.signedDistances(inJacket, value);

  CHECK(emptyView.subdomainAt(inJacket) == Domain<T>::Excluded);
  CHECK(value[0] < T(0));

  const Vec3 far(T(3), T(0), T(0));

  CHECK(keptView.subdomainAt(far) == 0U);
  CHECK(keptView.subdomainAt(inJacket) == Domain<T>::Excluded);
  CHECK(keptView.surfaceGroupOf(keptView.closestSurface(far, 0U)) == 0U);
}

TEST_CASE("SurfaceGroups: removing a surface group removes its selectors and renumbers the surface groups after it",
          "[SurfaceGroups]")
{
  DomainSetup surfaceGroups;

  const uint32_t first  = surfaceGroups.addSurfaceGroup();
  const uint32_t second = surfaceGroups.addSurfaceGroup();
  const uint32_t third  = surfaceGroups.addSurfaceGroup();

  surfaceGroups.add(first, SurfaceSelector::region(1));
  surfaceGroups.add(second, SurfaceSelector::region(2));
  surfaceGroups.add(third, SurfaceSelector::interface(0, 1));

  surfaceGroups.removeSurfaceGroup(second);

  REQUIRE(surfaceGroups.getNumSurfaceGroups() == 2U);
  REQUIRE(surfaceGroups.getNumSelectors() == 2U);
  CHECK(surfaceGroups.getSurfaceGroupOfSelector(0) == 0U);
  CHECK(surfaceGroups.getSelector(0).m_regionA == 1U);
  CHECK(surfaceGroups.getSurfaceGroupOfSelector(1) == 1U);
  CHECK(surfaceGroups.getSelector(1).m_regionB == 1U);

  RegionNames names;

  names.setSurfaceGroup(0, "dirichlet");
  names.setSurfaceGroup(1, "neumann");
  names.setSurfaceGroup(2, "jump");
  names.removeSurfaceGroup(1);

  CHECK(names.surfaceGroup(1) == "1 jump");

  // A surface named, and one picked by its labels (a mesh's face groups); the others by their parts.
  names.setSurface(3, "mesh, model.ebg:4");
  names.setPickedByLabel(3);

  CHECK(names.surface(3, false) == "mesh, model.ebg:4");
  CHECK(names.pickedByLabel(3));
  CHECK_FALSE(names.pickedByLabel(2));
  CHECK_FALSE(names.pickedByLabel(7));

  names.setPickedByLabel(3, false);

  CHECK_FALSE(names.pickedByLabel(3));

  // The remaining surfaces are tried last, even when added first.
  DomainSetup ordered;

  const uint32_t other = ordered.addSurfaceGroup();
  const uint32_t core  = ordered.addSurfaceGroup();

  ordered.add(other, SurfaceSelector::rest());
  ordered.add(core, SurfaceSelector::region(2));

  SurfaceRef<double> wall;

  wall.m_ownerRegion    = 2;
  wall.m_neighborRegion = 1;

  SurfaceRef<double> skin;

  skin.m_ownerRegion    = 1;
  skin.m_neighborRegion = 0;

  CHECK(ordered.surfaceGroupOf(wall) == core);
  CHECK(ordered.surfaceGroupOf(skin) == other);
}

TEST_CASE("Domain: its regions, the surfaces it touches, and its surface groups named per domain", "[Domain]")
{
  // Electrostatics in the air (0) and the jacket (1); flow in the air only.
  DomainSetup electrostatics;

  (void)electrostatics.addSubdomain({0});
  (void)electrostatics.addSubdomain({1});

  DomainSetup flow;

  (void)flow.addSubdomain({0});

  CHECK(electrostatics.computesIn(1));
  CHECK(!electrostatics.computesIn(2));
  CHECK(electrostatics.touches(2, 1));
  CHECK(!flow.touches(2, 1));
  CHECK(flow.touches(1, 0));

  RegionNames names;

  CHECK(names.getActiveDomain() == RegionNames::None);

  const uint32_t first  = names.addDomain("electrostatics", electrostatics);
  const uint32_t second = names.addDomain("flow", flow);

  CHECK(names.getNumDomains() == 2U);
  CHECK(names.getActiveDomain() == first);
  CHECK(names.domainName(second) == "flow");

  // Each domain's surface groups have names of their own.
  names.setSurfaceGroup(first, 0, "dirichlet");
  names.setSurfaceGroup(second, 0, "wall");

  CHECK(names.surfaceGroup(0, false) == "dirichlet");

  names.setActiveDomain(second);

  CHECK(names.surfaceGroup(0, false) == "wall");

  names.setSubdomain(second, 0, "air");

  CHECK(names.subdomain(0, false) == "air");
  CHECK(names.subdomain(1) == "subdomain 1");
  CHECK(names.getDomain(second).computesIn(0));
}

TEMPLATE_TEST_CASE("findSurfacePieces: every surface between every pair of regions, once each, with its parts",
                   "[Overlay][SurfacePieces]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Tape = Tape<T>;

  // The cable (jacket radius 1, region 1; core radius 0.5, region 2; air 0) with x > 0, 0 < z < 1 cut away. By hand:
  // the jacket's skin (1 | 0), the core's wall in the jacket (2 | 1), and the box's cut faces, x = 0 and z = 0 (its
  // parts), through both regions: one piece against each (1 | 0 and 2 | 0), each with those two parts. The face
  // z = 1 only touches the jacket at its top.
  const Expr<T> cut = Difference(cable<T>(), BoxSDF<T>(Vec3(T(0), T(-2), T(0)), Vec3(T(2), T(2), T(1))));

  Pool pool(hostMemoryResource());

  const Tape tape(pool, cut);

  const std::vector<SurfacePiece<T>> pieces = findSurfacePieces(tape, 32);

  REQUIRE(pieces.size() == 4U);

  uint32_t coreWall = 0;
  uint32_t onCore   = 0;
  uint32_t onAir    = 0;

  for (const SurfacePiece<T>& piece : pieces) {
    const SurfaceRef<T>& s = piece.m_surface;

    // The later region inside, and the normal pointing out of it: the point a little along it is outside.
    CHECK(s.m_ownerRegion > s.m_neighborRegion);
    CHECK(tape.region(piece.m_point + T(1e-3) * piece.m_normal) == s.m_neighborRegion);
    CHECK(tape.region(piece.m_point - T(1e-3) * piece.m_normal) == s.m_ownerRegion);
    CHECK(piece.m_crossings > 0U);

    // The box's pieces cross two of its faces; the spheres have one part.
    CHECK(piece.m_parts.size() == ((s.m_surface == 2U) ? 2U : 1U));

    coreWall += (s.m_ownerRegion == 2U && s.m_neighborRegion == 1U) ? 1U : 0U;
    onCore += (s.m_ownerRegion == 2U) ? 1U : 0U;
    onAir += (s.m_neighborRegion == 0U) ? 1U : 0U;
  }

  CHECK(coreWall == 1U);
  CHECK(onCore == 2U);
  CHECK(onAir == 3U);

  // A finer grid finds the same pieces.
  CHECK(findSurfacePieces(tape, 64).size() == 4U);

  // The pieces the index says the object may have hold every piece found.
  const ExprIndex<T>               index(cut);
  const SurfaceInventory           inventory(index);
  const std::vector<PossiblePiece> possible = possiblePieces(inventory);

  CHECK(inventory.getNumSurfaces() == 3U);
  CHECK(inventory.getNumParts(0) == 1U);
  CHECK(inventory.getNumParts(2) == 6U);
  CHECK(index.getNumPartsOfLeaf(2) == 6U);

  for (const SurfacePiece<T>& piece : pieces) {
    const SurfaceRef<T>& s = piece.m_surface;

    CHECK(std::any_of(possible.begin(), possible.end(), [&s](const PossiblePiece& a_possible) {
      return a_possible.m_surface == s.m_surface && a_possible.m_ownerRegion == s.m_ownerRegion &&
             a_possible.m_neighborRegion == s.m_neighborRegion;
    }));
  }

  // How surface groups cover them: the core's surfaces wholly; one face of the box against the jacket partly.
  const auto find = [&possible](const uint32_t a_surface, const uint32_t a_inside, const uint32_t a_outside) {
    return *std::find_if(possible.begin(), possible.end(), [&](const PossiblePiece& a_possible) {
      return a_possible.m_surface == a_surface && a_possible.m_ownerRegion == a_inside &&
             a_possible.m_neighborRegion == a_outside;
    });
  };

  DomainSetup surfaceGroups;

  const uint32_t core = surfaceGroups.addSurfaceGroup();
  const uint32_t face = surfaceGroups.addSurfaceGroup();

  surfaceGroups.add(core, SurfaceSelector::region(2));

  SurfaceRef<T> boxOnJacket;

  boxOnJacket.m_surface        = 2;
  boxOnJacket.m_part           = 0;
  boxOnJacket.m_ownerRegion    = 1;
  boxOnJacket.m_neighborRegion = 0;

  surfaceGroups.add(face, SurfaceSelector::piece(boxOnJacket).withPart(0));

  CHECK(coverageOf(find(1, 2, 1), inventory, surfaceGroups).m_coverage == Coverage::Full);
  CHECK(coverageOf(find(1, 2, 1), inventory, surfaceGroups).m_surfaceGroup == core);
  CHECK(coverageOf(find(2, 1, 0), inventory, surfaceGroups).m_coverage == Coverage::Partly);
  CHECK(coverageOf(find(0, 1, 0), inventory, surfaceGroups).m_coverage == Coverage::None);

  // Judged on some of its parts (those a ray found between its two regions): the face the group holds is all of it;
  // a list naming none of the box's parts judges every part.
  CHECK(coverageOf(find(2, 1, 0), inventory, surfaceGroups, {0U}).m_coverage == Coverage::Full);
  CHECK(coverageOf(find(2, 1, 0), inventory, surfaceGroups, {0U}).m_surfaceGroup == face);
  CHECK(coverageOf(find(2, 1, 0), inventory, surfaceGroups, {0U, 1U}).m_coverage == Coverage::Partly);
  CHECK(coverageOf(find(2, 1, 0), inventory, surfaceGroups, {}).m_coverage == Coverage::Partly);
  CHECK(coverageOf(find(2, 1, 0), inventory, surfaceGroups, {99U}).m_coverage == Coverage::Partly);

  // The remaining surfaces cover the rest, whatever the order they were added in; the core's stay the core's.
  DomainSetup rest = surfaceGroups;

  rest.add(face, SurfaceSelector::rest());

  for (const PossiblePiece& piece : possible) {
    CHECK(coverageOf(piece, inventory, rest).m_coverage == Coverage::Full);
  }

  CHECK(coverageOf(find(1, 2, 1), inventory, rest).m_surfaceGroup == core);
}

TEMPLATE_TEST_CASE("RegionName: names a region apart from its material, unique, only where a region is",
                   "[Overlay][RegionName]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto wire = [](const T a_x) {
    return MaterialLabel(SphereSDF<T>(Vec3T<T>(a_x, T(0), T(0)), T(0.2)), "copper");
  };

  // Two copper wires, told apart by their names; a name under a map is the region's; a named region group.
  const Expr<T> pair = RegionName(
    Overlay(RegionName(wire(T(-1)), "left"), RegionName(Translate(wire(T(0)), Vec3T<T>(T(1), T(0), T(0))), "right")),
    "wires");
  const Expr<T> cable =
    Overlay(MaterialLabel(BoxSDF<T>(Vec3T<T>(T(-2), T(-1), T(-1)), Vec3T<T>(T(2), T(1), T(1))), "PTFE"), pair);

  const ExprIndex<T> index(cable);

  REQUIRE(index.getNumRegions() == 4U);
  CHECK(index.getRegion(0).m_name.empty());
  CHECK(index.getRegion(1).m_name.empty());
  CHECK(index.getRegion(2).m_name == "left");
  CHECK(index.getRegion(3).m_name == "right");
  CHECK(index.getMaterialOf(2) == index.getMaterialOf(3));
  CHECK(index.findRegion("right") == 3U);
  CHECK(index.findRegion("PTFE") == ExprIndex<T>::None);
  CHECK(index.getRegionGroup(1).m_name == "wires");

  const RegionNames names{index};

  CHECK(names.region(0) == "0 background");
  CHECK(names.region(1) == "region 1 (PTFE)");
  CHECK(names.region(2) == "2 left (copper)");
  CHECK(names.region(3) == "3 right (copper)");

  // The same name twice; a name on part of a solid.
  REQUIRE(abortsWith([&] { (void)ExprIndex<T>(Overlay(RegionName(wire(T(0)), "w"), RegionName(wire(T(1)), "w"))); },
                     "ExprIndex: two regions are named \"w\""));
  REQUIRE(abortsWith([&] { (void)Union(RegionName(wire(T(0)), "w"), wire(T(1))); },
                     "Union: MaterialLabel and RegionName name a region"));
}

TEMPLATE_TEST_CASE("RegionNames: regions by their materials, surfaces by their labels, groups as named, else numbers",
                   "[Overlay][RegionNames]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // The cable with its core labeled; a region without a material.
  const Expr<T> labeled = Overlay(MaterialLabel(SphereSDF<T>(Vec3T<T>::zeros(), T(1)), "PTFE"),
                                  SurfaceLabel(SphereSDF<T>(Vec3T<T>::zeros(), T(0.5)), "conductor"));

  RegionNames names{ExprIndex<T>(labeled)};

  CHECK(names.region(0) == "0 background");
  CHECK(names.region(1) == "region 1 (PTFE)");
  CHECK(names.region(2) == "region 2");
  CHECK(names.region(7) == "region 7");
  CHECK(names.material(0) == "0 PTFE");
  CHECK(names.surface(0) == "surface 0");
  CHECK(names.surface(1) == "1 conductor");
  CHECK(names.surfaceLabel(0) == "0 conductor");

  names.setSurfaceGroup(1, "jump");

  CHECK(names.surfaceGroup(0) == "surface group 0");
  CHECK(names.surfaceGroup(1) == "1 jump");
  CHECK(RegionNames().region(1) == "region 1");

  // Colors by name: a material's when colored by material, a surface group's when colored by surface group, and
  // colored by region, a region's material's when the region has none of its own.
  names.setColor("PTFE", Vec3T<double>(0.5, 0.5, 0.5));
  names.setColor("jump", Vec3T<double>(0.0, 0.0, 1.0));

  Vec3T<double> color = Vec3T<double>::zeros();

  CHECK(names.colorOf(RegionColoring::Material, 0, color));
  CHECK(color[0] == 0.5);
  color = Vec3T<double>::zeros();
  CHECK(names.colorOf(RegionColoring::Region, 1, color));
  CHECK(color[0] == 0.5);
  CHECK(!names.colorOf(RegionColoring::Region, 2, color));
  CHECK(names.colorOf(RegionColoring::SurfaceGroup, 1, color));
  CHECK(color[2] == 1.0);
  CHECK(!names.colorOf(RegionColoring::SurfaceGroup, 0, color));
  CHECK(names.materialOf(1) == 0U);
  CHECK(names.materialOf(2) == RegionNames::None);

  // A region's own color wins over its material's.
  const Expr<T> named = Overlay(RegionName(MaterialLabel(SphereSDF<T>(Vec3T<T>::zeros(), T(1)), "PTFE"), "jacket"),
                                RegionName(MaterialLabel(SphereSDF<T>(Vec3T<T>::zeros(), T(0.5)), "PTFE"), "core"));

  RegionNames regions{ExprIndex<T>(named)};

  regions.setColor("PTFE", Vec3T<double>(0.5, 0.5, 0.5));
  regions.setColor("core", Vec3T<double>(1.0, 0.0, 0.0));

  CHECK(regions.colorOf(RegionColoring::Region, 1, color));
  CHECK(color[0] == 0.5);
  CHECK(regions.colorOf(RegionColoring::Region, 2, color));
  CHECK(color[0] == 1.0);
  CHECK(color[1] == 0.0);
}

TEMPLATE_TEST_CASE("ExprIndex: checkLabels reports a label a union covers or an intersection cuts away",
                   "[ExprIndex][checkLabels]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  // The warnings for an expression, its shapes evaluated through its tape.
  const auto check = [&pool](const Expr<T>& a_expr) {
    return ExprIndex<T>(a_expr).checkLabels(a_expr, Tape<T>(pool, a_expr));
  };

  const auto shell = SurfaceLabel(SphereSDF<T>(Vec3::zeros(), T(1)), "shell");
  const auto core  = SurfaceLabel(SphereSDF<T>(Vec3::zeros(), T(0.5)), "core");

  // A core inside the shell has no surface on the union: the shell's covers it.
  const std::vector<std::string> covered = check(Union(shell, core));

  REQUIRE(covered.size() == 1U);
  CHECK(
    covered[0] ==
    "surface label \"core\" cannot reach the boundary: leaf 1: it lies inside another operand of the union at node 0");

  // Moved, scaled or repeated as a whole, it is still covered.
  CHECK(check(Translate(Scale(Union(shell, core), T(2)), Vec3(T(3), T(0), T(0)))).size() == 1U);
  CHECK(check(FiniteRepetition(Union(shell, core), Vec3(T(3), T(3), T(3)), Vec3::zeros(), Vec3::ones())).size() == 1U);

  // A hole that misses the block cuts nothing: it lies outside the intersection's other operand.
  const auto block   = SurfaceLabel(BoxSDF<T>(-Vec3::ones(), Vec3::ones()), "block");
  const auto farHole = SurfaceLabel(Translate(SphereSDF<T>(Vec3::zeros(), T(0.3)), Vec3(T(5), T(0), T(0))), "far hole");
  const auto nearHole =
    SurfaceLabel(Translate(SphereSDF<T>(Vec3::zeros(), T(0.3)), Vec3(T(1), T(0), T(0))), "near hole");

  const std::vector<std::string> cut = check(Difference(block, Union(farHole, nearHole)));

  REQUIRE(cut.size() == 1U);
  CHECK(cut[0] == "surface label \"far hole\" cannot reach the boundary: leaf 1: it lies outside another operand of "
                  "the intersection at node 0");

  // A rod no one sphere holds, but two together do: each eighth of its box is inside one of them.
  const auto         rod = SurfaceLabel(BoxSDF<T>(Vec3(T(-1), T(-0.2), T(-0.2)), Vec3(T(1), T(0.2), T(0.2))), "rod");
  const SphereSDF<T> left(Vec3(T(-0.6), T(0), T(0)), T(0.8));
  const SphereSDF<T> right(Vec3(T(0.6), T(0), T(0)), T(0.8));

  CHECK(check(Union(std::vector<Expr<T>>{rod, left, right})).size() == 1U);

  // A label is lost only when every leaf carrying it is: one bead sticks out of the shell.
  const SphereSDF<T> bead(Vec3::zeros(), T(0.2));

  const auto inside   = SurfaceLabel(Union(bead, Translate(bead, Vec3(T(0.3), T(0), T(0)))), "beads");
  const auto stickOut = SurfaceLabel(Union(bead, Translate(bead, Vec3(T(1), T(0), T(0)))), "beads");

  CHECK(check(Union(shell, inside)).size() == 1U);
  CHECK(check(Union(shell, stickOut)).empty());

  // A label every leaf below it overrides is on no surface.
  const auto apart = SurfaceLabel(Translate(bead, Vec3(T(3), T(0), T(0))), "bead");

  const std::vector<std::string> shadowed = check(SurfaceLabel(Union(shell, apart), "outer"));

  REQUIRE(shadowed.size() == 1U);
  CHECK(shadowed[0] == "surface label \"outer\" is on no surface: every leaf below it has a label of its own");
}

TEMPLATE_TEST_CASE("ExprIndex: checkLabels reports only what it proves, and evaluates a mesh through the tape",
                   "[ExprIndex][checkLabels]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  const auto check = [&pool](const Expr<T>& a_expr) {
    return ExprIndex<T>(a_expr).checkLabels(a_expr, Tape<T>(pool, a_expr));
  };

  const SphereSDF<T> outer(Vec3::zeros(), T(1));
  const auto         core = SurfaceLabel(SphereSDF<T>(Vec3::zeros(), T(0.5)), "core");

  // Operations on the values, rather than the sets, leave a label unproved.
  CHECK(check(SmoothUnion(outer, core, T(0.1))).empty());
  CHECK(check(Offset(Union(outer, core), T(0.1))).empty());
  CHECK(check(Blur(Union(outer, core), T(0.1), T(0.5))).empty());

  // Surfaces that coincide exactly are not proved: a core filling the cavity it was cut from.
  CHECK(check(Union(Difference(outer, core), core)).empty());

  // A sphere inside a mesh: the mesh is evaluated through the tape.
  constexpr size_t K = BVH::DefaultBranchingRatio<T>();
  constexpr size_t W = TriangleSoA::DefaultWidth<T>();

  const auto mesh = Parser::readIntoDCEL<T>(std::string(EBGEOMETRY_TEST_DATA_DIR) + "/dodecahedron.stl", pool);
  const TriMeshSDF<T, K, W> tri(mesh, pool, BVH::Construction::SAH, 4);

  const auto pebble = SurfaceLabel(SphereSDF<T>(Vec3::zeros(), T(0.1)), "pebble");

  REQUIRE(tri.signedDistance(Vec3::zeros()) < T(-0.3));
  CHECK(check(Union(tri, pebble)).size() == 1U);
  CHECK(check(Union(tri, Translate(pebble, Vec3(T(3), T(0), T(0))))).empty());
}

TEMPLATE_TEST_CASE("ExprIndex: checkLabels reports regions with no volume and materials on none",
                   "[ExprIndex][checkLabels][Overlay]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  const auto check = [&pool](const Expr<T>& a_expr) {
    return ExprIndex<T>(a_expr).checkLabels(a_expr, Tape<T>(pool, a_expr));
  };

  const auto small = RegionName(MaterialLabel(SphereSDF<T>(Vec3::zeros(), T(0.5)), "steel"), "small");
  const auto big   = RegionName(MaterialLabel(SphereSDF<T>(Vec3::zeros(), T(1)), "water"), "big");

  // A small region under a big one has no volume, and its material is on no region; on top of it, it has both.
  const std::vector<std::string> buried = check(Overlay(small, big));

  REQUIRE(buried.size() == 2U);
  CHECK(buried[0] == "region 1 \"small\" has no volume: it lies under later regions of the Overlay at node 0");
  CHECK(buried[1] == "material \"steel\" is on no region with volume");
  CHECK(check(Overlay(big, small)).empty());

  // A region outside the mask has no volume: the box cut from the Overlay misses it.
  const auto apart = RegionName(Translate(SphereSDF<T>(Vec3::zeros(), T(0.3)), Vec3(T(3), T(0), T(0))), "apart");

  const std::vector<std::string> masked =
    check(Intersection(Overlay(big, apart), BoxSDF<T>(Vec3(T(-2), T(-2), T(-2)), Vec3(T(2), T(2), T(2)))));

  REQUIRE(masked.size() == 1U);
  CHECK(masked[0] == "region 2 \"apart\" has no volume: it lies outside another operand of the intersection at node 0");
  // Another expression than the index's.
  REQUIRE(abortsWith(
    [&pool] {
      const SphereSDF<T> ball(Vec3::zeros(), T(1));
      const Expr<T>      other = Union(ball, SphereSDF<T>(Vec3::zeros(), T(2)));

      (void)ExprIndex<T>(toExpr(ball)).checkLabels(other, Tape<T>(pool, other));
    },
    "ExprIndex: checkLabels was given an expression of"));
}

TEMPLATE_TEST_CASE(
  "Tape: firstRegionChange() finds a marched shape's surface wherever firstHit() does, grazing rays too",
  "[Overlay][Tape]",
  EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  // Shapes with no ray form (marched), each the one region of an Overlay: a crossing the march stops short of, at a
  // rounded edge or a flat face seen at a shallow angle, must still change the region.
  const std::vector<Expr<T>> shapes{toExpr(RoundedCylinderSDF<T>(Vec3::zeros(), T(1), T(0.3), T(2))),
                                    toExpr(RoundedCylinderSDF<T>(Vec3::zeros(), T(1), T(0.02), T(0.2))),
                                    toExpr(RoundedBoxSDF<T>(Vec3::zeros(), Vec3(T(2), T(0.4), T(2)), T(0.1))),
                                    toExpr(TorusSDF<T>(Vec3::zeros(), T(1), T(0.2)))};

  for (size_t s = 0; s < shapes.size(); s++) {
    const Tape<T> plain(pool, shapes[s]);
    const Tape<T> regions(pool, Overlay(std::vector<Expr<T>>{RegionName(shapes[s], "a")}));

    size_t hits = 0;

    // Rays from a ring around the shape, from low (grazing the flat faces) to high, aimed across it.
    for (int i = 0; i < 24; i++) {
      for (int j = 0; j < 12; j++) {
        const T    angle = T(2) * T(3.14159265358979) * T(i) / T(24);
        const T    y     = T(-1.2) + T(0.21) * T(j);
        const Vec3 from(T(4) * std::cos(angle), y + T(0.35), T(4) * std::sin(angle));
        const Vec3 to(T(-0.3) * std::cos(angle), y, T(0.2) * std::sin(angle));

        const Vec3    direction = (to - from) / (to - from).length();
        const RayT<T> ray{from, direction};

        const HitT<T>        hit    = plain.firstHit(ray, T(0), T(20));
        const SurfaceHitT<T> change = regions.firstRegionChange(ray, T(0), T(20));

        INFO("shape " << s << ", ray " << i << ", " << j);

        REQUIRE(change.m_hit.hit() == hit.hit());

        if (hit.hit()) {
          hits++;

          // Where the tape itself finds it (a marched crossing short of the surface, within the march's tolerance):
          // the region view shows what the tape shows.
          CHECK_THAT(change.m_hit.m_t, withinAbsT<T>(hit.m_t, 1.0e-4));

          CHECK(change.m_surface.m_ownerRegion == 0U);
          CHECK(change.m_surface.m_neighborRegion == 1U);
        }
      }
    }

    CHECK(hits > 30U);
  }
}

TEMPLATE_TEST_CASE("Tape: the interface tolerance is 0 by default, else a fraction of the box's diagonal or a length",
                   "[Overlay][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Tape = EBGeometry::Tape<T, TapeFeature::Weld>;

  // Two unit cubes side by side: a box from (0, 0, 0) to (2, 1, 1), diagonal sqrt(6).
  const Expr<T> pair =
    Overlay(BoxSDF<T>(Vec3::zeros(), Vec3::ones()), BoxSDF<T>(Vec3(T(1), T(0), T(0)), Vec3(T(2), T(1), T(1))));

  Pool pool(hostMemoryResource());

  const T diagonal = std::sqrt(T(6));

  // By default only exact contacts are welded.
  CHECK(Tape(pool, pair).getInterfaceTolerance() == T(0));

  TapeOptions options;

  options.m_interfaceTolerance = 1e-3;

  CHECK_THAT(Tape(pool, pair, options).getInterfaceTolerance(), withinAbsT<T>(T(1e-3) * diagonal, tightMargin<T>()));

  options.m_interfaceTolerance = 0.0;

  CHECK(Tape(pool, pair, options).getInterfaceTolerance() == T(0));

  // A length, given: it replaces the fraction, whatever the box.
  options.m_interfaceLength = 2.5e-4;

  CHECK_THAT(Tape(pool, pair, options).getInterfaceTolerance(), withinAbsT<T>(T(2.5e-4), tightMargin<T>()));

  options.m_interfaceLength = 0.0;

  CHECK(Tape(pool, pair, options).getInterfaceTolerance() == T(0));

  // A tape without TapeFeature::Weld does not weld: its tolerance is 0, whatever the options.
  CHECK(EBGeometry::Tape<T>(pool, pair).getInterfaceTolerance() == T(0));

  // A half-space has no bounded box.
  CHECK(Tape(pool, Overlay(PlaneSDF<T>(Vec3::zeros(), Vec3(T(0), T(0), T(1))), SphereSDF<T>(Vec3::zeros(), T(1))))
          .getInterfaceTolerance() == T(0));

  const std::string refused = "Tape: TapeOptions::m_interfaceTolerance is";

  REQUIRE(abortsWith(
    [&] {
      Pool        local(hostMemoryResource());
      TapeOptions bad;

      bad.m_interfaceTolerance = -1e-5;

      (void)Tape(local, pair, bad);
    },
    refused));
  REQUIRE(abortsWith(
    [&] {
      Pool        local(hostMemoryResource());
      TapeOptions bad;

      bad.m_interfaceLength = -1.0;

      (void)Tape(local, pair, bad);
    },
    "Tape: TapeOptions::m_interfaceLength is"));

  // A length the precision cannot hold (beyond float's largest value) is refused in float.
  if constexpr (std::is_same_v<T, float>) {
    REQUIRE(abortsWith(
      [&] {
        Pool        local(hostMemoryResource());
        TapeOptions bad;

        bad.m_interfaceLength = 1e39;

        (void)Tape(local, pair, bad);
      },
      "larger than the precision holds"));
  }
}

TEMPLATE_TEST_CASE("findNearGaps: regions a thin layer of the background separates, each pair once, with its gap",
                   "[Overlay][SurfacePieces]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Tape = Tape<T>;

  // Three unit cubes along x: the second a gap of 1e-3 past the first (reported at the default width, 1e-3 of the
  // diagonal, about 3.6e-3), the third 0.1 past the second (too wide).
  const T narrow = T(1e-3);
  const T wide   = T(0.1);

  const Expr<T> row =
    Overlay(BoxSDF<T>(Vec3::zeros(), Vec3::ones()),
            BoxSDF<T>(Vec3(T(1) + narrow, T(0), T(0)), Vec3(T(2) + narrow, T(1), T(1))),
            BoxSDF<T>(Vec3(T(2) + narrow + wide, T(0), T(0)), Vec3(T(3) + narrow + wide, T(1), T(1))));

  Pool pool(hostMemoryResource());

  const Tape tape(pool, row);

  const std::vector<NearGap<T>> gaps = findNearGaps(tape);

  REQUIRE(gaps.size() == 1U);
  CHECK(gaps[0].m_region == 1U);
  CHECK(gaps[0].m_other == 2U);
  CHECK_THAT(gaps[0].m_gap, withinAbsT<T>(narrow, 1e-2 * static_cast<double>(narrow)));
  CHECK(gaps[0].m_point[0] > T(1));
  CHECK(gaps[0].m_point[0] < T(1) + narrow);
  CHECK(tape.region(gaps[0].m_point) == 0U);

  // A wider width finds the second gap too; none is found in an object of one region.
  CHECK(findNearGaps(tape, 0.05).size() == 2U);
  CHECK(findNearGaps(Tape(pool, BoxSDF<T>(Vec3::zeros(), Vec3::ones()))).empty());
}

namespace {

// Two unit cubes along x, the second a gap past the first: copper (region 1, leaf 0) and PTFE (region 2, leaf 1).
template <class T>
Expr<T>
cubePair(const T a_gap)
{
  using Vec3 = Vec3T<T>;

  const auto first  = MaterialLabel(BoxSDF<T>(Vec3::zeros(), Vec3::ones()), "copper");
  const auto second = MaterialLabel(BoxSDF<T>(Vec3(T(1) + a_gap, T(0), T(0)), Vec3(T(2) + a_gap, T(1), T(1))), "PTFE");

  return Overlay(first, second);
}

// The regions a ray along x through the middle of the cubes changes between, in order.
template <class T, class TapeType>
std::vector<std::pair<uint32_t, uint32_t>>
changesAlongX(const TapeType& a_tape, const bool a_backwards)
{
  using Vec3 = Vec3T<T>;

  const T        sign = a_backwards ? T(-1) : T(1);
  const RayT<T>  ray{Vec3(a_backwards ? T(3) : T(-1), T(0.4142), T(0.5858)), Vec3(sign, T(0), T(0))};
  const uint32_t none = 0xFFFFFFFFU;

  std::vector<std::pair<uint32_t, uint32_t>> changes;

  T t = T(0);

  for (int i = 0; i < 16; i++) {
    const SurfaceHitT<T> change = a_tape.firstRegionChange(ray, t, T(4));

    if (!change.m_hit.hit()) {
      break;
    }

    changes.emplace_back(change.m_surface.m_ownerRegion, change.m_surface.m_neighborRegion);

    // The crossing between the cubes is on the second cube's face (leaf 1), from either side.
    if (change.m_surface.m_ownerRegion != 0U && change.m_surface.m_neighborRegion != 0U) {
      CHECK(change.m_surface.m_leaf == 1U);
    }

    t = change.m_hit.m_t;
  }

  (void)none;

  return changes;
}

} // namespace

TEMPLATE_TEST_CASE("Tape: abutting regions share one interface, the later one's face, from both sides",
                   "[Overlay][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Tape = EBGeometry::Tape<T, TapeFeature::Weld>;

  Pool pool(hostMemoryResource());

  // Flush (a gap of 0) at the default tolerance, a gap of 3e-6 of the diagonal (welded at a tolerance of 1e-5 of it;
  // a ray steps 1e-6 of it past a crossing), and one of 1e-4 welded at a tolerance of 1e-3.
  const T diagonal = std::sqrt(T(6));

  TapeOptions fine;
  TapeOptions coarse;

  fine.m_interfaceTolerance   = 1e-5;
  coarse.m_interfaceTolerance = 1e-3;

  for (const auto& [gap, options] : {std::make_pair(T(0), TapeOptions()),
                                     std::make_pair(T(3e-6) * diagonal, fine),
                                     std::make_pair(T(1e-4) * diagonal, coarse)}) {
    INFO("gap " << gap);

    const Tape tape(pool, cubePair<T>(gap), options);

    const ExprIndex<T> index(cubePair<T>(gap));

    const Vec3 inFirst(T(0.9), T(0.5), T(0.5));
    const Vec3 inSecond(T(1.1) + gap, T(0.5), T(0.5));

    // From the first cube: the second cube's face, with the second cube across it.
    const SurfaceRef<T> fromFirst = tape.closestSurface(inFirst, 1U);

    CHECK(fromFirst.m_leaf == 1U);
    CHECK(fromFirst.m_ownerRegion == 1U);
    CHECK(fromFirst.m_neighborRegion == 2U);
    CHECK(index.getMaterialLabelName(fromFirst.m_neighborMaterial) == "PTFE");

    // From the second cube: the same face, with the first cube (not the background) across it.
    const SurfaceRef<T> fromSecond = tape.closestSurface(inSecond, 2U);

    CHECK(fromSecond.m_leaf == 1U);
    CHECK(fromSecond.m_ownerRegion == 2U);
    CHECK(fromSecond.m_neighborRegion == 1U);
    CHECK(index.getMaterialLabelName(fromSecond.m_neighborMaterial) == "copper");

    // The gap belongs to the first cube: its function is negative there, the second's and the background's positive.
    if (gap > T(0)) {
      const Vec3 middle(T(1) + T(0.5) * gap, T(0.5), T(0.5));

      CHECK(tape.region(middle) == 1U);
      CHECK(tape.signedDistance(middle, 1U) < T(0));
      CHECK(tape.signedDistance(middle, 2U) > T(0));
      CHECK(tape.signedDistance(middle, 0U) > T(0));
    }

    // Both functions are zero on the second cube's face, and opposite on its side of it, to within the gap: there the
    // first cube's function is its distance to the first cube, exactly, as away from any contact.
    const Vec3 onFace(T(1) + gap, T(0.5), T(0.5));

    CHECK_THAT(tape.signedDistance(onFace, 1U), withinAbsT<T>(T(0), looseMargin<T>()));
    CHECK_THAT(tape.signedDistance(onFace, 2U), withinAbsT<T>(T(0), looseMargin<T>()));
    CHECK_THAT(tape.signedDistance(inSecond, 1U), withinAbsT<T>(T(0.1) + gap, looseMargin<T>()));
    CHECK_THAT(tape.signedDistance(inSecond, 2U), withinAbsT<T>(T(-0.1), looseMargin<T>()));

    // regionDistances() agrees with signedDistance(p, region), the background included.
    for (const Vec3& p : {inFirst, inSecond, onFace, Vec3(T(1) + T(0.5) * gap, T(0.5), T(0.5))}) {
      tape.regionDistances(p, [&](const uint32_t a_region, const T a_value) {
        CHECK_THAT(a_value, withinAbsT<T>(tape.signedDistance(p, a_region), tightMargin<T>()));
      });
    }

    // A ray through both cubes changes region three times, either way: one crossing between them.
    const std::vector<std::pair<uint32_t, uint32_t>> forward  = changesAlongX<T>(tape, false);
    const std::vector<std::pair<uint32_t, uint32_t>> backward = changesAlongX<T>(tape, true);

    REQUIRE(forward.size() == 3U);
    CHECK(forward[1] == std::make_pair(1U, 2U));
    REQUIRE(backward.size() == 3U);
    CHECK(backward[1] == std::make_pair(2U, 1U));
  }
}

TEMPLATE_TEST_CASE("Tape: a gap wider than the interface tolerance, or any gap at a tolerance of 0, stays background",
                   "[Overlay][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Tape = EBGeometry::Tape<T, TapeFeature::Weld>;

  Pool pool(hostMemoryResource());

  const T diagonal = std::sqrt(T(6));

  TapeOptions fine;

  fine.m_interfaceTolerance = 1e-5;

  // A gap of 1e-4 of the diagonal at a tolerance of 1e-5 of it, and one of 1e-5 (wider than the step a ray takes past
  // a crossing) at the default tolerance, 0.
  for (const auto& [gap, options] :
       {std::make_pair(T(1e-4) * diagonal, fine), std::make_pair(T(1e-5) * diagonal, TapeOptions())}) {
    INFO("gap " << gap);

    const Tape tape(pool, cubePair<T>(gap), options);

    const Vec3 middle(T(1) + T(0.5) * gap, T(0.5), T(0.5));

    CHECK(tape.region(middle) == 0U);
    CHECK(tape.signedDistance(middle, 0U) < T(0));
    CHECK(tape.signedDistance(middle, 1U) > T(0));

    // From the second cube, the background is across its face.
    CHECK(tape.closestSurface(Vec3(T(1.1) + gap, T(0.5), T(0.5)), 2U).m_neighborRegion == 0U);

    // From the first cube, its own face, with the background across.
    const SurfaceRef<T> own = tape.closestSurface(Vec3(T(0.9), T(0.5), T(0.5)), 1U);

    CHECK(own.m_leaf == 0U);
    CHECK(own.m_neighborRegion == 0U);

    // A ray crosses the gap: four changes of region.
    CHECK(changesAlongX<T>(tape, false).size() == 4U);
  }

  // Exactly flush at the default tolerance, 0: an exact tie, which names the later cube's face, with it across, from either
  // side.
  {
    const Tape flush(pool, cubePair<T>(T(0)));

    const SurfaceRef<T> fromFirst  = flush.closestSurface(Vec3(T(0.9), T(0.5), T(0.5)), 1U);
    const SurfaceRef<T> fromSecond = flush.closestSurface(Vec3(T(1.1), T(0.5), T(0.5)), 2U);

    CHECK(fromFirst.m_leaf == 1U);
    CHECK(fromFirst.m_neighborRegion == 2U);
    CHECK(fromSecond.m_leaf == 1U);
    CHECK(fromSecond.m_neighborRegion == 1U);
  }

  // At a tolerance of 1e-5 of the diagonal, the wide gap is reported and the welded one is not; at the default
  // tolerance, 0, both are reported.
  CHECK(findNearGaps(Tape(pool, cubePair<T>(T(1e-4) * diagonal), fine)).size() == 1U);
  CHECK(findNearGaps(Tape(pool, cubePair<T>(T(3e-6) * diagonal), fine)).empty());
  CHECK(findNearGaps(Tape(pool, cubePair<T>(T(3e-6) * diagonal))).size() == 1U);
}

TEMPLATE_TEST_CASE("Tape: two nearly touching spheres in different regions are welded by a neck",
                   "[Overlay][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Tape = EBGeometry::Tape<T, TapeFeature::Weld>;

  Pool pool(hostMemoryResource());

  // Spheres of radius 1 a gap g apart along x, with a tolerance eps of 1e-3 of the diagonal (box 4 + g by 2 by 2). Off
  // the axis by r, each is about g/2 + r^2/2 away, so the weld (their sum below eps) reaches r = sqrt(eps - g).
  TapeOptions options;

  options.m_interfaceTolerance = 1e-3;

  const T gap = T(1e-3);

  const Tape tape(
    pool, Overlay(SphereSDF<T>(Vec3::zeros(), T(1)), SphereSDF<T>(Vec3(T(2) + gap, T(0), T(0)), T(1))), options);

  const T eps  = tape.getInterfaceTolerance();
  const T neck = std::sqrt(eps - gap);

  REQUIRE(eps > gap);

  CHECK(tape.region(Vec3(T(1) + T(0.5) * gap, T(0), T(0))) == 1U);
  CHECK(tape.region(Vec3(T(1) + T(0.5) * gap, T(0.5) * neck, T(0))) == 1U);
  CHECK(tape.region(Vec3(T(1) + T(0.5) * gap, T(2) * neck, T(0))) == 0U);
}

TEMPLATE_TEST_CASE("Tape: faces side by side in one plane are not welded, and functions away from contacts are exact",
                   "[Overlay][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Tape = EBGeometry::Tape<T, TapeFeature::Weld>;

  // A channel bored along a block, flush with its ends (x = 0 and x = 4): the channel's caps lie in the plane of the
  // block's end faces. A tolerance of 1e-3 of the diagonal makes the band wide enough to probe.
  const BoxSDF<T>      block(Vec3::zeros(), Vec3(T(4), T(1), T(2)));
  const CylinderSDF<T> channel(Vec3(T(0), T(0.5), T(1)), Vec3(T(4), T(0.5), T(1)), T(0.25));

  TapeOptions options;

  options.m_interfaceTolerance = 1e-3;

  Pool pool(hostMemoryResource());

  const Tape tape(pool, Overlay(block, channel), options);

  const T eps = tape.getInterfaceTolerance();

  // Just outside the channel's cap, and outside the block's end face beside it: the background, not a weld.
  for (const T y : {T(0.5), T(0.6), T(0.9)}) {
    const Vec3 p(T(-0.25) * eps, y, T(1));

    INFO("y " << y);

    CHECK(tape.region(p) == 0U);
    CHECK(tape.signedDistance(p, 0U) < T(0));
  }

  // A ray along the channel enters it from the background, at the cap.
  const SurfaceHitT<T> entry =
    tape.firstRegionChange(RayT<T>{Vec3(T(-1), T(0.5), T(1)), Vec3(T(1), T(0), T(0))}, T(0), T(8));

  REQUIRE(entry.m_hit.hit());
  CHECK(entry.m_surface.m_ownerRegion == 0U);
  CHECK(entry.m_surface.m_neighborRegion == 2U);

  // Away from the band within the tolerance of both solids, each region's function is the plain one, exactly.
  for (const Vec3& p : {Vec3(T(1), T(0.1), T(0.3)),
                        Vec3(T(2), T(0.5), T(1)),
                        Vec3(T(-0.3), T(0.5), T(1)),
                        Vec3(T(3), T(0.9), T(1.4)),
                        Vec3(T(5), T(2), T(3))}) {
    INFO("point " << p);

    CHECK_THAT(tape.signedDistance(p, 1U),
               withinAbsT<T>(Math::max(block.signedDistance(p), -channel.signedDistance(p)), tightMargin<T>()));
    CHECK_THAT(tape.signedDistance(p, 2U), withinAbsT<T>(channel.signedDistance(p), tightMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("Tape: a weld is continuous at its edges, and a gap just wider than the tolerance stays open",
                   "[Overlay][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Tape = EBGeometry::Tape<T, TapeFeature::Weld>;

  Pool pool(hostMemoryResource());

  TapeOptions options;

  options.m_interfaceLength = 1e-2;

  const T eps = T(1e-2);

  // A welded gap of half the tolerance: along a line across it and its band, the first cube's function changes by
  // no more than a few times the step (no jump at the band's edges).
  {
    const T    gap = T(0.5) * eps;
    const Tape tape(pool, cubePair<T>(gap), options);

    const int steps = 400;
    const T   from  = T(1) - T(3) * eps;
    const T   dx    = T(6) * eps / T(steps);

    T previous = tape.signedDistance(Vec3(from, T(0.4), T(0.6)), 1U);

    for (int i = 1; i <= steps; i++) {
      const T value = tape.signedDistance(Vec3(from + T(i) * dx, T(0.4), T(0.6)), 1U);

      INFO("x " << from + T(i) * dx);

      CHECK(std::abs(value - previous) <= T(4) * dx);

      previous = value;
    }
  }

  // A gap a little wider than the tolerance is the outside across its whole width: no part of it is welded.
  {
    const T    gap = T(1.05) * eps;
    const Tape tape(pool, cubePair<T>(gap), options);

    for (int i = 1; i < 20; i++) {
      const Vec3 p(T(1) + gap * T(i) / T(20), T(0.4), T(0.6));

      INFO("x " << p[0]);

      CHECK(tape.region(p) == 0U);
    }
  }
}

TEMPLATE_TEST_CASE("Tape: a tape without TapeFeature::Weld keeps its regions' solids as they are",
                   "[Overlay][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  // A gap of 3e-6 of the diagonal, which a welding tape closes at a tolerance of 1e-5 of it.
  const T gap = T(3e-6) * std::sqrt(T(6));

  TapeOptions options;

  options.m_interfaceTolerance = 1e-5;

  const Tape<T> plain(pool, cubePair<T>(gap), options);

  const Vec3 middle(T(1) + T(0.5) * gap, T(0.4142), T(0.5858));

  CHECK(plain.getInterfaceTolerance() == T(0));
  CHECK(plain.region(middle) == 0U);
  CHECK(plain.signedDistance(middle, 1U) > T(0));
  CHECK(plain.signedDistance(middle, 0U) < T(0));

  // From the second cube, the outside is across its face; from the first, its own face.
  CHECK(plain.closestSurface(Vec3(T(1.1) + gap, T(0.5), T(0.5)), 2U).m_neighborRegion == 0U);
  CHECK(plain.closestSurface(Vec3(T(0.9), T(0.5), T(0.5)), 1U).m_leaf == 0U);

  const Tape<T, TapeFeature::Weld> welded(pool, cubePair<T>(gap), options);

  CHECK(welded.region(middle) == 1U);
}

TEMPLATE_TEST_CASE("Tape: a welding tape's regions agree with or without masks, and a ray through a flush stack",
                   "[Overlay][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Tape = EBGeometry::Tape<T, TapeFeature::Weld>;

  Pool pool(hostMemoryResource());

  TapeOptions options;

  options.m_interfaceLength = 1e-3;

  // Three unit blocks along x, the first two a welded gap apart, the last two flush; the same clipped by a box
  // larger than them all (a mask above every region, which changes no value here).
  const T gap = T(4e-4);

  const Expr<T> stack = Overlay(BoxSDF<T>(Vec3::zeros(), Vec3::ones()),
                                BoxSDF<T>(Vec3(T(1) + gap, T(0), T(0)), Vec3(T(2) + gap, T(1), T(1))),
                                BoxSDF<T>(Vec3(T(2) + gap, T(0), T(0)), Vec3(T(3) + gap, T(1), T(1))));

  const Expr<T> clipped = Intersection(stack, BoxSDF<T>(-T(10) * Vec3::ones(), T(10) * Vec3::ones()));

  const Tape plain(pool, stack, options);
  const Tape masked(pool, clipped, options);

  for (int i = -8; i <= 8; i++) {
    for (const T x0 : {T(1), T(2)}) {
      const Vec3 p(x0 + T(0.5) * gap + T(i) * T(2e-4), T(0.4142), T(0.5858));

      INFO("point " << p);

      CHECK(plain.region(p) == masked.region(p));
      CHECK(plain.signedDistance(p, plain.region(p)) <= T(0));

      for (uint32_t k = 0; k < plain.getNumRegions(); k++) {
        CHECK_THAT(plain.signedDistance(p, k), withinAbsT<T>(masked.signedDistance(p, k), looseMargin<T>()));
      }

      plain.regionDistances(p, [&](const uint32_t a_region, const T a_value) {
        CHECK_THAT(a_value, withinAbsT<T>(plain.signedDistance(p, a_region), tightMargin<T>()));
      });
    }
  }

  // A ray along x changes region four times: into the first block, across the welded gap, across the flush faces, and
  // out of the last; each crossing between blocks on the later block's face.
  for (const Tape* tape : {&plain, &masked}) {
    const RayT<T> ray{Vec3(T(-1), T(0.4142), T(0.5858)), Vec3(T(1), T(0), T(0))};

    std::vector<std::pair<uint32_t, uint32_t>> changes;

    T t = T(0);

    for (int i = 0; i < 8; i++) {
      const SurfaceHitT<T> change = tape->firstRegionChange(ray, t, T(8));

      if (!change.m_hit.hit()) {
        break;
      }

      changes.emplace_back(change.m_surface.m_ownerRegion, change.m_surface.m_neighborRegion);

      if (change.m_surface.m_ownerRegion != 0U && change.m_surface.m_neighborRegion != 0U) {
        CHECK(change.m_surface.m_leaf == change.m_surface.m_neighborRegion - 1U);
      }

      t = change.m_hit.m_t;
    }

    REQUIRE(changes.size() == 4U);
    CHECK(changes[1] == std::make_pair(1U, 2U));
    CHECK(changes[2] == std::make_pair(2U, 3U));
  }
}

TEMPLATE_TEST_CASE("Tape: where two regions' surfaces cross at an acute angle, the weld leaves a fillet, and none at "
                   "a right or obtuse one",
                   "[Overlay][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Tape = EBGeometry::Tape<T, TapeFeature::Weld>;

  Pool pool(hostMemoryResource());

  const T eps = T(1e-2);

  TapeOptions options;

  options.m_interfaceLength = static_cast<double>(eps);

  // A slab below z = 0 (region 1), and a slab above a plane through the y axis tilted by an angle (region 2): outside
  // both, on one side of the y axis, a wedge whose opening angle is the tilt (acute) or its supplement (obtuse).
  const BoxSDF<T> lower(Vec3(T(-2), T(-1), T(-1)), Vec3(T(2), T(1), T(0)));
  const BoxSDF<T> upper(Vec3(T(-2), T(-1), T(0)), Vec3(T(2), T(1), T(1)));

  for (const T degrees : {T(10), T(90), T(150)}) {
    INFO("tilt " << degrees);

    const Expr<T> tilted = Rotate(Expr<T>(upper), degrees, 1);
    const Tape    tape(pool, Overlay(Expr<T>(lower), tilted), options);

    const EBGeometry::Tape<T> a(pool, Expr<T>(lower));
    const EBGeometry::Tape<T> b(pool, tilted);

    // How much the faces face each other: the cosine between the slab's top and the tilted face's normals.
    const T c = std::max(T(0), std::cos(degrees * T(3.14159265358979) / T(180)));

    uint32_t welded = 0;
    T        reach  = T(0);

    for (int i = -60; i <= 60; i++) {
      for (int k = 1; k <= 30; k++) {
        const Vec3 p(T(i) * T(0.25) * eps, T(0), T(k) * T(0.05) * eps);
        const T    sa = a.signedDistance(p);
        const T    sb = b.signedDistance(p);

        if (!(sa > T(0) && sb > T(0))) {
          continue;
        }

        // Welded where (1 + c)(sa + sb) < 2 eps c, away from the boundary of that set.
        const T measure = (T(1) + c) * (sa + sb) - T(2) * eps * c;

        if (measure < -T(0.2) * eps) {
          CHECK(tape.region(p) == 1U);

          welded++;
          reach = std::max(reach, p.length());
        }
        else if (measure > T(0.2) * eps) {
          CHECK(tape.region(p) == 0U);
        }
      }
    }

    if (degrees < T(90)) {
      // A fillet along the crossing: it reaches about eps / (2 sin(tilt / 2)) along the wedge, here several eps.
      CHECK(welded > 0U);
      CHECK(reach > T(3) * eps);
    }
    else {
      CHECK(welded == 0U);
    }
  }
}
