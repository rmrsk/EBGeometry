// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for regions and materials (Overlay(), MaterialLabel() and ExprIndex's regions, groups and materials):
// the regions of an Overlay against a hand-numbered expression, the background, materials inherited from groups,
// nested Overlays flattened into one priority order, maps and masks kept, a tape's value as the whole object, and
// the rejected placements of a partition and a material.

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"
#include "TestRenderHelpers.hpp"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;

namespace {

// A cable: a copper core inside a PTFE jacket; the outside, region 0, has no material.
template <class T>
Expr<T>
cable()
{
  const auto core   = MaterialLabel(SphereSDF<T>(Vec3T<T>::zeros(), T(0.5)), "copper");
  const auto jacket = MaterialLabel(SphereSDF<T>(Vec3T<T>::zeros(), T(1)), "PTFE");

  return Overlay(jacket, core);
}

// A region's material name, or "-" for none.
template <class T>
std::string
materialName(const ExprIndex<T>& a_index, const uint32_t a_region)
{
  const MaterialLabelId material = a_index.getMaterialOf(a_region);

  return material.isNone() ? std::string("-") : a_index.getMaterialLabelName(material);
}

// What the device answers at a point: the region, the three regions' functions, the jacket's surface there with
// its sides, and the first change of region along +x.
template <class T>
struct RegionAnswer
{
  uint32_t m_region;
  T        m_values[3];
  uint32_t m_minus;
  uint32_t m_plus;
  uint32_t m_surface;
  uint32_t m_group;
  T        m_t;
  uint32_t m_from;
  uint32_t m_to;

  // Per subdomain of the domain: its function, its surface's role and boundary condition there.
  T           m_subdomainValues[2];
  SurfaceRole m_roles[2];
  uint32_t    m_conditions[2];

  // The domain's label there, and every label's function (the two subdomains', then the excluded regions').
  uint32_t m_label;
  T        m_labelValues[3];
};

// Device functor: the region queries of a tape at a point, and what a downstream code computing on a domain asks
// there, the domain taken whole.
template <class T, class TapeType>
struct RegionQuery
{
  TapeType m_tape;
  Domain   m_domain;

  EBGEOMETRY_HOST_DEVICE
  RegionAnswer<T>
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    RegionAnswer<T> answer;

    answer.m_region = m_tape.region(a_point);

    for (uint32_t k = 0; k < 3; k++) {
      answer.m_values[k] = m_tape.signedDistance(a_point, k);
    }

    const SurfaceRef<T> surface = m_tape.closestSurface(a_point, 1U);

    answer.m_minus   = surface.m_ownerRegion;
    answer.m_plus    = surface.m_neighborRegion;
    answer.m_surface = surface.m_surface;
    answer.m_group   = m_domain.m_scheme.surfaceGroupOf(surface);

    for (uint32_t d = 0; d < 2; d++) {
      const SurfaceRef<T> s = m_tape.closestSurface(a_point, m_domain.m_subdomains, d);

      answer.m_subdomainValues[d] = m_tape.signedDistance(a_point, m_domain.m_subdomains, d);
      answer.m_roles[d]           = m_domain.m_subdomains.role(s);
      answer.m_conditions[d]      = m_domain.m_scheme.surfaceGroupOf(s);
    }

    T values[Domain::MaxLabels];

    m_domain.signedDistances(m_tape, a_point, values);

    answer.m_label = m_domain.labelAt(m_tape, a_point);

    for (uint32_t l = 0; l < 3; l++) {
      answer.m_labelValues[l] = values[l];
    }

    const SurfaceHitT<T> change = m_tape.firstRegionChange(RayT<T>{a_point, Vec3T<T>(T(1), T(0), T(0))}, T(0), T(4));

    answer.m_t    = change.m_hit.hit() ? change.m_hit.m_t : T(-1);
    answer.m_from = change.m_surface.m_ownerRegion;
    answer.m_to   = change.m_surface.m_neighborRegion;

    return answer;
  }
};

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

  CHECK(unlabeled.getNumRegions() == 4U);
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

