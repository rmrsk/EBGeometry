// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file    EBGeometry_PointCloudHashGrid.hpp
 * @brief   A uniform spatial grid over a point cloud: fast build and high-level nearest-neighbor /
 *          closest-point queries, interchangeable with PointCloudBVH.
 * @author  Robert Marskar
 */

#ifndef EBGEOMETRY_POINTCLOUDHASHGRID_HPP
#define EBGEOMETRY_POINTCLOUDHASHGRID_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_PODVector.hpp"
#include "EBGeometry_PointCloud.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief A uniform spatial grid specialized for point clouds, with a fast build and turnkey queries.
 * @details PointCloudHashGrid answers the same nearest-neighbor / closest-point queries as
 * PointCloudBVH, with the same PointCloud::Hit result and the same query methods and accessors, so
 * the two are interchangeable -- but it circumvents the tree entirely. Points are counting-sorted
 * into a dense grid of cells (a CSR bucket array keyed by integer cell coordinates), which is an O(N)
 * build with no recursive partitioning and no tree nodes.
 *
 * Queries use an **expanding-shell** search: starting from the query point's cell, cells are visited
 * shell by shell (Chebyshev radius 0, 1, 2, ...). The search stops as soon as the k-th best distance
 * found is provably closer than any unvisited cell can hold -- an exact bound, so no neighbor is ever
 * missed. With the default cell size (~1 point/cell) this is usually one or two shells.
 *
 * @note This is the bounded-domain form of spatial hashing: cells are stored in a dense array sized
 * to the cloud's bounding box, which is O(N) memory for a compact cloud. A cloud that is sparse but
 * spread over a very large box would want true spatial hashing (a hash map keyed by cell id) instead.
 *
 * @note **Density matters.** The cell size is global, so PointCloudHashGrid is fastest on near-uniform
 * clouds. On strongly clustered / multi-scale clouds a single cell size is simultaneously too coarse
 * in dense regions and too fine in sparse ones, and a query in an empty region visits many empty
 * cells; PointCloudBVH adapts to local density and is the better choice there. Like PointCloudBVH,
 * the grid serves only point queries: neither exposes a signed distance or bounding volume, so
 * neither can be composed as a primitive inside an outer BVH/CSG.
 *
 * @note Device portability. Every array (positions and the two CSR arrays) is reserved from one
 * Pool, and the class stores only offsets into it, so a PointCloudHashGrid is trivially copyable.
 * Mirror the pool, call rebasedView(), and pass the returned value into a kernel; every query that
 * answers through a caller-supplied buffer (closestPoint(s), nearestNeighbor(s)) is device-callable.
 * allNearestNeighbors() returns a std::vector and stays host-only.
 *
 * @tparam T Floating-point precision.
 */
template <class T>
class PointCloudHashGrid
{
public:
  /**
   * @brief One query result: the cloud index of a matched point and its squared distance.
   * @details Shared with PointCloudBVH. A miss, and every result slot a multi-result query could not
   * fill, has Hit::valid() false; those queries also return the count found.
   */
  using Hit = PointCloud::Hit<T>;

  /**
   * @brief No default construction -- a point cloud is required.
   */
  PointCloudHashGrid() = delete;

  /**
   * @brief Build a uniform grid over a point cloud.
   * @details The cell size is derived from @p a_targetPerCell: cells are sized so that, at the cloud's
   * average density, each holds about that many points. ~1 point/cell is the query-time sweet spot;
   * larger values shrink the grid (less memory) at the cost of bigger per-cell scans. A point's cloud
   * index, which the queries report, is its position in @p a_positions.
   * @param[in,out] a_pool          Pool the grid's arrays are reserved from; must outlive this object.
   * @param[in]     a_positions     Point positions. Fewer than PointCloud::InvalidIndex, all finite.
   * @param[in]     a_targetPerCell Target average points per cell (sets the cell size). Must be > 0.
   */
  EBGEOMETRY_HOST
  inline PointCloudHashGrid(Pool& a_pool, const std::vector<Vec3T<T>>& a_positions, T a_targetPerCell = T(1));

