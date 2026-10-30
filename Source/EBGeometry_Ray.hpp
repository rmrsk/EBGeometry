// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Ray.hpp
 * @brief  Ray queries on any shape: where a ray, or the segment between two points, first crosses its
 * surface (firstHit()), and whether it crosses it at all (intersects()).
 * @details The queries need only a shape's signed distance and bounding box, so they work for every shape
 * (see isShape): analytic shapes, mesh SDFs, BVH unions, fused compositions, tapes and user types. Along
 * the part of the ray inside the shape's box they march: they sphere trace (step by the value, which never
 * steps over the surface) where the value is a distance or a bound on one, and step at a fixed length and
 * bisect on a sign change where it is not (DistanceQuality::NotADistance). A query that cannot decide
 * within its step budget says so (HitStatus::Unresolved) instead of reporting a miss.
 *
 * A shape with a ray form (see RayForm) is ray traced instead, which is exact and much faster: the plane,
 * sphere, box, cylinders and capsule in closed form (EBGeometry_AnalyticRayForms.hpp), the mesh SDFs
 * through their BVHs and faces (EBGeometry_MeshRayForms.hpp), and the compositions built from them, a
 * union, intersection, difference, complement or affine map of shapes with ray forms (the fused
 * operations and BVHUnion in EBGeometry_CSGRayForms.hpp, and Tape): those combine where the ray is
 * inside each operand (its spans, see RaySpansT), and march only the parts that have no ray form. An offset
 * or annulus of a shape whose level sets have a closed form (a plane, sphere, box, infinite cylinder, capsule
 * or mesh SDF, or an affine map of one) is traced exactly too (EBGeometry_OffsetRayForms.hpp): always as a
 * fused operation, and in a tape that lists TapeFeature::ExactOffsets. RayOptions::m_march marches any shape
 * instead.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_RAY_HPP
#define EBGEOMETRY_RAY_HPP

// Std includes
#include <cstdint>
#include <type_traits>

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_DistanceQuality.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief A ray: the points origin + t * direction.
 * @details The queries take the direction to be of unit length, so that t is a distance along the ray. This is
 * checked only with assertions on (EBGEOMETRY_EXPECT): with another length, a marched query steps the wrong
 * distance and can step over the surface.
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
   * @param[in] a_t Distance along the ray.
   * @return origin + a_t * direction.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Vec3T<T>
  at(T a_t) const noexcept;
};

/**
 * @brief The outcome of a ray query.
 */
enum class HitStatus : uint8_t
{
  Miss,      ///< The ray does not cross the surface in the interval.
  Hit,       ///< The ray crosses the surface; HitT says where.
  Unresolved ///< The query ran out of steps or passes before it could decide (a ray grazing the surface, say).
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
   * @brief Distance along the ray: of the crossing for a hit, of the last point reached for an unresolved
   * query, and infinity for a miss.
   */
  T m_t = Math::Limits<T>::infinity();

  /**
   * @brief The point at m_t (for a hit or an unresolved query).
   */
  Vec3T<T> m_point = Vec3T<T>::zeros();

  /**
   * @brief The finest element the shape can name at the crossing: the face of a mesh SDF, the deciding
   * leaf of a tape (see Tape::closestPrimitive()), what its members name for a BVH union (a member mesh's
   * face), and 0 for any other shape.
   */
  uint32_t m_primitive = 0;

  /**
   * @brief The outward unit normal at the crossing: from the shape's ray form, or the normalized gradient of
   * its signed distance for a marched crossing.
   */
  Vec3T<T> m_normal = Vec3T<T>::zeros();

  /**
   * @brief Whether the ray enters the object there (it was outside just before), rather than leaving it.
   */
  bool m_entering = false;

  /**
   * @brief Whether the crossing is exact (found by a ray form), rather than found to within
   * RayOptions::m_tolerance by marching.
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
 * @brief How a query marches, where a shape has no ray form.
 * @details Lengths are relative to the shape's scale: the diagonal of its bounding box, or m_scale if that
 * is positive. A shape whose box is unbounded (a plane, a complement) has no diagonal; give it a scale, or
 * the scale is 1.
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
   * Features thinner than this can be missed, and a missed feature is a miss, not an unresolved query.
   */
  T m_step = T(0.005);

  /**
   * @brief Whether to march the shape even where it has a ray form, as a shape without one is marched: a
   * reference to compare a ray form with, or a way round one.
   */
  bool m_march = false;
};

