// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   Schema.hpp
 * @brief  The JSON the probes write and PerfCompare reads: the field names, in one place, a writer for
 * the one-line objects, and a reader for exactly that form.
 * @details Each probe run writes one line describing the build (Key::Suite, Key::Git, ...) followed by
 * one line per case (Key::Case, the timings, Key::Metrics, Key::Params), or for a case whose input was
 * missing, Key::Case and Key::Skipped. Plain C++17 with no dependency on EBGeometry, so that PerfCompare
 * can use it.
 */

#ifndef EBGEOMETRY_PERF_SCHEMA_HPP
#define EBGEOMETRY_PERF_SCHEMA_HPP

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace Perf {

/**
 * @brief The field names of the JSON lines.
 */
namespace Key {

// The build line.
constexpr const char* Suite             = "suite";               ///< The probe's name.
constexpr const char* Git               = "git";                 ///< The commit measured.
constexpr const char* Compiler          = "compiler";            ///< The compiler and its version.
constexpr const char* Simd              = "simd";                ///< The host SIMD level.
constexpr const char* HostTunedDefaults = "host_tuned_defaults"; ///< Whether EBGEOMETRY_HOST_TUNED_DEFAULTS was set.
constexpr const char* GpuBackend        = "gpu_backend";         ///< host, CUDA or HIP.
constexpr const char* Cpu               = "cpu";                 ///< The CPU model.
constexpr const char* Allocator         = "allocator";           ///< How large allocations are served.
constexpr const char* Reps              = "reps";                ///< Timed repetitions per case.
constexpr const char* Smoke             = "smoke";               ///< Whether these were smoke sizes.

// A case line.
constexpr const char* Case          = "case";           ///< The case's name.
constexpr const char* Skipped       = "skipped";        ///< Why the case did not run (instead of the timings).
constexpr const char* MedianMs      = "median_ms";      ///< Median of the repetitions, in milliseconds.
constexpr const char* MinMs         = "min_ms";         ///< Fastest repetition, in milliseconds.
constexpr const char* IqrMs         = "iqr_ms";         ///< Interquartile range of the repetitions.
constexpr const char* TimesMs       = "times_ms";       ///< Every repetition, in milliseconds.
constexpr const char* Items         = "items";          ///< Items per repetition.
constexpr const char* UsPerItem     = "us_per_item";    ///< Median time per item, in microseconds.
constexpr const char* Checksum      = "checksum";       ///< A checksum of the results.
constexpr const char* IndexChecksum = "index_checksum"; ///< An exact hash of the points found, as hex text.
constexpr const char* PoolBytes     = "pool_bytes";     ///< The structure's bytes in its pool.
constexpr const char* Metrics       = "metrics";        ///< Further numbers (below).
constexpr const char* Params        = "params";         ///< The case's code-selecting parameters.
constexpr const char* RelativeTo    = "relative_to";    ///< A case whose time is reported relative to this one's.
constexpr const char* RelativeLabel = "relative_label"; ///< How the comparison names that ratio.

// Metrics of a host/device pair.
constexpr const char* HostMs         = "host_ms";          ///< The host side's median time.
constexpr const char* HostThreads    = "host_threads";     ///< The threads the host side ran on.
constexpr const char* HostOverDevice = "host_over_device"; ///< The device's speedup over the host.
constexpr const char* MaxDiff        = "max_diff";         ///< Largest host/device difference.
constexpr const char* Mismatches     = "mismatches";       ///< Inputs on which host and device disagree.

} // namespace Key

/**
 * @brief A string as a JSON string literal, on one line: quotes, backslashes and control characters
 * (a newline in an exception's message, say) are escaped.
 * @param[in] a_s The string.
 * @return The quoted, escaped string.
 */
inline std::string
jsonString(const std::string& a_s)
{
  std::string out = "\"";

  for (const char c : a_s) {
    if (c == '"' || c == '\\') {
      out += '\\';
      out += c;
    }
    else if (c == '\n') {
      out += "\\n";
    }
    else if (c == '\t') {
      out += "\\t";
    }
    else if (static_cast<unsigned char>(c) < 0x20) {
      char buf[7];
      std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned>(static_cast<unsigned char>(c)));
      out += buf;
    }
    else {
      out += c;
    }
  }

  return out + "\"";
}

/**
 * @brief A number as JSON, exactly (17 significant digits).
 * @param[in] a_v The number.
 * @return Its text.
 */
inline std::string
jsonNumber(double a_v)
{
  char buf[64];
  std::snprintf(buf, sizeof(buf), "%.17g", a_v);

  return buf;
}

/**
 * @brief A JSON object written on one line, its fields in the order they were added.
 */
class JsonObject
{
public:
  /**
   * @brief Add a string field.
   * @param[in] a_key   Name.
   * @param[in] a_value Value.
   * @return This object.
   */
  JsonObject&
  text(const char* a_key, const std::string& a_value)
  {
    return this->raw(a_key, jsonString(a_value));
  }

  /**
   * @brief Add a number field.
   * @param[in] a_key   Name.
   * @param[in] a_value Value.
   * @return This object.
   */
  JsonObject&
  number(const char* a_key, double a_value)
  {
    return this->raw(a_key, jsonNumber(a_value));
  }

