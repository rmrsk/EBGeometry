// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <type_traits>
#include <vector>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using namespace EBGeometry;

using T    = EBGEOMETRY_PRECISION;
using Vec3 = Vec3T<T>;
using AABB = BoundingVolumes::AABBT<T>;

namespace {

// The union of the sphere, torus and capsule below, as a struct rather than a lambda: a function
// handed to a GPU kernel must be trivially copyable and EBGEOMETRY_HOST_DEVICE, which a lambda that
// captures by reference is not. On the host either works.
struct ShapeUnion
{
  SphereSDF<T>  sphere;
  TorusSDF<T>   torus;
  CapsuleSDF<T> capsule;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3& a_point) const noexcept
  {
    return Math::min(Math::min(sphere.signedDistance(a_point), torus.signedDistance(a_point)),
                     capsule.signedDistance(a_point));
  }
};

// Both estimated boxes.
struct Boxes
{
  AABB cone;
  AABB shapes;
};

// A query for EBGeometry::GPU::transform(): a trivially copyable functor whose EBGEOMETRY_HOST_DEVICE
// call operator maps one input (here an octree depth) to one result. approximateBoundingVolumeOctree()
// keeps its own fixed stack and allocates nothing, so a whole estimate runs in one thread.
struct EstimateBoxes
{
  ConeSDF<T> cone;
  ShapeUnion shapes;
  Vec3       lo;
  Vec3       hi;

  EBGEOMETRY_HOST_DEVICE
  Boxes
  operator()(const unsigned int a_depth) const noexcept
  {
    return {approximateBoundingVolumeOctree(cone, lo, hi, a_depth, T(0)),
            approximateBoundingVolumeOctree(shapes, lo, hi, a_depth, T(0))};
  }
};

} // namespace

