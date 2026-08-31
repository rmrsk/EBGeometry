// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using namespace EBGeometry::DCEL;
using Catch::Matchers::WithinRel;

// MeshSDF<T, Meta, K> under test, with the mesh's own metadata type and the SIMD-optimal
// branching factor for the precision in use.
template <class T>
using TestMeshSDF = MeshSDF<T, DefaultMetaData, BVH::DefaultBranchingRatio<T>()>;

// Path to test data injected by CMake.
static const std::string g_dataDir = EBGEOMETRY_TEST_DATA_DIR;

// ─────────────────────────────────────────────────────────────────────────────
// Helper: load the test tetrahedron as a DCEL mesh
// ─────────────────────────────────────────────────────────────────────────────

template <class T>
static std::shared_ptr<MeshT<T, DefaultMetaData>>
loadTetrahedron(Pool& a_pool)
{
  auto mesh = Parser::readIntoDCEL<T>(g_dataDir + "/tetrahedron.stl", a_pool);

  // The mesh is attached to a_pool by its first reserve inside readIntoDCEL and is queryable from
  // that point on: no freeze, no bind. Each call site below uses its own fresh, single-use Pool.

  return mesh;
}

// Build a tetrahedron DCEL mesh entirely from hard-coded data (no file I/O).
// Vertices: A=(0,0,0) B=(1,0,0) C=(0,1,0) D=(0,0,1).
// Face winding is CCW-from-outside so that the new (x2-x0)×(x2-x1) formula
// yields outward normals — standard SDF convention: negative inside, positive
// outside.
template <class T>
static std::shared_ptr<MeshT<T, DefaultMetaData>>
buildTetrahedron(Pool& a_pool)
{
  std::vector<Vec3T<T>> verts = {
    {0.0, 0.0, 0.0}, // 0 = A
    {1.0, 0.0, 0.0}, // 1 = B
    {0.0, 1.0, 0.0}, // 2 = C
    {0.0, 0.0, 1.0}, // 3 = D
  };

  // Each row is one triangular face.  Winding chosen so outward normal follows
  // from (x2-x0)×(x2-x1):
  //   {0,2,1} → normal (0, 0,-1)   bottom face (z=0 plane)
  //   {0,1,3} → normal (0,-1, 0)   front  face (y=0 plane)
  //   {0,3,2} → normal (-1,0, 0)   left   face (x=0 plane)
  //   {1,2,3} → normal (1, 1, 1)/√3 slant  face
  std::vector<std::vector<size_t>> facets = {{0, 2, 1}, {0, 1, 3}, {0, 3, 2}, {1, 2, 3}};

  Soup::compress(verts, facets);

  auto mesh = std::make_shared<MeshT<T, DefaultMetaData>>();
  Soup::soupToDCEL(*mesh, a_pool, verts, facets, "tetrahedron-hard"); // reconciles internally

  return mesh;
}

// A double-returning margin appropriate for comparisons that are exact by construction (unit
// normals, symmetric distances) modulo floating-point reordering.
template <class T>
double
exactMargin()
{
  return std::is_same_v<T, float> ? 1.0e-4 : 1.0e-12;
}

// A looser margin for comparisons that chain a few more floating-point operations (two search
// algorithms agreeing, a near-zero surface-point SDF).
template <class T>
double
formulaMargin()
{
  return std::is_same_v<T, float> ? 1.0e-3 : 1.0e-10;
}

// A margin for comparing a brute-force scan against a SIMD/BVH-accelerated traversal, which
// chains many more floating-point operations across the mesh's faces.
template <class T>
double
traversalMargin()
{
  return std::is_same_v<T, float> ? 1.0e-2 : 1.0e-6;
}

// ─────────────────────────────────────────────────────────────────────────────
// VertexT tests
// ─────────────────────────────────────────────────────────────────────────────

template <class T>
using TestVertex = VertexT<T, DefaultMetaData>;

template <class T>
using TestFace = FaceT<T, DefaultMetaData>;

template <class T>
using TestEdge = EdgeT<T, DefaultMetaData>;

template <class T>
using TestMesh = MeshT<T, DefaultMetaData>;

TEMPLATE_TEST_CASE("VertexT: default construction leaves defined, zeroed state",
                   "[DCEL][Vertex]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;
  TestVertex<T> v;

  REQUIRE(v.getPosition() == Vec3T<T>::zeros());
  REQUIRE(v.getNormal() == Vec3T<T>::zeros());
  REQUIRE(v.getOutgoingEdgeIndex() == UINT32_MAX);
  REQUIRE(v.getMetaData() == 0);
}

TEMPLATE_TEST_CASE("VertexT: position-only and position+normal constructors",
                   "[DCEL][Vertex]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;
  const Vec3T<T> pos(1, 2, 3);
  const Vec3T<T> normal(0, 0, 1);

  TestVertex<T> vPosOnly(pos);
  REQUIRE(vPosOnly.getPosition() == pos);
  REQUIRE(vPosOnly.getNormal() == Vec3T<T>::zeros());

  TestVertex<T> vBoth(pos, normal);
  REQUIRE(vBoth.getPosition() == pos);
  REQUIRE(vBoth.getNormal() == normal);
}

TEMPLATE_TEST_CASE("VertexT: setPosition, setNormal, setEdge, define", "[DCEL][Vertex]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;
  TestVertex<T> v;

  v.setPosition(Vec3T<T>(4, 5, 6));
  REQUIRE(v.getPosition() == Vec3T<T>(4, 5, 6));

  v.setNormal(Vec3T<T>(1, 0, 0));
  REQUIRE(v.getNormal() == Vec3T<T>(1, 0, 0));

  v.setEdge(3);
  REQUIRE(v.getOutgoingEdgeIndex() == 3);

  // Setting the edge index to UINT32_MAX is explicitly valid.
  v.setEdge(UINT32_MAX);
  REQUIRE(v.getOutgoingEdgeIndex() == UINT32_MAX);

  v.define(Vec3T<T>(7, 8, 9), 3, Vec3T<T>(0, 1, 0));
  REQUIRE(v.getPosition() == Vec3T<T>(7, 8, 9));
  REQUIRE(v.getOutgoingEdgeIndex() == 3);
  REQUIRE(v.getNormal() == Vec3T<T>(0, 1, 0));
}

TEMPLATE_TEST_CASE("VertexT: normalizeNormalVector produces a unit vector",
                   "[DCEL][Vertex]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;
  TestVertex<T> v;
  v.setNormal(Vec3T<T>(3, 0, 0));
  v.normalizeNormalVector();

  REQUIRE_THAT(v.getNormal().length(), withinAbsT(T(1.0), exactMargin<T>()));
  REQUIRE_THAT(v.getNormal()[0], withinAbsT(T(1.0), exactMargin<T>()));
}

TEMPLATE_TEST_CASE("VertexT: flipNormal negates the normal vector", "[DCEL][Vertex]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;
  TestVertex<T> v(Vec3T<T>::zeros(), Vec3T<T>(1, 0, 0));
  v.flipNormal();

  REQUIRE(v.getNormal() == Vec3T<T>(-1, 0, 0));
}

TEMPLATE_TEST_CASE("VertexT: signedDistance and unsignedDistance2", "[DCEL][Vertex]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;
  TestVertex<T> v(Vec3T<T>(0, 0, 0), Vec3T<T>(0, 0, 1));

  // Point along the outward normal: positive distance.
  REQUIRE_THAT(v.signedDistance(Vec3T<T>(0, 0, 2)), WithinRel(T(2.0)));

  // Point against the outward normal: negative distance.
  REQUIRE_THAT(v.signedDistance(Vec3T<T>(0, 0, -2)), WithinRel(T(-2.0)));

  REQUIRE_THAT(v.unsignedDistance2(Vec3T<T>(3, 0, 0)), WithinRel(T(9.0)));
}

