// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_PrimitiveRef.hpp
 * @brief  PrimitiveRef: which leaf of a composition decides its value at a point.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_PRIMITIVEREF_HPP
#define EBGEOMETRY_PRIMITIVEREF_HPP

// Std includes
#include <cstdint>

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief The deciding leaf of a composition at a point: the primitive whose surface the composed value
 * comes from there, as Tape::closestPrimitive() reports it.
 * @details A union or intersection takes the leaf of the operand whose value it takes; a smooth one the
 * more extreme operand's; a blur its centre tap's, a mollification its heaviest tap's; a complement, offset,
 * annulus, map, elongation or repetition its operand's. For Difference(box, sphere), a point near the
 * carved cavity gets the sphere and a point near the box's outside gets the box. This is the leaf that
 * decides the value, not the geometrically nearest primitive, which could be one the composition removed.
 * A nested composite (a fused composition, which is one shape with one surface; a tape listed as a
 * primitive; a BVH union) is one leaf.
 *
 * The tape that made the reference returns the primitive itself: isPrimitive<P>(ref), primitive<P>(ref)
 * (by value: the shape, or a pool-resident one's handle rebuilt with the tape's location) and
 * visitPrimitive(ref, f).
 * @tparam T Floating-point precision.
 */
template <class T>
struct PrimitiveRef
{
  /**
   * @brief No leaf (an empty composition).
   */
  static constexpr uint32_t None = 0xFFFFFFFFU;

  /**
   * @brief The leaf: its number in expression order, depth first, each occurrence counted.
   */
  uint32_t m_leaf = None;

  /**
   * @brief The index of the primitive's type in the tape's type list (the built-in types, then
   * UserPrimitives).
   */
  uint32_t m_type = None;

  /**
   * @brief The primitive's index within that type's array.
   */
  uint32_t m_index = None;

  /**
   * @brief The point in the primitive's own frame (after the composition's maps), at which it can be
   * queried itself: its signed distance, closest face or normal.
   */
  Vec3T<T> m_localPoint = Vec3T<T>::zeros();
};

} // namespace EBGeometry

#endif
