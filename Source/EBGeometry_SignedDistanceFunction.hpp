// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  @file   EBGeometry_SignedDistanceFunction.hpp
  @brief  Abstract base class for representing a signed distance function.
  @author Robert Marskar
*/

#ifndef EBGEOMETRY_SIGNEDDISTANCEFUNCTION_HPP
#define EBGEOMETRY_SIGNEDDISTANCEFUNCTION_HPP

#include <memory>
#include <deque>

// Our includes
#include "EBGeometry_ImplicitFunction.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/**
  @brief Abstract representation of a signed distance function.
  @details Users can put whatever they like in here, e.g. analytic functions,
  DCEL meshes, or DCEL meshes stored in full or compact BVH trees. The
  signedDistance function must be implemented by the user. When computing it,
  the user can apply transformation operators (rotations, scaling, translations)
  by calling transformPoint on the input coordinate.
  @tparam T Floating-point precision.
*/
template <class T>
class SignedDistanceFunction : public ImplicitFunction<T>
{
  static_assert(std::is_floating_point_v<T>, "T must be a floating-point type");

public:
  /**
    @brief Default constructor.
  */
  SignedDistanceFunction() = default;

  /**
    @brief Destructor.
  */
  virtual ~SignedDistanceFunction() = default;

  /**
    @brief Implementation of ImplicitFunction::value; delegates to signedDistance().
    @param[in] a_point 3D query point.
    @return Signed distance at a_point.
  */
  [[nodiscard]] virtual T
  value(const Vec3T<T>& a_point) const noexcept override final;

  /**
    @brief Pure virtual signed distance function to be implemented by subclasses.
    @param[in] a_point 3D query point.
    @return Signed distance at a_point; negative inside, positive outside.
  */
  [[nodiscard]] virtual T
  signedDistance(const Vec3T<T>& a_point) const noexcept = 0;

  /**
    @brief Compute the outward normal using central finite differences.
    @details Approximates the gradient of the signed distance function at a_point using
    step size a_delta, then normalizes the result.
    @param[in] a_point 3D query point.
    @param[in] a_delta Finite-difference step size.
    @return Outward unit normal vector at a_point.
  */
  [[nodiscard]] inline virtual Vec3T<T>
  normal(const Vec3T<T>& a_point, const T& a_delta) const noexcept;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_SignedDistanceFunctionImplem.hpp"

#endif