TEMPLATE_TEST_CASE("RegionView: the regions' surfaces along a ray, internal ones and hidden regions included",
                   "[Overlay][RegionView]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Tape = Tape<T>;

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

  // Colored by material: the materials' numbers (PTFE 0, copper 1).
  const RegionView<Tape> byMaterial = all.withColoring(RegionColoring::Material);

  CHECK(byMaterial.firstHit(intoCore, T(0), T(5), options).m_primitive == 1U);
  CHECK(byMaterial.firstHit(intoJacket, T(0), T(5), options).m_primitive == 0U);

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

  // The value: the visible regions' subdomain function; a cut face's point names its region.
  Subdomains visible;

  (void)visible.addSubdomain({1, 2});

  for (const Vec3& p : {Vec3(T(-0.3), T(0.1), T(0.5)), Vec3(T(0.7), T(0), T(-0.5)), Vec3(T(2), T(2), T(2))}) {
    CHECK(all.signedDistance(p) == tape.signedDistance(p, visible, 0U));
  }

  // With section regions, the section's field is their subdomain's function, the background's included (what a code
  // computing in the air cuts), signed, zero on its boundary alone; the value stays the visible regions' (what is
  // solid for the render).
  Subdomains air;

  (void)air.addSubdomain({0});

  RegionSettings outside = all.getSettings();

  outside.m_sectionRegions = uint64_t(1);

  const RegionView<Tape> airView = all.withSettings(outside);

  for (const Vec3& p : {Vec3(T(-0.3), T(0.1), T(0.5)), Vec3(T(0.7), T(0), T(-0.5)), Vec3(T(2), T(2), T(2))}) {
    CHECK(airView.signedDistance(p) == tape.signedDistance(p, visible, 0U));
    CHECK(airView.contourValue(p) == tape.signedDistance(p, air, 0U));
    CHECK(airView.contourSample(p).m_secondary == (tape.signedDistance(p, air, 0U) > T(0)));
  }

  // With the whole domain shown (the air and the jacket; the core excluded), each point's field is its own label's
  // function, as a domain samples it; none in the core.
  Domain electrostatics;

  (void)electrostatics.m_subdomains.addSubdomain({0});
  (void)electrostatics.m_subdomains.addSubdomain({1});

  RegionSettings whole = all.getSettings();

  whole.m_sectionDomain    = true;
  whole.m_sectionLabels[0] = 0U;
  whole.m_sectionLabels[1] = 1U;
  whole.m_sectionLabels[2] = RegionSettings::ExcludedLabel;

  const RegionView<Tape> domainView = all.withSettings(whole);

  for (const Vec3& p : {Vec3(T(-0.3), T(0.1), T(0.5)),
                        Vec3(T(0.7), T(0), T(-0.5)),
                        Vec3(T(2), T(2), T(2)),
                        Vec3(T(-0.1), T(0), T(0))}) {
    T values[Domain::MaxLabels];

    electrostatics.signedDistances(tape, p, values);

    const uint32_t label = electrostatics.labelAt(tape, p);

    INFO("point " << p << ", label " << label);

    CHECK((label == Domain::Excluded) == (tape.region(p) == 2U));
    CHECK(domainView.contourValue(p) == values[(label == Domain::Excluded) ? 2U : label]);
    CHECK(domainView.contourSample(p).m_secondary == (label == Domain::Excluded));
  }

  // What a point is in, for a section plane's fill: its region's material, or its region when colored by region.
  {
    const ExprIndex<T> labels(cable<T>());

    const Vec3 inCore(T(-0.1), T(0), T(0));
    const Vec3 inAir(T(2), T(2), T(2));

    CHECK(all.sectionKey(inCore) == 2U);
    CHECK(all.sectionKey(inAir) == 0U);

    RegionSettings byMaterial = all.getSettings();

    byMaterial.m_coloring = RegionColoring::Material;

    CHECK(all.withSettings(byMaterial).sectionKey(inCore) == labels.findMaterialLabel("copper").value());
    CHECK(all.withSettings(byMaterial).sectionKey(inAir) == MaterialLabelId::none().value());
  }

  CHECK(all.closestLeaf(Vec3(T(-0.01), T(0.1), T(0.2))).m_leaf == 2U);

  // The piece a hit picks: the region seen inside, the one across outside (the core's cut face: core | air; the
  // core's wall seen from the jacket: core | jacket; the jacket's outer surface: jacket | air).
  const SurfaceRef<T> cutPiece  = all.pieceAt(coreFace.m_point, coreFace.m_normal);
  const SurfaceRef<T> wallPiece = all.pieceAt(inner.m_point, inner.m_normal);
  const SurfaceRef<T> skinPiece = all.pieceAt(outer.m_point, outer.m_normal);

  CHECK(cutPiece.m_ownerRegion == 2U);
  CHECK(cutPiece.m_neighborRegion == 0U);
  CHECK(wallPiece.m_ownerRegion == 2U);
  CHECK(wallPiece.m_neighborRegion == 1U);
  CHECK(skinPiece.m_ownerRegion == 1U);
  CHECK(skinPiece.m_neighborRegion == 0U);
  CHECK(wallPiece.m_surface != cutPiece.m_surface);
  CHECK(SurfaceSelector::piece(wallPiece).matches(wallPiece));
  CHECK(!SurfaceSelector::piece(wallPiece).matches(skinPiece));

  // With the core hidden, its wall seen from the jacket is the jacket's bore: the jacket is the region seen.
  const SurfaceRef<T> borePiece = all.withVisible(2, false).pieceAt(inner.m_point, inner.m_normal);

  CHECK(borePiece.m_ownerRegion == 1U);
  CHECK(borePiece.m_neighborRegion == 2U);
  CHECK(all.pieceAt(inner.m_point, Vec3::zeros()).m_surface == SurfaceRef<T>::None);

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
  Subdomains cfd;

  CHECK(cfd.addSubdomain({0}) == 0U);
  CHECK(cfd.role(outer) == SurfaceRole::Boundary);
  CHECK(cfd.role(inner) == SurfaceRole::None);

  // Electrostatics in the air and the jacket, the core a conductor: an interface and a boundary.
  Subdomains electrostatics;

  CHECK(electrostatics.addSubdomain({0}) == 0U);
  CHECK(electrostatics.addSubdomain(index.getRegionsOfMaterial(index.findMaterialLabel("PTFE"))) == 1U);
  CHECK(electrostatics.getNumSubdomains() == 2U);
  CHECK(electrostatics.subdomainOf(1) == 1U);
  CHECK(electrostatics.subdomainOf(2) == Subdomains::None);
  CHECK(electrostatics.role(outer) == SurfaceRole::Interface);
  CHECK(electrostatics.role(inner) == SurfaceRole::Boundary);

  // Heat in all three: two interfaces. The jacket and the core as one subdomain: their contact is internal.
  Subdomains heat;

  (void)heat.addSubdomain({0});
  (void)heat.addSubdomain({1});
  (void)heat.addSubdomain({2});

  CHECK(heat.role(outer) == SurfaceRole::Interface);
  CHECK(heat.role(inner) == SurfaceRole::Interface);

  Subdomains solid;

  (void)solid.addSubdomain(index.getRegionsOfRegionGroup(0));

  CHECK(solid.getNumRanges() == 1U);
  CHECK(solid.role(inner) == SurfaceRole::Internal);
  CHECK(solid.role(outer) == SurfaceRole::Boundary);

  // A subdomain's function is negative exactly where one of its regions is; the solid's has the whole object's sign.
  for (int i = -15; i <= 15; i++) {
    const Vec3 p(T(0.1) * T(i) + T(0.013), T(0.02), T(0));

    INFO("point " << p);

    CHECK((tape.signedDistance(p, solid, 0U) < T(0)) == (tape.signedDistance(p) < T(0)));
    CHECK((tape.signedDistance(p, electrostatics, 1U) < T(0)) == (tape.region(p) == 1U));
  }

  // The jacket and the core as one subdomain: on their contact the function is not zero (no sheet inside the subdomain);
  // across the outer sphere it is the distance, from both sides.
  for (const T x : {T(0.5), T(0.45), T(0.55)}) {
    INFO("x " << x);

    CHECK(tape.signedDistance(Vec3(x, T(0), T(0)), solid, 0U) < -T(0.2));
  }

  CHECK_THAT(tape.signedDistance(Vec3(T(0.95), T(0), T(0)), solid, 0U), withinAbsT<T>(-T(0.05), tightMargin<T>()));
  CHECK_THAT(tape.signedDistance(Vec3(T(1.05), T(0), T(0)), solid, 0U), withinAbsT<T>(T(0.05), tightMargin<T>()));

  // The subdomain's surface: from its region whose function is smallest.
  const SurfaceRef<T> jacket = tape.closestSurface(Vec3(T(0.5), T(0), T(0)), electrostatics, 1U);

  CHECK(jacket.m_ownerRegion == 1U);
  CHECK(jacket.m_neighborRegion == 2U);
  CHECK(electrostatics.role(jacket) == SurfaceRole::Boundary);

  REQUIRE(abortsWith([] { (void)Subdomains().addSubdomain({}); }, "Subdomains: a subdomain needs at least one region"));
  REQUIRE(abortsWith(
    [] {
      Subdomains subdomains;

      (void)subdomains.addSubdomain({1, 2});
      (void)subdomains.addSubdomain({2});
    },
    "Subdomains: region 2 is already in subdomain 0"));
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

  // Electrostatics: the air (0) and the jacket (1); the core, a conductor, excluded (label 2 of the values).
  Domain domain;

  (void)domain.m_subdomains.addSubdomain({0});
  (void)domain.m_subdomains.addSubdomain({1});

  REQUIRE(domain.getNumLabels() == 3U);

  // The whole domain as one subdomain: Excluded's function is its function's negative.
  Subdomains together;

  (void)together.addSubdomain({0, 1});

  const auto valueOf = [&](const Vec3& a_point, const uint32_t a_label) {
    T values[Domain::MaxLabels];

    domain.signedDistances(tape, a_point, values);

    return values[(a_label == Domain::Excluded) ? 2U : a_label];
  };

  // Everywhere: the values are signedDistance()'s, label by label; the label is the region's subdomain, and off the
  // surfaces its function alone is negative.
  size_t checked = 0;

  for (int i = 0; i < 25; i++) {
    for (int j = 0; j < 9; j++) {
      const Vec3 p(T(0.1) * T(i) - T(1.2) + T(0.013), T(0.007), T(0.1) * T(j) - T(0.4) + T(0.004));

      INFO("point " << p);

      T values[Domain::MaxLabels];

      domain.signedDistances(tape, p, values);

      const uint32_t label = domain.labelAt(tape, p);

      CHECK(values[0] == domain.signedDistance(tape, p, 0U));
      CHECK(values[1] == domain.signedDistance(tape, p, 1U));
      CHECK(values[2] == domain.signedDistance(tape, p, Domain::Excluded));
      CHECK(values[2] == -tape.signedDistance(p, together, 0U));
      CHECK(label == domain.m_subdomains.subdomainOf(tape.region(p)));

      if (std::abs(values[0]) > T(1e-3) && std::abs(values[1]) > T(1e-3) && std::abs(values[2]) > T(1e-3)) {
        const uint32_t negative = (values[0] < T(0)) ? 0U : (values[1] < T(0)) ? 1U : 2U;

        CHECK(int(values[0] < T(0)) + int(values[1] < T(0)) + int(values[2] < T(0)) == 1);
        CHECK(negative == ((label == Domain::Excluded) ? 2U : label));

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
    CHECK(valueOf(p, Domain::Excluded) > T(0.2));
  }

  for (const T r : {T(0.45), T(0.55)}) {
    const Vec3 p(-r, T(0), T(0));

    INFO("r " << r);

    CHECK(valueOf(p, 1U) == -valueOf(p, Domain::Excluded));
    CHECK((valueOf(p, 1U) < T(0)) == (r > T(0.5)));
    CHECK(valueOf(p, 0U) > T(0.2));
  }

  // Near a triple line (the core, the jacket and the cut's air meeting on the circle of radius 0.4 in the plane
  // z = 0.3): the labels still divide space, the core's side excluded, the jacket's side the jacket's, the cut the
  // air's.
  CHECK(domain.labelAt(tape, Vec3(T(0.36), T(0.01), T(0.32))) == Domain::Excluded);
  CHECK(domain.labelAt(tape, Vec3(T(0.44), T(0.01), T(0.32))) == 1U);
  CHECK(domain.labelAt(tape, Vec3(T(0.4), T(0.01), T(0.28))) == 0U);
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

    Subdomains jacketOnly;
    Subdomains coreOnly;
    Subdomains both;

    (void)jacketOnly.addSubdomain({1});
    (void)coreOnly.addSubdomain({2});
    (void)both.addSubdomain({1, 2});

    const SurfaceHitT<T> jacket = tape.firstSurface(ray, T(0), T(5), jacketOnly, 0U);
    const SurfaceHitT<T> core   = tape.firstSurface(ray, T(0), T(5), coreOnly, 0U);
    const SurfaceHitT<T> solid  = tape.firstSurface(ray, T(0), T(5), both, 0U);

    CHECK_THAT(jacket.m_hit.m_t, withinAbsT<T>(T(2) - std::sqrt(T(0.84)), looseMargin<T>()));
    CHECK_THAT(core.m_hit.m_t, withinAbsT<T>(T(2) - std::sqrt(T(0.09)), looseMargin<T>()));
    CHECK(core.m_surface.m_neighborRegion == 2U);
    CHECK(solid.m_surface.m_neighborRegion == 1U);

    // Leaving the solid: the contact between the jacket and the core is passed through.
    const SurfaceHitT<T> leave = tape.firstSurface(ray, solid.m_hit.m_t, T(5), both, 0U);

    CHECK_THAT(leave.m_hit.m_t, withinAbsT<T>(T(2) + std::sqrt(T(0.84)), looseMargin<T>()));
    CHECK(leave.m_surface.m_neighborRegion == 0U);
  }
}

TEMPLATE_TEST_CASE("Tape: the region queries on a device match the host, and a domain taken whole by a kernel",
                   "[Overlay][Tape][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Tape = Tape<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  Pool pool(hostMemoryResource());

  // The cable cut by a slab (x > 0, |z| < 0.3).
  const Tape tape(pool, Difference(cable<T>(), BoxSDF<T>(Vec3(T(0), T(-2), T(-0.3)), Vec3(T(2), T(2), T(0.3)))));

  pool.freeze();

  const Pool device = Pool::mirror(pool, deviceTestResource());

  std::vector<Vec3> points;

  for (int i = 0; i < 24; i++) {
    for (int j = 0; j < 6; j++) {
      points.emplace_back(T(0.1) * T(i) - T(1.2) + T(0.013), T(0.007), T(0.15) * T(j) - T(0.4));
    }
  }

  // The electrostatic domain: the air and the jacket, two subdomains; every conductor surface dirichlet, the
  // jacket's outside a jump.
  Domain domain;

  (void)domain.m_subdomains.addSubdomain({0});
  (void)domain.m_subdomains.addSubdomain({1});

  const uint32_t dirichlet = domain.m_scheme.addSurfaceGroup();
  const uint32_t jump      = domain.m_scheme.addSurfaceGroup();

  domain.m_scheme.add(dirichlet, SurfaceSelector::region(2));
  domain.m_scheme.add(jump, SurfaceSelector::interface(0, 1));

  const auto onDevice =
    evaluateOnDevice<RegionAnswer<T>>(RegionQuery<T, Tape>{tape.rebasedView(device), domain}, points);
  const auto onHost = evaluateOnHost<RegionAnswer<T>>(RegionQuery<T, Tape>{tape, domain}, points);

  size_t differ = 0;

  for (size_t i = 0; i < points.size(); i++) {
    INFO("point " << points[i]);

    const RegionAnswer<T>& d = onDevice[i];
    const RegionAnswer<T>& h = onHost[i];

    // Where two choices are equally right (a point on a surface), the device's rounding may pick the other.
    differ += (d.m_region != h.m_region || d.m_minus != h.m_minus || d.m_plus != h.m_plus ||
               d.m_surface != h.m_surface || d.m_group != h.m_group || d.m_from != h.m_from || d.m_to != h.m_to ||
               d.m_roles[0] != h.m_roles[0] || d.m_roles[1] != h.m_roles[1] || d.m_conditions[0] != h.m_conditions[0] ||
               d.m_conditions[1] != h.m_conditions[1] || d.m_label != h.m_label)
                ? 1U
                : 0U;

    for (uint32_t k = 0; k < 3; k++) {
      CHECK_THAT(d.m_values[k], withinAbsT<T>(h.m_values[k], looseMargin<T>()));
    }

    for (uint32_t k = 0; k < 2; k++) {
      CHECK_THAT(d.m_subdomainValues[k], withinAbsT<T>(h.m_subdomainValues[k], looseMargin<T>()));
    }

    for (uint32_t l = 0; l < 3; l++) {
      CHECK_THAT(d.m_labelValues[l], withinAbsT<T>(h.m_labelValues[l], looseMargin<T>()));
    }

    CHECK_THAT(d.m_t, withinAbsT<T>(h.m_t, looseMargin<T>()));
  }

  CHECK(differ <= 2U);

  // The jacket's subdomain meets the air (an interface: two subdomains) and the core (a boundary: the copper is in no
  // subdomain), with the jump and the dirichlet conditions.
  bool interface = false;
  bool boundary  = false;

  for (const RegionAnswer<T>& h : onHost) {
    interface = interface || (h.m_roles[1] == SurfaceRole::Interface && h.m_conditions[1] == jump);
    boundary  = boundary || (h.m_roles[1] == SurfaceRole::Boundary && h.m_conditions[1] == dirichlet);
  }

  CHECK(interface);
  CHECK(boundary);
}

TEMPLATE_TEST_CASE("ExprIndex: the pairs of regions each surface may separate, the interfaces",
                   "[Overlay][Scheme]",
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

TEMPLATE_TEST_CASE("Scheme: a solver's groups of surfaces, from pieces, regions, interfaces, labels and materials",
                   "[Overlay][Scheme]",
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
  Scheme electrostatics;

  const uint32_t dirichlet = electrostatics.addSurfaceGroup();
  const uint32_t jump      = electrostatics.addSurfaceGroup();

  electrostatics.add(dirichlet, SurfaceSelector::label(index.findSurfaceLabel("conductor")));
  electrostatics.add(jump, SurfaceSelector::interface(1, 0));

  CHECK(electrostatics.surfaceGroupOf(inner) == dirichlet);
  CHECK(electrostatics.surfaceGroupOf(outer) == jump);

  // CFD outside: every surface facing the outside (no-slip); the inner one is no surface to it.
  Scheme cfd;

  const uint32_t noSlip = cfd.addSurfaceGroup();

  cfd.add(noSlip, SurfaceSelector::region(0));

  CHECK(cfd.surfaceGroupOf(outer) == noSlip);
  CHECK(cfd.surfaceGroupOf(inner) == Scheme::None);

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
  CHECK(cfd.surfaceGroupOf(outer) == Scheme::None);

  REQUIRE(abortsWith(
    [] {
      Scheme scheme;

      for (uint32_t g = 0; g <= Scheme::MaxSurfaceGroups; g++) {
        (void)scheme.addSurfaceGroup();
      }
    },
    "Scheme: a scheme holds at most 8 surface groups"));
  REQUIRE(abortsWith([] { Scheme().add(0, SurfaceSelector()); }, "Scheme: surface group 0 of 0"));
}

TEST_CASE("Scheme: removing a surface group removes its selectors and renumbers the surface groups after it",
          "[Scheme]")
{
  Scheme scheme;

  const uint32_t first  = scheme.addSurfaceGroup();
  const uint32_t second = scheme.addSurfaceGroup();
  const uint32_t third  = scheme.addSurfaceGroup();

  scheme.add(first, SurfaceSelector::region(1));
  scheme.add(second, SurfaceSelector::region(2));
  scheme.add(third, SurfaceSelector::interface(0, 1));

  scheme.removeSurfaceGroup(second);

  REQUIRE(scheme.getNumSurfaceGroups() == 2U);
  REQUIRE(scheme.getNumSelectors() == 2U);
  CHECK(scheme.getSurfaceGroupOfSelector(0) == 0U);
  CHECK(scheme.getSelector(0).m_regionA == 1U);
  CHECK(scheme.getSurfaceGroupOfSelector(1) == 1U);
  CHECK(scheme.getSelector(1).m_regionB == 1U);

  RegionNames names;

  names.setSurfaceGroup(0, "dirichlet");
  names.setSurfaceGroup(1, "neumann");
  names.setSurfaceGroup(2, "jump");
  names.removeSurfaceGroup(1);

  CHECK(names.surfaceGroup(1) == "1 jump");

  // The remaining surfaces are tried last, even when added first.
  Scheme ordered;

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
  Domain electrostatics;

  (void)electrostatics.m_subdomains.addSubdomain({0});
  (void)electrostatics.m_subdomains.addSubdomain({1});

  Domain flow;

  (void)flow.m_subdomains.addSubdomain({0});

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

  // How a scheme covers them: the core's surfaces wholly; one face of the box against the jacket partly.
  const auto find = [&possible](const uint32_t a_surface, const uint32_t a_inside, const uint32_t a_outside) {
    return *std::find_if(possible.begin(), possible.end(), [&](const PossiblePiece& a_possible) {
      return a_possible.m_surface == a_surface && a_possible.m_ownerRegion == a_inside &&
             a_possible.m_neighborRegion == a_outside;
    });
  };

  Scheme scheme;

  const uint32_t core = scheme.addSurfaceGroup();
  const uint32_t face = scheme.addSurfaceGroup();

  scheme.add(core, SurfaceSelector::region(2));

  SurfaceRef<T> boxOnJacket;

  boxOnJacket.m_surface        = 2;
  boxOnJacket.m_part           = 0;
  boxOnJacket.m_ownerRegion    = 1;
  boxOnJacket.m_neighborRegion = 0;

  scheme.add(face, SurfaceSelector::piece(boxOnJacket).withPart(0));

  CHECK(coverageOf(find(1, 2, 1), inventory, scheme).m_coverage == Coverage::Full);
  CHECK(coverageOf(find(1, 2, 1), inventory, scheme).m_surfaceGroup == core);
  CHECK(coverageOf(find(2, 1, 0), inventory, scheme).m_coverage == Coverage::Partly);
  CHECK(coverageOf(find(0, 1, 0), inventory, scheme).m_coverage == Coverage::None);

  // The remaining surfaces cover the rest, whatever the order they were added in; the core's stay the core's.
  Scheme rest = scheme;

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

  // Colors by name: a material's when colored by material, a surface group's when colored by surface group.
  names.setColor("PTFE", Vec3T<double>(0.5, 0.5, 0.5));
  names.setColor("jump", Vec3T<double>(0.0, 0.0, 1.0));

  Vec3T<double> color;

  CHECK(names.colorOf(RegionColoring::Material, 0, color));
  CHECK(color[0] == 0.5);
  CHECK(!names.colorOf(RegionColoring::Region, 1, color));
  CHECK(names.colorOf(RegionColoring::SurfaceGroup, 1, color));
  CHECK(color[2] == 1.0);
  CHECK(!names.colorOf(RegionColoring::SurfaceGroup, 0, color));
  CHECK(names.materialOf(1) == 0U);
  CHECK(names.materialOf(2) == RegionNames::None);
}

TEMPLATE_TEST_CASE("Render: a surface between two regions near the section plane is no cut face",
                   "[Overlay][RegionView][Render]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Tape = Tape<T>;

  // A block (x from 1 to 2) in a big box of air; the plane across y keeps y < 0. The eye, inside the air just below
  // the plane, looks along +x at the block's face x = 1: a surface between two regions, inside the whole solid, close
  // to the plane but not on it. It is shaded as a surface, as with no plane, not as the plane's cut face.
  const Expr<T> object = Overlay(BoxSDF<T>(-T(4) * Vec3::ones(), T(4) * Vec3::ones()),
                                 BoxSDF<T>(Vec3(T(1), T(-1), T(-1)), Vec3(T(2), T(1), T(1))));

  Pool pool(hostMemoryResource());

  const Tape             tape(pool, object);
  const RegionView<Tape> view(tape);

  constexpr uint32_t size = 16;

  Render::Settings<T> settings = EBGeometryTestRender::plain<T>(size);

  settings.m_colorBy = Render::ColorBy::Normal;

  Render::Camera<T> camera;

  camera.m_target   = Vec3(T(1), T(-0.2), T(0));
  camera.m_distance = T(2);
  camera.m_pitch    = T(0);

  // The yaw that puts the eye on the -x side of the target.
  for (const T yaw : {T(0), T(90), T(180), T(270)}) {
    camera.m_yaw = yaw;

    if (camera.eye()[0] < T(0)) {
      break;
    }
  }

  REQUIRE(camera.eye()[0] < T(0));
  REQUIRE(tape.region(camera.eye()) == 1U);

  Render::Image uncut;
  Render::Image cut;

  (void)Render::Renderer<T>(settings).render(view, camera, uncut);

  settings.m_section       = true;
  settings.m_sectionNormal = Vec3(T(0), T(1), T(0));

  (void)Render::Renderer<T>(settings).render(view, camera, cut);

  const EBGeometryTestRender::Channels a = EBGeometryTestRender::channels(uncut.at(size / 2, size / 2));
  const EBGeometryTestRender::Channels b = EBGeometryTestRender::channels(cut.at(size / 2, size / 2));

  // Colored by normal: the face (normal -x) has no red, the cut face (normal +y) would be bright green and red. The
  // plane nearby darkens the face a little.
  CHECK(std::abs(a.r - b.r) <= 6);
  CHECK(std::abs(a.g - b.g) <= 16);
  CHECK(std::abs(a.b - b.b) <= 16);
}

TEMPLATE_TEST_CASE("Render: a view of regions shows what the render settings' region settings say",
                   "[Overlay][RegionView][Render]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Tape = Tape<T>;

  Pool pool(hostMemoryResource());

  const Tape             tape(pool, cable<T>());
  const RegionView<Tape> view(tape);

  Render::Settings<T> settings;

  settings.m_width  = 64;
  settings.m_height = 64;

  const Render::Camera<T> camera = Render::Camera<T>::fit(view.computeBoundingVolume());

  // Through the middle of the image: the jacket first; with only the core shown, or the jacket seen through, the core.
  const T middle = T(32);

  CHECK(Render::Renderer<T>(settings).pick(view, camera, middle, middle).m_primitive == 1U);

  settings.m_regions.showOnly(2);

  CHECK(Render::Renderer<T>(settings).pick(view, camera, middle, middle).m_primitive == 2U);

  settings.m_regions.m_hidden = 0U;
  settings.m_regions.setSeenThrough(1, true);

  CHECK(Render::Renderer<T>(settings).pick(view, camera, middle, middle).m_primitive == 2U);

  // Colored by group: the core's surface in the first group, the jacket's outside in none.
  settings.m_regions.clearSeenThrough();
  settings.m_regions.m_scheme.add(settings.m_regions.m_scheme.addSurfaceGroup(), SurfaceSelector::region(2));
  settings.m_regions.m_coloring = RegionColoring::SurfaceGroup;

  CHECK(Render::Renderer<T>(settings).pick(view, camera, middle, middle).m_primitive == Scheme::None);

  settings.m_regions.showOnly(2);

  CHECK(Render::Renderer<T>(settings).pick(view, camera, middle, middle).m_primitive == 0U);

  // The highlighted surface group's surfaces are keyed Highlighted whatever the coloring; others keep their keys.
  settings.m_regions.m_highlightGroup = 0U;
  settings.m_regions.m_coloring       = RegionColoring::Region;

  CHECK(Render::Renderer<T>(settings).pick(view, camera, middle, middle).m_primitive == RegionSettings::Highlighted);

  settings.m_regions.m_hidden = 0U;

  CHECK(Render::Renderer<T>(settings).pick(view, camera, middle, middle).m_primitive == 1U);

  // A hidden surface group's surfaces are passed through: the core's (group 0) alone shown, nothing is hit.
  settings.m_regions.m_highlightGroup = RegionSettings::None;
  settings.m_regions.setSurfaceGroupHidden(0, true);

  CHECK(settings.m_regions.isSurfaceGroupHidden(0));
  CHECK(!settings.m_regions.isSurfaceGroupHidden(RegionSettings::None));
  CHECK(Render::Renderer<T>(settings).pick(view, camera, middle, middle).m_primitive == 1U);

  settings.m_regions.showOnly(2);

  CHECK(!Render::Renderer<T>(settings).pick(view, camera, middle, middle).hit());

  settings.m_regions.setSurfaceGroupHidden(0, false);

  CHECK(Render::Renderer<T>(settings).pick(view, camera, middle, middle).m_primitive == 2U);

  // Highlighted regions (a subdomain's): every surface with one of them on either side keyed Highlighted, others not.
  // The jacket's outside (jacket | air) is lit by highlighting the jacket or the background; the core's surface alone
  // (the core shown alone, inside the jacket: core | jacket) by the jacket too, not by the background.
  settings.m_regions.m_hidden           = 0U;
  settings.m_regions.m_highlightRegions = uint64_t(1) << 1U;

  CHECK(settings.m_regions.isRegionHighlighted(1));
  CHECK(!settings.m_regions.isRegionHighlighted(2));
  CHECK(Render::Renderer<T>(settings).pick(view, camera, middle, middle).m_primitive == RegionSettings::Highlighted);

  settings.m_regions.showOnly(2);

  CHECK(Render::Renderer<T>(settings).pick(view, camera, middle, middle).m_primitive == RegionSettings::Highlighted);

  settings.m_regions.m_highlightRegions = uint64_t(1);

  CHECK(Render::Renderer<T>(settings).pick(view, camera, middle, middle).m_primitive != RegionSettings::Highlighted);

  settings.m_regions.m_hidden = 0U;

  CHECK(Render::Renderer<T>(settings).pick(view, camera, middle, middle).m_primitive == RegionSettings::Highlighted);

  // Selected surfaces (a viewer's picked parts): those an entry of the selection matches first are keyed Selected and
  // that entry, so that each is told apart. The jacket's outside (jacket | air) is hit through the middle; selecting
  // the core's surface alone leaves it its key, selecting the jacket's outside too keys it by its entry (1); the
  // core's surface shown alone, by its own (0).
  settings.m_regions.m_highlightRegions = 0U;
  settings.m_regions.m_selected[0]      = SurfaceSelector::interface(1, 2);
  settings.m_regions.m_numSelected      = 1U;

  CHECK(Render::Renderer<T>(settings).pick(view, camera, middle, middle).m_primitive == 1U);

  settings.m_regions.m_selected[1] = SurfaceSelector::interface(0, 1);
  settings.m_regions.m_numSelected = 2U;

  CHECK(Render::Renderer<T>(settings).pick(view, camera, middle, middle).m_primitive == RegionSettings::Selected + 1U);

  settings.m_regions.showOnly(2);

  CHECK(Render::Renderer<T>(settings).pick(view, camera, middle, middle).m_primitive == RegionSettings::Selected);

  // Only the selected surfaces shown: the jacket passed through, the core's surface keyed by its entry and the side
  // seen, from the core (entering it) or, with the core hidden, from the jacket (its bore).
  settings.m_regions.m_hidden       = 0U;
  settings.m_regions.m_onlySelected = true;
  settings.m_regions.m_numSelected  = 1U;

  CHECK(Render::Renderer<T>(settings).pick(view, camera, middle, middle).m_primitive == RegionSettings::Selected);

  settings.m_regions.setVisible(2, false);

  CHECK(Render::Renderer<T>(settings).pick(view, camera, middle, middle).m_primitive ==
        RegionSettings::Selected + RegionSettings::MaxSelected);

  settings.m_regions.m_onlySelected = false;
  settings.m_regions.m_numSelected  = 2U;

  settings.m_regions.showOnly(2);

  // Highlighting the selection's keys, a range of them, tints every selected surface and nothing else.
  settings.m_highlightPrimitive = RegionSettings::Selected;
  settings.m_highlightCount     = 2U;

  CHECK(settings.isHighlighted(RegionSettings::Selected));
  CHECK(settings.isHighlighted(RegionSettings::Selected + 1U));
  CHECK(!settings.isHighlighted(RegionSettings::Selected + 2U));
  CHECK(!settings.isHighlighted(2U));

  settings.m_regions.m_numSelected = 0U;

  CHECK(Render::Renderer<T>(settings).pick(view, camera, middle, middle).m_primitive == 2U);
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
