# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
#
# SPDX-License-Identifier: GPL-3.0-or-later

# The performance receipt, shared by Compare.cmake (which writes it with RECORD) and CheckReceipt.cmake
# (which CI runs on a pull request).
#
# The receipt, Performance/receipt.md, is the result of the full local comparison against the pull
# request's base branch, committed with the pull request. It has one section per kind of comparison: the
# host comparison (required) and, when recorded, a device comparison (cuda or hip; optional). Each
# section carries its verdict and names the code it measured by two fingerprints, the candidate's and
# the baseline's. A fingerprint is a hash of git's tree entries for what a comparison measures: the
# library (Source/, EBGeometry.hpp) and the probes (Performance/, but not its README.md or the receipt),
# so a change to the docs or to CI leaves it as it is, and any change to the code or the probes changes
# it. A section is valid only while both fingerprints match the code: one that measured other code is
# stale, never valid.
#
# Needs REPO (the repository's root) to be set by the includer.

set(PERF_RECEIPT_FILE "Performance/receipt.md")
set(PERF_RECEIPT_VERSION 3)
set(PERF_RECEIPT_SIDES host cuda hip)

# The paths a fingerprint covers, and those it leaves out (as git pathspecs).
set(PERF_MEASURED_PATHS Source EBGeometry.hpp Performance)
set(PERF_UNMEASURED_PATHS ":(exclude)Performance/README.md" ":(exclude)${PERF_RECEIPT_FILE}")

# perf_fingerprint(<commit> <var>): the fingerprint of what a comparison measures at a commit. (git ls-tree
# takes no exclude pathspecs, so the unmeasured paths are filtered out of its listing here.)
function(perf_fingerprint commit var)
  execute_process(COMMAND git ls-tree -r ${commit} -- ${PERF_MEASURED_PATHS} WORKING_DIRECTORY "${REPO}"
                  OUTPUT_VARIABLE entries RESULT_VARIABLE rc ERROR_VARIABLE err)

  if(NOT rc EQUAL 0)
    message(FATAL_ERROR "Receipt.cmake: cannot list ${commit}: ${err}")
  endif()

  string(REGEX REPLACE "[^\n]*\tPerformance/README[.]md\n" "" entries "${entries}")
  string(REGEX REPLACE "[^\n]*\tPerformance/receipt[.]md\n" "" entries "${entries}")

  if(entries STREQUAL "")
    message(FATAL_ERROR "Receipt.cmake: ${commit} has none of ${PERF_MEASURED_PATHS}")
  endif()

  string(SHA1 hash "${entries}")
  set(${var} "${hash}" PARENT_SCOPE)
endfunction()

