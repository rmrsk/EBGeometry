// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_PackedBVHImplem.hpp
 * @brief  Implementation of EBGeometry_PackedBVH.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_PACKEDBVHIMPLEM_HPP
#define EBGEOMETRY_PACKEDBVHIMPLEM_HPP

// Std includes
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_Array.hpp"
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_PackedBVH.hpp"

namespace EBGeometry {

namespace BVH {

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline PackedBVH<T, P, K>::PackedBVH(const Layout& a_layout, const PoolLocation& a_location) noexcept
  : PoolHandle(a_location),
    m_linearNodes(a_layout.m_linearNodes),
    m_primitives(a_layout.m_primitives),
    m_childAabbSoA(a_layout.m_childAabbSoA)
{}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline typename PackedBVH<T, P, K>::Layout
PackedBVH<T, P, K>::layout() const noexcept
{
  return Layout{m_linearNodes, m_primitives, m_childAabbSoA};
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline const void*
PackedBVH<T, P, K>::base() const noexcept
{
  return PoolHandle::base();
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline PackedBVH<T, P, K>
PackedBVH<T, P, K>::rebasedView(const Pool& a_pool) const noexcept
{
  const uint64_t endByte =
    Math::max(m_linearNodes.endByte(), Math::max(m_primitives.endByte(), m_childAabbSoA.endByte()));

  PackedBVH view = *this;

  view.setLocation(this->rebasedLocation(a_pool, endByte, "BVH::PackedBVH"));

  if (view.location().m_control == nullptr) {
    // A snapshot is what a kernel receives. The device traversal stack is smaller than the host's, so
    // a tree that finalize() accepted can still be too deep to traverse on device. This is the moment
    // the caller commits to that, and the last one that still runs on the host where it can say so.
    this->requireDepthFits(this->base(), DeviceTraversalDepth, "device view");
  }

  return view;
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline PackedBVH<T, P, K>
PackedBVH<T, P, K>::deepCopy(Pool& a_dstPool) const
{
  const void* srcBase = this->base();

  std::vector<Node> nodes(m_linearNodes.size());
  std::vector<P>    prims(m_primitives.size());

  for (uint32_t i = 0; i < m_linearNodes.size(); i++) {
    nodes[i] = m_linearNodes.at(srcBase, i);
  }

  for (uint32_t i = 0; i < m_primitives.size(); i++) {
    prims[i] = m_primitives.at(srcBase, i);
  }

  PackedBVH copy(Layout{}, PoolLocation{});

  copy.finalize(a_dstPool, nodes, prims);

  return copy;
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline void
PackedBVH<T, P, K>::finalize(Pool& a_pool, const std::vector<Node>& a_linearNodes, const std::vector<P>& a_primitives)
{
  // Every build path ends here, so this checks the library's own builders as well as adopted arrays:
  // a malformed array -- a leaf with no primitives read as an interior node, say -- would otherwise
  // surface as an out-of-bounds read in Release.
  PackedBVH::requireWellFormed(a_linearNodes, a_primitives.size());

  // Every array of one BVH must come from the same pool -- offsets from two different blocks cannot
  // both resolve against one base.
  this->attach(a_pool, "BVH::PackedBVH");

  m_linearNodes.reserveFrom(a_pool, static_cast<uint32_t>(a_linearNodes.size()));
  m_primitives.reserveFrom(a_pool, static_cast<uint32_t>(a_primitives.size()));

  // Resolve the base only after every reservation above: a reserve can grow the pool, which moves
  // the block and invalidates any address taken before it.
  void* poolBase = const_cast<void*>(this->base());

  m_linearNodes.assign(poolBase, a_linearNodes.data(), static_cast<uint32_t>(a_linearNodes.size()));
  m_primitives.assign(poolBase, a_primitives.data(), static_cast<uint32_t>(a_primitives.size()));

  this->buildSoA(&a_pool);

  // buildSoA() reserves, so re-resolve; after it nothing else moves the block. Reject here, once,
  // rather than letting pruneTraverse walk off its fixed stack later with no way to notice.
  this->requireDepthFits(this->base(), HostTraversalDepth, "host build");
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline void
PackedBVH<T, P, K>::buildSoA(Pool* a_pool)
{
  // Only interior nodes have children, so only they get a row; a leaf, typically most of the nodes,
  // gets none.
  uint32_t numInterior = 0;

  for (uint32_t i = 0; i < m_linearNodes.size(); i++) {
    if (!m_linearNodes.at(this->base(), i).isLeaf()) {
      numInterior++;
    }
  }

  if (a_pool != nullptr) {
    m_childAabbSoA.reserveFrom(*a_pool, numInterior);
  }

  EBGEOMETRY_EXPECT(m_childAabbSoA.m_capacity == numInterior);

  // Resolved after the reservation above, for the reason given in finalize().
  void* poolBase = const_cast<void*>(this->base());

  // Assembled host-side and copied in one shot, like the node and primitive arrays: a PODVector
  // never reallocates, so it has no way to be filled element-by-element without its final size
  // already fixed, and this keeps the one finalize pattern everywhere.
  std::vector<ChildAABBSoA> soaCache(numInterior);

  uint32_t row = 0;

  for (uint32_t i = 0; i < m_linearNodes.size(); i++) {
    Node& node = m_linearNodes.at(poolBase, i);

    if (!node.isLeaf()) {
      const auto& offsets = node.getChildOffsets();
      auto&       soa     = soaCache[row];

      node.m_childBoxRow = row++;

      for (size_t k = 0; k < K; k++) {
        const auto& bv = m_linearNodes.at(poolBase, offsets[k]).getBoundingVolume();
        const auto& lo = bv.getLowCorner();
        const auto& hi = bv.getHighCorner();

        soa.m_lo[0][k] = lo[0];
        soa.m_lo[1][k] = lo[1];
        soa.m_lo[2][k] = lo[2];

        soa.m_hi[0][k] = hi[0];
        soa.m_hi[1][k] = hi[1];
        soa.m_hi[2][k] = hi[2];
      }
    }
  }

  m_childAabbSoA.assign(poolBase, soaCache.data(), numInterior);
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline PODSpan<const P>
PackedBVH<T, P, K>::getPrimitives() const noexcept
{
  return m_primitives.bind(this->base());
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline PODSpan<P>
PackedBVH<T, P, K>::getPrimitives() noexcept
{
  return m_primitives.bind(const_cast<void*>(this->base()));
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline PODSpan<const typename PackedBVH<T, P, K>::Node>
PackedBVH<T, P, K>::getNodes() const noexcept
{
  return m_linearNodes.bind(this->base());
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline const EBGeometry::BoundingVolumes::AABBT<T>&
PackedBVH<T, P, K>::getBoundingVolume() const noexcept
{
  EBGEOMETRY_EXPECT(m_linearNodes.size() > 0);

  return m_linearNodes.at(this->base(), 0).getBoundingVolume();
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline EBGeometry::BoundingVolumes::AABBT<T>
PackedBVH<T, P, K>::computeBoundingVolume() const noexcept
{
  if (m_linearNodes.size() == 0) {
    return EBGeometry::BoundingVolumes::AABBT<T>();
  }

  return m_linearNodes.at(this->base(), 0).getBoundingVolume();
}

template <class T, class P, size_t K>
template <class NodeKey, class LeafEvaluator, class PrunePredicate, class ChildOrderer, class NodeKeyFactory>
EBGEOMETRY_HOST_DEVICE
inline void
PackedBVH<T, P, K>::traverse(LeafEvaluator&&  a_leafEvaluator,
                             PrunePredicate&& a_prunePredicate,
                             ChildOrderer&&   a_childOrderer,
                             NodeKeyFactory&& a_nodeKeyFactory) const noexcept
{
  // The key type: as given, or what the node-key factory returns.
  using Key = std::
    conditional_t<std::is_void_v<NodeKey>, std::decay_t<std::invoke_result_t<NodeKeyFactory&, const Node&>>, NodeKey>;
  using Entry = BVH::NodeAndKey<Key>;

  // An empty BVH has no root to descend from.
  if (m_linearNodes.size() == 0) {
    return;
  }

  // One resolution for the whole traversal. Safe because a query performs no reservation, so the
  // base cannot move underneath it.
  const void* poolBase = this->base();

  // One entry per interior node on the path from the root, so a tree of depth D needs at most D - 1
  // entries -- the bound traversalStackDepth() gives, and that every build checked the tree against.
  constexpr size_t stackDepth = PackedBVH::traversalStackDepth();

  StackEntry stack[stackDepth];
  size_t     top = 0;

  Array<Entry, K> children;

  uint32_t nodeIdx = 0U;
  Key      key     = static_cast<Key>(a_nodeKeyFactory(m_linearNodes.at(poolBase, 0)));

  while (true) {
    // Visit nodeIdx, and keep descending into the first child in visit order.
    while (true) {
      const Node& node = m_linearNodes.at(poolBase, nodeIdx);

      if (!a_prunePredicate(node, key)) {
        break;
      }

      if (node.isLeaf()) {
        a_leafEvaluator(
          m_primitives.bind(poolBase), size_t(node.getPrimitivesOffset()), size_t(node.getNumPrimitives()));

        break;
      }

      const auto& offsets = node.getChildOffsets();

      for (size_t k = 0; k < K; k++) {
        children[k] = Entry{offsets[k], static_cast<Key>(a_nodeKeyFactory(m_linearNodes.at(poolBase, offsets[k])))};
      }

      a_childOrderer(children);

      // children[K - 1] is visited now and children[0] last. The others wait on the stack as slots,
      // in the same order; their keys are computed again when they are visited.
      EBGEOMETRY_EXPECT(top < stackDepth);

      StackEntry& entry = stack[top++];

      entry.m_node  = nodeIdx;
      entry.m_count = static_cast<uint8_t>(K - 1);

      for (size_t i = 0; i < K - 1; i++) {
        size_t slot = 0;

        while (slot < K && offsets[slot] != children[i].first) {
          slot++;
        }

        EBGEOMETRY_EXPECT(slot < K);

        entry.m_slots[i] = static_cast<uint8_t>(slot);
      }

      nodeIdx = children[K - 1].first;
      key     = children[K - 1].second;
    }

    // Return to the deepest level with a child still to visit.
    if (top == 0) {
      return;
    }

    StackEntry& entry = stack[top - 1];

    const uint8_t slot = entry.m_slots[--entry.m_count];

    if (entry.m_count == 0) {
      top--;
    }

    nodeIdx = m_linearNodes.at(poolBase, entry.m_node).getChildOffsets()[slot];
    key     = static_cast<Key>(a_nodeKeyFactory(m_linearNodes.at(poolBase, nodeIdx)));
  }
}

template <class T, class P, size_t K>
inline size_t
PackedBVH<T, P, K>::maxNodeDepth(const void* a_base) const
{
  if (m_linearNodes.size() == 0) {
    return 0;
  }

  size_t maxDepth = 0;

  // (node index, depth of that node). A host-side build/mirror step, so std::vector is fine here --
  // this is precisely the heap stack pruneTraverse itself can no longer use.
  std::vector<std::pair<uint32_t, size_t>> stack;

  stack.reserve(64);
  stack.emplace_back(uint32_t(0), size_t(1));

  while (!stack.empty()) {
    const uint32_t idx   = stack.back().first;
    const size_t   depth = stack.back().second;

    stack.pop_back();

    if (depth > maxDepth) {
      maxDepth = depth;
    }

    const Node& node = m_linearNodes.at(a_base, idx);

    if (!node.isLeaf()) {
      const auto& offsets = node.getChildOffsets();

      for (size_t k = 0; k < K; k++) {
        stack.emplace_back(offsets[k], depth + 1);
      }
    }
  }

  return maxDepth;
}

template <class T, class P, size_t K>
inline void
PackedBVH<T, P, K>::requireDepthFits(const void* a_base, const size_t a_maxDepth, const char* a_context) const
{
  const size_t depth = this->maxNodeDepth(a_base);

  EBGEOMETRY_REQUIRE(depth <= a_maxDepth,
                     "BVH::PackedBVH: %s -- the tree is %zu levels deep, more than the %zu levels a traversal stack "
                     "holds.\n"
                     "  pruneTraverse would overflow its fixed stack, which Release builds do not detect. Rebuild "
                     "with a larger leaf size, or a partitioner that splits more evenly, before querying this BVH.",
                     a_context,
                     depth,
                     a_maxDepth);
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline void
PackedBVH<T, P, K>::requireWellFormed(const std::vector<Node>& a_linearNodes, const size_t a_numPrimitives)
{
  const size_t numNodes = a_linearNodes.size();

  // Report the first defect and stop: once one offset is wrong, later ones say nothing reliable. The
  // array must be a depth-first pre-order flattening: every child strictly after its parent and
  // inside the array, every leaf's primitives inside the primitive array.
  if (numNodes == 0) {
    EBGEOMETRY_REQUIRE(a_numPrimitives == 0,
                       "BVH::PackedBVH: adopted node array is malformed -- it is empty, but the primitive "
                       "array holds %zu primitives",
                       a_numPrimitives);

    return;
  }

  // Every node but the root has exactly one parent, so a traversal reaches each primitive once.
  std::vector<bool> hasParent(numNodes, false);

  for (size_t i = 0; i < numNodes; i++) {
    const Node& node = a_linearNodes[i];

    if (node.isLeaf()) {
      const size_t end = size_t(node.getPrimitivesOffset()) + size_t(node.getNumPrimitives());

      EBGEOMETRY_REQUIRE(end <= a_numPrimitives,
                         "BVH::PackedBVH: node array is malformed -- leaf %zu's primitive range ends at "
                         "%zu, past the primitive array's %zu primitives",
                         i,
                         end,
                         a_numPrimitives);

      continue;
    }

    // An interior node. A node meant as a leaf but holding no primitives lands here too, with its
    // child offsets unset (zero), and the first check below rejects it.
    for (const uint32_t child : node.getChildOffsets()) {
      EBGEOMETRY_REQUIRE(child > i && child < numNodes,
                         "BVH::PackedBVH: node array is malformed -- node %zu has child offset %zu, which "
                         "is not strictly after its parent and inside the array of %zu nodes (a leaf with no "
                         "primitives reads as an interior node like this)",
                         i,
                         size_t(child),
                         numNodes);

      EBGEOMETRY_REQUIRE(!hasParent[child],
                         "BVH::PackedBVH: node array is malformed -- node %zu names child %zu, which already has "
                         "another parent",
                         i,
                         size_t(child));

      hasParent[child] = true;
    }
  }

  for (size_t i = 1; i < numNodes; i++) {
    EBGEOMETRY_REQUIRE(hasParent[i], "BVH::PackedBVH: node array is malformed -- node %zu has no parent", i);
  }
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline void
PackedBVH<T, P, K>::computeChildDistances2(const ChildAABBSoA& a_soa, const Vec3T<T>& a_point, T (&a_dist2)[K]) noexcept
{
  // Device code takes the scalar path unconditionally: the x86 ISA macros below may still be
  // defined during nvcc's/hipcc's *host* pass over this same __host__ __device__ function, so the
  // guard has to be on the compilation pass rather than on the intrinsics being available.
#if !defined(EBGEOMETRY_DEVICE_COMPILE)

  // ──────────────────────────────────────────────────────────────────────────────
  // AVX-512F paths: K==8/double and K==16/float.
  //
  // These appear first and return, so on -mavx512f hardware the compiler dead-code-eliminates the
  // corresponding AVX branches below.
  //
  // Alignment: ChildAABBSoA is alignas(sizeof(T)*K), which is 64 bytes for both (K=8, T=double) and
  // (K=16, T=float) -- exactly what _mm512_load_pd / _mm512_load_ps require. The static_asserts
  // catch any mismatch. The *store* into a_dist2 is unaligned (storeu), so callers need not align
  // their output buffer; on an aligned address storeu costs the same as store on every CPU that has
  // AVX-512 at all.
  //
  // Recommended configurations on AVX-512 hardware:
  //   float  -> K=16, W=16  (one _mm512_load_ps covers all children and one leaf group)
  //   double -> K=8,  W=8   (one _mm512_load_pd covers all children; AVX-512F replaces
  //                          the 2x_mm256_load_pd emulation in the AVX fallback below)
  // ──────────────────────────────────────────────────────────────────────────────
#if defined(__AVX512F__)
  if constexpr (K == 8 && std::is_same_v<T, double>) {
    static_assert(alignof(ChildAABBSoA) == sizeof(T) * K,
                  "ChildAABBSoA alignment mismatch: _mm512_load_pd requires 64-byte alignment");

    const __m512d px   = _mm512_set1_pd(a_point[0]);
    const __m512d py   = _mm512_set1_pd(a_point[1]);
    const __m512d pz   = _mm512_set1_pd(a_point[2]);
    const __m512d zero = _mm512_setzero_pd();

    const __m512d lo_x = _mm512_load_pd(a_soa.m_lo[0]);
    const __m512d lo_y = _mm512_load_pd(a_soa.m_lo[1]);
    const __m512d lo_z = _mm512_load_pd(a_soa.m_lo[2]);
    const __m512d hi_x = _mm512_load_pd(a_soa.m_hi[0]);
    const __m512d hi_y = _mm512_load_pd(a_soa.m_hi[1]);
    const __m512d hi_z = _mm512_load_pd(a_soa.m_hi[2]);

    const __m512d dx = _mm512_max_pd(zero, _mm512_max_pd(_mm512_sub_pd(lo_x, px), _mm512_sub_pd(px, hi_x)));
    const __m512d dy = _mm512_max_pd(zero, _mm512_max_pd(_mm512_sub_pd(lo_y, py), _mm512_sub_pd(py, hi_y)));
    const __m512d dz = _mm512_max_pd(zero, _mm512_max_pd(_mm512_sub_pd(lo_z, pz), _mm512_sub_pd(pz, hi_z)));
    const __m512d d2 =
      _mm512_add_pd(_mm512_mul_pd(dx, dx), _mm512_add_pd(_mm512_mul_pd(dy, dy), _mm512_mul_pd(dz, dz)));

    _mm512_storeu_pd(a_dist2, d2);

    return;
  }

  if constexpr (K == 16 && std::is_same_v<T, float>) {
    static_assert(alignof(ChildAABBSoA) == sizeof(T) * K,
                  "ChildAABBSoA alignment mismatch: _mm512_load_ps requires 64-byte alignment");

    const __m512 px   = _mm512_set1_ps((float)a_point[0]);
    const __m512 py   = _mm512_set1_ps((float)a_point[1]);
    const __m512 pz   = _mm512_set1_ps((float)a_point[2]);
    const __m512 zero = _mm512_setzero_ps();

    const __m512 lo_x = _mm512_load_ps(a_soa.m_lo[0]);
    const __m512 lo_y = _mm512_load_ps(a_soa.m_lo[1]);
    const __m512 lo_z = _mm512_load_ps(a_soa.m_lo[2]);
    const __m512 hi_x = _mm512_load_ps(a_soa.m_hi[0]);
    const __m512 hi_y = _mm512_load_ps(a_soa.m_hi[1]);
    const __m512 hi_z = _mm512_load_ps(a_soa.m_hi[2]);

    const __m512 dx = _mm512_max_ps(zero, _mm512_max_ps(_mm512_sub_ps(lo_x, px), _mm512_sub_ps(px, hi_x)));
    const __m512 dy = _mm512_max_ps(zero, _mm512_max_ps(_mm512_sub_ps(lo_y, py), _mm512_sub_ps(py, hi_y)));
    const __m512 dz = _mm512_max_ps(zero, _mm512_max_ps(_mm512_sub_ps(lo_z, pz), _mm512_sub_ps(pz, hi_z)));
    const __m512 d2 = _mm512_add_ps(_mm512_mul_ps(dx, dx), _mm512_add_ps(_mm512_mul_ps(dy, dy), _mm512_mul_ps(dz, dz)));

    _mm512_storeu_ps(a_dist2, d2);

    return;
  }
#endif // __AVX512F__

  // ──────────────────────────────────────────────────────────────────────────────
  // AVX paths: K==4/double (single pass), K==8/float (single pass),
  //            K==8/double (two 4-wide passes -- superseded by AVX-512F above).
  // ──────────────────────────────────────────────────────────────────────────────
#if defined(__AVX__)
  if constexpr (K == 4 && std::is_same_v<T, double>) {
    static_assert(alignof(ChildAABBSoA) == sizeof(T) * K,
                  "ChildAABBSoA alignment mismatch: _mm256_load_pd requires 32-byte alignment");

    const __m256d px   = _mm256_set1_pd(a_point[0]);
    const __m256d py   = _mm256_set1_pd(a_point[1]);
    const __m256d pz   = _mm256_set1_pd(a_point[2]);
    const __m256d zero = _mm256_setzero_pd();

    const __m256d lo_x = _mm256_load_pd(a_soa.m_lo[0]);
    const __m256d lo_y = _mm256_load_pd(a_soa.m_lo[1]);
    const __m256d lo_z = _mm256_load_pd(a_soa.m_lo[2]);
    const __m256d hi_x = _mm256_load_pd(a_soa.m_hi[0]);
    const __m256d hi_y = _mm256_load_pd(a_soa.m_hi[1]);
    const __m256d hi_z = _mm256_load_pd(a_soa.m_hi[2]);

    const __m256d dx = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_x, px), _mm256_sub_pd(px, hi_x)));
    const __m256d dy = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_y, py), _mm256_sub_pd(py, hi_y)));
    const __m256d dz = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_z, pz), _mm256_sub_pd(pz, hi_z)));
    const __m256d d2 =
      _mm256_add_pd(_mm256_mul_pd(dx, dx), _mm256_add_pd(_mm256_mul_pd(dy, dy), _mm256_mul_pd(dz, dz)));

    _mm256_storeu_pd(a_dist2, d2);

    return;
  }

  if constexpr (K == 8 && std::is_same_v<T, float>) {
    static_assert(alignof(ChildAABBSoA) == sizeof(T) * K,
                  "ChildAABBSoA alignment mismatch: _mm256_load_ps requires 32-byte alignment");

    const __m256 px   = _mm256_set1_ps((float)a_point[0]);
    const __m256 py   = _mm256_set1_ps((float)a_point[1]);
    const __m256 pz   = _mm256_set1_ps((float)a_point[2]);
    const __m256 zero = _mm256_setzero_ps();

    const __m256 lo_x = _mm256_load_ps(a_soa.m_lo[0]);
    const __m256 lo_y = _mm256_load_ps(a_soa.m_lo[1]);
    const __m256 lo_z = _mm256_load_ps(a_soa.m_lo[2]);
    const __m256 hi_x = _mm256_load_ps(a_soa.m_hi[0]);
    const __m256 hi_y = _mm256_load_ps(a_soa.m_hi[1]);
    const __m256 hi_z = _mm256_load_ps(a_soa.m_hi[2]);

    const __m256 dx = _mm256_max_ps(zero, _mm256_max_ps(_mm256_sub_ps(lo_x, px), _mm256_sub_ps(px, hi_x)));
    const __m256 dy = _mm256_max_ps(zero, _mm256_max_ps(_mm256_sub_ps(lo_y, py), _mm256_sub_ps(py, hi_y)));
    const __m256 dz = _mm256_max_ps(zero, _mm256_max_ps(_mm256_sub_ps(lo_z, pz), _mm256_sub_ps(pz, hi_z)));
    const __m256 d2 = _mm256_add_ps(_mm256_mul_ps(dx, dx), _mm256_add_ps(_mm256_mul_ps(dy, dy), _mm256_mul_ps(dz, dz)));

    _mm256_storeu_ps(a_dist2, d2);

    return;
  }

  if constexpr (K == 8 && std::is_same_v<T, double>) {
    static_assert(alignof(ChildAABBSoA) == sizeof(T) * K,
                  "ChildAABBSoA alignment mismatch: _mm256_load_pd requires 32-byte alignment");

    const __m256d px   = _mm256_set1_pd(a_point[0]);
    const __m256d py   = _mm256_set1_pd(a_point[1]);
    const __m256d pz   = _mm256_set1_pd(a_point[2]);
    const __m256d zero = _mm256_setzero_pd();

    const __m256d lo_x0 = _mm256_load_pd(a_soa.m_lo[0]);
    const __m256d lo_y0 = _mm256_load_pd(a_soa.m_lo[1]);
    const __m256d lo_z0 = _mm256_load_pd(a_soa.m_lo[2]);
    const __m256d hi_x0 = _mm256_load_pd(a_soa.m_hi[0]);
    const __m256d hi_y0 = _mm256_load_pd(a_soa.m_hi[1]);
    const __m256d hi_z0 = _mm256_load_pd(a_soa.m_hi[2]);

    const __m256d dx0 = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_x0, px), _mm256_sub_pd(px, hi_x0)));
    const __m256d dy0 = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_y0, py), _mm256_sub_pd(py, hi_y0)));
    const __m256d dz0 = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_z0, pz), _mm256_sub_pd(pz, hi_z0)));
    const __m256d d2_0 =
      _mm256_add_pd(_mm256_mul_pd(dx0, dx0), _mm256_add_pd(_mm256_mul_pd(dy0, dy0), _mm256_mul_pd(dz0, dz0)));

    const __m256d lo_x1 = _mm256_load_pd(a_soa.m_lo[0] + 4);
    const __m256d lo_y1 = _mm256_load_pd(a_soa.m_lo[1] + 4);
    const __m256d lo_z1 = _mm256_load_pd(a_soa.m_lo[2] + 4);
    const __m256d hi_x1 = _mm256_load_pd(a_soa.m_hi[0] + 4);
    const __m256d hi_y1 = _mm256_load_pd(a_soa.m_hi[1] + 4);
    const __m256d hi_z1 = _mm256_load_pd(a_soa.m_hi[2] + 4);

    const __m256d dx1 = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_x1, px), _mm256_sub_pd(px, hi_x1)));
    const __m256d dy1 = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_y1, py), _mm256_sub_pd(py, hi_y1)));
    const __m256d dz1 = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_z1, pz), _mm256_sub_pd(pz, hi_z1)));
    const __m256d d2_1 =
      _mm256_add_pd(_mm256_mul_pd(dx1, dx1), _mm256_add_pd(_mm256_mul_pd(dy1, dy1), _mm256_mul_pd(dz1, dz1)));

    _mm256_storeu_pd(a_dist2, d2_0);
    _mm256_storeu_pd(a_dist2 + 4, d2_1);

    return;
  }
