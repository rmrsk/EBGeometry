// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// A scene of many objects: thousands of spheres on a lattice, and a "city" of randomly sized boxes.
// Each is put in a BVHUnion, which evaluates the union through a bounding volume hierarchy over the
// objects' boxes, and timed against a naive union that visits every object. The last section runs the
// unions on a GPU, timing the kernel alone. See README.md.

#include <chrono>
#include <cstddef>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T      = EBGEOMETRY_PRECISION;
using Vec3   = EBGeometry::Vec3T<T>;
using AABB   = EBGeometry::BoundingVolumes::AABBT<T>;
using Sphere = EBGeometry::SphereSDF<T>;
using Box    = EBGeometry::BoxSDF<T>;

// A BVHUnion holds one primitive type; K is the branching factor of its BVH.
constexpr std::size_t K = EBGeometry::BVH::DefaultBranchingRatio<T>();

using Spheres = EBGeometry::BVHUnion<T, Sphere, K>;
using City    = EBGeometry::BVHUnion<T, Box, K>;

// Run configuration: spheresPerAxis^3 spheres, (lotsPerAxis)^2 buildings, and the number of random
// query points. The naive union visits every object for every point, so these stay small enough for an
// unoptimised build to finish in seconds.
constexpr int         spheresPerAxis = 20;
constexpr int         lotsPerAxis    = 60;
constexpr std::size_t numPoints      = 2000;

// In a GPU build the last section uses many more points, so that the kernel, not the launch, is what
// the timing measures. In a plain C++ build that section runs on the host and keeps numPoints.
constexpr std::size_t numDevicePoints = EBGeometry::GPU::enabled ? std::size_t(1) << 20 : numPoints;

namespace {

// ── The code that runs on either side ────────────────────────────────────────────────────────────────
//
// A query for EBGeometry::GPU::transform(): a trivially copyable functor whose EBGEOMETRY_HOST_DEVICE
// call operator maps one point to one result. A BVHUnion is a descriptor of the objects and BVH it
// stored in a Pool, so the query holds it by value, and a kernel can receive it once it describes a
// pool the device can read.
template <class Union>
struct Distance
{
  Union m_union;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3& a_point) const noexcept
  {
    return m_union.signedDistance(a_point);
  }
};

// Random points in a box. A fixed seed gives the same points on every run.
std::vector<Vec3>
randomPoints(const AABB& a_box, const std::size_t a_count)
{
  std::mt19937_64                   rng(54321);
  std::uniform_real_distribution<T> unit(T(0), T(1));
  std::vector<Vec3>                 points;

  const Vec3 lo    = a_box.getLowCorner();
  const Vec3 delta = a_box.getHighCorner() - lo;

  for (std::size_t i = 0; i < a_count; i++) {
    points.emplace_back(lo + delta * Vec3(unit(rng), unit(rng), unit(rng)));
  }

  return points;
}

// Microseconds since a_start.
T
microsecondsSince(const std::chrono::steady_clock::time_point a_start)
{
  const std::chrono::duration<T, std::micro> elapsed = std::chrono::steady_clock::now() - a_start;

  return elapsed.count();
}

// Time the naive union and the BVH union of the same objects over the same points, on the host.
template <class Primitive, class Union>
void
compare(const std::string&            a_name,
        const std::vector<Primitive>& a_objects,
        const Union&                  a_union,
        const std::vector<Vec3>&      a_points)
{
  // The naive union: the smallest distance over every object.
  const auto naiveUnion = [&a_objects](const Vec3& a_point) {
    T distance = EBGeometry::Math::Limits<T>::max();

    for (const Primitive& object : a_objects) {
      distance = EBGeometry::Math::min(distance, object.signedDistance(a_point));
    }

    return distance;
  };

  std::vector<T> naive(a_points.size());
  std::vector<T> bvh(a_points.size());

  const auto start = std::chrono::steady_clock::now();

  for (std::size_t i = 0; i < a_points.size(); i++) {
    naive[i] = naiveUnion(a_points[i]);
  }

  const T    naiveTime = microsecondsSince(start) / T(a_points.size());
  const auto middle    = std::chrono::steady_clock::now();

  for (std::size_t i = 0; i < a_points.size(); i++) {
    bvh[i] = a_union.signedDistance(a_points[i]);
  }

  const T bvhTime = microsecondsSince(middle) / T(a_points.size());

  std::cout << a_name << ": " << a_objects.size() << " objects, " << a_points.size() << " points\n";
  std::cout << "  Naive union : " << naiveTime << " us per point\n";
  std::cout << "  BVHUnion    : " << bvhTime << " us per point (" << naiveTime / bvhTime << "x faster)\n";
  std::cout << "  At " << a_points[0] << ": naive " << naive[0] << ", BVHUnion " << bvh[0] << "\n\n";
}

