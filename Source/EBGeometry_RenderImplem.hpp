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
#include <cstdio>
#include <cstdlib>
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
    m_onPlane =
      a_settings.m_section && bounded &&
      (a_settings.m_contourWidth > T(0) || a_settings.m_sheetOpacity > T(0) || a_settings.m_isoSpacing > T(0));
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
    return this->trace(RayT<T>{a_origin, a_direction}, a_near, m_far);
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

    if (hit.m_status == HitStatus::Unresolved) {
      a_result.m_unresolved++;
    }

    // Where the ray meets the floor, if it goes down towards it.
    const T down   = dot(a_direction, m_up);
    const T tFloor = (m_hasFloor && down < T(0)) ? (m_floorLevel - dot(a_origin, m_up)) / down : T(-1);

    const bool onFloor = tFloor > a_near && (!hit.hit() || tFloor < hit.m_t);

    Vec3T<T> colour;

    if (hit.hit() && !onFloor) {
      a_result.m_hits++;

      if (a_result.m_primitive == 0xFFFFFFFFU) {
        a_result.m_primitive = hit.m_primitive;
      }

      if (!hit.m_entering) {
        a_result.m_insideHits++;
      }

      // A face cut by the section plane is where the plane, not the shape, gives the value: a fill, through
      // which the shape's next surface along the ray shows.
      const Vec3T<T>& p = hit.m_point;

      if (m_object.m_on && m_object.plane(p) >= m_object.shapeDistance(p)) {
        colour = this->shadeCutFace(ray, hit.m_t);
      }
      else {
        colour = this->shadeSurface(hit, a_direction, true);
      }
    }
    else if (onFloor) {
      colour = this->shadeFloor(ray.at(tFloor), a_direction, tFloor);
    }
    else {
      colour = this->background(a_direction);
    }

    if (m_onPlane) {
      // What the ray sees lies at the floor, at the hit, or nowhere nearer than the far end.
      const T tFront = onFloor ? tFloor : (hit.hit() ? hit.m_t : m_far);

      colour = this->drawOnPlane(ray, a_near, tFront, colour);
    }

    return colour;
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
    const HitT<T>  hit    = this->trace(RayT<T>{origin, m_light}, T(0), T(2) * m_scale);

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

      blocked += this->trace(RayT<T>{origin, r}, T(0), T(0.07) * m_scale).hit() ? 1 : 0;
    }

    return T(1) - T(0.2) * T(blocked);
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
   * @brief The colour of the section plane's cut face where a camera ray meets it: the fill, and through it
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
   * @brief The fill of the section plane's cut face: coloured as a surface facing along the plane's normal
   * would be, and lit by that normal a little from either side, without shadows, so that a slice reads the
   * same whichever way the light faces. Coloured by primitive, it takes the cut colour: inside a
   * composition the leaf that decides the value (a subtracted one, near its surface) need not be the
   * material there.
   * @return Linear RGB.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Vec3T<T>
  shadeCut() const noexcept
  {
    const Vec3T<T>& n = m_object.m_normal;

    Vec3T<T> albedo = m_settings.m_colour;

    if (m_settings.m_colourBy == ColourBy::Normal) {
      albedo = T(0.5) * (n + Vec3T<T>::ones());
    }
    else if (m_settings.m_colourBy == ColourBy::Primitive) {
      albedo = m_settings.m_cutColour;
    }

    return albedo * (T(0.6) + T(0.4) * std::abs(dot(n, m_light)));
  }

  /**
   * @brief The colour of the shape's surface where a ray hit it.
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

    Vec3T<T> albedo = m_settings.m_colour;

    if (m_settings.m_colourBy == ColourBy::Primitive) {
      const uint32_t id = a_hit.m_primitive;

      albedo = (id < Settings<T>::PrimitiveColours && m_settings.m_primitiveColours[id][0] >= T(0))
                 ? m_settings.m_primitiveColours[id]
                 : palette<T>(id);
    }
    else if (m_settings.m_colourBy == ColourBy::Normal) {
      albedo = T(0.5) * (n + Vec3T<T>::ones());
    }

    // Light the side that faces the eye; for a surface seen from inside that is the inner side.
    if (dot(n, a_direction) > T(0)) {
      n = -n;
    }

    if (a_hit.m_primitive == m_settings.m_highlightPrimitive ||
        m_settings.m_highlightPrimitive == Settings<T>::AllPrimitives) {
      albedo = T(0.35) * albedo + T(0.65) * m_settings.m_highlightColour;
    }

    if (!a_hit.m_entering && a_tint) {
      albedo = T(0.25) * albedo + T(0.75) * m_settings.m_insideColour;
    }

    return this->light(p, n, a_direction, albedo, a_tint);
  }

  /**
   * @brief What the section plane adds where a camera ray meets it in front of what the ray sees: the
   * shape's contour, a line of constant width on screen where the shape's value is near zero, and outside
   * the shape a faint sheet. Inside the shape the plane is the cut face, already shaded.
   * @param[in] a_ray    The camera ray.
   * @param[in] a_near   Where the ray starts to see.
   * @param[in] a_tFront Where it meets what it sees.
   * @param[in] a_colour What it sees.
   * @return Linear RGB.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline Vec3T<T>
  drawOnPlane(const RayT<T>& a_ray, const T a_near, const T a_tFront, const Vec3T<T>& a_colour) const noexcept
  {
    const T rate = dot(a_ray.m_direction, m_object.m_normal);

    if (rate == T(0)) {
      return a_colour;
    }

    const T t = -m_object.plane(a_ray.m_origin) / rate;

    if (!(t > a_near && t <= a_tFront + T(4) * m_eps)) {
      return a_colour;
    }

    const Vec3T<T> p = a_ray.at(t);

    for (int k = 0; k < 3; k++) {
      if (p[k] < m_boxLow[k] || p[k] > m_boxHigh[k]) {
        return a_colour;
      }
    }

    const T value = m_object.shapeDistance(p);
    const T width = m_footprint + m_footprintPerLength * t;
    const T half  = T(0.5) * m_settings.m_contourWidth * width;

    if (std::abs(value) < half) {
      return m_settings.m_contourColour;
    }

    // Lines of equal distance, every m_isoSpacing, thinner than the contour.
    Vec3T<T> colour = a_colour;

    if (m_settings.m_isoSpacing > T(0)) {
      const T s = m_settings.m_isoSpacing;
      const T r = value - s * std::floor(value / s + T(0.5));

      if (std::abs(r) < T(0.5) * width) {
        colour = T(0.4) * colour + T(0.6) * m_settings.m_isoColour;
      }
    }

    if (value > T(0) && m_settings.m_sheetOpacity > T(0)) {
      // The sheet's edge, a pixel and a half wide, where the plane leaves the box.
      T edge = Math::Limits<T>::infinity();

      for (int k = 0; k < 3; k++) {
        edge = Math::min(edge, Math::min(p[k] - m_boxLow[k], m_boxHigh[k] - p[k]));
      }

      const Vec3T<T>& over = (edge < T(1.5) * width) ? m_settings.m_sheetEdgeColour : m_settings.m_sheetColour;

      return (T(1) - m_settings.m_sheetOpacity) * colour + m_settings.m_sheetOpacity * over;
    }

    return colour;
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
    const T cell  = (m_settings.m_floorCheckerSize > T(0)) ? m_settings.m_floorCheckerSize : m_scale / T(6);
    const T a     = std::floor(dot(a_point, m_first) / cell);
    const T b     = std::floor(dot(a_point, m_second) / cell);
    const T check = (!m_settings.m_floorChecker || std::fmod(std::abs(a + b), T(2)) < T(0.5)) ? T(1) : T(0.9);

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
      return m_settings.m_background;
    }

    const T s = Math::clamp(dot(a_direction, m_up), T(0), T(1));

    return (T(1) - s) * m_settings.m_skyHorizon + s * m_settings.m_skyZenith;
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
  StoredPixel* m_results;

  /**
   * @brief Each pixel's object, in device memory, or null when not recorded.
   */
  uint32_t* m_primitives;

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
    const auto        pixel  = static_cast<uint32_t>(m_first + a_index);
    const PixelResult result = m_query(pixel);

    m_results[pixel] = StoredPixel::of(result);

    if (m_primitives != nullptr) {
      m_primitives[pixel] = result.m_primitive;
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
inline Array<Vec3T<T>, Settings<T>::PrimitiveColours>
Settings<T>::unsetColours() noexcept
{
  Array<Vec3T<T>, PrimitiveColours> colours;

  for (uint32_t i = 0; i < PrimitiveColours; i++) {
    colours[i] = Vec3T<T>(T(-1), T(-1), T(-1));
  }

  return colours;
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
 * @brief A colouring as a C++ expression.
 * @details Internal helper; not part of the public API.
 * @param[in] a_colourBy Colouring.
 * @return For example "EBGeometry::Render::ColourBy::Primitive".
 */
EBGEOMETRY_HOST
inline std::string
literal(const ColourBy a_colourBy)
{
  switch (a_colourBy) {
  case ColourBy::Primitive: {
    return "EBGeometry::Render::ColourBy::Primitive";
  }
  case ColourBy::Normal: {
    return "EBGeometry::Render::ColourBy::Normal";
  }
  default: {
    return "EBGeometry::Render::ColourBy::Uniform";
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
  field("settings", "m_colourBy", a_settings.m_colourBy, settings.m_colourBy);
  field("settings", "m_colour", a_settings.m_colour, settings.m_colour);

  for (uint32_t i = 0; i < Settings<T>::PrimitiveColours; i++) {
    const std::string name = "m_primitiveColours[" + std::to_string(i) + "]";

    field("settings", name.c_str(), a_settings.m_primitiveColours[i], settings.m_primitiveColours[i]);
  }

  field("settings", "m_recordPrimitives", a_settings.m_recordPrimitives, settings.m_recordPrimitives);
  field("settings", "m_highlightPrimitive", a_settings.m_highlightPrimitive, settings.m_highlightPrimitive);
  field("settings", "m_highlightColour", a_settings.m_highlightColour, settings.m_highlightColour);
  field("settings", "m_insideColour", a_settings.m_insideColour, settings.m_insideColour);
  field("settings", "m_cutColour", a_settings.m_cutColour, settings.m_cutColour);
  field("settings", "m_cutOpacity", a_settings.m_cutOpacity, settings.m_cutOpacity);
  field("settings", "m_floorColour", a_settings.m_floorColour, settings.m_floorColour);
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
  field("settings", "m_contourColour", a_settings.m_contourColour, settings.m_contourColour);
  field("settings", "m_isoSpacing", a_settings.m_isoSpacing, settings.m_isoSpacing);
  field("settings", "m_isoColour", a_settings.m_isoColour, settings.m_isoColour);
  field("settings", "m_sheetOpacity", a_settings.m_sheetOpacity, settings.m_sheetOpacity);
  field("settings", "m_sheetColour", a_settings.m_sheetColour, settings.m_sheetColour);
  field("settings", "m_sheetEdgeColour", a_settings.m_sheetEdgeColour, settings.m_sheetEdgeColour);
  field("settings", "m_rays.m_tolerance", a_settings.m_rays.m_tolerance, settings.m_rays.m_tolerance);
  field("settings", "m_rays.m_scale", a_settings.m_rays.m_scale, settings.m_rays.m_scale);
  field("settings", "m_rays.m_maxSteps", a_settings.m_rays.m_maxSteps, settings.m_rays.m_maxSteps);
  field("settings", "m_rays.m_step", a_settings.m_rays.m_step, settings.m_rays.m_step);
  field("settings", "m_rays.m_march", a_settings.m_rays.m_march, settings.m_rays.m_march);

  return code;
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

    // Perspective rays leave the eye; orthographic ones are parallel, behind the shape, spread as far as the
    // perspective rays are at the target. One call to shade(): two (one per kind) copied its whole body into
    // the kernel twice, and doubled what it spills.
    const Vec3T<T> origin    = m_camera.m_orthographic
                                 ? shader.orthographicStart(m_view.m_eye + m_view.m_distance * (x * right + y * up))
                                 : m_view.m_eye;
    const Vec3T<T> direction = m_camera.m_orthographic ? forward : RenderDetail::unit(forward + x * right + y * up);
    const T        near      = m_camera.m_orthographic ? shader.nearAlong(m_camera.m_nearClip) : m_camera.m_nearClip;

    colour += shader.shade(origin, direction, near, result);
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
  EBGEOMETRY_REQUIRE(m_settings.m_cutOpacity >= T(0) && m_settings.m_cutOpacity <= T(1),
                     "Render::Renderer: the cut face's opacity must be from 0 to 1 (%g)",
                     double(m_settings.m_cutOpacity));
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
}

template <class T>
inline Stats
Renderer<T>::collect(const std::vector<RenderDetail::StoredPixel>& a_results,
                     std::vector<uint32_t>&                        a_primitives,
                     Image&                                        a_image) const
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

  for (uint32_t i = 1; i < threads; i++) {
    pool.emplace_back(work);
  }

  work();

  for (auto& thread : pool) {
    thread.join();
  }

  Stats stats = this->collect(results, primitives, a_image);

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

  std::vector<RenderDetail::StoredPixel> results(pixels);
  std::vector<uint32_t>                  primitives(m_settings.m_recordPrimitives ? pixels : 0U, 0xFFFFFFFFU);

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  RenderDetail::StoredPixel* buffer = nullptr;
  uint32_t*                  ids    = nullptr;

  EBGEOMETRY_GPU_CHECK(GPU::memAlloc(reinterpret_cast<void**>(&buffer), sizeof(RenderDetail::StoredPixel) * pixels));

  if (!primitives.empty()) {
    EBGEOMETRY_GPU_CHECK(GPU::memAlloc(reinterpret_cast<void**>(&ids), sizeof(uint32_t) * pixels));
  }
#else
  RenderDetail::StoredPixel* buffer = results.data();
  uint32_t*                  ids    = primitives.empty() ? nullptr : primitives.data();
#endif

  bool                      stopped = false;
  [[maybe_unused]] uint32_t done    = 0; // Rows rendered; read only by a device copy.

  for (uint32_t row = 0; row < height; row += band) {
    if (a_cancel.load(std::memory_order_relaxed)) {
      stopped = true;

      break;
    }

    const uint32_t rows = Math::min(band, height - row);

    GPU::parallelFor(size_t(rows) * size_t(width), RenderDetail::BandOfPixels<T, S>{query, buffer, ids, row * width});

    done += rows;

    if (a_rowsDone != nullptr) {
      (*a_rowsDone) += rows;
    }
  }

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  EBGEOMETRY_GPU_CHECK(GPU::memcpy(
    results.data(), buffer, sizeof(RenderDetail::StoredPixel) * size_t(done) * size_t(width), GPU::MemcpyDeviceToHost));
  EBGEOMETRY_GPU_CHECK(GPU::memFree(buffer));

  if (ids != nullptr) {
    EBGEOMETRY_GPU_CHECK(
      GPU::memcpy(primitives.data(), ids, sizeof(uint32_t) * size_t(done) * size_t(width), GPU::MemcpyDeviceToHost));
    EBGEOMETRY_GPU_CHECK(GPU::memFree(ids));
  }
#endif

  Stats stats = this->collect(results, primitives, a_image);

  stats.m_complete = !stopped;

  return stats;
}

} // namespace EBGeometry::Render

#endif
