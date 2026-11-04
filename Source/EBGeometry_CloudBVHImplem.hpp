// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file    EBGeometry_CloudBVHImplem.hpp
 * @brief   Implementation of EBGeometry_CloudBVH.hpp: the build of PointCloudBVH and SphereCloudBVH, a linear BVH
 *          over Morton-sorted primitive centers, in host loops or through GPU::parallelFor().
 * @author  Robert Marskar
 */

#ifndef EBGEOMETRY_CLOUDBVHIMPLEM_HPP
#define EBGEOMETRY_CLOUDBVHIMPLEM_HPP

// Std includes
#include <algorithm>
#include <cstdint>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_CloudBVH.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_GPUAlgorithms.hpp"
#include "EBGeometry_GPUTransform.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_MemoryResource.hpp"
#include "EBGeometry_PointCloudDetail.hpp"
#include "EBGeometry_SFC.hpp"

namespace EBGeometry {

namespace PointCloudDetail {

/**
 * @brief The breadth-first index marking a leaf's absent children.
 */
inline constexpr uint32_t NoChildren = UINT32_MAX;

/**
 * @brief The position of the highest set bit of @p a_value, which must not be zero.
 * @details Internal helper; not part of the public API.
 * @param[in] a_value The value.
 * @return The bit's position, 0 to 63.
 */
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline uint32_t
highestBit(uint64_t a_value) noexcept
{
  uint32_t bit = 0;

  for (uint32_t shift = 32; shift > 0; shift /= 2) {
    if ((a_value >> shift) != 0) {
      a_value >>= shift;
      bit += shift;
    }
  }

  return bit;
}

/**
 * @brief Where the sorted keys [a_lo, a_hi) are cut in two: the first key with the highest bit where the range's
 * first and last codes differ set, or the middle when every code is equal.
 * @details Internal helper; not part of the public API.
 * @param[in] a_keys The sorted keys.
 * @param[in] a_lo   First key of the range.
 * @param[in] a_hi   One past the range's last key; at least a_lo + 2.
 * @return The first key of the cut's right side.
 */
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline uint32_t
radixSplit(const MortonKey* a_keys, const uint32_t a_lo, const uint32_t a_hi) noexcept
{
  EBGEOMETRY_EXPECT(a_hi >= a_lo + 2U);

  const uint64_t first = a_keys[a_lo].m_code;
  const uint64_t last  = a_keys[a_hi - 1U].m_code;

  if (first == last) {
    return a_lo + (a_hi - a_lo) / 2U;
  }

  // The codes share every bit above this one, and first's is 0, last's is 1: the right side starts at the first
  // code with it set.
  const uint32_t bit       = highestBit(first ^ last);
  const uint64_t threshold = ((first >> bit) | uint64_t{1}) << bit;

  uint32_t lo = a_lo;
  uint32_t hi = a_hi;

  while (lo < hi) {
    const uint32_t mid = lo + (hi - lo) / 2U;

    if (a_keys[mid].m_code < threshold) {
      lo = mid + 1U;
    }
    else {
      hi = mid;
    }
  }

  return lo;
}

/**
 * @brief Cut the sorted keys [a_lo, a_hi) into K ranges, a node's children: K - 1 binary cuts, the first giving
 * K/2 parts to the left, each made by radixSplit() and moved in to leave every part at least
 * max(1, floor((a_hi - a_lo) / (4K))) keys.
 * @details Internal helper; not part of the public API.
 * @tparam K Branching factor.
 * @param[in]  a_keys   The sorted keys.
 * @param[in]  a_lo     First key of the range.
 * @param[in]  a_hi     One past the range's last key; at least a_lo + K.
 * @param[out] a_bounds Range k is [a_bounds[k], a_bounds[k + 1]).
 */
template <size_t K>
EBGEOMETRY_HOST_DEVICE
inline void
splitRange(const MortonKey* a_keys, const uint32_t a_lo, const uint32_t a_hi, Array<uint32_t, K + 1>& a_bounds) noexcept
{
  EBGEOMETRY_EXPECT(a_hi >= a_lo + K);

  const uint32_t quota = Math::max(uint32_t{1}, (a_hi - a_lo) / static_cast<uint32_t>(4 * K));

  // A range still to cut: its keys, its first part and its number of parts.
  struct Part
  {
    uint32_t m_lo;
    uint32_t m_hi;
    uint32_t m_first;
    uint32_t m_parts;
  };

  Part     stack[K];
  uint32_t top = 0;

  stack[top++] = Part{a_lo, a_hi, 0, static_cast<uint32_t>(K)};

  a_bounds[0] = a_lo;
  a_bounds[K] = a_hi;

  while (top > 0) {
    const Part part = stack[--top];

    if (part.m_parts == 1) {
      continue;
    }

    const uint32_t left  = part.m_parts / 2U;
    const uint32_t right = part.m_parts - left;

    const uint32_t split =
      Math::clamp(radixSplit(a_keys, part.m_lo, part.m_hi), part.m_lo + left * quota, part.m_hi - right * quota);

    a_bounds[part.m_first + left] = split;

    EBGEOMETRY_EXPECT(top + 2U <= K);

    stack[top++] = Part{split, part.m_hi, part.m_first + left, right};
    stack[top++] = Part{part.m_lo, split, part.m_first, left};
  }
}

/**
 * @brief Build step 2: each primitive's key, at its center, and padding keys that sort last.
 * @tparam T      Floating-point precision.
 * @tparam Prim   The primitive type.
 * @tparam Source The primitives' source.
 */
template <class T, class Prim, class Source>
struct MortonKeys
{
  /// @brief The primitives.
  Source m_source;

