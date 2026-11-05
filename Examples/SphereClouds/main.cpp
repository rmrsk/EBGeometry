// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Particles of differing sizes packed in a cylinder: the signed distance to their union and the particle closest to a
// point, through a SphereCloudBVH built from the particles' centers and radii, kept in arrays of the program's own;
// the cylinder with the particles cut out of it, fused and through a tape, and a ray through both. The last part asks
// the closest particle on a GPU. See README.md.

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

using T     = EBGEOMETRY_PRECISION;
using Vec3  = EBGeometry::Vec3T<T>;
using Cloud = EBGeometry::SphereCloudBVH<T>;

// A tape that can hold a cloud: its type is listed among the tape's primitives, as a mesh SDF's would be.
using Porous = EBGeometry::Tape<T, Cloud>;

// What a query reports for the particle it finds: its index in the arrays the cloud was built from, and the signed
// distance to its surface (negative inside it).
using Hit = EBGeometry::SphereCloud::Hit<T>;

// The particles: up to numParticles with radii between minRadius and maxRadius, their centers in an upright cylinder
// (along y, from 0 to cylHeight) of radius cylRadius, tried at up to numTries random positions in all.
constexpr std::size_t   numParticles = 2400;
constexpr std::size_t   numTries     = 50 * numParticles;
constexpr T             minRadius    = T(0.01);
constexpr T             maxRadius    = T(0.035);
constexpr T             cylRadius    = T(0.4);
constexpr T             cylHeight    = T(1);
constexpr T             twoPi        = T(6.283185307179586);
constexpr std::uint64_t centerSeed   = 123456789ULL;
constexpr std::uint64_t radiusSeed   = 192837465ULL;
constexpr std::uint64_t querySeed    = 987654321ULL;

// ── The code that runs on either side ──────────────────────────────────────────────────────────────────
//
// A query for EBGeometry::GPU::transform(): a trivially copyable functor whose EBGEOMETRY_HOST_DEVICE call operator
// maps one point to one result. It holds the cloud by value: a cloud owns no memory, it describes the spheres and
// the BVH in a Pool, so a kernel can receive it once it describes a pool the device can read.
struct ClosestParticle
{
  Cloud m_cloud;

  EBGEOMETRY_HOST_DEVICE
  Hit
  operator()(const Vec3& a_point) const noexcept
  {
    return m_cloud.closestSphere(a_point);
  }
};