#endif // __AVX__

#if defined(__SSE4_1__)
  if constexpr (K == 4 && std::is_same_v<T, float>) {
    static_assert(alignof(ChildAABBSoA) == sizeof(T) * K,
                  "ChildAABBSoA alignment mismatch: _mm_load_ps requires 16-byte alignment");

    const __m128 px   = _mm_set1_ps((float)a_point[0]);
    const __m128 py   = _mm_set1_ps((float)a_point[1]);
    const __m128 pz   = _mm_set1_ps((float)a_point[2]);
    const __m128 zero = _mm_setzero_ps();

    const __m128 lo_x = _mm_load_ps(a_soa.m_lo[0]);
    const __m128 lo_y = _mm_load_ps(a_soa.m_lo[1]);
    const __m128 lo_z = _mm_load_ps(a_soa.m_lo[2]);
    const __m128 hi_x = _mm_load_ps(a_soa.m_hi[0]);
    const __m128 hi_y = _mm_load_ps(a_soa.m_hi[1]);
    const __m128 hi_z = _mm_load_ps(a_soa.m_hi[2]);

    const __m128 dx = _mm_max_ps(zero, _mm_max_ps(_mm_sub_ps(lo_x, px), _mm_sub_ps(px, hi_x)));
    const __m128 dy = _mm_max_ps(zero, _mm_max_ps(_mm_sub_ps(lo_y, py), _mm_sub_ps(py, hi_y)));
    const __m128 dz = _mm_max_ps(zero, _mm_max_ps(_mm_sub_ps(lo_z, pz), _mm_sub_ps(pz, hi_z)));
    const __m128 d2 = _mm_add_ps(_mm_mul_ps(dx, dx), _mm_add_ps(_mm_mul_ps(dy, dy), _mm_mul_ps(dz, dz)));

    _mm_storeu_ps(a_dist2, d2);

    return;
  }
