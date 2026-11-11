// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Regions of several materials in one object: a cable, a copper core inside a PTFE jacket, in a box of air.
// Overlay() stacks the parts as regions rather than one solid, so the core keeps its surface inside the jacket;
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
using Tape = EBGeometry::Tape<T>; // a tape answers for its regions too

int
main(int argc, char* argv[])
{
  // ── Build the object ───────────────────────────────────────────────────────────────────────────────
  //
  // A cable along z in a box of air: the air, the jacket on top of it, then the core on top of that (later operands
  // win where they overlap). A union would make one solid of them, and the core's surface would vanish inside the
  // jacket. Each region has a name (which region it is) and a material (what it is made of). The air is a region like
  // the others: its box, around the whole cable, is the outer boundary of what a solver computes on.
  const auto air = EBGeometry::RegionName(
    EBGeometry::MaterialLabel(EBGeometry::BoxSDF<T>(Vec3(T(-1.5), T(-1.5), T(-3)), Vec3(T(1.5), T(1.5), T(3))), "air"),
    "air");
  const auto jacket =
    EBGeometry::RegionName(EBGeometry::MaterialLabel(
                             EBGeometry::CylinderSDF<T>(Vec3(T(0), T(0), T(-2)), Vec3(T(0), T(0), T(2)), T(1)), "PTFE"),
                           "jacket");
  const auto core = EBGeometry::RegionName(
    EBGeometry::MaterialLabel(EBGeometry::CylinderSDF<T>(Vec3(T(0), T(0), T(-2.5)), Vec3(T(0), T(0), T(2.5)), T(0.4)),
                              "copper"),
    "core");

  const Expr object = EBGeometry::Overlay(air, jacket, core);

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const Tape tape(pool, object);

  // ── The regions and their materials ────────────────────────────────────────────────────────────────
  //
  // Region 0 is the outside, beyond the box, with no material; then each operand of the Overlay, in order. ExprIndex, on the host, holds the
  // names; the tape answers on the host and on a GPU.
  const EBGeometry::ExprIndex<T> index(object);

  std::cout << index.getNumRegions() << " regions:\n";

  for (uint32_t k = 0; k < index.getNumRegions(); k++) {
    const std::string&                name     = index.getRegion(k).m_name;
    const EBGeometry::MaterialLabelId material = index.getMaterialOf(k);

    std::cout << "  region " << k << ": " << (name.empty() ? std::string("outside") : name) << ", "
              << (material.isNone() ? std::string("no material") : index.getMaterialLabelName(material)) << '\n';
  }

  // A name with nothing to name (a region buried under later ones, a label on a covered surface) gets no boundary
  // condition, silently; checkLabels() reports the ones it can prove lost. Here there are none.
  for (const std::string& warning : index.checkLabels(object, tape)) {
    std::cout << "Warning: " << warning << '\n';
  }

  // ── Which region, and each region's function ───────────────────────────────────────────────────────
  //
  // Along a line through the cable: region(p) says where the point is, and signedDistance(p, region) is that
  // region's implicit function, negative inside it, zero on its whole surface (the interface with the core
  // included): what a cut-cell code cuts for the jacket alone.
  std::cout << std::fixed << std::setprecision(3) << "Along x, at z = 1.5:\n";

  for (const T x : {T(-1.2), T(-0.8), T(-0.2), T(0.2), T(0.8)}) {
    const Vec3 p(x, T(0), T(1.5));

    std::cout << "  x = " << std::setw(6) << x << ": region " << tape.region(p) << ", jacket's function "
              << std::setw(6) << tape.signedDistance(p, 2) << ", core's " << std::setw(6) << tape.signedDistance(p, 3)
              << '\n';
  }

  // ── What a solver cuts ─────────────────────────────────────────────────────────────────────────────
  //
  // A downstream solver computes in subdomains, each a set of regions, coupled through the interfaces between them;
  // each surface is then, at a point, a boundary (one side in its regions), an interface (two subdomains meet) or
  // nothing to it.
  // closestSurface(p, region) names the surface and the regions on its two sides there.
  EBGeometry::DomainSetup cfd;            // flow in the air only
  EBGeometry::DomainSetup electrostatics; // the air and the jacket; the core is a conductor

  cfd.addSubdomain({1});
  electrostatics.addSubdomain({1});
  electrostatics.addSubdomain({2});

  const auto roleName = [](const EBGeometry::SurfaceRole a_role) {
    return a_role == EBGeometry::SurfaceRole::Boundary    ? "boundary"
           : a_role == EBGeometry::SurfaceRole::Interface ? "interface"
           : a_role == EBGeometry::SurfaceRole::Internal  ? "internal"
                                                          : "not cut";
  };

  std::cout << "Surfaces of the jacket, at z = 1.5:\n";

  for (const T x : {T(1), T(0.4)}) {
    const EBGeometry::SurfaceRef<T> s = tape.closestSurface(Vec3(x, T(0), T(1.5)), 2);

    std::cout << "  x = " << x << ": between regions " << s.m_ownerRegion << " and " << s.m_neighborRegion
              << "; to flow " << roleName(cfd.role(s)) << ", to electrostatics " << roleName(electrostatics.role(s))
              << '\n';
  }

  // ── The domains ────────────────────────────────────────────────────────────────────────────────────
  //
  // A domain describes what a downstream solver computes on: its regions (the subdomains above) and its surface groups,
  // one per boundary condition: the group's name is the condition's tag, its number the id a kernel reads
  // (domain.surfaceGroupOf(surface)); the first selector that matches a surface decides. Electrostatics: every conductor
  // surface is dirichlet, the box grounded, the jacket's outside a jump; flow: the box is the far field, every other
  // surface of the air a wall. The setups describe the domains; a Domain is built from one, for the tape.
  const uint32_t dirichlet = electrostatics.addSurfaceGroup();
  const uint32_t ground    = electrostatics.addSurfaceGroup();
  const uint32_t jump      = electrostatics.addSurfaceGroup();

  electrostatics.add(dirichlet, EBGeometry::SurfaceSelector::region(3));
  electrostatics.add(ground, EBGeometry::SurfaceSelector::interface(1, 0));
  electrostatics.add(jump, EBGeometry::SurfaceSelector::interface(1, 2));

  const uint32_t farField = cfd.addSurfaceGroup();
  const uint32_t wall     = cfd.addSurfaceGroup();

  cfd.add(farField, EBGeometry::SurfaceSelector::interface(1, 0));
  cfd.add(wall, EBGeometry::SurfaceSelector::region(1));

  // The electrostatic domain, its tables in the tape's pool: what a kernel takes (after rebasedView() onto a device).
  const EBGeometry::Domain electrostatic(pool, tape, electrostatics);

  // What a cut-cell code samples at a node: every point has a label (a subdomain, or Excluded where the domain does
  // not compute, the core), and each label a function negative exactly in it. An edge between two nodes of different
  // labels crosses a surface, where the functions of both labels change sign.
  std::cout << "Electrostatics at nodes along x, at z = 1.5 (air, jacket, excluded):\n";

  for (const T x : {T(1.2), T(0.8), T(0.2)}) {
    T values[3];

    electrostatic.signedDistances(Vec3(x, T(0), T(1.5)), values);

    const uint32_t label = electrostatic.labelAt(Vec3(x, T(0), T(1.5)));

    std::cout << "  x = " << x << ": "
              << ((label == electrostatic.Excluded) ? std::string("excluded") : "subdomain " + std::to_string(label))
              << "; functions " << values[0] << ", " << values[1] << ", " << values[2] << '\n';
  }

  // ── Look at it ─────────────────────────────────────────────────────────────────────────────────────
  //
  // RegionView shows the regions' surfaces, internal ones included. The window's panel switches between the
  // domains, names the surface under a click and adds it to a surface group, and checks that every surface a domain
  // touches has one; C prints the active domain's setup as C++.
  EBGeometry::Render::Settings<T> settings;

  settings.m_colorBy = EBGeometry::Render::ColorBy::Primitive;

  // The window opens with the air hidden, so that the cable shows (V and the panel show it again), cut along its axis
  // by the section plane, which is filled by material, so that the core shows (X steps the plane on).
  settings.m_regions.setVisible(1, false);

  settings.m_section       = true;
  settings.m_sectionNormal = Vec3(T(1), T(0), T(0));

  // The names the window shows: the regions by their names and materials, the domains and their surface groups.
  EBGeometry::RegionNames names(index);

  const uint32_t electrostaticDomain = names.addDomain("electrostatics", electrostatics);
  const uint32_t flowDomain          = names.addDomain("flow", cfd);

  names.setSurfaceGroup(electrostaticDomain, dirichlet, "dirichlet");
  names.setSurfaceGroup(electrostaticDomain, ground, "ground");
  names.setSurfaceGroup(electrostaticDomain, jump, "jump");
  names.setSurfaceGroup(flowDomain, farField, "far field");
  names.setSurfaceGroup(flowDomain, wall, "wall");
  names.setSubdomain(electrostaticDomain, 0, "air");
  names.setSubdomain(electrostaticDomain, 1, "jacket");
  names.setSubdomain(flowDomain, 0, "air");

  // Colors by name (linear RGB): the materials, and the surface groups when colored by them.
  names.setColor("copper", EBGeometry::Vec3T<double>(0.75, 0.33, 0.12));
  names.setColor("PTFE", EBGeometry::Vec3T<double>(0.80, 0.80, 0.75));
  names.setColor("air", EBGeometry::Vec3T<double>(0.55, 0.70, 0.90));
  names.setColor("dirichlet", EBGeometry::Vec3T<double>(0.85, 0.15, 0.10));
  names.setColor("ground", EBGeometry::Vec3T<double>(0.20, 0.60, 0.25));
  names.setColor("jump", EBGeometry::Vec3T<double>(0.10, 0.35, 0.85));

  settings.m_regions.m_coloring = EBGeometry::RegionColoring::Material;

  const EBGeometry::RegionView<Tape> regions(tape);

  // From the side the plane keeps open.
  auto camera = EBGeometry::Render::Camera<T>::fit(regions.computeBoundingVolume());

  camera.m_yaw   = T(55);
  camera.m_pitch = T(25);

  // With the tape's pool, a GPU build renders on the device (G switches between the GPU and the CPU).
  EBGeometry::Render::view(regions, pool, camera, settings, names, argc, argv);

  return 0;
}
