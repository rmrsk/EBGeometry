// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// ebgstudio: check, print, list the params of, render and view a .ebg model (a file, or a folder holding model.ebg
// and setup.ebg). The language is the library's (Source/EBGeometry_Language.hpp); this program is its command line.

#include "EBGeometry.hpp"
#include "Source/EBGeometry_Language.hpp"
#include "Writer.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace {

// The precision models are built in: float unless the build says otherwise (EBGEOMETRY_STUDIO_PRECISION=double), in
// every build, so that a frame takes as long on the host as a user's own program in float, and a GPU (which computes
// in double many times slower than in float) is not slower than the host.
#ifndef EBGEOMETRY_STUDIO_PRECISION
#define EBGEOMETRY_STUDIO_PRECISION float
#endif

using T = EBGEOMETRY_STUDIO_PRECISION;

using namespace EBGeometry;

const char* const usage = R"(usage: ebgstudio MODEL [options]             open the model in the viewer
       ebgstudio <command> MODEL [options]

MODEL is a .ebg file, or a folder holding model.ebg (and setup.ebg, read after it).

commands:
  check   MODEL [--domain NAME]... [--all-domains] [--inventory]
                                                   read the model and report every error and warning; with
                                                   --domain, check that the domain is complete (--all-domains:
                                                   every domain the model has); with --inventory, list every
                                                   name the model offers. Exits 1 on an error or an incomplete
                                                   domain.
  print   MODEL [-o FILE] [--flatten]              the model in canonical form (the first file; --flatten
                                                   resolves the includes and the setup file into one text)
  params  MODEL [--json]                           the params, with their values and where they are written
  render  MODEL -o FILE [--size W H]               one frame, as the viewer opens, written as a PPM image (on
                                                   the GPU in a GPU build)
  view    MODEL                                    the viewer (as ebgstudio MODEL)

options of every command:
  -D NAME=VALUE                                    a value for a param, written as in a file
)";

// The command line, split.
struct Options
{
  std::string              m_command;
  std::string              m_model;
  std::string              m_output;
  std::vector<std::string> m_domains;
  Language::Overrides      m_overrides;
  bool                     m_inventory  = false;
  bool                     m_allDomains = false;
  bool                     m_flatten    = false;
  bool                     m_json       = false;
  uint32_t                 m_width      = 960;
  uint32_t                 m_height     = 960;
};

// Stop with a message and the usage.
[[noreturn]] void
fail(const std::string& a_message)
{
  std::cerr << "ebgstudio: " << a_message << "\n\n" << usage;

  std::exit(2);
}

Options
parseArguments(const int a_argc, char** a_argv)
{
  Options options;

  if (a_argc < 2) {
    fail("no model");
  }

  // A first word that is no command is the model, opened in the viewer.
  const std::string first = a_argv[1];
  const bool        command =
    first == "check" || first == "print" || first == "params" || first == "render" || first == "view";

  if (command && a_argc < 3) {
    fail("no model");
  }

  options.m_command = command ? first : "view";
  options.m_model   = command ? a_argv[2] : first;

  for (int i = command ? 3 : 2; i < a_argc; i++) {
    const std::string arg = a_argv[i];

    const auto value = [&]() -> std::string {
      if (i + 1 >= a_argc) {
        fail(arg + " needs a value");
      }

      return a_argv[++i];
    };

    if (arg == "-D" || (arg.rfind("-D", 0) == 0 && arg.size() > 2)) {
      const std::string assignment = arg == "-D" ? value() : arg.substr(2);
      const size_t      equals     = assignment.find('=');

      if (equals == std::string::npos || equals == 0) {
        fail("-D takes NAME=VALUE, got '" + assignment + "'");
      }

      options.m_overrides[assignment.substr(0, equals)] = assignment.substr(equals + 1);
    }
    else if (arg == "--domain") {
      options.m_domains.push_back(value());
    }
    else if (arg == "--inventory") {
      options.m_inventory = true;
    }
    else if (arg == "--all-domains") {
      options.m_allDomains = true;
    }
    else if (arg == "--flatten") {
      options.m_flatten = true;
    }
    else if (arg == "--json") {
      options.m_json = true;
    }
    else if (arg == "-o") {
      options.m_output = value();
    }
    else if (arg == "--size") {
      const auto size = [&](const std::string& a_text) {
        try {
          const unsigned long n = std::stoul(a_text);

          if (n >= 1 && n <= 16384) {
            return static_cast<uint32_t>(n);
          }
        } catch (const std::exception&) {
        }

        fail("--size takes a width and a height in pixels, 1 to 16384; got '" + a_text + "'");
      };

      options.m_width  = size(value());
      options.m_height = size(value());
    }
    else {
      fail("unknown option '" + arg + "'");
    }
  }

  return options;
}

