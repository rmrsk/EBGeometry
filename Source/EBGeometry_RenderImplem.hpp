// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_RenderImplem.hpp
 * @brief  Implementation of EBGeometry_Render.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_RENDERIMPLEM_HPP
#define EBGEOMETRY_RENDERIMPLEM_HPP

// Std includes
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <numeric>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_Constants.hpp"
#include "EBGeometry_GPUTransform.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_RegionView.hpp"
#include "EBGeometry_Render.hpp"

namespace EBGeometry::Render {

namespace RenderDetail {

/**
 * @brief A vector scaled to unit length.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_v Vector (not zero).
 * @return a_v / |a_v|.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline Vec3T<T>
unit(const Vec3T<T>& a_v) noexcept
{
  return a_v / a_v.length();
}

/**
 * @brief A shape cut by a half-space: what the renderer marches.
 * @details Internal helper; not part of the public API. With the cut on, the value is the larger of the
 * shape's and the distance to the plane (positive on the side the normal points to), which keeps the
 * shape's distance quality.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 */
/**
 * @brief Whether a shape reports its distance quality at run time (getDistanceQuality(): a Tape, a view of its
 * regions): the primary template is false.
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 */
template <class S, class = void>
struct HasDistanceQuality : std::false_type
{
};

/**
 * @brief A shape with getDistanceQuality().
 * @tparam S Shape type.
 */
template <class S>
struct HasDistanceQuality<S, std::void_t<decltype(std::declval<const S&>().getDistanceQuality())>> : std::true_type
{
};

/**
 * @brief The distance quality of a shape: its own at run time if it reports one, else its type's.
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
  if constexpr (HasDistanceQuality<S>::value) {
    return a_shape.getDistanceQuality();
  }
  else {
    (void)a_shape;

    return DistanceQualityOf<S>::value;
  }
}

/**
 * @brief Whether a shape names what a point is in, for the section plane's fill (sectionKey()): the primary template is
 * false.
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T, class = void>
struct HasSectionKey : std::false_type
{
};

/**
 * @brief A shape with sectionKey(point).
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T>
struct HasSectionKey<S, T, std::void_t<decltype(std::declval<const S&>().sectionKey(std::declval<Vec3T<T>>()))>>
  : std::true_type
{
};

/**
 * @brief Whether a shape has surfaces seen through that a renderer draws translucent (firstSeenThrough()): the primary
 * template is false.
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T, class = void>
struct HasSeenThrough : std::false_type
{
};

/**
 * @brief A shape with firstSeenThrough(ray, tMin, tMax, options).
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T>
struct HasSeenThrough<S,
                      T,
                      std::void_t<decltype(std::declval<const S&>().firstSeenThrough(
                        std::declval<RayT<T>>(), T(0), T(0), std::declval<RayOptions<T>>()))>> : std::true_type
{
};

/**
 * @brief Whether a shape has a field of its own for the section plane (contourSample(), negative inside, with lines that
 * may be secondary): the primary template is false.
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T, class = void>
struct HasContourSample : std::false_type
{
};

/**
 * @brief A shape with contourSample(point).
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T>
struct HasContourSample<S, T, std::void_t<decltype(std::declval<const S&>().contourSample(std::declval<Vec3T<T>>()))>>
  : std::true_type
{
};

/**
 * @brief Whether a shape has a field of its own for the section plane (contourValue(), negative inside): the primary
 * template is false.
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T, class = void>
struct HasContourValue : std::false_type
{
};

/**
 * @brief A shape with contourValue(point).
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T>
struct HasContourValue<S, T, std::void_t<decltype(std::declval<const S&>().contourValue(std::declval<Vec3T<T>>()))>>
  : std::true_type
{
};

/**
 * @brief The shape as the settings show it: a view of regions with the settings' region settings
 * (Settings::m_regions), any other shape as it is.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in] a_shape    The shape.
 * @param[in] a_settings The settings.
 * @return The shape to render.
 */
template <class T, class S>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline S
shownAs(const S& a_shape, const Settings<T>& a_settings) noexcept
{
  if constexpr (hasRegionVisibility<S>) {
    return a_shape.withSettings(a_settings.m_regions);
  }
  else {
    return a_shape;
  }
}

template <class T, class S>
struct Sectioned
{
  /**
   * @brief The shape.
   */
  S m_shape;

  /**
   * @brief Whether the plane cuts it.
   */
  bool m_on;

  /**
   * @brief A point on the plane.
   */
  Vec3T<T> m_point;

  /**
   * @brief The plane's unit normal.
   */
  Vec3T<T> m_normal;

  /**
   * @brief How far behind the plane, into the kept side, the cut asks what is there (two march tolerances): a face
   * of the shape lying in the plane has value zero on it, so asking on the plane itself drew no cut face there.
   */
  T m_behind = T(0);

  /**
   * @brief The shape's box, which holds its solid (see isShape): a point outside it is outside the solid. Unbounded
   * unless the renderer sets it.
   */
  Vec3T<T> m_low = Vec3T<T>(-Math::Limits<T>::infinity(), -Math::Limits<T>::infinity(), -Math::Limits<T>::infinity());

  /**
   * @brief See m_low.
   */
  Vec3T<T> m_high = Vec3T<T>(Math::Limits<T>::infinity(), Math::Limits<T>::infinity(), Math::Limits<T>::infinity());

