// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for the mesh signed distance functions on grid queries: TriMeshSDF and MeshSDF on the
// happy Buddha (the dodecahedron under --smoke), evaluated at the nodes of a regular grid over the mesh's
// box, in grid order, in float and double. This is the protocol of Sanchez, Fryazinov and Pasko, "Efficient
// evaluation of continuous signed distance to a polygonal mesh" (SCCG 2012), at a smaller grid. A host
// build runs the host cases; a CUDA or HIP build runs host/device pairs. The cases are in their own probe
// so that they cannot change how the Mesh probe's cases are compiled. See ../README.md and
// PerformanceTesting.rst.

#include <cstddef>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// The branching factor and the leaf sizes, as in the Mesh probe: TriMeshSDF's in groups of W triangles,
// MeshSDF's in faces.
constexpr std::size_t K             = 4;
constexpr std::size_t W             = 4;
constexpr std::size_t MaxLeafGroups = 4;
constexpr std::size_t MaxLeafFaces  = K - 1;

// Grid nodes along each axis, at full and at smoke size.
constexpr std::size_t GridNodes      = 48;
constexpr std::size_t SmokeGridNodes = 8;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

// The mesh: the happy Buddha (100k triangles, the paper's model), or the dodecahedron under --smoke.
std::string
gridMeshFile(const bool a_smoke)
{
  return a_smoke ? Perf::meshFile(true) : std::string(EBGEOMETRY_PERF_MESH_DIR) + "/happy.obj";
}

// The nodes of an n^3 grid spanning the box, x fastest.
template <class T>
std::vector<Vec3<T>>
gridPoints(const EBGeometry::BoundingVolumes::AABBT<T>& a_box, const std::size_t a_n)
{
  const Vec3<T> lo = a_box.getLowCorner();
  const Vec3<T> h  = (a_box.getHighCorner() - lo) / T(a_n - 1);

  std::vector<Vec3<T>> points;
  points.reserve(a_n * a_n * a_n);

  for (std::size_t k = 0; k < a_n; k++) {
    for (std::size_t j = 0; j < a_n; j++) {
      for (std::size_t i = 0; i < a_n; i++) {
        points.emplace_back(lo + Vec3<T>(T(i) * h[0], T(j) * h[1], T(k) * h[2]));
      }
    }
  }

  return points;
}

// The grid cases of one SDF type: on the host, and as a host/device pair if a_pairs.
template <class T, class SDF, class Make, class Params>
void
addGridCases(Perf::Suite& a_suite, const std::string& a_prefix, Make a_make, Params a_params)
{
  const std::string prec = Perf::precisionName<T>();

  a_suite.add(Perf::caseName({a_prefix, "query-grid", prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
    const SDF        sdf = a_make(pool, gridMeshFile(c.smoke()));
    const auto       q   = gridPoints<T>(sdf.computeBoundingVolume(), c.size(GridNodes, SmokeGridNodes));
    std::vector<T>   distances;

    Perf::measureHostQuery(c, Perf::SignedDistance<SDF, T>{sdf}, q, distances);

    c.checksum(Perf::checksumOf(distances));
    c.poolBytes(pool.usedBytes());
    c.param("gridNodes", c.size(GridNodes, SmokeGridNodes));
    a_params(c);
  });

  a_suite.add(Perf::caseName({a_prefix, "query-grid", prec, "hostdevice"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
    const SDF        sdf = a_make(pool, gridMeshFile(c.smoke()));
    const auto       q   = gridPoints<T>(sdf.computeBoundingVolume(), c.size(GridNodes, SmokeGridNodes));

    const auto distances = Perf::measureHostAndDevice<T>(
      c,
      pool,
      q,
      [&](const EBGeometry::Pool& a_pool) { return Perf::SignedDistance<SDF, T>{sdf.rebasedView(a_pool)}; },
      Perf::agreeDistance<T>);

    c.checksum(Perf::checksumOf(distances));
    c.param("gridNodes", c.size(GridNodes, SmokeGridNodes));
    a_params(c);
  });
}

template <class T>
void
addTriMeshSDF(Perf::Suite& a_suite)
{
  using SDF = EBGeometry::TriMeshSDF<T, K, W>;

  addGridCases<T, SDF>(
    a_suite,
    Perf::caseName({"TriMeshSDF", "w" + std::to_string(W)}),
    [](EBGeometry::Pool& a_pool, const std::string& a_file) {
      return SDF(
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
  using SDF = EBGeometry::MeshSDF<T, K>;

  const EBGeometry::BVH::ConstructionOptions options{MaxLeafFaces, 1, {}};

  addGridCases<T, SDF>(
    a_suite,
    "MeshSDF",
    [options](EBGeometry::Pool& a_pool, const std::string& a_file) {
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
  Perf::Suite suite(argc, argv, "MeshGrid");

  addTriMeshSDF<float>(suite);
  addTriMeshSDF<double>(suite);
  addMeshSDF<float>(suite);
  addMeshSDF<double>(suite);

  return suite.run();
}
