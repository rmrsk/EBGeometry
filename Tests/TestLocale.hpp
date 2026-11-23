// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

// A scope that switches the C library's LC_NUMERIC to a locale with a decimal comma, for tests that check that
// numbers are read and written with a decimal point whatever locale the program set.

#ifndef EBGEOMETRY_TESTLOCALE_HPP
#define EBGEOMETRY_TESTLOCALE_HPP

#include <clocale>
#include <string>

namespace EBGeometryTestLocale {

// Switches LC_NUMERIC to the first decimal-comma locale installed, and back to what it was when it goes out of
// scope. active() is false when none is installed (a test then skips).
class DecimalComma
{
public:
  DecimalComma()
  {
    const char* current = std::setlocale(LC_NUMERIC, nullptr);

    m_previous = (current != nullptr) ? current : "C";

    for (const char* name : {"de_DE.UTF-8", "de_DE.utf8", "nb_NO.UTF-8", "nb_NO.utf8", "fr_FR.UTF-8", "fr_FR.utf8"}) {
      if (std::setlocale(LC_NUMERIC, name) != nullptr) {
        m_active = true;

        break;
      }
    }
  }

  DecimalComma(const DecimalComma&) = delete;
  DecimalComma&
  operator=(const DecimalComma&) = delete;

  ~DecimalComma()
  {
    std::setlocale(LC_NUMERIC, m_previous.c_str());
  }

  bool
  active() const
  {
    return m_active;
  }

private:
  std::string m_previous;
  bool        m_active = false;
};

} // namespace EBGeometryTestLocale

#endif
