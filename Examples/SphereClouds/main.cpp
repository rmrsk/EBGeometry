// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Particles of differing sizes: the signed distance to their union and the particle closest to a point, through
// a SphereCloudBVH built from the particles' centers and radii, kept in arrays of the program's own; the cloud in a
// tape, cut out of a block, and a ray through both. The last part asks the closest particle on a GPU. See README.md.

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
using Block = EBGeometry::Tape<T, Cloud>;

// What a query reports for the particle it finds: its index in the arrays the cloud was built from, and the signed
// distance to its surface (negative inside it).
using Hit = EBGeometry::SphereCloud::Hit<T>;

// The particles: numParticles in the unit cube, with radii between minRadius and maxRadius.
constexpr std::size_t   numParticles = 2000;
constexpr T             minRadius    = T(0.01);
constexpr T             maxRadius    = T(0.06);
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
  // ── Build the cloud ────────────────────────────────────────────────────────────────────────────────
  //
  // A particle code keeps its particles in arrays of its own; here a center and a radius each. The cloud is built
  // from them (or from one array of SphereCloud::Sphere), and keeps its own copy in the pool, which must outlive
  // it and every copy of it. Per-particle data the cloud does not hold (a material, say) stays in the program's
  // arrays, indexed by what the queries report.
  const std::vector<Vec3> centers = EBGeometry::Random::samplePoints<T>(numParticles, centerSeed);
  const std::vector<Vec3> draws   = EBGeometry::Random::samplePoints<T>(numParticles, radiusSeed);

  std::vector<T>   radii(numParticles);
  std::vector<int> material(numParticles);

  for (std::size_t i = 0; i < numParticles; i++) {
    radii[i]    = minRadius + draws[i][0] * (maxRadius - minRadius);
    material[i] = (radii[i] > T(0.5) * (minRadius + maxRadius)) ? 1 : 0;
  }

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const Cloud cloud(pool, centers, radii);

  // ── Use it on the host ─────────────────────────────────────────────────────────────────────────────
  //
  // signedDistance(p) is the smallest |p - c| - r over the particles: negative inside a particle, the distance to
  // the nearest particle outside them all. closestSphere(p) says which particle that is.
  const std::vector<Vec3> queries = EBGeometry::Random::samplePoints<T>(4, querySeed);

  std::cout << std::fixed << std::setprecision(4) << cloud.numSpheres() << " particles, box "
            << cloud.computeBoundingVolume() << '\n';

  for (const Vec3& q : queries) {
    const Hit hit = cloud.closestSphere(q);

    std::cout << "  at " << q << ": distance " << cloud.signedDistance(q) << ", particle " << std::setw(4) << hit.index
              << " (radius " << radii[hit.index] << ", material " << material[hit.index] << ")\n";
  }

  std::cout << "  at particle 0's center: " << cloud.signedDistance(centers[0]) << " (its radius is " << radii[0]
            << ")\n";

  // ── The cloud in a tape ────────────────────────────────────────────────────────────────────────────
  //
  // The cloud is a shape, and takes part in CSG through a tape: here a block with the particles cut out of it.
  // The tape lives in the cloud's pool. A ray crosses the cloud exactly, through its BVH, and reports the particle
  // it meets; through the tape it is just as exact, and meets the block's surface wherever no particle cuts it.
  const EBGeometry::BoxSDF<T> box(Vec3(T(0.1), T(0.1), T(0.1)), Vec3(T(0.9), T(0.9), T(0.9)));
  const Block                 block(pool, EBGeometry::Difference(EBGeometry::Expr<T>(box), EBGeometry::Expr<T>(cloud)));

  const EBGeometry::RayT<T> ray{Vec3(T(0.5), T(0.5), T(-1)), Vec3(T(0), T(0), T(1))};
  const EBGeometry::HitT<T> particleHit = EBGeometry::firstHit(cloud, ray, T(0), T(3));
  const EBGeometry::HitT<T> blockHit    = EBGeometry::firstHit(block, ray, T(0), T(3));

  std::cout << "A ray up the z axis through (0.5, 0.5) meets particle " << particleHit.m_primitive
            << " at z = " << particleHit.m_point[2] << ", and the block at z = " << blockHit.m_point[2] << '\n';

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
  // A window on the block (built with VIEWER=SDL2; otherwise, with --no-view or without a display, view() prints
  // why and returns). X cuts it open. Given the pool, view() renders on the GPU in a GPU build, tracing the cloud
  // exactly there too.
  EBGeometry::Render::view(block, pool, argc, argv);

  return 0;
}
