// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   Inputs.hpp
 * @brief  The probes' inputs: random points from a generator of their own with named seeds, so that
 * every platform and standard library draws the same points; Morton ordering; and the mesh files.
 */

#ifndef EBGEOMETRY_PERF_INPUTS_HPP
#define EBGEOMETRY_PERF_INPUTS_HPP

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

// Where the meshes are: absolute paths, set by the build (Performance/CMakeLists.txt). A program built
// without them reports its mesh cases as skipped, with the reason.
#ifndef EBGEOMETRY_PERF_MESH_DIR
#define EBGEOMETRY_PERF_MESH_DIR ""
#endif

#ifndef EBGEOMETRY_PERF_SMOKE_MESH_DIR
#define EBGEOMETRY_PERF_SMOKE_MESH_DIR ""
#endif

namespace Perf {

/**
 * @brief The seeds of the probes' random inputs, one per input, so that no two inputs share a stream
 * and no seed is a magic number in a probe. Changing one changes that input and its cases' checksums.
 */
namespace Seed {

constexpr std::uint64_t Cloud        = 20261019; ///< PointCloud: the cloud's points.
constexpr std::uint64_t NearQueries  = 7;        ///< PointCloud: queries near the cloud.
constexpr std::uint64_t ShellRadii   = 8;        ///< PointCloud: radii of the queries near the sphere.
constexpr std::uint64_t FarQueries   = 9;        ///< PointCloud: queries far outside the cloud.
constexpr std::uint64_t MeshQueries  = 11;       ///< Mesh: queries around the mesh.
constexpr std::uint64_t UnionQueries = 13;       ///< BVHUnion: queries in the union's box.
constexpr std::uint64_t CylinderAxes = 21;       ///< BVHUnion: the cylinders' directions.
constexpr std::uint64_t BuildChecks  = 23;       ///< Every probe: where a build case checks what it built.
constexpr std::uint64_t TapeQueries  = 29;       ///< Tape: queries around each object.
constexpr std::uint64_t RayOrigins   = 31;       ///< BVH, Ray, RayCSG: where the rays and point queries start.
constexpr std::uint64_t RayScene     = 37;       ///< RayCSG: the scene's solids (and 38, their sizes).
constexpr std::uint64_t RayCrowd     = 41;       ///< RayCSG: the crowd's balls (and 42, their radii).
constexpr std::uint64_t Particles    = 43;       ///< RayParticles: where the particles start.
constexpr std::uint64_t ParticleDirs = 47;       ///< RayParticles: the directions they move in.

} // namespace Seed

/**
 * @brief Points uniformly at random in a box, from a fixed seed: a xorshift64* generator of the
 * harness's own, so that the points do not depend on the standard library.
 * @tparam T Precision.
 * @param[in] a_box  The box.
 * @param[in] a_n    Number of points.
 * @param[in] a_seed Seed (Perf::Seed).
 * @return The points.
 */
template <class T>
std::vector<EBGeometry::Vec3T<T>>
pointsInBox(const EBGeometry::BoundingVolumes::AABBT<T>& a_box, std::size_t a_n, std::uint64_t a_seed)
{
  std::uint64_t state = a_seed * 6364136223846793005ULL + 1442695040888963407ULL;

  const auto next = [&state]() {
    state ^= state >> 12;
    state ^= state << 25;
    state ^= state >> 27;

    return static_cast<double>((state * 2685821657736338717ULL) >> 11) * (1.0 / 9007199254740992.0);
  };

  const auto lo = a_box.getLowCorner();
  const auto d  = a_box.getHighCorner() - lo;

  std::vector<EBGeometry::Vec3T<T>> points(a_n);

  for (auto& p : points) {
    const T x = T(next());
    const T y = T(next());
    const T z = T(next());

    p = lo + EBGeometry::Vec3T<T>(x * d[0], y * d[1], z * d[2]);
  }

  return points;
}

/**
 * @brief The box [a_lo, a_hi] in every direction.
 * @tparam T Precision.
 * @param[in] a_lo Lower corner coordinate.
 * @param[in] a_hi Upper corner coordinate.
 * @return The box.
 */
template <class T>
EBGeometry::BoundingVolumes::AABBT<T>
cube(T a_lo, T a_hi)
{
  return EBGeometry::BoundingVolumes::AABBT<T>(EBGeometry::Vec3T<T>(a_lo, a_lo, a_lo),
                                               EBGeometry::Vec3T<T>(a_hi, a_hi, a_hi));
}

/**
 * @brief The order that sorts points along a Morton curve.
 * @tparam T Precision.
 * @param[in] a_points The points.
 * @return Their indices, in Morton order.
 */
template <class T>
std::vector<std::uint32_t>
mortonOrder(const std::vector<EBGeometry::Vec3T<T>>& a_points)
{
  return EBGeometry::SFC::order<EBGeometry::SFC::Morton>(a_points);
}

/**
 * @brief Points reordered along a Morton curve: the order a caller querying a grid or a box of cells
 * presents them in, which keeps neighbouring queries (a GPU warp, consecutive host iterations) in the
 * same part of a tree.
 * @tparam T Precision.
 * @param[in] a_points The points.
 * @return The points in Morton order.
 */
template <class T>
std::vector<EBGeometry::Vec3T<T>>
mortonOrdered(const std::vector<EBGeometry::Vec3T<T>>& a_points)
{
  const std::vector<std::uint32_t>  order = mortonOrder(a_points);
  std::vector<EBGeometry::Vec3T<T>> sorted(a_points.size());

  for (std::size_t i = 0; i < order.size(); i++) {
    sorted[i] = a_points[order[i]];
  }

  return sorted;
}

/**
 * @brief The order in which a query's inputs are presented.
 */
enum class Order
{
  Morton, ///< Along a Morton curve, as a caller querying a box of cells presents them.
  Random  ///< As drawn: neighbouring queries anywhere in the tree.
};

/**
 * @brief Both orders, for the cases that measure each.
 */
constexpr Order Orders[] = {Order::Morton, Order::Random};

/**
 * @brief An order's name in a case name.
 * @param[in] a_order The order.
 * @return "morton" or "random".
 */
inline const char*
orderName(Order a_order) noexcept
{
  return a_order == Order::Morton ? "morton" : "random";
}

/**
 * @brief Random points, presented in an order.
 * @tparam T Precision.
 * @param[in] a_order  The order.
 * @param[in] a_points The points, as drawn.
 * @return The points in that order.
 */
template <class T>
std::vector<EBGeometry::Vec3T<T>>
inOrder(Order a_order, const std::vector<EBGeometry::Vec3T<T>>& a_points)
{
  return a_order == Order::Morton ? mortonOrdered(a_points) : a_points;
}

/**
 * @brief The armadillo mesh from the common-3d-test-models submodule, or the dodecahedron from Tests/data
 * under --smoke.
 * @param[in] a_smoke Whether this is a smoke run.
 * @return Path to the mesh.
 * @throws std::runtime_error if the build did not say where the meshes are (the case is then skipped).
 */
inline std::string
meshFile(bool a_smoke)
{
  const std::string dir = a_smoke ? EBGEOMETRY_PERF_SMOKE_MESH_DIR : EBGEOMETRY_PERF_MESH_DIR;

  if (dir.empty()) {
    throw std::runtime_error("built without EBGEOMETRY_PERF_MESH_DIR; build through Performance/CMakeLists.txt");
  }

  return dir + (a_smoke ? "/dodecahedron.obj" : "/armadillo.obj");
}

} // namespace Perf

#endif
