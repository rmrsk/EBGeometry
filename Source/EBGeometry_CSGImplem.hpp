// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_CSGImplem.hpp
 * @brief  Implementation of EBGeometry_CSG.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_CSGIMPLEM_HPP
#define EBGEOMETRY_CSGIMPLEM_HPP

// Std includes
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_CSG.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_Transform.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

template <class T, class P>
std::shared_ptr<ImplicitFunction<T>>
Union(const std::vector<std::shared_ptr<P>>& a_implicitFunctions)
{
  static_assert(std::is_floating_point_v<T>, "Union requires a floating-point type T");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P>, "P must derive from ImplicitFunction<T>");

  EBGEOMETRY_REQUIRE(!a_implicitFunctions.empty(), "Union: the list of implicit functions must not be empty");

  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;

  implicitFunctions.reserve(a_implicitFunctions.size());

  for (const auto& f : a_implicitFunctions) {
    implicitFunctions.emplace_back(f);
  }

  return std::make_shared<UnionIF<T>>(implicitFunctions);
}

template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
Union(const std::shared_ptr<P1>& a_implicitFunction1, const std::shared_ptr<P2>& a_implicitFunction2)
{
  static_assert(std::is_floating_point_v<T>, "Union requires a floating-point type T");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P1>, "P1 must derive from ImplicitFunction<T>");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P2>, "P2 must derive from ImplicitFunction<T>");

  EBGEOMETRY_REQUIRE(a_implicitFunction1 != nullptr, "Union: the first implicit function must not be null");
  EBGEOMETRY_REQUIRE(a_implicitFunction2 != nullptr, "Union: the second implicit function must not be null");

  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;

  implicitFunctions.emplace_back(a_implicitFunction1);
  implicitFunctions.emplace_back(a_implicitFunction2);

  return std::make_shared<UnionIF<T>>(implicitFunctions);
}

template <class T, class P>
std::shared_ptr<ImplicitFunction<T>>
SmoothUnion(const std::vector<std::shared_ptr<P>>& a_implicitFunctions, const T a_smooth)
{
  static_assert(std::is_floating_point_v<T>, "SmoothUnion requires a floating-point type T");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P>, "P must derive from ImplicitFunction<T>");

  EBGEOMETRY_REQUIRE(!a_implicitFunctions.empty(), "SmoothUnion: the list of implicit functions must not be empty");
  EBGEOMETRY_REQUIRE(a_smooth > T(0), "SmoothUnion: the smoothing length must be positive (%g)", double(a_smooth));

  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;

  implicitFunctions.reserve(a_implicitFunctions.size());

  for (const auto& f : a_implicitFunctions) {
    implicitFunctions.emplace_back(f);
  }

  return std::make_shared<SmoothUnionIF<T>>(implicitFunctions, a_smooth);
}

template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
SmoothUnion(const std::shared_ptr<P1>& a_implicitFunction1,
            const std::shared_ptr<P2>& a_implicitFunction2,
            const T                    a_smooth)
{
  static_assert(std::is_floating_point_v<T>, "SmoothUnion requires a floating-point type T");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P1>, "P1 must derive from ImplicitFunction<T>");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P2>, "P2 must derive from ImplicitFunction<T>");

  EBGEOMETRY_REQUIRE(a_implicitFunction1 != nullptr, "SmoothUnion: the first implicit function must not be null");
  EBGEOMETRY_REQUIRE(a_implicitFunction2 != nullptr, "SmoothUnion: the second implicit function must not be null");
  EBGEOMETRY_REQUIRE(a_smooth > T(0), "SmoothUnion: the smoothing length must be positive (%g)", double(a_smooth));

  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;

  implicitFunctions.emplace_back(a_implicitFunction1);
  implicitFunctions.emplace_back(a_implicitFunction2);

  return std::make_shared<SmoothUnionIF<T>>(implicitFunctions, a_smooth);
}

