// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for the mesh signed distance functions: TriMeshSDF and MeshSDF on the armadillo
// mesh (the dodecahedron under --smoke), in float and double. A host build runs the host cases (builds,
// refits, signed distances); a CUDA or HIP build runs the signed distances as host/device pairs. See
// ../README.md and PerformanceTesting.rst.

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// The branching factor and the leaf sizes, spelled out rather than taken from the defaults, so that a
// change to those shows as a change of these cases' parameters: TriMeshSDF's in groups of W triangles,
// MeshSDF's in faces (its default, K - 1).
constexpr std::size_t K             = 4;
constexpr std::size_t MaxLeafGroups = 4;
constexpr std::size_t MaxLeafFaces  = K - 1;

// The numbers of queries, at full and at smoke size.
constexpr std::size_t HostQueries  = 100000;
constexpr std::size_t PairQueries  = 262144;
constexpr std::size_t SmokeQueries = 1000;

// The refits a refit repetition does: one takes a few milliseconds, too short to time steadily.
constexpr std::size_t Refits = 10;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

// Random points in the mesh's bounding box grown to 1.5 times its size, so that queries fall inside,
// near and away from the surface.
template <class T>
std::vector<Vec3<T>>
queryPoints(const EBGeometry::BoundingVolumes::AABBT<T>& a_box, std::size_t a_n)
{
  const auto c = T(0.5) * (a_box.getLowCorner() + a_box.getHighCorner());
  const auto h = T(0.75) * (a_box.getHighCorner() - a_box.getLowCorner());

  return Perf::pointsInBox<T>(EBGeometry::BoundingVolumes::AABBT<T>(c - h, c + h), a_n, Perf::Seed::MeshQueries);
}

