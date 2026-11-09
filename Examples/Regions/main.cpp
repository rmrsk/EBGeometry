// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Regions of several materials in one object: a cable, a copper core inside a PTFE jacket, in air, cut open by a
// slab. Overlay() makes the parts regions rather than one solid, so the core keeps its surface inside the jacket;
// MaterialLabel() names each region's material. The tape answers which region a point is in and each region's own
// implicit function; the viewer shows the regions, one color each, and hides them (V). See README.md.

#include <iomanip>
#include <iostream>
#include <string>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T    = EBGEOMETRY_PRECISION;
using Vec3 = EBGeometry::Vec3T<T>;
using Expr = EBGeometry::Expr<T>;
using Tape = EBGeometry::Tape<T, EBGeometry::TapeFeature::Regions>; // a tape that answers for its regions

int
main(int argc, char* argv[])
{
  // ── Build the object ───────────────────────────────────────────────────────────────────────────────
  //
  // A cable along z: the jacket, then the core on top of it (later operands win where they overlap), in air.
  // A union would make one solid of them, and the core's surface would vanish inside the jacket.
  // Each region has a name (which region it is) and a material (what it is made of).
  const auto jacket =
    EBGeometry::RegionName(EBGeometry::MaterialLabel(
                             EBGeometry::CylinderSDF<T>(Vec3(T(0), T(0), T(-2)), Vec3(T(0), T(0), T(2)), T(1)), "PTFE"),
                           "jacket");
  const auto core = EBGeometry::RegionName(
    EBGeometry::MaterialLabel(EBGeometry::CylinderSDF<T>(Vec3(T(0), T(0), T(-2.5)), Vec3(T(0), T(0), T(2.5)), T(0.4)),
                              "copper"),
    "core");

  const Expr cable = EBGeometry::Overlay(jacket, core, "air");

  // Cut open: a slab removed from every region (a mask), so that the core shows.
  const Expr object =
    EBGeometry::Difference(cable, EBGeometry::BoxSDF<T>(Vec3(T(0), T(-2), T(-1)), Vec3(T(2), T(2), T(1))));

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const Tape tape(pool, object);

  // ── The regions and their materials ────────────────────────────────────────────────────────────────
  //
  // Region 0 is the outside; then each operand of the Overlay, in order. ExprIndex, on the host, holds the
  // names; the tape answers on the host and on a GPU.
  const EBGeometry::ExprIndex<T> index(object);

  std::cout << index.getNumRegions() << " regions:\n";

  for (uint32_t k = 0; k < index.getNumRegions(); k++) {
    const std::string& name = index.getRegion(k).m_name;

    std::cout << "  region " << k << ": " << (name.empty() ? std::string("outside") : name) << ", "
              << index.getMaterialLabelName(index.getMaterialOf(k)) << '\n';
  }

  // ── Which region, and each region's function ───────────────────────────────────────────────────────
  //
  // Along a line through the cable: region(p) says where the point is, and signedDistance(p, region) is that
  // region's implicit function, negative inside it, zero on its whole surface (the interface with the core
  // included): what a cut-cell code cuts for the jacket alone.
  std::cout << std::fixed << std::setprecision(3) << "Along x, at z = 1.5:\n";

  for (const T x : {T(-1.5), T(-0.8), T(-0.2), T(0.2), T(0.8)}) {
    const Vec3 p(x, T(0), T(1.5));

    std::cout << "  x = " << std::setw(6) << x << ": region " << tape.region(p) << ", jacket's function "
              << std::setw(6) << tape.signedDistance(p, 1) << ", core's " << std::setw(6) << tape.signedDistance(p, 2)
              << '\n';
  }

  // ── What a solver cuts ─────────────────────────────────────────────────────────────────────────────
  //
  // A solver says which regions it computes in (Phases), and each surface is then, at a point, a boundary (one
  // side solved: its condition comes from its surface label), an interface (two phases meet) or nothing to it.
  // closestSurface(p, region) names the surface and the regions on its two sides there.
  EBGeometry::Phases cfd;            // flow in the air only
  EBGeometry::Phases electrostatics; // the air and the jacket; the core is a conductor

  cfd.addPhase({0});
  electrostatics.addPhase({0});
  electrostatics.addPhase({1});

  const auto roleName = [](const EBGeometry::SurfaceRole a_role) {
    return a_role == EBGeometry::SurfaceRole::Boundary    ? "boundary"
           : a_role == EBGeometry::SurfaceRole::Interface ? "interface"
           : a_role == EBGeometry::SurfaceRole::Internal  ? "internal"
                                                          : "not cut";
  };

  std::cout << "Surfaces of the jacket, at z = 1.5:\n";

  for (const T x : {T(1), T(0.4)}) {
    const EBGeometry::SurfaceRef<T> s = tape.closestSurface(Vec3(x, T(0), T(1.5)), 1);

    std::cout << "  x = " << x << ": between regions " << s.m_insideRegion << " and " << s.m_outsideRegion
              << "; to flow " << roleName(cfd.role(s)) << ", to electrostatics " << roleName(electrostatics.role(s))
              << '\n';
  }

  // ── Look at it ─────────────────────────────────────────────────────────────────────────────────────
  //
  // RegionView shows the regions' surfaces, internal ones included. What it shows lives in the render settings
  // (m_regions): the coloring (K), the regions shown (V), the surfaces seen through (T) or shown alone (L), and a
  // solver's scheme, here the electrostatic one: every conductor surface (dirichlet), the jacket's outside (jump).
  // K reaches the coloring by group. In the regions' menu (F5), a click names the piece of surface under it, Enter
  // adds it to the current group, and a panel lists the regions and the groups by their names; C prints the scheme
  // as C++.
  EBGeometry::Render::Settings<T> settings;

  settings.m_colorBy = EBGeometry::Render::ColorBy::Primitive;

  EBGeometry::Scheme& scheme = settings.m_regions.m_scheme;

  const uint32_t dirichlet = scheme.addSurfaceGroup();
  const uint32_t jump      = scheme.addSurfaceGroup();

  scheme.add(dirichlet, EBGeometry::SurfaceSelector::region(2));
  scheme.add(jump, EBGeometry::SurfaceSelector::interface(0, 1));

  // The names the window shows: the regions by their names and materials, and the surface groups.
  EBGeometry::RegionNames names(index);

  names.setSurfaceGroup(dirichlet, "dirichlet");
  names.setSurfaceGroup(jump, "jump");

  // Colors by name (linear RGB): the materials, and the surface groups when colored by them.
  names.setColor("copper", EBGeometry::Vec3T<double>(0.75, 0.33, 0.12));
  names.setColor("PTFE", EBGeometry::Vec3T<double>(0.80, 0.80, 0.75));
  names.setColor("dirichlet", EBGeometry::Vec3T<double>(0.85, 0.15, 0.10));
  names.setColor("jump", EBGeometry::Vec3T<double>(0.10, 0.35, 0.85));

  settings.m_regions.m_coloring = EBGeometry::RegionColoring::Material;

  const EBGeometry::RegionView<Tape> regions(tape);

  // From the side the cut is on, so that the core shows in it.
  auto camera = EBGeometry::Render::Camera<T>::fit(regions.computeBoundingVolume());

  camera.m_yaw = T(70);

  // With the tape's pool, a GPU build renders on the device (G switches between the GPU and the CPU).
  EBGeometry::Render::view(regions, pool, camera, settings, names, argc, argv);

  return 0;
}
