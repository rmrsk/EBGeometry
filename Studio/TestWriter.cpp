// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Tests of ebgstudio's writer (Writer.hpp): the viewer's edits of a domain written into a copy of a model folder, as
// the viewer makes them (a clicked surface, a renamed group, a new domain), then read again; undo and redo; no write
// over a file changed on disk; and no write at all when one of its files cannot be written.

#include "Writer.hpp"

#include <filesystem>
#include <fstream>
#include <string>

#include <unistd.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

using namespace EBGeometry;

namespace {

using T = double;

// A fresh copy of the test model folder.
std::string
copyOfWire(const std::string& a_name)
{
  namespace fs = std::filesystem;

  const fs::path to = fs::temp_directory_path() / ("ebgstudio-writer-" + a_name);

  fs::remove_all(to);
  fs::create_directories(to);

  for (const char* file : {"model.ebg", "setup.ebg"}) {
    fs::copy_file(fs::path(EBGEOMETRY_TEST_DATA_DIR) / "language" / "wire" / file, to / file);
  }

  return to.string();
}

std::string
text(const std::string& a_path)
{
  return Studio::readFile(a_path).value_or("");
}

} // namespace

TEST_CASE("Writer: a clicked surface without a label is named, and the group is written by the name", "[Studio]")
{
  const std::string folder = copyOfWire("click");

  Pool pool(hostMemoryResource());

  const auto model = Language::read<T>(folder, pool);

  Studio::Writer<T> writer(folder);

  writer.attach(model);

  // The viewer adds a group "walls" holding the piece of the wire's surface between the core and the gas, as a click
  // makes it.
  RegionNames names = model.names();
  DomainSetup setup = model.domain("flow").setup();

  const uint32_t walls = setup.addSurfaceGroup();

  SurfaceRef<T> piece;

  piece.m_surface        = 1; // the cylinder, the object's second leaf
  piece.m_ownerRegion    = 2;
  piece.m_neighborRegion = 1;

  setup.add(walls, SurfaceSelector::piece(piece));
  names.setSurfaceGroup(0, walls, "walls");

  const std::string question = writer.edited(names, setup);

  CHECK_THAT(question, Catch::Matchers::StartsWith("Name the cylinder at"));
  CHECK_THAT(question, Catch::Matchers::EndsWith("model.ebg:4"));

  CHECK(writer.named("wire").empty());

  // The label after the cylinder's call, the group at the end of the domain, the rest as written.
  CHECK_THAT(text(folder + "/model.ebg"),
             Catch::Matchers::ContainsSubstring("wire = cylinder((0, 0, -1), (0, 0, 1), 0.2).label(\"wire\")\n"));
  CHECK(text(folder + "/setup.ebg") == R"(ebg 0

material air { density = 1.2 }

domain flow {
  subdomain fluid = gas
  # where the air enters
  group inflow = label("inlet") { velocity = (1, 0, 0) }
  group far = remaining()
  group walls = label("wire", between = (core, gas))
}
)");

  // The model reads, and the group holds the piece.
  const auto again = Language::read<T>(folder, pool);

  const Language::ModelDomain& flow = again.domain("flow");

  REQUIRE(flow.getNumSurfaceGroups() == 3U);
  CHECK(flow.surfaceGroupName(2) == "walls");
  CHECK(flow.surfaceGroupProperties(0).vector<T>("velocity") == Vec3T<T>(1, 0, 0));

  piece.m_surfaceLabel = again.index().findSurfaceLabel("wire");

  CHECK(flow.setup().surfaceGroupOf(piece) == 2U);

  // Undo restores both files; redo writes them again.
  const std::string written = text(folder + "/setup.ebg");

  CHECK(writer.undo());
  CHECK(text(folder + "/setup.ebg") == text(std::string(EBGEOMETRY_TEST_DATA_DIR) + "/language/wire/setup.ebg"));
  CHECK(text(folder + "/model.ebg") == text(std::string(EBGEOMETRY_TEST_DATA_DIR) + "/language/wire/model.ebg"));
  CHECK(writer.redo());
  CHECK(text(folder + "/setup.ebg") == written);
  CHECK_FALSE(writer.redo());
}

