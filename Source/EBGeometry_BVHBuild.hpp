// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_BVHBuild.hpp
 * @brief  Host-side BVH construction: TreeBVH, the partitioners, and the PackedBVH constructors
 * that partition a primitive list.
 * @details Includes EBGeometry_PackedBVH.hpp. Code that only queries a PackedBVH, device code in
 * particular, can include that header alone.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_BVHBUILD_HPP
#define EBGEOMETRY_BVHBUILD_HPP

// Std includes
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_Array.hpp"
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_PackedBVH.hpp"
#include "EBGeometry_SFC.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

namespace BVH {

/**
 * @brief Build-phase helpers shared by PackedBVH's constructors. Not part of the public API.
 */
namespace detail {

/**
 * @brief Append one TreeBVH leaf's primitives to a flat primitive array under construction.
 * @details Used by PackedBVH's identity/converting pack() constructors and its partitioner
 * constructor, all of which route through a TreeBVH whose leaves hold shared_ptr<const P>; this
 * dereferences and copies each one by value. Appends to whatever @p a_dst already holds, so the
 * contract is the same no matter how many leaves are packed.
 * @tparam P Primitive type.
 * @param[in,out] a_dst       Flat primitive array under construction.
 * @param[in]     a_leafPrims One TreeBVH leaf's primitive list.
 */
template <class P>
EBGEOMETRY_HOST
inline void
appendTreeLeaf(std::vector<P>& a_dst, const PrimitiveList<P>& a_leafPrims)
{
  // NB: do NOT reserve(a_dst.size() + a_leafPrims.size()) here. appendTreeLeaf is called once per
  // leaf as the whole tree is packed, and reserving to an each-time-slightly-larger *exact* size
  // defeats std::vector's geometric growth -- it forces a fresh reallocation (copying every
  // element already appended) on every leaf, making a whole build O(N^2) in the primitive count.
  // Plain push_back grows the buffer geometrically and keeps construction linear.
  for (const auto& p : a_leafPrims) {
    a_dst.push_back(*p);
  }
}

/**
 * @brief Abort unless every partition a partitioner returned holds at least one primitive.
 * @details An empty partition would become a leaf with no primitives, which the packed layout
 * cannot represent: a node with no primitives reads as an interior node. The built-in partitioners
 * never return one; a caller-supplied partitioner might, and this catches it at build time, in
 * every build, where it is cheap and the message can say which call produced it.
 * @tparam List Partition type (a list of primitives with their bounding volumes).
 * @tparam K    Number of partitions.
 * @param[in] a_partitions Partitions returned by the partitioner.
 * @param[in] a_numInput   Number of primitives the partitioner was given.
 * @param[in] a_who        Name of the calling builder, for the message.
 */
template <class List, size_t K>
EBGEOMETRY_HOST
inline void
requireNonEmptyPartitions(const Array<List, K>& a_partitions, const size_t a_numInput, const char* a_who)
{
  for (size_t k = 0; k < K; k++) {
    EBGEOMETRY_REQUIRE(!a_partitions[k].empty(),
                       "%s: the partitioner returned an empty partition (%zu of %zu, from %zu primitives). A leaf "
                       "must hold at least one primitive; a partitioner must return %zu non-empty partitions.",
                       a_who,
                       k,
                       K,
                       a_numInput,
                       K);
  }
}

/**
 * @brief Materialise a single contiguous buffer into a flat primitive array under construction.
 * @details *Appends* @p a_block to @p a_dst, so the contract holds regardless of how many times
 * it is called. When @p a_dst is still empty -- the case for PackedBVH's single-call direct,
 * converting and ClusterSpec constructors -- it takes the fast path of stealing the
 * already-contiguous buffer wholesale, with no per-element move; otherwise it move-appends each
 * element.
 * @tparam P Primitive type.
 * @param[in,out] a_dst   Flat primitive array under construction.
 * @param[in]     a_block Single contiguous buffer holding every element to append.
 */
template <class P>
EBGEOMETRY_HOST
inline void
appendAliased(std::vector<P>& a_dst, const std::shared_ptr<std::vector<P>>& a_block)
{
  if (a_dst.empty()) {
    a_dst = std::move(*a_block);
  }
  else {
    a_dst.insert(a_dst.end(), std::make_move_iterator(a_block->begin()), std::make_move_iterator(a_block->end()));
  }
}

} // namespace detail

/**
 * @brief Utility: split a vector into K almost-equal contiguous chunks.
 * @tparam X Element type.
 * @tparam K Number of chunks.
 * @param[in] a_primitives Input vector.
 * @return Array of K sub-vectors whose sizes differ by at most 1.
 */
template <class X, size_t K>
auto EqualCounts = [](std::vector<X> a_primitives) noexcept -> Array<std::vector<X>, K> {
  static_assert(K >= 2, "EqualCounts<X, K>: branching factor K must be at least 2");

  EBGEOMETRY_EXPECT(!a_primitives.empty());

  const int length = a_primitives.size() / K;
  int       remain = a_primitives.size() % K;

  int begin = 0;
  int end   = 0;

  Array<std::vector<X>, K> chunks;

  for (size_t k = 0; k < K; k++) {
    end += (remain > 0) ? length + 1 : length;
    remain--;

    // Move the [begin, end) slice out -- the input is taken by value and not reused, so the elements
    // (e.g. shared_ptr primitive handles) transfer without copying or refcount churn.
    chunks[k] = std::vector<X>(std::make_move_iterator(a_primitives.begin() + begin),
                               std::make_move_iterator(a_primitives.begin() + end));

    begin = end;
  }

  return chunks;
};

/**
 * @brief Partitioner that sorts primitives by centroid along the longest axis and splits into K pieces.
 * @tparam T  Floating-point precision used for centroid comparisons.
 * @tparam P  Primitive type.
 * @tparam BV Bounding volume type.
 * @tparam K  Number of output sub-lists (tree branching factor).
 * @param[in] a_primsAndBVs Input (primitive, BV) pairs.
 * @return K sub-lists.
 */
template <class T, class P, class BV, size_t K>
auto PrimitiveCentroidPartitioner = [](PrimAndBVList<P, BV> a_primsAndBVs) noexcept -> Array<PrimAndBVList<P, BV>, K> {
  EBGEOMETRY_EXPECT(!a_primsAndBVs.empty());

  Vec3T<T> lo = +Vec3T<T>::max();
  Vec3T<T> hi = -Vec3T<T>::max();

  for (const auto& pbv : a_primsAndBVs) {
    lo = min(lo, pbv.first->getCentroid());
    hi = max(hi, pbv.first->getCentroid());
  }

  const size_t splitDir = (hi - lo).maxDir(true);

  // The input is taken by value; sort it in place (no working copy) and move it into EqualCounts.
  std::sort(a_primsAndBVs.begin(),
            a_primsAndBVs.end(),
            [splitDir](const PrimAndBV<P, BV>& pbv1, const PrimAndBV<P, BV>& pbv2) -> bool {
              return pbv1.first->getCentroid(splitDir) < pbv2.first->getCentroid(splitDir);
            });

  return BVH::EqualCounts<PrimAndBV<P, BV>, K>(std::move(a_primsAndBVs));
};

/**
 * @brief Partitioner that sorts primitives by bounding-volume centroid along the longest axis and splits into K pieces.
 * @tparam T  Floating-point precision used for centroid comparisons.
 * @tparam P  Primitive type.
 * @tparam BV Bounding volume type.
 * @tparam K  Number of output sub-lists (tree branching factor).
 * @param[in] a_primsAndBVs Input (primitive, BV) pairs.
 * @return K sub-lists.
 */
template <class T, class P, class BV, size_t K>
auto BVCentroidPartitioner = [](PrimAndBVList<P, BV> a_primsAndBVs) -> Array<PrimAndBVList<P, BV>, K> {
  EBGEOMETRY_EXPECT(!a_primsAndBVs.empty());

  Vec3T<T> lo = +Vec3T<T>::max();
  Vec3T<T> hi = -Vec3T<T>::max();

  for (const auto& pbv : a_primsAndBVs) {
    lo = min(lo, pbv.second.getCentroid());
    hi = max(hi, pbv.second.getCentroid());
  }

  const size_t splitDir = (hi - lo).maxDir(true);

  // The input is taken by value; sort it in place (no working copy) and move it into EqualCounts.
  std::sort(a_primsAndBVs.begin(),
            a_primsAndBVs.end(),
            [splitDir](const PrimAndBV<P, BV>& pbv1, const PrimAndBV<P, BV>& pbv2) -> bool {
              return pbv1.second.getCentroid()[splitDir] < pbv2.second.getCentroid()[splitDir];
            });

  return BVH::EqualCounts<PrimAndBV<P, BV>, K>(std::move(a_primsAndBVs));
};

/**
 * @brief Internal helper: 2-way binned SAH split on the sub-range [a_begin, a_end).
 * @details Evaluates 32 bins per axis and picks the split plane that minimises
 * @c SA(left)*N_left + SA(right)*N_right.  Partitions @p a_list in-place and returns
 * the split index (first element of the right group).
 * @note Requires BV == AABBT<T>: BV must support getLowCorner(), getHighCorner(),
 * getArea(), and construction from two Vec3T<T> corner arguments.
 * @tparam T  Floating-point precision.
 * @tparam P  Primitive type.
 * @tparam BV Bounding-volume type (must be AABBT<T>).
 * @param[in,out] a_list Primitives and their bounding volumes; partitioned in place around the split.
 * @param[in] a_begin First index of the sub-range to split.
 * @param[in] a_end One-past-the-last index of the sub-range to split.
 * @param[in] a_longestAxisOnly If true, evaluate candidate planes on only the longest centroid-bbox
 * axis instead of all three -- roughly a third of the binning work, for a small tree-quality cost
 * that is negligible on near-uniform inputs (e.g. point clouds). Default false (full 3-axis SAH).
 * @return Split index (index of the first element of the right group).
 */
template <class T, class P, class BV>
inline size_t
SAH2WaySplit(PrimAndBVList<P, BV>& a_list,
             const size_t          a_begin,
             const size_t          a_end,
             const bool            a_longestAxisOnly = false) noexcept
{
  static_assert(std::is_same_v<BV, EBGeometry::BoundingVolumes::AABBT<T>>, "SAH2WaySplit requires BV == AABBT<T>");

  EBGEOMETRY_EXPECT(a_begin < a_end);

  constexpr int BINS = 32;

  const size_t N = a_end - a_begin;

  // Centroid bounding box
  Vec3T<T> clo = +Vec3T<T>::max();
  Vec3T<T> chi = -Vec3T<T>::max();

  for (size_t i = a_begin; i < a_end; i++) {
    const auto& c = a_list[i].second.getCentroid();

    clo = min(clo, c);
    chi = max(chi, c);
  }

  T   bestCost  = Math::Limits<T>::max();
  T   bestPlane = T(0);
  int bestAxis  = -1;

  Vec3T<T> binLo[BINS];
  Vec3T<T> binHi[BINS];

  int binCnt[BINS];

  T   leftArea[BINS - 1];
  int leftCnt[BINS - 1];

  const int firstAxis = a_longestAxisOnly ? (chi - clo).maxDir(true) : 0;
  const int lastAxis  = a_longestAxisOnly ? firstAxis : 2;

  for (int axis = firstAxis; axis <= lastAxis; axis++) {
    const T lo  = clo[axis];
    const T hi  = chi[axis];
    const T ext = hi - lo;

    // An extent so small that BINS / ext overflows cannot be binned; leave that axis out.
    const T scale = T(BINS) / ext;

    if (!(ext > T(0)) || !std::isfinite(scale)) {
      continue;
    }

    for (int b = 0; b < BINS; b++) {
      binLo[b]  = Vec3T<T>::max();
      binHi[b]  = -Vec3T<T>::max();
      binCnt[b] = 0;
    }

    for (size_t i = a_begin; i < a_end; i++) {
      // Clamped in floating point before the conversion, which is undefined past int's range.
      const T   x = (a_list[i].second.getCentroid()[axis] - lo) * scale;
      const int b = (x > T(0)) ? ((x < T(BINS - 1)) ? static_cast<int>(x) : BINS - 1) : 0;
      binLo[b]    = min(binLo[b], a_list[i].second.getLowCorner());
      binHi[b]    = max(binHi[b], a_list[i].second.getHighCorner());
      binCnt[b]   = binCnt[b] + 1;
    }

    // Left prefix: accumulated AABB and count for bins [0..b]
    Vec3T<T> rlo  = Vec3T<T>::max();
    Vec3T<T> rhi  = -Vec3T<T>::max();
    int      rcnt = 0;

    for (int b = 0; b < BINS - 1; b++) {
      if (binCnt[b] > 0) {
        rlo = min(rlo, binLo[b]);
        rhi = max(rhi, binHi[b]);
      }

      rcnt        = rcnt + binCnt[b];
      leftArea[b] = (rcnt > 0) ? BV(rlo, rhi).getArea() : T(0);
      leftCnt[b]  = rcnt;
    }

    // Right suffix sweep; evaluate SAH cost at each candidate split boundary
    Vec3T<T> rrlo  = Vec3T<T>::max();
    Vec3T<T> rrhi  = -Vec3T<T>::max();
    int      rrcnt = 0;

    for (int b = BINS - 1; b >= 1; b--) {
      if (binCnt[b] > 0) {
        rrlo = min(rrlo, binLo[b]);
        rrhi = max(rrhi, binHi[b]);
      }

      rrcnt += binCnt[b];

      if (leftCnt[b - 1] > 0 && rrcnt > 0) {
        const T cost = leftArea[b - 1] * T(leftCnt[b - 1]) + BV(rrlo, rrhi).getArea() * T(rrcnt);

        if (cost < bestCost) {
          bestCost  = cost;
          bestAxis  = axis;
          bestPlane = lo + T(b) / scale;
        }
      }
    }
  }

  // All axes degenerate — equal split
  if (bestAxis < 0) {
    return a_begin + N / 2;
  }

  auto mid = std::partition(
    a_list.begin() + a_begin, a_list.begin() + a_end, [bestAxis, bestPlane](const PrimAndBV<P, BV>& pbv) noexcept {
      return pbv.second.getCentroid()[bestAxis] < bestPlane;
    });

  const size_t splitIdx = static_cast<size_t>(std::distance(a_list.begin(), mid));

  // Guard: if everything ended up on one side, fall back to equal split
  return (splitIdx == a_begin || splitIdx == a_end) ? a_begin + N / 2 : splitIdx;
}

/**
 * @brief Internal helper: recursively split [a_begin, a_end) into a_K groups via 2-way SAH.
 * @details Splits into std::floor(a_K/2) and std::ceil(a_K/2) sub-groups recursively.  For power-of-two
 * K this is equivalent to a balanced binary subdivision tree applied a_K times.
 * @tparam T  Floating-point precision.
 * @tparam P  Primitive type.
 * @tparam BV Bounding-volume type (must be AABBT<T>).
 * @param[in,out] a_list Primitives and their bounding volumes; partitioned in place.
 * @param[in] a_begin First index of the sub-range to split.
 * @param[in] a_end One-past-the-last index of the sub-range to split.
 * @param[in] a_K Number of groups to split the sub-range into.
 * @param[out] a_groups Resulting groups as [begin, end) index pairs.
 * @param[in] a_longestAxisOnly Forwarded to SAH2WaySplit: bin only the longest centroid-bbox axis.
 */
template <class T, class P, class BV>
inline void
SAHKWaySplit(PrimAndBVList<P, BV>&                   a_list,
             const size_t                            a_begin,
             const size_t                            a_end,
             const size_t                            a_K,
             std::vector<std::pair<size_t, size_t>>& a_groups,
             const bool                              a_longestAxisOnly = false) noexcept
{
  static_assert(std::is_same_v<BV, EBGeometry::BoundingVolumes::AABBT<T>>, "SAHKWaySplit requires BV == AABBT<T>");

  if (a_K <= 1 || a_begin >= a_end) {
    a_groups.emplace_back(a_begin, a_end);

    return;
  }

  const size_t K1 = a_K / 2;
  const size_t K2 = a_K - K1;

  // Clamp the SAH split so the left half has >= K1 elements and the right has >= K2.
  // This guarantees neither recursive call receives an under-populated range, which
  // would cause empty sub-lists to reach the TreeBVH constructor and crash on
  // AABBT(vector::front()) when the vector is empty.
  // The clamp is valid whenever a_end - a_begin >= a_K = K1 + K2.
  const size_t rawMid = SAH2WaySplit<T, P, BV>(a_list, a_begin, a_end, a_longestAxisOnly);
  const size_t mid    = Math::max(a_begin + K1, Math::min(a_end - K2, rawMid));

  SAHKWaySplit<T, P, BV>(a_list, a_begin, mid, K1, a_groups, a_longestAxisOnly);
  SAHKWaySplit<T, P, BV>(a_list, mid, a_end, K2, a_groups, a_longestAxisOnly);
}

/**
 * @brief Partitioner using binned SAH with recursive 2-way subdivision into K groups.
 * @details For each split, evaluates 32 candidate planes per axis and picks the one
 * minimising @c SA(left)*N_left + SA(right)*N_right (the standard ray-tracing SAH
 * cost without the traversal constant).  K groups are produced by recursively splitting
 * into std::floor(K/2) and std::ceil(K/2) subsets — exact for power-of-two K; a reasonable
 * approximation for other values.
 *
 * Recommended K values by ISA:
 * - AVX-512F, float  → K=16 (one @c _mm512_load_ps covers all K children)
 * - AVX-512F, double → K=8  (one @c _mm512_load_pd covers all K children)
 * - AVX,      float  → K=8  (one @c _mm256_load_ps)
 * - AVX,      double → K=4  (one @c _mm256_load_pd)
 * - SSE4.1,   float  → K=4  (one @c _mm_load_ps)
 *
 * @note Requires BV == AABBT<T>.
 * @tparam T  Floating-point precision.
 * @tparam P  Primitive type.
 * @tparam BV Bounding-volume type (must be AABBT<T>).
 * @tparam K  Number of output sub-lists (branching factor of the resulting BVH).
 * @tparam LongestAxisOnly If true, bin candidate planes on only the longest centroid-bbox axis per
 * split instead of all three -- ~a third of the binning work (measured ~20% faster builds on point
 * clouds) for a tree-quality cost that is negligible on near-uniform inputs. Default false.
 * @param[in] a_primsAndBVs Input (primitive, BV) pairs.
 * @return K sub-lists.
 */
template <class T, class P, class BV, size_t K, bool LongestAxisOnly = false>
auto BinnedSAHPartitioner = [](PrimAndBVList<P, BV> a_primsAndBVs) -> Array<PrimAndBVList<P, BV>, K> {
  EBGEOMETRY_EXPECT(!a_primsAndBVs.empty());

  // The input is taken by value; partition it in place (no working copy). SAHKWaySplit reorders it
  // and yields K [begin, end) index ranges, which we move out (disjoint, each moved once).
  std::vector<std::pair<size_t, size_t>> groups;
  groups.reserve(K);

  SAHKWaySplit<T, P, BV>(a_primsAndBVs, 0, a_primsAndBVs.size(), K, groups, LongestAxisOnly);

  Array<PrimAndBVList<P, BV>, K> result;
  for (size_t k = 0; k < K; k++) {
    const auto [b, e] = groups[k];
    result[k]         = PrimAndBVList<P, BV>(std::make_move_iterator(a_primsAndBVs.begin() + b),
                                     std::make_move_iterator(a_primsAndBVs.begin() + e));
  }

  return result;
};

/**
 * @brief Internal helper: 2-way spatial-midpoint split on the sub-range [a_begin, a_end).
 * @details Unlike SAH2WaySplit (which evaluates 32 binned candidate planes) or
 * BVCentroidPartitioner (which sorts by centroid), this computes exactly one split plane -- the
 * midpoint of the centroid bounding box along its longest axis -- and partitions around it with a
 * single std::partition pass. No sorting and no per-plane cost evaluation, at the cost of not
 * adapting to the primitive distribution the way SAH does: entirely sort-less, O(N) per split.
 * @tparam T  Floating-point precision.
 * @tparam P  Primitive type.
 * @tparam BV Bounding-volume type.
 * @param[in,out] a_list Primitives and their bounding volumes; partitioned in place around the split.
 * @param[in] a_begin First index of the sub-range to split.
 * @param[in] a_end One-past-the-last index of the sub-range to split.
 * @return Split index (index of the first element of the right group).
 */
template <class T, class P, class BV>
inline size_t
Midpoint2WaySplit(PrimAndBVList<P, BV>& a_list, const size_t a_begin, const size_t a_end) noexcept
{
  EBGEOMETRY_EXPECT(a_begin < a_end);

  const size_t N = a_end - a_begin;

  Vec3T<T> lo = +Vec3T<T>::max();
  Vec3T<T> hi = -Vec3T<T>::max();

  for (size_t i = a_begin; i < a_end; i++) {
    const auto& c = a_list[i].second.getCentroid();

    lo = min(lo, c);
    hi = max(hi, c);
  }

  const size_t splitDir = (hi - lo).maxDir(true);
  const T      midpoint = T(0.5) * (lo[splitDir] + hi[splitDir]);

  auto mid = std::partition(
    a_list.begin() + a_begin, a_list.begin() + a_end, [splitDir, midpoint](const PrimAndBV<P, BV>& pbv) noexcept {
      return pbv.second.getCentroid()[splitDir] < midpoint;
    });

  const size_t splitIdx = static_cast<size_t>(std::distance(a_list.begin(), mid));

  // Guard: if every centroid ended up on one side (e.g. all coincide on the split axis, or the
  // distribution is skewed entirely to one side of the midpoint), fall back to an equal-count
  // split so neither resulting group is ever empty.
  return (splitIdx == a_begin || splitIdx == a_end) ? a_begin + N / 2 : splitIdx;
}

/**
 * @brief Internal helper: recursively split [a_begin, a_end) into a_K groups via 2-way midpoint
 * splits.
 * @details Splits into std::floor(a_K/2) and std::ceil(a_K/2) sub-groups recursively -- exact for
 * power-of-two K; a reasonable approximation for other values. Structurally identical to
 * SAHKWaySplit, just calling Midpoint2WaySplit instead of SAH2WaySplit at each level.
 * @tparam T  Floating-point precision.
 * @tparam P  Primitive type.
 * @tparam BV Bounding-volume type.
 * @param[in,out] a_list Primitives and their bounding volumes; partitioned in place.
 * @param[in] a_begin First index of the sub-range to split.
 * @param[in] a_end One-past-the-last index of the sub-range to split.
 * @param[in] a_K Number of groups to split the sub-range into.
 * @param[out] a_groups Resulting groups as [begin, end) index pairs.
 */
template <class T, class P, class BV>
inline void
MidpointKWaySplit(PrimAndBVList<P, BV>&                   a_list,
                  const size_t                            a_begin,
                  const size_t                            a_end,
                  const size_t                            a_K,
                  std::vector<std::pair<size_t, size_t>>& a_groups) noexcept
{
  if (a_K <= 1 || a_begin >= a_end) {
    a_groups.emplace_back(a_begin, a_end);

    return;
  }

  const size_t K1 = a_K / 2;
  const size_t K2 = a_K - K1;

  // Clamp so the left half has >= K1 elements and the right has >= K2, guaranteeing neither
  // recursive call receives an under-populated range (see SAHKWaySplit's identical clamp for why:
  // an empty sub-list reaching the TreeBVH constructor crashes on AABBT(vector::front())).
  const size_t rawMid = Midpoint2WaySplit<T, P, BV>(a_list, a_begin, a_end);
  const size_t mid    = Math::max(a_begin + K1, Math::min(a_end - K2, rawMid));

  MidpointKWaySplit<T, P, BV>(a_list, a_begin, mid, K1, a_groups);
  MidpointKWaySplit<T, P, BV>(a_list, mid, a_end, K2, a_groups);
}

/**
 * @brief Partitioner that recursively bisects primitives by spatial midpoint (no sorting).
 * @details At every split, computes the midpoint of the centroid bounding box along its longest
 * axis and partitions primitives around it in one O(N) std::partition pass -- unlike
 * BVCentroidPartitioner (sorts by centroid) or BinnedSAHPartitioner (evaluates 32 candidate
 * planes per axis), no sorting or per-plane cost evaluation happens anywhere. K groups are
 * produced by recursively splitting into std::floor(K/2) and std::ceil(K/2) subsets, mirroring
 * BinnedSAHPartitioner's own K-way structure exactly (see MidpointKWaySplit).
 *
 * This is the fastest of the three top-down partitioners to build (no sort, no per-axis binning),
 * at the cost of build quality: it does not adapt to the primitive distribution at all, so a
 * heavily clustered or skewed input can produce noticeably less balanced (and less
 * query-efficient) trees than BVCentroidPartitioner or BinnedSAHPartitioner would.
 *
 * @tparam T  Floating-point precision.
 * @tparam P  Primitive type.
 * @tparam BV Bounding-volume type.
 * @tparam K  Number of output sub-lists (branching factor of the resulting BVH).
 * @param[in] a_primsAndBVs Input (primitive, BV) pairs.
 * @return K sub-lists.
 */
template <class T, class P, class BV, size_t K>
auto MidpointPartitioner = [](PrimAndBVList<P, BV> a_primsAndBVs) -> Array<PrimAndBVList<P, BV>, K> {
  EBGEOMETRY_EXPECT(!a_primsAndBVs.empty());

  // The input is taken by value; partition it in place (no working copy). MidpointKWaySplit reorders
  // it and yields K [begin, end) index ranges, which we move out (disjoint, each moved once). This
  // is the crux of the partitioner's cost: the midpoint split itself is a couple of std::partition
  // passes, so the plumbing (copies) is what dominated before -- now eliminated.
  std::vector<std::pair<size_t, size_t>> groups;
  groups.reserve(K);

  MidpointKWaySplit<T, P, BV>(a_primsAndBVs, 0, a_primsAndBVs.size(), K, groups);

  Array<PrimAndBVList<P, BV>, K> result;
  for (size_t k = 0; k < K; k++) {
    const auto [b, e] = groups[k];
    result[k]         = PrimAndBVList<P, BV>(std::make_move_iterator(a_primsAndBVs.begin() + b),
                                     std::make_move_iterator(a_primsAndBVs.begin() + e));
  }

  return result;
};

/**
 * @brief Default stop function: stop partitioning when the node holds fewer than K primitives.
 * @tparam T  Floating-point precision.
 * @tparam P  Primitive type.
 * @tparam BV Bounding volume type.
 * @tparam K  Tree branching factor.
 * @param[in] a_node BVH node.
 * @return True if the node has fewer than K primitives.
 */
template <class T, class P, class BV, size_t K>
auto DefaultLeafPredicate =
  [](const BVH::TreeBVH<T, P, BV, K>& a_node) noexcept -> bool { return (a_node.getPrimitives()).size() < K; };

/**
 * @brief Tree-structured BVH node used during construction.
 * @details Each node stores a bounding volume, a list of primitives (non-empty for leaf
 * nodes only), and K child pointers (non-null for interior nodes only).
 *
 * Build the tree by constructing a root node from a set of (primitive, BV) pairs and
 * then calling topDownSortAndPartition() or bottomUpSortAndPartition(). Once built, call
 * pack() to obtain a cache-friendly PackedBVH for traversal.
 *
 * @tparam T  Floating-point precision.
 * @tparam P  Primitive type. Construction imposes no interface requirement on P itself, except
 * that PrimitiveCentroidPartitioner calls P::getCentroid(); the other partitioners and the
 * bottom-up build work from the bounding volumes alone. PackedBVH imposes no interface requirement
 * on P either; any further requirement comes entirely from whatever leaf-eval a caller passes to
 * PackedBVH::pruneTraverse() or PackedBVH::traverse() (see PackedBVH below).
 * @tparam BV Bounding volume type.
 * @tparam K  Tree branching factor (must be >= 2).
 */
template <class T, class P, class BV, size_t K>
class TreeBVH : public std::enable_shared_from_this<TreeBVH<T, P, BV, K>>
{
  static_assert(std::is_floating_point_v<T>, "TreeBVH: T must be a floating-point type");
  static_assert(K >= 2, "TreeBVH: branching factor K must be at least 2");

public:
  /**
   * @brief Alias for the primitive list type.
   */
  using PrimitiveList = BVH::PrimitiveList<P>;

