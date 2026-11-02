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
#include "EBGeometry_BoundingVolumes.hpp"
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
 * the two are interchangeable -- but it circumvents the tree entirely. Points are sorted into a dense
 * grid of cells (a CSR bucket array keyed by integer cell coordinates), with no recursive partitioning
 * and no tree nodes: on the host by a counting sort, an O(N) build; in device-accessible memory by a
 * bitonic sort of (cell, cloud index) keys through GPU::parallelFor(), which needs no atomics, so the
 * grid does not depend on thread scheduling and is the host's, array for array. The grid keeps its own
 * copy of the positions in cell order, so a cell's points are read from consecutive memory, and a map
 * from each cloud index to its slot.
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
 * @note Device portability. Every array (the positions, the two CSR arrays, the slot map, and the
 * scratch of a build in device-accessible memory) is reserved from one Pool, and the class stores only
 * offsets into it, so a PointCloudHashGrid is trivially copyable. Build into device memory, or mirror a
 * host pool; call rebasedView() and pass the returned value into a kernel; every query that answers
 * through a caller-supplied buffer (closestPoint(s), nearestNeighbor(s)) is device-callable.
 * allNearestNeighbors() returns a std::vector and stays host-only.
 *
 * @tparam T Floating-point precision.
 */
template <class T>
class PointCloudHashGrid : public PoolHandle
{
public:
  /**
   * @brief One query result: the cloud index of a matched point and its squared distance.
   * @details Shared with PointCloudBVH. A miss, and every result slot a multi-result query could not
   * fill, has Hit::valid() false; those queries also return the count found.
   */
  using Hit = PointCloud::Hit<T>;

  /**
   * @brief Build a uniform grid over a point cloud: build(a_pool, a_positions, a_targetPerCell).
   * @param[in,out] a_pool          Pool the grid's arrays are reserved from; must outlive this object.
   * @param[in]     a_positions     Point positions. Fewer than PointCloud::InvalidIndex, all finite.
   * @param[in]     a_targetPerCell Target average points per cell (sets the cell size). Must be > 0.
   */
  EBGEOMETRY_HOST
  inline PointCloudHashGrid(Pool& a_pool, const std::vector<Vec3T<T>>& a_positions, T a_targetPerCell = T(1));

  /**
   * @brief Build a uniform grid over a point cloud, where the pool's memory is.
   * @details The cell size is derived from @p a_targetPerCell: cells are sized so that, at the cloud's
   * average density, each holds about that many points. ~1 point/cell is the query-time sweet spot;
   * larger values shrink the grid (less memory) at the cost of bigger per-cell scans. A point's cloud
   * index, which the queries report, is its index in @p a_source. The grid stores its own copy of the
   * positions, in cell order.
   *
   * The build runs where the pool's memory is. In device-accessible memory (device or managed memory in a
   * CUDA or HIP build; emulated device memory in a host build) it runs through GPU::parallelFor(): kernels in a
   * CUDA or HIP translation unit, which read @p a_source on the device, and host loops otherwise. In host
   * memory it runs on the host, which reads @p a_source there. Both give the same grid, array for array. The host
   * reads each point from the source three times (for the box, and twice for the sort), so an expensive source pays
   * that. A point with a non-finite coordinate is rejected: on the host by its index, through GPU::parallelFor()
   * only as "a point", since the launches report no more than that the box is not finite.
   * Freeze the pool and take rebasedView() to query a grid in device memory in a kernel.
   *
   * @p a_maxPoints reserves room for rebuild() with up to that many points: the arrays, cells for that many
   * points (the most the cell-size rule gives), and the build's scratch, so that rebuild() reserves nothing
   * and works on a frozen pool.
   * @tparam Source A source of positions: a trivially copyable type whose const call operator takes a cloud
   * index (uint32_t) and returns that point's Vec3T<T>; see PointCloud::FromArray.
   * @param[in,out] a_pool          Pool the grid's arrays are reserved from; must outlive the grid.
   * @param[in]     a_source    The positions.
   * @param[in]     a_numPoints     Number of points. At most 2^31, all finite.
   * @param[in]     a_targetPerCell Target average points per cell (sets the cell size). Must be > 0.
   * @param[in]     a_maxPoints     Most points a rebuild() may have; 0 (or fewer than a_numPoints) leaves room
   *                                for a_numPoints, in the cells this build needs.
   * @return The grid.
   */
  template <class Source>
  [[nodiscard]] EBGEOMETRY_HOST
  static inline PointCloudHashGrid
  build(Pool& a_pool, const Source& a_source, uint32_t a_numPoints, T a_targetPerCell = T(1), uint32_t a_maxPoints = 0);

