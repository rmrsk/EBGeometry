// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file    EBGeometry_PointCloudBVHParallelImplem.hpp
 * @brief   Implementation of PointCloudBVH::buildParallel(): a Morton-sorted, perfect K-ary tree built
 *          with GPU::parallelFor().
 * @details Every step is a functor run over one index per thread, so the same code runs as host loops
 *          in a host translation unit and as kernels in a CUDA or HIP one:
 *          1. a bounding box of the cloud (per chunk, then one thread merging the chunks);
 *          2. a Morton key per point, padded to a power of two with keys that sort last;
 *          3. a bitonic sort of the keys;
 *          4. the point groups (W consecutive sorted points each) and the per-point tables;
 *          5. the nodes, each written from its position in the perfect tree;
 *          6. the boxes, the leaves' from their points and then one level of interior nodes at a time.
 *          The tree's shape is fixed by the point count alone, which is what lets every node find its
 *          depth-first index, children and primitive range without a scan.
 * @author  Robert Marskar
 */

#ifndef EBGEOMETRY_POINTCLOUDBVHPARALLELIMPLEM_HPP
#define EBGEOMETRY_POINTCLOUDBVHPARALLELIMPLEM_HPP

// Std includes
#include <cstdint>
#include <vector>

// Our includes
#include "EBGeometry_Array.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_GPUTransform.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_PointCloudBVH.hpp"
#include "EBGeometry_PointCloudDetail.hpp"
#include "EBGeometry_SFC.hpp"

namespace EBGeometry {

/**
 * @brief The functors and tree arithmetic of PointCloudBVH::buildParallel().
 */
namespace PointCloudBVHParallel {

/**
 * @brief A point's Morton code and cloud index, ordered by code and then by index.
 */
struct Key
{
  /// @brief Morton code of the quantized position; UINT64_MAX for padding.
  uint64_t m_code;

  /// @brief Cloud index; UINT32_MAX for padding.
  uint32_t m_index;
};

/**
 * @brief Strict order on keys: by code, ties by index, so that the sort's result is unique.
 * @param[in] a_lhs Left key.
 * @param[in] a_rhs Right key.
 * @return True if @p a_lhs sorts before @p a_rhs.
 */
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline bool
less(const Key& a_lhs, const Key& a_rhs) noexcept
{
  return (a_lhs.m_code < a_rhs.m_code) || (a_lhs.m_code == a_rhs.m_code && a_lhs.m_index < a_rhs.m_index);
}

/**
 * @brief The shape of a perfect K-ary tree of depth d over G point groups.
 * @details The tree has L = K^d leaves, at most one per group, and leaf j holds the groups
 * [floor(j G / L), floor((j + 1) G / L)): every leaf at least one, and none more than one above
 * another. Nodes are numbered two ways: level by level (the root is 0, its children 1 to K, ...),
 * which is how threads are assigned and how interior nodes number their child-box rows, and depth
 * first, which is where PackedBVH stores them.
 * @tparam K Branching factor.
 */
template <size_t K>
struct TreeShape
{
  /// @brief Depth: the leaves are on level m_depth.
  uint32_t m_depth;

  /// @brief Number of point groups.
  uint64_t m_numGroups;

  /// @brief Number of leaves, K^m_depth.
  uint64_t m_numLeaves;

