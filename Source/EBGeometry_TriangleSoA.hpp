// SPDX-FileCopyrightText: 2024 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_TriangleSoA.hpp
 * @brief  Declaration of SoA triangle group for SIMD signed-distance evaluation.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_TRIANGLESOA_HPP
#define EBGEOMETRY_TRIANGLESOA_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <type_traits>

// Our includes
#include "EBGeometry_Array.hpp"
#include "EBGeometry_SIMD.hpp"
#include "EBGeometry_Triangle.hpp"
#include "EBGeometry_TriangleDistance.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Namespace for TriangleSoAT-related compile-time utilities.
 */
namespace TriangleSoA {

/**
 * @brief The SoA width W (triangles per TriangleSoAT group) that fills one SIMD register for type T under the compiler's SIMD flags.
 * @details Opt-in, for host-only code:
 *
 * | ISA       | T=float | T=double |
 * |-----------|---------|----------|
 * | AVX-512F  |   16    |    8     |
 * | AVX       |    8    |    4     |
 * | otherwise |    4    |    4     |
 *
 * The value depends on the flags each translation unit is compiled with, so a type spelled with it
 * can mean different types in different files, and different layouts in the host and device passes
 * of one CUDA or HIP compile. Never use it for a type that is shared with device code or with another
 * translation unit built with different flags; use DefaultWidth<T>() there.
 * @tparam T Floating-point precision type (float or double).
 * @return W for T under the current ISA.
 */
template <typename T>
[[nodiscard]] constexpr size_t
HostWidth() noexcept
{
  static_assert(std::is_floating_point_v<T>, "EBGeometry::TriangleSoA::HostWidth requires a floating-point T");
#if defined(__AVX512F__)
  if constexpr (std::is_same_v<T, double>) {
    return 8;
  }
  else {
    return 16;
  }
#elif defined(__AVX__)
  if constexpr (std::is_same_v<T, double>) {
    return 4;
  }
  else {
    return 8;
  }
#else
  return 4;
#endif
}

/**
 * @brief The default SoA width W (triangles per TriangleSoAT group): 4, or HostWidth<T>() in a build
 * that defines EBGEOMETRY_HOST_TUNED_DEFAULTS.
 * @details This is the value the library's class templates default to.
 *
 * By default it is 4 for both float and double and never depends on compiler flags, so a type
 * spelled with it has the same layout in a host-only file, in a file compiled with AVX, and in both
 * passes of a CUDA or HIP compile -- which is what lets an object be built on the host and used on a
 * device.
 *
 * A host-only build can define EBGEOMETRY_HOST_TUNED_DEFAULTS to make it return HostWidth<T>()
 * instead, the width that fills one SIMD register under the compiler's flags (8 for float under AVX,
 * for example): a wider group evaluates more triangles per instruction, which measured up to about
 * 20% faster TriMeshSDF queries in float. The macro changes this width only; the branching factor and
 * the point-group width stay 4, since widening them measured no faster, and slower under AVX-512.
 * The macro must be defined for every translation unit of the build, all compiled with the same SIMD
 * flags, or the same spelled type has different layouts in different files. It is a compile error in
 * a CUDA or HIP translation unit. EBGeometry's own CMake build defines it when EBGeometry is the
 * top-level project and no GPU backend is enabled.
 *
 * Usage: `size_t W = TriangleSoA::DefaultWidth<T>()` as a template-parameter default.
 * @tparam T Floating-point precision type (float or double).
 * @return 4, or HostWidth<T>() under EBGEOMETRY_HOST_TUNED_DEFAULTS.
 */
template <typename T>
[[nodiscard]] constexpr size_t
DefaultWidth() noexcept
{
  static_assert(std::is_floating_point_v<T>, "EBGeometry::TriangleSoA::DefaultWidth requires a floating-point T");

#if defined(EBGEOMETRY_HOST_TUNED_DEFAULTS)
  return HostWidth<T>();
#else
  return 4;
#endif
}

} // namespace TriangleSoA

