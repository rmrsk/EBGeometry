// SPDX-FileCopyrightText: 2023 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_ImplicitFunctionImplem.hpp
 * @brief  Implementation of EBGeometry_ImplicitFunction.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_IMPLICITFUNCTIONIMPLEM_HPP
#define EBGEOMETRY_IMPLICITFUNCTIONIMPLEM_HPP

// Std includes
#include <cmath>

// Our includes
#include "EBGeometry_FunctionQueries.hpp"
#include "EBGeometry_ImplicitFunction.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

template <class T>
T
ImplicitFunction<T>::operator()(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  return this->value(a_point);
}

template <class T>
BoundingVolumes::AABBT<T>
ImplicitFunction<T>::approximateBoundingVolumeOctree(const Vec3T<T>&    a_initialLowCorner,
                                                     const Vec3T<T>&    a_initialHighCorner,
                                                     const unsigned int a_maxTreeDepth,
                                                     const T&           a_safety) const
{
  // Qualified, so that lookup finds the free function rather than this member.
  return EBGeometry::approximateBoundingVolumeOctree(
    *this, a_initialLowCorner, a_initialHighCorner, a_maxTreeDepth, a_safety);
}

} // namespace EBGeometry

#endif