TEST_CASE("Writer: a renamed group keeps its properties; an unchanged one is kept as written", "[Studio]")
{
  const std::string folder = copyOfWire("rename");

  Pool pool(hostMemoryResource());

  const auto model = Language::read<T>(folder, pool);

  Studio::Writer<T> writer(folder);

  writer.attach(model);

  RegionNames names = model.names();

  names.setSurfaceGroup(0, 0, "entry");

  CHECK(writer.edited(names, model.domain("flow").setup()).empty());

  // The renamed group is written anew where it was, with its comment and properties; the other is kept as written.
  CHECK(text(folder + "/setup.ebg") == R"(ebg 0

material air { density = 1.2 }

domain flow {
  subdomain fluid = gas
  # where the air enters
  group entry = label("inlet") { velocity = (1, 0, 0) }
  group far = remaining()
}
)");
}

TEST_CASE("Writer: surface groups without a domain make a new one, named when asked", "[Studio]")
{
  const std::string folder = copyOfWire("new");

  Pool pool(hostMemoryResource());

  const auto model = Language::read<T>(folder, pool);

  Studio::Writer<T> writer(folder);

  writer.attach(model);

  // Names without domains, and one group holding the gas's surfaces.
  RegionNames names(model.index());
  DomainSetup setup;

  const uint32_t group = setup.addSurfaceGroup();

  setup.add(group, SurfaceSelector::region(1));
  names.setSurfaceGroup(group, "gas walls");

  CHECK(writer.edited(names, setup) == "Name the new domain");
  CHECK(writer.named("heat").empty());

  CHECK_THAT(text(folder + "/setup.ebg"),
             Catch::Matchers::EndsWith("}\n\ndomain heat {\n  group gas_walls = region(gas)\n}\n"));

  const auto again = Language::read<T>(folder, pool);

  CHECK(again.getNumDomains() == 2U);
}

TEST_CASE("Writer: no write over a file changed on disk since it was read", "[Studio]")
{
  const std::string folder = copyOfWire("changed");

  Pool pool(hostMemoryResource());

  const auto model = Language::read<T>(folder, pool);

  Studio::Writer<T> writer(folder);

  writer.attach(model);

  // The user saves setup.ebg in an editor after the model was read.
  std::ofstream(folder + "/setup.ebg", std::ios::app) << "# edited by hand\n";

  RegionNames names = model.names();

  names.setSurfaceGroup(0, 1, "outer");

  CHECK(writer.edited(names, model.domain("flow").setup()).empty());
  CHECK_THAT(writer.note(), Catch::Matchers::ContainsSubstring("changed on disk since it was read"));
  CHECK_THAT(text(folder + "/setup.ebg"), Catch::Matchers::EndsWith("# edited by hand\n"));
  CHECK_FALSE(writer.undo());
}

TEST_CASE("Writer: a file that is a link is written through it, the link kept", "[Studio]")
{
  namespace fs = std::filesystem;

  const std::string folder = copyOfWire("link");

  // setup.ebg a link to a file kept elsewhere.
  fs::rename(folder + "/setup.ebg", folder + "/shared-setup.ebg");
  fs::create_symlink("shared-setup.ebg", folder + "/setup.ebg");

  Pool pool(hostMemoryResource());

  const auto model = Language::read<T>(folder, pool);

  Studio::Writer<T> writer(folder);

  writer.attach(model);

  RegionNames names = model.names();

  names.setSurfaceGroup(0, 1, "outer");

  CHECK(writer.edited(names, model.domain("flow").setup()).empty());
  CHECK(fs::is_symlink(folder + "/setup.ebg"));
  CHECK_THAT(text(folder + "/shared-setup.ebg"), Catch::Matchers::ContainsSubstring("group outer = remaining()"));

  CHECK(writer.undo());
  CHECK(fs::is_symlink(folder + "/setup.ebg"));
  CHECK_THAT(text(folder + "/shared-setup.ebg"), Catch::Matchers::ContainsSubstring("group far = remaining()"));
}