/**
 * @brief SoA (Structure of Arrays) layout for W triangles, enabling SIMD evaluation.
 * @details signedDistance() and signedDistances() evaluate TriangleDistance::signedDistance(), the
 * one triangle kernel, on SIMD::NativeWidth<T, W>() lanes at a time: the widest SSE4.1, AVX or
 * AVX-512F register the compiler's flags provide whose lane count divides W (so W = 8 doubles under
 * AVX takes two 4-lane registers, and W = 4 doubles under SSE4.1 two 2-lane ones). Where no register
 * fits, and in every device pass, the same kernel runs on one triangle at a time.
 * @warning This type is over-aligned (up to 64 bytes, for AVX-512F) via alignas. The library's own
 * usage is safe: PackedBVH stores groups in a Pool-backed PODVector<TriangleSoAT>, which reserves
 * each array at alignof(TriangleSoAT) inside a block aligned to PoolBaseAlign (256 bytes), and the
 * build-time std::vector staging relies on C++17's std::allocator respecting over-alignment. If you allocate a TriangleSoAT yourself outside of that path —
 * a raw `new`, a container with a custom/pre-C++17-style allocator, placement-new into
 * externally-owned storage, or a `malloc`'d buffer — you are responsible for ensuring the memory is
 * aligned to `alignof(TriangleSoAT<T, W>)`; nothing in this class enforces or checks that, and a
 * misaligned SIMD load will crash (or silently read the wrong data on some platforms) rather than
 * failing gracefully.
 * @tparam T  Floating-point precision.
 * @tparam W  Triangles per group. Any W works; a multiple of the register's lane count (see the
 * details) is evaluated with SIMD.
 */
template <class T, size_t W>
struct TriangleSoAT
{
  static_assert(W > 0, "W must be positive");
  static_assert(std::is_floating_point_v<T>, "TriangleSoAT requires a floating-point type T");

public:
  /**
   * @brief Pack count triangles from tris[0..count-1] into this SoA group.
   * @details Pads lanes count..W-1 by repeating the last real triangle so that all W
   * lanes hold valid data and SIMD loads never read uninitialised memory.
   * @param[in] tris  Source triangle array with at least count elements. Must not be null.
   * @param[in] count Number of valid triangles to pack. Must satisfy 1 <= count <= W.
   */
  EBGEOMETRY_HOST
  void
  pack(const Triangle<T>* tris, uint32_t count) noexcept;

  /**
   * @brief Evaluate signed distance from a_point to the closest triangle in this group.
   * @details Returns the signed distance with minimum absolute value among m_validCount triangles,
   * the first such lane on a tie. The lanes are evaluated SIMD::NativeWidth<T, W>() at a time (see
   * the class description). Requires the group to have already been packed via pack() (1 <= m_validCount <= W).
   * @param[in] a_point Query point. Must be finite.
   * @return Signed distance from a_point to the closest valid triangle, with sign determined by
   * the outward normal of the nearest feature (face, edge, or vertex).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Signed distances from a_point to every one of the W lane triangles.
   * @details The per-lane analogue of signedDistance(): signedDistance() horizontally reduces the W
   * per-triangle distances to the single minimum-|value| entry, whereas this returns all W of them so
   * a caller can recover *which* lane won -- the piece the reduction discards. Computed with the
   * same kernel and lane count as signedDistance(), so each lane is exactly the value
   * signedDistance() compared. All W lanes are filled: padded lanes (m_validCount..W-1) repeat the last
   * real triangle's distance, matching pack()'s padding, so a caller iterating lanes should stop at
   * m_validCount (or de-duplicate). Requires the group to have already been packed via pack()
   * (1 <= m_validCount <= W).
   * @param[in] a_point Query point. Must be finite.
   * @return Per-lane signed distances, one per W lanes.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  Array<T, W>
  signedDistances(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief The lane nearest a point and its squared distance, without a sign.
   * @details The first lane, of m_validCount, with the smallest TriangleDistance::unsignedDistance2(): the
   * lane signedDistance() would pick (except where two lanes are equally near to rounding), found without
   * signing every lane. A nearest-triangle search keeps
   * the best of these and signs only the winner, with signedDistanceAt().
   * @param[in]  a_point The point.
   * @param[out] a_d2    The lane's squared distance.
   * @return The lane.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  nearestLane(const Vec3T<T>& a_point, T& a_d2) const noexcept;

  /**
   * @brief One lane's signed distance: on the host, bit for bit the value signedDistances() gives that lane;
   * on a device, the same to rounding.
   * @param[in] a_lane  Lane, below W.
   * @param[in] a_point The point.
   * @return The signed distance.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistanceAt(uint32_t a_lane, const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Where a ray first crosses a triangle in this group.
   * @details TriangleRay::crossing() on the m_validCount triangles, SIMD::NativeWidth<T, W>() at a time,
   * from either side of each triangle. The nearest crossing in [a_tMin, a_tMax] wins, the first such lane
   * on a tie. Requires the group to have already been packed via pack() (1 <= m_validCount <= W).
   * @param[in]  a_origin    Ray origin.
   * @param[in]  a_direction Ray direction.
   * @param[in]  a_tMin      Start of the interval.
   * @param[in]  a_tMax      End of the interval.
   * @param[out] a_lane      Set to the lane of the crossing; unchanged if there is none.
   * @return The distance along the ray of the crossing, or +infinity if there is none.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  rayCrossing(
    const Vec3T<T>& a_origin, const Vec3T<T>& a_direction, T a_tMin, T a_tMax, uint32_t& a_lane) const noexcept;

  /**
   * @brief Where a ray crosses each triangle in this group.
   * @details The per-lane analogue of rayCrossing(): the same kernel and lane count, without the reduction
   * to the nearest, so that a caller can collect every crossing. Lanes m_validCount..W-1 are +infinity.
   * Requires the group to have already been packed via pack() (1 <= m_validCount <= W).
   * @param[in]  a_origin    Ray origin.
   * @param[in]  a_direction Ray direction.
   * @param[in]  a_tMin      Start of the interval.
   * @param[in]  a_tMax      End of the interval.
   * @param[out] a_crossings W values: each lane's crossing, or +infinity.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  rayCrossings(
    const Vec3T<T>& a_origin, const Vec3T<T>& a_direction, T a_tMin, T a_tMax, T* a_crossings) const noexcept;

  /**
   * @brief The unit normal of one lane's triangle.
   * @param[in] a_lane Lane index. Must satisfy 0 <= a_lane < W.
   * @return The face normal.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  Vec3T<T>
  getNormal(uint32_t a_lane) const noexcept;

  /**
   * @brief Compute the bounding volume enclosing all valid triangles in this group.
   * @details Requires the group to have already been packed via pack() (1 <= m_validCount <= W).
   * @tparam BV Bounding volume type (e.g. AABBT<T>); must be constructible from a point array (a
   * Vec3T<T> pointer and a count).
   * @return Bounding volume enclosing all vertices of the m_validCount triangles.
   */
  template <class BV>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BV
  computeBoundingVolume() const noexcept;

protected:
  /**
   * @brief Signed distance from a_point to the single triangle stored in lane a_lane.
   * @details TriangleDistance::signedDistance() on one lane: the path signedDistance() and
   * signedDistances() take where no SIMD register fits. Reads only lane a_lane's
   * coordinates/normals; performs no reduction.
   * @param[in] a_lane  Lane index. Must satisfy 0 <= a_lane < W.
   * @param[in] a_point Query point.
   * @return Signed distance from a_point to lane a_lane's triangle, sign from its nearest feature.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistanceLane(uint32_t a_lane, const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief The triangle accessor TriangleDistance::signedDistance() reads lanes [m_offset, m_offset +
   * lanes of V) through: V = T reads one lane, V = SIMD::Pack<T, N> loads N lanes (m_offset a multiple
   * of N, so the loads are aligned).
   * @tparam V Scalar or pack type.
   */
  template <class V>
  struct LaneSource
  {
    /**
     * @brief The group.
     */
    const TriangleSoAT* m_group;

