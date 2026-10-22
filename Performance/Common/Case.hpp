// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   Case.hpp
 * @brief  One performance case while it runs: its name, the repetitions it times, and what it records
 * (a checksum, pool bytes, parameters, further metrics), written out as one JSON line.
 * @details Plain C++17 with no dependency on EBGeometry.
 */

#ifndef EBGEOMETRY_PERF_CASE_HPP
#define EBGEOMETRY_PERF_CASE_HPP

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

#include "Environment.hpp"
#include "Schema.hpp"
#include "Statistics.hpp"

namespace Perf {

/**
 * @brief A case name from its parts, joined with '/': group.../operation/precision/side.
 * @param[in] a_parts The parts, from general to specific.
 * @return The name.
 */
inline std::string
caseName(std::initializer_list<std::string> a_parts)
{
  std::string name;

  for (const auto& part : a_parts) {
    if (!name.empty()) {
      name += '/';
    }

    name += part;
  }

  return name;
}

/**
 * @brief A case name split into its parts: the group (the parts before the operation), the operation,
 * the precision and the side, from a name of the form group.../operation/precision/side.
 */
struct CaseName
{
  std::vector<std::string> group;     ///< The leading parts (component, input, ...).
  std::string              operation; ///< What is timed (build, query, ...).
  std::string              precision; ///< float or double.
  std::string              side;      ///< host or hostdevice.

  /**
   * @brief Split a case name.
   * @param[in] a_name The name.
   */
  explicit CaseName(const std::string& a_name)
  {
    std::vector<std::string> parts;
    std::size_t              begin = 0;

    while (true) {
      const auto slash = a_name.find('/', begin);

      parts.push_back(a_name.substr(begin, slash - begin));

      if (slash == std::string::npos) {
        break;
      }

      begin = slash + 1;
    }

    const std::size_t n = parts.size();

    side      = n >= 1 ? parts[n - 1] : "";
    precision = n >= 2 ? parts[n - 2] : "";
    operation = n >= 3 ? parts[n - 3] : "";
    group.assign(parts.begin(), parts.begin() + static_cast<std::ptrdiff_t>(n >= 3 ? n - 3 : 0));
  }

  /**
   * @brief The group, its parts joined.
   * @param[in] a_separator Between parts.
   * @return The joined group.
   */
  [[nodiscard]] std::string
  groupLabel(const std::string& a_separator) const
  {
    std::string out;

    for (std::size_t i = 0; i < group.size(); i++) {
      out += (i > 0 ? a_separator : "") + group[i];
    }

    return out;
  }
};

/**
 * @brief One case while it runs: what it timed and recorded.
 * @details A case function sets up its inputs (untimed), times the operation with measure() (or
 * measureFresh() for something that cannot run twice on the same state, such as a build), and records
 * a checksum of its results, its code-selecting parameters and, where there is one, its pool bytes.
 * Every timing runs the work once untimed (warm-up), then the configured number of times.
 */
class Case
{
public:
  /**
   * @brief A case about to run.
   * @param[in] a_name  Its name.
   * @param[in] a_reps  Timed repetitions.
   * @param[in] a_smoke Whether this is a smoke run.
   */
  Case(std::string a_name, int a_reps, bool a_smoke) : m_name(std::move(a_name)), m_reps(a_reps), m_smoke(a_smoke)
  {}

  /**
   * @brief Whether this is a smoke run, in which a case uses its small problem size.
   * @return True for --smoke.
   */
  [[nodiscard]] bool
  smoke() const noexcept
  {
    return m_smoke;
  }

  /**
   * @brief A problem size: the full one, or the smoke one under --smoke.
   * @param[in] a_full  Size for a timing run.
   * @param[in] a_smoke Size for a smoke run.
   * @return The size to use.
   */
  [[nodiscard]] std::size_t
  size(std::size_t a_full, std::size_t a_smoke) const noexcept
  {
    return m_smoke ? a_smoke : a_full;
  }

  /**
   * @brief Time a function as the case's own time.
   * @details A device case passes a function that launches its kernel and returns once it has finished
   * (GPU::transform and GPU::parallelFor already return then), so that wall time is kernel time.
   * @param[in] a_body        The work to time.
   * @param[in] a_itemsPerRep Items processed per repetition, for a time per item (0: none).
   */
  void
  measure(const std::function<void()>& a_body, std::size_t a_itemsPerRep = 0)
  {
    m_times = this->repeat(a_body);
    m_items = a_itemsPerRep;
  }

  /**
   * @brief Time something that cannot run twice on the same state (a build) as the case's own time:
   * a_setup prepares fresh state, untimed, before every run of a_body.
   * @param[in] a_setup Untimed preparation.
   * @param[in] a_body  The work to time.
   */
  void
  measureFresh(const std::function<void()>& a_setup, const std::function<void()>& a_body)
  {
    m_times = this->repeat(a_setup, a_body);
  }

  /**
   * @brief Time a function as measure() does, but return the times rather than recording them (for
   * the second side of a pair).
   * @param[in] a_body The work to time.
   * @return The time of each repetition, in milliseconds.
   */
  [[nodiscard]] std::vector<double>
  repeat(const std::function<void()>& a_body) const
  {
    return this->repeat([] {}, a_body);
  }

  /**
   * @brief Record a checksum of the results. Equal inputs should give equal checksums before and after
   * a change; a comparison reports any difference.
   * @param[in] a_value The checksum.
   */
  void
  checksum(double a_value) noexcept
  {
    m_checksum    = a_value;
    m_hasChecksum = true;
  }

