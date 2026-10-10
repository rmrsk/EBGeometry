// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_StreamOperators.hpp
 * @brief  Stream insertion (operator<<) for Vec3T and AABBT.
 * @details Kept out of the vector and bounding-volume headers so that code including those, device
 * code in particular, does not pull in the standard stream header. EBGeometry.hpp includes this
 * header.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_STREAMOPERATORS_HPP
#define EBGEOMETRY_STREAMOPERATORS_HPP

// Std includes
#include <ostream>

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Print a vector as (x,y,z).
 * @tparam T Floating-point precision.
 * @param[in,out] a_os  Output stream.
 * @param[in]     a_vec Vector to print.
 * @return Reference to the output stream after insertion.
 */
template <class T>
inline std::ostream&
operator<<(std::ostream& a_os, const Vec3T<T>& a_vec)
{
  a_os << '(' << a_vec[0] << ',' << a_vec[1] << ',' << a_vec[2] << ')';

  return a_os;
}

namespace BoundingVolumes {

/**
 * @brief Print a bounding box as (low corner, high corner).
 * @tparam T Floating-point precision.
 * @param[in,out] a_os   Output stream.
 * @param[in]     a_aabb Bounding box to print.
 * @return Reference to the output stream after insertion.
 */
template <class T>
inline std::ostream&
operator<<(std::ostream& a_os, const AABBT<T>& a_aabb)
{
  a_os << '(' << a_aabb.getLowCorner() << ", " << a_aabb.getHighCorner() << ')';

  return a_os;
}

} // namespace BoundingVolumes

} // namespace EBGeometry

#endif