TEMPLATE_TEST_CASE("VertexT: getMetaData reads and writes", "[DCEL][Vertex]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;
  TestVertex<T> v;
  REQUIRE(v.getMetaData() == 0);

  v.getMetaData() = 5;
  REQUIRE(v.getMetaData() == 5);

  v.setMetaData(9);
  REQUIRE(v.getMetaData() == 9);
}

TEMPLATE_TEST_CASE("VertexT: copy construction copies every member, including meta-data",
                   "[DCEL][Vertex]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  TestVertex<T> src(Vec3T<T>(1, 2, 3), Vec3T<T>(0, 0, 1));
  src.setEdge(7);
  src.getMetaData() = 42;

  TestVertex<T> copy(src);

  REQUIRE(copy.getPosition() == src.getPosition());
  REQUIRE(copy.getNormal() == src.getNormal());
  REQUIRE(copy.getOutgoingEdgeIndex() == 7);
  REQUIRE(copy.getMetaData() == 42);
}

TEMPLATE_TEST_CASE("VertexT: copy assignment has the same semantics as copy construction",
                   "[DCEL][Vertex]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  TestVertex<T> src(Vec3T<T>(1, 2, 3), Vec3T<T>(0, 0, 1));
  src.setEdge(7);
  src.getMetaData() = 42;

  TestVertex<T> dst;
  dst = src;

  REQUIRE(dst.getPosition() == src.getPosition());
  REQUIRE(dst.getNormal() == src.getNormal());
  REQUIRE(dst.getOutgoingEdgeIndex() == 7);
  REQUIRE(dst.getMetaData() == 42);
}

TEMPLATE_TEST_CASE("VertexT: move construction and move assignment transfer the entire state",
                   "[DCEL][Vertex]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  TestVertex<T> moveCtorSrc(Vec3T<T>(1, 2, 3), Vec3T<T>(0, 0, 1));
  moveCtorSrc.setEdge(7);
  moveCtorSrc.getMetaData() = 42;

  TestVertex<T> moved(std::move(moveCtorSrc));
  REQUIRE(moved.getPosition() == Vec3T<T>(1, 2, 3));
  REQUIRE(moved.getOutgoingEdgeIndex() == 7);
  REQUIRE(moved.getMetaData() == 42);

  TestVertex<T> moveAssignSrc(Vec3T<T>(4, 5, 6), Vec3T<T>(1, 0, 0));
  moveAssignSrc.setEdge(7);
  moveAssignSrc.getMetaData() = 7;

  TestVertex<T> moveAssignDst;
  moveAssignDst = std::move(moveAssignSrc);
  REQUIRE(moveAssignDst.getPosition() == Vec3T<T>(4, 5, 6));
  REQUIRE(moveAssignDst.getOutgoingEdgeIndex() == 7);
  REQUIRE(moveAssignDst.getMetaData() == 7);
}

// ─────────────────────────────────────────────────────────────────────────────
// EdgeT tests
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("EdgeT: default construction leaves defined, zeroed state",
                   "[DCEL][Edge]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;
  TestEdge<T> e;

  REQUIRE(e.getNormal() == Vec3T<T>::zeros());
  REQUIRE(e.getVertexIndex() == UINT32_MAX);
  REQUIRE(e.getPairEdgeIndex() == UINT32_MAX);
  REQUIRE(e.getNextEdgeIndex() == UINT32_MAX);
  REQUIRE(e.getFaceIndex() == UINT32_MAX);
  REQUIRE(e.getMetaData() == 0);
  REQUIRE(e.size() == 2);
}

TEMPLATE_TEST_CASE("EdgeT: partial (vertex) constructor sets only the starting vertex",
                   "[DCEL][Edge]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  TestEdge<T> e(3u);
  REQUIRE(e.getVertexIndex() == 3);
  REQUIRE(e.getPairEdgeIndex() == UINT32_MAX);
  REQUIRE(e.getNextEdgeIndex() == UINT32_MAX);
  REQUIRE(e.getFaceIndex() == UINT32_MAX);
  REQUIRE(e.getNormal() == Vec3T<T>::zeros());
}

TEMPLATE_TEST_CASE("EdgeT: define, setVertex, setPairEdge, setNextEdge, setFace, setMetaData",
                   "[DCEL][Edge]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  TestEdge<T> e;
  e.define(1, 2, 3);
  REQUIRE(e.getVertexIndex() == 1);
  REQUIRE(e.getPairEdgeIndex() == 2);
  REQUIRE(e.getNextEdgeIndex() == 3);

  e.setFace(4);
  REQUIRE(e.getFaceIndex() == 4);

  e.setMetaData(9);
  REQUIRE(e.getMetaData() == 9);

  // Setting indices to UINT32_MAX is explicitly valid.
  e.setVertex(UINT32_MAX);
  e.setPairEdge(UINT32_MAX);
  e.setNextEdge(UINT32_MAX);
  e.setFace(UINT32_MAX);
  REQUIRE(e.getVertexIndex() == UINT32_MAX);
  REQUIRE(e.getPairEdgeIndex() == UINT32_MAX);
  REQUIRE(e.getNextEdgeIndex() == UINT32_MAX);
  REQUIRE(e.getFaceIndex() == UINT32_MAX);
}

TEMPLATE_TEST_CASE("EdgeT: flipNormal negates the normal vector", "[DCEL][Edge]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool        pool(hostMemoryResource());
  TestMesh<T> mesh;
  mesh.reserveFaces(pool, 1);
  mesh.reserveEdges(pool, 1);

  TestFace<T> face;
  face.define(Vec3T<T>(0, 0, 1), UINT32_MAX);
  mesh.addFace(pool, face);

  TestEdge<T> edge;
  edge.setFace(0);
  mesh.addEdge(pool, edge);

  auto& e = mesh.getEdge(0);
  e.reconcile(mesh);

  REQUIRE(e.getNormal() == Vec3T<T>(0, 0, 1));
  e.flipNormal();
  REQUIRE(e.getNormal() == Vec3T<T>(0, 0, -1));
}

TEMPLATE_TEST_CASE("EdgeT: computeNormal with a single face returns that face's normal",
                   "[DCEL][Edge]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool        pool(hostMemoryResource());
  TestMesh<T> mesh;
  mesh.reserveFaces(pool, 1);
  mesh.reserveEdges(pool, 1);

  TestFace<T> face;
  face.define(Vec3T<T>(1, 0, 0), UINT32_MAX);
  mesh.addFace(pool, face);

  TestEdge<T> edge;
  edge.setFace(0);
  mesh.addEdge(pool, edge);

  REQUIRE(mesh.getEdge(0).computeNormal(mesh) == Vec3T<T>(1, 0, 0));
}

