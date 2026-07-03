// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"

#include <string>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using namespace EBGeometry::DCEL;
using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

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

  MeshSDF<double> sdf(mesh);

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
  auto            mesh = buildTetrahedron();
  MeshSDF<double> sdf(mesh);

  // Far outside: must be positive.
  REQUIRE(sdf.signedDistance(Vec3T<double>(2.0, 2.0, 2.0)) > 0.0);
  // Outside along each axis.
  REQUIRE(sdf.signedDistance(Vec3T<double>(-1.0, 0.0, 0.0)) > 0.0);
  REQUIRE(sdf.signedDistance(Vec3T<double>(0.0, -1.0, 0.0)) > 0.0);
  REQUIRE(sdf.signedDistance(Vec3T<double>(0.0, 0.0, -1.0)) > 0.0);
}

TEST_CASE("DCEL sign convention: interior point has negative SDF", "[DCEL][sign]")
{
  auto            mesh = buildTetrahedron();
  MeshSDF<double> sdf(mesh);

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
