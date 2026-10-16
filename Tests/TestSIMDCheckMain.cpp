// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// One of the two translation units of the TestSIMDCheck programs (see Tests/CMakeLists.txt). The
// programs build it and TestSIMDCheckOther.cpp with different settings, and the startup check in
// EBGeometry_SIMDCheck.hpp warns before main() runs (SIMD flags) or aborts (host-tuned defaults).

#include "EBGeometry.hpp"

#include <cstdio>

int
otherUnit();

int
main()
{
  std::printf("main() reached, other unit returned %d\n", otherUnit());

  return 0;
}
