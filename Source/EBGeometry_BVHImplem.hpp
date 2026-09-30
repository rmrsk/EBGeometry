// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_BVHImplem.hpp
 * @brief  Implementation of EBGeometry_BVH.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_BVHIMPLEM_HPP
#define EBGEOMETRY_BVHIMPLEM_HPP

// Std includes
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_BVH.hpp"
#include "EBGeometry_Math.hpp"

namespace EBGeometry {

namespace BVH {

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline PackedBVH<T, P, K>::PackedBVH(Pool& a_pool, std::vector<std::pair<P, BV>> a_primsAndBVs, const BuildSpec& a_spec)
{
  std::vector<BV> boxes;

  boxes.reserve(a_primsAndBVs.size());

  for (const auto& primAndBV : a_primsAndBVs) {
    boxes.push_back(primAndBV.second);
  }

  const Topology<T, K> topology = buildTopology<T, K>(boxes, a_spec);

  std::vector<P> primitives;

  primitives.reserve(a_primsAndBVs.size());

  for (const uint32_t item : topology.order) {
    primitives.push_back(std::move(a_primsAndBVs[item].first));
  }

  this->finalize(a_pool, topology.nodes, primitives);
}

template <class T, class P, size_t K>
template <class PackLeaf>
EBGEOMETRY_HOST
inline PackedBVH<T, P, K>::PackedBVH(Pool& a_pool, const Topology<T, K>& a_topology, PackLeaf&& a_packLeaf)
{
  std::vector<Node> nodes = a_topology.nodes;
  std::vector<P>    primitives;

  // Leaves are packed in the order their ranges appear in the topology's order array, which is the
  // order a depth-first walk visits them in, so the stored primitives stay in leaf order.
  std::vector<std::pair<uint32_t, size_t>> leaves;

  for (uint32_t n = 0; n < nodes.size(); n++) {
    for (size_t k = 0; k < K; k++) {
      if (nodes[n].isLeaf(k)) {
        leaves.emplace_back(n, k);
      }
    }
  }

  std::sort(leaves.begin(), leaves.end(), [&nodes](const auto& a_lhs, const auto& a_rhs) noexcept {
    return nodes[a_lhs.first].m_child[a_lhs.second] < nodes[a_rhs.first].m_child[a_rhs.second];
  });

  for (const auto& leaf : leaves) {
    Node&          node  = nodes[leaf.first];
    const size_t   k     = leaf.second;
    const uint32_t first = node.m_child[k];
    const uint32_t count = node.m_count[k];

    EBGEOMETRY_REQUIRE(size_t(first) + size_t(count) <= a_topology.order.size(),
                       "BVH::PackedBVH: a leaf of the topology indexes past its order array (%zu > %zu)",
                       size_t(first) + size_t(count),
                       a_topology.order.size());

    const size_t before = primitives.size();

    a_packLeaf(a_topology.order.data() + first, count, primitives);

    const size_t packed = primitives.size() - before;

    EBGEOMETRY_REQUIRE(packed > 0, "BVH::PackedBVH: the leaf packer must store at least one primitive per leaf");
    EBGEOMETRY_REQUIRE(primitives.size() < size_t(Node::EmptySlot),
                       "BVH::PackedBVH: %zu packed primitives is more than a BVH can index",
                       primitives.size());

    node.m_child[k] = static_cast<uint32_t>(before);
    node.m_count[k] = static_cast<uint32_t>(packed);
  }

  this->finalize(a_pool, nodes, primitives);
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline PackedBVH<T, P, K>::PackedBVH(Pool& a_pool, const std::vector<Node>& a_nodes, const std::vector<P>& a_primitives)
{
  PackedBVH::requireWellFormed(a_nodes.data(), a_nodes.size(), a_primitives.size());

  this->finalize(a_pool, a_nodes, a_primitives);
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline PackedBVH<T, P, K>::PackedBVH(Pool& a_pool, const PODVector<Node>& a_nodes, const PODVector<P>& a_primitives)
  : m_nodes(a_nodes), m_primitives(a_primitives)
{
  this->attachTo(a_pool);

  const uint64_t endByte = Math::max(m_nodes.endByte(), m_primitives.endByte());

  EBGEOMETRY_REQUIRE(endByte <= a_pool.usedBytes(),
                     "BVH::PackedBVH: the adopted arrays end at byte %llu, past the %llu bytes reserved from the pool",
                     static_cast<unsigned long long>(endByte),
                     static_cast<unsigned long long>(a_pool.usedBytes()));

  // A pool in device-only memory cannot be read here; its arrays are the caller's responsibility.
  if (a_pool.resource().isHostAccessible()) {
    const void* poolBase = this->base();

    PackedBVH::requireWellFormed(m_nodes.data(poolBase), m_nodes.size(), m_primitives.size());

    this->requireDepthFits(poolBase, HostTraversalDepth, "host build");
  }
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline void
PackedBVH<T, P, K>::attachTo(Pool& a_pool) noexcept
{
  // Every array of one BVH must come from the same pool -- offsets from two different blocks cannot
  // both resolve against one base.
  m_location.attach(a_pool, "BVH::PackedBVH");
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline const void*
PackedBVH<T, P, K>::base() const noexcept
{
  return m_location.base();
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline bool
PackedBVH<T, P, K>::isAttachedTo(const Pool& a_pool) const noexcept
{
  return m_location.isAttachedTo(a_pool);
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline PackedBVH<T, P, K>
PackedBVH<T, P, K>::rebasedView(const Pool& a_pool) const noexcept
{
  const uint64_t endByte = Math::max(m_nodes.endByte(), m_primitives.endByte());

  PackedBVH view = *this;

  view.m_location = m_location.rebasedOnto(a_pool, endByte, "BVH::PackedBVH");

  // A snapshot is what a kernel receives. The device traversal stack is smaller than the host's, so a
  // tree the host accepted can still be too deep for a device. This is the moment the caller commits
  // to that, and the last one that still runs on the host where it can say so -- unless the arrays
  // were adopted in device-only memory, which the host cannot read.
  if (view.m_location.m_control == nullptr && m_location.m_hostAccessible) {
    this->requireDepthFits(this->base(), DeviceTraversalDepth, "device view");
  }

  return view;
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline PoolLocation
PackedBVH<T, P, K>::location() const noexcept
{
  return m_location;
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline PackedBVH<T, P, K>
PackedBVH<T, P, K>::relocatedTo(const PoolLocation& a_location) const noexcept
{
  PackedBVH<T, P, K> view = *this;

  view.m_location = a_location;

  return view;
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline PackedBVH<T, P, K>
PackedBVH<T, P, K>::deepCopy(Pool& a_dstPool) const
{
  EBGEOMETRY_REQUIRE(m_location.m_hostAccessible,
                     "BVH::PackedBVH::deepCopy: the BVH's arrays are in memory the host cannot read");

  // Copy both arrays out before reserving anything: if a_dstPool is also the source pool, a reserve
  // can move its block and invalidate the spans.
  const auto nodeSpan = this->getNodes();
  const auto primSpan = this->getPrimitives();

  const std::vector<Node> nodes(nodeSpan.begin(), nodeSpan.end());
  const std::vector<P>    prims(primSpan.begin(), primSpan.end());

  PackedBVH copy = *this;

  copy.m_location   = PoolLocation{};
  copy.m_nodes      = PODVector<Node>{};
  copy.m_primitives = PODVector<P>{};

  copy.finalize(a_dstPool, nodes, prims);

  return copy;
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline void
PackedBVH<T, P, K>::finalize(Pool& a_pool, const std::vector<Node>& a_nodes, const std::vector<P>& a_primitives)
{
  this->attachTo(a_pool);

  m_nodes.reserveFrom(a_pool, static_cast<uint32_t>(a_nodes.size()));
  m_primitives.reserveFrom(a_pool, static_cast<uint32_t>(a_primitives.size()));

  // Resolve the base only after every reservation above: a reserve can grow the pool, which moves
  // the block and invalidates any address taken before it.
  void* poolBase = const_cast<void*>(this->base());

  m_nodes.assign(poolBase, a_nodes.data(), static_cast<uint32_t>(a_nodes.size()));
  m_primitives.assign(poolBase, a_primitives.data(), static_cast<uint32_t>(a_primitives.size()));

  // Reject here, once, rather than letting pruneTraverse walk off its fixed stack later with no way
  // to notice.
  this->requireDepthFits(poolBase, HostTraversalDepth, "host build");
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
  return m_nodes.bind(this->base());
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline typename PackedBVH<T, P, K>::BV
PackedBVH<T, P, K>::getBoundingVolume() const noexcept
{
  if (m_nodes.size() == 0) {
    return BV();
  }

  return m_nodes.at(this->base(), 0).getBoundingVolume();
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline typename PackedBVH<T, P, K>::BV
PackedBVH<T, P, K>::computeBoundingVolume() const noexcept
{
  return this->getBoundingVolume();
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline void
PackedBVH<T, P, K>::requireDepthFits(const void* a_base, const size_t a_depth, const char* a_context) const
{
  const size_t depth = treeDepth(m_nodes.data(a_base), m_nodes.size());

  EBGEOMETRY_REQUIRE(depth <= a_depth,
                     "BVH::PackedBVH: %s -- the tree is %zu levels deep, more than the %zu levels a traversal "
                     "stack holds.\n"
                     "  pruneTraverse would overflow its fixed stack, which Release builds do not detect. The "
                     "library's builders never make such a tree; check an adopted node array.",
                     a_context,
                     depth,
                     a_depth);
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline void
PackedBVH<T, P, K>::requireWellFormed(const Node* a_nodes, const size_t a_numNodes, const size_t a_numPrimitives)
{
  // Report the first defect and stop: once one index is wrong, later ones say nothing reliable.
  if (a_numNodes == 0) {
    EBGEOMETRY_REQUIRE(a_numPrimitives == 0,
                       "BVH::PackedBVH: adopted node array is malformed -- it is empty, but the primitive array "
                       "holds %zu primitives",
                       a_numPrimitives);

    return;
  }

  EBGEOMETRY_REQUIRE(a_numNodes <= size_t(Node::EmptySlot) / K,
                     "BVH::PackedBVH: adopted node array is malformed -- %zu nodes is more than a traversal can "
                     "index",
                     a_numNodes);

  std::vector<bool> hasParent(a_numNodes, false);

  for (size_t i = 0; i < a_numNodes; i++) {
    const Node& node = a_nodes[i];

    bool seenEmpty = false;

    for (size_t k = 0; k < K; k++) {
      if (node.isEmpty(k)) {
        seenEmpty = true;

        continue;
      }

      EBGEOMETRY_REQUIRE(!seenEmpty,
                         "BVH::PackedBVH: adopted node array is malformed -- node %zu has an occupied slot %zu after "
                         "an empty one",
                         i,
                         k);

      if (node.isLeaf(k)) {
        const size_t end = size_t(node.m_child[k]) + size_t(node.m_count[k]);

        EBGEOMETRY_REQUIRE(end <= a_numPrimitives,
                           "BVH::PackedBVH: adopted node array is malformed -- node %zu slot %zu's primitive range "
                           "ends at %zu, past the primitive array's %zu primitives",
                           i,
                           k,
                           end,
                           a_numPrimitives);
      }
      else {
        const size_t child = node.m_child[k];

        EBGEOMETRY_REQUIRE(child > i && child < a_numNodes && !hasParent[child],
                           "BVH::PackedBVH: adopted node array is malformed -- node %zu slot %zu names child %zu, "
                           "which is not after its parent, not inside the array of %zu nodes, or has another parent",
                           i,
                           k,
                           child,
                           a_numNodes);

        hasParent[child] = true;
      }
    }

    EBGEOMETRY_REQUIRE(
      !node.isEmpty(0), "BVH::PackedBVH: adopted node array is malformed -- node %zu has no occupied slot", i);
  }

  for (size_t i = 1; i < a_numNodes; i++) {
    EBGEOMETRY_REQUIRE(
      hasParent[i], "BVH::PackedBVH: adopted node array is malformed -- node %zu is not reachable from the root", i);
  }
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline float
PackedBVH<T, P, K>::lowerBound(const T a_dist2) noexcept
{
  if constexpr (std::is_same_v<T, float>) {
    return a_dist2;
  }
  else {
    // A double narrows to the nearest float, which may be above it. Shrinking the value by 2^-22
    // first keeps the rounded float below the original, since rounding moves it by at most 2^-24
    // relative -- except for results that would be float subnormals, which go to zero, and values
    // past the float range, which go to the largest float.
    if (!(a_dist2 >= T(Math::Limits<float>::min()))) {
      return 0.0F;
    }

    if (a_dist2 >= T(Math::Limits<float>::max())) {
      return Math::Limits<float>::max();
    }

    return static_cast<float>(a_dist2 * (T(1) - T(1.0 / 4194304.0)));
  }
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline void
PackedBVH<T, P, K>::computeChildDistances2(const Node& a_node, const Vec3T<T>& a_point, T (&a_dist2)[K]) noexcept
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
  // Alignment: the WideNode box rows are alignas(sizeof(T)*K), which is 64 bytes for both (K=8, T=double) and
  // (K=16, T=float) -- exactly what _mm512_load_pd / _mm512_load_ps require. The static_asserts
  // catch any mismatch. The *store* into a_dist2 is unaligned (storeu), so callers need not align
  // their output buffer; on an aligned address storeu costs the same as store on every CPU that has
  // AVX-512 at all.
  //
  // These serve the host-tuned branching factors (BVH::HostBranchingRatio): K=16 for float and K=8
  // for double, one register covering all slots. Whether they beat the default K=4 depends on the
  // workload; see the benchmarks in the Sphinx page on configuration options.
  // ──────────────────────────────────────────────────────────────────────────────
#if defined(__AVX512F__)
  if constexpr (K == 8 && std::is_same_v<T, double>) {
    static_assert(Node::RowAlignment == sizeof(T) * K,
                  "WideNode row alignment mismatch: _mm512_load_pd requires 64-byte alignment");

    const __m512d px   = _mm512_set1_pd(a_point[0]);
    const __m512d py   = _mm512_set1_pd(a_point[1]);
    const __m512d pz   = _mm512_set1_pd(a_point[2]);
    const __m512d zero = _mm512_setzero_pd();

    const __m512d lo_x = _mm512_load_pd(a_node.m_lo[0]);
    const __m512d lo_y = _mm512_load_pd(a_node.m_lo[1]);
    const __m512d lo_z = _mm512_load_pd(a_node.m_lo[2]);
    const __m512d hi_x = _mm512_load_pd(a_node.m_hi[0]);
    const __m512d hi_y = _mm512_load_pd(a_node.m_hi[1]);
    const __m512d hi_z = _mm512_load_pd(a_node.m_hi[2]);

    const __m512d dx = _mm512_max_pd(zero, _mm512_max_pd(_mm512_sub_pd(lo_x, px), _mm512_sub_pd(px, hi_x)));
    const __m512d dy = _mm512_max_pd(zero, _mm512_max_pd(_mm512_sub_pd(lo_y, py), _mm512_sub_pd(py, hi_y)));
    const __m512d dz = _mm512_max_pd(zero, _mm512_max_pd(_mm512_sub_pd(lo_z, pz), _mm512_sub_pd(pz, hi_z)));
    const __m512d d2 =
      _mm512_add_pd(_mm512_mul_pd(dx, dx), _mm512_add_pd(_mm512_mul_pd(dy, dy), _mm512_mul_pd(dz, dz)));

    _mm512_storeu_pd(a_dist2, d2);

    return;
  }

  if constexpr (K == 16 && std::is_same_v<T, float>) {
    static_assert(Node::RowAlignment == sizeof(T) * K,
                  "WideNode row alignment mismatch: _mm512_load_ps requires 64-byte alignment");

    const __m512 px   = _mm512_set1_ps((float)a_point[0]);
    const __m512 py   = _mm512_set1_ps((float)a_point[1]);
    const __m512 pz   = _mm512_set1_ps((float)a_point[2]);
    const __m512 zero = _mm512_setzero_ps();

    const __m512 lo_x = _mm512_load_ps(a_node.m_lo[0]);
    const __m512 lo_y = _mm512_load_ps(a_node.m_lo[1]);
    const __m512 lo_z = _mm512_load_ps(a_node.m_lo[2]);
    const __m512 hi_x = _mm512_load_ps(a_node.m_hi[0]);
    const __m512 hi_y = _mm512_load_ps(a_node.m_hi[1]);
    const __m512 hi_z = _mm512_load_ps(a_node.m_hi[2]);

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
    static_assert(Node::RowAlignment == sizeof(T) * K,
                  "WideNode row alignment mismatch: _mm256_load_pd requires 32-byte alignment");

    const __m256d px   = _mm256_set1_pd(a_point[0]);
    const __m256d py   = _mm256_set1_pd(a_point[1]);
    const __m256d pz   = _mm256_set1_pd(a_point[2]);
    const __m256d zero = _mm256_setzero_pd();

    const __m256d lo_x = _mm256_load_pd(a_node.m_lo[0]);
    const __m256d lo_y = _mm256_load_pd(a_node.m_lo[1]);
    const __m256d lo_z = _mm256_load_pd(a_node.m_lo[2]);
    const __m256d hi_x = _mm256_load_pd(a_node.m_hi[0]);
    const __m256d hi_y = _mm256_load_pd(a_node.m_hi[1]);
    const __m256d hi_z = _mm256_load_pd(a_node.m_hi[2]);

    const __m256d dx = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_x, px), _mm256_sub_pd(px, hi_x)));
    const __m256d dy = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_y, py), _mm256_sub_pd(py, hi_y)));
    const __m256d dz = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_z, pz), _mm256_sub_pd(pz, hi_z)));
    const __m256d d2 =
      _mm256_add_pd(_mm256_mul_pd(dx, dx), _mm256_add_pd(_mm256_mul_pd(dy, dy), _mm256_mul_pd(dz, dz)));

    _mm256_storeu_pd(a_dist2, d2);

    return;
  }

  if constexpr (K == 8 && std::is_same_v<T, float>) {
    static_assert(Node::RowAlignment == sizeof(T) * K,
                  "WideNode row alignment mismatch: _mm256_load_ps requires 32-byte alignment");

    const __m256 px   = _mm256_set1_ps((float)a_point[0]);
    const __m256 py   = _mm256_set1_ps((float)a_point[1]);
    const __m256 pz   = _mm256_set1_ps((float)a_point[2]);
    const __m256 zero = _mm256_setzero_ps();

    const __m256 lo_x = _mm256_load_ps(a_node.m_lo[0]);
    const __m256 lo_y = _mm256_load_ps(a_node.m_lo[1]);
    const __m256 lo_z = _mm256_load_ps(a_node.m_lo[2]);
    const __m256 hi_x = _mm256_load_ps(a_node.m_hi[0]);
    const __m256 hi_y = _mm256_load_ps(a_node.m_hi[1]);
    const __m256 hi_z = _mm256_load_ps(a_node.m_hi[2]);

    const __m256 dx = _mm256_max_ps(zero, _mm256_max_ps(_mm256_sub_ps(lo_x, px), _mm256_sub_ps(px, hi_x)));
    const __m256 dy = _mm256_max_ps(zero, _mm256_max_ps(_mm256_sub_ps(lo_y, py), _mm256_sub_ps(py, hi_y)));
    const __m256 dz = _mm256_max_ps(zero, _mm256_max_ps(_mm256_sub_ps(lo_z, pz), _mm256_sub_ps(pz, hi_z)));
    const __m256 d2 = _mm256_add_ps(_mm256_mul_ps(dx, dx), _mm256_add_ps(_mm256_mul_ps(dy, dy), _mm256_mul_ps(dz, dz)));

    _mm256_storeu_ps(a_dist2, d2);

    return;
  }

  if constexpr (K == 8 && std::is_same_v<T, double>) {
    static_assert(Node::RowAlignment == sizeof(T) * K,
                  "WideNode row alignment mismatch: _mm256_load_pd requires 32-byte alignment");

    const __m256d px   = _mm256_set1_pd(a_point[0]);
    const __m256d py   = _mm256_set1_pd(a_point[1]);
    const __m256d pz   = _mm256_set1_pd(a_point[2]);
    const __m256d zero = _mm256_setzero_pd();

    const __m256d lo_x0 = _mm256_load_pd(a_node.m_lo[0]);
    const __m256d lo_y0 = _mm256_load_pd(a_node.m_lo[1]);
    const __m256d lo_z0 = _mm256_load_pd(a_node.m_lo[2]);
    const __m256d hi_x0 = _mm256_load_pd(a_node.m_hi[0]);
    const __m256d hi_y0 = _mm256_load_pd(a_node.m_hi[1]);
    const __m256d hi_z0 = _mm256_load_pd(a_node.m_hi[2]);

    const __m256d dx0 = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_x0, px), _mm256_sub_pd(px, hi_x0)));
    const __m256d dy0 = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_y0, py), _mm256_sub_pd(py, hi_y0)));
    const __m256d dz0 = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_z0, pz), _mm256_sub_pd(pz, hi_z0)));
    const __m256d d2_0 =
      _mm256_add_pd(_mm256_mul_pd(dx0, dx0), _mm256_add_pd(_mm256_mul_pd(dy0, dy0), _mm256_mul_pd(dz0, dz0)));

