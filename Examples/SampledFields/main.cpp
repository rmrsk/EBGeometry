// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

// Distance fields given as samples on a grid (from CT or MRI, an image stack, or another code's level set), and a
// terrain given as heights: both as a SampledGridSDF, a shape held in a pool, queried on the host and on a GPU. At
// the end it opens a window on both, if built with the viewer. See README.md.

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T    = EBGEOMETRY_PRECISION;
using Vec3 = EBGeometry::Vec3T<T>;
using Grid = EBGeometry::SampledGridSDF<T>;

namespace {

// ── The code that runs on either side ────────────────────────────────────────────────────────────────
//
// A query for EBGeometry::GPU::transform(): a trivially copyable functor whose EBGEOMETRY_HOST_DEVICE call
// operator maps one point to one result. The grid it holds is a descriptor of samples in a Pool, so a kernel can
// receive it by value once it describes a pool the device can read.
struct Distance
{
  Grid m_grid;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3& a_point) const noexcept
  {
    return m_grid.signedDistance(a_point);
  }
};

} // namespace

int
main(int argc, char* argv[])
{
  // The samples live in the pool; a grid is a descriptor into it, so the pool must outlive every grid.
  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  // ── A field sampled on a grid ──────────────────────────────────────────────────────────────────────
  //
  // Here the samples stand for a scan: the distance to a peanut (two balls blended) at the nodes of a 48^3 grid
  // over [-1.2, 1.2]^3, x fastest, then y, then z, negative inside. Your own come from a file or another code.
  const std::uint32_t n       = 48;
  const Vec3          origin  = T(-1.2) * Vec3::ones();
  const T             spacing = T(2.4) / T(n - 1);

  std::vector<T> samples;

  for (std::uint32_t k = 0; k < n; k++) {
    for (std::uint32_t j = 0; j < n; j++) {
      for (std::uint32_t i = 0; i < n; i++) {
        const Vec3 p  = origin + spacing * Vec3(T(i), T(j), T(k));
        const T    d1 = (p - Vec3(T(-0.4), T(0), T(0))).length() - T(0.6);
        const T    d2 = (p - Vec3(T(0.45), T(0.1), T(0))).length() - T(0.5);
        const T    h  = std::max(T(0.25) - std::abs(d1 - d2), T(0)) / T(0.25);

        samples.push_back(std::min(d1, d2) - T(0.0625) * h * h);
      }
    }
  }

  // The grid interpolates the samples trilinearly and divides them by their largest slope, so its value never
  // exceeds the distance to its surface: a bound, safe for sphere tracing and for pruning in a BVH.
  const Grid peanut(pool, origin, spacing * Vec3::ones(), n, n, n, samples);

  // ── A height field ─────────────────────────────────────────────────────────────────────────────────
  //
  // Heights on a 65 x 65 grid in the xz-plane, x fastest, rolling hills. The solid is between a floor (the corner's
  // y) and the surface: a block of terrain, its sides cut by the grid's box.
  const std::uint32_t m  = 65;
  const T             dx = T(3) / T(m - 1);

  std::vector<T> heights;

  for (std::uint32_t k = 0; k < m; k++) {
    for (std::uint32_t i = 0; i < m; i++) {
      const T x = dx * T(i);
      const T z = dx * T(k);

      heights.push_back(T(0.3) + T(0.15) * std::sin(T(3) * x) * std::cos(T(2) * z) + T(0.1) * x);
    }
  }

  const Grid terrain = Grid::heightField(pool, Vec3(T(0), T(-0.5), T(0)), dx, dx, m, m, heights);

  // ── Use them on the host ───────────────────────────────────────────────────────────────────────────
  //
  // Both are shapes: signedDistance(), computeBoundingVolume() and distanceQuality, like any other.
  const std::vector<Vec3> points{Vec3(T(-0.4), T(0), T(0)), Vec3(T(0), T(1), T(0)), Vec3(T(2), T(0), T(0))};

  for (const Vec3& p : points) {
    std::cout << "peanut at " << p << ": " << peanut.signedDistance(p) << '\n';
  }

  std::cout << "terrain at (1.5, 1, 1.5): " << terrain.signedDistance(Vec3(T(1.5), T(1), T(1.5)))
            << ", at (1.5, 0, 1.5): " << terrain.signedDistance(Vec3(T(1.5), T(0), T(1.5))) << '\n';
  std::cout << "peanut's box: " << peanut.computeBoundingVolume().getLowCorner() << " to "
            << peanut.computeBoundingVolume().getHighCorner()
            << ", a bound: " << (Grid::distanceQuality == EBGeometry::DistanceQuality::Bound) << '\n';

  // A tape holds a grid like any shape (it must live in the tape's pool): the two side by side, for the window.
  const EBGeometry::Tape<T> scene(
    pool, EBGeometry::Union(peanut, EBGeometry::Translate(terrain, Vec3(T(1.5), T(-0.7), T(-1.5)))));

  // ── The same query on a GPU ────────────────────────────────────────────────────────────────────────
  //
  // freeze() the pool, mirror it into GPU::memoryResource() (device memory in a CUDA or HIP build), and give the
  // kernel a view of the grid on the mirror. In a plain C++ build these lines run as a host loop.
  pool.freeze();

  if (EBGeometry::GPU::enabled && !EBGeometry::GPU::deviceAvailable()) {
    std::cout << "\nOn the device: skipped, no GPU is visible\n";
  }
  else {
    const EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(pool, EBGeometry::GPU::memoryResource());

    std::vector<T> onDevice(points.size());

    EBGeometry::GPU::transform(Distance{peanut.rebasedView(devicePool)}, points, onDevice);

    std::cout << "\nThe peanut through GPU::transform (backend: " << EBGeometry::GPU::backendName << "):\n";

    for (const T d : onDevice) {
      std::cout << "  " << d << '\n';
    }
  }

  // ── Look at them ───────────────────────────────────────────────────────────────────────────────────
  //
  // Render::view() opens a window (it needs SDL2: make VIEWER=SDL2). Given the pool, it renders on the device in a
  // GPU build. --no-view, or no display, skips it.
  EBGeometry::Render::view(scene, pool, argc, argv);

  return 0;
}
