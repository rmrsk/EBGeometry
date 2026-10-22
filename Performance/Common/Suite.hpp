// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   Suite.hpp
 * @brief  A probe's set of cases: its command line, which cases a build keeps, and the loop that runs
 * the selected cases and hands each result to the reports.
 * @details Command line (every probe):
 *
 *   --list           print the selected case names and exit
 *   --case <glob>    run only the cases matching the glob (* and ?); may be repeated
 *   --reps <n>       timed repetitions per case (default 5), after one untimed warm-up
 *   --smoke          small problem sizes and one repetition (whatever --reps says): checks that a case
 *                    runs, times nothing
 *   --json           one JSON line per case instead of the table
 *   --json-out <f>   print the table, and also append the JSON lines to file f
 *   --no-banner      leave out what every probe of a run shares (Scripts/run-performance.sh prints it
 *                    once, from its first probe)
 */

#ifndef EBGEOMETRY_PERF_SUITE_HPP
#define EBGEOMETRY_PERF_SUITE_HPP

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <EBGeometry.hpp>

#include "Case.hpp"
#include "Report.hpp"

namespace Perf {

/**
 * @brief Whether a name matches a glob of * (any run of characters) and ? (any one character).
 * @param[in] a_glob Pattern.
 * @param[in] a_name Name.
 * @return True on a match.
 */
inline bool
globMatch(const char* a_glob, const char* a_name) noexcept
{
  if (*a_glob == '\0') {
    return *a_name == '\0';
  }

  if (*a_glob == '*') {
    return globMatch(a_glob + 1, a_name) || (*a_name != '\0' && globMatch(a_glob, a_name + 1));
  }

  return *a_name != '\0' && (*a_glob == '?' || *a_glob == *a_name) && globMatch(a_glob + 1, a_name + 1);
}

/**
 * @brief A probe's set of cases and its command line.
 */
class Suite
{
public:
  /**
   * @brief Parse the command line (see the file comment); a bad argument is reported and exits with 2.
   * @param[in] a_argc Argument count.
   * @param[in] a_argv Arguments.
   * @param[in] a_name The probe's name.
   */
  Suite(int a_argc, char** a_argv, std::string a_name) : m_name(std::move(a_name))
  {
    // A flag's value, or a clear error when it is missing.
    const auto value = [&](int& a_i, const std::string& a_flag) -> std::string {
      if (a_i + 1 >= a_argc) {
        this->fail(a_flag + " needs a value");
      }

      return a_argv[++a_i];
    };

    for (int i = 1; i < a_argc; i++) {
      const std::string arg = a_argv[i];

      if (arg == "--list") {
        m_list = true;
      }
      else if (arg == "--json") {
        m_json = true;
      }
      else if (arg == "--smoke") {
        m_smoke = true;
      }
      else if (arg == "--no-banner") {
        m_banner = false;
      }
      else if (arg == "--case") {
        m_globs.push_back(value(i, arg));
      }
      else if (arg == "--json-out") {
        m_jsonOut = value(i, arg);
      }
      else if (arg == "--reps") {
        const std::string text = value(i, arg);
        char*             end  = nullptr;
        const long        reps = std::strtol(text.c_str(), &end, 10);

        if (end == text.c_str() || *end != '\0' || reps < 1 || reps > 1000000) {
          this->fail("--reps needs a whole number of at least 1 (got '" + text + "')");
        }

        m_reps = static_cast<int>(reps);
      }
      else {
        this->fail("unknown argument '" + arg + "'");
      }
    }

    // A smoke run times nothing, so it takes one repetition, wherever --reps came on the line.
    if (m_smoke) {
      m_reps = 1;
    }

    m_deviceRuns = EBGeometry::GPU::enabled && EBGeometry::GPU::deviceAvailable();
  }