// The query cases both SDF types have: on the host and, if a_pairs, as host/device pairs.
// a_make(pool, file) reads the mesh into an SDF; a_params(case) records the SDF's parameters.
template <class T, class SDF, class Make, class Params>
void
addQueryCases(Perf::Suite& a_suite, const std::string& a_prefix, Make a_make, Params a_params, bool a_pairs)
{
  const std::string prec = Perf::precisionName<T>();

  a_suite.add(Perf::caseName({a_prefix, "query", prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
    const SDF        sdf = a_make(pool, Perf::meshFile(c.smoke()));
    const auto       q   = queryPoints<T>(sdf.computeBoundingVolume(), c.size(HostQueries, SmokeQueries));
    std::vector<T>   distances;

    Perf::measureHostQuery(c, Perf::SignedDistance<SDF, T>{sdf}, q, distances);

    c.checksum(Perf::checksumOf(distances));
    c.poolBytes(pool.usedBytes());
    a_params(c);
  });

  if (!a_pairs) {
    return;
  }

  for (const Perf::Order order : Perf::Orders) {
    a_suite.add(
      Perf::caseName({a_prefix, std::string("query-") + Perf::orderName(order), prec, "hostdevice"}),
      [=](Perf::Case& c) {
        EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
        const SDF        sdf = a_make(pool, Perf::meshFile(c.smoke()));
        const auto       q =
          Perf::inOrder(order, queryPoints<T>(sdf.computeBoundingVolume(), c.size(PairQueries, SmokeQueries)));

        const auto distances = Perf::measureHostAndDevice<T>(
          c,
          pool,
          q,
          [&](const EBGeometry::Pool& a_pool) { return Perf::SignedDistance<SDF, T>{sdf.rebasedView(a_pool)}; },
          Perf::agreeDistance<T>);

        c.checksum(Perf::checksumOf(distances));
        a_params(c);
      });
  }
}

// TriMeshSDF at width W. Widths above 4 are host-only (see ConfigurationOptions.rst), so they get no
// host/device pairs.
template <class T, std::size_t W>
void
addTriMeshSDF(Perf::Suite& a_suite)
{
  using SDF = EBGeometry::TriMeshSDF<T, K, W>;

  const std::string prefix = Perf::caseName({"TriMeshSDF", "w" + std::to_string(W)});

  const auto params = [](Perf::Case& c) {
    c.param("K", K);
    c.param("W", W);
    c.param("maxLeafGroups", MaxLeafGroups);
  };

  a_suite.add(Perf::caseName({prefix, "build", Perf::precisionName<T>(), "host"}), [=](Perf::Case& c) {
    const auto triangles = EBGeometry::Parser::readIntoTriangles<T>(Perf::meshFile(c.smoke()));
    const auto built     = Perf::measureBuild<SDF>(c, [&](EBGeometry::Pool& a_pool) {
      return SDF(triangles, a_pool, EBGeometry::BVH::Construction::SAH, MaxLeafGroups);
    });

    c.poolBytes(built.pool->usedBytes());
    c.checksum(static_cast<double>(built.object->signedDistance(Vec3<T>(T(0), T(0), T(0)))));
    c.param("triangles", triangles.size());
    params(c);
  });

  addQueryCases<T, SDF>(
    a_suite,
    prefix,
    [](EBGeometry::Pool& a_pool, const std::string& a_file) {
      return SDF(
        EBGeometry::Parser::readIntoTriangles<T>(a_file), a_pool, EBGeometry::BVH::Construction::SAH, MaxLeafGroups);
    },
    params,
    W <= 4);
}

template <class T>
void
addMeshSDF(Perf::Suite& a_suite)
{
  using SDF  = EBGeometry::MeshSDF<T, K>;
  using Mesh = EBGeometry::DCEL::MeshT<T>;

  const std::string prefix = "MeshSDF";
  const std::string prec   = Perf::precisionName<T>();

  const EBGeometry::BVH::ConstructionOptions options{MaxLeafFaces, 1, {}};

  const auto params = [](Perf::Case& c) {
    c.param("K", K);
    c.param("maxLeafFaces", MaxLeafFaces);
  };

  // The build alone: each repetition's pool has the mesh read into it, untimed.
  a_suite.add(Perf::caseName({prefix, "build", prec, "host"}), [=](Perf::Case& c) {
    const auto built = Perf::measureBuild<SDF>(
      c,
      [&](EBGeometry::Pool& a_pool) { return EBGeometry::Parser::readIntoDCEL<T>(Perf::meshFile(c.smoke()), a_pool); },
      [&](EBGeometry::Pool& a_pool, const Mesh& a_mesh) {
        return SDF(a_mesh, a_pool, EBGeometry::BVH::Construction::SAH, options);
      });

    c.poolBytes(built.pool->usedBytes());
    c.checksum(static_cast<double>(built.object->signedDistance(Vec3<T>(T(0), T(0), T(0)))));
    params(c);
  });

  // Refitting the BVH to the faces' boxes. The SDF is a descriptor, so getRoot() of a const one still
  // refits the BVH in its pool.
  a_suite.add(Perf::caseName({prefix, "refit", prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
    const SDF        sdf = EBGeometry::Parser::readIntoMeshSDF<T, K>(
      Perf::meshFile(c.smoke()), pool, EBGeometry::BVH::Construction::SAH, options);
    const auto mesh = sdf.getMesh();

    c.measure([&] {
      for (std::size_t r = 0; r < Refits; r++) {
        sdf.getRoot().refit([&mesh](std::uint32_t a_face) {
          return EBGeometry::BoundingVolumes::AABBT<T>(mesh.getFace(a_face).getAllVertexCoordinates(mesh));
        });
      }
    });

    c.checksum(static_cast<double>(sdf.computeBoundingVolume().getHighCorner()[0]));
    c.param("refits", Refits);
    params(c);
  });

  addQueryCases<T, SDF>(
    a_suite,
    prefix,
    [options](EBGeometry::Pool& a_pool, const std::string& a_file) {
      return EBGeometry::Parser::readIntoMeshSDF<T, K>(a_file, a_pool, EBGeometry::BVH::Construction::SAH, options);
    },
    params,
    true);
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "Mesh");

  addTriMeshSDF<float, 4>(suite);
  addTriMeshSDF<double, 4>(suite);
  addTriMeshSDF<float, 8>(suite);
  addMeshSDF<float>(suite);
  addMeshSDF<double>(suite);

  return suite.run();
}
