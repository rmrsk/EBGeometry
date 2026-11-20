// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

// The device tests of the ray queries of mesh SDFs (TestRayMesh.cpp): the three mesh SDFs' crossings, and the face
// hit at an edge or vertex that faces share, on a device against the host. They are apart from TestRayMesh.cpp so
// that a GPU build compiles only them with the device compiler.

#include "TestRayHelpers.hpp"

using namespace EBGeometry;
using namespace EBGeometryTestRay;

namespace {

// The face a ray hits first, as a number: -1 for none.
template <class T, class Shape>
struct HitFace
{
  Shape m_shape;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const RayT<T>& a_ray) const noexcept
  {
    const HitT<T> hit = firstHit(m_shape, a_ray, T(0), T(10));

    return hit.hit() ? T(hit.m_primitive) : -T(1);
  }
};

} // namespace

TEMPLATE_TEST_CASE("Ray: device ray queries of mesh SDFs match the host", "[Ray][gpu]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  const std::vector<RayT<T>> rays = deviceRays<T>();

  // The mesh SDFs, against device mirrors of their pool.
  Pool pool(hostMemoryResource());

  const auto cube     = Parser::readIntoTriMeshSDF<T, 4, 4>(unitCube(), pool);
  const auto polygons = Parser::readIntoMeshSDF<T, 4>(unitCube(), pool);
  const auto flat     = Parser::readIntoFlatMeshSDF<T>(unitCube(), pool);

  pool.freeze();

  const Pool device = Pool::mirror(pool, deviceTestResource());

  using M = std::decay_t<decltype(cube)>;
  using P = std::decay_t<decltype(polygons)>;
  using L = std::decay_t<decltype(flat)>;

  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, M>{cube.rebasedView(device)}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, M>{cube}, rays));
  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, P>{polygons.rebasedView(device)}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, P>{polygons}, rays));
  requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, L>{flat.rebasedView(device)}, rays),
                     evaluateOnHost<T>(FirstCrossing<T, L>{flat}, rays));

  // The same crossings without their normals.
  requireSameResults(evaluateOnDevice<T>(CrossingDistance<T, M>{cube.rebasedView(device)}, rays),
                     evaluateOnHost<T>(CrossingDistance<T, M>{cube}, rays));
}

TEMPLATE_TEST_CASE("Ray: at an edge or vertex that faces share, a device hits the face the host does",
                   "[Ray][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  const size_t n    = 4;
  const auto   soup = gridCube<T>(Vec3T<T>::zeros(), T(1), n);

  Pool       pool(hostMemoryResource());
  const auto mesh = soup.convertToDCEL(pool);

  const TriMeshSDF<T, 4, 4> triangles(soup.convertToTriangles(), pool, BVH::Construction::SAH, 4);
  const MeshSDF<T, 4>       polygons(mesh, pool, BVH::Construction::SAH);
  const FlatMeshSDF<T>      flat(mesh, pool);

  pool.freeze();

  const Pool device = Pool::mirror(pool, deviceTestResource());

  std::vector<RayT<T>> rays;

  for (const auto& r : sharedPointRays<T>(soup, n)) {
    rays.push_back(r.ray);
  }

  using M = TriMeshSDF<T, 4, 4>;
  using P = MeshSDF<T, 4>;
  using L = FlatMeshSDF<T>;

  // Face ids, exactly.
  requireSameResults(evaluateOnDevice<T>(HitFace<T, M>{triangles.rebasedView(device)}, rays),
                     evaluateOnHost<T>(HitFace<T, M>{triangles}, rays),
                     T(0));
  requireSameResults(evaluateOnDevice<T>(HitFace<T, P>{polygons.rebasedView(device)}, rays),
                     evaluateOnHost<T>(HitFace<T, P>{polygons}, rays),
                     T(0));
  requireSameResults(evaluateOnDevice<T>(HitFace<T, L>{flat.rebasedView(device)}, rays),
                     evaluateOnHost<T>(HitFace<T, L>{flat}, rays),
                     T(0));
}