  /**
   * @brief Alias for the 3D vector type.
   */
  using Vec3 = Vec3T<T>;

  /**
   * @brief Alias for this node type (used in traversal callbacks).
   */
  using Node = TreeBVH<T, P, BV, K>;

  /**
   * @brief Alias for a shared pointer to a node.
   */
  using NodePtr = std::shared_ptr<Node>;

  /**
   * @brief Alias for the partitioner type.
   */
  using Partitioner = BVH::Partitioner<P, BV, K>;

  /**
   * @brief Alias for the stop-function type.
   */
  using LeafPredicate = BVH::LeafPredicate<T, P, BV, K>;

  /**
   * @brief Default constructor. Creates an empty interior node.
   */
  TreeBVH() noexcept;

  /**
   * @brief Construct a leaf node from a set of (primitive, BV) pairs.
   * @param[in] a_primsAndBVs Primitives and their bounding volumes.
   */
  TreeBVH(const std::vector<PrimAndBV<P, BV>>& a_primsAndBVs);

  /**
   * @brief Construct a leaf node holding the given (primitive, bounding volume) pairs, moved in.
   * @details Rvalue overload: transfers the elements without copying or shared_ptr refcount churn.
   * Used where a caller can relinquish its list (e.g. topDownSortAndPartition moving a partitioner's
   * sub-list into a child node).
   * @param[in,out] a_primsAndBVs Primitives and their bounding volumes (consumed).
   */
  TreeBVH(std::vector<PrimAndBV<P, BV>>&& a_primsAndBVs);

