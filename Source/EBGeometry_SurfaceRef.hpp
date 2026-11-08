// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_SurfaceRef.hpp
 * @brief  SurfaceRef: which surface of a shape decides its value at a point, and the container protocol that
 * names one inside a shape holding several.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_SURFACEREF_HPP
#define EBGEOMETRY_SURFACEREF_HPP

// Std includes
#include <cstdint>
#include <type_traits>
#include <utility>

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief The surface that decides a shape's value at a point, as closestSurface() reports it.
 * @details A shape holding several surfaces (a container: a tape, a BVH union, a sphere cloud, or a user's
 * shape with getNumSurfaces() and closestSurface()) numbers them from 0 to getNumSurfaces() - 1, and names the
 * one that decides, at any depth: a sphere of a cloud inside a union inside a tape. Every other shape is one
 * surface, number 0. A tape numbers its surfaces across the whole scene: each leaf covers the range
 * [Tape::getFirstSurface(leaf), + the leaf's getNumSurfaces()), in leaf order, so per-member data of a
 * container is an array indexed by m_surface minus its leaf's first surface.
 * @tparam T Floating-point precision.
 */
template <class T>
struct SurfaceRef
{
  /**
   * @brief No surface (an empty shape), no leaf or no label.
   */
  static constexpr uint32_t None = 0xFFFFFFFFU;

  /**
   * @brief The surface, numbered within the shape that was asked (across the scene when that is a tape).
   */
  uint32_t m_surface = None;

  /**
   * @brief Set by a tape: the leaf the surface belongs to, as Tape::closestLeaf() numbers it.
   */
  uint32_t m_leaf = None;

  /**
   * @brief Set by a tape: the leaf's label (Tape::getLabel()), or None.
   */
  uint32_t m_label = None;

  /**
   * @brief The point in the frame of the deepest shape reached: the primitive, or the container member, whose
   * surface it is.
   */
  Vec3T<T> m_localPoint = Vec3T<T>::zeros();
};

namespace SurfaceDetail {

/**
 * @brief Container detection: the primary template is a shape with one surface.
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T, class = void>
struct HasSurfaces : std::false_type
{
};

/**
 * @brief Container detection: a shape with getNumSurfaces() and closestSurface(const Vec3T<T>&).
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T>
struct HasSurfaces<S,
                   T,
                   std::void_t<decltype(std::declval<const S&>().getNumSurfaces()),
                               decltype(std::declval<const S&>().closestSurface(std::declval<const Vec3T<T>&>()))>>
  : std::is_same<std::decay_t<decltype(std::declval<const S&>().closestSurface(std::declval<const Vec3T<T>&>()))>,
                 SurfaceRef<T>>
{
};

} // namespace SurfaceDetail

/**
 * @brief Whether a shape holds several surfaces: it has `uint32_t getNumSurfaces() const` (host) and
 * `SurfaceRef<T> closestSurface(const Vec3T<T>&) const` (host and device), which name its members.
 * @details Tape, BVHUnion, BVHSmoothUnion and SphereCloudBVH are containers. A user's shape becomes one by
 * defining both; closestSurface() must then number its surfaces from 0 to getNumSurfaces() - 1, the same on
 * every call, and set SurfaceRef::m_surface and m_localPoint. Every other shape is one surface.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T>
inline constexpr bool hasSurfaces = SurfaceDetail::HasSurfaces<S, T>::value;

namespace SurfaceDetail {

/**
 * @brief How many surfaces a shape holds: a container's getNumSurfaces(), 1 for any other shape.
 * @details Internal helper; not part of the public API. Host only.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in] a_shape The shape.
 * @return The count.
 */
template <class T, class S>
[[nodiscard]] EBGEOMETRY_HOST
inline uint32_t
numSurfacesOf(const S& a_shape)
{
  if constexpr (hasSurfaces<S, T>) {
    return a_shape.getNumSurfaces();
  }
  else {
    return 1U;
  }
}

/**
 * @brief The surface of a shape that decides its value at a point: a container's closestSurface(), surface 0
 * at the point itself for any other shape.
 * @details Internal helper; not part of the public API. Out of line on a device, like the ray helpers: a
 * container's closestSurface() is a traversal, and inlined at each call site it would be copied into every
 * drill of every type.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in] a_shape The shape.
 * @param[in] a_point The point, in the shape's frame.
 * @return The surface.
 */
template <class T, class S>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline SurfaceRef<T>
closestSurfaceOf(const S& a_shape, const Vec3T<T>& a_point) noexcept
{
  if constexpr (hasSurfaces<S, T>) {
    return a_shape.closestSurface(a_point);
  }
  else {
    SurfaceRef<T> surface;

    surface.m_surface    = 0U;
    surface.m_localPoint = a_point;

    return surface;
  }
}

} // namespace SurfaceDetail

} // namespace EBGeometry

#endif