  /**
   * @brief K to the power @p a_e.
   * @param[in] a_e Exponent.
   * @return K^a_e.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline uint64_t
  power(const uint32_t a_e) noexcept
  {
    uint64_t p = 1;

    for (uint32_t i = 0; i < a_e; i++) {
      p *= K;
    }

    return p;
  }

  /**
   * @brief Nodes in a perfect tree of height @p a_h (a single node has height 0).
   * @param[in] a_h Height.
   * @return (K^(a_h + 1) - 1) / (K - 1).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline uint64_t
  subtreeSize(const uint32_t a_h) noexcept
  {
    return (power(a_h + 1) - 1) / (K - 1);
  }

  /**
   * @brief Level-order index of the first node on level @p a_level.
   * @param[in] a_level Level.
   * @return (K^a_level - 1) / (K - 1).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline uint64_t
  levelBegin(const uint32_t a_level) noexcept
  {
    return (power(a_level) - 1) / (K - 1);
  }

  /**
   * @brief Number of nodes.
   * @return subtreeSize(m_depth).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint64_t
  numNodes() const noexcept
  {
    return subtreeSize(m_depth);
  }

  /**
   * @brief Number of interior nodes.
   * @return The nodes above the leaf level.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint64_t
  numInterior() const noexcept
  {
    return levelBegin(m_depth);
  }

  /**
   * @brief Depth-first index of node @p a_j of level @p a_level.
   * @details The base-K digits of @p a_j, most significant first, name the child taken at each level
   * on the way down, and taking child a below a node skips a subtrees of the next level's height.
   * @param[in] a_level Level.
   * @param[in] a_j     Position on the level, left to right.
   * @return Depth-first index.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint64_t
  depthFirst(const uint32_t a_level, const uint64_t a_j) const noexcept
  {
    uint64_t index = 0;
    uint64_t scale = power(a_level);

    for (uint32_t m = 1; m <= a_level; m++) {
      scale /= K;

      const uint64_t digit = (a_j / scale) % K;

      index += 1 + digit * subtreeSize(m_depth - m);
    }

    return index;
  }

  /**
   * @brief First group of leaf @p a_leaf; firstGroup(m_numLeaves) is the group count.
   * @param[in] a_leaf Leaf, 0 to m_numLeaves.
   * @return floor(a_leaf G / L).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint64_t
  firstGroup(const uint64_t a_leaf) const noexcept
  {
    return a_leaf * m_numGroups / m_numLeaves;
  }

  /**
   * @brief The leaf holding group @p a_group.
   * @details The largest j with firstGroup(j) <= a_group, that is ceil((a_group + 1) L / G) - 1.
   * @param[in] a_group Group.
   * @return Leaf.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint64_t
  leafOf(const uint64_t a_group) const noexcept
  {
    return ((a_group + 1) * m_numLeaves + m_numGroups - 1) / m_numGroups - 1;
  }
};

/**
 * @brief Step 1a: the bounding box of each chunk of points.
 * @tparam T Precision.
 */
template <class T>
struct ChunkBoxes
{
  /// @brief Point positions.
  PODVector<Vec3T<T>> m_positions;

  /// @brief Chunk lower corners (scratch).
  PODVector<Vec3T<T>> m_lo;

  /// @brief Chunk upper corners (scratch).
  PODVector<Vec3T<T>> m_hi;

  /// @brief Base of the pool holding m_positions.
  void* m_base;

  /// @brief Base of the scratch pool.
  void* m_scratch;

  /// @brief Number of points.
  uint32_t m_numPoints;

  /// @brief Points per chunk.
  uint32_t m_chunk;

  /**
   * @brief Box of chunk @p a_c.
   * @param[in] a_c Chunk.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_c) const noexcept
  {
    const uint32_t begin = static_cast<uint32_t>(a_c) * m_chunk;
    const uint32_t end   = Math::min(m_numPoints, begin + m_chunk);

    Vec3T<T> lo = +Vec3T<T>::max();
    Vec3T<T> hi = -Vec3T<T>::max();

    for (uint32_t i = begin; i < end; i++) {
      const Vec3T<T>& p = m_positions.at(m_base, i);

      lo = min(lo, p);
      hi = max(hi, p);
    }

    m_lo.at(m_scratch, static_cast<uint32_t>(a_c)) = lo;
    m_hi.at(m_scratch, static_cast<uint32_t>(a_c)) = hi;
  }
};

/**
 * @brief Step 1b: one thread merges the chunk boxes into element 0.
 * @tparam T Precision.
 */
template <class T>
struct MergeBoxes
{
  /// @brief Chunk lower corners (scratch).
  PODVector<Vec3T<T>> m_lo;

  /// @brief Chunk upper corners (scratch).
  PODVector<Vec3T<T>> m_hi;

  /// @brief Base of the scratch pool.
  void* m_scratch;

  /**
   * @brief Merge every chunk's box.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(std::size_t /*a_thread*/) const noexcept
  {
    Vec3T<T> lo = m_lo.at(m_scratch, 0);
    Vec3T<T> hi = m_hi.at(m_scratch, 0);

    for (uint32_t c = 1; c < m_lo.size(); c++) {
      lo = min(lo, m_lo.at(m_scratch, c));
      hi = max(hi, m_hi.at(m_scratch, c));
    }

    m_lo.at(m_scratch, 0) = lo;
    m_hi.at(m_scratch, 0) = hi;
  }
};

/**
 * @brief Step 2: the Morton key of every point, and padding keys after the last point.
 * @tparam T Precision.
 */
template <class T>
struct MortonKeys
{
  /// @brief Point positions.
  PODVector<Vec3T<T>> m_positions;