TEST_CASE("Writer: undoing a write that made the setup file removes it", "[Studio]")
{
  namespace fs = std::filesystem;

  const std::string folder = copyOfWire("made");

  fs::remove(folder + "/setup.ebg");

  // The model without its setup: no materials, no domains.
  std::ofstream(folder + "/model.ebg", std::ios::app) << "";

  Pool pool(hostMemoryResource());

  const auto model = Language::read<T>(folder, pool);

  Studio::Writer<T> writer(folder);

  writer.attach(model);

  RegionNames names(model.index());
  DomainSetup setup;

  setup.add(setup.addSurfaceGroup(), SurfaceSelector::region(1));
  names.setSurfaceGroup(0, "walls");

  CHECK(writer.edited(names, setup) == "Name the new domain");
  CHECK(writer.named("flow").empty());
  CHECK(text(folder + "/setup.ebg") == "ebg 0\n\ndomain flow {\n  group walls = region(gas)\n}\n");

  CHECK(writer.undo());
  CHECK_FALSE(fs::exists(folder + "/setup.ebg"));
  CHECK(writer.redo());
  CHECK(fs::exists(folder + "/setup.ebg"));
}

TEST_CASE("Writer: what cannot be named where it is made is not written", "[Studio]")
{
  const std::string folder = copyOfWire("refused");

  // Two spheres sharing a label, a box used twice, and the wire as before.
  std::ofstream(folder + "/model.ebg") << R"(ebg 0

ball = sphere((3, 0, 0), 0.5).label("wall")
other = sphere((-3, 0, 0), 0.5).label("wall")
block = box((0, 3, 0), (1, 4, 1))

object overlay {
  region gas  = box((-4, -4, -4), (4, 6, 4))
  region core = union(ball, other, block, block.translate((0, 2, 0)))
}
)";
  std::ofstream(folder + "/setup.ebg") << "ebg 0\n";

  Pool pool(hostMemoryResource());

  const auto model = Language::read<T>(folder, pool);

  const std::string before = text(folder + "/model.ebg");

  const auto clickOn = [&](const uint32_t a_surface) {
    Studio::Writer<T> writer(folder);

    writer.attach(model);

    RegionNames names(model.index());
    DomainSetup setup;

    SurfaceRef<T> piece;

    piece.m_surface        = a_surface;
    piece.m_ownerRegion    = 2;
    piece.m_neighborRegion = 1;

    setup.add(setup.addSurfaceGroup(), SurfaceSelector::piece(piece));
    names.setSurfaceGroup(0, "picked");

    CHECK(writer.edited(names, setup) == "Name the new domain");
    CHECK(writer.named("flow").empty());

    return writer.note();
  };

  // A sphere whose label it shares: a new label would clash with it.
  CHECK_THAT(clickOn(1), Catch::Matchers::ContainsSubstring("is labeled \"wall\" with other shapes"));

  // The box used twice: a label where it is made would name both.
  CHECK_THAT(clickOn(3), Catch::Matchers::ContainsSubstring("is used 2 times"));

  CHECK(text(folder + "/model.ebg") == before);
}

TEST_CASE("Writer: a group naming a region group is kept as written, renamed or not", "[Studio]")
{
  const std::string folder = copyOfWire("region-group");

  // A cable of two regions, and a group naming the interface of the whole cable with the air.
  std::ofstream(folder + "/model.ebg") << R"(ebg 0

cable = overlay {
  region jacket = cylinder((0, 0, -1), (0, 0, 1), 0.5) material ptfe
  region core   = cylinder((0, 0, -1), (0, 0, 1), 0.2) material copper
}

object overlay {
  region air   = box((-2, -2, -2), (2, 2, 2)).label(xmin = "inlet") material air
  region cable = cable
}
)";
  std::ofstream(folder + "/setup.ebg") << R"(ebg 0