TEMPLATE_TEST_CASE("EdgeT: computeNormal averages both incident faces' normals",
                   "[DCEL][Edge]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool        pool(hostMemoryResource());
  TestMesh<T> mesh;
  mesh.reserveFaces(pool, 2);
  mesh.reserveEdges(pool, 2);

  TestFace<T> face0;
  face0.define(Vec3T<T>(1, 0, 0), UINT32_MAX);
  mesh.addFace(pool, face0);

  TestFace<T> face1;
  face1.define(Vec3T<T>(0, 1, 0), UINT32_MAX);
  mesh.addFace(pool, face1);

  mesh.addEdge(pool, TestEdge<T>()); // edge under test
  mesh.addEdge(pool, TestEdge<T>()); // its pair edge

  mesh.getEdge(0).setFace(0);
  mesh.getEdge(1).setFace(1);
  mesh.getEdge(0).setPairEdge(1);

  const Vec3T<T> expected = Vec3T<T>(1, 1, 0) / Vec3T<T>(1, 1, 0).length();
  const Vec3T<T> actual   = mesh.getEdge(0).computeNormal(mesh);

  REQUIRE_THAT(actual[0], withinAbsT(expected[0], exactMargin<T>()));
  REQUIRE_THAT(actual[1], withinAbsT(expected[1], exactMargin<T>()));
  REQUIRE_THAT(actual[2], withinAbsT(expected[2], exactMargin<T>()));
}

TEMPLATE_TEST_CASE("EdgeT: reconcile stores computeNormal's result", "[DCEL][Edge]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool        pool(hostMemoryResource());
  TestMesh<T> mesh;
  mesh.reserveFaces(pool, 1);
  mesh.reserveEdges(pool, 1);

  TestFace<T> face;
  face.define(Vec3T<T>(0, 1, 0), UINT32_MAX);
  mesh.addFace(pool, face);

  TestEdge<T> edge;
  edge.setFace(0);
  mesh.addEdge(pool, edge);

  auto& e = mesh.getEdge(0);
  e.reconcile(mesh);

  REQUIRE(e.getNormal() == Vec3T<T>(0, 1, 0));
}

TEMPLATE_TEST_CASE("EdgeT: signedDistance and unsignedDistance2 on a simple segment",
                   "[DCEL][Edge]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool        pool(hostMemoryResource());
  TestMesh<T> mesh;
  mesh.reserveVertices(pool, 2);
  mesh.reserveEdges(pool, 2);
  mesh.reserveFaces(pool, 1);

  // Build a two-vertex chain: e0 (start v0) -> e1 (start v1), so e0's "other vertex" is v1.
  mesh.addVertex(pool, TestVertex<T>(Vec3T<T>(0, 0, 0)));
  mesh.addVertex(pool, TestVertex<T>(Vec3T<T>(1, 0, 0)));

  mesh.addEdge(pool, TestEdge<T>(0u));
  mesh.addEdge(pool, TestEdge<T>(1u));

  // Outward normal perpendicular to the edge, pointing +y.
  TestFace<T> face;
  face.define(Vec3T<T>(0, 1, 0), UINT32_MAX);
  mesh.addFace(pool, face);

  mesh.getEdge(0).setNextEdge(1);
  mesh.getEdge(0).setFace(0);
  mesh.getEdge(0).reconcile(mesh);

  REQUIRE(mesh.getEdge(0).getOtherVertex(mesh).getPosition() == mesh.getVertex(1).getPosition());

  // Point above the middle of the edge: on the normal side, distance 2.
  REQUIRE_THAT(mesh.getEdge(0).signedDistance(Vec3T<T>(0.5, 2, 0), mesh), WithinRel(T(2.0)));

  // Point below the middle of the edge: against the normal, negative distance.
  REQUIRE_THAT(mesh.getEdge(0).signedDistance(Vec3T<T>(0.5, -2, 0), mesh), WithinRel(T(-2.0)));

  // unsignedDistance2 clamps the projection to the segment.
  REQUIRE_THAT(mesh.getEdge(0).unsignedDistance2(Vec3T<T>(0.5, 3, 0), mesh), WithinRel(T(9.0)));
}

TEMPLATE_TEST_CASE("EdgeT: copy construction copies every member, including meta-data",
                   "[DCEL][Edge]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  TestEdge<T> src;
  src.define(1, 2, 3);
  src.setFace(4);
  src.setMetaData(42);

  TestEdge<T> copy(src);
  REQUIRE(copy.getVertexIndex() == 1);
  REQUIRE(copy.getPairEdgeIndex() == 2);
  REQUIRE(copy.getNextEdgeIndex() == 3);
  REQUIRE(copy.getFaceIndex() == 4);
  REQUIRE(copy.getMetaData() == 42);
}

TEMPLATE_TEST_CASE("EdgeT: copy assignment has the same semantics as copy construction",
                   "[DCEL][Edge]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  TestEdge<T> src;
  src.define(1, 2, 3);
  src.setFace(4);
  src.setMetaData(42);

  TestEdge<T> dst;
  dst = src;

  REQUIRE(dst.getVertexIndex() == 1);
  REQUIRE(dst.getPairEdgeIndex() == 2);
  REQUIRE(dst.getNextEdgeIndex() == 3);
  REQUIRE(dst.getFaceIndex() == 4);
  REQUIRE(dst.getMetaData() == 42);
}

TEMPLATE_TEST_CASE("EdgeT: move construction and move assignment transfer the entire state",
                   "[DCEL][Edge]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  TestEdge<T> moveCtorSrc;
  moveCtorSrc.define(1, 2, 3);
  moveCtorSrc.setFace(4);
  moveCtorSrc.setMetaData(42);

  TestEdge<T> moved(std::move(moveCtorSrc));
  REQUIRE(moved.getVertexIndex() == 1);
  REQUIRE(moved.getPairEdgeIndex() == 2);
  REQUIRE(moved.getNextEdgeIndex() == 3);
  REQUIRE(moved.getFaceIndex() == 4);
  REQUIRE(moved.getMetaData() == 42);

  TestEdge<T> moveAssignSrc;
  moveAssignSrc.define(1, 2, 3);
  moveAssignSrc.setFace(4);
  moveAssignSrc.setMetaData(7);

  TestEdge<T> moveAssignDst;
  moveAssignDst = std::move(moveAssignSrc);
  REQUIRE(moveAssignDst.getVertexIndex() == 1);
  REQUIRE(moveAssignDst.getPairEdgeIndex() == 2);
  REQUIRE(moveAssignDst.getNextEdgeIndex() == 3);
  REQUIRE(moveAssignDst.getFaceIndex() == 4);
  REQUIRE(moveAssignDst.getMetaData() == 7);
}

// ─────────────────────────────────────────────────────────────────────────────
// EdgeIteratorT tests
// ─────────────────────────────────────────────────────────────────────────────

template <class T>
using TestEdgeIterator = EdgeIteratorT<T, DefaultMetaData>;

TEMPLATE_TEST_CASE("EdgeIteratorT: iterating a face visits exactly its own half-edges and loops back",
                   "[DCEL][Iterator]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool        pool(hostMemoryResource());
  auto        mesh = buildTetrahedron<T>(pool);
  const auto& face = mesh->getFace(0);

  std::vector<uint32_t> visited;
  for (TestEdgeIterator<T> it(*mesh, face); it.ok(); ++it) {
    REQUIRE(mesh->getEdge(it()).getFaceIndex() == 0);
    visited.push_back(it());
  }

  REQUIRE(visited.size() == 3); // Every face in a tetrahedron is a triangle.
  REQUIRE(visited[0] == face.getHalfEdgeIndex());
  REQUIRE(mesh->getEdge(visited[0]).getNextEdgeIndex() == visited[1]);
  REQUIRE(mesh->getEdge(visited[1]).getNextEdgeIndex() == visited[2]);
  REQUIRE(mesh->getEdge(visited[2]).getNextEdgeIndex() == visited[0]); // Loops back to the start.
}