  /// @brief Cloud box, in element 0 of each (scratch).
  PODVector<Vec3T<T>> m_lo;

  /// @brief Cloud box upper corner, in element 0 (scratch).
  PODVector<Vec3T<T>> m_hi;

  /// @brief Keys, a power of two of them (scratch).
  PODVector<Key> m_keys;

  /// @brief Base of the pool holding m_positions.
  void* m_base;

  /// @brief Base of the scratch pool.
  void* m_scratch;

  /// @brief Number of points.
  uint32_t m_numPoints;

  /**
   * @brief Key @p a_i.
   * @param[in] a_i Key index.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_i) const noexcept
  {
    const auto i = static_cast<uint32_t>(a_i);

    Key& key = m_keys.at(m_scratch, i);

    if (i >= m_numPoints) {
      key.m_code  = UINT64_MAX;
      key.m_index = UINT32_MAX;

      return;
    }

    const Vec3T<T>& lo = m_lo.at(m_scratch, 0);
    const Vec3T<T>& hi = m_hi.at(m_scratch, 0);
    const Vec3T<T>& p  = m_positions.at(m_base, i);

    constexpr T span = static_cast<T>(SFC::ValidSpan);

    SFC::Index cell;

    // Halved before subtracting: a float cloud spanning more than the largest float has an extent of
    // infinity, which would give 0 or NaN here. The ratio is the same.
    for (int dir = 0; dir < 3; dir++) {
      const T extent = T(0.5) * hi[dir] - T(0.5) * lo[dir];
      const T scaled = (extent > T(0)) ? (T(0.5) * p[dir] - T(0.5) * lo[dir]) / extent * span : T(0);

      cell[dir] = static_cast<unsigned int>(Math::clamp(scaled, T(0), span));
    }

    key.m_code  = SFC::Morton::encode(cell);
    key.m_index = i;
  }
};

/**
 * @brief Step 3: one compare-exchange pass of a bitonic sort, one thread per pair.
 */
struct BitonicPass
{
  /// @brief Keys (scratch).
  PODVector<Key> m_keys;

  /// @brief Base of the scratch pool.
  void* m_scratch;

  /// @brief Size of the bitonic sequences being merged.
  uint32_t m_k;

  /// @brief Distance between the two keys of a pair.
  uint32_t m_j;

  /**
   * @brief Pair @p a_t.
   * @param[in] a_t Pair index, below half the key count.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_t) const noexcept
  {
    const auto     t = static_cast<uint32_t>(a_t);
    const uint32_t i = (t / m_j) * 2 * m_j + t % m_j;
    const uint32_t l = i + m_j;

    Key& a = m_keys.at(m_scratch, i);
    Key& b = m_keys.at(m_scratch, l);

    const bool ascending = (i & m_k) == 0;

    if (ascending ? less(b, a) : less(a, b)) {
      const Key swap = a;

      a = b;
      b = swap;
    }
  }
};

/**
 * @brief Step 4a: pack point group @p g from sorted points g W to g W + W - 1.
 * @tparam T Precision.
 * @tparam K Branching factor.
 * @tparam W Group width.
 */
template <class T, size_t K, size_t W>
struct PackGroups
{
  /// @brief The BVH's arrays.
  typename PointCloudBVH<T, K, W>::Packed::Layout m_bvh;

  /// @brief Point positions.
  PODVector<Vec3T<T>> m_positions;

  /// @brief Sorted keys (scratch).
  PODVector<Key> m_keys;

  /// @brief Base of the pool holding the BVH and the positions.
  void* m_base;

  /// @brief Base of the scratch pool.
  void* m_scratch;

  /// @brief Number of points.
  uint32_t m_numPoints;

