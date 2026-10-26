// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_RayImplem.hpp
 * @brief  Implementation of EBGeometry_Ray.hpp.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_RAYIMPLEM_HPP
#define EBGEOMETRY_RAYIMPLEM_HPP

// Std includes
#include <cmath>
#include <cstddef>
#include <type_traits>
#include <utility>

// Our includes
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Ray.hpp"

namespace EBGeometry {

namespace RayDetail {

/**
 * @brief Whether a shape names the face closest to a point (the mesh SDFs' getClosestFace()).
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T, class = void>
struct HasClosestFace : std::false_type
{
};

/**
 * @brief A shape with getClosestFace(point).faceId.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T>
struct HasClosestFace<S,
                      T,
                      std::void_t<decltype(std::declval<const S&>().getClosestFace(std::declval<Vec3T<T>>()).faceId)>>
  : std::true_type
{
};

/**
 * @brief Whether a shape names the deciding leaf at a point (Tape::closestPrimitive()).
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T, class = void>
struct HasClosestPrimitive : std::false_type
{
};

/**
 * @brief A shape with closestPrimitive(point).m_leaf.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T>
struct HasClosestPrimitive<
  S,
  T,
  std::void_t<decltype(std::declval<const S&>().closestPrimitive(std::declval<Vec3T<T>>()).m_leaf)>> : std::true_type
{
};

/**
 * @brief Whether a shape knows its distance quality only at run time (Tape::getDistanceQuality()).
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 */
template <class S, class = void>
struct HasRuntimeQuality : std::false_type
{
};

/**
 * @brief A shape with getDistanceQuality().
 * @tparam S Shape type.
 */
template <class S>
struct HasRuntimeQuality<S, std::void_t<decltype(std::declval<const S&>().getDistanceQuality())>> : std::true_type
{
};

/**
 * @brief Whether a shape has a ray form: its RayForm has a static firstHit() (see RayForm).
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T, class = void>
struct HasRayFirstHit : std::false_type
{
};

/**
 * @brief A ray form with a static firstHit() returning HitT<T>.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T>
struct HasRayFirstHit<
  S,
  T,
  std::enable_if_t<std::is_same_v<decltype(RayForm<S>::firstHit(std::declval<const S&>(),
                                                                std::declval<const RayT<T>&>(),
                                                                std::declval<T>(),
                                                                std::declval<T>(),
                                                                std::declval<const RayOptions<T>&>())),
                                  HitT<T>>>> : std::true_type
{
};

/**
 * @brief Whether a shape's ray form has its own intersects().
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T, class = void>
struct HasRayIntersects : std::false_type
{
};

/**
 * @brief A ray form with a static intersects() returning HitStatus.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T>
struct HasRayIntersects<
  S,
  T,
  std::enable_if_t<std::is_same_v<decltype(RayForm<S>::intersects(std::declval<const S&>(),
                                                                  std::declval<const RayT<T>&>(),
                                                                  std::declval<T>(),
                                                                  std::declval<T>(),
                                                                  std::declval<const RayOptions<T>&>())),
                                  HitStatus>>> : std::true_type
{
};

/**
 * @brief Whether a shape's ray form has its own spans().
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T, class = void>
struct HasRaySpans : std::false_type
{
};

/**
 * @brief A ray form with a static spans() writing a RaySpansT<T>.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T>
struct HasRaySpans<S,
                   T,
                   std::void_t<decltype(RayForm<S>::spans(std::declval<const S&>(),
                                                          std::declval<const RayT<T>&>(),
                                                          std::declval<T>(),
                                                          std::declval<T>(),
                                                          std::declval<const RayOptions<T>&>(),
                                                          std::declval<bool>(),
                                                          std::declval<RaySpansT<T>&>()))>> : std::true_type
{
};

/**
 * @brief A shape's distance quality: its run-time one if it has one, its static one otherwise.
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 * @param[in] a_shape The shape.
 * @return The quality.
 */
template <class S>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline DistanceQuality
qualityOf(const S& a_shape) noexcept
{
  if constexpr (HasRuntimeQuality<S>::value) {
    return a_shape.getDistanceQuality();
  }
  else {
    return distanceQualityOf<S>;
  }
}

/**
 * @brief The finest element a shape can name at a point (see HitT::m_primitive).
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in] a_shape The shape.
 * @param[in] a_point The point.
 * @return The deciding leaf, the closest face, or 0.
 */
template <class T, class S>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline uint32_t
primitiveOf(const S& a_shape, const Vec3T<T>& a_point) noexcept
{
  if constexpr (HasClosestPrimitive<S, T>::value) {
    return a_shape.closestPrimitive(a_point).m_leaf;
  }
  else if constexpr (HasClosestFace<S, T>::value) {
    return a_shape.getClosestFace(a_point).faceId;
  }
  else {
    return 0U;
  }
}

/**
 * @brief Clip a ray's interval to a box.
 * @details Internal helper; not part of the public API. Handles unbounded boxes and directions parallel to
 * a face.
 * @tparam T Floating-point precision.
 * @param[in]     a_lo  Lower corner.
 * @param[in]     a_hi  Upper corner.
 * @param[in]     a_ray The ray.
 * @param[in,out] a_t0  Start of the interval.
 * @param[in,out] a_t1  End of the interval.
 * @return Whether anything of the interval is left.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline bool
clip(const Vec3T<T>& a_lo, const Vec3T<T>& a_hi, const RayT<T>& a_ray, T& a_t0, T& a_t1) noexcept
{
  for (size_t dir = 0; dir < 3; dir++) {
    const T o = a_ray.m_origin[dir];
    const T d = a_ray.m_direction[dir];

    if (!(a_lo[dir] <= a_hi[dir])) {
      return false;
    }

    if (d == T(0)) {
      if (o < a_lo[dir] || o > a_hi[dir]) {
        return false;
      }
    }
    else {
      const T inv = T(1) / d;

      T tNear = (a_lo[dir] - o) * inv;
      T tFar  = (a_hi[dir] - o) * inv;

      if (tNear > tFar) {
        const T swap = tNear;

        tNear = tFar;
        tFar  = swap;
      }

      a_t0 = Math::max(a_t0, tNear);
      a_t1 = Math::min(a_t1, tFar);
    }
  }

  return a_t0 <= a_t1;
}

/**
 * @brief The length scale of a query (see RayOptions).
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_lo      Lower corner of the shape's box.
 * @param[in] a_hi      Upper corner of the shape's box.
 * @param[in] a_options How to march.
 * @return The scale.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline T
scaleOf(const Vec3T<T>& a_lo, const Vec3T<T>& a_hi, const RayOptions<T>& a_options) noexcept
{
  if (a_options.m_scale > T(0)) {
    return a_options.m_scale;
  }

  const T diagonal = (a_hi - a_lo).length();

  return (std::isfinite(diagonal) && diagonal > T(0)) ? diagonal : T(1);
}

/**
 * @brief The outward unit normal of a shape's surface at a point: its normalised gradient, by central
 * differences of step a_h.
 * @details Internal helper; not part of the public API. Zero where the gradient vanishes.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in] a_shape The shape.
 * @param[in] a_point The point.
 * @param[in] a_h     Step.
 * @return The normal.
 */
template <class T, class S>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline Vec3T<T>
gradientNormal(const S& a_shape, const Vec3T<T>& a_point, const T a_h) noexcept
{
  Vec3T<T> gradient = Vec3T<T>::zeros();

  for (size_t dir = 0; dir < 3; dir++) {
    Vec3T<T> step = Vec3T<T>::zeros();

    step[dir] = a_h;

    gradient[dir] = a_shape.signedDistance(a_point + step) - a_shape.signedDistance(a_point - step);
  }

  const T length = gradient.length();

  return (length > T(0)) ? gradient / length : Vec3T<T>::zeros();
}

/**
 * @brief The first crossing of a shape's surface along a ray, by marching its signed distance: sphere
 * tracing where the value is a distance or a bound on one, fixed steps and bisection where it is not.
 * @details Internal helper; not part of the public API. What firstHit() does for a shape with no ray form.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in] a_shape   The shape.
 * @param[in] a_ray     The ray, with a unit direction.
 * @param[in] a_tMin    Start of the interval.
 * @param[in] a_tMax    End of the interval.
 * @param[in] a_options How to march.
 * @return The crossing, a miss, or an unresolved query.
 */
template <class T, class S>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline HitT<T>
march(const S& a_shape, const RayT<T>& a_ray, const T a_tMin, const T a_tMax, const RayOptions<T>& a_options) noexcept
{
  EBGEOMETRY_EXPECT(std::abs(a_ray.m_direction.length() - T(1)) < T(1e-3));

  const auto box   = a_shape.computeBoundingVolume();
  const T    scale = RayDetail::scaleOf(box.getLowCorner(), box.getHighCorner(), a_options);
  const T    eps   = a_options.m_tolerance * scale;

  HitT<T> hit;

  const Vec3T<T> pad = eps * Vec3T<T>::ones();

  T t0 = a_tMin;
  T t1 = a_tMax;

  if (!(a_tMin <= a_tMax) || !RayDetail::clip(box.getLowCorner() - pad, box.getHighCorner() + pad, a_ray, t0, t1)) {
    return hit;
  }

  const auto value = [&a_shape, &a_ray](const T a_t) noexcept -> T { return a_shape.signedDistance(a_ray.at(a_t)); };

  const auto found = [&a_shape, &a_ray, &hit, eps](const T a_t, const bool a_entering) noexcept -> HitT<T> {
    hit.m_status    = HitStatus::Hit;
    hit.m_t         = a_t;
    hit.m_point     = a_ray.at(a_t);
    hit.m_primitive = RayDetail::primitiveOf(a_shape, hit.m_point);
    hit.m_normal    = RayDetail::gradientNormal(a_shape, hit.m_point, eps);
    hit.m_entering  = a_entering;

    return hit;
  };

  const auto bisect = [&value, eps](T a_a, T a_b, const bool a_inside) noexcept -> T {
    for (int i = 0; i < 64 && a_b - a_a > eps; i++) {
      const T mid = T(0.5) * (a_a + a_b);

      if ((value(mid) < T(0)) == a_inside) {
        a_a = mid;
      }
      else {
        a_b = mid;
      }
    }

    return T(0.5) * (a_a + a_b);
  };

  const T v0 = value(t0);

  if (std::abs(v0) < eps) {
    return found(t0, value(Math::min(t0 + T(2) * eps, t1)) < v0);
  }

  const bool inside = v0 < T(0);

  T t     = t0;
  T tPrev = t0;

  if (RayDetail::qualityOf(a_shape) != DistanceQuality::NotADistance) {
    for (uint32_t step = 0; step < a_options.m_maxSteps; step++) {
      const T v = (step == 0U) ? v0 : value(t);

      if ((v < T(0)) != inside) {
        return found(bisect(tPrev, t, inside), !inside);
      }

      if (std::abs(v) < eps) {
        return found(t, !inside);
      }

      tPrev = t;
      t += std::abs(v);

      if (t > t1) {
        return hit;
      }
    }
  }
  else {
    const T h = a_options.m_step * scale;

    for (uint32_t step = 0; step < a_options.m_maxSteps; step++) {
      tPrev = t;
      t     = Math::min(t + h, t1);

      if ((value(t) < T(0)) != inside) {
        return found(bisect(tPrev, t, inside), !inside);
      }

      if (t >= t1) {
        return hit;
      }
    }
  }

  hit.m_status = HitStatus::Unresolved;
  hit.m_t      = t;
  hit.m_point  = a_ray.at(t);

  return hit;
}

} // namespace RayDetail

template <class T>
EBGEOMETRY_HOST_DEVICE
inline Vec3T<T>
RayT<T>::at(const T a_t) const noexcept
{
  return m_origin + a_t * m_direction;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline bool
HitT<T>::hit() const noexcept
{
  return m_status == HitStatus::Hit;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline bool
SpanEndT<T>::crossing() const noexcept
{
  return m_kind != Open;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
RaySpansT<T>::reset(const T a_horizon) noexcept
{
  m_count   = 0;
  m_horizon = a_horizon;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
RaySpansT<T>::add(const SpanEndT<T>& a_in, const SpanEndT<T>& a_out) noexcept
{
  if (m_count < Capacity) {
    m_spans[m_count].m_in  = a_in;
    m_spans[m_count].m_out = a_out;

    m_count++;
  }
  else {
    m_horizon = Math::min(m_horizon, a_in.m_t);
  }
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
RaySpansT<T>::addClipped(const T         a_in,
                         const Vec3T<T>& a_inNormal,
                         const T         a_out,
                         const Vec3T<T>& a_outNormal,
                         const uint32_t  a_primitive,
                         const T         a_t0,
                         const T         a_t1) noexcept
{
  if (!(a_in <= a_out) || a_out < a_t0 || a_in > a_t1) {
    return;
  }

  // An end outside the interval, or at infinity (a half-space), is no crossing in it.
  const bool inOpen  = a_in < a_t0 || !(a_in > -Math::Limits<T>::infinity());
  const bool outOpen = a_out > a_t1 || !(a_out < Math::Limits<T>::infinity());

  const SpanEndT<T> in  = inOpen ? SpanEndT<T>{a_t0, Vec3T<T>::zeros(), 0U, SpanEndT<T>::Open}
                                 : SpanEndT<T>{a_in, a_inNormal, a_primitive, SpanEndT<T>::Exact};
  const SpanEndT<T> out = outOpen ? SpanEndT<T>{a_t1, Vec3T<T>::zeros(), 0U, SpanEndT<T>::Open}
                                  : SpanEndT<T>{a_out, a_outNormal, a_primitive, SpanEndT<T>::Exact};

  this->add(in, out);
}

template <class T, class S>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
firstHit(const S&                       a_shape,
         const RayT<T>&                 a_ray,
         const typename RayT<T>::Scalar a_tMin,
         const typename RayT<T>::Scalar a_tMax,
         const RayOptions<T>&           a_options) noexcept
{
  EBGEOMETRY_EXPECT(std::abs(a_ray.m_direction.length() - T(1)) < T(1e-3));

  if constexpr (RayDetail::HasRayFirstHit<S, T>::value) {
    if (!a_options.m_march) {
      return RayForm<S>::firstHit(a_shape, a_ray, a_tMin, a_tMax, a_options);
    }
  }

  return RayDetail::march(a_shape, a_ray, a_tMin, a_tMax, a_options);
}

template <class T, class S>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
firstHit(const S&             a_shape,
         const Vec3T<T>&      a_from,
         const Vec3T<T>&      a_to,
         const T              a_tMin,
         const RayOptions<T>& a_options) noexcept
{
  const Vec3T<T> span   = a_to - a_from;
  const T        length = span.length();

  if (!(length > T(0))) {
    return HitT<T>();
  }

  return firstHit(a_shape, RayT<T>{a_from, span / length}, a_tMin, length, a_options);
}

template <class T, class S>
EBGEOMETRY_HOST_DEVICE
inline HitStatus
intersects(const S&                       a_shape,
           const RayT<T>&                 a_ray,
           const typename RayT<T>::Scalar a_tMin,
           const typename RayT<T>::Scalar a_tMax,
           const RayOptions<T>&           a_options) noexcept
{
  if constexpr (RayDetail::HasRayFirstHit<S, T>::value && RayDetail::HasRayIntersects<S, T>::value) {
    if (!a_options.m_march) {
      return RayForm<S>::intersects(a_shape, a_ray, a_tMin, a_tMax, a_options);
    }
  }

  return firstHit(a_shape, a_ray, a_tMin, a_tMax, a_options).m_status;
}

template <class T, class S>
EBGEOMETRY_HOST_DEVICE
inline HitStatus
intersects(const S&             a_shape,
           const Vec3T<T>&      a_from,
           const Vec3T<T>&      a_to,
           const T              a_tMin,
           const RayOptions<T>& a_options) noexcept
{
  const Vec3T<T> span   = a_to - a_from;
  const T        length = span.length();

  if (!(length > T(0))) {
    return HitStatus::Miss;
  }

  const Vec3T<T> direction = span / length;

  // A marched shape: different signs at the two ends mean the segment crosses the surface, with no search.
  // A shape with a ray form searches instead: its search is cheaper than two values, and exact where the
  // sign is not (an open mesh).
  if (!RayDetail::HasRayFirstHit<S, T>::value || a_options.m_march) {
    if ((a_shape.signedDistance(a_from + a_tMin * direction) < T(0)) != (a_shape.signedDistance(a_to) < T(0))) {
      return HitStatus::Hit;
    }
  }

  return intersects(a_shape, RayT<T>{a_from, direction}, a_tMin, length, a_options);
}

} // namespace EBGeometry

#endif
