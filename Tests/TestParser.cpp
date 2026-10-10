// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Cross-format parser behaviour: binary fixtures against their ASCII counterparts, and malformed or
// truncated input, which must throw Parser::ParseError rather than crash or yield a partial mesh.

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include <unistd.h>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

using namespace EBGeometry;

namespace {

std::string
dataPath(const std::string& a_filename)
{
  return std::string(EBGEOMETRY_TEST_DATA_DIR) + "/" + a_filename;
}

std::string
readBytes(const std::string& a_path)
{
  std::ifstream in(a_path, std::ios::binary);

  return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

// Writes a_content to a scratch file with the given name and returns its path. The directory is
// private to this process: ctest runs the float and double copies of a test in parallel, and with a
// shared directory one could rewrite a file while the other was reading it.
std::string
writeScratch(const std::string& a_name, const std::string& a_content)
{
  const auto dir = std::filesystem::temp_directory_path() / ("ebgeometry_test_parser_" + std::to_string(::getpid()));

  std::filesystem::create_directories(dir);

  const std::string path = (dir / a_name).string();

  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  out << a_content;

  return path;
}

template <class T>
std::vector<Vec3T<T>>
probePoints()
{
  return {Vec3T<T>(T(0), T(0), T(0)),
          Vec3T<T>(T(2), T(0.5), T(-0.25)),
          Vec3T<T>(T(-1.2), T(1.1), T(0.9)),
          Vec3T<T>(T(0.3), T(-2), T(1.7))};
}

} // namespace

TEMPLATE_TEST_CASE("Parser: binary STL, PLY and VTK fixtures read the same mesh as their ASCII counterparts",
                   "[Parser]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  // dodecahedron_binary.stl stores a colour in every facet's attribute bytes (two colours
  // alternating), as some exporters do. The reader must ignore it and return every facet.
  for (const std::string ext : {"stl", "ply", "vtk"}) {
    INFO("format: " << ext);

    const auto ascii  = Parser::readIntoDCEL<T>(dataPath("dodecahedron." + ext), pool);
    const auto binary = Parser::readIntoDCEL<T>(dataPath("dodecahedron_binary." + ext), pool);

    REQUIRE(binary.numFaces() == 36);
    REQUIRE(binary.numVertices() == ascii.numVertices());

    const FlatMeshSDF<T> asciiSDF(ascii, pool);
    const FlatMeshSDF<T> binarySDF(binary, pool);

    // Binary STL stores float32 coordinates, so compare at float precision.
    for (const auto& p : probePoints<T>()) {
      REQUIRE_THAT(binarySDF.signedDistance(p), withinAbsT(asciiSDF.signedDistance(p), looseMargin<float>()));
    }
  }
}

TEMPLATE_TEST_CASE("Parser: an ASCII STL with blank lines inside facets reads normally",
                   "[Parser]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  std::string content = readBytes(dataPath("tetrahedron.stl"));

  for (size_t pos = content.find("outer loop"); pos != std::string::npos; pos = content.find("outer loop", pos + 1)) {
    content.insert(content.find('\n', pos) + 1, "\n\n");
  }

  const auto mesh = Parser::readIntoDCEL<T>(writeScratch("blank_lines.stl", content), pool);

  REQUIRE(mesh.numFaces() == 4);
  REQUIRE(mesh.numVertices() == 4);
}

TEMPLATE_TEST_CASE("Parser: missing, empty, truncated and corrupted files throw ParseError",
                   "[Parser]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  std::vector<std::string> paths;

  paths.push_back(dataPath("does_not_exist.stl"));

  for (const std::string ext : {"stl", "ply", "vtk", "obj"}) {
    paths.push_back(writeScratch("empty." + ext, ""));
  }

  // Truncation is detectable in every format that states how much data follows (binary STL, PLY and
  // VTK counts) or how it ends (ASCII STL's 'endsolid'). OBJ states neither, so it is not listed.
  for (const std::string name : {"dodecahedron.stl",
                                 "dodecahedron.ply",
                                 "dodecahedron.vtk",
                                 "dodecahedron_binary.stl",
                                 "dodecahedron_binary.ply",
                                 "dodecahedron_binary.vtk"}) {
    const std::string content = readBytes(dataPath(name));

    for (const double fraction : {0.1, 0.5, 0.9, 0.99}) {
      paths.push_back(writeScratch("truncated_" + std::to_string(int(fraction * 100)) + "_" + name,
                                   content.substr(0, static_cast<size_t>(fraction * double(content.size())))));
    }
  }

  // A face that refers to a vertex the file does not have.
  paths.push_back(writeScratch("bad_index.obj", "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 7\n"));
  paths.push_back(writeScratch("bad_negative_index.obj", "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 -9\n"));

  // A PLY face whose corner count exceeds the number of vertices, and a legacy VTK polygon whose
  // corner count does.
  paths.push_back(writeScratch("bad_list.ply",
                               "ply\nformat ascii 1.0\nelement vertex 3\nproperty float x\nproperty float y\n"
                               "property float z\nelement face 1\nproperty list uchar int vertex_indices\n"
                               "end_header\n0 0 0\n1 0 0\n0 1 0\n200 0 1 2\n"));
  paths.push_back(writeScratch("bad_count.vtk",
                               "# vtk DataFile Version 3.0\nbad\nASCII\nDATASET POLYDATA\nPOINTS 3 float\n"
                               "0 0 0 1 0 0 0 1 0\nPOLYGONS 1 4\n1000000000 0 1 2\n"));

  // A header count that is not a number, and counts so large that reserving them outright would fail.
  paths.push_back(writeScratch("bad_header_count.ply",
                               "ply\nformat ascii 1.0\nelement vertex three\nproperty float x\nproperty float y\n"
                               "property float z\nend_header\n"));
  paths.push_back(writeScratch("huge_header_count.ply",
                               "ply\nformat ascii 1.0\nelement vertex 999999999999999\nproperty float x\n"
                               "property float y\nproperty float z\nend_header\n0 0 0\n"));
  paths.push_back(writeScratch("huge_count.vtk",
                               "# vtk DataFile Version 3.0\nbad\nASCII\nDATASET POLYDATA\n"
                               "POINTS 999999999999999 float\n0 0 0\n"));

  for (const auto& path : paths) {
    INFO("file: " << path);

    REQUIRE_THROWS_AS((Parser::readIntoDCEL<T>(path, pool)), Parser::ParseError);
  }

  // The readers that build a distance function throw before building anything.
  REQUIRE_THROWS_AS((Parser::readIntoTriMeshSDF<T, 4, 4>(dataPath("does_not_exist.stl"), pool)), Parser::ParseError);
  REQUIRE_THROWS_AS((Parser::readIntoMeshSDF<T, 4>(dataPath("does_not_exist.stl"), pool)), Parser::ParseError);
}

TEMPLATE_TEST_CASE("Parser: ParseError names the file, the line and the reason", "[Parser]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  const auto check = [&pool](const std::string& a_path, const std::size_t a_line, const std::string& a_reason) {
    INFO("file: " << a_path);

    try {
      [[maybe_unused]] const auto mesh = Parser::readIntoDCEL<T>(a_path, pool);

      FAIL("no ParseError was thrown");
    } catch (const Parser::ParseError& e) {
      REQUIRE(e.file() == a_path);
      REQUIRE(e.line() == a_line);
      REQUIRE(e.reason() == a_reason);

      const std::string where = a_line > 0 ? a_path + ":" + std::to_string(a_line) : a_path;

      REQUIRE(std::string(e.what()) == where + ": " + a_reason);
    }
  };

