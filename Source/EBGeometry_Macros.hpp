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

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>

#include "EBGeometry_GPU.hpp"

#if defined(EBGEOMETRY_HIP)
// Declares the device printf used below. It must be visible before the first header that uses the
// macros: in a template, a call that does not depend on a template parameter binds at the point of
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
// inline copy at every check site makes device compiles of large files take many minutes.
__device__ __noinline__ inline void
deviceAssertionFailed(const char* a_cond, const char* a_file, const int a_line)
{
  printf("EBGeometry device assertion failed: (%s)\n  file: %s\n  line: %d\n", a_cond, a_file, a_line);
  EBGEOMETRY_DEVICE_TRAP();
}
} // namespace MacrosDetail
} // namespace EBGeometry
#endif

namespace EBGeometry {

/**
 * @brief What a failed EBGEOMETRY_REQUIRE throws on the host inside a ThrowOnRequire scope: its message, as it would
 * have been printed.
 */
class RequireError : public std::runtime_error
{
public:
  using std::runtime_error::runtime_error;
};

namespace MacrosDetail {

/**
 * @brief How many ThrowOnRequire scopes are open on this thread.
 * @return The count.
 */
inline int&
throwingScopes() noexcept
{
  static thread_local int scopes = 0;

  return scopes;
}

/**
 * @brief A failed EBGEOMETRY_REQUIRE on the host: throws RequireError inside a ThrowOnRequire scope, prints and aborts
 * otherwise.
 * @details One call at each check, out of line, so that a check adds no more code to the function it is in than a call
 * (the message is formatted here, on failure only).
 * @param[in] a_cond   The failed condition.
 * @param[in] a_file   The file.
 * @param[in] a_line   The line.
 * @param[in] a_format The message, a printf format.
 * @param[in] ...      Its arguments.
 */
[[noreturn]]
#if defined(__GNUC__)
__attribute__((format(printf, 4, 5)))
#endif
inline void
requireFailed(const char* a_cond, const char* a_file, const int a_line, const char* a_format, ...)
{
  // The message whole: its length first, then the text.
  va_list arguments;
  va_list again;

  va_start(arguments, a_format);
  va_copy(again, arguments);

  const int length = std::vsnprintf(nullptr, 0, a_format, arguments);

  std::string message(length > 0 ? static_cast<size_t>(length) : 0U, '\0');

  if (length > 0) {
    std::vsnprintf(&message[0], message.size() + 1, a_format, again);
  }

  va_end(again);
  va_end(arguments);

  if (throwingScopes() > 0) {
    throw RequireError(message);
  }

  std::fprintf(stderr, "%s\n  check: (%s)\n  file: %s\n  line: %d\n", message.c_str(), a_cond, a_file, a_line);
  std::abort();
}

} // namespace MacrosDetail

/**
 * @brief While an object of this class lives, a failed EBGEOMETRY_REQUIRE on this thread throws RequireError instead
 * of aborting: for a program that builds objects from input it does not control (the .ebg reader), so that a bad value
 * is an error it reports, not the end of the program.
 * @details Host only; scopes nest. The scope is per thread: a failed check on another thread, even one started inside
 * the scope, still aborts. A check in a function that cannot throw (a noexcept function) still ends the program, since
 * the exception cannot leave it; the constructors of the built-in shapes and the expression builders can throw. What
 * the throwing call was building is not built, but a pool it was building into keeps what it allocated before the
 * check failed (the tape compiler builds BVHs into its pool as it compiles), until the pool is destroyed.
 */
class ThrowOnRequire
{
public:
  /**
   * @brief Open the scope.
   */
  ThrowOnRequire() noexcept
  {
    ++MacrosDetail::throwingScopes();
  }

  /**
   * @brief Close the scope.
   */
  ~ThrowOnRequire() noexcept
  {
    --MacrosDetail::throwingScopes();
  }

  ThrowOnRequire(const ThrowOnRequire&) = delete;
  ThrowOnRequire&
  operator=(const ThrowOnRequire&) = delete;
};

} // namespace EBGeometry

/**
 * @brief Always-on check of user input and one-time host setup.
 *
 * @details Evaluates @a cond in every build, whether or not @c EBGEOMETRY_ENABLE_ASSERTIONS is
 * defined. If it is false, prints @c "EBGeometry::" followed by the formatted message, the failed
 * condition, the file and the line to @c stderr, and calls @c std::abort(); inside a ThrowOnRequire scope, it throws
 * RequireError with the message instead (host only).
 *
 * The library uses two kinds of check:
 * - @c EBGEOMETRY_EXPECT for internal invariants, including on hot paths such as a signed-distance
 *   query. It compiles to nothing unless assertions are enabled.
 * - @c EBGEOMETRY_REQUIRE for what a caller controls -- constructor arguments, sizes and counts,
 *   memory resources -- checked once, outside any query loop, where a Release build would otherwise
 *   continue into a wrong answer or undefined behavior. Its cost is a few comparisons per object
 *   built.
 *
 * The file readers do not abort on a bad file: they throw EBGeometry::Parser::ParseError.
 *
 * The message is a @c printf format string literal, starting with the class or function that
 * checks, followed by its arguments:
 * @code{.cpp}
 * EBGEOMETRY_REQUIRE(a_radius > T(0), "SphereSDF: the radius must be positive (%g)", double(a_radius));
 * @endcode
 * On a device compilation pass (CUDA/HIP) the macro prints only the failed condition, with the device
 * @c printf, and traps.
 *
 * @param cond Boolean-convertible expression to test. Always evaluated.
 * @param ...  A @c printf format string literal, then its arguments.
 */
#if defined(EBGEOMETRY_DEVICE_COMPILE)
#define EBGEOMETRY_REQUIRE(cond, ...)                                               \
  do {                                                                              \
    if (!(cond)) {                                                                  \
      ::EBGeometry::MacrosDetail::deviceAssertionFailed(#cond, __FILE__, __LINE__); \
    }                                                                               \
  } while (0)
#else
#define EBGEOMETRY_REQUIRE(cond, ...)                                                                   \
  do {                                                                                                  \
    if (!(cond)) {                                                                                      \
      ::EBGeometry::MacrosDetail::requireFailed(#cond, __FILE__, __LINE__, "EBGeometry::" __VA_ARGS__); \
    }                                                                                                   \
  } while (0)
#endif

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
#if defined(EBGEOMETRY_DEVICE_COMPILE)
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
