// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   Report.hpp
 * @brief  Where a probe's results go as its cases complete: the table on the terminal (one layout for
 * a host build, one for the host/device pairs of a GPU build) and the JSON lines PerfCompare reads.
 */

#ifndef EBGEOMETRY_PERF_REPORT_HPP
#define EBGEOMETRY_PERF_REPORT_HPP

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include <EBGeometry.hpp>

#include "Case.hpp"
#include "Environment.hpp"
#include "Format.hpp"
#include "Schema.hpp"

namespace Perf {

/**
 * @brief The line describing what built and ran the cases: the first JSON line of every probe run.
 * @param[in] a_suite The probe's name.
 * @param[in] a_reps  Timed repetitions per case.
 * @param[in] a_smoke Whether this is a smoke run.
 * @return The JSON line, without a newline.
 */
inline std::string
buildJson(const std::string& a_suite, int a_reps, bool a_smoke)
{
  return JsonObject()
    .text(Key::Suite, a_suite)
    .text(Key::Git, gitSha())
    .text(Key::Compiler, compilerName())
    .text(Key::Simd, simdLevel())
    .flag(Key::HostTunedDefaults, hostTunedDefaults())
    .text(Key::GpuBackend, EBGeometry::GPU::backendName)
    .text(Key::Cpu, cpuModel())
    .count(Key::Reps, static_cast<std::size_t>(a_reps))
    .flag(Key::Smoke, a_smoke)
    .str();
}

/**
 * @brief JSON lines, written to a stream as the cases complete.
 */
class JsonReport
{
public:
  /**
   * @brief A report to a stream, starting with the build line.
   * @param[in,out] a_out   The stream.
   * @param[in]     a_suite The probe's name.
   * @param[in]     a_reps  Timed repetitions per case.
   * @param[in]     a_smoke Whether this is a smoke run.
   */
  JsonReport(std::ostream& a_out, const std::string& a_suite, int a_reps, bool a_smoke) : m_out(a_out)
  {
    m_out << buildJson(a_suite, a_reps, a_smoke) << "\n" << std::flush;
  }

  /**
   * @brief A case that ran.
   * @param[in] a_case The case.
   */
  void
  result(const Case& a_case)
  {
    m_out << a_case.json() << "\n" << std::flush;
  }

  /**
   * @brief A case that could not run.
   * @param[in] a_name   Its name.
   * @param[in] a_reason Why.
   */
  void
  skipped(const std::string& a_name, const std::string& a_reason)
  {
    m_out << JsonObject().text(Key::Case, a_name).text(Key::Skipped, a_reason).str() << "\n" << std::flush;
  }

private:
  std::ostream& m_out;
};

/**
 * @brief The table on the terminal: a banner with what the cases share, a rule with the probe's name,
 * a column header, then the cases grouped by component and input, printed as they complete. A host
 * build shows each case's median, spread, time per item and pool bytes; a GPU build shows each pair's
 * host and device time per item, the device's speedup, its spread, and whether the answers agree.
 */
class TableReport
{
public:
  /**
   * @brief Start the table.
   * @param[in] a_suite     The probe's name.
   * @param[in] a_cases     The number of cases selected.
   * @param[in] a_opWidth   The width of the longest operation name.
   * @param[in] a_reps      Timed repetitions per case.
   * @param[in] a_smoke     Whether this is a smoke run.
   * @param[in] a_banner    Whether to print the banner (Scripts/run-performance.sh prints it once).
   */
  TableReport(
    const std::string& a_suite, std::size_t a_cases, std::size_t a_opWidth, int a_reps, bool a_smoke, bool a_banner)
    : m_gpu(EBGeometry::GPU::enabled), m_opWidth(a_opWidth)
  {
    if (a_banner) {
      this->banner(a_reps, a_smoke);
    }

    this->title(a_suite, a_cases);
  }

  /**
   * @brief Say that a GPU build found no GPU, and so has no cases to run.
   */
  void
  noDevice() const
  {
    std::printf("  No GPU is visible: a GPU build runs only host/device pairs.\n\n");
  }

  /**
   * @brief Print the column header.
   */
  void
  header() const
  {
    std::vector<std::string> cells;

    for (const auto& c : this->columns()) {
      cells.push_back(c.header);
    }

    std::printf("%s\n", m_style.dim(this->row("operation", "precision", cells)).c_str());
  }

  /**
   * @brief Start a new group of cases, if a_name begins one.
   * @param[in] a_name The next case's name.
   */
  void
  group(const CaseName& a_name)
  {
    const std::string label = a_name.groupLabel(" · ");

    if (!m_started || label != m_group) {
      m_started = true;
      m_group   = label;
      std::printf("  %s\n", m_style.bold(label).c_str());
    }
  }