TEMPLATE_TEST_CASE("EdgeIteratorT: constructing from an edge index directly matches constructing from its face",
                   "[DCEL][Iterator]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool        pool(hostMemoryResource());
  auto        mesh = buildTetrahedron<T>(pool);
  const auto& face = mesh->getFace(0);

  std::vector<uint32_t> fromFace;
  for (TestEdgeIterator<T> it(*mesh, face); it.ok(); ++it) {
    fromFace.push_back(it());
  }

  std::vector<uint32_t> fromEdge;
  for (TestEdgeIterator<T> it(*mesh, face.getHalfEdgeIndex()); it.ok(); ++it) {
    fromEdge.push_back(it());
  }

  REQUIRE(fromFace == fromEdge);
}

TEMPLATE_TEST_CASE("EdgeIteratorT: ok() is immediately false for an unset starting edge index",
                   "[DCEL][Iterator]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  TestMesh<T>         mesh;
  TestEdgeIterator<T> it(mesh, UINT32_MAX);
  REQUIRE_FALSE(it.ok());
}

TEMPLATE_TEST_CASE("EdgeIteratorT: reset returns the iterator to its starting edge",
                   "[DCEL][Iterator]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool        pool(hostMemoryResource());
  auto        mesh = buildTetrahedron<T>(pool);
  const auto& face = mesh->getFace(0);

  TestEdgeIterator<T> it(*mesh, face);
  const auto          start = it();

  ++it;
  ++it;
  REQUIRE(it() != start);

  it.reset();
  REQUIRE(it() == start);
  REQUIRE(it.ok());
}

TEMPLATE_TEST_CASE("EdgeIteratorT: copy and move both preserve iteration state",
                   "[DCEL][Iterator]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool        pool(hostMemoryResource());
  auto        mesh = buildTetrahedron<T>(pool);
  const auto& face = mesh->getFace(0);

  TestEdgeIterator<T> src(*mesh, face);
  ++src;
  const auto afterOneStep = src();

  TestEdgeIterator<T> copied(src);
  REQUIRE(copied() == afterOneStep);
  ++copied;
  REQUIRE(copied() != src()); // Independent copies: advancing one must not advance the other.

  TestEdgeIterator<T> copyAssigned(*mesh, face);
  copyAssigned = src;
  REQUIRE(copyAssigned() == afterOneStep);

  TestEdgeIterator<T> moved(std::move(src));
  REQUIRE(moved() == afterOneStep);

  TestEdgeIterator<T> moveAssigned(*mesh, face);
  moveAssigned = std::move(copied);
  REQUIRE(moveAssigned.ok());
}

// ─────────────────────────────────────────────────────────────────────────────
// MeshT tests
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("MeshT: a freshly constructed mesh is empty", "[DCEL][Mesh]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  TestMesh<T> mesh;

  REQUIRE(mesh.numVertices() == 0);
  REQUIRE(mesh.numEdges() == 0);
  REQUIRE(mesh.numFaces() == 0);
  REQUIRE(mesh.getAllVertexCoordinates().empty());

  // An empty mesh has no faces to measure a distance to.
  const auto inf = std::numeric_limits<T>::infinity();
  REQUIRE(mesh.signedDistance(Vec3T<T>(0, 0, 0)) == inf);
  REQUIRE(mesh.unsignedDistance2(Vec3T<T>(0, 0, 0)) == inf);
}

TEMPLATE_TEST_CASE("MeshT: reserveX/addX/getX/numX populate the mesh", "[DCEL][Mesh]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool        pool(hostMemoryResource());
  TestMesh<T> mesh;

  mesh.reserveVertices(pool, 1);
  mesh.reserveEdges(pool, 1);
  mesh.reserveFaces(pool, 1);

  const uint32_t vIdx = mesh.addVertex(pool, TestVertex<T>(Vec3T<T>(1, 2, 3)));
  const uint32_t eIdx = mesh.addEdge(pool, TestEdge<T>(0u));
  const uint32_t fIdx = mesh.addFace(pool, TestFace<T>(0u));

  REQUIRE(vIdx == 0);
  REQUIRE(eIdx == 0);
  REQUIRE(fIdx == 0);

  REQUIRE(mesh.numVertices() == 1);
  REQUIRE(mesh.numEdges() == 1);
  REQUIRE(mesh.numFaces() == 1);
  REQUIRE(mesh.getVertex(0).getPosition() == Vec3T<T>(1, 2, 3));
  REQUIRE(mesh.getEdge(0).getVertexIndex() == 0);
  REQUIRE(mesh.getFace(0).getHalfEdgeIndex() == 0);
}

TEMPLATE_TEST_CASE("MeshT: copy and move are both allowed, and the whole type is trivially copyable",
                   "[DCEL][Mesh]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;
  // Unlike the Pool-owning design this superseded, MeshT holds only plain values (three
  // PODVectors, the search algorithm, and a resolved base pointer) -- copying is a cheap,
  // always-safe descriptor copy that shares the underlying data (see deepCopy()'s docs).
  static_assert(std::is_copy_constructible_v<TestMesh<T>>);
  static_assert(std::is_copy_assignable_v<TestMesh<T>>);
  static_assert(std::is_move_constructible_v<TestMesh<T>>);
  static_assert(std::is_move_assignable_v<TestMesh<T>>);
  static_assert(std::is_trivially_copyable_v<TestMesh<T>>);
}

TEMPLATE_TEST_CASE("MeshT: move construction and move assignment transfer ownership",
                   "[DCEL][Mesh]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  auto       moveCtorSrc = buildTetrahedron<T>(pool);
  const auto v0Position  = moveCtorSrc->getVertex(0).getPosition();

  TestMesh<T> moved(std::move(*moveCtorSrc));
  REQUIRE(moved.numVertices() == 4);
  REQUIRE(moved.numFaces() == 4);
  REQUIRE(moved.getVertex(0).getPosition() == v0Position);

  auto        moveAssignSrc = buildTetrahedron<T>(pool);
  const auto  v0PositionB   = moveAssignSrc->getVertex(0).getPosition();
  TestMesh<T> moveAssignDst;
  moveAssignDst = std::move(*moveAssignSrc);
  REQUIRE(moveAssignDst.numVertices() == 4);
  REQUIRE(moveAssignDst.getVertex(0).getPosition() == v0PositionB);
}