    const __m256d lo_x1 = _mm256_load_pd(a_node.m_lo[0] + 4);
    const __m256d lo_y1 = _mm256_load_pd(a_node.m_lo[1] + 4);
    const __m256d lo_z1 = _mm256_load_pd(a_node.m_lo[2] + 4);
    const __m256d hi_x1 = _mm256_load_pd(a_node.m_hi[0] + 4);
    const __m256d hi_y1 = _mm256_load_pd(a_node.m_hi[1] + 4);
    const __m256d hi_z1 = _mm256_load_pd(a_node.m_hi[2] + 4);

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
    static_assert(Node::RowAlignment == sizeof(T) * K,
                  "WideNode row alignment mismatch: _mm_load_ps requires 16-byte alignment");

    const __m128 px   = _mm_set1_ps((float)a_point[0]);
    const __m128 py   = _mm_set1_ps((float)a_point[1]);
    const __m128 pz   = _mm_set1_ps((float)a_point[2]);
    const __m128 zero = _mm_setzero_ps();

    const __m128 lo_x = _mm_load_ps(a_node.m_lo[0]);
    const __m128 lo_y = _mm_load_ps(a_node.m_lo[1]);
    const __m128 lo_z = _mm_load_ps(a_node.m_lo[2]);
    const __m128 hi_x = _mm_load_ps(a_node.m_hi[0]);
    const __m128 hi_y = _mm_load_ps(a_node.m_hi[1]);
    const __m128 hi_z = _mm_load_ps(a_node.m_hi[2]);