  /**
   * @brief Closest cloud point to an arbitrary query point.
   * @param[in] a_query Query point (need not be in the cloud).
   * @return The nearest point and its squared distance.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Hit
  closestPoint(const Vec3T<T>& a_query) const noexcept;

  /**
   * @brief The a_k closest cloud points to an arbitrary query point, nearest first.
   * @param[in]  a_query Query point (need not be in the cloud).
   * @param[in]  a_k     Number of neighbors requested.
   * @param[out] a_out   Buffer of at least a_k Hits; filled [0, returned count), ascending by distance.
   * @return The number of neighbors found (min(a_k, cloud size)).
   */
  EBGEOMETRY_HOST_DEVICE
  inline std::size_t
  closestPoints(const Vec3T<T>& a_query, std::size_t a_k, Hit* a_out) const noexcept;

  /**
   * @brief Nearest *other* point to a point already in the cloud.
   * @param[in] a_point Cloud index of the query point; excluded from its own result.
   * @return The nearest other point and its squared distance.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Hit
  nearestNeighbor(uint32_t a_point) const noexcept;

  /**
   * @brief The a_k nearest *other* points to a point already in the cloud, nearest first.
   * @param[in]  a_point Cloud index of the query point; excluded from its own result.
   * @param[in]  a_k     Number of neighbors requested.
   * @param[out] a_out   Buffer of at least a_k Hits; filled [0, returned count), ascending.
   * @return The number of neighbors found (min(a_k, cloud size - 1)).
   */
  EBGEOMETRY_HOST_DEVICE
  inline std::size_t
  nearestNeighbors(uint32_t a_point, std::size_t a_k, Hit* a_out) const noexcept;

  /**
   * @brief For every point, its a_k nearest *other* points (the k-nearest-neighbor graph).
   * @details Processes points in cell (spatial) order so consecutive queries touch nearby cells and
   * stay hot in cache. Result is flattened row-major: entry [i*a_k + j] is the j-th nearest neighbor
   * of point i (ascending by distance).
   * @param[in] a_k Number of neighbors per point.
   * @return A vector of size numPoints()*a_k of Hits.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline std::vector<Hit>
  allNearestNeighbors(std::size_t a_k = 1) const;

  /**
   * @brief Number of points in the cloud.
   * @return The point count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline std::size_t
  numPoints() const noexcept
  {
    return m_positions.size();
  }

  /**
   * @brief Position of the point with the given cloud index.
   * @param[in] a_index Cloud index.
   * @return The point's position.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const Vec3T<T>&
  position(uint32_t a_index) const noexcept
  {
    EBGEOMETRY_EXPECT(a_index < m_positions.size());

    return m_positions.at(this->base(), a_index);
  }

  /**
   * @brief Resolve the base address every array of this object is an offset from.
   * @return Base address of the pool block holding this object.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const void*
  base() const noexcept
  {
    return m_location.base();
  }

  /**
   * @brief Produce a copy of this object that resolves against @p a_pool.
   * @details Same contract as PackedBVH::rebasedView(): mirror the pool first, then rebase, then
   * copy the returned value into a kernel.
   * @param[in] a_pool Pool to rebase onto: the object's own pool or one in its mirror chain.
   * @return A copy of this object resolving against @p a_pool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline PointCloudHashGrid
  rebasedView(const Pool& a_pool) const noexcept;

  /**
   * @brief Duplicate this object's storage into @p a_dstPool.
   * @details The copy constructor copies descriptors only, leaving both objects resolving against
   * the same pool memory. This is the operation that gives genuinely independent storage.
   * @param[in,out] a_dstPool Pool to reserve the copy's arrays from.
   * @return A PointCloudHashGrid with the same cells and cloud, backed by @p a_dstPool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline PointCloudHashGrid
  deepCopy(Pool& a_dstPool) const;

  /**
   * @brief Check whether this object's arrays were reserved from @p a_pool.
   * @param[in] a_pool Pool to test against.
   * @return True if this object is attached to @p a_pool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline bool
  isAttachedTo(const Pool& a_pool) const noexcept
  {
    return m_location.isAttachedTo(a_pool);
  }

private:
  /**
   * @brief Reserve the grid's arrays from @p a_pool and copy the given host arrays into them.
   * @details Reserves first and resolves the base only afterwards, since a reserve can grow the pool
   * and move the block. The sources must therefore be host arrays outside @p a_pool.
   * @param[in,out] a_pool       Pool to reserve from.
   * @param[in]     a_positions  Point positions (indexed by cloud index).
   * @param[in]     a_cellStart  CSR offsets, one per cell plus one.
   * @param[in]     a_cellPoints Cloud indices sorted by cell.
   */
  EBGEOMETRY_HOST
  inline void
  storeArrays(Pool&                             a_pool,
              const std::vector<Vec3T<T>>&      a_positions,
              const std::vector<std::uint32_t>& a_cellStart,
              const std::vector<std::uint32_t>& a_cellPoints);