  /**
   * @brief Group @p a_g.
   * @param[in] a_g Group.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_g) const noexcept
  {
    const uint32_t begin = static_cast<uint32_t>(a_g * W);

    // Every group starts before the last point, so the count is at least 1. Saying so lets GCC see that
    // packUnchecked() never reads lane count - 1 = UINT32_MAX, which it otherwise warns about.
    const uint32_t count = Math::max(1U, Math::min(static_cast<uint32_t>(W), m_numPoints - begin));

    Array<Vec3T<T>, W> positions{};
    Array<uint32_t, W> ids{};

    for (uint32_t s = 0; s < count; s++) {
      ids[s]       = m_keys.at(m_scratch, begin + s).m_index;
      positions[s] = m_positions.at(m_base, ids[s]);
    }

    m_bvh.m_primitives.at(m_base, static_cast<uint32_t>(a_g)).packUnchecked(positions.data(), ids.data(), count);
  }
};

/**
 * @brief Step 4b: the per-point tables -- the leaf order, and each point's own-leaf group range.
 * @tparam K Branching factor.
 * @tparam W Group width.
 */
template <size_t K, size_t W>
struct PointTables
{
  /// @brief Tree shape.
  TreeShape<K> m_shape;

  /// @brief Sorted keys (scratch).
  PODVector<Key> m_keys;

  /// @brief Point indices in leaf order.
  PODVector<uint32_t> m_order;

  /// @brief Per-point own-leaf group offset.
  PODVector<uint32_t> m_leafOff;

  /// @brief Per-point own-leaf group count.
  PODVector<uint32_t> m_leafCnt;

  /// @brief Base of the pool holding the tables.
  void* m_base;

  /// @brief Base of the scratch pool.
  void* m_scratch;

  /**
   * @brief Sorted point @p a_s.
   * @param[in] a_s Position in the sorted order.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_s) const noexcept
  {
    const auto     s     = static_cast<uint32_t>(a_s);
    const uint32_t point = m_keys.at(m_scratch, s).m_index;
    const uint64_t leaf  = m_shape.leafOf(s / W);
    const uint64_t first = m_shape.firstGroup(leaf);

    m_order.at(m_base, s)       = point;
    m_leafOff.at(m_base, point) = static_cast<uint32_t>(first);
    m_leafCnt.at(m_base, point) = static_cast<uint32_t>(m_shape.firstGroup(leaf + 1) - first);
  }
};

/**
 * @brief Step 5: every node's primitive range or children, from its level-order index.
 * @tparam T Precision.
 * @tparam K Branching factor.
 * @tparam W Group width.
 */
template <class T, size_t K, size_t W>
struct WriteNodes
{
  /// @brief The BVH's arrays.
  typename PointCloudBVH<T, K, W>::Packed::Layout m_bvh;

  /// @brief Tree shape.
  TreeShape<K> m_shape;

  /// @brief Base of the pool holding the BVH.
  void* m_base;

  /**
   * @brief Node @p a_t in level order.
   * @param[in] a_t Level-order index.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_t) const noexcept
  {
    using Node = typename PointCloudBVH<T, K, W>::Node;

    const uint64_t t = a_t;

    uint32_t level = 0;

    while (TreeShape<K>::levelBegin(level + 1) <= t) {
      level++;
    }

    const uint64_t j = t - TreeShape<K>::levelBegin(level);

    Node node{};

    if (level == m_shape.m_depth) {
      const uint64_t first = m_shape.firstGroup(j);

      node.setPrimitivesOffset(static_cast<uint32_t>(first));
      node.setNumPrimitives(static_cast<uint32_t>(m_shape.firstGroup(j + 1) - first));
    }
    else {
      for (size_t k = 0; k < K; k++) {
        node.setChildOffset(static_cast<uint32_t>(m_shape.depthFirst(level + 1, j * K + k)), k);
      }

      // Interior nodes are exactly the level-order indices below the leaf level, so t numbers rows.
      node.m_childBoxRow = static_cast<uint32_t>(t);
    }

    m_bvh.m_linearNodes.at(m_base, static_cast<uint32_t>(m_shape.depthFirst(level, j))) = node;
  }
};

/**
 * @brief Step 6a: every leaf's box, from its points.
 * @tparam T Precision.
 * @tparam K Branching factor.
 * @tparam W Group width.
 */
template <class T, size_t K, size_t W>
struct LeafBoxes
{
  /// @brief The BVH's arrays.
  typename PointCloudBVH<T, K, W>::Packed::Layout m_bvh;

  /// @brief Point positions.
  PODVector<Vec3T<T>> m_positions;

  /// @brief Sorted keys (scratch).
  PODVector<Key> m_keys;

  /// @brief Tree shape.
  TreeShape<K> m_shape;

  /// @brief Base of the pool holding the BVH and the positions.
  void* m_base;

  /// @brief Base of the scratch pool.
  void* m_scratch;

  /// @brief Number of points.
  uint32_t m_numPoints;

