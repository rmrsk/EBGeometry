// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Choosing GPU::transform()'s launch settings: the block size and the threads the CUDA compiler is asked to
// keep resident on a multiprocessor, which fixes how many registers each thread may use. Each setting is a
// kernel of its own, compiled in advance, so tuning means timing a few of them on a sample of the real
// queries and running the rest with the fastest. The counterpart of HostTuning; meaningful in a GPU build
// only. See README.md.

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

// The mesh's BVH branching factor and leaf width, spelled out: a tape lists TriMeshSDF<T, 4, 4> among its
// primitive types, and a host-tuned build would otherwise read the mesh with another width.
constexpr std::size_t K = 4;
constexpr std::size_t W = 4;

// The number of query points, and how many of them each candidate setting is timed on.
constexpr std::size_t numPoints = 1000000;
constexpr std::size_t numSample = 100000;

namespace {

// ── The query ────────────────────────────────────────────────────────────────────────────────────────
//
// A tape holding a mesh with a ball cut out of it: the kind of query whose kernel uses many registers, and
// whose speed depends on how many it gets.
struct Distance
{
  EBGeometry::Tape<T> m_tape;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3& a_point) const noexcept
  {
    return m_tape.signedDistance(a_point);
  }
};

using In  = EBGeometry::PODSpan<const Vec3>;
using Out = EBGeometry::PODSpan<T>;

// ── The candidates ───────────────────────────────────────────────────────────────────────────────────
//
// GPU::transform<BlockSize, MinResidentThreads>() is a different kernel for every pair, so the candidates
// are a list of launch functions, one per pair, each compiled here. MinResidentThreads 0 lets the compiler
// pick the register budget itself; GPU::DefaultMinResidentThreads (384) holds a thread to 168 registers,
// 768 to 80. A HIP build ignores it.
template <unsigned int BlockSize, unsigned int MinResidentThreads>
void
launch(const Distance& a_query, const In a_in, const Out a_out)
{
  EBGeometry::GPU::transform<BlockSize, MinResidentThreads>(a_query, a_in, a_out);
}

struct Candidate
{
  unsigned int blockSize;
  unsigned int minResidentThreads;
  void (*run)(const Distance&, In, Out);
};

const Candidate candidates[] = {
  {64, 0, launch<64, 0>},
  {64, 384, launch<64, 384>},
  {64, 768, launch<64, 768>},
  {128, 0, launch<128, 0>},
  {128, 384, launch<128, 384>},
  {128, 768, launch<128, 768>},
  {256, 0, launch<256, 0>},
  {256, 384, launch<256, 384>},
  {256, 768, launch<256, 768>},
};

// Milliseconds a candidate takes over the inputs: the fastest of a few launches, after one to warm up.
// transform() returns once the kernel has finished, so wall time is kernel time.
double
timeLaunch(const Candidate& a_candidate, const Distance& a_query, const In a_in, const Out a_out)
{
  a_candidate.run(a_query, a_in, a_out);

  double best = 1e30;

  for (int i = 0; i < 3; i++) {
    const auto start = std::chrono::steady_clock::now();

    a_candidate.run(a_query, a_in, a_out);

    best = std::min(best, std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count());
  }

  return best;
}

} // namespace

