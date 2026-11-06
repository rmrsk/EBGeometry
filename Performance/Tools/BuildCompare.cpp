// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// BuildCompare: compares what two builds cost, from the records BuildCost wrote for each side, and writes
// a Markdown section for the comparison's report:
//
//   BuildCompare --base <base.jsonl> --head <head.jsonl> --title <text> [--machine <text>]
//                [--minor <percent>] [--major <percent>] [--markdown <file>]
//
// Per translation unit (a probe, or a test file): the CPU time and the peak memory of its compile, base
// against head, flagged minor or MAJOR when they grow by at least --minor or --major percent (and by more
// than a floor, 2 s or 100 MB, so that small units do not flag on noise). Per device kernel: registers,
// cumulative stack and spill bytes, every change listed, flagged when registers change, spills grow, or the
// stack grows by more than 10% and 256 bytes. The section reports; it does not judge: the exit status is 0
// whatever it finds (3 for a usage error). A missing record is an empty build.

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace {

// One JSON line of a record: a flat object of strings and numbers, as BuildCost writes it.
using Fields = std::map<std::string, std::string>;

bool
parseLine(const std::string& a_line, Fields& a_fields)
{
  size_t i = a_line.find('{');

  if (i == std::string::npos) {
    return false;
  }

  i++;

  const auto skip = [&]() {
    while (i < a_line.size() && (a_line[i] == ' ' || a_line[i] == ',')) {
      i++;
    }
  };

  const auto string = [&](std::string& a_out) {
    if (i >= a_line.size() || a_line[i] != '"') {
      return false;
    }

    i++;

    while (i < a_line.size() && a_line[i] != '"') {
      if (a_line[i] == '\\' && i + 1 < a_line.size()) {
        i++;

        const char c = a_line[i];

        a_out += (c == 'n') ? '\n' : (c == 't') ? '\t' : c;
      }
      else {
        a_out += a_line[i];
      }

      i++;
    }

    i++;

    return true;
  };

  for (;;) {
    skip();

    if (i >= a_line.size() || a_line[i] == '}') {
      return true;
    }

    std::string key;
    std::string value;

    if (!string(key) || i >= a_line.size() || a_line[i] != ':') {
      return false;
    }

    i++;

    if (i < a_line.size() && a_line[i] == '"') {
      if (!string(value)) {
        return false;
      }
    }
    else {
      while (i < a_line.size() && a_line[i] != ',' && a_line[i] != '}') {
        value += a_line[i++];
      }
    }

    a_fields[key] = value;
  }
}

// What one translation unit cost.
struct Unit
{
  double cpu      = 0;
  double peak     = 0;
  int    compiles = 0;
};

// One kernel's statistics.
struct Kernel
{
  long registers = 0;
  long stack     = 0;
  long stores    = 0;
  long loads     = 0;
};

// A side's record.
struct Build
{
  std::map<std::string, Unit>                           units;
  std::map<std::pair<std::string, std::string>, Kernel> kernels;
};

// A translation unit's name from its source: the probe for .../Performance/<Probe>/main.cpp, the file's stem
// for .../Tests/<File>.cpp. Empty for anything else (the harness's own tools, fetched dependencies), which is
// not measured. Only the part after the last Performance/ or Tests/ counts, so a checkout under a folder
// named Tools/ or Tests/ is measured as any other.
std::string
unitName(const std::string& a_source)
{
  const auto perf = a_source.rfind("/Performance/");

  if (perf != std::string::npos) {
    const std::string rest  = a_source.substr(perf + 13);
    const auto        slash = rest.find('/');

    if (slash != std::string::npos && rest.substr(slash) == "/main.cpp" && rest.substr(0, slash) != "Tools") {
      return rest.substr(0, slash);
    }

    return "";
  }

  const auto tests = a_source.rfind("/Tests/");

  if (tests != std::string::npos) {
    const std::string rest = a_source.substr(tests + 7);

    if (rest.find('/') == std::string::npos && rest.size() > 4 && rest.compare(rest.size() - 4, 4, ".cpp") == 0) {
      return rest.substr(0, rest.size() - 4);
    }
  }

  return "";
}

// A side's record; a missing one is an empty build (nothing compiled, every unit of the other side new),
// which the section then shows as such.
void
read(const std::string& a_path, Build& a_build)
{
  std::ifstream in(a_path);

  std::string line;

  while (std::getline(in, line)) {
    Fields f;

    if (line.empty() || !parseLine(line, f)) {
      continue;
    }

    const std::string unit = unitName(f["source"]);

    if (unit.empty()) {
      continue;
    }

    if (f["type"] == "compile") {
      Unit& u = a_build.units[unit];

      u.cpu += std::atof(f["cpu_s"].c_str());
      u.peak = std::max(u.peak, std::atof(f["peak_mb"].c_str()));
      u.compiles++;
    }
    else if (f["type"] == "kernel") {
      Kernel& k = a_build.kernels[{unit, f["name"]}];

      k.registers = std::atol(f["registers"].c_str());
      k.stack     = std::atol(f["stack"].c_str());
      k.stores    = std::atol(f["spill_stores"].c_str());
      k.loads     = std::atol(f["spill_loads"].c_str());
    }
  }
}

std::string
format(const char* a_format, double a_value)
{
  char buffer[64];

  std::snprintf(buffer, sizeof(buffer), a_format, a_value);

  return buffer;
}

std::string
percent(double a_base, double a_head)
{
  if (a_base <= 0) {
    return "-";
  }

  return format("%+.1f%%", 100.0 * (a_head - a_base) / a_base);
}

// The flag for a growth from a_base to a_head: "MAJOR", "minor" or "".
std::string
flag(double a_base, double a_head, double a_floor, double a_minor, double a_major)
{
  if (a_base <= 0 || a_head - a_base < a_floor) {
    return "";
  }

  const double change = 100.0 * (a_head - a_base) / a_base;

  return change >= a_major ? "MAJOR" : (change >= a_minor ? "minor" : "");
}

std::string
worse(const std::string& a_x, const std::string& a_y)
{
  if (a_x == "MAJOR" || a_y == "MAJOR") {
    return "MAJOR";
  }

  return (a_x == "minor" || a_y == "minor") ? "minor" : "";
}

// A kernel's name shortened for a table: the namespaces every kernel shares dropped, and cut at a_width.
std::string
shortName(std::string a_name, size_t a_width)
{
  for (const std::string ns : {"(anonymous namespace)::", "EBGeometry::GPU::TransformDetail::", "EBGeometry::"}) {
    for (size_t at = a_name.find(ns); at != std::string::npos; at = a_name.find(ns)) {
      a_name.erase(at, ns.size());
    }
  }

  for (size_t at = a_name.find('|'); at != std::string::npos; at = a_name.find('|')) {
    a_name.replace(at, 1, "/");
  }

  return a_name.size() > a_width ? a_name.substr(0, a_width) + "…" : a_name;
}

} // namespace

