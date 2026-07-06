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
#include <climits>
#include <cstdint>

// Our includes
#include "EBGeometry_SFC.hpp"

namespace EBGeometry {
namespace SFC {

inline uint64_t
Morton::encode(const Index& a_point) noexcept
{
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
  constexpr uint64_t base = static_cast<uint64_t>(SFC::ValidSpan) + 1ULL;

  const unsigned int z = static_cast<unsigned int>(a_code / (base * base));
  const unsigned int y = static_cast<unsigned int>((a_code - z * base * base) / base);
  const unsigned int x = static_cast<unsigned int>(a_code - z * base * base - y * base);

  return Index({x, y, z});
}
} // namespace SFC
} // namespace EBGeometry

#endif
