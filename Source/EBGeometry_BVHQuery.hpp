// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_BVHQuery.hpp
 * @brief  What PackedBVH::traverse() tracks through a tree: query types, the child-box rows they test,
 * and the order the children are visited in.
 * @details A query type decides, for the K child boxes of a node, how far each one is from the query: its
 * key. PackedBVH::traverse() enters a child whose key is within the caller's bound, in the order a visit
 * order policy gives. The library ships PointQuery (the squared distance from a point to a box) and
 * RayQuery (where a ray enters a box); a user can write their own query type with the same three
 * functions (see PointQuery for the interface).
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_BVHQUERY_HPP
#define EBGEOMETRY_BVHQUERY_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

// Our includes
#include "EBGeometry_Array.hpp"
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

namespace BVH {

/**
 * @brief The boxes of the K children of one interior node, in SoA layout: m_lo[axis][child] and
 * m_hi[axis][child], so that one axis of all K children loads as one SIMD register.
 * @details What a query type tests when PackedBVH::traverse() expands a node. Read-only for users: a
 * PackedBVH builds and refits its rows.
 * @tparam T Floating-point precision.
 * @tparam K Branching factor.
 */
template <class T, size_t K>
struct ChildBoxes
{
  /**
   * @brief Alignment of one axis row, in bytes.
   * @details A row is sizeof(T) * K bytes wide, and every SIMD path loads a whole row with one aligned
   * load, so the row wants to be aligned to its own width. That width is only a legal alignment when it
   * is a power of two, which it is for exactly the (T, K) combinations that have a SIMD path (16, 32 and
   * 64 bytes). For every other K (3, 5, 6, 7, ...) no SIMD path is compiled, nothing performs an aligned
   * vector load on the row, and T's natural alignment is both sufficient and legal. Asking for
   * sizeof(T) * K unconditionally would make PackedBVH<double, P, 3> and its odd-K siblings fail to
   * compile ("requested alignment 24 is not a positive power of 2").
   */
  static constexpr size_t RowAlignment =
    (((sizeof(T) * K) & ((sizeof(T) * K) - 1)) == 0) ? (sizeof(T) * K) : alignof(T);

  /**
   * @brief Lower corners: m_lo[axis][child], axis in {0, 1, 2}, child in {0, ..., K - 1}.
   */
  alignas(RowAlignment) T m_lo[3][K];

  /**
   * @brief Upper corners: m_hi[axis][child], axis in {0, 1, 2}, child in {0, ..., K - 1}.
   */
  alignas(RowAlignment) T m_hi[3][K];
};

/**
 * @brief Visit the children within the bound nearest first (smallest key first). The default: a
 * nearest-primitive search or a closest-hit ray tightens its bound fastest this way.
 */
struct NearestFirst
{
};

/**
 * @brief Visit the children within the bound farthest first (largest key first).
 */
struct FarthestFirst
{
};

/**
 * @brief Visit the children within the bound in the order they are stored (first slot first), without
 * sorting: for an any-hit query or a search that collects every leaf it reaches, where order does not
 * matter.
 */
struct AsStored
{
};

/**
 * @brief The default visit callback of PackedBVH::traverse(): enter every node that passes the bound.
 * @details The walk recognises it and calls nothing.
 */
struct DescendAll
{
  /**
   * @brief Always descend.
   * @tparam Args The visit callback's argument types.
   * @param[in] a_args The visit callback's arguments, ignored.
   * @return true.
   */
  template <class... Args>
  EBGEOMETRY_HOST_DEVICE
  constexpr bool
  operator()(const Args&... a_args) const noexcept
  {
    ((void)a_args, ...);

    return true;
  }
};

/**
 * @brief The key type of a query type: Q::Key if it declares one, else Q::Scalar.
 * @details A key needs <= and > against the bound (of the same type), and < for NearestFirst and
 * FarthestFirst.
 * @tparam Q Query type.
 */
template <class Q, class = void>
struct QueryKey
{
  /**
   * @brief The key type.
   */
  using type = typename Q::Scalar;
};

/**
 * @brief A query type that declares its key type.
 * @tparam Q Query type.
 */
template <class Q>
struct QueryKey<Q, std::void_t<typename Q::Key>>
{
  /**
   * @brief The key type.
   */
  using type = typename Q::Key;
};

/**
 * @brief How many queries a query type carries. 1 unless the type says otherwise.
 * @details PackedBVH::traverse() supports one query per walk (Lanes = 1). The trait keeps room for
 * query types that carry several queries walked together (packets), which can be added later without
 * changing the single-query interface: a query type declares nothing and is one lane.
 * @tparam Q Query type.
 */
template <class Q, class = void>
struct QueryLanes
{
  /**
   * @brief Number of queries.
   */
  static constexpr uint32_t value = 1;
};

/**
 * @brief A query type that declares `static constexpr uint32_t Lanes`.
 * @tparam Q Query type.
 */
template <class Q>
struct QueryLanes<Q, std::void_t<decltype(Q::Lanes)>>
{
  /**
   * @brief Number of queries.
   */
  static constexpr uint32_t value = Q::Lanes;
};

/**
 * @brief Whether a type is a query type: it names its precision (Scalar) and keys a node's children.
 * @details What PackedBVH::traverse() requires of its first argument, and checks, for a clear compile
 * error when something else is passed.
 * @tparam Q Candidate type.
 */
template <class Q, class = void>
struct IsQuery : std::false_type
{
};

/**
 * @brief A type with Scalar and a root key(box) is a query type.
 * @tparam Q Candidate type.
 */
template <class Q>
struct IsQuery<Q,
               std::void_t<typename Q::Scalar,
                           decltype(std::declval<const Q&>().key(
                             std::declval<const BoundingVolumes::AABBT<typename Q::Scalar>&>()))>> : std::true_type
{
};

/**
 * @brief A point tracked through a BVH: the key of a box is the squared distance from the point to it
 * (0 inside it).
 * @details The query of nearest-primitive searches: with a squared-distance bound and NearestFirst, the walk visits
 * exactly the boxes nearer than the nearest primitive found so far.
 *
 * The interface every query type has (the walk calls nothing else):
 * - keys(row): the keys of all K children of a node at once (the SIMD kernel); +infinity for a child the
 *   query cannot reach, which the walk never enters whatever the bound;
 * - key(row, k): one child's key again, bit for bit what keys() gave, for when the walk returns to a
 *   level;
 * - key(box): the root's key (the root has no parent row).
 * @tparam T Floating-point precision.
 */
template <class T>
struct PointQuery
{
  /**
   * @brief Floating-point precision.
   */
  using Scalar = T;