  /// @brief Half the cloud box's lower corner.
  Vec3T<T> m_halfLo;

  /// @brief Cells per half unit length: the code's span over half the box's longest side.
  T m_scale;

  /// @brief The keys.
  MortonKey* m_keys;

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

    if (i >= m_numPoints) {
      m_keys[i] = MortonKey{Math::Limits<uint64_t>::max(), Math::Limits<uint32_t>::max()};

      return;
    }

    const Vec3T<T> p = Prim::center(m_source(i));

    SFC::Index cell;

    for (size_t axis = 0; axis < 3; axis++) {
      // In halves, so that a box wider than the largest finite value does not overflow.
      const T offset = (T(0.5) * p[axis] - m_halfLo[axis]) * m_scale;

      cell[axis] = static_cast<unsigned int>(Math::min(T(SFC::ValidSpan), Math::max(T(0), offset)));
    }

    m_keys[i] = MortonKey{SFC::Morton::encode(cell), i};
  }
};

/**
 * @brief Build step 3, first: the root, all the points.
 */
struct RootNode
{
  /// @brief The build's nodes.
  BuildNode* m_nodes;

  /// @brief Number of points.
  uint32_t m_numPoints;

  /**
   * @brief Write the root.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t) const noexcept
  {
    m_nodes[0] = BuildNode{0, m_numPoints, 0, 0, NoChildren, 0, 0};
  }
};

/**
 * @brief Build step 3, for one level: whether each node is cut, as 0 or 1 for the scan that ranks the cut ones.
 */
struct SplitFlags
{
  /// @brief The build's nodes.
  const BuildNode* m_nodes;

  /// @brief The flags, by breadth-first index.
  uint32_t* m_flags;

  /// @brief The level's first node.
  uint32_t m_begin;

  /// @brief The largest leaf.
  uint32_t m_leafSize;

  /**
   * @brief Flag node m_begin + @p a_i.
   * @param[in] a_i The node's index in its level.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_i) const noexcept
  {
    const uint32_t   i    = m_begin + static_cast<uint32_t>(a_i);
    const BuildNode& node = m_nodes[i];

    m_flags[i] = (node.m_hi - node.m_lo > m_leafSize) ? 1U : 0U;
  }
};

/**
 * @brief Build step 3, for one level: each cut node's K children, at the next level's start plus K times its rank.
 * @tparam K Branching factor.
 */
template <size_t K>
struct SplitEmit
{
  /// @brief The sorted keys.
  const MortonKey* m_keys;

  /// @brief The build's nodes.
  BuildNode* m_nodes;

  /// @brief Each cut node's rank in its level, by breadth-first index.
  const uint32_t* m_ranks;

  /// @brief The level's first node.
  uint32_t m_begin;

  /// @brief The next level's first node.
  uint32_t m_end;

  /// @brief The largest leaf.
  uint32_t m_leafSize;

  /**
   * @brief Cut node m_begin + @p a_i, if it is not a leaf.
   * @param[in] a_i The node's index in its level.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_i) const noexcept
  {
    const uint32_t i    = m_begin + static_cast<uint32_t>(a_i);
    BuildNode&     node = m_nodes[i];

    if (node.m_hi - node.m_lo <= m_leafSize) {
      node.m_firstChild = NoChildren;

      return;
    }

    const uint32_t first = m_end + static_cast<uint32_t>(K) * m_ranks[i];

    Array<uint32_t, K + 1> bounds;

    splitRange<K>(m_keys, node.m_lo, node.m_hi, bounds);

    node.m_firstChild = first;

    for (size_t k = 0; k < K; k++) {
      m_nodes[first + k] = BuildNode{bounds[k], bounds[k + 1], i, static_cast<uint32_t>(k), NoChildren, 0, 0};
    }
  }
};

/**
 * @brief Build step 4, for one level from the deepest up: each node's subtree size and box.
 * @tparam T      Floating-point precision.
 * @tparam K      Branching factor.
 * @tparam Prim   The primitive type.
 * @tparam Source The primitives' source.
 */
template <class T, size_t K, class Prim, class Source>
struct SubtreeUp
{
  /// @brief The primitives.
  Source m_source;

  /// @brief The sorted keys.
  const MortonKey* m_keys;

  /// @brief The build's nodes.
  BuildNode* m_nodes;

  /// @brief Each node's box.
  BoundingVolumes::AABBT<T>* m_boxes;

