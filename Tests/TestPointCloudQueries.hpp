// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Shared by the PointCloudBVH and PointCloudHashGrid tests: random clouds, an independent brute-force
// oracle, and query functors over either structure. The two answer the same queries with the same
// PointCloud::Hit, so every functor below is templated on the cloud type and the same checks run on
// both, on the host and in a device kernel. Test infrastructure only.

#ifndef EBGEOMETRY_TEST_POINTCLOUDQUERIES_HPP
#define EBGEOMETRY_TEST_POINTCLOUDQUERIES_HPP

#include "EBGeometry.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace EBGeometryTestPointCloud {

using namespace EBGeometry;

// A fixed, reproducible random cloud of n points in the unit cube.
template <class T>
std::vector<Vec3T<T>>
makeCloud(std::size_t a_n, unsigned a_seed)
{
  std::mt19937                      rng(a_seed);
  std::uniform_real_distribution<T> dist(T(0), T(1));
  std::vector<Vec3T<T>>             pos(a_n);

  for (std::size_t i = 0; i < a_n; i++) {
    pos[i] = Vec3T<T>(dist(rng), dist(rng), dist(rng));
  }

  return pos;
}

// Brute-force k nearest (squared) distances to a_query, optionally excluding one index. Sorted. An
// oracle independent of the library, including of PointCloud::closestPointsBruteForce().
template <class T>
std::vector<T>
bruteForce(const std::vector<Vec3T<T>>& a_pos, const Vec3T<T>& a_query, std::size_t a_k, std::size_t a_exclude)
{
  std::vector<T> d2;

  d2.reserve(a_pos.size());

  for (std::size_t i = 0; i < a_pos.size(); i++) {
    if (i == a_exclude) {
      continue;
    }

    d2.push_back((a_pos[i] - a_query).length2());
  }

  std::sort(d2.begin(), d2.end());
  d2.resize(std::min(a_k, d2.size()));

  return d2;
}

// The cloud queries the functors below run. The first three take an arbitrary query point, the last
// three a cloud point's own index. The brute-force ones run PointCloud::closestPoint(s)BruteForce()
// over the cloud's own stored positions, which are contiguous in its pool.
enum class CloudQuery
{
  ClosestPoint,
  ClosestPointBruteForce,
  ClosestPoints,
  NearestNeighbor,
  NearestNeighborBruteForce,
  NearestNeighbors
};

// k for the k-nearest queries (closestPoints/nearestNeighbors).
constexpr std::size_t g_cloudK = 3;

/**
 * @brief One hit of a query at an arbitrary point: the nearest, or the a_rank-th nearest (0 = nearest)
 * for ClosestPoints. A self query, or a rank past the hits found, gives an invalid Hit.
 */
template <class Cloud, class T>
EBGEOMETRY_HOST_DEVICE
PointCloud::Hit<T>
cloudHit(const Cloud& a_cloud, const CloudQuery a_query, const std::size_t a_rank, const Vec3T<T>& a_point) noexcept
{
  PointCloud::Hit<T> out[g_cloudK];

  switch (a_query) {
  case CloudQuery::ClosestPoint:
    return a_cloud.closestPoint(a_point);
  case CloudQuery::ClosestPointBruteForce:
    return PointCloud::closestPointBruteForce(&a_cloud.position(0), uint32_t(a_cloud.numPoints()), a_point);
  case CloudQuery::ClosestPoints:
    return (a_rank < a_cloud.closestPoints(a_point, g_cloudK, out)) ? out[a_rank] : PointCloud::Hit<T>{};
  default:
    return PointCloud::Hit<T>{};
  }
}

/**
 * @brief One hit of a query about cloud point a_point: its nearest other point, or the a_rank-th nearest
 * (0 = nearest) for NearestNeighbors. A point query, or a rank past the hits found, gives an invalid Hit.
 */
