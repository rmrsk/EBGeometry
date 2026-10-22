# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
#
# SPDX-License-Identifier: GPL-3.0-or-later

# Check a pull request's performance receipt: that the comparison was run, on this code, against the
# current base, and that its verdict may be merged. Run by CI's Performance-receipt job; it needs only
# CMake and git (with the base branch and the pull request's history fetched).
#
#   cmake -DBASE_REF=origin/dev -DHEAD_REF=<the pull request's head commit> -P Performance/CheckReceipt.cmake
#
# A pull request that changes none of what a comparison measures (see Receipt.cmake: the library and the
# probes) needs no receipt: it passes, whatever receipt it inherited. Otherwise its host receipt,
# Performance/Receipt/host.md at HEAD_REF, must exist and
#
#   * have measured this code:     its head fingerprint is HEAD_REF's;
#   * have measured today's base:  its base fingerprint is BASE_REF's, so a base whose code moved on
#                                  since needs a rebase and a new comparison;
#   * not be red, unless accepted: a reason written after "accepted:".
#
# Any other receipt is stale, and fails: an old receipt never stands for new code. Device receipts
# (cuda.md, hip.md) are optional; they are reported, never required. SUMMARY, if set, is a file the
# result is appended to as Markdown (CI's job summary).

cmake_minimum_required(VERSION 3.16)

get_filename_component(REPO "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
include("${CMAKE_CURRENT_LIST_DIR}/Receipt.cmake")

foreach(var BASE_REF HEAD_REF)
  if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
    message(FATAL_ERROR "CheckReceipt.cmake: set ${var} (see the comment at the top)")
  endif()
endforeach()

set(rerun "Scripts/run-performance.sh compare ${BASE_REF} --record")
set(lines "")

# Report a line, to the terminal and to the summary.
macro(say text)
  message("${text}")
  string(APPEND lines "${text}\n")
endmacro()

# End with the result: pass, or fail with what to do.
macro(finish passed)
  if(DEFINED SUMMARY AND NOT SUMMARY STREQUAL "")
    file(APPEND "${SUMMARY}" "### Performance receipt\n\n${lines}\n")
  endif()

  if(NOT ${passed})
    message(FATAL_ERROR "CheckReceipt.cmake: the performance receipt does not stand for this pull request (see above)")
  endif()

  return()
endmacro()

# What the receipt must match: this code, and the base as it is now.
execute_process(COMMAND git merge-base ${BASE_REF} ${HEAD_REF} WORKING_DIRECTORY "${REPO}" OUTPUT_VARIABLE fork
                OUTPUT_STRIP_TRAILING_WHITESPACE RESULT_VARIABLE rc ERROR_VARIABLE err)

if(NOT rc EQUAL 0)
  message(FATAL_ERROR "CheckReceipt.cmake: no common history of ${BASE_REF} and ${HEAD_REF} (fetch both in "
                      "full): ${err}")
endif()

perf_fingerprint(${HEAD_REF} head_now)
perf_fingerprint(${BASE_REF} base_now)
perf_fingerprint(${fork} fork_now)

if(head_now STREQUAL fork_now)
  say("No change to the library or the probes against ${BASE_REF}: no performance receipt is required.")
  finish(TRUE)
endif()

# The host receipt, as the pull request committed it.
execute_process(COMMAND git show ${HEAD_REF}:${PERF_RECEIPT_DIR}/host.md WORKING_DIRECTORY "${REPO}"
                OUTPUT_VARIABLE receipt RESULT_VARIABLE rc ERROR_QUIET)

if(NOT rc EQUAL 0)
  say("**No performance receipt.** This pull request changes the library or the probes, and carries no")
  say("${PERF_RECEIPT_DIR}/host.md. Run the comparison locally, on a quiet machine, and commit its receipt:")
  say("")
  say("    ${rerun}")
  finish(FALSE)
endif()

string(PREPEND receipt "\n")

foreach(field version verdict accepted base base-fingerprint head head-fingerprint date)
  perf_receipt_field("${receipt}" ${field} r_${field})
endforeach()

say("Performance receipt (host): `${r_head}` against `${r_base}`, measured ${r_date}: **${r_verdict}**.")
set(passed TRUE)

if(NOT r_version STREQUAL PERF_RECEIPT_VERSION)
  say("- **Unreadable**: receipt version '${r_version}', expected ${PERF_RECEIPT_VERSION}. Record a new one: `${rerun}`")
  finish(FALSE)
endif()

if(NOT r_head-fingerprint STREQUAL head_now)
  say("- **Stale**: it measured other code (`${r_head}`); the library or the probes changed since. A receipt")
  say("  stands only for the code it measured. Record a new one and commit it: `${rerun}`")
  set(passed FALSE)
endif()

if(NOT r_base-fingerprint STREQUAL base_now)
  say("- **Stale base**: it compared against `${r_base}`, and ${BASE_REF}'s code has changed since. Rebase on")
  say("  ${BASE_REF}, then record a new receipt and commit it: `${rerun}`")
  set(passed FALSE)
endif()

if(r_verdict STREQUAL "red")
  if(r_accepted STREQUAL "")
    say("- **Red verdict**: fix the regression and record again, or, if it is accepted, write the reason")
    say("  after \"accepted:\" in the receipt.")
    set(passed FALSE)
  else()
    say("- Red verdict, **accepted**: ${r_accepted}")
  endif()
elseif(r_verdict STREQUAL "yellow")
  say("- Yellow verdict: a minor regression or a changed checksum; the pull request should say why.")
elseif(NOT r_verdict STREQUAL "green")
  say("- **Unreadable** verdict '${r_verdict}'. Record a new receipt: `${rerun}`")
  set(passed FALSE)
endif()

if(passed)
  say("- It measured this code, against ${BASE_REF} as it is now.")
endif()

# Device receipts: reported, never required.
foreach(side cuda hip)
  execute_process(COMMAND git show ${HEAD_REF}:${PERF_RECEIPT_DIR}/${side}.md WORKING_DIRECTORY "${REPO}"
                  OUTPUT_VARIABLE device RESULT_VARIABLE rc ERROR_QUIET)

  if(rc EQUAL 0)
    string(PREPEND device "\n")
    perf_receipt_field("${device}" verdict d_verdict)
    perf_receipt_field("${device}" head-fingerprint d_head)
    perf_receipt_field("${device}" base-fingerprint d_base)

    if(d_head STREQUAL head_now AND d_base STREQUAL base_now)
      say("- Device receipt (${side}): ${d_verdict}, measured on this code.")
    else()
      say("- Device receipt (${side}): ${d_verdict}, but stale (measured other code); optional, not checked.")
    endif()
  endif()
endforeach()

finish(passed)
