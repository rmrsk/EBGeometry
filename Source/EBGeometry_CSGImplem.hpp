/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_CSGImplem.hpp
  @brief  Implementation of EBGeometry_CSG.hpp
  @author Robert Marskar
*/

#ifndef EBGeometry_CSGImplem
#define EBGeometry_CSGImplem

// Our includes
#include "EBGeometry_CSG.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

template <class T>
std::shared_ptr<ImplicitFunction<T>>
CSG::Union(const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctions) noexcept
{
  return std::make_shared<UnionIF<T>>(a_implicitFunctions);
}

template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
CSG::Union(const std::shared_ptr<P1>& a_implicitFunction1, const std::shared_ptr<P2>& a_implicitFunction2) noexcept
{
  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;

  implicitFunctions.emplace_back(a_implicitFunction1);
  implicitFunctions.emplace_back(a_implicitFunction2);

  return std::make_shared<UnionIF<T>>(implicitFunctions);
}

template <class T>
std::shared_ptr<ImplicitFunction<T>>
CSG::SmoothUnion(const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctions,
                 const T                                                  a_smooth) noexcept
{
  return std::make_shared<SmoothUnion<T>>(a_implicitFunctions, a_smooth);
}

template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
CSG::SmoothUnion(const std::shared_ptr<P1>& a_implicitFunction1,
                 const std::shared_ptr<P2>& a_implicitFunction2,
                 const T                    a_smooth) noexcept
{
  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;

  implicitFunctions.emplace_back(a_implicitFunction1);
  implicitFunctions.emplace_back(a_implicitFunction2);

  return std::make_shared<SmoothUnionIF<T>>(implicitFunctions, a_smooth);
}

template <class T>
std::shared_ptr<ImplicitFunction<T>>
CSG::Intersection(const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctions) noexcept
{
  return std::make_shared<IntersectionIF<T>>(a_implicitFunctions);
}

template <class T>
std::shared_ptr<ImplicitFunction<T>>
CSG::Intersection(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunctionA,
                  const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunctionB) noexcept
{
  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;

  implicitFunctions.emplace_back(a_implicitFunctionA);
  implicitFunctions.emplace_back(a_implicitFunctionB);

  return std::make_shared<IntersectionIF<T>>(implicitFunctions);
}

template <class T>
std::shared_ptr<ImplicitFunction<T>>
CSG::SmoothIntersection(const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctions,
                        const T                                                  a_smooth) noexcept
{
  return std::make_shared<SmoothIntersection<T>>(a_implicitFunctions, a_smooth);
}

template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
CSG::SmoothIntersection(const std::shared_ptr<P1>& a_implicitFunction1,
                        const std::shared_ptr<P2>& a_implicitFunction2,
                        const T                    a_smooth) noexcept
{
  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;

  implicitFunctions.emplace_back(a_implicitFunction1);
  implicitFunctions.emplace_back(a_implicitFunction2);

  return std::make_shared<SmoothIntersectionIF<T>>(implicitFunctions, a_smooth);
}

template <class T>
std::shared_ptr<ImplicitFunction<T>>
CSG::Difference(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunctionA,
                const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunctionB) noexcept
{
  return std::make_shared<DifferenceIF<T>>(a_implicitFunctionA, a_implicitFunctionB);
}

template <class T>
UnionIF<T>::UnionIF(const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctions) noexcept
{
  for (const auto& prim : a_implicitFunctions) {
    m_implicitFunctions.emplace_back(prim);
  }
}

template <class T>
T
UnionIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  T ret = std::numeric_limits<T>::infinity();

  for (const auto& prim : m_implicitFunctions) {
    ret = std::min(ret, prim->value(a_point));
  }

  return ret;
}

template <class T>
SmoothUnionIF<T>::SmoothUnionIF(const std::vector<std::shared_ptr<ImplicitFunction<T>>>&    a_implicitFunctions,
                                const T                                                     a_smooth,
                                const std::function<T(const T& a_, const T& b, const T& s)> a_smoothMin) noexcept
{
  for (const auto& prim : a_implicitFunctions) {
    m_implicitFunctions.emplace_back(prim);
  }

  m_smooth = std::max(a_smooth, std::numeric_limits<T>::min());

  m_smoothMin = a_smoothMin;
}

template <class T>
T
SmoothUnionIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  T ret = std::numeric_limits<T>::infinity();

  if (m_implicitFunctions.size() == 1) {
    ret = m_implicitFunctions.front()->value(a_point);
  }
  else if (m_implicitFunctions.size() > 1) {
    T a = std::numeric_limits<T>::infinity();
    T b = std::numeric_limits<T>::infinity();

    for (const auto& implicitFunction : m_implicitFunctions) {
      const T curValue = implicitFunction->value(a_point);

      if (curValue < a) {
        b = a;
        a = curValue;
      }
      else if (curValue < b) {
        b = curValue;
      }
    }

    // Smooth minimum function.
#if 1
    ret = m_smoothMin(a, b, m_smooth);
#else
    const T h = std::max(m_smooth - std::abs(a - b), 0.0) / m_smooth;

    ret = std::min(a, b) - 0.25 * h * h * m_smooth;
#endif
  }

  return ret;
}

template <class T>
IntersectionIF<T>::IntersectionIF(const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctions) noexcept
{
  for (const auto& prim : a_implicitFunctions) {
    m_implicitFunctions.emplace_back(prim);
  }
}

template <class T>
T
IntersectionIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  T ret = -std::numeric_limits<T>::infinity();

  for (const auto& prim : m_implicitFunctions) {
    ret = std::max(ret, prim->value(a_point));
  }

  return ret;
}

template <class T>
SmoothIntersectionIF<T>::SmoothIntersectionIF(
  const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctions,
  const T                                                  a_smooth,
  const std::function<T(const T&, const T&, const T&)>&    a_smoothMax) noexcept
{
  for (const auto& prim : a_implicitFunctions) {
    m_implicitFunctions.emplace_back(prim);
  }

  m_smooth = std::max(a_smooth, std::numeric_limits<T>::min());

  m_smoothMax = a_smoothMax;
}

template <class T>
T
SmoothIntersectionIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  T ret = std::numeric_limits<T>::infinity();

  if (m_implicitFunctions.size() == 1) {
    ret = m_implicitFunctions.front()->value(a_point);
  }
  else if (m_implicitFunctions.size() > 1) {
    T a = -std::numeric_limits<T>::infinity();
    T b = -std::numeric_limits<T>::infinity();

    for (const auto& implicitFunction : m_implicitFunctions) {
      const T curValue = implicitFunction->value(a_point);

      if (curValue > a) {
        b = a;
        a = curValue;
      }
      else if (curValue > b) {
        b = curValue;
      }
    }

    ret = m_smoothMax(a, b, m_smooth);
  }

  return ret;
}

template <class T>
DifferenceIF<T>::DifferenceIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunctionA,
                              const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunctionB) noexcept
{
  m_implicitFunctionA = a_implicitFunctionA;
  m_implicitFunctionB = a_implicitFunctionB;
}

template <class T>
T
DifferenceIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  return std::min(m_implicitFunctionA->value(a_point), -m_implicitFunctionB->value(a_point));
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
