// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_BVHRefit.hpp
 * @brief  Refitting a PackedBVH where its pool is: one tree level at a time on a device, the host's
 * sweep otherwise.
 * @details PackedBVH::refit() is one sequential sweep, which on a device would run on a single thread.
 * BVH::refit() refits the same boxes a level at a time, deepest first, one thread per node, through
 * GPU::parallelFor(): within a level every node is independent. The order it needs, the nodes grouped
 * by depth, is computed once on the host by BVH::RefitSchedule, since it depends only on the tree's
 * shape, which a refit never changes.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_BVHREFIT_HPP
#define EBGEOMETRY_BVHREFIT_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_GPUTransform.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_PODVector.hpp"
#include "EBGeometry_PackedBVH.hpp"
#include "EBGeometry_Pool.hpp"

namespace EBGeometry {

namespace BVH {

/**
 * @brief A PackedBVH's nodes grouped by tree depth, deepest level first: the order in which BVH::refit()
 * refits them, one level at a time.
 * @details Opt-in: only a BVH that is refitted with BVH::refit() needs one. Build it on the host after the
 * BVH is built and before its pool is frozen: the node list, 4 bytes per node, is reserved from the BVH's
 * pool, so a mirror of the pool carries it along, and one schedule serves the BVH in its own pool and in
 * every mirror of it. The level boundaries stay in the schedule, on the host, which launches the levels.
 * The tree's shape never changes in a refit, so a schedule is built once; a rebuilt BVH needs a new one.
 *
 * Grouping by depth rather than refitting in one sweep is what lets a level's nodes run in parallel:
 * a node needs its children's boxes, and every child is one level deeper. A schedule-free alternative,
 * repeated passes over all nodes until the boxes settle (one per tree level after the leaves), needs no
 * memory but measured 3 to 4 times slower than the schedule on a device (float and double, 1M and 4M
 * boxes, K = 4, RTX 3080), from re-reading every node in every pass; it was ruled out.
 */
class RefitSchedule
{
public:
  /**
   * @brief Group the nodes of @p a_bvh by depth.
   * @details Aborts, in every build, if @p a_bvh does not live in @p a_pool or the pool is frozen.
   * @tparam T Floating-point precision.
   * @tparam P Primitive type.
   * @tparam K Branching factor.
   * @param[in,out] a_pool The pool @p a_bvh lives in, which the node list is reserved from. It must not
   * be frozen.
   * @param[in]     a_bvh  The BVH, on the host.
   */
  template <class T, class P, size_t K>
  EBGEOMETRY_HOST
  RefitSchedule(Pool& a_pool, const PackedBVH<T, P, K>& a_bvh);

  /**
   * @brief Number of tree levels; 0 for an empty BVH.
   * @return The level count.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline size_t
  numLevels() const noexcept
  {
    return m_levelBegin.empty() ? 0 : m_levelBegin.size() - 1;
  }

private:
  /**
   * @brief BVH::refit() reads the node list and the levels.
   */
  template <class T, class P, size_t K, class BVConstructor>
  friend void
  refit(Pool&                     a_pool,
        const PackedBVH<T, P, K>& a_bvh,
        const RefitSchedule&      a_schedule,
        const BVConstructor&      a_bvConstructor);

  /**
   * @brief Node indices, deepest level first, in the BVH's pool.
   */
  PODVector<uint32_t> m_nodes;

  /**
   * @brief Where each level starts in m_nodes, with the node count at the end; empty for an empty BVH.
   */
  std::vector<uint32_t> m_levelBegin;

  /**
   * @brief Pool::rootId() of the pool the schedule was built in, so that BVH::refit() can tell a pool
   * of another mirror chain.
   */
  uint64_t m_rootId = 0;