// A string as JSON.
std::string
json(const std::string& a_text)
{
  std::string out = "\"";

  for (const char c : a_text) {
    switch (c) {
    case '"': {
      out += "\\\"";

      break;
    }
    case '\\': {
      out += "\\\\";

      break;
    }
    case '\n': {
      out += "\\n";

      break;
    }
    default: {
      if (static_cast<unsigned char>(c) < 0x20) {
        char escaped[8];

        std::snprintf(escaped, sizeof(escaped), "\\u%04x", static_cast<unsigned>(c));

        out += escaped;
      }
      else {
        out += c;
      }

      break;
    }
    }
  }

  return out + "\"";
}

int
check(const Options& a_options, Pool& a_pool)
{
  const auto model = Language::read<T>(a_options.m_model, a_pool, a_options.m_overrides);

  for (const Language::Diagnostic& warning : model.warnings()) {
    std::cerr << warning.str() << "\n";
  }

  if (a_options.m_inventory) {
    std::cout << Language::inventory(model);
  }

  int status = 0;

  std::vector<std::string> domains = a_options.m_domains;

  if (a_options.m_allDomains) {
    for (uint32_t d = 0; d < model.getNumDomains(); d++) {
      if (std::find(domains.begin(), domains.end(), model.domain(d).name()) == domains.end()) {
        domains.push_back(model.domain(d).name());
      }
    }
  }

  if (!domains.empty()) {
    const Tape<T> tape(a_pool, model.object());

    for (const std::string& domain : domains) {
      const Language::Completeness result = Language::completeness(model, tape, domain);

      for (const Language::Diagnostic& finding : result.m_findings) {
        std::cerr << finding.str() << "\n";
      }

      std::cout << "domain '" << domain << "': " << (result.m_complete ? "complete" : "incomplete") << "\n";

      status = result.m_complete ? status : 1;
    }
  }

  return status;
}

int
print(const Options& a_options, Pool& a_pool)
{
  const auto        model = Language::read<T>(a_options.m_model, a_pool, a_options.m_overrides);
  const std::string text  = Language::print(model, a_options.m_flatten);

  if (a_options.m_output.empty()) {
    std::cout << text;

    return 0;
  }

  std::ofstream out(a_options.m_output);

  out << text;
  out.close();

  if (out.fail()) {
    std::cerr << "ebgstudio: cannot write " << a_options.m_output << "\n";

    return 1;
  }

  return 0;
}

int
params(const Options& a_options, Pool& a_pool)
{
  const auto model = Language::read<T>(a_options.m_model, a_pool, a_options.m_overrides);

  if (a_options.m_json) {
    std::cout << "[";

    for (size_t i = 0; i < model.params().size(); i++) {
      const Language::Param& p = model.params()[i];

      std::cout << (i > 0 ? ",\n " : "\n ") << "{\"name\": " << json(p.m_name) << ", \"value\": " << json(p.m_value)
                << ", \"type\": " << json(p.m_type) << ", \"file\": " << json(p.m_span.m_file)
                << ", \"line\": " << p.m_span.m_line << ", \"overridden\": " << (p.m_overridden ? "true" : "false")
                << "}";
    }

    std::cout << (model.params().empty() ? "]\n" : "\n]\n");

    return 0;
  }

  for (const Language::Param& p : model.params()) {
    std::cout << p.m_name << " = " << p.m_value << "  (" << p.m_type << ", " << p.m_span.m_file << ":"
              << p.m_span.m_line << (p.m_overridden ? ", overridden" : "") << ")\n";
  }

  return 0;
}

// The camera and settings a model's view block gives, over the viewer's defaults: regions colored by material.
template <class S>
void
viewOf(const Language::Model<T>& a_model,
       const S&                  a_shape,
       Render::Camera<T>&        a_camera,
       Render::Settings<T>&      a_settings)
{
  const Language::Properties& view = a_model.view();

  a_settings.m_colorBy            = Render::ColorBy::Primitive;
  a_settings.m_regions.m_coloring = RegionColoring::Material;

  // The view block's camera, or one fitted to the shape (the default camera for one that is not bounded).
  a_camera = Render::ViewerDetail::initialCamera<T>(a_shape);

  if (view.has("camera")) {
    const Vec3T<T> target = view.vector<T>("target", a_shape.computeBoundingVolume().getCentroid());

    a_camera = Render::Camera<T>::lookAt(view.vector<T>("camera"), target, view.vector<T>("up", Vec3T<T>(0, 1, 0)));
  }

  // The background: a plain color instead of the sky's gradient, and the floor under the object or not.
  if (view.has("background")) {
    a_settings.m_sky        = false;
    a_settings.m_background = view.vector<T>("background");
  }

  a_settings.m_sky   = view.flag("sky", a_settings.m_sky);
  a_settings.m_floor = view.flag("floor", a_settings.m_floor);
}

