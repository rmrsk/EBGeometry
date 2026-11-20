// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

// A model read from text: a cable and a sensor in a box of air, written in the .ebg language (cable/model.ebg holds the
// geometry, cable/setup.ebg the materials and three domains), read at run time instead of composed in C++. The program
// compiles the object into a tape, builds the flow domain for it, and puts the fluid's properties in the pool as rows of
// its own type, which a query reads on the host and on a GPU. It also compiles the conduction domain on its own, without
// the sensor it never meets. See README.md.

#include <iostream>
#include <string>
#include <vector>

#include <EBGeometry.hpp>
#include <Source/EBGeometry_Language.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T       = EBGEOMETRY_PRECISION;
using Vec3    = EBGeometry::Vec3T<T>;
using Tape    = EBGeometry::Tape<T>;
using Domain  = EBGeometry::Domain<T>;
using Regions = EBGeometry::RegionView<Tape>;

namespace {

// ── What a solver keeps per subdomain ────────────────────────────────────────────────────────────────
//
// The language passes properties through unread: the program says what they mean, as a row of its own type, filled on
// the host from a subdomain's properties and read in kernels by the subdomain's number.
struct Fluid
{
  T m_density;
  T m_viscosity;
};

// ── The code that runs on either side ────────────────────────────────────────────────────────────────
//
// At a point: the flow domain's subdomain there, and the fluid's density, or 0 outside the fluid (in the cable, beyond the
// box). A trivially copyable functor holding the domain and the table by value, as a kernel receives them.
struct FluidDensity
{
  Domain                             m_domain;
  EBGeometry::Language::Table<Fluid> m_fluids;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3& a_point) const noexcept
  {
    const uint32_t subdomain = m_domain.subdomainAt(a_point);

    return subdomain == Domain::Excluded ? T(0) : m_fluids[subdomain].m_density;
  }
};

} // namespace

