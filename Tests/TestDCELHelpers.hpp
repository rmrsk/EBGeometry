// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// Test helpers shared by the DCEL tests (TestDCEL, TestDCELDevice): the test tetrahedron, read from disk or built
// from hard-coded data, and short names for the DCEL classes.

#ifndef EBGEOMETRY_TESTDCELHELPERS_HPP
#define EBGEOMETRY_TESTDCELHELPERS_HPP

#include "EBGeometry.hpp"
#include "TestUtilities.hpp"

#include <memory>
#include <vector>

namespace EBGeometryTestDCEL {

using namespace EBGeometry;
using namespace EBGeometry::DCEL;

template <class T>
using TestVertex = VertexT<T>;

template <class T>
using TestFace = FaceT<T>;

template <class T>
using TestEdge = EdgeT<T>;

template <class T>
using TestMesh = MeshT<T>;

template <class T>
using TestEdgeIterator = EdgeIteratorT<T>;

// The test tetrahedron, read from Tests/data as a DCEL mesh.
template <class T>
inline std::shared_ptr<MeshT<T>>
loadTetrahedron(Pool& a_pool)
{
  // The mesh is attached to a_pool by its first reserve inside readIntoDCEL and is queryable from
  // that point on: no freeze, no bind. Each call site below uses its own fresh, single-use Pool.
  // readIntoDCEL returns the mesh descriptor by value; it is wrapped here only so that the many
  // call sites below can share one handle.
  return std::make_shared<MeshT<T>>(Parser::readIntoDCEL<T>(dataPath("tetrahedron.stl"), a_pool));
}

// Build a tetrahedron DCEL mesh entirely from hard-coded data (no file I/O).
// Vertices: A=(0,0,0) B=(1,0,0) C=(0,1,0) D=(0,0,1).
// Face winding is CCW-from-outside so that the new (x2-x0)×(x2-x1) formula
// yields outward normals — standard SDF convention: negative inside, positive
// outside.
template <class T>
inline std::shared_ptr<MeshT<T>>
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

  auto mesh = std::make_shared<MeshT<T>>();
  Soup::soupToDCEL(*mesh, a_pool, verts, facets, "tetrahedron-hard"); // reconciles internally

  return mesh;
}

// A hand-built mesh of three vertices and one face whose half-edges link as a_next says: edge i
// starts at vertex i and its next edge is a_next[i]. Nothing here checks the links, so a_next can
// describe a broken loop that the file readers would never produce.
template <class T>
inline void
buildLinkedEdges(TestMesh<T>& a_mesh, Pool& a_pool, const std::vector<uint32_t>& a_next)
{
  const auto numEdges = static_cast<uint32_t>(a_next.size());

  a_mesh.reserveVertices(a_pool, 3);
  a_mesh.reserveEdges(a_pool, numEdges);
  a_mesh.reserveFaces(a_pool, 1);

  (void)a_mesh.addVertex(a_pool, TestVertex<T>(Vec3T<T>(0, 0, 0)));
  (void)a_mesh.addVertex(a_pool, TestVertex<T>(Vec3T<T>(1, 0, 0)));
  (void)a_mesh.addVertex(a_pool, TestVertex<T>(Vec3T<T>(0, 1, 0)));

  for (uint32_t e = 0; e < numEdges; e++) {
    TestEdge<T> edge(e % 3);

    edge.setNextEdge(a_next[e]);
    edge.setFace(0);

    (void)a_mesh.addEdge(a_pool, edge);
  }

  (void)a_mesh.addFace(a_pool, TestFace<T>(0u));
}

} // namespace EBGeometryTestDCEL

#endif
