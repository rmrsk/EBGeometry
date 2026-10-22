// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Ray.hpp
 * @brief  Ray queries on any shape: where a ray first crosses its surface (firstHit()), and whether a
 * segment crosses it (intersects()).
 * @details The queries need only the shape's signed distance and bounding box, so they work for every
 * shape (see isShape): analytic shapes, mesh SDFs, BVH unions, Tape and user types. Along the part of
 * the ray inside the shape's box they sphere trace (step by the distance to the surface, which never
 * steps over it) when the value is a distance or a bound on one, and step at a fixed length and bisect
 * on a sign change when it is not (DistanceQuality::NotADistance). A query that cannot decide within
 * its step budget says so (HitStatus::Unresolved) instead of reporting a miss.
 *
 * A shape with a ray form of its own (see RayForm) is ray traced instead: TriMeshSDF follows the ray
 * through its BVH and intersects its triangles, which is exact and much faster than sphere tracing.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_RAY_HPP
#define EBGEOMETRY_RAY_HPP

// Std includes
#include <cstdint>

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_DistanceQuality.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_MeshDistanceFunctions.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief A ray: the points origin + t * direction.
 * @details The queries take the direction to be of unit length, so that t is a distance along the ray.
 * @tparam T Floating-point precision.
 */
template <class T>
struct RayT
{
  /**
   * @brief Floating-point precision.
   */
  using Scalar = T;

  /**
   * @brief Where the ray starts.
   */
  Vec3T<T> m_origin;

  /**
   * @brief Unit direction.
   */
  Vec3T<T> m_direction;

  /**
   * @brief The point at a distance along the ray.
   * @param[in] a_t Distance.
   * @return origin + a_t * direction.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Vec3T<T>
  at(const T a_t) const noexcept;
};

/**
 * @brief The outcome of a ray query.
 */
enum class HitStatus : uint8_t
{
  Miss,      ///< The ray does not cross the surface in the interval.
  Hit,       ///< The ray crosses the surface; HitT says where.
  Unresolved ///< The query ran out of steps before it could decide (a ray grazing the surface, say).
};

/**
 * @brief Where a ray first crosses a surface.
 * @tparam T Floating-point precision.
 */
template <class T>
struct HitT
{
  /**
   * @brief Whether the ray crosses the surface.
   */
  HitStatus m_status = HitStatus::Miss;

  /**
   * @brief Distance along the ray: of the crossing for a hit, of the last point reached for an
   * unresolved query, and infinity for a miss.
   */
  T m_t = Math::Limits<T>::infinity();

  /**
   * @brief The point at m_t (for a hit or an unresolved query).
   */
  Vec3T<T> m_point = Vec3T<T>::zeros();

  /**
   * @brief The finest element the shape can name at the crossing: the leaf for a Tape (see
   * Tape::primitiveAt()), the closest face for a mesh SDF, and 0 for any other shape.
   */
  uint32_t m_primitive = 0;

  /**
   * @brief Whether the ray enters the object there (it was outside just before), rather than leaving it.
   */
  bool m_entering = false;

  /**
   * @brief The outward unit normal at the crossing, when the shape's ray form knows it (see RayForm);
   * zero otherwise.
   */
  Vec3T<T> m_normal = Vec3T<T>::zeros();

  /**
   * @brief Whether the crossing is exact, rather than found to within RayOptions::m_tolerance.
   */
  bool m_exact = false;

  /**
   * @brief Whether the ray crosses the surface.
   * @return m_status == HitStatus::Hit.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  hit() const noexcept;
};

/**
 * @brief How a ray query marches.
 * @details Lengths are relative to the shape's scale: the diagonal of its bounding box, or m_scale if
 * that is positive. A shape whose box is unbounded (a plane, a complement) has no diagonal; give it a
 * scale, or the scale is 1.
 * @tparam T Floating-point precision.
 */
template <class T>
struct RayOptions
{
  /**
   * @brief A crossing is accepted where |value| is below this, times the scale.
   */
  T m_tolerance = T(1e-4);

  /**
   * @brief The shape's length scale; 0 takes the diagonal of its bounding box.
   */
  T m_scale = T(0);

  /**
   * @brief Steps a query may take before it is unresolved.
   */
  uint32_t m_maxSteps = 512;

