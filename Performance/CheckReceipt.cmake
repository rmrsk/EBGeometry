# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
#
# SPDX-License-Identifier: GPL-3.0-or-later

# Check a pull request's performance receipt: that the comparison was run, on this code, against the
# current base, and that its verdicts may be merged. Run by CI's Performance-receipt job; it needs only
# CMake and git (with the base branch and the pull request's history fetched).
#
#   cmake -DBASE_REF=origin/dev -DHEAD_REF=<the pull request's head commit> -P Performance/CheckReceipt.cmake
#
# A pull request that changes none of what a comparison measures (see Receipt.cmake: the library and the
# probes) needs no receipt: it passes, whatever receipt it inherited. Otherwise Performance/receipt.md at
# HEAD_REF must have a host section, and every section it has (host, and cuda or hip when a device
# comparison was recorded) must
#
#   * have measured this code:     its head fingerprint is HEAD_REF's;
#   * have measured today's base:  its base fingerprint is BASE_REF's, so a base whose code moved on
#                                  since needs a rebase and a new comparison;
#   * be green, or be accepted:    a yellow or red verdict passes only with a reason on its
#                                  "**Accepted:**" line, under the section's heading; on GitHub, the
#                                  check then also warns, so a pass on an accepted regression shows.
#
# Who accepted it is in git history: the commit that wrote the reason.
#
# Anything else is stale or unaccepted, and fails: an old receipt never stands for new code. SUMMARY, if
# set, is a file the result is appended to as Markdown (CI's job summary).

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

# The receipt, as the pull request committed it.
execute_process(COMMAND git show ${HEAD_REF}:${PERF_RECEIPT_FILE} WORKING_DIRECTORY "${REPO}" OUTPUT_VARIABLE receipt
                RESULT_VARIABLE rc ERROR_QUIET)
perf_receipt_section("${receipt}" host host_section)

if(NOT rc EQUAL 0 OR host_section STREQUAL "")
  say("**No performance receipt.** This pull request changes the library or the probes, and")
  say("${PERF_RECEIPT_FILE} has no host comparison. Run the comparison locally, on a quiet machine, and")
  say("commit the receipt it writes:")
  say("")
  say("    ${rerun}")
  finish(FALSE)
endif()

set(passed TRUE)

# Check every section present: the host one, and a device one when it was recorded.
foreach(side IN LISTS PERF_RECEIPT_SIDES)
  perf_receipt_section("${receipt}" ${side} section)

  if(section STREQUAL "")
    continue()
  endif()

  foreach(field version verdict base base-fingerprint head head-fingerprint date)
    perf_receipt_field("${section}" ${field} r_${field})
  endforeach()

  perf_receipt_accepted("${section}" r_accepted)

  if(side STREQUAL "host")
    set(again "${rerun}")
  else()
    set(again "${rerun} --gpu ${side}")
  endif()

  say("**${side}**: `${r_head}` against `${r_base}`, measured ${r_date}: **${r_verdict}**.")

  if(NOT r_version STREQUAL PERF_RECEIPT_VERSION)
    say("- **Unreadable**: section version '${r_version}', expected ${PERF_RECEIPT_VERSION}. Record it again: `${again}`")
    set(passed FALSE)
    continue()
  endif()

  set(fresh TRUE)

  if(NOT r_head-fingerprint STREQUAL head_now)
    say("- **Stale**: it measured other code (`${r_head}`); the library or the probes changed since. A")
    say("  receipt stands only for the code it measured. Record it again and commit it: `${again}`")
    set(fresh FALSE)
  endif()

  if(NOT r_base-fingerprint STREQUAL base_now)
    say("- **Stale base**: it compared against `${r_base}`, and ${BASE_REF}'s code has changed since. Rebase")
    say("  on ${BASE_REF}, then record it again and commit it: `${again}`")
    set(fresh FALSE)
  endif()

  if(NOT fresh)
    set(passed FALSE)
  endif()

  if(r_verdict STREQUAL "red" OR r_verdict STREQUAL "yellow")
    if(r_verdict STREQUAL "red")
      set(what "Red verdict (a major regression, answers that differ, or a case that no longer runs)")
    else()
      set(what "Yellow verdict (a minor regression, or a changed checksum)")
    endif()

    if(r_accepted STREQUAL "")
      say("- **${what}**: fix it and record again, or, if it is accepted, replace the \"-\" after")
      say("  \"Accepted:\" under this section's heading with the reason.")
      set(passed FALSE)
    else()
      say("- ${what}, **accepted**: ${r_accepted}")

      # A pass on an accepted regression shows as a warning on the check, not as a plain pass.
      if(DEFINED ENV{GITHUB_ACTIONS})
        execute_process(COMMAND ${CMAKE_COMMAND} -E echo
                                "::warning title=Performance receipt (${side})::${r_verdict} verdict, accepted: ${r_accepted}")
      endif()
    endif()
  elseif(NOT r_verdict STREQUAL "green")
    say("- **Unreadable** verdict '${r_verdict}'. Record it again: `${again}`")
    set(passed FALSE)
  endif()

  if(fresh)
    say("- It measured this code, against ${BASE_REF} as it is now.")
  endif()
endforeach()

finish(passed)