domain flow {
  subdomain fluid = air
  group walls = interface(cable, air) # the cable
  group inflow = label("inlet")
  group far = remaining()
}
)";

  Pool pool(hostMemoryResource());

  const auto model = Language::read<T>(folder, pool);

  Studio::Writer<T> writer(folder);

  writer.attach(model);

  RegionNames names = model.names();

  names.setSurfaceGroup(0, 0, "cable_walls");
  names.setSurfaceGroup(0, 2, "outer");

  CHECK(writer.edited(names, model.domain("flow").setup()).empty());

  CHECK(text(folder + "/setup.ebg") == R"(ebg 0

domain flow {
  subdomain fluid = air
  group cable_walls = interface(cable, air) # the cable
  group inflow = label("inlet")
  group outer = remaining()
}
)");
}

TEST_CASE("Writer: a write that cannot write one of its files changes none", "[Studio]")
{
  namespace fs = std::filesystem;

  // The click of the first test: a label in model.ebg and a group in setup.ebg, setup.ebg written second.
  const auto clickAndName = [](const std::string& a_folder) {
    Pool pool(hostMemoryResource());

    const auto model = Language::read<T>(a_folder, pool);

    Studio::Writer<T> writer(a_folder);

    writer.attach(model);

    RegionNames names = model.names();
    DomainSetup setup = model.domain("flow").setup();

    const uint32_t walls = setup.addSurfaceGroup();

    SurfaceRef<T> piece;

    piece.m_surface        = 1;
    piece.m_ownerRegion    = 2;
    piece.m_neighborRegion = 1;

    setup.add(walls, SurfaceSelector::piece(piece));
    names.setSurfaceGroup(0, walls, "walls");

    CHECK_THAT(writer.edited(names, setup), Catch::Matchers::StartsWith("Name the cylinder at"));
    CHECK(writer.named("wire").empty());
    CHECK_THAT(writer.note(), Catch::Matchers::StartsWith("not written"));
    CHECK_FALSE(writer.undo());
  };

  const std::string original = std::string(EBGEOMETRY_TEST_DATA_DIR) + "/language/wire/";

  SECTION("setup.ebg's temporary cannot be made")
  {
    const std::string folder = copyOfWire("unwritable-temporary");

    fs::create_directory(folder + "/setup.ebg.ebgstudio-new");

    clickAndName(folder);

    CHECK(text(folder + "/model.ebg") == text(original + "model.ebg"));
    CHECK(text(folder + "/setup.ebg") == text(original + "setup.ebg"));
    CHECK_FALSE(fs::exists(folder + "/model.ebg.ebgstudio-new"));
  }

  SECTION("setup.ebg is read-only")
  {
    // Root writes a read-only file all the same.
    if (geteuid() == 0) {
      SKIP("running as root");
    }

    const std::string folder = copyOfWire("read-only");

    fs::permissions(folder + "/setup.ebg", fs::perms::owner_read | fs::perms::group_read | fs::perms::others_read);

    clickAndName(folder);

    fs::permissions(folder + "/setup.ebg", fs::perms::owner_write, fs::perm_options::add);

    CHECK(text(folder + "/model.ebg") == text(original + "model.ebg"));
    CHECK(text(folder + "/setup.ebg") == text(original + "setup.ebg"));
    CHECK_FALSE(fs::exists(folder + "/model.ebg.ebgstudio-new"));
  }
}

