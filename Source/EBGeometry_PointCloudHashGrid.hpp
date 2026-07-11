// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file    EBGeometry_PointCloudHashGrid.hpp
 * @brief   A uniform spatial grid over a point cloud: fast build and high-level nearest-neighbor /
 *          closest-point queries, drop-in with PointCloudBVH.
 * @author  Robert Marskar
 */

#ifndef EBGeometry_PointCloudHashGrid
#define EBGeometry_PointCloudHashGrid

// Std includes
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

// Our includes
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief A uniform spatial grid specialized for point clouds, with a fast build and turnkey queries.
 * @details PointCloudHashGrid answers the same nearest-neighbor / closest-point queries as
 * PointCloudBVH and exposes the **same public interface** (same Hit type, same query methods and
 * accessors), so the two are drop-in interchangeable -- but it circumvents the tree entirely. Points
 * are counting-sorted into a dense grid of cells (a CSR bucket array keyed by integer cell
 * coordinates), which is an O(N) build with no recursive partitioning and no tree nodes.
 *
 * Queries use an **expanding-shell** search: starting from the query point's cell, cells are visited
 * shell by shell (Chebyshev radius 0, 1, 2, ...). The search stops as soon as the k-th best distance
 * found is provably closer than any unvisited cell can hold -- an exact bound, so no neighbor is ever
 * missed. With the default cell size (~1 point/cell) this is almost always one or two shells.
 *
 * @note This is the bounded-domain form of spatial hashing: cells are stored in a dense array sized
 * to the cloud's bounding box, which is O(N) memory for a compact cloud. A cloud that is sparse but
 * spread over a very large box would want true spatial hashing (a hash map keyed by cell id) instead.
 *
 * @note **Density matters.** The cell size is global, so PointCloudHashGrid is fastest on near-uniform
 * clouds. On strongly clustered / multi-scale clouds a single cell size is simultaneously too coarse
 * in dense regions and too fine in sparse ones; PointCloudBVH adapts to local density and is the
 * better choice there. The grid also serves only point queries -- unlike PointCloudBVH it is not a
 * BVH and cannot be composed as a primitive inside an outer BVH/CSG.
 *
 * @tparam T    Floating-point precision.
 * @tparam Meta User metadata type stored per point and returned via metadata(). Defaults to the
 *              cloud index itself (std::size_t).
 */
template <class T, class Meta = std::size_t>
class PointCloudHashGrid
{
public:
  /**
   * @brief One query result: the cloud index of a matched point and its squared distance.
   * @details Identical in shape to PointCloudBVH::Hit. @c index is the point's position in the input
   * @c positions / @c metadata arrays; @c distanceSquared avoids a sqrt on the hot path.
   * @note A "no match" result (an empty cloud, or a self-query on a cloud with no other point) is
   * signalled by @c distanceSquared == std::numeric_limits<T>::max(); test that rather than @c index,
   * since the default @c index of 0 is indistinguishable from a genuine match on point 0. The
   * multi-result queries instead report the count found via their return value.
   */
  struct Hit
  {
    std::size_t index           = 0;                             ///< Cloud index of the matched point.
    T           distanceSquared = std::numeric_limits<T>::max(); ///< Squared distance from the query to it.
  };

  /**
   * @brief No default construction -- a point cloud is required.
   */
  PointCloudHashGrid() = delete;

  /**
   * @brief Build a uniform grid over a point cloud.
   * @details The cell size is derived from @p a_targetPerCell: cells are sized so that, at the cloud's
   * average density, each holds about that many points. ~1 point/cell is the query-time sweet spot;
   * larger values shrink the grid (less memory) at the cost of bigger per-cell scans.
   * @param[in] a_positions     Point positions.
   * @param[in] a_metadata      Per-point user metadata (same length/order as a_positions).
   * @param[in] a_targetPerCell Target average points per cell (sets the cell size). Must be > 0.
   */
  inline PointCloudHashGrid(const std::vector<Vec3T<T>>& a_positions,
                            const std::vector<Meta>&     a_metadata,
                            T                            a_targetPerCell = T(1));

  /**
   * @brief Closest cloud point to an arbitrary query point.
   * @param[in] a_query Query point (need not be in the cloud).
   * @return The nearest point and its squared distance.
   */
  [[nodiscard]] inline Hit
  closestPoint(const Vec3T<T>& a_query) const noexcept;