  /**
   * @brief Destructor.
   */
  ~TreeBVH() noexcept = default;

  /**
   * @brief Deleted copy constructor.
   * @details A TreeBVH is a recursive structure of shared_ptr-linked children (m_children);
   * a compiler-generated copy would only alias the same child subtrees rather than cloning them.
   * topDownSortAndPartition()/bottomUpSortAndPartition() mutate a node's children in place, so such
   * a "copy" could silently share mutable state with the original instead of being independent.
   * Disallowed outright rather than doing the wrong thing; use deepCopy() to replicate a tree
   * independently.
   * @param[in] a_other Other instance (unused; deleted).
   */
  TreeBVH(const TreeBVH& a_other) = delete;

  /**
   * @brief Deleted copy assignment operator.
   * @details See the copy constructor for why copying is disallowed.
   * @param[in] a_other Other instance (unused; deleted).
   * @return Reference to *this (unused; deleted).
   */
  TreeBVH&
  operator=(const TreeBVH& a_other) = delete;

  /**
   * @brief Move constructor.
   * @details Explicitly defaulted: the user-declared destructor above would otherwise suppress
   * the implicitly-generated move constructor.
   * @param[in,out] a_other Other instance to move from.
   */
  TreeBVH(TreeBVH&& a_other) noexcept = default;

