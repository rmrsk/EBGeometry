// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

// A domain-decomposed particle code under MPI. The domain is a box around an object, cut into slabs along x, one
// per rank; each rank owns the particles in its slab. Every rank holds the whole object (a TriMeshSDF it reads
// itself), so a particle's path is traced against the object on the rank that owns the particle, with no
// communication. Each step moves the particles, reflects them off the object where their path first crosses its
// surface (firstHit() on the segment from x to x + v dt) and off the box's walls, and then sends the particles that
// left a rank's slab to the rank that owns them now (MPI_Alltoallv). That migration is the only communication.
// See README.md.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>

// The C bindings of MPI only: the C++ bindings, removed from the standard, are not needed and do not compile cleanly
// with -Wextra.
#define OMPI_SKIP_MPICXX 1
#define MPICH_SKIP_MPICXX 1
#include <mpi.h>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float) or make (PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T    = EBGEOMETRY_PRECISION;
using Vec3 = EBGeometry::Vec3T<T>;
using Hit  = EBGeometry::HitT<T>;

// The mesh SDF's BVH branching factor and triangles per leaf group, spelled out.
constexpr std::size_t K = 4;
constexpr std::size_t W = 4;

using Mesh = EBGeometry::TriMeshSDF<T, K, W>;

// The run: particles seeded per rank, steps, and how often rank 0 prints. A particle moves stepLength (as a fraction
// of the box's size) per step, and bounces at most maxBounces times in one step.
constexpr std::size_t particlesPerRank = 2000;
constexpr int         numSteps         = 200;
constexpr int         printEvery       = 50;
constexpr T           stepLength       = T(0.02);
constexpr int         maxBounces       = 8;

namespace {

// A particle is trivially copyable, so the migration sends it as bytes.
struct Particle
{
  Vec3 x;
  Vec3 v;
};

// The slab of the box a rank owns: the x-range [lo + rank * width, lo + (rank + 1) * width).
int
ownerOf(const Vec3& a_x, const T a_lo, const T a_width, const int a_numRanks) noexcept
{
  const int slab = static_cast<int>(std::floor((a_x[0] - a_lo) / a_width));

  return std::clamp(slab, 0, a_numRanks - 1);
}

// The specular reflection of a direction off a surface with unit normal n.
Vec3
reflect(const Vec3& a_v, const Vec3& a_n) noexcept
{
  return a_v - T(2) * dot(a_v, a_n) * a_n;
}

// Moves a particle a time dt (one unit) along its path, reflecting it off the object's surface wherever the path
// crosses it. The trace is the library's segment query: the first crossing of the surface between x and x + v dt.
// After a bounce the particle sits on the surface, so the next trace starts a little off it, at offset. Returns the
// number of bounces.
int
move(Particle& a_particle, const Mesh& a_mesh, const T a_offset) noexcept
{
  T   remaining = a_particle.v.length();
  int bounces   = 0;

  while (remaining > T(0) && bounces < maxBounces) {
    const Vec3 direction = a_particle.v / a_particle.v.length();
    const Vec3 to        = a_particle.x + remaining * direction;
    const Hit  hit       = EBGeometry::firstHit(a_mesh, a_particle.x, to, bounces > 0 ? a_offset : T(0));

    if (!hit.hit()) {
      a_particle.x = to;

      break;
    }

    // Reflect at the crossing, and go on with what is left of the step.
    a_particle.x = hit.m_point + a_offset * hit.m_normal;
    a_particle.v = reflect(a_particle.v, hit.m_normal);
    remaining -= hit.m_t;
    bounces++;
  }

  return bounces;
}

// Reflects a particle off the walls of the box [lo, hi].
void
reflectOffWalls(Particle& a_particle, const Vec3& a_lo, const Vec3& a_hi) noexcept
{
  for (std::size_t dir = 0; dir < 3; dir++) {
    if (a_particle.x[dir] < a_lo[dir]) {
      a_particle.x[dir] = T(2) * a_lo[dir] - a_particle.x[dir];
      a_particle.v[dir] = -a_particle.v[dir];
    }
    else if (a_particle.x[dir] > a_hi[dir]) {
      a_particle.x[dir] = T(2) * a_hi[dir] - a_particle.x[dir];
      a_particle.v[dir] = -a_particle.v[dir];
    }
  }
}

// Sends every particle to the rank whose slab holds it, and returns the particles this rank owns afterwards. Each
// rank first tells every other how many bytes it sends it (MPI_Alltoall), then the particles go in one
// MPI_Alltoallv.
std::vector<Particle>
migrate(const std::vector<Particle>& a_particles, const T a_lo, const T a_width, const int a_numRanks)
{
  std::vector<std::vector<Particle>> outgoing(static_cast<std::size_t>(a_numRanks));

  for (const Particle& p : a_particles) {
    outgoing[static_cast<std::size_t>(ownerOf(p.x, a_lo, a_width, a_numRanks))].push_back(p);
  }

  std::vector<int>      sendCounts(static_cast<std::size_t>(a_numRanks));
  std::vector<int>      sendDispls(static_cast<std::size_t>(a_numRanks));
  std::vector<int>      recvCounts(static_cast<std::size_t>(a_numRanks));
  std::vector<int>      recvDispls(static_cast<std::size_t>(a_numRanks));
  std::vector<Particle> sendBuffer;

  for (std::size_t r = 0; r < outgoing.size(); r++) {
    sendDispls[r] = static_cast<int>(sendBuffer.size() * sizeof(Particle));
    sendCounts[r] = static_cast<int>(outgoing[r].size() * sizeof(Particle));

    sendBuffer.insert(sendBuffer.end(), outgoing[r].begin(), outgoing[r].end());
  }

  MPI_Alltoall(sendCounts.data(), 1, MPI_INT, recvCounts.data(), 1, MPI_INT, MPI_COMM_WORLD);

  int recvBytes = 0;

  for (std::size_t r = 0; r < recvCounts.size(); r++) {
    recvDispls[r] = recvBytes;
    recvBytes += recvCounts[r];
  }

  std::vector<Particle> received(static_cast<std::size_t>(recvBytes) / sizeof(Particle));

  MPI_Alltoallv(sendBuffer.data(),
                sendCounts.data(),
                sendDispls.data(),
                MPI_BYTE,
                received.data(),
                recvCounts.data(),
                recvDispls.data(),
                MPI_BYTE,
                MPI_COMM_WORLD);

  return received;
}

// The sum of a count over all ranks.
long long
sumOverRanks(const long long a_count)
{
  long long sum = 0;

  MPI_Allreduce(&a_count, &sum, 1, MPI_LONG_LONG, MPI_SUM, MPI_COMM_WORLD);

  return sum;
}

int
run(int argc, char* argv[])
{
  int rank     = 0;
  int numRanks = 1;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &numRanks);

