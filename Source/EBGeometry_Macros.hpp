// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Macros.hpp
 * @brief  Utility macros for EBGeometry.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_MACROS_HPP
#define EBGEOMETRY_MACROS_HPP

#include <cstdio>
#include <cstdlib>

#include "EBGeometry_GPU.hpp"

/**
 * @brief Runtime precondition assertion for EBGeometry.
 *
 * @details When @c EBGEOMETRY_ENABLE_ASSERTIONS is defined (e.g. via
 * @c -DEBGEOMETRY_ENABLE_ASSERTIONS on the compiler command line,
 * in @c CXXFLAGS for GNU Make, or via CMake's
 * @c target_compile_definitions), this macro evaluates @a cond and,
 * if it is false, prints a diagnostic message to @c stderr and calls
 * @c std::abort().
 *
 * When @c EBGEOMETRY_ENABLE_ASSERTIONS is not defined the macro expands
 * to @c ((void)0): @a cond is not evaluated at all, so an assertion costs
 * exactly nothing in release. A variable computed solely to be asserted is
 * then unused and must be marked @c [[maybe_unused]] at its declaration.
 * On a device (CUDA/HIP) compilation pass with assertions enabled the macro
 * uses the device-capable @c assert() instead of @c std::fprintf / @c std::abort.
 *
 * @par Enabling assertions
 * @code{.cmake}
 * # CMake
 * target_compile_definitions(MyTarget PRIVATE EBGEOMETRY_ENABLE_ASSERTIONS)
 * @endcode
 * @code{.makefile}
 * # GNU Make
 * CXXFLAGS += -DEBGEOMETRY_ENABLE_ASSERTIONS
 * @endcode
 * @code{.sh}
 * # Command line
 * g++ -DEBGEOMETRY_ENABLE_ASSERTIONS ...
 * @endcode
 *
 * @param cond  Boolean-convertible expression to test.
 */
#if defined(EBGEOMETRY_ENABLE_ASSERTIONS)
#if defined(EBGEOMETRY_DEVICE_COMPILE)
// Device compilation pass (CUDA/HIP): std::fprintf/std::abort are host-only, so fall back to the
// device-capable assert(). It still aborts the kernel on failure and prints the failing expression.
#include <cassert>
#define EBGEOMETRY_EXPECT(cond) assert(cond)
#else
#define EBGEOMETRY_EXPECT(cond)                                                                   \
  do {                                                                                            \
    if (!(cond)) {                                                                                \
      std::fprintf(stderr,                                                                        \
                   "EBGeometry assertion failed: (%s)\n  file: %s\n  line: %d\n  function: %s\n", \
                   #cond,                                                                         \
                   __FILE__,                                                                      \
                   __LINE__,                                                                      \
                   static_cast<const char*>(__func__));                                           \
      std::abort();                                                                               \
    }                                                                                             \
  } while (0)
#endif
#else
// Assertions off: expand to a no-op that does NOT evaluate cond -- an assertion costs exactly
// nothing in release, including not running its predicate. A local computed solely to be asserted
// therefore becomes unused; mark such a declaration [[maybe_unused]] at its site.
#define EBGEOMETRY_EXPECT(cond) ((void)0)
#endif

#endif // EBGEOMETRY_MACROS_HPP