  check(dataPath("does_not_exist.obj"), 0, "cannot open the file");
  check(dataPath("does_not_exist.stl"), 0, "cannot open the file");

  check(writeScratch("bad_vertex.stl",
                     "solid s\nfacet normal 0 0 1\nouter loop\nvertex 0 0 0\nvertex 1 zero 0\nvertex 0 1 0\n"
                     "endloop\nendfacet\nendsolid s\n"),
        5,
        "malformed vertex line 'vertex 1 zero 0'");

  check(writeScratch("no_endsolid.stl",
                     "solid s\nfacet normal 0 0 1\nouter loop\nvertex 0 0 0\nvertex 1 0 0\nvertex 0 1 0\n"
                     "endloop\nendfacet\n"),
        8,
        "the file ends without 'endsolid'; it is probably truncated");

  check(writeScratch("bad_vertex.obj", "v 0 0 0\nv 1 0\nv 0 1 0\nf 1 2 3\n"), 2, "malformed vertex line 'v 1 0'");
  check(writeScratch("bad_face.obj", "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 two 3\n"), 4, "malformed face index 'two'");
  check(writeScratch("zero_index.obj", "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 0 1 2\n"), 4, "malformed face index '0'");

  check(writeScratch("bad_header_count.ply",
                     "ply\nformat ascii 1.0\nelement vertex three\nproperty float x\nproperty float y\n"
                     "property float z\nend_header\n"),
        3,
        "the vertex count 'three' is not a number");