template <class T, class P>
std::shared_ptr<ImplicitFunction<T>>
Intersection(const std::vector<std::shared_ptr<P>>& a_implicitFunctions)
{
  static_assert(std::is_floating_point_v<T>, "Intersection requires a floating-point type T");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P>, "P must derive from ImplicitFunction<T>");

  EBGEOMETRY_REQUIRE(!a_implicitFunctions.empty(), "Intersection: the list of implicit functions must not be empty");

  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;

  implicitFunctions.reserve(a_implicitFunctions.size());

  for (const auto& f : a_implicitFunctions) {
    implicitFunctions.emplace_back(f);
  }

  return std::make_shared<IntersectionIF<T>>(implicitFunctions);
}

template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
Intersection(const std::shared_ptr<P1>& a_implicitFunction1, const std::shared_ptr<P2>& a_implicitFunction2)
{
  static_assert(std::is_floating_point_v<T>, "Intersection requires a floating-point type T");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P1>, "P1 must derive from ImplicitFunction<T>");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P2>, "P2 must derive from ImplicitFunction<T>");

  EBGEOMETRY_REQUIRE(a_implicitFunction1 != nullptr, "Intersection: the first implicit function must not be null");
  EBGEOMETRY_REQUIRE(a_implicitFunction2 != nullptr, "Intersection: the second implicit function must not be null");

  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;

  implicitFunctions.emplace_back(a_implicitFunction1);
  implicitFunctions.emplace_back(a_implicitFunction2);

  return std::make_shared<IntersectionIF<T>>(implicitFunctions);
}

template <class T, class P>
std::shared_ptr<ImplicitFunction<T>>
SmoothIntersection(const std::vector<std::shared_ptr<P>>& a_implicitFunctions, const T a_smooth)
{
  static_assert(std::is_floating_point_v<T>, "SmoothIntersection requires a floating-point type T");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P>, "P must derive from ImplicitFunction<T>");

  EBGEOMETRY_REQUIRE(!a_implicitFunctions.empty(),
                     "SmoothIntersection: the list of implicit functions must not be empty");
  EBGEOMETRY_REQUIRE(
    a_smooth > T(0), "SmoothIntersection: the smoothing length must be positive (%g)", double(a_smooth));

  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;

  for (const auto& f : a_implicitFunctions) {
    implicitFunctions.emplace_back(f);
  }

  return std::make_shared<SmoothIntersectionIF<T>>(implicitFunctions, a_smooth);
}

template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
SmoothIntersection(const std::shared_ptr<P1>& a_implicitFunction1,
                   const std::shared_ptr<P2>& a_implicitFunction2,
                   const T                    a_smooth)
{
  static_assert(std::is_floating_point_v<T>, "SmoothIntersection requires a floating-point type T");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P1>, "P1 must derive from ImplicitFunction<T>");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P2>, "P2 must derive from ImplicitFunction<T>");

  EBGEOMETRY_REQUIRE(a_implicitFunction1 != nullptr,
                     "SmoothIntersection: the first implicit function must not be null");
  EBGEOMETRY_REQUIRE(a_implicitFunction2 != nullptr,
                     "SmoothIntersection: the second implicit function must not be null");
  EBGEOMETRY_REQUIRE(
    a_smooth > T(0), "SmoothIntersection: the smoothing length must be positive (%g)", double(a_smooth));

  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;

  implicitFunctions.emplace_back(a_implicitFunction1);
  implicitFunctions.emplace_back(a_implicitFunction2);

  return std::make_shared<SmoothIntersectionIF<T>>(implicitFunctions, a_smooth);
}

template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
Difference(const std::shared_ptr<P1>& a_implicitFunctionA, const std::shared_ptr<P2>& a_implicitFunctionB)
{
  static_assert(std::is_floating_point_v<T>, "Difference requires a floating-point type T");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P1>, "P1 must derive from ImplicitFunction<T>");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P2>, "P2 must derive from ImplicitFunction<T>");

  EBGEOMETRY_REQUIRE(a_implicitFunctionA != nullptr, "Difference: implicit function A must not be null");
  EBGEOMETRY_REQUIRE(a_implicitFunctionB != nullptr, "Difference: implicit function B must not be null");

  return std::make_shared<DifferenceIF<T>>(a_implicitFunctionA, a_implicitFunctionB);
}

