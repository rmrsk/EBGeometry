// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for regions and materials (Overlay(), MaterialLabel() and ExprIndex's regions, groups and materials):
// the regions of an Overlay against a hand-numbered expression, the background, materials inherited from groups,
// nested Overlays flattened into one priority order, maps and masks kept, a tape's value as the whole object, and
// the rejected placements of a partition and a material.

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"

#include <cstdint>
#include <string>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;

namespace {

// A cable: a copper core inside a PTFE jacket, in air.
template <class T>
Expr<T>
cable()
{
  const auto core   = MaterialLabel(SphereSDF<T>(Vec3T<T>::zeros(), T(0.5)), "copper");
  const auto jacket = MaterialLabel(SphereSDF<T>(Vec3T<T>::zeros(), T(1)), "PTFE");

  return Overlay(jacket, core, "air");
}

// A region's material name, or "-" for none.
template <class T>
std::string
materialName(const ExprIndex<T>& a_index, const uint32_t a_region)
{
  const MaterialLabelId material = a_index.getMaterialOf(a_region);

  return material.isNone() ? std::string("-") : a_index.getMaterialLabelName(material);
}

} // namespace

TEMPLATE_TEST_CASE("Overlay: the background is region 0, then the operands in priority order, with their materials",
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
  CHECK(materialName(index, 0) == "air");
  CHECK(materialName(index, 1) == "PTFE");
  CHECK(materialName(index, 2) == "copper");

  // The background's material is numbered first, then the others as they appear.
  REQUIRE(index.getNumMaterialLabels() == 3U);
  CHECK(index.findMaterialLabel("air").value() == 0U);
  CHECK(index.findMaterialLabel("PTFE").value() == 1U);
  CHECK(index.findMaterialLabel("copper").value() == 2U);
  CHECK(index.findMaterialLabel("steel").isNone());

  REQUIRE(index.getNumGroups() == 1U);
  CHECK(index.getGroup(0).m_node == 0U);
  CHECK(index.getGroup(0).m_parent == None);
  CHECK(index.getGroup(0).m_firstRegion == 1U);
  CHECK(index.getGroup(0).m_numRegions == 2U);
  CHECK(index.getRegion(1).m_group == 0U);

  CHECK(index.getRegionOfLeaf(0) == 1U);
  CHECK(index.getRegionOfLeaf(1) == 2U);

  // Materials and surface labels are numbered apart.
  CHECK(index.getNumSurfaceLabels() == 0U);

  // Without a background the outside has no material; an expression with no Overlay is one region.
  const ExprIndex<T> noBackground(
    Overlay(SphereSDF<T>(Vec3T<T>::zeros(), T(1)), BoxSDF<T>(-Vec3T<T>::ones(), Vec3T<T>::ones())));

  CHECK(noBackground.getNumRegions() == 3U);
  CHECK(materialName(noBackground, 0) == "-");
  CHECK(materialName(noBackground, 1) == "-");

  const ExprIndex<T> plain(MaterialLabel(
    Union(SphereSDF<T>(Vec3T<T>::zeros(), T(1)), BoxSDF<T>(-Vec3T<T>::ones(), Vec3T<T>::ones())), "steel"));

  REQUIRE(plain.getNumRegions() == 2U);
  CHECK(plain.getRegion(1).m_node == 0U);
  CHECK(plain.getRegion(1).m_group == None);
  CHECK(materialName(plain, 1) == "steel");
  CHECK(plain.getNumGroups() == 0U);
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
                                                     Translate(second, Vec3(T(2), T(0), T(0)))},
                                "air");

  const ExprIndex<T> index(scene);

  // Regions: background, matrix, shell 1, grain 1, shell 2, grain 2.
  REQUIRE(index.getNumRegions() == 6U);
  CHECK(materialName(index, 0) == "air");
  CHECK(materialName(index, 1) == "resin");
  CHECK(materialName(index, 2) == "steel");
  CHECK(materialName(index, 3) == "steel");
  CHECK(materialName(index, 4) == "steel");
  CHECK(materialName(index, 5) == "carbon");

  // Groups, depth first: the scene, then each particle, a contiguous range inside the scene's.
  REQUIRE(index.getNumGroups() == 3U);
  CHECK(index.getGroup(0).m_firstRegion == 1U);
  CHECK(index.getGroup(0).m_numRegions == 5U);
  CHECK(index.getGroup(0).m_parent == None);
  CHECK(index.getGroup(1).m_firstRegion == 2U);
  CHECK(index.getGroup(1).m_numRegions == 2U);
  CHECK(index.getGroup(1).m_parent == 0U);
  CHECK(index.getGroup(2).m_firstRegion == 4U);
  CHECK(index.getGroup(2).m_numRegions == 2U);
  CHECK(index.getRegion(1).m_group == 0U);
  CHECK(index.getRegion(3).m_group == 1U);
  CHECK(index.getRegion(5).m_group == 2U);

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
  const char* material  = "MaterialLabel names the material of a region";

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

  REQUIRE(abortsWith([] { (void)Overlay(SphereSDF<T>(Vec3::zeros(), T(2)), cable<T>()); },
                     "Overlay: only the outermost Overlay names the background"));
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
    Intersection(Overlay(std::vector<Expr<T>>{matrix, Overlay(shell, grain), Overlay(shell2, grain2)}, "air"), domain),
    slab);

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

  const Tape<T, TapeFeature::Regions> tape(pool, scene);
  const Tape<T>                       plain(pool, scene);
  const ExprIndex<T>                  index(scene);

  REQUIRE(tape.getNumRegions() == 6U);
  REQUIRE(index.getNumRegions() == 6U);

  // The region programs leave the leaves as the whole expression numbers them.
  CHECK(tape.getNumPrimitives() == plain.getNumPrimitives());

  const Tape<T, TapeFeature::Regions> copy = tape.deepCopy(pool);

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
  const Tape<T, TapeFeature::Regions> one(pool, Union(matrix, shell));

  CHECK(one.getNumRegions() == 2U);
  CHECK(one.region(Vec3::zeros()) == 1U);
  CHECK(one.region(T(5) * Vec3::ones()) == 0U);
  CHECK(one.signedDistance(Vec3::zeros(), 1U) == one.signedDistance(Vec3::zeros()));
  CHECK(one.signedDistance(Vec3::zeros(), 0U) == -one.signedDistance(Vec3::zeros()));
}

