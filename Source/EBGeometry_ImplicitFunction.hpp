// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_ImplicitFunction.hpp
 * @brief  Abstract base class for representing an implicit function.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_IMPLICITFUNCTION_HPP
#define EBGEOMETRY_IMPLICITFUNCTION_HPP

// Std includes
#include <type_traits>

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_FunctionQueries.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Abstract representation of an implicit function (not necessarily a signed distance function).
 * @details The value function must be implemented by subclasses.  Points with value < 0 are considered
 * inside the object; points with value > 0 are outside.
 * @tparam T Floating-point precision.
 */
template <class T>
class ImplicitFunction
{
  static_assert(std::is_floating_point_v<T>, "T must be a floating-point type");

public:
  /**
   * @brief Default constructor.
   */
  ImplicitFunction() = default;

  /**
   * @brief Default destructor.
   */
  virtual ~ImplicitFunction() = default;

  /**
   * @brief Value function. Points are outside the object if value > 0.0 and inside if value < 0.0.
   * @param[in] a_point 3D query point.
   * @return Implicit function value at a_point.
   */
  [[nodiscard]] virtual T
  value(const Vec3T<T>& a_point) const noexcept = 0;

  /**
   * @brief Call operator — alternative signature for the value function.
   * @param[in] a_point 3D query point.
   * @return Implicit function value at a_point (delegates to value()).
   */
  [[nodiscard]] T
  operator()(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Estimate the bounding box of the implicit surface by octree subdivision.
   * @details Calls the free function EBGeometry::approximateBoundingVolumeOctree() with this object.
   * @param[in] a_initialLowCorner  Low corner of the initial box.
   * @param[in] a_initialHighCorner High corner of the initial box.
   * @param[in] a_maxTreeDepth      Subdivision depth; at most MaxOctreeDepth.
   * @param[in] a_safety            Safety factor for the cell test; 0 = exact, 1 = a margin of one
   * half-diagonal.
   * @return Box enclosing the cells the surface may cross at the deepest level.
   */
  [[nodiscard]] inline BoundingVolumes::AABBT<T>
  approximateBoundingVolumeOctree(const Vec3T<T>&    a_initialLowCorner,
                                  const Vec3T<T>&    a_initialHighCorner,
                                  const unsigned int a_maxTreeDepth,
                                  const T&           a_safety = T(0)) const;
};

} // namespace EBGeometry

#include "EBGeometry_ImplicitFunctionImplem.hpp"

#endif
