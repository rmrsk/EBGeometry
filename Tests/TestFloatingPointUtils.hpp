// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Shared helpers for writing precision-independent (TEMPLATE_TEST_CASE<float, double>) Catch2
// tests. Not part of the library itself -- test infrastructure only.
//
// Catch::Matchers::WithinRel has type-specific overloads (its no-epsilon-argument form defaults
// to 100*numeric_limits<T>::epsilon(), separately for float and double), so it needs no help here
// as long as its argument is T rather than a hardcoded double. Catch::Matchers::WithinAbs has only
// ever accepted double, with no type-specific default margin, so absolute-tolerance comparisons
// need an explicit, type-scaled margin -- and passing a T target to it directly triggers
// -Wdouble-promotion, hence withinAbsT()'s explicit cast.

#ifndef EBGEOMETRY_TEST_FLOATINGPOINTUTILS_HPP
#define EBGEOMETRY_TEST_FLOATINGPOINTUTILS_HPP

#include <limits>

#include <catch2/matchers/catch_matchers_floating_point.hpp>

// The type list every TEMPLATE_TEST_CASE(..., EBGEOMETRY_TEST_PRECISIONS) in the suite is
// instantiated over. Local builds only compile/run `double` (fast iteration, matches what the
// CMake preset otherwise builds); CI additionally defines EBGEOMETRY_TEST_BOTH_PRECISIONS (see
// Tests/CMakeLists.txt's EBGEOMETRY_TEST_BOTH_PRECISIONS option) to exercise `float` too.
#if defined(EBGEOMETRY_TEST_BOTH_PRECISIONS)
#define EBGEOMETRY_TEST_PRECISIONS float, double
#else
#define EBGEOMETRY_TEST_PRECISIONS double
#endif

// A margin appropriate for a comparison that is exactly representable in either precision (e.g. a
// normal computed from axis-aligned input, or an early-exit zero with no arithmetic involved).
template <class T>
double
tightMargin()
{
  return static_cast<double>(T(1000) * std::numeric_limits<T>::epsilon());
}

// A margin appropriate for a comparison whose expected value is only approximately zero even in
// double (a genuine sqrt/division/trigonometric result, or the accumulated error of several such
// operations).
template <class T>
double
looseMargin()
{
  return static_cast<double>(T(1.0e5) * std::numeric_limits<T>::epsilon());
}

// WithinAbs(double, double): wraps the explicit cast needed to pass a T target without an
// -Wdouble-promotion warning when T = float.
template <class T>
Catch::Matchers::WithinAbsMatcher
withinAbsT(const T a_target, const double a_margin)
{
  return Catch::Matchers::WithinAbs(static_cast<double>(a_target), a_margin);
}

#endif