  /**
   * @brief Build a uniform grid over points on the host, where the pool's memory is; see the build() above.
   * @details For a pool in device memory, the positions are first copied to scratch there.
   * @param[in,out] a_pool          Pool the grid's arrays are reserved from; must outlive the grid.
   * @param[in]     a_positions     Point positions. At most 2^31, all finite.
   * @param[in]     a_targetPerCell Target average points per cell (sets the cell size). Must be > 0.
   * @param[in]     a_maxPoints     Most points a rebuild() may have (see the build() above).
   * @return The grid.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static inline PointCloudHashGrid
  build(Pool& a_pool, const std::vector<Vec3T<T>>& a_positions, T a_targetPerCell = T(1), uint32_t a_maxPoints = 0);

  /**
   * @brief Build the grid again, in the arrays build() reserved, from new positions.
   * @details The same grid as a new build() from these positions, with nothing reserved: it runs where the build
   * did, and works on a frozen pool. More points than maxPoints() is rejected. If the cloud's box would need more
   * cells than were reserved, the cells are made larger until it does not; queries stay exact, only slower. A grid
   * built with room for more points (build()'s a_maxPoints) has room for the most cells its points can need; one
   * built without has the cells of its first build only. A point with a non-finite coordinate is rejected, as in
   * build(). A view taken with rebasedView() before
   * the rebuild still describes the old grid (its cell size and counts): take a new one, which copies nothing. Call
   * it on the grid build() returned, not on a view.
   * @tparam Source A source of positions, as for build().
   * @param[in] a_source    The positions.
   * @param[in] a_numPoints     Number of points, at most maxPoints(). All finite.
   * @param[in] a_targetPerCell Target average points per cell. Must be > 0.
   */
  template <class Source>
  EBGEOMETRY_HOST
  inline void
  rebuild(const Source& a_source, uint32_t a_numPoints, T a_targetPerCell = T(1));

  /**
   * @brief The most points rebuild() can take.
   * @return The reserved point count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  maxPoints() const noexcept
  {
    return m_positions.m_capacity;
  }

  /**
   * @brief A build through GPU::parallelFor(): the sort key of every point, (cell << 32) | cloud index, and
   * padding keys that sort last. Defined in EBGeometry_PointCloudHashGridBuildImplem.hpp.
   * @details Internal; not part of the public API. The three functors are public only because nvcc
   * rejects a private nested type as the template argument of a kernel; as nested types they can still
   * use the grid's private cell arithmetic.
   * @tparam Source The positions' source.
   */
  template <class Source>
  struct CellKeys;

  /**
   * @brief A build through GPU::parallelFor(): where each cell's points start in the sorted keys.
   */
  struct CellStarts;

  /**
   * @brief A build through GPU::parallelFor(): each slot's cloud index and position, in cell order, and each
   * point's slot.
   * @tparam Source The positions' source.
   */
  template <class Source>
  struct CellGather;

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

    const void* base = this->base();

    return m_positions.at(base, m_slotOf.at(base, a_index));
  }

  /**
   * @brief The base address this object's arrays are offsets from; see PoolHandle::base().
   * @return The base address: const void* for a const object, void* otherwise.
   */
  using PoolHandle::base;

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
   * the same pool memory. This is the operation that gives genuinely independent storage. On the host: both
   * pools' memory must be host-accessible. The copy has room for its own points only, so rebuild() it within that,
   * and it builds on the host.
   * @param[in,out] a_dstPool Pool to reserve the copy's arrays from.
   * @return A PointCloudHashGrid with the same cells and cloud, backed by @p a_dstPool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline PointCloudHashGrid
  deepCopy(Pool& a_dstPool) const;

