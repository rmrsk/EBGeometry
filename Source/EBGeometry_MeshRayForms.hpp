// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

/**
 * @file   EBGeometry_MeshRayForms.hpp
 * @brief  The ray forms (see RayForm) of the mesh SDFs. TriMeshSDF: its BVH walked along the ray
 * (PackedBVH::traverse() with BVH::RayQuery), and its triangles crossed exactly (TriangleRay::crossing()).
 * MeshSDF: the same walk, its polygon faces crossed exactly (DCEL::FaceT::rayCrossing()). FlatMeshSDF:
 * every face, without a BVH. With this header included, firstHit() and intersects() on a mesh SDF are exact
 * instead of marched, and so are compositions of mesh SDFs (see EBGeometry_CSGRayForms.hpp), through
 * spans(): the crossings along the ray, nearest first, paired into spans by the side of the face the ray
 * crosses from. A closed mesh is needed for the spans to mean inside and outside.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_MESHRAYFORMS_HPP
#define EBGEOMETRY_MESHRAYFORMS_HPP

// Std includes
#include <cstddef>
#include <cstdint>

// Our includes
#include "EBGeometry_MeshDistanceFunctions.hpp"
#include "EBGeometry_Ray.hpp"

namespace EBGeometry {

/**
 * @brief TriMeshSDF's ray form: the nearest triangle the ray crosses, found by walking the BVH nearest box
 * first and stopping at boxes farther than the nearest crossing so far. Exact, and callable on a device.
 * @details A crossing is reported from either side of a triangle; it is entering where the ray runs against
 * the triangle's outward normal. HitT::m_primitive is the id of the face the triangle was cut from (see
 * TriMeshSDF::getClosestFace()) and HitT::m_normal is the triangle's normal. Where the ray crosses several
 * triangles within a few ulps of the nearest crossing (through an edge or a vertex they share), the one with the
 * lowest face id is the hit, so that the host and a device, which round the crossings differently, name the same
 * face and normal.
 * @tparam T Floating-point precision.
 * @tparam K BVH branching factor.
 * @tparam W Triangles per leaf group.
 */
template <class T, size_t K, size_t W>
struct RayForm<TriMeshSDF<T, K, W>>
{
  /**
   * @brief The first crossing of the mesh along a ray.
   * @param[in] a_shape   The mesh.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options Unused: the crossing is found exactly.
   * @return The crossing, with its face id and normal, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  firstHit(const TriMeshSDF<T, K, W>& a_shape,
           const RayT<T>&             a_ray,
           T                          a_tMin,
           T                          a_tMax,
           const RayOptions<T>&       a_options) noexcept;

  /**
   * @brief Whether a ray crosses the mesh: the walk stops at the first crossing it finds.
   * @param[in] a_shape   The mesh.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options Unused: the crossing is found exactly.
   * @return Hit or Miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitStatus
  intersects(const TriMeshSDF<T, K, W>& a_shape,
             const RayT<T>&             a_ray,
             T                          a_tMin,
             T                          a_tMax,
             const RayOptions<T>&       a_options) noexcept;

  /**
   * @brief Where a ray is inside the mesh over an interval: its crossings, nearest first, paired into spans
   * (see RayDetail::NearestCrossings), or with a_firstOnly only the first crossing (see firstHit()).
   * @param[in]  a_shape     The mesh.
   * @param[in]  a_ray       Ray, with a unit direction.
   * @param[in]  a_tMin      Start of the interval.
   * @param[in]  a_tMax      End of the interval.
   * @param[in]  a_options   Unused: the crossings are found exactly.
   * @param[in]  a_firstOnly Whether only the first crossing is needed.
   * @param[out] a_spans     The list.
   */
  EBGEOMETRY_HOST_DEVICE
  static inline void
  spans(const TriMeshSDF<T, K, W>& a_shape,
        const RayT<T>&             a_ray,
        T                          a_tMin,
        T                          a_tMax,
        const RayOptions<T>&       a_options,
        bool                       a_firstOnly,
        RaySpansT<T>&              a_spans) noexcept;
};