  /**
   * @brief Integer cell coordinate of a coordinate value along one axis, clamped into [0, a_n).
   * @param[in] a_x  Coordinate value (one component of a point).
   * @param[in] a_lo Grid lower corner on this axis.
   * @param[in] a_n  Number of cells on this axis.
   * @return The clamped integer cell coordinate.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline int
  cellCoord(T a_x, T a_lo, int a_n) const noexcept;

  /**
   * @brief Linear cell id from clamped integer cell coordinates.
   * @param[in] a_ix Cell coordinate along x.
   * @param[in] a_iy Cell coordinate along y.
   * @param[in] a_iz Cell coordinate along z.
   * @return The linear cell index.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline std::size_t
  cellIndex(int a_ix, int a_iy, int a_iz) const noexcept;

  /**
   * @brief Linear cell id of the cell containing a point.
   * @param[in] a_p Point.
   * @return The linear cell index of the point's (clamped) cell.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline std::size_t
  cellIndexOf(const Vec3T<T>& a_p) const noexcept;

  /**
   * @brief The shared query core: fill the a_k nearest to a_query into a_out via expanding-shell search.
   * @param[in]  a_query   Query point.
   * @param[in]  a_k       Neighbors requested.
   * @param[out] a_out     Buffer of at least a_k Hits (kept sorted ascending).
   * @param[in]  a_exclude Cloud index to exclude (self-queries); PointCloud::InvalidIndex to exclude
   *                       nothing.
   * @return The number of neighbors found.
   */
  EBGEOMETRY_HOST_DEVICE
  inline std::size_t
  query(const Vec3T<T>& a_query, std::size_t a_k, Hit* a_out, uint32_t a_exclude) const noexcept;

  /**
   * @brief Pool the arrays below are offsets into.
   */
  PoolLocation m_location;

  /**
   * @brief Point positions, indexed by cloud index.
   */
  PODVector<Vec3T<T>> m_positions;

  /**
   * @brief CSR offsets, size nCells + 1.
   */
  PODVector<std::uint32_t> m_cellStart;

  /**
   * @brief Cloud indices sorted by cell, size numPoints().
   */
  PODVector<std::uint32_t> m_cellPoints;

  /**
   * @brief Lower corner of the grid (cloud bounding-box minimum).
   */
  Vec3T<T> m_lo;

  /**
   * @brief Cell size (edge length).
   */
  T m_h = T(1);

  /**
   * @brief 1 / m_h, cached.
   */
  T m_invH = T(1);

  /**
   * @brief Grid dimensions in cells (along x, y, z).
   */
  int m_nx = 1, m_ny = 1, m_nz = 1;
};

/**
 * @brief A PointCloudHashGrid must be trivially copyable, for the same reason a PackedBVH must: a
 * rebasedView() is byte-copied into a device address space with no pointer patching.
 */
static_assert(std::is_trivially_copyable_v<PointCloudHashGrid<float>>,
              "PointCloudHashGrid<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<PointCloudHashGrid<double>>,
              "PointCloudHashGrid<double> must be trivially copyable");

} // namespace EBGeometry

#include "EBGeometry_PointCloudHashGridImplem.hpp"

#endif
