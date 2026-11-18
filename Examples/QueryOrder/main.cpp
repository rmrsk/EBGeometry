// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

// Running queries in Morton order: particles that arrive in no spatial order are queried against a mesh, once in
// the order they are stored and once in the order GPU::mortonOrder() gives, read from the particles through a
// source of positions. The results land at the same indices either way; only the time changes. On the host in a
// plain C++ build, on the GPU in a GPU build. See README.md.

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T    = EBGEOMETRY_PRECISION;
using Vec3 = EBGeometry::Vec3T<T>;

// The mesh's BVH branching factor and leaf width, spelled out (see HostTuning for choosing them).
constexpr std::size_t K = 4;
constexpr std::size_t W = 4;

using Mesh = EBGeometry::TriMeshSDF<T, K, W>;

constexpr std::uint32_t numParticles = 200000;

namespace {

// ── The user's data and the query ────────────────────────────────────────────────────────────────────
//
// A particle as a simulation stores it: more than a position.
struct Particle
{
  Vec3 position;
  Vec3 velocity;
};

// The source of positions mortonOrder() reads: point i, from the particles where they are.
struct Positions
{
  const Particle* particles;

  EBGEOMETRY_HOST_DEVICE
  Vec3
  operator()(const std::uint32_t a_i) const noexcept
  {
    return particles[a_i].position;
  }
};

// The query, as a kernel of the user's own: each particle's distance to the wall, written at its index.
struct WallDistance
{
  Mesh            wall;
  const Particle* particles;
  T*              distances;

  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_i) const noexcept
  {
    distances[a_i] = wall.signedDistance(particles[a_i].position);
  }
};

// Milliseconds a call takes: the fastest of three, after one to warm up. The launches return once the kernel has
// finished, so wall time is kernel time.
template <class F>
double
timeIt(const F& a_run)
{
  a_run();

  double best = 1e30;

  for (int i = 0; i < 3; i++) {
    const auto start = std::chrono::steady_clock::now();

    a_run();

    best = std::min(best, std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count());
  }

  return best;
}

} // namespace

// The function-try-block reports a mesh file that cannot be read, rather than letting the exception terminate the
// program.
int
main(int argc, char* argv[])
try {
  // ── The wall and the particles ─────────────────────────────────────────────────────────────────────
  //
  // The default mesh comes from the common-3d-test-models submodule (git submodule update --init); pass another
  // on the command line.
  const std::string file = (argc > 1) ? argv[1] : "../../common-3d-test-models/data/armadillo.obj";

  std::cout << "Reading '" << file << "'\n";

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const Mesh wall = EBGeometry::Parser::readIntoTriMeshSDF<T, K, W>(file, pool);
  const Vec3 lo   = wall.computeBoundingVolume().getLowCorner();
  const Vec3 hi   = wall.computeBoundingVolume().getHighCorner();

  // Particles in the mesh's box, stored in the order they arrived: no spatial order at all.
  std::mt19937_64                   rng(12345);
  std::uniform_real_distribution<T> unit(T(0), T(1));
  std::vector<Particle>             arrived(numParticles);

  for (Particle& p : arrived) {
    p.position = lo + (hi - lo) * Vec3(unit(rng), unit(rng), unit(rng));
    p.velocity = Vec3::zeros();
  }

  if (EBGeometry::GPU::enabled && !EBGeometry::GPU::deviceAvailable()) {
    std::cout << "No GPU is visible: nothing to run.\n";

    return 0;
  }

  // ── Where the launches read ────────────────────────────────────────────────────────────────────────
  //
  // The particles, room for the distances and room for the order, in one pool mirrored where the launches run (a
  // GPU's memory in a GPU build, host memory otherwise), next to the mirrored mesh.
  EBGeometry::Pool                     io(EBGeometry::hostMemoryResource());
  EBGeometry::PODVector<Particle>      particles;
  EBGeometry::PODVector<T>             distances;
  EBGeometry::PODVector<std::uint32_t> order;
  const std::vector<T>                 zeros(numParticles, T(0));
  const std::vector<std::uint32_t>     none(numParticles, 0U);

  particles.reserveFrom(io, numParticles);
  distances.reserveFrom(io, numParticles);
  order.reserveFrom(io, numParticles);
  particles.assign(io.base(), arrived.data(), numParticles);
  distances.assign(io.base(), zeros.data(), numParticles);
  order.assign(io.base(), none.data(), numParticles);

  pool.freeze();
  io.freeze();

  const EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(pool, EBGeometry::GPU::memoryResource());
  const EBGeometry::Pool deviceIO   = EBGeometry::Pool::mirror(io, EBGeometry::GPU::memoryResource());

  const Particle* devParticles = particles.data(static_cast<const void*>(deviceIO.base()));
  T*              devDistances = distances.data(deviceIO.base());

  const WallDistance query{wall.rebasedView(devicePool), devParticles, devDistances};

  // ── In the order they arrived, then in Morton order ────────────────────────────────────────────────
  const double asArrived = timeIt([&] { EBGeometry::GPU::parallelFor(numParticles, query); });

  // The order is computed once, where the pool is, from the particles themselves; while they stay put, every
  // later launch reuses it.
  const double ordering =
    timeIt([&] { EBGeometry::GPU::mortonOrder(deviceIO, Positions{devParticles}, numParticles, order); });

  const EBGeometry::PODSpan<const std::uint32_t> inOrder = order.bind(static_cast<const void*>(deviceIO.base()));

  const double sorted = timeIt([&] { EBGeometry::GPU::parallelFor(numParticles, query, inOrder); });

  std::cout << '\n'
            << numParticles << " distances to the mesh (" << EBGeometry::GPU::backendName << "):\n"
            << std::fixed << std::setprecision(2) << "  in the order the particles arrived  " << std::setw(9)
            << asArrived << " ms\n"
            << "  computing the Morton order          " << std::setw(9) << ordering << " ms (once)\n"
            << "  in Morton order                     " << std::setw(9) << sorted << " ms\n";

  // The distances sit at the particles' own indices, whichever order ran last.
  const EBGeometry::Pool back = EBGeometry::Pool::mirror(deviceIO, EBGeometry::hostMemoryResource());

  std::cout << "The first few distances:";

  for (std::uint32_t i = 0; i < 4; i++) {
    std::cout << ' ' << std::setprecision(4) << distances.at(back.base(), i);
  }

  std::cout << '\n';

  return 0;
} catch (const EBGeometry::Parser::ParseError& e) {
  std::cerr << "Cannot read the mesh: " << e.what() << '\n'
            << "The default mesh is in the common-3d-test-models submodule: git submodule update --init\n";

  return 1;
}
