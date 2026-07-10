// SPDX-FileCopyrightText: 2024 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_SFCImplem.hpp
 * @brief  Implementation of EBGeometry_SFC.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_SFCIMPLEM_HPP
#define EBGEOMETRY_SFCIMPLEM_HPP

// Std includes
#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <vector>

// Our includes
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_SFC.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {
namespace SFC {

inline uint64_t
Morton::encode(const Index& a_point) noexcept
{
  EBGEOMETRY_EXPECT(a_point[0] <= ValidSpan);
  EBGEOMETRY_EXPECT(a_point[1] <= ValidSpan);
  EBGEOMETRY_EXPECT(a_point[2] <= ValidSpan);

  uint64_t code = 0;

  const uint_fast32_t x = a_point[0];
  const uint_fast32_t y = a_point[1];
  const uint_fast32_t z = a_point[2];

  auto PartBy3 = [](const uint_fast32_t a) -> uint64_t {
    uint64_t b = a & Mask_64[0];

    b = (b | b << 32) & Mask_64[1];
    b = (b | b << 16) & Mask_64[2];
    b = (b | b << 8) & Mask_64[3];
    b = (b | b << 4) & Mask_64[4];
    b = (b | b << 2) & Mask_64[5];

    return b;
  };

  code |= PartBy3(x) | PartBy3(y) << 1 | PartBy3(z) << 2;

  return code;
}

inline Index
Morton::decode(const uint64_t& a_code) noexcept
{
  EBGEOMETRY_EXPECT(a_code <= ((static_cast<uint64_t>(1) << (3 * ValidBits)) - 1));

  auto getEveryThirdBit = [](const uint64_t m) -> uint_fast32_t {
    uint_fast32_t x = m & Mask_64[5];

    x = (x ^ (x >> 2)) & Mask_64[4];
    x = (x ^ (x >> 4)) & Mask_64[3];
    x = (x ^ (x >> 8)) & Mask_64[2];
    x = (x ^ (x >> 16)) & Mask_64[1];
    x = (x ^ (x >> 32)) & Mask_64[0];

    return x;
  };

  const unsigned int x = static_cast<unsigned int>(getEveryThirdBit(a_code));
  const unsigned int y = static_cast<unsigned int>(getEveryThirdBit(a_code >> 1));
  const unsigned int z = static_cast<unsigned int>(getEveryThirdBit(a_code >> 2));

  return Index({x, y, z});
}

inline uint64_t
Nested::encode(const Index& a_point) noexcept
{
  EBGEOMETRY_EXPECT(a_point[0] <= ValidSpan);
  EBGEOMETRY_EXPECT(a_point[1] <= ValidSpan);
  EBGEOMETRY_EXPECT(a_point[2] <= ValidSpan);

  // Use base = ValidSpan + 1 = 2^ValidBits so that every coordinate in
  // [0, ValidSpan] maps to a unique code. Using ValidSpan as the base would
  // make coordinate == ValidSpan alias to coordinate 0 of the next "row".
  constexpr uint64_t base = static_cast<uint64_t>(SFC::ValidSpan) + 1ULL;

  const uint64_t x = a_point[0];
  const uint64_t y = a_point[1];
  const uint64_t z = a_point[2];

  return x + y * base + z * base * base;
}

inline Index
Nested::decode(const uint64_t& a_code) noexcept
{
  constexpr uint64_t base    = static_cast<uint64_t>(SFC::ValidSpan) + 1ULL;
  constexpr uint64_t maxCode = static_cast<uint64_t>(SFC::ValidSpan) * (1ULL + base + base * base);

  EBGEOMETRY_EXPECT(a_code <= maxCode);

  const unsigned int z = static_cast<unsigned int>(a_code / (base * base));
  const unsigned int y = static_cast<unsigned int>((a_code - z * base * base) / base);
  const unsigned int x = static_cast<unsigned int>(a_code - z * base * base - y * base);

  return Index({x, y, z});
}

template <class T>
inline std::vector<Index>
computeBins(const std::vector<Vec3T<T>>& a_points) noexcept
{
  // The space-filling curves operate on positive integer coordinates only, using up to 2^21 valid
  // bits per direction. Normalize the real-valued points into that grid.
  Vec3T<T> minCoord = +Vec3T<T>::infinity();
  Vec3T<T> maxCoord = -Vec3T<T>::infinity();

  for (const auto& p : a_points) {
    minCoord = min(minCoord, p);
    maxCoord = max(maxCoord, p);
  }

  Vec3T<T> delta = (maxCoord - minCoord) / ValidSpan;

  // A zero delta component means every point coincides on that axis (a planar cloud or duplicate
  // points). The numerator below is then also exactly zero there for every point, so any nonzero
  // divisor yields the same, correct bin index of 0; clamp to 1 only to avoid the divide-by-zero.
  for (size_t axis = 0; axis < 3; axis++) {
    if (delta[axis] == T(0)) {
      delta[axis] = T(1);
    }
  }

  std::vector<Index> bins;
  bins.reserve(a_points.size());

  for (const auto& p : a_points) {
    const Vec3T<T> curBin = (p - minCoord) / delta;

    bins.emplace_back(Index{static_cast<unsigned int>(std::floor(curBin[0])),
                            static_cast<unsigned int>(std::floor(curBin[1])),
                            static_cast<unsigned int>(std::floor(curBin[2]))});
  }

  return bins;
}

template <class Curve, class T>
inline std::vector<uint32_t>
order(const std::vector<Vec3T<T>>& a_points) noexcept
{
  const std::vector<Index> bins = computeBins<T>(a_points);

  std::vector<Code> codes;
  codes.reserve(a_points.size());
  for (const auto& bin : bins) {
    codes.push_back(Curve::encode(bin));
  }

  std::vector<uint32_t> indices(a_points.size());
  std::iota(indices.begin(), indices.end(), uint32_t(0));
  std::sort(indices.begin(), indices.end(), [&codes](uint32_t a_lhs, uint32_t a_rhs) noexcept -> bool {
    return codes[a_lhs] < codes[a_rhs];
  });

  return indices;
}

} // namespace SFC
} // namespace EBGeometry

#endif
