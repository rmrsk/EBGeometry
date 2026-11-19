// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

// Performance cases for reading a mesh file: the armadillo (the dodecahedron under --smoke) read into a
// cleaned polygon soup, into a DCEL mesh and into triangles, and the cleaning of a soup alone, in float and
// double, on the host. See ../README.md and PerformanceTesting.rst.

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

template <class T>
using Soup = EBGeometry::PolygonSoup<T>;

// A soup's checksums: its vertex coordinates summed, and its faces' vertex indices hashed in order.
template <class T>
void
recordSoup(Perf::Case& a_case, const Soup<T>& a_soup)
{
  std::vector<T>      coordinates;
  Perf::IndexChecksum indices;

  for (const Vec3<T>& x : a_soup.getVertexCoordinates()) {
    for (int d = 0; d < 3; d++) {
      coordinates.push_back(x[d]);
    }
  }

  for (const auto& facet : a_soup.getFacets()) {
    indices.add(static_cast<std::uint32_t>(facet.size()));

    for (const std::size_t v : facet) {
      indices.add(static_cast<std::uint32_t>(v));
    }
  }

  a_case.checksum(Perf::checksumOf(coordinates));
  a_case.indexChecksum(indices.value());
}

// The same soup as a binary STL file lists it: three copies of the vertices of every triangle, so that cleaning
// it merges them. Polygons are cut into fans, as a file holding only triangles would have them.
template <class T>
Soup<T>
unmerged(const Soup<T>& a_soup)
{
  std::vector<Vec3<T>>                  vertices;
  std::vector<std::vector<std::size_t>> facets;

  for (const auto& facet : a_soup.getFacets()) {
    for (std::size_t i = 1; i + 1 < facet.size(); i++) {
      const std::size_t first = vertices.size();

      vertices.push_back(a_soup.getVertexCoordinates()[facet[0]]);
      vertices.push_back(a_soup.getVertexCoordinates()[facet[i]]);
      vertices.push_back(a_soup.getVertexCoordinates()[facet[i + 1]]);
      facets.push_back({first, first + 1, first + 2});
    }
  }

  return Soup<T>(a_soup.getID(), std::move(vertices), std::move(facets));
}

template <class T>
void
addCases(Perf::Suite& a_suite)
{
  const std::string prefix = "MeshIngest/armadillo";
  const std::string prec   = Perf::precisionName<T>();

  // ── The file read into a cleaned soup: the text parse, merging the vertices, removing degenerate faces ──

  a_suite.add(Perf::caseName({prefix, "read-soup", prec, "host"}), [=](Perf::Case& c) {
    const std::string file = Perf::meshFile(c.smoke());
    Soup<T>           soup;

    c.measure([&] { soup = EBGeometry::Parser::readIntoPolygonSoup<T>(file); });

    recordSoup(c, soup);
  });

  // ── Cleaning alone, of the soup a binary STL file gives: no text to parse ──

  a_suite.add(Perf::caseName({prefix, "clean", prec, "host"}), [=](Perf::Case& c) {
    const Soup<T> input = unmerged(EBGeometry::Parser::readIntoPolygonSoup<T>(Perf::meshFile(c.smoke())));
    Soup<T>       soup;

    c.measureFresh([&] { soup = input; }, [&] { soup.clean(); });
    c.param("vertices", input.numVertices());

    recordSoup(c, soup);
  });

  // ── The file read into a DCEL mesh: the soup, then the half-edges, their pairs and the normals ──

  a_suite.add(Perf::caseName({prefix, "read-dcel", prec, "host"}), [=](Perf::Case& c) {
    const std::string file  = Perf::meshFile(c.smoke());
    const auto        built = Perf::measureBuild<EBGeometry::DCEL::MeshT<T>>(
      c, [&](EBGeometry::Pool& a_pool) { return EBGeometry::Parser::readIntoDCEL<T>(file, a_pool); });

    const EBGeometry::DCEL::MeshT<T>& mesh = *built.object;

    std::vector<T>      values;
    Perf::IndexChecksum indices;

    for (std::uint32_t v = 0; v < mesh.numVertices(); v++) {
      for (int d = 0; d < 3; d++) {
        values.push_back(mesh.getVertex(v).getNormal()[d]);
      }
    }

    for (std::uint32_t f = 0; f < mesh.numFaces(); f++) {
      values.push_back(mesh.getFace(f).getArea());
    }

    for (std::uint32_t e = 0; e < mesh.numEdges(); e++) {
      indices.add(mesh.getEdge(e).getVertexIndex());
      indices.add(mesh.getEdge(e).getPairEdgeIndex());
    }

    c.checksum(Perf::checksumOf(values));
    c.indexChecksum(indices.value());
    c.poolBytes(built.pool->usedBytes());
  });

  // ── The file read into triangles: the DCEL mesh, then the triangles cut from its faces ──

  a_suite.add(Perf::caseName({prefix, "read-triangles", prec, "host"}), [=](Perf::Case& c) {
    const std::string                    file = Perf::meshFile(c.smoke());
    std::vector<EBGeometry::Triangle<T>> triangles;

    c.measure([&] { triangles = EBGeometry::Parser::readIntoTriangles<T>(file); });

    std::vector<T>      values;
    Perf::IndexChecksum indices;

    for (const auto& triangle : triangles) {
      for (int k = 0; k < 3; k++) {
        for (int d = 0; d < 3; d++) {
          values.push_back(triangle.getVertexPositions()[k][d]);
          values.push_back(triangle.getVertexNormals()[k][d]);
          values.push_back(triangle.getEdgeNormals()[k][d]);
        }
      }

      indices.add(triangle.getFaceId());
    }

    c.checksum(Perf::checksumOf(values));
    c.indexChecksum(indices.value());
  });
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "MeshIngest");

  addCases<float>(suite);
  addCases<double>(suite);

  return suite.run();
}
