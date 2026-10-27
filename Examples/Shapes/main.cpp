// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// The built-in analytic shapes: how to construct each one, evaluate its signed distance, ask for its
// bounding box, and see how far its value can be trusted as a distance. Every shape is a small value
// type, so the last section evaluates one in a GPU kernel too. See README.md.

#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T    = EBGEOMETRY_PRECISION;
using Vec3 = EBGeometry::Vec3T<T>;

namespace {

// ── The code that runs on either side ────────────────────────────────────────────────────────────────
//
// A query for EBGeometry::GPU::transform(): a trivially copyable functor whose EBGEOMETRY_HOST_DEVICE
// call operator maps one point to one result. A shape is a plain value, so the query holds it by value
// and reaches a kernel with no memory to manage.
template <class Shape>
struct Distance
{
  Shape m_shape;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3& a_point) const noexcept
  {
    return m_shape.signedDistance(a_point);
  }
};

const char*
qualityName(const EBGeometry::DistanceQuality a_quality)
{
  switch (a_quality) {
  case EBGeometry::DistanceQuality::Exact:
    return "exact";
  case EBGeometry::DistanceQuality::Bound:
    return "bound";
  default:
    return "not a distance";
  }
}

// One row of the table: the shape's value at three points, its bounding box along x, and its quality.
template <class Shape>
void
report(const std::string& a_name, const Shape& a_shape)
{
  const EBGeometry::BoundingVolumes::AABBT<T> box = a_shape.computeBoundingVolume();

  const auto coordinate = [](const T a_x) -> std::string {
    if (std::abs(a_x) >= EBGeometry::Math::Limits<T>::max()) {
      return a_x < 0 ? "-inf" : "+inf";
    }

    std::ostringstream text;

    text << std::fixed << std::setprecision(2) << a_x;

    return text.str();
  };

  std::cout << std::setw(18) << std::left << a_name << std::right << std::setw(11)
            << a_shape.signedDistance(Vec3::zeros()) << std::setw(11)
            << a_shape.signedDistance(Vec3(T(0.5), T(0.5), T(0.5))) << std::setw(11)
            << a_shape.signedDistance(Vec3(T(2), T(0), T(0))) << "   [" << coordinate(box.getLowCorner()[0]) << ", "
            << coordinate(box.getHighCorner()[0]) << "]   " << qualityName(Shape::distanceQuality) << '\n';
}

} // namespace