int
main(int argc, char** argv)
{
  std::string basePath;
  std::string headPath;
  std::string title = "Build costs";
  std::string machine;
  std::string markdown;
  double      minor = 10;
  double      major = 25;

  for (int i = 1; i + 1 < argc; i += 2) {
    const std::string a = argv[i];
    const std::string v = argv[i + 1];

    if (a == "--base") {
      basePath = v;
    }
    else if (a == "--head") {
      headPath = v;
    }
    else if (a == "--title") {
      title = v;
    }
    else if (a == "--machine") {
      machine = v;
    }
    else if (a == "--markdown") {
      markdown = v;
    }
    else if (a == "--minor") {
      minor = std::atof(v.c_str());
    }
    else if (a == "--major") {
      major = std::atof(v.c_str());
    }
    else {
      std::cerr << "BuildCompare: unknown option " << a << "\n";

      return 3;
    }
  }

  Build base;
  Build head;

  if (basePath.empty() || headPath.empty()) {
    std::cerr << "BuildCompare: --base and --head are required\n";

    return 3;
  }

  read(basePath, base);
  read(headPath, head);

  std::set<std::string> names;

  for (const auto& [name, u] : base.units) {
    names.insert(name);
  }
  for (const auto& [name, u] : head.units) {
    names.insert(name);
  }

  std::ostringstream table;
  double             baseCpu  = 0;
  double             headCpu  = 0;
  double             basePeak = 0;
  double             headPeak = 0;
  int                flagged  = 0;

  table << "| unit | CPU base | CPU head | change | peak base | peak head | change | flag |\n"
        << "|---|---:|---:|---:|---:|---:|---:|---|\n";

  for (const auto& name : names) {
    const auto b = base.units.find(name);
    const auto h = head.units.find(name);

    if (b == base.units.end() || h == head.units.end()) {
      const Unit& u = (b == base.units.end()) ? h->second : b->second;

      table << "| `" << name << "` | " << (b == base.units.end() ? "-" : format("%.1f s", u.cpu)) << " | "
            << (h == head.units.end() ? "-" : format("%.1f s", u.cpu)) << " | | "
            << (b == base.units.end() ? "-" : format("%.0f MB", u.peak)) << " | "
            << (h == head.units.end() ? "-" : format("%.0f MB", u.peak)) << " | | "
            << (b == base.units.end() ? "new" : "gone") << " |\n";

      continue;
    }

    const Unit& x = b->second;
    const Unit& y = h->second;

    baseCpu += x.cpu;
    headCpu += y.cpu;
    basePeak = std::max(basePeak, x.peak);
    headPeak = std::max(headPeak, y.peak);

    const std::string f = worse(flag(x.cpu, y.cpu, 2.0, minor, major), flag(x.peak, y.peak, 100.0, minor, major));

    flagged += f.empty() ? 0 : 1;

    table << "| `" << name << "` | " << format("%.1f s", x.cpu) << " | " << format("%.1f s", y.cpu) << " | "
          << percent(x.cpu, y.cpu) << " | " << format("%.0f MB", x.peak) << " | " << format("%.0f MB", y.peak) << " | "
          << percent(x.peak, y.peak) << " | " << (f.empty() ? "~" : "**" + f + "**") << " |\n";
  }

  const std::string totalFlag =
    worse(flag(baseCpu, headCpu, 2.0, minor, major), flag(basePeak, headPeak, 100.0, minor, major));

  table << "| **all, in both** | " << format("%.1f s", baseCpu) << " | " << format("%.1f s", headCpu) << " | "
        << percent(baseCpu, headCpu) << " | " << format("%.0f MB", basePeak) << " | " << format("%.0f MB", headPeak)
        << " | " << percent(basePeak, headPeak) << " | " << (totalFlag.empty() ? "~" : "**" + totalFlag + "**")
        << " |\n";

  // The kernels whose statistics differ, flagged first.
  struct Row
  {
    std::string flag;
    std::string text;
  };

  std::vector<Row> rows;
  size_t           compared = 0;
  size_t           added    = 0;
  size_t           removed  = 0;

  for (const auto& [key, k] : head.kernels) {
    const auto b = base.kernels.find(key);

    if (b == base.kernels.end()) {
      added++;

      continue;
    }

    compared++;

    const Kernel& x = b->second;
    const Kernel& y = k;

    if (x.registers == y.registers && x.stack == y.stack && x.stores == y.stores && x.loads == y.loads) {
      continue;
    }

    std::string why;

    if (x.registers != y.registers) {
      why += "registers ";
    }
    if (y.stores > x.stores || y.loads > x.loads) {
      why += "spills ";
    }
    if (y.stack > x.stack && double(y.stack - x.stack) > std::max(256.0, 0.1 * double(x.stack))) {
      why += "stack ";
    }

    std::ostringstream text;

    text << "| `" << key.first << "` | `" << shortName(key.second, 90) << "` | " << x.registers << " → " << y.registers
         << " | " << x.stack << " → " << y.stack << " | " << x.stores << "/" << x.loads << " → " << y.stores << "/"
         << y.loads << " | " << (why.empty() ? "~" : "**" + why.substr(0, why.size() - 1) + "**") << " |\n";
    rows.push_back({why, text.str()});
  }

  for (const auto& [key, k] : base.kernels) {
    if (head.kernels.find(key) == head.kernels.end()) {
      removed++;
    }
  }

  std::stable_sort(
    rows.begin(), rows.end(), [](const Row& a_x, const Row& a_y) { return !a_x.flag.empty() && a_y.flag.empty(); });

  size_t kernelFlags = 0;

  for (const auto& r : rows) {
    kernelFlags += r.flag.empty() ? 0 : 1;
  }

  std::ostringstream out;

  out << "\n### " << title << "\n\n";

  if (!machine.empty()) {
    out << machine << "\n\n";
  }

  out << "Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, "
         "every process the compiler ran) and its peak memory; minor at "
      << format("%g", minor) << "% more, MAJOR at " << format("%g", major)
      << "% (and at least 2 s or 100 MB). Total CPU " << format("%.1f s", baseCpu) << " → " << format("%.1f s", headCpu)
      << " (" << percent(baseCpu, headCpu) << "), the largest peak " << format("%.0f MB", basePeak) << " → "
      << format("%.0f MB", headPeak) << "; " << flagged << " unit(s) flagged.\n\n"
      << table.str();

  if (compared + added + removed > 0) {
    out << "\nDevice kernels: " << compared << " in both, " << rows.size() << " changed (" << kernelFlags
        << " flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), " << added
        << " new, " << removed << " gone. Spills are bytes stored/loaded per thread.\n";

    if (!rows.empty()) {
      constexpr size_t shown = 60;

      out << "\n| unit | kernel | registers | stack | spills | flag |\n|---|---|---:|---:|---:|---|\n";

      for (size_t i = 0; i < std::min(rows.size(), shown); i++) {
        out << rows[i].text;
      }

      if (rows.size() > shown) {
        out << "\n" << rows.size() - shown << " more changed kernels are in the records.\n";
      }
    }
  }

  const std::string text = out.str();

  std::cout << text;

  if (!markdown.empty()) {
    std::ofstream file(markdown);

    file << text;
  }

  return 0;
}
