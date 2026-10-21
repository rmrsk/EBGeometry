// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for the mesh signed distance functions: TriMeshSDF and MeshSDF on the armadillo
// mesh (the dodecahedron under --smoke), in float and double. A host build runs the host cases (builds,
// refits, signed distances); a CUDA or HIP build runs the signed distances as host/device pairs. See
// ../README.md.

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// Branching factor, and the leaf size in triangle groups, spelled out (see ../PointCloud/main.cpp).
constexpr std::size_t K             = 4;
constexpr std::size_t MaxLeafGroups = 4;

// Signed distance to an SDF: one query per point, on either side.
template <class SDF, class T>
struct Distance
{
  SDF m_sdf;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const EBGeometry::Vec3T<T>& a_point) const noexcept
  {
    return m_sdf.signedDistance(a_point);
  }
};

template <class T>
double
sumOf(const std::vector<T>& a_values)
{
  double s = 0.0;

  for (const T v : a_values) {
    s += double(v);
  }

  return s;
}

// Random points in the mesh's bounding box grown to 1.5 times its size, so that queries fall inside,
// near and away from the surface.
template <class T>
std::vector<EBGeometry::Vec3T<T>>
queryPoints(const EBGeometry::BoundingVolumes::AABBT<T>& a_box, std::size_t a_n)
{
  const auto c = T(0.5) * (a_box.getLowCorner() + a_box.getHighCorner());
  const auto h = T(0.75) * (a_box.getHighCorner() - a_box.getLowCorner());

  return Perf::pointsInBox<T>(EBGeometry::BoundingVolumes::AABBT<T>(c - h, c + h), a_n, 11u);
}

// The query cases shared by both SDF types: on the host and, where a device is visible, on it.
template <class T, class SDF>
void
addQueryCases(Perf::Suite&                                              a_suite,
              const std::string&                                        a_prefix,
              std::function<SDF(EBGeometry::Pool&, const std::string&)> a_make,
              std::function<void(Perf::Case&)>                          a_params,
              bool                                                      a_device)
{
  const std::string prec = std::is_same_v<T, float> ? "float" : "double";

  a_suite.add(a_prefix + "query/" + prec + "/host", [=](Perf::Case& c) {
    EBGeometry::Pool       pool(EBGeometry::hostMemoryResource());
    const SDF              sdf = a_make(pool, Perf::meshFile(c.smoke()));
    const auto             q   = queryPoints<T>(sdf.computeBoundingVolume(), c.size(100000, 1000));
    std::vector<T>         d(q.size());
    const Distance<SDF, T> query{sdf};

    c.measure(
      [&] {
        for (std::size_t i = 0; i < q.size(); i++) {
          d[i] = query(q[i]);
        }
      },
      q.size());

    c.checksum(sumOf(d));
    c.poolBytes(pool.usedBytes());
    a_params(c);
  });

  // A width wider than 4 is host-only: no pair for it.
  if (!a_device) {
    return;
  }

  // The same points through the same query on both sides, in Morton order (as a caller querying a box
  // of cells presents them) and in random order: the speed of each side, and whether they agree.
  for (const std::string order : {"morton", "random"}) {
    a_suite.add(a_prefix + "query-" + order + "/" + prec + "/hostdevice", [=](Perf::Case& c) {
      EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
      const SDF        sdf    = a_make(pool, Perf::meshFile(c.smoke()));
      const auto       random = queryPoints<T>(sdf.computeBoundingVolume(), c.size(262144, 1000));
      const auto       q      = order == "morton" ? Perf::mortonOrdered(random) : random;

      // A disagreement: a different sign, or a difference beyond a thousand ulps of the distance.
      const auto agree = [](const T a_host, const T a_device) {
        const double diff = std::abs(double(a_host) - double(a_device));
        const double tol =
          1000.0 * double(EBGeometry::Math::Limits<T>::epsilon()) * std::max(1.0, std::abs(double(a_host)));

        return std::make_pair(diff, (a_host < T(0)) != (a_device < T(0)) || diff > tol);
      };

      const auto d = Perf::measureHostAndDevice<T>(
        c, pool, q, [&](const EBGeometry::Pool& a_pool) { return Distance<SDF, T>{sdf.rebasedView(a_pool)}; }, agree);

      c.checksum(sumOf(d));
      a_params(c);
    });
  }
}

