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
 * prints the failing expression with the device @c printf and traps, instead of
 * @c std::fprintf / @c std::abort. Unlike @c assert(), this does not depend on
 * @c NDEBUG, which Release configurations define.
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
#if defined(EBGEOMETRY_HIP)
// Declares the device printf used below. It must be visible before the first header that uses the
// macro: in a template, a call that does not depend on a template parameter binds at the point of
// definition, and would otherwise bind to the host printf.
#include <hip/hip_runtime.h>
#endif
#if defined(EBGEOMETRY_DEVICE_COMPILE)
// Device compilation pass (CUDA/HIP): std::fprintf/std::abort are host-only. assert() is not an
// option either: NDEBUG, which CMake's Release configurations define, would silently remove every
// device check while the host checks stayed. Print with the device printf and trap instead.
#if defined(EBGEOMETRY_CUDA)
#define EBGEOMETRY_DEVICE_TRAP() __trap()
#else
#define EBGEOMETRY_DEVICE_TRAP() __builtin_trap()
#endif
namespace EBGeometry {
namespace MacrosDetail {
// Out of line, like the backends' own assert: the device printf expands to a lot of code, and an
// inline copy at every assertion site makes device compiles of large files take many minutes.
__device__ __noinline__ inline void
deviceAssertionFailed(const char* a_cond, const char* a_file, const int a_line)
{
  printf("EBGeometry device assertion failed: (%s)\n  file: %s\n  line: %d\n", a_cond, a_file, a_line);
  EBGEOMETRY_DEVICE_TRAP();
}
} // namespace MacrosDetail
} // namespace EBGeometry
#define EBGEOMETRY_EXPECT(cond)                                                     \
  do {                                                                              \
    if (!(cond)) {                                                                  \
      ::EBGeometry::MacrosDetail::deviceAssertionFailed(#cond, __FILE__, __LINE__); \
    }                                                                               \
  } while (0)
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