  /**
   * @brief Leaf @p a_j.
   * @param[in] a_j Leaf, left to right.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_j) const noexcept
  {
    const uint64_t begin = m_shape.firstGroup(a_j) * W;
    const uint64_t end   = Math::min(uint64_t{m_numPoints}, m_shape.firstGroup(a_j + 1) * W);

    Vec3T<T> lo = +Vec3T<T>::max();
    Vec3T<T> hi = -Vec3T<T>::max();

    for (uint64_t s = begin; s < end; s++) {
      const Vec3T<T>& p = m_positions.at(m_base, m_keys.at(m_scratch, static_cast<uint32_t>(s)).m_index);

      lo = min(lo, p);
      hi = max(hi, p);
    }

    const auto leaf = static_cast<uint32_t>(m_shape.depthFirst(m_shape.m_depth, a_j));

    m_bvh.m_linearNodes.at(m_base, leaf).setBoundingVolume(BoundingVolumes::AABBT<T>(lo, hi));
  }
};

/**
 * @brief Step 6b: the boxes and child-box rows of one level of interior nodes, whose children are final.
 * @tparam T Precision.
 * @tparam K Branching factor.
 * @tparam W Group width.
 */
template <class T, size_t K, size_t W>
struct InteriorBoxes
{
  /// @brief The BVH's arrays.
  typename PointCloudBVH<T, K, W>::Packed::Layout m_bvh;

  /// @brief Tree shape.
  TreeShape<K> m_shape;

  /// @brief Base of the pool holding the BVH.
  void* m_base;

  /// @brief Level.
  uint32_t m_level;