  /**
   * @brief Add a case; it runs only if the build keeps it and the command line selects it.
   * @details A host build keeps the host cases; a CUDA or HIP build keeps only the host/device pairs
   * (side hostdevice), and only with a visible GPU. A GPU build's host side is not compiled as a host
   * build is (a CUDA build's is not vectorised, since nvcc cannot compile host SIMD code; a HIP build's
   * has AVX2 and FMA), so host times come from a host build, and the pairs give the device's time, its
   * speedup over the host and whether the two agree.
   * @param[in] a_name The case's name, from general to specific: component/input.../operation/precision/side.
   * @param[in] a_run  The case.
   */
  void
  add(const std::string& a_name, std::function<void(Case&)> a_run)
  {
    const bool paired = CaseName(a_name).side == "hostdevice";

    if (EBGeometry::GPU::enabled ? (paired && m_deviceRuns) : !paired) {
      m_cases.emplace_back(a_name, std::move(a_run));
    }
  }

  /**
   * @brief Run the selected cases, or list them.
   * @return The program's exit code.
   */
  [[nodiscard]] int
  run() const
  {
    std::vector<const Entry*> selected;
    std::size_t               opWidth = 9;

    for (const auto& c : m_cases) {
      if (this->selects(c.first)) {
        selected.push_back(&c);
        opWidth = std::max(opWidth, CaseName(c.first).operation.size());
      }
    }

    if (m_list) {
      for (const Entry* c : selected) {
        std::printf("%s\n", c->first.c_str());
      }

      return 0;
    }

    // The reports: JSON on standard output, or the table; and the JSON in a file, if asked for.
    std::unique_ptr<JsonReport>    json;
    std::unique_ptr<TableReport>   table;
    std::unique_ptr<std::ofstream> file;
    std::unique_ptr<JsonReport>    fileJson;

    if (m_json) {
      json = std::make_unique<JsonReport>(std::cout, m_name, m_reps, m_smoke);
    }
    else {
      table = std::make_unique<TableReport>(m_name, selected.size(), opWidth, m_reps, m_smoke, m_banner);

      if (EBGeometry::GPU::enabled && !m_deviceRuns) {
        table->noDevice();

        return 0;
      }

      table->header();
    }

    if (!m_jsonOut.empty()) {
      file = std::make_unique<std::ofstream>(m_jsonOut, std::ios::app);

      if (!*file) {
        this->fail("cannot write " + m_jsonOut);
      }

      fileJson = std::make_unique<JsonReport>(*file, m_name, m_reps, m_smoke);
    }

    for (const Entry* c : selected) {
      const CaseName name(c->first);
      Case           running(c->first, m_reps, m_smoke);

      if (table) {
        table->group(name);
      }

      // A case whose input is missing (the mesh submodule not checked out) is reported as skipped, so
      // that the other cases still run.
      try {
        c->second(running);
      } catch (const std::exception& e) {
        for (JsonReport* r : {json.get(), fileJson.get()}) {
          if (r != nullptr) {
            r->skipped(c->first, e.what());
          }
        }

        if (table) {
          table->skipped(name, e.what());
        }

        continue;
      }

      for (JsonReport* r : {json.get(), fileJson.get()}) {
        if (r != nullptr) {
          r->result(running);
        }
      }

      if (table) {
        table->result(name, running);
      }
    }

    return 0;
  }

private:
  using Entry = std::pair<std::string, std::function<void(Case&)>>;

  // Report a command-line or output error and exit with 2.
  [[noreturn]] void
  fail(const std::string& a_message) const
  {
    std::fprintf(stderr, "%s: %s\n", m_name.c_str(), a_message.c_str());
    std::exit(2);
  }

  // Whether the command line selects a case: every case without --case, else those matching a glob.
  [[nodiscard]] bool
  selects(const std::string& a_name) const
  {
    return m_globs.empty() || std::any_of(m_globs.begin(), m_globs.end(), [&a_name](const std::string& g) {
             return globMatch(g.c_str(), a_name.c_str());
           });
  }

  std::string              m_name;
  std::vector<Entry>       m_cases;
  std::vector<std::string> m_globs;
  std::string              m_jsonOut;
  int                      m_reps       = 5;
  bool                     m_smoke      = false;
  bool                     m_list       = false;
  bool                     m_json       = false;
  bool                     m_banner     = true;
  bool                     m_deviceRuns = false;
};

} // namespace Perf

#endif