/**
 * @brief MeshSDF's ray form: the nearest polygon face the ray crosses, found by walking the BVH nearest box
 * first and stopping at boxes farther than the nearest crossing so far. Exact, and callable on a device.
 * @details A crossing is reported from either side of a face; it is entering where the ray runs against the
 * face's outward normal. HitT::m_primitive is the face's id (see MeshSDF::getClosestFace()) and
 * HitT::m_normal its normal. The faces are taken to be planar and convex (see DCEL::FaceT::rayCrossing()). Where
 * the ray crosses several faces within a few ulps of the nearest crossing (through an edge or a vertex they
 * share), the one with the lowest id is the hit, so that the host and a device name the same face.
 * @tparam T Floating-point precision.
 * @tparam K BVH branching factor.
 */
template <class T, size_t K>
struct RayForm<MeshSDF<T, K>>
{
  /**
   * @brief The first crossing of the mesh along a ray.
   * @param[in] a_shape   The mesh.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options Unused: the crossing is found exactly.
   * @return The crossing, with its face id and normal, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  firstHit(
    const MeshSDF<T, K>& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax, const RayOptions<T>& a_options) noexcept;

  /**
   * @brief Whether a ray crosses the mesh: the walk stops at the first crossing it finds.
   * @param[in] a_shape   The mesh.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options Unused: the crossing is found exactly.
   * @return Hit or Miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitStatus
  intersects(
    const MeshSDF<T, K>& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax, const RayOptions<T>& a_options) noexcept;

  /**
   * @brief Where a ray is inside the mesh over an interval: its crossings, nearest first, paired into spans
   * (see RayDetail::NearestCrossings), or with a_firstOnly only the first crossing (see firstHit()).
   * @param[in]  a_shape     The mesh.
   * @param[in]  a_ray       Ray, with a unit direction.
   * @param[in]  a_tMin      Start of the interval.
   * @param[in]  a_tMax      End of the interval.
   * @param[in]  a_options   Unused: the crossings are found exactly.
   * @param[in]  a_firstOnly Whether only the first crossing is needed.
   * @param[out] a_spans     The list.
   */
  EBGEOMETRY_HOST_DEVICE
  static inline void
  spans(const MeshSDF<T, K>& a_shape,
        const RayT<T>&       a_ray,
        T                    a_tMin,
        T                    a_tMax,
        const RayOptions<T>& a_options,
        bool                 a_firstOnly,
        RaySpansT<T>&        a_spans) noexcept;
};

/**
 * @brief FlatMeshSDF's ray form: every face tested, as FlatMeshSDF's signed distance tests every face (no
 * BVH). Exact, and callable on a device; as for MeshSDF otherwise.
 * @tparam T Floating-point precision.
 */
template <class T>
struct RayForm<FlatMeshSDF<T>>
{
  /**
   * @brief The first crossing of the mesh along a ray.
   * @param[in] a_shape   The mesh.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options Unused: the crossing is found exactly.
   * @return The crossing, with its face id and normal, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  firstHit(
    const FlatMeshSDF<T>& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax, const RayOptions<T>& a_options) noexcept;

  /**
   * @brief Where a ray is inside the mesh over an interval: its crossings, nearest first, paired into spans
   * (see RayDetail::NearestCrossings), or with a_firstOnly only the first crossing (see firstHit()).
   * @param[in]  a_shape     The mesh.
   * @param[in]  a_ray       Ray, with a unit direction.
   * @param[in]  a_tMin      Start of the interval.
   * @param[in]  a_tMax      End of the interval.
   * @param[in]  a_options   Unused: the crossings are found exactly.
   * @param[in]  a_firstOnly Whether only the first crossing is needed.
   * @param[out] a_spans     The list.
   */
  EBGEOMETRY_HOST_DEVICE
  static inline void
  spans(const FlatMeshSDF<T>& a_shape,
        const RayT<T>&        a_ray,
        T                     a_tMin,
        T                     a_tMax,
        const RayOptions<T>&  a_options,
        bool                  a_firstOnly,
        RaySpansT<T>&         a_spans) noexcept;
};