/**
 * @brief One end of a span: where a ray enters or leaves an object.
 * @details An end is either a crossing of the object's surface, or the end of the interval the ray was
 * traced over (or of what is known of it; see RaySpansT), which is no crossing. A crossing carries no
 * normal, so that span lists stay small (on a device they live in each thread's local memory, where every
 * word copied or compared costs a memory operation, which is most of what tracing costs): a composition
 * names the leaf it lies on (m_leaf), and asks that leaf for the normal once, at the crossing the trace ends
 * at.
 * @tparam T Floating-point precision.
 */
template <class T>
struct SpanEndT
{
  /**
   * @brief What an end is.
   */
  enum Kind : uint8_t
  {
    Open    = 0, ///< No crossing: an end of the interval, or of what is known.
    Exact   = 1, ///< A crossing found exactly, by a ray form.
    Marched = 2  ///< A crossing found by marching, to within RayOptions::m_tolerance.
  };

  /**
   * @brief Distance along the ray.
   */
  T m_t;

  /**
   * @brief Which of a composition's leaves the crossing lies on, numbered by the composition; a shape that
   * is not a composition leaves it 0.
   */
  uint32_t m_leaf;

  /**
   * @brief What the end is.
   */
  Kind m_kind;

  /**
   * @brief Whether the end is a crossing of the surface.
   * @return m_kind != Open.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  crossing() const noexcept;
};

/**
 * @brief Where a ray is inside an object over an interval [t0, t1]: its spans, in order, each from where it
 * enters to where it leaves, complete up to a horizon.
 * @details A span that begins before t0 begins at t0 with an Open end, and one that ends after t1 ends at
 * t1 with an Open end. The list holds at most Capacity spans. Past that, add() drops a span and lowers the
 * horizon to where it begins: the list is then complete only on [t0, m_horizon], and a query needing more
 * starts again from the horizon. A shape whose ray form only finds the first crossing (see RayForm)
 * describes the interval up to it in the same way.
 * @tparam T Floating-point precision.
 */
template <class T>
struct RaySpansT
{
  /**
   * @brief Most spans held.
   */
  static constexpr uint32_t Capacity = 8;

  /**
   * @brief A span: where the ray enters, and where it leaves.
   */
  struct Span
  {
    /**
     * @brief Where the ray enters.
     */
    SpanEndT<T> m_in;

    /**
     * @brief Where it leaves.
     */
    SpanEndT<T> m_out;
  };

  /**
   * @brief The spans, in order and disjoint.
   */
  Span m_spans[Capacity];

  /**
   * @brief How many are held.
   */
  uint32_t m_count = 0;

  /**
   * @brief The list is complete on [t0, m_horizon]: nothing about the object beyond it is known.
   */
  T m_horizon = Math::Limits<T>::infinity();