  /// @brief The level's first node.
  uint32_t m_begin;

  /**
   * @brief Node m_begin + @p a_i.
   * @param[in] a_i The node's index in its level.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_i) const noexcept
  {
    const uint32_t i    = m_begin + static_cast<uint32_t>(a_i);
    BuildNode&     node = m_nodes[i];

    Vec3T<T> lo = +Vec3T<T>::max();
    Vec3T<T> hi = -Vec3T<T>::max();

    if (node.m_firstChild == NoChildren) {
      for (uint32_t j = node.m_lo; j < node.m_hi; j++) {
        Prim::grow(lo, hi, m_source(m_keys[j].m_index));
      }

      node.m_subtree = 1;
    }
    else {
      uint32_t subtree = 1;

      for (size_t k = 0; k < K; k++) {
        const uint32_t child = node.m_firstChild + static_cast<uint32_t>(k);

        lo = min(lo, m_boxes[child].getLowCorner());
        hi = max(hi, m_boxes[child].getHighCorner());

        subtree += m_nodes[child].m_subtree;
      }

      node.m_subtree = subtree;
    }

    m_boxes[i] = BoundingVolumes::AABBT<T>(lo, hi);
  }
};

/**
 * @brief Build step 4, for one level from the root down: each cut node's children's depth-first indices, the
 * first after it and each one after its previous sibling's subtree.
 * @tparam K Branching factor.
 */
template <size_t K>
struct DepthFirst
{
  /// @brief The build's nodes.
  BuildNode* m_nodes;

  /// @brief The level's first node.
  uint32_t m_begin;

  /**
   * @brief Node m_begin + @p a_i.
   * @param[in] a_i The node's index in its level.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_i) const noexcept
  {
    const BuildNode& node = m_nodes[m_begin + static_cast<uint32_t>(a_i)];

    if (node.m_firstChild == NoChildren) {
      return;
    }

    uint32_t next = node.m_depthFirst + 1U;

    for (size_t k = 0; k < K; k++) {
      BuildNode& child = m_nodes[node.m_firstChild + static_cast<uint32_t>(k)];

      child.m_depthFirst = next;
      next += child.m_subtree;
    }
  }
};

/**
 * @brief Build step 5: by depth-first index, 1 for an interior node and each leaf's group count, for the scans
 * that number the rows and place the groups.
 * @tparam W Points per group.
 */
template <size_t W>
struct MarkDepthFirst
{
  /// @brief The build's nodes.
  const BuildNode* m_nodes;

  /// @brief Interior flags, by depth-first index.
  uint32_t* m_rows;

  /// @brief Group counts, by depth-first index.
  uint32_t* m_groups;

  /**
   * @brief Node @p a_i.
   * @param[in] a_i The node's breadth-first index.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_i) const noexcept
  {
    const BuildNode& node = m_nodes[a_i];
    const bool       leaf = node.m_firstChild == NoChildren;

    m_rows[node.m_depthFirst] = leaf ? 0U : 1U;
    m_groups[node.m_depthFirst] =
      leaf ? (node.m_hi - node.m_lo + static_cast<uint32_t>(W) - 1U) / static_cast<uint32_t>(W) : 0U;
  }
};

/**
 * @brief Build step 6: write every node of the packed BVH, its row and its box slot; a leaf also packs its groups
 * and writes its primitives' leaf tables, the primitives and their order.
 * @tparam T      Floating-point precision.
 * @tparam K      Branching factor.
 * @tparam W      Primitives per group.
 * @tparam Prim   The primitive type.
 * @tparam Source The primitives' source.
 */
template <class T, size_t K, size_t W, class Prim, class Source>
struct WriteNodes
{
  /// @brief The primitive.
  using Item = typename Prim::Item;

  /// @brief The packed BVH.
  using Packed = BVH::PackedBVH<T, typename Prim::Group, K>;

  /// @brief The primitives.
  Source m_source;

  /// @brief The sorted keys.
  const MortonKey* m_keys;

  /// @brief The build's nodes.
  const BuildNode* m_build;

  /// @brief The build nodes' boxes.
  const BoundingVolumes::AABBT<T>* m_boxes;

  /// @brief Each interior node's row, by depth-first index.
  const uint32_t* m_rows;

  /// @brief Each leaf's first group, by depth-first index.
  const uint32_t* m_groups;

  /// @brief The packed nodes.
  typename Packed::Node* m_nodes;

  /// @brief The child-box rows.
  BVH::ChildBoxes<T, K>* m_childBoxes;

  /// @brief The root box.
  BoundingVolumes::AABBT<T>* m_rootBox;

  /// @brief The groups.
  typename Prim::Group* m_primitives;

  /// @brief The primitives, by cloud index.
  Item* m_items;

  /// @brief Each primitive's leaf's first group, by cloud index.
  uint32_t* m_leafOff;

  /// @brief Each primitive's leaf's group count, by cloud index.
  uint32_t* m_leafCnt;

  /// @brief Cloud indices in leaf order.
  uint32_t* m_order;