int
render(const Options& a_options, Pool& a_pool)
{
  if (a_options.m_output.empty()) {
    fail("render needs -o FILE");
  }

  const auto model = Language::read<T>(a_options.m_model, a_pool, a_options.m_overrides);

  const Tape<T>             tape(a_pool, model.object());
  const RegionView<Tape<T>> regions(tape);

  Render::Camera<T>   camera;
  Render::Settings<T> settings;

  viewOf(model, regions, camera, settings);

  settings.m_width  = a_options.m_width;
  settings.m_height = a_options.m_height;

  // The names' colors, as the viewer gives them.
  const RegionNames names = model.names();

  for (uint32_t i = 0; i < Render::Settings<T>::PrimitiveColors; i++) {
    Vec3T<double> color;

    if (names.colorOf(settings.m_regions.m_coloring, i, color)) {
      settings.m_primitiveColors[i] = Vec3T<T>(T(color[0]), T(color[1]), T(color[2]));
    }
    if (names.colorOf(RegionColoring::Material, i, color)) {
      settings.m_materialColors[i] = Vec3T<T>(T(color[0]), T(color[1]), T(color[2]));
    }
  }

  Render::Image image;

  // On the device in a GPU build with a GPU: the pool mirrored, the regions viewed there.
  if (GPU::enabled && GPU::deviceAvailable()) {
    a_pool.freeze();

    const Pool device = Pool::mirror(a_pool, GPU::memoryResource());

    Render::Renderer<T>(settings).renderOnDevice(regions.rebasedView(device), camera, image);
  }
  else {
    Render::Renderer<T>(settings).render(regions, camera, image);
  }

  if (!Render::writeImage(image, a_options.m_output)) {
    std::cerr << "ebgstudio: cannot write " << a_options.m_output << "\n";

    return 1;
  }

  return 0;
}

// Files and their modification times.
using Stamps = std::vector<std::pair<std::string, std::filesystem::file_time_type>>;

// Each file's modification time now (the earliest time for a file that does not exist).
Stamps
stampsOf(const std::vector<std::string>& a_files)
{
  Stamps stamps;

  for (const std::string& file : a_files) {
    std::error_code ec;

    stamps.emplace_back(file, std::filesystem::last_write_time(file, ec));
  }

  return stamps;
}

// The files a reload watches: the given ones, a model's first files (a folder's model.ebg and setup.ebg, even before
// they exist), and every file their texts include, resolved as the language does. The reader reports none of the
// files of a model that fails, so a file a failing version includes first (one not written yet, or with an error) is
// found here, and fixing or making it reloads.
std::vector<std::string>
watched(const Options& a_options, std::vector<std::string> a_files)
{
  namespace fs = std::filesystem;

  std::vector<std::string> texts;

  if (fs::is_directory(a_options.m_model)) {
    texts.push_back((fs::path(a_options.m_model) / "model.ebg").string());
    texts.push_back((fs::path(a_options.m_model) / "setup.ebg").string());
  }
  else {
    texts.push_back(a_options.m_model);
  }

  // The texts grow as their includes are found; only these are parsed (a mesh among a_files is not).
  for (size_t i = 0; i < texts.size(); i++) {
    const auto text = Studio::readFile(texts[i]);

    if (!text.has_value()) {
      continue;
    }

    try {
      for (const Language::Syntax::Statement& s : Language::Syntax::parse(*text, texts[i]).m_statements) {
        const std::string path = (fs::path(texts[i]).parent_path() / s.m_name).string();

        if (s.m_kind == Language::Syntax::StatementKind::Include &&
            std::find(texts.begin(), texts.end(), path) == texts.end()) {
          texts.push_back(path);
        }
      }
    } catch (const Language::Error&) {
    }
  }

  for (const std::string& file : texts) {
    if (std::find(a_files.begin(), a_files.end(), file) == a_files.end()) {
      a_files.push_back(file);
    }
  }

  return a_files;
}