// Run a union on every point through GPU::transform, and print how long the kernel took.
template <class Union>
void
runOnDevice(const std::string& a_name, const Union& a_onDevice, const std::vector<Vec3>& a_points)
{
  // The points and room for the results, in a pool of their own, mirrored into the device's memory
  // once. PODVector<T> is an array in a pool: an offset and a size, which resolve against whichever
  // copy of the pool they are given.
  const auto n = static_cast<uint32_t>(a_points.size());

  EBGeometry::Pool            hostPool(EBGeometry::hostMemoryResource());
  EBGeometry::PODVector<Vec3> in;
  EBGeometry::PODVector<T>    out;
  const std::vector<T>        zeros(n, T(0));

  in.reserveFrom(hostPool, n);
  out.reserveFrom(hostPool, n);
  in.assign(hostPool.base(), a_points.data(), n);
  out.assign(hostPool.base(), zeros.data(), n);
  hostPool.freeze();

  const EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(hostPool, EBGeometry::GPU::memoryResource());

  const Distance<Union> query{a_onDevice};

  // Given the pool both arrays live in, transform() checks that the pool is frozen and in memory the
  // device can reach, then reads the points and writes the results where they already are: the time is
  // the kernel's alone. The first launch in a program also pays for loading the kernel onto the device,
  // so one untimed call comes first.
  EBGeometry::GPU::transform(query, devicePool, in, out);

  const auto poolStart = std::chrono::steady_clock::now();

  EBGeometry::GPU::transform(query, devicePool, in, out);

  const T poolTime = microsecondsSince(poolStart);

  // The vector overload is one call from host vector to host vector, which also allocates device memory
  // and copies the points in and the results out. Its time includes all of that.
  std::vector<T> viaVectors(a_points.size());

  const auto vectorStart = std::chrono::steady_clock::now();

  EBGeometry::GPU::transform(query, a_points, viaVectors);

  const T vectorTime = microsecondsSince(vectorStart);

  // Copy the results back to the host to read them.
  const EBGeometry::Pool back = EBGeometry::Pool::mirror(devicePool, EBGeometry::hostMemoryResource());

  std::cout << a_name << " through GPU::transform (backend: " << EBGeometry::GPU::backendName << "), " << n
            << " points:\n";
  std::cout << "  In the pool  : " << T(1000) * poolTime / T(n) << " ns per point (the queries alone)\n";
  std::cout << "  Over vectors : " << T(1000) * vectorTime / T(n)
            << " ns per point (in a GPU build, with allocation and copies)\n";
  std::cout << "  At " << a_points[0] << ": " << out.at(back.base(), 0) << " (pool), " << viaVectors[0]
            << " (vectors)\n\n";
}

} // namespace