template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
SmoothDifference(const std::shared_ptr<P1>& a_implicitFunctionA,
                 const std::shared_ptr<P2>& a_implicitFunctionB,
                 const T                    a_smoothLen)
{
  static_assert(std::is_floating_point_v<T>, "SmoothDifference requires a floating-point type T");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P1>, "P1 must derive from ImplicitFunction<T>");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P2>, "P2 must derive from ImplicitFunction<T>");

  EBGEOMETRY_REQUIRE(a_implicitFunctionA != nullptr, "SmoothDifference: implicit function A must not be null");
  EBGEOMETRY_REQUIRE(a_implicitFunctionB != nullptr, "SmoothDifference: implicit function B must not be null");
  EBGEOMETRY_REQUIRE(
    a_smoothLen > T(0), "SmoothDifference: the smoothing length must be positive (%g)", double(a_smoothLen));

  return std::make_shared<SmoothDifferenceIF<T>>(a_implicitFunctionA, a_implicitFunctionB, a_smoothLen);
}

template <class T, class P>
std::shared_ptr<ImplicitFunction<T>>
FiniteRepetition(const std::shared_ptr<P>& a_implicitFunction,
                 const Vec3T<T>&           a_period,
                 const Vec3T<T>&           a_repeatLo,
                 const Vec3T<T>&           a_repeatHi)
{
  static_assert(std::is_floating_point_v<T>, "FiniteRepetition requires a floating-point type T");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P>, "P must derive from ImplicitFunction<T>");

  EBGEOMETRY_REQUIRE(a_implicitFunction != nullptr, "FiniteRepetition: the implicit function must not be null");

  EBGEOMETRY_REQUIRE(a_period[0] > T(0) && a_period[1] > T(0) && a_period[2] > T(0),
                     "FiniteRepetition: the period must be positive in every direction (%g, %g, %g)",
                     double(a_period[0]),
                     double(a_period[1]),
                     double(a_period[2]));

  return std::make_shared<FiniteRepetitionIF<T>>(a_implicitFunction, a_period, a_repeatLo, a_repeatHi);
}

template <class T>
UnionIF<T>::UnionIF(const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctions)
{
  EBGEOMETRY_REQUIRE(!a_implicitFunctions.empty(), "UnionIF: the list of implicit functions must not be empty");

  for (const auto& prim : a_implicitFunctions) {
    EBGEOMETRY_REQUIRE(prim != nullptr, "UnionIF: the list of implicit functions must not contain a null entry");

    m_implicitFunctions.emplace_back(prim);
  }
}

template <class T>
T
UnionIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  T ret = Math::Limits<T>::infinity();

  for (const auto& prim : m_implicitFunctions) {
    const T v = prim->value(a_point);

    EBGEOMETRY_EXPECT(!std::isnan(v));

    ret = Math::min(ret, v);
  }

  return ret;
}

template <class T>
SmoothUnionIF<T>::SmoothUnionIF(const std::vector<std::shared_ptr<ImplicitFunction<T>>>&    a_implicitFunctions,
                                const T                                                     a_smoothLen,
                                const std::function<T(const T& a_, const T& b, const T& s)> a_smoothMin)
{
  EBGEOMETRY_REQUIRE(!a_implicitFunctions.empty(), "SmoothUnionIF: the list of implicit functions must not be empty");
  EBGEOMETRY_REQUIRE(
    a_smoothLen > T(0), "SmoothUnionIF: the smoothing length must be positive (%g)", double(a_smoothLen));

  for (const auto& prim : a_implicitFunctions) {
    EBGEOMETRY_REQUIRE(prim != nullptr, "SmoothUnionIF: the list of implicit functions must not contain a null entry");

    m_implicitFunctions.emplace_back(prim);
  }

  m_smoothLen = Math::max(a_smoothLen, Math::Limits<T>::min());
  m_smoothMin = a_smoothMin;
}

