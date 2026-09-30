// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Cross-format parser behaviour: binary fixtures against their ASCII counterparts, and malformed or
// truncated input, which must read as an empty mesh with a message rather than crash or yield a
// partial mesh.

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;

namespace {

using Meta = DCEL::DefaultMetaData;

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

// Writes a_content to a scratch file with the given name and returns its path.
std::string
writeScratch(const std::string& a_name, const std::string& a_content)
{
  const auto dir = std::filesystem::temp_directory_path() / "ebgeometry_test_parser";

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

    const auto ascii  = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron." + ext), pool);
    const auto binary = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron_binary." + ext), pool);

    REQUIRE(binary.numFaces() == 36);
    REQUIRE(binary.numVertices() == ascii.numVertices());

    const FlatMeshSDF<T, Meta> asciiSDF(ascii, pool);
    const FlatMeshSDF<T, Meta> binarySDF(binary, pool);

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

  const auto mesh = Parser::readIntoDCEL<T, Meta>(writeScratch("blank_lines.stl", content), pool);

  REQUIRE(mesh.numFaces() == 4);
  REQUIRE(mesh.numVertices() == 4);
}

TEMPLATE_TEST_CASE("Parser: missing, empty, truncated and corrupted files read as empty meshes",
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

  for (const auto& path : paths) {
    INFO("file: " << path);

    const auto mesh = Parser::readIntoDCEL<T, Meta>(path, pool);

    REQUIRE(mesh.numFaces() == 0);
  }
}

TEMPLATE_TEST_CASE("Parser: readIntoTriangleBVH keeps the intermediate DCEL mesh out of the caller's Pool",
                   "[Parser]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // A Pool never frees individual reservations, so a mesh built in the caller's Pool would stay there
  // and be mirrored to the device with the BVH.
  Pool direct(hostMemoryResource());

  const auto sdf = Parser::readIntoTriangleBVH<T, Meta, 4, 4>(dataPath("dodecahedron.stl"), direct);

  Pool       viaMesh(hostMemoryResource());
  const auto mesh      = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.stl"), viaMesh);
  const auto meshBytes = viaMesh.usedBytes();

  const TriMeshSDF<T, Meta, 4, 4> fromMesh(mesh, viaMesh, BVH::Build::SAH, 4);

  REQUIRE(meshBytes > 0);
  REQUIRE(direct.usedBytes() + meshBytes <= viaMesh.usedBytes() + PoolBaseAlign);
  REQUIRE(sdf.signedDistance(Vec3T<T>::zeros()) == fromMesh.signedDistance(Vec3T<T>::zeros()));
}

TEST_CASE("Mesh distance functions and BVH unions refuse to build from nothing", "[Parser][death]")
{
  using T = double;

  // EBGEOMETRY_REQUIREs, so they abort in every build.
  REQUIRE(abortsWith(
    [] {
      Pool                            pool(hostMemoryResource());
      const DCEL::MeshT<T, Meta>      empty;
      const TriMeshSDF<T, Meta, 4, 4> sdf(empty, pool, BVH::Build::SAH, 2);
    },
    "TriMeshSDF: the mesh has no faces"));

  REQUIRE(abortsWith(
    [] {
      Pool                       pool(hostMemoryResource());
      const DCEL::MeshT<T, Meta> empty;
      const MeshSDF<T, Meta, 4>  sdf(empty, pool, BVH::Build::SAH);
    },
    "MeshSDF: the mesh has no faces"));

  REQUIRE(abortsWith(
    [] {
      Pool                        pool(hostMemoryResource());
      [[maybe_unused]] const auto sdf =
        Parser::readIntoTriangleBVH<T, Meta, 4, 4>(dataPath("does_not_exist.stl"), pool);
    },
    "TriMeshSDF: the mesh has no faces"));

  REQUIRE(abortsWith(
    [] {
      Pool                                 pool(hostMemoryResource());
      const BVHUnionIF<T, SphereSDF<T>, 4> u(pool, {}, {});
    },
    "BVHUnionIF: a union needs at least one primitive"));
}
