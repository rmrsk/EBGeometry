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
#include <iterator>
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
 * @brief Configuration for the ClusterSAH direct PackedBVH construction path.
 * @details ClusterSAH first groups primitives into small, spatially-tight *clusters* (buckets of at
 * most @c maxClusterSize primitives, formed by a cheap density-adaptive midpoint subdivision that
 * stops early), then runs binned SAH top-down over the clusters -- so SAH partitions ~N/maxClusterSize
 * boxes instead of all N primitives. The result is a near-SAH-quality tree at a fraction of the
 * single-threaded build cost, robust across uniform, surface, and clustered primitive distributions
 * (unlike a fixed Cartesian grid, which overcrowds on non-uniform data). @c maxClusterSize trades
 * build time (larger -> fewer, cheaper SAH units, faster build) against query quality (larger ->
 * coarser leaves).
 */
struct ClusterSpec
{
  size_t maxClusterSize = 8; ///< Maximum primitives per cluster (the leaf/bucket granularity). Must be > 0.
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
 * @brief Forward declaration of the tree-structured BVH. Needed by LeafPredicate and the
 * default-partitioner lambdas defined below.
 */
template <class T, class P, class BV, size_t K>
class TreeBVH;

/**
 * @brief Convenience alias for a list of shared primitive pointers.
 * @tparam P Primitive type bounded by the BVH.
 */
template <class P>
using PrimitiveList = std::vector<std::shared_ptr<const P>>;

/**
 * @brief Default storage policy for PackedBVH: primitives stored as std::shared_ptr<const P>,
 * exactly as PackedBVH has always stored them.
 * @details A storage policy is a stateless struct bundling an associated storage representation
 * (@c StorageType) with the handful of operations PackedBVH needs to build and read it: @c get()
 * (per-element access, used by every leaf-visit callback), @c appendTreeLeaf() (copying a TreeBVH
 * leaf's primitives into PackedBVH's flat array during the identity pack() constructor), and
 * @c appendAliased() (appending a converting packWith() constructor's single contiguous
 * conversion buffer to PackedBVH's flat array). Both @c appendTreeLeaf() and @c appendAliased()
 * are appenders: they add to whatever @p a_dst already holds rather than replacing it, so every
 * policy behaves identically no matter how many times PackedBVH calls them. Swapping the policy
 * changes nothing about tree
 * construction or traversal -- TreeBVH itself always stores primitives as shared_ptr, regardless
 * of which policy the PackedBVH built from it uses -- only what PackedBVH's own primitive array
 * holds.
 * @tparam P Primitive type.
 */
template <class P>
struct SharedPtrStorage
{
  /**
   * @brief Storage representation: a shared pointer to a const primitive, as PackedBVH has
   * always stored it.
   */
  using StorageType = std::shared_ptr<const P>;

  /**
   * @brief Dereference stored element to the primitive it refers to.
   * @param[in] a_stored Stored element.
   * @return Reference to the underlying primitive.
   */
  [[nodiscard]] static const P&
  get(const StorageType& a_stored) noexcept
  {
    return *a_stored;
  }

  /**
   * @brief Append one TreeBVH leaf's primitives to PackedBVH's flat primitive array.
   * @details Used by PackedBVH's identity pack() constructor. Trivial for this policy: the source
   * leaf's shared_ptrs are copied in as-is.
   * @param[in,out] a_dst       PackedBVH's flat primitive array.
   * @param[in]     a_leafPrims One TreeBVH leaf's primitive list.
   */
  static void
  appendTreeLeaf(std::vector<StorageType>& a_dst, const PrimitiveList<P>& a_leafPrims)
  {
    a_dst.insert(a_dst.end(), a_leafPrims.begin(), a_leafPrims.end());
  }

