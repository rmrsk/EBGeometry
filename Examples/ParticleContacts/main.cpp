// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Particles of differing sizes and their contacts: for each particle, the particles it overlaps or comes within a
// gap of, through SphereCloudBVH::forEachContact(). The contacts are listed per particle on the host, in arrays of
// the program's own, and a contact force is summed per particle on a GPU. See README.md.

#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

#include <EBGeometry.hpp>
#include <Source/EBGeometry_Random.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T     = EBGEOMETRY_PRECISION;
using Vec3  = EBGeometry::Vec3T<T>;
using Cloud = EBGeometry::SphereCloudBVH<T>;

// What forEachContact() reports for each contact: the other particle's index, and the separation of the two
// surfaces, |c_i - c_j| - (r_i + r_j), as its distance (negative when they overlap).
using Hit = EBGeometry::SphereCloud::Hit<T>;

// The particles: numParticles in the unit cube, with radii between minRadius and maxRadius. Two particles are in
// contact when their surfaces are less than gap apart.
constexpr std::size_t   numParticles = 3000;
constexpr T             minRadius    = T(0.01);
constexpr T             maxRadius    = T(0.04);
constexpr T             gap          = T(0.005);
constexpr std::uint64_t centerSeed   = 123456789ULL;
constexpr std::uint64_t radiusSeed   = 192837465ULL;

// ── The code that runs on either side ──────────────────────────────────────────────────────────────────
//
// A contact force for EBGeometry::GPU::transform(): one particle's index in, the summed force of the particles it
// overlaps out, each pushing along the line of centers in proportion to the overlap. The query visits the
// particle's contacts and calls the lambda once for each.
struct ContactForce
{
  Cloud m_cloud;

  EBGEOMETRY_HOST_DEVICE
  Vec3
  operator()(const std::uint32_t a_index) const noexcept
  {
    const Vec3 center = m_cloud.sphere(a_index).m_center;
    Vec3       force  = Vec3::zeros();

    m_cloud.forEachContact(a_index, gap, [this, &center, &force](const Hit& a_contact) {
      const Vec3 apart = center - m_cloud.sphere(a_contact.index).m_center;

      if (a_contact.distance < T(0) && apart.length() > T(0)) {
        force += (-a_contact.distance / apart.length()) * apart;
      }
    });

    return force;
  }
};

int
main(int argc, char* argv[])
{
  // ── Build the cloud ────────────────────────────────────────────────────────────────────────────────
  const std::vector<Vec3> centers = EBGeometry::Random::samplePoints<T>(numParticles, centerSeed);
  const std::vector<Vec3> draws   = EBGeometry::Random::samplePoints<T>(numParticles, radiusSeed);

  std::vector<T> radii(numParticles);

  for (std::size_t i = 0; i < numParticles; i++) {
    radii[i] = minRadius + draws[i][0] * (maxRadius - minRadius);
  }

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const Cloud cloud(pool, centers, radii);

  // ── One particle's contacts ────────────────────────────────────────────────────────────────────────
  //
  // forEachContact() visits them in no set order. A contact is mutual: particle 0 is in each of these particles'
  // contacts too, at the same separation.
  std::cout << std::fixed << std::setprecision(4) << "Particle 0 (radius " << radii[0] << ") is in contact with\n";

  cloud.forEachContact(0U, gap, [](const Hit& a_contact) {
    std::cout << "  particle " << std::setw(4) << a_contact.index << ", separation " << a_contact.distance
              << (a_contact.distance < T(0) ? " (overlapping)" : "") << '\n';
  });

  // ── A contact list, in the program's own arrays ────────────────────────────────────────────────────
  //
  // A list holds each particle's contacts in one row: count them, add up the counts into where each row starts,
  // then fill the rows. Each contact appears in the rows of both its particles. The same three steps work on a
  // GPU, with GPU::exclusiveScan() for the second.
  std::vector<std::uint32_t> rowStart(numParticles + 1, 0U);

  for (std::uint32_t i = 0; i < numParticles; i++) {
    cloud.forEachContact(i, gap, [&rowStart, i](const Hit&) { rowStart[i + 1]++; });
  }

  for (std::size_t i = 0; i < numParticles; i++) {
    rowStart[i + 1] += rowStart[i];
  }

  std::vector<Hit>           contacts(rowStart[numParticles]);
  std::vector<std::uint32_t> filled(rowStart.begin(), rowStart.end() - 1);

  for (std::uint32_t i = 0; i < numParticles; i++) {
    cloud.forEachContact(i, gap, [&contacts, &filled, i](const Hit& a_contact) { contacts[filled[i]++] = a_contact; });
  }

  std::cout << contacts.size() / 2 << " contacts between " << numParticles << " particles; particle 1 has "
            << rowStart[2] - rowStart[1] << ", particle 2 has " << rowStart[3] - rowStart[2] << '\n';

  // ── The contact force on a GPU ─────────────────────────────────────────────────────────────────────
  //
  // Freeze the pool, mirror it into GPU::memoryResource(), and take a rebasedView() of the cloud that resolves
  // against the copy. GPU::transform() runs ContactForce once per particle, one thread each, with no list and no
  // atomics. In a plain C++ build the same lines run on the host.
  pool.freeze();

  if (EBGeometry::GPU::enabled && !EBGeometry::GPU::deviceAvailable()) {
    std::cout << "On the device: skipped, no GPU is visible\n";
  }
  else {
    const EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(pool, EBGeometry::GPU::memoryResource());

    std::vector<std::uint32_t> particles(numParticles);
    std::vector<Vec3>          forces(numParticles);

    for (std::uint32_t i = 0; i < numParticles; i++) {
      particles[i] = i;
    }

    EBGeometry::GPU::transform(ContactForce{cloud.rebasedView(devicePool)}, particles, forces);

    std::cout << "Contact forces through GPU::transform (backend: " << EBGeometry::GPU::backendName << "):\n";

    for (std::uint32_t i = 0; i < 3; i++) {
      std::cout << "  particle " << i << ": " << forces[i] << '\n';
    }
  }

  // ── Look at it ─────────────────────────────────────────────────────────────────────────────────────
  //
  // A window on the particles (built with VIEWER=SDL2; otherwise, with --no-view or without a display, view()
  // prints why and returns). X cuts the cloud open.
  EBGeometry::Render::view(cloud, pool, argc, argv);

  return 0;
}
