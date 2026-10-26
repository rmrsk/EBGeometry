// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
 * TriMeshSDF::getClosestFace()) and HitT::m_normal is the triangle's normal.
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
 * HitT::m_normal its normal. The faces are taken to be planar and convex (see DCEL::FaceT::rayCrossing()).
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

  const auto leaf = [&groups, &a_ray, a_tMin](Nearest& a_state, size_t a_offset, size_t a_count) noexcept {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      uint32_t face   = UINT32_MAX;
      Vec3T<T> normal = Vec3T<T>::zeros();

      const T t = groups[static_cast<uint32_t>(i)].rayCrossing(
        a_ray.m_origin, a_ray.m_direction, a_tMin, a_state.m_t, face, normal);

      if (face != UINT32_MAX && t <= a_state.m_t) {
        a_state = Nearest{t, face, normal};
      }
    }
  };

  const auto bound = [](const Nearest& a_state) noexcept -> T { return a_state.m_t; };

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

  const auto leaf = [&faceIds, &mesh, &a_ray, a_tMin](Nearest& a_state, size_t a_offset, size_t a_count) noexcept {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      const uint32_t face = faceIds[static_cast<uint32_t>(i)];
      const T        t = mesh.getFace(face).rayCrossing(a_ray.m_origin, a_ray.m_direction, a_tMin, a_state.m_t, mesh);

      // A crossing (finite: none is +infinity, however far the interval goes) is within the nearest so far.
      if (t < Math::Limits<T>::infinity()) {
        a_state = Nearest{t, face};
      }
    }
  };

  const auto bound = [](const Nearest& a_state) noexcept -> T { return a_state.m_t; };

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
    const T t = mesh.getFace(i).rayCrossing(a_ray.m_origin, a_ray.m_direction, a_tMin, nearest, mesh);

    if (t < Math::Limits<T>::infinity()) {
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

  crossings.toSpans(a_tMin, crossings.m_count == 0U && RayDetail::insideAt(a_shape, a_ray.at(a_tMin)), a_spans);
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

  crossings.toSpans(a_tMin, crossings.m_count == 0U && RayDetail::insideAt(a_shape, a_ray.at(a_tMin)), a_spans);
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

  crossings.toSpans(a_tMin, crossings.m_count == 0U && RayDetail::insideAt(a_shape, a_ray.at(a_tMin)), a_spans);
}

} // namespace EBGeometry

#endif
