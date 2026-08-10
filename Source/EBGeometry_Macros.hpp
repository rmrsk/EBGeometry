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
 * When @c EBGEOMETRY_ENABLE_ASSERTIONS is not defined the macro does
 * @b not evaluate the condition at all: it expands to a discarded
 * @c sizeof, which is an unevaluated context. The expression is therefore
 * never executed (guaranteed zero runtime cost and no side effects, even
 * at @c -O0), yet is still parsed -- so it stays syntax-checked and any
 * variables/parameters that appear only inside assertions are still
 * considered "used" and do not trigger unused-entity warnings.
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
#else
// Unevaluated sizeof: cond is parsed (syntax-checked, names count as "used") but never evaluated,
// so disabled assertions have exactly zero runtime cost and cannot have observable side effects.
#define EBGEOMETRY_EXPECT(cond) (static_cast<void>(sizeof((cond))))
#endif

#endif // EBGEOMETRY_MACROS_HPP