  check(writeScratch("no_faces.obj", "v 0 0 0\nv 1 0 0\nv 0 1 0\n"), 0, "the mesh has no faces");
}

TEMPLATE_TEST_CASE("Parser: faces that cannot form a half-edge mesh throw ParseError",
                   "[Parser]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  const std::string verts = "v 0 0 0\nv 1 0 0\nv 0 1 0\nv 0 0 1\n";

  const auto throwsWith = [&pool](const std::string& a_path, const std::string& a_reason) {
    INFO("file: " << a_path);

    REQUIRE_THROWS_WITH((Parser::readIntoDCEL<T>(a_path, pool)), Catch::Matchers::ContainsSubstring(a_reason));
  };

  // A damaged 'endfacet' line, after which the next facet's vertices are read into this one.
  throwsWith(writeScratch("merged_facets.stl",
                          "solid s\nfacet normal 0 0 1\nouter loop\nvertex 0 0 0\nvertex 0 1 0\nvertex 1 0 0\n"
                          "endloop\nendfacet\"facet normal 0 0 1\nouter loop\nvertex 1 0 0\nvertex 0 1 0\n"
                          "vertex 0 0 1\nendloop\nendfacet\nendsolid s\n"),
             "merged_facets.stl:14: the facet ending here has 6 vertices; an STL facet has exactly 3");

  // One face of the tetrahedron wound the wrong way round.
  throwsWith(writeScratch("flipped.obj", verts + "f 1 2 3\nf 1 2 4\nf 1 4 3\nf 2 3 4\n"),
             "they are oriented inconsistently, or more than two faces share that edge");

  // A flat, double-sided triangle: two copies with opposite windings. Every edge is shared correctly,
  // but the two faces on either side of it fold back onto each other.
  throwsWith(writeScratch("folded.obj", verts + "f 1 2 3\nf 1 3 2\n"), "fold back onto each other");
}

