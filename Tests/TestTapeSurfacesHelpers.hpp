// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// Test helpers shared by the surface tests (TestTapeSurfaces, TestTapeSurfacesDevice): random spheres and query
// points, the same in every build and precision.

#ifndef EBGEOMETRY_TESTTAPESURFACESHELPERS_HPP
#define EBGEOMETRY_TESTTAPESURFACESHELPERS_HPP

#include "EBGeometry.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace EBGeometryTestSurfaces {

using namespace EBGeometry;

// A small linear congruential generator: the same numbers in every build and precision.
struct Lcg
{
  uint64_t m_state = 12345U;

  double
  next() noexcept
  {
    m_state = m_state * 6364136223846793005ULL + 1442695040888963407ULL;

    return double(m_state >> 11) / double(1ULL << 53);
  }
};

// Spheres of radius 0.2 to 0.5 at random places in [0, 8]^3, in no spatial order.
template <class T>
std::vector<SphereCloud::Sphere<T>>
randomSpheres(const size_t a_count, const uint64_t a_seed)
{
  Lcg rng{a_seed};

  std::vector<SphereCloud::Sphere<T>> spheres;

  for (size_t i = 0; i < a_count; i++) {
    const Vec3T<T> center(T(8 * rng.next()), T(8 * rng.next()), T(8 * rng.next()));

    spheres.push_back(SphereCloud::Sphere<T>{center, T(0.2 + 0.3 * rng.next())});
  }

  return spheres;
}

// Points spread over [-1, 9]^3.
template <class T>
std::vector<Vec3T<T>>
queryPoints(const size_t a_count)
{
  Lcg rng{777U};

  std::vector<Vec3T<T>> points;

  for (size_t i = 0; i < a_count; i++) {
    points.emplace_back(T(10 * rng.next() - 1), T(10 * rng.next() - 1), T(10 * rng.next() - 1));
  }

  return points;
}

} // namespace EBGeometryTestSurfaces

#endif