TEMPLATE_TEST_CASE("MeshT: reconcile computes positive face areas and unit-length normals",
                   "[DCEL][Mesh]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());
  auto mesh = buildTetrahedron<T>(pool);

  for (uint32_t i = 0; i < mesh->numFaces(); i++) {
    const auto& f = mesh->getFace(i);
    REQUIRE(f.getArea() > T(0.0));
    REQUIRE_THAT(f.getNormal().length(), withinAbsT(T(1.0), exactMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("MeshT: flip negates all vertex, edge, and face normals", "[DCEL][Mesh]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());
  auto mesh = buildTetrahedron<T>(pool);

  std::vector<Vec3T<T>> vertexNormals, edgeNormals, faceNormals;
  for (uint32_t i = 0; i < mesh->numVertices(); i++) {
    vertexNormals.push_back(mesh->getVertex(i).getNormal());
  }
  for (uint32_t i = 0; i < mesh->numEdges(); i++) {
    edgeNormals.push_back(mesh->getEdge(i).getNormal());
  }
  for (uint32_t i = 0; i < mesh->numFaces(); i++) {
    faceNormals.push_back(mesh->getFace(i).getNormal());
  }

  mesh->flip();

  for (uint32_t i = 0; i < mesh->numVertices(); i++) {
    REQUIRE((mesh->getVertex(i).getNormal() - (-vertexNormals[i])).length() < T(exactMargin<T>()));
  }
  for (uint32_t i = 0; i < mesh->numEdges(); i++) {
    REQUIRE((mesh->getEdge(i).getNormal() - (-edgeNormals[i])).length() < T(exactMargin<T>()));
  }
  for (uint32_t i = 0; i < mesh->numFaces(); i++) {
    REQUIRE((mesh->getFace(i).getNormal() - (-faceNormals[i])).length() < T(exactMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("MeshT: signedDistance agrees between Direct and Direct2 search algorithms",
                   "[DCEL][Mesh]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());
  auto mesh = buildTetrahedron<T>(pool);

  const std::vector<Vec3T<T>> queryPoints = {{0.1, 0.1, 0.1}, {2.0, 2.0, 2.0}, {-1.0, -1.0, -1.0}, {0.25, 0.25, 0.0}};

  for (const auto& p : queryPoints) {
    const T distDirect  = mesh->signedDistance(p, MeshT<T, DefaultMetaData>::SearchAlgorithm::Direct);
    const T distDirect2 = mesh->signedDistance(p, MeshT<T, DefaultMetaData>::SearchAlgorithm::Direct2);

    REQUIRE_THAT(distDirect, withinAbsT(distDirect2, formulaMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("MeshT: setSearchAlgorithm changes the algorithm used by the no-argument overload",
                   "[DCEL][Mesh]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());
  auto mesh = buildTetrahedron<T>(pool);

  const Vec3T<T> p(0.1, 0.1, 0.1);

  mesh->setSearchAlgorithm(MeshT<T, DefaultMetaData>::SearchAlgorithm::Direct);
  const T distDirect = mesh->signedDistance(p);

  mesh->setSearchAlgorithm(MeshT<T, DefaultMetaData>::SearchAlgorithm::Direct2);
  const T distDirect2 = mesh->signedDistance(p);

  REQUIRE_THAT(distDirect, withinAbsT(distDirect2, formulaMargin<T>()));
}

TEMPLATE_TEST_CASE("MeshT: getAllVertexCoordinates matches the vertex positions",
                   "[DCEL][Mesh]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());
  auto mesh = buildTetrahedron<T>(pool);

  const auto coords = mesh->getAllVertexCoordinates();
  REQUIRE(coords.size() == mesh->numVertices());

  for (size_t i = 0; i < coords.size(); i++) {
    REQUIRE(coords[i] == mesh->getVertex(static_cast<uint32_t>(i)).getPosition());
  }
}

TEMPLATE_TEST_CASE("FaceT: getSmallestCoordinate/getHighestCoordinate bound the face's vertices",
                   "[DCEL][Face]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());
  auto mesh = buildTetrahedron<T>(pool);

  // Face 0 is {A,C,B} = (0,0,0), (0,1,0), (1,0,0) -- flat in the z=0 plane.
  REQUIRE(mesh->getFace(0).getSmallestCoordinate(*mesh) == Vec3T<T>(0.0, 0.0, 0.0));
  REQUIRE(mesh->getFace(0).getHighestCoordinate(*mesh) == Vec3T<T>(1.0, 1.0, 0.0));

  // Face 3 is the slant face {B,C,D} = (1,0,0), (0,1,0), (0,0,1).
  REQUIRE(mesh->getFace(3).getSmallestCoordinate(*mesh) == Vec3T<T>(0.0, 0.0, 0.0));
  REQUIRE(mesh->getFace(3).getHighestCoordinate(*mesh) == Vec3T<T>(1.0, 1.0, 1.0));

  // Both stream the half-edge loop rather than materializing a coordinate list (so that they stay
  // device-callable); tie that implementation back to the container-based source of truth for
  // every face, not just the two spot-checked above.
  for (uint32_t f = 0; f < mesh->numFaces(); f++) {
    const auto& face   = mesh->getFace(f);
    const auto  coords = face.getAllVertexCoordinates(*mesh);

    REQUIRE(!coords.empty());

    Vec3T<T> expectedLo = coords.front();
    Vec3T<T> expectedHi = coords.front();

    for (const auto& c : coords) {
      expectedLo = min(expectedLo, c);
      expectedHi = max(expectedHi, c);
    }

    REQUIRE(face.getSmallestCoordinate(*mesh) == expectedLo);
    REQUIRE(face.getHighestCoordinate(*mesh) == expectedHi);
  }
}

TEMPLATE_TEST_CASE("MeshT: deepCopy produces an independent mesh with the same geometry",
                   "[DCEL][Mesh]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool srcPool(hostMemoryResource());
  Pool dstPool(hostMemoryResource());

  auto mesh = buildTetrahedron<T>(srcPool);
  auto copy = mesh->deepCopy(dstPool);

  REQUIRE(copy != nullptr);
  REQUIRE(copy->numVertices() == mesh->numVertices());
  REQUIRE(copy->numEdges() == mesh->numEdges());
  REQUIRE(copy->numFaces() == mesh->numFaces());

  for (uint32_t i = 0; i < mesh->numFaces(); i++) {
    REQUIRE((copy->getFace(i).getNormal() - mesh->getFace(i).getNormal()).length() < T(exactMargin<T>()));
    REQUIRE_THAT(copy->getFace(i).getArea(), withinAbsT(mesh->getFace(i).getArea(), exactMargin<T>()));
  }

  // The copy must be usable for signed-distance queries (validates that the projection axes were
  // correctly rebuilt for every face). Done before mutating the copy, since the point-in-face test
  // projects the face's vertices on the fly and so reflects the live geometry.
  const Vec3T<T> p(0.1, 0.1, 0.1);
  REQUIRE_THAT(copy->signedDistance(p), withinAbsT(mesh->signedDistance(p), formulaMargin<T>()));

  // Mutating the copy must not affect the original -- this is the meaningful test of independence
  // now that vertices/edges/faces are plain values rather than shared_ptr-identified objects.
  copy->getVertex(0).setPosition(Vec3T<T>(999, 999, 999));
  REQUIRE(mesh->getVertex(0).getPosition() != Vec3T<T>(999, 999, 999));
}

TEMPLATE_TEST_CASE("MeshT: deepCopy preserves a prior flip() instead of silently re-deriving normals",
                   "[DCEL][Mesh]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool srcPool(hostMemoryResource());
  Pool dstPool(hostMemoryResource());

  auto mesh = buildTetrahedron<T>(srcPool);
  mesh->flip();

  auto copy = mesh->deepCopy(dstPool);

  for (uint32_t i = 0; i < mesh->numFaces(); i++) {
    REQUIRE((copy->getFace(i).getNormal() - mesh->getFace(i).getNormal()).length() < T(exactMargin<T>()));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Pool residency: a mesh stays queryable across everything its Pool does
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("MeshT: a mesh keeps answering after its Pool grows and moves the block",
                   "[DCEL][Mesh][PoolResidency]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // Start deliberately tiny so the reserves below are certain to grow the block.
  Pool pool(hostMemoryResource(), 64);

  auto mesh = buildTetrahedron<T>(pool);

  const Vec3T<T> p(0.1, 0.1, 0.1);

  const void* const baseBeforeGrow = pool.base();
  const T           before         = mesh->signedDistance(p);
  const Vec3T<T>    vertexBefore   = mesh->getVertex(0).getPosition();

  // Keep building into the same Pool. Under the old freeze-then-bind contract this was impossible:
  // querying required a frozen Pool, and reserving into a frozen Pool aborts.
  auto second = buildTetrahedron<T>(pool);

  REQUIRE(pool.base() != baseBeforeGrow); // the block really did move

  // Same answers, with no rebinding of any kind.
  REQUIRE_THAT(mesh->signedDistance(p), withinAbsT(before, exactMargin<T>()));
  REQUIRE(mesh->getVertex(0).getPosition() == vertexBefore);
  REQUIRE_THAT(second->signedDistance(p), withinAbsT(before, exactMargin<T>()));

  // Both meshes resolve through the same Pool, and know it.
  REQUIRE(mesh->isAttachedTo(pool));
  REQUIRE(second->isAttachedTo(pool));
}

TEMPLATE_TEST_CASE("MeshT: an element survives a growing reserve when carried by value",
                   "[DCEL][Mesh][PoolResidency]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // The reference-returning accessors hand back an address resolved at the moment of the call, so
  // a grow invalidates it (see the warning on Pool::reserve). This is the sanctioned way to carry
  // an element across a reserve: snapshot by value, then write back through a fresh accessor.
  Pool pool(hostMemoryResource(), 64);

  auto mesh = buildTetrahedron<T>(pool);

  VertexT<T, DefaultMetaData> vertex = mesh->getVertex(0); // by value, deliberately not by reference

  auto other = buildTetrahedron<T>(pool); // may grow, moving the block

  vertex.setPosition(Vec3T<T>(7, 8, 9));
  mesh->getVertex(0) = vertex; // fresh resolution against the current base

  REQUIRE(mesh->getVertex(0).getPosition() == Vec3T<T>(7, 8, 9));

  // The write landed in this mesh only.
  REQUIRE(other->getVertex(0).getPosition() != Vec3T<T>(7, 8, 9));
}

TEMPLATE_TEST_CASE("MeshT: a mesh survives its Pool being moved, including a vector reallocation",
                   "[DCEL][Mesh][PoolResidency]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // A mesh points at its Pool's control block, not at the Pool object, so relocating the Pool is
  // invisible to it. The vector case is the sharp one: the source Pool is moved *and destroyed*.
  std::vector<Pool> pools;
  pools.reserve(1);
  pools.emplace_back(hostMemoryResource());

  auto mesh = buildTetrahedron<T>(pools[0]);

  const Vec3T<T> p(0.1, 0.1, 0.1);
  const T        before = mesh->signedDistance(p);

  while (pools.size() < 16) {
    pools.emplace_back(hostMemoryResource());
  }

  REQUIRE_THAT(mesh->signedDistance(p), withinAbsT(before, exactMargin<T>()));
  REQUIRE(mesh->isAttachedTo(pools[0]));

  // And again after a plain move-construction.
  Pool moved(std::move(pools[0]));

  REQUIRE_THAT(mesh->signedDistance(p), withinAbsT(before, exactMargin<T>()));
  REQUIRE(mesh->isAttachedTo(moved));
}

TEMPLATE_TEST_CASE("MeshT: deepCopy into the same Pool is safe even when it grows",
                   "[DCEL][Mesh][PoolResidency]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // deepCopy reserves three times from the destination Pool while reading the source. With the
  // source and destination being the same Pool, those reserves move the very block being read
  // from -- safe only because every read re-resolves through the control block.
  Pool pool(hostMemoryResource(), 64);

  auto mesh = buildTetrahedron<T>(pool);
  auto copy = mesh->deepCopy(pool);

  REQUIRE(copy->numVertices() == mesh->numVertices());
  REQUIRE(copy->numFaces() == mesh->numFaces());

  const Vec3T<T> p(0.1, 0.1, 0.1);
  REQUIRE_THAT(copy->signedDistance(p), withinAbsT(mesh->signedDistance(p), formulaMargin<T>()));

  for (uint32_t i = 0; i < mesh->numVertices(); i++) {
    REQUIRE(copy->getVertex(i).getPosition() == mesh->getVertex(i).getPosition());
  }
}

TEMPLATE_TEST_CASE("MeshT: rebasedView onto a host-to-host mirror answers identically",
                   "[DCEL][Mesh][PoolResidency]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // The offset/rebase proof applied to real geometry rather than a synthetic struct, and without a
  // GPU: mirroring host-to-host produces a byte-identical block at a different address, and the
  // same descriptor must resolve against it unchanged.
  Pool pool(hostMemoryResource());

  auto mesh = buildTetrahedron<T>(pool);

  pool.freeze();

  Pool mirrored = Pool::mirror(pool, hostMemoryResource());

  REQUIRE(mirrored.base() != pool.base());
  REQUIRE(mirrored.mirrorOf() == pool.id());

  const auto view = mesh->rebasedView(mirrored);

  REQUIRE(view.isAttachedTo(mirrored));
  REQUIRE(view.numVertices() == mesh->numVertices());
  REQUIRE(view.numFaces() == mesh->numFaces());

  const Vec3T<T> p(0.1, 0.1, 0.1);
  REQUIRE_THAT(view.signedDistance(p), withinAbsT(mesh->signedDistance(p), exactMargin<T>()));

  for (uint32_t i = 0; i < mesh->numVertices(); i++) {
    REQUIRE(view.getVertex(i).getPosition() == mesh->getVertex(i).getPosition());
  }

  // The mirror is an independent copy: corrupting the source must not disturb the view.
  std::memset(pool.base(), 0, pool.usedBytes());

  REQUIRE_THAT(view.signedDistance(p), withinAbsT(mesh->rebasedView(mirrored).signedDistance(p), exactMargin<T>()));
}

// ─────────────────────────────────────────────────────────────────────────────
// Soup tests
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Soup::containsDegeneratePolygons returns false for a valid triangle soup",
                   "[Soup]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T                                       = TestType;
  const std::vector<Vec3T<T>>            verts  = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
  const std::vector<std::vector<size_t>> facets = {{0, 1, 2}};

  REQUIRE_FALSE(Soup::containsDegeneratePolygons(verts, facets));
}

TEMPLATE_TEST_CASE("Soup::containsDegeneratePolygons detects a facet with too few vertices",
                   "[Soup]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T                                       = TestType;
  const std::vector<Vec3T<T>>            verts  = {{0, 0, 0}, {1, 0, 0}};
  const std::vector<std::vector<size_t>> facets = {{0, 1}};

  REQUIRE(Soup::containsDegeneratePolygons(verts, facets));
}

TEMPLATE_TEST_CASE("Soup::containsDegeneratePolygons detects a facet with a repeated vertex",
                   "[Soup]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;
  // Facet lists the same vertex index twice -- degenerate even though it has 3 entries.
  const std::vector<Vec3T<T>>            verts  = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
  const std::vector<std::vector<size_t>> facets = {{0, 1, 1}};

  REQUIRE(Soup::containsDegeneratePolygons(verts, facets));
}

TEMPLATE_TEST_CASE("Soup::compress removes duplicate vertices and reindexes facets consistently",
                   "[Soup]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;
  // Two triangles sharing an edge, each listing its own copies of the shared vertices A=(0,0,0)
  // and B=(1,0,0). After compression, both facets must reference the SAME compressed indices for
  // the shared vertices.
  std::vector<Vec3T<T>> verts = {
    {0, 0, 0}, // 0 = A (triangle 1)
    {1, 0, 0}, // 1 = B (triangle 1)
    {0, 1, 0}, // 2 = C (triangle 1)
    {1, 0, 0}, // 3 = B (triangle 2, duplicate of 1)
    {0, 0, 0}, // 4 = A (triangle 2, duplicate of 0)
    {0, -1, 0} // 5 = D (triangle 2)
  };
  std::vector<std::vector<size_t>> facets = {{0, 1, 2}, {3, 4, 5}};

  Soup::compress(verts, facets);

  REQUIRE(verts.size() == 4); // A, B, C, D -- each unique position appears once.
  REQUIRE(facets.size() == 2);
  REQUIRE(facets[0][0] == facets[1][1]); // Both facets' A-reference now points at the same index.
  REQUIRE(facets[0][1] == facets[1][0]); // Both facets' B-reference now points at the same index.
}

TEMPLATE_TEST_CASE("Soup::compress on empty input clears the facet list", "[Soup]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T                                 = TestType;
  std::vector<Vec3T<T>>            verts  = {};
  std::vector<std::vector<size_t>> facets = {{0, 1, 2}};

  Soup::compress(verts, facets);

  REQUIRE(verts.empty());
  REQUIRE(facets.empty());
}

TEMPLATE_TEST_CASE("Soup::soupToDCEL builds correct vertex, edge, and face counts",
                   "[Soup]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());
  auto mesh = buildTetrahedron<T>(pool);

  REQUIRE(mesh->numVertices() == 4);
  REQUIRE(mesh->numFaces() == 4);
  REQUIRE(mesh->numEdges() == 12); // 4 triangular faces * 3 half-edges each.
}

TEMPLATE_TEST_CASE("Soup::soupToDCEL reconciles every half-edge's pair edge on a closed mesh",
                   "[Soup]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());
  auto mesh = buildTetrahedron<T>(pool);

  for (uint32_t i = 0; i < mesh->numEdges(); i++) {
    REQUIRE(mesh->getEdge(i).getPairEdgeIndex() != UINT32_MAX);
    REQUIRE(mesh->getEdge(mesh->getEdge(i).getPairEdgeIndex()).getPairEdgeIndex() == i); // Pairing must be symmetric.
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Structural tests
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("DCEL: tetrahedron loads without error", "[DCEL]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());
  auto mesh = loadTetrahedron<T>(pool);
  REQUIRE(mesh != nullptr);
}

TEMPLATE_TEST_CASE("Parser::readIntoDCEL returns a valid, empty mesh (not nullptr) for an "
                   "unrecognized file extension",
                   "[DCEL][Parser]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());
  auto mesh = Parser::readIntoDCEL<T>(g_dataDir + "/tetrahedron.unsupported-extension", pool);

  REQUIRE(mesh != nullptr);
  REQUIRE(mesh->numVertices() == 0);
  REQUIRE(mesh->numEdges() == 0);
  REQUIRE(mesh->numFaces() == 0);

  const auto inf = std::numeric_limits<T>::infinity();
  REQUIRE(mesh->signedDistance(Vec3T<T>(0, 0, 0)) == inf);
  REQUIRE(mesh->unsignedDistance2(Vec3T<T>(0, 0, 0)) == inf);
}

TEMPLATE_TEST_CASE("DCEL: tetrahedron has correct face and vertex counts", "[DCEL]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());
  auto mesh = loadTetrahedron<T>(pool);
  REQUIRE(mesh != nullptr);

  // A tetrahedron has 4 faces and 4 vertices
  REQUIRE(mesh->numFaces() == 4);
  REQUIRE(mesh->numVertices() == 4);

  // 4 faces × 3 half-edges each = 12 half-edges
  REQUIRE(mesh->numEdges() == 12);
}

TEMPLATE_TEST_CASE("DCEL: all faces have a half-edge", "[DCEL]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());
  auto mesh = loadTetrahedron<T>(pool);
  REQUIRE(mesh != nullptr);

  for (uint32_t i = 0; i < mesh->numFaces(); i++) {
    REQUIRE(mesh->getFace(i).getHalfEdgeIndex() != UINT32_MAX);
  }
}

TEMPLATE_TEST_CASE("DCEL: all half-edges have a pair and a face", "[DCEL]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());
  auto mesh = loadTetrahedron<T>(pool);
  REQUIRE(mesh != nullptr);

  for (uint32_t i = 0; i < mesh->numEdges(); i++) {
    REQUIRE(mesh->getEdge(i).getPairEdgeIndex() != UINT32_MAX);
    REQUIRE(mesh->getEdge(i).getFaceIndex() != UINT32_MAX);
  }
}

TEMPLATE_TEST_CASE("DCEL: all face normals are unit-length", "[DCEL]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());
  auto mesh = loadTetrahedron<T>(pool);
  REQUIRE(mesh != nullptr);

  for (uint32_t i = 0; i < mesh->numFaces(); i++) {
    REQUIRE_THAT(mesh->getFace(i).getNormal().length(), withinAbsT(T(1.0), formulaMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("DCEL: sanityCheck completes without crashing", "[DCEL]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());
  auto mesh = loadTetrahedron<T>(pool);
  REQUIRE(mesh != nullptr);

  // sanityCheck() returns void; it prints to stderr only if the mesh has errors.
  // For a well-formed tetrahedron it should complete without aborting.
  REQUIRE_NOTHROW(mesh->sanityCheck("tetrahedron"));
}

// ─────────────────────────────────────────────────────────────────────────────
// Signed-distance tests using MeshSDF (brute-force O(N) evaluator)
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("MeshSDF: tetrahedron signed distances", "[DCEL][MeshSDF]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;
  // Tetrahedron vertices: (0,0,0), (1,0,0), (0,1,0), (0,0,1).
  // Standard SDF convention: negative inside, positive outside.

  Pool pool(hostMemoryResource());
  auto mesh = loadTetrahedron<T>(pool);
  REQUIRE(mesh != nullptr);

  TestMeshSDF<T> sdf(mesh, pool, BVH::Build::SAH);

  SECTION("centroid is inside (SDF < 0)")
  {
    const T d = sdf.signedDistance(Vec3T<T>(0.25, 0.25, 0.25));
    REQUIRE(d < T(0.0));
  }

  SECTION("exterior point has positive SDF")
  {
    const T d = sdf.signedDistance(Vec3T<T>(2.0, 2.0, 2.0));
    REQUIRE(d > T(0.0));
  }

  SECTION("point on a face centroid is near-zero")
  {
    // Bottom face (z=0): vertices (0,0,0),(0,1,0),(1,0,0), centroid = (1/3, 1/3, 0)
    const T d = sdf.signedDistance(Vec3T<T>(1.0 / 3.0, 1.0 / 3.0, 0.0));
    REQUIRE_THAT(d, withinAbsT(T(0.0), formulaMargin<T>()));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Sign-convention regression: hard-coded DCEL, no file I/O.
// If the cross-product in computeNormal is ever accidentally inverted this
// test will immediately flip from pass to fail.
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("DCEL sign convention: exterior point has positive SDF", "[DCEL][sign]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool           pool(hostMemoryResource());
  auto           mesh = buildTetrahedron<T>(pool);
  TestMeshSDF<T> sdf(mesh, pool, BVH::Build::SAH);

  // Far outside: must be positive.
  REQUIRE(sdf.signedDistance(Vec3T<T>(2.0, 2.0, 2.0)) > T(0.0));
  // Outside along each axis.
  REQUIRE(sdf.signedDistance(Vec3T<T>(-1.0, 0.0, 0.0)) > T(0.0));
  REQUIRE(sdf.signedDistance(Vec3T<T>(0.0, -1.0, 0.0)) > T(0.0));
  REQUIRE(sdf.signedDistance(Vec3T<T>(0.0, 0.0, -1.0)) > T(0.0));
}

TEMPLATE_TEST_CASE("DCEL sign convention: interior point has negative SDF", "[DCEL][sign]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool           pool(hostMemoryResource());
  auto           mesh = buildTetrahedron<T>(pool);
  TestMeshSDF<T> sdf(mesh, pool, BVH::Build::SAH);

  // Centroid of the tetrahedron is clearly inside.
  REQUIRE(sdf.signedDistance(Vec3T<T>(0.25, 0.25, 0.25)) < T(0.0));
}

// ─────────────────────────────────────────────────────────────────────────────
// FastTriMeshSDF (SIMD-accelerated BVH)
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("FastTriMeshSDF: matches MeshSDF for tetrahedron",
                   "[DCEL][FastTriMeshSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T                = TestType;
  const std::string path = g_dataDir + "/tetrahedron.stl";

  Pool pool(hostMemoryResource());
  auto fast = Parser::readIntoTriangleBVH<T>(path, pool);
  auto mesh = Parser::readIntoMesh<T>(path, pool);

  REQUIRE(fast != nullptr);
  REQUIRE(mesh != nullptr);

  // Compare a handful of query points.  Near-zero values use WithinAbs.
  const std::vector<Vec3T<T>> queries = {
    {0.25, 0.25, 0.25}, // centroid (inside)
    {2.0, 2.0, 2.0},    // far outside
    {-1.0, 0.0, 0.0},   // outside along -x
  };

  for (const auto& q : queries) {
    const T dBrute = mesh->signedDistance(q);
    const T dFast  = fast->signedDistance(q);
    REQUIRE_THAT(dFast, WithinRel(dBrute, T(traversalMargin<T>())));
  }

  // Edge point: both should be near-zero
  const T dEdgeBrute = mesh->signedDistance(Vec3T<T>(0.5, 0.0, 0.0));
  const T dEdgeFast  = fast->signedDistance(Vec3T<T>(0.5, 0.0, 0.0));
  REQUIRE_THAT(dEdgeFast, withinAbsT(dEdgeBrute, traversalMargin<T>()));
}

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
// ─────────────────────────────────────────────────────────────────────────────
// Device: the DCEL query surface (MeshT/VertexT/EdgeT/FaceT/EdgeIteratorT) is
// callable from a kernel and matches the host
// ─────────────────────────────────────────────────────────────────────────────

template <class T>
EBGEOMETRY_GLOBAL
void
dcelDeviceKernel(TestMesh<T> a_mesh, Vec3T<T> a_point, T* a_out)
{
  const TestFace<T>& face = a_mesh.getFace(0);

  uint32_t edgeCount = 0;

  for (TestEdgeIterator<T> it(a_mesh, face); it.ok(); ++it) {
    ++edgeCount;
  }

  const T vertexTerm = a_mesh.getVertex(1).getPosition().length();

  // getSmallestCoordinate/getHighestCoordinate reduce componentwise over the half-edge loop
  // without materializing a std::vector, which is what keeps them callable from here.
  const T bboxTerm = (face.getHighestCoordinate(a_mesh) - face.getSmallestCoordinate(a_mesh)).length();

  const T baseTerms =
    face.signedDistance(a_point, a_mesh) + a_mesh.unsignedDistance2(a_point) + T(edgeCount) + vertexTerm + bboxTerm;

  // MeshT's own public, device-callable signedDistance() (the algorithm-dispatching entry point --
  // Direct2 by default -- that used to be EBGEOMETRY_HOST-only because of a std::cerr call in its
  // switch's defensive default case; see the class-level note in EBGeometry_DCEL_Mesh.hpp).
  const T meshSignedDist = a_mesh.signedDistance(a_point);

  // setInsideOutsideAlgorithm() plus the WindingNumber/SubtendedAngle branches of
  // FaceT::isPointInsideFace (buildTetrahedron leaves every face at the default CrossingNumber,
  // already exercised by the calls above).
  a_mesh.setInsideOutsideAlgorithm(InsideOutsideAlgorithm::WindingNumber);
  const T windingDist = face.signedDistance(a_point, a_mesh);

  a_mesh.setInsideOutsideAlgorithm(InsideOutsideAlgorithm::SubtendedAngle);
  const T subtendedDist = face.signedDistance(a_point, a_mesh);

  // flip(): negating every normal must exactly negate signedDistance for the same point (the
  // inside/outside classification itself is magnitude-based, so it is unaffected by which
  // InsideOutsideAlgorithm is currently selected) -- self-verifying, no separate host expectation
  // needed for this term beyond "it sums to zero".
  const T preFlipDist = face.signedDistance(a_point, a_mesh);
  a_mesh.flip();
  const T postFlipDist    = face.signedDistance(a_point, a_mesh);
  const T flipConsistency = preFlipDist + postFlipDist;

  a_out[0] = baseTerms + meshSignedDist + windingDist + subtendedDist + flipConsistency;
}

TEMPLATE_TEST_CASE("MeshT/VertexT/EdgeT/FaceT/EdgeIteratorT: device query surface matches the host",
                   "[DCEL][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  Pool hostPool(hostMemoryResource());
  auto mesh = buildTetrahedron<T>(hostPool);

  const Vec3T<T> point(2.0, 2.0, 2.0);

  const TestFace<T>& face      = mesh->getFace(0);
  const uint32_t     edgeCount = 3; // every face built by buildTetrahedron is a triangle
  const T            bboxTerm  = (face.getHighestCoordinate(*mesh) - face.getSmallestCoordinate(*mesh)).length();
  const T            baseTerms = face.signedDistance(point, *mesh) + mesh->unsignedDistance2(point) + T(edgeCount) +
                      mesh->getVertex(1).getPosition().length() + bboxTerm;

  const T meshSignedDist = mesh->signedDistance(point);

  // Mirror now, while hostPool is still in its pristine (CrossingNumber, unflipped) state: the
  // device kernel runs the exact same WindingNumber/SubtendedAngle/flip() progression below,
  // starting from this same snapshot, so mutating hostPool afterward (to compute the matching host
  // expectation) does not need to -- and must not -- touch the already-mirrored device copy.
  Pool devicePool = Pool::mirror(hostPool, deviceMemoryResource());

  mesh->setInsideOutsideAlgorithm(InsideOutsideAlgorithm::WindingNumber);
  const T windingDist = mesh->getFace(0).signedDistance(point, *mesh);

  mesh->setInsideOutsideAlgorithm(InsideOutsideAlgorithm::SubtendedAngle);
  const T subtendedDist = mesh->getFace(0).signedDistance(point, *mesh);

  // Mirrors the kernel's own flip() self-consistency check: this term is mathematically ~0
  // regardless of host/device, so it is included in hostVal purely so a broken flip() (device-side
  // or, in principle, host-side) shows up as a real mismatch rather than being masked by summation.
  const T preFlipDist = mesh->getFace(0).signedDistance(point, *mesh);
  mesh->flip();
  const T postFlipDist    = mesh->getFace(0).signedDistance(point, *mesh);
  const T flipConsistency = preFlipDist + postFlipDist;

  const T hostVal = baseTerms + meshSignedDist + windingDist + subtendedDist + flipConsistency;

  const TestMesh<T> deviceView = mesh->rebasedView(devicePool);

  DeviceBuffer<T> deviceOut;

  dcelDeviceKernel<T><<<1, 1>>>(deviceView, point, deviceOut.get());
  (void)GPU::deviceSynchronize();

  REQUIRE_THAT(readScalar(deviceOut.get()), WithinRel(hostVal, gpuTol<T>()));
}
#endif
