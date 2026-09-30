// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_BVHBuild.hpp
 * @brief  The BVH node layout, the build specification, and the one host-side BVH builder.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_BVHBUILD_HPP
#define EBGEOMETRY_BVHBUILD_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

namespace BVH {

/**
 * @brief How the builder partitions the primitives.
 * @details Every strategy honours BuildSpec::maxLeafSize the same way: a leaf holds at most that
 * many primitives.
 */
enum class Strategy
{
  SAH,              ///< Top-down, binned surface area heuristic. The default: the best trees for queries.
  Centroid,         ///< Top-down, equal-count split along the longest axis of the primitive centroids.
  Midpoint,         ///< Top-down, split at the spatial midpoint of the centroids' longest axis.
  ClusterSAH,       ///< Cluster the primitives into leaf-sized groups first, then binned SAH over the groups.
  SpaceFillingCurve ///< Sort along a space-filling curve (BuildSpec::curve), then merge bottom-up.
};

/**
 * @brief The space-filling curve a Strategy::SpaceFillingCurve build sorts along.
 */
enum class Curve
{
  Morton, ///< Morton (Z-order) curve.
  Nested, ///< Nested (row-major) ordering.
  Hilbert ///< Hilbert curve.
};

/**
 * @brief How to build a BVH: a strategy, a curve (for Strategy::SpaceFillingCurve only), and the
 * maximum number of primitives per leaf.
 * @details Means the same thing to every class that builds a BVH. A class that groups several
 * primitives into one stored element (TriMeshSDF packs triangles into SIMD groups, PointCloudBVH
 * packs points) counts the primitives it was given, before grouping.
 */
struct BuildSpec
{
  /**
   * @brief Partitioning strategy.
   */
  Strategy strategy = Strategy::SAH;

  /**
   * @brief Space-filling curve, used by Strategy::SpaceFillingCurve only.
   */
  Curve curve = Curve::Morton;

  /**
   * @brief Maximum number of primitives in one leaf. Must be positive.
   */
  uint32_t maxLeafSize = 4;
};

/**
 * @brief Deepest tree, in node levels, that a host traversal handles.
 * @details PackedBVH::pruneTraverse keeps a fixed stack sized from this. The builder never produces
 * a deeper tree; an adopted node array is checked against it.
 */
inline constexpr uint32_t HostTraversalDepth = 64;

/**
 * @brief Deepest tree, in node levels, that a device traversal handles.
 * @details Smaller than the host value, since device stack memory is per thread. The builder falls
 * back to Strategy::Centroid if the requested strategy produces a deeper tree, and
 * PackedBVH::rebasedView() checks every device view against it.
 */
inline constexpr uint32_t DeviceTraversalDepth = 32;

/**
 * @brief One interior node of a BVH: up to K child slots, each a leaf, an interior node, or empty.
 * @details A slot holds its child's bounding box, stored as structure-of-arrays rows so that one
 * SIMD load reads one axis of all K boxes, and either the index of an interior child node or the
 * primitive range of a leaf. Leaves are not nodes of their own, so no box is stored twice and no
 * node is ever a leaf with zero primitives.
 *
 * The occupied slots come first. An empty slot has an inverted box (lo = +inf, hi = -inf), so its
 * distance to any point is +inf. Interior children always have higher indices than their parent,
 * and the root is node 0.
 * @tparam T Floating-point precision.
 * @tparam K Number of child slots per node.
 */
template <class T, size_t K>
struct WideNode
{
  static_assert(std::is_floating_point_v<T>, "WideNode: T must be a floating-point type");
  static_assert(K >= 2, "WideNode: the branching factor K must be at least 2");

  /**
   * @brief The m_child value of an empty slot.
   */
  static constexpr uint32_t EmptySlot = 0xFFFFFFFFU;

  /**
   * @brief Alignment of one box row, in bytes.
   * @details A row is sizeof(T)*K bytes, and the SIMD paths of PackedBVH load a whole row at once,
   * so it is aligned to its own width where that is a power of two. For every other K no SIMD path
   * exists and the natural alignment of T is enough.
   */
  static constexpr size_t RowAlignment =
    (((sizeof(T) * K) & ((sizeof(T) * K) - 1)) == 0) ? (sizeof(T) * K) : alignof(T);

  /**
   * @brief Low corners of the slot boxes: m_lo[axis][slot].
   */
  alignas(RowAlignment) T m_lo[3][K];

  /**
   * @brief High corners of the slot boxes: m_hi[axis][slot].
   */
  alignas(RowAlignment) T m_hi[3][K];

  /**
   * @brief Per slot: the child node index (interior), the first primitive (leaf), or EmptySlot.
   */
  uint32_t m_child[K];

