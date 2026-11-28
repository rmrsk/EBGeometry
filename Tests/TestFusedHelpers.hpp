// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// Test helpers shared by the fused CSG tests (TestFused, TestFusedDevice): the mesh file, the query points and the
// analytic shapes the tests compose.

#ifndef EBGEOMETRY_TESTFUSEDHELPERS_HPP
#define EBGEOMETRY_TESTFUSEDHELPERS_HPP

#include "EBGeometry.hpp"

#include <string>
#include <vector>

namespace EBGeometryTestFused {

using namespace EBGeometry;

// Query points spread over a few units around the origin, none on a symmetry plane.
template <class T>
std::vector<Vec3T<T>>
probes()
{
  std::vector<Vec3T<T>> points;

  for (int i = -4; i <= 4; i++) {
    for (int j = -3; j <= 3; j++) {
      for (int k = -2; k <= 2; k++) {
        points.emplace_back(T(0.45) * T(i) + T(0.05), T(0.5) * T(j) - T(0.1), T(0.6) * T(k) + T(0.2));
      }
    }
  }

  return points;
}

// The analytic shapes the tests compose.
template <class T>
struct Shapes
{
  BoxSDF<T>              box{Vec3T<T>(T(-1), T(-0.8), T(-0.6)), Vec3T<T>(T(0.9), T(1), T(0.7))};
  SphereSDF<T>           ball{Vec3T<T>(T(0.1), T(-0.1), T(0)), T(1.2)};
  SphereSDF<T>           small{Vec3T<T>(T(0.8), T(0.3), T(-0.2)), T(0.5)};
  InfiniteCylinderSDF<T> drill{Vec3T<T>(T(0), T(0.1), T(0)), T(0.4), 0};
  TorusSDF<T>            torus{Vec3T<T>(T(-0.3), T(0), T(0.2)), T(0.9), T(0.2)};
  PlaneSDF<T>            plane{Vec3T<T>(T(0.1), T(0.2), T(0.3)), Vec3T<T>(T(1), T(2), T(-1))};
};

} // namespace EBGeometryTestFused

#endif
