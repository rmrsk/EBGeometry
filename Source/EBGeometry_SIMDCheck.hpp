// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_SIMDCheck.hpp
 * @brief  Startup check that the translation units using the SIMD code agree on their SIMD settings.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_SIMDCHECK_HPP
#define EBGEOMETRY_SIMDCHECK_HPP

// Std includes
#include <cstdio>
#include <cstdlib>

// Our includes
#include "EBGeometry_GPU.hpp"

namespace EBGeometry {

/**
 * @brief Startup check that the SIMD settings agree across translation units.
 * @details EBGEOMETRY_HOST_TUNED_DEFAULTS changes the default triangle-group width, and with it
 * defined that width follows the SIMD flags, so two translation units that disagree on the macro, or
 * that both define it but differ in SIMD flags, give one spelled type two layouts. Every translation
 * unit that includes the SIMD headers records its settings before main(), and one that disagrees in
 * that way aborts, naming both files.
 *
 * The SIMD flags themselves (__SSE4_1__, __AVX__, __AVX512F__) choose the inline bodies of
 * PackedBVH::computeChildDistances2(), TriangleSoA and PointSoA, but not the layout of any type
 * spelled with the default widths. Two translation units compiled with different flags define those
 * inline functions two ways, and the linker keeps one: the program can run instructions only the
 * other unit was compiled for, or get the other unit's results (the SIMD and scalar paths agree for
 * finite inputs). That prints one warning, naming both files. A build that mixes the flags on purpose
 * can define EBGEOMETRY_ALLOW_MIXED_SIMD in those translation units to silence it; a layout mismatch
 * still aborts.
 */
namespace SIMDCheckDetail {

/**
 * @brief The settings this translation unit was compiled with, one bit each.
 * @details Internal linkage on purpose: an inline variable with a different value per translation
 * unit would itself be the violation this header detects.
 */
static constexpr unsigned CompiledSettings = 0u
#if defined(__SSE4_1__)
                                             | 1u
#endif
#if defined(__AVX__)
                                             | 2u
#endif
#if defined(__AVX512F__)
                                             | 4u
#endif
#if defined(EBGEOMETRY_HOST_TUNED_DEFAULTS)
                                             | 8u
#endif
  ;

/**
 * @brief Print the settings in a_settings to stderr.
 * @param[in] a_settings Bits as in CompiledSettings.
 */
inline void
printSettings(const unsigned a_settings) noexcept
{
  if (a_settings == 0u) {
    std::fprintf(stderr, "none");
  }

  const char* const names[] = {"SSE4.1", "AVX", "AVX-512F", "EBGEOMETRY_HOST_TUNED_DEFAULTS"};

  const char* separator = "";

  for (unsigned bit = 0; bit < 4; bit++) {
    if ((a_settings & (1u << bit)) != 0u) {
      std::fprintf(stderr, "%s%s", separator, names[bit]);

      separator = " ";
    }
  }
}

/**
 * @brief Bit of CompiledSettings that changes type layouts.
 */
static constexpr unsigned LayoutSettings = 8u;

/**
 * @brief Print the two units' settings and file names to stderr.
 * @param[in] a_firstSettings Settings of the first unit recorded.
 * @param[in] a_firstUnit     File name of the first unit recorded.
 * @param[in] a_settings      Settings of this unit.
 * @param[in] a_unit          File name of this unit.
 */
inline void
printUnits(const unsigned a_firstSettings,
           const char*    a_firstUnit,
           const unsigned a_settings,
           const char*    a_unit) noexcept
{
  std::fprintf(stderr, "  ");
  printSettings(a_firstSettings);
  std::fprintf(stderr, ": %s\n  ", a_firstUnit);
  printSettings(a_settings);
  std::fprintf(stderr, ": %s\n", a_unit);
}

/**
 * @brief Record one translation unit's settings and compare them with the first unit's.
 * @details Aborts if the two disagree on EBGEOMETRY_HOST_TUNED_DEFAULTS, or both define it and
 * differ in SIMD flags. Otherwise prints one warning, the first time two units differ in SIMD flags,
 * unless either defines EBGEOMETRY_ALLOW_MIXED_SIMD.
 * @param[in] a_settings   Bits as in CompiledSettings.
 * @param[in] a_allowMixed True if this unit defines EBGEOMETRY_ALLOW_MIXED_SIMD.
 * @param[in] a_unit       The translation unit's file name, for the messages.
 * @return True.
 */
inline bool
recordSettings(const unsigned a_settings, const bool a_allowMixed, const char* a_unit) noexcept
{
  static const unsigned    s_firstSettings   = a_settings;
  static const bool        s_firstAllowMixed = a_allowMixed;
  static const char* const s_firstUnit       = a_unit;
  static bool              s_warned          = false;

  // Under the host-tuned defaults the default triangle-group width follows the SIMD flags, so then
  // any difference changes a layout.
  const bool layoutDiffers = (a_settings & LayoutSettings) != (s_firstSettings & LayoutSettings) ||
                             ((a_settings & LayoutSettings) != 0u && a_settings != s_firstSettings);

  if (layoutDiffers) {
    std::fprintf(stderr, "EBGeometry: translation units disagree on the settings that set type layouts\n");
    printUnits(s_firstSettings, s_firstUnit, a_settings, a_unit);
    std::fprintf(stderr,
                 "The default triangle-group width, and so the layout of every type spelled with it, then "
                 "differs between them. Define EBGEOMETRY_HOST_TUNED_DEFAULTS in every translation unit or "
                 "in none, and with it compile them all with the same SIMD flags.\n");
    std::abort();
  }

  if (a_settings != s_firstSettings && !a_allowMixed && !s_firstAllowMixed && !s_warned) {
    s_warned = true;

    std::fprintf(stderr, "EBGeometry warning: translation units were compiled with different SIMD flags\n");
    printUnits(s_firstSettings, s_firstUnit, a_settings, a_unit);
    std::fprintf(stderr,
                 "The linker keeps one unit's SIMD code for both. Compile them with the same flags, or define "
                 "EBGEOMETRY_ALLOW_MIXED_SIMD in them to silence this warning.\n");
  }

  return true;
}

#if defined(EBGEOMETRY_ALLOW_MIXED_SIMD)
/**
 * @brief True if this translation unit defines EBGEOMETRY_ALLOW_MIXED_SIMD.
 */
static constexpr bool AllowMixed = true;
#else
/**
 * @brief True if this translation unit defines EBGEOMETRY_ALLOW_MIXED_SIMD.
 */
static constexpr bool AllowMixed = false;
#endif

// One per translation unit, before main(). Host passes only: a device pass has no SIMD code.
#if !defined(EBGEOMETRY_DEVICE_COMPILE)
#if defined(__BASE_FILE__)
[[maybe_unused]] static const bool s_settingsRecorded = recordSettings(CompiledSettings, AllowMixed, __BASE_FILE__);
#else
[[maybe_unused]] static const bool s_settingsRecorded = recordSettings(CompiledSettings, AllowMixed, __FILE__);
#endif
#endif

} // namespace SIMDCheckDetail

} // namespace EBGeometry

#endif