#endif // __SSE4_1__

#endif // !EBGEOMETRY_DEVICE_COMPILE

  // Scalar path: every (T, K) with no compiled ISA path above, and all device code.
  //
  // The per-axis clamp and the dx*dx + (dy*dy + dz*dz) association below are written out so that
  // they match the SIMD paths exactly, and every path produces bit-identical results for finite inputs,
  // unless the compiler contracts the multiply and add below into a fused multiply-add.

  for (size_t k = 0; k < K; k++) {
    a_dist2[k] = PackedBVH::scalarChildDistance2(a_soa, a_point, k);
  }
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline T
PackedBVH<T, P, K>::scalarChildDistance2(const ChildAABBSoA& a_soa,
                                         const Vec3T<T>&     a_point,
                                         const size_t        a_slot) noexcept
{
  T delta[3];

  for (size_t dir = 0; dir < 3; dir++) {
    const T p     = a_point[dir];
    const T lower = a_soa.m_lo[dir][a_slot] - p;
    const T upper = p - a_soa.m_hi[dir][a_slot];

    const T d = (upper > lower) ? upper : lower;

    delta[dir] = (d > T(0.0)) ? d : T(0.0);
  }

  return delta[0] * delta[0] + (delta[1] * delta[1] + delta[2] * delta[2]);
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline T
PackedBVH<T, P, K>::childDistance2(const ChildAABBSoA& a_soa, const Vec3T<T>& a_point, const size_t a_slot) noexcept
{
#if defined(EBGEOMETRY_DEVICE_COMPILE)
  return PackedBVH::scalarChildDistance2(a_soa, a_point, a_slot);
#else
  // The SIMD paths compute the same quantity, but a compiler may contract the scalar expression into
  // fused multiply-adds where the intrinsics do not, so the row goes through the path that sorted it.
  T dist2[K];

  PackedBVH::computeChildDistances2(a_soa, a_point, dist2);

  return dist2[a_slot];
#endif
}

template <class T, class P, size_t K>
template <class State, class LeafEvaluator, class PruneDistSquared>
EBGEOMETRY_HOST_DEVICE
inline void
PackedBVH<T, P, K>::pruneTraverse(const Vec3T<T>&    a_point,
                                  State&             a_state,
                                  LeafEvaluator&&    a_evalLeaf,
                                  PruneDistSquared&& a_pruneDist2) const noexcept
{
  // An empty BVH has no root to descend from. Nothing that builds through a TreeBVH or through the
  // direct constructors can reach this -- they assert a non-empty primitive list and would have
  // died long before -- but finalize() will happily produce a zero-node BVH from an empty build
  // result, which is how PointCloudBVH represents an empty cloud (it guards its own traversal the
  // same way). Guarding here rather than at each call site means the pool-backed build path cannot
  // grow a caller that reads node 0 out of bounds, which in Release is silent.
  if (m_linearNodes.size() == 0) {
    return;
  }

  // One resolution for the whole traversal. Safe because a query performs no reservation, so the
  // base cannot move underneath it.
  const void* poolBase = this->base();

  // One entry per interior node on the path from the root, so a tree of depth D needs at most D - 1
  // entries -- the bound traversalStackDepth() gives, and that every build checked the tree against.
  constexpr size_t stackDepth = PackedBVH::traversalStackDepth();

  StackEntry stack[stackDepth];
  size_t     top = 0;

  // The root's box contains everything, so its distance is taken as zero.
  if (T(0) > a_pruneDist2(a_state)) {
    return;
  }

  uint32_t nodeIdx = 0U;

  while (true) {
    // Visit nodeIdx, and keep descending into the nearest child within the bound.
    while (true) {
      const Node& node = m_linearNodes.at(poolBase, nodeIdx);

      if (node.isLeaf()) {
        a_evalLeaf(a_state, node.getPrimitivesOffset(), node.getNumPrimitives());

        break;
      }

      // Read the pruning bound once per node expanded. A leaf visited anywhere earlier -- in any
      // subtree, not just this one -- may have tightened it.
      const T pruneDist2 = a_pruneDist2(a_state);

      T dist2[K];

      PackedBVH::computeChildDistances2(m_childAabbSoA.at(poolBase, node.getChildBoxRow()), a_point, dist2);

      const auto& offsets = node.getChildOffsets();

      // The children within the bound, sorted into descending distance order, so the nearest is last.
      // It is visited now and the others wait on the stack in the same order, which is what makes the
      // bound tighten quickly. Children out of bound never reach the stack.
      //
      // Insertion sort, not std::sort: K is a handful of elements, where insertion sort is simply
      // faster than an introsort's setup, and std::sort is not callable from device code. It is also
      // stable, so children whose boxes are exactly equidistant keep their child-slot order rather
      // than an unspecified one, which keeps traversal order deterministic.
      uint8_t slots[K];
      size_t  numSlots = 0;

      for (size_t k = 0; k < K; k++) {
        EBGEOMETRY_EXPECT(offsets[k] > nodeIdx);

        if (dist2[k] <= pruneDist2) {
          size_t j = numSlots++;

          while (j > 0 && dist2[slots[j - 1]] < dist2[k]) {
            slots[j] = slots[j - 1];
            j--;
          }

          slots[j] = static_cast<uint8_t>(k);
        }
      }

      if (numSlots == 0) {
        break;
      }

      if (numSlots > 1) {
        EBGEOMETRY_EXPECT(top < stackDepth);

        StackEntry& entry = stack[top++];

        entry.m_node  = nodeIdx;
        entry.m_count = static_cast<uint8_t>(numSlots - 1);

        for (size_t i = 0; i + 1 < numSlots; i++) {
          entry.m_slots[i] = slots[i];
        }
      }

      nodeIdx = offsets[slots[numSlots - 1]];
    }

    // Return to the deepest level with a child still within the bound. The bound may have tightened
    // since the level was sorted; its children wait nearest last, so the first one out of bound
    // drops the rest of the level with it.
    bool resumed = false;

    while (top > 0 && !resumed) {
      StackEntry&   entry  = stack[top - 1];
      const Node&   parent = m_linearNodes.at(poolBase, entry.m_node);
      const uint8_t slot   = entry.m_slots[entry.m_count - 1];

      const T dist2 =
        PackedBVH::childDistance2(m_childAabbSoA.at(poolBase, parent.getChildBoxRow()), a_point, size_t(slot));

      if (dist2 > a_pruneDist2(a_state)) {
        top--;

        continue;
      }

      if (--entry.m_count == 0) {
        top--;
      }

      nodeIdx = parent.getChildOffsets()[slot];
      resumed = true;
    }

    if (!resumed) {
      return;
    }
  }
}

template <class T, class P, size_t K>
template <class BVConstructor>
inline void
PackedBVH<T, P, K>::refit(const BVConstructor& a_bvConstructor)
{
  // m_linearNodes is a depth-first pre-order flattening, so every child has a higher index than its
  // parent. Sweeping the array in reverse therefore refits all of a node's children before the node
  // itself -- no recursion or explicit stack needed. Boxes are merged pairwise, so a refit (meant as
  // cheap per-frame maintenance) allocates nothing.
  //
  // refit() reserves nothing, so one resolution covers the whole sweep.
  void* poolBase = const_cast<void*>(this->base());

  for (uint32_t i = m_linearNodes.size(); i-- > 0;) {
    Node& node = m_linearNodes.at(poolBase, i);

    BV bv;

    if (node.isLeaf()) {
      const uint32_t offset = node.getPrimitivesOffset();
      const uint32_t count  = node.getNumPrimitives();

      for (uint32_t p = 0; p < count; p++) {
        bv = bv.merged(a_bvConstructor(m_primitives.at(poolBase, offset + p)));
      }
    }
    else {
      const auto& childOffsets = node.getChildOffsets();

      for (size_t k = 0; k < K; k++) {
        bv = bv.merged(m_linearNodes.at(poolBase, childOffsets[k]).getBoundingVolume());
      }
    }

    node.setBoundingVolume(bv);
  }

  // Node count is unchanged, so the existing cache is refilled rather than re-reserved.
  this->buildSoA(nullptr);
}

} // namespace BVH

} // namespace EBGeometry

#endif