  /**
   * @brief Move assignment operator.
   * @param[in,out] a_other Other instance to move from.
   * @return Reference to *this.
   */
  TreeBVH&
  operator=(TreeBVH&& a_other) noexcept = default;

  /**
   * @brief Recursively partition this node top-down.
   * @details The stop criterion and partitioner determine the tree shape. Every one of the K
   * sub-lists the partitioner returns must hold at least one primitive: an empty one would become a
   * leaf with no primitives, which aborts, in every build, with a message naming the partition.
   * @param[in] a_partitioner Partitioning function. Divides a (primitive, BV) list into K non-empty sub-lists.
   * @param[in] a_stopCrit    Stop function. Returns true when a node should become a leaf.
   */
  inline void
  topDownSortAndPartition(const Partitioner&   a_partitioner = BVCentroidPartitioner<T, P, BV, K>,
                          const LeafPredicate& a_stopCrit    = DefaultLeafPredicate<T, P, BV, K>);

  /**
   * @brief Recursively partition this node bottom-up along a space-filling curve.
   * @details S must provide encode() and decode() functions returning SFC indices.
   * Primitives are sorted by their bounding-volume centroid projected onto the curve,
   * then split evenly into K^d leaves (d = floor(log_K(N)), so at most K primitives each) and
   * merged upwards in groups of K to the root.
   * @tparam S Space-filling curve type (e.g. Morton, Nested).
   */
  template <typename S>
  inline void
  bottomUpSortAndPartition();

