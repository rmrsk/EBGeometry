// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_BVH.hpp
 * @brief  Declaration of bounding volume hierarchy (BVH) classes.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_BVH_HPP
#define EBGEOMETRY_BVH_HPP

// Std includes
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(__SSE4_1__) || defined(__AVX__)
#include <immintrin.h>
#endif

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_SFC.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Namespace for various bounding volume hierarchy (BVH) functionalities.
 */
namespace BVH {

/**
 * @brief Enum for specifying the BVH construction strategy.
 */
enum class Build
{
  TopDown, ///< Recursive top-down partitioning.
  Morton,  ///< Bottom-up construction along a Morton space-filling curve.
  Nested,  ///< Bottom-up construction along a Nested space-filling curve.
  SAH      ///< Recursive top-down with binned Surface Area Heuristic splitting. This is the recommended
           ///< default: generally produces better-balanced trees and lower traversal cost than TopDown.
           ///< Use with BinnedSAHPartitioner. See BinnedSAHPartitioner for recommended K values per ISA.
};

/**
 * @brief Returns the SIMD-optimal BVH branching factor for type T on the current target ISA.
 * @details Maps the floating-point type and the compile-time ISA to the K that fills one
 * SIMD register exactly:
 *
 * | ISA       | T=float | T=double |
 * |-----------|---------|----------|
 * | AVX-512F  |   16    |    8     |
 * | AVX       |    8    |    4     |
 * | SSE4.1    |    4    |    4     |
 * | fallback  |    4    |    4     |
 *
 * Usage: `size_t K = BVH::DefaultBranchingRatio<T>()` as a template-parameter default.
 * @tparam T Floating-point precision type (float or double).
 * @return Optimal K for the current ISA and T.
 */
template <typename T>
[[nodiscard]] constexpr size_t
DefaultBranchingRatio() noexcept
{
  static_assert(std::is_floating_point_v<T>, "BVH::DefaultBranchingRatio requires a floating-point T");
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
 * @brief Forward declaration of the tree-structured BVH. Needed by StopFunction and the
 * default-partitioner lambdas defined below.
 */
template <class T, class P, class BV, size_t K>
class TreeBVH;

/**
 * @brief Forward declaration of the linearised BVH. Needed so that TreeBVH::pack() and
 * TreeBVH::packWith() can name their return types before PackedBVH is fully defined.
 */
template <class T, class P, size_t K>
class PackedBVH;

/**
 * @brief Convenience alias for a list of shared primitive pointers.
 * @tparam P Primitive type bounded by the BVH.
 */
template <class P>
using PrimitiveList = std::vector<std::shared_ptr<const P>>;

/**
 * @brief Convenience alias for a (primitive, bounding-volume) pair.
 * @tparam P  Primitive type.
 * @tparam BV Bounding volume type.
 */
template <class P, class BV>
using PrimAndBV = std::pair<std::shared_ptr<const P>, BV>;

/**
 * @brief Convenience alias for a list of (primitive, bounding-volume) pairs.
 * @tparam P  Primitive type.
 * @tparam BV Bounding volume type enclosing the implicit surface of each primitive.
 */
template <class P, class BV>
using PrimAndBVList = std::vector<PrimAndBV<P, BV>>;

/**
 * @brief Polymorphic partitioner: splits a list of (primitive, BV) pairs into K sub-lists.
 * @tparam P  Primitive type.
 * @tparam BV Bounding volume type.
 * @tparam K  Tree branching factor.
 * @param[in] a_primsAndBVs Input primitives and their bounding volumes.
 * @return K-element array of sub-lists.
 */
template <class P, class BV, size_t K>
using Partitioner = std::function<std::array<PrimAndBVList<P, BV>, K>(const PrimAndBVList<P, BV>& a_primsAndBVs)>;

/**
 * @brief Predicate for deciding when a TreeBVH node should become a leaf (i.e., no further splitting).
 * @tparam T  Floating-point precision.
 * @tparam P  Primitive type.
 * @tparam BV Bounding volume type.
 * @tparam K  Tree branching factor.
 * @param[in] a_node BVH node under consideration.
 * @return True if the node should not be sub-divided further.
 */
template <class T, class P, class BV, size_t K>
using StopFunction = std::function<bool(const TreeBVH<T, P, BV, K>& a_node)>;

/**
 * @brief Leaf-update callback for TreeBVH::traverse.
 * @details Called once for every leaf node visited during traversal.
 * @tparam P Primitive type.
 * @param[in] a_primitives Primitive list stored in the leaf.
 */
template <class P>
using Updater = std::function<void(const PrimitiveList<P>& a_primitives)>;

/**
 * @brief Leaf-update callback for PackedBVH::traverse.
 * @details Receives a view into the global primitive array (offset + count) rather than a
 * temporary sub-list, avoiding a heap allocation per leaf visit.
 * @tparam P Primitive type.
 * @param[in] a_primitives Global primitive list.
 * @param[in] a_offset     Index of the first primitive belonging to this leaf.
 * @param[in] a_count      Number of primitives in this leaf.
 */
template <class P>
using PackedUpdater = std::function<void(const PrimitiveList<P>& a_primitives, size_t a_offset, size_t a_count)>;

/**
 * @brief Node-visit predicate for BVH traversal.
 * @details Must return true to descend into the node and false to prune it.
 * @tparam NodeType Node type (TreeBVH or PackedBVH::Node).
 * @tparam Meta     Caller-supplied auxiliary data type attached to each stack entry
 * (e.g. a running minimum distance).
 * @param[in] a_node Node under consideration.
 * @param[in] a_meta Caller-supplied meta-data for this stack entry.
 * @return True if the subtree rooted at a_node should be visited.
 */
template <class NodeType, class Meta>
using Visiter = std::function<bool(const NodeType& a_node, const Meta& a_meta)>;

/**
 * @brief Child-ordering callback for TreeBVH traversal.
 * @details Sorts an array of (child-node-pointer, meta) pairs in-place so that
 * the most promising child is visited first.
 * @tparam NodeType Node type (TreeBVH).
 * @tparam Meta     Auxiliary data type attached to each stack entry.
 * @tparam K        Tree branching factor.
 * @param[in,out] a_children K child nodes together with their meta-data.
 */
template <class NodeType, class Meta, size_t K>
using Sorter = std::function<void(std::array<std::pair<std::shared_ptr<const NodeType>, Meta>, K>& a_children)>;

/**
 * @brief Child-ordering callback for PackedBVH traversal.
 * @details Same role as Sorter but uses 32-bit node indices instead of shared_ptrs,
 * halving the stack-entry size.
 * @tparam Meta Auxiliary data type attached to each stack entry.
 * @tparam K    Tree branching factor.
 * @param[in,out] a_children K (node-index, meta) pairs to sort.
 */
template <class Meta, size_t K>
using PackedSorter = std::function<void(std::array<std::pair<uint32_t, Meta>, K>& a_children)>;

/**
 * @brief Meta-data factory called once per node during BVH traversal.
 * @details Produces the Meta value that will be passed to Visiter and Sorter for
 * each child of the current node.
 * @tparam NodeType Node type (TreeBVH or PackedBVH::Node).
 * @tparam Meta     Auxiliary data type to produce.
 * @param[in] a_node Current node.
 * @return Meta value for a_node's children.
 */
template <class NodeType, class Meta>
using MetaUpdater = std::function<Meta(const NodeType& a_node)>;

/**
 * @brief Utility: split a vector into K almost-equal contiguous chunks.
 * @tparam X Element type.
 * @tparam K Number of chunks.
 * @param[in] a_primitives Input vector.
 * @return Array of K sub-vectors whose sizes differ by at most 1.
 */
template <class X, size_t K>
auto EqualCounts = [](const std::vector<X>& a_primitives) noexcept -> std::array<std::vector<X>, K> {
  static_assert(K >= 2, "EqualCounts<X, K>: branching factor K must be at least 2");

  EBGEOMETRY_EXPECT(!a_primitives.empty());

  const int length = a_primitives.size() / K;
  int       remain = a_primitives.size() % K;

  int begin = 0;
  int end   = 0;

  std::array<std::vector<X>, K> chunks;

  for (size_t k = 0; k < K; k++) {
    end += (remain > 0) ? length + 1 : length;
    remain--;

    chunks[k] = std::vector<X>(a_primitives.begin() + begin, a_primitives.begin() + end);

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
auto PrimitiveCentroidPartitioner =
  [](const PrimAndBVList<P, BV>& a_primsAndBVs) noexcept -> std::array<PrimAndBVList<P, BV>, K> {
  EBGEOMETRY_EXPECT(!a_primsAndBVs.empty());

  Vec3T<T> lo = +Vec3T<T>::max();
  Vec3T<T> hi = -Vec3T<T>::max();

  for (const auto& pbv : a_primsAndBVs) {
    lo = min(lo, pbv.first->getCentroid());
    hi = max(hi, pbv.first->getCentroid());
  }

  const size_t splitDir = (hi - lo).maxDir(true);

  PrimAndBVList<P, BV> sortedPrimsAndBVs(a_primsAndBVs);

  std::sort(sortedPrimsAndBVs.begin(),
            sortedPrimsAndBVs.end(),
            [splitDir](const PrimAndBV<P, BV>& pbv1, const PrimAndBV<P, BV>& pbv2) -> bool {
              return pbv1.first->getCentroid(splitDir) < pbv2.first->getCentroid(splitDir);
            });

  return BVH::EqualCounts<PrimAndBV<P, BV>, K>(sortedPrimsAndBVs);
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
auto BVCentroidPartitioner = [](const PrimAndBVList<P, BV>& a_primsAndBVs) -> std::array<PrimAndBVList<P, BV>, K> {
  EBGEOMETRY_EXPECT(!a_primsAndBVs.empty());

  Vec3T<T> lo = +Vec3T<T>::max();
  Vec3T<T> hi = -Vec3T<T>::max();

  for (const auto& pbv : a_primsAndBVs) {
    lo = min(lo, pbv.second.getCentroid());
    hi = max(hi, pbv.second.getCentroid());
  }

  const size_t splitDir = (hi - lo).maxDir(true);

  PrimAndBVList<P, BV> sortedPrimsAndBVs(a_primsAndBVs);

  std::sort(sortedPrimsAndBVs.begin(),
            sortedPrimsAndBVs.end(),
            [splitDir](const PrimAndBV<P, BV>& pbv1, const PrimAndBV<P, BV>& pbv2) -> bool {
              return pbv1.second.getCentroid()[splitDir] < pbv2.second.getCentroid()[splitDir];
            });

  return BVH::EqualCounts<PrimAndBV<P, BV>, K>(sortedPrimsAndBVs);
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
 * @return Split index (index of the first element of the right group).
 */
template <class T, class P, class BV>
inline size_t
SAH2WaySplit(PrimAndBVList<P, BV>& a_list, const size_t a_begin, const size_t a_end) noexcept
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

  T   bestCost  = std::numeric_limits<T>::max();
  T   bestPlane = T(0);
  int bestAxis  = -1;

  Vec3T<T> binLo[BINS];
  Vec3T<T> binHi[BINS];

  int binCnt[BINS];

  T   leftArea[BINS - 1];
  int leftCnt[BINS - 1];

  for (int axis = 0; axis < 3; axis++) {
    const T lo  = clo[axis];
    const T hi  = chi[axis];
    const T ext = hi - lo;

    if (ext <= T(0)) {
      continue;
    }

    const T scale = T(BINS) / ext;

    for (int b = 0; b < BINS; b++) {
      binLo[b]  = Vec3T<T>::max();
      binHi[b]  = -Vec3T<T>::max();
      binCnt[b] = 0;
    }

    for (size_t i = a_begin; i < a_end; i++) {
      const int b = std::min(BINS - 1, (int)((a_list[i].second.getCentroid()[axis] - lo) * scale));
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
 */
template <class T, class P, class BV>
inline void
SAHKWaySplit(PrimAndBVList<P, BV>&                   a_list,
             const size_t                            a_begin,
             const size_t                            a_end,
             const size_t                            a_K,
             std::vector<std::pair<size_t, size_t>>& a_groups) noexcept
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
  const size_t rawMid = SAH2WaySplit<T, P, BV>(a_list, a_begin, a_end);
  const size_t mid    = std::max(a_begin + K1, std::min(a_end - K2, rawMid));

  SAHKWaySplit<T, P, BV>(a_list, a_begin, mid, K1, a_groups);
  SAHKWaySplit<T, P, BV>(a_list, mid, a_end, K2, a_groups);
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
 * @param[in] a_primsAndBVs Input (primitive, BV) pairs.
 * @return K sub-lists.
 */
template <class T, class P, class BV, size_t K>
auto BinnedSAHPartitioner = [](const PrimAndBVList<P, BV>& a_primsAndBVs) -> std::array<PrimAndBVList<P, BV>, K> {
  EBGEOMETRY_EXPECT(!a_primsAndBVs.empty());

  PrimAndBVList<P, BV>                   working(a_primsAndBVs);
  std::vector<std::pair<size_t, size_t>> groups;
  groups.reserve(K);

  SAHKWaySplit<T, P, BV>(working, 0, working.size(), K, groups);

  std::array<PrimAndBVList<P, BV>, K> result;
  for (size_t k = 0; k < K; k++) {
    const auto [b, e] = groups[k];
    result[k]         = PrimAndBVList<P, BV>(working.begin() + b, working.begin() + e);
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
auto DefaultStopFunction =
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
 * @tparam P  Primitive type. Must provide getCentroid() and signedDistance(Vec3T<T>).
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
  using StopFunction = BVH::StopFunction<T, P, BV, K>;

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
   * @brief Destructor.
   */
  ~TreeBVH() noexcept;

  /**
   * @brief Recursively partition this node top-down.
   * @details The stop criterion and partitioner determine the tree shape.
   * @param[in] a_partitioner Partitioning function. Divides a (primitive, BV) list into K sub-lists.
   * @param[in] a_stopCrit    Stop function. Returns true when a node should become a leaf.
   */
  inline void
  topDownSortAndPartition(const Partitioner&  a_partitioner = BVCentroidPartitioner<T, P, BV, K>,
                          const StopFunction& a_stopCrit    = DefaultStopFunction<T, P, BV, K>);

  /**
   * @brief Recursively partition this node bottom-up along a space-filling curve.
   * @details S must provide encode() and decode() functions returning SFC indices.
   * Primitives are sorted by their bounding-volume centroid projected onto the curve,
   * then grouped into leaves of size K and merged upwards to the root.
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
  [[nodiscard]] inline const std::array<std::shared_ptr<TreeBVH<T, P, BV, K>>, K>&
  getChildren() const noexcept;

  /**
   * @brief Recursion-free BVH traversal using an explicit LIFO stack (depth-first order).
   * @details The traversal maintains a stack of (node, Meta) pairs. It is seeded with the
   * root node paired with @p a_metaUpdater applied to the root. On each iteration:
   *
   * 1. Pop the top (node, meta) pair.
   * 2. Call @p a_visiter(node, meta). If it returns false the entire subtree rooted at
   * that node is skipped (pruned) and the loop continues with the next stack entry.
   * 3. If the node is a leaf, call @p a_updater with the leaf's primitive list.
   * 4. If the node is an interior node:
   * a. Compute a Meta value for each of the K children by calling
   * @p a_metaUpdater on each child.
   * b. Collect the K (child, Meta) pairs into a local array and pass them to
   * @p a_sorter, which reorders the array in-place.
   * c. Push all K pairs onto the stack in sorted order.
   *
   * Because the stack is LIFO, the child pushed last is visited first. @p a_sorter
   * should therefore place the most promising child last in the array. For a
   * nearest-distance query this means sorting children in descending order of
   * distance — farthest child first, nearest child last — so the nearest child
   * sits on top of the stack and is expanded next.
   *
   * @tparam Meta Auxiliary data type carried on the traversal stack (e.g. a running minimum distance).
   * @param[in] a_updater     Called at each leaf with the leaf's primitive list.
   * @param[in] a_visiter     Called at each node; return true to descend, false to prune.
   * @param[in] a_sorter      Reorders the K (child, Meta) pairs in-place before they are
   * pushed; the last element after sorting is visited first.
   * @param[in] a_metaUpdater Produces a Meta value for a node; called once for the root
   * and once per child of every interior node that is visited.
   */
  template <class Meta>
  inline void
  traverse(const BVH::Updater<P>&              a_updater,
           const BVH::Visiter<Node, Meta>&     a_visiter,
           const BVH::Sorter<Node, Meta, K>&   a_sorter,
           const BVH::MetaUpdater<Node, Meta>& a_metaUpdater) const noexcept;

  /**
   * @brief Flatten this tree into a cache-friendly PackedBVH with the same primitive type.
   * @details Requires BV == AABBT<T>; enforced by static_assert at instantiation.
   * @return Shared pointer to the resulting PackedBVH.
   */
  [[nodiscard]] inline std::shared_ptr<PackedBVH<T, P, K>>
  pack() const;

  /**
   * @brief Flatten and convert this tree into a PackedBVH with a different primitive type Q.
   * @details a_converter is called once per leaf:
   * a_converter(leafPrims, 0, count) → std::vector<Q>
   * All returned values are accumulated into a single contiguous buffer, then exposed
   * through aliased shared_ptrs to preserve cache locality.
   * Requires BV == AABBT<T>; enforced by static_assert at instantiation.
   * @tparam Q         Destination primitive type.
   * @tparam Converter Callable: (PrimitiveList<P>, uint32_t offset, uint32_t count) → std::vector<Q>.
   * @param[in] a_converter Leaf-conversion function.
   * @return Shared pointer to the resulting PackedBVH<T, Q, K>.
   */
  template <class Q, class Converter>
  [[nodiscard]] inline std::shared_ptr<PackedBVH<T, Q, K>>
  packWith(Converter&& a_converter) const;

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
  std::array<std::shared_ptr<TreeBVH<T, P, BV, K>>, K> m_children;

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
  setChildren(const std::array<std::shared_ptr<TreeBVH<T, P, BV, K>>, K>& a_children) noexcept;
};

/**
 * @brief Linearised, AABB-backed BVH with SIMD-accelerated traversal.
 * @details PackedBVH is the runtime query class. It stores a flat depth-first array of
 * Node structs, a contiguous primitive list, and a per-node SoA AABB cache that enables
 * SIMD child tests.
 *
 * Instances are obtained by calling TreeBVH::pack() (same primitive type) or
 * TreeBVH::packWith<Q>(converter) (type-converting pack).
 *
 * SIMD paths are selected at compile time via if constexpr:
 * - K==4, T==float  → SSE4.1 (__m128)
 * - K==4, T==double → AVX    (__m256d)
 * - K==8, T==float  → AVX    (__m256)
 * - K==8, T==double → AVX    (two __m256d passes)
 * All other combinations fall back to scalar traversal.
 *
 * @tparam T Floating-point precision.
 * @tparam P Primitive type. Must provide signedDistance(Vec3T<T>).
 * @tparam K BVH branching factor.
 */
template <class T, class P, size_t K>
class PackedBVH
{
  static_assert(std::is_floating_point_v<T>, "PackedBVH: T must be a floating-point type");
  static_assert(K >= 2, "PackedBVH: branching factor K must be at least 2");

public:
  /**
   * @brief AABB type used for all bounding volumes in this BVH.
   */
  using BV = EBGeometry::BoundingVolumes::AABBT<T>;

  /**
   * @brief Compact BVH node stored in the flat node array.
   * @details Each node holds a bounding volume, a primitive range (leaves) or child
   * index table (interior nodes). Exposed as a public nested type so that users can
   * write traverse() callbacks (Visiter, MetaUpdater) that inspect nodes.
   */
  struct Node
  {
    /**
     * @brief Axis-aligned bounding box for this node's subtree.
     */
    BV m_bv{};

    /**
     * @brief Index of the first primitive in the global primitive list (leaf nodes only).
     */
    uint32_t m_primOff{};

    /**
     * @brief Number of primitives in this leaf (zero for interior nodes).
     */
    uint32_t m_numPrims{};

    /**
     * @brief Depth-first indices of the K child nodes (interior nodes only).
     */
    std::array<uint32_t, K> m_childOff{};

    /**
     * @brief Set the bounding volume for this node.
     * @param[in] a_bv Bounding volume.
     */
    inline void
    setBoundingVolume(const BV& a_bv) noexcept
    {
      m_bv = a_bv;
    }

    /**
     * @brief Set the primitive offset for this leaf node.
     * @param[in] a_off Index into the global primitive list.
     */
    inline void
    setPrimitivesOffset(uint32_t a_off) noexcept
    {
      m_primOff = a_off;
    }

    /**
     * @brief Set the primitive count for this leaf node.
     * @param[in] a_n Number of primitives.
     */
    inline void
    setNumPrimitives(uint32_t a_n) noexcept
    {
      m_numPrims = a_n;
    }

    /**
     * @brief Set the depth-first index of the k-th child.
     * @param[in] a_off Node index of the child.
     * @param[in] a_k   Child slot (0 … K-1).
     */
    inline void
    setChildOffset(uint32_t a_off, size_t a_k) noexcept
    {
      EBGEOMETRY_EXPECT(a_k < K);
      m_childOff[a_k] = a_off;
    }

    /**
     * @brief Get the bounding volume.
     * @return Reference to m_bv.
     */
    [[nodiscard]] inline const BV&
    getBoundingVolume() const noexcept
    {
      return m_bv;
    }

    /**
     * @brief Get the primitive offset (leaf nodes only).
     * @return Index of the first primitive in the global list.
     */
    [[nodiscard]] inline uint32_t
    getPrimitivesOffset() const noexcept
    {
      return m_primOff;
    }

    /**
     * @brief Get the primitive count.
     * @return Number of primitives; zero for interior nodes.
     */
    [[nodiscard]] inline uint32_t
    getNumPrimitives() const noexcept
    {
      return m_numPrims;
    }

    /**
     * @brief Get the child index table.
     * @return Reference to the K-element child-offset array.
     */
    [[nodiscard]] inline const std::array<uint32_t, K>&
    getChildOffsets() const noexcept
    {
      return m_childOff;
    }

    /**
     * @brief Return true if this is a leaf node.
     * @return True if m_numPrims > 0 (leaf), false otherwise (interior).
     */
    [[nodiscard]] inline bool
    isLeaf() const noexcept
    {
      return m_numPrims > 0;
    }

    /**
     * @brief Get the distance from a_point to this node's bounding volume.
     * @param[in] a_point Query point.
     * @return Distance to the bounding-box surface, or zero if inside.
     */
    [[nodiscard]] inline T
    getDistanceToBoundingVolume(const Vec3T<T>& a_point) const noexcept
    {
      return m_bv.getDistance(a_point);
    }
  };

  /**
   * @brief Deleted default constructor. Use TreeBVH::pack() or TreeBVH::packWith() to construct.
   */
  PackedBVH() = delete;

  /**
   * @brief Construct by packing a TreeBVH (identity primitive type).
   * @details Walks the tree depth-first, fills m_linearNodes and m_primitives directly,
   * then builds the SoA AABB cache. The source tree must have been built with
   * BV == AABBT<T>; bounding volumes are reused without conversion.
   * @param[in] a_tree Source tree.
   */
  inline PackedBVH(const TreeBVH<T, P, BV, K>& a_tree);

  /**
   * @brief Construct by packing a TreeBVH with primitive-type conversion.
   * @details The source tree holds primitives of type Q; the packed BVH holds
   * primitives of type P.  The most common reason for this mismatch is an SoA packing
   * step: a tree built over individual triangles (Q = Triangle<T>) can be repacked
   * into a BVH whose leaves hold SIMD-width groups (P = TriangleSoAT<T,W>), enabling
   * vectorised distance evaluation at every leaf visit.
   *
   * The converter is called once per leaf of the source tree and must return a
   * @c std::vector<P> containing all target primitives for that leaf:
   *
   * @code
   * a_converter(leafPrims, offset, count) → std::vector<P>
   * @endcode
   *
   * where @p leafPrims is the leaf's @c PrimitiveList<Q>, @p offset is the index of
   * the first primitive in the global list, and @p count is the number of primitives in
   * the leaf.  All returned vectors are stored contiguously; the resulting PackedBVH holds
   * aliased @c shared_ptr into that buffer so no extra copies are made during traversal.
   *
   * The source tree must have been built with BV == AABBT<T>; bounding volumes are reused
   * without conversion.
   *
   * @tparam Q         Source primitive type stored in the source tree.
   * @tparam Converter Callable: (PrimitiveList<Q>, uint32_t offset, uint32_t count) → std::vector<P>.
   * @param[in] a_tree      Source tree.
   * @param[in] a_converter Leaf-conversion function.
   */
  template <class Q, class Converter>
  inline PackedBVH(const TreeBVH<T, Q, BV, K>& a_tree, Converter&& a_converter);

  /**
   * @brief Virtual destructor.
   */
  inline virtual ~PackedBVH() = default;

  /**
   * @brief Get the global primitive list (in leaf-traversal order).
   * @return Reference to m_primitives.
   */
  [[nodiscard]] inline const std::vector<std::shared_ptr<const P>>&
  getPrimitives() const noexcept;

  /**
   * @brief Get the bounding volume of the root node.
   * @return Reference to the root node's bounding volume.
   */
  [[nodiscard]] inline const BV&
  getBoundingVolume() const noexcept;

  /**
   * @brief Compute and return the bounding volume of this BVH.
   * @details Identical to getBoundingVolume() but presents a getCentroid()-compatible
   * interface, enabling PackedBVH to serve as a primitive in an outer TreeBVH hierarchy.
   * @return Root node bounding volume.
   */
  [[nodiscard]] inline BV
  computeBoundingVolume() const noexcept;

  /**
   * @brief Recursion-free BVH traversal using a vector-backed LIFO stack (depth-first order).
   * @details The traversal mirrors TreeBVH::traverse() in structure but works directly on the
   * flat node array, using 32-bit indices instead of shared_ptr<const Node>. This halves the
   * stack-entry size and avoids reference-count traffic on every push and pop.
   *
   * The stack is a std::vector used as a LIFO queue via push_back/pop_back, pre-reserved to
   * 64 entries to avoid reallocation for typical tree depths. It is seeded with node index 0
   * (the root) paired with @p a_metaUpdater applied to the root node. On each iteration:
   *
   * 1. Pop the back entry to obtain a (nodeIdx, meta) pair.
   * 2. Look up the node at m_linearNodes[nodeIdx].
   * 3. Call @p a_visiter(node, meta). If it returns false the entire subtree rooted at
   * that node is skipped (pruned) and the loop continues.
   * 4. If the node is a leaf, call @p a_updater with the global primitive list
   * m_primitives, the leaf's primitive offset, and its primitive count. The updater
   * receives a view into the shared list rather than a freshly allocated sub-list,
   * avoiding a heap allocation per leaf visit.
   * 5. If the node is an interior node:
   * a. Collect the K child indices from node.getChildOffsets(), look each child up in
   * m_linearNodes, and call @p a_metaUpdater on each to produce a Meta value.
   * b. Bundle the K (childIdx, Meta) pairs into a local array and pass it to
   * @p a_sorter, which reorders the array in-place.
   * c. Push all K pairs onto the back of the stack in sorted order.
   *
   * Because the stack is LIFO, the child pushed last is visited first. @p a_sorter
   * should therefore place the most promising child last in the array. For a
   * nearest-distance query this means sorting children in descending order of
   * distance — farthest child first, nearest child last — so the nearest child
   * sits at the back of the vector and is expanded next.
   *
   * @tparam Meta Auxiliary data type carried on the traversal stack (e.g. a running minimum distance).
   * @param[in] a_updater     Called at each leaf with the global primitive list, offset, and count.
   * @param[in] a_visiter     Called at each node; return true to descend, false to prune.
   * @param[in] a_sorter      Reorders the K (childIdx, Meta) pairs in-place before they are
   * pushed; the last element after sorting is visited first.
   * @param[in] a_metaUpdater Produces a Meta value for a node; called once for the root
   * and once per child of every interior node that is visited.
   */
  template <class Meta>
  inline void
  traverse(const BVH::PackedUpdater<P>&        a_updater,
           const BVH::Visiter<Node, Meta>&     a_visiter,
           const BVH::PackedSorter<Meta, K>&   a_sorter,
           const BVH::MetaUpdater<Node, Meta>& a_metaUpdater) const noexcept;

  /**
   * @brief Compute the signed distance from a_point to the nearest primitive.
   * @details Uses SIMD traversal where available and falls back to scalar otherwise.
   * @param[in] a_point Query point.
   * @return Signed distance to the nearest primitive surface.
   */
  [[nodiscard]] inline T
  signedDistance(const Vec3T<T>& a_point) const noexcept;

protected:
  /**
   * @brief Flat depth-first node array.
   */
  std::vector<Node> m_linearNodes;

  /**
   * @brief Global primitive list in leaf-traversal order.
   */
  std::vector<std::shared_ptr<const P>> m_primitives;

  /**
   * @brief SoA layout of K children's AABBs for a single interior node.
   * @details m_lo[axis][child] / m_hi[axis][child] layout places each axis row in a
   * contiguous T[K] buffer that can be loaded as a single SIMD register.
   * Private implementation detail of PackedBVH; never exposed in any public interface.
   */
  struct ChildAABBSoA
  {
    /**
     * @brief Lower corners: m_lo[axis][child], axis in {0,1,2}, child in {0,…,K-1}.
     */
    alignas(sizeof(T) * K) T m_lo[3][K];

    /**
     * @brief Upper corners: m_hi[axis][child], axis in {0,1,2}, child in {0,…,K-1}.
     */
    alignas(sizeof(T) * K) T m_hi[3][K];
  };

  /**
   * @brief Per-node SoA AABB cache used by the SIMD traversal in signedDistance().
   */
  std::vector<ChildAABBSoA> m_childAabbSoA;

  /**
   * @brief Populate m_childAabbSoA from the completed m_linearNodes array.
   * @details Called at the end of every constructor after m_linearNodes is fully built.
   */
  inline void
  buildSoA();
};

} // namespace BVH

} // namespace EBGeometry

#include "EBGeometry_BVHImplem.hpp"

#endif
