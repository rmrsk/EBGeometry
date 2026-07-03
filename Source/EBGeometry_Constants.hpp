// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  @file   EBGeometry_Constants.hpp
  @brief  Mathematical constants for EBGeometry.
  @author Robert Marskar
*/

#ifndef EBGEOMETRY_CONSTANTS_HPP
#define EBGEOMETRY_CONSTANTS_HPP

namespace EBGeometry {

  /**
    @brief The mathematical constant π, typed as T.
    @details Defined to 20 significant digits, which covers the full precision of both
    @c float (7 significant digits) and @c long double (18--21 significant digits).
    @tparam T Floating-point type.
  */
  template <class T>
  inline constexpr T pi = T(3.14159265358979323846L);

} // namespace EBGeometry

#endif