namespace RayDetail {

/**
 * @brief A TriMeshSDF is traced on past an interval (see TracedOnCheaply).
 * @tparam T Floating-point precision.
 * @tparam K BVH branching factor.
 * @tparam W Triangles per leaf group.
 */
template <class T, size_t K, size_t W>
struct TracedOnCheaply<TriMeshSDF<T, K, W>> : std::true_type
{
};

/**
 * @brief A TriMeshSDF's crossing names its face, its part (see HitNamesPart).
 * @tparam T Floating-point precision.
 * @tparam K BVH branching factor.
 * @tparam W Triangles per leaf group.
 */
template <class T, size_t K, size_t W>
struct HitNamesPart<TriMeshSDF<T, K, W>> : std::true_type
{
};

/**
 * @brief A MeshSDF is traced on past an interval (see TracedOnCheaply).
 * @tparam T Floating-point precision.
 * @tparam K BVH branching factor.
 */
template <class T, size_t K>
struct TracedOnCheaply<MeshSDF<T, K>> : std::true_type
{
};

/**
 * @brief A MeshSDF's crossing names its face, its part (see HitNamesPart).
 * @tparam T Floating-point precision.
 * @tparam K BVH branching factor.
 */
template <class T, size_t K>
struct HitNamesPart<MeshSDF<T, K>> : std::true_type
{
};

/**
 * @brief A FlatMeshSDF is traced on past an interval (see TracedOnCheaply): its trace visits every face, and so does
 * the distance query it saves.
 * @tparam T Floating-point precision.
 */
template <class T>
struct TracedOnCheaply<FlatMeshSDF<T>> : std::true_type
{
};

/**
 * @brief A FlatMeshSDF's crossing names its face, its part (see HitNamesPart).
 * @tparam T Floating-point precision.
 */
template <class T>
struct HitNamesPart<FlatMeshSDF<T>> : std::true_type
{
};

/**
 * @brief A crossing of a mesh face as a HitT.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_ray    Ray.
 * @param[in] a_t      Distance along the ray, or +infinity for none.
 * @param[in] a_face   The face's id.
 * @param[in] a_normal The face's outward unit normal.
 * @return The hit, or a miss.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline HitT<T>
faceHit(const RayT<T>& a_ray, const T a_t, const uint32_t a_face, const Vec3T<T>& a_normal) noexcept
{
  HitT<T> hit;

  if (a_face != UINT32_MAX && a_t < Math::Limits<T>::infinity()) {
    hit.m_status    = HitStatus::Hit;
    hit.m_t         = a_t;
    hit.m_point     = a_ray.at(a_t);
    hit.m_primitive = a_face;
    hit.m_normal    = a_normal;
    hit.m_entering  = dot(a_ray.m_direction, a_normal) < T(0);
    hit.m_exact     = true;
  }

  return hit;
}

/**
 * @brief How far past a mesh's nearest crossing so far its first hit still looks: crossings within a few ulps of
 * the nearest are one tie, which the face with the lowest id wins (see replacesNearest()).
 * @details Internal helper; not part of the public API. A ray through an edge or a vertex shared by several faces
 * crosses each of them at the same point, at distances that differ by each face's rounding: a few ulps of the
 * distance and of the ray origin's coordinates, as the crossing test's own widening measures them
 * (TriangleRay::crossing()). A host and a device round them differently (a device fuses multiplies and adds), so
 * the strictly nearest crossing picks one face on one side and another on the other, with another normal. The
 * window is 16 ulps of the distance plus the origin's coordinates: on a float armadillo (99,976 triangles) seen
 * from about 300 away, the faces at a vertex crossed 0 to 9 ulps apart, and a host's and a device's crossing of one
 * face 1 to 4. Ties broken by the lowest face id in that window left one pixel of 262,144 with another face on the
 * device (a ray grazing an edge, which one side's widened inside test counts as crossing a face and the other does
 * not), against 35 broken by distance alone.
 * @tparam T Floating-point precision.
 * @param[in] a_ray      Ray.
 * @param[in] a_nearestT The nearest crossing so far.
 * @param[in] a_found    Whether there is one.
 * @param[in] a_tMax     End of the query's interval, which the reach never passes.
 * @return How far along the ray a face may still be crossed and win: a_tMax until a crossing is found.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline T
tieReach(const RayT<T>& a_ray, const T a_nearestT, const bool a_found, const T a_tMax) noexcept
{
  if (!a_found) {
    return a_tMax;
  }

  const Vec3T<T>& o = a_ray.m_origin;
  const T         window =
    T(16) * Math::Limits<T>::epsilon() * (std::abs(a_nearestT) + std::abs(o[0]) + std::abs(o[1]) + std::abs(o[2]));

  return Math::min(a_nearestT + window, a_tMax);
}

/**
 * @brief Whether a face's crossing replaces a mesh's nearest crossing so far in its first hit: there is none so
 * far, or it is nearer by more than the tie window, or within the window with a lower face id (see tieReach()).
 * @details Internal helper; not part of the public API. Every mesh SDF's firstHit() keeps its nearest crossing
 * with this rule, so the face it names does not depend on the order the walk visits the faces, nor on the rounding
 * of the crossings' distances, for crossings within the window.
 * @tparam T Floating-point precision.
 * @param[in] a_ray         Ray.
 * @param[in] a_t           The face's crossing (finite).
 * @param[in] a_face        The face's id.
 * @param[in] a_nearestT    The nearest crossing so far.
 * @param[in] a_nearestFace Its face's id, UINT32_MAX for none so far.
 * @return Whether the face's crossing is the nearest now.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline bool
replacesNearest(
  const RayT<T>& a_ray, const T a_t, const uint32_t a_face, const T a_nearestT, const uint32_t a_nearestFace) noexcept
{
  if (a_nearestFace == UINT32_MAX) {
    return true;
  }

  const T reach  = tieReach(a_ray, a_nearestT, true, Math::Limits<T>::infinity());
  const T window = reach - a_nearestT;

  return a_t < a_nearestT - window || (a_t <= reach && a_face < a_nearestFace);
}

/**
 * @brief The nearest crossings of a mesh along a ray, in order, as a BVH walk finds them: at most Capacity,
 * enough for the spans a list holds and one more.
 * @details Internal helper; not part of the public API. Once full, a nearer crossing replaces the farthest,
 * and the walk's bound is the farthest, so that it leaves out the boxes beyond; the crossings are then
 * known only up to it.
 * @tparam T Floating-point precision.
 */
template <class T>
struct NearestCrossings
{
  /**
   * @brief Most crossings held.
   */
  static constexpr uint32_t Capacity = 2U * RaySpansT<T>::Capacity + 1U;