TEMPLATE_TEST_CASE("RegionView: the regions' surfaces along a ray, internal ones and hidden regions included",
                   "[Overlay][RegionView]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Tape = Tape<T, TapeFeature::Regions>;

  // The cable (concentric spheres: jacket radius 1, region 1, PTFE; core radius 0.5, region 2, copper; air 0), its
  // half x > 0 cut away along 0 < z < 1.
  const Expr<T> cut = Difference(cable<T>(), BoxSDF<T>(Vec3(T(0), T(-2), T(0)), Vec3(T(2), T(2), T(1))));

  Pool pool(hostMemoryResource());

  const Tape             tape(pool, cut);
  const RegionView<Tape> all(tape);

  REQUIRE(all.getNumRegions() == 3U);
  CHECK(!all.isVisible(0));
  CHECK(all.isVisible(1));
  CHECK(all.isVisible(2));

  const RayOptions<T> options;

  // From the cut side: the core's and the jacket's cut faces, outward normals facing the ray.
  const RayT<T> intoCore{Vec3(T(1.5), T(0.1), T(0.2)), Vec3(T(-1), T(0), T(0))};
  const RayT<T> intoJacket{Vec3(T(1.5), T(0.8), T(0.5)), Vec3(T(-1), T(0), T(0))};

  const HitT<T> coreFace   = all.firstHit(intoCore, T(0), T(5), options);
  const HitT<T> jacketFace = all.firstHit(intoJacket, T(0), T(5), options);

  REQUIRE(coreFace.hit());
  REQUIRE(jacketFace.hit());
  CHECK(coreFace.m_primitive == 2U);
  CHECK(jacketFace.m_primitive == 1U);
  CHECK_THAT(coreFace.m_t, withinAbsT<T>(T(1.5), looseMargin<T>()));
  CHECK(coreFace.m_entering);
  CHECK(dot(coreFace.m_normal, intoCore.m_direction) < T(0));

  // Along x at z = -0.4: the jacket, then, seeing on from inside it, the core's surface inside it.
  const RayT<T> along{Vec3(T(-2), T(0), T(-0.4)), Vec3(T(1), T(0), T(0))};

  const HitT<T> outer = all.firstHit(along, T(0), T(5), options);
  const HitT<T> inner = all.firstHit(along, outer.m_t, T(5), options);

  CHECK_THAT(outer.m_t, withinAbsT<T>(T(2) - std::sqrt(T(0.84)), looseMargin<T>()));
  CHECK(outer.m_primitive == 1U);
  CHECK_THAT(inner.m_t, withinAbsT<T>(T(2) - std::sqrt(T(0.09)), looseMargin<T>()));
  CHECK(inner.m_primitive == 2U);
  CHECK(dot(inner.m_normal, along.m_direction) < T(0));

  // The core alone: the jacket is seen through, and a ray that meets only hidden regions misses.
  const RegionView<Tape> core = all.withOnly(2);

  CHECK(!core.isVisible(1));
  CHECK(core.isVisible(2));
  CHECK_THAT(core.firstHit(along, T(0), T(5), options).m_t, withinAbsT<T>(T(2) - std::sqrt(T(0.09)), looseMargin<T>()));
  CHECK(!core.firstHit(RayT<T>{Vec3(T(-2), T(0.8), T(-0.4)), Vec3(T(1), T(0), T(0))}, T(0), T(5), options).hit());

  // The core seen through: where the ray meets the core, it meets the jacket's bore, the same surface shown as the
  // jacket's; the jacket seen through: the core's wall.
  const RegionView<Tape> coreThrough = all.withSeenThrough(2, true);

  CHECK(coreThrough.isSeenThrough(2));

  const HitT<T> bore = coreThrough.firstHit(along, outer.m_t, T(5), options);

  CHECK(bore.m_primitive == 1U);
  CHECK_THAT(bore.m_t, withinAbsT<T>(T(2) - std::sqrt(T(0.09)), looseMargin<T>()));
  CHECK(all.withSeenThrough(1, true).firstHit(along, T(0), T(5), options).m_primitive == 2U);
  CHECK(!coreThrough.withNoneSeenThrough().isSeenThrough(2));

  // Colored by material: the materials' numbers (air 0, PTFE 1, copper 2).
  const RegionView<Tape> byMaterial = all.withColoring(RegionColoring::Material);

  CHECK(byMaterial.firstHit(intoCore, T(0), T(5), options).m_primitive == 2U);
  CHECK(byMaterial.firstHit(intoJacket, T(0), T(5), options).m_primitive == 1U);

  // Colored by interface: the pair of regions on the surface's two sides; showing only the jacket | core interface,
  // the jacket's outer surface (jacket | air) is passed through, and so is the core's cut face (core | air).
  const RegionView<Tape> byInterface = all.withColoring(RegionColoring::Interface);
  const uint32_t         inside      = RegionView<Tape>::interfaceKey(1, 2);

  CHECK(byInterface.firstHit(along, outer.m_t, T(5), options).m_primitive == inside);
  CHECK(RegionView<Tape>::interfaceKey(2, 1) == inside);

  const RegionView<Tape> onlyInside = byInterface.withOnlyKey(inside);

  const HitT<T> first = onlyInside.firstHit(along, T(0), T(5), options);

  CHECK(first.m_primitive == inside);
  CHECK_THAT(first.m_t, withinAbsT<T>(T(2) - std::sqrt(T(0.09)), looseMargin<T>()));
  // Through the core's cut face (passed through), on to where the ray leaves the core for the jacket.
  CHECK_THAT(onlyInside.firstHit(intoCore, T(0), T(5), options).m_t,
             withinAbsT<T>(T(1.5) + std::sqrt(T(0.25) - T(0.05)), looseMargin<T>()));
  CHECK(onlyInside.withOnlyKey(RegionView<Tape>::None).firstHit(intoCore, T(0), T(5), options).hit());

  // The value: the visible regions' phase function; a cut face's point names its region.
  Phases visible;

  (void)visible.addPhase({1, 2});

  for (const Vec3& p : {Vec3(T(-0.3), T(0.1), T(0.5)), Vec3(T(0.7), T(0), T(-0.5)), Vec3(T(2), T(2), T(2))}) {
    CHECK(all.signedDistance(p) == tape.signedDistance(p, visible, 0U));
  }

  CHECK(all.closestLeaf(Vec3(T(-0.01), T(0.1), T(0.2))).m_leaf == 2U);

  CHECK(all.withVisible(1, false).isVisible(2));
  CHECK(!all.withVisible(1, false).isVisible(1));
  CHECK(all.withOnly(1).withAllVisible().isVisible(2));
  CHECK(hasRegionVisibility<RegionView<Tape>>);
  CHECK(!hasRegionVisibility<Tape>);
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

  const Tape<T, TapeFeature::Regions> tape(pool, Difference(cable<T>(), slab));

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

  const Tape<T, TapeFeature::Regions> nested(pool, Overlay(matrix, Intersection(Overlay(shell, grain), half)));

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
  const Tape<T, TapeFeature::Regions> cableTape(pool, cable<T>());

  const Vec3 outer(T(1), T(0), T(0));
  const Vec3 inner(T(0.5), T(0), T(0));

  // The outer sphere: the jacket's own surface, between the jacket and the air.
  const SurfaceRef<T> jacketOuter = cableTape.closestSurface(outer, 1U);

  CHECK(jacketOuter.m_leaf == 0U);
  CHECK(jacketOuter.m_minusRegion == 1U);
  CHECK(jacketOuter.m_plusRegion == 0U);

  // The inner sphere, from the jacket: an interface with the core, on the core's surface.
  const SurfaceRef<T> jacketInner = cableTape.closestSurface(inner, 1U);

  CHECK(jacketInner.m_leaf == 1U);
  CHECK(jacketInner.m_minusRegion == 1U);
  CHECK(jacketInner.m_plusRegion == 2U);

  // The same interface from the core, and the outer sphere from the air.
  const SurfaceRef<T> core = cableTape.closestSurface(inner, 2U);

  CHECK(core.m_leaf == 1U);
  CHECK(core.m_minusRegion == 2U);
  CHECK(core.m_plusRegion == 1U);

  const SurfaceRef<T> air = cableTape.closestSurface(outer, 0U);

  CHECK(air.m_leaf == 0U);
  CHECK(air.m_minusRegion == 0U);
  CHECK(air.m_plusRegion == 1U);

  // A triple point: a sphere centered on another's surface faces the air on its outer half and the other sphere on
  // its inner half, so its sides belong to the point.
  const SphereSDF<T> big(Vec3::zeros(), T(1));
  const SphereSDF<T> small(Vec3(T(1), T(0), T(0)), T(0.3));

  const Tape<T, TapeFeature::Regions> triple(pool, Overlay(big, small));

  CHECK(triple.closestSurface(Vec3(T(1.3), T(0), T(0)), 2U).m_plusRegion == 0U);
  CHECK(triple.closestSurface(Vec3(T(0.7), T(0), T(0)), 2U).m_plusRegion == 1U);

  // A mask's cut: the slab's face where it cuts the core is the slab's surface, with its label, the core inside
  // and the air across.
  const BoxSDF<T> slab(Vec3(T(0), T(-2), T(-0.3)), Vec3(T(2), T(2), T(0.3)));

  const Expr<T>                       cut = Difference(cable<T>(), SurfaceLabel(slab, "cut"));
  const Tape<T, TapeFeature::Regions> cutTape(pool, cut);
  const ExprIndex<T>                  index(cut);

  const SurfaceRef<T> face = cutTape.closestSurface(Vec3(T(0.1), T(0), T(0.3)), 2U);

  CHECK(face.m_leaf == 2U);
  CHECK(face.m_surfaceLabel == index.findSurfaceLabel("cut"));
  CHECK(face.m_minusRegion == 2U);
  CHECK(face.m_plusRegion == 0U);

  // An object with no Overlay: its surface, between it and the background.
  const Tape<T, TapeFeature::Regions> one(pool, toExpr(big));

  CHECK(one.closestSurface(Vec3(T(1), T(0), T(0)), 1U).m_minusRegion == 1U);
  CHECK(one.closestSurface(Vec3(T(1), T(0), T(0)), 1U).m_plusRegion == 0U);
}