  /**
   * @brief Step length, times the scale, where the value is not a distance (DistanceQuality::NotADistance).
   * Features thinner than this can be missed.
   */
  T m_step = T(0.005);
};

/**
 * @brief How to ray trace a shape directly, instead of sphere tracing its signed distance.
 * @details The primary template says a shape has no ray form (available is false), and firstHit()
 * sphere traces it. A specialization for a shape type S says it has one:
 * @code
 *   template <>
 *   struct EBGeometry::RayForm<MyShape>
 *   {
 *     static constexpr bool available = true;
 *
 *     EBGEOMETRY_HOST_DEVICE static HitT<T>
 *     firstHit(const MyShape& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax) noexcept;
 *   };
 * @endcode
 * Its firstHit() returns the first crossing in [a_tMin, a_tMax], entering or leaving, with every field
 * of HitT filled in (HitT::m_exact true if the crossing is exact). TriMeshSDF has one. The second
 * template parameter is for partial specializations that depend on a condition.
 * @tparam S Shape type.
 */
template <class S, class = void>
struct RayForm
{
  /**
   * @brief Whether S has a ray form.
   */
  static constexpr bool available = false;
};

/**
 * @brief TriMeshSDF's ray form: the ray followed through the BVH, nearest box first, and intersected
 * with the triangles in each leaf it reaches (Möller-Trumbore), stopping at boxes beyond the nearest
 * crossing found. Exact, and callable on a device.
 * @tparam T Floating-point precision.
 * @tparam K BVH branching factor.
 * @tparam W Triangles per group.
 */
template <class T, size_t K, size_t W>
struct RayForm<TriMeshSDF<T, K, W>>
{
  /**
   * @brief TriMeshSDF has a ray form.
   */
  static constexpr bool available = true;

  /**
   * @brief The first crossing of the mesh along a ray.
   * @param[in] a_sdf  The mesh.
   * @param[in] a_ray  Ray, with a unit direction.
   * @param[in] a_tMin Start of the interval.
   * @param[in] a_tMax End of the interval.
   * @return The crossing (face id in HitT::m_primitive, the normal interpolated from the vertex normals),
   * or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  firstHit(const TriMeshSDF<T, K, W>& a_sdf, const RayT<T>& a_ray, T a_tMin, T a_tMax) noexcept;
};

/**
 * @brief The first crossing of a shape's surface along a ray, entering or leaving.
 * @details Searches [a_tMin, a_tMax], clipped to the shape's bounding box. The crossing is reported
 * whichever way the ray goes through it: a ray that starts inside the object reports where it leaves,
 * with HitT::m_entering false. Sphere traces when the shape's value is a distance or a bound on one
 * (its distanceQuality, or a Tape's getDistanceQuality()), which cannot step over a surface, and steps
 * at RayOptions::m_step and bisects otherwise. A shape with a ray form (see RayForm) is ray traced
 * instead, and a_options does not apply. Callable on a device.
 * @tparam T Floating-point precision.
 * @tparam S Shape type (see isShape).
 * @param[in] a_shape   Shape.
 * @param[in] a_ray     Ray, with a unit direction.
 * @param[in] a_tMin    Start of the interval.
 * @param[in] a_tMax    End of the interval (may be infinite).
 * @param[in] a_options How to march.
 * @return The crossing, a miss, or an unresolved query.
 */
template <class T, class S>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline HitT<T>
firstHit(const S&                 a_shape,
         const RayT<T>&           a_ray,
         typename RayT<T>::Scalar a_tMin,
         typename RayT<T>::Scalar a_tMax,
         const RayOptions<T>&     a_options = RayOptions<T>()) noexcept;

/**
 * @brief Whether the segment between two points crosses a shape's surface.
 * @details When the shape's value has different signs at the two ends the segment crosses the surface,
 * and no march is needed; otherwise this is firstHit() along the segment, which finds a segment that
 * goes in and out again. Callable on a device.
 * @tparam T Floating-point precision.
 * @tparam S Shape type (see isShape).
 * @param[in] a_shape   Shape.
 * @param[in] a_from    One end.
 * @param[in] a_to      The other end.
 * @param[in] a_options How to march.
 * @return HitStatus::Hit, HitStatus::Miss, or HitStatus::Unresolved if the march ran out of steps.
 */
template <class T, class S>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline HitStatus
intersects(const S&             a_shape,
           const Vec3T<T>&      a_from,
           const Vec3T<T>&      a_to,
           const RayOptions<T>& a_options = RayOptions<T>()) noexcept;

} // namespace EBGeometry

#include "EBGeometry_RayImplem.hpp"

#endif