    /**
     * @brief First lane.
     */
    size_t m_offset;

    /**
     * @brief One value per lane, from the array starting at @p a_lanes.
     * @param[in] a_lanes W values.
     * @return The value (or pack of values) at m_offset.
     */
    [[nodiscard]] EBGEOMETRY_HOST_DEVICE
    V
    fetch(const T* a_lanes) const noexcept
    {
      if constexpr (std::is_same_v<V, T>) {
        return a_lanes[m_offset];
      }
      else {
        return V::load(a_lanes + m_offset);
      }
    }

    /**
     * @brief Vertex position.
     * @param[in] a_k Vertex, 0 to 2.
     * @return Its lanes.
     */
    [[nodiscard]] EBGEOMETRY_HOST_DEVICE
    TriangleDistance::Vec3V<V>
    vertex(const size_t a_k) const noexcept
    {
      return {fetch(m_group->m_vx[a_k]), fetch(m_group->m_vy[a_k]), fetch(m_group->m_vz[a_k])};
    }

    /**
     * @brief Face normal.
     * @return Its lanes.
     */
    [[nodiscard]] EBGEOMETRY_HOST_DEVICE
    TriangleDistance::Vec3V<V>
    normal() const noexcept
    {
      return {fetch(m_group->m_nx), fetch(m_group->m_ny), fetch(m_group->m_nz)};
    }

    /**
     * @brief Vertex pseudonormal.
     * @param[in] a_k Vertex, 0 to 2.
     * @return Its lanes.
     */
    [[nodiscard]] EBGEOMETRY_HOST_DEVICE
    TriangleDistance::Vec3V<V>
    vertexNormal(const size_t a_k) const noexcept
    {
      return {fetch(m_group->m_vnx[a_k]), fetch(m_group->m_vny[a_k]), fetch(m_group->m_vnz[a_k])};
    }

    /**
     * @brief Edge pseudonormal.
     * @param[in] a_k Edge, 0 to 2.
     * @return Its lanes.
     */
    [[nodiscard]] EBGEOMETRY_HOST_DEVICE
    TriangleDistance::Vec3V<V>
    edgeNormal(const size_t a_k) const noexcept
    {
      return {fetch(m_group->m_enx[a_k]), fetch(m_group->m_eny[a_k]), fetch(m_group->m_enz[a_k])};
    }
  };