  /**
   * @brief Return true if this is a leaf node (no children, non-empty primitive list).
   * @return True if this node holds primitives directly (i.e. is a leaf).
   */
  [[nodiscard]] inline bool
  isLeaf() const noexcept;

  /**
   * @brief Return true if the tree has already been partitioned.
   * @return True if topDownSortAndPartition() or bottomUpSortAndPartition() has been called.
   */
  [[nodiscard]] inline bool
  isPartitioned() const noexcept;

  /**
   * @brief Get the bounding volume for this node.
   * @return Reference to m_boundingVolume.
   */
  [[nodiscard]] inline const BV&
  getBoundingVolume() const noexcept;

  /**
   * @brief Get the primitives stored in this node.
   * @details Non-empty only for leaf nodes.
   * @return Reference to m_primitives.
   */
  [[nodiscard]] inline const PrimitiveList&
  getPrimitives() const noexcept;

  /**
   * @brief Get the per-primitive bounding volumes stored in this node.
   * @details Non-empty only for leaf nodes.
   * @return Reference to m_boundingVolumes.
   */
  [[nodiscard]] inline const std::vector<BV>&
  getBoundingVolumes() const noexcept;

  /**
   * @brief Get the distance from a_point to this node's bounding volume.
   * @details Returns zero if a_point is inside the bounding volume.
   * @param[in] a_point Query point.
   * @return Distance to the bounding volume surface, or zero if inside.
   */
  [[nodiscard]] inline T
  getDistanceToBoundingVolume(const Vec3& a_point) const noexcept;

