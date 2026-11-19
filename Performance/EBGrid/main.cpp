// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

// Performance cases for an embedded-boundary workload: the armadillo's signed distance (TriMeshSDF and MeshSDF) at
// the cell centers of a uniform grid around it, the domain split into boxes of cells and visited box by box and cell
// by cell, as AMReX and Chombo generate an embedded boundary. The same points in three orders: the grid's, their
// Morton order (GPU::mortonOrder()), and shuffled. The grid order is the workload; the other two bound what ordering
// can still give on it (Morton) and what it is worth (shuffled). On the host, and as host/device pairs. See
// ../README.md and PerformanceTesting.rst.

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// The branching factor and the leaf sizes, spelled out as in ../Mesh/main.cpp: TriMeshSDF's leaves in groups of W
// triangles, MeshSDF's in faces (its default, K - 1).
constexpr std::size_t K             = 4;
constexpr std::size_t W             = 4;
constexpr std::size_t MaxLeafGroups = 4;
constexpr std::size_t MaxLeafFaces  = K - 1;

// Cells per box side and per domain side (a multiple of the box side). A box's cells are visited x fastest, so a warp
// of 32 threads in grid order takes one row of cells along x, as it would in an embedded-boundary generator's
// kernel over a box of 32^3 cells. The pairs' grid of 2 x 2 x 2 such boxes keeps a repetition on the device at a few
// milliseconds. The host runs on one core, so it gets a grid of 2 x 2 x 2 boxes of 16^3 cells.
constexpr std::size_t PairBoxCells  = 32;
constexpr std::size_t PairCells     = 64;
constexpr std::size_t HostBoxCells  = 16;
constexpr std::size_t HostCells     = 32;
constexpr std::size_t SmokeBoxCells = 4;
constexpr std::size_t SmokeCells    = 8;

// The domain is a cube around the mesh's box, this many times its longest side, so the outer boxes hold no surface.
constexpr double DomainScale = 1.25;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

template <class T>
using TriMesh = EBGeometry::TriMeshSDF<T, K, W>;

template <class T>
using Mesh = EBGeometry::MeshSDF<T, K>;

// The order in which the cell centers are queried.
enum class Order
{
  Grid,    ///< Box by box, and in a box cell by cell (x fastest): an embedded-boundary generator's loop.
  Morton,  ///< Along a Morton curve over the points' box (GPU::mortonOrder()).
  Shuffled ///< A random permutation, from a fixed seed.
};

constexpr Order Orders[] = {Order::Grid, Order::Morton, Order::Shuffled};

const char*
orderName(Order a_order)
{
  switch (a_order) {
  case Order::Grid: {
    return "grid";
  }
  case Order::Morton: {
    return "morton";
  }
  default: {
    return "shuffled";
  }
  }
}

// The cell centers of a cube of a_cells^3 cells around a_box, in grid order: boxes of a_boxCells^3 cells, the boxes
// x fastest, and in each box the cells x fastest.
template <class T>
std::vector<Vec3<T>>
gridPoints(const EBGeometry::BoundingVolumes::AABBT<T>& a_box, std::size_t a_cells, std::size_t a_boxCells)
{
  const Vec3<T> extent = a_box.getHighCorner() - a_box.getLowCorner();
  const Vec3<T> center = T(0.5) * (a_box.getLowCorner() + a_box.getHighCorner());
  const T       side   = T(DomainScale) * extent[extent.maxDir(true)];
  const T       dx     = side / T(a_cells);
  const Vec3<T> lo     = center - T(0.5) * Vec3<T>(side, side, side);
  const auto    boxes  = a_cells / a_boxCells;

  std::vector<Vec3<T>> points;

  points.reserve(a_cells * a_cells * a_cells);

  for (std::size_t bz = 0; bz < boxes; bz++) {
    for (std::size_t by = 0; by < boxes; by++) {
      for (std::size_t bx = 0; bx < boxes; bx++) {
        for (std::size_t k = 0; k < a_boxCells; k++) {
          for (std::size_t j = 0; j < a_boxCells; j++) {
            for (std::size_t i = 0; i < a_boxCells; i++) {
              const auto x = static_cast<T>(bx * a_boxCells + i) + T(0.5);
              const auto y = static_cast<T>(by * a_boxCells + j) + T(0.5);
              const auto z = static_cast<T>(bz * a_boxCells + k) + T(0.5);

              points.emplace_back(lo + dx * Vec3<T>(x, y, z));
            }
          }
        }
      }
    }
  }

  return points;
}

// The permutation that presents the grid's points in an order: entry i is the grid index queried i-th.
template <class T>
std::vector<std::uint32_t>
permutation(Order a_order, const std::vector<Vec3<T>>& a_points)
{
  const auto                 n = static_cast<std::uint32_t>(a_points.size());
  std::vector<std::uint32_t> perm(n);

  for (std::uint32_t i = 0; i < n; i++) {
    perm[i] = i;
  }

  if (a_order == Order::Morton) {
    EBGeometry::GPU::mortonOrder(EBGeometry::hostMemoryResource(),
                                 EBGeometry::PointCloud::FromArray<T>{a_points.data()},
                                 EBGeometry::PODSpan<std::uint32_t>{perm.data(), n});
  }
  else if (a_order == Order::Shuffled) {
    // Fisher-Yates, from a xorshift64* generator of the probe's own (not the standard library's distributions).
    std::uint64_t state = Perf::Seed::GridShuffle * 6364136223846793005ULL + 1442695040888963407ULL;

    for (std::uint32_t i = n - 1; i > 0; i--) {
      state ^= state >> 12;
      state ^= state << 25;
      state ^= state >> 27;

      const auto j = static_cast<std::uint32_t>(((state * 2685821657736338717ULL) >> 32) % (i + 1));

      std::swap(perm[i], perm[j]);
    }
  }

  return perm;
}

