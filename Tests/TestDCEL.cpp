// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"

#include <cstdint>
#include <string>
#include <type_traits>

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
loadTetrahedron()
{
  return Parser::readIntoDCEL<T>(g_dataDir + "/tetrahedron.stl");
}

// Build a tetrahedron DCEL mesh entirely from hard-coded data (no file I/O).
// Vertices: A=(0,0,0) B=(1,0,0) C=(0,1,0) D=(0,0,1).
// Face winding is CCW-from-outside so that the new (x2-x0)×(x2-x1) formula
// yields outward normals — standard SDF convention: negative inside, positive
// outside.
template <class T>
static std::shared_ptr<MeshT<T, DefaultMetaData>>
buildTetrahedron()
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
  Soup::soupToDCEL(*mesh, verts, facets, "tetrahedron-hard");
  mesh->reconcile();

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

  TestMesh<T> mesh;
  mesh.getFaces().emplace_back();
  mesh.getFaces()[0].define(Vec3T<T>(0, 0, 1), UINT32_MAX);
  mesh.getEdges().emplace_back();
  mesh.getEdges()[0].setFace(0);

  auto& e = mesh.getEdges()[0];
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

  TestMesh<T> mesh;
  mesh.getFaces().emplace_back();
  mesh.getFaces()[0].define(Vec3T<T>(1, 0, 0), UINT32_MAX);
  mesh.getEdges().emplace_back();
  mesh.getEdges()[0].setFace(0);

  REQUIRE(mesh.getEdges()[0].computeNormal(mesh) == Vec3T<T>(1, 0, 0));
}

TEMPLATE_TEST_CASE("EdgeT: computeNormal averages both incident faces' normals",
                   "[DCEL][Edge]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  TestMesh<T> mesh;
  auto&       faces = mesh.getFaces();
  auto&       edges = mesh.getEdges();

  faces.emplace_back();
  faces[0].define(Vec3T<T>(1, 0, 0), UINT32_MAX);
  faces.emplace_back();
  faces[1].define(Vec3T<T>(0, 1, 0), UINT32_MAX);

  edges.emplace_back(); // edge under test
  edges.emplace_back(); // its pair edge

  edges[0].setFace(0);
  edges[1].setFace(1);
  edges[0].setPairEdge(1);

  const Vec3T<T> expected = Vec3T<T>(1, 1, 0) / Vec3T<T>(1, 1, 0).length();
  const Vec3T<T> actual   = edges[0].computeNormal(mesh);

  REQUIRE_THAT(actual[0], withinAbsT(expected[0], exactMargin<T>()));
  REQUIRE_THAT(actual[1], withinAbsT(expected[1], exactMargin<T>()));
  REQUIRE_THAT(actual[2], withinAbsT(expected[2], exactMargin<T>()));
}

TEMPLATE_TEST_CASE("EdgeT: reconcile stores computeNormal's result", "[DCEL][Edge]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  TestMesh<T> mesh;
  mesh.getFaces().emplace_back();
  mesh.getFaces()[0].define(Vec3T<T>(0, 1, 0), UINT32_MAX);
  mesh.getEdges().emplace_back();
  mesh.getEdges()[0].setFace(0);

  auto& e = mesh.getEdges()[0];
  e.reconcile(mesh);

  REQUIRE(e.getNormal() == Vec3T<T>(0, 1, 0));
}