  /**
   * @brief Materialise a converting packWith() constructor's single contiguous conversion buffer
   * into PackedBVH's flat primitive array.
   * @details Uses the shared_ptr aliasing constructor so every element shares one control block
   * (one allocation for the whole buffer) while still presenting as an independent
   * shared_ptr<const P> per element.
   * @param[in,out] a_dst   PackedBVH's flat primitive array.
   * @param[in]     a_block Single contiguous buffer holding every converted primitive.
   */
  static void
  appendAliased(std::vector<StorageType>& a_dst, const std::shared_ptr<std::vector<P>>& a_block)
  {
    a_dst.reserve(a_dst.size() + a_block->size());

    for (size_t i = 0; i < a_block->size(); i++) {
      a_dst.emplace_back(a_block, &(*a_block)[i]);
    }
  }
};

/**
 * @brief Storage policy that stores primitives directly by value, with no pointer indirection at
 * all.
 * @details Removes the per-primitive heap allocation and pointer-chasing that
 * SharedPtrStorage<P> pays on every leaf visit -- worthwhile when P is a self-contained value type
 * (no shared ownership needed) and leaves are visited often, e.g. a nearest-neighbor search over a
 * point cloud.
 * @tparam P Primitive type.
 */
template <class P>
struct ValueStorage
{
  /**
   * @brief Storage representation: the primitive itself, stored by value.
   */
  using StorageType = P;

  /**
   * @brief Identity access -- the stored element already is the primitive.
   * @param[in] a_stored Stored element.
   * @return Reference to the primitive.
   */
  [[nodiscard]] static const P&
  get(const StorageType& a_stored) noexcept
  {
    return a_stored;
  }

  /**
   * @brief Append one TreeBVH leaf's primitives to PackedBVH's flat primitive array.
   * @details Used by PackedBVH's identity pack() constructor. TreeBVH's own leaf primitives are
   * always shared_ptr<const P> (TreeBVH itself is unaffected by the packed BVH's storage policy),
   * so this policy dereferences and copies each one by value; unavoidable here, since the source
   * is shared_ptr-based regardless of the destination policy.
   * @param[in,out] a_dst       PackedBVH's flat primitive array.
   * @param[in]     a_leafPrims One TreeBVH leaf's primitive list.
   */
  static void
  appendTreeLeaf(std::vector<StorageType>& a_dst, const PrimitiveList<P>& a_leafPrims)
  {
    // NB: do NOT reserve(a_dst.size() + a_leafPrims.size()) here. appendTreeLeaf is called once per
    // leaf as the whole tree is packed, and reserving to an each-time-slightly-larger *exact* size
    // defeats std::vector's geometric growth -- it forces a fresh reallocation (copying every
    // element already appended) on every leaf, making a whole build O(N^2) in the primitive count.
    // Both the identity pack() constructor and the direct top-down PackedBVH constructor append
    // leaves this way, so the quadratic cost would hit every ValueStorage build over many leaves.
    // Plain push_back grows the buffer geometrically and keeps construction linear.
    for (const auto& p : a_leafPrims) {
      a_dst.push_back(*p);
    }
  }