  /**
   * @brief Get the K child nodes of this interior node.
   * @details All K children are non-null for interior nodes; the array is unused for leaf nodes.
   * @return Reference to m_children.
   */
  [[nodiscard]] inline const Array<std::shared_ptr<TreeBVH<T, P, BV, K>>, K>&
  getChildren() const noexcept;

  /**
   * @brief Produce an independent deep copy of this tree.
   * @details Recursively clones the node hierarchy: the returned tree owns brand-new nodes, so it
   * can be partitioned or otherwise mutated without affecting this tree (and vice versa). The
   * copy constructor is deleted precisely because a shallow copy would instead alias these mutable
   * child nodes; this is the explicit, correct way to replicate a tree. Primitives are *shared*,
   * not cloned -- each node's std::shared_ptr<const P> entries are copied by handle, matching how a
   * TreeBVH normally references its (immutable) primitives, e.g. faces shared with a DCEL mesh.
   * Works in either state: an unpartitioned tree clones to an unpartitioned leaf (build once, then
   * partition the copies differently), and a partitioned tree clones its full sub-structure.
   * @return Shared pointer to an independent clone of this tree.
   */
  [[nodiscard]] inline std::shared_ptr<TreeBVH<T, P, BV, K>>
  deepCopy() const;

  /**
   * @brief Recursion-free BVH traversal using an explicit LIFO stack (depth-first order).
   * @details The traversal maintains a stack of (node, NodeKey) pairs. It is seeded with the
   * root node paired with @p a_nodeKeyFactory applied to the root. On each iteration:
   *
   * 1. Pop the top (node, nodeKey) pair.
   * 2. Call @p a_prunePredicate(node, nodeKey). If it returns false the entire subtree rooted at
   * that node is skipped (pruned) and the loop continues with the next stack entry.
   * 3. If the node is a leaf, call @p a_leafEvaluator with the leaf's primitive list.
   * 4. If the node is an interior node:
   * a. Compute a NodeKey value for each of the K children by calling
   * @p a_nodeKeyFactory on each child.
   * b. Collect the K (child, NodeKey) pairs into a local array and pass them to
   * @p a_childOrderer, which reorders the array in-place.
   * c. Push all K pairs onto the stack in sorted order.
   *
   * Because the stack is LIFO, the child pushed last is visited first. @p a_childOrderer
   * should therefore place the most promising child last in the array. For a
   * nearest-distance query this means sorting children in descending order of
   * distance — farthest child first, nearest child last — so the nearest child
   * sits on top of the stack and is expanded next.
   *
   * @tparam NodeKey Auxiliary data type carried on the traversal stack (e.g. a running minimum distance).
   * @param[in] a_leafEvaluator     Called at each leaf with the leaf's primitive list.
   * @param[in] a_prunePredicate     Called at each node; return true to descend, false to prune.
   * @param[in] a_childOrderer      Reorders the K (child, NodeKey) pairs in-place before they are
   * pushed; the last element after sorting is visited first.
   * @param[in] a_nodeKeyFactory Produces a NodeKey value for a node; called once for the root
   * and once per child of every interior node that is visited.
   */
  template <class NodeKey>
  inline void
  traverse(const BVH::LeafEvaluator<P>&               a_leafEvaluator,
           const BVH::PrunePredicate<Node, NodeKey>&  a_prunePredicate,
           const BVH::ChildOrderer<Node, NodeKey, K>& a_childOrderer,
           const BVH::NodeKeyFactory<Node, NodeKey>&  a_nodeKeyFactory) const noexcept;

