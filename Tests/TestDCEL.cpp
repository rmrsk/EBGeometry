// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"

#include <string>
#include <type_traits>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using namespace EBGeometry::DCEL;
using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

// MeshSDF<T, Meta, K> under test, with the mesh's own metadata type and the
// SIMD-optimal branching factor for double precision.
using TestMeshSDF = MeshSDF<double, DefaultMetaData, BVH::DefaultBranchingRatio<double>()>;

// Path to test data injected by CMake.
static const std::string g_dataDir = EBGEOMETRY_TEST_DATA_DIR;

// ─────────────────────────────────────────────────────────────────────────────
// Helper: load the test tetrahedron as a DCEL mesh
// ─────────────────────────────────────────────────────────────────────────────

static std::shared_ptr<MeshT<double, DefaultMetaData>>
loadTetrahedron()
{
  return Parser::readIntoDCEL<double>(g_dataDir + "/tetrahedron.stl");
}

// Build a tetrahedron DCEL mesh entirely from hard-coded data (no file I/O).
// Vertices: A=(0,0,0) B=(1,0,0) C=(0,1,0) D=(0,0,1).
// Face winding is CCW-from-outside so that the new (x2-x0)×(x2-x1) formula
// yields outward normals — standard SDF convention: negative inside, positive
// outside.
static std::shared_ptr<MeshT<double, DefaultMetaData>>
buildTetrahedron()
{
  std::vector<Vec3T<double>> verts = {
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

  auto mesh = std::make_shared<MeshT<double, DefaultMetaData>>();
  Soup::soupToDCEL(*mesh, verts, facets, "tetrahedron-hard");
  mesh->reconcile();

  return mesh;
}

// ─────────────────────────────────────────────────────────────────────────────
// VertexT tests
// ─────────────────────────────────────────────────────────────────────────────

using TestVertex = VertexT<double, DefaultMetaData>;
using TestFace   = FaceT<double, DefaultMetaData>;
using TestEdge   = EdgeT<double, DefaultMetaData>;
using TestMesh   = MeshT<double, DefaultMetaData>;

TEST_CASE("VertexT: default construction leaves defined, zeroed state", "[DCEL][Vertex]")
{
  TestVertex v;

  REQUIRE(v.getPosition() == Vec3T<double>::zeros());
  REQUIRE(v.getNormal() == Vec3T<double>::zeros());
  REQUIRE(v.getOutgoingEdge() == nullptr);
  REQUIRE(v.getFaces().empty());
  REQUIRE(v.getMetaData() == 0);
}

TEST_CASE("VertexT: position-only and position+normal constructors", "[DCEL][Vertex]")
{
  const Vec3T<double> pos(1, 2, 3);
  const Vec3T<double> normal(0, 0, 1);

  TestVertex vPosOnly(pos);
  REQUIRE(vPosOnly.getPosition() == pos);
  REQUIRE(vPosOnly.getNormal() == Vec3T<double>::zeros());
  REQUIRE(vPosOnly.getFaces().empty());

  TestVertex vBoth(pos, normal);
  REQUIRE(vBoth.getPosition() == pos);
  REQUIRE(vBoth.getNormal() == normal);
}

TEST_CASE("VertexT: setPosition, setNormal, setEdge, define", "[DCEL][Vertex]")
{
  TestVertex v;
  auto       edge = std::make_shared<TestEdge>();

  v.setPosition(Vec3T<double>(4, 5, 6));
  REQUIRE(v.getPosition() == Vec3T<double>(4, 5, 6));

  v.setNormal(Vec3T<double>(1, 0, 0));
  REQUIRE(v.getNormal() == Vec3T<double>(1, 0, 0));

  v.setEdge(edge);
  REQUIRE(v.getOutgoingEdge() == edge);

  // Setting the edge pointer to nullptr is explicitly valid.
  v.setEdge(nullptr);
  REQUIRE(v.getOutgoingEdge() == nullptr);

  v.define(Vec3T<double>(7, 8, 9), edge, Vec3T<double>(0, 1, 0));
  REQUIRE(v.getPosition() == Vec3T<double>(7, 8, 9));
  REQUIRE(v.getOutgoingEdge() == edge);
  REQUIRE(v.getNormal() == Vec3T<double>(0, 1, 0));
}

TEST_CASE("VertexT: addFace appends to the face list", "[DCEL][Vertex]")
{
  TestVertex v;
  auto       face = std::make_shared<TestFace>();

  REQUIRE(v.getFaces().empty());
  v.addFace(face);
  REQUIRE(v.getFaces().size() == 1);
  REQUIRE(v.getFaces()[0] == face);
}

TEST_CASE("VertexT: normalizeNormalVector produces a unit vector", "[DCEL][Vertex]")
{
  TestVertex v;
  v.setNormal(Vec3T<double>(3, 0, 0));
  v.normalizeNormalVector();

  REQUIRE_THAT(v.getNormal().length(), WithinAbs(1.0, 1.e-12));
  REQUIRE_THAT(v.getNormal()[0], WithinAbs(1.0, 1.e-12));
}

TEST_CASE("VertexT: flipNormal negates the normal vector", "[DCEL][Vertex]")
{
  TestVertex v(Vec3T<double>::zeros(), Vec3T<double>(1, 0, 0));
  v.flipNormal();

  REQUIRE(v.getNormal() == Vec3T<double>(-1, 0, 0));
}

TEST_CASE("VertexT: signedDistance and unsignedDistance2", "[DCEL][Vertex]")
{
  TestVertex v(Vec3T<double>(0, 0, 0), Vec3T<double>(0, 0, 1));

  // Point along the outward normal: positive distance.
  REQUIRE_THAT(v.signedDistance(Vec3T<double>(0, 0, 2)), WithinRel(2.0));

  // Point against the outward normal: negative distance.
  REQUIRE_THAT(v.signedDistance(Vec3T<double>(0, 0, -2)), WithinRel(-2.0));

  REQUIRE_THAT(v.unsignedDistance2(Vec3T<double>(3, 0, 0)), WithinRel(9.0));
}

TEST_CASE("VertexT: getMetaData reads and writes", "[DCEL][Vertex]")
{
  TestVertex v;
  REQUIRE(v.getMetaData() == 0);

  v.getMetaData() = 5;
  REQUIRE(v.getMetaData() == 5);

  v.setMetaData(9);
  REQUIRE(v.getMetaData() == 9);
}

TEST_CASE("VertexT: copy construction only copies position, normal, and outgoing edge", "[DCEL][Vertex]")
{
  auto face = std::make_shared<TestFace>();
  auto edge = std::make_shared<TestEdge>();

  TestVertex src(Vec3T<double>(1, 2, 3), Vec3T<double>(0, 0, 1));
  src.setEdge(edge);
  src.addFace(face);
  src.getMetaData() = 42;

  TestVertex copy(src);

  REQUIRE(copy.getPosition() == src.getPosition());
  REQUIRE(copy.getNormal() == src.getNormal());
  REQUIRE(copy.getOutgoingEdge() == edge);
  REQUIRE(copy.getFaces().empty());
  REQUIRE(copy.getMetaData() == 0);
}

TEST_CASE("VertexT: copy assignment has the same narrow semantics as copy construction", "[DCEL][Vertex]")
{
  auto face = std::make_shared<TestFace>();
  auto edge = std::make_shared<TestEdge>();

  TestVertex src(Vec3T<double>(1, 2, 3), Vec3T<double>(0, 0, 1));
  src.setEdge(edge);
  src.addFace(face);
  src.getMetaData() = 42;

  TestVertex dst;
  dst = src;

  REQUIRE(dst.getPosition() == src.getPosition());
  REQUIRE(dst.getNormal() == src.getNormal());
  REQUIRE(dst.getOutgoingEdge() == edge);
  REQUIRE(dst.getFaces().empty());
  REQUIRE(dst.getMetaData() == 0);
}

TEST_CASE("VertexT: move construction and move assignment transfer the entire state", "[DCEL][Vertex]")
{
  auto face = std::make_shared<TestFace>();
  auto edge = std::make_shared<TestEdge>();

  TestVertex moveCtorSrc(Vec3T<double>(1, 2, 3), Vec3T<double>(0, 0, 1));
  moveCtorSrc.setEdge(edge);
  moveCtorSrc.addFace(face);
  moveCtorSrc.getMetaData() = 42;

  TestVertex moved(std::move(moveCtorSrc));
  REQUIRE(moved.getPosition() == Vec3T<double>(1, 2, 3));
  REQUIRE(moved.getOutgoingEdge() == edge);
  REQUIRE(moved.getFaces().size() == 1);
  REQUIRE(moved.getMetaData() == 42);

  TestVertex moveAssignSrc(Vec3T<double>(4, 5, 6), Vec3T<double>(1, 0, 0));
  moveAssignSrc.setEdge(edge);
  moveAssignSrc.addFace(face);
  moveAssignSrc.getMetaData() = 7;

  TestVertex moveAssignDst;
  moveAssignDst = std::move(moveAssignSrc);
  REQUIRE(moveAssignDst.getPosition() == Vec3T<double>(4, 5, 6));
  REQUIRE(moveAssignDst.getOutgoingEdge() == edge);
  REQUIRE(moveAssignDst.getFaces().size() == 1);
  REQUIRE(moveAssignDst.getMetaData() == 7);
}

// ─────────────────────────────────────────────────────────────────────────────
// EdgeT tests
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("EdgeT: default construction leaves defined, zeroed state", "[DCEL][Edge]")
{
  TestEdge e;

  REQUIRE(e.getNormal() == Vec3T<double>::zeros());
  REQUIRE(e.getVertex() == nullptr);
  REQUIRE(e.getPairEdge() == nullptr);
  REQUIRE(e.getNextEdge() == nullptr);
  REQUIRE(e.getFace() == nullptr);
  REQUIRE(e.getMetaData() == 0);
  REQUIRE(e.size() == 2);
}

TEST_CASE("EdgeT: partial (vertex) constructor sets only the starting vertex", "[DCEL][Edge]")
{
  auto v = std::make_shared<TestVertex>(Vec3T<double>(1, 2, 3));

  TestEdge e(v);
  REQUIRE(e.getVertex() == v);
  REQUIRE(e.getPairEdge() == nullptr);
  REQUIRE(e.getNextEdge() == nullptr);
  REQUIRE(e.getFace() == nullptr);
  REQUIRE(e.getNormal() == Vec3T<double>::zeros());
}

TEST_CASE("EdgeT: define, setVertex, setPairEdge, setNextEdge, setFace, setMetaData", "[DCEL][Edge]")
{
  auto v1   = std::make_shared<TestVertex>();
  auto pair = std::make_shared<TestEdge>();
  auto next = std::make_shared<TestEdge>();
  auto face = std::make_shared<TestFace>();

  TestEdge e;
  e.define(v1, pair, next);
  REQUIRE(e.getVertex() == v1);
  REQUIRE(e.getPairEdge() == pair);
  REQUIRE(e.getNextEdge() == next);

  e.setFace(face);
  REQUIRE(e.getFace() == face);

  e.setMetaData(9);
  REQUIRE(e.getMetaData() == 9);

  // Setting pointers to nullptr is explicitly valid.
  e.setVertex(nullptr);
  e.setPairEdge(nullptr);
  e.setNextEdge(nullptr);
  e.setFace(nullptr);
  REQUIRE(e.getVertex() == nullptr);
  REQUIRE(e.getPairEdge() == nullptr);
  REQUIRE(e.getNextEdge() == nullptr);
  REQUIRE(e.getFace() == nullptr);
}

TEST_CASE("EdgeT: flipNormal negates the normal vector", "[DCEL][Edge]")
{
  auto face = std::make_shared<TestFace>();
  face->define(Vec3T<double>(0, 0, 1), nullptr);

  TestEdge e;
  e.setFace(face);
  e.reconcile();

  REQUIRE(e.getNormal() == Vec3T<double>(0, 0, 1));
  e.flipNormal();
  REQUIRE(e.getNormal() == Vec3T<double>(0, 0, -1));
}

TEST_CASE("EdgeT: computeNormal with a single face returns that face's normal", "[DCEL][Edge]")
{
  auto face = std::make_shared<TestFace>();
  face->define(Vec3T<double>(1, 0, 0), nullptr);

  TestEdge e;
  e.setFace(face);

  REQUIRE(e.computeNormal() == Vec3T<double>(1, 0, 0));
}

TEST_CASE("EdgeT: computeNormal averages both incident faces' normals", "[DCEL][Edge]")
{
  auto face     = std::make_shared<TestFace>();
  auto pairFace = std::make_shared<TestFace>();
  face->define(Vec3T<double>(1, 0, 0), nullptr);
  pairFace->define(Vec3T<double>(0, 1, 0), nullptr);

  auto pairEdge = std::make_shared<TestEdge>();
  pairEdge->setFace(pairFace);

  TestEdge e;
  e.setFace(face);
  e.setPairEdge(pairEdge);

  const Vec3T<double> expected = Vec3T<double>(1, 1, 0) / Vec3T<double>(1, 1, 0).length();
  const Vec3T<double> actual   = e.computeNormal();

  REQUIRE_THAT(actual[0], WithinAbs(expected[0], 1.e-12));
  REQUIRE_THAT(actual[1], WithinAbs(expected[1], 1.e-12));
  REQUIRE_THAT(actual[2], WithinAbs(expected[2], 1.e-12));
}

TEST_CASE("EdgeT: reconcile stores computeNormal's result", "[DCEL][Edge]")
{
  auto face = std::make_shared<TestFace>();
  face->define(Vec3T<double>(0, 1, 0), nullptr);

  TestEdge e;
  e.setFace(face);
  e.reconcile();

  REQUIRE(e.getNormal() == Vec3T<double>(0, 1, 0));
}

TEST_CASE("EdgeT: signedDistance and unsignedDistance2 on a simple segment", "[DCEL][Edge]")
{
  // Build a two-vertex chain: e1 (start v1) -> e2 (start v2), so e1's "other vertex" is v2.
  auto v1 = std::make_shared<TestVertex>(Vec3T<double>(0, 0, 0));
  auto v2 = std::make_shared<TestVertex>(Vec3T<double>(1, 0, 0));

  auto e1 = std::make_shared<TestEdge>();
  auto e2 = std::make_shared<TestEdge>();
  e1->define(v1, nullptr, e2);
  e2->setVertex(v2);

  // Outward normal perpendicular to the edge, pointing +y.
  auto face = std::make_shared<TestFace>();
  face->define(Vec3T<double>(0, 1, 0), nullptr);
  e1->setFace(face);
  e1->reconcile();

  REQUIRE(e1->getOtherVertex() == v2);

  // Point above the middle of the edge: on the normal side, distance 2.
  REQUIRE_THAT(e1->signedDistance(Vec3T<double>(0.5, 2, 0)), WithinRel(2.0));

  // Point below the middle of the edge: against the normal, negative distance.
  REQUIRE_THAT(e1->signedDistance(Vec3T<double>(0.5, -2, 0)), WithinRel(-2.0));

  // unsignedDistance2 clamps the projection to the segment.
  REQUIRE_THAT(e1->unsignedDistance2(Vec3T<double>(0.5, 3, 0)), WithinRel(9.0));
}

TEST_CASE("EdgeT: copy construction copies topology pointers but not meta-data", "[DCEL][Edge]")
{
  auto v    = std::make_shared<TestVertex>();
  auto pair = std::make_shared<TestEdge>();
  auto next = std::make_shared<TestEdge>();
  auto face = std::make_shared<TestFace>();

  TestEdge src;
  src.define(v, pair, next);
  src.setFace(face);
  src.setMetaData(42);

  TestEdge copy(src);
  REQUIRE(copy.getVertex() == v);
  REQUIRE(copy.getPairEdge() == pair);
  REQUIRE(copy.getNextEdge() == next);
  REQUIRE(copy.getFace() == face);
  REQUIRE(copy.getMetaData() == 0);
}

TEST_CASE("EdgeT: copy assignment has the same semantics as copy construction", "[DCEL][Edge]")
{
  auto v    = std::make_shared<TestVertex>();
  auto pair = std::make_shared<TestEdge>();
  auto next = std::make_shared<TestEdge>();
  auto face = std::make_shared<TestFace>();

  TestEdge src;
  src.define(v, pair, next);
  src.setFace(face);
  src.setMetaData(42);

  TestEdge dst;
  dst = src;

  REQUIRE(dst.getVertex() == v);
  REQUIRE(dst.getPairEdge() == pair);
  REQUIRE(dst.getNextEdge() == next);
  REQUIRE(dst.getFace() == face);
  REQUIRE(dst.getMetaData() == 0);
}

TEST_CASE("EdgeT: move construction and move assignment transfer the entire state", "[DCEL][Edge]")
{
  auto v    = std::make_shared<TestVertex>();
  auto pair = std::make_shared<TestEdge>();
  auto next = std::make_shared<TestEdge>();
  auto face = std::make_shared<TestFace>();

  TestEdge moveCtorSrc;
  moveCtorSrc.define(v, pair, next);
  moveCtorSrc.setFace(face);
  moveCtorSrc.setMetaData(42);

  TestEdge moved(std::move(moveCtorSrc));
  REQUIRE(moved.getVertex() == v);
  REQUIRE(moved.getPairEdge() == pair);
  REQUIRE(moved.getNextEdge() == next);
  REQUIRE(moved.getFace() == face);
  REQUIRE(moved.getMetaData() == 42);

  TestEdge moveAssignSrc;
  moveAssignSrc.define(v, pair, next);
  moveAssignSrc.setFace(face);
  moveAssignSrc.setMetaData(7);

  TestEdge moveAssignDst;
  moveAssignDst = std::move(moveAssignSrc);
  REQUIRE(moveAssignDst.getVertex() == v);
  REQUIRE(moveAssignDst.getPairEdge() == pair);
  REQUIRE(moveAssignDst.getNextEdge() == next);
  REQUIRE(moveAssignDst.getFace() == face);
  REQUIRE(moveAssignDst.getMetaData() == 7);
}

// ─────────────────────────────────────────────────────────────────────────────
// EdgeIteratorT tests
// ─────────────────────────────────────────────────────────────────────────────

using TestEdgeIterator = EdgeIteratorT<double, DefaultMetaData>;

TEST_CASE("EdgeIteratorT: iterating a face visits exactly its own half-edges and loops back", "[DCEL][Iterator]")
{
  auto mesh = buildTetrahedron();
  auto face = mesh->getFaces()[0];

  std::vector<std::shared_ptr<TestEdge>> visited;
  for (TestEdgeIterator it(*face); it.ok(); ++it) {
    REQUIRE(it()->getFace() == face);
    visited.push_back(it());
  }

  REQUIRE(visited.size() == 3); // Every face in a tetrahedron is a triangle.
  REQUIRE(visited[0] == face->getHalfEdge());
  REQUIRE(visited[0]->getNextEdge() == visited[1]);
  REQUIRE(visited[1]->getNextEdge() == visited[2]);
  REQUIRE(visited[2]->getNextEdge() == visited[0]); // Loops back to the start.
}

TEST_CASE("EdgeIteratorT: constructing from an edge directly matches constructing from its face", "[DCEL][Iterator]")
{
  auto mesh = buildTetrahedron();
  auto face = mesh->getFaces()[0];

  std::vector<std::shared_ptr<TestEdge>> fromFace;
  for (TestEdgeIterator it(*face); it.ok(); ++it) {
    fromFace.push_back(it());
  }

  std::vector<std::shared_ptr<TestEdge>> fromEdge;
  for (TestEdgeIterator it(face->getHalfEdge()); it.ok(); ++it) {
    fromEdge.push_back(it());
  }

  REQUIRE(fromFace == fromEdge);
}

TEST_CASE("EdgeIteratorT: ok() is immediately false for a null starting edge", "[DCEL][Iterator]")
{
  TestEdgeIterator it(std::shared_ptr<TestEdge>(nullptr));
  REQUIRE_FALSE(it.ok());
}

TEST_CASE("EdgeIteratorT: reset returns the iterator to its starting edge", "[DCEL][Iterator]")
{
  auto mesh = buildTetrahedron();
  auto face = mesh->getFaces()[0];

  TestEdgeIterator it(*face);
  const auto       start = it();

  ++it;
  ++it;
  REQUIRE(it() != start);

  it.reset();
  REQUIRE(it() == start);
  REQUIRE(it.ok());
}

TEST_CASE("EdgeIteratorT: copy and move both preserve iteration state", "[DCEL][Iterator]")
{
  auto mesh = buildTetrahedron();
  auto face = mesh->getFaces()[0];

  TestEdgeIterator src(*face);
  ++src;
  const auto afterOneStep = src();

  TestEdgeIterator copied(src);
  REQUIRE(copied() == afterOneStep);
  ++copied;
  REQUIRE(copied() != src()); // Independent copies: advancing one must not advance the other.

  TestEdgeIterator copyAssigned(*face);
  copyAssigned = src;
  REQUIRE(copyAssigned() == afterOneStep);

  TestEdgeIterator moved(std::move(src));
  REQUIRE(moved() == afterOneStep);

  TestEdgeIterator moveAssigned(*face);
  moveAssigned = std::move(copied);
  REQUIRE(moveAssigned.ok());
}

// ─────────────────────────────────────────────────────────────────────────────
// MeshT tests
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("MeshT: default construction is empty", "[DCEL][Mesh]")
{
  TestMesh mesh;

  REQUIRE(mesh.getVertices().empty());
  REQUIRE(mesh.getEdges().empty());
  REQUIRE(mesh.getFaces().empty());
  REQUIRE(mesh.getAllVertexCoordinates().empty());

  // An empty mesh has no faces to measure a distance to.
  const auto inf = std::numeric_limits<double>::infinity();
  REQUIRE(mesh.signedDistance(Vec3T<double>(0, 0, 0)) == inf);
  REQUIRE(mesh.unsignedDistance2(Vec3T<double>(0, 0, 0)) == inf);
}

TEST_CASE("MeshT: full constructor assigns vertices, edges, and faces", "[DCEL][Mesh]")
{
  std::vector<std::shared_ptr<TestVertex>> verts = {std::make_shared<TestVertex>()};
  std::vector<std::shared_ptr<TestEdge>>   edges = {std::make_shared<TestEdge>()};
  std::vector<std::shared_ptr<TestFace>>   faces = {std::make_shared<TestFace>()};

  TestMesh mesh(faces, edges, verts);

  REQUIRE(mesh.getVertices().size() == 1);
  REQUIRE(mesh.getEdges().size() == 1);
  REQUIRE(mesh.getFaces().size() == 1);
  REQUIRE(mesh.getVertices()[0] == verts[0]);
  REQUIRE(mesh.getEdges()[0] == edges[0]);
  REQUIRE(mesh.getFaces()[0] == faces[0]);
}

TEST_CASE("MeshT: copy is disallowed, move is allowed", "[DCEL][Mesh]")
{
  static_assert(!std::is_copy_constructible_v<TestMesh>);
  static_assert(!std::is_copy_assignable_v<TestMesh>);
  static_assert(std::is_move_constructible_v<TestMesh>);
  static_assert(std::is_move_assignable_v<TestMesh>);
}

TEST_CASE("MeshT: move construction and move assignment transfer ownership", "[DCEL][Mesh]")
{
  auto moveCtorSrc = buildTetrahedron();
  auto v0          = moveCtorSrc->getVertices()[0];

  TestMesh moved(std::move(*moveCtorSrc));
  REQUIRE(moved.getVertices().size() == 4);
  REQUIRE(moved.getFaces().size() == 4);
  REQUIRE(moved.getVertices()[0] == v0);

  auto     moveAssignSrc = buildTetrahedron();
  auto     v0b           = moveAssignSrc->getVertices()[0];
  TestMesh moveAssignDst;
  moveAssignDst = std::move(*moveAssignSrc);
  REQUIRE(moveAssignDst.getVertices().size() == 4);
  REQUIRE(moveAssignDst.getVertices()[0] == v0b);
}

TEST_CASE("MeshT: reconcile computes positive face areas and unit-length normals", "[DCEL][Mesh]")
{
  auto mesh = buildTetrahedron();

  for (const auto& f : mesh->getFaces()) {
    REQUIRE(f->getArea() > 0.0);
    REQUIRE_THAT(f->getNormal().length(), WithinAbs(1.0, 1.e-12));
  }
}

TEST_CASE("MeshT: flip negates all vertex, edge, and face normals", "[DCEL][Mesh]")
{
  auto mesh = buildTetrahedron();

  std::vector<Vec3T<double>> vertexNormals, edgeNormals, faceNormals;
  for (const auto& v : mesh->getVertices()) {
    vertexNormals.push_back(v->getNormal());
  }
  for (const auto& e : mesh->getEdges()) {
    edgeNormals.push_back(e->getNormal());
  }
  for (const auto& f : mesh->getFaces()) {
    faceNormals.push_back(f->getNormal());
  }

  mesh->flip();

  for (size_t i = 0; i < mesh->getVertices().size(); i++) {
    REQUIRE((mesh->getVertices()[i]->getNormal() - (-vertexNormals[i])).length() < 1.e-12);
  }
  for (size_t i = 0; i < mesh->getEdges().size(); i++) {
    REQUIRE((mesh->getEdges()[i]->getNormal() - (-edgeNormals[i])).length() < 1.e-12);
  }
  for (size_t i = 0; i < mesh->getFaces().size(); i++) {
    REQUIRE((mesh->getFaces()[i]->getNormal() - (-faceNormals[i])).length() < 1.e-12);
  }
}

TEST_CASE("MeshT: signedDistance agrees between Direct and Direct2 search algorithms", "[DCEL][Mesh]")
{
  auto mesh = buildTetrahedron();

  const std::vector<Vec3T<double>> queryPoints = {
    {0.1, 0.1, 0.1}, {2.0, 2.0, 2.0}, {-1.0, -1.0, -1.0}, {0.25, 0.25, 0.0}};

  for (const auto& p : queryPoints) {
    const double distDirect  = mesh->signedDistance(p, MeshT<double, DefaultMetaData>::SearchAlgorithm::Direct);
    const double distDirect2 = mesh->signedDistance(p, MeshT<double, DefaultMetaData>::SearchAlgorithm::Direct2);

    REQUIRE_THAT(distDirect, WithinAbs(distDirect2, 1.e-10));
  }
}

TEST_CASE("MeshT: setSearchAlgorithm changes the algorithm used by the no-argument overload", "[DCEL][Mesh]")
{
  auto mesh = buildTetrahedron();

  const Vec3T<double> p(0.1, 0.1, 0.1);

  mesh->setSearchAlgorithm(MeshT<double, DefaultMetaData>::SearchAlgorithm::Direct);
  const double distDirect = mesh->signedDistance(p);

  mesh->setSearchAlgorithm(MeshT<double, DefaultMetaData>::SearchAlgorithm::Direct2);
  const double distDirect2 = mesh->signedDistance(p);

  REQUIRE_THAT(distDirect, WithinAbs(distDirect2, 1.e-10));
}

TEST_CASE("MeshT: getAllVertexCoordinates matches the vertex positions", "[DCEL][Mesh]")
{
  auto mesh = buildTetrahedron();

  const auto coords = mesh->getAllVertexCoordinates();
  REQUIRE(coords.size() == mesh->getVertices().size());

  for (size_t i = 0; i < coords.size(); i++) {
    REQUIRE(coords[i] == mesh->getVertices()[i]->getPosition());
  }
}

TEST_CASE("MeshT: deepCopy produces an independent mesh with the same geometry", "[DCEL][Mesh]")
{
  auto mesh = buildTetrahedron();
  auto copy = mesh->deepCopy();

  REQUIRE(copy != nullptr);
  REQUIRE(copy->getVertices().size() == mesh->getVertices().size());
  REQUIRE(copy->getEdges().size() == mesh->getEdges().size());
  REQUIRE(copy->getFaces().size() == mesh->getFaces().size());

  // Topology must be fully decoupled: no shared vertex/edge/face pointers.
  for (size_t i = 0; i < mesh->getVertices().size(); i++) {
    REQUIRE(copy->getVertices()[i] != mesh->getVertices()[i]);
  }
  for (size_t i = 0; i < mesh->getFaces().size(); i++) {
    REQUIRE(copy->getFaces()[i] != mesh->getFaces()[i]);
    REQUIRE((copy->getFaces()[i]->getNormal() - mesh->getFaces()[i]->getNormal()).length() < 1.e-12);
    REQUIRE_THAT(copy->getFaces()[i]->getArea(), WithinAbs(mesh->getFaces()[i]->getArea(), 1.e-12));
  }

  // Mutating the copy must not affect the original.
  copy->getVertices()[0]->setPosition(Vec3T<double>(999, 999, 999));
  REQUIRE(mesh->getVertices()[0]->getPosition() != Vec3T<double>(999, 999, 999));

  // The copy must still be usable for signed-distance queries (validates that the 2D embedding
  // was correctly rebuilt for every face).
  const Vec3T<double> p(0.1, 0.1, 0.1);
  REQUIRE_THAT(copy->signedDistance(p), WithinAbs(mesh->signedDistance(p), 1.e-10));
}

TEST_CASE("MeshT: deepCopy preserves a prior flip() instead of silently re-deriving normals", "[DCEL][Mesh]")
{
  auto mesh = buildTetrahedron();
  mesh->flip();

  auto copy = mesh->deepCopy();

  for (size_t i = 0; i < mesh->getFaces().size(); i++) {
    REQUIRE((copy->getFaces()[i]->getNormal() - mesh->getFaces()[i]->getNormal()).length() < 1.e-12);
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Soup tests
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("Soup::containsDegeneratePolygons returns false for a valid triangle soup", "[Soup]")
{
  const std::vector<Vec3T<double>>       verts  = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
  const std::vector<std::vector<size_t>> facets = {{0, 1, 2}};

  REQUIRE_FALSE(Soup::containsDegeneratePolygons(verts, facets));
}

TEST_CASE("Soup::containsDegeneratePolygons detects a facet with too few vertices", "[Soup]")
{
  const std::vector<Vec3T<double>>       verts  = {{0, 0, 0}, {1, 0, 0}};
  const std::vector<std::vector<size_t>> facets = {{0, 1}};

  REQUIRE(Soup::containsDegeneratePolygons(verts, facets));
}

TEST_CASE("Soup::containsDegeneratePolygons detects a facet with a repeated vertex", "[Soup]")
{
  // Facet lists the same vertex index twice -- degenerate even though it has 3 entries.
  const std::vector<Vec3T<double>>       verts  = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
  const std::vector<std::vector<size_t>> facets = {{0, 1, 1}};

  REQUIRE(Soup::containsDegeneratePolygons(verts, facets));
}

TEST_CASE("Soup::compress removes duplicate vertices and reindexes facets consistently", "[Soup]")
{
  // Two triangles sharing an edge, each listing its own copies of the shared vertices A=(0,0,0)
  // and B=(1,0,0). After compression, both facets must reference the SAME compressed indices for
  // the shared vertices.
  std::vector<Vec3T<double>> verts = {
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

TEST_CASE("Soup::compress on empty input clears the facet list", "[Soup]")
{
  std::vector<Vec3T<double>>       verts  = {};
  std::vector<std::vector<size_t>> facets = {{0, 1, 2}};

  Soup::compress(verts, facets);

  REQUIRE(verts.empty());
  REQUIRE(facets.empty());
}

TEST_CASE("Soup::soupToDCEL builds correct vertex, edge, and face counts", "[Soup]")
{
  auto mesh = buildTetrahedron();

  REQUIRE(mesh->getVertices().size() == 4);
  REQUIRE(mesh->getFaces().size() == 4);
  REQUIRE(mesh->getEdges().size() == 12); // 4 triangular faces * 3 half-edges each.
}

TEST_CASE("Soup::soupToDCEL reconciles every half-edge's pair edge on a closed mesh", "[Soup]")
{
  auto mesh = buildTetrahedron();

  for (const auto& e : mesh->getEdges()) {
    REQUIRE(e->getPairEdge() != nullptr);
    REQUIRE(e->getPairEdge()->getPairEdge() == e); // Pairing must be symmetric.
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Structural tests
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("DCEL: tetrahedron loads without error", "[DCEL]")
{
  auto mesh = loadTetrahedron();
  REQUIRE(mesh != nullptr);
}

TEST_CASE("DCEL: tetrahedron has correct face and vertex counts", "[DCEL]")
{
  auto mesh = loadTetrahedron();
  REQUIRE(mesh != nullptr);

  // A tetrahedron has 4 faces and 4 vertices
  REQUIRE(mesh->getFaces().size() == 4);
  REQUIRE(mesh->getVertices().size() == 4);

  // 4 faces × 3 half-edges each = 12 half-edges
  REQUIRE(mesh->getEdges().size() == 12);
}

TEST_CASE("DCEL: all faces have a half-edge", "[DCEL]")
{
  auto mesh = loadTetrahedron();
  REQUIRE(mesh != nullptr);

  for (const auto& face : mesh->getFaces()) {
    REQUIRE(face != nullptr);
    REQUIRE(face->getHalfEdge() != nullptr);
  }
}

TEST_CASE("DCEL: all half-edges have a pair and a face", "[DCEL]")
{
  auto mesh = loadTetrahedron();
  REQUIRE(mesh != nullptr);

  for (const auto& edge : mesh->getEdges()) {
    REQUIRE(edge != nullptr);
    REQUIRE(edge->getPairEdge() != nullptr);
    REQUIRE(edge->getFace() != nullptr);
  }
}

TEST_CASE("DCEL: all face normals are unit-length", "[DCEL]")
{
  auto mesh = loadTetrahedron();
  REQUIRE(mesh != nullptr);

  for (const auto& face : mesh->getFaces()) {
    REQUIRE_THAT(face->getNormal().length(), WithinAbs(1.0, 1e-10));
  }
}

TEST_CASE("DCEL: sanityCheck completes without crashing", "[DCEL]")
{
  auto mesh = loadTetrahedron();
  REQUIRE(mesh != nullptr);

  // sanityCheck() returns void; it prints to stderr only if the mesh has errors.
  // For a well-formed tetrahedron it should complete without aborting.
  REQUIRE_NOTHROW(mesh->sanityCheck("tetrahedron"));
}

// ─────────────────────────────────────────────────────────────────────────────
// Signed-distance tests using MeshSDF (brute-force O(N) evaluator)
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("MeshSDF: tetrahedron signed distances", "[DCEL][MeshSDF]")
{
  // Tetrahedron vertices: (0,0,0), (1,0,0), (0,1,0), (0,0,1).
  // Standard SDF convention: negative inside, positive outside.

  auto mesh = loadTetrahedron();
  REQUIRE(mesh != nullptr);

  TestMeshSDF sdf(mesh, BVH::Build::SAH);

  SECTION("centroid is inside (SDF < 0)")
  {
    const double d = sdf.signedDistance(Vec3T<double>(0.25, 0.25, 0.25));
    REQUIRE(d < 0.0);
  }

  SECTION("exterior point has positive SDF")
  {
    const double d = sdf.signedDistance(Vec3T<double>(2.0, 2.0, 2.0));
    REQUIRE(d > 0.0);
  }

  SECTION("point on a face centroid is near-zero")
  {
    // Bottom face (z=0): vertices (0,0,0),(0,1,0),(1,0,0), centroid = (1/3, 1/3, 0)
    const double d = sdf.signedDistance(Vec3T<double>(1.0 / 3.0, 1.0 / 3.0, 0.0));
    REQUIRE_THAT(d, WithinAbs(0.0, 1e-10));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Sign-convention regression: hard-coded DCEL, no file I/O.
// If the cross-product in computeNormal is ever accidentally inverted this
// test will immediately flip from pass to fail.
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("DCEL sign convention: exterior point has positive SDF", "[DCEL][sign]")
{
  auto        mesh = buildTetrahedron();
  TestMeshSDF sdf(mesh, BVH::Build::SAH);

  // Far outside: must be positive.
  REQUIRE(sdf.signedDistance(Vec3T<double>(2.0, 2.0, 2.0)) > 0.0);
  // Outside along each axis.
  REQUIRE(sdf.signedDistance(Vec3T<double>(-1.0, 0.0, 0.0)) > 0.0);
  REQUIRE(sdf.signedDistance(Vec3T<double>(0.0, -1.0, 0.0)) > 0.0);
  REQUIRE(sdf.signedDistance(Vec3T<double>(0.0, 0.0, -1.0)) > 0.0);
}

TEST_CASE("DCEL sign convention: interior point has negative SDF", "[DCEL][sign]")
{
  auto        mesh = buildTetrahedron();
  TestMeshSDF sdf(mesh, BVH::Build::SAH);

  // Centroid of the tetrahedron is clearly inside.
  REQUIRE(sdf.signedDistance(Vec3T<double>(0.25, 0.25, 0.25)) < 0.0);
}

// ─────────────────────────────────────────────────────────────────────────────
// FastTriMeshSDF (SIMD-accelerated BVH)
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("FastTriMeshSDF: matches MeshSDF for tetrahedron", "[DCEL][FastTriMeshSDF]")
{
  const std::string path = g_dataDir + "/tetrahedron.stl";
  auto              fast = Parser::readIntoTriangleBVH<double>(path);
  auto              mesh = Parser::readIntoMesh<double>(path);

  REQUIRE(fast != nullptr);
  REQUIRE(mesh != nullptr);

  // Compare a handful of query points.  Near-zero values use WithinAbs.
  const std::vector<Vec3T<double>> queries = {
    {0.25, 0.25, 0.25}, // centroid (inside)
    {2.0, 2.0, 2.0},    // far outside
    {-1.0, 0.0, 0.0},   // outside along -x
  };

  for (const auto& q : queries) {
    const double dBrute = mesh->signedDistance(q);
    const double dFast  = fast->signedDistance(q);
    REQUIRE_THAT(dFast, WithinRel(dBrute, 1e-6));
  }

  // Edge point: both should be near-zero
  const double dEdgeBrute = mesh->signedDistance(Vec3T<double>(0.5, 0.0, 0.0));
  const double dEdgeFast  = fast->signedDistance(Vec3T<double>(0.5, 0.0, 0.0));
  REQUIRE_THAT(dEdgeFast, WithinAbs(dEdgeBrute, 1e-6));
}