// A model read, with what the viewer shows of it: everything lives in the model's own pool.
struct Loaded
{
  std::unique_ptr<Pool>                m_pool;
  std::unique_ptr<Language::Model<T>>  m_model;
  std::unique_ptr<Tape<T>>             m_tape;
  std::unique_ptr<RegionView<Tape<T>>> m_regions;
  RegionNames                          m_names;
  Stamps                               m_stamps;
};

// Read a model into a pool of its own; throws Language::Error. a_before: the stamps of the files to watch, taken before
// the read, so that a file saved while it is read reloads again.
std::unique_ptr<Loaded>
load(const Options& a_options, const Stamps& a_before)
{
  auto loaded = std::make_unique<Loaded>();

  // A model the language reads can still exceed what the tape takes (its call depth, its stack): an error, not the end
  // of the viewer.
  const ThrowOnRequire checks;

  loaded->m_pool = std::make_unique<Pool>(hostMemoryResource());
  loaded->m_model =
    std::make_unique<Language::Model<T>>(Language::read<T>(a_options.m_model, *loaded->m_pool, a_options.m_overrides));
  loaded->m_tape    = std::make_unique<Tape<T>>(*loaded->m_pool, loaded->m_model->object());
  loaded->m_regions = std::make_unique<RegionView<Tape<T>>>(*loaded->m_tape);
  loaded->m_names   = loaded->m_model->names();

  for (const Language::Diagnostic& warning : loaded->m_model->warnings()) {
    std::cerr << warning.str() << "\n";
  }

  // The files stamped before the read, and every other file it read (a mesh first named in this version), as it is now.
  loaded->m_stamps = a_before;

  for (const std::string& file : loaded->m_model->files()) {
    const bool stamped =
      std::any_of(a_before.begin(), a_before.end(), [&file](const auto& a_stamp) { return a_stamp.first == file; });

    if (!stamped) {
      loaded->m_stamps.push_back(stampsOf({file}).front());
    }
  }

  return loaded;
}

// Whether a file the model read has changed since it was read.
bool
changed(const Stamps& a_stamps)
{
  for (const auto& [file, stamp] : a_stamps) {
    std::error_code ec;

    if (std::filesystem::last_write_time(file, ec) != stamp) {
      return true;
    }
  }

  return false;
}