  /**
   * @brief Per slot: the number of primitives (leaf, at least one), or zero (interior or empty).
   */
  uint32_t m_count[K];

  /**
   * @brief A node with every slot empty.
   * @return The node.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline WideNode
  empty() noexcept
  {
    WideNode node;

    for (size_t k = 0; k < K; k++) {
      node.setEmpty(k);
    }

    return node;
  }

  /**
   * @brief Make slot @p a_k empty.
   * @param[in] a_k Slot.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  setEmpty(const size_t a_k) noexcept
  {
    EBGEOMETRY_EXPECT(a_k < K);

    for (size_t dir = 0; dir < 3; dir++) {
      m_lo[dir][a_k] = +Math::Limits<T>::infinity();
      m_hi[dir][a_k] = -Math::Limits<T>::infinity();
    }

    m_child[a_k] = EmptySlot;
    m_count[a_k] = 0;
  }

  /**
   * @brief Make slot @p a_k a leaf.
   * @param[in] a_k      Slot.
   * @param[in] a_box    Bounding box of the leaf's primitives.
   * @param[in] a_offset Index of the leaf's first primitive.
   * @param[in] a_count  Number of primitives in the leaf; at least one.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  setLeaf(const size_t                     a_k,
          const BoundingVolumes::AABBT<T>& a_box,
          const uint32_t                   a_offset,
          const uint32_t                   a_count) noexcept
  {
    EBGEOMETRY_EXPECT(a_k < K);
    EBGEOMETRY_EXPECT(a_count > 0);

    this->setBoundingVolume(a_k, a_box);

    m_child[a_k] = a_offset;
    m_count[a_k] = a_count;
  }

  /**
   * @brief Make slot @p a_k an interior child.
   * @param[in] a_k    Slot.
   * @param[in] a_box  Bounding box of the child's subtree.
   * @param[in] a_node Index of the child node.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  setInterior(const size_t a_k, const BoundingVolumes::AABBT<T>& a_box, const uint32_t a_node) noexcept
  {
    EBGEOMETRY_EXPECT(a_k < K);
    EBGEOMETRY_EXPECT(a_node != EmptySlot);

    this->setBoundingVolume(a_k, a_box);

    m_child[a_k] = a_node;
    m_count[a_k] = 0;
  }

  /**
   * @brief Replace the bounding box of slot @p a_k.
   * @param[in] a_k   Slot.
   * @param[in] a_box New box.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  setBoundingVolume(const size_t a_k, const BoundingVolumes::AABBT<T>& a_box) noexcept
  {
    EBGEOMETRY_EXPECT(a_k < K);

    for (size_t dir = 0; dir < 3; dir++) {
      m_lo[dir][a_k] = a_box.getLowCorner()[dir];
      m_hi[dir][a_k] = a_box.getHighCorner()[dir];
    }
  }

  /**
   * @brief Whether slot @p a_k is empty.
   * @param[in] a_k Slot.
   * @return True if the slot holds nothing.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  isEmpty(const size_t a_k) const noexcept
  {
    return m_count[a_k] == 0 && m_child[a_k] == EmptySlot;
  }

  /**
   * @brief Whether slot @p a_k is a leaf.
   * @param[in] a_k Slot.
   * @return True if the slot holds a primitive range.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  isLeaf(const size_t a_k) const noexcept
  {
    return m_count[a_k] > 0;
  }

  /**
   * @brief Whether slot @p a_k is an interior child.
   * @param[in] a_k Slot.
   * @return True if the slot holds a child node.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  isInterior(const size_t a_k) const noexcept
  {
    return m_count[a_k] == 0 && m_child[a_k] != EmptySlot;
  }

  /**
   * @brief Number of occupied slots. They are the first ones.
   * @return Number of leaf and interior slots.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline size_t
  numChildren() const noexcept
  {
    size_t n = 0;

    while (n < K && !this->isEmpty(n)) {
      n++;
    }

    return n;
  }

  /**
   * @brief Child node index of an interior slot.
   * @param[in] a_k Slot.
   * @return Node index.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getChild(const size_t a_k) const noexcept
  {
    EBGEOMETRY_EXPECT(this->isInterior(a_k));

    return m_child[a_k];
  }

  /**
   * @brief First primitive of a leaf slot.
   * @param[in] a_k Slot.
   * @return Index into the primitive array.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getPrimitivesOffset(const size_t a_k) const noexcept
  {
    EBGEOMETRY_EXPECT(this->isLeaf(a_k));

    return m_child[a_k];
  }

  /**
   * @brief Number of primitives in a leaf slot.
   * @param[in] a_k Slot.
   * @return Primitive count; zero for an interior or empty slot.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getNumPrimitives(const size_t a_k) const noexcept
  {
    return m_count[a_k];
  }

  /**
   * @brief Bounding box of slot @p a_k.
   * @param[in] a_k Slot.
   * @return The box; the default (inverted) box for an empty slot.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline BoundingVolumes::AABBT<T>
  getBoundingVolume(const size_t a_k) const noexcept
  {
    if (this->isEmpty(a_k)) {
      return BoundingVolumes::AABBT<T>();
    }

    return BoundingVolumes::AABBT<T>(Vec3T<T>(m_lo[0][a_k], m_lo[1][a_k], m_lo[2][a_k]),
                                     Vec3T<T>(m_hi[0][a_k], m_hi[1][a_k], m_hi[2][a_k]));
  }

  /**
   * @brief Bounding box of the whole node: the union of its slot boxes.
   * @return The box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline BoundingVolumes::AABBT<T>
  getBoundingVolume() const noexcept
  {
    BoundingVolumes::AABBT<T> box;

    for (size_t k = 0; k < K; k++) {
      if (!this->isEmpty(k)) {
        box = box.merged(this->getBoundingVolume(k));
      }
    }

    return box;
  }

  /**
   * @brief Squared distance from @p a_point to the box of slot @p a_k.
   * @details The same arithmetic, in the same order, as every path of PackedBVH's vectorised
   * distance kernel, so the two agree bit for bit.
   * @param[in] a_k     Slot.
   * @param[in] a_point Query point.
   * @return Squared distance; zero inside the box, +inf for an empty slot.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  getDistance2(const size_t a_k, const Vec3T<T>& a_point) const noexcept
  {
    T delta[3];

    for (size_t dir = 0; dir < 3; dir++) {
      const T lower = m_lo[dir][a_k] - a_point[dir];
      const T upper = a_point[dir] - m_hi[dir][a_k];
      const T d     = (upper > lower) ? upper : lower;

      delta[dir] = (d > T(0)) ? d : T(0);
    }

    return delta[0] * delta[0] + (delta[1] * delta[1] + delta[2] * delta[2]);
  }
};

static_assert(std::is_trivially_copyable_v<WideNode<float, 4>>, "WideNode must be trivially copyable");
static_assert(std::is_trivially_copyable_v<WideNode<double, 4>>, "WideNode must be trivially copyable");

/**
 * @brief The shape of a BVH, before any primitive is stored: its nodes, and the order the leaves
 * visit the input primitives in.
 * @details A leaf slot's primitive range [offset, offset + count) indexes @c order, and
 * @c order[i] is the index of an input primitive. The root is node 0, every interior child has a
 * higher index than its parent, and a BVH over no primitives has no nodes.
 * @tparam T Floating-point precision.
 * @tparam K Branching factor.
 */
template <class T, size_t K>
struct Topology
{
  /**
   * @brief Node array; root at index 0.
   */
  std::vector<WideNode<T, K>> nodes;