  /**
   * @brief Add a whole-number field.
   * @param[in] a_key   Name.
   * @param[in] a_value Value.
   * @return This object.
   */
  JsonObject&
  count(const char* a_key, std::size_t a_value)
  {
    return this->raw(a_key, std::to_string(a_value));
  }

  /**
   * @brief Add a boolean field.
   * @param[in] a_key   Name.
   * @param[in] a_value Value.
   * @return This object.
   */
  JsonObject&
  flag(const char* a_key, bool a_value)
  {
    return this->raw(a_key, a_value ? "true" : "false");
  }

  /**
   * @brief Add an array of numbers.
   * @param[in] a_key    Name.
   * @param[in] a_values Values.
   * @return This object.
   */
  JsonObject&
  numbers(const char* a_key, const std::vector<double>& a_values)
  {
    std::string list = "[";

    for (std::size_t i = 0; i < a_values.size(); i++) {
      list += (i > 0 ? "," : "") + jsonNumber(a_values[i]);
    }

    return this->raw(a_key, list + "]");
  }

  /**
   * @brief Add a nested object.
   * @param[in] a_key    Name.
   * @param[in] a_object The object.
   * @return This object.
   */
  JsonObject&
  object(const char* a_key, const JsonObject& a_object)
  {
    return this->raw(a_key, a_object.str());
  }

  /**
   * @brief Add a field whose value is already JSON text.
   * @param[in] a_key  Name.
   * @param[in] a_json Value, as JSON.
   * @return This object.
   */
  JsonObject&
  raw(const std::string& a_key, const std::string& a_json)
  {
    m_body += (m_body.empty() ? "" : ",") + jsonString(a_key) + ":" + a_json;

    return *this;
  }

  /**
   * @brief The object's text, on one line.
   * @return The JSON.
   */
  [[nodiscard]] std::string
  str() const
  {
    return "{" + m_body + "}";
  }

private:
  std::string m_body;
};

/**
 * @brief One JSON line as the probes write it, read field by field.
 * @details Not a general JSON parser: it finds a field by its quoted name followed by a colon, which is
 * unambiguous in these lines (names are unique across the line, nested objects included, and no string
 * value contains a quoted name followed by a colon).
 */
class JsonLine
{
public:
  /**
   * @brief Read a line.
   * @param[in] a_line The text of one JSON object.
   */
  explicit JsonLine(std::string a_line) : m_line(std::move(a_line))
  {}

  /**
   * @brief Whether the line has a field.
   * @param[in] a_key Name.
   * @return True if it does.
   */
  [[nodiscard]] bool
  has(const char* a_key) const
  {
    return this->find(a_key) != std::string::npos;
  }

  /**
   * @brief A string field.
   * @param[in] a_key Name.
   * @return Its value, unescaped (the escapes jsonString() writes), or empty if absent.
   */
  [[nodiscard]] std::string
  text(const char* a_key) const
  {
    std::size_t i = this->find(a_key);

    if (i == std::string::npos || m_line[i] != '"') {
      return "";
    }

    std::string out;

    for (i++; i < m_line.size() && m_line[i] != '"'; i++) {
      if (m_line[i] != '\\' || i + 1 >= m_line.size()) {
        out += m_line[i];
        continue;
      }

      const char e = m_line[++i];

      if (e == 'n') {
        out += '\n';
      }
      else if (e == 't') {
        out += '\t';
      }
      else if (e == 'u' && i + 4 < m_line.size()) {
        out += static_cast<char>(std::strtol(m_line.substr(i + 1, 4).c_str(), nullptr, 16));
        i += 4;
      }
      else {
        out += e;
      }
    }

    return out;
  }

  /**
   * @brief A number field.
   * @param[in] a_key      Name.
   * @param[in] a_fallback Value if the field is absent.
   * @return Its value.
   */
  [[nodiscard]] double
  number(const char* a_key, double a_fallback = 0.0) const
  {
    const std::size_t i = this->find(a_key);

    return i == std::string::npos ? a_fallback : std::atof(m_line.c_str() + i);
  }

  /**
   * @brief An array-of-numbers field.
   * @param[in] a_key Name.
   * @return Its values, or none if absent.
   */
  [[nodiscard]] std::vector<double>
  numbers(const char* a_key) const
  {
    const std::size_t i = this->find(a_key);

    if (i == std::string::npos || m_line[i] != '[') {
      return {};
    }

    std::vector<double> out;
    std::stringstream   list(m_line.substr(i + 1, m_line.find(']', i) - i - 1));
    std::string         item;

    while (std::getline(list, item, ',')) {
      out.push_back(std::atof(item.c_str()));
    }

    return out;
  }

private:
  // The position just past "<key>":, or npos.
  [[nodiscard]] std::size_t
  find(const char* a_key) const
  {
    const std::string pattern = jsonString(a_key) + ":";
    const std::size_t at      = m_line.find(pattern);

    return at == std::string::npos ? at : at + pattern.size();
  }

  std::string m_line;
};

} // namespace Perf

#endif