TEMPLATE_TEST_CASE("Parser: with OnDefect::Warn, inconsistent orientation and folds load with a warning",
                   "[Parser][OnDefect]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const std::string verts   = "v 0 0 0\nv 1 0 0\nv 0 1 0\nv 0 0 1\n";
  const std::string flipped = writeScratch("warn_flipped.obj", verts + "f 1 2 3\nf 1 2 4\nf 1 4 3\nf 2 3 4\n");
  const std::string folded  = writeScratch("warn_folded.obj", verts + "f 1 2 3\nf 1 3 2\n");

  Pool pool(hostMemoryResource());

  // The default still throws.
  REQUIRE_THROWS_AS((Parser::readIntoDCEL<T>(flipped, pool)), Parser::ParseError);

  for (const auto& file : {flipped, folded}) {
    INFO("file: " << file);

    // The warning goes to std::cerr; capture it.
    std::ostringstream captured;
    std::streambuf*    old = std::cerr.rdbuf(captured.rdbuf());

    DCEL::MeshT<T> mesh;

    REQUIRE_NOTHROW(mesh = Parser::readIntoDCEL<T>(file, pool, Parser::OnDefect::Warn));

    std::cerr.rdbuf(old);

    REQUIRE(mesh.numFaces() > 0);
    REQUIRE_THAT(captured.str(), Catch::Matchers::ContainsSubstring("loading it anyway"));

    // The mesh answers queries, and the BVH agrees with the brute-force scan.
    const FlatMeshSDF<T> flat(mesh, pool);
    const MeshSDF<T, 4>  packed(mesh, pool, BVH::Construction::SAH);

    for (const auto& q : {Vec3T<T>(T(0.2), T(0.2), T(0.2)), Vec3T<T>(T(2), T(-1), T(0.5))}) {
      REQUIRE(std::isfinite(flat.signedDistance(q)));
      REQUIRE(packed.signedDistance(q) == flat.signedDistance(q));
    }
  }

  // Every reader that builds a mesh accepts the option.
  std::ostringstream captured;
  std::streambuf*    old = std::cerr.rdbuf(captured.rdbuf());

  REQUIRE_NOTHROW(Parser::readIntoFlatMeshSDF<T>(flipped, pool, Parser::OnDefect::Warn));
  REQUIRE_NOTHROW(Parser::readIntoMeshSDF<T>(flipped, pool, BVH::Construction::SAH, Parser::OnDefect::Warn));
  REQUIRE_NOTHROW(Parser::readIntoTriMeshSDF<T>(flipped, pool, BVH::Construction::SAH, Parser::OnDefect::Warn));
  REQUIRE_NOTHROW(Parser::readIntoTriMeshSDF<T>(folded, pool, BVH::Construction::SAH, Parser::OnDefect::Warn));
  REQUIRE_NOTHROW(Parser::readIntoTriangles<T>(flipped, Parser::OnDefect::Warn));
  REQUIRE_NOTHROW(Parser::readIntoDCEL<T>(std::vector<std::string>{flipped, folded}, pool, Parser::OnDefect::Warn));

  // Near a fold only the sign is unreliable: the double-sided triangle lies in z = 0, so a point
  // 0.5 above its interior is 0.5 away, whatever sign each mesh SDF gives it.
  const Vec3T<T> above(T(0.2), T(0.2), T(0.5));

  const auto foldMesh = Parser::readIntoMeshSDF<T>(folded, pool, BVH::Construction::SAH, Parser::OnDefect::Warn);
  const auto foldTris = Parser::readIntoTriMeshSDF<T>(folded, pool, BVH::Construction::SAH, Parser::OnDefect::Warn);

  std::cerr.rdbuf(old);

  REQUIRE_THAT(std::abs(foldMesh.signedDistance(above)), withinAbsT(T(0.5), looseMargin<T>()));
  REQUIRE_THAT(std::abs(foldTris.signedDistance(above)), withinAbsT(T(0.5), looseMargin<T>()));
}

TEMPLATE_TEST_CASE("Parser: a face that visits a vertex twice throws even with OnDefect::Warn",
                   "[Parser][OnDefect]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // A cube of quads with one side written as a pentagon that returns to its first vertex. The
  // half-edge mesh built from such a face is corrupt, so it is never loaded.
  const std::string cube = "v 0 0 0\nv 1 0 0\nv 1 1 0\nv 0 1 0\nv 0 0 1\nv 1 0 1\nv 1 1 1\nv 0 1 1\n"
                           "f 1 4 3 2\nf 5 6 7 8\nf 1 2 6 5\nf 2 3 7 6\nf 3 4 8 7\nf 4 1 5 8 1\n";

  Pool pool(hostMemoryResource());

  REQUIRE_THROWS_WITH(
    (Parser::readIntoDCEL<T>(writeScratch("repeated_vertex.obj", cube), pool, Parser::OnDefect::Warn)),
    Catch::Matchers::ContainsSubstring("visits the same vertex twice"));
}

TEST_CASE("Soup::findRepeatedVertex reports only a face that visits a vertex twice", "[Parser]")
{
  REQUIRE(Soup::findRepeatedVertex({{0, 2, 1}, {0, 1, 3}}).empty());
  REQUIRE(Soup::findRepeatedVertex({{0, 1, 2}, {0, 1, 2, 3, 1}}) == "face 1 visits the same vertex twice");

  // An edge used twice in one direction is findTopologyDefect's business, not this one's.
  REQUIRE(Soup::findRepeatedVertex({{0, 1, 2}, {0, 1, 3}}).empty());
}

TEST_CASE("Soup::findTopologyDefect reports faces that cannot be joined into a half-edge mesh", "[Parser]")
{
  // A closed tetrahedron has no defect.
  const std::vector<std::vector<size_t>> tet = {{0, 2, 1}, {0, 1, 3}, {0, 3, 2}, {1, 2, 3}};

  REQUIRE(Soup::findTopologyDefect(tet).empty());

  // An open surface (one face missing) has none either: a hole is not a topology defect.
  REQUIRE(Soup::findTopologyDefect({{0, 2, 1}, {0, 1, 3}, {0, 3, 2}}).empty());

  REQUIRE(Soup::findTopologyDefect({{0, 1, 2, 3, 1}}) == "face 0 visits the same vertex twice");

  REQUIRE(Soup::findTopologyDefect({{0, 1, 2}, {0, 1, 3}}) ==
          "faces 0 and 1 both run from vertex 0 to vertex 1: they are oriented inconsistently, or more than two "
          "faces share that edge");

  // Three faces on one edge: whatever their orientation, one direction is used twice.
  REQUIRE_FALSE(Soup::findTopologyDefect({{0, 1, 2}, {1, 0, 3}, {1, 0, 4}}).empty());
}