  // ── The geometry, on every rank ──────────────────────────────────────────────────────────────────────
  //
  // Every rank reads the mesh into a TriMeshSDF of its own, in a pool of its own that outlives it. Holding the whole
  // object is what lets a rank trace any of its particles without asking another rank (SharedGeometry shows how to
  // build it once and share one copy per node instead).
  const std::string file = argc >= 2 ? std::string(argv[1]) : "../../../Examples/data/dodecahedron.stl";

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const Mesh mesh = EBGeometry::Parser::readIntoTriMeshSDF<T, K, W>(file, pool);

  // The box the particles fly in: the object's box, grown by its own size on every side.
  const auto box    = mesh.computeBoundingVolume();
  const Vec3 size   = box.getHighCorner() - box.getLowCorner();
  const Vec3 lo     = box.getLowCorner() - size;
  const Vec3 hi     = box.getHighCorner() + size;
  const T    width  = (hi[0] - lo[0]) / T(numRanks);
  const T    step   = stepLength * (hi - lo).length();
  const T    offset = T(1e-4) * size.length();

  if (rank == 0) {
    std::cout << "Read " << file << " on every rank; the box " << lo << " to " << hi << " is cut along x into one slab"
              << " per rank (" << numRanks << ")\n";
  }

  // ── Seed the particles ───────────────────────────────────────────────────────────────────────────────
  //
  // Each rank seeds its own particles in its slab, outside the object, with random directions.
  std::mt19937_64                   rng(12345U + static_cast<std::uint64_t>(rank));
  std::uniform_real_distribution<T> unit(T(0), T(1));
  std::normal_distribution<T>       normal(T(0), T(1));
  std::vector<Particle>             particles;

  while (particles.size() < particlesPerRank) {
    const Vec3 x(
      lo[0] + (T(rank) + unit(rng)) * width, lo[1] + unit(rng) * (hi[1] - lo[1]), lo[2] + unit(rng) * (hi[2] - lo[2]));

    if (mesh.signedDistance(x) > T(0)) {
      const Vec3 d(normal(rng), normal(rng), normal(rng));

      particles.push_back(Particle{x, step * d / d.length()});
    }
  }

  const long long numParticles = sumOverRanks(static_cast<long long>(particles.size()));

  // ── Step ─────────────────────────────────────────────────────────────────────────────────────────────
  //
  // Move every particle this rank owns, reflecting it off the object and the walls, then migrate. Only the
  // migration communicates; rank 0 prints the totals now and then.
  long long bounces  = 0;
  long long migrated = 0;

  for (int n = 1; n <= numSteps; n++) {
    for (Particle& p : particles) {
      bounces += move(p, mesh, offset);

      reflectOffWalls(p, lo, hi);

      if (ownerOf(p.x, lo[0], width, numRanks) != rank) {
        migrated++;
      }
    }

    particles = migrate(particles, lo[0], width, numRanks);

    if (n % printEvery == 0) {
      const long long total            = sumOverRanks(static_cast<long long>(particles.size()));
      const long long bouncesOverRanks = sumOverRanks(bounces);
      const long long migratedOverRank = sumOverRanks(migrated);

      if (rank == 0) {
        std::cout << "Step " << n << ": " << total << " particles, " << bouncesOverRanks
                  << " bounces off the object and " << migratedOverRank << " migrations so far\n";
      }
    }
  }

  // ── After the run ────────────────────────────────────────────────────────────────────────────────────
  //
  // Particles bounce off the object, so none should be inside it; and the migration loses none.
  long long inside = 0;

  for (const Particle& p : particles) {
    if (mesh.signedDistance(p.x) < T(0)) {
      inside++;
    }
  }

  const long long total       = sumOverRanks(static_cast<long long>(particles.size()));
  const long long insideTotal = sumOverRanks(inside);

  if (rank == 0) {
    std::cout << "After " << numSteps << " steps: " << total << " of " << numParticles << " particles, " << insideTotal
              << " inside the object\n";
  }

  return total == numParticles ? 0 : 1;
}

} // namespace

int
main(int argc, char* argv[])
{
  MPI_Init(&argc, &argv);

  int status = 0;

  // A mesh file that cannot be read is reported, and the run stops on every rank.
  try {
    status = run(argc, argv);
  } catch (const EBGeometry::Parser::ParseError& e) {
    std::cerr << "Cannot read the mesh: " << e.what() << '\n';

    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  MPI_Finalize();

  return status;
}