int
main()
{
  // Numerically approximate a tight bounding box around a shape: start from a deliberately loose
  // box, then recursively subdivide into octree cells (each cell splits into 8 children) and
  // discard the ones that don't touch the shape, down to a maximum of 8 levels of refinement. The
  // last argument (0.0) is a safety margin on the intersection test: 0 means exact, larger values
  // pad the result to guard against a coarse initial box missing part of the shape.
  const Vec3         initLo   = -10 * Vec3::ones();
  const Vec3         initHi   = +10 * Vec3::ones();
  const unsigned int maxDepth = 8;

  // Any shape can be passed directly: here a finite cone with its tip at the origin, height 2 and a
  // 60 degree opening angle.
  const ConeSDF<T> cone(Vec3::zeros(), T(2.0), T(60.0));

  const auto coneBV = approximateBoundingVolumeOctree(cone, initLo, initHi, maxDepth, T(0.0));

  std::cout << "Approximate bounding volume of the cone = " << coneBV << '\n';

  // So can anything else that returns a distance for a point, such as a lambda. This one takes the
  // union (the pointwise minimum) of a sphere, a torus around it and a capsule poking out of it,
  // whose combined extent is no longer obvious from any one formula.
  const SphereSDF<T>  sphere(Vec3::zeros(), T(1.0));
  const TorusSDF<T>   torus(Vec3::zeros(), T(2.0), T(0.25));
  const CapsuleSDF<T> capsule(Vec3::zeros(), Vec3(T(1.0), T(2.0), T(3.0)), T(0.25));

  const auto shapeUnion = [&](const Vec3& a_point) -> T {
    return std::min({sphere.signedDistance(a_point), torus.signedDistance(a_point), capsule.signedDistance(a_point)});
  };

  const auto unionBV = approximateBoundingVolumeOctree(shapeUnion, initLo, initHi, maxDepth, T(0.0));

  std::cout << "Approximate bounding volume of the union = " << unionBV << '\n';

  // Check both against the exact boxes, worked out from the shapes. Every leaf cell the surface
  // passes through is kept, so the estimate holds the exact box, and it can overshoot it by at most
  // one leaf cell's diagonal on each side.
  const T cellDiagonal = std::sqrt(T(3)) * (initHi[0] - initLo[0]) / T(1U << maxDepth);
  const T tolerance    = std::is_same_v<T, float> ? T(1.0e-5) : T(1.0e-12);

  const auto encloses = [&](const BoundingVolumes::AABBT<T>& a_estimate, const Vec3& a_lo, const Vec3& a_hi) {
    bool good = true;

    for (int dir = 0; dir < 3; dir++) {
      const T loGap = a_lo[dir] - a_estimate.getLowCorner()[dir];
      const T hiGap = a_estimate.getHighCorner()[dir] - a_hi[dir];

      good = good && (loGap >= -tolerance) && (loGap <= cellDiagonal);
      good = good && (hiGap >= -tolerance) && (hiGap <= cellDiagonal);
    }

    return good;
  };

  // The analytic shapes know their exact boxes. The cone opens along -y from its tip, with base
  // radius height * tan(half the angle); the union's box is the box around the three shapes' boxes.
  const auto exactCone  = cone.computeBoundingVolume();
  const auto exactUnion = BoundingVolumes::AABBT<T>(std::vector<BoundingVolumes::AABBT<T>>{
    sphere.computeBoundingVolume(), torus.computeBoundingVolume(), capsule.computeBoundingVolume()});

  const bool coneGood  = encloses(coneBV, exactCone.getLowCorner(), exactCone.getHighCorner());
  const bool unionGood = encloses(unionBV, exactUnion.getLowCorner(), exactUnion.getHighCorner());

  std::cout << "Exact bounding volume of the cone = " << exactCone << '\n';
  std::cout << "Exact bounding volume of the union = " << exactUnion << '\n';
  std::cout << "Both boxes hold the exact ones, within one leaf cell: " << ((coneGood && unionGood) ? "yes" : "NO")
            << '\n';

  // ── The same estimates on a GPU ────────────────────────────────────────────────────────────────────
  //
  // Compiled for a GPU (GPU=HIP or GPU=CUDA), EBGeometry::GPU::transform() runs both estimates in a
  // kernel, one octree depth per thread, and each box is compared with the host's at the same depth.
  // The shapes are plain values, so nothing needs copying to the device first.
  bool deviceGood = true;

  if (!GPU::enabled) {
    std::cout << "On the device: not built (plain C++); compile with GPU=HIP or GPU=CUDA to run it\n";
  }
  else if (!GPU::deviceAvailable()) {
    std::cout << "On the device: skipped, no GPU is visible\n";
  }
  else {
    const EstimateBoxes estimate{cone, ShapeUnion{sphere, torus, capsule}, initLo, initHi};

    std::vector<unsigned int> depths;

    for (unsigned int depth = 1; depth <= maxDepth; depth++) {
      depths.push_back(depth);
    }

    const std::vector<Boxes> onDevice = GPU::transform(estimate, depths);

    const auto same = [&](const AABB& a_device, const AABB& a_host) {
      const T scale = T(1) + (initHi - initLo).length();

      return (a_device.getLowCorner() - a_host.getLowCorner()).length() <= tolerance * scale &&
             (a_device.getHighCorner() - a_host.getHighCorner()).length() <= tolerance * scale;
    };

    int differ = 0;

    for (std::size_t i = 0; i < depths.size(); i++) {
      const Boxes onHost = estimate(depths[i]);

      differ += same(onDevice[i].cone, onHost.cone) ? 0 : 1;
      differ += same(onDevice[i].shapes, onHost.shapes) ? 0 : 1;
    }

    std::cout << "On the device (" << GPU::backendName << "): both estimates at depths 1 to " << maxDepth << ", "
              << differ << " boxes differ from the host\n";

    deviceGood = (differ == 0);
  }

  return (coneGood && unionGood && deviceGood) ? 0 : 1;
}