int
main(int argc, char* argv[])
{
  // ── Build the scenes ───────────────────────────────────────────────────────────────────────────────
  //
  // A BVHUnion takes the objects and one bounding box per object. It copies the objects into a Pool
  // and builds its BVH over the boxes there, so the vectors can go once it is built; the pool must
  // outlive the union and every copy of it.
  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  // Spheres of radius 1, a radius apart, on a lattice.
  const T radius  = T(1);
  const T spacing = T(3) * radius;

  std::vector<Sphere> spheres;
  std::vector<AABB>   sphereBoxes;

  for (int i = 0; i < spheresPerAxis; i++) {
    for (int j = 0; j < spheresPerAxis; j++) {
      for (int k = 0; k < spheresPerAxis; k++) {
        const Vec3 center = spacing * Vec3(T(i), T(j), T(k));

        spheres.emplace_back(center, radius);
        sphereBoxes.push_back(spheres.back().computeBoundingVolume());
      }
    }
  }

  const Spheres packedSpheres(pool, spheres, sphereBoxes);

  // A city: one building per lot, of random width, length and height, never overlapping its
  // neighbours. Lots are maxWidth + gap wide.
  const T maxWidth  = T(3);
  const T maxHeight = T(20);
  const T gap       = T(0.1);

  std::mt19937_64                   rng(12345);
  std::uniform_real_distribution<T> unit(T(0), T(1));
  std::vector<Box>                  buildings;
  std::vector<AABB>                 buildingBoxes;

  for (int i = 0; i < lotsPerAxis; i++) {
    for (int j = 0; j < lotsPerAxis; j++) {
      const T width  = T(1) + unit(rng) * (maxWidth - T(1));
      const T length = T(1) + unit(rng) * (maxWidth - T(1));
      const T height = T(1) + unit(rng) * (maxHeight - T(1));

      const Vec3 lo(T(i) * (maxWidth + gap), T(j) * (maxWidth + gap), T(0));
      const Vec3 hi = lo + Vec3(width, length, height);

      buildings.emplace_back(lo, hi);
      buildingBoxes.emplace_back(lo, hi);
    }
  }

  const City city(pool, buildings, buildingBoxes);

  // ── Use them on the host ───────────────────────────────────────────────────────────────────────────
  //
  // signedDistance(point) is the union's value, the same as the naive minimum over every object; the
  // BVH only skips objects whose boxes show they cannot be the closest. computeBoundingVolume() is the
  // box around them all, used here to place the query points.
  const std::vector<Vec3> spherePoints = randomPoints(packedSpheres.computeBoundingVolume(), numPoints);
  const std::vector<Vec3> cityPoints   = randomPoints(city.computeBoundingVolume(), numPoints);

  compare("Packed spheres", spheres, packedSpheres, spherePoints);
  compare("Random city", buildings, city, cityPoints);

  // ── The same query on a GPU ────────────────────────────────────────────────────────────────────────
  //
  // The pool holding both unions is frozen, mirrored into GPU::memoryResource() (device memory in a CUDA
  // or HIP build) in one piece, and each union rebased onto the copy; runOnDevice() above then puts the
  // query points on the device as well, so that the timed call does nothing but the queries. In a plain
  // C++ build, memoryResource() is host memory standing in for a device and transform() is a host loop,
  // so these lines also run there.
  if (EBGeometry::GPU::enabled && !EBGeometry::GPU::deviceAvailable()) {
    std::cout << "On the device: skipped, no GPU is visible\n";
  }
  else {
    pool.freeze();

    const EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(pool, EBGeometry::GPU::memoryResource());

    runOnDevice("Packed spheres",
                packedSpheres.rebasedView(devicePool),
                randomPoints(packedSpheres.computeBoundingVolume(), numDevicePoints));
    runOnDevice(
      "Random city", city.rebasedView(devicePool), randomPoints(city.computeBoundingVolume(), numDevicePoints));
  }

  // ── Look at it ─────────────────────────────────────────────────────────────────────────────────────
  //
  // Render::view() opens a window on any shape, a BVH union included -- here the random city. Drag to
  // orbit, scroll to zoom, X cuts it open.
  // The window needs SDL2: build with make VIEWER=SDL2 or cmake -DEBGEOMETRY_VIEWER=SDL2. Otherwise, with
  // --no-view, or without a display, view() prints why and returns. A surface seen from inside the solid
  // is drawn red.
  EBGeometry::Render::view(city, argc, argv);

  return 0;
}
