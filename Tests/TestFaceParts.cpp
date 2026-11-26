// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// Parts of a mesh picked by face: the parts file read and written, its checks, and the overlay on a file's groups.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

using namespace EBGeometry;

namespace {

namespace fs = std::filesystem;

std::string
dataFile(const std::string& a_filename)
{
  return std::string(EBGEOMETRY_TEST_DATA_DIR) + "/" + a_filename;
}

fs::path
scratchFolder(const std::string& a_name)
{
  const fs::path folder = fs::temp_directory_path() / ("ebgeometry-face-parts-" + a_name);

  fs::remove_all(folder);
  fs::create_directories(folder);

  return folder;
}

void
write(const fs::path& a_file, const std::string& a_text)
{
  std::ofstream(a_file, std::ios::binary) << a_text;
}

std::string
readText(const fs::path& a_file)
{
  std::ifstream      in(a_file, std::ios::binary);
  std::ostringstream out;

  out << in.rdbuf();

  return out.str();
}

std::string
hex(const uint64_t a_value)
{
  std::ostringstream out;

  out << std::hex << a_value;

  return out.str();
}

/**
 * @brief The reason a parts file is rejected for, or "" if it reads.
 */
template <class T>
std::string
rejection(const fs::path& a_file, const PolygonSoup<T>& a_soup)
{
  try {
    (void)Parser::readFaceParts(a_file.string(), a_soup);
  } catch (const Parser::ParseError& e) {
    return e.reason();
  }

  return "";
}

} // namespace

TEMPLATE_TEST_CASE("FaceParts: the fingerprint follows the face numbering, not the precision or the coordinates",
                   "[FaceParts]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto cube  = Parser::readIntoPolygonSoup<T>(dataFile("cube_groups.obj"));
  const auto dodec = Parser::readIntoPolygonSoup<T>(dataFile("dodecahedron.obj"));

  // The same file in the other precision: the same fingerprint.
  CHECK(Parser::meshFingerprint(cube) ==
        Parser::meshFingerprint(Parser::readIntoPolygonSoup<double>(dataFile("cube_groups.obj"))));

  // Another mesh: another fingerprint.
  CHECK(Parser::meshFingerprint(cube) != Parser::meshFingerprint(dodec));

  // Moved vertices with the same faces: the same fingerprint, since every face id still names the same face.
  const fs::path folder = scratchFolder("fingerprint-" + std::to_string(sizeof(T)));

  std::string moved = readText(dataFile("cube_groups.obj"));

  moved.replace(moved.find("v 1 1 1"), 7, "v 2 2 2");

  write(folder / "moved.obj", moved);

  CHECK(Parser::meshFingerprint(Parser::readIntoPolygonSoup<T>((folder / "moved.obj").string())) ==
        Parser::meshFingerprint(cube));

  // The faces in another order: another fingerprint, since face ids now name other faces.
  std::string swapped = readText(dataFile("cube_groups.obj"));

  swapped.replace(swapped.find("f 1 4 3 2"), 9, "f 5 6 7 8");
  swapped.replace(swapped.rfind("f 5 6 7 8"), 9, "f 1 4 3 2");

  write(folder / "swapped.obj", swapped);

  CHECK(Parser::meshFingerprint(Parser::readIntoPolygonSoup<T>((folder / "swapped.obj").string())) !=
        Parser::meshFingerprint(cube));
}

TEMPLATE_TEST_CASE("FaceParts: a parts file is read into one entry per face, with ranges and continued parts",
                   "[FaceParts]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto     soup   = Parser::readIntoPolygonSoup<T>(dataFile("dodecahedron.obj"));
  const fs::path folder = scratchFolder("read-" + std::to_string(sizeof(T)));

  REQUIRE(soup.numFacets() == 36U);

  write(folder / "dodecahedron.parts",
        "# Picked in the studio.\n"
        "ebg-parts 1\n"
        "mesh \"dodecahedron.obj\" fingerprint " +
          hex(Parser::meshFingerprint(soup)) +
          "\n"
          "\n"
          "inlet 0 2-4   # the inlet\n"
          "outlet 11\n"
          "inlet 7\n"
          "empty\n");

  const FaceGroups parts = Parser::readFaceParts((folder / "dodecahedron.parts").string(), soup);

  REQUIRE(parts.m_names == std::vector<std::string>{"inlet", "outlet", "empty"});

  std::vector<uint32_t> expected(36, FaceGroups::None);

  for (const size_t f : {0, 2, 3, 4, 7}) {
    expected[f] = 0;
  }

  expected[11] = 1;

  CHECK(parts.m_group == expected);
}