template <class Cloud, class T>
EBGEOMETRY_HOST_DEVICE
PointCloud::Hit<T>
cloudHit(const Cloud& a_cloud, const CloudQuery a_query, const std::size_t a_rank, const uint32_t a_point) noexcept
{
  PointCloud::Hit<T> out[g_cloudK];

  switch (a_query) {
  case CloudQuery::NearestNeighbor:
    return a_cloud.nearestNeighbor(a_point);
  case CloudQuery::NearestNeighborBruteForce:
    return PointCloud::closestPointBruteForce(
      &a_cloud.position(0), uint32_t(a_cloud.numPoints()), a_cloud.position(a_point), a_point);
  case CloudQuery::NearestNeighbors:
    return (a_rank < a_cloud.nearestNeighbors(a_point, g_cloudK, out)) ? out[a_rank] : PointCloud::Hit<T>{};
  default:
    return PointCloud::Hit<T>{};
  }
}

// The functors below are what the device tests evaluate, one query (a point, or a cloud index) per
// thread. They are plain host-device code so the host suite runs exactly the same code. Each holds
// the cloud by value, as a kernel receives it.

// The matched point's index for each query (Q = Vec3T<T> for the point queries, uint32_t for the self
// queries).
template <class Cloud, class T, class Q>
struct CloudHitIndexQuery
{
  Cloud       m_cloud;
  CloudQuery  m_query;
  std::size_t m_rank;

  EBGEOMETRY_HOST_DEVICE
  std::size_t
  operator()(const Q& a_q) const noexcept
  {
    return cloudHit<Cloud, T>(m_cloud, m_query, m_rank, a_q).index;
  }
};

// The squared distance to the matched point for each query.
template <class Cloud, class T, class Q>
struct CloudHitDistanceQuery
{
  Cloud       m_cloud;
  CloudQuery  m_query;
  std::size_t m_rank;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Q& a_q) const noexcept
  {
    return cloudHit<Cloud, T>(m_cloud, m_query, m_rank, a_q).distanceSquared;
  }
};

// The number of hits closestPoints() returns for each query point.
template <class Cloud, class T>
struct ClosestPointsFoundQuery
{
  Cloud m_cloud;

  EBGEOMETRY_HOST_DEVICE
  std::size_t
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    PointCloud::Hit<T> out[g_cloudK];

    return m_cloud.closestPoints(a_point, g_cloudK, out);
  }
};

// The number of hits nearestNeighbors() returns for each cloud point.
template <class Cloud, class T>
struct NearestNeighborsFoundQuery
{
  Cloud m_cloud;

  EBGEOMETRY_HOST_DEVICE
  std::size_t
  operator()(const uint32_t& a_point) const noexcept
  {
    PointCloud::Hit<T> out[g_cloudK];

    return m_cloud.nearestNeighbors(a_point, g_cloudK, out);
  }
};

// Component m_axis of each cloud point's stored position.
template <class Cloud, class T>
struct CloudPositionQuery
{
  Cloud       m_cloud;
  std::size_t m_axis;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const uint32_t& a_point) const noexcept
  {
    return m_cloud.position(a_point)[m_axis];
  }
};

// The cloud's size, whatever the query.
template <class Cloud>
struct CloudSizeQuery
{
  Cloud m_cloud;

  EBGEOMETRY_HOST_DEVICE
  std::size_t
  operator()(const uint32_t& /*a_point*/) const noexcept
  {
    return m_cloud.numPoints();
  }
};

/**
 * @brief Compare two clouds through every functor above.
 * @details For each functor, calls a_sameIndices(fFirst, fSecond, queries) for an integer result and
 * a_sameValues(fFirst, fSecond, queries) for a floating-point one, where fFirst holds a_first and
 * fSecond holds a_second. The callables decide where each runs and how closely the results must agree.
 */