  /**
   * @brief A crossing.
   */
  struct Crossing
  {
    /**
     * @brief Distance along the ray.
     */
    T m_t;

    /**
     * @brief Whether the ray enters there: it runs against the face's outward normal.
     */
    bool m_entering;
  };

  /**
   * @brief The crossings, nearest first.
   */
  Crossing m_crossings[Capacity];

  /**
   * @brief How many are held.
   */
  uint32_t m_count = 0;

  /**
   * @brief End of the interval.
   */
  T m_tMax;

  /**
   * @brief Take a crossing.
   * @param[in] a_t        Distance along the ray.
   * @param[in] a_entering Whether the ray enters there.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  insert(const T a_t, const bool a_entering) noexcept
  {
    if (m_count == Capacity) {
      if (!(a_t < m_crossings[Capacity - 1U].m_t)) {
        return;
      }

      m_count--;
    }

    uint32_t i = m_count;

    while (i > 0 && m_crossings[i - 1U].m_t > a_t) {
      m_crossings[i] = m_crossings[i - 1U];

      i--;
    }

    m_crossings[i] = Crossing{a_t, a_entering};

    m_count++;
  }

  /**
   * @brief How far the crossings are known: the end of the interval, or the farthest held once full.
   * @return The distance; also the walk's bound.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  horizon() const noexcept
  {
    return (m_count == Capacity) ? m_crossings[Capacity - 1U].m_t : m_tMax;
  }

  /**
   * @brief The spans: the crossings paired by the side of the face the ray crosses from.
   * @param[in]  a_t0     Start of the interval.
   * @param[in]  a_inside Whether the ray is inside at a_t0; read only if there are no crossings.
   * @param[out] a_spans  The list.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  toSpans(const T a_t0, const bool a_inside, RaySpansT<T>& a_spans) const noexcept
  {
    const T horizon = this->horizon();

    a_spans.reset(horizon);

    if (m_count == 0U) {
      if (a_inside) {
        a_spans.add(RayDetail::openAt(a_t0), RayDetail::openAt(horizon));
      }

      return;
    }

    CrossingPairer<T> pairer;

    for (uint32_t i = 0; i < m_count; i++) {
      const Crossing& c = m_crossings[i];

      pairer.take(SpanEndT<T>{c.m_t, 0U, SpanEndT<T>::Exact}, c.m_entering, a_t0, a_spans);
    }

    pairer.finish(horizon, a_spans);
  }
};

} // namespace RayDetail

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
RayForm<TriMeshSDF<T, K, W>>::firstHit(const TriMeshSDF<T, K, W>& a_shape,
                                       const RayT<T>&             a_ray,
                                       const T                    a_tMin,
                                       const T                    a_tMax,
                                       const RayOptions<T>& /*a_options*/) noexcept
{
  // The state is the nearest crossing so far; it starts at the far end, so that the walk's bound (the
  // state's distance) leaves out every box beyond it.
  struct Nearest
  {
    T        m_t;
    uint32_t m_face;
    Vec3T<T> m_normal;
  };

  Nearest nearest{a_tMax, UINT32_MAX, Vec3T<T>::zeros()};

  const auto bvh    = a_shape.getRoot();
  const auto groups = bvh.getPrimitives();

  // Lane by lane, so that a tie within a group goes to its lowest face id too (RayDetail::replacesNearest()).
  const auto leaf = [&groups, &a_ray, a_tMin, a_tMax](Nearest& a_state, size_t a_offset, size_t a_count) noexcept {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      const auto& group = groups[static_cast<uint32_t>(i)];
      const T     reach = RayDetail::tieReach(a_ray, a_state.m_t, a_state.m_face != UINT32_MAX, a_tMax);

      T crossings[W];

      group.rayCrossings(a_ray.m_origin, a_ray.m_direction, a_tMin, reach, crossings);

      for (uint32_t lane = 0; lane < uint32_t(W); lane++) {
        const T        t    = crossings[lane];
        const uint32_t face = group.getFaceId(lane);

        if (t < Math::Limits<T>::infinity() &&
            RayDetail::replacesNearest(a_ray, t, face, a_state.m_t, a_state.m_face)) {
          a_state = Nearest{t, face, group.getNormal(lane)};
        }
      }
    }
  };