  /**
   * @brief Materialise a converting packWith() constructor's single contiguous conversion buffer
   * into PackedBVH's flat primitive array.
   * @details Like SharedPtrStorage<P>'s equivalent, this *appends* @p a_block to @p a_dst, so the
   * two policies honour the same contract regardless of how many times it is called. When @p a_dst
   * is still empty (the case for PackedBVH's single-call converting constructor) it takes the fast
   * path of stealing the already-contiguous buffer wholesale -- no aliasing shared_ptr machinery,
   * no control-block allocation, and no per-element move; otherwise it move-appends each element.
   * @param[in,out] a_dst   PackedBVH's flat primitive array.
   * @param[in]     a_block Single contiguous buffer holding every converted primitive.
   */
  static void
  appendAliased(std::vector<StorageType>& a_dst, const std::shared_ptr<std::vector<P>>& a_block)
  {
    if (a_dst.empty()) {
      a_dst = std::move(*a_block);
    }
    else {
      a_dst.insert(a_dst.end(), std::make_move_iterator(a_block->begin()), std::make_move_iterator(a_block->end()));
    }
  }
};

/**
 * @brief Forward declaration of the linearised BVH. Needed so that TreeBVH::pack() and
 * TreeBVH::packWith() can name their return types before PackedBVH is fully defined.
 * @details StoragePolicy defaults to SharedPtrStorage<P>, preserving today's exact behaviour for
 * every existing 3-argument PackedBVH<T, P, K> instantiation.
 */
template <class T, class P, size_t K, class StoragePolicy = SharedPtrStorage<P>>
class PackedBVH;

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
 * @details The input list is taken *by value* (a sink): callers move their list in, and an
 * implementation should partition it in place and *move* the K sub-lists out -- so a whole top-down
 * build reorders primitive handles rather than repeatedly copying them (the built-in partitioners
 * all do this). The split arithmetic is the cheap part; avoiding the copies is what keeps
 * construction fast.
 * @tparam P  Primitive type.
 * @tparam BV Bounding volume type.
 * @tparam K  Tree branching factor.
 * @param[in] a_primsAndBVs Input primitives and their bounding volumes (consumed).
 * @return K-element array of sub-lists.
 */
template <class P, class BV, size_t K>
using Partitioner = std::function<std::array<PrimAndBVList<P, BV>, K>(PrimAndBVList<P, BV> a_primsAndBVs)>;

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
using LeafPredicate = std::function<bool(const TreeBVH<T, P, BV, K>& a_node)>;

/**
 * @brief Leaf-evaluation callback for TreeBVH::traverse.
 * @details Called once for every leaf node visited during traversal.
 * @tparam P Primitive type.
 * @param[in] a_primitives Primitive list stored in the leaf.
 */
template <class P>
using LeafEvaluator = std::function<void(const PrimitiveList<P>& a_primitives)>;

/**
 * @brief Leaf-evaluation callback for PackedBVH::traverse.
 * @details Receives a view into the global primitive array (offset + count) rather than a
 * temporary sub-list, avoiding a heap allocation per leaf visit. The primitive array's element
 * type depends on the PackedBVH's storage policy (StorageType), not necessarily
 * std::shared_ptr<const P> -- see SharedPtrStorage/ValueStorage.
 * @tparam P             Primitive type.
 * @tparam StoragePolicy PackedBVH storage policy (default: SharedPtrStorage<P>, matching every
 * PackedBVH<T, P, K> that does not name a storage policy explicitly).
 * @param[in] a_primitives Global primitive array (element type StoragePolicy::StorageType).
 * @param[in] a_offset     Index of the first primitive belonging to this leaf.
 * @param[in] a_count      Number of primitives in this leaf.
 */
template <class P, class StoragePolicy = SharedPtrStorage<P>>
using PackedLeafEvaluator = std::function<void(
  const std::vector<typename StoragePolicy::StorageType>& a_primitives, size_t a_offset, size_t a_count)>;

/**
 * @brief Node-visit predicate for BVH traversal.
 * @details Must return true to descend into the node and false to prune it.
 * @tparam NodeType Node type (TreeBVH or PackedBVH::Node).
 * @tparam NodeKey  Caller-supplied per-node key attached to each stack entry
 * (e.g. a running minimum distance).
 * @param[in] a_node Node under consideration.
 * @param[in] a_nodeKey Caller-supplied key for this stack entry.
 * @return True if the subtree rooted at a_node should be visited.
 */
template <class NodeType, class NodeKey>
using PrunePredicate = std::function<bool(const NodeType& a_node, const NodeKey& a_nodeKey)>;

/**
 * @brief Child-ordering callback for TreeBVH traversal.
 * @details Sorts an array of (child-node-pointer, key) pairs in-place so that
 * the most promising child is visited first.
 * @tparam NodeType Node type (TreeBVH).
 * @tparam NodeKey  Per-node key attached to each stack entry.
 * @tparam K        Tree branching factor.
 * @param[in,out] a_children K child nodes together with their node keys.
 */
template <class NodeType, class NodeKey, size_t K>
using ChildOrderer =
  std::function<void(std::array<std::pair<std::shared_ptr<const NodeType>, NodeKey>, K>& a_children)>;

/**
 * @brief Child-ordering callback for PackedBVH traversal.
 * @details Same role as ChildOrderer but uses 32-bit node indices instead of shared_ptrs,
 * halving the stack-entry size.
 * @tparam NodeKey Per-node key attached to each stack entry.
 * @tparam K    Tree branching factor.
 * @param[in,out] a_children K (node-index, key) pairs to sort.
 */
template <class NodeKey, size_t K>
using PackedChildOrderer = std::function<void(std::array<std::pair<uint32_t, NodeKey>, K>& a_children)>;

/**
 * @brief Node-key factory called once per node during BVH traversal.
 * @details Produces the NodeKey value that will be passed to PrunePredicate and ChildOrderer for
 * each child of the current node.
 * @tparam NodeType Node type (TreeBVH or PackedBVH::Node).
 * @tparam NodeKey  Per-node key type to produce.
 * @param[in] a_node Current node.
 * @return NodeKey value for a_node's children.
 */
template <class NodeType, class NodeKey>
using NodeKeyFactory = std::function<NodeKey(const NodeType& a_node)>;

/**
 * @brief Utility: split a vector into K almost-equal contiguous chunks.
 * @tparam X Element type.
 * @tparam K Number of chunks.
 * @param[in] a_primitives Input vector.
 * @return Array of K sub-vectors whose sizes differ by at most 1.
 */
template <class X, size_t K>
auto EqualCounts = [](std::vector<X> a_primitives) noexcept -> std::array<std::vector<X>, K> {
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
auto PrimitiveCentroidPartitioner =
  [](PrimAndBVList<P, BV> a_primsAndBVs) noexcept -> std::array<PrimAndBVList<P, BV>, K> {
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
auto BVCentroidPartitioner = [](PrimAndBVList<P, BV> a_primsAndBVs) -> std::array<PrimAndBVList<P, BV>, K> {
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
auto BinnedSAHPartitioner = [](PrimAndBVList<P, BV> a_primsAndBVs) -> std::array<PrimAndBVList<P, BV>, K> {
  EBGEOMETRY_EXPECT(!a_primsAndBVs.empty());

  // The input is taken by value; partition it in place (no working copy). SAHKWaySplit reorders it
  // and yields K [begin, end) index ranges, which we move out (disjoint, each moved once).
  std::vector<std::pair<size_t, size_t>> groups;
  groups.reserve(K);

  SAHKWaySplit<T, P, BV>(a_primsAndBVs, 0, a_primsAndBVs.size(), K, groups);

  std::array<PrimAndBVList<P, BV>, K> result;
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
  const size_t mid    = std::max(a_begin + K1, std::min(a_end - K2, rawMid));

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
auto MidpointPartitioner = [](PrimAndBVList<P, BV> a_primsAndBVs) -> std::array<PrimAndBVList<P, BV>, K> {
  EBGEOMETRY_EXPECT(!a_primsAndBVs.empty());

  // The input is taken by value; partition it in place (no working copy). MidpointKWaySplit reorders
  // it and yields K [begin, end) index ranges, which we move out (disjoint, each moved once). This
  // is the crux of the partitioner's cost: the midpoint split itself is a couple of std::partition
  // passes, so the plumbing (copies) is what dominated before -- now eliminated.
  std::vector<std::pair<size_t, size_t>> groups;
  groups.reserve(K);

  MidpointKWaySplit<T, P, BV>(a_primsAndBVs, 0, a_primsAndBVs.size(), K, groups);

  std::array<PrimAndBVList<P, BV>, K> result;
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
 * @tparam P  Primitive type. Must provide getCentroid() -- construction/partitioning never
 * calls any other method on it. PackedBVH itself imposes no interface requirement on P either;
 * any further requirement comes entirely from whatever leaf-eval a caller passes to
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
  ~TreeBVH() noexcept;

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
   * @details The stop criterion and partitioner determine the tree shape.
   * @param[in] a_partitioner Partitioning function. Divides a (primitive, BV) list into K sub-lists.
   * @param[in] a_stopCrit    Stop function. Returns true when a node should become a leaf.
   */
  inline void
  topDownSortAndPartition(const Partitioner&   a_partitioner = BVCentroidPartitioner<T, P, BV, K>,
                          const LeafPredicate& a_stopCrit    = DefaultLeafPredicate<T, P, BV, K>);

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
   * @brief Flatten this tree into a cache-friendly PackedBVH with the same primitive type.
   * @details Requires BV == AABBT<T>; enforced by static_assert at instantiation.
   * @tparam StoragePolicy Storage policy for the resulting PackedBVH's primitive array (default:
   * BVH::SharedPtrStorage<P>, matching every existing caller that does not name one explicitly).
   * @return Shared pointer to the resulting PackedBVH.
   */
  template <class StoragePolicy = BVH::SharedPtrStorage<P>>
  [[nodiscard]] inline std::shared_ptr<PackedBVH<T, P, K, StoragePolicy>>
  pack() const;

  /**
   * @brief Flatten and convert this tree into a PackedBVH with a different primitive type Q.
   * @details a_converter is called once per leaf:
   * a_converter(leafPrims, 0, count) → std::vector<Q>
   * All returned values are accumulated into a single contiguous buffer, then exposed
   * through the resulting PackedBVH's storage policy (StorageType) to preserve cache locality.
   * Requires BV == AABBT<T>; enforced by static_assert at instantiation.
   * @tparam Q             Destination primitive type.
   * @tparam Converter     Callable: (PrimitiveList<P>, uint32_t offset, uint32_t count) → std::vector<Q>.
   * @tparam StoragePolicy Storage policy for the resulting PackedBVH's primitive array (default:
   * BVH::SharedPtrStorage<Q>, matching every existing caller that does not name one explicitly).
   * @param[in] a_converter Leaf-conversion function.
   * @return Shared pointer to the resulting PackedBVH<T, Q, K, StoragePolicy>.
   */
  template <class Q, class Converter, class StoragePolicy = BVH::SharedPtrStorage<Q>>
  [[nodiscard]] inline std::shared_ptr<PackedBVH<T, Q, K, StoragePolicy>>
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
 * PackedBVH imposes no interface requirement on P by itself -- it holds primitives opaquely and
 * only ever hands them back to a caller-supplied callback (traverse()'s LeafEvaluator, or
 * pruneTraverse()'s LeafEvaluator). Nearest-surface (signed-distance-style) queries are not a
 * PackedBVH member; callers build their own thin wrapper around pruneTraverse(), supplying
 * whatever primitive interface their own query needs.
 *
 * SIMD paths are selected at compile time via if constexpr, in pruneTraverse():
 * - K==4, T==float  → SSE4.1 (__m128)
 * - K==4, T==double → AVX    (__m256d)
 * - K==8, T==float  → AVX    (__m256)
 * - K==8, T==double → AVX    (two __m256d passes)
 * All other combinations fall back to scalar traversal.
 *
 * What StoragePolicy governs: purely the representation of PackedBVH's own primitive array
 * (StorageType -- std::shared_ptr<const P> by default, or a raw P with ValueStorage<P>). It has
 * no effect on TreeBVH (always shared_ptr-based) or on tree construction/traversal -- see
 * SharedPtrStorage/ValueStorage above for the exact operations a storage policy provides.
 *
 * @tparam T             Floating-point precision.
 * @tparam P             Primitive type.
 * @tparam K             BVH branching factor.
 * @tparam StoragePolicy Governs how the primitive array is stored (default: SharedPtrStorage<P>).
 */
template <class T, class P, size_t K, class StoragePolicy>
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
   * @brief Storage representation of one entry in the primitive array, as determined by
   * StoragePolicy (std::shared_ptr<const P> for the default SharedPtrStorage<P>, or P itself for
   * ValueStorage<P>).
   */
  using StorageType = typename StoragePolicy::StorageType;

  /**
   * @brief Compact BVH node stored in the flat node array.
   * @details Each node holds a bounding volume, a primitive range (leaves) or child
   * index table (interior nodes). Exposed as a public nested type so that users can
   * write traverse() callbacks (PrunePredicate, NodeKeyFactory) that inspect nodes.
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

    /**
     * @brief Get the squared distance from a_point to this node's bounding volume.
     * @details Avoids the sqrt that getDistanceToBoundingVolume() pays. pruneTraverse()'s
     * scalar-fallback branch-and-bound compares against a squared pruning bound, so it uses this
     * directly rather than taking a square root only to square it again.
     * @param[in] a_point Query point.
     * @return Squared distance to the bounding-box surface, or zero if inside.
     */
    [[nodiscard]] inline T
    getDistanceToBoundingVolume2(const Vec3T<T>& a_point) const noexcept
    {
      return m_bv.getDistance2(a_point);
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
   * the leaf.  All returned vectors are stored contiguously in one buffer, which this
   * PackedBVH's storage policy then materialises into its own primitive array -- via aliased
   * @c shared_ptr (no extra copies) for the default SharedPtrStorage<P>, or by taking ownership
   * of the buffer directly for ValueStorage<P>.
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
   * @brief Construct directly from a flat primitive list, without ever building a TreeBVH.
   * @details Bypasses TreeBVH entirely: no per-node shared_ptr<TreeBVH> allocation, and (with
   * StoragePolicy = ValueStorage<P>) no per-primitive shared_ptr allocation either. Primitives are
   * sorted along the space-filling curve @p S (same normalization as
   * TreeBVH::bottomUpSortAndPartition(), via SFC::computeBins()), then cut into leaves by one
   * linear left-to-right scan at @p a_targetLeafSize -- unlike
   * TreeBVH::bottomUpSortAndPartition(), which derives a leaf count of K^floor(log_K(N)) purely
   * from N and K, this lets the caller control leaf size directly.
   *
   * Because the resulting leaf count generally isn't a power of K, the K-ary merge pads up to the
   * next power of K by re-using the last real leaf's index in place of a missing child -- so every
   * interior node still has exactly K children (no change to Node's shape or to traverse()/
   * pruneTraverse(), which assume this), at the cost of that one leaf's primitives potentially
   * being visited more than once by a query in the (bounded, rare) case where the real leaf count
   * isn't already a power of K. This never duplicates primitive data, only (cheap) Node entries.
   *
   * @tparam S Space-filling curve type (e.g. SFC::Morton, SFC::Nested). Defaults to SFC::Morton;
   * a constructor template's own parameters cannot be explicitly specified the way a named
   * function template's can (constructors have no name of their own to attach a template-argument
   * list to), so @p a_sfc is a stateless tag value purely so @p S can be deduced from it -- pass
   * e.g. @c SFC::Nested{} to select a different curve, or omit it entirely for the default.
   * @param[in] a_primsAndBVs   Primitives and their bounding volumes, taken by value (a sink
   * parameter the caller can std::move in) -- never requires shared_ptr-wrapping regardless of
   * this PackedBVH's StoragePolicy.
   * @param[in] a_targetLeafSize Target number of primitives per leaf. Must be > 0.
   * @param[in] a_sfc Unused tag value; see @p S.
   */
  template <class S = SFC::Morton>
  inline PackedBVH(std::vector<std::pair<P, BV>> a_primsAndBVs, size_t a_targetLeafSize, S a_sfc = S{});

  /**
   * @brief Construct directly from a flat primitive list via top-down (optionally SAH) recursive
   * partitioning, without ever building a TreeBVH.
   * @details Reuses the existing (shared_ptr-based) Partitioner/LeafPredicate machinery --
   * BVCentroidPartitioner, BinnedSAHPartitioner, PrimitiveCentroidPartitioner, or any
   * caller-supplied one -- exactly as TreeBVH::topDownSortAndPartition() does, but writes nodes
   * directly into m_linearNodes in depth-first pre-order as the recursion unwinds, instead of
   * building a persistent, shared_ptr<TreeBVH>-linked tree first. Unlike the SFC-build
   * constructor above, no relayout pass is needed here: top-down recursion visits the root before
   * its children, matching PackedBVH's "root at index 0" invariant for free.
   *
   * This still shared_ptr-wraps each primitive once up front (needed to reuse the existing
   * Partitioner/LeafPredicate signatures, which operate on PrimAndBVList), and a lightweight,
   * stack-local TreeBVH is constructed (and immediately discarded) at every split purely to
   * evaluate the LeafPredicate and read off its primitive list -- exactly the same primitive-handle
   * copying TreeBVH::topDownSortAndPartition() already does at every node. What this constructor
   * avoids is the *persistent*, heap-allocated shared_ptr<TreeBVH> node kept alive for the tree's
   * lifetime at every level -- measured as the dominant cost of the traditional
   * build-then-pack() path (see the "Direct construction" section of the Sphinx docs).
   *
   * @param[in] a_primsAndBVs Primitives and their bounding volumes, taken by value (a sink
   * parameter the caller can std::move in) -- never requires shared_ptr-wrapping by the caller,
   * regardless of this PackedBVH's StoragePolicy.
   * @param[in] a_partitioner Partitioning function. Divides a (primitive, BV) list into K
   * sub-lists. Defaults to BVCentroidPartitioner; pass BinnedSAHPartitioner for an SAH build.
   * @param[in] a_stopCrit Stop function. Returns true when a node should become a leaf. Defaults
   * to DefaultLeafPredicate.
   */
  inline PackedBVH(std::vector<std::pair<P, BV>>          a_primsAndBVs,
                   const BVH::Partitioner<P, BV, K>&      a_partitioner = BVCentroidPartitioner<T, P, BV, K>,
                   const BVH::LeafPredicate<T, P, BV, K>& a_stopCrit    = DefaultLeafPredicate<T, P, BV, K>);

  /**
   * @brief Construct directly via ClusterSAH: cluster primitives, then SAH over the clusters.
   * @details A single-threaded build that gets close to full-SAH tree quality at a fraction of the
   * build cost, by shrinking what SAH has to partition. In two passes, both keeping @p P by value
   * (no shared_ptr anywhere): (1) group the primitives into buckets of at most
   * @c a_spec.maxClusterSize each, using a cheap density-adaptive midpoint subdivision that stops as
   * soon as a bucket is small enough -- so buckets are spatially tight and follow the primitive
   * density (robust on surface/clustered data, where a fixed Cartesian grid would overcrowd); (2) run
   * binned SAH top-down over the @em buckets (their bounding boxes) to build the flat node array,
   * with each leaf holding one or a few buckets' primitives. SAH thus partitions ~N/maxClusterSize
   * boxes rather than all N primitives -- the source of the speedup. Requires BV == AABBT<T>;
   * enforced by static_assert at instantiation. Disambiguated from the SFC-build constructor by the
   * @c ClusterSpec parameter type.
   * @param[in] a_primsAndBVs Primitives and their bounding volumes, taken by value (a sink parameter
   * the caller can std::move in) -- never requires shared_ptr-wrapping regardless of StoragePolicy.
   * @param[in] a_spec Clustering configuration (bucket size). See ClusterSpec.
   */
  inline PackedBVH(std::vector<std::pair<P, BV>> a_primsAndBVs, BVH::ClusterSpec a_spec);

  /**
   * @brief Destructor.
   * @details Not virtual: PackedBVH is not intended to be subclassed or used polymorphically.
   */
  inline ~PackedBVH() = default;

  /**
   * @brief Copy constructor.
   * @details Explicitly defaulted for documentation purposes: unlike TreeBVH, PackedBVH's
   * members (m_linearNodes, m_primitives, m_childAabbSoA) are all owned value containers with no
   * shared mutable substructure, so the implicitly-generated deep copy is correct and safe under
   * both BVH::SharedPtrStorage (primitives are aliased shared_ptr, the same sharing model as
   * TreeBVH) and BVH::ValueStorage (primitives are copied by value -- safe as long as the
   * primitive type's own copy constructor is complete; see DCEL::FaceT's copy-constructor
   * documentation for a case where it deliberately is not, which is why MeshSDF never uses
   * BVH::ValueStorage).
   * @param[in] a_other Other instance to copy.
   */
  PackedBVH(const PackedBVH& a_other) = default;

  /**
   * @brief Copy assignment operator.
   * @param[in] a_other Other instance to copy.
   * @return Reference to *this.
   */
  PackedBVH&
  operator=(const PackedBVH& a_other) = default;

  /**
   * @brief Move constructor.
   * @details Explicitly defaulted: the user-declared destructor above would otherwise suppress
   * the implicitly-generated move constructor.
   * @param[in,out] a_other Other instance to move from.
   */
  PackedBVH(PackedBVH&& a_other) noexcept = default;

  /**
   * @brief Move assignment operator.
   * @param[in,out] a_other Other instance to move from.
   * @return Reference to *this.
   */
  PackedBVH&
  operator=(PackedBVH&& a_other) noexcept = default;

  /**
   * @brief Get the global primitive list (in leaf-traversal order).
   * @return Reference to m_primitives.
   */
  [[nodiscard]] inline const std::vector<StorageType>&
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
   * (the root) paired with @p a_nodeKeyFactory applied to the root node. On each iteration:
   *
   * 1. Pop the back entry to obtain a (nodeIdx, nodeKey) pair.
   * 2. Look up the node at m_linearNodes[nodeIdx].
   * 3. Call @p a_prunePredicate(node, nodeKey). If it returns false the entire subtree rooted at
   * that node is skipped (pruned) and the loop continues.
   * 4. If the node is a leaf, call @p a_leafEvaluator with the global primitive list
   * m_primitives, the leaf's primitive offset, and its primitive count. The leafEvaluator
   * receives a view into the shared list rather than a freshly allocated sub-list,
   * avoiding a heap allocation per leaf visit.
   * 5. If the node is an interior node:
   * a. Collect the K child indices from node.getChildOffsets(), look each child up in
   * m_linearNodes, and call @p a_nodeKeyFactory on each to produce a NodeKey value.
   * b. Bundle the K (childIdx, NodeKey) pairs into a local array and pass it to
   * @p a_childOrderer, which reorders the array in-place.
   * c. Push all K pairs onto the back of the stack in sorted order.
   *
   * Because the stack is LIFO, the child pushed last is visited first. @p a_childOrderer
   * should therefore place the most promising child last in the array. For a
   * nearest-distance query this means sorting children in descending order of
   * distance — farthest child first, nearest child last — so the nearest child
   * sits at the back of the vector and is expanded next.
   *
   * @tparam NodeKey Auxiliary data type carried on the traversal stack (e.g. a running minimum distance).
   * @param[in] a_leafEvaluator     Called at each leaf with the global primitive list, offset, and count.
   * @param[in] a_prunePredicate     Called at each node; return true to descend, false to prune.
   * @param[in] a_childOrderer      Reorders the K (childIdx, NodeKey) pairs in-place before they are
   * pushed; the last element after sorting is visited first.
   * @param[in] a_nodeKeyFactory Produces a NodeKey value for a node; called once for the root
   * and once per child of every interior node that is visited.
   */
  template <class NodeKey>
  inline void
  traverse(const BVH::PackedLeafEvaluator<P, StoragePolicy>& a_leafEvaluator,
           const BVH::PrunePredicate<Node, NodeKey>&         a_prunePredicate,
           const BVH::PackedChildOrderer<NodeKey, K>&        a_childOrderer,
           const BVH::NodeKeyFactory<Node, NodeKey>&         a_nodeKeyFactory) const noexcept;

  /**
   * @brief Generic SIMD-accelerated, distance-pruned traversal.
   * @details Same box-pruning strategy as @c traverse() above (skip subtrees already farther than
   * the current best; visit the closest-looking child first), but the box-vs-point distance test
   * is vectorised across all @c K children at once (@c if @c constexpr dispatch on @c (K, T);
   * falls back to the generic @c traverse() above when no compiled ISA path matches), and the
   * search itself is expressed through three caller-supplied pieces instead of four fixed
   * callbacks. @c State is whatever the search remembers between leaf visits. @c LeafEvaluator is
   * called only at leaves and is the sole place @c State may change. @c PruneDistSquared turns the
   * *current* @c State into a squared-distance pruning bound; it is re-read fresh at every node
   * visited (never cached), so a leaf visited anywhere earlier on the stack immediately tightens
   * the pruning applied to nodes visited afterwards, regardless of which subtree it came from.
   * Splitting the bound from the leaf scan like this is what lets a primitive with no notion of
   * "signed distance" reuse the same SIMD box test -- e.g. a nearest-neighbor search can track a
   * plain running squared distance in @c State (no @c abs(), no sqrt anywhere in the hot path),
   * while MeshSDF::signedDistance() and TriMeshSDF::signedDistance() track a signed value and
   * square its magnitude for the bound. Both are ordinary instantiations of this one method.
   *
   * @note Only the pruning *bound* (@c PruneDistSquared) is customisable; the per-child quantity it is
   * compared against -- Euclidean squared distance to a child's AABB -- is not, since that is the
   * one computation vectorised uniformly across @c K children, and branch-and-bound pruning is
   * only sound if it is a true lower bound on the distance to anything inside that box. PackedBVH
   * also hardcodes AABBT<T> as its sole bounding volume. See
   * https://github.com/rmrsk/EBGeometry/issues/96 for a sketch of what generalizing this would
   * look like, if that is ever needed.
   *
   * @tparam State        Caller-defined running search state, carried by reference through the whole traversal.
   * @tparam LeafEvaluator Callable: (State&, size_t offset, size_t count) noexcept -> void. Scans
   * primitives [offset, offset+count) and updates a_state in place.
   * @tparam PruneDistSquared    Callable: (const State&) noexcept -> T. Returns the current squared-distance
   * pruning bound derived from a_state; a node farther than this (in squared distance) is pruned.
   * @param[in]     a_point      Query point.
   * @param[in,out] a_state      Running search state; mutated by a_evalLeaf, read by a_pruneDist2.
   * @param[in]     a_evalLeaf   Leaf-visit callback.
   * @param[in]     a_pruneDist2 Pruning-bound callback.
   */
  template <class State, class LeafEvaluator, class PruneDistSquared>
  inline void
  pruneTraverse(const Vec3T<T>&    a_point,
                State&             a_state,
                LeafEvaluator&&    a_evalLeaf,
                PruneDistSquared&& a_pruneDist2) const noexcept;

protected:
  /**
   * @brief Flat depth-first node array.
   */
  std::vector<Node> m_linearNodes;

  /**
   * @brief Global primitive list in leaf-traversal order.
   */
  std::vector<StorageType> m_primitives;

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
   * @brief Per-node SoA AABB cache used by the SIMD traversal in pruneTraverse().
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
