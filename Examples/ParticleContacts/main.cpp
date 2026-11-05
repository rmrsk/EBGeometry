// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Particles of differing sizes packed in a cylinder, and their contacts: for each particle, the particles that come
// within a gap of it, through SphereCloudBVH::forEachContact(). Particles seeded at random overlap, so they are first
// pushed apart, a round at a time, rebuilding the cloud after each round, until none overlap. Their contacts are then
// listed per particle on the host, in arrays of the program's own, and a near-contact force is summed per particle on
// a GPU. See README.md.

#include <algorithm>
#include <cmath>
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

using T       = EBGEOMETRY_PRECISION;
using Vec3    = EBGeometry::Vec3T<T>;
using Cloud   = EBGeometry::SphereCloudBVH<T>;
using Spheres = EBGeometry::SphereCloud::
  FromCentersAndRadii<T, EBGeometry::PointCloud::FromArray<T>, EBGeometry::SphereCloud::FromRadii<T>>;

// What forEachContact() reports for each contact: the other particle's index, and the separation of the two
// surfaces, |c_i - c_j| - (r_i + r_j), as its distance (negative when they overlap).
using Hit = EBGeometry::SphereCloud::Hit<T>;

// The particles: numParticles seeded in an upright cylinder (along y), with radii between minRadius and maxRadius. Two
// particles are in contact when their surfaces are less than gap apart. Overlapping particles are pushed until they
// are about skin apart, in at most maxRounds rounds.
constexpr std::uint32_t numParticles = 3000;
constexpr T             cylRadius    = T(0.4);
constexpr T             cylHeight    = T(1.3);
constexpr T             twoPi        = T(6.283185307179586);
constexpr T             minRadius    = T(0.01);
constexpr T             maxRadius    = T(0.04);
constexpr T             gap          = T(0.005);
constexpr T             skin         = T(0.0005);
constexpr int           maxRounds    = 200;
constexpr std::size_t   leafSize     = 64;
constexpr std::uint64_t centerSeed   = 123456789ULL;
constexpr std::uint64_t radiusSeed   = 192837465ULL;

// ── The code that runs on either side ──────────────────────────────────────────────────────────────────
//
// A repulsion for EBGeometry::GPU::transform(): one particle's index in, the summed push of the particles closer to
// it than m_range out. Each pushes along the line of centers, in proportion to how much closer than m_range it is,
// as the short-range forces of a particle code do. The query visits those particles and calls the lambda for each.
struct Repulsion
{
  Cloud m_cloud;
  T     m_range;

  EBGEOMETRY_HOST_DEVICE
  Vec3
  operator()(const std::uint32_t a_index) const noexcept
  {
    const Vec3 center = m_cloud.sphere(a_index).m_center;
    Vec3       push   = Vec3::zeros();

    m_cloud.forEachContact(a_index, m_range, [this, &center, &push](const Hit& a_contact) {
      const Vec3 apart  = center - m_cloud.sphere(a_contact.index).m_center;
      const T    length = apart.length();

      if (length > T(0)) {
        push += ((m_range - a_contact.distance) / length) * apart;
      }
    });

    return push;
  }
};

namespace {

// Keeps a particle in the cylinder: its center at least its radius from the wall and from both ends.
void
keepInside(Vec3& a_center, const T a_radius)
{
  const T rho  = std::sqrt(a_center[0] * a_center[0] + a_center[2] * a_center[2]);
  const T most = cylRadius - a_radius;

  if (rho > most) {
    a_center[0] *= most / rho;
    a_center[2] *= most / rho;
  }

  a_center[1] = std::clamp(a_center[1], a_radius, cylHeight - a_radius);
}

} // namespace

int
main(int argc, char* argv[])
{
  // ── Build the cloud, with room to rebuild ──────────────────────────────────────────────────────────
  //
  // Centers uniform in the cylinder, from uniform draws in the unit cube. The cloud reads the program's own centers
  // and radii through Spheres; a particle is known by its index in them. The last argument of build() reserves room
  // for rebuilds of up to that many particles.
  std::vector<Vec3>       centers = EBGeometry::Random::samplePoints<T>(numParticles, centerSeed);
  const std::vector<Vec3> draws   = EBGeometry::Random::samplePoints<T>(numParticles, radiusSeed);

  std::vector<T> radii(numParticles);

  for (std::size_t i = 0; i < numParticles; i++) {
    const T rho   = cylRadius * std::sqrt(centers[i][0]);
    const T theta = twoPi * centers[i][1];

    centers[i] = Vec3(rho * std::cos(theta), cylHeight * centers[i][2], rho * std::sin(theta));
    radii[i]   = minRadius + draws[i][0] * (maxRadius - minRadius);

    keepInside(centers[i], radii[i]);
  }

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const Spheres spheres{EBGeometry::PointCloud::FromArray<T>{centers.data()},
                        EBGeometry::SphereCloud::FromRadii<T>{radii.data()}};

  Cloud cloud = Cloud::build(pool, spheres, numParticles, leafSize, numParticles);

  // ── Push the particles apart ───────────────────────────────────────────────────────────────────────
  //
  // A contact within a gap of 0 is an overlap. Each round moves every particle by half the push of the particles
  // closer to it than skin (half, since both particles of a pair move), keeps it in the cylinder, then rebuilds the
  // cloud from the moved centers, in the arrays build() reserved.
  std::vector<Vec3> push(numParticles);

  for (int round = 0;; round++) {
    std::size_t overlaps = 0;

    for (std::uint32_t i = 0; i < numParticles; i++) {
      cloud.forEachContact(i, T(0), [&overlaps](const Hit&) { overlaps++; });
    }

    if (round % 10 == 0 || overlaps == 0) {
      std::cout << "Round " << std::setw(3) << round << ": " << overlaps / 2 << " overlapping pairs\n";
    }

    if (overlaps == 0 || round == maxRounds) {
      break;
    }

    const Repulsion repulsion{cloud, skin};

    for (std::uint32_t i = 0; i < numParticles; i++) {
      push[i] = repulsion(i);
    }

    for (std::size_t i = 0; i < numParticles; i++) {
      centers[i] += T(0.5) * push[i];

      keepInside(centers[i], radii[i]);
    }

    cloud.rebuild(spheres, numParticles);
  }

  // ── One particle's contacts ────────────────────────────────────────────────────────────────────────
  //
  // forEachContact() visits them in no set order. A contact is mutual: particle 0 is in each of these particles'
  // contacts too, at the same separation.
  std::cout << std::fixed << std::setprecision(4) << "Particle 0 (radius " << radii[0] << ") is in contact with\n";

  cloud.forEachContact(0U, gap, [](const Hit& a_contact) {
    std::cout << "  particle " << std::setw(4) << a_contact.index << ", separation " << a_contact.distance << '\n';
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

  // ── The near-contact force on a GPU ────────────────────────────────────────────────────────────────
  //
  // Freeze the pool, mirror it into GPU::memoryResource(), and take a rebasedView() of the cloud that resolves
  // against the copy. GPU::transform() runs Repulsion over the gap once per particle, one thread each, with no list
  // and no atomics. In a plain C++ build the same lines run on the host.
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

    EBGeometry::GPU::transform(Repulsion{cloud.rebasedView(devicePool), gap}, particles, forces);

    std::cout << "Near-contact forces through GPU::transform (backend: " << EBGeometry::GPU::backendName << "):\n";

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