template <class T>
T
SmoothUnionIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  T ret = Math::Limits<T>::infinity();

  if (m_implicitFunctions.size() == 1) {
    ret = m_implicitFunctions.front()->value(a_point);

    EBGEOMETRY_EXPECT(!std::isnan(ret));
  }
  else if (m_implicitFunctions.size() > 1) {
    T a = Math::Limits<T>::infinity();
    T b = Math::Limits<T>::infinity();

    for (const auto& implicitFunction : m_implicitFunctions) {
      const T curValue = implicitFunction->value(a_point);

      EBGEOMETRY_EXPECT(!std::isnan(curValue));

      if (curValue < a) {
        b = a;
        a = curValue;
      }
      else if (curValue < b) {
        b = curValue;
      }
    }

    ret = m_smoothMin(a, b, m_smoothLen);
  }

  return ret;
}

template <class T>
IntersectionIF<T>::IntersectionIF(const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctions) noexcept
{
  EBGEOMETRY_REQUIRE(!a_implicitFunctions.empty(), "IntersectionIF: the list of implicit functions must not be empty");

  for (const auto& prim : a_implicitFunctions) {
    EBGEOMETRY_REQUIRE(prim != nullptr, "IntersectionIF: the list of implicit functions must not contain a null entry");

    m_implicitFunctions.emplace_back(prim);
  }
}

template <class T>
T
IntersectionIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  T ret = -Math::Limits<T>::infinity();

  for (const auto& prim : m_implicitFunctions) {
    const T v = prim->value(a_point);

    EBGEOMETRY_EXPECT(!std::isnan(v));

    ret = Math::max(ret, v);
  }

  return ret;
}

template <class T>
SmoothIntersectionIF<T>::SmoothIntersectionIF(
  const std::shared_ptr<ImplicitFunction<T>>&           a_implicitFunctionA,
  const std::shared_ptr<ImplicitFunction<T>>&           a_implicitFunctionB,
  const T                                               a_smoothLen,
  const std::function<T(const T&, const T&, const T&)>& a_smoothMax) noexcept
{
  EBGEOMETRY_REQUIRE(a_implicitFunctionA != nullptr, "SmoothIntersectionIF: implicit function A must not be null");
  EBGEOMETRY_REQUIRE(a_implicitFunctionB != nullptr, "SmoothIntersectionIF: implicit function B must not be null");
  EBGEOMETRY_REQUIRE(
    a_smoothLen > T(0), "SmoothIntersectionIF: the smoothing length must be positive (%g)", double(a_smoothLen));

  m_implicitFunctions.emplace_back(a_implicitFunctionA);
  m_implicitFunctions.emplace_back(a_implicitFunctionB);

  m_smoothLen = Math::max(a_smoothLen, Math::Limits<T>::min());
  m_smoothMax = a_smoothMax;
}

template <class T>
SmoothIntersectionIF<T>::SmoothIntersectionIF(
  const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctions,
  const T                                                  a_smoothLen,
  const std::function<T(const T&, const T&, const T&)>&    a_smoothMax) noexcept
{
  EBGEOMETRY_REQUIRE(!a_implicitFunctions.empty(),
                     "SmoothIntersectionIF: the list of implicit functions must not be empty");
  EBGEOMETRY_REQUIRE(
    a_smoothLen > T(0), "SmoothIntersectionIF: the smoothing length must be positive (%g)", double(a_smoothLen));

  for (const auto& prim : a_implicitFunctions) {
    EBGEOMETRY_REQUIRE(prim != nullptr,
                       "SmoothIntersectionIF: the list of implicit functions must not contain a null entry");

    m_implicitFunctions.emplace_back(prim);
  }

  m_smoothLen = Math::max(a_smoothLen, Math::Limits<T>::min());
  m_smoothMax = a_smoothMax;
}