  const auto bound = [&a_ray, a_tMax](const Nearest& a_state) noexcept -> T {
    return RayDetail::tieReach(a_ray, a_state.m_t, a_state.m_face != UINT32_MAX, a_tMax);
  };

  bvh.traverse(BVH::RayQuery<T>(a_ray.m_origin, a_ray.m_direction, a_tMin), nearest, leaf, bound);

  HitT<T> hit;

  if (nearest.m_face != UINT32_MAX) {
    hit.m_status    = HitStatus::Hit;
    hit.m_t         = nearest.m_t;
    hit.m_point     = a_ray.at(nearest.m_t);
    hit.m_primitive = nearest.m_face;
    hit.m_normal    = nearest.m_normal;
    hit.m_entering  = dot(a_ray.m_direction, nearest.m_normal) < T(0);
    hit.m_exact     = true;
  }

  return hit;
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline HitStatus
RayForm<TriMeshSDF<T, K, W>>::intersects(const TriMeshSDF<T, K, W>& a_shape,
                                         const RayT<T>&             a_ray,
                                         const T                    a_tMin,
                                         const T                    a_tMax,
                                         const RayOptions<T>& /*a_options*/) noexcept
{
  // The walk ends once a crossing is found: the bound drops to -infinity, which no box is within.
  bool found = false;

  const auto bvh    = a_shape.getRoot();
  const auto groups = bvh.getPrimitives();

  const auto leaf = [&groups, &a_ray, a_tMin, a_tMax](bool& a_found, size_t a_offset, size_t a_count) noexcept {
    for (size_t i = a_offset; i < a_offset + a_count && !a_found; i++) {
      uint32_t face   = UINT32_MAX;
      Vec3T<T> normal = Vec3T<T>::zeros();

      const T t =
        groups[static_cast<uint32_t>(i)].rayCrossing(a_ray.m_origin, a_ray.m_direction, a_tMin, a_tMax, face, normal);

      a_found = face != UINT32_MAX && t <= a_tMax;
    }
  };

  const auto bound = [a_tMax](const bool& a_found) noexcept -> T {
    return a_found ? -Math::Limits<T>::infinity() : a_tMax;
  };

  bvh.traverse(BVH::RayQuery<T>(a_ray.m_origin, a_ray.m_direction, a_tMin), found, leaf, bound);

  return found ? HitStatus::Hit : HitStatus::Miss;
}

template <class T, size_t K>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
RayForm<MeshSDF<T, K>>::firstHit(const MeshSDF<T, K>& a_shape,
                                 const RayT<T>&       a_ray,
                                 const T              a_tMin,
                                 const T              a_tMax,
                                 const RayOptions<T>& /*a_options*/) noexcept
{
  // The state is the nearest crossing so far and its face; it starts at the far end, so that the walk's
  // bound leaves out every box beyond it.
  struct Nearest
  {
    T        m_t;
    uint32_t m_face;
  };

  Nearest nearest{a_tMax, UINT32_MAX};

  const auto bvh     = a_shape.getRoot();
  const auto mesh    = a_shape.getMesh();
  const auto faceIds = bvh.getPrimitives();

  const auto leaf = [&faceIds, &mesh, &a_ray, a_tMin, a_tMax](
                      Nearest& a_state, size_t a_offset, size_t a_count) noexcept {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      const uint32_t face  = faceIds[static_cast<uint32_t>(i)];
      const T        reach = RayDetail::tieReach(a_ray, a_state.m_t, a_state.m_face != UINT32_MAX, a_tMax);
      const T        t     = mesh.getFace(face).rayCrossing(a_ray.m_origin, a_ray.m_direction, a_tMin, reach, mesh);

      // A crossing is finite: none is +infinity, however far the interval goes.
      if (t < Math::Limits<T>::infinity() && RayDetail::replacesNearest(a_ray, t, face, a_state.m_t, a_state.m_face)) {
        a_state = Nearest{t, face};
      }
    }
  };