TEMPLATE_TEST_CASE("EdgeT: signedDistance and unsignedDistance2 on a simple segment",
                   "[DCEL][Edge]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  TestMesh<T> mesh;
  auto&       vertices = mesh.getVertices();
  auto&       edges    = mesh.getEdges();
  auto&       faces    = mesh.getFaces();

  // Build a two-vertex chain: e0 (start v0) -> e1 (start v1), so e0's "other vertex" is v1.
  vertices.emplace_back(Vec3T<T>(0, 0, 0));
  vertices.emplace_back(Vec3T<T>(1, 0, 0));

  edges.emplace_back(0u);
  edges.emplace_back(1u);
  edges[0].setNextEdge(1);

  // Outward normal perpendicular to the edge, pointing +y.
  faces.emplace_back();
  faces[0].define(Vec3T<T>(0, 1, 0), UINT32_MAX);
  edges[0].setFace(0);
  edges[0].reconcile(mesh);

  REQUIRE(edges[0].getOtherVertex(mesh).getPosition() == vertices[1].getPosition());

  // Point above the middle of the edge: on the normal side, distance 2.
  REQUIRE_THAT(edges[0].signedDistance(Vec3T<T>(0.5, 2, 0), mesh), WithinRel(T(2.0)));

  // Point below the middle of the edge: against the normal, negative distance.
  REQUIRE_THAT(edges[0].signedDistance(Vec3T<T>(0.5, -2, 0), mesh), WithinRel(T(-2.0)));

  // unsignedDistance2 clamps the projection to the segment.
  REQUIRE_THAT(edges[0].unsignedDistance2(Vec3T<T>(0.5, 3, 0), mesh), WithinRel(T(9.0)));
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

  auto        mesh = buildTetrahedron<T>();
  const auto& face = mesh->getFaces()[0];

  std::vector<uint32_t> visited;
  for (TestEdgeIterator<T> it(*mesh, face); it.ok(); ++it) {
    REQUIRE(mesh->getEdges()[it()].getFaceIndex() == 0);
    visited.push_back(it());
  }

  REQUIRE(visited.size() == 3); // Every face in a tetrahedron is a triangle.
  REQUIRE(visited[0] == face.getHalfEdgeIndex());
  REQUIRE(mesh->getEdges()[visited[0]].getNextEdgeIndex() == visited[1]);
  REQUIRE(mesh->getEdges()[visited[1]].getNextEdgeIndex() == visited[2]);
  REQUIRE(mesh->getEdges()[visited[2]].getNextEdgeIndex() == visited[0]); // Loops back to the start.
}