TEMPLATE_TEST_CASE("Phases: a solver's regions give each surface a role, and each phase a function",
                   "[Overlay][Tape][Phases]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  const Tape<T, TapeFeature::Regions> tape(pool, cable<T>());
  const ExprIndex<T>                  index(cable<T>());

  const SurfaceRef<T> outer = tape.closestSurface(Vec3(T(1), T(0), T(0)), 1U);   // the jacket and the air
  const SurfaceRef<T> inner = tape.closestSurface(Vec3(T(0.5), T(0), T(0)), 1U); // the jacket and the core

  // CFD in the air: the outer sphere is its boundary, the inner one nothing to it.
  Phases cfd;

  CHECK(cfd.addPhase({0}) == 0U);
  CHECK(cfd.role(outer) == SurfaceRole::Boundary);
  CHECK(cfd.role(inner) == SurfaceRole::None);

  // Electrostatics in the air and the jacket, the core a conductor: an interface and a boundary.
  Phases electrostatics;

  CHECK(electrostatics.addPhase({0}) == 0U);
  CHECK(electrostatics.addPhase(index.getRegionsOfMaterial(index.findMaterialLabel("PTFE"))) == 1U);
  CHECK(electrostatics.getNumPhases() == 2U);
  CHECK(electrostatics.phaseOf(1) == 1U);
  CHECK(electrostatics.phaseOf(2) == Phases::None);
  CHECK(electrostatics.role(outer) == SurfaceRole::Interface);
  CHECK(electrostatics.role(inner) == SurfaceRole::Boundary);

  // Heat in all three: two interfaces. The jacket and the core as one phase: their contact is internal.
  Phases heat;

  (void)heat.addPhase({0});
  (void)heat.addPhase({1});
  (void)heat.addPhase({2});

  CHECK(heat.role(outer) == SurfaceRole::Interface);
  CHECK(heat.role(inner) == SurfaceRole::Interface);

  Phases solid;

  (void)solid.addPhase(index.getRegionsOfGroup(0));

  CHECK(solid.getNumRanges() == 1U);
  CHECK(solid.role(inner) == SurfaceRole::Internal);
  CHECK(solid.role(outer) == SurfaceRole::Boundary);

  // A phase's function is negative exactly where one of its regions is; the solid's has the whole object's sign.
  for (int i = -15; i <= 15; i++) {
    const Vec3 p(T(0.1) * T(i) + T(0.013), T(0.02), T(0));

    INFO("point " << p);

    CHECK((tape.signedDistance(p, solid, 0U) < T(0)) == (tape.signedDistance(p) < T(0)));
    CHECK((tape.signedDistance(p, electrostatics, 1U) < T(0)) == (tape.region(p) == 1U));
  }

  // The jacket and the core as one phase: on their contact the function is not zero (no sheet inside the phase);
  // across the outer sphere it is the distance, from both sides.
  for (const T x : {T(0.5), T(0.45), T(0.55)}) {
    INFO("x " << x);

    CHECK(tape.signedDistance(Vec3(x, T(0), T(0)), solid, 0U) < -T(0.2));
  }

  CHECK_THAT(tape.signedDistance(Vec3(T(0.95), T(0), T(0)), solid, 0U), withinAbsT<T>(-T(0.05), tightMargin<T>()));
  CHECK_THAT(tape.signedDistance(Vec3(T(1.05), T(0), T(0)), solid, 0U), withinAbsT<T>(T(0.05), tightMargin<T>()));

  // The phase's surface: from its region whose function is smallest.
  const SurfaceRef<T> jacket = tape.closestSurface(Vec3(T(0.5), T(0), T(0)), electrostatics, 1U);

  CHECK(jacket.m_minusRegion == 1U);
  CHECK(jacket.m_plusRegion == 2U);
  CHECK(electrostatics.role(jacket) == SurfaceRole::Boundary);

  REQUIRE(abortsWith([] { (void)Phases().addPhase({}); }, "Phases: a phase needs at least one region"));
  REQUIRE(abortsWith(
    [] {
      Phases phases;

      (void)phases.addPhase({1, 2});
      (void)phases.addPhase({2});
    },
    "Phases: region 2 is already in phase 0"));
}

