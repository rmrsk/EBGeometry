// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_BVHQueryImplem.hpp
 * @brief  Implementation of EBGeometry_BVHQuery.hpp.
 * @details DRAFT (bvh_traversal, #230): placeholder bodies, so the interface can be reviewed. Each says
 * what the real one will do.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_BVHQUERYIMPLEM_HPP
#define EBGEOMETRY_BVHQUERYIMPLEM_HPP

// Our includes
#include "EBGeometry_BVHQuery.hpp"

namespace EBGeometry {

namespace BVH {

template <class T>
template <size_t K>
EBGEOMETRY_HOST_DEVICE
inline Array<T, K>
PointQuery<T>::keys(const ChildBoxes<T, K>& /*a_row*/) const noexcept
{
  // TODO: PackedBVH::computeChildDistances2(), moved here unchanged (SIMD paths by (T, K) and ISA,
  // scalar on a device), so pruneTraverse()'s callers see bit-for-bit the same distances.
  Array<T, K> keys;

  for (size_t k = 0; k < K; k++) {
    keys[k] = Math::Limits<T>::infinity();
  }

  return keys;
}

template <class T>
template <size_t K>
EBGEOMETRY_HOST_DEVICE
inline T
PointQuery<T>::key(const ChildBoxes<T, K>& /*a_row*/, const size_t /*a_k*/) const noexcept
{
  // TODO: PackedBVH::childDistance2(), moved here.
  return Math::Limits<T>::infinity();
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline T
PointQuery<T>::key(const BoundingVolumes::AABBT<T>& /*a_box*/) const noexcept
{
  // TODO: the squared distance from m_point to the box. (pruneTraverse() takes the root's as 0; the
  // real distance prunes a point far from the whole tree at once.)
  return T(0);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline RayQuery<T>::RayQuery(const Vec3T<T>& a_origin, const Vec3T<T>& a_direction, const T a_tMin) noexcept
  : m_origin(a_origin),
    m_inverseDirection(T(1) / a_direction[0], T(1) / a_direction[1], T(1) / a_direction[2]),
    m_tMin(a_tMin)
{
  // TODO: EBGEOMETRY_EXPECT a finite origin and a unit direction.
}

template <class T>
template <size_t K>
EBGEOMETRY_HOST_DEVICE
inline Array<T, K>
RayQuery<T>::keys(const ChildBoxes<T, K>& /*a_row*/) const noexcept
{
  // TODO: per axis, t0 = (lo - o) * invD and t1 = (hi - o) * invD across the K children (one SIMD
  // register per axis on the host); entry = max(tMin, min(t0, t1) over the axes), exit = min(max(t0, t1)
  // over the axes); key = entry if entry <= exit, else +infinity. Min/max ordered so a NaN (0 * inf)
  // loses; boxes padded by a few ulps of their extent.
  Array<T, K> keys;

  for (size_t k = 0; k < K; k++) {
    keys[k] = Math::Limits<T>::infinity();
  }

  return keys;
}

template <class T>
template <size_t K>
EBGEOMETRY_HOST_DEVICE
inline T
RayQuery<T>::key(const ChildBoxes<T, K>& /*a_row*/, const size_t /*a_k*/) const noexcept
{
  // TODO: the slab test for child a_k, bit for bit the value keys() gives it.
  return Math::Limits<T>::infinity();
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline T
RayQuery<T>::key(const BoundingVolumes::AABBT<T>& /*a_box*/) const noexcept
{
  // TODO: the slab test for one box.
  return Math::Limits<T>::infinity();
}

} // namespace BVH

} // namespace EBGeometry

#endif