  /**
   * @brief Refit this subtree's bounding volumes in place after its primitives have moved.
   * @details Keeps the tree *topology* -- the node hierarchy and each leaf's primitive assignment --
   * exactly as it is, recomputing only the bounding volumes bottom-up: at every leaf, each
   * primitive's bounding volume is recomputed via @p a_bvConstructor and the leaf's node volume is
   * set to their union; at every interior node, the node volume is set to the union of its K
   * children's (already-refitted) volumes. This is the cheap way to keep a BVH valid for a *moving*
   * geometry -- primitives that shift a little between frames without changing which leaf they
   * belong to -- avoiding a full rebuild-and-repack. Because it never re-partitions, a geometry that
   * has deformed enough for primitives to migrate across the tree will accumulate looser (lower
   * query quality) bounding volumes over time and should periodically be rebuilt from scratch
   * instead.
   *
   * The primitives are reached by handle (@c std::shared_ptr<const P>): @p a_bvConstructor is called
   * with each primitive and must return that primitive's *current* bounding volume. Whatever moved
   * the geometry is expected to have updated the state the primitives read (e.g. a shared DCEL
   * mesh's vertex positions) before refit() is called.
   *
   * @tparam BVConstructor Callable: (const P&) -> BV, returning one primitive's current bounding volume.
   * @param[in] a_bvConstructor Bounding-volume constructor for a single primitive.
   * @return Reference to this node's refitted bounding volume (so a parent can union its children).
   */
  template <class BVConstructor>
  inline const BV&
  refit(const BVConstructor& a_bvConstructor);

