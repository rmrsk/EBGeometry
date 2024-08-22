/* EBGeometry
 * Copyright © 2024 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_SFCImplem.hpp
  @brief  Implementation of EBGeometry_SFC.hpp
  @author Robert Marskar
*/

#ifndef _EBGeometry_SFCImplem_
#define _EBGeometry_SFCImplem_

// Std includes
#include <climits>

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
        uint64_t x = a & Mask_64[0];

        x = (x | x << 32) & Mask_64[1];
        x = (x | x << 16) & Mask_64[2];
        x = (x | x << 8) & Mask_64[3];
        x = (x | x << 4) & Mask_64[4];
        x = (x | x << 2) & Mask_64[5];

        return x;
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

      const unsigned int x = (unsigned int)getEveryThirdBit(a_code);
      const unsigned int y = (unsigned int)getEveryThirdBit(a_code >> 1);
      const unsigned int z = (unsigned int)getEveryThirdBit(a_code >> 2);

      return Index({x, y, z});
    }

    inline uint64_t
    Nested::encode(const Index& a_point) noexcept
    {
      uint64_t code = 0;

      const uint32_t x = a_point[0];
      const uint32_t y = a_point[1];
      const uint32_t z = a_point[2];

      code = x + (y * SFC::ValidSpan) + (z * SFC::ValidSpan * SFC::ValidSpan);

      return code;
    }

    inline Index
    Nested::decode(const uint64_t& a_code) noexcept
    {
      const unsigned int z = a_code / (SFC::ValidSpan * SFC::ValidSpan);
      const unsigned int y = (a_code - z * SFC::ValidSpan * SFC::ValidSpan) / SFC::ValidSpan;
      const unsigned int x = a_code - z * SFC::ValidSpan * SFC::ValidSpan - y * SFC::ValidSpan;

      return Index({x, y, z});
    }
  } // namespace SFC
} // namespace EBGeometry

#endif