    const __m128 dx = _mm_max_ps(zero, _mm_max_ps(_mm_sub_ps(lo_x, px), _mm_sub_ps(px, hi_x)));
    const __m128 dy = _mm_max_ps(zero, _mm_max_ps(_mm_sub_ps(lo_y, py), _mm_sub_ps(py, hi_y)));
    const __m128 dz = _mm_max_ps(zero, _mm_max_ps(_mm_sub_ps(lo_z, pz), _mm_sub_ps(pz, hi_z)));
    const __m128 d2 = _mm_add_ps(_mm_mul_ps(dx, dx), _mm_add_ps(_mm_mul_ps(dy, dy), _mm_mul_ps(dz, dz)));

    _mm_storeu_ps(a_dist2, d2);

    return;
  }
#endif // __SSE4_1__

#endif // !EBGEOMETRY_DEVICE_COMPILE

  // Scalar path: every (T, K) with no compiled ISA path above, and all device code. WideNode's own
  // per-slot distance uses the same arithmetic in the same order as the SIMD paths, so every path
  // produces bit-identical results.
  for (size_t k = 0; k < K; k++) {
    a_dist2[k] = a_node.getDistance2(k, a_point);
  }
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
  // An empty BVH has no root to descend from.
  if (m_nodes.size() == 0) {
    return;
  }

  // One resolution for the whole traversal. Safe because a query performs no reservation, so the
  // base cannot move underneath it.
  const void* poolBase = this->base();

  constexpr size_t stackDepth = PackedBVH::traversalStackDepth();

  StackEntry stack[stackDepth];

  size_t top = 0;

  // The node to expand next: its occupied slots within the pruning bound are pushed, farthest first,
  // so that the nearest is popped next.
  uint32_t nodeToExpand = 0;
  bool     expand       = true;

  while (expand) {
    const Node& node = m_nodes.at(poolBase, nodeToExpand);

    T dist2[K];

    PackedBVH::computeChildDistances2(node, a_point, dist2);

    // Nothing between here and the pushes below can change a_state, so one read of the bound serves
    // the whole node.
    const T pruneDist2 = a_pruneDist2(a_state);

    // The slots within the bound, sorted into descending distance. Insertion sort: K is a handful of
    // elements, std::sort is not callable from device code, and a stable sort keeps equidistant
    // slots in slot order, so the traversal is deterministic.
    uint32_t slots[K];
    size_t   numSlots = 0;

    for (size_t k = 0; k < K; k++) {
      if (node.isEmpty(k)) {
        break;
      }

      if (dist2[k] <= pruneDist2) {
        size_t j = numSlots++;

        while (j > 0 && dist2[slots[j - 1]] < dist2[k]) {
          slots[j] = slots[j - 1];
          j--;
        }

        slots[j] = static_cast<uint32_t>(k);
      }
    }

    for (size_t i = 0; i < numSlots; i++) {
      EBGEOMETRY_EXPECT(top < stackDepth);

      stack[top++] =
        StackEntry{nodeToExpand * static_cast<uint32_t>(K) + slots[i], PackedBVH::lowerBound(dist2[slots[i]])};
    }

    expand = false;

    while (top > 0) {
      const StackEntry entry = stack[--top];

      // A leaf visited since this entry was pushed -- in any subtree -- may have tightened the bound.
      // The stored distance never exceeds the true one, so this never skips a slot that could matter.
      if (T(entry.m_dist2) > a_pruneDist2(a_state)) {
        continue;
      }

      const Node&    parent = m_nodes.at(poolBase, entry.m_slot / static_cast<uint32_t>(K));
      const uint32_t slot   = entry.m_slot % static_cast<uint32_t>(K);

      if (parent.isLeaf(slot)) {
        a_evalLeaf(a_state, size_t(parent.m_child[slot]), size_t(parent.m_count[slot]));
      }
      else {
        nodeToExpand = parent.m_child[slot];
        expand       = true;

        break;
      }
    }
  }
}

template <class T, class P, size_t K>
template <class BVConstructor>
EBGEOMETRY_HOST
inline void
PackedBVH<T, P, K>::refit(const BVConstructor& a_bvConstructor)
{
  // Every child follows its parent in the node array, so a reverse sweep refits all of a node's
  // children before the node itself. refit() reserves nothing, so one resolution covers the sweep.
  void* poolBase = const_cast<void*>(this->base());

  for (uint32_t i = m_nodes.size(); i-- > 0;) {
    Node& node = m_nodes.at(poolBase, i);

    for (size_t k = 0; k < K; k++) {
      if (node.isLeaf(k)) {
        BV bv;

        for (uint32_t p = node.m_child[k]; p < node.m_child[k] + node.m_count[k]; p++) {
          bv = bv.merged(a_bvConstructor(m_primitives.at(poolBase, p)));
        }

        node.setBoundingVolume(k, bv);
      }
      else if (node.isInterior(k)) {
        node.setBoundingVolume(k, m_nodes.at(poolBase, node.m_child[k]).getBoundingVolume());
      }
    }
  }
}

} // namespace BVH

} // namespace EBGeometry

#endif
