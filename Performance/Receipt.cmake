# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
#
# SPDX-License-Identifier: GPL-3.0-or-later

# What a performance receipt is measured against, shared by Compare.cmake (which writes a receipt with
# RECORD) and CheckReceipt.cmake (which CI runs on a pull request).
#
# A receipt, Performance/Receipt/<host|cuda|hip>.md, is the result of a full local comparison, committed
# with the pull request. It names the code it measured by two fingerprints: the candidate's and the
# baseline's. A fingerprint is a hash of git's tree entries for what a comparison measures: the library
# (Source/, EBGeometry.hpp) and the probes (Performance/, but not its README.md or the receipts), so a
# change to the docs or to CI leaves it as it is, and any change to the code or the probes changes it.
# A receipt is valid only while both fingerprints match the code: a receipt that measured other code is
# stale, never valid.
#
# Needs REPO (the repository's root) to be set by the includer.

set(PERF_RECEIPT_DIR "Performance/Receipt")
set(PERF_RECEIPT_VERSION 1)

# The paths a fingerprint covers, and those it leaves out (as git pathspecs).
set(PERF_MEASURED_PATHS Source EBGeometry.hpp Performance)
set(PERF_UNMEASURED_PATHS ":(exclude)Performance/README.md" ":(exclude)${PERF_RECEIPT_DIR}")

# perf_fingerprint(<commit> <var>): the fingerprint of what a comparison measures at a commit. (git ls-tree
# takes no exclude pathspecs, so the unmeasured paths are filtered out of its listing here.)
function(perf_fingerprint commit var)
  execute_process(COMMAND git ls-tree -r ${commit} -- ${PERF_MEASURED_PATHS} WORKING_DIRECTORY "${REPO}"
                  OUTPUT_VARIABLE entries RESULT_VARIABLE rc ERROR_VARIABLE err)

  if(NOT rc EQUAL 0)
    message(FATAL_ERROR "Receipt.cmake: cannot list ${commit}: ${err}")
  endif()

  string(REGEX REPLACE "[^\n]*\tPerformance/README[.]md\n" "" entries "${entries}")
  string(REGEX REPLACE "[^\n]*\t${PERF_RECEIPT_DIR}/[^\n]*\n" "" entries "${entries}")

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

# perf_receipt_field(<text> <key> <var>): a field of a receipt's header ("key: value" on a line of its own).
function(perf_receipt_field text key var)
  if(text MATCHES "\n${key}:[ ]*([^\n]*)")
    string(STRIP "${CMAKE_MATCH_1}" value)
  else()
    set(value "")
  endif()

  set(${var} "${value}" PARENT_SCOPE)
endfunction()
