# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
#
# SPDX-License-Identifier: GPL-3.0-or-later

# Checks that perf_write_receipt() (Receipt.cmake) writes a report whole: a yellow or red verdict has a
# semicolon, and a receipt once kept only the text before it (#227). Run by ctest with
#   cmake -DREPO=<an empty directory> -P Performance/TestReceipt.cmake
# The receipt is written under REPO, never into the repository.
cmake_minimum_required(VERSION 3.16)

if(NOT REPO)
  message(FATAL_ERROR "TestReceipt.cmake: set REPO to an empty directory")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/Receipt.cmake")

file(REMOVE_RECURSE "${REPO}/Performance")
file(MAKE_DIRECTORY "${REPO}/Performance")

set(report "**VERDICT: 2 minor regressions; rerun them with --rounds 10 before deciding (see Flagged above)**

| case | base | head | change |
|---|---:|---:|---:|
| `A/b/c/float/host` | 1.00 ms | 1.05 ms | +5.0% |
| `A/b/c/double/host` | 2.00 ms | 2.00 ms | +0.0% |")

foreach(side host cuda)
  perf_write_receipt(
    SIDE ${side} LEVEL 1 BASE_REF origin/main BASE aaaaaaa HEAD bbbbbbb BASE_FINGERPRINT 1111 HEAD_FINGERPRINT 2222
    CASES 2 ROUNDS 2 REPS 3 REPORT "${report}\nThese runs failed: x; y\n")
endforeach()

file(READ "${REPO}/${PERF_RECEIPT_FILE}" receipt)

foreach(side host cuda)
  perf_receipt_section("${receipt}" ${side} section)

  if(section STREQUAL "")
    message(FATAL_ERROR "TestReceipt.cmake: no ${side} section in the receipt")
  endif()

  foreach(expected "2 minor regressions; rerun them with --rounds 10" "| `A/b/c/double/host` | 2.00 ms | 2.00 ms | +0.0% |"
                   "These runs failed: x; y")
    string(FIND "${section}" "${expected}" at)

    if(at LESS 0)
      message(FATAL_ERROR "TestReceipt.cmake: the ${side} section lost \"${expected}\":\n${section}")
    endif()
  endforeach()

  perf_receipt_field("${section}" verdict verdict)

  if(NOT verdict STREQUAL "yellow")
    message(FATAL_ERROR "TestReceipt.cmake: the ${side} section's verdict is \"${verdict}\", not yellow")
  endif()
endforeach()

message(STATUS "TestReceipt.cmake: both sections keep their whole report")
