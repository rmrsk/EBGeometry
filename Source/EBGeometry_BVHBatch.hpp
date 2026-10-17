// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file    EBGeometry_BVHBatch.hpp
 * @brief   Many queries at once: a space-filling-curve query order, and PackedBVH::pruneTraverse() run
 *          over a batch with GPU::parallelFor().
 * @details The single-query pruneTraverse() is unchanged and stays the primitive. A batch runs it once
 *          per query, one thread per query on a device and one iteration per query on the host, in a
 *          caller-chosen order. The order is the cheap lever the GPU roadmap starts from: sorted along a
 *          Morton curve, consecutive queries are close together, so on a device a warp's threads descend
 *          the same branches of the tree (less divergence) and read the same nodes (fewer memory
 *          transactions), and on the host consecutive queries find the nodes and leaves they need still
 *          in cache. Queries that already arrive in grid order, cell by cell, are close to that order
 *          already; queries in random order gain the most.
 * @author  Robert Marskar
 */

#ifndef EBGEOMETRY_BVHBATCH_HPP
#define EBGEOMETRY_BVHBATCH_HPP

// Std includes
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_GPUTransform.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_PODVector.hpp"
#include "EBGeometry_PackedBVH.hpp"
#include "EBGeometry_SFC.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

namespace BVH {

/**
 * @brief The order of @p a_points along a Morton curve over their bounding box.
 * @details Each point is quantized to SFC::ValidBits bits per axis of its box and keyed by its Morton
 * code, ties broken by index, so the order is unique. Pass the result to pruneTraverseBatch() or to the
 * ordered GPU::transform(). Host-only: the points must be host memory, and the order is a host vector
 * the caller copies to wherever the batch runs.
 * @tparam T Precision.
 * @param[in] a_points The query points; every coordinate finite.
 * @return A permutation of 0, ..., size - 1: the indices of the points in curve order.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST
inline std::vector<uint32_t>
mortonOrder(PODSpan<const Vec3T<T>> a_points)
{
  const uint32_t n = a_points.size();

  Vec3T<T> lo = +Vec3T<T>::max();
  Vec3T<T> hi = -Vec3T<T>::max();

  for (uint32_t i = 0; i < n; i++) {
    lo = min(lo, a_points[i]);
    hi = max(hi, a_points[i]);
  }

  constexpr T span = static_cast<T>(SFC::ValidSpan);

  std::vector<std::pair<uint64_t, uint32_t>> keys(n);

  for (uint32_t i = 0; i < n; i++) {
    SFC::Index cell;

    // Halved before subtracting, so that a box wider than the largest value of T has a finite extent.
    for (int dir = 0; dir < 3; dir++) {
      const T extent = T(0.5) * hi[dir] - T(0.5) * lo[dir];
      const T scaled = (extent > T(0)) ? (T(0.5) * a_points[i][dir] - T(0.5) * lo[dir]) / extent * span : T(0);

      cell[dir] = static_cast<unsigned int>(Math::clamp(scaled, T(0), span));
    }

    keys[i] = {SFC::Morton::encode(cell), i};
  }

  std::sort(keys.begin(), keys.end());

  std::vector<uint32_t> order(n);

  for (uint32_t i = 0; i < n; i++) {
    order[i] = keys[i].second;
  }

  return order;
}

/**
 * @brief The functor pruneTraverseBatch() runs: one pruneTraverse() per query.
 * @details Internal; not part of the public API.
 * @tparam T                Precision.
 * @tparam Bvh              PackedBVH type.
 * @tparam State            Search state type.
 * @tparam LeafEvaluator    Leaf callback type.
 * @tparam PruneDistSquared Pruning-bound callback type.
 */
template <class T, class Bvh, class State, class LeafEvaluator, class PruneDistSquared>
struct PruneTraverseQuery
{
  /// @brief The BVH, a view the batch can read.
  Bvh m_bvh;

  /// @brief The query points.
  const Vec3T<T>* m_points;

  /// @brief One state per query.
  State* m_states;

  /// @brief The order, or null for index order.
  const uint32_t* m_order;

  /// @brief Leaf callback.
  LeafEvaluator m_evalLeaf;