  /**
   * @brief Node @p a_j of the level.
   * @param[in] a_j Position on the level.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_j) const noexcept
  {
    using Packed = typename PointCloudBVH<T, K, W>::Packed;

    const auto index = static_cast<uint32_t>(m_shape.depthFirst(m_level, a_j));

    auto& node = m_bvh.m_linearNodes.at(m_base, index);

    BoundingVolumes::AABBT<T> bv;

    for (size_t k = 0; k < K; k++) {
      bv = bv.merged(m_bvh.m_linearNodes.at(m_base, node.getChildOffsets()[k]).getBoundingVolume());
    }

    node.setBoundingVolume(bv);

    Packed::writeChildBoxRow(m_bvh, m_base, index);
  }
};

} // namespace PointCloudBVHParallel

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST
inline PointCloudBVH<T, K, W>::PointCloudBVH(const Pool&                     a_pool,
                                             const typename Packed::Layout&  a_bvh,
                                             const PODVector<Vec3T<T>>&      a_positions,
                                             const PODVector<std::uint32_t>& a_leafOff,
                                             const PODVector<std::uint32_t>& a_leafCnt,
                                             const PODVector<std::uint32_t>& a_order) noexcept
  : m_bvhLayout(a_bvh), m_positions(a_positions), m_leafOff(a_leafOff), m_leafCnt(a_leafCnt), m_order(a_order)
{
  this->attach(a_pool, "PointCloudBVH");
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST
inline PointCloudBVH<T, K, W>
PointCloudBVH<T, K, W>::buildParallel(Pool&                        a_pool,
                                      const std::vector<Vec3T<T>>& a_positions,
                                      std::size_t                  a_targetLeafSize)
{
  using namespace PointCloudBVHParallel;

  static_assert(std::is_floating_point_v<T>, "PointCloudBVH requires a floating-point type T");
  static_assert(K >= 2, "PointCloudBVH requires a branching factor K >= 2");
  static_assert(W >= 1, "PointCloudBVH requires a SIMD width W >= 1");

  PointCloudDetail::requireValidCloud("PointCloudBVH::buildParallel", a_positions);

  EBGEOMETRY_REQUIRE(a_targetLeafSize >= 1,
                     "PointCloudBVH::buildParallel: the target leaf size must be at least 1 (%zu)",
                     a_targetLeafSize);

  // The positions are copied in from the host, and the kernels write every other array in place.
  EBGEOMETRY_REQUIRE(a_pool.resource().isHostAccessible() && (!GPU::enabled || a_pool.resource().isDeviceAccessible()),
                     "PointCloudBVH::buildParallel: the pool must be host-accessible, and device-accessible as "
                     "well in a %s build (managed memory)",
                     GPU::backendName);

  // The keys are padded to a power of two that is counted in 32 bits.
  EBGEOMETRY_REQUIRE(a_positions.size() <= (std::size_t{1} << 31),
                     "PointCloudBVH::buildParallel: at most 2^31 points (%zu)",
                     a_positions.size());

  // No points: the constructor's empty BVH.
  if (a_positions.empty()) {
    return PointCloudBVH(a_pool, a_positions, a_targetLeafSize);
  }

  const auto numPoints = static_cast<uint32_t>(a_positions.size());

  // The shallowest perfect tree whose leaves hold at most about a_targetLeafSize points, never with more
  // leaves than groups, so that no leaf is empty.
  TreeShape<K> shape{0, (uint64_t{numPoints} + W - 1) / W, 1};

  const uint64_t groupsPerLeaf = Math::max(uint64_t{1}, static_cast<uint64_t>(a_targetLeafSize) / W);

  while (shape.m_numLeaves * K <= shape.m_numGroups && shape.m_numLeaves * groupsPerLeaf < shape.m_numGroups) {
    shape.m_numLeaves *= K;
    shape.m_depth++;
  }

  EBGEOMETRY_REQUIRE(shape.numNodes() <= UINT32_MAX,
                     "PointCloudBVH::buildParallel: %llu nodes do not fit the 32-bit indices",
                     static_cast<unsigned long long>(shape.numNodes()));

  const auto numNodes    = static_cast<uint32_t>(shape.numNodes());
  const auto numInterior = static_cast<uint32_t>(shape.numInterior());
  const auto numGroups   = static_cast<uint32_t>(shape.m_numGroups);

  uint32_t numKeys = 1;

  while (numKeys < numPoints) {
    numKeys *= 2;
  }

  constexpr uint32_t chunk     = 256;
  const uint32_t     numChunks = (numPoints + chunk - 1) / chunk;

  // Every reservation before any base is resolved: a reserve can move the block.
  const typename Packed::Layout bvh = Packed::reserveLayout(a_pool, numNodes, numInterior, numGroups);

  PODVector<Vec3T<T>>      positions;
  PODVector<std::uint32_t> leafOff;
  PODVector<std::uint32_t> leafCnt;
  PODVector<std::uint32_t> order;

  positions.reserveFrom(a_pool, numPoints);
  leafOff.reserveSizedFrom(a_pool, numPoints);
  leafCnt.reserveSizedFrom(a_pool, numPoints);
  order.reserveSizedFrom(a_pool, numPoints);

  Pool scratch(a_pool.resource());

  PODVector<Vec3T<T>> lo;
  PODVector<Vec3T<T>> hi;
  PODVector<Key>      keys;

  lo.reserveSizedFrom(scratch, numChunks);
  hi.reserveSizedFrom(scratch, numChunks);
  keys.reserveSizedFrom(scratch, numKeys);

  void* base        = a_pool.base();
  void* scratchBase = scratch.base();

  positions.assign(base, a_positions.data(), numPoints);

  GPU::parallelFor(numChunks, ChunkBoxes<T>{positions, lo, hi, base, scratchBase, numPoints, chunk});
  GPU::parallelFor(1, MergeBoxes<T>{lo, hi, scratchBase});
  GPU::parallelFor(numKeys, MortonKeys<T>{positions, lo, hi, keys, base, scratchBase, numPoints});

  // k != 0: at 2^31 keys, the last k doubles to 0 rather than past numKeys.
  for (uint32_t k = 2; k != 0 && k <= numKeys; k *= 2) {
    for (uint32_t j = k / 2; j > 0; j /= 2) {
      GPU::parallelFor(numKeys / 2, BitonicPass{keys, scratchBase, k, j});
    }
  }

  GPU::parallelFor(numGroups, PackGroups<T, K, W>{bvh, positions, keys, base, scratchBase, numPoints});
  GPU::parallelFor(numPoints, PointTables<K, W>{shape, keys, order, leafOff, leafCnt, base, scratchBase});
  GPU::parallelFor(numNodes, WriteNodes<T, K, W>{bvh, shape, base});
  GPU::parallelFor(shape.m_numLeaves, LeafBoxes<T, K, W>{bvh, positions, keys, shape, base, scratchBase, numPoints});

  for (uint32_t level = shape.m_depth; level-- > 0;) {
    GPU::parallelFor(TreeShape<K>::power(level), InteriorBoxes<T, K, W>{bvh, shape, base, level});
  }

  return PointCloudBVH(a_pool, bvh, positions, leafOff, leafCnt, order);
}

} // namespace EBGeometry

#endif
