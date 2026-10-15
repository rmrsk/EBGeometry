// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cmath>
#include <cstddef>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T    = EBGEOMETRY_PRECISION;
using Vec3 = EBGeometry::Vec3T<T>;

namespace {

// The values of every shape at one point.
struct ShapeValues
{
  T value[12];
};

// A query for EBGeometry::GPU::transform(): a trivially copyable functor whose EBGEOMETRY_HOST_DEVICE
// call operator maps one point to one result. The shapes are plain values, so the query holds them by
// value and reaches a kernel with no memory to manage.
struct AllShapes
{
  EBGeometry::PlaneSDF<T>            plane;
  EBGeometry::SphereSDF<T>           sphere;
  EBGeometry::BoxSDF<T>              box;
  EBGeometry::TorusSDF<T>            torus;
  EBGeometry::CylinderSDF<T>         finiteCylinder;
  EBGeometry::InfiniteCylinderSDF<T> infiniteCylinder;
  EBGeometry::CapsuleSDF<T>          capsule;
  EBGeometry::InfiniteConeSDF<T>     infiniteCone;
  EBGeometry::ConeSDF<T>             cone;
  EBGeometry::PerlinSDF<T>           perlin;
  EBGeometry::RoundedBoxSDF<T>       roundBox;
  EBGeometry::RoundedCylinderSDF<T>  roundCylinder;

  EBGEOMETRY_HOST_DEVICE
  ShapeValues
  operator()(const Vec3& a_point) const noexcept
  {
    return {{plane.signedDistance(a_point),
             sphere.signedDistance(a_point),
             box.signedDistance(a_point),
             torus.signedDistance(a_point),
             finiteCylinder.signedDistance(a_point),
             infiniteCylinder.signedDistance(a_point),
             capsule.signedDistance(a_point),
             infiniteCone.signedDistance(a_point),
             cone.signedDistance(a_point),
             perlin.signedDistance(a_point),
             roundBox.signedDistance(a_point),
             roundCylinder.signedDistance(a_point)}};
  }
};

} // namespace