private:
  /**
   * @brief An empty grid, to be reserved and filled.
   */
  PointCloudHashGrid() noexcept = default;

  /**
   * @brief Reserve the grid's arrays from @p a_pool.
   * @param[in,out] a_pool      Pool to reserve from (not frozen).
   * @param[in]     a_maxPoints Points to make room for.
   * @param[in]     a_maxCells  Cells to make room for.
   */
  EBGEOMETRY_HOST
  inline void
  reserveArrays(Pool& a_pool, uint32_t a_maxPoints, uint32_t a_maxCells);

  /**
   * @brief The box of a cloud, where the build runs.
   * @tparam Source The positions' source.
   * @param[in] a_source The positions.
   * @param[in] a_numPoints  Number of points.
   * @param[in] a_who        The calling function, for the message when a point is not finite.
   * @return The box; zeros for no points.
   */
  template <class Source>
  EBGEOMETRY_HOST
  inline BoundingVolumes::AABBT<T>
  boxOf(const Source& a_source, uint32_t a_numPoints, const char* a_who);

  /**
   * @brief Fill the reserved arrays for a cloud, the grid shaped for it: the counting sort on the host, or the
   * sort of cell keys through GPU::parallelFor().
   * @tparam Source The positions' source.
   * @param[in] a_source The positions.
   * @param[in] a_numPoints  Number of points.
   */
  template <class Source>
  EBGEOMETRY_HOST
  inline void
  fill(const Source& a_source, uint32_t a_numPoints);

  /**
   * @brief Set the grid's corner, cell size and dimensions for a cloud with box [a_lo, a_hi].
   * @details The one cell-size rule of both builds: cells sized so that each holds about
   * @p a_targetPerCell points at the cloud's average density, coarsened until the cell count stays
   * within a budget proportional to the point count.
   * @param[in] a_lo            Lower corner of the cloud's box.
   * @param[in] a_hi            Upper corner of the cloud's box.
   * @param[in] a_numPoints     Number of points.
   * @param[in] a_targetPerCell Target average points per cell (> 0).
   * @param[in] a_cellCap       Most cells, if fewer than the rule's own budget (the cells a rebuild has room for).
   */
  EBGEOMETRY_HOST
  inline void
  setShape(const Vec3T<T>& a_lo,
           const Vec3T<T>& a_hi,
           std::size_t     a_numPoints,
           T               a_targetPerCell,
           std::size_t     a_cellCap = Math::Limits<std::size_t>::max()) noexcept;

  /**
   * @brief Number of cells.
   * @return m_nx m_ny m_nz.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline std::size_t
  numCells() const noexcept
  {
    return std::size_t(m_nx) * std::size_t(m_ny) * std::size_t(m_nz);
  }

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
   * @brief Point positions in cell order: slot s holds point m_cellPoints[s].
   */
  PODVector<Vec3T<T>> m_positions;

  /**
   * @brief CSR offsets into the slots, size nCells + 1.
   */
  PODVector<std::uint32_t> m_cellStart;

  /**
   * @brief Each slot's cloud index, size numPoints().
   */
  PODVector<std::uint32_t> m_cellPoints;

  /**
   * @brief Each point's slot, by cloud index, size numPoints().
   */
  PODVector<std::uint32_t> m_slotOf;

  /**
   * @brief A build through GPU::parallelFor(): the cell keys, a power of two of them; empty for a grid in host memory.
   */
  PODVector<std::uint64_t> m_keys;

  /**
   * @brief A build through GPU::parallelFor(): the box reduction's scratch; empty for a grid in host memory.
   */
  PODVector<Vec3T<T>> m_boxScratch;

  /**
   * @brief Whether the grid builds through GPU::parallelFor() (its pool's memory is device-accessible).
   */
  bool m_parallel = false;

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

#include "EBGeometry_PointCloudHashGridBuildImplem.hpp"
#include "EBGeometry_PointCloudHashGridImplem.hpp"

#endif
