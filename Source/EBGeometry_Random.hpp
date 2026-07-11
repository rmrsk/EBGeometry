// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Random.hpp
 * @brief  Centralized random sampling utilities.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_RANDOM_HPP
#define EBGEOMETRY_RANDOM_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <vector>

// Our includes
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Namespace for random sampling utilities (a single, centralized home for RNG use).
 */
namespace Random {

/**
 * @brief Sample a_count uniform random points in the unit cube [0,1]^3.
 * @details Deterministic for a given seed: the same a_seed always produces the same points (a
 * std::mt19937_64 seeded with a_seed drives a std::uniform_real_distribution over [0,1)), so
 * callers get reproducible run-to-run results.
 * @tparam T Floating-point precision.
 * @param[in] a_count Number of points to sample.
 * @param[in] a_seed  RNG seed.
 * @return Vector of a_count points, each with components in [0, 1).
 */
template <class T>
[[nodiscard]] inline std::vector<Vec3T<T>>
samplePoints(size_t a_count, uint64_t a_seed);

} // namespace Random

} // namespace EBGeometry

#include "EBGeometry_RandomImplem.hpp"

#endif