int
main(int argc, char* argv[])
{
  // ── Pack the particles ─────────────────────────────────────────────────────────────────────────────
  //
  // A particle code keeps its particles in arrays of its own; here a center and a radius each. They are placed one at
  // a time, largest first, each at the first random position in the cylinder where it overlaps no particle placed
  // before it: a loose random packing. A center can lie near the wall, so a particle can stick out of the cylinder.
  const std::vector<Vec3> tries = EBGeometry::Random::samplePoints<T>(numTries, centerSeed);
  const std::vector<Vec3> draws = EBGeometry::Random::samplePoints<T>(numParticles, radiusSeed);

  std::vector<T> sizes(numParticles);

  for (std::size_t i = 0; i < numParticles; i++) {
    sizes[i] = minRadius + draws[i][0] * (maxRadius - minRadius);
  }

  std::sort(sizes.begin(), sizes.end(), [](const T a, const T b) { return a > b; });

  std::vector<Vec3> centers;
  std::vector<T>    radii;
  std::size_t       next = 0;

  for (const T radius : sizes) {
    while (next < numTries) {
      const Vec3& u     = tries[next++];
      const T     rho   = cylRadius * std::sqrt(u[0]);
      const T     theta = twoPi * u[1];
      const Vec3  center(rho * std::cos(theta), cylHeight * u[2], rho * std::sin(theta));

      bool free = true;

      for (std::size_t j = 0; j < centers.size() && free; j++) {
        free = (center - centers[j]).length() >= radius + radii[j];
      }

      if (free) {
        centers.push_back(center);
        radii.push_back(radius);

        break;
      }
    }
  }

  // ── Build the cloud ────────────────────────────────────────────────────────────────────────────────
  //
  // The cloud is built from the program's arrays (or from one array of SphereCloud::Sphere), and keeps its own copy
  // in the pool, which must outlive it and every copy of it. Per-particle data the cloud does not hold (a material,
  // say) stays in the program's arrays, indexed by what the queries report.
  std::vector<int> material(radii.size());

  for (std::size_t i = 0; i < radii.size(); i++) {
    material[i] = (radii[i] > T(0.5) * (minRadius + maxRadius)) ? 1 : 0;
  }

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const Cloud cloud(pool, centers, radii);

  // ── Use it on the host ─────────────────────────────────────────────────────────────────────────────
  //
  // signedDistance(p) is the smallest |p - c| - r over the particles: negative inside a particle, the distance to
  // the nearest particle outside them all. closestSphere(p) says which particle that is.
  std::vector<Vec3> queries = EBGeometry::Random::samplePoints<T>(4, querySeed);

  for (Vec3& q : queries) {
    q = Vec3(cylRadius * (T(2) * q[0] - T(1)), cylHeight * q[1], cylRadius * (T(2) * q[2] - T(1)));
  }

  std::cout << std::fixed << std::setprecision(4) << cloud.numSpheres() << " particles, box "
            << cloud.computeBoundingVolume() << '\n';

  for (const Vec3& q : queries) {
    const Hit hit = cloud.closestSphere(q);

    std::cout << "  at " << q << ": distance " << cloud.signedDistance(q) << ", particle " << std::setw(4) << hit.index
              << " (radius " << radii[hit.index] << ", material " << material[hit.index] << ")\n";
  }

  std::cout << "  at particle 0's center: " << cloud.signedDistance(centers[0]) << " (its radius is " << radii[0]
            << ")\n";

  // ── The cylinder with the particles cut out ────────────────────────────────────────────────────────
  //
  // The cloud is a shape, and takes part in CSG like any other: here the cylinder minus the particles, a porous
  // solid. Fused::Difference fixes the composition at compile time, and is the faster where it is known then; a tape,
  // which lists the cloud among its primitives, holds a composition built at run time. Both live in the cloud's pool.
  // A ray crosses the cloud exactly, through its BVH, and reports the particle it meets; through either composition it
  // is just as exact, and meets the cylinder's wall wherever no particle cuts it.
  const EBGeometry::CylinderSDF<T> cylinder(Vec3(T(0), T(0), T(0)), Vec3(T(0), cylHeight, T(0)), cylRadius);

  const auto   porous = EBGeometry::Fused::Difference(cylinder, cloud);
  const Porous porousTape(pool, EBGeometry::Difference(EBGeometry::Expr<T>(cylinder), EBGeometry::Expr<T>(cloud)));

  const EBGeometry::RayT<T> ray{Vec3(T(-1), T(0.5), T(0)), Vec3(T(1), T(0), T(0))};
  const EBGeometry::HitT<T> particleHit = EBGeometry::firstHit(cloud, ray, T(0), T(3));
  const EBGeometry::HitT<T> porousHit   = EBGeometry::firstHit(porous, ray, T(0), T(3));
  const EBGeometry::HitT<T> tapeHit     = EBGeometry::firstHit(porousTape, ray, T(0), T(3));

  std::cout << "A ray along x at height 0.5 meets particle " << particleHit.m_primitive
            << " at x = " << particleHit.m_point[0] << ", and the porous cylinder at x = " << porousHit.m_point[0]
            << " (through the tape: " << tapeHit.m_point[0] << ")\n";

  // Two objects in one scene, side by side: the packed particles, and the porous cylinder beside them.
  const auto scene = EBGeometry::Fused::Union(cloud, EBGeometry::Fused::Translate(porous, Vec3(T(1), T(0), T(0))));

  // ── The same query on a GPU ────────────────────────────────────────────────────────────────────────
  //
  // Freeze the pool, mirror it in one piece into GPU::memoryResource() (device memory in a CUDA or HIP build), and
  // take a rebasedView() of the cloud that resolves against the copy. GPU::transform() then runs the query once
  // per point in a kernel. In a plain C++ build the same lines run on the host.
  pool.freeze();

  if (EBGeometry::GPU::enabled && !EBGeometry::GPU::deviceAvailable()) {
    std::cout << "On the device: skipped, no GPU is visible\n";
  }
  else {
    const EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(pool, EBGeometry::GPU::memoryResource());

    std::vector<Hit> hits(queries.size());

    EBGeometry::GPU::transform(ClosestParticle{cloud.rebasedView(devicePool)}, queries, hits);

    std::cout << "Through GPU::transform (backend: " << EBGeometry::GPU::backendName << "):\n";

    for (std::size_t q = 0; q < queries.size(); q++) {
      std::cout << "  query " << q << ": particle " << std::setw(4) << hits[q].index << ", distance "
                << hits[q].distance << '\n';
    }
  }

  // ── Look at it ─────────────────────────────────────────────────────────────────────────────────────
  //
  // A window on the scene (built with VIEWER=SDL2; otherwise, with --no-view or without a display, view() prints
  // why and returns). X cuts it open. Given the pool, view() renders on the GPU in a GPU build, tracing the cloud
  // exactly there too.
  EBGeometry::Render::view(scene, pool, argc, argv);

  return 0;
}
