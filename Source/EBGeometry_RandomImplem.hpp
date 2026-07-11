// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_RandomImplem.hpp
 * @brief  Implementation of EBGeometry_Random.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_RANDOMIMPLEM_HPP
#define EBGEOMETRY_RANDOMIMPLEM_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

// Our includes
#include "EBGeometry_Random.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

namespace Random {

template <class T>
inline std::vector<Vec3T<T>>
samplePoints(size_t a_count, uint64_t a_seed)
{
  std::mt19937_64                   rng(a_seed);
  std::uniform_real_distribution<T> dist(T(0.0), T(1.0));

  std::vector<Vec3T<T>> points;

  points.reserve(a_count);

  for (size_t i = 0; i < a_count; i++) {
    points.emplace_back(dist(rng), dist(rng), dist(rng));
  }

  return points;
}

} // namespace Random

} // namespace EBGeometry

#endif