  /**
   * @brief Whether a point is inside the shape, uncut: outside its box it is not, without asking the shape. The cut
   * asks this where a ray's kept interval begins or ends on the plane, for every such ray, near the shape or not; on an
   * assembly of meshes the shape's value there cost ten times the ray's own crossing.
   * @param[in] a_point Point.
   * @return True if the shape's value there is negative.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  insideAt(const Vec3T<T>& a_point) const noexcept
  {
    for (int k = 0; k < 3; k++) {
      if (a_point[k] < m_low[k] || a_point[k] > m_high[k]) {
        return false;
      }
    }

    return this->shapeDistance(a_point) < T(0);
  }

  /**
   * @brief A point moved behind the plane, into the kept side, where the cut face asks what is there.
   * @param[in] a_point Point on the plane.
   * @return The point behind it.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Vec3T<T>
  behind(const Vec3T<T>& a_point) const noexcept
  {
    return a_point - m_behind * m_normal;
  }

  /**
   * @brief The distance to the plane, positive on the removed side.
   * @param[in] a_point Point.
   * @return The distance.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  plane(const Vec3T<T>& a_point) const noexcept
  {
    return dot(a_point - m_point, m_normal);
  }

  /**
   * @brief The shape's value, uncut.
   * @details Out of line on a device, like shapeHit(): the renderer reaches it from several places, and
   * a copy of a mesh's or a tape's distance inlined at each made the kernels too large to compile.
   * @param[in] a_point Point.
   * @return The value.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline T
  shapeDistance(const Vec3T<T>& a_point) const noexcept
  {
    return m_shape.signedDistance(a_point);
  }

  /**
   * @brief The field the section plane's contours and lines are drawn from, negative inside: the shape's
   * contourSample() or contourValue() when it has one (a region view's, zero on its interfaces too), else its value;
   * out of line on a device, as shapeDistance().
   * @param[in] a_point Point.
   * @return The field, and whether its lines there are secondary.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline ContourSample<T>
  contourDistance(const Vec3T<T>& a_point) const noexcept
  {
    if constexpr (HasContourSample<S, T>::value) {
      return m_shape.contourSample(a_point);
    }
    else if constexpr (HasContourValue<S, T>::value) {
      return ContourSample<T>{m_shape.contourValue(a_point)};
    }
    else {
      return ContourSample<T>{m_shape.signedDistance(a_point)};
    }
  }

  /**
   * @brief What a point is in, for the section plane's fill: the shape's sectionKey() (see HasSectionKey); out of line
   * on a device, as shapeDistance().
   * @param[in] a_point Point.
   * @return The key, or Settings::NoPrimitive for a shape without one.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline uint32_t
  sectionKey(const Vec3T<T>& a_point) const noexcept
  {
    if constexpr (HasSectionKey<S, T>::value) {
      return m_shape.sectionKey(a_point);
    }
    else {
      (void)a_point;

      return 0xFFFFFFFFU;
    }
  }

  /**
   * @brief The shape's first crossing along a ray, uncut (firstHit()); out of line on a device.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options How to march the shape.
   * @return The crossing, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline HitT<T>
  shapeHit(const RayT<T>& a_ray, const T a_tMin, const T a_tMax, const RayOptions<T>& a_options) const noexcept
  {
    return EBGeometry::firstHit(m_shape, a_ray, a_tMin, a_tMax, a_options);
  }

  /**
   * @brief The first surface seen through along a ray, in front of tMax, on the kept side of the plane (see
   * HasSeenThrough); a miss for a shape without such surfaces. Out of line on a device.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval: where the ray meets what it sees.
   * @param[in] a_options How to march the shape.
   * @return The crossing, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline HitT<T>
  seenThroughHit(const RayT<T>& a_ray, const T a_tMin, const T a_tMax, const RayOptions<T>& a_options) const noexcept
  {
    if constexpr (HasSeenThrough<S, T>::value) {
      // Only the kept side's part of the ray: a surface the plane removes, opaque or not, is not in front of anything.
      T tMin = a_tMin;
      T tMax = a_tMax;

      if (m_on) {
        const T start = this->plane(a_ray.m_origin);
        const T rate  = dot(a_ray.m_direction, m_normal);

        if (rate == T(0)) {
          if (start > T(0)) {
            return HitT<T>();
          }
        }
        else if (rate > T(0)) {
          tMax = Math::min(tMax, -start / rate);
        }
        else {
          tMin = Math::max(tMin, -start / rate);
        }
      }

      return (tMin < tMax) ? m_shape.firstSeenThrough(a_ray, tMin, tMax, a_options) : HitT<T>();
    }
    else {
      (void)a_ray;
      (void)a_tMin;
      (void)a_tMax;
      (void)a_options;

      return HitT<T>();
    }
  }

  /**
   * @brief The shape's first crossing along a ray without its normal, uncut (firstCrossing()); out of line on a
   * device.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options How to march the shape.
   * @return The crossing, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline CrossingT<T>
  shapeCrossing(const RayT<T>& a_ray, const T a_tMin, const T a_tMax, const RayOptions<T>& a_options) const noexcept
  {
    return EBGeometry::firstCrossing(m_shape, a_ray, a_tMin, a_tMax, a_options);
  }

  /**
   * @brief The cut shape's value.
   * @param[in] a_point Point.
   * @return The value.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    const T v = this->shapeDistance(a_point);

    return m_on ? Math::max(v, this->plane(a_point)) : v;
  }

  /**
   * @brief The shape's box.
   * @return The box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    return m_shape.computeBoundingVolume();
  }

  /**
   * @brief The shape's distance quality.
   * @return The quality.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline DistanceQuality
  getDistanceQuality() const noexcept
  {
    return RayDetail::qualityOf(m_shape);
  }

  /**
   * @brief The shape's finest element at a point (see HitT::m_primitive).
   * @param[in] a_point Point.
   * @return The element.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  primitiveAt(const Vec3T<T>& a_point) const noexcept
  {
    return RayDetail::primitiveOf(m_shape, a_point);
  }
};

/**
 * @brief A color for a primitive id: hues spread by the golden ratio, so neighboring ids differ.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_id Primitive id.
 * @return Linear RGB.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline Vec3T<T>
palette(const uint32_t a_id) noexcept
{
  const T h = T(0.08) + T(0.61803398875) * T(a_id % 1024U);
  const T f = T(6) * (h - std::floor(h));
  const T s = T(0.55);
  const T v = T(0.85);

  // HSV to RGB, with the hue's sector picked by its integer part.
  const int sector = static_cast<int>(f) % 6;
  const T   frac   = f - std::floor(f);
  const T   p      = v * (T(1) - s);
  const T   q      = v * (T(1) - s * frac);
  const T   t      = v * (T(1) - s * (T(1) - frac));

  switch (sector) {
  case 0:
    return Vec3T<T>(v, t, p);
  case 1:
    return Vec3T<T>(q, v, p);
  case 2:
    return Vec3T<T>(p, v, t);
  case 3:
    return Vec3T<T>(p, q, v);
  case 4:
    return Vec3T<T>(t, p, v);
  default:
    return Vec3T<T>(v, p, q);
  }
}

/**
 * @brief A color ramp from dark to bright (dark purple, red, orange, pale yellow), as the depth and cost views
 * use it.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_s Where on the ramp, from 0 (dark) to 1 (bright); clamped.
 * @return Display RGB in [0, 1] (gamma-encoded).
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline Vec3T<T>
ramp(const T a_s) noexcept
{
  const Vec3T<T> stops[5] = {Vec3T<T>(T(0.05), T(0.03), T(0.15)),
                             Vec3T<T>(T(0.40), T(0.07), T(0.43)),
                             Vec3T<T>(T(0.78), T(0.22), T(0.33)),
                             Vec3T<T>(T(0.98), T(0.55), T(0.15)),
                             Vec3T<T>(T(0.99), T(0.95), T(0.70))};

  const T   s = T(4) * Math::clamp(a_s, T(0), T(1));
  const int i = Math::min(static_cast<int>(s), 3);
  const T   f = s - T(i);

  return (T(1) - f) * stops[i] + f * stops[i + 1];
}

/**
 * @brief A time stamp for the cost views: the clock cycle counter on a device, nanoseconds on the host.
 * @details Internal helper; not part of the public API.
 * @return The time stamp.
 */
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline uint64_t
timeStamp() noexcept
{
#if defined(EBGEOMETRY_DEVICE_COMPILE)
  return static_cast<uint64_t>(clock64());
#else
  return static_cast<uint64_t>(
    std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
#endif
}

/**
 * @brief Everything one pixel's rays share: the cut shape, the scene's scale, the floor, the light.
 * @details Internal helper; not part of the public API. Aligned to a cache line: built on the stack for each pixel, its
 * fields read for every ray, it otherwise straddled cache lines or not as each run's stack placed it, and host renders
 * of a plain float tape ran in two speeds, up to 9% apart, from one run of the same binary to the next (a field added
 * to Sectioned shifted the layout).
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 */
template <class T, class S>
class alignas(64) Shader
{
public:
  /**
   * @brief How many surfaces seen through are drawn translucent along a camera ray at most, the nearest ones; those
   * behind them are left out.
   */
  static constexpr int SeenThroughLayers = 4;

  /**
   * @brief Set up the scene for a shape, a camera's view and settings.
   * @param[in] a_shape    Shape.
   * @param[in] a_view     The camera's view.
   * @param[in] a_settings Settings.
   */
  EBGEOMETRY_HOST_DEVICE
  Shader(const S& a_shape, const CameraView<T>& a_view, const Settings<T>& a_settings) noexcept
    : m_object{shownAs(a_shape, a_settings),
               a_settings.m_section,
               a_settings.m_sectionPoint,
               unit(a_settings.m_sectionNormal)},
      m_settings(a_settings),
      m_eye(a_view.m_eye),
      m_up(a_view.m_axis),
      m_baseLight(unit(a_settings.m_lightFromCamera
                         ? a_settings.m_light[0] * a_view.m_right + a_settings.m_light[1] * a_view.m_up -
                             a_settings.m_light[2] * a_view.m_forward
                         : a_settings.m_light)),
      m_light(m_baseLight),
      m_first(a_view.m_first),
      m_second(a_view.m_second),
      m_forward(a_view.m_forward),
      m_footprint(a_view.m_footprint),
      m_footprintPerLength(a_view.m_footprintPerLength)
  {

    const auto     box = a_shape.computeBoundingVolume();
    const Vec3T<T> lo  = box.getLowCorner();
    const Vec3T<T> hi  = box.getHighCorner();

    const T    diagonal = (hi - lo).length();
    const bool bounded  = std::isfinite(diagonal) && diagonal > T(0);

    m_scale = (a_settings.m_rays.m_scale > T(0)) ? a_settings.m_rays.m_scale : (bounded ? diagonal : a_view.m_distance);
    m_eps   = a_settings.m_rays.m_tolerance * m_scale;

    m_object.m_behind = T(2) * m_eps;
    m_far             = bounded ? ((m_eye - box.getCentroid()).length() + diagonal) : T(50) * m_scale;

    if (bounded) {
      m_object.m_low  = lo;
      m_object.m_high = hi;
    }

    m_distanceLike = RenderDetail::qualityOf(a_shape) != DistanceQuality::NotADistance;

    m_rays         = a_settings.m_rays;
    m_rays.m_scale = m_scale;

    // The floor sits just under the lowest corner along the up axis.
    T low = Math::Limits<T>::infinity();

    for (int corner = 0; corner < 8; corner++) {
      const Vec3T<T> c((corner & 1) ? hi[0] : lo[0], (corner & 2) ? hi[1] : lo[1], (corner & 4) ? hi[2] : lo[2]);

      low = Math::min(low, dot(c, m_up));
    }

    m_hasFloor   = a_settings.m_floor && std::isfinite(low) && lo[0] <= hi[0];
    m_floorLevel = low - T(0.01) * m_scale;

    // Orthographic rays start far enough behind the plane through the eye that the whole box is in front of
    // them, so that zooming in (the eye nearer) does not cut the shape open.
    T behind = T(0);

    if (a_view.m_orthographic && bounded) {
      for (int corner = 0; corner < 8; corner++) {
        const Vec3T<T> c((corner & 1) ? hi[0] : lo[0], (corner & 2) ? hi[1] : lo[1], (corner & 4) ? hi[2] : lo[2]);

        behind = Math::max(behind, dot(m_eye - c, m_forward));
      }

      behind += T(0.01) * m_scale;
    }

    m_backoff = behind;
    m_far += behind;

    // The section plane's contour and sheet are drawn within the shape's box, a little enlarged.
    const Vec3T<T> margin = T(0.05) * (hi - lo);

    m_boxLow  = lo - margin;
    m_boxHigh = hi + margin;
    m_onPlane = a_settings.m_section && bounded &&
                (a_settings.m_contourWidth > T(0) || a_settings.m_sheetOpacity > T(0) ||
                 a_settings.m_isoSpacing > T(0) || a_settings.m_sectionFill > T(0));
  }

  /**
   * @brief Whether the eye is inside the (cut) solid.
   * @return True if so.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  eyeInside() const noexcept
  {
    return m_object.signedDistance(m_eye) < T(0);
  }

  /**
   * @brief Where an orthographic camera ray through a point of the plane through the eye starts: behind the
   * shape's box.
   * @param[in] a_point Point on the plane through the eye.
   * @return The start.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Vec3T<T>
  orthographicStart(const Vec3T<T>& a_point) const noexcept
  {
    return a_point - m_backoff * m_forward;
  }

  /**
   * @brief Where along a camera ray seeing starts: the near clip, measured from the plane through the eye.
   * @param[in] a_nearClip The camera's near clip.
   * @return The distance along the ray from its start.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  nearAlong(const T a_nearClip) const noexcept
  {
    return (a_nearClip > T(0)) ? a_nearClip + m_backoff : T(0);
  }

  /**
   * @brief The cut shape's first crossing along a camera ray (for picking).
   * @param[in] a_origin    Where the ray starts.
   * @param[in] a_direction Unit direction.
   * @param[in] a_near      Where along it to start looking.
   * @return The crossing, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline HitT<T>
  cast(const Vec3T<T>& a_origin, const Vec3T<T>& a_direction, const T a_near) const noexcept
  {
    bool through = false;

    return this->visible(RayT<T>{a_origin, a_direction}, a_near, through);
  }

  /**
   * @brief Every crossing of the cut shape along a camera ray, nearest first, hidden primitives included (for
   * picking through depth).
   * @details Each search starts 4 tolerances past the last crossing, so two crossings closer than that (a sheet
   * thinner than it) count as one; as in visible(), partInside() and the x-ray view.
   * @param[in]  a_origin    Where the ray starts.
   * @param[in]  a_direction Unit direction.
   * @param[in]  a_near      Where along it to start looking.
   * @param[out] a_hits      The crossings, at most Renderer<T>::MaxCrossings, appended.
   */
  EBGEOMETRY_HOST
  inline void
  castAll(const Vec3T<T>& a_origin, const Vec3T<T>& a_direction, const T a_near, std::vector<HitT<T>>& a_hits) const
  {
    const RayT<T> ray{a_origin, a_direction};

    T t = a_near;

    for (uint32_t k = 0; k < Renderer<T>::MaxCrossings; k++) {
      const HitT<T> hit = this->trace(ray, t, m_far);

      if (!hit.hit()) {
        break;
      }

      a_hits.push_back(hit);

      t = this->past(hit);
    }
  }

  /**
   * @brief Move the light a little, for one of a pixel's rays: over a pixel's rays the light then covers a
   * small disc, as a light of some size does, which softens the edges of hard shadows.
   * @param[in] a_dx Where the ray is in its pixel, across (0 to 1).
   * @param[in] a_dy Where the ray is in its pixel, down (0 to 1).
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  jitterLight(const T a_dx, const T a_dy) noexcept
  {
    const Vec3T<T> t1 = unit(cross(
      (std::abs(m_baseLight[0]) < T(0.9)) ? Vec3T<T>(T(1), T(0), T(0)) : Vec3T<T>(T(0), T(1), T(0)), m_baseLight));
    const Vec3T<T> t2 = cross(m_baseLight, t1);

    m_light = unit(m_baseLight + T(0.04) * ((a_dx - T(0.5)) * t1 + (a_dy - T(0.5)) * t2));
  }

  /**
   * @brief The color seen along one camera ray, counting what it hit.
   * @param[in]     a_origin    Where the ray starts: the eye, or a point on the plane through it for an
   * orthographic camera.
   * @param[in]     a_direction Unit direction.
   * @param[in]     a_near      How far along the ray it starts to see (the camera's near clip).
   * @param[in,out] a_result    Counts.
   * @return Linear RGB.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Vec3T<T>
  shade(const Vec3T<T>& a_origin, const Vec3T<T>& a_direction, const T a_near, PixelResult& a_result) const noexcept
  {
    const RayT<T> ray{a_origin, a_direction};
    const HitT<T> hit = this->trace(ray, a_near, m_far);

    return this->shadeHit(ray, a_near, hit, a_result);
  }

  /**
   * @brief What one camera ray shows in the views other than the plain shaded one (see ViewMode), with hidden
   * primitives seen through and a part's inside filled; counts what it hit.
   * @param[in]     a_origin    Where the ray starts: the eye, or a point on the plane through it for an
   * orthographic camera.
   * @param[in]     a_direction Unit direction.
   * @param[in]     a_near      How far along the ray it starts to see (the camera's near clip).
   * @param[in,out] a_result    Counts.
   * @param[in,out] a_cost      In a cost view, the time the ray's query took is added here.
   * @return Linear RGB (black in a cost view, whose color comes from the frame's costs).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Vec3T<T>
  inspect(const Vec3T<T>& a_origin,
          const Vec3T<T>& a_direction,
          const T         a_near,
          PixelResult&    a_result,
          uint64_t&       a_cost) const noexcept
  {
    const RayT<T>  ray{a_origin, a_direction};
    const ViewMode mode = m_settings.m_viewMode;

    if (mode == ViewMode::CostHit) {
      const uint64_t start = timeStamp();
      const HitT<T>  hit   = this->trace(ray, a_near, m_far);

      a_cost += timeStamp() - start;

      this->count(hit.m_status, hit.m_entering, a_result);

      if (hit.hit() && a_result.m_primitive == 0xFFFFFFFFU) {
        a_result.m_primitive = hit.m_primitive;
      }

      return Vec3T<T>::zeros();
    }

    if (mode == ViewMode::CostCrossing) {
      const uint64_t     start    = timeStamp();
      const CrossingT<T> crossing = this->crossing(ray, a_near, m_far);

      a_cost += timeStamp() - start;

      this->count(crossing.m_status, crossing.m_entering, a_result);

      return Vec3T<T>::zeros();
    }

    if (mode == ViewMode::XRay) {
      // On black, every surface the ray crosses but a hidden one glows faintly, more where it is seen edge-on, with a
      // modest highlight from the light, and lets through 1 - m_xrayOpacity of what lies behind it. The highlighted
      // primitive glows in its color; the first surface is recorded, for outlining a picked object.
      const Vec3T<T> half = unit(m_light - a_direction);

      Vec3T<T> color   = Vec3T<T>::zeros();
      T        through = T(1);
      T        t       = a_near;

      for (uint32_t k = 0; k < Renderer<T>::MaxCrossings && through > T(1e-3); k++) {
        const HitT<T> hit = this->trace(ray, t, m_far);

        if (k == 0) {
          this->count(hit.m_status, hit.m_entering, a_result);
        }

        if (!hit.hit()) {
          break;
        }

        if (!m_settings.hides(hit.m_primitive)) {
          const Vec3T<T> n      = (hit.m_normal.length2() > T(0)) ? hit.m_normal : this->normal(hit.m_point);
          const T        facing = std::abs(dot(n, a_direction));
          const T        glow   = m_settings.m_xrayOpacity * (T(0.3) + T(0.7) * (T(1) - facing));
          const T shine = T(0.5) * m_settings.m_specular * std::pow(std::abs(dot(n, half)), m_settings.m_shininess);

          const bool highlighted = m_settings.isHighlighted(hit.m_primitive);

          const Vec3T<T> tint = highlighted ? T(2) * m_settings.m_highlightColor : Vec3T<T>::ones();

          if (a_result.m_primitive == 0xFFFFFFFFU) {
            a_result.m_primitive = hit.m_primitive;
          }

          color += through * (glow * tint + m_settings.m_xrayOpacity * shine * Vec3T<T>::ones());
          through *= T(1) - m_settings.m_xrayOpacity;
        }

        t = this->past(hit);
      }

      return color;
    }

    if (mode == ViewMode::Depth) {
      Vec3T<T> point;
      bool     seen = false;

      if (m_settings.hidesAny()) {
        bool          through = false;
        const HitT<T> hit     = this->visible(ray, a_near, through);

        this->count(hit.m_status, hit.m_entering, a_result);

        seen  = hit.hit();
        point = hit.m_point;
      }
      else {
        const CrossingT<T> crossing = this->crossing(ray, a_near, m_far);

        this->count(crossing.m_status, crossing.m_entering, a_result);

        seen  = crossing.hit();
        point = crossing.m_point;
      }

      return seen ? this->depthColor(point) : this->background(a_direction);
    }

    // The shaded view, looking through hidden primitives.
    bool          through = false;
    const HitT<T> hit     = this->visible(ray, a_near, through);

    Vec3T<T> color;

    if (through && hit.hit()) {
      // Seen through a hidden surface, a surface is shaded as one seen from outside: from inside the solid, it is
      // the inside of what was hidden.
      HitT<T> seen = hit;

      seen.m_entering = true;

      color = this->shadeHit(ray, a_near, seen, a_result);
    }
    else {
      color = this->shadeHit(ray, a_near, hit, a_result);
    }

    if (m_settings.m_volumePrimitive != Settings<T>::NoPrimitive) {
      // Opaque within about a tenth of the scene's scale.
      const T alpha = m_settings.m_volumeOpacity * (T(1) - std::exp(-T(10) / m_scale * this->partInside(ray, a_near)));

      color = (T(1) - alpha) * color + alpha * m_settings.m_volumeColor;
    }

    return color;
  }

private:
  /**
   * @brief The color seen along a camera ray where it found a crossing (or none), counting what it hit.
   * @param[in]     a_ray    The camera ray.
   * @param[in]     a_near   How far along the ray it starts to see.
   * @param[in]     a_hit    What it hit.
   * @param[in,out] a_result Counts.
   * @return Linear RGB.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Vec3T<T>
  shadeHit(const RayT<T>& a_ray, const T a_near, const HitT<T>& a_hit, PixelResult& a_result) const noexcept
  {
    const Vec3T<T>& direction = a_ray.m_direction;

    if (a_hit.m_status == HitStatus::Unresolved) {
      a_result.m_unresolved++;
    }

    // Where the ray meets the floor, if it goes down towards it.
    const T down   = dot(direction, m_up);
    const T tFloor = (m_hasFloor && down < T(0)) ? (m_floorLevel - dot(a_ray.m_origin, m_up)) / down : T(-1);

    const bool onFloor = tFloor > a_near && (!a_hit.hit() || tFloor < a_hit.m_t);

    Vec3T<T> color;

    if (a_hit.hit() && !onFloor) {
      a_result.m_hits++;

      if (a_result.m_primitive == 0xFFFFFFFFU) {
        a_result.m_primitive = a_hit.m_primitive;
      }

      if (!a_hit.m_entering) {
        a_result.m_insideHits++;
      }

      // A face cut by the section plane is where the plane, not the shape, gives the value: a fill, through
      // which the shape's next surface along the ray shows. It lies on the plane: a surface inside the solid (between
      // two regions of a view of regions) has a value below the plane's near the plane too, and is no cut face.
      const Vec3T<T>& p = a_hit.m_point;
      const T         d = m_object.m_on ? m_object.plane(p) : T(0);

      if (m_object.m_on && std::abs(d) <= T(4) * m_eps && d >= m_object.shapeDistance(p)) {
        color = this->shadeCutFace(a_ray, a_hit.m_t);
      }
      else {
        color = this->shadeSurface(a_hit, direction, true);
      }
    }
    else if (onFloor) {
      color = this->shadeFloor(a_ray.at(tFloor), direction, tFloor);
    }
    else {
      color = this->background(direction);
    }

    // What the ray sees lies at the floor, at the hit, or nowhere nearer than the far end.
    const T tFront = onFloor ? tFloor : (a_hit.hit() ? a_hit.m_t : m_far);

    // The surfaces seen through in front of it, up to SeenThroughLayers of them, are drawn translucent over it, back
    // to front, and the section plane in the gap where it lies: under the surfaces in front of it, over those behind.
    if constexpr (HasSeenThrough<S, T>::value) {
      if (m_settings.m_seenThroughOpacity > T(0)) {
        Vec3T<T> shades[SeenThroughLayers];
        T        at[SeenThroughLayers];
        int      layers = 0;
        T        from   = a_near;

        while (layers < SeenThroughLayers) {
          const HitT<T> veil = m_object.seenThroughHit(a_ray, from, tFront, m_rays);

          if (!veil.hit()) {
            break;
          }

          shades[layers] = this->shadeSurface(veil, direction, true);
          at[layers]     = veil.m_t;
          from           = this->past(veil);

          layers++;
        }

        if (layers > 0) {
          const T opacity = m_settings.m_seenThroughOpacity;

          // The gaps between layers: the plane is drawn in the one it lies in. drawOnPlane() takes a plane a little
          // behind the far end too (a face in the plane); a gap ending at a layer ends that much short of it, so that a
          // plane by a layer is drawn once, under it.
          T behind = tFront;

          for (int i = layers - 1; i >= 0; i--) {
            if (m_onPlane) {
              color = this->drawOnPlane(a_ray, at[i], behind, color);
            }

            color  = opacity * shades[i] + (T(1) - opacity) * color;
            behind = at[i] - T(4) * m_eps;
          }

          return m_onPlane ? this->drawOnPlane(a_ray, a_near, behind, color) : color;
        }
      }
    }

    if (m_onPlane) {
      color = this->drawOnPlane(a_ray, a_near, tFront, color);
    }

    return color;
  }

  /**
   * @brief Count a camera ray's crossing (for the views that do not shade it).
   * @param[in]     a_status   What the query found.
   * @param[in]     a_entering Whether a crossing enters the solid.
   * @param[in,out] a_result   Counts.
   */
  EBGEOMETRY_HOST_DEVICE
  static inline void
  count(const HitStatus a_status, const bool a_entering, PixelResult& a_result) noexcept
  {
    if (a_status == HitStatus::Unresolved) {
      a_result.m_unresolved++;
    }
    else if (a_status == HitStatus::Hit) {
      a_result.m_hits++;

      if (!a_entering) {
        a_result.m_insideHits++;
      }
    }
  }

  /**
   * @brief The first crossing along a camera ray that is not on a hidden primitive (Settings::m_hidden).
   * @param[in]  a_ray     The camera ray.
   * @param[in]  a_near    Where along it to start looking.
   * @param[out] a_through Whether the ray passed through a hidden primitive first.
   * @return The crossing, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline HitT<T>
  visible(const RayT<T>& a_ray, const T a_near, bool& a_through) const noexcept
  {
    T t = a_near;

    HitT<T> hit = this->trace(a_ray, t, m_far);

    for (uint32_t k = 1; k < Renderer<T>::MaxCrossings && hit.hit() && m_settings.hides(hit.m_primitive); k++) {
      a_through = true;
      t         = this->past(hit);
      hit       = this->trace(a_ray, t, m_far);
    }

    return hit;
  }

  /**
   * @brief How long a camera ray is inside the filled part (Settings::m_volumePrimitive), from a_near to the far
   * end, followed from crossing to crossing: a stretch inside the solid is the part's when the ray enters it through
   * the part's surface.
   * @details A part thinner than 4 tolerances along the ray can be stepped over (see castAll()), and the stretch
   * then runs on to the next crossing.
   * @param[in] a_ray  The camera ray.
   * @param[in] a_near Where along it to start.
   * @return The length.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  partInside(const RayT<T>& a_ray, const T a_near) const noexcept
  {
    const uint32_t filled = m_settings.m_volumePrimitive;

    // A ray that starts inside is inside the part there.
    const Vec3T<T> start = a_ray.at(a_near);

    bool in     = m_object.signedDistance(start) < T(0);
    bool mine   = in && (filled == Settings<T>::AllPrimitives || m_object.primitiveAt(start) == filled);
    T    from   = a_near;
    T    t      = a_near;
    T    length = T(0);

    for (uint32_t k = 0; k < Renderer<T>::MaxCrossings; k++) {
      const HitT<T> hit = this->trace(a_ray, t, m_far);

      if (!hit.hit()) {
        break;
      }

      if (hit.m_entering) {
        in   = true;
        mine = filled == Settings<T>::AllPrimitives || hit.m_primitive == filled;
        from = hit.m_t;
      }
      else if (in) {
        length += mine ? hit.m_t - from : T(0);
        in = false;
      }

      t = this->past(hit);
    }

    if (in && mine) {
      length += m_far - from;
    }

    return length;
  }

  /**
   * @brief The depth view's color of a point: by its distance from the eye along the view, from the nearest to
   * the farthest corner of the shape's box (bright to dark).
   * @param[in] a_point Point.
   * @return Linear RGB.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Vec3T<T>
  depthColor(const Vec3T<T>& a_point) const noexcept
  {
    T lo = Math::Limits<T>::infinity();
    T hi = -Math::Limits<T>::infinity();

    for (int corner = 0; corner < 8; corner++) {
      const Vec3T<T> c((corner & 1) ? m_boxHigh[0] : m_boxLow[0],
                       (corner & 2) ? m_boxHigh[1] : m_boxLow[1],
                       (corner & 4) ? m_boxHigh[2] : m_boxLow[2]);

      const T d = dot(c - m_eye, m_forward);

      lo = Math::min(lo, d);
      hi = Math::max(hi, d);
    }

    lo = Math::max(lo, T(0));

    if (!(std::isfinite(lo) && std::isfinite(hi) && hi > lo)) {
      lo = T(0);
      hi = m_far;
    }

    const Vec3T<T> c = ramp<T>(T(1) - (dot(a_point - m_eye, m_forward) - lo) / (hi - lo));

    // The ramp is in display colors; the pixel's gamma encoding undoes this.
    return Vec3T<T>(std::pow(c[0], T(2.2)), std::pow(c[1], T(2.2)), std::pow(c[2], T(2.2)));
  }

  /**
   * @brief The cut shape.
   */
  Sectioned<T, S> m_object;

  /**
   * @brief The settings.
   */
  const Settings<T>& m_settings;

  /**
   * @brief The eye.
   */
  Vec3T<T> m_eye;

  /**
   * @brief Unit up axis.
   */
  Vec3T<T> m_up;

  /**
   * @brief Unit direction towards the light, as set.
   */
  Vec3T<T> m_baseLight;

  /**
   * @brief Unit direction towards the light, moved a little for each ray of a pixel (see jitterLight()).
   */
  Vec3T<T> m_light;

  /**
   * @brief Horizontal unit vectors (see Camera::horizontal()).
   */
  Vec3T<T> m_first;

  /**
   * @brief Horizontal unit vectors (see Camera::horizontal()).
   */
  Vec3T<T> m_second;

  /**
   * @brief Unit direction the camera looks in.
   */
  Vec3T<T> m_forward;

  /**
   * @brief How far behind the plane through the eye orthographic rays start (0 for a perspective camera).
   */
  T m_backoff;

  /**
   * @brief The scene's length scale.
   */
  T m_scale;

  /**
   * @brief The march's tolerance, as a length.
   */
  T m_eps;

  /**
   * @brief How far camera rays march.
   */
  T m_far;

  /**
   * @brief How rays march, with the scale filled in.
   */
  RayOptions<T> m_rays;

  /**
   * @brief Whether there is a floor.
   */
  bool m_hasFloor;

  /**
   * @brief Height of the floor along the up axis.
   */
  T m_floorLevel;

  /**
   * @brief How wide a pixel is at a distance t along a camera ray: m_footprint + m_footprintPerLength t.
   */
  T m_footprint;

  /**
   * @brief See m_footprint.
   */
  T m_footprintPerLength;

  /**
   * @brief Low corner of the box the section plane's contour and sheet are drawn in.
   */
  Vec3T<T> m_boxLow;

  /**
   * @brief High corner of that box.
   */
  Vec3T<T> m_boxHigh;

  /**
   * @brief Whether to draw anything on the section plane.
   */
  bool m_onPlane;

  /**
   * @brief Whether the shape's value is a distance or a distance bound (not DistanceQuality::NotADistance): its section
   * field then changes by at most 1 per unit length, which lets the plane skip the lines' slope where no line can be.
   */
  bool m_distanceLike = false;

  /**
   * @brief The cut shape's first crossing along a ray (firstHit()).
   * @details Every ray the shader casts goes through here, out of line on a device: inlined at each of
   * its callers, the shape's whole trace was copied once per ray kind and the kernels grew too large to
   * compile.
   * @param[in] a_ray  Ray, with a unit direction.
   * @param[in] a_tMin Where along it to start looking.
   * @param[in] a_tMax How far along it to look.
   * @return The crossing, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline HitT<T>
  trace(const RayT<T>& a_ray, const T a_tMin, const T a_tMax) const noexcept
  {
    return firstHit(m_object, a_ray, a_tMin, a_tMax, m_rays);
  }

  /**
   * @brief The cut shape's first crossing along a ray, without its normal (firstCrossing()); out of line on a
   * device, like trace().
   * @param[in] a_ray  Ray, with a unit direction.
   * @param[in] a_tMin Where along it to start looking.
   * @param[in] a_tMax How far along it to look.
   * @return The crossing, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline CrossingT<T>
  crossing(const RayT<T>& a_ray, const T a_tMin, const T a_tMax) const noexcept
  {
    return firstCrossing(m_object, a_ray, a_tMin, a_tMax, m_rays);
  }

  /**
   * @brief The cut shape's unit normal at a point, from finite differences on a tetrahedron.
   * @param[in] a_point Point.
   * @return The normal.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Vec3T<T>
  normal(const Vec3T<T>& a_point) const noexcept
  {
    const T        h = T(5e-4) * m_scale;
    const Vec3T<T> a(T(1), T(-1), T(-1));
    const Vec3T<T> b(T(-1), T(-1), T(1));
    const Vec3T<T> c(T(-1), T(1), T(-1));
    const Vec3T<T> d(T(1), T(1), T(1));

    const Vec3T<T> g = a * m_object.signedDistance(a_point + h * a) + b * m_object.signedDistance(a_point + h * b) +
                       c * m_object.signedDistance(a_point + h * c) + d * m_object.signedDistance(a_point + h * d);

    const T length = g.length();

    return (length > T(0)) ? g / length : m_up;
  }

  /**
   * @brief Where a ray goes on from past a crossing, to find the next one.
   * @details A crossing found exactly (by a ray form) is passed by a tenth of the tolerance; a marched one, placed
   * within the tolerance, by four. Four for both stepped over a surface that close behind an exact crossing: the
   * an inner region's wall just inside an outer region's end face, seen through the outer one.
   * @param[in] a_hit The crossing.
   * @return The distance along the ray.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  past(const HitT<T>& a_hit) const noexcept
  {
    return a_hit.m_t + (a_hit.m_exact ? T(0.1) : T(4)) * m_eps;
  }

  /**
   * @brief Where a ray first meets a surface that is not seen through (Settings::m_hidden) in an interval.
   * @details With no surface seen through, one trace, as a plain shape's shadow and occlusion rays always were;
   * stepping past surfaces seen through is blockedPastHidden(), out of line on the host: inlined into the lighting of
   * every shaded pixel, it made host renders of a plain float tape 10% slower.
   * @param[in] a_ray  Ray, with a unit direction.
   * @param[in] a_tMax End of the interval.
   * @return The distance along the ray, or infinity if it meets none.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  blocked(const RayT<T>& a_ray, const T a_tMax) const noexcept
  {
    if (m_settings.hidesAny()) {
      return this->blockedPastHidden(a_ray, a_tMax);
    }

    // A view of regions showing the whole object, uncut: its surface is the tape's, which one trace finds (the view's
    // search for each change of region asks which region a point is in, for keys a shadow does not need: shadows and
    // occlusion took most of such a frame). Not when every shape is marched (m_march), which the trace below honors and
    // the tape's ray program does not.
    if constexpr (hasRegionVisibility<S>) {
      if (!m_object.m_on && !m_rays.m_march && m_object.m_shape.showsWholeObject()) {
        const HitT<T> plain = m_object.m_shape.objectHit(a_ray, a_tMax, m_rays);

        return plain.hit() ? plain.m_t : Math::Limits<T>::infinity();
      }
    }

    const HitT<T> hit = this->trace(a_ray, T(0), a_tMax);

    return hit.hit() ? hit.m_t : Math::Limits<T>::infinity();
  }

  /**
   * @brief Where a ray first meets a surface that is not seen through in an interval, some being seen through: a
   * surface seen through casts no shadow, and the ray goes on past it, as a camera ray does.
   * @param[in] a_ray  Ray, with a unit direction.
   * @param[in] a_tMax End of the interval.
   * @return The distance along the ray, or infinity if it meets none.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_HOST_NOINLINE
  T
  blockedPastHidden(const RayT<T>& a_ray, const T a_tMax) const noexcept
  {
    HitT<T> hit = this->trace(a_ray, T(0), a_tMax);

    for (uint32_t k = 1; k < Renderer<T>::MaxCrossings && hit.hit() && m_settings.hides(hit.m_primitive); k++) {
      hit = this->trace(a_ray, this->past(hit), a_tMax);
    }

    return (hit.hit() && !m_settings.hides(hit.m_primitive)) ? hit.m_t : Math::Limits<T>::infinity();
  }

  /**
   * @brief How much light reaches a point from the light's direction: 0 in shadow, 1 in light.
   * @details One ray towards the light (firstHit()); the light moving a little between a pixel's rays
   * softens the edges. A surface seen through (Settings::m_hidden) casts no shadow.
   * @param[in] a_point  Point.
   * @param[in] a_normal Unit normal there.
   * @return The fraction of light.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  shadow(const Vec3T<T>& a_point, const Vec3T<T>& a_normal) const noexcept
  {
    const Vec3T<T> origin = a_point + T(4) * m_eps * a_normal;

    return (this->blocked(RayT<T>{origin, m_light}, T(2) * m_scale) < Math::Limits<T>::infinity()) ? T(0) : T(1);
  }

  /**
   * @brief Ambient occlusion at a point: 1 in the open, less in creases and corners.
   * @param[in] a_point  Point.
   * @param[in] a_normal Unit normal there.
   * @return The occlusion factor.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  occlusion(const Vec3T<T>& a_point, const Vec3T<T>& a_normal) const noexcept
  {
    // Four short rays, tilted around the normal, each darkening by up to a fifth as its blocker is near: (1 - t/reach)^2,
    // so that a blocker at the surface (a crease, a corner) darkens fully and one near the end of the reach not at all.
    // At a flat fifth for any blocker within reach, a part of a large assembly a reach away (0.07 of the scene's
    // diagonal, a meter on a jet) printed its sharp-edged shape on a thin plate beside it, which read as seeing through.
    const Vec3T<T> t1 =
      unit(cross((std::abs(a_normal[0]) < T(0.9)) ? Vec3T<T>(T(1), T(0), T(0)) : Vec3T<T>(T(0), T(1), T(0)), a_normal));
    const Vec3T<T> t2     = cross(a_normal, t1);
    const Vec3T<T> origin = a_point + T(4) * m_eps * a_normal;

    const T reach = T(0.07) * m_scale;

    T dark = T(0);

    for (int k = 0; k < 4; k++) {
      const T        a = T(0.5) * pi<T> * T(k);
      const Vec3T<T> r = unit(a_normal + T(0.8) * (std::cos(a) * t1 + std::sin(a) * t2));
      const T        t = this->blocked(RayT<T>{origin, r}, reach);

      if (t < reach) {
        const T near = T(1) - t / reach;

        dark += near * near;
      }
    }

    return T(1) - T(0.2) * dark;
  }

  /**
   * @brief The light a surface point with an albedo and a normal reflects towards the eye.
   * @param[in] a_point     Point.
   * @param[in] a_normal    Unit normal, towards the eye.
   * @param[in] a_direction Unit direction of the camera ray.
   * @param[in] a_albedo    Albedo.
   * @param[in] a_shade     Whether shadows and occlusion may darken it (not for a surface seen through the
   * cut face, inside the solid, where they would be everywhere).
   * @return Linear RGB.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Vec3T<T>
  light(const Vec3T<T>& a_point,
        const Vec3T<T>& a_normal,
        const Vec3T<T>& a_direction,
        const Vec3T<T>& a_albedo,
        const bool      a_shade = true) const noexcept
  {
    const Vec3T<T> half = unit(m_light - a_direction);

    const T lit  = (a_shade && m_settings.m_shadows) ? this->shadow(a_point, a_normal) : T(1);
    const T diff = Math::max(T(0), dot(a_normal, m_light)) * lit;
    const T spec = std::pow(Math::max(T(0), dot(a_normal, half)), m_settings.m_shininess) * diff;
    const T sky  = T(0.5) + T(0.5) * dot(a_normal, m_up);
    const T ao   = (a_shade && m_settings.m_occlusion) ? this->occlusion(a_point, a_normal) : T(1);

    return a_albedo * (m_settings.m_lightIntensity * diff + m_settings.m_ambient * sky * ao) +
           m_settings.m_specular * spec * Vec3T<T>::ones();
  }

  /**
   * @brief The color of the section plane's cut face where a camera ray meets it: the fill, and through it
   * the shape's next surface along the ray.
   * @details Out of line on a device: inlined, this second shading of a surface (and its trace) doubled what
   * the render kernels spill to local memory, for every pixel, section or no section.
   * @param[in] a_ray The camera ray.
   * @param[in] a_t   Where it meets the cut face.
   * @return Linear RGB.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline Vec3T<T>
  shadeCutFace(const RayT<T>& a_ray, const T a_t) const noexcept
  {
    const Vec3T<T> fill = this->shadeCut();

    if (m_settings.m_cutOpacity >= T(1)) {
      return fill;
    }

    const HitT<T>  behind = this->trace(a_ray, a_t + T(4) * m_eps, m_far);
    const Vec3T<T> seen =
      behind.hit() ? this->shadeSurface(behind, a_ray.m_direction, false) : this->background(a_ray.m_direction);

    return m_settings.m_cutOpacity * fill + (T(1) - m_settings.m_cutOpacity) * seen;
  }

  /**
   * @brief The fill of the section plane's cut face: colored as a surface facing along the plane's normal
   * would be, and lit by that normal a little from either side, without shadows, so that a slice reads the
   * same whichever way the light faces. Colored by primitive, it takes the cut color: inside a
   * composition the leaf that decides the value (a subtracted one, near its surface) need not be the
   * material there.
   * @return Linear RGB.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Vec3T<T>
  shadeCut() const noexcept
  {
    const Vec3T<T>& n = m_object.m_normal;

    Vec3T<T> albedo = m_settings.m_color;

    if (m_settings.m_colorBy == ColorBy::Normal) {
      albedo = T(0.5) * (n + Vec3T<T>::ones());
    }
    else if (m_settings.m_colorBy == ColorBy::Primitive) {
      albedo = m_settings.m_cutColor;
    }

    return albedo * (T(0.6) + T(0.4) * std::abs(dot(n, m_light)));
  }

  /**
   * @brief The color of the shape's surface where a ray hit it.
   * @param[in] a_hit       The hit.
   * @param[in] a_direction Unit direction of the ray.
   * @param[in] a_tint      Whether a surface seen from inside the solid is tinted, and shadows and occlusion
   * darken it (not when it is seen through the cut face, which looks into the solid on purpose, and where
   * the solid around it would shadow and occlude it everywhere).
   * @return Linear RGB.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Vec3T<T>
  shadeSurface(const HitT<T>& a_hit, const Vec3T<T>& a_direction, const bool a_tint) const noexcept
  {
    const Vec3T<T>& p = a_hit.m_point;

    Vec3T<T> n = (a_hit.m_normal.length2() > T(0)) ? a_hit.m_normal : this->normal(p);

    Vec3T<T> albedo = m_settings.m_color;

    if (m_settings.m_colorBy == ColorBy::Primitive) {
      albedo = this->keyColor(a_hit.m_primitive);
    }
    else if (m_settings.m_colorBy == ColorBy::Normal) {
      albedo = T(0.5) * (n + Vec3T<T>::ones());
    }

    // Light the side that faces the eye; for a surface seen from inside that is the inner side.
    if (dot(n, a_direction) > T(0)) {
      n = -n;
    }

    // A surface with no key (a view of regions colored by group shows a surface in no group so) is never the one
    // highlighted.
    if (m_settings.isHighlighted(a_hit.m_primitive)) {
      albedo = T(0.35) * albedo + T(0.65) * m_settings.m_highlightColor;
    }

    if (!a_hit.m_entering && a_tint) {
      albedo = T(0.25) * albedo + T(0.75) * m_settings.m_insideColor;
    }

    return this->light(p, n, a_direction, albedo, a_tint);
  }

  /**
   * @brief The color of a key, colored by primitive: the settings' (m_primitiveColors), else the palette's; no key (a
   * view of regions: a surface in no surface group, a region without a material) is a neutral gray.
   * @param[in] a_key The key.
   * @return Linear RGB.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Vec3T<T>
  keyColor(const uint32_t a_key) const noexcept
  {
    return (a_key < Settings<T>::PrimitiveColors && m_settings.m_primitiveColors[a_key][0] >= T(0))
             ? m_settings.m_primitiveColors[a_key]
           : (a_key == Settings<T>::NoPrimitive) ? Vec3T<T>(T(0.6), T(0.6), T(0.6))
                                                 : palette<T>(a_key);
  }

  /**
   * @brief The color of a material, for the section plane's fill: the settings' (m_materialColors), else the
   * palette's; no material is a neutral gray.
   * @param[in] a_material The material's number.
   * @return Linear RGB.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Vec3T<T>
  materialColor(const uint32_t a_material) const noexcept
  {
    return (a_material < Settings<T>::PrimitiveColors && m_settings.m_materialColors[a_material][0] >= T(0))
             ? m_settings.m_materialColors[a_material]
           : (a_material == Settings<T>::NoPrimitive) ? Vec3T<T>(T(0.6), T(0.6), T(0.6))
                                                      : palette<T>(a_material);
  }

  /**
   * @brief What the section plane adds where a camera ray meets it in front of what the ray sees: the
   * shape's contour, a line of constant width on screen where the shape's value is near zero, and outside
   * the shape a faint sheet. Inside the shape the plane is the cut face, already shaded.
   * @param[in] a_ray    The camera ray.
   * @param[in] a_near   Where the ray starts to see.
   * @param[in] a_tFront Where it meets what it sees.
   * @param[in] a_color What it sees.
   * @return Linear RGB.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline Vec3T<T>
  drawOnPlane(const RayT<T>& a_ray, const T a_near, const T a_tFront, const Vec3T<T>& a_color) const noexcept
  {
    const T rate = dot(a_ray.m_direction, m_object.m_normal);

    if (rate == T(0)) {
      return a_color;
    }

    const T t = -m_object.plane(a_ray.m_origin) / rate;

    if (!(t > a_near && t <= a_tFront + T(4) * m_eps)) {
      return a_color;
    }

    const Vec3T<T> p = a_ray.at(t);

    for (int k = 0; k < 3; k++) {
      if (p[k] < m_boxLow[k] || p[k] > m_boxHigh[k]) {
        return a_color;
      }
    }

    const T width = m_footprint + m_footprintPerLength * t;
    const T half  = T(0.5) * m_settings.m_contourWidth * width;

    // What is there is asked a little behind the plane, into the kept side, by the cut's own nudge (two march
    // tolerances), so that a face of the shape lying in the plane is inside the solid there. Asked further behind, a
    // surface meeting the plane at an angle had its contour moved along the plane by the depth over the angle's
    // tangent: several pixels at shallow angles. The face in the plane draws no contour all the same: its field is
    // flat along the plane, and a flat field draws no line (below).
    const Vec3T<T> q = p - m_object.m_behind * m_object.m_normal;

    const T value = m_object.shapeDistance(q);

    // The contours and the lines of equal distance follow the shape's contour field (a region view's is zero on its
    // interfaces too); which side of the surface the point is on follows its value.
    const ContourSample<T> sample =
      (RenderDetail::HasContourSample<S, T>::value || RenderDetail::HasContourValue<S, T>::value)
        ? m_object.contourDistance(q)
        : ContourSample<T>{value};
    // The field's slope along the plane, from steps of a pixel's width along two directions in it.
    // The field is a distance in space: where a surface meets the plane at a shallow angle it changes along the plane
    // by much less than one per unit of length, and a line drawn where it is small widened into a band. Divided by
    // the slope, it is the distance along the plane, and every line is as wide on screen however the surface meets the
    // plane. Where the field is flat along the plane (a face lying in it) no line is drawn, and elsewhere the slope is kept
    // above a floor (below).
    const Vec3T<T>& n = m_object.m_normal;
    const Vec3T<T>  a = cross(n, Vec3T<T>::unit(n.minDir(true)));
    const Vec3T<T>  u = a / a.length();
    const Vec3T<T>  v = cross(n, u);

    // Each direction takes the steeper of a step forward and one back: the field can have a kink at zero (it is zero on
    // an interface between two regions), and one step across it saw the field come back up to about where it was.
    const T    f     = sample.m_value;
    const bool lines = m_settings.m_isoSpacing > T(0) && (!sample.m_secondary || m_settings.m_isoSecondary);
    const T    rest  = lines ? f - m_settings.m_isoSpacing * std::floor(f / m_settings.m_isoSpacing + T(0.5)) : T(0);

    // The four steps cost four of the field's evaluations per pixel of the plane. EBGeometry's distances, and the bounds
    // it composes from them, change by at most 1 per unit length, so their slope is at most sqrt(2), and a pixel whose
    // value, and whose distance to the nearest line of equal distance, are both more than sqrt(2) line widths can have
    // no line: there the slope is not asked, and taken so small that no line is drawn. The contour is safe for any
    // bound (on its line |f| is at most the distance to the surface); a user shape that counts as a bound but changes
    // faster may lose lines of equal distance here, and should declare itself NotADistance. Measured on an assembly of 67 meshes cut by the plane: a third of the frame's time.
    const T    rootTwo = std::sqrt(T(2));
    const bool near =
      !m_distanceLike || std::abs(f) < rootTwo * half || (lines && std::abs(rest) < rootTwo * T(0.75) * width);

    T slope = Math::Limits<T>::min();

    if (near) {
      const T du = Math::max(std::abs(m_object.contourDistance(q + width * u).m_value - f),
                             std::abs(m_object.contourDistance(q - width * u).m_value - f)) /
                   width;
      const T dv = Math::max(std::abs(m_object.contourDistance(q + width * v).m_value - f),
                             std::abs(m_object.contourDistance(q - width * v).m_value - f)) /
                   width;

      const T steep = std::sqrt(du * du + dv * dv);

      // Flat along the plane (a face lying in it): no line. Otherwise the slope is kept above a floor, so that a
      // surface meeting the plane at a few degrees keeps its contour, and a ridge draws a thin one, not a blob.
      slope = (steep < T(1e-3)) ? Math::Limits<T>::min() : Math::max(steep, T(0.05));
    }

    const T field = sample.m_value / slope;

    if (std::abs(field) < half) {
      return m_settings.m_contourColor;
    }

    // The fill by what the point is in, in place of the sheet.
    const T  fill  = RenderDetail::HasSectionKey<S, T>::value ? m_settings.m_sectionFill : T(0);
    Vec3T<T> color = a_color;

    if (fill > T(0)) {
      const Vec3T<T> what =
        sample.m_secondary ? m_settings.m_fillSecondaryColor : this->materialColor(m_object.sectionKey(q));

      color = (T(1) - fill) * a_color + fill * what;
    }

    // Lines of equal distance, every m_isoSpacing, thinner than the contour.

    if (lines) {
      const T r = rest / slope;

      if (std::abs(r) < T(0.75) * width) {
        color = sample.m_secondary ? m_settings.m_isoSecondaryColor : m_settings.m_isoColor;
      }
    }

    if (value > T(0) && m_settings.m_sheetOpacity > T(0) && fill == T(0)) {
      // The sheet's edge, a pixel and a half wide, where the plane leaves the box.
      T edge = Math::Limits<T>::infinity();

      for (int k = 0; k < 3; k++) {
        edge = Math::min(edge, Math::min(p[k] - m_boxLow[k], m_boxHigh[k] - p[k]));
      }

      const Vec3T<T>& over = (edge < T(1.5) * width) ? m_settings.m_sheetEdgeColor : m_settings.m_sheetColor;

      return (T(1) - m_settings.m_sheetOpacity) * color + m_settings.m_sheetOpacity * over;
    }

    return color;
  }

  /**
   * @brief The color of the floor where a camera ray hit it: a faint checkerboard, shadowed by the
   * shape, fading into the background far away.
   * @param[in] a_point     Point on the floor.
   * @param[in] a_direction Unit direction of the camera ray.
   * @param[in] a_t         Distance along the ray.
   * @return Linear RGB.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Vec3T<T>
  shadeFloor(const Vec3T<T>& a_point, const Vec3T<T>& a_direction, const T a_t) const noexcept
  {
    const T cell  = (m_settings.m_floorCheckerSize > T(0)) ? m_settings.m_floorCheckerSize : m_scale / T(6);
    const T a     = std::floor(dot(a_point, m_first) / cell);
    const T b     = std::floor(dot(a_point, m_second) / cell);
    const T check = (!m_settings.m_floorChecker || std::fmod(std::abs(a + b), T(2)) < T(0.5)) ? T(1) : T(0.9);

    const Vec3T<T> lit = this->light(a_point, m_up, a_direction, check * m_settings.m_floorColor);

    const T fade = std::exp(-(a_t * a_t) / (T(36) * m_scale * m_scale));

    return fade * lit + (T(1) - fade) * this->background(a_direction);
  }

  /**
   * @brief The background along a ray that hits nothing.
   * @param[in] a_direction Unit direction.
   * @return Linear RGB.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Vec3T<T>
  background(const Vec3T<T>& a_direction) const noexcept
  {
    if (!m_settings.m_sky) {
      return m_settings.m_background;
    }

    const T s = Math::clamp(dot(a_direction, m_up), T(0), T(1));

    return (T(1) - s) * m_settings.m_skyHorizon + s * m_settings.m_skyZenith;
  }
};

/**
 * @brief Count a finished row of a device render: atomically on a device, where the frame's threads add to
 * one counter; a plain increment on the host, where a render's rows are counted one launch at a time.
 * @details Internal helper; not part of the public API.
 * @param[in,out] a_rows The counter.
 * @return The count with this row.
 */
EBGEOMETRY_HOST_DEVICE
inline uint32_t
countRow(uint32_t* a_rows) noexcept
{
#if defined(EBGEOMETRY_DEVICE_COMPILE)
  return atomicAdd(a_rows, 1U) + 1U;
#else
  return ++(*a_rows);
#endif
}

/**
 * @brief The pixels of a device render: pixel a_first + i, for every index i of the launch, into the frame's
 * results, or none of them once the render is canceled.
 * @details Internal helper; not part of the public API. The pixel indices come from the launch, so nothing
 * is copied to the device but the functor. A device render is one launch over the whole frame; a canceled
 * one has its threads read m_cancel, a flag in device memory (copied there by the host while the kernel runs),
 * and skip their pixel once it is set, leaving it black and setting m_skipped, which says whether the render
 * completed. Each row's last pixel counts the row, in m_rows (device memory) and in m_progress (host memory
 * mapped into the device's, for the host to show while the kernel runs). The threads of
 * a row run in no set order, so the count is progress only: a row may be counted before all of its pixels
 * are done.
 * @tparam Q Pixel functor type: PixelQuery for the shaded view, InspectQuery for the others.
 */
template <class Q>
struct FrameOfPixels
{
  /**
   * @brief The pixel functor.
   */
  Q m_query;

  /**
   * @brief The frame's results, in device memory.
   */
  StoredPixel* m_results;

  /**
   * @brief Each pixel's object, in device memory, or null when not recorded.
   */
  uint32_t* m_primitives;

  /**
   * @brief Nonzero once the render is canceled, or null when it cannot be.
   */
  const volatile uint32_t* m_cancel;

  /**
   * @brief Set to nonzero by a pixel that skips its work, in device memory, or null when the render cannot be
   * canceled.
   */
  uint32_t* m_skipped;

  /**
   * @brief The rows whose last pixel is done, in device memory, or null when not counted.
   */
  uint32_t* m_rows;

  /**
   * @brief The finished rows, as last written, in mapped host memory, or null when not shown.
   */
  volatile uint32_t* m_progress;

  /**
   * @brief The launch's first pixel.
   */
  uint32_t m_first;

  /**
   * @brief Pixels per row.
   */
  uint32_t m_width;

  /**
   * @brief Shade pixel m_first + a_index.
   * @param[in] a_index Index within the launch.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  operator()(const std::size_t a_index) const noexcept
  {
    const auto pixel = static_cast<uint32_t>(m_first + a_index);

    if (m_cancel != nullptr && *m_cancel != 0U) {
      m_results[pixel] = StoredPixel{};
      *m_skipped       = 1U;

      if (m_primitives != nullptr) {
        m_primitives[pixel] = 0xFFFFFFFFU;
      }

      return;
    }

    const PixelResult result = m_query(pixel);

    m_results[pixel] = StoredPixel::of(result);

    if (m_primitives != nullptr) {
      m_primitives[pixel] = result.m_primitive;
    }

    if (m_rows != nullptr && pixel % m_width == m_width - 1U) {
      const uint32_t rows = countRow(m_rows);

      // The host keeps the largest count it reads, since writes from different threads may arrive out of
      // order.
      if (m_progress != nullptr) {
        *m_progress = rows;
      }
    }
  }
};

} // namespace RenderDetail

} // namespace EBGeometry::Render

namespace EBGeometry {

/**
 * @brief The ray form of a shape cut by the section plane: the intersection of the shape with a
 * half-space, along a ray. firstHit() on the shape (by its own ray form, or by sphere tracing) over the
 * part of the ray on the kept side, and a crossing on the plane where the ray passes through it inside
 * the solid.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 */
template <class T, class S>
struct RayForm<Render::RenderDetail::Sectioned<T, S>>
{
  /**
   * @brief The first crossing of the cut shape along a ray.
   * @param[in] a_cut  The cut shape.
   * @param[in] a_ray  Ray, with a unit direction.
   * @param[in] a_tMin Start of the interval.
   * @param[in] a_tMax End of the interval.
   * @param[in] a_options How to march the shape.
   * @return The crossing, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  firstHit(const Render::RenderDetail::Sectioned<T, S>& a_cut,
           const RayT<T>&                               a_ray,
           const T                                      a_tMin,
           const T                                      a_tMax,
           const RayOptions<T>&                         a_options) noexcept
  {
    if (!a_cut.m_on) {
      return a_cut.shapeHit(a_ray, a_tMin, a_tMax, a_options);
    }

    // The part [a, b] of the interval on the kept side of the plane.
    const T start = a_cut.plane(a_ray.at(a_tMin));
    const T rate  = dot(a_ray.m_direction, a_cut.m_normal);

    T a = a_tMin;
    T b = a_tMax;

    if (rate == T(0)) {
      if (start > T(0)) {
        return HitT<T>();
      }
    }
    else if (rate > T(0)) {
      b = Math::min(b, a_tMin - start / rate);
    }
    else {
      a = Math::max(a, a_tMin - start / rate);
    }

    if (!(a <= b)) {
      return HitT<T>();
    }

    // A crossing on the cut face: where the ray enters the kept side, or leaves it, inside the solid.
    const auto onPlane = [&a_cut, &a_ray](const T a_t, const bool a_entering) noexcept {
      HitT<T> hit;

      hit.m_status    = HitStatus::Hit;
      hit.m_t         = a_t;
      hit.m_point     = a_ray.at(a_t);
      hit.m_primitive = a_cut.primitiveAt(a_cut.behind(hit.m_point));
      hit.m_entering  = a_entering;
      hit.m_exact     = true;
      hit.m_normal    = a_cut.m_normal;

      return hit;
    };

    if (a > a_tMin && a_cut.insideAt(a_cut.behind(a_ray.at(a)))) {
      return onPlane(a, true);
    }

    const HitT<T> hit = a_cut.shapeHit(a_ray, a, b, a_options);

    if (hit.m_status == HitStatus::Miss && b < a_tMax && a_cut.insideAt(a_cut.behind(a_ray.at(b)))) {
      return onPlane(b, false);
    }

    return hit;
  }

  /**
   * @brief The first crossing of the cut shape along a ray, without its normal and primitive: as firstHit(), with
   * the shape's firstCrossing().
   * @param[in] a_cut  The cut shape.
   * @param[in] a_ray  Ray, with a unit direction.
   * @param[in] a_tMin Start of the interval.
   * @param[in] a_tMax End of the interval.
   * @param[in] a_options How to march the shape.
   * @return The crossing, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline CrossingT<T>
  firstCrossing(const Render::RenderDetail::Sectioned<T, S>& a_cut,
                const RayT<T>&                               a_ray,
                const T                                      a_tMin,
                const T                                      a_tMax,
                const RayOptions<T>&                         a_options) noexcept
  {
    if (!a_cut.m_on) {
      return a_cut.shapeCrossing(a_ray, a_tMin, a_tMax, a_options);
    }

    // The part [a, b] of the interval on the kept side of the plane, as in firstHit().
    const T start = a_cut.plane(a_ray.at(a_tMin));
    const T rate  = dot(a_ray.m_direction, a_cut.m_normal);

    T a = a_tMin;
    T b = a_tMax;

    if (rate == T(0)) {
      if (start > T(0)) {
        return CrossingT<T>();
      }
    }
    else if (rate > T(0)) {
      b = Math::min(b, a_tMin - start / rate);
    }
    else {
      a = Math::max(a, a_tMin - start / rate);
    }

    if (!(a <= b)) {
      return CrossingT<T>();
    }

    const auto onPlane = [&a_ray](const T a_t, const bool a_entering) noexcept {
      CrossingT<T> crossing;

      crossing.m_status   = HitStatus::Hit;
      crossing.m_t        = a_t;
      crossing.m_point    = a_ray.at(a_t);
      crossing.m_entering = a_entering;
      crossing.m_exact    = true;

      return crossing;
    };

    if (a > a_tMin && a_cut.insideAt(a_cut.behind(a_ray.at(a)))) {
      return onPlane(a, true);
    }

    const CrossingT<T> crossing = a_cut.shapeCrossing(a_ray, a, b, a_options);

    if (crossing.m_status == HitStatus::Miss && b < a_tMax && a_cut.insideAt(a_cut.behind(a_ray.at(b)))) {
      return onPlane(b, false);
    }

    return crossing;
  }
};

} // namespace EBGeometry

namespace EBGeometry::Render {

template <class T>
EBGEOMETRY_HOST
inline Camera<T>
Camera<T>::fit(const BoundingVolumes::AABBT<T>& a_box, const Vec3T<T>& a_up)
{
  const Vec3T<T> lo = a_box.getLowCorner();
  const Vec3T<T> hi = a_box.getHighCorner();

  const T diagonal = (hi - lo).length();

  EBGEOMETRY_REQUIRE(std::isfinite(diagonal) && lo[0] <= hi[0] && lo[1] <= hi[1] && lo[2] <= hi[2],
                     "Render::Camera: fit() needs a bounded, non-empty box (diagonal %g); give an unbounded "
                     "shape's camera with lookAt() or a box of your own",
                     double(diagonal));
  EBGEOMETRY_REQUIRE(a_up.length() > T(0), "Render::Camera: the up axis must not be zero");

  Camera camera;

  camera.m_target   = T(0.5) * (lo + hi);
  camera.m_up       = a_up;
  camera.m_distance = Math::max(T(0.5) * diagonal, Math::Limits<T>::min()) /
                      std::sin(T(0.5) * camera.m_fieldOfView * pi<T> / T(180)) * T(1.05);

  return camera;
}

template <class T>
EBGEOMETRY_HOST
inline Camera<T>
Camera<T>::lookAt(const Vec3T<T>& a_eye, const Vec3T<T>& a_target, const Vec3T<T>& a_up, const T a_fieldOfView)
{
  Camera camera;

  camera.m_target      = a_target;
  camera.m_up          = a_up;
  camera.m_fieldOfView = a_fieldOfView;

  const Vec3T<T> offset   = a_eye - a_target;
  const T        distance = offset.length();

  EBGEOMETRY_REQUIRE(distance > T(0), "Render::Camera: the eye must not be at the target");
  EBGEOMETRY_REQUIRE(a_up.length() > T(0), "Render::Camera: the up axis must not be zero");

  const Vec3T<T> w = offset / distance;
  const Vec3T<T> u = RenderDetail::unit(a_up);

  Vec3T<T> first;
  Vec3T<T> second;

  camera.horizontal(first, second);

  const T degrees = T(180) / pi<T>;

  camera.m_distance = distance;
  camera.m_pitch    = std::asin(Math::clamp(dot(w, u), T(-1), T(1))) * degrees;

  // Straight along the up axis the yaw is undefined; take 0 rather than whatever rounding leaves.
  const T across = std::sqrt(dot(w, first) * dot(w, first) + dot(w, second) * dot(w, second));

  camera.m_yaw = (across > T(1e-6)) ? std::atan2(dot(w, first), dot(w, second)) * degrees : T(0);

  return camera;
}

template <class T>
EBGEOMETRY_HOST
inline Camera<T>
Camera<T>::fit(const BoundingVolumes::AABBT<T>& a_box, const View a_view, const Vec3T<T>& a_up)
{
  Camera camera = Camera::fit(a_box, a_up);

  switch (a_view) {
  case View::Front: {
    camera.m_yaw   = T(0);
    camera.m_pitch = T(0);

    break;
  }
  case View::Back: {
    camera.m_yaw   = T(180);
    camera.m_pitch = T(0);

    break;
  }
  case View::Right: {
    camera.m_yaw   = T(90);
    camera.m_pitch = T(0);

    break;
  }
  case View::Left: {
    camera.m_yaw   = T(-90);
    camera.m_pitch = T(0);

    break;
  }
  case View::Top: {
    camera.m_yaw   = T(0);
    camera.m_pitch = T(90);

    break;
  }
  case View::Bottom: {
    camera.m_yaw   = T(0);
    camera.m_pitch = T(-90);

    break;
  }
  default: {
    break;
  }
  }

  return camera;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
Camera<T>::horizontal(Vec3T<T>& a_first, Vec3T<T>& a_second) const noexcept
{
  const Vec3T<T> u = RenderDetail::unit(m_up);

  // The second direction is +z made perpendicular to the up axis (or +x, if up is close to z).
  Vec3T<T> reference(T(0), T(0), T(1));

  if (std::abs(dot(reference, u)) > T(0.9)) {
    reference = Vec3T<T>(T(1), T(0), T(0));
  }

  a_second = RenderDetail::unit(reference - dot(reference, u) * u);
  a_first  = cross(u, a_second);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline Vec3T<T>
Camera<T>::eye() const noexcept
{
  Vec3T<T> first;
  Vec3T<T> second;

  this->horizontal(first, second);

  const T radians = pi<T> / T(180);
  const T y       = m_yaw * radians;
  const T p       = m_pitch * radians;

  const Vec3T<T> w =
    std::cos(p) * std::sin(y) * first + std::sin(p) * RenderDetail::unit(m_up) + std::cos(p) * std::cos(y) * second;

  return m_target + m_distance * w;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
Camera<T>::frame(Vec3T<T>& a_forward, Vec3T<T>& a_right, Vec3T<T>& a_up) const noexcept
{
  Vec3T<T> first;
  Vec3T<T> second;

  this->horizontal(first, second);

  // The right is horizontal and set by the yaw alone, so it stays defined looking straight along the up axis.
  const T y = m_yaw * pi<T> / T(180);

  a_forward = RenderDetail::unit(m_target - this->eye());
  a_right   = std::cos(y) * first - std::sin(y) * second;
  a_up      = cross(a_right, a_forward);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline Array<Vec3T<T>, Settings<T>::PrimitiveColors>
Settings<T>::unsetColors() noexcept
{
  Array<Vec3T<T>, PrimitiveColors> colors;

  for (uint32_t i = 0; i < PrimitiveColors; i++) {
    colors[i] = Vec3T<T>(T(-1), T(-1), T(-1));
  }

  return colors;
}

namespace RenderDetail {

/**
 * @brief A number as a C++ literal of precision T, with the fewest digits that read back as the same value.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_value Value.
 * @return For example "T(0.25)".
 */
template <class T>
EBGEOMETRY_HOST
inline std::string
literal(const T a_value)
{
  char text[64];

  // Nine significant digits always read back a float, seventeen a double.
  const int maxDigits = (sizeof(T) <= sizeof(float)) ? 9 : 17;

  for (int digits = 1; digits <= maxDigits; digits++) {
    std::snprintf(text, sizeof(text), "%.*g", digits, double(a_value));

    if (T(std::strtod(text, nullptr)) == a_value) {
      break;
    }
  }

  return std::string("T(") + text + ")";
}

/**
 * @brief A vector as a C++ expression of precision T.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_v Vector.
 * @return For example "EBGeometry::Vec3T<T>(T(1), T(0), T(0))".
 */
template <class T>
EBGEOMETRY_HOST
inline std::string
literal(const Vec3T<T>& a_v)
{
  return "EBGeometry::Vec3T<T>(" + literal(a_v[0]) + ", " + literal(a_v[1]) + ", " + literal(a_v[2]) + ")";
}

/**
 * @brief A coloring as a C++ expression.
 * @details Internal helper; not part of the public API.
 * @param[in] a_colorBy Coloring.
 * @return For example "EBGeometry::Render::ColorBy::Primitive".
 */
EBGEOMETRY_HOST
inline std::string
literal(const ColorBy a_colorBy)
{
  switch (a_colorBy) {
  case ColorBy::Primitive: {
    return "EBGeometry::Render::ColorBy::Primitive";
  }
  case ColorBy::Normal: {
    return "EBGeometry::Render::ColorBy::Normal";
  }
  default: {
    return "EBGeometry::Render::ColorBy::Uniform";
  }
  }
}

/**
 * @brief A view mode as a C++ literal.
 * @details Internal helper; not part of the public API.
 * @param[in] a_mode View mode.
 * @return The enumerator's qualified name.
 */
EBGEOMETRY_HOST
inline std::string
literal(const ViewMode a_mode)
{
  switch (a_mode) {
  case ViewMode::Depth: {
    return "EBGeometry::Render::ViewMode::Depth";
  }
  case ViewMode::XRay: {
    return "EBGeometry::Render::ViewMode::XRay";
  }
  case ViewMode::CostHit: {
    return "EBGeometry::Render::ViewMode::CostHit";
  }
  case ViewMode::CostCrossing: {
    return "EBGeometry::Render::ViewMode::CostCrossing";
  }
  default: {
    return "EBGeometry::Render::ViewMode::Shaded";
  }
  }
}

/**
 * @brief A flag as a C++ literal.
 * @details Internal helper; not part of the public API.
 * @param[in] a_flag Flag.
 * @return "true" or "false".
 */
EBGEOMETRY_HOST
inline std::string
literal(const bool a_flag)
{
  return a_flag ? "true" : "false";
}

/**
 * @brief A count as a C++ literal.
 * @details Internal helper; not part of the public API.
 * @param[in] a_count Count.
 * @return For example "960".
 */
EBGEOMETRY_HOST
inline std::string
literal(const uint32_t a_count)
{
  return std::to_string(a_count);
}

/**
 * @brief Whether two vectors are equal, component by component.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_a One vector.
 * @param[in] a_b The other.
 * @return True if equal.
 */
template <class T>
EBGEOMETRY_HOST
inline bool
same(const Vec3T<T>& a_a, const Vec3T<T>& a_b)
{
  return a_a[0] == a_b[0] && a_a[1] == a_b[1] && a_a[2] == a_b[2];
}

/**
 * @brief Whether two values are equal.
 * @details Internal helper; not part of the public API.
 * @tparam V Value type.
 * @param[in] a_a One value.
 * @param[in] a_b The other.
 * @return True if equal.
 */
template <class V>
EBGEOMETRY_HOST
inline bool
same(const V& a_a, const V& a_b)
{
  return a_a == a_b;
}

} // namespace RenderDetail

template <class T>
EBGEOMETRY_HOST
inline std::string
toCode(const Camera<T>& a_camera, const Settings<T>& a_settings)
{
  using RenderDetail::literal;
  using RenderDetail::same;

  const Camera<T>   camera;
  const Settings<T> settings;

  std::string code = "EBGeometry::Render::Camera<T> camera;\n";

  // One statement per field that differs from its default.
  const auto field = [&code](const char* a_object, const char* a_name, const auto& a_value, const auto& a_default) {
    if (!same(a_value, a_default)) {
      code += std::string(a_object) + "." + a_name + " = " + literal(a_value) + ";\n";
    }
  };

  field("camera", "m_target", a_camera.m_target, camera.m_target);
  field("camera", "m_yaw", a_camera.m_yaw, camera.m_yaw);
  field("camera", "m_pitch", a_camera.m_pitch, camera.m_pitch);
  field("camera", "m_distance", a_camera.m_distance, camera.m_distance);
  field("camera", "m_fieldOfView", a_camera.m_fieldOfView, camera.m_fieldOfView);
  field("camera", "m_up", a_camera.m_up, camera.m_up);
  field("camera", "m_orthographic", a_camera.m_orthographic, camera.m_orthographic);
  field("camera", "m_nearClip", a_camera.m_nearClip, camera.m_nearClip);

  code += "EBGeometry::Render::Settings<T> settings;\n";

  field("settings", "m_width", a_settings.m_width, settings.m_width);
  field("settings", "m_height", a_settings.m_height, settings.m_height);
  field("settings", "m_supersampling", a_settings.m_supersampling, settings.m_supersampling);
  field("settings", "m_threads", a_settings.m_threads, settings.m_threads);
  field("settings", "m_shadows", a_settings.m_shadows, settings.m_shadows);
  field("settings", "m_occlusion", a_settings.m_occlusion, settings.m_occlusion);
  field("settings", "m_floor", a_settings.m_floor, settings.m_floor);
  field("settings", "m_sky", a_settings.m_sky, settings.m_sky);
  field("settings", "m_colorBy", a_settings.m_colorBy, settings.m_colorBy);
  field("settings", "m_color", a_settings.m_color, settings.m_color);

  for (uint32_t i = 0; i < Settings<T>::PrimitiveColors; i++) {
    const std::string name = "m_primitiveColors[" + std::to_string(i) + "]";

    field("settings", name.c_str(), a_settings.m_primitiveColors[i], settings.m_primitiveColors[i]);
  }

  for (uint32_t i = 0; i < Settings<T>::PrimitiveColors; i++) {
    const std::string name = "m_materialColors[" + std::to_string(i) + "]";

    field("settings", name.c_str(), a_settings.m_materialColors[i], settings.m_materialColors[i]);
  }

  field("settings", "m_recordPrimitives", a_settings.m_recordPrimitives, settings.m_recordPrimitives);
  field("settings", "m_highlightPrimitive", a_settings.m_highlightPrimitive, settings.m_highlightPrimitive);
  field("settings", "m_highlightCount", a_settings.m_highlightCount, settings.m_highlightCount);
  field("settings", "m_highlightColor", a_settings.m_highlightColor, settings.m_highlightColor);
  field("settings", "m_insideColor", a_settings.m_insideColor, settings.m_insideColor);
  field("settings", "m_cutColor", a_settings.m_cutColor, settings.m_cutColor);
  field("settings", "m_cutOpacity", a_settings.m_cutOpacity, settings.m_cutOpacity);
  field("settings", "m_seenThroughOpacity", a_settings.m_seenThroughOpacity, settings.m_seenThroughOpacity);
  field("settings", "m_floorColor", a_settings.m_floorColor, settings.m_floorColor);
  field("settings", "m_lightFromCamera", a_settings.m_lightFromCamera, settings.m_lightFromCamera);
  field("settings", "m_light", a_settings.m_light, settings.m_light);
  field("settings", "m_lightIntensity", a_settings.m_lightIntensity, settings.m_lightIntensity);
  field("settings", "m_ambient", a_settings.m_ambient, settings.m_ambient);
  field("settings", "m_specular", a_settings.m_specular, settings.m_specular);
  field("settings", "m_shininess", a_settings.m_shininess, settings.m_shininess);
  field("settings", "m_skyHorizon", a_settings.m_skyHorizon, settings.m_skyHorizon);
  field("settings", "m_skyZenith", a_settings.m_skyZenith, settings.m_skyZenith);
  field("settings", "m_background", a_settings.m_background, settings.m_background);
  field("settings", "m_floorChecker", a_settings.m_floorChecker, settings.m_floorChecker);
  field("settings", "m_floorCheckerSize", a_settings.m_floorCheckerSize, settings.m_floorCheckerSize);
  field("settings", "m_section", a_settings.m_section, settings.m_section);
  field("settings", "m_sectionPoint", a_settings.m_sectionPoint, settings.m_sectionPoint);
  field("settings", "m_sectionNormal", a_settings.m_sectionNormal, settings.m_sectionNormal);
  field("settings", "m_contourWidth", a_settings.m_contourWidth, settings.m_contourWidth);
  field("settings", "m_contourColor", a_settings.m_contourColor, settings.m_contourColor);
  field("settings", "m_isoSpacing", a_settings.m_isoSpacing, settings.m_isoSpacing);
  field("settings", "m_isoColor", a_settings.m_isoColor, settings.m_isoColor);
  field("settings", "m_isoSecondaryColor", a_settings.m_isoSecondaryColor, settings.m_isoSecondaryColor);
  field("settings", "m_isoSecondary", a_settings.m_isoSecondary, settings.m_isoSecondary);
  field("settings", "m_sheetOpacity", a_settings.m_sheetOpacity, settings.m_sheetOpacity);
  field("settings", "m_sectionFill", a_settings.m_sectionFill, settings.m_sectionFill);
  field("settings", "m_fillSecondaryColor", a_settings.m_fillSecondaryColor, settings.m_fillSecondaryColor);
  field("settings", "m_sheetColor", a_settings.m_sheetColor, settings.m_sheetColor);
  field("settings", "m_sheetEdgeColor", a_settings.m_sheetEdgeColor, settings.m_sheetEdgeColor);
  field("settings", "m_rays.m_tolerance", a_settings.m_rays.m_tolerance, settings.m_rays.m_tolerance);
  field("settings", "m_rays.m_scale", a_settings.m_rays.m_scale, settings.m_rays.m_scale);
  field("settings", "m_rays.m_maxSteps", a_settings.m_rays.m_maxSteps, settings.m_rays.m_maxSteps);
  field("settings", "m_rays.m_step", a_settings.m_rays.m_step, settings.m_rays.m_step);
  field("settings", "m_rays.m_march", a_settings.m_rays.m_march, settings.m_rays.m_march);
  field("settings", "m_viewMode", a_settings.m_viewMode, settings.m_viewMode);
  field("settings", "m_xrayOpacity", a_settings.m_xrayOpacity, settings.m_xrayOpacity);

  for (uint32_t i = 0; i < Settings<T>::HiddenPrimitives; i++) {
    const std::string name = "m_hidden[" + std::to_string(i) + "]";

    field("settings", name.c_str(), a_settings.m_hidden[i], settings.m_hidden[i]);
  }

  field("settings", "m_volumePrimitive", a_settings.m_volumePrimitive, settings.m_volumePrimitive);
  field("settings", "m_volumeColor", a_settings.m_volumeColor, settings.m_volumeColor);
  field("settings", "m_volumeOpacity", a_settings.m_volumeOpacity, settings.m_volumeOpacity);

  return code;
}

inline double
costPlace(const Stats& a_stats, const double a_cost) noexcept
{
  // The pixels were colored by log2 of their cost, between those of m_costLow and m_costHigh.
  if (a_stats.m_costUnit == CostUnit::None || !(a_stats.m_costLow > 0.0) || !(a_stats.m_costHigh > a_stats.m_costLow) ||
      !(a_cost > 0.0)) {
    return 0.0;
  }

  const double place = std::log2(a_cost / a_stats.m_costLow) / std::log2(a_stats.m_costHigh / a_stats.m_costLow);

  return Math::clamp(place, 0.0, 1.0);
}

inline uint32_t
costColor(const double a_place) noexcept
{
  const double        place = (a_place >= 0.0) ? Math::min(a_place, 1.0) : 0.0;
  const Vec3T<double> c     = RenderDetail::ramp<double>(place);

  uint32_t color = 0xFFU;

  for (int k = 0; k < 3; k++) {
    color |= static_cast<uint32_t>(255.0 * c[k] + 0.5) << (24 - 8 * k);
  }

  return color;
}

inline bool
writeImage(const Image& a_image, const std::string& a_filename)
{
  // An image whose pixels do not fill its size is not written.
  if (a_image.m_pixels.size() < size_t(a_image.m_width) * size_t(a_image.m_height)) {
    return false;
  }

  FILE* file = std::fopen(a_filename.c_str(), "wb");

  if (file == nullptr) {
    return false;
  }

  bool written = std::fprintf(file, "P6\n%u %u\n255\n", unsigned(a_image.m_width), unsigned(a_image.m_height)) > 0;

  std::vector<unsigned char> row(3 * size_t(a_image.m_width));

  for (uint32_t r = 0; r < a_image.m_height && written; r++) {
    for (uint32_t c = 0; c < a_image.m_width; c++) {
      const uint32_t rgba = a_image.m_pixels[size_t(r) * a_image.m_width + c];

      row[3 * size_t(c) + 0] = static_cast<unsigned char>(rgba >> 24);
      row[3 * size_t(c) + 1] = static_cast<unsigned char>(rgba >> 16);
      row[3 * size_t(c) + 2] = static_cast<unsigned char>(rgba >> 8);
    }

    written = std::fwrite(row.data(), 1, row.size(), file) == row.size();
  }

  return (std::fclose(file) == 0) && written;
}

inline uint32_t
Image::at(const uint32_t a_column, const uint32_t a_row) const noexcept
{
  EBGEOMETRY_EXPECT(a_column < m_width && a_row < m_height);

  return m_pixels[size_t(a_row) * size_t(m_width) + size_t(a_column)];
}

template <class T, class S>
EBGEOMETRY_HOST_DEVICE
inline PixelQuery<T, S>::PixelQuery(const S& a_shape, const Camera<T>& a_camera, const Settings<T>& a_settings) noexcept
  : m_shape(a_shape), m_camera(a_camera), m_settings(a_settings)
{
  m_view.m_eye      = a_camera.eye();
  m_view.m_axis     = RenderDetail::unit(a_camera.m_up);
  m_view.m_tanHalf  = std::tan(T(0.5) * a_camera.m_fieldOfView * pi<T> / T(180));
  m_view.m_distance = a_camera.m_distance;

  // A pixel spans 2 tan(fov/2) / height of the view's height at unit distance.
  const T pixel = T(2) * m_view.m_tanHalf / T(Math::max(a_settings.m_height, 1U));

  m_view.m_footprint          = a_camera.m_orthographic ? a_camera.m_distance * pixel : T(0);
  m_view.m_footprintPerLength = a_camera.m_orthographic ? T(0) : pixel;
  m_view.m_orthographic       = a_camera.m_orthographic;

  a_camera.frame(m_view.m_forward, m_view.m_right, m_view.m_up);
  a_camera.horizontal(m_view.m_first, m_view.m_second);
}

template <class T, class S>
EBGEOMETRY_HOST_DEVICE
inline PixelResult
PixelQuery<T, S>::operator()(const uint32_t& a_index) const noexcept
{
  RenderDetail::Shader<T, S> shader(m_shape, m_view, m_settings);

  PixelResult result;

  if (a_index == 0U) {
    result.m_cameraInside = shader.eyeInside() ? 1U : 0U;
  }

  const Vec3T<T>& forward = m_view.m_forward;
  const Vec3T<T>& right   = m_view.m_right;
  const Vec3T<T>& up      = m_view.m_up;

  const T tanHalf = m_view.m_tanHalf;
  const T w       = T(m_settings.m_width);
  const T h       = T(m_settings.m_height);
  const T column  = T(a_index % m_settings.m_width);
  const T row     = T(a_index / m_settings.m_width);

  const uint32_t n = m_settings.m_supersampling;

  Vec3T<T> color = Vec3T<T>::zeros();

  for (uint32_t s = 0; s < n * n; s++) {
    const T dx = (T(s % n) + T(0.5)) / T(n);
    const T dy = (T(s / n) + T(0.5)) / T(n);

    // Screen coordinates: x from -w/h to w/h, y from -1 (bottom) to 1 (top), times tan(fov/2).
    const T x = (T(2) * (column + dx) - w) / h * tanHalf;
    const T y = (h - T(2) * (row + dy)) / h * tanHalf;

    if (n > 1U) {
      shader.jitterLight(dx, dy);
    }

    // Perspective rays leave the eye; orthographic ones are parallel, behind the shape, spread as far as the
    // perspective rays are at the target. One call to shade(): two (one per kind) copied its whole body into
    // the kernel twice, and doubled what it spills.
    const Vec3T<T> origin    = m_camera.m_orthographic
                                 ? shader.orthographicStart(m_view.m_eye + m_view.m_distance * (x * right + y * up))
                                 : m_view.m_eye;
    const Vec3T<T> direction = m_camera.m_orthographic ? forward : RenderDetail::unit(forward + x * right + y * up);
    const T        near      = m_camera.m_orthographic ? shader.nearAlong(m_camera.m_nearClip) : m_camera.m_nearClip;

    color += shader.shade(origin, direction, near, result);
  }

  color = color / T(n * n);

  uint32_t rgba = 0xFFU;

  for (int k = 0; k < 3; k++) {
    const T g = std::pow(Math::clamp(color[k], T(0), T(1)), T(1) / T(2.2));

    rgba |= static_cast<uint32_t>(T(255) * g + T(0.5)) << (24 - 8 * k);
  }

  result.m_rgba = rgba;

  return result;
}

namespace RenderDetail {

/**
 * @brief One pixel of a rendered image in the views PixelQuery does not draw: the views other than the shaded one
 * (see ViewMode), and the shaded view with hidden primitives or a filled part.
 * @details Internal helper; not part of the public API. A functor of its own, so that the shaded view's kernel
 * holds none of this code: on a device, every function a kernel can reach takes part in its register allocation,
 * run or not. In a cost view, the pixel's m_rgba holds its cost (at least 1; 0 is a pixel a canceled render did
 * not reach), which Renderer turns into a color once the frame's range is known.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 */
template <class T, class S>
struct InspectQuery
{
  /**
   * @brief The shape, the camera and the settings, as the shaded view's functor holds them.
   */
  PixelQuery<T, S> m_base;

  /**
   * @brief Draw pixel a_index (row a_index / width, column a_index % width).
   * @param[in] a_index Pixel index.
   * @return The pixel.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline PixelResult
  operator()(const uint32_t& a_index) const noexcept
  {
    const CameraView<T>& view     = m_base.m_view;
    const Settings<T>&   settings = m_base.m_settings;
    const Camera<T>&     camera   = m_base.m_camera;

    Shader<T, S> shader(m_base.m_shape, view, settings);

    PixelResult result;

    if (a_index == 0U) {
      result.m_cameraInside = shader.eyeInside() ? 1U : 0U;
    }

    const T w      = T(settings.m_width);
    const T h      = T(settings.m_height);
    const T column = T(a_index % settings.m_width);
    const T row    = T(a_index / settings.m_width);

    // A cost view times one ray per pixel, through its center.
    const bool     timed = settings.m_viewMode == ViewMode::CostHit || settings.m_viewMode == ViewMode::CostCrossing;
    const uint32_t n     = timed ? 1U : settings.m_supersampling;

    Vec3T<T> color = Vec3T<T>::zeros();
    uint64_t cost  = 0U;

    for (uint32_t s = 0; s < n * n; s++) {
      const T dx = (T(s % n) + T(0.5)) / T(n);
      const T dy = (T(s / n) + T(0.5)) / T(n);

      // The rays PixelQuery casts.
      const T x = (T(2) * (column + dx) - w) / h * view.m_tanHalf;
      const T y = (h - T(2) * (row + dy)) / h * view.m_tanHalf;

      if (n > 1U) {
        shader.jitterLight(dx, dy);
      }

      const Vec3T<T> origin =
        camera.m_orthographic
          ? shader.orthographicStart(view.m_eye + view.m_distance * (x * view.m_right + y * view.m_up))
          : view.m_eye;
      const Vec3T<T> direction =
        camera.m_orthographic ? view.m_forward : unit(view.m_forward + x * view.m_right + y * view.m_up);
      const T near = camera.m_orthographic ? shader.nearAlong(camera.m_nearClip) : camera.m_nearClip;

      color += shader.inspect(origin, direction, near, result, cost);
    }

    if (timed) {
      result.m_rgba = static_cast<uint32_t>(Math::clamp(cost, uint64_t(1), uint64_t(0xFFFFFFFFU)));

      return result;
    }

    color = color / T(n * n);

    uint32_t rgba = 0xFFU;

    for (int k = 0; k < 3; k++) {
      const T g = std::pow(Math::clamp(color[k], T(0), T(1)), T(1) / T(2.2));

      rgba |= static_cast<uint32_t>(T(255) * g + T(0.5)) << (24 - 8 * k);
    }

    result.m_rgba = rgba;

    return result;
  }
};

} // namespace RenderDetail

template <class T>
EBGEOMETRY_HOST_DEVICE
inline Array<uint32_t, Settings<T>::HiddenPrimitives>
Settings<T>::noneHidden() noexcept
{
  Array<uint32_t, HiddenPrimitives> hidden;

  for (uint32_t i = 0; i < HiddenPrimitives; i++) {
    hidden[i] = NoPrimitive;
  }

  return hidden;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline bool
Settings<T>::hides(const uint32_t a_primitive) const noexcept
{
  bool hidden = false;

  for (uint32_t i = 0; i < HiddenPrimitives; i++) {
    hidden = hidden || (a_primitive != NoPrimitive && m_hidden[i] == a_primitive);
  }

  return hidden;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline bool
Settings<T>::hidesAny() const noexcept
{
  bool any = false;

  for (uint32_t i = 0; i < HiddenPrimitives; i++) {
    any = any || m_hidden[i] != NoPrimitive;
  }

  return any;
}

template <class T>
Renderer<T>::Renderer(const Settings<T>& a_settings) noexcept : m_settings(a_settings)
{}

template <class T>
inline Settings<T>&
Renderer<T>::settings() noexcept
{
  return m_settings;
}

template <class T>
inline const Settings<T>&
Renderer<T>::settings() const noexcept
{
  return m_settings;
}

template <class T>
template <class S>
inline HitT<T>
Renderer<T>::pick(const S& a_shape, const Camera<T>& a_camera, const T a_column, const T a_row) const
{
  this->check(a_camera);

  // The ray PixelQuery casts through the point.
  const PixelQuery<T, S>             query(a_shape, a_camera, m_settings);
  const RenderDetail::CameraView<T>& view = query.m_view;
  const RenderDetail::Shader<T, S>   shader(a_shape, view, m_settings);

  const T w = T(m_settings.m_width);
  const T h = T(m_settings.m_height);
  const T x = (T(2) * a_column - w) / h * view.m_tanHalf;
  const T y = (h - T(2) * a_row) / h * view.m_tanHalf;

  if (a_camera.m_orthographic) {
    return shader.cast(shader.orthographicStart(view.m_eye + view.m_distance * (x * view.m_right + y * view.m_up)),
                       view.m_forward,
                       shader.nearAlong(a_camera.m_nearClip));
  }

  return shader.cast(
    view.m_eye, RenderDetail::unit(view.m_forward + x * view.m_right + y * view.m_up), a_camera.m_nearClip);
}

template <class T>
template <class S>
inline std::vector<HitT<T>>
Renderer<T>::pickAll(const S& a_shape, const Camera<T>& a_camera, const T a_column, const T a_row) const
{
  this->check(a_camera);

  // pick()'s ray.
  const PixelQuery<T, S>             query(a_shape, a_camera, m_settings);
  const RenderDetail::CameraView<T>& view = query.m_view;
  const RenderDetail::Shader<T, S>   shader(a_shape, view, m_settings);

  const T w = T(m_settings.m_width);
  const T h = T(m_settings.m_height);
  const T x = (T(2) * a_column - w) / h * view.m_tanHalf;
  const T y = (h - T(2) * a_row) / h * view.m_tanHalf;

  std::vector<HitT<T>> hits;

  if (a_camera.m_orthographic) {
    shader.castAll(shader.orthographicStart(view.m_eye + view.m_distance * (x * view.m_right + y * view.m_up)),
                   view.m_forward,
                   shader.nearAlong(a_camera.m_nearClip),
                   hits);
  }
  else {
    shader.castAll(
      view.m_eye, RenderDetail::unit(view.m_forward + x * view.m_right + y * view.m_up), a_camera.m_nearClip, hits);
  }

  return hits;
}

template <class T>
inline bool
Renderer<T>::inspecting() const noexcept
{
  return m_settings.m_viewMode != ViewMode::Shaded || m_settings.hidesAny() ||
         m_settings.m_volumePrimitive != Settings<T>::NoPrimitive;
}

template <class T>
inline void
Renderer<T>::check(const Camera<T>& a_camera) const
{
  EBGEOMETRY_REQUIRE(m_settings.m_width > 0U && m_settings.m_height > 0U,
                     "Render::Renderer: the image must have pixels (%u x %u)",
                     unsigned(m_settings.m_width),
                     unsigned(m_settings.m_height));
  EBGEOMETRY_REQUIRE(m_settings.m_supersampling >= 1U && m_settings.m_supersampling <= 15U,
                     "Render::Renderer: supersampling must be from 1 to 15 (%u)",
                     unsigned(m_settings.m_supersampling));
  EBGEOMETRY_REQUIRE(std::isfinite(a_camera.m_distance) && a_camera.m_distance > T(0),
                     "Render::Renderer: the camera distance must be positive and finite (%g)",
                     double(a_camera.m_distance));
  EBGEOMETRY_REQUIRE(a_camera.m_fieldOfView > T(0) && a_camera.m_fieldOfView < T(180),
                     "Render::Renderer: the field of view must be between 0 and 180 degrees (%g)",
                     double(a_camera.m_fieldOfView));
  EBGEOMETRY_REQUIRE(std::abs(a_camera.m_pitch) <= T(90),
                     "Render::Renderer: the camera pitch must be from -90 to 90 degrees (%g)",
                     double(a_camera.m_pitch));
  EBGEOMETRY_REQUIRE(std::isfinite(a_camera.m_nearClip) && a_camera.m_nearClip >= T(0),
                     "Render::Renderer: the near clip must be finite and not negative (%g)",
                     double(a_camera.m_nearClip));
  EBGEOMETRY_REQUIRE(m_settings.m_isoSpacing >= T(0),
                     "Render::Renderer: the iso-line spacing must not be negative (%g)",
                     double(m_settings.m_isoSpacing));
  EBGEOMETRY_REQUIRE(m_settings.m_sectionFill >= T(0) && m_settings.m_sectionFill <= T(1),
                     "Render::Renderer: the section plane's fill must be from 0 to 1 (%g)",
                     double(m_settings.m_sectionFill));
  EBGEOMETRY_REQUIRE(m_settings.m_cutOpacity >= T(0) && m_settings.m_cutOpacity <= T(1),
                     "Render::Renderer: the cut face's opacity must be from 0 to 1 (%g)",
                     double(m_settings.m_cutOpacity));
  EBGEOMETRY_REQUIRE(m_settings.m_seenThroughOpacity >= T(0) && m_settings.m_seenThroughOpacity <= T(1),
                     "Render::Renderer: the opacity of the surfaces seen through must be from 0 to 1 (%g)",
                     double(m_settings.m_seenThroughOpacity));
  EBGEOMETRY_REQUIRE(m_settings.m_contourWidth >= T(0) && m_settings.m_sheetOpacity >= T(0) &&
                       m_settings.m_sheetOpacity <= T(1),
                     "Render::Renderer: the contour width must not be negative, and the sheet's opacity must be "
                     "from 0 to 1 (%g, %g)",
                     double(m_settings.m_contourWidth),
                     double(m_settings.m_sheetOpacity));
  EBGEOMETRY_REQUIRE(m_settings.m_shininess >= T(1),
                     "Render::Renderer: the shininess must be at least 1 (%g)",
                     double(m_settings.m_shininess));
  EBGEOMETRY_REQUIRE(m_settings.m_lightIntensity >= T(0) && m_settings.m_ambient >= T(0) &&
                       m_settings.m_specular >= T(0) && m_settings.m_floorCheckerSize >= T(0),
                     "Render::Renderer: the light intensity, ambient level, specular strength and checker size "
                     "must not be negative (%g, %g, %g, %g)",
                     double(m_settings.m_lightIntensity),
                     double(m_settings.m_ambient),
                     double(m_settings.m_specular),
                     double(m_settings.m_floorCheckerSize));
  EBGEOMETRY_REQUIRE(a_camera.m_up.length() > T(0), "Render::Renderer: the camera's up axis must not be zero");
  EBGEOMETRY_REQUIRE(m_settings.m_light.length() > T(0), "Render::Renderer: the light direction must not be zero");
  EBGEOMETRY_REQUIRE(!m_settings.m_section || m_settings.m_sectionNormal.length() > T(0),
                     "Render::Renderer: the section plane's normal must not be zero");
  EBGEOMETRY_REQUIRE(m_settings.m_xrayOpacity >= T(0) && m_settings.m_xrayOpacity <= T(1) &&
                       m_settings.m_volumeOpacity >= T(0) && m_settings.m_volumeOpacity <= T(1),
                     "Render::Renderer: the x-ray's and the fill's opacities must be from 0 to 1 (%g, %g)",
                     double(m_settings.m_xrayOpacity),
                     double(m_settings.m_volumeOpacity));
}

template <class T>
inline Stats
Renderer<T>::collect(const std::vector<RenderDetail::StoredPixel>& a_results,
                     std::vector<uint32_t>&                        a_primitives,
                     Image&                                        a_image,
                     const CostUnit                                a_unit,
                     const double                                  a_seconds) const
{
  a_image.m_width  = m_settings.m_width;
  a_image.m_height = m_settings.m_height;
  a_image.m_pixels.resize(a_results.size());
  a_image.m_primitives.swap(a_primitives);
  a_primitives.clear();

  Stats stats;

  for (size_t i = 0; i < a_results.size(); i++) {
    a_image.m_pixels[i] = a_results[i].m_rgba;

    stats.m_hits += a_results[i].m_hits;
    stats.m_insideHits += a_results[i].m_insideHits;
    stats.m_unresolved += a_results[i].m_unresolved;
  }

  const bool     timed = m_settings.m_viewMode == ViewMode::CostHit || m_settings.m_viewMode == ViewMode::CostCrossing;
  const uint64_t n     = timed ? 1U : m_settings.m_supersampling;

  stats.m_rays         = uint64_t(a_results.size()) * n * n;
  stats.m_cameraInside = !a_results.empty() && a_results.front().m_cameraInside != 0U;

  if (timed) {
    // Each pixel holds its cost (0 where a canceled render did not reach it): colored on a log scale over the
    // frame's costs without the cheapest and the dearest hundredth of its pixels, which a thread interrupted
    // mid-ray (a whole row of slow pixels on the host) would otherwise stretch the scale with. The costs are put in
    // buckets of a sixteenth of a doubling, from their bits alone, so that this pass, run on the host after a
    // device render too, takes no sort and no logarithm per pixel: one pass to count, one to color.
    constexpr uint32_t PerDoubling = 16;
    constexpr uint32_t Buckets     = 32 * PerDoubling;

    // The highest set bit of a cost, and its bucket: that bit and the four below it.
    const auto highestBit = [](uint32_t a_cost) noexcept {
      uint32_t exponent = 0;

      for (uint32_t shift = 16; shift > 0; shift /= 2) {
        if ((a_cost >> shift) != 0U) {
          a_cost >>= shift;
          exponent += shift;
        }
      }

      return exponent;
    };

    const auto bucketOf = [&highestBit](const uint32_t a_cost) noexcept {
      const uint32_t exponent = highestBit(a_cost);
      const uint32_t fraction =
        (exponent >= 4U) ? (a_cost >> (exponent - 4U)) & 15U : (a_cost << (4U - exponent)) & 15U;

      return exponent * PerDoubling + fraction;
    };

    std::vector<uint64_t> counts(Buckets, 0U);

    uint64_t total = 0;

    for (const uint32_t cost : a_image.m_pixels) {
      if (cost != 0U) {
        counts[bucketOf(cost)]++;
        total++;
      }
    }

    // The buckets of the cheapest and the dearest pixel kept, a hundredth of the pixels cut from either end.
    const uint64_t cut  = total / 100U;
    uint32_t       low  = 0;
    uint32_t       high = Buckets - 1U;

    for (uint64_t below = 0; low < Buckets - 1U && below + counts[low] <= cut; low++) {
      below += counts[low];
    }

    for (uint64_t above = 0; high > low && above + counts[high] <= cut; high--) {
      above += counts[high];
    }

    std::vector<uint32_t> colors(Buckets);

    for (uint32_t b = 0; b < Buckets; b++) {
      colors[b] = costColor((high > low) ? (double(b) - double(low)) / double(high - low) : 0.0);
    }

    for (uint32_t& pixel : a_image.m_pixels) {
      if (pixel != 0U) {
        pixel = colors[bucketOf(pixel)];
      }
    }

    // A bucket's cost is that of its lower end: 2^(b / 16).
    const auto costOf = [](const uint32_t a_bucket) { return std::exp2(double(a_bucket) / double(PerDoubling)); };

    stats.m_costUnit = a_unit;
    stats.m_costLow  = (total == 0U) ? 0.0 : costOf(low);
    stats.m_costHigh = (total == 0U) ? 0.0 : costOf(high);

    // The rays traced: those of the pixels reached, which a canceled render does not all reach.
    stats.m_raysPerSecond = (a_seconds > 0.0) ? double(total) / a_seconds : 0.0;
  }

  return stats;
}

template <class T>
template <class S>
inline Stats
Renderer<T>::render(const S& a_shape, const Camera<T>& a_camera, Image& a_image) const
{
  const std::atomic<bool> never{false};

  return this->render(a_shape, a_camera, a_image, never);
}

template <class T>
template <class S>
inline Stats
Renderer<T>::render(const S&                 a_shape,
                    const Camera<T>&         a_camera,
                    Image&                   a_image,
                    const std::atomic<bool>& a_cancel,
                    std::atomic<uint32_t>*   a_rowsDone) const
{
  static_assert(isShape<S>, "Render::Renderer: the object to render must be a shape (see isShape)");

  this->check(a_camera);

  const PixelQuery<T, S> query{a_shape, a_camera, m_settings};

  if (this->inspecting()) {
    return this->renderOnHost(RenderDetail::InspectQuery<T, S>{query}, a_image, a_cancel, a_rowsDone);
  }

  return this->renderOnHost(query, a_image, a_cancel, a_rowsDone);
}

template <class T>
template <class Q>
inline Stats
Renderer<T>::renderOnHost(const Q&                 a_query,
                          Image&                   a_image,
                          const std::atomic<bool>& a_cancel,
                          std::atomic<uint32_t>*   a_rowsDone) const
{
  const uint32_t width  = m_settings.m_width;
  const uint32_t height = m_settings.m_height;

  const Q& query = a_query;

  // Rows not reached by a canceled render stay black.
  std::vector<RenderDetail::StoredPixel> results(size_t(width) * size_t(height));
  std::vector<uint32_t>                  primitives(m_settings.m_recordPrimitives ? results.size() : 0U, 0xFFFFFFFFU);

  // Each thread takes the next row until none are left, so slow rows do not hold the others back.
  std::atomic<uint32_t> next{0};
  std::atomic<bool>     stopped{false};

  const auto work = [&query, &results, &primitives, &next, &stopped, &a_cancel, a_rowsDone, width, height]() {
    for (uint32_t row = next++; row < height; row = next++) {
      if (a_cancel.load(std::memory_order_relaxed)) {
        stopped = true;

        return;
      }

      for (uint32_t column = 0; column < width; column++) {
        const uint32_t    index  = row * width + column;
        const PixelResult result = query(index);

        results[index] = RenderDetail::StoredPixel::of(result);

        if (!primitives.empty()) {
          primitives[index] = result.m_primitive;
        }
      }

      if (a_rowsDone != nullptr) {
        (*a_rowsDone)++;
      }
    }
  };

  const uint32_t hardware = std::thread::hardware_concurrency();
  const uint32_t threads =
    Math::min(height, (m_settings.m_threads > 0U) ? m_settings.m_threads : Math::max(1U, hardware));

  std::vector<std::thread> pool;

  pool.reserve(threads - 1U);

  // The tracing only, for Stats::m_raysPerSecond: from the first thread started to the last joined.
  const auto start = std::chrono::steady_clock::now();

  for (uint32_t i = 1; i < threads; i++) {
    pool.emplace_back(work);
  }

  work();

  for (auto& thread : pool) {
    thread.join();
  }

  const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();

  Stats stats = this->collect(results, primitives, a_image, CostUnit::Nanoseconds, seconds);

  stats.m_complete = !stopped;

  return stats;
}

template <class T>
template <class S>
inline Stats
Renderer<T>::renderOnDevice(const S& a_deviceShape, const Camera<T>& a_camera, Image& a_image) const
{
  return this->renderFrameOnDevice(a_deviceShape, a_camera, a_image, nullptr, nullptr);
}

template <class T>
template <class S>
inline Stats
Renderer<T>::renderOnDevice(const S&                 a_deviceShape,
                            const Camera<T>&         a_camera,
                            Image&                   a_image,
                            const std::atomic<bool>& a_cancel,
                            std::atomic<uint32_t>*   a_rowsDone) const
{
  return this->renderFrameOnDevice(a_deviceShape, a_camera, a_image, &a_cancel, a_rowsDone);
}

template <class T>
template <class S>
inline Stats
Renderer<T>::renderFrameOnDevice(const S&                 a_deviceShape,
                                 const Camera<T>&         a_camera,
                                 Image&                   a_image,
                                 const std::atomic<bool>* a_cancel,
                                 std::atomic<uint32_t>*   a_rowsDone) const
{
  static_assert(isShape<S>, "Render::Renderer: the object to render must be a shape (see isShape)");
  static_assert(std::is_trivially_copyable_v<PixelQuery<T, S>>,
                "Render::Renderer: the shape must be trivially copyable to render it on a device");

  this->check(a_camera);

  const PixelQuery<T, S> query{a_deviceShape, a_camera, m_settings};

  if (this->inspecting()) {
    return this->renderFrameWith(RenderDetail::InspectQuery<T, S>{query}, a_image, a_cancel, a_rowsDone);
  }

  return this->renderFrameWith(query, a_image, a_cancel, a_rowsDone);
}

template <class T>
template <class Q>
inline Stats
Renderer<T>::renderFrameWith(const Q&                 a_query,
                             Image&                   a_image,
                             const std::atomic<bool>* a_cancel,
                             std::atomic<uint32_t>*   a_rowsDone) const
{
  const uint32_t width  = m_settings.m_width;
  const uint32_t height = m_settings.m_height;
  const size_t   pixels = size_t(width) * size_t(height);

  const Q& query = a_query;

  // Pixels not reached by a canceled render stay black.
  std::vector<RenderDetail::StoredPixel> results(pixels);
  std::vector<uint32_t>                  primitives(m_settings.m_recordPrimitives ? pixels : 0U, 0xFFFFFFFFU);

  bool stopped = false;

  // The tracing only, for Stats::m_raysPerSecond: each launch until it completes (GPU::parallelFor() waits for the
  // device), not allocating the buffers or copying the image back.
  double seconds = 0.0;

  const auto traced = [&seconds](const size_t a_n, const auto& a_frame) {
    const auto start = std::chrono::steady_clock::now();

    GPU::parallelFor(a_n, a_frame);

    seconds += std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
  };

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  // One launch over the whole frame, into one buffer in device memory that is copied back once at the end.
  // A launch per band of rows, checking the cancel flag between them, waits for each band's slowest block
  // and leaves the device partly idle at its end: 8 bands took 1.6-2 times as long as one launch. Row-major
  // pixel order measured fastest (tiles, shuffled segments and shuffled pixels were all slower).
  RenderDetail::StoredPixel* buffer = nullptr;
  uint32_t*                  ids    = nullptr;

  RenderDetail::FrameOfPixels<Q> frame{query, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0U, width};

  const bool launch = (a_cancel == nullptr) || !a_cancel->load(std::memory_order_relaxed);

  if (launch) {
    EBGEOMETRY_GPU_CHECK(GPU::memAlloc(reinterpret_cast<void**>(&buffer), sizeof(RenderDetail::StoredPixel) * pixels));

    if (!primitives.empty()) {
      EBGEOMETRY_GPU_CHECK(GPU::memAlloc(reinterpret_cast<void**>(&ids), sizeof(uint32_t) * pixels));
    }

    frame.m_results    = buffer;
    frame.m_primitives = ids;
  }

  if (a_cancel == nullptr) {
    traced(pixels, frame);
  }
  else if (!launch) {
    stopped = true;
  }
  else {
    // While this thread waits for the launch, another passes the cancel flag to the device and the progress
    // back. The kernel reads the flag from device memory, where that thread copies it on a stream of its own,
    // which runs alongside the kernel; read from host memory mapped into the device's, the flag cost every pixel
    // a read across the bus, about 10 ms of a 7 ms frame (RTX 3080, 960 x 960 pixels). The kernel writes the
    // progress, once a row, to that mapped memory, passed to it as it is, which unified virtual addressing
    // (every 64-bit CUDA and HIP platform) allows. Whether a pixel skipped its work, which says whether the
    // render completed, and the count of rows are in device memory too.
    uint32_t*   mapped = nullptr; // The progress, then a 1 to copy to the device's flag.
    uint32_t*   counts = nullptr; // Whether a pixel skipped its work, the rows, the cancel flag.
    GPU::Stream stream = nullptr;

    const uint32_t zeros[3] = {0U, 0U, 0U};

    EBGEOMETRY_GPU_CHECK(GPU::memAllocHostMapped(reinterpret_cast<void**>(&mapped), 2U * sizeof(uint32_t)));
    EBGEOMETRY_GPU_CHECK(GPU::memAlloc(reinterpret_cast<void**>(&counts), 3U * sizeof(uint32_t)));
    EBGEOMETRY_GPU_CHECK(GPU::memcpy(counts, zeros, 3U * sizeof(uint32_t), GPU::MemcpyHostToDevice));
    EBGEOMETRY_GPU_CHECK(GPU::streamCreateNonBlocking(&stream));

    volatile uint32_t* const progress = mapped;

    *progress = 0U;
    mapped[1] = 1U;

    frame.m_cancel   = counts + 2;
    frame.m_skipped  = counts;
    frame.m_rows     = counts + 1;
    frame.m_progress = progress;

    std::mutex              mutex;
    std::condition_variable wake;
    bool                    finished = false;
    uint32_t                shown    = 0; // Rows added to a_rowsDone.

    const auto report = [a_rowsDone, &shown](const uint32_t a_rows) {
      if (a_rows > shown) {
        if (a_rowsDone != nullptr) {
          (*a_rowsDone) += a_rows - shown;
        }

        shown = a_rows;
      }
    };

    // EBGEOMETRY_GPU_CHECK aborts, never throws, so a failed copy here ends the program rather than this thread.
    std::thread relay([&]() {
      std::unique_lock<std::mutex> lock(mutex);

      bool sent = false;

      while (!finished) {
        if (!sent && a_cancel->load(std::memory_order_relaxed)) {
          EBGEOMETRY_GPU_CHECK(
            GPU::memcpyAsync(counts + 2, mapped + 1, sizeof(uint32_t), GPU::MemcpyHostToDevice, stream));

          sent = true;
        }

        report(*progress);

        wake.wait_for(lock, std::chrono::milliseconds(1));
      }
    });

    traced(pixels, frame);

    {
      const std::lock_guard<std::mutex> lock(mutex);

      finished = true;
    }

    wake.notify_one();
    relay.join();

    uint32_t done[2] = {0U, 0U};

    EBGEOMETRY_GPU_CHECK(GPU::streamSynchronize(stream));
    EBGEOMETRY_GPU_CHECK(GPU::streamDestroy(stream));
    EBGEOMETRY_GPU_CHECK(GPU::memcpy(done, counts, 2U * sizeof(uint32_t), GPU::MemcpyDeviceToHost));
    EBGEOMETRY_GPU_CHECK(GPU::memFree(counts));
    EBGEOMETRY_GPU_CHECK(GPU::memFreeHost(mapped));

    report(done[1]);

    stopped = done[0] != 0U;
  }

  if (launch) {
    EBGEOMETRY_GPU_CHECK(
      GPU::memcpy(results.data(), buffer, sizeof(RenderDetail::StoredPixel) * pixels, GPU::MemcpyDeviceToHost));
    EBGEOMETRY_GPU_CHECK(GPU::memFree(buffer));

    if (ids != nullptr) {
      EBGEOMETRY_GPU_CHECK(GPU::memcpy(primitives.data(), ids, sizeof(uint32_t) * pixels, GPU::MemcpyDeviceToHost));
      EBGEOMETRY_GPU_CHECK(GPU::memFree(ids));
    }
  }
#else
  // A host loop: one launch over the frame, or, when the render can be canceled, one per row, with the flag
  // checked between them.
  RenderDetail::FrameOfPixels<Q> frame{query,
                                       results.data(),
                                       primitives.empty() ? nullptr : primitives.data(),
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       0U,
                                       width};

  if (a_cancel == nullptr) {
    traced(pixels, frame);
  }
  else {
    for (uint32_t row = 0; row < height; row++) {
      if (a_cancel->load(std::memory_order_relaxed)) {
        stopped = true;

        break;
      }

      frame.m_first = row * width;

      traced(width, frame);

      if (a_rowsDone != nullptr) {
        (*a_rowsDone)++;
      }
    }
  }
#endif

  // A device counts clock cycles, reported as they are; a build without one times its host loop in nanoseconds.
#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  constexpr CostUnit unit = CostUnit::Cycles;
#else
  constexpr CostUnit unit = CostUnit::Nanoseconds;
#endif

  Stats stats = this->collect(results, primitives, a_image, unit, seconds);

  stats.m_complete = !stopped;

  return stats;
}

} // namespace EBGeometry::Render

#endif