  /**
   * @brief A case that ran.
   * @param[in] a_name Its name.
   * @param[in] a_case The case.
   */
  void
  result(const CaseName& a_name, const Case& a_case) const
  {
    const Summary     s      = a_case.summary();
    const std::size_t items  = a_case.items();
    const std::string spread = formatNumber("%.1f%%", s.median > 0.0 ? 100.0 * s.iqr / s.median : 0.0);
    const std::string count  = items > 0 ? formatCount(items) : "";

    if (!m_gpu) {
      std::printf("%s\n",
                  this
                    ->row(a_name.operation,
                          a_name.precision,
                          {count,
                           formatTime(s.median),
                           spread,
                           items > 0 ? formatTime(s.median / static_cast<double>(items)) : "",
                           a_case.poolBytesUsed() > 0 ? formatBytes(a_case.poolBytesUsed()) : ""})
                    .c_str());
      std::fflush(stdout);

      return;
    }

    // Per item, so that pairs of different sizes compare; the totals are in the JSON.
    const double n          = items > 0 ? static_cast<double>(items) : 1.0;
    const double hostMs     = a_case.metricOr(Key::HostMs, 0.0);
    const double mismatches = a_case.metricOr(Key::Mismatches, 0.0);
    const bool   differ     = mismatches > 0.0;

    std::string answers = "agree";

    if (differ) {
      char buf[96];
      std::snprintf(
        buf, sizeof(buf), "%.0f of %zu DIFFER (max diff %.3g)", mismatches, items, a_case.metricOr(Key::MaxDiff, 0.0));
      answers = buf;
    }

    std::string line = this->row(a_name.operation,
                                 a_name.precision,
                                 {count,
                                  formatTime(hostMs / n),
                                  formatTime(s.median / n),
                                  formatNumber("%.1fx", s.median > 0.0 ? hostMs / s.median : 0.0),
                                  spread,
                                  answers});

    // A disagreement in red: the answers column is the last, so it is the line's tail.
    if (differ) {
      const auto at = line.rfind(answers);
      line          = line.substr(0, at) + m_style.red(line.substr(at));
    }

    std::printf("%s\n", line.c_str());
    std::fflush(stdout);
  }

  /**
   * @brief A case that could not run.
   * @param[in] a_name   Its name.
   * @param[in] a_reason Why.
   */
  void
  skipped(const CaseName& a_name, const std::string& a_reason) const
  {
    std::printf("%s\n", m_style.red(this->row(a_name.operation, a_name.precision, {"skipped: " + a_reason})).c_str());
    std::fflush(stdout);
  }

private:
  struct Column
  {
    std::string header;
    std::size_t width;
  };

  [[nodiscard]] std::vector<Column>
  columns() const
  {
    if (m_gpu) {
      return {
        {"items", 9}, {"host/item", 10}, {"device/item", 11}, {"speedup", 8}, {"device spread", 13}, {"answers", 7}};
    }

    return {{"items", 9}, {"median", 10}, {"spread", 7}, {"per item", 10}, {"pool", 9}};
  }

  // The operation and precision, then the cells right-aligned in their columns, trailing blanks
  // trimmed. A row with fewer cells than columns (a skipped case) ends early.
  [[nodiscard]] std::string
  row(const std::string& a_op, const std::string& a_precision, const std::vector<std::string>& a_cells) const
  {
    const std::vector<Column> cols = this->columns();
    std::string line = "    " + pad(a_op, m_opWidth, Align::Left) + "  " + pad(a_precision, 9, Align::Left);

    for (std::size_t j = 0; j < a_cells.size(); j++) {
      line += "  " + pad(a_cells[j], j < cols.size() && a_cells.size() == cols.size() ? cols[j].width : 0);
    }

    trimRight(line);

    return line;
  }

  void
  banner(int a_reps, bool a_smoke) const
  {
    const std::size_t threads = m_gpu ? static_cast<std::size_t>(hostThreads()) : 1;

    std::string facts = "EBGeometry performance · git " + gitSha() + " · " + compilerName() + " · GPU " +
                        (m_gpu ? EBGeometry::GPU::backendName : "none") + " · host: " + counted(threads, "thread") +
                        ", SIMD " + simdLevel();

    if (m_gpu && std::string(simdLevel()) == "none") {
      facts += " (nvcc cannot compile host SIMD code)";
    }

    const std::string times =
      a_reps == 1 ? "from a single repetition" : "medians of " + std::to_string(a_reps) + " repetitions";
    const std::string legend = m_gpu ? "host and device run the same inputs; times per item are " + times +
                                         "; device spread = the device's interquartile range / median"
                                     : "times are " + times + "; spread = interquartile range / median";

    std::printf("\n%s\n%s\n", m_style.dim(facts).c_str(), m_style.dim(legend).c_str());

    if (a_smoke) {
      std::printf("%s\n", m_style.dim("smoke sizes: checks that the cases run, times nothing").c_str());
    }
  }

  // A heavy rule across the table (at least 96 columns, so that the probes of a run line up), carrying
  // the probe's name and its number of cases.
  void
  title(const std::string& a_suite, std::size_t a_cases) const
  {
    std::size_t width = 4 + m_opWidth + 2 + 9;

    for (const auto& c : this->columns()) {
      width += 2 + c.width;
    }

    const std::string left  = "━━ " + a_suite + " ";
    const std::string right = " " + counted(a_cases, "case") + " ━━";
    const std::size_t used  = displayWidth(left) + displayWidth(right);
    std::string       fill;

    for (std::size_t i = used; i < std::max<std::size_t>({width, used + 4, 96}); i++) {
      fill += "━";
    }

    std::printf("\n%s\n", m_style.bold(left + fill + right).c_str());
  }

  bool        m_gpu;
  std::size_t m_opWidth;
  Style       m_style;
  bool        m_started = false;
  std::string m_group;
};

} // namespace Perf

#endif