# perf_measured_changes(<var>): what the working tree changes in the measured paths (tracked or not), as
# git status prints it; empty when they are as committed.
function(perf_measured_changes var)
  execute_process(COMMAND git status --porcelain -- ${PERF_MEASURED_PATHS} ${PERF_UNMEASURED_PATHS}
                  WORKING_DIRECTORY "${REPO}" OUTPUT_VARIABLE changes OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(${var} "${changes}" PARENT_SCOPE)
endfunction()

# perf_receipt_section(<text> <side> <var>): one side's section of a receipt, from its begin marker to its
# end marker, or empty if the receipt has none.
function(perf_receipt_section text side var)
  set(begin "<!-- BEGIN receipt ${side} -->")
  set(end "<!-- END receipt ${side} -->")
  string(FIND "${text}" "${begin}" at)
  string(FIND "${text}" "${end}" stop)
  set(section "")

  if(at GREATER_EQUAL 0 AND stop GREATER at)
    string(LENGTH "${end}" end_length)
    math(EXPR length "${stop} + ${end_length} - ${at}")
    string(SUBSTRING "${text}" ${at} ${length} section)
  endif()

  set(${var} "${section}" PARENT_SCOPE)
endfunction()

# perf_receipt_field(<section> <key> <var>): a field of a section's header ("key: value" on a line of its
# own).
function(perf_receipt_field text key var)
  if(text MATCHES "\n${key}:[ ]*([^\n]*)")
    string(STRIP "${CMAKE_MATCH_1}" value)
  else()
    set(value "")
  endif()

  set(${var} "${value}" PARENT_SCOPE)
endfunction()

# perf_receipt_accepted(<section> <var>): the reason a section's yellow or red verdict was accepted, from
# its "**Accepted:**" line under the heading, where a reader of the rendered receipt sees it; empty while
# that line holds the "-" --record writes.
function(perf_receipt_accepted text var)
  set(value "")

  if(text MATCHES "\n\\*\\*Accepted:\\*\\*[ ]*([^\n]*)")
    string(STRIP "${CMAKE_MATCH_1}" value)

    if(value STREQUAL "-")
      set(value "")
    endif()
  endif()

  set(${var} "${value}" PARENT_SCOPE)
endfunction()

# perf_skipped_cases(<work_dir> <var>): the cases a comparison skipped, from its base.jsonl and head.jsonl,
# as a list of "<side>: <case> (<reason>)", each once; empty if every case ran on both sides.
function(perf_skipped_cases work_dir var)
  set(skipped "")

  foreach(side base head)
    set(lines "")

    if(EXISTS "${work_dir}/${side}.jsonl")
      file(STRINGS "${work_dir}/${side}.jsonl" lines REGEX "\"skipped\":")
    endif()

    foreach(line IN LISTS lines)
      string(REGEX MATCH "\"case\":\"([^\"]*)\"" _ "${line}")
      set(name "${CMAKE_MATCH_1}")
      string(REGEX MATCH "\"skipped\":\"([^\"]*)\"" _ "${line}")
      list(APPEND skipped "${side}: ${name} (${CMAKE_MATCH_1})")
    endforeach()
  endforeach()

  list(REMOVE_DUPLICATES skipped)
  set(${var} "${skipped}" PARENT_SCOPE)
endfunction()

# perf_write_receipt(SIDE <host|cuda|hip> LEVEL <0|1|2> BASE_REF <ref> BASE <commit> HEAD <commit>
#                    BASE_FINGERPRINT <hash> HEAD_FINGERPRINT <hash> CASES <n> ROUNDS <n> REPS <n>
#                    REPORT <markdown>)
# Write SIDE's section of the receipt in the working tree. The other sections are kept only if they
# measured the same code against the same base (both fingerprints equal), and dropped otherwise, so a
# receipt never carries a stale section.
function(perf_write_receipt)
  cmake_parse_arguments(R "" "SIDE;LEVEL;BASE_REF;BASE;HEAD;BASE_FINGERPRINT;HEAD_FINGERPRINT;CASES;ROUNDS;REPS;REPORT"
                        "" ${ARGN})

  set(verdict_names green yellow red)
  list(GET verdict_names ${R_LEVEL} verdict)
  string(TIMESTAMP year "%Y")
  string(TIMESTAMP today "%Y-%m-%d")
  string(STRIP "${R_REPORT}" report)

  if(R_SIDE STREQUAL "host")
    set(title "Host comparison")
  else()
    string(TOUPPER ${R_SIDE} upper)
    set(title "Device comparison (${upper})")
  endif()

  set(mine "<!-- BEGIN receipt ${R_SIDE} -->
<!--receipt
version: ${PERF_RECEIPT_VERSION}
side: ${R_SIDE}
verdict: ${verdict}
base: ${R_BASE}
base-fingerprint: ${R_BASE_FINGERPRINT}
head: ${R_HEAD}
head-fingerprint: ${R_HEAD_FINGERPRINT}
cases: ${R_CASES}
rounds: ${R_ROUNDS}
reps: ${R_REPS}
date: ${today}
-->

## ${title}: ${verdict}

**Accepted:** -

`${R_HEAD}` against `${R_BASE}` (${R_BASE_REF}), ${today}.

${report}

<!-- END receipt ${R_SIDE} -->")

  set(path "${REPO}/${PERF_RECEIPT_FILE}")
  set(old "")

  if(EXISTS "${path}")
    file(READ "${path}" old)
  endif()

  set(sections "")

  foreach(side IN LISTS PERF_RECEIPT_SIDES)
    if(side STREQUAL R_SIDE)
      string(APPEND sections "\n${mine}\n")
    else()
      perf_receipt_section("${old}" ${side} other)

      if(NOT other STREQUAL "")
        perf_receipt_field("${other}" base-fingerprint other_base)
        perf_receipt_field("${other}" head-fingerprint other_head)

        if(other_base STREQUAL R_BASE_FINGERPRINT AND other_head STREQUAL R_HEAD_FINGERPRINT)
          string(APPEND sections "\n${other}\n")
        else()
          message(STATUS "Dropped the receipt's ${side} section: it measured other code")
        endif()
      endif()
    endif()
  endforeach()

  file(WRITE "${path}" "<!--
SPDX-FileCopyrightText: ${year} Robert Marskar <robert.marskar@sintef.no>

SPDX-License-Identifier: GPL-3.0-or-later
-->

<!-- Written by Scripts/run-performance.sh compare --record and checked by CI (Performance/CheckReceipt.cmake).
     Commit it with the pull request. Do not edit it, except to accept a yellow or red verdict: replace the
     \"-\" after \"Accepted:\" under that section's heading with the reason; git history records who did.
     See Docs/Sphinx/source/PerformanceTesting.rst. -->

# Performance receipt
${sections}")
  message(STATUS "Wrote the ${R_SIDE} section of ${PERF_RECEIPT_FILE} (${verdict}); commit it")
endfunction()
