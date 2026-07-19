// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_ImplicitFunction.hpp
 * @brief  Type-erased base class and concrete-leaf template for implicit functions.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_IMPLICITFUNCTION_HPP
#define EBGEOMETRY_IMPLICITFUNCTION_HPP

// Std includes
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Forward declaration of the host-side tape builder (see EBGeometry_Tape.hpp).
 * @details Declared here so that the @c flatten lowering hook can appear on the implicit-function
 * base and leaf templates without pulling in the tape header (which itself depends on these
 * classes). The definition is only needed in EBGeometry_TapeImplem.hpp.
 * @tparam T Floating-point precision.
 */
template <class T>
class TapeBuilder;

/**
 * @brief Primary implicit-function template.
 * @details This template has two roles, selected by the @c Op parameter:
 *   - @c Op = @c void (the default) selects the type-erased base class -- the common handle held by
 *     CSG and transform nodes as @c std::shared_ptr<ImplicitFunction<T>>. See the @c <T,void>
 *     partial specialization.
 *   - @c Op != @c void selects a concrete trait-based leaf: the single source of truth for a
 *     primitive's distance formula lives in the trait's @c Op::eval, and this template is a thin
 *     value()-providing wrapper around it.
 * @tparam T  Floating-point precision.
 * @tparam Op Distance-formula trait (or @c void for the base).
 */
template <class T, class Op = void>
class ImplicitFunction;

/**
 * @brief Type-erased base class for implicit functions (not necessarily signed distance functions).
 * @details The value function must be implemented by subclasses. Points with value < 0 are considered
 * inside the object; points with value > 0 are outside. This is the common handle referred to by every
 * @c std::shared_ptr<ImplicitFunction<T>> in CSG/transform composition.
 * @tparam T Floating-point precision.
 */
template <class T>
class ImplicitFunction<T, void>
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
   * @brief Return whether or not this implicit function is also a signed distance function.
   * @details A true signed distance function additionally promises that the magnitude of value() is
   * the Euclidean distance to the surface. Analytic primitives set this to true; general CSG and
   * transform nodes leave it at the default.
   * @return True if this implicit function is a signed distance function.
   */
  [[nodiscard]] bool
  isSignedDistance() const noexcept
  {
    return m_sdf;
  }

  /**
   * @brief Compute the outward normal using central finite differences.
   * @details Approximates the gradient of value() at a_point using step size a_delta, then normalizes
   * the result. Previously lived on SignedDistanceFunction; now available on every implicit function.
   * @param[in] a_point 3D query point.
   * @param[in] a_delta Finite-difference step size. Must be strictly positive.
   * @return Outward unit normal vector at a_point. Undefined if the finite-difference gradient is
   * exactly zero (e.g. a_point sits on a local extremum of value()).
   */
  [[nodiscard]] inline Vec3T<T>
  normal(const Vec3T<T>& a_point, const T& a_delta) const noexcept;

  /**
   * @brief Compute an approximation to the bounding volume for the implicit surface using octree subdivision.
   * @details Recursively subdivides the initial box and marks each child cell as intersected when
   * `|value(center)| <= (1+a_safety)*half_width`. The bounding volume is built from the corners of all
   * intersected leaf cells. Relies on the implicit function being reasonably close to a signed distance
   * function; if octree subdivision fails entirely, returns the maximally representable bounding volume.
   * @tparam BV Bounding volume type; must be constructible from `std::vector<Vec3T<T>>`.
   * @param[in] a_initialLowCorner  Low corner of the initial search box.
   * @param[in] a_initialHighCorner High corner of the initial search box.
   * @param[in] a_maxTreeDepth      Maximum permitted octree depth.
   * @param[in] a_safety            Safety factor for intersection test; 0 = exact, 1 = 1x cell-width margin.
   * @return Bounding volume enclosing the approximate zero-isosurface region.
   */
  template <class BV>
  [[nodiscard]] inline BV
  approximateBoundingVolumeOctree(const Vec3T<T>&    a_initialLowCorner,
                                  const Vec3T<T>&    a_initialHighCorner,
                                  const unsigned int a_maxTreeDepth,
                                  const T&           a_safety = 0.0) const;

  /**
   * @brief Lower this implicit function into a flat tape (see EBGeometry_Tape.hpp).
   * @details Non-pure virtual: the base default emits a single opaque host-callback clause that, at
   * evaluation, invokes this node's value() on the host. Trait-based leaves and the CSG/transform
   * nodes override this to emit native clauses; any node without an override (Perlin, Blur, Mollify,
   * FiniteRepetition, the BVH unions, or a user subclass) keeps the opaque-host default. Host only.
   * @param[in,out] a_builder   Tape builder accumulating the flattened clauses.
   * @param[in]     a_coordSlot Coordinate slot holding this subtree's input frame.
   * @return Value slot holding this subtree's result.
   */
  [[nodiscard]] EBGEOMETRY_HOST virtual int
  flatten(TapeBuilder<T>& a_builder, int a_coordSlot) const;

