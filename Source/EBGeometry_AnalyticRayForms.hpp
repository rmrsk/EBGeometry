// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_AnalyticRayForms.hpp
 * @brief  Ray forms (see RayForm) for the analytic shapes that have a closed-form crossing: PlaneSDF,
 * SphereSDF, BoxSDF, CylinderSDF, InfiniteCylinderSDF and CapsuleSDF. With this header included, firstHit()
 * and intersects() on these shapes are exact instead of marched, and so are compositions of them (see
 * EBGeometry_CSGRayForms.hpp): each is convex, so the ray is inside it over one span, which spans() gives.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_ANALYTICRAYFORMS_HPP
#define EBGEOMETRY_ANALYTICRAYFORMS_HPP

// Our includes
#include "EBGeometry_AnalyticDistanceFunctions.hpp"
#include "EBGeometry_Ray.hpp"

namespace EBGeometry {

/**
 * @brief PlaneSDF's ray form: the crossing of the plane, where the ray goes from one side to the other. Exact, and callable on a device.
 * @tparam T Floating-point precision.
 */
template <class T>
struct RayForm<PlaneSDF<T>>
{
  /**
   * @brief The first crossing of the plane along a ray.
   * @param[in] a_shape   The plane.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options Unused: the crossing is found in closed form.
   * @return The crossing, with its outward normal, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  firstHit(
    const PlaneSDF<T>& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax, const RayOptions<T>& a_options) noexcept;

  /**
   * @brief Where a ray is inside the plane over an interval: the half-space behind it.
   * @param[in]  a_shape     The plane.
   * @param[in]  a_ray       Ray, with a unit direction.
   * @param[in]  a_tMin      Start of the interval.
   * @param[in]  a_tMax      End of the interval.
   * @param[in]  a_options   Unused: the span is found in closed form.
   * @param[in]  a_firstOnly Unused: the span costs no more than the first crossing.
   * @param[out] a_spans     The list.
   */
  EBGEOMETRY_HOST_DEVICE
  static inline void
  spans(const PlaneSDF<T>&   a_shape,
        const RayT<T>&       a_ray,
        T                    a_tMin,
        T                    a_tMax,
        const RayOptions<T>& a_options,
        bool                 a_firstOnly,
        RaySpansT<T>&        a_spans) noexcept;
};

/**
 * @brief SphereSDF's ray form: the roots of a quadratic. Exact, and callable on a device.
 * @tparam T Floating-point precision.
 */
template <class T>
struct RayForm<SphereSDF<T>>
{
  /**
   * @brief The first crossing of the sphere along a ray.
   * @param[in] a_shape   The sphere.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options Unused: the crossing is found in closed form.
   * @return The crossing, with its outward normal, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  firstHit(
    const SphereSDF<T>& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax, const RayOptions<T>& a_options) noexcept;

  /**
   * @brief Where a ray is inside the sphere over an interval: one span, the sphere being convex.
   * @param[in]  a_shape     The sphere.
   * @param[in]  a_ray       Ray, with a unit direction.
   * @param[in]  a_tMin      Start of the interval.
   * @param[in]  a_tMax      End of the interval.
   * @param[in]  a_options   Unused: the span is found in closed form.
   * @param[in]  a_firstOnly Unused: the span costs no more than the first crossing.
   * @param[out] a_spans     The list.
   */
  EBGEOMETRY_HOST_DEVICE
  static inline void
  spans(const SphereSDF<T>&  a_shape,
        const RayT<T>&       a_ray,
        T                    a_tMin,
        T                    a_tMax,
        const RayOptions<T>& a_options,
        bool                 a_firstOnly,
        RaySpansT<T>&        a_spans) noexcept;
};

/**
 * @brief BoxSDF's ray form: the slab test, axis by axis. Exact, and callable on a device.
 * @tparam T Floating-point precision.
 */
template <class T>
struct RayForm<BoxSDF<T>>
{
  /**
   * @brief The first crossing of the box along a ray.
   * @param[in] a_shape   The box.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options Unused: the crossing is found in closed form.
   * @return The crossing, with its outward normal, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  firstHit(const BoxSDF<T>& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax, const RayOptions<T>& a_options) noexcept;

  /**
   * @brief Where a ray is inside the box over an interval: one span, the box being convex.
   * @param[in]  a_shape     The box.
   * @param[in]  a_ray       Ray, with a unit direction.
   * @param[in]  a_tMin      Start of the interval.
   * @param[in]  a_tMax      End of the interval.
   * @param[in]  a_options   Unused: the span is found in closed form.
   * @param[in]  a_firstOnly Unused: the span costs no more than the first crossing.
   * @param[out] a_spans     The list.
   */
  EBGEOMETRY_HOST_DEVICE
  static inline void
  spans(const BoxSDF<T>&     a_shape,
        const RayT<T>&       a_ray,
        T                    a_tMin,
        T                    a_tMax,
        const RayOptions<T>& a_options,
        bool                 a_firstOnly,
        RaySpansT<T>&        a_spans) noexcept;
};

/**
 * @brief CylinderSDF's ray form: the infinite cylinder around its axis, clipped by the slab between its caps. Exact, and callable on a device.
 * @tparam T Floating-point precision.
 */
template <class T>
struct RayForm<CylinderSDF<T>>
{
  /**
   * @brief The first crossing of the cylinder along a ray.
   * @param[in] a_shape   The cylinder.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options Unused: the crossing is found in closed form.
   * @return The crossing, with its outward normal, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  firstHit(
    const CylinderSDF<T>& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax, const RayOptions<T>& a_options) noexcept;

  /**
   * @brief Where a ray is inside the cylinder over an interval: one span, the cylinder being convex.
   * @param[in]  a_shape     The cylinder.
   * @param[in]  a_ray       Ray, with a unit direction.
   * @param[in]  a_tMin      Start of the interval.
   * @param[in]  a_tMax      End of the interval.
   * @param[in]  a_options   Unused: the span is found in closed form.
   * @param[in]  a_firstOnly Unused: the span costs no more than the first crossing.
   * @param[out] a_spans     The list.
   */
  EBGEOMETRY_HOST_DEVICE
  static inline void
  spans(const CylinderSDF<T>& a_shape,
        const RayT<T>&        a_ray,
        T                     a_tMin,
        T                     a_tMax,
        const RayOptions<T>&  a_options,
        bool                  a_firstOnly,
        RaySpansT<T>&         a_spans) noexcept;
};

/**
 * @brief InfiniteCylinderSDF's ray form: the roots of a quadratic, in the plane across its axis. Exact, and callable on a device.
 * @tparam T Floating-point precision.
 */
template <class T>
struct RayForm<InfiniteCylinderSDF<T>>
{
  /**
   * @brief The first crossing of the cylinder along a ray.
   * @param[in] a_shape   The cylinder.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options Unused: the crossing is found in closed form.
   * @return The crossing, with its outward normal, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  firstHit(const InfiniteCylinderSDF<T>& a_shape,
           const RayT<T>&                a_ray,
           T                             a_tMin,
           T                             a_tMax,
           const RayOptions<T>&          a_options) noexcept;

  /**
   * @brief Where a ray is inside the cylinder over an interval: one span, the cylinder being convex.
   * @param[in]  a_shape     The cylinder.
   * @param[in]  a_ray       Ray, with a unit direction.
   * @param[in]  a_tMin      Start of the interval.
   * @param[in]  a_tMax      End of the interval.
   * @param[in]  a_options   Unused: the span is found in closed form.
   * @param[in]  a_firstOnly Unused: the span costs no more than the first crossing.
   * @param[out] a_spans     The list.
   */
  EBGEOMETRY_HOST_DEVICE
  static inline void
  spans(const InfiniteCylinderSDF<T>& a_shape,
        const RayT<T>&                a_ray,
        T                             a_tMin,
        T                             a_tMax,
        const RayOptions<T>&          a_options,
        bool                          a_firstOnly,
        RaySpansT<T>&                 a_spans) noexcept;
};

/**
 * @brief CapsuleSDF's ray form: the span of the cylinder between its centres and of the two balls at its ends, which, the capsule being convex, is one interval. Exact, and callable on a device.
 * @tparam T Floating-point precision.
 */
template <class T>
struct RayForm<CapsuleSDF<T>>
{
  /**
   * @brief The first crossing of the capsule along a ray.
   * @param[in] a_shape   The capsule.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options Unused: the crossing is found in closed form.
   * @return The crossing, with its outward normal, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  firstHit(
    const CapsuleSDF<T>& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax, const RayOptions<T>& a_options) noexcept;

  /**
   * @brief Where a ray is inside the capsule over an interval: one span, the capsule being convex.
   * @param[in]  a_shape     The capsule.
   * @param[in]  a_ray       Ray, with a unit direction.
   * @param[in]  a_tMin      Start of the interval.
   * @param[in]  a_tMax      End of the interval.
   * @param[in]  a_options   Unused: the span is found in closed form.
   * @param[in]  a_firstOnly Unused: the span costs no more than the first crossing.
   * @param[out] a_spans     The list.
   */
  EBGEOMETRY_HOST_DEVICE
  static inline void
  spans(const CapsuleSDF<T>& a_shape,
        const RayT<T>&       a_ray,
        T                    a_tMin,
        T                    a_tMax,
        const RayOptions<T>& a_options,
        bool                 a_firstOnly,
        RaySpansT<T>&        a_spans) noexcept;
};

} // namespace EBGeometry

#include "EBGeometry_AnalyticRayFormsImplem.hpp"

#endif