  /**
   * @brief Empty the list, complete up to a horizon.
   * @param[in] a_horizon Horizon: the end of the interval, for a list that will be complete.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  reset(T a_horizon) noexcept;

  /**
   * @brief Append a span after the last one, or, if the list is full, drop it and lower the horizon to
   * where it begins.
   * @param[in] a_in  Where the ray enters.
   * @param[in] a_out Where it leaves.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  add(const SpanEndT<T>& a_in, const SpanEndT<T>& a_out) noexcept;

  /**
   * @brief Append the span [a_in, a_out] cut to [a_t0, a_t1]: an end outside it (or infinite) becomes an
   * Open end there. Nothing is added if the span misses the interval.
   * @param[in] a_in  Where the ray enters (or -infinity); above a_out for a ray that misses.
   * @param[in] a_out Where it leaves (or +infinity).
   * @param[in] a_t0  Start of the interval.
   * @param[in] a_t1  End of the interval.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  addClipped(T a_in, T a_out, T a_t0, T a_t1) noexcept;
};

/**
 * @brief How to ray trace a shape directly, instead of marching its signed distance.
 * @details The primary template is empty: a shape has no ray form, and the queries march it. A
 * specialisation for a shape type S gives it one, through the functions it defines; they are found by
 * their signatures, so one with a wrong signature is not used (`static_assert(hasRayForm<S, T>)` checks it):
 * @code
 *   template <class T>
 *   struct EBGeometry::RayForm<MyShape<T>>
 *   {
 *     EBGEOMETRY_HOST_DEVICE static HitT<T>
 *     firstHit(const MyShape<T>& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax, const RayOptions<T>& a_options) noexcept;
 *
 *     EBGEOMETRY_HOST_DEVICE static void
 *     spans(const MyShape<T>& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax, const RayOptions<T>& a_options,
 *           bool a_firstOnly, RaySpansT<T>& a_spans) noexcept;
 *   };
 * @endcode
 * firstHit() makes the shape a ray form: it returns the first crossing in [a_tMin, a_tMax], entering or
 * leaving, with every field of HitT filled in. The others are optional. spans() writes where the ray is
 * inside the object over [a_tMin, a_tMax] (see RaySpansT), which is what an intersection, difference or
 * complement of the shape with others needs to be traced exactly (the ends carry no normal: the composition
 * calls firstHit() near the crossing it ends at for that); a_firstOnly says that only the first
 * crossing is needed (the list may then stop after it, its horizon there). Without spans(), a composition
 * finds the spans by calling firstHit() from one crossing to the next. intersects(), with firstHit()'s
 * arguments and returning HitStatus, is a yes/no query that can stop at the first crossing it finds;
 * without it, intersects() calls firstHit(). The second template parameter is for partial
 * specialisations that depend on a condition.
 * @tparam S Shape type.
 */
template <class S, class = void>
struct RayForm
{
};

/**
 * @brief The first crossing of a shape's surface along a ray, entering or leaving.
 * @details Searches [a_tMin, a_tMax], clipped to the shape's bounding box. The crossing is reported
 * whichever way the ray goes through it: a ray that starts inside the object reports where it leaves, with
 * HitT::m_entering false. A shape with a ray form is ray traced; any other is marched (see the file
 * description and RayOptions). Callable on a device.
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
firstHit(const S&                 a_shape,
         const RayT<T>&           a_ray,
         typename RayT<T>::Scalar a_tMin,
         typename RayT<T>::Scalar a_tMax,
         const RayOptions<T>&     a_options = RayOptions<T>()) noexcept;

/**
 * @brief The first crossing of a shape's surface on the segment from a_from to a_to.
 * @details The ray from a_from towards a_to, over [a_tMin, |a_to - a_from|]: HitT::m_t is the distance from
 * a_from. A crossing at a_tMin is a hit; a caller stepping off a surface it is sitting on passes a small
 * a_tMin. A segment of zero length is a miss. Callable on a device.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in] a_shape   The shape.
 * @param[in] a_from    Start of the segment.
 * @param[in] a_to      End of the segment.
 * @param[in] a_tMin    Distance from a_from where the search starts.
 * @param[in] a_options How to march.
 * @return The crossing, a miss, or an unresolved query.
 */
template <class T, class S>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline HitT<T>
firstHit(const S&             a_shape,
         const Vec3T<T>&      a_from,
         const Vec3T<T>&      a_to,
         T                    a_tMin    = T(0),
         const RayOptions<T>& a_options = RayOptions<T>()) noexcept;

/**
 * @brief Whether a ray crosses a shape's surface in [a_tMin, a_tMax].
 * @details Stops at the first crossing found, not the nearest, where the shape's ray form allows it.
 * Callable on a device.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in] a_shape   The shape.
 * @param[in] a_ray     The ray, with a unit direction.
 * @param[in] a_tMin    Start of the interval.
 * @param[in] a_tMax    End of the interval.
 * @param[in] a_options How to march.
 * @return Hit, Miss, or Unresolved.
 */
template <class T, class S>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline HitStatus
intersects(const S&                 a_shape,
           const RayT<T>&           a_ray,
           typename RayT<T>::Scalar a_tMin,
           typename RayT<T>::Scalar a_tMax,
           const RayOptions<T>&     a_options = RayOptions<T>()) noexcept;

/**
 * @brief Whether the segment from a_from to a_to crosses a shape's surface.
 * @details For a marched shape, where the value has different signs at the two ends the segment crosses the
 * surface and no search is needed; otherwise the segment is searched, since it may go in and out again. A
 * shape with a ray form is always searched. Callable on a device.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in] a_shape   The shape.
 * @param[in] a_from    Start of the segment.
 * @param[in] a_to      End of the segment.
 * @param[in] a_tMin    Distance from a_from where the search starts.
 * @param[in] a_options How to march.
 * @return Hit, Miss, or Unresolved.
 */
template <class T, class S>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline HitStatus
intersects(const S&             a_shape,
           const Vec3T<T>&      a_from,
           const Vec3T<T>&      a_to,
           T                    a_tMin    = T(0),
           const RayOptions<T>& a_options = RayOptions<T>()) noexcept;

} // namespace EBGeometry

#include "EBGeometry_RayImplem.hpp"
#include "EBGeometry_RaySpans.hpp"

#endif
