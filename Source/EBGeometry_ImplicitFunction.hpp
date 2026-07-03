// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  @file   EBGeometry_ImplicitFunction.hpp
  @brief  Abstract base class for representing an implicit function.
  @author Robert Marskar
*/

#ifndef EBGEOMETRY_IMPLICITFUNCTION_HPP
#define EBGEOMETRY_IMPLICITFUNCTION_HPP

// Our includes
#include "EBGeometry_Vec.hpp"
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/**
  @brief Abstract representation of an implicit function (not necessarily a signed distance function).
  @details The value function must be implemented by subclasses.  Points with value < 0 are considered
  inside the object; points with value > 0 are outside.
  @tparam T Floating-point precision.
*/
template <class T>
class ImplicitFunction
{
  static_assert(std::is_floating_point_v<T>, "T must be a floating-point type");

public:
  /**
    @brief Disallowed, use the full constructor
  */
  ImplicitFunction() = default;

  /**
    @brief Destructor (does nothing)
  */
  virtual ~ImplicitFunction() = default;

  /**
    @brief Value function. Points are outside the object if value > 0.0 and inside if value < 0.0.
    @param[in] a_point 3D query point.
    @return Implicit function value at a_point.
  */
  [[nodiscard]] virtual T
  value(const Vec3T<T>& a_point) const noexcept = 0;

  /**
    @brief Call operator — alternative signature for the value function.
    @param[in] a_point 3D query point.
    @return Implicit function value at a_point (delegates to value()).
  */
  [[nodiscard]] T
  operator()(const Vec3T<T>& a_point) const noexcept;

  /**
    @brief Compute an approximation to the bounding volume for the implicit surface using octree subdivision.
    @details Recursively subdivides the initial box and marks each child cell as intersected when
    `|value(center)| <= (1+a_safety)*half_width`. The bounding volume is built from the corners of all
    intersected leaf cells. Relies on the implicit function being reasonably close to a signed distance
    function; if octree subdivision fails entirely, returns the maximally representable bounding volume.
    @tparam BV         Bounding volume type; must be constructible from `std::vector<Vec3T<T>>`.
    @param[in] a_initialLowCorner  Low corner of the initial search box.
    @param[in] a_initialHighCorner High corner of the initial search box.
    @param[in] a_maxTreeDepth      Maximum permitted octree depth.
    @param[in] a_safety            Safety factor for intersection test; 0 = exact, 1 = 1x cell-width margin.
    @return Bounding volume enclosing the approximate zero-isosurface region.
  */
  template <class BV>
  [[nodiscard]] inline BV
  approximateBoundingVolumeOctree(const Vec3T<T>&    a_initialLowCorner,
                                  const Vec3T<T>&    a_initialHighCorner,
                                  const unsigned int a_maxTreeDepth,
                                  const T&           a_safety = 0.0) const;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_ImplicitFunctionImplem.hpp"

#endif
