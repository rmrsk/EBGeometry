// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_SphereCloudRayForm.hpp
 * @brief  The ray form (see RayForm) of SphereCloudBVH: its BVH walked along the ray (PackedBVH::traverse() with
 * BVH::RayQuery), each sphere crossed exactly (SphereAoSoA::rayIntervals()), and the spheres' spans united, so that
 * where spheres overlap the ray crosses only the surface of their union. With this header included, firstHit() on a
 * sphere cloud is exact instead of marched, and so are the compositions it is part of, through spans().
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_SPHERECLOUDRAYFORM_HPP
#define EBGEOMETRY_SPHERECLOUDRAYFORM_HPP

// Std includes
#include <cstddef>
#include <cstdint>

// Our includes
#include "EBGeometry_Ray.hpp"
#include "EBGeometry_SphereCloudBVH.hpp"

namespace EBGeometry {

/**
 * @brief SphereCloudBVH's ray form: the union of the spans of the spheres the ray passes through, found by walking
 * the BVH nearest box first and leaving out the boxes beyond what the union so far is known up to. Exact, and
 * callable on a device.
 * @details As BVHUnion's ray form, with a sphere for a member: unless the ray starts inside a sphere, only the
 * first crossings are needed, and the walk stops at the first box beyond the nearest. A ray that starts inside
 * leaves the union where no sphere holds it any more, not where it leaves the first sphere. HitT::m_primitive is
 * the cloud index of the sphere crossed, and HitT::m_normal its outward normal there. A sphere of radius 0 is
 * crossed by no ray, except one through its center, which touches it.
 * @tparam T Floating-point precision.
 * @tparam K BVH branching factor.
 * @tparam W Spheres per leaf group.
 */
template <class T, size_t K, size_t W>
struct RayForm<SphereCloudBVH<T, K, W>>
{
  /**
   * @brief The first crossing of the union of the spheres along a ray.
   * @param[in] a_shape   The cloud.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options Unused: the crossings are found exactly.
   * @return The crossing, with its sphere's cloud index and outward normal, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  firstHit(const SphereCloudBVH<T, K, W>& a_shape,
           const RayT<T>&                 a_ray,
           T                              a_tMin,
           T                              a_tMax,
           const RayOptions<T>&           a_options) noexcept;

  /**
   * @brief The first crossing of the union along a ray, without the normal or sphere there: the trace of
   * firstHit() without asking the sphere it ends on.
   * @param[in] a_shape   The cloud.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options Unused: the crossings are found exactly.
   * @return The crossing, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline CrossingT<T>
  firstCrossing(const SphereCloudBVH<T, K, W>& a_shape,
                const RayT<T>&                 a_ray,
                T                              a_tMin,
                T                              a_tMax,
                const RayOptions<T>&           a_options) noexcept;

  /**
   * @brief Where a ray is inside the union over an interval, each crossing named by its sphere's cloud index.
   * @param[in]  a_shape     The cloud.
   * @param[in]  a_ray       Ray, with a unit direction.
   * @param[in]  a_tMin      Start of the interval.
   * @param[in]  a_tMax      End of the interval.
   * @param[in]  a_options   Unused: the crossings are found exactly.
   * @param[in]  a_firstOnly Whether only the first crossing is needed.
   * @param[out] a_spans     The list.
   */
  EBGEOMETRY_HOST_DEVICE
  static inline void
  spans(const SphereCloudBVH<T, K, W>& a_shape,
        const RayT<T>&                 a_ray,
        T                              a_tMin,
        T                              a_tMax,
        const RayOptions<T>&           a_options,
        bool                           a_firstOnly,
        RaySpansT<T>&                  a_spans) noexcept;