// The viewer, reading the model again whenever one of its files changes: the last good model stays on an error,
// which the title shows; the camera and what is shown stay across a reload. The viewer's edits of a domain's surface
// groups are written into the files (see Writer.hpp), Ctrl+Z and Ctrl+Y undo and redo them, and the files' change
// reloads the model.
int
view(const Options& a_options)
{
  // What is read, and how long it takes, in the terminal; the model's name in the window's title.
  const std::string model = std::filesystem::path(a_options.m_model).filename().string().empty()
                              ? a_options.m_model
                              : std::filesystem::path(a_options.m_model).filename().string();

  const auto timed = [&](const char* a_what, const Stamps& a_before) {
    std::cout << "ebgstudio: " << a_what << " " << a_options.m_model << " ..." << std::flush;

    const auto start = std::chrono::steady_clock::now();

    std::unique_ptr<Loaded> loaded;

    try {
      loaded = load(a_options, a_before);
    } catch (...) {
      std::cout << " failed" << std::endl;

      throw;
    }

    std::cout << " done in " << std::fixed << std::setprecision(2)
              << std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count() << " s ("
              << loaded->m_model->files().size() << " file(s), " << loaded->m_model->index().getNumRegions()
              << " regions, " << loaded->m_model->index().getNumLeaves() << " shapes)" << std::endl;

    return loaded;
  };

  std::unique_ptr<Loaded> current = timed("reading", stampsOf(watched(a_options, {})));
  std::unique_ptr<Loaded> next;

  std::string note;

  auto stamps   = current->m_stamps;
  auto lastPoll = std::chrono::steady_clock::now();

  Studio::Writer<T> writer(a_options.m_model);

  writer.attach(*current->m_model);

  Render::ViewerDetail::Hooks<T> hooks;

  hooks.m_note = [&note, &writer, &model]() {
    const std::string extra = note.empty() ? writer.note() : note;

    return model + (extra.empty() ? "" : " | " + extra);
  };
  hooks.m_edited = [&writer](const RegionNames& a_names, const DomainSetup& a_setup) {
    return writer.edited(a_names, a_setup);
  };
  hooks.m_named  = [&writer](const std::string& a_name) { return writer.named(a_name); };
  hooks.m_undo   = [&writer]() { return writer.undo(); };
  hooks.m_redo   = [&writer]() { return writer.redo(); };
  hooks.m_reload = [&]() {
    const auto now = std::chrono::steady_clock::now();

    if (now - lastPoll < std::chrono::milliseconds(250)) {
      return false;
    }

    lastPoll = now;

    if (!changed(stamps)) {
      return false;
    }

    // Wait for the next change, whatever this read gives: of the last good model's files and of every file the texts
    // include now, stamped before the read.
    std::vector<std::string> files;

    for (const auto& [file, stamp] : current->m_stamps) {
      files.push_back(file);
    }

    stamps = stampsOf(watched(a_options, files));

    try {
      next = timed("reloading", stamps);
      note.clear();

      return true;
    } catch (const Language::Error& e) {
      std::cerr << e.what() << "\n";

      note = e.diagnostics().front().str() + " (showing the last good model)";

      return false;
    } catch (const RequireError& e) {
      std::cerr << e.what() << "\n";

      note = std::string(e.what()) + " (showing the last good model)";

      return false;
    }
  };

  Render::Camera<T>   camera;
  Render::Settings<T> settings;

  {
    const ThrowOnRequire checks;

    viewOf(*current->m_model, *current->m_regions, camera, settings);
  }

  // The background the view block gave last: a reload applies the block's only when it changed, so that saving the
  // file keeps what the keys (B, F) set in the window.
  const auto backgroundOf = [](const Render::Settings<T>& a_settings) {
    return std::make_tuple(a_settings.m_sky,
                           a_settings.m_floor,
                           a_settings.m_background[0],
                           a_settings.m_background[1],
                           a_settings.m_background[2]);
  };

  auto viewBackground = backgroundOf(settings);

  std::cout << "ebgstudio: opening the viewer (it finds the model's pieces of surface first)" << std::endl;

  // Without a window (a build without the viewer, no display) the viewer says why, and the command fails.
#ifdef EBGEOMETRY_VIEWER_SDL2
  const bool window = Render::ViewerDetail::displayAvailable();
#else
  const bool window = false;
#endif

  char  name[] = "ebgstudio";
  char* argv[] = {name, nullptr};

  while (true) {
    Render::ViewerDetail::startInPool(
      *current->m_regions, *current->m_pool, camera, settings, 1, argv, &current->m_names, &hooks);

    if (!hooks.m_reloading) {
      break;
    }

    current = std::move(next);
    stamps  = current->m_stamps;

    writer.attach(*current->m_model);

    // R goes to the new model's own view: its view block's camera, or one fitted to its geometry. A view block the
    // window cannot use (a value of the wrong type) is reported, and the window keeps what it shows.
    Render::Settings<T> fresh;
    bool                viewRead = false;

    {
      const ThrowOnRequire checks;

      try {
        viewOf(*current->m_model, *current->m_regions, hooks.m_home, fresh);

        hooks.m_hasHome = true;
        viewRead        = true;
      } catch (const RequireError&) {
        hooks.m_hasHome = false;
      } catch (const Language::Error& e) {
        std::cerr << e.what() << "\n";

        hooks.m_hasHome = false;
      }
    }
    camera = hooks.m_camera;

    // What is shown stays; the surface groups shown are the new model's domains'. The view block's background, when
    // it changed in the file.
    settings                           = hooks.m_settings;
    settings.m_regions.m_surfaceGroups = SurfaceGroups();

    if (viewRead && backgroundOf(fresh) != viewBackground) {
      settings.m_sky        = fresh.m_sky;
      settings.m_floor      = fresh.m_floor;
      settings.m_background = fresh.m_background;
      viewBackground        = backgroundOf(fresh);
    }
  }

  Render::ViewerDetail::closeWindow(hooks);

  return window ? 0 : 1;
}

} // namespace

int
main(int argc, char** argv)
{
  if (argc >= 2 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")) {
    std::cout << usage;

    return 0;
  }

  const Options options = parseArguments(argc, argv);

  Pool pool(hostMemoryResource());

  try {
    if (options.m_command == "view") {
      return view(options);
    }

    // What the library refuses beyond the language (a tape deeper than it takes, a camera at its target) is an error.
    const ThrowOnRequire checks;

    if (options.m_command == "check") {
      return check(options, pool);
    }
    if (options.m_command == "print") {
      return print(options, pool);
    }
    if (options.m_command == "params") {
      return params(options, pool);
    }
    if (options.m_command == "render") {
      return render(options, pool);
    }
  } catch (const Language::Error& e) {
    std::cerr << e.what() << "\n";

    return 1;
  } catch (const RequireError& e) {
    std::cerr << options.m_model << ": error: " << e.what() << "\n";

    return 1;
  }

  fail("unknown command '" + options.m_command + "'");
}
