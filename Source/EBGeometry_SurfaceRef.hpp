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
#include "EBGeometry_LabelId.hpp"
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
   * @brief Set by a tape: the surface's label, or none. A part labeled through face groups (SurfaceLabel(shape,
   * FaceGroups)) has its group's label; any other the leaf's (Tape::getSurfaceLabel()).
   */
  SurfaceLabelId m_surfaceLabel;

  /**
   * @brief The part of the deepest shape reached, read from its own formula at the local point (see hasParts): a
   * box's face, a cylinder's side or cap, a mesh's face; 0 for a shape with one part.
   */
  uint32_t m_part = 0;

  /**
   * @brief Set by a tape's region queries: the owner, the region the surface is reported from at the queried point
   * (for Tape::closestSurface(p, region), the region asked about; for a crossing along a ray, the region the ray
   * leaves; for a piece, the later region, on top in the Overlay), or None. The normal points from the owner to the
   * neighbor. Which side is in a subdomain is a question for the subdomains (Subdomains::role()), not a property of
   * the surface.
   */
  uint32_t m_ownerRegion = None;

  /**
   * @brief Set by a tape's region queries: the neighbor, the region across the surface from the owner at the queried
   * point (for a crossing along a ray, the region the ray enters; 0 for the background), or None.
   */
  uint32_t m_neighborRegion = None;

  /**
   * @brief Set with the sides: the owner's material (m_ownerRegion's MaterialLabel()), or none.
   */
  MaterialLabelId m_ownerMaterial;

  /**
   * @brief Set with the sides: the neighbor's material (m_neighborRegion's), or none.
   */
  MaterialLabelId m_neighborMaterial;

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
 * @brief Part detection: the primary template is a shape with one part.
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T, class = void>
struct HasParts : std::false_type
{
};

/**
 * @brief Part detection: a shape with getNumParts() and closestPart(const Vec3T<T>&).
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T>
struct HasParts<S,
                T,
                std::void_t<decltype(std::declval<const S&>().getNumParts()),
                            decltype(std::declval<const S&>().closestPart(std::declval<const Vec3T<T>&>()))>>
  : std::true_type
{
};

} // namespace SurfaceDetail

/**
 * @brief Whether a shape has several parts: it has `uint32_t getNumParts() const` (host) and
 * `uint32_t closestPart(const Vec3T<T>&) const` (host and device), which reads the part from its own formula at a
 * point in its frame: the term that decides the value there (a box's face, a cylinder's side or cap, a mesh's
 * face). A part belongs to the shape, never to a composition of it: a face a difference cuts into two pieces is
 * one part. A shape without them has one part, 0.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T>
inline constexpr bool hasParts = SurfaceDetail::HasParts<S, T>::value;

namespace SurfaceDetail {

/**
 * @brief How many parts a shape has: its getNumParts(), 1 for a shape without parts.
 * @details Internal helper; not part of the public API. Host only.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in] a_shape The shape.
 * @return The count.
 */
template <class T, class S>
[[nodiscard]] EBGEOMETRY_HOST
inline uint32_t
numPartsOf(const S& a_shape)
{
  if constexpr (hasParts<S, T>) {
    return a_shape.getNumParts();
  }
  else {
    return 1U;
  }
}

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

    if constexpr (hasParts<S, T>) {
      surface.m_part = a_shape.closestPart(a_point);
    }

    return surface;
  }
}

} // namespace SurfaceDetail

/**
 * @brief The surface of any shape that decides its value at a point: a container's closestSurface() (a tape, a BVH
 * union, a sphere cloud), and for any other shape surface 0 with its part (see hasParts). For code templated on a
 * shape. Callable on a device.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in] a_shape The shape.
 * @param[in] a_point Query point, in the shape's frame.
 * @return The surface.
 */
template <class T, class S>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline SurfaceRef<T>
closestSurface(const S& a_shape, const Vec3T<T>& a_point) noexcept
{
  return SurfaceDetail::closestSurfaceOf<T>(a_shape, a_point);
}

} // namespace EBGeometry

#endif