TEST_CASE("Writer: a mesh is picked by its face group, or whole without one, and written by its label", "[Studio]")
{
  namespace fs = std::filesystem;

  // An octahedron in a box of air, its faces in two groups (top, bottom), and the same without groups.
  const std::string vertices = "v 1 0 0\nv -1 0 0\nv 0 1 0\nv 0 -1 0\nv 0 0 1\nv 0 0 -1\n";
  const std::string top      = "f 1 3 5\nf 1 6 3\nf 2 5 3\nf 2 3 6\n";
  const std::string bottom   = "f 1 5 4\nf 1 4 6\nf 2 4 5\nf 2 6 4\n";

  const auto folderWith = [&](const std::string& a_name, const std::string& a_mesh) {
    const fs::path to = fs::temp_directory_path() / ("ebgstudio-writer-" + a_name);

    fs::remove_all(to);
    fs::create_directories(to);

    std::ofstream(to / "part.obj") << a_mesh;
    std::ofstream(to / "model.ebg") << "ebg 0\n\nair  = box((-2, -2, -2), (2, 2, 2))\npart = mesh(\"part.obj\")\n\n"
                                       "object overlay {\n  region gas   = air material air\n  region solid = part "
                                       "material steel\n}\n";
    std::ofstream(to / "setup.ebg") << "ebg 0\n\nmaterial air { density = 1.2 }\nmaterial steel { density = 7850 }\n\n"
                                       "domain flow {\n  subdomain fluid = gas\n  group far = remaining()\n}\n";

    return to.string();
  };

  Pool pool(hostMemoryResource());

  // A piece of the mesh's surface between the solid and the gas, as a click finds it: the mesh is the second leaf.
  SurfaceRef<T> piece;

  piece.m_surface        = 1;
  piece.m_ownerRegion    = 2;
  piece.m_neighborRegion = 1;

  SECTION("with face groups: the group clicked, by its label")
  {
    const std::string folder = folderWith("mesh-groups", vertices + "g top\n" + top + "g bottom\n" + bottom);
    const auto        model  = Language::read<T>(folder, pool);

    Studio::Writer<T> writer(folder);

    writer.attach(model);

    RegionNames names = model.names();
    DomainSetup setup = model.domain("flow").setup();

    CHECK(names.pickedByLabel(1));
    CHECK_FALSE(names.pickedByLabel(0));

    // What the window picks of a face in the group top: the faces labeled top on this piece.
    SurfaceSelector selector = SurfaceSelector::piece(piece);

    selector.m_label = model.index().findSurfaceLabel("top").value();

    const uint32_t lid = setup.addSurfaceGroup();

    setup.add(lid, selector);
    names.setSurfaceGroup(0, lid, "lid");

    const std::string asked = writer.edited(names, setup);

    CAPTURE(asked);
    CHECK(asked.empty());
    CHECK_THAT(text(folder + "/setup.ebg"),
               Catch::Matchers::ContainsSubstring("group lid = label(\"top\", between = (solid, gas))"));
    CHECK(text(folder + "/model.ebg").find(".label(") == std::string::npos);
  }

  SECTION("without face groups: the whole mesh, named first")
  {
    const std::string folder = folderWith("mesh-plain", vertices + top + bottom);
    const auto        model  = Language::read<T>(folder, pool);

    Studio::Writer<T> writer(folder);

    writer.attach(model);

    RegionNames names = model.names();
    DomainSetup setup = model.domain("flow").setup();

    CHECK(names.pickedByLabel(1));

    const uint32_t skin = setup.addSurfaceGroup();

    setup.add(skin, SurfaceSelector::piece(piece));
    names.setSurfaceGroup(0, skin, "skin");

    CHECK_THAT(writer.edited(names, setup), Catch::Matchers::StartsWith("Name the mesh at"));
    CHECK(writer.named("hull").empty());
    CHECK_THAT(text(folder + "/model.ebg"),
               Catch::Matchers::ContainsSubstring("part = mesh(\"part.obj\").label(\"hull\")"));
    CHECK_THAT(text(folder + "/setup.ebg"),
               Catch::Matchers::ContainsSubstring("group skin = label(\"hull\", between = (solid, gas))"));
  }
}