TEMPLATE_TEST_CASE("FaceParts: writing and reading back gives the same parts", "[FaceParts]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto     soup   = Parser::readIntoPolygonSoup<T>(dataFile("dodecahedron.obj"));
  const fs::path folder = scratchFolder("roundtrip-" + std::to_string(sizeof(T)));
  const uint32_t N      = FaceGroups::None;

  FaceGroups parts;

  parts.m_names = {"a", "b", "nothing"};
  parts.m_group = {1, 1, 1, N, 0, 1, 1, N, 0, 0, 0, N};

  parts.m_group.resize(soup.numFacets(), N);

  Parser::writeFaceParts((folder / "x.parts").string(), parts, soup, "dodecahedron.obj");

  // Consecutive faces as ranges; a part with no faces kept by its name.
  const std::string text = readText(folder / "x.parts");

  CHECK_THAT(text, Catch::Matchers::StartsWith("ebg-parts 1\nmesh \"dodecahedron.obj\" fingerprint "));
  CHECK_THAT(text, Catch::Matchers::ContainsSubstring("\na 4 8-10\n"));
  CHECK_THAT(text, Catch::Matchers::ContainsSubstring("\nb 0-2 5-6\n"));
  CHECK_THAT(text, Catch::Matchers::EndsWith("\nnothing\n"));

  const FaceGroups back = Parser::readFaceParts((folder / "x.parts").string(), soup);

  CHECK(back.m_names == parts.m_names);
  CHECK(back.m_group == parts.m_group);

  // A long part is continued on further lines, and still reads back.
  const auto big = Parser::readIntoPolygonSoup<T>(dataFile("lblock.stl"));

  FaceGroups every;

  every.m_names = {"every_other"};
  every.m_group.assign(big.numFacets(), N);

  for (size_t f = 0; f < big.numFacets(); f += 2) {
    every.m_group[f] = 0;
  }

  Parser::writeFaceParts((folder / "big.parts").string(), every, big, "lblock.stl");

  std::istringstream lines(readText(folder / "big.parts"));
  std::string        line;

  while (std::getline(lines, line)) {
    CHECK(line.size() <= 110U);
  }

  CHECK(Parser::readFaceParts((folder / "big.parts").string(), big).m_group == every.m_group);
}

TEMPLATE_TEST_CASE("FaceParts: a malformed parts file, or one for another mesh, is rejected at its line",
                   "[FaceParts]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto        soup   = Parser::readIntoPolygonSoup<T>(dataFile("dodecahedron.obj"));
  const fs::path    folder = scratchFolder("errors-" + std::to_string(sizeof(T)));
  const std::string head   = "ebg-parts 1\nmesh \"d.obj\" fingerprint " + hex(Parser::meshFingerprint(soup)) + "\n";

  struct Case
  {
    std::string m_text;
    std::string m_reason;
    size_t      m_line;
  };

  const std::vector<Case> cases{
    {"ebg-parts 2\n", "expected 'ebg-parts 1'", 1},
    {"ebg-parts 1\nmesh d.obj fingerprint 12\n", "expected 'mesh \"<file>\" fingerprint <hex>'", 2},
    {"ebg-parts 1\nmesh \"d.obj\" fingerprint xyz\n", "is not a hexadecimal number", 2},
    {"ebg-parts 1\nmesh \"d.obj\" fingerprint 1234\n", "picked on another version of the mesh 'd.obj'", 2},
    {head + "a 1 x\n", "'x' is not a face id", 3},
    {head + "a 5-3\n", "'5-3' is not a face id", 3},
    {head + "a -3\n", "'-3' is not a face id", 3},
    {head + "a 36\n", "face 36 of part 'a' is out of range (the mesh has 36 faces)", 3},
    {head + "a 1-4\nb 4\n", "face 4 is in two parts, 'a' and 'b'", 4},
    {"# nothing but a comment\n", "has no 'ebg-parts 1' and 'mesh' lines", 1},
  };

  for (const Case& c : cases) {
    INFO(c.m_text);

    write(folder / "bad.parts", c.m_text);

    try {
      (void)Parser::readFaceParts((folder / "bad.parts").string(), soup);

      FAIL("the parts file reads");
    } catch (const Parser::ParseError& e) {
      CHECK_THAT(e.reason(), Catch::Matchers::ContainsSubstring(c.m_reason));
      CHECK(e.line() == c.m_line);
    }
  }

  CHECK_THAT(rejection(folder / "missing.parts", soup), Catch::Matchers::ContainsSubstring("cannot open"));

  // Writing checks the parts too.
  FaceGroups parts;

  parts.m_names = {"has space"};
  parts.m_group.assign(soup.numFacets(), FaceGroups::None);

  CHECK_THROWS_AS(Parser::writeFaceParts((folder / "w.parts").string(), parts, soup, "d.obj"), Parser::ParseError);

  parts.m_names = {"ok"};
  parts.m_group.assign(3, 0);

  CHECK_THROWS_AS(Parser::writeFaceParts((folder / "w.parts").string(), parts, soup, "d.obj"), Parser::ParseError);
}

TEMPLATE_TEST_CASE("FaceParts: picked parts lie on top of the file's groups", "[FaceParts]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto       soup = Parser::readIntoPolygonSoup<T>(dataFile("cube_groups.obj"));
  const FaceGroups file = soup.getFaceGroups();
  const uint32_t   N    = FaceGroups::None;

  // cube_groups.obj: bottom, top, walls (three faces) and one face in no group.
  REQUIRE(file.m_names == std::vector<std::string>{"bottom", "top", "walls"});
  REQUIRE(file.m_group == std::vector<uint32_t>{0, 1, 2, 2, 2, N});

  // A wall and the ungrouped face picked as "inlet"; another wall picked as "top", joining that group.
  FaceGroups picked;

  picked.m_names = {"inlet", "top"};
  picked.m_group = {N, N, 0, 1, N, 0};

  const FaceGroups both = Parser::overlayFaceGroups(file, picked);

  CHECK(both.m_names == std::vector<std::string>{"bottom", "top", "walls", "inlet"});
  CHECK(both.m_group == std::vector<uint32_t>{0, 1, 3, 1, 2, 3});

  // A mesh file with no groups: the picked parts alone.
  const FaceGroups alone = Parser::overlayFaceGroups(FaceGroups{}, picked);

  CHECK(alone.m_names == picked.m_names);
  CHECK(alone.m_group == picked.m_group);

  // Another face count: an error, not a wrong overlay.
  FaceGroups wrong = picked;

  wrong.m_group.push_back(N);

  CHECK_THROWS_AS(Parser::overlayFaceGroups(file, wrong), Parser::ParseError);
}