  /**
   * @brief Flatten this tree into a cache-friendly PackedBVH with the same primitive type.
   * @details Requires BV == AABBT<T>; enforced by static_assert at instantiation.
   * @param[in,out] a_pool Pool the packed arrays are reserved from; must outlive the result.
   * @return Shared pointer to the resulting PackedBVH.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline std::shared_ptr<PackedBVH<T, P, K>>
  pack(Pool& a_pool) const;

  /**
   * @brief Flatten and convert this tree into a PackedBVH with a different primitive type Q.
   * @details a_converter is called once per leaf:
   * a_converter(leafPrims, 0, count) → std::vector<Q>
   * All returned values are accumulated into a single contiguous buffer, which becomes the
   * resulting PackedBVH's primitive array, preserving cache locality.
   * Requires BV == AABBT<T>; enforced by static_assert at instantiation.
   * @tparam Q         Destination primitive type.
   * @tparam Converter Callable: (PrimitiveList<P>, uint32_t offset, uint32_t count) → std::vector<Q>.
   * @param[in,out] a_pool Pool the packed arrays are reserved from; must outlive the result.
   * @param[in] a_converter Leaf-conversion function.
   * @return Shared pointer to the resulting PackedBVH<T, Q, K>.
   */
  template <class Q, class Converter>
  [[nodiscard]] EBGEOMETRY_HOST
  inline std::shared_ptr<PackedBVH<T, Q, K>>
  packWith(Pool& a_pool, Converter&& a_converter) const;

protected:
  /**
   * @brief Bounding volume enclosing all primitives in this subtree.
   */
  BV m_boundingVolume;

  /**
   * @brief True after topDownSortAndPartition() or bottomUpSortAndPartition() has been called.
   */
  bool m_partitioned;

  /**
   * @brief Primitives stored in this node. Non-empty for leaf nodes only.
   */
  std::vector<std::shared_ptr<const P>> m_primitives;

  /**
   * @brief Per-primitive bounding volumes. Non-empty for leaf nodes only.
   */
  std::vector<BV> m_boundingVolumes;

  /**
   * @brief K child nodes. Non-null for interior nodes only.
   */
  Array<std::shared_ptr<TreeBVH<T, P, BV, K>>, K> m_children;

  /**
   * @brief Non-const accessor for the primitive list (used during construction).
   * @return Reference to m_primitives.
   */
  inline PrimitiveList&
  getPrimitives() noexcept;

  /**
   * @brief Non-const accessor for the per-primitive bounding volumes (used during construction).
   * @return Reference to m_boundingVolumes.
   */
  inline std::vector<BV>&
  getBoundingVolumes() noexcept;

  /**
   * @brief Set the K child nodes, converting this node from a leaf into an interior node.
   * @param[in] a_children New child nodes.
   */
  inline void
  setChildren(const Array<std::shared_ptr<TreeBVH<T, P, BV, K>>, K>& a_children) noexcept;
};

} // namespace BVH

} // namespace EBGeometry

#include "EBGeometry_BVHBuildImplem.hpp"

#endif
