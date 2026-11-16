// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

// A model read from text: a cable in a box of air, written in the .ebg language (cable/model.ebg holds the geometry,
// cable/setup.ebg the materials and two domains), read at run time instead of composed in C++. The program compiles
// the object into a tape, builds the flow domain for it, and puts the fluid's properties in the pool as rows of its own
// type, which a query reads on the host and on a GPU. See README.md.

#include <iostream>
#include <string>
#include <vector>

#include <EBGeometry.hpp>
#include <Source/EBGeometry_Language.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T      = EBGEOMETRY_PRECISION;
using Vec3   = EBGeometry::Vec3T<T>;
using Tape   = EBGeometry::Tape<T>;
using Domain = EBGeometry::Domain<T>;

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
  // domains and their surface groups.
  EBGeometry::Render::Settings<T> settings;
  settings.m_colorBy            = EBGeometry::Render::ColorBy::Primitive;
  settings.m_regions.m_coloring = EBGeometry::RegionColoring::Material;
  settings.m_regions.setVisible(1, false);

  const EBGeometry::RegionView<Tape> regions(tape);

  auto camera = EBGeometry::Render::Camera<T>::lookAt(Vec3(7, 5, 9), Vec3(1, 0, 0));

  EBGeometry::Render::view(regions, pool, camera, settings, model.names(), argc, argv);

  return 0;
}