protected:
  /**
   * @brief Whether or not this implicit function is a signed distance function.
   * @details Defaults to true. Concrete trait-based leaves set this from Op::isSignedDistance.
   */
  bool m_sdf = true;
};

/**
 * @brief Concrete trait-based leaf implicit function.
 * @details Every trait-based analytic primitive is an instantiation of this template. The distance
 * formula lives in exactly one place -- the trait's @c Op::eval -- and this class is a thin wrapper
 * that stores the primitive's parameters (@c Op::Params) and forwards value() to @c Op::eval.
 * @tparam T  Floating-point precision.
 * @tparam Op Distance-formula trait. Must provide a nested @c Params type, a
 * `static constexpr bool isSignedDistance`, and `static T eval(const Params&, const Vec3T<T>&)`.
 */
template <class T, class Op>
class ImplicitFunction
#ifndef EBGEOMETRY_DOXYGEN
  // The base-specifier is hidden from Doxygen 1.9.8, whose class-relation analysis cannot tell the
  // leaf specialization ImplicitFunction<T, Op> apart from its base ImplicitFunction<T, void> and
  // otherwise emits a spurious "recursive class relation" error. The real compiler always sees it.
  : public ImplicitFunction<T, void>
#endif
{
  static_assert(std::is_floating_point_v<T>, "T must be a floating-point type");

public:
  /**
   * @brief Alias for the trait's parameter bundle.
   */
  using Params = typename Op::Params;

  /**
   * @brief Full constructor.
   * @param[in] a_params Trait parameter bundle.
   * @param[in] a_sdf    Whether this instance is a signed distance function (defaults to Op::isSignedDistance).
   */
  EBGEOMETRY_HOST_DEVICE explicit ImplicitFunction(const Params& a_params,
                                                   const bool    a_sdf = Op::isSignedDistance) noexcept
    : m_params(a_params)
  {
    this->m_sdf = a_sdf;
  }

  /**
   * @brief Destructor.
   */
  ~ImplicitFunction() override = default;

  /**
   * @brief Value function. Delegates to the trait's eval.
   * @param[in] a_point 3D query point.
   * @return Op::eval(m_params, a_point).
   */
  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override
  {
    return Op::eval(m_params, a_point);
  }

  /**
   * @brief Get the trait parameter bundle.
   * @return Const reference to the stored parameters.
   */
  [[nodiscard]] const Params&
  getParams() const noexcept
  {
    return m_params;
  }

  /**
   * @brief Lower this leaf into a native tape clause (or an opaque host callback for an unregistered
   * @c Op).
   * @details If @c Op has a registered @ref Opcode this emits a single leaf clause via
   * @c TapeBuilder::emitLeaf; otherwise it falls back to an opaque host callback. Host only.
   * @param[in,out] a_builder   Tape builder accumulating the flattened clauses.
   * @param[in]     a_coordSlot Coordinate slot holding the query point's frame.
   * @return Value slot holding this leaf's result.
   */
  [[nodiscard]] EBGEOMETRY_HOST int
  flatten(TapeBuilder<T>& a_builder, int a_coordSlot) const override;

protected:
  /**
   * @brief Trait parameter bundle.
   */
  Params m_params;
};

} // namespace EBGeometry

#include "EBGeometry_ImplicitFunctionImplem.hpp"

#endif
