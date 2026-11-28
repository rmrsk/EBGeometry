// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

#include "EBGeometry.hpp"
#include "TestDCELHelpers.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"
#include "TestUtilities.hpp"

#include <array>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using namespace EBGeometry::DCEL;
using namespace EBGeometryTestDCEL;
using Catch::Matchers::WithinRel;

// MeshSDF<T, K> under test, with the mesh's own metadata type and the SIMD-optimal
// branching factor for the precision in use.
template <class T>
using TestMeshSDF = MeshSDF<T, BVH::DefaultBranchingRatio<T>()>;

// ─────────────────────────────────────────────────────────────────────────────
// VertexT tests
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("VertexT: default construction leaves defined, zeroed state",
                   "[DCEL][Vertex]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;
  TestVertex<T> v;

  REQUIRE(v.getPosition() == Vec3T<T>::zeros());
  REQUIRE(v.getNormal() == Vec3T<T>::zeros());
  REQUIRE(v.getOutgoingEdgeIndex() == UINT32_MAX);
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

TEMPLATE_TEST_CASE("VertexT: copy construction copies every member", "[DCEL][Vertex]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  TestVertex<T> src(Vec3T<T>(1, 2, 3), Vec3T<T>(0, 0, 1));
  src.setEdge(7);

  TestVertex<T> copy(src);

  REQUIRE(copy.getPosition() == src.getPosition());
  REQUIRE(copy.getNormal() == src.getNormal());
  REQUIRE(copy.getOutgoingEdgeIndex() == 7);
}

TEMPLATE_TEST_CASE("VertexT: copy assignment has the same semantics as copy construction",
                   "[DCEL][Vertex]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  TestVertex<T> src(Vec3T<T>(1, 2, 3), Vec3T<T>(0, 0, 1));
  src.setEdge(7);

  TestVertex<T> dst;
  dst = src;

  REQUIRE(dst.getPosition() == src.getPosition());
  REQUIRE(dst.getNormal() == src.getNormal());
  REQUIRE(dst.getOutgoingEdgeIndex() == 7);
}

TEMPLATE_TEST_CASE("VertexT: move construction and move assignment transfer the entire state",
                   "[DCEL][Vertex]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  TestVertex<T> moveCtorSrc(Vec3T<T>(1, 2, 3), Vec3T<T>(0, 0, 1));
  moveCtorSrc.setEdge(7);

  TestVertex<T> moved(std::move(moveCtorSrc));
  REQUIRE(moved.getPosition() == Vec3T<T>(1, 2, 3));
  REQUIRE(moved.getOutgoingEdgeIndex() == 7);

  TestVertex<T> moveAssignSrc(Vec3T<T>(4, 5, 6), Vec3T<T>(1, 0, 0));
  moveAssignSrc.setEdge(7);

  TestVertex<T> moveAssignDst;
  moveAssignDst = std::move(moveAssignSrc);
  REQUIRE(moveAssignDst.getPosition() == Vec3T<T>(4, 5, 6));
  REQUIRE(moveAssignDst.getOutgoingEdgeIndex() == 7);
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

TEMPLATE_TEST_CASE("EdgeT: define, setVertex, setPairEdge, setNextEdge, setFace",
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

TEMPLATE_TEST_CASE("EdgeT: copy construction copies every member", "[DCEL][Edge]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  TestEdge<T> src;
  src.define(1, 2, 3);
  src.setFace(4);

  TestEdge<T> copy(src);
  REQUIRE(copy.getVertexIndex() == 1);
  REQUIRE(copy.getPairEdgeIndex() == 2);
  REQUIRE(copy.getNextEdgeIndex() == 3);
  REQUIRE(copy.getFaceIndex() == 4);
}

TEMPLATE_TEST_CASE("EdgeT: copy assignment has the same semantics as copy construction",
                   "[DCEL][Edge]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  TestEdge<T> src;
  src.define(1, 2, 3);
  src.setFace(4);

  TestEdge<T> dst;
  dst = src;

  REQUIRE(dst.getVertexIndex() == 1);
  REQUIRE(dst.getPairEdgeIndex() == 2);
  REQUIRE(dst.getNextEdgeIndex() == 3);
  REQUIRE(dst.getFaceIndex() == 4);
}

TEMPLATE_TEST_CASE("EdgeT: move construction and move assignment transfer the entire state",
                   "[DCEL][Edge]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  TestEdge<T> moveCtorSrc;
  moveCtorSrc.define(1, 2, 3);
  moveCtorSrc.setFace(4);

  TestEdge<T> moved(std::move(moveCtorSrc));
  REQUIRE(moved.getVertexIndex() == 1);
  REQUIRE(moved.getPairEdgeIndex() == 2);
  REQUIRE(moved.getNextEdgeIndex() == 3);
  REQUIRE(moved.getFaceIndex() == 4);

  TestEdge<T> moveAssignSrc;
  moveAssignSrc.define(1, 2, 3);
  moveAssignSrc.setFace(4);

  TestEdge<T> moveAssignDst;
  moveAssignDst = std::move(moveAssignSrc);
  REQUIRE(moveAssignDst.getVertexIndex() == 1);
  REQUIRE(moveAssignDst.getPairEdgeIndex() == 2);
  REQUIRE(moveAssignDst.getNextEdgeIndex() == 3);
  REQUIRE(moveAssignDst.getFaceIndex() == 4);
}

// ─────────────────────────────────────────────────────────────────────────────
// EdgeIteratorT tests
// ─────────────────────────────────────────────────────────────────────────────

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

TEMPLATE_TEST_CASE("MeshT::sanityCheck finishes on a face whose half-edge loop does not close",
                   "[DCEL][Mesh][regression]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // Gathering such a face's vertices never finished before the check skipped it.
  for (const auto& next : {std::vector<uint32_t>{1, 2, 1}, std::vector<uint32_t>{1, 7, 0}}) {
    Pool        pool(hostMemoryResource());
    TestMesh<T> mesh;

    buildLinkedEdges(mesh, pool, next);

    mesh.sanityCheck("broken loop");
  }

  SUCCEED();
}

TEST_CASE("MeshT: reconciling a hand-built face whose half-edge loop does not close aborts", "[DCEL][Mesh][death]")
{
  using T = double;

  // Edge 0 -> 1 -> 2 -> 1 -> 2 -> ...: a cycle that never returns to edge 0, so iterating the face
  // never ends. reconcile() rejects it before anything iterates it.
  REQUIRE(abortsWith(
    [] {
      Pool        pool(hostMemoryResource());
      TestMesh<T> mesh;

      buildLinkedEdges(mesh, pool, {1, 2, 1});

      mesh.reconcile(); // must abort
    },
    "DCEL::MeshT::reconcile: the half-edge loop of face 0 does not close (it reaches edge 1 after 3 steps"));

  // A next index outside the edge array is caught by the same check.
  REQUIRE(abortsWith(
    [] {
      Pool        pool(hostMemoryResource());
      TestMesh<T> mesh;

      buildLinkedEdges(mesh, pool, {1, 7, 0});

      mesh.reconcile(); // must abort
    },
    "DCEL::MeshT::reconcile: the half-edge loop of face 0 does not close (it reaches edge 7 after 2 steps"));

  // A face whose half-edge was never set.
  REQUIRE(abortsWith(
    [] {
      Pool        pool(hostMemoryResource());
      TestMesh<T> mesh;

      mesh.reserveVertices(pool, 0);
      mesh.reserveEdges(pool, 0);
      mesh.reserveFaces(pool, 1);

      (void)mesh.addFace(pool, TestFace<T>(UINT32_MAX));

      mesh.reconcile(); // must abort
    },
    "DCEL::MeshT::reconcile: face 0 has no half-edge"));

  // A closed loop naming a vertex past the end of the vertex array: rejected before reconcile() reads it.
  REQUIRE(abortsWith(
    [] {
      Pool        pool(hostMemoryResource());
      TestMesh<T> mesh;

      mesh.reserveVertices(pool, 3);
      mesh.reserveEdges(pool, 3);
      mesh.reserveFaces(pool, 1);

      (void)mesh.addVertex(pool, TestVertex<T>(Vec3T<T>(0, 0, 0)));
      (void)mesh.addVertex(pool, TestVertex<T>(Vec3T<T>(1, 0, 0)));
      (void)mesh.addVertex(pool, TestVertex<T>(Vec3T<T>(0, 1, 0)));

      const std::initializer_list<std::pair<uint32_t, uint32_t>> edges{{0, 1}, {1, 2}, {7, 0}};

      for (const auto& [vertex, next] : edges) {
        TestEdge<T> edge(vertex);

        edge.setNextEdge(next);
        edge.setFace(0);

        (void)mesh.addEdge(pool, edge);
      }

      (void)mesh.addFace(pool, TestFace<T>(0u));

      mesh.reconcile(); // must abort
    },
    "DCEL::MeshT::reconcile: face 0 references vertex 7, but the mesh has 3 vertices"));
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
    const T distDirect  = mesh->signedDistance(p, MeshT<T>::SearchAlgorithm::Direct);
    const T distDirect2 = mesh->signedDistance(p, MeshT<T>::SearchAlgorithm::Direct2);

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

  mesh->setSearchAlgorithm(MeshT<T>::SearchAlgorithm::Direct);
  const T distDirect = mesh->signedDistance(p);

  mesh->setSearchAlgorithm(MeshT<T>::SearchAlgorithm::Direct2);
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

TEMPLATE_TEST_CASE("MeshT: deepCopy of an empty mesh is an empty mesh", "[DCEL][Mesh]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool           pool(hostMemoryResource());
  const MeshT<T> empty;
  const auto     copy = empty.deepCopy(pool);

  REQUIRE(copy.numVertices() == 0);
  REQUIRE(copy.numEdges() == 0);
  REQUIRE(copy.numFaces() == 0);
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

  REQUIRE(copy.numVertices() == mesh->numVertices());
  REQUIRE(copy.numEdges() == mesh->numEdges());
  REQUIRE(copy.numFaces() == mesh->numFaces());

  for (uint32_t i = 0; i < mesh->numFaces(); i++) {
    REQUIRE((copy.getFace(i).getNormal() - mesh->getFace(i).getNormal()).length() < T(exactMargin<T>()));
    REQUIRE_THAT(copy.getFace(i).getArea(), withinAbsT(mesh->getFace(i).getArea(), exactMargin<T>()));
  }

  // The copy must be usable for signed-distance queries (validates that the projection axes were
  // correctly rebuilt for every face). Done before mutating the copy, since the point-in-face test
  // projects the face's vertices on the fly and so reflects the live geometry.
  const Vec3T<T> p(0.1, 0.1, 0.1);
  REQUIRE_THAT(copy.signedDistance(p), withinAbsT(mesh->signedDistance(p), formulaMargin<T>()));

  // Mutating the copy must not affect the original -- this is the meaningful test of independence
  // now that vertices/edges/faces are plain values rather than shared_ptr-identified objects.
  copy.getVertex(0).setPosition(Vec3T<T>(999, 999, 999));
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
    REQUIRE((copy.getFace(i).getNormal() - mesh->getFace(i).getNormal()).length() < T(exactMargin<T>()));
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

  VertexT<T> vertex = mesh->getVertex(0); // by value, deliberately not by reference

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

  REQUIRE(copy.numVertices() == mesh->numVertices());
  REQUIRE(copy.numFaces() == mesh->numFaces());

  const Vec3T<T> p(0.1, 0.1, 0.1);
  REQUIRE_THAT(copy.signedDistance(p), withinAbsT(mesh->signedDistance(p), formulaMargin<T>()));

  for (uint32_t i = 0; i < mesh->numVertices(); i++) {
    REQUIRE(copy.getVertex(i).getPosition() == mesh->getVertex(i).getPosition());
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
// Caller-controlled setup and topology checks (EBGEOMETRY_REQUIRE: abort in every build)
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("MeshT: rebasedView onto a pool that is not a mirror of the mesh's own pool aborts", "[DCEL][Mesh][death]")
{
  using T = double;

  REQUIRE(abortsWith(
    [] {
      Pool pool(hostMemoryResource());
      Pool otherPool(hostMemoryResource());

      auto mesh = buildTetrahedron<T>(pool);

      (void)buildTetrahedron<T>(otherPool);

      pool.freeze();
      otherPool.freeze();

      // A byte-identical block, but mirrored from the wrong pool: the mesh's offsets mean nothing there.
      const Pool wrongMirror = Pool::mirror(otherPool, hostMemoryResource());

      (void)mesh->rebasedView(wrongMirror); // must abort
    },
    "DCEL::MeshT::rebasedView: the pool must be the object's own pool or a mirror of it"));
}

TEST_CASE("MeshT: reserving a mesh's arrays from two different pools aborts", "[DCEL][Mesh][death]")
{
  using T = double;

  REQUIRE(abortsWith(
    [] {
      Pool poolA(hostMemoryResource());
      Pool poolB(hostMemoryResource());

      TestMesh<T> mesh;

      mesh.reserveVertices(poolA, 3);
      mesh.reserveEdges(poolB, 3); // must abort
    },
    "DCEL::MeshT::attachTo: all of its arrays must be reserved from the same Pool"));
}

TEST_CASE("MeshT: reconciling a hand-built face with fewer than 3 vertices aborts", "[DCEL][Mesh][death]")
{
  using T = double;

  REQUIRE(abortsWith(
    [] {
      Pool        pool(hostMemoryResource());
      TestMesh<T> mesh;

      mesh.reserveVertices(pool, 2);
      mesh.reserveEdges(pool, 2);
      mesh.reserveFaces(pool, 1);

      (void)mesh.addVertex(pool, TestVertex<T>(Vec3T<T>(0, 0, 0)));
      (void)mesh.addVertex(pool, TestVertex<T>(Vec3T<T>(1, 0, 0)));

      // A two-edge loop 0 -> 1 -> 0: a "face" that cannot span a plane.
      for (uint32_t e = 0; e < 2; e++) {
        TestEdge<T> edge(e);

        edge.setNextEdge((e + 1) % 2);
        edge.setFace(0);

        (void)mesh.addEdge(pool, edge);
      }

      (void)mesh.addFace(pool, TestFace<T>(0u));

      mesh.reconcile(); // must abort
    },
    "DCEL::MeshT::reconcile: face 0 has 2 vertices; a face needs at least 3"));
}

TEST_CASE("MeshT: reconciling a hand-built face that visits a vertex twice aborts", "[DCEL][Mesh][death]")
{
  using T = double;

  // The file readers reject such a face (Soup::containsDegeneratePolygons), so it can only be built
  // by hand. The angle-weighted vertex normal needs exactly one incoming and one outgoing half-edge
  // per face at the vertex; before this was an always-on check, a release build read past the end of
  // the two-element neighbor list.
  REQUIRE(abortsWith(
    [] {
      Pool        pool(hostMemoryResource());
      TestMesh<T> mesh;

      // Boundary loop 0 -> 1 -> 2 -> 0 -> 3 -> (back to 0): vertex 0 is visited twice. Its Newell
      // normal is +z, so the face is not skipped as zero-area.
      const std::vector<Vec3T<T>> positions = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, -1, 0}};
      const std::vector<uint32_t> loop      = {0, 1, 2, 0, 3};

      mesh.reserveVertices(pool, static_cast<uint32_t>(positions.size()));
      mesh.reserveEdges(pool, static_cast<uint32_t>(loop.size()));
      mesh.reserveFaces(pool, 1);

      for (const auto& x : positions) {
        (void)mesh.addVertex(pool, TestVertex<T>(x));
      }

      for (uint32_t e = 0; e < loop.size(); e++) {
        TestEdge<T> edge(loop[e]);

        edge.setNextEdge((e + 1) % static_cast<uint32_t>(loop.size()));
        edge.setFace(0);

        (void)mesh.addEdge(pool, edge);
      }

      (void)mesh.addFace(pool, TestFace<T>(0u));

      mesh.reconcile(); // must abort
    },
    "DCEL::VertexT::computeVertexNormalAngleWeighted: face 0 must visit vertex 0 exactly once (found 4"));
}

// ─────────────────────────────────────────────────────────────────────────────
// Soup tests
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("Soup::soupToDCEL: a face with fewer than 3 vertices or an out-of-range vertex index aborts", "[Soup][death]")
{
  using T = double;

  const std::vector<Vec3T<T>> verts = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};

  // Both used to be EBGEOMETRY_EXPECT only: without assertions, a short face was skipped with a
  // warning and an out-of-range index wrote past the vertex array.
  REQUIRE(abortsWith(
    [&verts] {
      Pool        pool(hostMemoryResource());
      TestMesh<T> mesh;

      Soup::soupToDCEL(mesh, pool, verts, {{0, 1, 2}, {0, 1}}, "short");
    },
    "Soup::soupToDCEL: face 1 of 'short' has 2 vertices, but a face needs at least 3"));

  REQUIRE(abortsWith(
    [&verts] {
      Pool        pool(hostMemoryResource());
      TestMesh<T> mesh;

      Soup::soupToDCEL(mesh, pool, verts, {{0, 1, 5}}, "outside");
    },
    "Soup::soupToDCEL: face 0 of 'outside' names vertex 5, but there are only 3 vertices"));
}

TEST_CASE("Soup::reconcilePairEdgesDCEL: a half-edge with an out-of-range next edge aborts", "[Soup][death]")
{
  using T = double;

  REQUIRE(abortsWith(
    [] {
      Pool        pool(hostMemoryResource());
      TestMesh<T> mesh;

      buildLinkedEdges(mesh, pool, {1, 7, 0});

      Soup::reconcilePairEdgesDCEL(mesh);
    },
    "Soup::reconcilePairEdgesDCEL: half-edge 1 has next edge 7, but there are only 3 half-edges"));
}

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

TEMPLATE_TEST_CASE("Soup::containsDegeneratePolygons detects a zero-area facet with distinct, collinear vertices",
                   "[Soup]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T                                       = TestType;
  const std::vector<Vec3T<T>>            verts  = {{0, 0, 0}, {1, 0, 0}, {T(0.5), 0, 0}};
  const std::vector<std::vector<size_t>> facets = {{0, 1, 2}};

  REQUIRE(Soup::isZeroArea(verts, facets[0]));
  REQUIRE(Soup::containsDegeneratePolygons(verts, facets));
}

TEMPLATE_TEST_CASE("Soup::isZeroArea keeps a small, well-shaped facet far from the origin",
                   "[Soup]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // A face of a mesh file, 3e-3 across and about 5 from the origin: its doubled area is 4.7e-4 times its longest edge
  // squared, 60 times the zero-area tolerance. In single precision, a Newell normal summed from the vertices' absolute
  // positions cancels to exactly zero here; taken relative to one vertex it does not.
  const std::vector<Vec3T<T>>            verts  = {{T(3.05267167f), T(5.25392294f), T(4.7220273f)},
                                                   {T(3.05496812f), T(5.25439119f), T(4.72036695f)},
                                                   {T(3.052912f), T(5.25397062f), T(4.72185421f)}};
  const std::vector<std::vector<size_t>> facets = {{0, 1, 2}};

  REQUIRE_FALSE(Soup::isZeroArea(verts, facets[0]));
  REQUIRE_FALSE(Soup::containsDegeneratePolygons(verts, facets));
}

TEMPLATE_TEST_CASE("Soup::removeDegeneratePolygons repairs a T-junction filler and drops collapsed facets",
                   "[Soup]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // A square split into two triangles along its diagonal 0-2, where one side of the diagonal has
  // been split at its midpoint 4 and the T-junction closed by the zero-area triangle {2, 0, 4}.
  // Facet {0, 4, 4} collapses to a segment once its repeated vertex is merged.
  const std::vector<Vec3T<T>> verts = {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}, {T(0.5), T(0.5), 0}};

  std::vector<std::vector<size_t>> facets = {{0, 1, 4}, {1, 2, 4}, {2, 0, 4}, {0, 2, 3}, {0, 4, 4}};

  // The kept facets are the original 0, 1 and 3, in order.
  REQUIRE(Soup::removeDegeneratePolygons(verts, facets) == std::vector<size_t>{0, 1, 3});
  REQUIRE(facets.size() == 3);
  REQUIRE_FALSE(Soup::containsDegeneratePolygons(verts, facets));

  // The facet across the filler's longest edge (0 -> 2 reversed, in {0, 2, 3}) gained its middle
  // vertex, so every directed edge now has its reverse and the mesh is closed around vertex 4.
  REQUIRE(facets[2] == std::vector<size_t>{0, 4, 2, 3});
}

TEMPLATE_TEST_CASE("DCEL: a zero-area face built without the soup repair gets a zero normal, never NaN",
                   "[DCEL][Face]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  // soupToDCEL directly, bypassing Soup::removeDegeneratePolygons: face 2 is a zero-area filler.
  const std::vector<Vec3T<T>> verts = {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}, {T(0.5), T(0.5), 0}};

  const std::vector<std::vector<size_t>> facets = {{0, 1, 4}, {1, 2, 4}, {2, 0, 4}, {0, 2, 3}};

  MeshT<T> mesh;
  Soup::soupToDCEL(mesh, pool, verts, facets, "t-junction");

  REQUIRE(mesh.getFace(2).getNormal() == Vec3T<T>::zeros());

  for (uint32_t v = 0; v < mesh.numVertices(); v++) {
    const Vec3T<T>& n = mesh.getVertex(v).getNormal();

    REQUIRE(std::isfinite(n[0]));
    REQUIRE(std::isfinite(n[1]));
    REQUIRE(std::isfinite(n[2]));
  }

  for (uint32_t e = 0; e < mesh.numEdges(); e++) {
    const Vec3T<T>& n = mesh.getEdge(e).getNormal();

    REQUIRE(std::isfinite(n[0]));
    REQUIRE(std::isfinite(n[1]));
    REQUIRE(std::isfinite(n[2]));
  }
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

  const std::vector<Vec3T<T>> original = verts;

  const std::vector<size_t> newIndex = Soup::compress(verts, facets);

  REQUIRE(verts.size() == 4); // A, B, C, D -- each unique position appears once.
  REQUIRE(facets.size() == 2);
  REQUIRE(facets[0][0] == facets[1][1]); // Both facets' A-reference now points at the same index.
  REQUIRE(facets[0][1] == facets[1][0]); // Both facets' B-reference now points at the same index.

  // The returned map sends every original vertex to the compressed vertex at its position.
  REQUIRE(newIndex.size() == original.size());

  for (size_t v = 0; v < original.size(); v++) {
    REQUIRE(verts[newIndex[v]] == original[v]);
  }

  // Compressing again changes nothing.
  const std::vector<Vec3T<T>>            compressed       = verts;
  const std::vector<std::vector<size_t>> compressedFacets = facets;

  const std::vector<size_t> identity = Soup::compress(verts, facets);

  REQUIRE(verts == compressed);
  REQUIRE(facets == compressedFacets);
  REQUIRE(identity == std::vector<size_t>{0, 1, 2, 3});
}

TEMPLATE_TEST_CASE("Soup::compress merges every copy of a position, -0 and +0 alike, in a large soup",
                   "[Soup]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // Positions on a small lattice, each listed a varying number of times in a scrambled order, as a binary STL
  // file lists a vertex once per facet. Every coordinate 0 is written as -0 on some copies.
  std::vector<Vec3T<T>> verts;

  for (size_t i = 0; i < 3000; i++) {
    const size_t k     = (i * 7919) % 401;
    const size_t row   = (k / 7) % 7;
    const size_t layer = k / 49;
    const T      x     = T(k % 7) - T(3);
    const T      y     = T(row) - T(3);
    const T      z     = T(layer) - T(4);

    const auto signedZero = [i](T a_coordinate) { return (a_coordinate == T(0) && i % 2 == 1) ? -T(0) : a_coordinate; };

    verts.emplace_back(signedZero(x), signedZero(y), signedZero(z));
  }

  std::vector<std::vector<size_t>> facets;

  for (size_t f = 0; f + 2 < verts.size(); f += 3) {
    facets.push_back({f, f + 1, f + 2});
  }

  const std::vector<Vec3T<T>>            original       = verts;
  const std::vector<std::vector<size_t>> originalFacets = facets;

  const std::vector<size_t> newIndex = Soup::compress(verts, facets);

  // As many vertices as distinct positions, counted by brute force.
  size_t numDistinct = 0;

  for (size_t i = 0; i < original.size(); i++) {
    bool seen = false;

    for (size_t j = 0; j < i && !seen; j++) {
      seen = (original[j] == original[i]);
    }

    numDistinct += seen ? 0 : 1;
  }

  REQUIRE(verts.size() == numDistinct);

  // Sorted lexicographically, so no two equal.
  for (size_t v = 1; v < verts.size(); v++) {
    REQUIRE(verts[v - 1].lessLX(verts[v]));
  }

  // Every input vertex maps to its position, and the facets follow the map.
  REQUIRE(newIndex.size() == original.size());

  for (size_t v = 0; v < original.size(); v++) {
    REQUIRE(verts[newIndex[v]] == original[v]);
  }

  REQUIRE(facets.size() == originalFacets.size());

  for (size_t f = 0; f < facets.size(); f++) {
    for (size_t i = 0; i < 3; i++) {
      REQUIRE(facets[f][i] == newIndex[originalFacets[f][i]]);
    }
  }
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

TEMPLATE_TEST_CASE("Parser::readIntoDCEL throws for an unrecognized file extension",
                   "[DCEL][Parser]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool              pool(hostMemoryResource());
  const std::string file = dataPath("tetrahedron.unsupported-extension");

  REQUIRE_THROWS_MATCHES(Parser::readIntoDCEL<T>(file, pool),
                         Parser::ParseError,
                         Catch::Matchers::Message(file + ": unsupported file type; the extension must be .stl, "
                                                         ".ply, .vtk or .obj"));
}

TEMPLATE_TEST_CASE("DCEL: an empty mesh reports infinite distances", "[DCEL]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const DCEL::MeshT<T> mesh;

  REQUIRE(mesh.numVertices() == 0);
  REQUIRE(mesh.numEdges() == 0);
  REQUIRE(mesh.numFaces() == 0);

  const auto inf = std::numeric_limits<T>::infinity();
  REQUIRE(mesh.signedDistance(Vec3T<T>(0, 0, 0)) == inf);
  REQUIRE(mesh.unsignedDistance2(Vec3T<T>(0, 0, 0)) == inf);
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

  TestMeshSDF<T> sdf(*mesh, pool, BVH::Construction::SAH);

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
  TestMeshSDF<T> sdf(*mesh, pool, BVH::Construction::SAH);

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
  TestMeshSDF<T> sdf(*mesh, pool, BVH::Construction::SAH);

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
  const std::string path = dataPath("tetrahedron.stl");

  Pool pool(hostMemoryResource());
  auto fast = Parser::readIntoTriMeshSDF<T>(path, pool);
  auto mesh = Parser::readIntoFlatMeshSDF<T>(path, pool);

  // Compare a handful of query points.  Near-zero values use WithinAbs.
  const std::vector<Vec3T<T>> queries = {
    {0.25, 0.25, 0.25}, // centroid (inside)
    {2.0, 2.0, 2.0},    // far outside
    {-1.0, 0.0, 0.0},   // outside along -x
  };

  for (const auto& q : queries) {
    const T dBrute = mesh.signedDistance(q);
    const T dFast  = fast.signedDistance(q);
    REQUIRE_THAT(dFast, WithinRel(dBrute, T(traversalMargin<T>())));
  }

  // Edge point: both should be near-zero
  const T dEdgeBrute = mesh.signedDistance(Vec3T<T>(0.5, 0.0, 0.0));
  const T dEdgeFast  = fast.signedDistance(Vec3T<T>(0.5, 0.0, 0.0));
  REQUIRE_THAT(dEdgeFast, withinAbsT(dEdgeBrute, traversalMargin<T>()));
}

// ─────────────────────────────────────────────────────────────────────────────
// Point-in-face containment (FaceT): the winding-number, crossing-number and subtended-angle tests, which
// project the face's vertices on the fly, exercised directly on small hand-built faces, including concave
// polygons and non-axis-aligned planes that a mesh-level test would not necessarily hit, through a helper
// that wires up a single DCEL face from a vertex list.
// ─────────────────────────────────────────────────────────────────────────────

namespace {

template <class T>
std::array<DCEL::InsideOutsideAlgorithm, 3>
allAlgorithms()
{
  return {DCEL::InsideOutsideAlgorithm::WindingNumber,
          DCEL::InsideOutsideAlgorithm::CrossingNumber,
          DCEL::InsideOutsideAlgorithm::SubtendedAngle};
}

// Test-only subclass that exposes FaceT's protected point-in-face test (used internally by
// signedDistance()) so the containment algorithms can be exercised directly. MeshT stores plain
// FaceT<T> by value, so a mesh-owned face is never an AccessibleFace and must not be cast to one;
// instead, isPointInsideFace() below copies the face into an AccessibleFace and asks the copy.
// FaceT is a trivially copyable value type whose containment test reads only its own members and
// the mesh passed in, so the copy gives the same answer as the original.
template <class T>
struct AccessibleFace : public DCEL::FaceT<T>
{
  explicit AccessibleFace(const DCEL::FaceT<T>& a_face) : DCEL::FaceT<T>(a_face)
  {}

  using DCEL::FaceT<T>::isPointInsideFace;
};

template <class T>
[[nodiscard]] bool
isPointInsideFace(const DCEL::FaceT<T>& a_face, const Vec3T<T>& a_point, const DCEL::MeshT<T>& a_mesh)
{
  return AccessibleFace<T>(a_face).isPointInsideFace(a_point, a_mesh);
}

// A single DCEL face wired up from a vertex list, held inside a real DCEL::MeshT so the face's
// on-the-fly (half-edge-walking, index-based) containment test can be queried against it.
// reconcile() derives the normal, centroid, and 2D-projection axes from the geometry, matching how
// a parsed mesh's faces are set up.
template <class T>
struct BuiltFace
{
  using Mesh   = DCEL::MeshT<T>;
  using Vertex = DCEL::VertexT<T>;
  using Edge   = DCEL::EdgeT<T>;
  using Face   = DCEL::FaceT<T>;

  // m_mesh's vertex/edge/face storage is reserved from m_pool, and m_mesh is bound to it (see the
  // constructor) -- both members must stay together at a fixed address, so BuiltFace is not safe
  // to copy or move. Every use site relies on C++17's guaranteed copy elision for prvalue returns
  // (e.g. `return BuiltFace<T>(...)`), which needs no copy/move constructor at all.
  Pool m_pool;
  Mesh m_mesh;

  explicit BuiltFace(const std::vector<Vec3T<T>>& a_positions) : m_pool(hostMemoryResource())
  {
    const uint32_t N = static_cast<uint32_t>(a_positions.size());

    m_mesh.reserveVertices(m_pool, N);
    m_mesh.reserveEdges(m_pool, N);
    m_mesh.reserveFaces(m_pool, 1);

    for (const auto& p : a_positions) {
      m_mesh.addVertex(m_pool, Vertex(p));
    }

    for (uint32_t i = 0; i < N; i++) {
      m_mesh.addEdge(m_pool, Edge(i)); // half-edge i starts at vertex i
    }

    for (uint32_t i = 0; i < N; i++) {
      m_mesh.getEdge(i).setNextEdge((i + 1) % N);
      m_mesh.getEdge(i).setFace(0);
    }

    m_mesh.addFace(m_pool, Face(0u)); // half-edge index 0

    m_mesh.getFace(0).reconcile(m_mesh);
  }

  BuiltFace(const BuiltFace&) = delete;
  BuiltFace(BuiltFace&&)      = delete;

  [[nodiscard]] bool
  isPointInside(const Vec3T<T>& a_point, DCEL::InsideOutsideAlgorithm a_algorithm)
  {
    auto& face = m_mesh.getFace(0);

    face.setInsideOutsideAlgorithm(a_algorithm);

    return isPointInsideFace(face, a_point, m_mesh);
  }
};

// A unit square in the z=0 plane.
template <class T>
BuiltFace<T>
unitSquare()
{
  using Vec3 = Vec3T<T>;

  return BuiltFace<T>({Vec3(0, 0, 0), Vec3(1, 0, 0), Vec3(1, 1, 0), Vec3(0, 1, 0)});
}

// An L-shaped concave hexagon in the z=0 plane: the unit square with its top-right quadrant notched
// out. The point (0.75, 0.75, 0) sits in that notch -- inside the convex hull of the vertices, but
// outside the actual polygon.
template <class T>
BuiltFace<T>
lShape()
{
  using Vec3 = Vec3T<T>;

  return BuiltFace<T>(
    {Vec3(0, 0, 0), Vec3(1, 0, 0), Vec3(1, 0.5, 0), Vec3(0.5, 0.5, 0), Vec3(0.5, 1, 0), Vec3(0, 1, 0)});
}

} // namespace

TEMPLATE_TEST_CASE("FaceT: a point clearly inside a square reads as inside for every algorithm",
                   "[DCEL][Face][Polygon2D]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  BuiltFace<T> square = unitSquare<T>();

  for (const auto algo : allAlgorithms<T>()) {
    REQUIRE(square.isPointInside(Vec3(0.5, 0.5, 0), algo));
  }
}

TEMPLATE_TEST_CASE("FaceT: a point clearly outside a square reads as outside for every algorithm",
                   "[DCEL][Face][Polygon2D]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  BuiltFace<T> square = unitSquare<T>();

  for (const auto algo : allAlgorithms<T>()) {
    REQUIRE_FALSE(square.isPointInside(Vec3(2, 2, 0), algo));
    REQUIRE_FALSE(square.isPointInside(Vec3(-1, 0.5, 0), algo));
  }
}

TEMPLATE_TEST_CASE("FaceT: containment ignores the coordinate normal to the polygon's plane",
                   "[DCEL][Face][Polygon2D]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  BuiltFace<T> square = unitSquare<T>();

  // The polygon lies in z=0, but containment only depends on (x, y): the projection direction
  // discards z, so a point "floating" well above or below the plane still reads as inside when its
  // (x, y) projects into the square (isPointInsideFace first projects onto the plane).
  for (const auto algo : allAlgorithms<T>()) {
    REQUIRE(square.isPointInside(Vec3(0.5, 0.5, 100), algo));
    REQUIRE(square.isPointInside(Vec3(0.5, 0.5, -100), algo));
  }
}

TEMPLATE_TEST_CASE("FaceT: a concave polygon correctly excludes points in its notch, for every algorithm",
                   "[DCEL][Face][Polygon2D]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  BuiltFace<T> lPolygon = lShape<T>();

  // Inside the solid part of the L.
  for (const auto algo : allAlgorithms<T>()) {
    REQUIRE(lPolygon.isPointInside(Vec3(0.25, 0.25, 0), algo));
  }

  // Inside the convex hull of the vertices, but in the notched-out region: must read as outside.
  for (const auto algo : allAlgorithms<T>()) {
    REQUIRE_FALSE(lPolygon.isPointInside(Vec3(0.75, 0.75, 0), algo));
  }

  // Clearly outside entirely.
  for (const auto algo : allAlgorithms<T>()) {
    REQUIRE_FALSE(lPolygon.isPointInside(Vec3(-1, -1, 0), algo));
  }
}

TEMPLATE_TEST_CASE("FaceT: containment works for a triangle in a non-axis-aligned plane",
                   "[DCEL][Face][Polygon2D]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // A triangle in the plane x + y + z = 3, with an oblique (but not axis-aligned) normal.
  BuiltFace<T> triangle({Vec3(3, 0, 0), Vec3(0, 3, 0), Vec3(0, 0, 3)});

  // The triangle's centroid (1, 1, 1) must project to the inside.
  for (const auto algo : allAlgorithms<T>()) {
    REQUIRE(triangle.isPointInside(Vec3(1, 1, 1), algo));
  }

  // A point outside the triangle but still in the same plane.
  for (const auto algo : allAlgorithms<T>()) {
    REQUIRE_FALSE(triangle.isPointInside(Vec3(3, 3, -3), algo));
  }
}

TEMPLATE_TEST_CASE("FaceT: the three containment algorithms agree over a grid of query points",
                   "[DCEL][Face][Polygon2D]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  BuiltFace<T> lPolygon = lShape<T>();

  // Offset the grid so it never lands exactly on one of the L-shape's edges or vertices (all at
  // multiples of 0.25) -- boundary points are inherently ambiguous between algorithms and aren't a
  // meaningful place to expect exact agreement.
  for (int i = -2; i <= 6; i++) {
    for (int j = -2; j <= 6; j++) {
      const Vec3 p(T(0.25 * i + 0.07), T(0.25 * j + 0.11), T(0));

      const bool winding  = lPolygon.isPointInside(p, DCEL::InsideOutsideAlgorithm::WindingNumber);
      const bool crossing = lPolygon.isPointInside(p, DCEL::InsideOutsideAlgorithm::CrossingNumber);
      const bool subtend  = lPolygon.isPointInside(p, DCEL::InsideOutsideAlgorithm::SubtendedAngle);

      REQUIRE(winding == crossing);
      REQUIRE(winding == subtend);
    }
  }
}