  /**
   * @brief Input primitive indices, in leaf order.
   */
  std::vector<uint32_t> order;
};

/**
 * @brief Build the shape of a BVH over a set of primitive bounding boxes.
 * @details The one builder behind every BVH in the library. Host-only. The primitives are known
 * only through their boxes; the centroid of each box stands in for its primitive where a strategy
 * needs a point.
 *
 * Every leaf holds at most @c a_spec.maxLeafSize primitives, and every node has at least two
 * occupied slots unless the root holds a single leaf, so a BVH over N primitives has at most
 * max(1, N - 1) nodes. If the requested strategy produces a tree deeper than DeviceTraversalDepth,
 * the build is repeated with Strategy::Centroid, whose equal-count splits keep the tree shallow.
 * @tparam T Floating-point precision.
 * @tparam K Branching factor.
 * @param[in] a_boxes One bounding box per primitive. Each must be finite and not inverted.
 * @param[in] a_spec  Build specification.
 * @return The topology.
 */
template <class T, size_t K>
[[nodiscard]] EBGEOMETRY_HOST
Topology<T, K>
buildTopology(const std::vector<BoundingVolumes::AABBT<T>>& a_boxes, const BuildSpec& a_spec);

/**
 * @brief Depth of a node array, in node levels (the root alone is depth 1; no nodes is depth 0).
 * @tparam T Floating-point precision.
 * @tparam K Branching factor.
 * @param[in] a_nodes Node array (root at index 0, children after parents).
 * @param[in] a_count Number of nodes.
 * @return Depth.
 */
template <class T, size_t K>
[[nodiscard]] EBGEOMETRY_HOST
size_t
treeDepth(const WideNode<T, K>* a_nodes, size_t a_count);

} // namespace BVH

} // namespace EBGeometry

#include "EBGeometry_BVHBuildImplem.hpp"

#endif
