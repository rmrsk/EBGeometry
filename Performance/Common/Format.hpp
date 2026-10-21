// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   Format.hpp
 * @brief  Text formatting shared by the probes and PerfCompare: times, counts and sizes in readable
 * units, padding that counts characters rather than bytes, terminal styling, and a table that renders
 * to the terminal and to Markdown.
 * @details Plain C++17 with no dependency on EBGeometry, so that PerfCompare can use it. Terminal
 * detection uses isatty() (POSIX: Linux and macOS, as the performance tooling supports).
 */

#ifndef EBGEOMETRY_PERF_FORMAT_HPP
#define EBGEOMETRY_PERF_FORMAT_HPP

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <vector>

namespace Perf {

/**
 * @brief The width of a string in characters: UTF-8 code points, so that a unit like µs lines up.
 * @param[in] a_s The string.
 * @return Its width.
 */
inline std::size_t
displayWidth(const std::string& a_s)
{
  std::size_t chars = 0;

  for (const char c : a_s) {
    chars += (static_cast<unsigned char>(c) & 0xC0) != 0x80 ? 1 : 0;
  }

  return chars;
}

/**
 * @brief How a cell is aligned in its column.
 */
enum class Align
{
  Left,
  Right
};

/**
 * @brief Pad a string to a width in characters.
 * @param[in] a_s     The string.
 * @param[in] a_width The width.
 * @param[in] a_align Left (pad on the right) or right (pad on the left).
 * @return The padded string.
 */
inline std::string
pad(const std::string& a_s, std::size_t a_width, Align a_align = Align::Right)
{
  const std::size_t chars = displayWidth(a_s);
  const std::string fill(a_width > chars ? a_width - chars : 0, ' ');

  return a_align == Align::Left ? a_s + fill : fill + a_s;
}

/**
 * @brief Remove trailing blanks.
 * @param[in,out] a_s The string.
 */
inline void
trimRight(std::string& a_s)
{
  a_s.erase(a_s.find_last_not_of(' ') + 1);
}

/**
 * @brief A time in milliseconds, with a unit that keeps it between 1 and 1000 (ns, µs, ms or s) and
 * three significant digits.
 * @param[in] a_ms Milliseconds.
 * @return The formatted time.
 */
inline std::string
formatTime(double a_ms)
{
  const char* unit  = "ms";
  double      value = a_ms;

  if (a_ms >= 1000.0) {
    unit  = "s";
    value = a_ms / 1000.0;
  }
  else if (a_ms < 1e-3) {
    unit  = "ns";
    value = a_ms * 1e6;
  }
  else if (a_ms < 1.0) {
    unit  = "µs";
    value = a_ms * 1e3;
  }

  const int decimals = value < 10.0 ? 2 : (value < 100.0 ? 1 : 0);

  char buf[32];
  std::snprintf(buf, sizeof(buf), "%.*f %s", decimals, value, unit);

  return buf;
}

/**
 * @brief A count with thousands separators (262,144).
 * @param[in] a_count The count.
 * @return The formatted count.
 */
inline std::string
formatCount(std::size_t a_count)
{
  const std::string digits = std::to_string(a_count);
  std::string       out;

  for (std::size_t i = 0; i < digits.size(); i++) {
    if (i > 0 && (digits.size() - i) % 3 == 0) {
      out += ',';
    }

    out += digits[i];
  }

  return out;
}

/**
 * @brief A byte count in kB, MB or GB.
 * @param[in] a_bytes Bytes.
 * @return The formatted size.
 */
inline std::string
formatBytes(std::size_t a_bytes)
{
  const double b = static_cast<double>(a_bytes);
  char         buf[32];

  if (b >= 1e9) {
    std::snprintf(buf, sizeof(buf), "%.2f GB", b / 1e9);
  }
  else if (b >= 1e6) {
    std::snprintf(buf, sizeof(buf), "%.1f MB", b / 1e6);
  }
  else {
    std::snprintf(buf, sizeof(buf), "%.1f kB", b / 1e3);
  }

  return buf;
}

/**
 * @brief A number with printf formatting, as a string.
 * @param[in] a_format The printf format, with one conversion for a double.
 * @param[in] a_value  The number.
 * @return The formatted number.
 */
inline std::string
formatNumber(const char* a_format, double a_value)
{
  char buf[64];
  std::snprintf(buf, sizeof(buf), a_format, a_value);

  return buf;
}

/**
 * @brief The plural of a counted word: "1 case", "2 cases".
 * @param[in] a_count The count.
 * @param[in] a_word  The singular word.
 * @return The count and the word.
 */
inline std::string
counted(std::size_t a_count, const std::string& a_word)
{
  return std::to_string(a_count) + " " + a_word + (a_count == 1 ? "" : "s");
}

/**
 * @brief Terminal styling, only when standard output is a terminal, NO_COLOR is unset and TERM is not
 * "dumb", so that piped and redirected output (logs, CI) stays plain text.
 */
class Style
{
public:
  /**
   * @brief Style for standard output, as its terminal allows.
   */
  Style() : m_on(Style::terminalSupportsIt())
  {}

