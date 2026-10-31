// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Searching a point cloud: the closest points to arbitrary query points, and the nearest neighbors of
// points that are in the cloud (the k-nearest-neighbor graph). Two classes answer the same questions
// through the same interface, PointCloudBVH (a tree, adapts to clustered clouds) and PointCloudHashGrid
// (a uniform grid, builds faster on near-uniform clouds), so the program runs once with each. The last
// part of each run asks the same questions on a GPU. See README.md.

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

#include <EBGeometry.hpp>
#include <Source/EBGeometry_Random.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T    = EBGEOMETRY_PRECISION;
using Vec3 = EBGeometry::Vec3T<T>;

// What both classes report for a point they find: its cloud index (its position in the array the
// cloud was built from) and its squared distance.
using Hit = EBGeometry::PointCloud::Hit<T>;

// The cloud, the query points, and how many neighbors to find.
constexpr std::size_t   numPoints    = 20000;
constexpr std::size_t   numQueries   = 4;
constexpr std::size_t   numNeighbors = 3;
constexpr std::uint64_t pointSeed    = 123456789ULL;
constexpr std::uint64_t querySeed    = 987654321ULL;

namespace {

// ── The code that runs on either side ────────────────────────────────────────────────────────────────
//
// Queries for EBGeometry::GPU::transform(): trivially copyable functors whose EBGEOMETRY_HOST_DEVICE call
// operator maps one input to one result. Each holds the cloud by value. A cloud owns no memory: it is a
// descriptor of the points and the search structure in a Pool, so a kernel can receive it by value once
// it describes a pool the device can read.

// The closest point in the cloud to an arbitrary point.
template <class Cloud>
struct ClosestPoint
{
  Cloud m_cloud;

  EBGEOMETRY_HOST_DEVICE
  Hit
  operator()(const Vec3& a_point) const noexcept
  {
    return m_cloud.closestPoint(a_point);
  }
};

// The k = numNeighbors nearest other points of a point in the cloud, nearest first.
struct Neighbors
{
  Hit hit[numNeighbors];
};

template <class Cloud>
struct NearestNeighbors
{
  Cloud m_cloud;