  /**
   * @brief The crossing at a distance along a ray on one of the spheres.
   * @param[in] a_shape   The cloud.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_t       The crossing's distance.
   * @param[in] a_sphere  The sphere's cloud index, as spans() names it.
   * @param[in] a_options Unused.
   * @return The crossing, with the sphere's cloud index and its outward normal there.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  leafHit(const SphereCloudBVH<T, K, W>& a_shape,
          const RayT<T>&                 a_ray,
          T                              a_t,
          uint32_t                       a_sphere,
          const RayOptions<T>&           a_options) noexcept;
};

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
RayForm<SphereCloudBVH<T, K, W>>::firstHit(const SphereCloudBVH<T, K, W>& a_shape,
                                           const RayT<T>&                 a_ray,
                                           const T                        a_tMin,
                                           const T                        a_tMax,
                                           const RayOptions<T>&           a_options) noexcept
{
  const auto trace =
    [&a_shape, &a_ray, &a_options](const T a_t0, const T a_t1, const bool a_all, RaySpansT<T>& a_spans) noexcept {
      spans(a_shape, a_ray, a_t0, a_t1, a_options, !a_all, a_spans);
    };

  const auto resolve = [&a_shape, &a_ray, &a_options](const T a_t, const uint32_t a_sphere) noexcept {
    return leafHit(a_shape, a_ray, a_t, a_sphere, a_options);
  };

  return RayDetail::traceSpans(trace, resolve, a_ray, a_tMin, a_tMax);
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline CrossingT<T>
RayForm<SphereCloudBVH<T, K, W>>::firstCrossing(const SphereCloudBVH<T, K, W>& a_shape,
                                                const RayT<T>&                 a_ray,
                                                const T                        a_tMin,
                                                const T                        a_tMax,
                                                const RayOptions<T>&           a_options) noexcept
{
  const auto trace =
    [&a_shape, &a_ray, &a_options](const T a_t0, const T a_t1, const bool a_all, RaySpansT<T>& a_spans) noexcept {
      spans(a_shape, a_ray, a_t0, a_t1, a_options, !a_all, a_spans);
    };

  return RayDetail::traceCrossing(trace, a_ray, a_tMin, a_tMax);
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline void
RayForm<SphereCloudBVH<T, K, W>>::spans(const SphereCloudBVH<T, K, W>& a_shape,
                                        const RayT<T>&                 a_ray,
                                        const T                        a_tMin,
                                        const T                        a_tMax,
                                        const RayOptions<T>& /*a_options*/,
                                        const bool    a_firstOnly,
                                        RaySpansT<T>& a_spans) noexcept
{
  a_spans.reset(a_tMax);

  if (a_shape.numSpheres() == 0U) {
    return;
  }

  const auto bvh    = a_shape.getBVH();
  const auto groups = bvh.getPrimitives();

  // One sphere's span at a time, united into the list. Each is cut to what the union so far is known up to, beyond
  // which it can change nothing known.
  RaySpansT<T> operand;

  const auto leaf = [&groups, &a_ray, a_tMin, a_firstOnly, &operand](
                      RaySpansT<T>& a_union, const size_t a_offset, const size_t a_count) noexcept {
    for (size_t g = a_offset; g < a_offset + a_count; g++) {
      const auto& group = groups[static_cast<uint32_t>(g)];

      Array<T, W> entry;
      Array<T, W> exit;

      group.rayIntervals(a_ray.m_origin, a_ray.m_direction, entry, exit);

      for (uint32_t lane = 0; lane < W; lane++) {
        if (entry[lane] <= exit[lane] && exit[lane] >= a_tMin && entry[lane] <= a_union.m_horizon) {
          operand.reset(a_union.m_horizon);
          operand.addClipped(entry[lane], exit[lane], a_tMin, a_union.m_horizon);

          RayDetail::nameLeaf(group.getSphereId(lane), operand);
          RayDetail::uniteInto(a_union, operand);

          if (a_firstOnly) {
            RayDetail::keepFirst(a_union);
          }
        }
      }
    }
  };

  // A box the ray enters beyond what the union is known up to can change nothing known.
  const auto bound = [](const RaySpansT<T>& a_union) noexcept -> T { return a_union.m_horizon; };

  bvh.traverse(BVH::RayQuery<T>(a_ray.m_origin, a_ray.m_direction, a_tMin), a_spans, leaf, bound);
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
RayForm<SphereCloudBVH<T, K, W>>::leafHit(const SphereCloudBVH<T, K, W>& a_shape,
                                          const RayT<T>&                 a_ray,
                                          const T                        a_t,
                                          const uint32_t                 a_sphere,
                                          const RayOptions<T>& /*a_options*/) noexcept
{
  const SphereCloud::Sphere<T> sphere = a_shape.sphere(a_sphere);

  HitT<T> hit;

  hit.m_status    = HitStatus::Hit;
  hit.m_t         = a_t;
  hit.m_point     = a_ray.at(a_t);
  hit.m_primitive = a_sphere;
  hit.m_normal    = (sphere.m_radius > T(0)) ? (hit.m_point - sphere.m_center) / sphere.m_radius : Vec3T<T>::zeros();
  hit.m_entering  = dot(a_ray.m_direction, hit.m_normal) < T(0);
  hit.m_exact     = true;

  return hit;
}

} // namespace EBGeometry

#endif
