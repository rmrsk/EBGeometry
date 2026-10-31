// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Choosing the BVH branching factor K and the SIMD width W: the portable defaults, which any code can
// use, against the values tuned to the SIMD flags of this build, which only host-only code should. The
// timings show whether host tuning pays off on this machine. Host-only by design: device code always
// uses the defaults. See README.md.

#include <chrono>
#include <cstddef>
#include <iostream>
#include <random>
#include <string>
#include <type_traits>
#include <vector>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T    = EBGEOMETRY_PRECISION;
using Vec3 = EBGeometry::Vec3T<T>;

// Two choices of the BVH branching factor K and the SIMD width W (the number of triangles or points
// evaluated together in one leaf group).
//
// The defaults are 4 for both float and double, whatever the compiler flags. A type spelled with them
// is therefore the same type everywhere: in a file built with -mavx and in one built without, and in
// both passes of a CUDA or HIP compile. That is what lets an object built on the host be copied to a
// GPU and used there. Use them for anything that device code also uses, or that is passed between
// files built with different flags. The one exception: a host-only build that defines
// EBGEOMETRY_HOST_TUNED_DEFAULTS (EBGeometry's own CMake build does) makes the default triangle width
// the host-tuned one, so there the two mesh columns below differ in K only.
constexpr std::size_t DefaultK = EBGeometry::BVH::DefaultBranchingRatio<T>();
constexpr std::size_t DefaultW = EBGeometry::TriangleSoA::DefaultWidth<T>();

// The host-tuned values fill one SIMD register under the flags this file is compiled with (this
// example is built with -march=native). They are opt-in, for code that only ever runs on the host
// and is compiled with one set of flags.
constexpr std::size_t HostK      = EBGeometry::BVH::HostBranchingRatio<T>();
constexpr std::size_t HostW      = EBGeometry::TriangleSoA::HostWidth<T>();
constexpr std::size_t HostPointW = EBGeometry::PointSoA::HostWidth<T>();

// Leaving K and W out gives the defaults.
static_assert(std::is_same_v<EBGeometry::PointCloudBVH<T>,
                             EBGeometry::PointCloudBVH<T, DefaultK, EBGeometry::PointSoA::DefaultWidth<T>()>>);

// Runs a_query over every point and returns the elapsed time in seconds; a_results receives the
// result for each point.
template <class F>
double
timeQueries(const std::vector<Vec3>& a_points, F&& a_query, std::vector<T>& a_results)
{
  a_results.resize(a_points.size());

  const auto start = std::chrono::steady_clock::now();

  for (std::size_t i = 0; i < a_points.size(); i++) {
    a_results[i] = a_query(a_points[i]);
  }

  return std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
}

// The function-try-block reports a mesh file that cannot be read, rather than letting the exception
// terminate the program.
int
main(int argc, char* argv[])
try {
  std::cout << "Precision: " << (sizeof(T) == 4 ? "float" : "double") << "\n"
            << "Default  K = " << DefaultK << ", W = " << DefaultW << "\n"
            << "Host     K = " << HostK << ", W = " << HostW << " (tuned to this build's SIMD flags)\n";
#if defined(EBGEOMETRY_HOST_TUNED_DEFAULTS)
  std::cout << "This build defines EBGEOMETRY_HOST_TUNED_DEFAULTS, so the default triangle width is the host-tuned\n"
            << "one. Build this example on its own (make, or a direct compile) to compare with the portable 4.\n";
#endif
  std::cout << "\n";

  std::mt19937                      rng(42);
  std::uniform_real_distribution<T> unit(T(0), T(1));

  const auto randomPoint = [&](const T a_lo, const T a_hi) {
    return Vec3(a_lo + (a_hi - a_lo) * unit(rng), a_lo + (a_hi - a_lo) * unit(rng), a_lo + (a_hi - a_lo) * unit(rng));
  };

  // ── A point cloud: closest-point queries, built both ways ─────────────────────────────────────────
  {
    constexpr std::size_t numPoints = 200000;

    std::vector<Vec3> points;

    points.reserve(numPoints);

    for (std::size_t i = 0; i < numPoints; i++) {
      points.emplace_back(randomPoint(T(0), T(1)));
    }

    std::vector<Vec3> queries;

    queries.reserve(numPoints);

    for (std::size_t i = 0; i < numPoints; i++) {
      queries.emplace_back(randomPoint(T(0), T(1)));
    }

    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const EBGeometry::PointCloudBVH<T>                    portable(pool, points);
    const EBGeometry::PointCloudBVH<T, HostK, HostPointW> tuned(pool, points);

    std::vector<T> portableResults;
    std::vector<T> tunedResults;

    const double portableTime =
      timeQueries(queries, [&](const Vec3& p) { return portable.closestPoint(p).distanceSquared; }, portableResults);
    const double tunedTime =
      timeQueries(queries, [&](const Vec3& p) { return tuned.closestPoint(p).distanceSquared; }, tunedResults);

    std::cout << "PointCloudBVH, " << numPoints << " points and queries\n"
              << "  default K/W: " << portableTime << " s\n"
              << "  host K/W:    " << tunedTime << " s\n"
              << "  squared distance to the closest point from the first query: " << portableResults[0] << " and "
              << tunedResults[0] << "\n\n";
  }

  // ── A triangle mesh: signed distance queries, built both ways ─────────────────────────────────────
  {
    // Pass a mesh on the command line, or use the small dodecahedron in Examples/data. The path is
    // relative to this example's folder, which is the working directory when it is run.
    const std::string file = (argc >= 2) ? std::string(argv[1]) : std::string("../data/dodecahedron.stl");

    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const auto portable = EBGeometry::Parser::readIntoTriMeshSDF<T>(file, pool);
    const auto tuned    = EBGeometry::Parser::readIntoTriMeshSDF<T, HostK, HostW>(file, pool);

    const auto box = portable.computeBoundingVolume();

    std::vector<Vec3> queries;

    queries.reserve(200000);

    for (std::size_t i = 0; i < 200000; i++) {
      const Vec3 u = randomPoint(T(-0.25), T(1.25));

      queries.emplace_back(box.getLowCorner() + u * (box.getHighCorner() - box.getLowCorner()));
    }

    // The two differ only in how the BVH groups the triangles, so they give the same distances, to
    // rounding; only the time differs.
    std::vector<T> portableResults;
    std::vector<T> tunedResults;

    const double portableTime =
      timeQueries(queries, [&](const Vec3& p) { return portable.signedDistance(p); }, portableResults);
    const double tunedTime = timeQueries(queries, [&](const Vec3& p) { return tuned.signedDistance(p); }, tunedResults);

    std::cout << "TriMeshSDF, " << file << ", " << queries.size() << " queries\n"
              << "  default K/W: " << portableTime << " s\n"
              << "  host K/W:    " << tunedTime << " s\n"
              << "  signed distance at the first query: " << portableResults[0] << " and " << tunedResults[0] << "\n";
  }

  return 0;
} catch (const EBGeometry::Parser::ParseError& e) {
  std::cerr << "Cannot read the mesh: " << e.what() << '\n';

  return 1;
}
