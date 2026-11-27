// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// Test helpers shared by the CSG tests (TestCSG, TestCSGDevice): rows of spheres, boxes and cylinders, a grid of
// meshes, a mix of bounded and unbounded primitives, their boxes, and brute-force unions to check the BVH unions
// against.

#ifndef EBGEOMETRY_TESTCSGHELPERS_HPP
#define EBGEOMETRY_TESTCSGHELPERS_HPP

#include "EBGeometry.hpp"
#include "TestUtilities.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <string>
#include <vector>

namespace EBGeometryTestCSG {

using namespace EBGeometry;

template <class T>
using BV = BoundingVolumes::AABBT<T>;

template <class T>
BV<T>
sphereBV(const Vec3T<T>& a_center, const T a_radius)
{
  return {a_center - a_radius * Vec3T<T>::ones(), a_center + a_radius * Vec3T<T>::ones()};
}

// A small scene of spheres on a line, spaced so neighbors are disjoint -- enough primitives to
// give the BVH real partitioning depth (unlike a scene of two spheres).
inline constexpr int NumRowSpheres = 12;

template <class T>
Vec3T<T>
rowCenter(const int a_i)
{
  return Vec3T<T>(T(3.0) * T(a_i), T(0), T(0));
}

// The row as plain value-type spheres, which is what the BVH unions store.
template <class T>
std::vector<SphereSDF<T>>
sphereRow()
{
  std::vector<SphereSDF<T>> spheres;

  spheres.reserve(NumRowSpheres);

  for (int i = 0; i < NumRowSpheres; i++) {
    spheres.emplace_back(rowCenter<T>(i), T(1));
  }

  return spheres;
}

template <class T>
std::vector<BV<T>>
sphereRowBVs()
{
  std::vector<BV<T>> bvs;

  bvs.reserve(NumRowSpheres);

  for (int i = 0; i < NumRowSpheres; i++) {
    bvs.push_back(sphereBV<T>(rowCenter<T>(i), T(1)));
  }

  return bvs;
}

template <class T>
std::vector<Vec3T<T>>
lineQueryPoints()
{
  std::vector<Vec3T<T>> pts;

  for (const T x : sweepValues<T>(T(-2.0), T(35.0), 28)) {
    pts.emplace_back(x, T(0.25), T(-0.5));
  }

  return pts;
}

// Brute-force sharp union: the smallest primitive value found by a full linear scan.
template <class T, class Primitive>
T
bruteMin(const std::vector<Primitive>& a_primitives, const Vec3T<T>& a_point)
{
  T nearest = std::numeric_limits<T>::infinity();

  for (const auto& primitive : a_primitives) {
    nearest = std::min(nearest, primitive.signedDistance(a_point));
  }

  return nearest;
}

// Brute-force smooth union: blend the two smallest primitive values found by a full linear scan
// instead of the pruned BVH traversal.
template <class T, class Primitive, class Blend>
T
bruteTwoNearest(const std::vector<Primitive>& a_primitives, const Vec3T<T>& a_point, const T a_smoothLen, Blend a_blend)
{
  T a = std::numeric_limits<T>::infinity();
  T b = std::numeric_limits<T>::infinity();

  for (const auto& primitive : a_primitives) {
    const T d = primitive.signedDistance(a_point);

    if (d < a) {
      b = a;
      a = d;
    }
    else if (d < b) {
      b = d;
    }
  }

  return a_blend(a, b, a_smoothLen);
}

template <class T>
using TestTriMesh = TriMeshSDF<T, 4, 4>;

// A 3x2 grid of dodecahedra (circumradius ~1.4), each translated before its TriMeshSDF is built, all
// in a_pool. This is how a union of several copies of one mesh is built without a Translate wrapper.
template <class T>
std::vector<TestTriMesh<T>>
dodecahedronGrid(Pool& a_pool)
{
  const auto triangles = Parser::readIntoTriangles<T>(std::string(EBGEOMETRY_TEST_DATA_DIR) + "/dodecahedron.obj");

  std::vector<TestTriMesh<T>> meshes;

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 2; j++) {
      const Vec3T<T> shift(T(4) * T(i), T(4) * T(j), T(0));

      auto shifted = triangles;

      for (auto& triangle : shifted) {
        auto vertices = triangle.getVertexPositions();

        for (auto& v : vertices) {
          v = v + shift;
        }

        triangle.setVertexPositions(vertices);
      }

      meshes.emplace_back(shifted, a_pool, BVH::Construction::SAH, 1);
    }
  }

  return meshes;
}