int
main(int argc, char* argv[])
{
  // ── Read the model ─────────────────────────────────────────────────────────────────────────────────
  //
  // The setup file includes the geometry (model.ebg); read() reads it and what it includes. A model with errors throws
  // them all, with their files, lines and columns.
  const std::string file = (argc > 1 && argv[1][0] != '-') ? argv[1] : "cable/setup.ebg";

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const auto model = EBGeometry::Language::read<T>(file, pool);

  // ── Build what the program computes on ─────────────────────────────────────────────────────────────
  //
  // The program compiles the object, builds the cfd domain of the model for that tape, and fills the fluid's table from
  // the subdomains' properties: the subdomain's own over its regions' material's. Both live in the tape's pool: one
  // mirror, one lifetime.
  const Tape tape(pool, model.object());

  const auto&  cfd = model.domain("cfd");
  const Domain flow(pool, tape, cfd.setup());

  const auto fluids = cfd.volumes<Fluid>(pool, [](const EBGeometry::Language::Properties& a_properties) {
    return Fluid{T(a_properties.number("density")), T(a_properties.number("viscosity", 0.0))};
  });

  // Is every subdomain given properties, and every piece of surface bounding the domain a boundary condition? If not,
  // the findings say what is missing, at the lines to change.
  const auto check = EBGeometry::Language::completeness(model, tape, "cfd");

  std::cout << "Domain cfd: " << (check.m_complete ? "complete" : "incomplete") << "\n";

  for (const auto& finding : check.m_findings) {
    std::cout << "  " << finding.str() << "\n";
  }

  // ── The pieces of the fluid ────────────────────────────────────────────────────────────────────────
  //
  // Is the fluid one connected piece of space? The description cannot say; the geometry does. The search runs at a
  // resolution, best the solver's cell size (a gap thinner than it may be missed), over a box, here the air box (the
  // object's own box), so that a piece reaching it is on the domain's boundary and one that does not is enclosed. The
  // groups are the domain's subdomains, then its excluded regions.
  EBGeometry::ComponentOptions<T> options;

  options.m_resolution = T(0.05);
  options.m_box        = tape.computeBoundingVolume();

  const auto pieces = EBGeometry::connectedComponents(flow, options);

  std::cout << "Fluid: " << pieces.componentsOf(0U).size() << " connected components at resolution 0.05\n";

  for (const uint32_t c : pieces.componentsOf(0U)) {
    const auto& piece = pieces.component(c);

    std::cout << "  at " << piece.m_seed << ", volume " << piece.m_volume
              << (piece.m_reachesBox ? ", on the box" : ", enclosed") << "\n";
  }

  // ── A domain on its own ────────────────────────────────────────────────────────────────────────────
  //
  // Conduction computes in the cable alone. Its own object keeps the cable's regions and the regions that may touch
  // them, those whose bounding boxes meet theirs: the air around the cable, not the sensor, whose box lies apart. Its
  // regions are numbered anew; its setup is the domain's, translated, with the same subdomains and surface groups.
  const auto   conduction = model.domainObject("conduction");
  const Tape   cable(pool, conduction.object());
  const Domain heat(pool, cable, conduction.setup());

  std::cout << "Domain conduction, on its own: " << conduction.neighborhood().getNumRegions() - 1U << " of "
            << model.index().getNumRegions() - 1U << " regions kept:";

  for (uint32_t r = 1; r < conduction.neighborhood().getNumRegions(); r++) {
    std::cout << " " << model.index().getRegion(conduction.neighborhood().wholeRegion(r)).m_name;
  }

  std::cout << "; " << heat.getNumSubdomainFunctions() - 1U << " subdomains, as in the whole model\n";

  // The viewer gets every domain's own shape: its cost views of a domain shown alone (Only) time that domain's.
  std::vector<EBGeometry::Render::DomainShape<Regions>> domains;

  for (uint32_t d = 0; d < model.getNumDomains(); d++) {
    const auto own = model.domainObject(model.domain(d).name());

    std::vector<uint32_t> wholeRegion;

    for (uint32_t r = 0; r < own.neighborhood().getNumRegions(); r++) {
      wholeRegion.push_back(own.neighborhood().wholeRegion(r));
    }

    domains.push_back({Regions(Tape(pool, own.object())), wholeRegion});
  }

  // ── On the host ────────────────────────────────────────────────────────────────────────────────────
  //
  // Along a line across the cable: the fluid's density, read from the table by the domain's subdomain there.
  std::vector<Vec3> points;

  for (int i = 0; i <= 8; i++) {
    points.emplace_back(T(-1) + T(0.25) * T(i), T(0.05), T(0));
  }

  const FluidDensity query{flow, fluids};

  std::cout << "Fluid density along y = 0.05, z = 0 (on the host):";

  for (const Vec3& p : points) {
    std::cout << " " << query(p);
  }

  std::cout << "\n";

  // ── The same query through GPU::transform ──────────────────────────────────────────────────────────
  //
  // Freeze the pool, mirror it into GPU::memoryResource() (device memory in a CUDA or HIP build), and take device
  // views of the domain (which holds its tape) and the table. Without a GPU, this part is skipped.
  if (EBGeometry::GPU::enabled && !EBGeometry::GPU::deviceAvailable()) {
    std::cout << "No GPU visible: the device part is skipped.\n";
  }
  else {
    pool.freeze();

    const EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(pool, EBGeometry::GPU::memoryResource());

    std::vector<T> values(points.size());

    EBGeometry::GPU::transform(
      FluidDensity{flow.rebasedView(devicePool), fluids.rebasedView(devicePool)}, points, values);

    std::cout << "The same, through GPU::transform (" << EBGeometry::GPU::backendName << "):";

    for (const T v : values) {
      std::cout << " " << v;
    }

    std::cout << "\n";
  }

  // ── Look at it ─────────────────────────────────────────────────────────────────────────────────────
  //
  // The regions, colored by material, with the air hidden, and the model's names: the panel switches between its
  // domains and their surface groups. With a domain shown alone, the cost views (M) trace its own shape.
  EBGeometry::Render::Settings<T> settings;
  settings.m_colorBy            = EBGeometry::Render::ColorBy::Primitive;
  settings.m_regions.m_coloring = EBGeometry::RegionColoring::Material;
  settings.m_regions.setVisible(1, false);

  const Regions regions(tape);

  auto camera = EBGeometry::Render::Camera<T>::lookAt(Vec3(7, 5, 9), Vec3(1, 0, 0));

  EBGeometry::Render::view(regions, pool, camera, settings, model.names(), domains, argc, argv);

  return 0;
}
