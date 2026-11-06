// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_AnalyticRayForms.hpp
 * @brief  Ray forms (see RayForm) for the analytic shapes that have a closed-form crossing: PlaneSDF,
 * SphereSDF, BoxSDF, CylinderSDF, InfiniteCylinderSDF, CapsuleSDF, EllipsoidSDF, CappedConeSDF, PolygonPrismSDF,
 * TubeSDF, CutSphereSDF and WedgeSDF. With this header included, firstHit() and intersects() on these shapes are
 * exact instead of marched, and so are compositions of them (see EBGeometry_CSGRayForms.hpp). Each is convex, so
 * the ray is inside it over one span, which spans() gives, except the tube, whose hollow can split it in two.
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
 * @brief CapsuleSDF's ray form: the span of the cylinder between its centers and of the two balls at its ends, which, the capsule being convex, is one interval. Exact, and callable on a device.
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

/**
 * @brief EllipsoidSDF's ray form: the roots of a quadratic, in coordinates where the ellipsoid is the unit sphere. Exact, and callable on a device.
 * @tparam T Floating-point precision.
 */
template <class T>
struct RayForm<EllipsoidSDF<T>>
{
  /**
   * @brief The first crossing of the ellipsoid along a ray.
   * @param[in] a_shape   The ellipsoid.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options Unused: the crossing is found in closed form.
   * @return The crossing, with its outward normal, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  firstHit(
    const EllipsoidSDF<T>& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax, const RayOptions<T>& a_options) noexcept;

  /**
   * @brief Where a ray is inside the ellipsoid over an interval: one span, the ellipsoid being convex.
   * @param[in]  a_shape     The ellipsoid.
   * @param[in]  a_ray       Ray, with a unit direction.
   * @param[in]  a_tMin      Start of the interval.
   * @param[in]  a_tMax      End of the interval.
   * @param[in]  a_options   Unused: the spans are found in closed form.
   * @param[in]  a_firstOnly Unused: the spans cost no more than the first crossing.
   * @param[out] a_spans     The list.
   */
  EBGEOMETRY_HOST_DEVICE
  static inline void
  spans(const EllipsoidSDF<T>& a_shape,
        const RayT<T>&         a_ray,
        T                      a_tMin,
        T                      a_tMax,
        const RayOptions<T>&   a_options,
        bool                   a_firstOnly,
        RaySpansT<T>&          a_spans) noexcept;
};

/**
 * @brief CappedConeSDF's ray form: the roots of the cone's quadratic, clipped by the slab between its ends. Exact, and callable on a device.
 * @tparam T Floating-point precision.
 */
template <class T>
struct RayForm<CappedConeSDF<T>>
{
  /**
   * @brief The first crossing of the frustum along a ray.
   * @param[in] a_shape   The frustum.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options Unused: the crossing is found in closed form.
   * @return The crossing, with its outward normal, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  firstHit(
    const CappedConeSDF<T>& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax, const RayOptions<T>& a_options) noexcept;

  /**
   * @brief Where a ray is inside the frustum over an interval: one span, the frustum being convex.
   * @param[in]  a_shape     The frustum.
   * @param[in]  a_ray       Ray, with a unit direction.
   * @param[in]  a_tMin      Start of the interval.
   * @param[in]  a_tMax      End of the interval.
   * @param[in]  a_options   Unused: the spans are found in closed form.
   * @param[in]  a_firstOnly Unused: the spans cost no more than the first crossing.
   * @param[out] a_spans     The list.
   */
  EBGEOMETRY_HOST_DEVICE
  static inline void
  spans(const CappedConeSDF<T>& a_shape,
        const RayT<T>&          a_ray,
        T                       a_tMin,
        T                       a_tMax,
        const RayOptions<T>&    a_options,
        bool                    a_firstOnly,
        RaySpansT<T>&           a_spans) noexcept;
};

/**
 * @brief PolygonPrismSDF's ray form: the half-spaces of its sides, clipped by the slab between its ends. Exact, and callable on a device.
 * @tparam T Floating-point precision.
 */
template <class T>
struct RayForm<PolygonPrismSDF<T>>
{
  /**
   * @brief The first crossing of the prism along a ray.
   * @param[in] a_shape   The prism.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options Unused: the crossing is found in closed form.
   * @return The crossing, with its outward normal, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  firstHit(const PolygonPrismSDF<T>& a_shape,
           const RayT<T>&            a_ray,
           T                         a_tMin,
           T                         a_tMax,
           const RayOptions<T>&      a_options) noexcept;

  /**
   * @brief Where a ray is inside the prism over an interval: one span, the prism being convex.
   * @param[in]  a_shape     The prism.
   * @param[in]  a_ray       Ray, with a unit direction.
   * @param[in]  a_tMin      Start of the interval.
   * @param[in]  a_tMax      End of the interval.
   * @param[in]  a_options   Unused: the spans are found in closed form.
   * @param[in]  a_firstOnly Unused: the spans cost no more than the first crossing.
   * @param[out] a_spans     The list.
   */
  EBGEOMETRY_HOST_DEVICE
  static inline void
  spans(const PolygonPrismSDF<T>& a_shape,
        const RayT<T>&            a_ray,
        T                         a_tMin,
        T                         a_tMax,
        const RayOptions<T>&      a_options,
        bool                      a_firstOnly,
        RaySpansT<T>&             a_spans) noexcept;
};

