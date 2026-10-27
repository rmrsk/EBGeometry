// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Packed spheres: spheres of random sizes at random places in a cylinder, joined by a smooth union so that
// neighbours that touch merge with a fillet. A BVHSmoothUnion evaluates it through a BVH over the spheres'
// boxes and blends the two nearest, so a point costs a few spheres, not all of them. See README.md.

#include <iomanip>
#include <iostream>
#include <random>
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

// The BVH branching factor, spelled out, and the smooth minimum that blends the two nearest spheres.
constexpr std::size_t K = 4;

using Packing = EBGeometry::BVHSmoothUnion<T, Sphere, K, EBGeometry::SmoothMinOp<T>>;

// numSpheres spheres, centred in a cylinder along x of the given radius and five times as long, with radii
// between minRadius and maxRadius; the union blends over blendLength.
constexpr int numSpheres     = 3000;
constexpr T   cylinderRadius = T(26);
constexpr T   minRadius      = T(0.6);
constexpr T   maxRadius      = T(3);
constexpr T   blendLength    = T(1.5);

int
main(int argc, char* argv[])
{
  // ── Build the packing ──────────────────────────────────────────────────────────────────────────────
  //
  // Rejection sampling: draw a centre uniformly in the cylinder's box and keep it only if it lies inside
  // the cylinder, then draw the radius. A fixed seed gives the same packing on every run. The union copies
  // the spheres into the pool, which must outlive it.
  std::mt19937_64                   rng(0);
  std::uniform_real_distribution<T> unit(T(-1), T(1));

  std::vector<Sphere> spheres;
  std::vector<AABB>   boxes;

  while (spheres.size() < numSpheres) {
    const Vec3 center = cylinderRadius * Vec3(T(2.5) * unit(rng), unit(rng), unit(rng));

    if (center[1] * center[1] + center[2] * center[2] <= cylinderRadius * cylinderRadius) {
      const T radius = minRadius + T(0.5) * (unit(rng) + T(1)) * (maxRadius - minRadius);

      spheres.emplace_back(center, radius);
      boxes.push_back(spheres.back().computeBoundingVolume());
    }
  }

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const Packing packing(pool, spheres, boxes, blendLength);

  // ── Use it on the host ─────────────────────────────────────────────────────────────────────────────
  //
  // signedDistance() is negative inside any sphere or fillet; the corner of the box is outside them all.
  std::cout << std::fixed << std::setprecision(4) << spheres.size() << " spheres, box "
            << packing.computeBoundingVolume() << "\n  value at the centre " << packing.signedDistance(Vec3::zeros())
            << ", at the corner " << packing.signedDistance(packing.computeBoundingVolume().getLowCorner()) << '\n';

  // ── Look at it ─────────────────────────────────────────────────────────────────────────────────────
  //
  // A window on the packing (built with VIEWER=SDL2; otherwise, with --no-view or without a display,
  // view() prints why and returns). X cuts it open to show the spheres inside.
  EBGeometry::Render::view(packing, argc, argv);

  return 0;
}