template <class T, std::size_t W>
void
addTriMeshSDF(Perf::Suite& a_suite, bool a_device)
{
  using SDF = EBGeometry::TriMeshSDF<T, K, W>;

  const std::string prec   = std::is_same_v<T, float> ? "float" : "double";
  const std::string prefix = "TriMeshSDF/w" + std::to_string(W) + "/";

  const auto params = [](Perf::Case& c) {
    c.param("K", K);
    c.param("W", W);
    c.param("maxLeafGroups", MaxLeafGroups);
  };

  a_suite.add(prefix + "build/" + prec + "/host", [=](Perf::Case& c) {
    const auto triangles = EBGeometry::Parser::readIntoTriangles<T>(Perf::meshFile(c.smoke()));

    std::unique_ptr<EBGeometry::Pool> pool;
    std::unique_ptr<SDF>              sdf;

    c.measureFresh(
      [&] {
        sdf.reset();
        pool = std::make_unique<EBGeometry::Pool>(EBGeometry::hostMemoryResource());
      },
      [&] { sdf = std::make_unique<SDF>(triangles, *pool, EBGeometry::BVH::Construction::SAH, MaxLeafGroups); });

    c.poolBytes(pool->usedBytes());
    c.checksum(double(sdf->signedDistance(EBGeometry::Vec3T<T>(T(0), T(0), T(0)))));
    c.param("triangles", triangles.size());
    params(c);
  });

  const std::function<SDF(EBGeometry::Pool&, const std::string&)> make = [](EBGeometry::Pool&  a_pool,
                                                                            const std::string& a_file) {
    return SDF(
      EBGeometry::Parser::readIntoTriangles<T>(a_file), a_pool, EBGeometry::BVH::Construction::SAH, MaxLeafGroups);
  };

  // A width wider than 4 is host-only (see ConfigurationOptions.rst): no device case for it.
  addQueryCases<T, SDF>(a_suite, prefix, make, params, a_device);
}

template <class T>
void
addMeshSDF(Perf::Suite& a_suite)
{
  using SDF  = EBGeometry::MeshSDF<T, K>;
  using Mesh = EBGeometry::DCEL::MeshT<T>;

  const std::string prec   = std::is_same_v<T, float> ? "float" : "double";
  const std::string prefix = "MeshSDF/";

  const auto params = [](Perf::Case& c) { c.param("K", K); };

  a_suite.add(prefix + "build/" + prec + "/host", [=](Perf::Case& c) {
    std::unique_ptr<EBGeometry::Pool> pool;
    std::unique_ptr<Mesh>             mesh;
    std::unique_ptr<SDF>              sdf;

    c.measureFresh(
      [&] {
        sdf.reset();
        mesh.reset();
        pool = std::make_unique<EBGeometry::Pool>(EBGeometry::hostMemoryResource());
        mesh = std::make_unique<Mesh>(EBGeometry::Parser::readIntoDCEL<T>(Perf::meshFile(c.smoke()), *pool));
      },
      [&] { sdf = std::make_unique<SDF>(*mesh, *pool, EBGeometry::BVH::Construction::SAH); });

    c.poolBytes(pool->usedBytes());
    c.checksum(double(sdf->signedDistance(EBGeometry::Vec3T<T>(T(0), T(0), T(0)))));
    params(c);
  });

  a_suite.add(prefix + "refit/" + prec + "/host", [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
    const SDF        sdf  = EBGeometry::Parser::readIntoMeshSDF<T, K>(Perf::meshFile(c.smoke()), pool);
    const auto       mesh = sdf.getMesh();

    // A single refit takes a few milliseconds, too short to time steadily: each repetition does several.
    constexpr std::size_t refits = 10;

    c.measure([&] {
      for (std::size_t r = 0; r < refits; r++) {
        sdf.getRoot().refit([&mesh](uint32_t a_face) {
          return EBGeometry::BoundingVolumes::AABBT<T>(mesh.getFace(a_face).getAllVertexCoordinates(mesh));
        });
      }
    });

    c.checksum(double(sdf.computeBoundingVolume().getHighCorner()[0]));
    c.param("refits", refits);
    params(c);
  });

  const std::function<SDF(EBGeometry::Pool&, const std::string&)> make = [](EBGeometry::Pool&  a_pool,
                                                                            const std::string& a_file) {
    return EBGeometry::Parser::readIntoMeshSDF<T, K>(a_file, a_pool);
  };

  addQueryCases<T, SDF>(a_suite, prefix, make, params, true);
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "Mesh");

  addTriMeshSDF<float, 4>(suite, true);
  addTriMeshSDF<double, 4>(suite, true);
  addTriMeshSDF<float, 8>(suite, false);
  addMeshSDF<float>(suite);
  addMeshSDF<double>(suite);

  return suite.run();
}