int
main(int argc, char* argv[])
{
  // ── Construct the shapes ───────────────────────────────────────────────────────────────────────────
  //
  // The constructors take the shape's position before its sizes. A shape with a built-in axis (the
  // torus, both cones, the rounded cylinder) has it along y, and a size is that of the finished shape.
  // See the class documentation in EBGeometry_AnalyticDistanceFunctions.hpp for every parameter.

  // A plane through the origin, oriented by its normal vector (normalized for you).
  const EBGeometry::PlaneSDF<T> plane(Vec3::zeros(), Vec3::ones());

  // A sphere: center, radius.
  const EBGeometry::SphereSDF<T> sphere(Vec3::zeros(), T(1));

  // An axis-aligned box: low corner, high corner.
  const EBGeometry::BoxSDF<T> box(Vec3::zeros(), Vec3::ones());

  // A torus around the y axis: center, major radius (the ring), minor radius (the tube).
  const EBGeometry::TorusSDF<T> torus(Vec3::zeros(), T(1), T(0.1));

  // A capped cylinder: the centers of its two end caps, radius.
  const EBGeometry::CylinderSDF<T> cylinder(Vec3::zeros(), Vec3::ones(), T(0.25));

  // An infinite cylinder: a point on its axis, radius, and the Cartesian axis it runs along (2 = z).
  const EBGeometry::InfiniteCylinderSDF<T> infiniteCylinder(Vec3::zeros(), T(0.1), 2);

  // A capsule (a cylinder with hemispherical caps): the outer tips of its two caps, radius.
  const EBGeometry::CapsuleSDF<T> capsule(Vec3::zeros(), Vec3::ones(), T(0.25));

  // An infinite cone opening along -y: apex, full opening angle in degrees.
  const EBGeometry::InfiniteConeSDF<T> infiniteCone(Vec3::zeros(), T(45));

  // A finite cone opening along -y: apex, height, full opening angle in degrees.
  const EBGeometry::ConeSDF<T> cone(Vec3::zeros(), T(1), T(45));

  // A box with rounded edges and corners: center, outer side lengths, rounding radius.
  const EBGeometry::RoundedBoxSDF<T> roundedBox(Vec3::zeros(), T(1.2) * Vec3::ones(), T(0.1));

  // A cylinder along y with rounded edges: center, radius, rounding radius, height.
  const EBGeometry::RoundedCylinderSDF<T> roundedCylinder(Vec3::zeros(), T(1), T(0.1), T(1));

  // Fractal Perlin noise, which is not the distance to any surface: amplitude, per-axis frequency,
  // per-octave amplitude decay (persistence), and the number of octaves.
  const EBGeometry::PerlinSDF<T> perlin(T(1), Vec3::ones(), T(0.5), 10);

  // ── Use them on the host ───────────────────────────────────────────────────────────────────────────
  //
  // signedDistance(point) is negative inside, positive outside and zero on the surface.
  // computeBoundingVolume() is the shape's axis-aligned box, infinite along any direction in which the
  // shape is unbounded. distanceQuality says whether the value is the exact Euclidean distance, a lower
  // bound on it, or only a sign; composition (CSG, blending) and BVH pruning depend on it.
  std::cout << std::fixed << std::setprecision(4);
  std::cout << std::setw(18) << std::left << "shape" << std::right << std::setw(11) << "(0,0,0)" << std::setw(11)
            << "(.5,.5,.5)" << std::setw(11) << "(2,0,0)" << "   box along x   quality\n";

  report("plane", plane);
  report("sphere", sphere);
  report("box", box);
  report("torus", torus);
  report("cylinder", cylinder);
  report("infinite cylinder", infiniteCylinder);
  report("capsule", capsule);
  report("infinite cone", infiniteCone);
  report("cone", cone);
  report("rounded box", roundedBox);
  report("rounded cylinder", roundedCylinder);
  report("Perlin noise", perlin);

  // ── The same query on a GPU ────────────────────────────────────────────────────────────────────────
  //
  // GPU::transform(query, points, results) runs the query once per point, writing into results (which
  // the caller sizes): in a kernel when this file is compiled as CUDA or HIP (make GPU=CUDA, or
  // -DEBGEOMETRY_EXAMPLE_GPU=CUDA), and in a host loop otherwise, so
  // the same lines compile and run in both builds. It copies the points to the device and the results
  // back. A shape needs no copying of its own: it reaches the kernel inside the query.
  if (EBGeometry::GPU::enabled && !EBGeometry::GPU::deviceAvailable()) {
    std::cout << "\nThe torus on the device: skipped, no GPU is visible\n";
  }
  else {
    std::vector<Vec3> points;

    for (int i = 0; i <= 8; i++) {
      points.emplace_back(T(0.25) * T(i), T(0), T(0));
    }

    std::vector<T> distances(points.size());

    EBGeometry::GPU::transform(Distance<EBGeometry::TorusSDF<T>>{torus}, points, distances);

    std::cout << "\nThe torus along the x axis, through GPU::transform (backend: " << EBGeometry::GPU::backendName
              << "):\n";

    for (std::size_t i = 0; i < points.size(); i++) {
      std::cout << "  x = " << points[i][0] << "   distance = " << std::setw(8) << distances[i] << '\n';
    }
  }

  // ── Look at them ───────────────────────────────────────────────────────────────────────────────────
  //
  // The bounded shapes side by side, two rows of four. Shapes of different types are put together with the
  // expression builder and a Tape (see the BasicCSG example); Translate moves each to its place. A window
  // opens when built with VIEWER=SDL2; otherwise, with --no-view or without a display, view() prints why
  // and returns.
  const std::vector<EBGeometry::Expr<T>> bounded{
    sphere, box, torus, cylinder, capsule, cone, roundedBox, roundedCylinder};

  std::vector<EBGeometry::Expr<T>> gallery;

  for (std::size_t i = 0; i < bounded.size(); i++) {
    gallery.push_back(EBGeometry::Translate(bounded[i], Vec3(T(3) * T(i % 4), T(0), T(3) * T(i / 4))));
  }

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  EBGeometry::Render::view(EBGeometry::Tape<T>(pool, EBGeometry::Union(gallery)), argc, argv);

  return 0;
}
