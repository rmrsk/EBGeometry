// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_MeshRayForms.hpp
 * @brief  The ray forms (see RayForm) of the mesh SDFs. TriMeshSDF: its BVH walked along the ray
 * (PackedBVH::traverse() with BVH::RayQuery), and its triangles crossed exactly (TriangleRay::crossing()).
 * MeshSDF: the same walk, its polygon faces crossed exactly (DCEL::FaceT::rayCrossing()). FlatMeshSDF:
 * every face, without a BVH. With this header included, firstHit() and intersects() on a mesh SDF are exact
 * instead of marched.
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
   * @brief TriMeshSDF has a ray form.
   */
  static constexpr bool available = true;

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
   * @brief MeshSDF has a ray form.
   */
  static constexpr bool available = true;

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
   * @brief FlatMeshSDF has a ray form.
   */
  static constexpr bool available = true;

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

      if (t <= a_state.m_t) {
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

      a_found = face.rayCrossing(a_ray.m_origin, a_ray.m_direction, a_tMin, a_tMax, mesh) <= a_tMax;
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

    if (t <= nearest) {
      nearest = t;
      face    = i;
    }
  }

  return face == UINT32_MAX ? HitT<T>() : RayDetail::faceHit(a_ray, nearest, face, mesh.getFace(face).getNormal());
}

} // namespace EBGeometry

#endif