  /**
   * @brief Record an exact checksum of the points a set of queries found (Probe.hpp's IndexChecksum); a
   * comparison reports any difference.
   * @param[in] a_value The checksum.
   */
  void
  indexChecksum(std::uint64_t a_value) noexcept
  {
    m_indexChecksum    = a_value;
    m_hasIndexChecksum = true;
  }

  /**
   * @brief Record the bytes the measured structure occupies in its pool (Pool::usedBytes()).
   * @param[in] a_bytes Bytes.
   */
  void
  poolBytes(std::size_t a_bytes) noexcept
  {
    m_poolBytes = a_bytes;
  }

  /**
   * @brief Record a code-selecting parameter (a branching factor, a width, a point count).
   * @param[in] a_key   Name.
   * @param[in] a_value Value.
   */
  void
  param(const std::string& a_key, std::size_t a_value)
  {
    m_params.emplace_back(a_key, a_value);
  }

  /**
   * @brief Record a further number (Key::HostMs and the other pair metrics).
   * @param[in] a_key   Name.
   * @param[in] a_value Value.
   */
  void
  metric(const std::string& a_key, double a_value)
  {
    m_metrics.emplace_back(a_key, a_value);
  }

  /**
   * @brief The case's name.
   * @return The name.
   */
  [[nodiscard]] const std::string&
  name() const noexcept
  {
    return m_name;
  }

  /**
   * @brief The summary of the case's own times, in milliseconds.
   * @return Median, interquartile range and minimum.
   */
  [[nodiscard]] Summary
  summary() const
  {
    return summarize(m_times);
  }

  /**
   * @brief Items per repetition (0: none).
   * @return The count.
   */
  [[nodiscard]] std::size_t
  items() const noexcept
  {
    return m_items;
  }

  /**
   * @brief The pool bytes recorded with poolBytes() (0: none).
   * @return The bytes.
   */
  [[nodiscard]] std::size_t
  poolBytesUsed() const noexcept
  {
    return m_poolBytes;
  }

  /**
   * @brief A metric recorded with metric().
   * @param[in] a_key      Name.
   * @param[in] a_fallback Value if it was not recorded.
   * @return The value.
   */
  [[nodiscard]] double
  metricOr(const std::string& a_key, double a_fallback) const
  {
    for (const auto& m : m_metrics) {
      if (m.first == a_key) {
        return m.second;
      }
    }

    return a_fallback;
  }

  /**
   * @brief The case as one JSON line (Schema.hpp).
   * @return The JSON, without a newline.
   */
  [[nodiscard]] std::string
  json() const
  {
    const Summary s = this->summary();

    JsonObject out;
    out.text(Key::Case, m_name)
      .number(Key::MedianMs, s.median)
      .number(Key::MinMs, s.minimum)
      .number(Key::IqrMs, s.iqr)
      .numbers(Key::TimesMs, m_times);

    if (m_items > 0) {
      out.count(Key::Items, m_items).number(Key::UsPerItem, 1e3 * s.median / static_cast<double>(m_items));
    }

    if (m_hasChecksum) {
      out.number(Key::Checksum, m_checksum);
    }

    // As hex text, since a JSON number would lose the hash's low bits.
    if (m_hasIndexChecksum) {
      char hex[17];
      std::snprintf(hex, sizeof(hex), "%016llx", static_cast<unsigned long long>(m_indexChecksum));
      out.text(Key::IndexChecksum, hex);
    }

    if (m_poolBytes > 0) {
      out.count(Key::PoolBytes, m_poolBytes);
    }

    if (!m_metrics.empty()) {
      JsonObject metrics;

      for (const auto& m : m_metrics) {
        metrics.raw(m.first, jsonNumber(m.second));
      }

      out.object(Key::Metrics, metrics);
    }

    JsonObject params;

    for (const auto& p : m_params) {
      params.raw(p.first, std::to_string(p.second));
    }

    return out.object(Key::Params, params).str();
  }

private:
  // Run a_setup then a_body once (warm-up), then m_reps times with a_body timed.
  [[nodiscard]] std::vector<double>
  repeat(const std::function<void()>& a_setup, const std::function<void()>& a_body) const
  {
    a_setup();
    a_body();

    std::vector<double> times;

    for (int r = 0; r < m_reps; r++) {
      a_setup();

      // Untimed: what the last repetition freed goes back to the system, so that a pool built in this
      // one starts on fresh pages, as a first build's does (fixAllocatorPolicy()).
      releaseFreedMemory();

      const auto start = std::chrono::steady_clock::now();
      a_body();
      const auto stop = std::chrono::steady_clock::now();

      times.push_back(std::chrono::duration<double, std::milli>(stop - start).count());
    }

    return times;
  }

  std::string                                      m_name;
  int                                              m_reps;
  bool                                             m_smoke;
  std::vector<double>                              m_times;
  std::size_t                                      m_items            = 0;
  double                                           m_checksum         = 0.0;
  bool                                             m_hasChecksum      = false;
  std::uint64_t                                    m_indexChecksum    = 0;
  bool                                             m_hasIndexChecksum = false;
  std::size_t                                      m_poolBytes        = 0;
  std::vector<std::pair<std::string, std::size_t>> m_params;
  std::vector<std::pair<std::string, double>>      m_metrics;
};

} // namespace Perf

#endif
