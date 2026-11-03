// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Particles that move, and every particle's 8 nearest neighbors after each step. The particles live where the
// computation does (device memory in a CUDA or HIP build), in the program's own layout: one array per coordinate.
// A PointCloudHashGrid is built over them once, with room to grow, and rebuilt in place after every step, from
// the particles' own arrays and without copying them to the host. See README.md.

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T    = EBGEOMETRY_PRECISION;
using Vec3 = EBGeometry::Vec3T<T>;
using Grid = EBGeometry::PointCloudHashGrid<T>;
using Hit  = EBGeometry::PointCloud::Hit<T>;

constexpr uint32_t    numParticles = 50000;
constexpr int         numSteps     = 5;
constexpr T           timeStep     = T(0.02);
constexpr std::size_t k            = 8;

namespace {

// ── The code that runs on either side ────────────────────────────────────────────────────────────────
//
// Trivially copyable structs with an EBGEOMETRY_HOST_DEVICE call operator, for GPU::parallelFor() and
// GPU::transform() (structs rather than lambdas, which nvcc passes to kernels only with --extended-lambda).

// The particles as the program keeps them: positions and velocities, one array per component.
struct Particles
{
  T* x;
  T* y;
  T* z;
  T* vx;
  T* vy;
  T* vz;
};

// Where the grid reads particle i's position: any struct like this one works, whatever the layout.
struct FromParticles
{
  Particles p;

  EBGEOMETRY_HOST_DEVICE
  Vec3
  operator()(const uint32_t i) const noexcept
  {
    return Vec3(p.x[i], p.y[i], p.z[i]);
  }
};

// A number in [0, 1) from a particle index and a salt (splitmix64), for the starting state.
EBGEOMETRY_HOST_DEVICE
T
uniform(const uint64_t i, const uint64_t salt) noexcept
{
  uint64_t h = i * 0x9E3779B97F4A7C15ULL + salt;

  h = (h ^ (h >> 30)) * 0xBF58476D1CE4E5B9ULL;
  h = (h ^ (h >> 27)) * 0x94D049BB133111EBULL;
  h = h ^ (h >> 31);

  return T(h >> 11) * T(1.0 / 9007199254740992.0);
}

// The starting state: particles crowded into a corner of the unit cube, moving outwards.
struct Start
{
  Particles p;

  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t i) const noexcept
  {
    p.x[i]  = T(0.3) * uniform(i, 1);
    p.y[i]  = T(0.3) * uniform(i, 2);
    p.z[i]  = T(0.3) * uniform(i, 3);
    p.vx[i] = uniform(i, 4);
    p.vy[i] = uniform(i, 5);
    p.vz[i] = uniform(i, 6);
  }
};

// One time step: every particle moves, and bounces off the unit cube's walls.
struct Move
{
  Particles p;
  T         dt;

  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t i) const noexcept
  {
    T* const position[3] = {p.x, p.y, p.z};
    T* const velocity[3] = {p.vx, p.vy, p.vz};

    for (int d = 0; d < 3; d++) {
      T& x = position[d][i];
      T& v = velocity[d][i];

      x += v * dt;

      if (x < T(0) || x > T(1)) {
        v = -v;
        x = (x < T(0)) ? -x : T(2) - x;
      }
    }
  }
};

// The query: the distance from particle i to its 8th nearest other particle, a measure of how crowded it is.
// nearestNeighbors() reports each neighbor's index into the particles' arrays, so a program reads whatever it
// keeps per particle (velocity, mass, species) from its own arrays: p.vx[hits[j].index], and so on.
struct EighthNeighbor
{
  Grid grid;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const uint32_t& i) const noexcept
  {
    Hit hits[k];

    return (grid.nearestNeighbors(i, k, hits) == k) ? std::sqrt(hits[k - 1].distanceSquared) : T(0);
  }
};

} // namespace

int
main()
{
  if (EBGeometry::GPU::enabled && !EBGeometry::GPU::deviceAvailable()) {
    std::cout << "No GPU is visible, so the example does not run.\n";

    return 0;
  }

  // ── The particles ──────────────────────────────────────────────────────────────────────────────────
  //
  // In GPU::memoryResource(): device memory in a CUDA or HIP build, host memory otherwise. Reserve every array
  // first and take the pool's base after: a reservation can move the pool's block.
  EBGeometry::Pool particlePool(EBGeometry::GPU::memoryResource());

  EBGeometry::PODVector<T> arrays[6];

  for (auto& a : arrays) {
    a.reserveSizedFrom(particlePool, numParticles);
  }

  void* const base = particlePool.base();

  const Particles particles{arrays[0].data(base),
                            arrays[1].data(base),
                            arrays[2].data(base),
                            arrays[3].data(base),
                            arrays[4].data(base),
                            arrays[5].data(base)};

  EBGeometry::GPU::parallelFor(numParticles, Start{particles});

  // ── The grid: built once, with room to rebuild ─────────────────────────────────────────────────────────
  //
  // build() runs where the pool's memory is, here on the device, reading the positions through FromParticles. Its
  // last argument reserves room for rebuilds of up to that many particles, so rebuild() reserves nothing: the
  // pool can be frozen, which is what makes a view of the grid safe to pass to kernels.
  EBGeometry::Pool gridPool(EBGeometry::GPU::memoryResource());

  Grid grid = Grid::build(gridPool, FromParticles{particles}, numParticles, T(1), numParticles);

  gridPool.freeze();

  // ── Step, rebuild, query ───────────────────────────────────────────────────────────────────────────────
  //
  // After the particles move, rebuild() sorts them into cells again, in place. A view taken before a rebuild
  // describes the old grid, so each step takes a new one (it copies nothing).
  std::vector<uint32_t> indices(numParticles);

  for (uint32_t i = 0; i < numParticles; i++) {
    indices[i] = i;
  }

  std::vector<T> eighth(numParticles);

  std::cout << std::fixed << std::setprecision(4) << numParticles
            << " particles (backend: " << EBGeometry::GPU::backendName
            << "); distance to each particle's 8th nearest neighbor:\n";

  for (int step = 0; step <= numSteps; step++) {
    if (step > 0) {
      EBGeometry::GPU::parallelFor(numParticles, Move{particles, timeStep});

      grid.rebuild(FromParticles{particles}, numParticles);
    }

    EBGeometry::GPU::transform(EighthNeighbor{grid.rebasedView(gridPool)}, indices, eighth);

    T lo  = eighth[0];
    T hi  = eighth[0];
    T sum = T(0);

    for (const T d : eighth) {
      lo = (d < lo) ? d : lo;
      hi = (d > hi) ? d : hi;
      sum += d;
    }

    std::cout << "  step " << step << ": mean " << sum / T(numParticles) << ", from " << lo << " to " << hi << '\n';
  }

  return 0;
}