int
main()
{

  // Various analytic shapes. Call ::signedDistance(Vec3) on any of them to evaluate the
  // signed distance at a point (see the class declarations in EBGeometry_AnalyticDistanceFunctions.hpp
  // for the full parameter documentation). Each shape is a plain value type, so it can also be
  // copied into a GPU kernel and evaluated there.

  // A plane through the origin, oriented by its normal vector.
  const EBGeometry::PlaneSDF<T> plane(Vec3::zeros(), Vec3::ones());

  // A sphere: center, radius.
  const EBGeometry::SphereSDF<T> sphere(Vec3::zeros(), 1.0);

  // An axis-aligned box: low corner, high corner.
  const EBGeometry::BoxSDF<T> box(Vec3::zeros(), Vec3::ones());

  // A torus whose ring lies in the xz-plane, around the y axis: center, major radius (center of the
  // tube to the torus' own center), minor radius (radius of the tube itself).
  const EBGeometry::TorusSDF<T> torus(Vec3::zeros(), 1.0, 0.1);

  // A finite (capped) cylinder: center of one end cap, center of the other end cap, radius.
  const EBGeometry::CylinderSDF<T> finiteCylinder(Vec3::zeros(), Vec3::ones(), 0.1);

  // An infinite cylinder: a point on its axis, radius, and which Cartesian axis (0=x, 1=y,
  // 2=z) it runs along -- here, the z-axis.
  const EBGeometry::InfiniteCylinderSDF<T> infiniteCylinder(Vec3::zeros(), 0.1, 2);

  // A capsule (a cylinder with hemispherical end caps): outer tip of one hemispherical cap,
  // outer tip of the other, radius.
  const EBGeometry::CapsuleSDF<T> capsule(Vec3::zeros(), Vec3::ones(), 0.1);

  // An infinite (single-nap) cone opening along -y: apex position, full opening angle in degrees.
  const EBGeometry::InfiniteConeSDF<T> infiniteCone(Vec3::zeros(), 45.0);

  // A finite cone opening along -y: apex position, height (apex to base), full opening angle in
  // degrees.
  const EBGeometry::ConeSDF<T> cone(Vec3::zeros(), 1.0, 45.0);

  // A fractal Perlin noise field (not a distance function to a specific shape -- see this
  // example's README): amplitude, per-axis spatial frequency, per-octave amplitude decay
  // ("persistence"), and number of octaves summed together.
  const EBGeometry::PerlinSDF<T> perlin(1.0, Vec3::ones(), 0.5, 10);

  // A box with rounded edges and corners: center, outer side lengths (rounding included),
  // edge/corner rounding radius.
  const EBGeometry::RoundedBoxSDF<T> roundBox(Vec3::zeros(), T(1.2) * Vec3::ones(), 0.1);

  // A cylinder along the y-axis with rounded edges: center, radius, edge rounding radius, height.
  const EBGeometry::RoundedCylinderSDF<T> roundCylinder(Vec3::zeros(), 1.0, 0.1, 1.0);

  // Check each shape against its distance at a few points, worked out by hand from the description
  // above. Positive is outside, negative inside, zero on the surface.
  const T tolerance = std::is_same_v<T, float> ? T(1.0e-5) : T(1.0e-12);

  const T sqrt3        = std::sqrt(T(3));
  const T sinHalfAngle = std::sin(T(22.5) * EBGeometry::pi<T> / T(180)); // Half of the cones' 45 degrees.

  int failures = 0;

  const auto check = [&](const std::string& a_name, const T a_value, const T a_expected) {
    if (std::abs(a_value - a_expected) > tolerance * (T(1) + std::abs(a_expected))) {
      std::cerr << a_name << ": expected " << a_expected << ", got " << a_value << "\n";

      failures++;
    }
  };

  // The plane's normal is normalized, so the distance is along the unit diagonal.
  check("plane, along the normal", plane.signedDistance(Vec3::ones()), sqrt3);
  check("plane, against the normal", plane.signedDistance(-Vec3::ones()), -sqrt3);

  check("sphere, at its center", sphere.signedDistance(Vec3::zeros()), T(-1));
  check("sphere, outside", sphere.signedDistance(Vec3(2, 0, 0)), T(1));

  check("box, at its center", box.signedDistance(T(0.5) * Vec3::ones()), T(-0.5));
  check("box, outside a face", box.signedDistance(Vec3(2, 0.5, 0.5)), T(1));

  // The tube's centre-line is the unit circle in the xz-plane.
  check("torus, on its centre-line", torus.signedDistance(Vec3(0, 0, 1)), T(-0.1));
  check("torus, at its center", torus.signedDistance(Vec3::zeros()), T(0.9));
  check("torus, above its centre-line", torus.signedDistance(Vec3(1, 0.5, 0)), T(0.4));

  check("cylinder, on its axis", finiteCylinder.signedDistance(T(0.5) * Vec3::ones()), T(-0.1));
  check("cylinder, beyond a cap", finiteCylinder.signedDistance(T(2) * Vec3::ones()), sqrt3);

  check("infinite cylinder, on its axis", infiniteCylinder.signedDistance(Vec3(0, 0, -3)), T(-0.1));
  check("infinite cylinder, outside", infiniteCylinder.signedDistance(Vec3(1, 0, 7)), T(0.9));

  // The capsule's tips are on its surface.
  check("capsule, at a tip", capsule.signedDistance(Vec3::zeros()), T(0));
  check("capsule, on its axis", capsule.signedDistance(T(0.5) * Vec3::ones()), T(-0.1));
  check("capsule, beyond a tip", capsule.signedDistance(T(2) * Vec3::ones()), sqrt3);

  // The cones open along -y from their tips at the origin.
  check("infinite cone, at the tip", infiniteCone.signedDistance(Vec3::zeros()), T(0));
  check("infinite cone, above the tip", infiniteCone.signedDistance(Vec3(0, 1, 0)), T(1));
  check("infinite cone, on its axis", infiniteCone.signedDistance(Vec3(0, -1, 0)), -sinHalfAngle);

  check("cone, above the tip", cone.signedDistance(Vec3(0, 1, 0)), T(1));
  check("cone, on its axis", cone.signedDistance(Vec3(0, -0.5, 0)), T(-0.5) * sinHalfAngle);
  check("cone, below its base", cone.signedDistance(Vec3(0, -3, 0)), T(2));

  // Noise is not a distance, but it is defined everywhere.
  check("Perlin noise, finite", std::isfinite(perlin.signedDistance(Vec3(0.3, 0.6, 0.9))) ? T(1) : T(0), T(1));

  // The rounded box is centred at the origin with outer half-sides 0.6.
  check("rounded box, at its center", roundBox.signedDistance(Vec3::zeros()), T(-0.6));
  check("rounded box, outside a face", roundBox.signedDistance(Vec3(1.6, 0, 0)), T(1));

  // The rounded cylinder is centred at the origin with its axis along y, and half as high as wide.
  check("rounded cylinder, at its center", roundCylinder.signedDistance(Vec3::zeros()), T(-0.5));
  check("rounded cylinder, above a cap", roundCylinder.signedDistance(Vec3(0, 1.5, 0)), T(1));

  // Every shape also gives its axis-aligned bounding box, extending to the largest representable
  // value along any direction in which the shape is unbounded.
  const auto torusBox = torus.computeBoundingVolume();
  const auto coneBox  = cone.computeBoundingVolume();

  check("torus box, top", torusBox.getHighCorner()[1], T(0.1));
  check("torus box, side", torusBox.getHighCorner()[2], T(1.1));
  check("cone box, base", coneBox.getLowCorner()[1], T(-1));
  check("cone box, base radius", coneBox.getHighCorner()[0], std::tan(T(22.5) * EBGeometry::pi<T> / T(180)));
  check("infinite cylinder box, along its axis",
        infiniteCylinder.computeBoundingVolume().getHighCorner()[2],
        EBGeometry::Math::Limits<T>::max());

  // And how far its value can be trusted as a distance: every shape here is exact, except the noise.
  static_assert(EBGeometry::TorusSDF<T>::distanceQuality == EBGeometry::DistanceQuality::Exact);
  static_assert(EBGeometry::PerlinSDF<T>::distanceQuality == EBGeometry::DistanceQuality::NotADistance);

  std::cout << "Shapes checked against hand-worked distances; " << failures << " failed\n";

  // ── The same shapes on a GPU ───────────────────────────────────────────────────────────────────────
  //
  // Compiled as CUDA or HIP (GPU=HIP or GPU=CUDA), EBGeometry::GPU::transform() runs a query in a
  // kernel, one thread per input; in a plain C++ build it runs a host loop, so the same line compiles in
  // both. Here every shape is evaluated on a grid on the device and compared with the host's values.
  // They agree to rounding rather than bit for bit, since a device compiler may fuse a multiply and an
  // add where the host compiler does not.
  if (!EBGeometry::GPU::enabled) {
    std::cout << "On the device: not built (plain C++); compile with GPU=HIP or GPU=CUDA to run it\n";
  }
  else if (!EBGeometry::GPU::deviceAvailable()) {
    std::cout << "On the device: skipped, no GPU is visible\n";
  }
  else {
    const AllShapes shapes{plane,
                           sphere,
                           box,
                           torus,
                           finiteCylinder,
                           infiniteCylinder,
                           capsule,
                           infiniteCone,
                           cone,
                           perlin,
                           roundBox,
                           roundCylinder};

    std::vector<Vec3> points;

    constexpr int n = 20;

    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        for (int k = 0; k < n; k++) {
          points.emplace_back(T(0.2) * (T(i) - T(9.63)), T(0.2) * (T(j) - T(9.47)), T(0.2) * (T(k) - T(9.39)));
        }
      }
    }

    const std::vector<ShapeValues> onDevice = EBGeometry::GPU::transform(shapes, points);

    const T rounding = std::is_same_v<T, float> ? T(1.0e-4) : T(1.0e-9);

    int mismatches = 0;

    for (std::size_t i = 0; i < points.size(); i++) {
      const ShapeValues onHost = shapes(points[i]);

      for (int s = 0; s < 12; s++) {
        if (std::abs(onDevice[i].value[s] - onHost.value[s]) > rounding * (T(1) + std::abs(onHost.value[s]))) {
          mismatches++;
        }
      }
    }

    std::cout << "On the device (" << EBGeometry::GPU::backendName << "): 12 shapes at " << points.size() << " points, "
              << mismatches << " differ from the host\n";

    failures += mismatches;
  }

  return failures == 0 ? 0 : 1;
}