// The grid's points in an order, and the permutation to put their results back in grid order.
template <class T>
struct Queries
{
  std::vector<Vec3<T>>       points;
  std::vector<std::uint32_t> perm;
};

template <class T>
Queries<T>
queriesInOrder(Perf::Case&                                  a_case,
               Order                                        a_order,
               const EBGeometry::BoundingVolumes::AABBT<T>& a_box,
               std::size_t                                  a_cells,
               std::size_t                                  a_boxCells)
{
  const std::size_t cells    = a_case.size(a_cells, SmokeCells);
  const std::size_t boxCells = a_case.size(a_boxCells, SmokeBoxCells);
  const auto        grid     = gridPoints<T>(a_box, cells, boxCells);

  Queries<T> q{std::vector<Vec3<T>>(grid.size()), permutation<T>(a_order, grid)};

  for (std::size_t i = 0; i < grid.size(); i++) {
    q.points[i] = grid[q.perm[i]];
  }

  a_case.param("cells", cells);
  a_case.param("boxCells", boxCells);

  return q;
}

// The results back in grid order, so that the three orders share one checksum.
template <class T>
double
gridChecksum(const std::vector<T>& a_results, const std::vector<std::uint32_t>& a_perm)
{
  std::vector<T> grid(a_results.size());

  for (std::size_t i = 0; i < a_results.size(); i++) {
    grid[a_perm[i]] = a_results[i];
  }

  return Perf::checksumOf(grid);
}

// The cases of one SDF: each order on the host and as a host/device pair, the Morton and shuffled cases naming the
// grid's. a_make(pool, file) reads the mesh into the SDF; a_params(case) records its parameters.
template <class T, class SDF, class Make, class Params>
void
addCases(Perf::Suite& a_suite, const std::string& a_sdf, Make a_make, Params a_params)
{
  using Distance = Perf::SignedDistance<SDF, T>;

  const std::string prec  = Perf::precisionName<T>();
  const std::string input = a_sdf + "/armadillo";

  for (const Order order : Orders) {
    const std::string operation = std::string("distance-") + orderName(order);
    const std::string label     = std::string("grid/") + orderName(order);

    a_suite.add(Perf::caseName({"EBGrid", input, operation, prec, "host"}), [=](Perf::Case& c) {
      EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
      const SDF        sdf = a_make(pool, Perf::meshFile(c.smoke()));
      const auto       q   = queriesInOrder<T>(c, order, sdf.computeBoundingVolume(), HostCells, HostBoxCells);
      std::vector<T>   out;

      Perf::measureHostQuery(c, Distance{sdf}, q.points, out);

      c.checksum(gridChecksum(out, q.perm));
      c.poolBytes(pool.usedBytes());
      a_params(c);

      if (order != Order::Grid) {
        c.relativeTo(Perf::caseName({"EBGrid", input, "distance-grid", prec, "host"}), label);
      }
    });

    a_suite.add(Perf::caseName({"EBGrid", input, operation, prec, "hostdevice"}), [=](Perf::Case& c) {
      EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
      const SDF        sdf = a_make(pool, Perf::meshFile(c.smoke()));
      const auto       q   = queriesInOrder<T>(c, order, sdf.computeBoundingVolume(), PairCells, PairBoxCells);
      const auto       out = Perf::measureHostAndDevice<T>(
        c,
        pool,
        q.points,
        [sdf](const EBGeometry::Pool& a_on) { return Distance{sdf.rebasedView(a_on)}; },
        Perf::agreeDistance<T>);

      c.checksum(gridChecksum(out, q.perm));
      c.poolBytes(pool.usedBytes());
      a_params(c);

      if (order != Order::Grid) {
        c.relativeTo(Perf::caseName({"EBGrid", input, "distance-grid", prec, "hostdevice"}), label);
      }
    });
  }
}

template <class T>
void
addTriMeshSDF(Perf::Suite& a_suite)
{
  addCases<T, TriMesh<T>>(
    a_suite,
    "TriMeshSDF",
    [](EBGeometry::Pool& a_pool, const std::string& a_file) {
      return TriMesh<T>(
        EBGeometry::Parser::readIntoTriangles<T>(a_file), a_pool, EBGeometry::BVH::Construction::SAH, MaxLeafGroups);
    },
    [](Perf::Case& c) {
      c.param("K", K);
      c.param("W", W);
      c.param("maxLeafGroups", MaxLeafGroups);
    });
}

template <class T>
void
addMeshSDF(Perf::Suite& a_suite)
{
  addCases<T, Mesh<T>>(
    a_suite,
    "MeshSDF",
    [](EBGeometry::Pool& a_pool, const std::string& a_file) {
      const EBGeometry::BVH::ConstructionOptions options{MaxLeafFaces, 1, {}};

      return EBGeometry::Parser::readIntoMeshSDF<T, K>(a_file, a_pool, EBGeometry::BVH::Construction::SAH, options);
    },
    [](Perf::Case& c) {
      c.param("K", K);
      c.param("maxLeafFaces", MaxLeafFaces);
    });
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "EBGrid");

  addTriMeshSDF<float>(suite);
  addTriMeshSDF<double>(suite);
  addMeshSDF<float>(suite);
  addMeshSDF<double>(suite);

  return suite.run();
}