  /**
   * @brief Key type: a squared distance.
   */
  using Key = T;

  /**
   * @brief The point.
   */
  Vec3T<T> m_point;

  /**
   * @brief The squared distances from the point to the K child boxes of a node.
   * @details SSE4.1, AVX and AVX-512 paths on the host, a scalar loop on a device (the kernel that is
   * QueryDetail::pointDistances2()).
   * @tparam K Branching factor.
   * @param[in] a_row The node's child boxes.
   * @return One squared distance per child.
   */
  template <size_t K>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Array<T, K>
  keys(const ChildBoxes<T, K>& a_row) const noexcept;

  /**
   * @brief The squared distance from the point to one child box, bit for bit what keys() gives it.
   * @tparam K Branching factor.
   * @param[in] a_row The node's child boxes.
   * @param[in] a_k   Child slot, below K.
   * @return The squared distance.
   */
  template <size_t K>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  key(const ChildBoxes<T, K>& a_row, size_t a_k) const noexcept;

  /**
   * @brief The squared distance from the point to a box (the root's).
   * @param[in] a_box Box.
   * @return The squared distance.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  key(const BoundingVolumes::AABBT<T>& a_box) const noexcept;
};

/**
 * @brief Deduce PointQuery's precision from its point: BVH::PointQuery{p}.
 * @tparam T Floating-point precision.
 */
template <class T>
PointQuery(Vec3T<T>) -> PointQuery<T>;

/**
 * @brief A ray tracked through a BVH: the key of a box is the distance along the ray at which it enters
 * the box, or +infinity if it misses it.
 * @details The ray is origin + t * direction for t >= tMin, with a unit direction so that t is a
 * distance. The caller's bound is the far end: the nearest crossing found so far for a closest-hit query,
 * a segment's length for a segment, or -infinity to end the walk (any-hit after the first crossing).
 * The slab test is robust to a zero direction component (its inverse is +-infinity) and drops the NaN of
 * 0 * infinity; boxes are padded by a few ulps so a ray through a face or edge shared by two boxes enters
 * one of them. This type holds no RayT or HitT: those belong to the ray queries built on it.
 * @tparam T Floating-point precision.
 */
template <class T>
struct RayQuery
{
  /**
   * @brief Floating-point precision.
   */
  using Scalar = T;

  /**
   * @brief Key type: a distance along the ray.
   */
  using Key = T;

  /**
   * @brief Ray origin.
   */
  Vec3T<T> m_origin;

  /**
   * @brief 1 / direction, per component (+-infinity for a zero component).
   */
  Vec3T<T> m_inverseDirection;

  /**
   * @brief Start of the ray: crossings before it are ignored.
   */
  T m_tMin;

  /**
   * @brief The ray origin + t * direction, t >= tMin.
   * @param[in] a_origin    Origin.
   * @param[in] a_direction Unit direction.
   * @param[in] a_tMin      Start of the ray.
   */
  EBGEOMETRY_HOST_DEVICE
  inline RayQuery(const Vec3T<T>& a_origin, const Vec3T<T>& a_direction, T a_tMin = T(0)) noexcept;

  /**
   * @brief Where the ray enters the K child boxes of a node (+infinity where it misses one).
   * @details One slab test per axis across the K children: SIMD on the host, a scalar loop on a device.
   * @tparam K Branching factor.
   * @param[in] a_row The node's child boxes.
   * @return One entry distance per child.
   */
  template <size_t K>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Array<T, K>
  keys(const ChildBoxes<T, K>& a_row) const noexcept;

  /**
   * @brief Where the ray enters one child box, bit for bit what keys() gives it.
   * @tparam K Branching factor.
   * @param[in] a_row The node's child boxes.
   * @param[in] a_k   Child slot, below K.
   * @return The entry distance, or +infinity.
   */
  template <size_t K>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  key(const ChildBoxes<T, K>& a_row, size_t a_k) const noexcept;

  /**
   * @brief Where the ray enters a box (the root's).
   * @param[in] a_box Box.
   * @return The entry distance, or +infinity.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  key(const BoundingVolumes::AABBT<T>& a_box) const noexcept;
};

} // namespace BVH

} // namespace EBGeometry

#include "EBGeometry_BVHQueryImplem.hpp"

#endif