  /**
   * @brief Bold text.
   * @param[in] a_s The text.
   * @return The styled text.
   */
  [[nodiscard]] std::string
  bold(const std::string& a_s) const
  {
    return this->wrap("\033[1m", a_s);
  }

  /**
   * @brief Dimmed text.
   * @param[in] a_s The text.
   * @return The styled text.
   */
  [[nodiscard]] std::string
  dim(const std::string& a_s) const
  {
    return this->wrap("\033[2m", a_s);
  }

  /**
   * @brief Red text.
   * @param[in] a_s The text.
   * @return The styled text.
   */
  [[nodiscard]] std::string
  red(const std::string& a_s) const
  {
    return this->wrap("\033[31m", a_s);
  }

  /**
   * @brief Bold green, yellow or red text, for a verdict of level 0, 1 or 2.
   * @param[in] a_level 0 (good), 1 (look closer) or 2 (bad).
   * @param[in] a_s     The text.
   * @return The styled text.
   */
  [[nodiscard]] std::string
  verdict(int a_level, const std::string& a_s) const
  {
    static const char* const codes[] = {"\033[1;32m", "\033[1;33m", "\033[1;31m"};

    return this->wrap(codes[std::clamp(a_level, 0, 2)], a_s);
  }

private:
  [[nodiscard]] static bool
  terminalSupportsIt()
  {
    const char* term = std::getenv("TERM");

    return isatty(fileno(stdout)) != 0 && std::getenv("NO_COLOR") == nullptr &&
           !(term != nullptr && std::string(term) == "dumb");
  }

  [[nodiscard]] std::string
  wrap(const char* a_code, const std::string& a_s) const
  {
    return m_on ? a_code + a_s + "\033[0m" : a_s;
  }

  bool m_on;
};

/**
 * @brief A table of text cells, rendered for the terminal (aligned columns, each as wide as its widest
 * cell) or as Markdown.
 */
class Table
{
public:
  /**
   * @brief A column: its header and alignment.
   */
  struct Column
  {
    std::string header; ///< The header.
    Align       align;  ///< The alignment of its cells.
  };

  /**
   * @brief A table with these columns.
   * @param[in] a_columns The columns.
   */
  explicit Table(std::vector<Column> a_columns) : m_columns(std::move(a_columns))
  {}

  /**
   * @brief Append a row: one cell per column.
   * @param[in] a_cells The cells.
   */
  void
  add(std::vector<std::string> a_cells)
  {
    a_cells.resize(m_columns.size());
    m_rows.push_back(std::move(a_cells));
  }

  /**
   * @brief The table for the terminal: the header, then one line per row, each indented and with its
   * trailing blanks removed.
   * @param[in] a_indent Spaces before each line.
   * @return The lines, each ending in a newline.
   */
  [[nodiscard]] std::string
  terminal(std::size_t a_indent = 2) const
  {
    std::vector<std::size_t> widths(m_columns.size());

    for (std::size_t j = 0; j < m_columns.size(); j++) {
      widths[j] = displayWidth(m_columns[j].header);

      for (const auto& row : m_rows) {
        widths[j] = std::max(widths[j], displayWidth(row[j]));
      }
    }

    std::string out = this->line(this->headers(), widths, a_indent);

    for (const auto& row : m_rows) {
      out += this->line(row, widths, a_indent);
    }

    return out;
  }

  /**
   * @brief The table as Markdown.
   * @return The Markdown table, each line ending in a newline.
   */
  [[nodiscard]] std::string
  markdown() const
  {
    std::string out  = "|";
    std::string rule = "|";

    for (const auto& c : m_columns) {
      out += " " + c.header + " |";
      rule += c.align == Align::Right ? "---:|" : "---|";
    }

    out += "\n" + rule + "\n";

    for (const auto& row : m_rows) {
      out += "|";

      for (const auto& cell : row) {
        out += " " + cell + " |";
      }

      out += "\n";
    }

    return out;
  }

private:
  [[nodiscard]] std::vector<std::string>
  headers() const
  {
    std::vector<std::string> h;

    for (const auto& c : m_columns) {
      h.push_back(c.header);
    }

    return h;
  }

  [[nodiscard]] std::string
  line(const std::vector<std::string>& a_cells, const std::vector<std::size_t>& a_widths, std::size_t a_indent) const
  {
    std::string out(a_indent, ' ');

    for (std::size_t j = 0; j < a_cells.size(); j++) {
      out += (j > 0 ? "  " : "") + pad(a_cells[j], a_widths[j], m_columns[j].align);
    }

    trimRight(out);

    return out + "\n";
  }

  std::vector<Column>                   m_columns;
  std::vector<std::vector<std::string>> m_rows;
};

} // namespace Perf

#endif