TEMPLATE_TEST_CASE("Parser: OBJ vertices that no face uses, and comments after a face, are ignored",
                   "[Parser]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // A tetrahedron, as is, with an extra vertex that no face refers to, and with a comment after its
  // first face. OBJ allows the first; some exporters write the second.
  const std::string verts = "v 0 0 0\nv 1 0 0\nv 0 1 0\nv 0 0 1\n";
  const std::string first = "f 1 3 2";
  const std::string rest  = "f 1 2 4\nf 1 4 3\nf 2 3 4\n";

  Pool       pool(hostMemoryResource());
  const auto plain = Parser::readIntoDCEL<T>(writeScratch("tet.obj", verts + first + "\n" + rest), pool);
  const auto unused =
    Parser::readIntoDCEL<T>(writeScratch("tet_unused.obj", verts + "v 5 5 5\n" + first + "\n" + rest), pool);
  const auto noted =
    Parser::readIntoDCEL<T>(writeScratch("tet_comment.obj", verts + first + " # the base\n" + rest), pool);

  REQUIRE(unused.numFaces() == 4);
  REQUIRE(noted.numFaces() == 4);

  for (const auto& p : probePoints<T>()) {
    REQUIRE(unused.signedDistance(p) == plain.signedDistance(p));
    REQUIRE(noted.signedDistance(p) == plain.signedDistance(p));
  }
}

TEMPLATE_TEST_CASE("Parser: readIntoTriMeshSDF keeps the intermediate DCEL mesh out of the caller's Pool",
                   "[Parser]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // A Pool never frees individual reservations, so a mesh built in the caller's Pool would stay there
  // and be mirrored to the device with the BVH.
  Pool direct(hostMemoryResource());

  const auto sdf = Parser::readIntoTriMeshSDF<T, 4, 4>(dataPath("dodecahedron.stl"), direct);

  Pool       viaMesh(hostMemoryResource());
  const auto mesh      = Parser::readIntoDCEL<T>(dataPath("dodecahedron.stl"), viaMesh);
  const auto meshBytes = viaMesh.usedBytes();

  const TriMeshSDF<T, 4, 4> fromMesh(mesh, viaMesh, BVH::Construction::SAH, 4);

  REQUIRE(meshBytes > 0);
  REQUIRE(direct.usedBytes() + meshBytes <= viaMesh.usedBytes() + PoolBaseAlign);
  REQUIRE(sdf.signedDistance(Vec3T<T>::zeros()) == fromMesh.signedDistance(Vec3T<T>::zeros()));
}

TEMPLATE_TEST_CASE("Parser: every reader returns a clean soup, and readIntoPolygonSoup picks the reader by extension",
                   "[Parser]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  for (const std::string file : {"dodecahedron.stl",
                                 "dodecahedron_binary.stl",
                                 "dodecahedron.ply",
                                 "dodecahedron_binary.ply",
                                 "dodecahedron.vtk",
                                 "dodecahedron_binary.vtk",
                                 "dodecahedron.obj"}) {
    INFO("file: " << file);

    const PolygonSoup<T> soup = Parser::readIntoPolygonSoup<T>(dataPath(file));

    CHECK(soup.isClean());
    CHECK(soup.getID() == dataPath(file));
    CHECK(soup.numVertices() == 20);
    CHECK(soup.numFacets() == 36);
  }

  CHECK(Parser::readSTL<T>(dataPath("tetrahedron.stl")).numVertices() == 4);
  CHECK(
    Parser::readIntoPolygonSoup<T>(std::vector<std::string>{dataPath("tetrahedron.stl"), dataPath("cube_quads.obj")})
      .size() == 2);

  CHECK_THROWS_WITH(Parser::readIntoPolygonSoup<T>("mesh.off"),
                    "mesh.off: unsupported file type; the extension must be .stl, .ply, .vtk or .obj");
}

