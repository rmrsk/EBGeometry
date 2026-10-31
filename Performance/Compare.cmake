# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
#
# SPDX-License-Identifier: GPL-3.0-or-later

# Compare the performance of two commits: a baseline (BASE) and a candidate (HEAD).
#
#   Scripts/run-performance.sh compare [BASE [HEAD]] [GLOB...] [options]     (the usual way)
#   cmake [-D<VAR>=<value> ...] -P Performance/Compare.cmake                 (what it runs)
#
# Needs only CMake, git and a C++ compiler (nvcc or hipcc for GPU=CUDA/HIP). Each side is exported with
# git archive (no worktree, no new refs) and built from its own Performance/ folder in Release, for the
# host's full instruction set; a baseline from before Performance/ existed borrows the candidate's.
# Every case then runs in a fresh process, alternating between the sides for ROUNDS rounds (the order
# swaps each round), pinned to one core when taskset is available. PerfCompare reports the result, which
# is printed and written to <WORK_DIR>/report.md together with the commands to repeat it by hand.
#
# Variables (all optional):
#   BASE       baseline commit or branch                         default: origin/dev
#   HEAD       candidate commit or branch; WORKTREE uses the     default: HEAD
#              working tree as it is, uncommitted changes included
#   GPU        OFF, CUDA or HIP                                   default: OFF
#   GPU_ARCH   CMAKE_CUDA_ARCHITECTURES / CMAKE_HIP_ARCHITECTURES  default: the toolchain's
#   CMAKE_CUDA_COMPILER / CMAKE_HIP_COMPILER  the device compiler   default: the one on the PATH
#   CASES      glob(s) of case names to run, a CMake list         default: *
#   ROUNDS     runs of each case per side; 1 shows the times      default: 2
#              without judging them (not with RECORD)
#   REPS       timed repetitions inside each run                  default: 3
#   CORE       core to pin to with taskset; OFF to not pin        default: 2 (with GPU: OFF, and only OFF)
#   MINOR      percent slower that asks for a look                default: 3
#   MAJOR      percent slower that is a red flag                  default: 10
#   SMOKE      ON: a quick check, not a measurement: one round of one repetition at the --smoke
#              sizes; reports whether every case ran on both sides, the checksums and the host/device
#              agreement, and no timing verdicts                   default: OFF
#   WORK_DIR   where the trees, builds and results go             default: build/performance/compare
#   FAIL_ON_MAJOR  exit non-zero on a major regression            default: ON
#   RECORD     ON: write the result as this comparison's section of     default: OFF
#              the performance receipt, Performance/receipt.md (see Receipt.cmake), to commit with the pull
#              request; CI checks it. Only for a full comparison of committed code: not with SMOKE or
#              CASES, and not while the measured paths have uncommitted changes.

cmake_minimum_required(VERSION 3.16)

