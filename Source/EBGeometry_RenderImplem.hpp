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
#include <cmath>
#include <cstddef>
#include <numeric>
#include <thread>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_Constants.hpp"
#include "EBGeometry_GPUTransform.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
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
 * @brief A colour for a primitive id: hues spread by the golden ratio, so neighbouring ids differ.
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
 * @brief Everything one pixel's rays share: the cut shape, the scene's scale, the floor, the light.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 */
template <class T, class S>
class Shader
{
public:
  /**
   * @brief Set up the scene for a shape, a camera's view and settings.
   * @param[in] a_shape    Shape.
   * @param[in] a_view     The camera's view.
   * @param[in] a_settings Settings.
   */
  EBGEOMETRY_HOST_DEVICE
  Shader(const S& a_shape, const CameraView<T>& a_view, const Settings<T>& a_settings) noexcept
    : m_object{a_shape, a_settings.m_section, a_settings.m_sectionPoint, unit(a_settings.m_sectionNormal)},
      m_settings(a_settings),
      m_eye(a_view.m_eye),
      m_up(a_view.m_axis),
      m_baseLight(unit(a_settings.m_light)),
      m_light(m_baseLight),
      m_first(a_view.m_first),
      m_second(a_view.m_second)
  {

    const auto     box = a_shape.computeBoundingVolume();
    const Vec3T<T> lo  = box.getLowCorner();
    const Vec3T<T> hi  = box.getHighCorner();

    const T    diagonal = (hi - lo).length();
    const bool bounded  = std::isfinite(diagonal) && diagonal > T(0);

    m_scale = (a_settings.m_rays.m_scale > T(0)) ? a_settings.m_rays.m_scale : (bounded ? diagonal : a_view.m_distance);
    m_eps   = a_settings.m_rays.m_tolerance * m_scale;
    m_far   = bounded ? ((m_eye - box.getCentroid()).length() + diagonal) : T(50) * m_scale;

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
   * @brief The colour seen along one camera ray, counting what it hit.
   * @param[in]     a_direction Unit direction from the eye.
   * @param[in,out] a_result    Counts.
   * @return Linear RGB.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Vec3T<T>
  shade(const Vec3T<T>& a_direction, PixelResult& a_result) const noexcept
  {
    const RayT<T> ray{m_eye, a_direction};
    const HitT<T> hit = this->trace(ray, m_far);

    if (hit.m_status == HitStatus::Unresolved) {
      a_result.m_unresolved++;
    }

    // Where the ray meets the floor, if it goes down towards it.
    const T down   = dot(a_direction, m_up);
    const T tFloor = (m_hasFloor && down < T(0)) ? (m_floorLevel - dot(m_eye, m_up)) / down : T(-1);

    const bool onFloor = tFloor > T(0) && (!hit.hit() || tFloor < hit.m_t);

    if (hit.hit() && !onFloor) {
      a_result.m_hits++;

      if (!hit.m_entering) {
        a_result.m_insideHits++;
      }

      return this->shadeSurface(hit, a_direction);
    }

    if (onFloor) {
      return this->shadeFloor(ray.at(tFloor), a_direction, tFloor);
    }

    return this->background(a_direction);
  }

private:
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
   * @brief The cut shape's first crossing along a ray from its origin (firstHit()).
   * @details Every ray the shader casts goes through here, out of line on a device: inlined at each of
   * its callers, the shape's whole trace was copied once per ray kind and the kernels grew too large to
   * compile.
   * @param[in] a_ray  Ray, with a unit direction.
   * @param[in] a_tMax How far along it to look.
   * @return The crossing, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline HitT<T>
  trace(const RayT<T>& a_ray, const T a_tMax) const noexcept
  {
    return firstHit(m_object, a_ray, T(0), a_tMax, m_rays);
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
   * @brief How much light reaches a point from the light's direction: 0 in shadow, 1 in light.
   * @details One ray towards the light (firstHit()); the light moving a little between a pixel's rays
   * softens the edges.
   * @param[in] a_point  Point.
   * @param[in] a_normal Unit normal there.
   * @return The fraction of light.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  shadow(const Vec3T<T>& a_point, const Vec3T<T>& a_normal) const noexcept
  {
    const Vec3T<T> origin = a_point + T(4) * m_eps * a_normal;
    const HitT<T>  hit    = this->trace(RayT<T>{origin, m_light}, T(2) * m_scale);

    return hit.hit() ? T(0) : T(1);
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
    // The share of four short rays, tilted around the normal, that hit the shape.
    const Vec3T<T> t1 =
      unit(cross((std::abs(a_normal[0]) < T(0.9)) ? Vec3T<T>(T(1), T(0), T(0)) : Vec3T<T>(T(0), T(1), T(0)), a_normal));
    const Vec3T<T> t2     = cross(a_normal, t1);
    const Vec3T<T> origin = a_point + T(4) * m_eps * a_normal;

    int blocked = 0;

    for (int k = 0; k < 4; k++) {
      const T        a = T(0.5) * pi<T> * T(k);
      const Vec3T<T> r = unit(a_normal + T(0.8) * (std::cos(a) * t1 + std::sin(a) * t2));

      blocked += this->trace(RayT<T>{origin, r}, T(0.07) * m_scale).hit() ? 1 : 0;
    }

    return T(1) - T(0.2) * T(blocked);
  }

  /**
   * @brief The light a surface point with an albedo and a normal reflects towards the eye.
   * @param[in] a_point     Point.
   * @param[in] a_normal    Unit normal, towards the eye.
   * @param[in] a_direction Unit direction of the camera ray.
   * @param[in] a_albedo    Albedo.
   * @return Linear RGB.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Vec3T<T>
  light(const Vec3T<T>& a_point,
        const Vec3T<T>& a_normal,
        const Vec3T<T>& a_direction,
        const Vec3T<T>& a_albedo) const noexcept
  {
    const Vec3T<T> half = unit(m_light - a_direction);

    const T lit  = m_settings.m_shadows ? this->shadow(a_point, a_normal) : T(1);
    const T diff = Math::max(T(0), dot(a_normal, m_light)) * lit;
    const T spec = std::pow(Math::max(T(0), dot(a_normal, half)), T(48)) * diff;
    const T sky  = T(0.5) + T(0.5) * dot(a_normal, m_up);
    const T ao   = m_settings.m_occlusion ? this->occlusion(a_point, a_normal) : T(1);

    return a_albedo * (T(1.05) * diff + T(0.35) * sky * ao) + T(0.3) * spec * Vec3T<T>::ones();
  }

  /**
   * @brief The colour of the shape's surface where a camera ray hit it.
   * @param[in] a_hit       The hit.
   * @param[in] a_direction Unit direction of the camera ray.
   * @return Linear RGB.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Vec3T<T>
  shadeSurface(const HitT<T>& a_hit, const Vec3T<T>& a_direction) const noexcept
  {
    const Vec3T<T>& p = a_hit.m_point;

    // A face cut by the section plane is where the plane, not the shape, gives the value.
    const bool cut = m_object.m_on && m_object.plane(p) >= m_object.shapeDistance(p);

    Vec3T<T> n = cut ? m_object.m_normal : ((a_hit.m_normal.length2() > T(0)) ? a_hit.m_normal : this->normal(p));

    // Light the side that faces the eye; for a surface seen from inside that is the inner side.
    if (dot(n, a_direction) > T(0)) {
      n = -n;
    }

    Vec3T<T> albedo = m_settings.m_colour;

    if (cut) {
      albedo = m_settings.m_cutColour;
    }
    else if (m_settings.m_colourBy == ColourBy::Primitive) {
      albedo = palette<T>(a_hit.m_primitive);
    }

    if (!a_hit.m_entering && !cut) {
      albedo = T(0.25) * albedo + T(0.75) * m_settings.m_insideColour;
    }

    return this->light(p, n, a_direction, albedo);
  }

  /**
   * @brief The colour of the floor where a camera ray hit it: a faint checkerboard, shadowed by the
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
    const T cell  = m_scale / T(6);
    const T a     = std::floor(dot(a_point, m_first) / cell);
    const T b     = std::floor(dot(a_point, m_second) / cell);
    const T check = (std::fmod(std::abs(a + b), T(2)) < T(0.5)) ? T(1) : T(0.9);

    const Vec3T<T> lit = this->light(a_point, m_up, a_direction, check * m_settings.m_floorColour);

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
      return Vec3T<T>(T(0.16), T(0.17), T(0.19));
    }

    const T s = Math::clamp(dot(a_direction, m_up), T(0), T(1));

    return (T(1) - s) * Vec3T<T>(T(0.82), T(0.84), T(0.88)) + s * Vec3T<T>(T(0.32), T(0.45), T(0.66));
  }
};

/**
 * @brief One band of rows of a device render: pixel a_first + i, for every index i of the launch, into the
 * frame's results.
 * @details Internal helper; not part of the public API. The pixel indices come from the launch, so nothing
 * is copied to the device but the functor.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 */
template <class T, class S>
struct BandOfPixels
{
  /**
   * @brief The pixel functor.
   */
  PixelQuery<T, S> m_query;

  /**
   * @brief The frame's results, in device memory.
   */
  PixelResult* m_results;

  /**
   * @brief The band's first pixel.
   */
  uint32_t m_first;

  /**
   * @brief Shade pixel m_first + a_index.
   * @param[in] a_index Index within the band.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  operator()(const std::size_t a_index) const noexcept
  {
    const auto pixel = static_cast<uint32_t>(m_first + a_index);

    m_results[pixel] = m_query(pixel);
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
      hit.m_primitive = a_cut.primitiveAt(hit.m_point);
      hit.m_entering  = a_entering;
      hit.m_exact     = true;
      hit.m_normal    = a_cut.m_normal;

      return hit;
    };

    if (a > a_tMin && a_cut.shapeDistance(a_ray.at(a)) < T(0)) {
      return onPlane(a, true);
    }

    const HitT<T> hit = a_cut.shapeHit(a_ray, a, b, a_options);

    if (hit.m_status == HitStatus::Miss && b < a_tMax && a_cut.shapeDistance(a_ray.at(b)) < T(0)) {
      return onPlane(b, false);
    }

    return hit;
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
  camera.m_yaw      = std::atan2(dot(w, first), dot(w, second)) * degrees;

  EBGEOMETRY_REQUIRE(std::abs(camera.m_pitch) < T(90),
                     "Render::Camera: the eye must not be straight above or below the target along the up "
                     "axis (pitch %g)",
                     double(camera.m_pitch));

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
  a_forward = RenderDetail::unit(m_target - this->eye());
  a_right   = RenderDetail::unit(cross(a_forward, RenderDetail::unit(m_up)));
  a_up      = cross(a_right, a_forward);
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

  Vec3T<T> colour = Vec3T<T>::zeros();

  for (uint32_t s = 0; s < n * n; s++) {
    const T dx = (T(s % n) + T(0.5)) / T(n);
    const T dy = (T(s / n) + T(0.5)) / T(n);

    // Screen coordinates: x from -w/h to w/h, y from -1 (bottom) to 1 (top), times tan(fov/2).
    const T x = (T(2) * (column + dx) - w) / h * tanHalf;
    const T y = (h - T(2) * (row + dy)) / h * tanHalf;

    if (n > 1U) {
      shader.jitterLight(dx, dy);
    }

    colour += shader.shade(RenderDetail::unit(forward + x * right + y * up), result);
  }

  colour = colour / T(n * n);

  uint32_t rgba = 0xFFU;

  for (int k = 0; k < 3; k++) {
    const T g = std::pow(Math::clamp(colour[k], T(0), T(1)), T(1) / T(2.2));

    rgba |= static_cast<uint32_t>(T(255) * g + T(0.5)) << (24 - 8 * k);
  }

  result.m_rgba = rgba;

  return result;
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
  EBGEOMETRY_REQUIRE(std::abs(a_camera.m_pitch) < T(90),
                     "Render::Renderer: the camera pitch must be between -90 and 90 degrees (%g)",
                     double(a_camera.m_pitch));
  EBGEOMETRY_REQUIRE(a_camera.m_up.length() > T(0), "Render::Renderer: the camera's up axis must not be zero");
  EBGEOMETRY_REQUIRE(m_settings.m_light.length() > T(0), "Render::Renderer: the light direction must not be zero");
  EBGEOMETRY_REQUIRE(!m_settings.m_section || m_settings.m_sectionNormal.length() > T(0),
                     "Render::Renderer: the section plane's normal must not be zero");
}

template <class T>
inline Stats
Renderer<T>::collect(const std::vector<PixelResult>& a_results, Image& a_image) const
{
  a_image.m_width  = m_settings.m_width;
  a_image.m_height = m_settings.m_height;
  a_image.m_pixels.resize(a_results.size());

  Stats stats;

  for (size_t i = 0; i < a_results.size(); i++) {
    a_image.m_pixels[i] = a_results[i].m_rgba;

    stats.m_hits += a_results[i].m_hits;
    stats.m_insideHits += a_results[i].m_insideHits;
    stats.m_unresolved += a_results[i].m_unresolved;
  }

  stats.m_rays         = uint64_t(a_results.size()) * m_settings.m_supersampling * m_settings.m_supersampling;
  stats.m_cameraInside = !a_results.empty() && a_results.front().m_cameraInside != 0U;

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

  const uint32_t width  = m_settings.m_width;
  const uint32_t height = m_settings.m_height;

  const PixelQuery<T, S> query{a_shape, a_camera, m_settings};

  // Rows not reached by a cancelled render stay black.
  std::vector<PixelResult> results(size_t(width) * size_t(height));

  // Each thread takes the next row until none are left, so slow rows do not hold the others back.
  std::atomic<uint32_t> next{0};
  std::atomic<bool>     stopped{false};

  const auto work = [&query, &results, &next, &stopped, &a_cancel, a_rowsDone, width, height]() {
    for (uint32_t row = next++; row < height; row = next++) {
      if (a_cancel.load(std::memory_order_relaxed)) {
        stopped = true;

        return;
      }

      for (uint32_t column = 0; column < width; column++) {
        const uint32_t index = row * width + column;

        results[index] = query(index);
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

  for (uint32_t i = 1; i < threads; i++) {
    pool.emplace_back(work);
  }

  work();

  for (auto& thread : pool) {
    thread.join();
  }

  Stats stats = this->collect(results, a_image);

  stats.m_complete = !stopped;

  return stats;
}

template <class T>
template <class S>
inline Stats
Renderer<T>::renderOnDevice(const S& a_deviceShape, const Camera<T>& a_camera, Image& a_image) const
{
  const std::atomic<bool> never{false};

  return this->renderOnDevice(a_deviceShape, a_camera, a_image, never);
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
  static_assert(isShape<S>, "Render::Renderer: the object to render must be a shape (see isShape)");
  static_assert(std::is_trivially_copyable_v<PixelQuery<T, S>>,
                "Render::Renderer: the shape must be trivially copyable to render it on a device");

  this->check(a_camera);

  const uint32_t width  = m_settings.m_width;
  const uint32_t height = m_settings.m_height;

  const PixelQuery<T, S> query{a_deviceShape, a_camera, m_settings};

  // A launch per band of rows, so that a cancelled render stops after the band under way: 8 bands, or
  // fewer for a small image (each launch waits for the device, tens of microseconds, so more bands cost a
  // small frame more than they gain). The bands write into one buffer for the frame, in device memory,
  // that is copied back once at the end.
  const uint32_t band   = Math::max(1U, (height + 7U) / 8U);
  const size_t   pixels = size_t(width) * size_t(height);

  std::vector<PixelResult> results(pixels);

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  PixelResult* buffer = nullptr;

  EBGEOMETRY_GPU_CHECK(GPU::memAlloc(reinterpret_cast<void**>(&buffer), sizeof(PixelResult) * pixels));
#else
  PixelResult* buffer = results.data();
#endif

  bool     stopped = false;
  uint32_t done    = 0;

  for (uint32_t row = 0; row < height; row += band) {
    if (a_cancel.load(std::memory_order_relaxed)) {
      stopped = true;

      break;
    }

    const uint32_t rows = Math::min(band, height - row);

    GPU::parallelFor(size_t(rows) * size_t(width), RenderDetail::BandOfPixels<T, S>{query, buffer, row * width});

    done += rows;

    if (a_rowsDone != nullptr) {
      (*a_rowsDone) += rows;
    }
  }

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  EBGEOMETRY_GPU_CHECK(
    GPU::memcpy(results.data(), buffer, sizeof(PixelResult) * size_t(done) * size_t(width), GPU::MemcpyDeviceToHost));
  EBGEOMETRY_GPU_CHECK(GPU::memFree(buffer));
#endif

  Stats stats = this->collect(results, a_image);

  stats.m_complete = !stopped;

  return stats;
}

} // namespace EBGeometry::Render

#endif