  const auto bound = [&a_ray, a_tMax](const Nearest& a_state) noexcept -> T {
    return RayDetail::tieReach(a_ray, a_state.m_t, a_state.m_face != UINT32_MAX, a_tMax);
  };

  bvh.traverse(BVH::RayQuery<T>(a_ray.m_origin, a_ray.m_direction, a_tMin), nearest, leaf, bound);

  return nearest.m_face == UINT32_MAX
           ? HitT<T>()
           : RayDetail::faceHit(a_ray, nearest.m_t, nearest.m_face, mesh.getFace(nearest.m_face).getNormal());
}

template <class T, size_t K>
EBGEOMETRY_HOST_DEVICE
inline HitStatus
RayForm<MeshSDF<T, K>>::intersects(const MeshSDF<T, K>& a_shape,
                                   const RayT<T>&       a_ray,
                                   const T              a_tMin,
                                   const T              a_tMax,
                                   const RayOptions<T>& /*a_options*/) noexcept
{
  // The walk ends once a crossing is found: the bound drops to -infinity, which no box is within.
  bool found = false;

  const auto bvh     = a_shape.getRoot();
  const auto mesh    = a_shape.getMesh();
  const auto faceIds = bvh.getPrimitives();

  const auto leaf = [&faceIds, &mesh, &a_ray, a_tMin, a_tMax](bool& a_found, size_t a_offset, size_t a_count) noexcept {
    for (size_t i = a_offset; i < a_offset + a_count && !a_found; i++) {
      const auto& face = mesh.getFace(faceIds[static_cast<uint32_t>(i)]);

      // None is +infinity, which an unbounded interval would contain.
      a_found = face.rayCrossing(a_ray.m_origin, a_ray.m_direction, a_tMin, a_tMax, mesh) < Math::Limits<T>::infinity();
    }
  };

  const auto bound = [a_tMax](const bool& a_found) noexcept -> T {
    return a_found ? -Math::Limits<T>::infinity() : a_tMax;
  };

  bvh.traverse(BVH::RayQuery<T>(a_ray.m_origin, a_ray.m_direction, a_tMin), found, leaf, bound);

  return found ? HitStatus::Hit : HitStatus::Miss;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
RayForm<FlatMeshSDF<T>>::firstHit(const FlatMeshSDF<T>& a_shape,
                                  const RayT<T>&        a_ray,
                                  const T               a_tMin,
                                  const T               a_tMax,
                                  const RayOptions<T>& /*a_options*/) noexcept
{
  const auto mesh = a_shape.getMesh();

  T        nearest = a_tMax;
  uint32_t face    = UINT32_MAX;

  for (uint32_t i = 0; i < mesh.numFaces(); i++) {
    const T reach = RayDetail::tieReach(a_ray, nearest, face != UINT32_MAX, a_tMax);
    const T t     = mesh.getFace(i).rayCrossing(a_ray.m_origin, a_ray.m_direction, a_tMin, reach, mesh);

    if (t < Math::Limits<T>::infinity() && RayDetail::replacesNearest(a_ray, t, i, nearest, face)) {
      nearest = t;
      face    = i;
    }
  }

  return face == UINT32_MAX ? HitT<T>() : RayDetail::faceHit(a_ray, nearest, face, mesh.getFace(face).getNormal());
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline void
RayForm<TriMeshSDF<T, K, W>>::spans(const TriMeshSDF<T, K, W>& a_shape,
                                    const RayT<T>&             a_ray,
                                    const T                    a_tMin,
                                    const T                    a_tMax,
                                    const RayOptions<T>&       a_options,
                                    const bool                 a_firstOnly,
                                    RaySpansT<T>&              a_spans) noexcept
{
  if (a_firstOnly) {
    RayDetail::firstSpan(a_shape, a_ray, a_tMin, a_tMax, a_options, a_spans);

    return;
  }

  RayDetail::NearestCrossings<T> crossings;

  crossings.m_tMax = a_tMax;

  const auto bvh    = a_shape.getRoot();
  const auto groups = bvh.getPrimitives();

  const auto leaf =
    [&groups, &a_ray, a_tMin](RayDetail::NearestCrossings<T>& a_state, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = a_offset; i < a_offset + a_count; i++) {
        const auto& group = groups[static_cast<uint32_t>(i)];

        T t[W];

        group.rayCrossings(a_ray.m_origin, a_ray.m_direction, a_tMin, a_state.horizon(), t);

        for (size_t lane = 0; lane < W; lane++) {
          if (t[lane] < Math::Limits<T>::infinity()) {
            a_state.insert(t[lane], dot(a_ray.m_direction, group.getNormal(lane)) < T(0));
          }
        }
      }
    };

  const auto bound = [](const RayDetail::NearestCrossings<T>& a_state) noexcept -> T { return a_state.horizon(); };

  bvh.traverse(BVH::RayQuery<T>(a_ray.m_origin, a_ray.m_direction, a_tMin), crossings, leaf, bound);

  crossings.toSpans(
    a_tMin, crossings.m_count == 0U && RayDetail::insideUncrossed(a_shape, a_ray, a_tMin, a_tMax, a_options), a_spans);
}

template <class T, size_t K>
EBGEOMETRY_HOST_DEVICE
inline void
RayForm<MeshSDF<T, K>>::spans(const MeshSDF<T, K>& a_shape,
                              const RayT<T>&       a_ray,
                              const T              a_tMin,
                              const T              a_tMax,
                              const RayOptions<T>& a_options,
                              const bool           a_firstOnly,
                              RaySpansT<T>&        a_spans) noexcept
{
  if (a_firstOnly) {
    RayDetail::firstSpan(a_shape, a_ray, a_tMin, a_tMax, a_options, a_spans);

    return;
  }

  RayDetail::NearestCrossings<T> crossings;

  crossings.m_tMax = a_tMax;

  const auto bvh     = a_shape.getRoot();
  const auto mesh    = a_shape.getMesh();
  const auto faceIds = bvh.getPrimitives();

  const auto leaf = [&faceIds, &mesh, &a_ray, a_tMin](
                      RayDetail::NearestCrossings<T>& a_state, size_t a_offset, size_t a_count) noexcept {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      const uint32_t face = faceIds[static_cast<uint32_t>(i)];
      const auto&    f    = mesh.getFace(face);
      const T        t    = f.rayCrossing(a_ray.m_origin, a_ray.m_direction, a_tMin, a_state.horizon(), mesh);

      if (t < Math::Limits<T>::infinity()) {
        a_state.insert(t, dot(a_ray.m_direction, f.getNormal()) < T(0));
      }
    }
  };

  const auto bound = [](const RayDetail::NearestCrossings<T>& a_state) noexcept -> T { return a_state.horizon(); };

  bvh.traverse(BVH::RayQuery<T>(a_ray.m_origin, a_ray.m_direction, a_tMin), crossings, leaf, bound);

  crossings.toSpans(
    a_tMin, crossings.m_count == 0U && RayDetail::insideUncrossed(a_shape, a_ray, a_tMin, a_tMax, a_options), a_spans);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
RayForm<FlatMeshSDF<T>>::spans(const FlatMeshSDF<T>& a_shape,
                               const RayT<T>&        a_ray,
                               const T               a_tMin,
                               const T               a_tMax,
                               const RayOptions<T>&  a_options,
                               const bool            a_firstOnly,
                               RaySpansT<T>&         a_spans) noexcept
{
  if (a_firstOnly) {
    RayDetail::firstSpan(a_shape, a_ray, a_tMin, a_tMax, a_options, a_spans);

    return;
  }

  RayDetail::NearestCrossings<T> crossings;

  crossings.m_tMax = a_tMax;

  const auto mesh = a_shape.getMesh();

  for (uint32_t i = 0; i < mesh.numFaces(); i++) {
    const auto& f = mesh.getFace(i);
    const T     t = f.rayCrossing(a_ray.m_origin, a_ray.m_direction, a_tMin, crossings.horizon(), mesh);

    if (t < Math::Limits<T>::infinity()) {
      crossings.insert(t, dot(a_ray.m_direction, f.getNormal()) < T(0));
    }
  }

  crossings.toSpans(
    a_tMin, crossings.m_count == 0U && RayDetail::insideUncrossed(a_shape, a_ray, a_tMin, a_tMax, a_options), a_spans);
}

} // namespace EBGeometry

#endif