/**
 * @brief TubeSDF's ray form: the outer cylinder's span less the inner cylinder's, which leaves up to two spans. Exact, and callable on a device.
 * @tparam T Floating-point precision.
 */
template <class T>
struct RayForm<TubeSDF<T>>
{
  /**
   * @brief The first crossing of the tube along a ray.
   * @param[in] a_shape   The tube.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options Unused: the crossing is found in closed form.
   * @return The crossing, with its outward normal, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  firstHit(
    const TubeSDF<T>& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax, const RayOptions<T>& a_options) noexcept;

  /**
   * @brief Where a ray is inside the tube over an interval: up to two spans, before and after the hollow.
   * @param[in]  a_shape     The tube.
   * @param[in]  a_ray       Ray, with a unit direction.
   * @param[in]  a_tMin      Start of the interval.
   * @param[in]  a_tMax      End of the interval.
   * @param[in]  a_options   Unused: the spans are found in closed form.
   * @param[in]  a_firstOnly Unused: the spans cost no more than the first crossing.
   * @param[out] a_spans     The list.
   */
  EBGEOMETRY_HOST_DEVICE
  static inline void
  spans(const TubeSDF<T>&    a_shape,
        const RayT<T>&       a_ray,
        T                    a_tMin,
        T                    a_tMax,
        const RayOptions<T>& a_options,
        bool                 a_firstOnly,
        RaySpansT<T>&        a_spans) noexcept;
};

/**
 * @brief CutSphereSDF's ray form: the sphere's span clipped by the half-space above the cut. Exact, and callable on a device.
 * @tparam T Floating-point precision.
 */
template <class T>
struct RayForm<CutSphereSDF<T>>
{
  /**
   * @brief The first crossing of the cut sphere along a ray.
   * @param[in] a_shape   The cut sphere.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options Unused: the crossing is found in closed form.
   * @return The crossing, with its outward normal, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  firstHit(
    const CutSphereSDF<T>& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax, const RayOptions<T>& a_options) noexcept;

  /**
   * @brief Where a ray is inside the cut sphere over an interval: one span, the cut sphere being convex.
   * @param[in]  a_shape     The cut sphere.
   * @param[in]  a_ray       Ray, with a unit direction.
   * @param[in]  a_tMin      Start of the interval.
   * @param[in]  a_tMax      End of the interval.
   * @param[in]  a_options   Unused: the spans are found in closed form.
   * @param[in]  a_firstOnly Unused: the spans cost no more than the first crossing.
   * @param[out] a_spans     The list.
   */
  EBGEOMETRY_HOST_DEVICE
  static inline void
  spans(const CutSphereSDF<T>& a_shape,
        const RayT<T>&         a_ray,
        T                      a_tMin,
        T                      a_tMax,
        const RayOptions<T>&   a_options,
        bool                   a_firstOnly,
        RaySpansT<T>&          a_spans) noexcept;
};

/**
 * @brief WedgeSDF's ray form: the intersection of the half-spaces behind its two faces. Exact, and callable on a device.
 * @tparam T Floating-point precision.
 */
template <class T>
struct RayForm<WedgeSDF<T>>
{
  /**
   * @brief The first crossing of the wedge along a ray.
   * @param[in] a_shape   The wedge.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options Unused: the crossing is found in closed form.
   * @return The crossing, with its outward normal, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  firstHit(
    const WedgeSDF<T>& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax, const RayOptions<T>& a_options) noexcept;

  /**
   * @brief Where a ray is inside the wedge over an interval: one span, the wedge being convex.
   * @param[in]  a_shape     The wedge.
   * @param[in]  a_ray       Ray, with a unit direction.
   * @param[in]  a_tMin      Start of the interval.
   * @param[in]  a_tMax      End of the interval.
   * @param[in]  a_options   Unused: the spans are found in closed form.
   * @param[in]  a_firstOnly Unused: the spans cost no more than the first crossing.
   * @param[out] a_spans     The list.
   */
  EBGEOMETRY_HOST_DEVICE
  static inline void
  spans(const WedgeSDF<T>&   a_shape,
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