// The function-try-block reports a mesh file that cannot be read, rather than letting the exception
// terminate the program.
int
main(int argc, char* argv[])
try {
  // ── Build the query ────────────────────────────────────────────────────────────────────────────────
  //
  // The default mesh comes from the common-3d-test-models submodule (git submodule update --init); pass
  // another on the command line. Arguments starting with -- are options, which this example has none of.
  std::string file = "../../common-3d-test-models/data/armadillo.obj";

  for (int i = 1; i < argc; i++) {
    if (std::string(argv[i]).rfind("--", 0) != 0) {
      file = argv[i];

      break;
    }
  }

  std::cout << "Reading '" << file << "'\n";

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const auto mesh = EBGeometry::Parser::readIntoTriMeshSDF<T, K, W>(file, pool);
  const auto box  = mesh.computeBoundingVolume();
  const Vec3 lo   = box.getLowCorner();
  const Vec3 hi   = box.getHighCorner();

  const EBGeometry::SphereSDF<T> hole(T(0.5) * (lo + hi), T(0.25) * (hi - lo).length());
  const EBGeometry::Tape<T>      tape(pool, EBGeometry::Difference(mesh, hole));

  // Random points in the mesh's box, which a kernel reads in the order they are stored.
  std::mt19937_64                   rng(12345);
  std::uniform_real_distribution<T> unit(T(0), T(1));
  std::vector<Vec3>                 points(numPoints);

  for (Vec3& p : points) {
    p = lo + (hi - lo) * Vec3(unit(rng), unit(rng), unit(rng));
  }

  // Launch settings mean nothing to a host loop: a host build says so, and a GPU build with no visible GPU
  // has nothing to time.
  if (!EBGeometry::GPU::enabled) {
    std::cout << "A host build runs GPU::transform() as a host loop; build it for a GPU (make GPU=CUDA, see "
                 "README.md) to tune the launch settings.\n";

    return 0;
  }

  if (!EBGeometry::GPU::deviceAvailable()) {
    std::cout << "No GPU is visible: nothing to tune.\n";

    return 0;
  }

  // ── Put the query and the points on the device ─────────────────────────────────────────────────────
  //
  // What a launch costs is the kernel alone only when nothing is allocated or copied around it, so the
  // tape's pool is mirrored once, and the points and the results go into a pool of their own, mirrored
  // the same way. The launches then run on spans of that copy.
  EBGeometry::Pool            io(EBGeometry::hostMemoryResource());
  EBGeometry::PODVector<Vec3> in;
  EBGeometry::PODVector<T>    out;
  const std::vector<T>        zeros(numPoints, T(0));

  in.reserveFrom(io, numPoints);
  out.reserveFrom(io, numPoints);
  in.assign(io.base(), points.data(), numPoints);
  out.assign(io.base(), zeros.data(), numPoints);

  pool.freeze();
  io.freeze();

  const EBGeometry::Pool deviceTape = EBGeometry::Pool::mirror(pool, EBGeometry::GPU::memoryResource());
  EBGeometry::Pool       deviceIO   = EBGeometry::Pool::mirror(io, EBGeometry::GPU::memoryResource());

  const Distance query{tape.rebasedView(deviceTape)};

  const In  all     = in.bind(static_cast<const void*>(deviceIO.base()));
  const Out results = out.bind(deviceIO.base());

  // ── Time each candidate on a sample ────────────────────────────────────────────────────────────────
  //
  // A sample of the real queries, not the whole set: tuning should cost little next to the work it tunes.
  const In  sampleIn{all.m_ptr, static_cast<std::uint32_t>(numSample)};
  const Out sampleOut{results.m_ptr, static_cast<std::uint32_t>(numSample)};

  std::cout << "\nThe tape's signed distance at " << numSample << " points (backend: " << EBGeometry::GPU::backendName
            << "):\n\n";
  std::cout << "  block size   resident threads   ms per launch\n";

  std::size_t fastest = 0;
  double      best    = 1e30;

  for (std::size_t i = 0; i < std::size(candidates); i++) {
    const double ms = timeLaunch(candidates[i], query, sampleIn, sampleOut);

    std::cout << "  " << std::setw(10) << candidates[i].blockSize << "   " << std::setw(16)
              << candidates[i].minResidentThreads << "   " << std::setw(13) << std::fixed << std::setprecision(3) << ms
              << (candidates[i].minResidentThreads == EBGeometry::GPU::DefaultMinResidentThreads &&
                      candidates[i].blockSize == EBGeometry::GPU::DefaultBlockSize
                    ? "   (the defaults)"
                    : "")
              << '\n';

    if (ms < best) {
      best    = ms;
      fastest = i;
    }
  }

  // ── Run the rest with the fastest ──────────────────────────────────────────────────────────────────
  //
  // The best setting depends on the device and on the query (and on the precision), so an application
  // that launches one query many times can tune once at startup like this, and keep the winner.
  const Candidate& chosen = candidates[fastest];

  const In  restIn{all.m_ptr + numSample, static_cast<std::uint32_t>(numPoints - numSample)};
  const Out restOut{results.m_ptr + numSample, static_cast<std::uint32_t>(numPoints - numSample)};

  const auto start = std::chrono::steady_clock::now();

  chosen.run(query, restIn, restOut);

  const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();

  std::cout << "\nThe fastest: block size " << chosen.blockSize << ", " << chosen.minResidentThreads
            << " resident threads. The other " << numPoints - numSample << " points with it: " << ms << " ms\n";

  // The results are on the device; copy them back to read a few.
  const EBGeometry::Pool back = EBGeometry::Pool::mirror(deviceIO, EBGeometry::hostMemoryResource());

  std::cout << "The first few distances:";

  for (std::uint32_t i = 0; i < 4; i++) {
    std::cout << ' ' << std::setprecision(4) << out.at(back.base(), i);
  }

  std::cout << '\n';

  return 0;
} catch (const EBGeometry::Parser::ParseError& e) {
  std::cerr << "Cannot read the mesh: " << e.what() << '\n'
            << "The default mesh is in the common-3d-test-models submodule: git submodule update --init\n";

  return 1;
}
