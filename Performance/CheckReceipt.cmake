# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
#
# SPDX-License-Identifier: BSD-3-Clause

# Check a pull request's performance receipt: that the comparison was run, on this code, against the base
# it is built on, and that its verdicts may be merged. Run by CI's Performance-receipt job; it needs only
# CMake and git (with the base branch and the pull request's history fetched).
#
#   cmake -DBASE_REF=origin/main -DHEAD_REF=<the pull request's head commit> -P Performance/CheckReceipt.cmake
#
# A pull request that changes none of what a comparison measures (see Receipt.cmake: the library and the
# probes) needs no receipt: it passes, whatever receipts it inherited. Otherwise it must add or change exactly
# one file under Performance/receipts/ (its branch's receipt, written by --record; the earlier receipts it
# deletes do not count), which must have a host section, and every section it has (host, and cuda or hip when
# a device comparison was recorded) must
#
#   * have measured this code:     its head fingerprint is HEAD_REF's;
#   * have measured its own base:  its base fingerprint is that of the merge-base of HEAD_REF and
#                                  BASE_REF, the base the pull request's code is built on. A pull request
#                                  keeps its receipt while BASE_REF moves on without it; a rebase changes
#                                  its base and needs a new comparison. Merges that land on BASE_REF
#                                  meanwhile are not measured together by any one receipt; CLAUDE.md
#                                  says when that combination is compared;
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

# What the receipt must match: this code, and the base it is built on (the fork point from BASE_REF).
execute_process(COMMAND git merge-base ${BASE_REF} ${HEAD_REF} WORKING_DIRECTORY "${REPO}" OUTPUT_VARIABLE fork
                OUTPUT_STRIP_TRAILING_WHITESPACE RESULT_VARIABLE rc ERROR_VARIABLE err)

if(NOT rc EQUAL 0)
  message(FATAL_ERROR "CheckReceipt.cmake: no common history of ${BASE_REF} and ${HEAD_REF} (fetch both in "
                      "full): ${err}")
endif()

perf_fingerprint(${HEAD_REF} head_now)
perf_fingerprint(${fork} fork_now)

if(head_now STREQUAL fork_now)
  say("No change to the library or the probes against ${BASE_REF}: no performance receipt is required.")
  finish(TRUE)
endif()

# The receipt: the one file under the receipts directory that the pull request adds or changes, as committed.
execute_process(COMMAND git diff --name-only --diff-filter=AMR ${fork} ${HEAD_REF} -- ${PERF_RECEIPT_DIR}
                WORKING_DIRECTORY "${REPO}" OUTPUT_VARIABLE receipt_files OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REPLACE "\n" ";" receipt_files "${receipt_files}")
list(LENGTH receipt_files nreceipts)

if(nreceipts GREATER 1)
  string(REPLACE ";" ", " listed "${receipt_files}")
  say("**More than one receipt.** This pull request changes ${listed}; a pull request carries one receipt,")
  say("its branch's. Keep that one and restore the others.")
  finish(FALSE)
endif()

set(receipt "")
set(rc 1)

if(nreceipts EQUAL 1)
  execute_process(COMMAND git show ${HEAD_REF}:${receipt_files} WORKING_DIRECTORY "${REPO}" OUTPUT_VARIABLE receipt
                  RESULT_VARIABLE rc ERROR_QUIET)
endif()

perf_receipt_section("${receipt}" host host_section)

if(NOT rc EQUAL 0 OR host_section STREQUAL "")
  say("**No performance receipt.** This pull request changes the library or the probes, and adds no receipt")
  say("with a host comparison under ${PERF_RECEIPT_DIR}/. Run the comparison locally, on a quiet machine, on")
  say("the pull request's branch, and commit the receipt it writes:")
  say("")
  say("    ${rerun}")
  finish(FALSE)
endif()

say("Receipt: `${receipt_files}`.")

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

  if(NOT r_base-fingerprint STREQUAL fork_now)
    string(SUBSTRING "${fork}" 0 7 fork_short)
    say("- **Other base**: it compared against `${r_base}`, while this pull request is built on `${fork_short}`")
    say("  (it was rebased, or the receipt measured a newer ${BASE_REF}). Rebase on ${BASE_REF} if needed, then")
    say("  record it again and commit it: `${again}`")
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
        # A workflow command's message escapes %, CR and LF; anything else is taken literally.
        set(note "${r_verdict} verdict, accepted: ${r_accepted}")
        string(REPLACE "%" "%25" note "${note}")
        string(REPLACE "\r" "%0D" note "${note}")
        string(REPLACE "\n" "%0A" note "${note}")
        execute_process(COMMAND ${CMAKE_COMMAND} -E echo "::warning title=Performance receipt (${side})::${note}")
      endif()
    endif()
  elseif(NOT r_verdict STREQUAL "green")
    say("- **Unreadable** verdict '${r_verdict}'. Record it again: `${again}`")
    set(passed FALSE)
  endif()

  if(fresh)
    say("- It measured this code, against the base it is built on.")
  endif()
endforeach()

finish(passed)