template <class Cloud, class T, class SameIndices, class SameValues>
void
compareCloudQueries(const Cloud&                 a_first,
                    const Cloud&                 a_second,
                    const std::vector<Vec3T<T>>& a_points,
                    const std::vector<uint32_t>& a_indices,
                    const SameIndices&           a_sameIndices,
                    const SameValues&            a_sameValues)
{
  using Vec3 = Vec3T<T>;

  const auto rankCount = [](const CloudQuery a_query) {
    return (a_query == CloudQuery::ClosestPoints || a_query == CloudQuery::NearestNeighbors) ? g_cloudK : 1;
  };

  for (const auto query : {CloudQuery::ClosestPoint, CloudQuery::ClosestPointBruteForce, CloudQuery::ClosestPoints}) {
    for (std::size_t rank = 0; rank < rankCount(query); rank++) {
      INFO("point query " << static_cast<int>(query) << ", rank " << rank);
      a_sameIndices(CloudHitIndexQuery<Cloud, T, Vec3>{a_first, query, rank},
                    CloudHitIndexQuery<Cloud, T, Vec3>{a_second, query, rank},
                    a_points);
      a_sameValues(CloudHitDistanceQuery<Cloud, T, Vec3>{a_first, query, rank},
                   CloudHitDistanceQuery<Cloud, T, Vec3>{a_second, query, rank},
                   a_points);
    }
  }

  for (const auto query :
       {CloudQuery::NearestNeighbor, CloudQuery::NearestNeighborBruteForce, CloudQuery::NearestNeighbors}) {
    for (std::size_t rank = 0; rank < rankCount(query); rank++) {
      INFO("self query " << static_cast<int>(query) << ", rank " << rank);
      a_sameIndices(CloudHitIndexQuery<Cloud, T, uint32_t>{a_first, query, rank},
                    CloudHitIndexQuery<Cloud, T, uint32_t>{a_second, query, rank},
                    a_indices);
      a_sameValues(CloudHitDistanceQuery<Cloud, T, uint32_t>{a_first, query, rank},
                   CloudHitDistanceQuery<Cloud, T, uint32_t>{a_second, query, rank},
                   a_indices);
    }
  }

  a_sameIndices(ClosestPointsFoundQuery<Cloud, T>{a_first}, ClosestPointsFoundQuery<Cloud, T>{a_second}, a_points);
  a_sameIndices(
    NearestNeighborsFoundQuery<Cloud, T>{a_first}, NearestNeighborsFoundQuery<Cloud, T>{a_second}, a_indices);

  for (std::size_t axis = 0; axis < 3; axis++) {
    INFO("axis " << axis);
    a_sameValues(CloudPositionQuery<Cloud, T>{a_first, axis}, CloudPositionQuery<Cloud, T>{a_second, axis}, a_indices);
  }

  a_sameIndices(CloudSizeQuery<Cloud>{a_first}, CloudSizeQuery<Cloud>{a_second}, a_indices);
}

// The cloud the query-functor tests share: 500 points in the unit cube.
template <class T>
std::vector<Vec3T<T>>
functorTestCloud()
{
  return makeCloud<T>(500, 4321u);
}

// Every cloud index of a cloud of a_n points.
inline std::vector<uint32_t>
allIndices(const std::size_t a_n)
{
  std::vector<uint32_t> indices(a_n);

  for (std::size_t i = 0; i < a_n; i++) {
    indices[i] = static_cast<uint32_t>(i);
  }

  return indices;
}

// Query points around and inside the unit-cube cloud.
template <class T>
std::vector<Vec3T<T>>
functorTestPoints()
{
  std::vector<Vec3T<T>> points;

  // A grid over [-0.25, 1.25]^3, offset so that no point lies on a grid-aligned plane.
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      for (int k = 0; k < 10; k++) {
        points.emplace_back(T(-0.25) + T(0.15) * (T(i) + T(0.37)),
                            T(-0.25) + T(0.15) * (T(j) + T(0.53)),
                            T(-0.25) + T(0.15) * (T(k) + T(0.61)));
      }
    }
  }

  return points;
}

} // namespace EBGeometryTestPointCloud

#endif