  /**
   * @brief Where the BVH's node array starts in the pool. Every array of a pool has its own offset and
   * a mirror keeps them, so with m_rootId it names the one BVH the schedule was built for: another BVH
   * of the same size in the same pool, or the same BVH rebuilt there, has another offset.
   */
  uint64_t m_nodesOffset = 0;
};

/**
 * @brief Refit the copy of @p a_bvh that lives in @p a_pool, after its primitives have moved: every box
 * becomes the union of what it encloses again, with each primitive's current box from
 * @p a_bvConstructor.
 * @details The same boxes as PackedBVH::refit(), and the same use: for primitives that move without
 * changing which primitives the BVH holds or their order (adding, removing or reordering them needs a
 * rebuild), and not a substitute for a rebuild once a deformation is large enough to make the tree loose.
 *
 * Runs where @p a_pool's memory is. On a device-accessible pool, every tree level is one
 * GPU::parallelFor() with a thread per node: device kernels in a CUDA or HIP build, host loops in a host
 * build (over emulated device memory). On a host pool it is PackedBVH::refit()'s sweep. @p a_pool is the
 * BVH's own pool or any mirror of it (a device pool is frozen, as every mirror is). On a device,
 * @p a_bvConstructor runs in the kernels, so it must be trivially copyable and EBGEOMETRY_HOST_DEVICE and
 * read only memory the device can reach; in a CUDA or HIP translation unit it must be
 * EBGEOMETRY_HOST_DEVICE whatever the pool, since the kernels are compiled for it. A constructor that is
 * not trivially copyable is accepted for a host pool only.
 *
 * How the primitives were moved is the caller's business (any kernel, any library, a copy from the host),
 * as long as that work has finished: the levels run on the default stream, and refit() returns once the
 * boxes are done. Work on a stream that does not synchronize with the default stream must be
 * synchronized first.
 *
 * Aborts, in every build, on a schedule built in a pool of another mirror chain or for another BVH (one
 * with a different node count, or another BVH of the pool, or this one rebuilt), and, through
 * PackedBVH::rebasedView(), when @p a_bvh does not fit in @p a_pool.
 * @tparam T             Floating-point precision.
 * @tparam P             Primitive type.
 * @tparam K             Branching factor.
 * @tparam BVConstructor Callable: (const P&) -> AABBT<T>, returning one primitive's current box.
 * @param[in,out] a_pool          The pool whose copy of the BVH is refitted: the BVH's own pool or a mirror.
 * @param[in]     a_bvh           The BVH, as built (a handle to its own pool).
 * @param[in]     a_schedule      The schedule built for @p a_bvh.
 * @param[in]     a_bvConstructor Bounding-volume constructor for a single primitive.
 */
template <class T, class P, size_t K, class BVConstructor>
EBGEOMETRY_HOST
inline void
refit(Pool&                     a_pool,
      const PackedBVH<T, P, K>& a_bvh,
      const RefitSchedule&      a_schedule,
      const BVConstructor&      a_bvConstructor);

namespace RefitDetail {

/**
 * @brief The functor BVH::refit() runs over the nodes of one level, one index per node.
 * @tparam Bvh           PackedBVH type.
 * @tparam BVConstructor Primitive bounding-volume constructor.
 */
template <class Bvh, class BVConstructor>
struct Level
{
  /**
   * @brief The BVH, rebased onto the pool being refitted.
   * @details Mutable because GPU::parallelFor() calls a const operator() and the refit writes the
   * BVH's boxes: through the pool, which the handle's non-const base() reaches.
   */
  mutable Bvh m_bvh;

  /**
   * @brief The schedule's node list, resolving against the same pool.
   */
  PODVector<uint32_t> m_nodes;

  /**
   * @brief First entry of the level in m_nodes.
   */
  uint32_t m_begin;

  /**
   * @brief Primitive bounding-volume constructor.
   */
  BVConstructor m_bvConstructor;

  /**
   * @brief Refit the level's node @p a_i.
   * @param[in] a_i Index within the level.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  operator()(const std::size_t a_i) const
  {
    void* base = m_bvh.base();

    m_bvh.refitNode(base, m_nodes.at(base, m_begin + static_cast<uint32_t>(a_i)), m_bvConstructor);
  }
};

} // namespace RefitDetail

} // namespace BVH

} // namespace EBGeometry

#include "EBGeometry_BVHRefitImplem.hpp"

#endif
