// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_RayImplem.hpp
 * @brief  Implementation of EBGeometry_Ray.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_RAYIMPLEM_HPP
#define EBGEOMETRY_RAYIMPLEM_HPP

// Std includes
#include <cmath>
#include <type_traits>
#include <utility>

// Our includes
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Ray.hpp"

namespace EBGeometry {

namespace RayDetail {

/**
 * @brief Whether a shape names the leaf at a point (Tape::primitiveAt()).
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T, class = void>
struct HasPrimitiveAt : std::false_type
{
};

/**
 * @brief Whether a shape names the leaf at a point (Tape::primitiveAt()).
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T>
struct HasPrimitiveAt<S,
                      T,
                      std::void_t<decltype(std::declval<const S&>().primitiveAt(std::declval<const Vec3T<T>&>()))>>
  : std::true_type
{
};

/**
 * @brief Whether a shape names the closest face to a point (the mesh SDFs' getClosestFace()).
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T, class = void>
struct HasClosestFace : std::false_type
{
};

/**
 * @brief Whether a shape names the closest face to a point (the mesh SDFs' getClosestFace()).
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T>
struct HasClosestFace<
  S,
  T,
  std::void_t<decltype(std::declval<const S&>().getClosestFace(std::declval<const Vec3T<T>&>()).faceId)>>
  : std::true_type
{
};

/**
 * @brief Whether a shape reports its distance quality at run time (Tape::getDistanceQuality()).
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 */
template <class S, class = void>
struct HasRuntimeQuality : std::false_type
{
};

/**
 * @brief Whether a shape reports its distance quality at run time (Tape::getDistanceQuality()).
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 */
template <class S>
struct HasRuntimeQuality<S, std::void_t<decltype(std::declval<const S&>().getDistanceQuality())>> : std::true_type
{
};

/**
 * @brief A shape's distance quality: its run-time one if it has one, its static one otherwise.
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 * @param[in] a_shape Shape.
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
 * @param[in] a_shape Shape.
 * @param[in] a_point Point.
 * @return The leaf, the closest face, or 0.
 */
template <class T, class S>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline uint32_t
primitiveOf(const S& a_shape, const Vec3T<T>& a_point) noexcept
{
  if constexpr (HasPrimitiveAt<S, T>::value) {
    return a_shape.primitiveAt(a_point);
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
 * @details Internal helper; not part of the public API. Handles unbounded boxes and directions parallel
 * to a face.
 * @tparam T Floating-point precision.
 * @param[in]     a_lo  Low corner.
 * @param[in]     a_hi  High corner.
 * @param[in]     a_ray Ray.
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

      T tLo = (a_lo[dir] - o) * inv;
      T tHi = (a_hi[dir] - o) * inv;

      if (tLo > tHi) {
        const T swap = tLo;

        tLo = tHi;
        tHi = swap;
      }

      a_t0 = Math::max(a_t0, tLo);
      a_t1 = Math::min(a_t1, tHi);
    }
  }

  return a_t0 <= a_t1;
}

/**
 * @brief The length scale of a query (see RayOptions).
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_lo      Low corner of the shape's box.
 * @param[in] a_hi      High corner.
 * @param[in] a_options Options.
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

  const auto box = a_shape.computeBoundingVolume();
  const T    eps = a_options.m_tolerance * RayDetail::scaleOf(box.getLowCorner(), box.getHighCorner(), a_options);

  HitT<T> hit;

  // Only the part of the ray inside the box (widened by the tolerance) can cross the surface.
  const Vec3T<T> pad = eps * Vec3T<T>::ones();

  T t0 = a_tMin;
  T t1 = a_tMax;

  if (!(a_tMin <= a_tMax) || !RayDetail::clip(box.getLowCorner() - pad, box.getHighCorner() + pad, a_ray, t0, t1)) {
    return hit;
  }

  const auto value = [&a_shape, &a_ray](const T a_t) noexcept -> T { return a_shape.signedDistance(a_ray.at(a_t)); };

  const auto found = [&a_shape, &a_ray, &hit](const T a_t, const bool a_entering) noexcept -> HitT<T> {
    hit.m_status    = HitStatus::Hit;
    hit.m_t         = a_t;
    hit.m_point     = a_ray.at(a_t);
    hit.m_primitive = RayDetail::primitiveOf(a_shape, hit.m_point);
    hit.m_entering  = a_entering;

    return hit;
  };

  // The crossing between a and b, where the value changes sign: halve the interval until it is shorter
  // than the tolerance. a keeps the starting side's sign.
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

  // Starting on the surface: the side the ray goes to decides which way it crosses.
  if (std::abs(v0) < eps) {
    return found(t0, value(Math::min(t0 + T(2) * eps, t1)) < T(0));
  }

  const bool inside = v0 < T(0);

  T t     = t0;
  T tPrev = t0;

  if (RayDetail::qualityOf(a_shape) != DistanceQuality::NotADistance) {
    // Sphere tracing: |value| is at most the distance to the surface, so a step of |value| cannot cross it.
    for (uint32_t step = 0; step < a_options.m_maxSteps; step++) {
      const T v = (step == 0U) ? v0 : value(t);

      if ((v < T(0)) != inside) {
        // A value that is not quite a bound (rounding, say) stepped across: find the crossing.
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
    // Fixed steps, then bisection on the first change of sign.
    const T h = a_options.m_step * RayDetail::scaleOf(box.getLowCorner(), box.getHighCorner(), a_options);

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

template <class T, class S>
EBGEOMETRY_HOST_DEVICE
inline HitStatus
intersects(const S& a_shape, const Vec3T<T>& a_from, const Vec3T<T>& a_to, const RayOptions<T>& a_options) noexcept
{
  const T a = a_shape.signedDistance(a_from);
  const T b = a_shape.signedDistance(a_to);

  if ((a < T(0)) != (b < T(0)) || a == T(0) || b == T(0)) {
    return HitStatus::Hit;
  }

  const Vec3T<T> d = a_to - a_from;
  const T        L = d.length();

  if (!(L > T(0))) {
    return HitStatus::Miss;
  }

  const RayT<T> ray{a_from, d / L};

  return firstHit(a_shape, ray, T(0), L, a_options).m_status;
}

} // namespace EBGeometry

#endif