TEMPLATE_TEST_CASE("Tape: firstRegionChange() tracks a particle through the regions, and firstSurface() a phase",
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

  const Tape<T, TapeFeature::Regions> tape(pool, cut);
  const ExprIndex<T>                  index(cut);

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
      CHECK(change.m_surface.m_minusRegion == leftRegions[i]);
      CHECK(change.m_surface.m_plusRegion == enteredRegions[i]);
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
    CHECK(c.m_surface.m_minusRegion == 2U);
    CHECK(c.m_surface.m_plusRegion == 0U);
    CHECK(c.m_surface.m_surfaceLabel == index.findSurfaceLabel("cut"));
    CHECK(!tape.firstRegionChange(ray, c.m_hit.m_t, T(5)).m_hit.hit());
  }

  // From inside the core, and as a particle's step.
  {
    const SurfaceHitT<T> fromCore = tape.firstRegionChange(Vec3(T(-0.2), T(0), T(0.4)), Vec3(T(2), T(0), T(0.4)));

    REQUIRE(fromCore.m_hit.hit());
    CHECK_THAT(fromCore.m_hit.m_t, withinAbsT<T>(T(0.5), looseMargin<T>()));
    CHECK(fromCore.m_surface.m_minusRegion == 2U);
    CHECK(fromCore.m_surface.m_plusRegion == 1U);
  }

  // Rendering: the first entry into the visible regions, the others seen through.
  {
    const RayT<T> ray{Vec3(T(-2), T(0), T(0.4)), Vec3(T(1), T(0), T(0))};

    Phases jacketOnly;
    Phases coreOnly;
    Phases both;

    (void)jacketOnly.addPhase({1});
    (void)coreOnly.addPhase({2});
    (void)both.addPhase({1, 2});

    const SurfaceHitT<T> jacket = tape.firstSurface(ray, T(0), T(5), jacketOnly, 0U);
    const SurfaceHitT<T> core   = tape.firstSurface(ray, T(0), T(5), coreOnly, 0U);
    const SurfaceHitT<T> solid  = tape.firstSurface(ray, T(0), T(5), both, 0U);

    CHECK_THAT(jacket.m_hit.m_t, withinAbsT<T>(T(2) - std::sqrt(T(0.84)), looseMargin<T>()));
    CHECK_THAT(core.m_hit.m_t, withinAbsT<T>(T(2) - std::sqrt(T(0.09)), looseMargin<T>()));
    CHECK(core.m_surface.m_plusRegion == 2U);
    CHECK(solid.m_surface.m_plusRegion == 1U);

    // Leaving the solid: the contact between the jacket and the core is passed through.
    const SurfaceHitT<T> leave = tape.firstSurface(ray, solid.m_hit.m_t, T(5), both, 0U);

    CHECK_THAT(leave.m_hit.m_t, withinAbsT<T>(T(2) + std::sqrt(T(0.84)), looseMargin<T>()));
    CHECK(leave.m_surface.m_plusRegion == 0U);
  }
}