  /// @brief Pruning-bound callback.
  PruneDistSquared m_pruneDist2;

  /**
   * @brief Run the i-th query of the order.
   * @param[in] a_i Position in the order.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_i) const noexcept
  {
    const uint32_t q = (m_order != nullptr) ? m_order[a_i] : static_cast<uint32_t>(a_i);

    m_bvh.pruneTraverse(m_points[q], m_states[q], m_evalLeaf, m_pruneDist2);
  }
};

/**
 * @brief Run PackedBVH::pruneTraverse() for every query of a batch: one thread per query in a CUDA or
 * HIP translation unit, one loop iteration per query otherwise.
 * @details Query q traverses with state a_states[q], exactly as a single pruneTraverse(a_points[q],
 * a_states[q], ...) would, so the states come out the same whatever the order; the order changes only
 * which queries run side by side. Each state is initialized by the caller, and carries whatever the
 * leaf callback needs to know about its query, the query point included: one callback serves the whole
 * batch, so unlike the lambda of a single pruneTraverse() it cannot capture a point. The callbacks must be
 * trivially copyable functors with EBGEOMETRY_HOST_DEVICE call operators (a lambda is not one on a
 * device), since a copy reaches every thread; and the BVH, the points, the states and the order must
 * be memory the batch can reach: on a device, a rebasedView() of a mirror and device-accessible arrays.
 * Aborts, in every build, if the states or a non-empty order do not match the point count.
 * @tparam BlockSize        Threads per block; see GPU::parallelFor().
 * @tparam T                Precision.
 * @tparam P                Primitive type.
 * @tparam K                Branching factor.
 * @tparam State            Search state type.
 * @tparam LeafEvaluator    Leaf callback: (State&, size_t offset, size_t count) noexcept.
 * @tparam PruneDistSquared Pruning bound: (const State&) noexcept -> T.
 * @param[in]     a_bvh        The BVH.
 * @param[in]     a_points     The query points.
 * @param[in,out] a_states     One initialized state per query, updated in place.
 * @param[in]     a_evalLeaf   Leaf callback.
 * @param[in]     a_pruneDist2 Pruning-bound callback.
 * @param[in]     a_order      The order to run the queries in (mortonOrder(), say), or empty for index
 *                             order.
 */
template <unsigned int BlockSize = GPU::DefaultBlockSize,
          class T,
          class P,
          size_t K,
          class State,
          class LeafEvaluator,
          class PruneDistSquared>
EBGEOMETRY_HOST
inline void
pruneTraverseBatch(const PackedBVH<T, P, K>& a_bvh,
                   PODSpan<const Vec3T<T>>   a_points,
                   PODSpan<State>            a_states,
                   const LeafEvaluator&      a_evalLeaf,
                   const PruneDistSquared&   a_pruneDist2,
                   PODSpan<const uint32_t>   a_order = PODSpan<const uint32_t>{})
{
  static_assert(std::is_trivially_copyable_v<State>, "BVH::pruneTraverseBatch: the state must be trivially copyable");
  static_assert(std::is_trivially_copyable_v<LeafEvaluator>,
                "BVH::pruneTraverseBatch: the leaf callback must be trivially copyable");
  static_assert(std::is_trivially_copyable_v<PruneDistSquared>,
                "BVH::pruneTraverseBatch: the pruning-bound callback must be trivially copyable");

  EBGEOMETRY_REQUIRE(a_states.size() == a_points.size() && (a_order.size() == 0 || a_order.size() == a_points.size()),
                     "BVH::pruneTraverseBatch: %u points need as many states (%u) and order entries (%u, or none)",
                     unsigned(a_points.size()),
                     unsigned(a_states.size()),
                     unsigned(a_order.size()));

  using Query = PruneTraverseQuery<T, PackedBVH<T, P, K>, State, LeafEvaluator, PruneDistSquared>;

  const uint32_t* order = (a_order.size() > 0) ? a_order.m_ptr : nullptr;

  GPU::parallelFor<BlockSize>(a_points.size(),
                              Query{a_bvh, a_points.m_ptr, a_states.m_ptr, order, a_evalLeaf, a_pruneDist2});
}

} // namespace BVH

} // namespace EBGeometry

#endif