template <class T>
T
SmoothIntersectionIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  T ret = Math::Limits<T>::infinity();

  if (m_implicitFunctions.size() == 1) {
    ret = m_implicitFunctions.front()->value(a_point);

    EBGEOMETRY_EXPECT(!std::isnan(ret));
  }
  else if (m_implicitFunctions.size() > 1) {
    T a = -Math::Limits<T>::infinity();
    T b = -Math::Limits<T>::infinity();

    for (const auto& implicitFunction : m_implicitFunctions) {
      const T curValue = implicitFunction->value(a_point);

      EBGEOMETRY_EXPECT(!std::isnan(curValue));

      if (curValue > a) {
        b = a;
        a = curValue;
      }
      else if (curValue > b) {
        b = curValue;
      }
    }

    ret = m_smoothMax(a, b, m_smoothLen);
  }

  return ret;
}

template <class T>
DifferenceIF<T>::DifferenceIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunctionA,
                              const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunctionB) noexcept
{
  EBGEOMETRY_REQUIRE(a_implicitFunctionA != nullptr, "DifferenceIF: implicit function A must not be null");
  EBGEOMETRY_REQUIRE(a_implicitFunctionB != nullptr, "DifferenceIF: implicit function B must not be null");

  m_implicitFunctionA = a_implicitFunctionA;
  m_implicitFunctionB = a_implicitFunctionB;
}

template <class T>
DifferenceIF<T>::DifferenceIF(const std::shared_ptr<ImplicitFunction<T>>&              a_implicitFunctionA,
                              const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctionsB) noexcept
{
  EBGEOMETRY_REQUIRE(a_implicitFunctionA != nullptr, "DifferenceIF: implicit function A must not be null");
  EBGEOMETRY_REQUIRE(!a_implicitFunctionsB.empty(),
                     "DifferenceIF: the list of subtracted implicit functions must not be empty");

  m_implicitFunctionA = a_implicitFunctionA;
  m_implicitFunctionB = EBGeometry::Union<T>(a_implicitFunctionsB);
}

template <class T>
T
DifferenceIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  const T a = m_implicitFunctionA->value(a_point);
  const T b = m_implicitFunctionB->value(a_point);

  EBGEOMETRY_EXPECT(!std::isnan(a));
  EBGEOMETRY_EXPECT(!std::isnan(b));

  return Math::max(a, -b);
}

template <class T>
SmoothDifferenceIF<T>::SmoothDifferenceIF(
  const std::shared_ptr<ImplicitFunction<T>>&                 a_implicitFunctionA,
  const std::shared_ptr<ImplicitFunction<T>>&                 a_implicitFunctionB,
  const T                                                     a_smoothLen,
  const std::function<T(const T& a, const T& b, const T& s)>& a_smoothMax) noexcept
{
  EBGEOMETRY_REQUIRE(a_implicitFunctionA != nullptr, "SmoothDifferenceIF: implicit function A must not be null");
  EBGEOMETRY_REQUIRE(a_implicitFunctionB != nullptr, "SmoothDifferenceIF: implicit function B must not be null");
  EBGEOMETRY_REQUIRE(
    a_smoothLen > T(0), "SmoothDifferenceIF: the smoothing length must be positive (%g)", double(a_smoothLen));

  m_smoothIntersectionIF = std::make_shared<SmoothIntersectionIF<T>>(
    a_implicitFunctionA, EBGeometry::Complement<T>(a_implicitFunctionB), a_smoothLen, a_smoothMax);
}