  /**
   * @brief Node @p a_i.
   * @param[in] a_i The node's breadth-first index.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_i) const noexcept
  {
    const auto       i     = static_cast<uint32_t>(a_i);
    const BuildNode& build = m_build[i];

    typename Packed::Node node{};

    node.m_boxSlot = (i == 0) ? Packed::RootBoxSlot
                              : m_rows[m_build[build.m_parent].m_depthFirst] * static_cast<uint32_t>(K) + build.m_slot;

    if (i == 0) {
      m_rootBox[0] = m_boxes[0];
    }

    if (build.m_firstChild != NoChildren) {
      const uint32_t row = m_rows[build.m_depthFirst];

      node.m_childBoxRow = row;

      for (size_t k = 0; k < K; k++) {
        const uint32_t                   child = build.m_firstChild + static_cast<uint32_t>(k);
        const BoundingVolumes::AABBT<T>& box   = m_boxes[child];

        node.m_childOff[k] = m_build[child].m_depthFirst;

        for (size_t axis = 0; axis < 3; axis++) {
          m_childBoxes[row].m_lo[axis][k] = box.getLowCorner()[axis];
          m_childBoxes[row].m_hi[axis][k] = box.getHighCorner()[axis];
        }
      }

      m_nodes[build.m_depthFirst] = node;

      return;
    }

    const uint32_t first = m_groups[build.m_depthFirst];
    const uint32_t count = (build.m_hi - build.m_lo + static_cast<uint32_t>(W) - 1U) / static_cast<uint32_t>(W);

    node.m_primOff  = first;
    node.m_numPrims = count;

    m_nodes[build.m_depthFirst] = node;

    for (uint32_t g = 0; g < count; g++) {
      const uint32_t lo    = build.m_lo + g * static_cast<uint32_t>(W);
      const uint32_t valid = Math::min(static_cast<uint32_t>(W), build.m_hi - lo);

      // Zeroed, so that an optimizer that cannot see valid >= 1 reads nothing uninitialized.
      Array<Item, W>     items{};
      Array<uint32_t, W> ids{};

      for (uint32_t j = 0; j < valid; j++) {
        const uint32_t index = m_keys[lo + j].m_index;

        ids[j]   = index;
        items[j] = m_source(index);

        m_items[index]   = items[j];
        m_leafOff[index] = first;
        m_leafCnt[index] = count;
        m_order[lo + j]  = index;
      }

      m_primitives[first + g].packUnchecked(items.data(), ids.data(), valid);
    }
  }
};

template <class T, size_t K, size_t W, class Prim>
EBGEOMETRY_HOST
inline typename CloudBVH<T, K, W, Prim>::Sizes
CloudBVH<T, K, W, Prim>::sizesFor(const uint32_t a_numItems, const uint32_t a_leafSize, const char* a_who) noexcept
{
  if (a_numItems == 0) {
    return Sizes{0, 0, 0};
  }

  if (a_numItems <= a_leafSize) {
    return Sizes{1, 0, (a_numItems + static_cast<uint32_t>(W) - 1U) / static_cast<uint32_t>(W)};
  }

  // Every leaf is the child of a cut node, of more than a_leafSize primitives, so it holds at least its quota of
  // them.
  const uint64_t n        = a_numItems;
  const uint64_t quota    = Math::max(uint64_t{1}, (uint64_t(a_leafSize) + 1U) / uint64_t(4 * K));
  const uint64_t leaves   = n / quota;
  const uint64_t interior = (leaves - 1U) / uint64_t(K - 1);
  const uint64_t nodes    = 1U + uint64_t(K) * interior;
  const uint64_t used     = 1U + uint64_t(K - 1) * interior;
  const uint64_t groups   = (n + used * uint64_t(W - 1)) / uint64_t(W);

  EBGEOMETRY_REQUIRE(nodes <= UINT32_MAX && groups <= UINT32_MAX,
                     "%s: %u %s may need more nodes than 32-bit indices count",
                     a_who,
                     unsigned(a_numItems),
                     Prim::Plural);

  return Sizes{static_cast<uint32_t>(nodes), static_cast<uint32_t>(interior), static_cast<uint32_t>(groups)};
}

template <class T, size_t K, size_t W, class Prim>
EBGEOMETRY_HOST
inline typename CloudBVH<T, K, W, Prim>::Scratch
CloudBVH<T, K, W, Prim>::reserveScratch(Pool&          a_pool,
                                        const uint32_t a_numItems,
                                        const uint32_t a_leafSize,
                                        const char*    a_who)
{
  const Sizes sizes = sizesFor(a_numItems, a_leafSize, a_who);

  Scratch scratch;

  scratch.m_keys.reserveSizedFrom(a_pool, keyCount(a_numItems));
  scratch.m_box.reserveSizedFrom(a_pool, GPU::boundingBoxScratchSize(a_numItems));
  scratch.m_nodes.reserveSizedFrom(a_pool, sizes.m_nodes);
  scratch.m_boxes.reserveSizedFrom(a_pool, sizes.m_nodes);
  scratch.m_rows.reserveSizedFrom(a_pool, sizes.m_nodes);
  scratch.m_groups.reserveSizedFrom(a_pool, sizes.m_nodes);
  scratch.m_scan.reserveSizedFrom(a_pool, GPU::exclusiveScanScratchSize(sizes.m_nodes));

  return scratch;
}

template <class T, size_t K, size_t W, class Prim>
EBGEOMETRY_HOST
inline void
CloudBVH<T, K, W, Prim>::reserveArrays(Pool& a_pool, const Sizes& a_sizes, const uint32_t a_numItems)
{
  this->attach(a_pool, Prim::ClassName, m_parallel);

  m_bvhLayout.m_linearNodes.reserveSizedFrom(a_pool, a_sizes.m_nodes);
  m_bvhLayout.m_primitives.reserveSizedFrom(a_pool, a_sizes.m_groups);
  m_bvhLayout.m_childAabbSoA.reserveSizedFrom(a_pool, a_sizes.m_interior);
  m_bvhLayout.m_rootBox.reserveSizedFrom(a_pool, a_sizes.m_nodes > 0 ? 1U : 0U);

  m_items.reserveSizedFrom(a_pool, a_numItems);
  m_leafOff.reserveSizedFrom(a_pool, a_numItems);
  m_leafCnt.reserveSizedFrom(a_pool, a_numItems);
  m_order.reserveSizedFrom(a_pool, a_numItems);
}

template <class T, size_t K, size_t W, class Prim>
template <class F>
EBGEOMETRY_HOST
inline void
CloudBVH<T, K, W, Prim>::run(const std::size_t a_count, const F& a_f) const
{
  if (m_parallel) {
    GPU::parallelFor(a_count, a_f);

    return;
  }

  for (std::size_t i = 0; i < a_count; i++) {
    a_f(i);
  }
}

template <class T, size_t K, size_t W, class Prim>
EBGEOMETRY_HOST
inline std::uint32_t
CloudBVH<T, K, W, Prim>::scan(std::uint32_t* a_values, const std::uint32_t a_count, std::uint32_t* a_scratch) const
{
  if (m_parallel) {
    return GPU::exclusiveScan(a_values, a_count, a_scratch);
  }

  std::uint32_t sum = 0;

  for (std::uint32_t i = 0; i < a_count; i++) {
    const std::uint32_t value = a_values[i];

    a_values[i] = sum;
    sum += value;
  }

  return sum;
}

template <class T, size_t K, size_t W, class Prim>
template <class U>
EBGEOMETRY_HOST
inline std::vector<U>
CloudBVH<T, K, W, Prim>::toHost(const PODVector<U>& a_array, const void* a_base)
{
  const U* first = a_array.data(a_base);

  return std::vector<U>(first, first + a_array.size());
}

template <class T, size_t K, size_t W, class Prim>
template <class Source>
EBGEOMETRY_HOST
inline typename CloudBVH<T, K, W, Prim>::Sizes
CloudBVH<T, K, W, Prim>::shape(
  const Scratch& a_scratch, void* a_base, const Source& a_source, const uint32_t a_numItems, const char* a_who)
{
  // Step 1: the box of the centers, every coordinate checked, and what else the primitive needs checked.
  Vec3T<T>* boxScratch = a_scratch.m_box.data(a_base);

  Prim::requireValidWhereBuilt(a_source, a_numItems, m_parallel, boxScratch, a_who);

  AABB box;

  if constexpr (std::is_same_v<Item, Vec3T<T>>) {
    box = boxOf<T>(a_source, a_numItems, m_parallel, boxScratch, a_who, Prim::Singular);
  }
  else {
    box = boxOf<T>(Centers<Prim, Source>{a_source}, a_numItems, m_parallel, boxScratch, a_who, Prim::Singular);
  }

  m_depth = 0;

  if (a_numItems == 0) {
    return Sizes{0, 0, 0};
  }

  MortonKey* keys   = a_scratch.m_keys.data(a_base);
  BuildNode* nodes  = a_scratch.m_nodes.data(a_base);
  AABB*      boxes  = a_scratch.m_boxes.data(a_base);
  uint32_t*  rows   = a_scratch.m_rows.data(a_base);
  uint32_t*  groups = a_scratch.m_groups.data(a_base);
  uint32_t*  scans  = a_scratch.m_scan.data(a_base);

  const uint32_t capacity = a_scratch.m_nodes.size();

  // Step 2: the keys, sorted. GPU::bitonicSort() sorts a power of two of them, the padding last; the host sorts
  // the primitives' only. The keys are distinct, so both give one order.
  const Vec3T<T> halfLo = T(0.5) * box.getLowCorner();
  const Vec3T<T> extent = T(0.5) * box.getHighCorner() - halfLo;
  const T        side   = Math::max(extent[0], Math::max(extent[1], extent[2]));
  const T        scale  = (side > T(0)) ? T(SFC::ValidSpan) / side : T(0);
  const uint32_t count  = m_parallel ? keyCount(a_numItems) : a_numItems;

  this->run(count, MortonKeys<T, Prim, Source>{a_source, halfLo, scale, keys, a_numItems});

  if (m_parallel) {
    GPU::bitonicSort(keys, count);
  }
  else {
    std::sort(keys, keys + a_numItems);
  }

  // Step 3: the levels, breadth first. Each is [begin, end) of the node array, and its cut nodes' children are the
  // next level.
  const std::size_t maxDepth = m_parallel ? BVH::DeviceTraversalDepth : BVH::HostTraversalDepth;

  std::vector<uint32_t> levels{0, 1};

  this->run(1, RootNode{nodes, a_numItems});

  // Read only by the check against the row count below, so unused with assertions off.
  [[maybe_unused]] uint32_t numInterior = 0;

  while (true) {
    const uint32_t begin = levels[levels.size() - 2];
    const uint32_t end   = levels.back();

    this->run(end - begin, SplitFlags{nodes, rows, begin, m_leafSize});

    const uint32_t cut = this->scan(rows + begin, end - begin, scans);

    // The cut nodes' children are one level deeper. sizesFor() bounds the node count of every cloud of this many
    // primitives; the check costs one comparison per level, and guards the scratch a wrong bound would overrun.
    EBGEOMETRY_REQUIRE(cut == 0 || levels.size() <= maxDepth,
                       "%s: the tree would be more than %zu levels deep, more than its traversal allows",
                       a_who,
                       maxDepth);
    EBGEOMETRY_REQUIRE(uint64_t(end) + uint64_t(K) * cut <= capacity,
                       "%s: the tree needs more than the %u nodes reserved for it",
                       a_who,
                       unsigned(capacity));

    this->run(end - begin, SplitEmit<K>{keys, nodes, rows, begin, end, m_leafSize});

    if (cut == 0) {
      break;
    }

    numInterior += cut;
    levels.push_back(end + static_cast<uint32_t>(K) * cut);
  }

  const uint32_t numNodes = levels.back();

  m_depth = static_cast<uint32_t>(levels.size() - 1);

  // Step 4: subtree sizes and boxes from the deepest level up, then depth-first indices from the root down.
  for (std::size_t level = levels.size() - 1; level > 0; level--) {
    const uint32_t begin = levels[level - 1];

    this->run(levels[level] - begin, SubtreeUp<T, K, Prim, Source>{a_source, keys, nodes, boxes, begin});
  }

  for (std::size_t level = 1; level < levels.size(); level++) {
    const uint32_t begin = levels[level - 1];

    this->run(levels[level] - begin, DepthFirst<K>{nodes, begin});
  }

  // Step 5: the rows and the groups, numbered in depth-first order.
  this->run(numNodes, MarkDepthFirst<W>{nodes, rows, groups});

  const uint32_t numRows   = this->scan(rows, numNodes, scans);
  const uint32_t numGroups = this->scan(groups, numNodes, scans);

  EBGEOMETRY_EXPECT(numRows == numInterior);

  return Sizes{numNodes, numRows, numGroups};
}

template <class T, size_t K, size_t W, class Prim>
template <class Source>
EBGEOMETRY_HOST
inline void
CloudBVH<T, K, W, Prim>::write(
  const Scratch& a_scratch, void* a_base, const Sizes& a_sizes, const Source& a_source, const uint32_t a_numItems)
{
  EBGEOMETRY_EXPECT(a_sizes.m_nodes <= m_bvhLayout.m_linearNodes.m_capacity);
  EBGEOMETRY_EXPECT(a_sizes.m_groups <= m_bvhLayout.m_primitives.m_capacity);
  EBGEOMETRY_EXPECT(a_sizes.m_interior <= m_bvhLayout.m_childAabbSoA.m_capacity);
  EBGEOMETRY_EXPECT(a_numItems <= m_items.m_capacity);

  m_bvhLayout.m_linearNodes.m_size  = a_sizes.m_nodes;
  m_bvhLayout.m_primitives.m_size   = a_sizes.m_groups;
  m_bvhLayout.m_childAabbSoA.m_size = a_sizes.m_interior;
  m_bvhLayout.m_rootBox.m_size      = a_sizes.m_nodes > 0 ? 1U : 0U;

  m_items.m_size   = a_numItems;
  m_leafOff.m_size = a_numItems;
  m_leafCnt.m_size = a_numItems;
  m_order.m_size   = a_numItems;

  void* base = PoolHandle::base();

  this->run(a_sizes.m_nodes,
            WriteNodes<T, K, W, Prim, Source>{a_source,
                                              a_scratch.m_keys.data(a_base),
                                              a_scratch.m_nodes.data(a_base),
                                              a_scratch.m_boxes.data(a_base),
                                              a_scratch.m_rows.data(a_base),
                                              a_scratch.m_groups.data(a_base),
                                              m_bvhLayout.m_linearNodes.data(base),
                                              m_bvhLayout.m_childAabbSoA.data(base),
                                              m_bvhLayout.m_rootBox.data(base),
                                              m_bvhLayout.m_primitives.data(base),
                                              m_items.data(base),
                                              m_leafOff.data(base),
                                              m_leafCnt.data(base),
                                              m_order.data(base)});
}

template <class T, size_t K, size_t W, class Prim>
template <class Source>
EBGEOMETRY_HOST
inline void
CloudBVH<T, K, W, Prim>::initialize(Pool&             a_pool,
                                    const Source&     a_source,
                                    const uint32_t    a_numItems,
                                    const std::size_t a_targetLeafSize,
                                    const uint32_t    a_maxItems,
                                    const char*       a_who)
{
  EBGEOMETRY_REQUIRE(
    a_targetLeafSize >= 1, "%s: the target leaf size must be at least 1 (%zu)", a_who, a_targetLeafSize);

  // The keys are padded to a power of two that is counted in 32 bits.
  const uint32_t room = Math::max(a_numItems, a_maxItems);

  EBGEOMETRY_REQUIRE(room <= (uint32_t{1} << 31), "%s: at most 2^31 %s (%u)", a_who, Prim::Plural, unsigned(room));
  EBGEOMETRY_REQUIRE(a_maxItems == 0 || a_maxItems >= a_numItems,
                     "%s: room for %u %s in a rebuild, fewer than the %u built",
                     a_who,
                     unsigned(a_maxItems),
                     Prim::Plural,
                     unsigned(a_numItems));

  m_parallel = a_pool.resource().isDeviceAccessible();
  m_maxItems = a_maxItems;
  m_leafSize = static_cast<uint32_t>(Math::min(a_targetLeafSize, std::size_t(UINT32_MAX - 1)));
  m_leafSize = Math::max(m_leafSize, static_cast<uint32_t>(K));

  if (a_maxItems > 0) {
    // Room for a rebuild: the arrays at the sizes any cloud of that many primitives can need, and the scratch with
    // them, so that a rebuild reserves nothing.
    this->reserveArrays(a_pool, sizesFor(a_maxItems, m_leafSize, a_who), a_maxItems);
    m_scratch = reserveScratch(a_pool, a_maxItems, m_leafSize, a_who);

    // Resolved after every reservation above: a reserve can move the block.
    void* base = PoolHandle::base();

    const Sizes sizes = this->shape(m_scratch, base, a_source, a_numItems, a_who);

    this->write(m_scratch, base, sizes, a_source, a_numItems);

    return;
  }

  // The scratch in a pool of its own, released on return, and the arrays at the sizes the tree needs.
  Pool scratchPool(a_pool.resource());

  const Scratch scratch = reserveScratch(scratchPool, a_numItems, m_leafSize, a_who);
  void*         base    = scratchPool.base();

  const Sizes sizes = this->shape(scratch, base, a_source, a_numItems, a_who);

  this->reserveArrays(a_pool, sizes, a_numItems);
  this->write(scratch, base, sizes, a_source, a_numItems);
}

template <class T, size_t K, size_t W, class Prim>
EBGEOMETRY_HOST
inline void
CloudBVH<T, K, W, Prim>::initialize(Pool&                    a_pool,
                                    const std::vector<Item>& a_items,
                                    const std::size_t        a_targetLeafSize,
                                    const uint32_t           a_maxItems,
                                    const char*              a_who)
{
  EBGEOMETRY_REQUIRE(a_items.size() < std::size_t(PointCloud::InvalidIndex),
                     "%s: too many %s for 32-bit indices (%zu %s, limit %zu)",
                     a_who,
                     Prim::Plural,
                     a_items.size(),
                     Prim::Plural,
                     std::size_t(PointCloud::InvalidIndex) - 1);

  const auto numItems = static_cast<uint32_t>(a_items.size());

  using FromArray = typename Prim::FromArray;

  // A build on the host checks every primitive as it finds the box. A build through GPU::parallelFor() reports
  // only that some primitive is not valid, so the primitives are checked here first.
  if (a_pool.resource().isDeviceAccessible()) {
    Prim::requireValid(a_who, a_items);
  }

  // So does one through GPU::parallelFor() in a host build (in emulated device memory). A kernel cannot read the
  // vector: the primitives go to scratch in the pool's memory first, released on return.
  if (!a_pool.resource().isDeviceAccessible() || !GPU::enabled) {
    this->initialize(a_pool, FromArray{a_items.data()}, numItems, a_targetLeafSize, a_maxItems, a_who);

    return;
  }

  Pool scratch(a_pool.resource());

  PODVector<Item> items;

  items.reserveSizedFrom(scratch, numItems);

  Item* data = items.data(scratch.base());

  if (numItems > 0) {
    a_pool.resource().copy(
      data, a_pool.resource(), a_items.data(), hostMemoryResource(), sizeof(Item) * std::size_t(numItems));
  }

  this->initialize(a_pool, FromArray{data}, numItems, a_targetLeafSize, a_maxItems, a_who);
}

template <class T, size_t K, size_t W, class Prim>
template <class Source>
EBGEOMETRY_HOST
inline void
CloudBVH<T, K, W, Prim>::reinitialize(const Source& a_source, const uint32_t a_numItems, const char* a_who)
{
  EBGEOMETRY_REQUIRE(
    m_maxItems > 0, "%s: the BVH has no room for a rebuild (see build()'s %s)", a_who, Prim::MaxParameter);
  EBGEOMETRY_REQUIRE(a_numItems <= m_maxItems,
                     "%s: %u %s, but the BVH has room for %u (see build()'s %s)",
                     a_who,
                     unsigned(a_numItems),
                     Prim::Plural,
                     unsigned(m_maxItems),
                     Prim::MaxParameter);

  void* base = PoolHandle::base();

  const Sizes sizes = this->shape(m_scratch, base, a_source, a_numItems, a_who);

  this->write(m_scratch, base, sizes, a_source, a_numItems);
}

template <class T, size_t K, size_t W, class Prim>
EBGEOMETRY_HOST
inline void
CloudBVH<T, K, W, Prim>::rebase(const Pool& a_pool) noexcept
{
  const auto end = [](const auto&... a_arrays) noexcept {
    uint64_t byte = 0;

    ((byte = Math::max(byte, a_arrays.endByte())), ...);

    return byte;
  };

  const uint64_t endByte = Math::max(end(m_bvhLayout.m_linearNodes,
                                         m_bvhLayout.m_primitives,
                                         m_bvhLayout.m_childAabbSoA,
                                         m_bvhLayout.m_rootBox,
                                         m_items,
                                         m_leafOff,
                                         m_leafCnt,
                                         m_order),
                                     end(m_scratch.m_keys,
                                         m_scratch.m_box,
                                         m_scratch.m_nodes,
                                         m_scratch.m_boxes,
                                         m_scratch.m_rows,
                                         m_scratch.m_groups,
                                         m_scratch.m_scan));

  // Every array is a pool-relative offset, identical in any mirror of the pool, so only the location changes.
  this->setLocation(this->rebasedLocation(a_pool, endByte, Prim::ClassName));

  // A snapshot is what a kernel receives, and the device traversal stack is smaller than the host's. The depth is
  // the build's, so this reads nothing from a pool that may be in device memory.
  if (this->location().m_control == nullptr) {
    EBGEOMETRY_REQUIRE(m_depth <= BVH::DeviceTraversalDepth,
                       "%s::rebasedView: the tree is %u levels deep, more than a device traversal allows (%zu)",
                       Prim::ClassName,
                       unsigned(m_depth),
                       BVH::DeviceTraversalDepth);
  }
}

template <class T, size_t K, size_t W, class Prim>
EBGEOMETRY_HOST
inline void
CloudBVH<T, K, W, Prim>::copyInto(CloudBVH& a_copy, Pool& a_dstPool) const
{
  // Read everything out before reserving anything: a_dstPool may be this object's own pool, and a reserve there can
  // move the block under any address taken from it.
  const void* srcBase = this->base();

  const auto nodes   = toHost(m_bvhLayout.m_linearNodes, srcBase);
  const auto groups  = toHost(m_bvhLayout.m_primitives, srcBase);
  const auto rows    = toHost(m_bvhLayout.m_childAabbSoA, srcBase);
  const auto rootBox = toHost(m_bvhLayout.m_rootBox, srcBase);
  const auto items   = toHost(m_items, srcBase);
  const auto leafOff = toHost(m_leafOff, srcBase);
  const auto leafCnt = toHost(m_leafCnt, srcBase);
  const auto order   = toHost(m_order, srcBase);

  const auto numItems = static_cast<std::uint32_t>(items.size());

  a_copy.setLocation(PoolLocation{});

  a_copy.m_scratch  = Scratch{};
  a_copy.m_maxItems = 0;
  a_copy.m_parallel = false;

  a_copy.reserveArrays(a_dstPool,
                       Sizes{static_cast<std::uint32_t>(nodes.size()),
                             static_cast<std::uint32_t>(rows.size()),
                             static_cast<std::uint32_t>(groups.size())},
                       numItems);

  // Resolve the base only after every reservation above.
  void* base = a_copy.base();

  a_copy.m_bvhLayout.m_linearNodes.assign(base, nodes.data(), static_cast<std::uint32_t>(nodes.size()));
  a_copy.m_bvhLayout.m_primitives.assign(base, groups.data(), static_cast<std::uint32_t>(groups.size()));
  a_copy.m_bvhLayout.m_childAabbSoA.assign(base, rows.data(), static_cast<std::uint32_t>(rows.size()));
  a_copy.m_bvhLayout.m_rootBox.assign(base, rootBox.data(), static_cast<std::uint32_t>(rootBox.size()));
  a_copy.m_items.assign(base, items.data(), numItems);
  a_copy.m_leafOff.assign(base, leafOff.data(), numItems);
  a_copy.m_leafCnt.assign(base, leafCnt.data(), numItems);
  a_copy.m_order.assign(base, order.data(), numItems);
}

} // namespace PointCloudDetail

} // namespace EBGeometry

#endif
