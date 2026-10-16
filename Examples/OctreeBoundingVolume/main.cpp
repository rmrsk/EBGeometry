// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Estimating a tight bounding box for anything that gives a distance at a point, by octree refinement:
// first for one analytic shape, then for a union of three written as a small functor. The estimate
// needs no memory of its own, so the last section runs it in a GPU kernel too. See README.md.

#include <cstddef>
#include <iostream>
#include <vector>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T    = EBGEOMETRY_PRECISION;
using Vec3 = EBGeometry::Vec3T<T>;
using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

namespace {

// ── The code that runs on either side ────────────────────────────────────────────────────────────────
//
// The union (the pointwise minimum) of a sphere, a torus around it and a capsule poking out of it,
// whose combined extent is not obvious from any one formula. It is a struct rather than a lambda: a
// function that reaches a GPU kernel must be trivially copyable with an EBGEOMETRY_HOST_DEVICE call
// operator, and the same struct serves the host.
struct ShapeUnion
{
  EBGeometry::SphereSDF<T>  sphere;
  EBGeometry::TorusSDF<T>   torus;
  EBGeometry::CapsuleSDF<T> capsule;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3& a_point) const noexcept
  {
    return EBGeometry::Math::min(EBGeometry::Math::min(sphere.signedDistance(a_point), torus.signedDistance(a_point)),
                                 capsule.signedDistance(a_point));
  }
};

// A query for EBGeometry::GPU::transform(): it maps one octree depth to the union's estimated box at
// that depth. approximateBoundingVolumeOctree() keeps a fixed stack and allocates nothing, so a whole
// estimate runs in one thread.
struct EstimateBox
{
  ShapeUnion shapes;
  Vec3       lo;
  Vec3       hi;

  EBGEOMETRY_HOST_DEVICE
  AABB
  operator()(const unsigned int a_depth) const noexcept
  {
    return EBGeometry::approximateBoundingVolumeOctree(shapes, lo, hi, a_depth, T(0));
  }
};

} // namespace

int
main()
{
  // ── Build the shapes ───────────────────────────────────────────────────────────────────────────────

  // A finite cone with its tip at the origin, height 2 and a 60 degree opening angle.
  const EBGeometry::ConeSDF<T> cone(Vec3::zeros(), T(2), T(60));

  const ShapeUnion shapes{EBGeometry::SphereSDF<T>(Vec3::zeros(), T(1)),
                          EBGeometry::TorusSDF<T>(Vec3::zeros(), T(2), T(0.25)),
                          EBGeometry::CapsuleSDF<T>(Vec3::zeros(), Vec3(T(1), T(2), T(3)), T(0.25))};

  // ── Use it on the host ─────────────────────────────────────────────────────────────────────────────
  //
  // approximateBoundingVolumeOctree(f, lo, hi, depth, margin) starts from the box [lo, hi], which must
  // hold the surface, splits it into eight cells, keeps the cells the surface can pass through, and
  // repeats down to the given depth. The result is the box around the kept cells, so it holds the
  // surface and overshoots it by at most one leaf cell. The margin pads the intersection test; 0 means
  // none. f can be any shape, or anything else that returns a distance for a point.
  const Vec3         initLo   = -10 * Vec3::ones();
  const Vec3         initHi   = +10 * Vec3::ones();
  const unsigned int maxDepth = 8;

  const AABB coneBox  = EBGeometry::approximateBoundingVolumeOctree(cone, initLo, initHi, maxDepth, T(0));
  const AABB unionBox = EBGeometry::approximateBoundingVolumeOctree(shapes, initLo, initHi, maxDepth, T(0));

  // The analytic shapes also know their exact boxes; for the union, the box around the three.
  const AABB exactUnion(std::vector<AABB>{shapes.sphere.computeBoundingVolume(),
                                          shapes.torus.computeBoundingVolume(),
                                          shapes.capsule.computeBoundingVolume()});

  std::cout << "Leaf cell size after " << maxDepth << " levels: " << (initHi[0] - initLo[0]) / T(1U << maxDepth)
            << "\n\n";
  std::cout << "Cone,  estimated: " << coneBox << '\n';
  std::cout << "Cone,  exact    : " << cone.computeBoundingVolume() << '\n';
  std::cout << "Union, estimated: " << unionBox << '\n';
  std::cout << "Union, exact    : " << exactUnion << '\n';

  // ── The same query on a GPU ────────────────────────────────────────────────────────────────────────
  //
  // GPU::transform(query, depths) runs one estimate per depth: in a kernel when this file is compiled as
  // CUDA or HIP, and in a host loop otherwise. The shapes are plain values inside the query, so nothing
  // needs copying to the device first. Deeper estimates are tighter.
  if (EBGeometry::GPU::enabled && !EBGeometry::GPU::deviceAvailable()) {
    std::cout << "\nOn the device: skipped, no GPU is visible\n";
  }
  else {
    std::vector<unsigned int> depths;

    for (unsigned int depth = 1; depth <= maxDepth; depth++) {
      depths.push_back(depth);
    }

    std::vector<AABB> boxes(depths.size());

    EBGeometry::GPU::transform(EstimateBox{shapes, initLo, initHi}, depths, boxes);

    std::cout << "\nThe union's estimate by depth, through GPU::transform (backend: " << EBGeometry::GPU::backendName
              << "):\n";

    for (std::size_t i = 0; i < depths.size(); i++) {
      std::cout << "  depth " << depths[i] << ": " << boxes[i] << '\n';
    }
  }

  return 0;
}
