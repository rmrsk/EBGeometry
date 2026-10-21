# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
#
# SPDX-License-Identifier: GPL-3.0-or-later

# Compare the performance of two commits: a baseline (BASE) and a candidate (HEAD).
#
#   cmake [-D<VAR>=<value> ...] -P Performance/Compare.cmake
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
#   SIDE       host, device or all                                default: host (all when GPU is set)
#   GPU        OFF, CUDA or HIP                                   default: OFF
#   GPU_ARCH   CMAKE_CUDA_ARCHITECTURES / CMAKE_HIP_ARCHITECTURES  default: the toolchain's
#   CMAKE_CUDA_COMPILER / CMAKE_HIP_COMPILER  the device compiler   default: the one on the PATH
#   CASES      glob of case names to run                          default: *
#   ROUNDS     runs of each case per side                         default: 3
#   REPS       timed repetitions inside each run                  default: 5
#   CORE       core to pin to with taskset; OFF to not pin        default: 2
#   MINOR      percent slower that asks for a look                default: 3
#   MAJOR      percent slower that is a red flag                  default: 10
#   WORK_DIR   where the trees, builds and results go             default: build/perf-compare
#   FAIL_ON_MAJOR  exit non-zero on a major regression            default: ON

cmake_minimum_required(VERSION 3.16)

get_filename_component(REPO "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

foreach(pair "BASE;origin/dev" "HEAD;HEAD" "GPU;OFF" "CASES;*" "ROUNDS;3" "REPS;5" "CORE;2" "MINOR;3" "MAJOR;10"
             "WORK_DIR;${REPO}/build/perf-compare" "FAIL_ON_MAJOR;ON")
  list(GET pair 0 var)
  list(GET pair 1 default)

  if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
    set(${var} "${default}")
  endif()
endforeach()

if(NOT DEFINED SIDE)
  if(GPU STREQUAL "OFF")
    set(SIDE host)
  else()
    set(SIDE all)
  endif()
endif()

get_filename_component(WORK_DIR "${WORK_DIR}" ABSOLUTE BASE_DIR "${REPO}")

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

# What changed in the probes themselves, so a reader knows when the measurement changed too.
execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files --ignore-eol "${base_src}/Performance/Common/PerfHarness.hpp"
                        "${head_src}/Performance/Common/PerfHarness.hpp" RESULT_VARIABLE harness_differs)
file(GLOB head_probes RELATIVE "${head_src}/Performance" "${head_src}/Performance/*/main.cpp")
list(REMOVE_ITEM head_probes Compare/main.cpp)
set(changed_probes "")

foreach(p IN LISTS head_probes)
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

if(NOT harness_differs EQUAL 0)
  list(APPEND changed_probes "Performance/Common/PerfHarness.hpp")
endif()

# ── Run ───────────────────────────────────────────────────────────────────────────────────────────────
find_program(TASKSET taskset)
set(pin "")

if(TASKSET AND NOT CORE STREQUAL "OFF")
  set(pin ${TASKSET} -c ${CORE})
endif()

set(jobs "")

foreach(p IN LISTS head_probes)
  get_filename_component(probe "${p}" DIRECTORY)
  execute_process(COMMAND "${WORK_DIR}/head-build/bin/${probe}.ex" --list --side ${SIDE} --case "${CASES}"
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
  message(FATAL_ERROR "Compare.cmake: no case matches SIDE=${SIDE} CASES=${CASES}")
endif()

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

      execute_process(COMMAND ${pin} "${exe}" --case "${name}" --side all --reps ${REPS} --json
                      OUTPUT_VARIABLE out RESULT_VARIABLE rc)

      if(NOT rc EQUAL 0)
        message(WARNING "Compare.cmake: ${side} ${probe} --case ${name} exited with ${rc}")
      endif()

      file(APPEND "${WORK_DIR}/${side}.jsonl" "${out}")
    endforeach()
  endforeach()
endforeach()

# ── Report ────────────────────────────────────────────────────────────────────────────────────────────
execute_process(
  COMMAND "${WORK_DIR}/head-build/bin/PerfCompare.ex" --base "${WORK_DIR}/base.jsonl" --head "${WORK_DIR}/head.jsonl"
          --minor ${MINOR} --major ${MAJOR} --markdown "${WORK_DIR}/report.md"
  RESULT_VARIABLE verdict)

set(notes "")

if(borrowed)
  string(APPEND notes "\n${borrowed}\n")
endif()

if(changed_probes)
  string(REPLACE ";" ", " changed "${changed_probes}")
  string(APPEND notes "\nThe probes differ between the sides (each side ran its own): ${changed}\n")
endif()

set(gpu_manual "")

if(NOT GPU STREQUAL "OFF")
  if(DEFINED GPU_ARCH)
    string(APPEND gpu_manual " -DGPU_ARCH=${GPU_ARCH}")
  endif()

  if(DEFINED CMAKE_${GPU}_COMPILER)
    string(APPEND gpu_manual " -DCMAKE_${GPU}_COMPILER=${CMAKE_${GPU}_COMPILER}")
  endif()
endif()

if(pin)
  set(pin_text "pinned to core ${CORE} with taskset")
else()
  set(pin_text "not pinned")
endif()

set(manual "
Run it manually
---------------
Whole comparison (${ROUNDS} rounds x ${REPS} repetitions, ${pin_text}):

    cmake -DBASE=${BASE} -DHEAD=${HEAD} -DSIDE=${SIDE} -DGPU=${GPU} -DCASES='${CASES}' -DROUNDS=${ROUNDS} -DREPS=${REPS} -DWORK_DIR=${WORK_DIR}${gpu_manual} -P Performance/Compare.cmake

One case, both sides, by hand (the builds are kept under ${WORK_DIR}):

    ${WORK_DIR}/base-build/bin/<Probe>.ex --case '<case name>' --reps 10
    ${WORK_DIR}/head-build/bin/<Probe>.ex --case '<case name>' --reps 10

A probe on its own, in this tree:

    cmake --preset perf && cmake --build --preset perf
    build/perf/Performance/bin/<Probe>.ex --list                  # the cases
    build/perf/Performance/bin/<Probe>.ex --case 'PointCloudBVH/*' --side host

Results: ${WORK_DIR}/base.jsonl and head.jsonl; this report: ${WORK_DIR}/report.md
")

message("${notes}${manual}")
file(APPEND "${WORK_DIR}/report.md" "${notes}\n<details><summary>Run it manually</summary>\n\n```${manual}```\n</details>\n")

if(verdict EQUAL 2 AND FAIL_ON_MAJOR)
  message(FATAL_ERROR "Compare.cmake: at least one MAJOR performance regression (see the table above)")
endif()
