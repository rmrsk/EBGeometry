// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

// A scope that switches the program to a locale with a decimal comma, both the global C++ locale (which streams
// follow) and the C library's (which std::strtod and std::snprintf follow), for tests that check that numbers are
// read and written with a decimal point whatever locale the program set.

#ifndef EBGEOMETRY_TESTLOCALE_HPP
#define EBGEOMETRY_TESTLOCALE_HPP

#include <clocale>
#include <locale>
#include <stdexcept>
#include <string>

namespace EBGeometryTestLocale {

// Makes the first decimal-comma locale installed the global C++ locale (which also sets the C library's), and
// restores both when it goes out of scope. active() is false when none is installed (a test then skips).
class DecimalComma
{
public:
  DecimalComma()
  {
    const char* current = std::setlocale(LC_ALL, nullptr);

    m_previousC = (current != nullptr) ? current : "C";

    for (const char* name : {"de_DE.UTF-8", "de_DE.utf8", "nb_NO.UTF-8", "nb_NO.utf8", "fr_FR.UTF-8", "fr_FR.utf8"}) {
      try {
        m_previous = std::locale::global(std::locale(name));
        m_active   = true;

        break;
      } catch (const std::runtime_error&) {
        // Not installed: try the next.
      }
    }
  }

  DecimalComma(const DecimalComma&) = delete;
  DecimalComma&
  operator=(const DecimalComma&) = delete;

  ~DecimalComma()
  {
    if (m_active) {
      std::locale::global(m_previous);
    }

    std::setlocale(LC_ALL, m_previousC.c_str());
  }

  bool
  active() const
  {
    return m_active;
  }

private:
  std::locale m_previous;
  std::string m_previousC;
  bool        m_active = false;
};

} // namespace EBGeometryTestLocale

#endif
