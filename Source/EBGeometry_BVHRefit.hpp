// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_BVHRefit.hpp
 * @brief  Refitting a PackedBVH one tree level at a time, one thread per node: on a device in a CUDA
 * or HIP build, in a host loop otherwise.
 * @details PackedBVH::refit() is one sequential sweep. BVH::refitParallel() refits the same boxes a
 * level at a time, deepest first, through GPU::parallelFor(); within a level every node is independent
 * (PackedBVH::refitNode()). The order it needs, the nodes grouped by depth, is computed once on the
 * host by BVH::RefitSchedule, since it depends only on the tree's shape.
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
 * @brief A PackedBVH's nodes grouped by depth, deepest level first: the order refitParallel() refits
 * them in.
 * @details Built once on the host from the tree's shape, which a refit never changes. The node list is
 * reserved from the BVH's own pool, so a mirror of that pool carries it along and one schedule serves
 * the BVH and every rebased view of it; the level boundaries stay on the host, which launches one
 * refit per level. Build it before the pool is frozen.
 */
class RefitSchedule
{
public:
  /**
   * @brief Group the nodes of @p a_bvh by depth.
   * @tparam T Floating-point precision.
   * @tparam P Primitive type.
   * @tparam K Branching factor.
   * @param[in,out] a_pool The pool @p a_bvh lives in, which the node list is reserved from; it must
   * not be frozen.
   * @param[in]     a_bvh  The BVH; host-accessible.
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

  /**
   * @brief The nodes of level @p a_level, counted from the deepest: entries [levelBegin(a_level),
   * levelBegin(a_level + 1)) of nodes().
   * @param[in] a_level Level, at most numLevels().
   * @return The first entry of the level.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline uint32_t
  levelBegin(const size_t a_level) const noexcept
  {
    return m_levelBegin[a_level];
  }

  /**
   * @brief The node indices, deepest level first, in the BVH's pool.
   * @return The node list.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline const PODVector<uint32_t>&
  nodes() const noexcept
  {
    return m_nodes;
  }

private:
  /**
   * @brief Node indices, deepest level first.
   */
  PODVector<uint32_t> m_nodes;

  /**
   * @brief Where each level starts in m_nodes, with the total count at the end.
   */
  std::vector<uint32_t> m_levelBegin;
};

/**
 * @brief The functor refitParallel() runs on every node of one level.
 * @details Internal; not part of the public API.
 * @tparam Bvh           PackedBVH type.
 * @tparam BVConstructor Primitive bounding-volume constructor.
 */
template <class Bvh, class BVConstructor>
struct RefitLevel
{
  /**
   * @brief The BVH, a handle into the pool the refit writes.
   */
  mutable Bvh m_bvh;

  /**
   * @brief The schedule's node list.
   */
  PODVector<uint32_t> m_nodes;

  /**
   * @brief First entry of the level.
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
  void
  operator()(const std::size_t a_i) const noexcept
  {
    const uint32_t node = m_nodes.at(m_bvh.location().base(), m_begin + static_cast<uint32_t>(a_i));

    m_bvh.refitNode(node, m_bvConstructor);
  }
};

/**
 * @brief Refit @p a_bvh one level at a time, deepest first, one thread per node.
 * @details The same boxes as PackedBVH::refit(): the per-node work is PackedBVH::refitNode() in both.
 * Each level is one GPU::parallelFor(), so in a CUDA or HIP translation unit the levels run as device
 * kernels on @p a_bvh, which must then be a rebased view of a device-accessible mirror; elsewhere they
 * run as host loops. @p a_bvConstructor runs where the refit runs, so for a device it must be
 * EBGEOMETRY_HOST_DEVICE and trivially copyable, and read only device-accessible memory.
 * @tparam T             Floating-point precision.
 * @tparam P             Primitive type.
 * @tparam K             Branching factor.
 * @tparam BVConstructor Callable: (const P&) -> AABBT<T>.
 * @param[in,out] a_bvh       The BVH, or a rebased view of a mirror of its pool; its boxes are
 * rewritten.
 * @param[in] a_schedule      The schedule built for the BVH (it travels with the pool).
 * @param[in] a_bvConstructor Bounding-volume constructor for a single primitive.
 */
template <class T, class P, size_t K, class BVConstructor>
EBGEOMETRY_HOST
inline void
refitParallel(PackedBVH<T, P, K>& a_bvh, const RefitSchedule& a_schedule, const BVConstructor& a_bvConstructor);

} // namespace BVH

} // namespace EBGeometry

#include "EBGeometry_BVHRefitImplem.hpp"

#endif
