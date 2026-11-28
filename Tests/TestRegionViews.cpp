// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// Test suite for viewing regions: RegionView's surfaces along rays and its whole object, the renderer's region
// settings (cut faces, surfaces seen through, translucent layers) and regionNeighborhood().

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestRenderHelpers.hpp"
#include "TestUtilities.hpp"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;

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
  DomainSetup visible;

  (void)visible.addSubdomain({1, 2});

  const Subdomains visibleBuilt = visible.buildSubdomains(pool, tape.getNumRegions());

  for (const Vec3& p : {Vec3(T(-0.3), T(0.1), T(0.5)), Vec3(T(0.7), T(0), T(-0.5)), Vec3(T(2), T(2), T(2))}) {
    CHECK(all.signedDistance(p) == tape.signedDistance(p, visibleBuilt, 0U));
  }

  // With section regions, the section's field is their subdomain's function, the background's included (what a code
  // computing in the air cuts), signed, zero on its boundary alone; the value stays the visible regions' (what is
  // solid for the render).
  DomainSetup air;

  (void)air.addSubdomain({0});

  const Subdomains airBuilt = air.buildSubdomains(pool, tape.getNumRegions());

  RegionSettings outside = all.getSettings();

  outside.m_sectionRegions = uint64_t(1);

  const RegionView<Tape> airView = all.withSettings(outside);

  for (const Vec3& p : {Vec3(T(-0.3), T(0.1), T(0.5)), Vec3(T(0.7), T(0), T(-0.5)), Vec3(T(2), T(2), T(2))}) {
    CHECK(airView.signedDistance(p) == tape.signedDistance(p, visibleBuilt, 0U));
    CHECK(airView.contourValue(p) == tape.signedDistance(p, airBuilt, 0U));
    CHECK(airView.contourSample(p).m_secondary == (tape.signedDistance(p, airBuilt, 0U) > T(0)));
  }

  // With the whole domain shown (the air and the jacket; the core excluded), each point's field is its own subdomain's
  // function, as a domain samples it; none in the core.
  DomainSetup electrostaticsSetup;

  (void)electrostaticsSetup.addSubdomain({0});
  (void)electrostaticsSetup.addSubdomain({1});

  const Domain electrostatics(pool, tape, electrostaticsSetup);

  RegionSettings whole = all.getSettings();

  whole.m_sectionDomain        = true;
  whole.m_sectionSubdomains[0] = 0U;
  whole.m_sectionSubdomains[1] = 1U;
  whole.m_sectionSubdomains[2] = RegionSettings::ExcludedSubdomain;

  const RegionView<Tape> domainView = all.withSettings(whole);

  for (const Vec3& p : {Vec3(T(-0.3), T(0.1), T(0.5)),
                        Vec3(T(0.7), T(0), T(-0.5)),
                        Vec3(T(2), T(2), T(2)),
                        Vec3(T(-0.1), T(0), T(0))}) {
    T values[3];

    electrostatics.signedDistances(p, values);

    const uint32_t subdomain = electrostatics.subdomainAt(p);

    INFO("point " << p << ", subdomain " << subdomain);

    CHECK((subdomain == Domain<T>::Excluded) == (tape.region(p) == 2U));
    CHECK(domainView.contourValue(p) == values[(subdomain == Domain<T>::Excluded) ? 2U : subdomain]);
    CHECK(domainView.contourSample(p).m_secondary == (subdomain == Domain<T>::Excluded));
  }

  // What a point is in, for a section plane's fill: its region's material, whatever the surfaces are colored by; none
  // outside.
  {
    const ExprIndex<T> labels(cable<T>());

    const Vec3 inCore(T(-0.1), T(0), T(0));
    const Vec3 inAir(T(2), T(2), T(2));

    for (const RegionColoring coloring : {RegionColoring::Region, RegionColoring::Material, RegionColoring::Surface}) {
      RegionSettings colored = all.getSettings();

      colored.m_coloring = coloring;

      CHECK(all.withSettings(colored).sectionKey(inCore) == labels.findMaterialLabel("copper").value());
      CHECK(all.withSettings(colored).sectionKey(inAir) == MaterialLabelId::none().value());
    }
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

TEMPLATE_TEST_CASE("Render: a surface seen through is drawn translucent over what is behind it, and not picked",
                   "[Overlay][RegionView][Render]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Tape = Tape<T>;

  Pool pool(hostMemoryResource());

  const Tape             tape(pool, cable<T>());
  const RegionView<Tape> view(tape);

  constexpr uint32_t size = 16;

  Render::Settings<T> settings = EBGeometryTestRender::plain<T>(size);

  settings.m_colorBy = Render::ColorBy::Primitive;

  const Render::Camera<T> camera = Render::Camera<T>::fit(view.computeBoundingVolume());

  // Through the middle of the image: the jacket, opaque; with it seen through and drawn fully transparent, the core;
  // translucent, a blend of the two; fully opaque, the jacket again.
  const auto middle = [&](const bool a_through, const T a_opacity) {
    Render::Settings<T> s = settings;

    s.m_regions.setSeenThrough(1, a_through);
    s.m_seenThroughOpacity = a_opacity;

    Render::Image image;

    (void)Render::Renderer<T>(s).render(view, camera, image);

    return EBGeometryTestRender::channels(image.at(size / 2, size / 2));
  };

  const EBGeometryTestRender::Channels jacket = middle(false, T(0.3));
  const EBGeometryTestRender::Channels core   = middle(true, T(0));
  const EBGeometryTestRender::Channels veiled = middle(true, T(0.3));
  const EBGeometryTestRender::Channels opaque = middle(true, T(1));

  REQUIRE((jacket.r != core.r || jacket.g != core.g || jacket.b != core.b));

  const auto between = [](const int a_value, const int a_one, const int a_other) {
    return a_value >= Math::min(a_one, a_other) - 1 && a_value <= Math::max(a_one, a_other) + 1;
  };

  CHECK(between(veiled.r, jacket.r, core.r));
  CHECK(between(veiled.g, jacket.g, core.g));
  CHECK(between(veiled.b, jacket.b, core.b));
  CHECK((veiled.r != core.r || veiled.g != core.g || veiled.b != core.b));
  CHECK((veiled.r != jacket.r || veiled.g != jacket.g || veiled.b != jacket.b));

  CHECK(std::abs(opaque.r - jacket.r) <= 1);
  CHECK(std::abs(opaque.g - jacket.g) <= 1);
  CHECK(std::abs(opaque.b - jacket.b) <= 1);

  // However it is drawn, a pick passes through it to the core.
  settings.m_regions.setSeenThrough(1, true);

  CHECK(Render::Renderer<T>(settings).pick(view, camera, T(size / 2), T(size / 2)).m_primitive == 2U);

  // The jacket seen through as a region, whatever the coloring: the same, colored by material too; Shift + T's
  // clearSeenThrough() makes it opaque again.
  settings.m_regions.clearSeenThrough();
  settings.m_regions.setRegionSeenThrough(1, true);

  CHECK(settings.m_regions.isRegionSeenThrough(1));
  CHECK(!settings.m_regions.isRegionSeenThrough(2));
  CHECK(Render::Renderer<T>(settings).pick(view, camera, T(size / 2), T(size / 2)).m_primitive == 2U);

  settings.m_regions.m_coloring = RegionColoring::Material;

  const RayT<T> into{camera.eye(), (Vec3T<T>::zeros() - camera.eye()) / (Vec3T<T>::zeros() - camera.eye()).length()};
  const RegionView<Tape> regionThrough = view.withSettings(settings.m_regions);

  const HitT<T> veil = regionThrough.firstSeenThrough(into, T(0), T(100), RayOptions<T>());
  const HitT<T> seen = regionThrough.firstHit(into, T(0), T(100), RayOptions<T>());

  REQUIRE(veil.hit());
  REQUIRE(seen.hit());
  CHECK(veil.m_t < seen.m_t);
  CHECK_THAT(veil.m_t, withinAbsT<T>((camera.eye()).length() - T(1), looseMargin<T>()));
  CHECK_THAT(seen.m_t, withinAbsT<T>((camera.eye()).length() - T(0.5), looseMargin<T>()));

  // What is highlighted or picked stays solid: the jacket seen through but highlighted, or with its outside picked, is
  // hit, keyed so, and nothing in front of it is seen through.
  {
    RegionSettings lit = settings.m_regions;

    lit.m_highlightRegions = uint64_t(1) << 1;

    const HitT<T> highlighted = view.withSettings(lit).firstHit(into, T(0), T(100), RayOptions<T>());

    CHECK(highlighted.m_primitive == RegionSettings::Highlighted);
    CHECK_THAT(highlighted.m_t, withinAbsT<T>((camera.eye()).length() - T(1), looseMargin<T>()));
    CHECK(!view.withSettings(lit).firstSeenThrough(into, T(0), T(100), RayOptions<T>()).hit());

    RegionSettings chosen = settings.m_regions;

    chosen.m_numSelected = 1U;
    chosen.m_selected[0] = SurfaceSelector::interface(1, 0);

    const HitT<T> picked = view.withSettings(chosen).firstHit(into, T(0), T(100), RayOptions<T>());

    CHECK(picked.m_primitive == RegionSettings::Selected);
    CHECK_THAT(picked.m_t, withinAbsT<T>((camera.eye()).length() - T(1), looseMargin<T>()));
  }

  // A surface group seen through: the jacket's outside, in group 0. A pick passes through it to the core, and it is
  // the layer in front.
  settings.m_regions.clearSeenThrough();
  settings.m_regions.m_coloring = RegionColoring::Region;
  DomainSetup groups;

  groups.add(groups.addSurfaceGroup(), SurfaceSelector::interface(1, 0));

  settings.m_regions.m_surfaceGroups = groups.buildSurfaceGroups(pool);
  settings.m_regions.setSurfaceGroupSeenThrough(0, true);

  CHECK(settings.m_regions.isSurfaceGroupSeenThrough(0));
  CHECK(!settings.m_regions.isSurfaceGroupSeenThrough(1));
  CHECK(Render::Renderer<T>(settings).pick(view, camera, T(size / 2), T(size / 2)).m_primitive == 2U);
  CHECK_THAT(view.withSettings(settings.m_regions).firstSeenThrough(into, T(0), T(100), RayOptions<T>()).m_t,
             withinAbsT<T>((camera.eye()).length() - T(1), looseMargin<T>()));

  settings.m_regions.clearSeenThrough();

  CHECK(!settings.m_regions.isSurfaceGroupSeenThrough(0));
  CHECK(!settings.m_regions.isRegionSeenThrough(1));
  CHECK(!view.withSettings(settings.m_regions).firstSeenThrough(into, T(0), T(100), RayOptions<T>()).hit());

  // Several layers: three nested spheres, the two outer ones seen through. Both are drawn over the inner one; with the
  // middle one hidden instead, only the outer one is, and the pixel differs.
  const Expr<T>          nested = Overlay(SphereSDF<T>(Vec3T<T>::zeros(), T(1.5)),
                                 SphereSDF<T>(Vec3T<T>::zeros(), T(1)),
                                 SphereSDF<T>(Vec3T<T>::zeros(), T(0.5)));
  const Tape             layered(pool, nested);
  const RegionView<Tape> layers(layered);

  const auto center = [&](const RegionSettings& a_regions) {
    Render::Settings<T> s = EBGeometryTestRender::plain<T>(size);

    s.m_colorBy = Render::ColorBy::Primitive;
    s.m_regions = a_regions;

    Render::Image image;

    (void)Render::Renderer<T>(s).render(layers, Render::Camera<T>::fit(layers.computeBoundingVolume()), image);

    return EBGeometryTestRender::channels(image.at(size / 2, size / 2));
  };

  RegionSettings two;

  two.setRegionSeenThrough(1, true);
  two.setRegionSeenThrough(2, true);

  RegionSettings one = two;

  one.setVisible(2, false);

  const EBGeometryTestRender::Channels both  = center(two);
  const EBGeometryTestRender::Channels outer = center(one);

  CHECK((both.r != outer.r || both.g != outer.g || both.b != outer.b));
}

TEMPLATE_TEST_CASE("Render: translucent layers are found on the kept side of the cut, and thin ones are all drawn",
                   "[Overlay][RegionView][Render]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Tape = Tape<T>;

  Pool pool(hostMemoryResource());

  constexpr uint32_t size = 17;

  const auto middle =
    [&](const RegionView<Tape>& a_view, const Render::Settings<T>& a_settings, const Render::Camera<T>& a_camera) {
      Render::Image image;

      (void)Render::Renderer<T>(a_settings).render(a_view, a_camera, image);

      return image.at(size / 2, size / 2);
    };

  // An opaque box that the cut removes, on the ray's line in front of a translucent sphere: it hides nothing. The pixel
  // is the same as with the box off the line, and differs from the sphere drawn opaque or left out.
  {
    const Tape onLine(pool,
                      Overlay(BoxSDF<T>(Vec3(T(-3), T(-0.5), T(-0.5)), Vec3(T(-2), T(0.5), T(0.5))),
                              SphereSDF<T>(Vec3::zeros(), T(1)),
                              SphereSDF<T>(Vec3::zeros(), T(0.5))));
    const Tape offLine(pool,
                       Overlay(BoxSDF<T>(Vec3(T(-3), T(2.5), T(-0.5)), Vec3(T(-2), T(3.5), T(0.5))),
                               SphereSDF<T>(Vec3::zeros(), T(1)),
                               SphereSDF<T>(Vec3::zeros(), T(0.5))));

    Render::Settings<T> settings = EBGeometryTestRender::plain<T>(size);

    settings.m_colorBy       = Render::ColorBy::Primitive;
    settings.m_section       = true;
    settings.m_sectionPoint  = Vec3(T(-1.5), T(0), T(0));
    settings.m_sectionNormal = Vec3(T(-1), T(0), T(0));
    settings.m_sectionFill   = T(0);
    settings.m_contourWidth  = T(0);
    settings.m_sheetOpacity  = T(0);
    settings.m_shadows       = false;

    settings.m_regions.setRegionSeenThrough(2, true);

    Render::Camera<T> camera;

    camera.m_distance = T(6);

    for (const T yaw : {T(0), T(90), T(180), T(270)}) {
      camera.m_yaw = yaw;

      if (camera.eye()[0] < T(-4)) {
        break;
      }
    }

    REQUIRE(camera.eye()[0] < T(-4));

    const uint32_t cutAway = middle(RegionView<Tape>(onLine), settings, camera);

    CHECK(cutAway == middle(RegionView<Tape>(offLine), settings, camera));

    settings.m_seenThroughOpacity = T(0);

    CHECK(cutAway != middle(RegionView<Tape>(onLine), settings, camera));
  }

  // Two translucent shells a thousandth apart, over a core: both drawn, as when they are a twentieth apart (the pixel
  // within a few levels); with one of them skipped, it was some 17 levels off.
  {
    Render::Settings<T> settings = EBGeometryTestRender::plain<T>(size);

    settings.m_colorBy   = Render::ColorBy::Primitive;
    settings.m_shadows   = false;
    settings.m_occlusion = false;

    settings.m_regions.setRegionSeenThrough(1, true);
    settings.m_regions.setRegionSeenThrough(2, true);

    const auto shells = [&](const T a_gap) {
      const Tape tape(pool,
                      Overlay(SphereSDF<T>(Vec3::zeros(), T(1)),
                              SphereSDF<T>(Vec3::zeros(), T(1) - a_gap),
                              SphereSDF<T>(Vec3::zeros(), T(0.5))));

      const RegionView<Tape> view(tape);

      return EBGeometryTestRender::channels(
        middle(view, settings, Render::Camera<T>::fit(SphereSDF<T>(Vec3::zeros(), T(1)).computeBoundingVolume())));
    };

    const EBGeometryTestRender::Channels thin = shells(T(0.001));
    const EBGeometryTestRender::Channels wide = shells(T(0.05));

    CHECK(std::abs(thin.r - wide.r) <= 4);
    CHECK(std::abs(thin.g - wide.g) <= 4);
    CHECK(std::abs(thin.b - wide.b) <= 4);
  }
}

TEST_CASE("RegionSettings: another coloring clears the keys seen through, not the regions and surface groups",
          "[RegionView]")
{
  RegionSettings regions;

  regions.setSeenThrough(5U, true);
  regions.setRegionSeenThrough(2U, true);
  regions.setSurfaceGroupSeenThrough(1U, true);
  regions.m_only = 3U;

  regions.setColoring(RegionColoring::Material);

  // A key meant another thing under the old coloring; a region or a surface group means the same under any.
  CHECK(!regions.isSeenThrough(5U));
  CHECK(regions.m_only == RegionSettings::None);
  CHECK(regions.isRegionSeenThrough(2U));
  CHECK(regions.isSurfaceGroupSeenThrough(1U));

  // Making every surface opaque clears all three.
  regions.clearSeenThrough();

  CHECK(!regions.isRegionSeenThrough(2U));
  CHECK(!regions.isSurfaceGroupSeenThrough(1U));
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
  DomainSetup groups;

  groups.add(groups.addSurfaceGroup(), SurfaceSelector::region(2));

  settings.m_regions.m_surfaceGroups = groups.buildSurfaceGroups(pool);
  settings.m_regions.m_coloring      = RegionColoring::SurfaceGroup;

  CHECK(Render::Renderer<T>(settings).pick(view, camera, middle, middle).m_primitive == SurfaceGroups::None);

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

  // A range of surfaces (a level a viewer picks): the jacket's (surface 0) and the core's (1) select the jacket's
  // outside; the core's alone does not.
  const SurfaceSelector entry = settings.m_regions.m_selected[0];

  settings.m_regions.m_hidden      = 0U;
  settings.m_regions.m_selected[0] = SurfaceSelector::surfaces(0, 2);
  settings.m_regions.m_numSelected = 1U;

  CHECK(Render::Renderer<T>(settings).pick(view, camera, middle, middle).m_primitive == RegionSettings::Selected);

  settings.m_regions.m_selected[0] = SurfaceSelector::surfaces(1, 1);

  CHECK(Render::Renderer<T>(settings).pick(view, camera, middle, middle).m_primitive == 1U);

  settings.m_regions.m_selected[0] = entry;

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

TEMPLATE_TEST_CASE("RegionView: shows the whole object only with every region shown and nothing seen through or hidden",
                   "[Overlay][RegionView]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  const Tape<T> tape(pool,
                     Overlay(std::vector<Expr<T>>{RegionName(toExpr(BoxSDF<T>(Vec3(-2, -2, -2), Vec3(2, 2, 2))), "air"),
                                                  RegionName(toExpr(SphereSDF<T>(Vec3::zeros(), T(1))), "ball")}));

  const RegionView<Tape<T>> view(tape);

  CHECK(view.showsWholeObject());
  CHECK_FALSE(view.withVisible(1, false).showsWholeObject());
  CHECK_FALSE(view.withOnly(2).showsWholeObject());
  CHECK_FALSE(view.withSeenThrough(1, true).showsWholeObject());
  CHECK(view.withVisible(1, false).withAllVisible().showsWholeObject());

  RegionSettings settings = view.getSettings();

  settings.setSurfaceGroupHidden(0, true);

  CHECK_FALSE(view.withSettings(settings).showsWholeObject());

  // Showing the whole object, a ray from outside meets the view's surface where it meets the tape's.
  const RayT<T> ray{Vec3(-5, T(0.3), T(0.2)), Vec3(1, 0, 0)};

  CHECK_THAT(view.firstHit(ray, T(0), T(20), RayOptions<T>()).m_t,
             withinAbsT<T>(tape.firstHit(ray, T(0), T(20)).m_t, looseMargin<T>()));
}

TEMPLATE_TEST_CASE(
  "RegionView: showing the whole object, its rays and objectHit() hit where the tape's do, marched shapes included",
  "[Overlay][RegionView]",
  EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  // Two regions, one of a marched smooth union, the other exact; rays from all around, grazing ones included.
  const Expr<T> blob =
    SmoothUnion(std::vector<Expr<T>>{toExpr(SphereSDF<T>(Vec3(T(-0.4), 0, 0), T(0.5))),
                                     toExpr(RoundedCylinderSDF<T>(Vec3(T(0.4), 0, 0), T(0.5), T(0.1), T(0.6)))},
                T(0.2));
  const Expr<T> box = toExpr(BoxSDF<T>(Vec3(T(-1.5), T(-1), T(-0.2)), Vec3(T(1.5), T(-0.6), T(0.2))));

  const Tape<T> tape(pool, Overlay(std::vector<Expr<T>>{RegionName(box, "base"), RegionName(blob, "blob")}));

  const RegionView<Tape<T>> view(tape);

  REQUIRE(view.showsWholeObject());

  size_t hits = 0;

  for (int i = 0; i < 36; i++) {
    for (int j = 0; j < 9; j++) {
      const T    angle = T(2) * T(3.14159265358979) * T(i) / T(36);
      const T    y     = T(-1.1) + T(0.25) * T(j);
      const Vec3 from(T(4) * std::cos(angle), y + T(0.3), T(4) * std::sin(angle));
      const Vec3 to(T(0.1) * std::cos(angle), y, T(0.1) * std::sin(angle));

      const RayT<T> ray{from, (to - from) / (to - from).length()};

      const HitT<T> plain = tape.firstHit(ray, T(0), T(20));
      const HitT<T> seen  = view.firstHit(ray, T(0), T(20), RayOptions<T>());
      const HitT<T> outer = view.objectHit(ray, T(20), RayOptions<T>());

      INFO("ray " << i << ", " << j);

      REQUIRE(seen.hit() == plain.hit());
      REQUIRE(outer.hit() == plain.hit());

      if (plain.hit()) {
        hits++;

        CHECK_THAT(seen.m_t, withinAbsT<T>(plain.m_t, 2.0e-3));
        CHECK_THAT(outer.m_t, withinAbsT<T>(plain.m_t, tightMargin<T>()));
      }
    }
  }

  CHECK(hits > 100U);
}

namespace {

// A scene of regions for the neighborhoods: a box (1) and a ball inside it (2); a box far away (3); a nested
// Overlay, moved (a box, 4, and a ball in it, 5), apart from the first box; and a box touching the first one's
// +x face (6).
template <class T>
Expr<T>
neighbors()
{
  using Vec3 = Vec3T<T>;

  const auto box  = RegionName(SurfaceLabel(BoxSDF<T>(-Vec3::ones(), Vec3::ones()), "box"), "box");
  const auto ball = RegionName(MaterialLabel(SphereSDF<T>(Vec3::zeros(), T(0.5)), "water"), "ball");
  const auto far  = RegionName(BoxSDF<T>(T(5) * Vec3::ones(), T(6) * Vec3::ones()), "far");

  const auto inner = Overlay(RegionName(BoxSDF<T>(T(-0.5) * Vec3::ones(), T(0.5) * Vec3::ones()), "cell"),
                             RegionName(SurfaceLabel(SphereSDF<T>(Vec3::zeros(), T(0.2)), "bead"), "bead"));

  const auto moved = Translate(inner, Vec3(T(3), T(0), T(0)));
  const auto touch = RegionName(BoxSDF<T>(Vec3(T(1), T(-0.5), T(-0.5)), Vec3(T(2), T(0.5), T(0.5))), "touch");

  return Overlay(std::vector<Expr<T>>{toExpr(box), toExpr(ball), toExpr(far), moved, toExpr(touch)});
}

// The regions a neighborhood keeps, as the whole expression numbers them.
template <class T>
std::vector<uint32_t>
keptRegions(const RegionNeighborhood<T>& a_neighborhood)
{
  std::vector<uint32_t> regions;

  for (uint32_t r = 0; r < a_neighborhood.getNumRegions(); r++) {
    regions.push_back(a_neighborhood.wholeRegion(r));
  }

  return regions;
}

} // namespace

TEMPLATE_TEST_CASE("regionNeighborhood: keeps the regions given and those whose boxes meet theirs, in their order",
                   "[Overlay][RegionNeighborhood]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Expr<T>      scene = neighbors<T>();
  const ExprIndex<T> index(scene);

  REQUIRE(index.getNumRegions() == 7U);

  // The ball: the box around it; the box touching the first one meets the box, not the ball.
  const auto ball = regionNeighborhood(scene, {2U});

  CHECK(keptRegions(ball) == std::vector<uint32_t>{0U, 1U, 2U});
  CHECK(ball.region(1U) == 1U);
  CHECK(ball.region(3U) == RegionNeighborhood<T>::None);
  CHECK(ball.region(6U) == RegionNeighborhood<T>::None);

  // The box: the ball inside it, and the box on its face (touching counts).
  CHECK(keptRegions(regionNeighborhood(scene, {1U})) == std::vector<uint32_t>{0U, 1U, 2U, 6U});

  // The moved cell: its ball, through the map; the nested Overlay is kept as a group, renumbered.
  const auto         cell = regionNeighborhood(scene, {4U});
  const ExprIndex<T> cellIndex(cell.object());

  CHECK(keptRegions(cell) == std::vector<uint32_t>{0U, 4U, 5U});
  CHECK(cellIndex.getNumRegionGroups() == 2U);
  CHECK(cellIndex.getRegion(cell.region(5U)).m_name == "bead");

  // The object shrinks to what is kept.
  const BoundingVolumes::AABBT<T> box = cell.object().computeBoundingVolume();

  CHECK(box.getLowCorner()[0] >= T(2.5) - T(looseMargin<T>()));
  CHECK(box.getHighCorner()[0] <= T(3.5) + T(looseMargin<T>()));

  // Through a rotation: a box turned 45 degrees about z, whose corner reaches x = 1, touches the first box's face and
  // is kept with it; moved away, it is dropped.
  const auto turned = [](const T a_x) {
    return Translate(Rotate(BoxSDF<T>(T(-0.5) * Vec3::ones(), T(0.5) * Vec3::ones()), T(45), 2), Vec3(a_x, T(0), T(0)));
  };

  const T reach = T(0.5) * std::sqrt(T(2));

  CHECK(keptRegions(regionNeighborhood(Overlay(BoxSDF<T>(-Vec3::ones(), Vec3::ones()), turned(T(1) + reach)), {1U})) ==
        std::vector<uint32_t>{0U, 1U, 2U});
  CHECK(keptRegions(regionNeighborhood(Overlay(BoxSDF<T>(-Vec3::ones(), Vec3::ones()), turned(T(1.5) + reach)),
                                       {1U})) == std::vector<uint32_t>{0U, 1U});

  // With the background, everything; an object with no Overlay is its own neighborhood.
  CHECK(regionNeighborhood(scene, {0U}).getNumRegions() == 7U);

  const auto one = regionNeighborhood(toExpr(SphereSDF<T>(Vec3::zeros(), T(1))), {1U});

  CHECK(keptRegions(one) == std::vector<uint32_t>{0U, 1U});

  // The surfaces of the kept leaves keep their order; a dropped leaf's have none.
  for (uint32_t s = 0; s < ExprIndex<T>(ball.object()).getNumSurfaces(); s++) {
    CHECK(ball.surface(ball.wholeSurface(s)) == s);
  }

  CHECK(ball.surface(index.getFirstSurface(2U)) == RegionNeighborhood<T>::None);

  REQUIRE(abortsWith([&] { (void)regionNeighborhood(scene, {}); }, "regionNeighborhood: no regions given"));
  REQUIRE(abortsWith([&] { (void)regionNeighborhood(scene, {7U}); }, "regionNeighborhood: region 7"));
}

TEMPLATE_TEST_CASE("regionNeighborhood: the regions given keep their signs, surfaces and the regions across them",
                   "[Overlay][RegionNeighborhood][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  // The scene clipped by a slab, a mask whose leaf belongs to no region and is always kept.
  const Expr<T> scene = Difference(neighbors<T>(), BoxSDF<T>(Vec3(T(-10), T(-10), T(0.7)), Vec3(T(10), T(10), T(10))));

  const Tape<T> whole(pool, scene);

  const std::vector<std::vector<uint32_t>> givens{{1U}, {2U}, {4U, 6U}};

  for (const std::vector<uint32_t>& given : givens) {
    const auto    neighborhood = regionNeighborhood(scene, given);
    const Tape<T> mine(pool, neighborhood.object());

    REQUIRE(neighborhood.getNumRegions() < 7U);

    uint32_t surfaces = 0;

    for (const Vec3& p : gridPoints<T>(T(4), 24)) {
      const Vec3 q = p + Vec3(T(1.5), T(0.013), T(-0.021));

      // A point in a region given is in it in both; elsewhere, in a region not given in both.
      const uint32_t in     = whole.region(q);
      const uint32_t inMine = neighborhood.wholeRegion(mine.region(q));
      const bool     isIn   = std::find(given.begin(), given.end(), in) != given.end();

      CHECK((isIn ? inMine == in : std::find(given.begin(), given.end(), inMine) == given.end()));

      for (const uint32_t r : given) {
        const T a = whole.signedDistance(q, r);
        const T b = mine.signedDistance(q, neighborhood.region(r));

        if (std::abs(a) > T(looseMargin<T>())) {
          CHECK((a < T(0)) == (b < T(0)));
        }

        // Near the region, its closest surface: the same surface, between the same regions.
        if (std::abs(a) < T(0.2)) {
          const SurfaceRef<T> s = whole.closestSurface(q, r);
          const SurfaceRef<T> t = mine.closestSurface(q, neighborhood.region(r));

          CHECK(neighborhood.wholeSurface(t.m_surface) == s.m_surface);
          CHECK(neighborhood.wholeRegion(t.m_ownerRegion) == s.m_ownerRegion);
          CHECK(neighborhood.wholeRegion(t.m_neighborRegion) == s.m_neighborRegion);

          surfaces++;
        }
      }
    }

    CHECK(surfaces > 20U);
  }
}

TEMPLATE_TEST_CASE("regionNeighborhood: a setup translates by numbers and names, leaving out what was dropped",
                   "[Overlay][RegionNeighborhood][Domain]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  const Expr<T>      scene = neighbors<T>();
  const ExprIndex<T> index(scene);
  const Tape<T>      whole(pool, scene);

  // A domain in the box and the ball, with a group per kind of selector, two of them naming only what is dropped.
  DomainSetup setup;

  (void)setup.addSubdomain({1U});
  (void)setup.addSubdomain({2U});

  const uint32_t walls   = setup.addSurfaceGroup();
  const uint32_t ball    = setup.addSurfaceGroup();
  const uint32_t touch   = setup.addSurfaceGroup();
  const uint32_t labeled = setup.addSurfaceGroup();
  const uint32_t water   = setup.addSurfaceGroup();
  const uint32_t rest    = setup.addSurfaceGroup();

  setup.add(walls, SurfaceSelector::interface(1U, 0U));
  setup.add(ball, SurfaceSelector::surface(index.getFirstSurface(1U)));
  setup.add(touch, SurfaceSelector::interface(1U, 6U));
  setup.add(labeled, SurfaceSelector::label(index.findSurfaceLabel("box")));
  setup.add(labeled, SurfaceSelector::label(index.findSurfaceLabel("bead")));
  setup.add(water, SurfaceSelector::materials(index.findMaterialLabel("water"), index.findMaterialLabel("water")));
  setup.add(rest, SurfaceSelector::interface(1U, 3U));
  setup.add(rest, SurfaceSelector::rest());

  const auto        neighborhood = regionNeighborhood(scene, {1U, 2U});
  const DomainSetup translated   = neighborhood.setup(setup);

  // The subdomains and the groups keep their numbers; the bead's label and the far box are gone with their regions.
  CHECK(translated.getNumSubdomains() == 2U);
  CHECK(translated.getNumSurfaceGroups() == 6U);
  CHECK(translated.getNumSelectors() == setup.getNumSelectors() - 2U);
  CHECK(translated.getRegions(0) == std::vector<uint32_t>{neighborhood.region(1U)});

  // Built for the two tapes, the domains agree: the same subdomain at each point, and the same group on each surface.
  const Tape<T> mine(pool, neighborhood.object());
  const Domain  a(pool, whole, setup);
  const Domain  b(pool, mine, translated);

  uint32_t grouped = 0;

  for (const Vec3& p : gridPoints<T>(T(2.5), 20)) {
    const Vec3 q = p + Vec3(T(0.017), T(-0.011), T(0.023));

    CHECK(a.subdomainAt(q) == b.subdomainAt(q));

    for (const uint32_t r : {1U, 2U}) {
      if (std::abs(whole.signedDistance(q, r)) < T(0.2)) {
        const uint32_t group = a.surfaceGroupOf(whole.closestSurface(q, r));

        CHECK(b.surfaceGroupOf(mine.closestSurface(q, neighborhood.region(r))) == group);

        grouped += (group != SurfaceGroups::None) ? 1U : 0U;
      }
    }
  }

  CHECK(grouped > 50U);

  // A subdomain in a region the neighborhood dropped cannot be translated.
  DomainSetup far;

  (void)far.addSubdomain({3U});

  REQUIRE(abortsWith([&] { (void)neighborhood.setup(far); }, "RegionNeighborhood: region 3 of subdomain 0"));
}
