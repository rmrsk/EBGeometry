// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// Helpers shared by several test files: paths to the test data, comparison margins, query point sets,
// brute-force references and small scenes. Not part of the library itself -- test infrastructure only. A
// helper used by one file stays in that file; one used by the files of one component goes in that
// component's Test<Component>Helpers.hpp; one used across components goes here, once.

#ifndef EBGEOMETRY_TEST_UTILITIES_HPP
#define EBGEOMETRY_TEST_UTILITIES_HPP

#include "EBGeometry.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <random>
#include <string>
#include <type_traits>
#include <vector>

// ── Test data ────────────────────────────────────────────────────────────────

// The path of a file in Tests/data.
inline std::string
dataPath(const std::string& a_filename)
{
  return std::string(EBGEOMETRY_TEST_DATA_DIR) + "/" + a_filename;
}

// The path of the regular dodecahedron (20 vertices, 12 pentagons cut into 36 triangles, watertight and
// orientable), in STL.
inline std::string
dodecahedron()
{
  return dataPath("dodecahedron.stl");
}

// ── Margins ──────────────────────────────────────────────────────────────────

// A margin for a hand-computed closed form or a partition-of-unity sum: a handful more floating-point
// operations than the exact comparisons of TestFloatingPointUtils.hpp.
template <class T>
double
formulaMargin()
{
  return std::is_same_v<T, float> ? 1.0e-3 : 1.0e-9;
}

// A margin for comparing a brute-force scan against a BVH or SIMD traversal (or two build strategies),
// which chain many more floating-point operations across a mesh's faces.
template <class T>
double
traversalMargin()
{
  return std::is_same_v<T, float> ? 1.0e-2 : 1.0e-6;
}

// ── Point sets ───────────────────────────────────────────────────────────────

// a_count + 1 evenly spaced values from a_lo to a_hi inclusive, for sweeping a query grid with an integer
// counter rather than a floating-point one.
template <class T>
std::vector<T>
sweepValues(const T a_lo, const T a_hi, const int a_count)
{
  std::vector<T> values;
  values.reserve(static_cast<size_t>(a_count) + 1);

  for (int i = 0; i <= a_count; i++) {
    values.push_back(a_lo + (a_hi - a_lo) * T(i) / T(a_count));
  }

  return values;
}

// Points on an a_n^3 grid filling a cube of half-side a_half about the origin, off its symmetry planes.
template <class T>
std::vector<EBGeometry::Vec3T<T>>
gridPoints(const T a_half, const int a_n)
{
  std::vector<EBGeometry::Vec3T<T>> points;

  for (int i = 0; i < a_n; i++) {
    for (int j = 0; j < a_n; j++) {
      for (int k = 0; k < a_n; k++) {
        const auto at = [&](const int a_i) { return -a_half + T(2) * a_half * (T(a_i) + T(0.37)) / T(a_n); };

        points.emplace_back(at(i), at(j), at(k));
      }
    }
  }

  return points;
}

// Random points in the cube [-a_extent, a_extent]^3, for comparing two shapes that should agree.
template <class T>
std::vector<EBGeometry::Vec3T<T>>
randomPoints(const T a_extent, const int a_count, const unsigned a_seed = 2468)
{
  std::mt19937                      rng(a_seed);
  std::uniform_real_distribution<T> coord(-a_extent, a_extent);
  std::vector<EBGeometry::Vec3T<T>> points;

  for (int i = 0; i < a_count; i++) {
    points.emplace_back(coord(rng), coord(rng), coord(rng));
  }

  return points;
}

// Four disjoint, well-separated positions, spread out along x so each one is unambiguously the closest for a
// query point placed near it.
template <class T>
std::vector<EBGeometry::Vec3T<T>>
fourPositions()
{
  std::vector<EBGeometry::Vec3T<T>> positions;

  for (int i = 0; i < 4; i++) {
    positions.emplace_back(T(3.0 * i), T(0), T(0));
  }

  return positions;
}

// Query points for fourPositions(): one near each position, and two far from all of them.
template <class T>
std::vector<EBGeometry::Vec3T<T>>
fourPositionsQueries()
{
  using Vec3 = EBGeometry::Vec3T<T>;

  return {
    Vec3(0.1, 0.1, 0.1),
    Vec3(3.2, -0.1, 0.05),
    Vec3(6.0, 0.3, 0),
    Vec3(9.1, 0, 0),
    Vec3(-5, -5, -5),
    Vec3(20, 20, 20),
  };
}

// ── References ───────────────────────────────────────────────────────────────

// The signed distance from a point to one sphere's surface, computed without the library.
template <class T>
T
sphereDistance(const EBGeometry::SphereCloud::Sphere<T>& a_sphere, const EBGeometry::Vec3T<T>& a_point)
{
  const EBGeometry::Vec3T<T> d = a_point - a_sphere.m_center;

  return std::sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]) - a_sphere.m_radius;
}

// The smallest signed distance over every sphere, by brute force.
template <class T>
T
bruteForceDistance(const std::vector<EBGeometry::SphereCloud::Sphere<T>>& a_spheres,
                   const EBGeometry::Vec3T<T>&                            a_point)
{
  T best = std::numeric_limits<T>::max();

  for (const auto& sphere : a_spheres) {
    best = std::min(best, sphereDistance(sphere, a_point));
  }

  return best;
}

// Memory a build through GPU::parallelFor() writes and the host reads: managed memory in a CUDA or HIP build,
// where the build's steps run as kernels, and emulated device memory otherwise, where they run as host loops.
inline EBGeometry::MemoryResource&
parallelBuildResource() noexcept
{
#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  return EBGeometry::managedMemoryResource();
#else
  return EBGeometry::emulatedDeviceMemoryResource();
#endif
}

// ── Scenes ───────────────────────────────────────────────────────────────────

// A cable: a copper core inside a PTFE jacket; the outside, region 0, has no material.
template <class T>
EBGeometry::Expr<T>
cable()
{
  using namespace EBGeometry;

  const auto core   = MaterialLabel(SphereSDF<T>(Vec3T<T>::zeros(), T(0.5)), "copper");
  const auto jacket = MaterialLabel(SphereSDF<T>(Vec3T<T>::zeros(), T(1)), "PTFE");

  return Overlay(jacket, core);
}

#endif // EBGEOMETRY_TEST_UTILITIES_HPP