  /**
   * @brief Signed distances to the lanes below m_validCount, @p N at a time, with SIMD::Pack<T, N>.
   * @details Host-only: the packs exist only in a host pass. Lanes of a chunk that starts at or past
   * m_validCount are not written.
   * @tparam N Lane count, SIMD::NativeWidth<T, W>(); must be > 1 and divide W.
   * @param[in]  a_point     Query point.
   * @param[out] a_distances W values, aligned to N * sizeof(T).
   */
  template <size_t N>
  EBGEOMETRY_HOST
  void
  signedDistancesSIMD(const Vec3T<T>& a_point, T* a_distances) const noexcept;

  /**
   * @brief Every lane's squared unsigned distance with SIMD packs of N lanes, as signedDistancesSIMD().
   * @tparam N Lanes per pack.
   * @param[in]  a_point The point.
   * @param[out] a_d2    W squared distances (lanes at or past m_validCount unspecified).
   */
  template <size_t N>
  EBGEOMETRY_HOST
  void
  unsignedDistances2SIMD(const Vec3T<T>& a_point, T* a_d2) const noexcept;

  /**
   * @brief Ray crossings of the lanes below m_validCount, @p N at a time, with SIMD::Pack<T, N>.
   * @details Host-only, like signedDistancesSIMD(). Lanes of a chunk that starts at or past m_validCount
   * are not written.
   * @tparam N Lane count, SIMD::NativeWidth<T, W>(); must be > 1 and divide W.
   * @param[in]  a_origin    Ray origin.
   * @param[in]  a_direction Ray direction.
   * @param[in]  a_tMin      Start of the interval.
   * @param[in]  a_tMax      End of the interval.
   * @param[out] a_crossings W values, aligned to N * sizeof(T): the crossing, or +infinity.
   */
  template <size_t N>
  EBGEOMETRY_HOST
  void
  rayCrossingsSIMD(
    const Vec3T<T>& a_origin, const Vec3T<T>& a_direction, T a_tMin, T a_tMax, T* a_crossings) const noexcept;

  /**
   * @brief x-coordinates of vertex positions. m_vx[i][j] = x-coord of vertex i for triangle j.
   */
  alignas(W * sizeof(T)) T m_vx[3][W];

  /**
   * @brief y-coordinates of vertex positions. m_vy[i][j] = y-coord of vertex i for triangle j.
   */
  T m_vy[3][W];

  /**
   * @brief z-coordinates of vertex positions. m_vz[i][j] = z-coord of vertex i for triangle j.
   */
  T m_vz[3][W];

  /**
   * @brief x-components of face normals. m_nx[j] = x-component of the face normal for triangle j.
   */
  alignas(W * sizeof(T)) T m_nx[W];

  /**
   * @brief y-components of face normals.
   */
  T m_ny[W];

  /**
   * @brief z-components of face normals.
   */
  T m_nz[W];

  /**
   * @brief x-components of vertex normals. m_vnx[i][j] = x of vertex normal i for triangle j.
   */
  alignas(W * sizeof(T)) T m_vnx[3][W];

  /**
   * @brief y-components of vertex normals. m_vny[i][j] = y of vertex normal i for triangle j.
   */
  T m_vny[3][W];

  /**
   * @brief z-components of vertex normals. m_vnz[i][j] = z of vertex normal i for triangle j.
   */
  T m_vnz[3][W];

  /**
   * @brief x-components of edge normals. m_enx[i][j] = x of edge normal i for triangle j.
   */
  alignas(W * sizeof(T)) T m_enx[3][W];

  /**
   * @brief y-components of edge normals. m_eny[i][j] = y of edge normal i for triangle j.
   */
  T m_eny[3][W];

  /**
   * @brief z-components of edge normals. m_enz[i][j] = z of edge normal i for triangle j.
   */
  T m_enz[3][W];

  /**
   * @brief Number of valid (non-padded) triangles in this group (1..W).
   * @details Zero-initialized so that a default-constructed (not-yet-packed) group reliably fails
   * the EBGEOMETRY_EXPECT(m_validCount >= 1) precondition in signedDistance() and
   * computeBoundingVolume(), rather than reading whatever indeterminate value happened to be there.
   */
  uint32_t m_validCount = 0;
};

static_assert(std::is_trivially_copyable_v<TriangleSoAT<float, 4>>, "TriangleSoAT<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<TriangleSoAT<double, 4>>, "TriangleSoAT<double> must be trivially copyable");

} // namespace EBGeometry

#include "EBGeometry_TriangleSoAImplem.hpp"

#endif