get_filename_component(REPO "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

foreach(pair "BASE;origin/dev" "HEAD;HEAD" "GPU;OFF" "CASES;*" "ROUNDS;2" "REPS;3" "MINOR;3" "MAJOR;10" "SMOKE;OFF"
             "WORK_DIR;${REPO}/build/performance/compare" "FAIL_ON_MAJOR;ON" "RECORD;OFF")
  list(GET pair 0 var)
  list(GET pair 1 default)

  if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
    set(${var} "${default}")
  endif()
endforeach()

# A host comparison is pinned to one core; a GPU comparison's host side uses them all, so pinning it
# would measure the device against one thread (OpenMP sees only the cores it may run on).
if(NOT GPU STREQUAL "OFF" AND DEFINED CORE AND NOT "${CORE}" STREQUAL "" AND NOT CORE STREQUAL "OFF")
  message(FATAL_ERROR "Compare.cmake: CORE cannot be set with GPU: the host side of a GPU comparison uses every "
                      "hardware thread but one, and pinned to one core it would run on one thread")
endif()

if(NOT DEFINED CORE OR "${CORE}" STREQUAL "")
  if(GPU STREQUAL "OFF")
    set(CORE 2)
  else()
    set(CORE OFF)
  endif()
endif()

if(SMOKE)
  set(ROUNDS 1)
  set(REPS 1)
elseif(ROUNDS LESS 1)
  message(FATAL_ERROR "Compare.cmake: ROUNDS must be at least 1 (got ${ROUNDS})")
elseif(ROUNDS LESS 2 AND RECORD)
  # PerfCompare judges a case by its runs (fresh processes), and one run per side cannot tell a change
  # from the noise between processes: such a comparison shows times, and cannot stand as a receipt.
  message(FATAL_ERROR "Compare.cmake: a receipt (RECORD) needs at least 2 rounds (got ROUNDS=${ROUNDS})")
endif()

get_filename_component(WORK_DIR "${WORK_DIR}" ABSOLUTE BASE_DIR "${REPO}")

include("${CMAKE_CURRENT_LIST_DIR}/Receipt.cmake")

# A receipt stands for the whole suite on committed code, so it is refused before anything is built for
# a run that could not be one.
if(RECORD)
  if(SMOKE)
    message(FATAL_ERROR "Compare.cmake: a receipt (RECORD) needs a full comparison, not a smoke check")
  endif()

  if(NOT CASES STREQUAL "*")
    message(FATAL_ERROR "Compare.cmake: a receipt (RECORD) needs every case, not CASES=${CASES}")
  endif()

  # The mesh cases read armadillo.obj from the submodule; without it they would be skipped on both
  # sides, and the receipt would cover less than it claims (checked again, for every case, after the run).
  if(NOT EXISTS "${REPO}/common-3d-test-models/data/armadillo.obj")
    message(FATAL_ERROR "Compare.cmake: a receipt (RECORD) needs the mesh cases, and common-3d-test-models is "
                        "not checked out in ${REPO}: run git submodule update --init")
  endif()

  if(HEAD STREQUAL "WORKTREE")
    perf_measured_changes(changes)

    if(changes)
      message(FATAL_ERROR "Compare.cmake: a receipt (RECORD) measures committed code; commit or stash these "
                          "first:\n${changes}")
    endif()
  endif()
endif()

# Run a command with its output captured (compiler warnings would otherwise bury the report); show
# the output only if it fails.
function(run_or_die)
  execute_process(${ARGN} RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)

  if(NOT rc EQUAL 0)
    string(REPLACE ";" " " cmd "${ARGN}")
    message(FATAL_ERROR "Compare.cmake: failed (${rc}): ${cmd}\n${out}\n${err}")
  endif()
endfunction()

# ── Export and build each side ────────────────────────────────────────────────────────────────────────
file(REMOVE_RECURSE "${WORK_DIR}/base" "${WORK_DIR}/head" "${WORK_DIR}/base-build" "${WORK_DIR}/head-build")
file(MAKE_DIRECTORY "${WORK_DIR}")

set(borrowed "")

foreach(side base head)
  string(TOUPPER ${side} SIDE_VAR)
  set(ref "${${SIDE_VAR}}")

  if(side STREQUAL "head" AND ref STREQUAL "WORKTREE")
    set(src "${REPO}")
    execute_process(COMMAND git rev-parse --short HEAD WORKING_DIRECTORY "${REPO}" OUTPUT_VARIABLE sha
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(sha "${sha}+worktree")
  else()
    set(src "${WORK_DIR}/${side}")
    execute_process(COMMAND git rev-parse --short "${ref}" WORKING_DIRECTORY "${REPO}" OUTPUT_VARIABLE sha
                    OUTPUT_STRIP_TRAILING_WHITESPACE RESULT_VARIABLE rc)

    if(NOT rc EQUAL 0)
      message(FATAL_ERROR "Compare.cmake: '${ref}' is not a commit in ${REPO}")
    endif()

    file(MAKE_DIRECTORY "${src}")
    run_or_die(COMMAND git archive --format=tar -o "${WORK_DIR}/${side}.tar" "${ref}" WORKING_DIRECTORY "${REPO}")
    run_or_die(COMMAND ${CMAKE_COMMAND} -E tar xf "${WORK_DIR}/${side}.tar" WORKING_DIRECTORY "${src}")
    file(REMOVE "${WORK_DIR}/${side}.tar")
  endif()

  set(${side}_src "${src}")
  set(${side}_sha "${sha}")
endforeach()

# A receipt's commits and fingerprints. A working tree recorded from has no uncommitted changes in what is
# measured (checked above), so it measures its HEAD commit, and says so.
if(RECORD)
  if(HEAD STREQUAL "WORKTREE")
    set(head_commit HEAD)
    string(REPLACE "+worktree" "" head_sha "${head_sha}")
  else()
    set(head_commit "${HEAD}")
  endif()

  execute_process(COMMAND git rev-parse "${BASE}^{commit}" WORKING_DIRECTORY "${REPO}" OUTPUT_VARIABLE base_full
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND git rev-parse "${head_commit}^{commit}" WORKING_DIRECTORY "${REPO}"
                  OUTPUT_VARIABLE head_full OUTPUT_STRIP_TRAILING_WHITESPACE)
  perf_fingerprint(${base_full} base_fingerprint)
  perf_fingerprint(${head_full} head_fingerprint)
endif()

# A baseline from before Performance/ existed is measured with the candidate's probes.
if(NOT EXISTS "${base_src}/Performance/CMakeLists.txt")
  file(COPY "${head_src}/Performance" DESTINATION "${base_src}")
  set(borrowed "The baseline has no Performance/ folder; it was measured with the candidate's probes.")
endif()

set(gpu_args -DEBGEOMETRY_PERF_GPU=${GPU})

if(DEFINED GPU_ARCH AND NOT GPU STREQUAL "OFF")
  list(APPEND gpu_args -DCMAKE_${GPU}_ARCHITECTURES=${GPU_ARCH})
endif()

# The device compiler, when it is not the one on the PATH (CMAKE_CUDA_COMPILER / CMAKE_HIP_COMPILER).
if(NOT GPU STREQUAL "OFF" AND DEFINED CMAKE_${GPU}_COMPILER)
  list(APPEND gpu_args -DCMAKE_${GPU}_COMPILER=${CMAKE_${GPU}_COMPILER})
endif()

foreach(side base head)
  message(STATUS "Building ${side} (${${side}_sha}) ...")
  run_or_die(
    COMMAND ${CMAKE_COMMAND} -S "${${side}_src}/Performance" -B "${WORK_DIR}/${side}-build" -DCMAKE_BUILD_TYPE=Release
            -DEBGEOMETRY_PERF_GIT_SHA=${${side}_sha} -DEBGEOMETRY_PERF_MESH_DIR=${REPO}/common-3d-test-models/data
            -DEBGEOMETRY_PERF_SMOKE_MESH_DIR=${${side}_src}/Tests/data ${gpu_args}
  )
  run_or_die(COMMAND ${CMAKE_COMMAND} --build "${WORK_DIR}/${side}-build" --parallel)
endforeach()

# What changed in the probes themselves (each probe, and the harness in Common/), so that a reader knows
# when the measurement changed too.
file(GLOB head_probes RELATIVE "${head_src}/Performance" "${head_src}/Performance/*/main.cpp")
file(GLOB head_harness RELATIVE "${head_src}/Performance" "${head_src}/Performance/Common/*")
set(changed_probes "")

foreach(p IN LISTS head_probes head_harness)
  if(EXISTS "${base_src}/Performance/${p}")
    execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files --ignore-eol "${base_src}/Performance/${p}"
                            "${head_src}/Performance/${p}" RESULT_VARIABLE differs)
  else()
    set(differs 1)
  endif()

  if(NOT differs EQUAL 0)
    list(APPEND changed_probes "Performance/${p}")
  endif()
endforeach()

# ── Run ───────────────────────────────────────────────────────────────────────────────────────────────
find_program(TASKSET taskset)
set(pin "")

if(TASKSET AND NOT CORE STREQUAL "OFF")
  set(pin ${TASKSET} -c ${CORE})
endif()

set(jobs "")
set(case_args "")

# CASES may hold several globs (a CMake list).
foreach(glob IN LISTS CASES)
  list(APPEND case_args --case "${glob}")
endforeach()

foreach(p IN LISTS head_probes)
  get_filename_component(probe "${p}" DIRECTORY)
  execute_process(COMMAND "${WORK_DIR}/head-build/bin/${probe}.ex" --list ${case_args}
                  OUTPUT_VARIABLE names OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REPLACE "\n" ";" names "${names}")

  foreach(case_name IN LISTS names)
    if(NOT case_name STREQUAL "")
      list(APPEND jobs "${probe}|${case_name}")
    endif()
  endforeach()
endforeach()

list(LENGTH jobs njobs)

if(njobs EQUAL 0)
  message(FATAL_ERROR "Compare.cmake: no case matches CASES=${CASES}")
endif()

set(smoke_arg "")

if(SMOKE)
  set(smoke_arg --smoke)
endif()

# Runs that crashed or exited with an error, reported with the comparison.
set(failed "")

file(WRITE "${WORK_DIR}/base.jsonl" "")
file(WRITE "${WORK_DIR}/head.jsonl" "")

foreach(round RANGE 1 ${ROUNDS})
  message(STATUS "Round ${round}/${ROUNDS}: ${njobs} cases per side")

  math(EXPR odd "${round} % 2")

  if(odd)
    set(order base head)
  else()
    set(order head base)
  endif()

  foreach(job IN LISTS jobs)
    string(REPLACE "|" ";" job "${job}")
    list(GET job 0 probe)
    list(GET job 1 name)

    foreach(side IN LISTS order)
      set(exe "${WORK_DIR}/${side}-build/bin/${probe}.ex")

      if(NOT EXISTS "${exe}")
        continue()
      endif()

      execute_process(COMMAND ${pin} "${exe}" --case "${name}" --reps ${REPS} --json ${smoke_arg}
                      OUTPUT_VARIABLE out RESULT_VARIABLE rc)

      if(NOT rc EQUAL 0)
        message(WARNING "Compare.cmake: ${side} ${probe} --case ${name} exited with ${rc}")
        list(APPEND failed "${side}: ${name} (exit ${rc})")
      endif()

      file(APPEND "${WORK_DIR}/${side}.jsonl" "${out}")
    endforeach()
  endforeach()
endforeach()

list(LENGTH failed nfailed)

# ── Report ────────────────────────────────────────────────────────────────────────────────────────────
execute_process(
  COMMAND "${WORK_DIR}/head-build/bin/PerfCompare.ex" --base "${WORK_DIR}/base.jsonl" --head "${WORK_DIR}/head.jsonl"
          --minor ${MINOR} --major ${MAJOR} --markdown "${WORK_DIR}/report.md" ${smoke_arg} --no-verdict
          --failed-runs ${nfailed} --level-file "${WORK_DIR}/level.txt"
  RESULT_VARIABLE verdict)

# The report as PerfCompare wrote it, before the commands to repeat it (local paths) are added: a receipt
# carries this.
set(report_text "")

if(EXISTS "${WORK_DIR}/report.md")
  file(READ "${WORK_DIR}/report.md" report_text)
endif()

set(notes "")

if(borrowed)
  string(APPEND notes "\n${borrowed}\n")
endif()

if(failed)
  string(REPLACE ";" "\n  " failed_text "${failed}")
  string(APPEND notes "\nThese runs failed (crashed or exited with an error):\n  ${failed_text}\n")
endif()

if(changed_probes)
  string(REPLACE ";" ", " changed "${changed_probes}")
  string(APPEND notes "\nThe probes differ between the sides (each side ran its own): ${changed}\n")
endif()

if(pin)
  set(pin_text "pinned to core ${CORE} with taskset")
else()
  set(pin_text "not pinned")
endif()

set(rounds_s "s")
set(reps_s "s")

if(ROUNDS EQUAL 1)
  set(rounds_s "")
endif()

if(REPS EQUAL 1)
  set(reps_s "")
endif()

if(SMOKE)
  set(what_text "a smoke check: smoke sizes, one repetition")
else()
  set(what_text "${ROUNDS} round${rounds_s} x ${REPS} repetition${reps_s}")
endif()

# The same comparison as one command of the front door, Scripts/run-performance.sh.
set(again "Scripts/run-performance.sh compare ${BASE}")

if(NOT HEAD STREQUAL "WORKTREE")
  string(APPEND again " ${HEAD}")
endif()

foreach(glob IN LISTS CASES)
  if(NOT glob STREQUAL "*")
    string(APPEND again " '${glob}'")
  endif()
endforeach()

if(NOT GPU STREQUAL "OFF")
  string(TOLOWER ${GPU} gpu_lower)
  string(APPEND again " --gpu ${gpu_lower}")

  if(DEFINED GPU_ARCH)
    string(APPEND again " --gpu-arch ${GPU_ARCH}")
  endif()

  if(DEFINED CMAKE_${GPU}_COMPILER)
    string(APPEND again " --${gpu_lower}-compiler ${CMAKE_${GPU}_COMPILER}")
  endif()
endif()

if(RECORD)
  string(APPEND again " --record")
endif()

if(SMOKE)
  string(APPEND again " --smoke")
else()
  if(NOT ROUNDS EQUAL 2)
    string(APPEND again " --rounds ${ROUNDS}")
  endif()

  if(NOT REPS EQUAL 3)
    string(APPEND again " --reps ${REPS}")
  endif()
endif()

if(CORE STREQUAL "OFF" AND GPU STREQUAL "OFF")
  string(APPEND again " --no-pin")
elseif(NOT CORE STREQUAL "OFF" AND (NOT CORE EQUAL 2 OR NOT GPU STREQUAL "OFF"))
  string(APPEND again " --core ${CORE}")
endif()

set(manual "
Run it manually
---------------
The whole comparison again (${what_text}, ${pin_text}):

    ${again}

One case, both sides, by hand (the builds are kept under ${WORK_DIR}):

    ${WORK_DIR}/base-build/bin/<Probe>.ex --case '<case name>' --reps 10
    ${WORK_DIR}/head-build/bin/<Probe>.ex --case '<case name>' --reps 10

The cases in this tree, and a run of some of them:

    Scripts/run-performance.sh list
    Scripts/run-performance.sh 'PointCloudBVH/*'

Results: ${WORK_DIR}/base.jsonl and head.jsonl; this report: ${WORK_DIR}/report.md
")

message("${notes}${manual}")
file(APPEND "${WORK_DIR}/report.md" "${notes}\n<details><summary>Run it manually</summary>\n\n```${manual}```\n</details>\n")

# The verdict, last, so it is the line a reader sees when the run ends.
execute_process(
  COMMAND "${WORK_DIR}/head-build/bin/PerfCompare.ex" --base "${WORK_DIR}/base.jsonl" --head "${WORK_DIR}/head.jsonl"
          --minor ${MINOR} --major ${MAJOR} ${smoke_arg} --verdict-only --failed-runs ${nfailed}
  RESULT_VARIABLE verdict_again)

# PerfCompare exits 0 (green or yellow) or 2 (red). Anything else means there is no verdict (3: it could
# not judge, a usage error or an unreadable file; a signal's name: it crashed), which must never pass.
# Both calls judge the same results, so they must agree.
foreach(code IN ITEMS "${verdict}" "${verdict_again}")
  if(NOT code MATCHES "^(0|2)$")
    message(FATAL_ERROR "Compare.cmake: PerfCompare could not judge the results (exit '${code}'); see its message above")
  endif()
endforeach()

if(NOT verdict STREQUAL verdict_again)
  message(FATAL_ERROR "Compare.cmake: PerfCompare's report and verdict calls disagree (${verdict}, ${verdict_again})")
endif()

# The receipt (RECORD): this comparison's section of Performance/receipt.md (Receipt.cmake), for
# CheckReceipt.cmake and readers. Every verdict is recorded; only a green one passes CI on its own, a
# yellow or red one once a reason replaces the "-" on its "**Accepted:**" line.
if(RECORD)
  # A receipt stands for every case: a case skipped on either side (a missing input, a case that threw)
  # leaves it covering less than it claims, so none is written.
  perf_skipped_cases("${WORK_DIR}" skipped)

  if(skipped)
    string(REPLACE ";" "\n  " skipped_text "${skipped}")
    message(FATAL_ERROR "Compare.cmake: no receipt written: these cases were skipped, so it would not cover "
                        "every case:\n  ${skipped_text}")
  endif()

  file(STRINGS "${WORK_DIR}/level.txt" level LIMIT_COUNT 1)

  if(GPU STREQUAL "OFF")
    set(receipt_side host)
  else()
    string(TOLOWER ${GPU} receipt_side)
  endif()

  perf_write_receipt(
    SIDE ${receipt_side} LEVEL ${level} BASE_REF ${BASE} BASE ${base_sha} HEAD ${head_sha}
    BASE_FINGERPRINT ${base_fingerprint} HEAD_FINGERPRINT ${head_fingerprint} CASES ${njobs} ROUNDS ${ROUNDS}
    REPS ${REPS} REPORT "${report_text}\n${notes}")
endif()

if(SMOKE)
  set(failing OFF)

  if(verdict EQUAL 2 OR failed)
    set(failing ON)
  endif()
else()
  set(failing OFF)

  if((verdict EQUAL 2 OR failed) AND FAIL_ON_MAJOR)
    set(failing ON)
  endif()
endif()

# Through Scripts/run-performance.sh the exit status goes to a file the script exits with, so that no
# CMake error block follows the verdict; run on its own, a failing comparison is a CMake error.
if(DEFINED PERF_STATUS_FILE)
  if(failing)
    file(WRITE "${PERF_STATUS_FILE}" "2")
  else()
    file(WRITE "${PERF_STATUS_FILE}" "0")
  endif()
elseif(failing)
  message(FATAL_ERROR "Compare.cmake: the verdict above is red")
endif()