TEMPLATE_TEST_CASE("Parser: PLY and VTK properties stay aligned with the face ids the mesh SDFs report",
                   "[Parser]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // A tetrahedron with A = (0,0,0), B = (1,0,0), C = (0,1,0) and D = (0,0,1). File face 1 is
  // degenerate (it visits A twice) and is removed, so file faces 0, 2, 3 and 4 become faces 0 to 3.
  // In the PLY file, vertex 4 is a second copy of A, merged into it.
  const std::string ply = "ply\nformat ascii 1.0\nelement vertex 5\nproperty float x\nproperty float y\n"
                          "property float z\nproperty float temperature\nelement face 5\n"
                          "property list uchar int vertex_indices\nproperty float quality\nend_header\n"
                          "0 0 0 1\n1 0 0 2\n0 1 0 3\n0 0 1 4\n0 0 0 5\n"
                          "3 0 2 1 10\n3 0 0 1 99\n3 4 1 3 20\n3 0 3 2 30\n3 1 2 3 40\n";

  const std::string vtk = "# vtk DataFile Version 3.0\ntetrahedron\nASCII\nDATASET POLYDATA\n"
                          "POINTS 4 float\n0 0 0\n1 0 0\n0 1 0\n0 0 1\n"
                          "POLYGONS 5 20\n3 0 2 1\n3 0 0 1\n3 0 1 3\n3 0 3 2\n3 1 2 3\n"
                          "POINT_DATA 4\nSCALARS temperature float 1\nLOOKUP_TABLE default\n1 2 3 4\n"
                          "CELL_DATA 5\nSCALARS quality float 1\nLOOKUP_TABLE default\n10 99 20 30 40\n";

  // The temperature at each corner, and the quality of the face nearest each probe point.
  const std::vector<std::pair<Vec3T<T>, T>> temperatures = {
    {Vec3T<T>(0, 0, 0), T(1)}, {Vec3T<T>(1, 0, 0), T(2)}, {Vec3T<T>(0, 1, 0), T(3)}, {Vec3T<T>(0, 0, 1), T(4)}};

  const std::vector<std::pair<Vec3T<T>, T>> qualities = {{Vec3T<T>(T(0.25), T(0.25), T(-1)), T(10)},
                                                         {Vec3T<T>(T(0.25), T(-1), T(0.25)), T(20)},
                                                         {Vec3T<T>(T(-1), T(0.25), T(0.25)), T(30)},
                                                         {Vec3T<T>(T(1), T(1), T(1)), T(40)}};

  std::ostringstream captured;
  std::streambuf*    old = std::cerr.rdbuf(captured.rdbuf());

  const std::vector<std::string> files = {writeScratch("properties.ply", ply), writeScratch("properties.vtk", vtk)};

  for (const auto& file : files) {
    INFO("file: " << file);

    Pool pool(hostMemoryResource());

    const PolygonSoup<T> soup = Parser::readIntoPolygonSoup<T>(file);

    REQUIRE(soup.numVertices() == 4);
    REQUIRE(soup.numFacets() == 4);
    REQUIRE(soup.getVertexPropertyNames() == std::vector<std::string>{"temperature"});
    REQUIRE(soup.getFacePropertyNames() == std::vector<std::string>{"quality"});

    for (const auto& corner : temperatures) {
      size_t v = 0;

      while (soup.getVertexCoordinates()[v] != corner.first) {
        v++;
      }

      CHECK(soup.getVertexProperty("temperature")[v] == corner.second);
    }

    const auto flat = Parser::readIntoFlatMeshSDF<T>(file, pool);
    const auto mesh = Parser::readIntoMeshSDF<T>(file, pool);
    const auto tris = Parser::readIntoTriMeshSDF<T>(file, pool);

    for (const auto& probe : qualities) {
      CHECK(soup.getFaceProperty("quality")[flat.getClosestFace(probe.first).faceId] == probe.second);
      CHECK(soup.getFaceProperty("quality")[mesh.getClosestFace(probe.first).faceId] == probe.second);
      CHECK(soup.getFaceProperty("quality")[tris.getClosestFace(probe.first).faceId] == probe.second);
    }
  }

  // CELL_DATA that also covers cells that are not polygons cannot be aligned with the faces.
  const std::string extraCells = vtk.substr(0, vtk.find("CELL_DATA")) +
                                 "CELL_DATA 6\nSCALARS quality float 1\nLOOKUP_TABLE default\n10 99 20 30 40 50\n";

  captured.str("");

  const PolygonSoup<T> dropped = Parser::readVTK<T>(writeScratch("extra_cells.vtk", extraCells));

  std::cerr.rdbuf(old);

  CHECK(dropped.numFacets() == 4);
  CHECK(dropped.hasVertexProperty("temperature"));
  CHECK_FALSE(dropped.hasFaceProperty("quality"));
  CHECK_THAT(captured.str(), Catch::Matchers::ContainsSubstring("skipping face property 'quality'"));
}

