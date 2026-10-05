// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_ParseError.hpp
 * @brief  Declaration of the exception the file readers throw.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_PARSEERROR_HPP
#define EBGEOMETRY_PARSEERROR_HPP

#include <cstddef>
#include <stdexcept>
#include <string>

namespace EBGeometry {
namespace Parser {

/**
 * @brief Thrown by the file readers, and by the file data classes' convertToDCEL(), when a file cannot
 * be read into a mesh.
 * @details Reports the file, the line where the problem was found (0 when there is no meaningful
 * line, as in a binary file or a file that could not be opened), and the reason. what() combines
 * them as "file:line: reason", or "file: reason" when the line is 0.
 */
class ParseError : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param[in] a_file   File being read.
   * @param[in] a_line   1-based line where the problem was found, or 0.
   * @param[in] a_reason What is wrong.
   */
  ParseError(const std::string& a_file, const std::size_t a_line, const std::string& a_reason)
    : std::runtime_error(a_file + (a_line > 0 ? ":" + std::to_string(a_line) : std::string()) + ": " + a_reason),
      m_file(a_file),
      m_line(a_line),
      m_reason(a_reason)
  {}

  /**
   * @brief File being read.
   * @return The file name as passed to the reader.
   */
  [[nodiscard]] const std::string&
  file() const noexcept
  {
    return m_file;
  }

  /**
   * @brief Line where the problem was found.
   * @return 1-based line number, or 0 when there is no meaningful line.
   */
  [[nodiscard]] std::size_t
  line() const noexcept
  {
    return m_line;
  }

  /**
   * @brief What is wrong.
   * @return The reason, without the file and line.
   */
  [[nodiscard]] const std::string&
  reason() const noexcept
  {
    return m_reason;
  }

private:
  std::string m_file;
  std::size_t m_line;
  std::string m_reason;
};

/**
 * @brief What a file reader does with a mesh whose faces do not form a clean, closed surface.
 * @details Covers the defects a reader can survive: neighbouring faces oriented inconsistently, an
 * edge shared by three or more faces, and faces that fold back onto each other. Such a mesh loads
 * and gives correct distances, but the sign of the distance near the defect is unreliable. Faults
 * that would corrupt the half-edge mesh itself (an index out of range, a non-finite coordinate, a
 * face that visits a vertex twice) always throw. Holes are never a defect.
 */
enum class OnDefect
{
  Throw, ///< Throw a ParseError naming the defect. The default.
  Warn   ///< Print a warning to std::cerr and load the mesh anyway.
};

} // namespace Parser
} // namespace EBGeometry

#endif