template <class T>
SmoothDifferenceIF<T>::SmoothDifferenceIF(
  const std::shared_ptr<ImplicitFunction<T>>&                 a_implicitFunctionA,
  const std::vector<std::shared_ptr<ImplicitFunction<T>>>&    a_implicitFunctionsB,
  const T                                                     a_smoothLen,
  const std::function<T(const T& a, const T& b, const T& s)>& a_smoothMax) noexcept
{
  EBGEOMETRY_REQUIRE(a_implicitFunctionA != nullptr, "SmoothDifferenceIF: implicit function A must not be null");
  EBGEOMETRY_REQUIRE(!a_implicitFunctionsB.empty(),
                     "SmoothDifferenceIF: the list of subtracted implicit functions must not be empty");
  EBGEOMETRY_REQUIRE(
    a_smoothLen > T(0), "SmoothDifferenceIF: the smoothing length must be positive (%g)", double(a_smoothLen));

  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;

  implicitFunctions.reserve(1 + a_implicitFunctionsB.size());
  implicitFunctions.emplace_back(a_implicitFunctionA);

  for (const auto& subtractedFunction : a_implicitFunctionsB) {
    EBGEOMETRY_REQUIRE(subtractedFunction != nullptr,
                       "SmoothDifferenceIF: the list of subtracted implicit functions must not contain a null entry");

    implicitFunctions.emplace_back(EBGeometry::Complement<T>(subtractedFunction));
  }

  m_smoothIntersectionIF = std::make_shared<SmoothIntersectionIF<T>>(implicitFunctions, a_smoothLen, a_smoothMax);
}

template <class T>
T
SmoothDifferenceIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(m_smoothIntersectionIF != nullptr);

  return m_smoothIntersectionIF->value(a_point);
}

template <class T>
FiniteRepetitionIF<T>::FiniteRepetitionIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction,
                                          const Vec3T<T>&                             a_period,
                                          const Vec3T<T>&                             a_repeatLo,
                                          const Vec3T<T>&                             a_repeatHi) noexcept
{
  EBGEOMETRY_REQUIRE(a_implicitFunction != nullptr, "FiniteRepetitionIF: the implicit function must not be null");
  EBGEOMETRY_REQUIRE(a_period[0] > T(0) && a_period[1] > T(0) && a_period[2] > T(0),
                     "FiniteRepetitionIF: the period must be positive in every direction (%g, %g, %g)",
                     double(a_period[0]),
                     double(a_period[1]),
                     double(a_period[2]));
  EBGEOMETRY_REQUIRE(a_repeatLo[0] >= T(0) && a_repeatLo[1] >= T(0) && a_repeatLo[2] >= T(0),
                     "FiniteRepetitionIF: the low repetition counts must not be negative (%g, %g, %g)",
                     double(a_repeatLo[0]),
                     double(a_repeatLo[1]),
                     double(a_repeatLo[2]));
  EBGEOMETRY_REQUIRE(a_repeatHi[0] >= T(0) && a_repeatHi[1] >= T(0) && a_repeatHi[2] >= T(0),
                     "FiniteRepetitionIF: the high repetition counts must not be negative (%g, %g, %g)",
                     double(a_repeatHi[0]),
                     double(a_repeatHi[1]),
                     double(a_repeatHi[2]));

  m_implicitFunction = a_implicitFunction;
  m_period           = a_period;

  // Whole tiles only: rounding after clamping to a fractional bound would otherwise produce a tile
  // past it.
  for (size_t i = 0; i < 3; i++) {
    m_repeatLo[i] = std::round(a_repeatLo[i]);
    m_repeatHi[i] = std::round(a_repeatHi[i]);
  }
}

template <class T>
T
FiniteRepetitionIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  Vec3T<T> q;

  for (size_t i = 0; i < 3; i++) {
    // Math::min/Math::max rather than Math::clamp, so the result stays defined if the bounds cross.
    q[i] = a_point[i] -
           m_period[i] * std::round(Math::min(Math::max(a_point[i] / m_period[i], -m_repeatLo[i]), m_repeatHi[i]));
  }

  const T ret = m_implicitFunction->value(q);

  EBGEOMETRY_EXPECT(!std::isnan(ret));

  return ret;
}

} // namespace EBGeometry

#endif