TEMPLATE_TEST_CASE("EdgeIteratorT: constructing from an edge index directly matches constructing from its face",
                   "[DCEL][Iterator]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  auto        mesh = buildTetrahedron<T>();
  const auto& face = mesh->getFaces()[0];

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

  auto        mesh = buildTetrahedron<T>();
  const auto& face = mesh->getFaces()[0];

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

  auto        mesh = buildTetrahedron<T>();
  const auto& face = mesh->getFaces()[0];

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

TEMPLATE_TEST_CASE("MeshT: default construction is empty", "[DCEL][Mesh]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;
  TestMesh<T> mesh;

  REQUIRE(mesh.getVertices().empty());
  REQUIRE(mesh.getEdges().empty());
  REQUIRE(mesh.getFaces().empty());
  REQUIRE(mesh.getAllVertexCoordinates().empty());

  // An empty mesh has no faces to measure a distance to.
  const auto inf = std::numeric_limits<T>::infinity();
  REQUIRE(mesh.signedDistance(Vec3T<T>(0, 0, 0)) == inf);
  REQUIRE(mesh.unsignedDistance2(Vec3T<T>(0, 0, 0)) == inf);
}

TEMPLATE_TEST_CASE("MeshT: full constructor assigns vertices, edges, and faces",
                   "[DCEL][Mesh]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  std::vector<TestVertex<T>> verts = {TestVertex<T>(Vec3T<T>(1, 2, 3))};
  std::vector<TestEdge<T>>   edges = {TestEdge<T>(0u)};
  std::vector<TestFace<T>>   faces = {TestFace<T>(0u)};

  TestMesh<T> mesh(faces, edges, verts);

  REQUIRE(mesh.getVertices().size() == 1);
  REQUIRE(mesh.getEdges().size() == 1);
  REQUIRE(mesh.getFaces().size() == 1);
  REQUIRE(mesh.getVertices()[0].getPosition() == Vec3T<T>(1, 2, 3));
  REQUIRE(mesh.getEdges()[0].getVertexIndex() == 0);
  REQUIRE(mesh.getFaces()[0].getHalfEdgeIndex() == 0);
}

TEMPLATE_TEST_CASE("MeshT: copy is disallowed, move is allowed", "[DCEL][Mesh]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;
  static_assert(!std::is_copy_constructible_v<TestMesh<T>>);
  static_assert(!std::is_copy_assignable_v<TestMesh<T>>);
  static_assert(std::is_move_constructible_v<TestMesh<T>>);
  static_assert(std::is_move_assignable_v<TestMesh<T>>);
}

TEMPLATE_TEST_CASE("MeshT: move construction and move assignment transfer ownership",
                   "[DCEL][Mesh]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  auto       moveCtorSrc = buildTetrahedron<T>();
  const auto v0Position  = moveCtorSrc->getVertices()[0].getPosition();

  TestMesh<T> moved(std::move(*moveCtorSrc));
  REQUIRE(moved.getVertices().size() == 4);
  REQUIRE(moved.getFaces().size() == 4);
  REQUIRE(moved.getVertices()[0].getPosition() == v0Position);

  auto        moveAssignSrc = buildTetrahedron<T>();
  const auto  v0PositionB   = moveAssignSrc->getVertices()[0].getPosition();
  TestMesh<T> moveAssignDst;
  moveAssignDst = std::move(*moveAssignSrc);
  REQUIRE(moveAssignDst.getVertices().size() == 4);
  REQUIRE(moveAssignDst.getVertices()[0].getPosition() == v0PositionB);
}

TEMPLATE_TEST_CASE("MeshT: reconcile computes positive face areas and unit-length normals",
                   "[DCEL][Mesh]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T   = TestType;
  auto mesh = buildTetrahedron<T>();

  for (const auto& f : mesh->getFaces()) {
    REQUIRE(f.getArea() > T(0.0));
    REQUIRE_THAT(f.getNormal().length(), withinAbsT(T(1.0), exactMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("MeshT: flip negates all vertex, edge, and face normals", "[DCEL][Mesh]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T   = TestType;
  auto mesh = buildTetrahedron<T>();

  std::vector<Vec3T<T>> vertexNormals, edgeNormals, faceNormals;
  for (const auto& v : mesh->getVertices()) {
    vertexNormals.push_back(v.getNormal());
  }
  for (const auto& e : mesh->getEdges()) {
    edgeNormals.push_back(e.getNormal());
  }
  for (const auto& f : mesh->getFaces()) {
    faceNormals.push_back(f.getNormal());
  }

  mesh->flip();

  for (size_t i = 0; i < mesh->getVertices().size(); i++) {
    REQUIRE((mesh->getVertices()[i].getNormal() - (-vertexNormals[i])).length() < T(exactMargin<T>()));
  }
  for (size_t i = 0; i < mesh->getEdges().size(); i++) {
    REQUIRE((mesh->getEdges()[i].getNormal() - (-edgeNormals[i])).length() < T(exactMargin<T>()));
  }
  for (size_t i = 0; i < mesh->getFaces().size(); i++) {
    REQUIRE((mesh->getFaces()[i].getNormal() - (-faceNormals[i])).length() < T(exactMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("MeshT: signedDistance agrees between Direct and Direct2 search algorithms",
                   "[DCEL][Mesh]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T   = TestType;
  auto mesh = buildTetrahedron<T>();

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
  using T   = TestType;
  auto mesh = buildTetrahedron<T>();

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
  using T   = TestType;
  auto mesh = buildTetrahedron<T>();

  const auto coords = mesh->getAllVertexCoordinates();
  REQUIRE(coords.size() == mesh->getVertices().size());

  for (size_t i = 0; i < coords.size(); i++) {
    REQUIRE(coords[i] == mesh->getVertices()[i].getPosition());
  }
}

TEMPLATE_TEST_CASE("MeshT: deepCopy produces an independent mesh with the same geometry",
                   "[DCEL][Mesh]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T   = TestType;
  auto mesh = buildTetrahedron<T>();
  auto copy = mesh->deepCopy();

  REQUIRE(copy != nullptr);
  REQUIRE(copy->getVertices().size() == mesh->getVertices().size());
  REQUIRE(copy->getEdges().size() == mesh->getEdges().size());
  REQUIRE(copy->getFaces().size() == mesh->getFaces().size());

  for (size_t i = 0; i < mesh->getFaces().size(); i++) {
    REQUIRE((copy->getFaces()[i].getNormal() - mesh->getFaces()[i].getNormal()).length() < T(exactMargin<T>()));
    REQUIRE_THAT(copy->getFaces()[i].getArea(), withinAbsT(mesh->getFaces()[i].getArea(), exactMargin<T>()));
  }

  // The copy must be usable for signed-distance queries (validates that the projection axes were
  // correctly rebuilt for every face). Done before mutating the copy, since the point-in-face test
  // projects the face's vertices on the fly and so reflects the live geometry.
  const Vec3T<T> p(0.1, 0.1, 0.1);
  REQUIRE_THAT(copy->signedDistance(p), withinAbsT(mesh->signedDistance(p), formulaMargin<T>()));

  // Mutating the copy must not affect the original -- this is the meaningful test of independence
  // now that vertices/edges/faces are plain values rather than shared_ptr-identified objects.
  copy->getVertices()[0].setPosition(Vec3T<T>(999, 999, 999));
  REQUIRE(mesh->getVertices()[0].getPosition() != Vec3T<T>(999, 999, 999));
}

TEMPLATE_TEST_CASE("MeshT: deepCopy preserves a prior flip() instead of silently re-deriving normals",
                   "[DCEL][Mesh]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T   = TestType;
  auto mesh = buildTetrahedron<T>();
  mesh->flip();

  auto copy = mesh->deepCopy();

  for (size_t i = 0; i < mesh->getFaces().size(); i++) {
    REQUIRE((copy->getFaces()[i].getNormal() - mesh->getFaces()[i].getNormal()).length() < T(exactMargin<T>()));
  }
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
  using T   = TestType;
  auto mesh = buildTetrahedron<T>();

  REQUIRE(mesh->getVertices().size() == 4);
  REQUIRE(mesh->getFaces().size() == 4);
  REQUIRE(mesh->getEdges().size() == 12); // 4 triangular faces * 3 half-edges each.
}

TEMPLATE_TEST_CASE("Soup::soupToDCEL reconciles every half-edge's pair edge on a closed mesh",
                   "[Soup]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T   = TestType;
  auto mesh = buildTetrahedron<T>();

  const auto& edges = mesh->getEdges();
  for (uint32_t i = 0; i < edges.size(); i++) {
    REQUIRE(edges[i].getPairEdgeIndex() != UINT32_MAX);
    REQUIRE(edges[edges[i].getPairEdgeIndex()].getPairEdgeIndex() == i); // Pairing must be symmetric.
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Structural tests
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("DCEL: tetrahedron loads without error", "[DCEL]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T   = TestType;
  auto mesh = loadTetrahedron<T>();
  REQUIRE(mesh != nullptr);
}

TEMPLATE_TEST_CASE("DCEL: tetrahedron has correct face and vertex counts", "[DCEL]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T   = TestType;
  auto mesh = loadTetrahedron<T>();
  REQUIRE(mesh != nullptr);

  // A tetrahedron has 4 faces and 4 vertices
  REQUIRE(mesh->getFaces().size() == 4);
  REQUIRE(mesh->getVertices().size() == 4);

  // 4 faces × 3 half-edges each = 12 half-edges
  REQUIRE(mesh->getEdges().size() == 12);
}

TEMPLATE_TEST_CASE("DCEL: all faces have a half-edge", "[DCEL]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T   = TestType;
  auto mesh = loadTetrahedron<T>();
  REQUIRE(mesh != nullptr);

  for (const auto& face : mesh->getFaces()) {
    REQUIRE(face.getHalfEdgeIndex() != UINT32_MAX);
  }
}

TEMPLATE_TEST_CASE("DCEL: all half-edges have a pair and a face", "[DCEL]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T   = TestType;
  auto mesh = loadTetrahedron<T>();
  REQUIRE(mesh != nullptr);

  for (const auto& edge : mesh->getEdges()) {
    REQUIRE(edge.getPairEdgeIndex() != UINT32_MAX);
    REQUIRE(edge.getFaceIndex() != UINT32_MAX);
  }
}

TEMPLATE_TEST_CASE("DCEL: all face normals are unit-length", "[DCEL]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T   = TestType;
  auto mesh = loadTetrahedron<T>();
  REQUIRE(mesh != nullptr);

  for (const auto& face : mesh->getFaces()) {
    REQUIRE_THAT(face.getNormal().length(), withinAbsT(T(1.0), formulaMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("DCEL: sanityCheck completes without crashing", "[DCEL]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T   = TestType;
  auto mesh = loadTetrahedron<T>();
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

  auto mesh = loadTetrahedron<T>();
  REQUIRE(mesh != nullptr);

  TestMeshSDF<T> sdf(mesh, BVH::Build::SAH);

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
  using T             = TestType;
  auto           mesh = buildTetrahedron<T>();
  TestMeshSDF<T> sdf(mesh, BVH::Build::SAH);

  // Far outside: must be positive.
  REQUIRE(sdf.signedDistance(Vec3T<T>(2.0, 2.0, 2.0)) > T(0.0));
  // Outside along each axis.
  REQUIRE(sdf.signedDistance(Vec3T<T>(-1.0, 0.0, 0.0)) > T(0.0));
  REQUIRE(sdf.signedDistance(Vec3T<T>(0.0, -1.0, 0.0)) > T(0.0));
  REQUIRE(sdf.signedDistance(Vec3T<T>(0.0, 0.0, -1.0)) > T(0.0));
}

TEMPLATE_TEST_CASE("DCEL sign convention: interior point has negative SDF", "[DCEL][sign]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T             = TestType;
  auto           mesh = buildTetrahedron<T>();
  TestMeshSDF<T> sdf(mesh, BVH::Build::SAH);

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
  auto              fast = Parser::readIntoTriangleBVH<T>(path);
  auto              mesh = Parser::readIntoMesh<T>(path);

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