  EBGEOMETRY_HOST_DEVICE
  Neighbors
  operator()(const std::uint32_t a_point) const noexcept
  {
    Neighbors result;

    m_cloud.nearestNeighbors(a_point, numNeighbors, result.hit);

    return result;
  }
};

// One point found by a search, with the user data the example keeps for it.
void
print(const Hit& a_hit, const std::vector<int>& a_octant)
{
  std::cout << "cloud index " << std::setw(5) << a_hit.index << ", distance " << std::sqrt(a_hit.distanceSquared)
            << ", octant " << a_octant[a_hit.index];
}

// Everything the example does with one kind of cloud. Cloud is PointCloudBVH<T> or PointCloudHashGrid<T>.
template <class Cloud>
void
run(const char*              a_name,
    const std::vector<Vec3>& a_positions,
    const std::vector<Vec3>& a_queries,
    const std::vector<int>&  a_octant)
{
  std::cout << "── " << a_name << " ──\n";

  // ── Build the cloud ────────────────────────────────────────────────────────────────────────────────
  //
  // One constructor groups the points, builds the search structure and stores both in the pool, which
  // must outlive the cloud and every copy of it.
  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const Cloud cloud(pool, a_positions);

  // ── Use it on the host ─────────────────────────────────────────────────────────────────────────────
  //
  // closestPoint(q) is the nearest point to any point q; closestPoints(q, k, out) writes the k nearest
  // to out, nearest first, and returns how many it found.
  std::cout << "closestPoint() and closestPoints() for an arbitrary point " << a_queries[0] << ":\n";
  std::cout << "  nearest: ";
  print(cloud.closestPoint(a_queries[0]), a_octant);
  std::cout << '\n';

  Hit               nearest[numNeighbors];
  const std::size_t found = cloud.closestPoints(a_queries[0], numNeighbors, nearest);

  for (std::size_t j = 0; j < found; j++) {
    std::cout << "  #" << j << "     : ";
    print(nearest[j], a_octant);
    std::cout << '\n';
  }

  // nearestNeighbor(i) is the nearest OTHER point to point i of the cloud, and allNearestNeighbors(k)
  // is the whole k-nearest-neighbor graph in one batched call: entry i * k + j is point i's j-th
  // nearest neighbor.
  const std::vector<Hit> graph = cloud.allNearestNeighbors(numNeighbors);

  std::cout << "nearestNeighbor(0)    : ";
  print(cloud.nearestNeighbor(0), a_octant);
  std::cout << "\nallNearestNeighbors(" << numNeighbors << "): " << graph.size() << " entries; point 1's are\n";

  for (std::size_t j = 0; j < numNeighbors; j++) {
    std::cout << "  #" << j << "     : ";
    print(graph[numNeighbors + j], a_octant);
    std::cout << '\n';
  }

  // ── The same queries on a GPU ──────────────────────────────────────────────────────────────────────
  //
  // Freeze the pool, mirror it in one piece into GPU::memoryResource() (device memory in a CUDA or HIP
  // build), and take a rebasedView() of the cloud that resolves against the copy. GPU::transform() then
  // runs a query once per input in a kernel. allNearestNeighbors() is a host-only batch, so on a device
  // the graph is computed one point per thread, over the points' cloud indices. In a plain C++ build
  // the same lines run on the host. Points at equal distance may be reported in either order.
  if (EBGeometry::GPU::enabled && !EBGeometry::GPU::deviceAvailable()) {
    std::cout << "On the device: skipped, no GPU is visible\n\n";

    return;
  }

  pool.freeze();

  const EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(pool, EBGeometry::GPU::memoryResource());
  const Cloud            onDevice   = cloud.rebasedView(devicePool);

  std::vector<Hit> closest(a_queries.size());

  EBGeometry::GPU::transform(ClosestPoint<Cloud>{onDevice}, a_queries, closest);

  std::vector<std::uint32_t> indices(a_positions.size());

  for (std::size_t i = 0; i < indices.size(); i++) {
    indices[i] = static_cast<std::uint32_t>(i);
  }

  std::vector<Neighbors> neighbors(indices.size());

  EBGeometry::GPU::transform(NearestNeighbors<Cloud>{onDevice}, indices, neighbors);

  std::cout << "Through GPU::transform (backend: " << EBGeometry::GPU::backendName << "):\n";

  for (std::size_t q = 0; q < a_queries.size(); q++) {
    std::cout << "  closest to query " << q << ": ";
    print(closest[q], a_octant);
    std::cout << '\n';
  }

  std::cout << "  point 1's nearest  : ";
  print(neighbors[1].hit[0], a_octant);
  std::cout << "\n\n";
}

} // namespace

int
main()
{
  // Random points in the unit cube, and a few query points that are not in the cloud.
  const std::vector<Vec3> positions = EBGeometry::Random::samplePoints<T>(numPoints, pointSeed);
  const std::vector<Vec3> queries   = EBGeometry::Random::samplePoints<T>(numQueries, querySeed);

  // A cloud carries no user data. A search reports a point's cloud index, so per-point data lives in an
  // array of your own, indexed the same way. Here each point gets a label: the eighth of the unit cube
  // it lies in.
  std::vector<int> octant(numPoints);

  for (std::size_t i = 0; i < numPoints; i++) {
    octant[i] =
      (positions[i][0] > T(0.5) ? 1 : 0) + (positions[i][1] > T(0.5) ? 2 : 0) + (positions[i][2] > T(0.5) ? 4 : 0);
  }

  std::cout << numPoints << " random points in the unit cube\n\n";

  // The two classes are interchangeable: the same code runs with either.
  run<EBGeometry::PointCloudBVH<T>>("PointCloudBVH", positions, queries, octant);
  run<EBGeometry::PointCloudHashGrid<T>>("PointCloudHashGrid", positions, queries, octant);

  return 0;
}