template <class T>
std::vector<BV<T>>
boundingVolumes(const std::vector<TestTriMesh<T>>& a_meshes)
{
  std::vector<BV<T>> bvs;

  for (const auto& mesh : a_meshes) {
    bvs.push_back(mesh.computeBoundingVolume());
  }

  return bvs;
}

template <class T>
std::vector<Vec3T<T>>
gridQueryPoints()
{
  std::vector<Vec3T<T>> pts;

  for (const T x : sweepValues<T>(T(-2.0), T(10.0), 13)) {
    pts.emplace_back(x, T(1.5), T(0.3));
    pts.emplace_back(x, T(4.2), T(-1.0));
  }

  return pts;
}

// Either a sphere or an infinite cylinder: one primitive type with bounded and unbounded members.
template <class T>
struct SphereOrCylinder
{
  SphereSDF<T>           m_sphere;
  InfiniteCylinderSDF<T> m_cylinder;
  bool                   m_isCylinder = false;

  EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    return m_isCylinder ? m_cylinder.signedDistance(a_point) : m_sphere.signedDistance(a_point);
  }

  BV<T>
  computeBoundingVolume() const noexcept
  {
    return m_isCylinder ? m_cylinder.computeBoundingVolume() : m_sphere.computeBoundingVolume();
  }
};

// 300 small spheres scattered in [-6, 6]^3 and four thin infinite cylinders, one along x, two along
// y and one along z.
template <class T>
std::vector<SphereOrCylinder<T>>
spheresAndCylinders()
{
  std::mt19937                      rng(97531);
  std::uniform_real_distribution<T> coord(T(-6), T(6));

  std::vector<SphereOrCylinder<T>> primitives;

  for (int i = 0; i < 300; i++) {
    primitives.push_back({SphereSDF<T>(Vec3T<T>(coord(rng), coord(rng), coord(rng)), T(0.3)), {}, false});
  }

  for (const size_t axis : {size_t(0), size_t(1), size_t(2), size_t(1)}) {
    primitives.push_back(
      {{}, InfiniteCylinderSDF<T>(Vec3T<T>(coord(rng), coord(rng), coord(rng)), T(0.2), axis), true});
  }

  return primitives;
}

template <class T, class Primitive>
std::vector<BV<T>>
boxesOf(const std::vector<Primitive>& a_primitives)
{
  std::vector<BV<T>> bvs;

  bvs.reserve(a_primitives.size());

  for (const auto& primitive : a_primitives) {
    bvs.push_back(primitive.computeBoundingVolume());
  }

  return bvs;
}

// A scene of three shape types and an empty fourth: the sphere row, boxes between its spheres, and infinite
// cylinders along z (whose unbounded boxes keep them out of the BVH).
template <class T>
std::vector<BoxSDF<T>>
boxRow()
{
  std::vector<BoxSDF<T>> boxes;

  for (int i = 0; i + 1 < NumRowSpheres; i += 2) {
    const T x = T(3) * T(i) + T(1.5);

    boxes.emplace_back(Vec3T<T>(x - T(0.4), T(-0.3), T(-0.3)), Vec3T<T>(x + T(0.4), T(0.3), T(0.3)));
  }

  return boxes;
}

template <class T>
std::vector<InfiniteCylinderSDF<T>>
cylinderRow()
{
  return {InfiniteCylinderSDF<T>(Vec3T<T>(T(7.5), T(2), T(0)), T(0.4), 2),
          InfiniteCylinderSDF<T>(Vec3T<T>(T(22.5), T(-2), T(0)), T(0.6), 2)};
}

// Points along and around the row, between and inside its members.
template <class T>
std::vector<Vec3T<T>>
rowQueryPoints()
{
  std::vector<Vec3T<T>> points;

  for (const T x : sweepValues<T>(T(-2), T(36), 39)) {
    for (const T y : {T(-2.5), T(0.1), T(1.9)}) {
      points.emplace_back(x, y, T(0.2));
    }
  }

  return points;
}

// The minimum, and the two smallest values, over every primitive of every array.
template <class T, class... Ps>
std::vector<T>
allValues(const Vec3T<T>& a_point, const std::vector<Ps>&... a_primitives)
{
  std::vector<T> values;

  const auto add = [&](const auto& a_ofType) {
    for (const auto& primitive : a_ofType) {
      values.push_back(primitive.signedDistance(a_point));
    }
  };

  (add(a_primitives), ...);

  return values;
}

} // namespace EBGeometryTestCSG

#endif