TEMPLATE_TEST_CASE("Parser: the readers taking leaf-size settings build what the constructors build",
                   "[Parser]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using Packed = MeshSDF<T, 4>;
  using Tri    = TriMeshSDF<T, 4, 4>;

  const std::string file = dataPath("dodecahedron.stl");

  Pool pool(hostMemoryResource());

  BVH::ConstructionOptions packedOptions = Packed::defaultConstructionOptions();
  BVH::ConstructionOptions triOptions    = Tri::defaultConstructionOptions(1);

  packedOptions.maxLeafSize = 8;
  triOptions.maxLeafSize    = 2;

  const auto mesh = Parser::readIntoDCEL<T>(file, pool);

  const Packed packed(mesh, pool, BVH::Construction::SAH, packedOptions);
  const Tri    tri(mesh, pool, BVH::Construction::SAH, triOptions);

  const auto packedRead = Parser::readIntoMeshSDF<T, 4>(file, pool, BVH::Construction::SAH, packedOptions);
  const auto triRead    = Parser::readIntoTriMeshSDF<T, 4, 4>(file, pool, BVH::Construction::SAH, triOptions);

  const auto packedReads =
    Parser::readIntoMeshSDF<T, 4>(std::vector<std::string>{file, file}, pool, BVH::Construction::SAH, packedOptions);
  const auto triReads =
    Parser::readIntoTriMeshSDF<T, 4, 4>(std::vector<std::string>{file, file}, pool, BVH::Construction::SAH, triOptions);

  REQUIRE(packedReads.size() == 2);
  REQUIRE(triReads.size() == 2);

  CHECK(packedRead.getRoot().getNodes().size() == packed.getRoot().getNodes().size());
  CHECK(triRead.getRoot().getNodes().size() == tri.getRoot().getNodes().size());
  CHECK(packedReads[1].getRoot().getNodes().size() == packed.getRoot().getNodes().size());
  CHECK(triReads[1].getRoot().getNodes().size() == tri.getRoot().getNodes().size());

  for (const auto& p : probePoints<T>()) {
    CHECK(packedRead.signedDistance(p) == packed.signedDistance(p));
    CHECK(triRead.signedDistance(p) == tri.signedDistance(p));
  }

  // The default for TriMeshSDF is four groups per leaf.
  const Tri  fourGroups(mesh, pool, BVH::Construction::SAH, 4);
  const auto defaulted = Parser::readIntoTriMeshSDF<T, 4, 4>(file, pool);

  CHECK(defaulted.getRoot().getNodes().size() == fourGroups.getRoot().getNodes().size());
}

TEST_CASE("Mesh distance functions and BVH unions refuse to build from nothing", "[Parser][death]")
{
  using T = double;

  // EBGEOMETRY_REQUIREs, so they abort in every build.
  REQUIRE(abortsWith(
    [] {
      Pool                      pool(hostMemoryResource());
      const DCEL::MeshT<T>      empty;
      const TriMeshSDF<T, 4, 4> sdf(empty, pool, BVH::Construction::SAH, 2);
    },
    "TriMeshSDF: the mesh has no faces"));

  REQUIRE(abortsWith(
    [] {
      Pool                 pool(hostMemoryResource());
      const DCEL::MeshT<T> empty;
      const MeshSDF<T, 4>  sdf(empty, pool, BVH::Construction::SAH);
    },
    "MeshSDF: the mesh has no faces"));

  REQUIRE(abortsWith(
    [] {
      Pool                               pool(hostMemoryResource());
      const BVHUnion<T, SphereSDF<T>, 4> u(pool, {}, {});
    },
    "BVHUnion: a union needs at least one primitive"));
}