  /**
   * @brief The a_k closest cloud points to an arbitrary query point, nearest first.
   * @param[in]  a_query Query point (need not be in the cloud).
   * @param[in]  a_k     Number of neighbors requested.
   * @param[out] a_out   Buffer of at least a_k Hits; filled [0, returned count), ascending by distance.
   * @return The number of neighbors found (min(a_k, cloud size)).
   */
  inline std::size_t
  closestPoints(const Vec3T<T>& a_query, std::size_t a_k, Hit* a_out) const noexcept;

  /**
   * @brief Nearest *other* point to a point already in the cloud.
   * @param[in] a_point Cloud index of the query point; excluded from its own result.
   * @return The nearest other point and its squared distance.
   */
  [[nodiscard]] inline Hit
  nearestNeighbor(std::size_t a_point) const noexcept;

  /**
   * @brief The a_k nearest *other* points to a point already in the cloud, nearest first.
   * @param[in]  a_point Cloud index of the query point; excluded from its own result.
   * @param[in]  a_k     Number of neighbors requested.
   * @param[out] a_out   Buffer of at least a_k Hits; filled [0, returned count), ascending.
   * @return The number of neighbors found (min(a_k, cloud size - 1)).
   */
  inline std::size_t
  nearestNeighbors(std::size_t a_point, std::size_t a_k, Hit* a_out) const noexcept;

  /**
   * @brief For every point, its a_k nearest *other* points (the k-nearest-neighbor graph).
   * @details Processes points in cell (spatial) order so consecutive queries touch nearby cells and
   * stay hot in cache. Result is flattened row-major: entry [i*a_k + j] is the j-th nearest neighbor
   * of point i (ascending by distance).
   * @param[in] a_k Number of neighbors per point.
   * @return A vector of size numPoints()*a_k of Hits.
   */
  [[nodiscard]] inline std::vector<Hit>
  allNearestNeighbors(std::size_t a_k = 1) const;

  /**
   * @brief Brute-force closest point to an arbitrary query point (O(N) reference for closestPoint()).
   * @param[in] a_query Query point (need not be in the cloud).
   * @return The nearest point and its squared distance.
   * @warning For debugging and testing only -- an O(N) reference implementation, never a hot path.
   */
  [[nodiscard]] inline Hit
  closestPointBruteForce(const Vec3T<T>& a_query) const noexcept;

  /**
   * @brief Brute-force a_k closest points to an arbitrary query point (O(N) reference for
   * closestPoints()).
   * @param[in]  a_query Query point (need not be in the cloud).
   * @param[in]  a_k     Number of neighbors requested.
   * @param[out] a_out   Buffer of at least a_k Hits; filled [0, returned count), ascending by distance.
   * @return The number of neighbors found (min(a_k, cloud size)).
   * @warning For debugging and testing only -- an O(N*k) reference implementation, never a hot path.
   */
  inline std::size_t
  closestPointsBruteForce(const Vec3T<T>& a_query, std::size_t a_k, Hit* a_out) const;

  /**
   * @brief Brute-force nearest *other* point to a cloud point (O(N) reference for nearestNeighbor()).
   * @param[in] a_point Cloud index of the query point; excluded from its own result.
   * @return The nearest other point and its squared distance.
   * @warning For debugging and testing only -- an O(N) reference implementation, never a hot path.
   */
  [[nodiscard]] inline Hit
  nearestNeighborBruteForce(std::size_t a_point) const noexcept;

  /**
   * @brief Brute-force a_k nearest *other* points to a cloud point (O(N) reference for
   * nearestNeighbors()).
   * @param[in]  a_point Cloud index of the query point; excluded from its own result.
   * @param[in]  a_k     Number of neighbors requested.
   * @param[out] a_out   Buffer of at least a_k Hits; filled [0, returned count), ascending.
   * @return The number of neighbors found (min(a_k, cloud size - 1)).
   * @warning For debugging and testing only -- an O(N*k) reference implementation, never a hot path.
   */
  inline std::size_t
  nearestNeighborsBruteForce(std::size_t a_point, std::size_t a_k, Hit* a_out) const;

  /**
   * @brief Number of points in the cloud.
   * @return The point count.
   */
  [[nodiscard]] inline std::size_t
  numPoints() const noexcept
  {
    return m_positions.size();
  }

