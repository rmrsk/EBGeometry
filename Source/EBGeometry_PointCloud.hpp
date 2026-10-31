// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_PointCloud.hpp
 * @brief  Query results and helpers shared by the point-cloud search structures.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_POINTCLOUD_HPP
#define EBGEOMETRY_POINTCLOUD_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <vector>

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Query results and helpers shared by PointCloudBVH and PointCloudHashGrid.
 * @details Both structures answer the same queries with the same Hit type, so code written against
 * one works with the other. A point is identified by its **cloud index**: its position in the
 * positions array the structure was built from. Per-point user data lives in the caller's own
 * array, indexed by it.
 */
namespace PointCloud {

/**
 * @brief The cloud index of "no point": a miss, or a result slot a query could not fill.
 * @details A cloud holds fewer points than this (the constructors check it), so it never names a
 * real point.
 */
inline constexpr uint32_t InvalidIndex = Math::Limits<uint32_t>::max();

/**
 * @brief One query result: the cloud index of a matched point and its squared distance.
 * @details @c distanceSquared avoids a sqrt on the hot path. A default-constructed Hit is a miss:
 * @c index == InvalidIndex and @c distanceSquared == Math::Limits<T>::max(). Queries return it when
 * there is no point to report (an empty cloud, or a self-query on a cloud with one point), and leave
 * it in the result slots they could not fill.
 * @tparam T Floating-point precision.
 */
template <class T>
struct Hit
{
  uint32_t index           = InvalidIndex;           ///< Cloud index of the matched point.
  T        distanceSquared = Math::Limits<T>::max(); ///< Squared distance from the query to it.

  /**
   * @brief Whether this is a match rather than a miss.
   * @return True if @c index refers to a point.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  bool
  valid() const noexcept
  {
    return index != InvalidIndex;
  }
};

/**
 * @brief The k nearest points seen so far, kept sorted in a caller-supplied buffer.
 * @details The search structures feed every candidate point to insert(), and prune with bound().
 * The buffer holds the found() best candidates in ascending order of distance; ties keep the
 * candidate inserted first. A candidate is inserted at most once per query by every caller, so no
 * de-duplication is done. Callable on the host and on a device, and allocates nothing.
 * @tparam T Floating-point precision.
 */
template <class T>
class KBest
{
public:
  /**
   * @brief Start an empty set.
   * @param[out] a_out     Buffer of at least a_k Hits. Its first found() entries hold the result.
   * @param[in]  a_k       Number of neighbors wanted. Must be > 0.
   * @param[in]  a_exclude Cloud index never to report (a self-query's own point), or InvalidIndex.
   */
  EBGEOMETRY_HOST_DEVICE
  KBest(Hit<T>* a_out, const std::size_t a_k, const uint32_t a_exclude = InvalidIndex) noexcept
    : m_out(a_out), m_k(a_k), m_exclude(a_exclude)
  {
    EBGEOMETRY_EXPECT(a_out != nullptr);
    EBGEOMETRY_EXPECT(a_k > 0);
  }

  /**
   * @brief Offer one candidate.
   * @details Rejected if it is the excluded point or no closer than bound(); otherwise inserted in
   * order, dropping the farthest candidate once k are held.
   * @param[in] a_distanceSquared Squared distance from the query to the candidate.
   * @param[in] a_index           Cloud index of the candidate.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  insert(const T a_distanceSquared, const uint32_t a_index) noexcept
  {
    if (a_distanceSquared >= m_bound || a_index == m_exclude) {
      return;
    }

    std::size_t slot = (m_found < m_k) ? m_found++ : (m_k - 1);

    for (; slot > 0 && m_out[slot - 1].distanceSquared > a_distanceSquared; slot--) {
      m_out[slot] = m_out[slot - 1];
    }

    m_out[slot] = Hit<T>{a_index, a_distanceSquared};

    if (m_found == m_k) {
      m_bound = m_out[m_k - 1].distanceSquared;
    }
  }

  /**
   * @brief The squared distance a candidate must beat to be inserted.
   * @details Math::Limits<T>::max() until k candidates are held, then the k-th smallest distance.
   * A search may skip any region at least this far from the query.
   * @return The pruning bound.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  bound() const noexcept
  {
    return m_bound;
  }

  /**
   * @brief Number of candidates held, at most k.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  std::size_t
  found() const noexcept
  {
    return m_found;
  }

private:
  Hit<T>*     m_out;                            ///< Caller's buffer, sorted ascending.
  std::size_t m_k;                              ///< Capacity: neighbors wanted.
  uint32_t    m_exclude;                        ///< Cloud index never reported.
  std::size_t m_found = 0;                      ///< Candidates held.
  T           m_bound = Math::Limits<T>::max(); ///< Distance to beat.
};

/**
 * @brief The a_k points of a cloud nearest to a query point, by a full scan.
 * @details An O(N) reference for checking PointCloudBVH and PointCloudHashGrid, never a hot path.
 * It follows their result contract: the buffer is filled in ascending order of distance, and the
 * count found is returned. For a self-query (the nearest *other* points to cloud point i), pass
 * that point's position as @p a_query and i as @p a_exclude. Callable on the host and on a device.
 * @tparam T Floating-point precision.
 * @param[in]  a_positions Point positions, indexed by cloud index. May be null if a_numPoints is 0.
 * @param[in]  a_numPoints Number of points.
 * @param[in]  a_query     Query point.
 * @param[in]  a_k         Number of neighbors wanted. Must be > 0.
 * @param[out] a_out       Buffer of at least a_k Hits.
 * @param[in]  a_exclude   Cloud index never to report, or InvalidIndex.
 * @return The number of neighbors found: min(a_k, number of points not excluded).
 */
template <class T>
EBGEOMETRY_HOST_DEVICE
inline std::size_t
closestPointsBruteForce(const Vec3T<T>*   a_positions,
                        const uint32_t    a_numPoints,
                        const Vec3T<T>&   a_query,
                        const std::size_t a_k,
                        Hit<T>*           a_out,
                        const uint32_t    a_exclude = InvalidIndex) noexcept
{
  EBGEOMETRY_EXPECT(a_positions != nullptr || a_numPoints == 0);

  KBest<T> best(a_out, a_k, a_exclude);

  for (uint32_t i = 0; i < a_numPoints; i++) {
    best.insert((a_positions[i] - a_query).length2(), i);
  }

  return best.found();
}

/**
 * @brief The point of a cloud nearest to a query point, by a full scan.
 * @details closestPointsBruteForce() with a_k = 1; a miss if every point is excluded or the cloud
 * is empty.
 * @tparam T Floating-point precision.
 * @param[in] a_positions Point positions, indexed by cloud index. May be null if a_numPoints is 0.
 * @param[in] a_numPoints Number of points.
 * @param[in] a_query     Query point.
 * @param[in] a_exclude   Cloud index never to report, or InvalidIndex.
 * @return The nearest point and its squared distance.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline Hit<T>
closestPointBruteForce(const Vec3T<T>* a_positions,
                       const uint32_t  a_numPoints,
                       const Vec3T<T>& a_query,
                       const uint32_t  a_exclude = InvalidIndex) noexcept
{
  Hit<T> hit;

  closestPointsBruteForce(a_positions, a_numPoints, a_query, 1, &hit, a_exclude);

  return hit;
}

} // namespace PointCloud

} // namespace EBGeometry

#endif