  /**
   * @brief Position of the point with the given cloud index.
   * @param[in] a_index Cloud index.
   * @return The point's position.
   */
  [[nodiscard]] inline const Vec3T<T>&
  position(std::size_t a_index) const noexcept
  {
    return m_positions[a_index];
  }

  /**
   * @brief User metadata of the point with the given cloud index.
   * @param[in] a_index Cloud index.
   * @return The point's user metadata.
   */
  [[nodiscard]] inline const Meta&
  metadata(std::size_t a_index) const noexcept
  {
    return m_metadata[a_index];
  }

private:
  /**
   * @brief Integer cell coordinate of a coordinate value along one axis, clamped into [0, a_n).
   * @param[in] a_x  Coordinate value (one component of a point).
   * @param[in] a_lo Grid lower corner on this axis.
   * @param[in] a_n  Number of cells on this axis.
   * @return The clamped integer cell coordinate.
   */
  [[nodiscard]] inline int
  cellCoord(T a_x, T a_lo, int a_n) const noexcept;

  /**
   * @brief Linear cell id from clamped integer cell coordinates.
   * @param[in] a_ix Cell coordinate along x.
   * @param[in] a_iy Cell coordinate along y.
   * @param[in] a_iz Cell coordinate along z.
   * @return The linear cell index.
   */
  [[nodiscard]] inline std::size_t
  cellIndex(int a_ix, int a_iy, int a_iz) const noexcept;

  /**
   * @brief Linear cell id of the cell containing a point.
   * @param[in] a_p Point.
   * @return The linear cell index of the point's (clamped) cell.
   */
  [[nodiscard]] inline std::size_t
  cellIndexOf(const Vec3T<T>& a_p) const noexcept;

  /**
   * @brief The shared query core: fill the a_k nearest to a_query into a_out via expanding-shell search.
   * @param[in]  a_query   Query point.
   * @param[in]  a_k       Neighbors requested.
   * @param[out] a_out     Buffer of at least a_k Hits (kept sorted ascending).
   * @param[out] a_found   Number of neighbors found.
   * @param[in]  a_exclude Cloud index to exclude (self-queries); s_none to exclude nothing.
   */
  inline void
  query(
    const Vec3T<T>& a_query, std::size_t a_k, Hit* a_out, std::size_t& a_found, std::size_t a_exclude) const noexcept;

  /**
   * @brief Brute-force single nearest by full scan (shared by the k==1 brute-force queries).
   * @param[in] a_query   Query point.
   * @param[in] a_exclude Cloud index to exclude, or s_none to exclude nothing.
   * @return The nearest (non-excluded) point and its squared distance; a default Hit if none.
   */
  [[nodiscard]] inline Hit
  bruteForceOne(const Vec3T<T>& a_query, std::size_t a_exclude) const noexcept;

  /**
   * @brief Brute-force a_k nearest by full scan (shared by the k-nearest brute-force queries).
   * @param[in]  a_query   Query point.
   * @param[in]  a_k       Neighbors requested.
   * @param[out] a_out     Buffer of at least a_k Hits; filled ascending by distance.
   * @param[in]  a_exclude Cloud index to exclude, or s_none to exclude nothing.
   * @return The number of neighbors found.
   */
  inline std::size_t
  bruteForceK(const Vec3T<T>& a_query, std::size_t a_k, Hit* a_out, std::size_t a_exclude) const;

  /**
   * @brief Sentinel meaning "exclude no point".
   */
  static constexpr std::size_t s_none = std::numeric_limits<std::size_t>::max();

  std::vector<Vec3T<T>> m_positions; ///< Point positions, indexed by cloud index.
  std::vector<Meta>     m_metadata;  ///< User metadata, indexed by cloud index.

  Vec3T<T> m_lo;                         ///< Lower corner of the grid (cloud bounding-box minimum).
  T        m_h    = T(1);                ///< Cell size (edge length).
  T        m_invH = T(1);                ///< 1 / m_h, cached.
  int      m_nx = 1, m_ny = 1, m_nz = 1; ///< Grid dimensions in cells.

  std::vector<std::uint32_t> m_cellStart;  ///< CSR offsets, size nCells + 1.
  std::vector<std::uint32_t> m_cellPoints; ///< Cloud indices sorted by cell, size numPoints().
};

} // namespace EBGeometry

#include "EBGeometry_PointCloudHashGridImplem.hpp"

#endif
