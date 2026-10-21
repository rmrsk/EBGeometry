# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
#
# SPDX-License-Identifier: GPL-3.0-or-later

# The front door to the performance probes; run it through Scripts/run-performance.sh, which passes its
# arguments on (cmake -P Performance/RunPerformance.cmake -- <arguments>). Needs only CMake, git and a
# C++ compiler (nvcc or hipcc for --gpu).
#
#   Scripts/run-performance.sh [run] [GLOB...] [options]          build if needed, run the cases
#   Scripts/run-performance.sh list [GLOB...] [options]           print the case names
#   Scripts/run-performance.sh compare [BASE [HEAD]] [GLOB...] [options]
#                                                                 compare two commits (Compare.cmake)
#
# With no GLOB, every case runs. A GLOB (* and ?) selects cases by name, for example
# 'PointCloudBVH/*' or '*/double/*'; several may be given. For compare, an argument that git resolves
# to a commit is BASE (default origin/dev) and then HEAD (default: the working tree, uncommitted
# changes included); anything else is a GLOB.
#
# Options:
#   --gpu cuda|hip          build the probes for a GPU: run the host/device pairs instead of the host cases
#   --gpu-arch <arch>       CMAKE_CUDA_ARCHITECTURES / CMAKE_HIP_ARCHITECTURES (e.g. 86)
#   --cuda-compiler <path>  nvcc to use (CMake also honours CUDACXX); --hip-compiler likewise
#   --reps <n>              timed repetitions per case (default 5)
#   --rounds <n>            compare: runs of each case per side (default 3)
#   --smoke                 tiny sizes, one repetition: run checks that the cases run; compare checks
#                           that both commits build, every case runs on both sides, the checksums
#                           match and host/device agree, with no timing verdicts (about a minute)
#   --core <n> | --no-pin   the core to pin to with taskset (default 2 for a host run, none with --gpu,
#                           whose host side uses every core), or no pinning
#   --help
#
# A host build runs the host cases, on one core; a GPU build runs the host/device pairs, the host side on
# every core (OpenMP). run prints a table per probe and saves every result as JSON lines under
# build/performance/results/<date>-<commit>.jsonl. Each probe runs in a process of its own.

cmake_minimum_required(VERSION 3.16)

get_filename_component(REPO "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
set(THIS_FILE "${CMAKE_CURRENT_LIST_FILE}")

# ── Arguments ─────────────────────────────────────────────────────────────────────────────────────────
set(args "")
set(after_dashes OFF)
math(EXPR last_arg "${CMAKE_ARGC} - 1")

foreach(i RANGE 0 ${last_arg})
  if(after_dashes)
    list(APPEND args "${CMAKE_ARGV${i}}")
  elseif("${CMAKE_ARGV${i}}" STREQUAL "--")
    set(after_dashes ON)
  endif()
endforeach()

function(usage)
  file(READ "${THIS_FILE}" text)
  string(REGEX MATCH "#   Scripts/run-performance.sh \\[run\\].*a process of its own\\." text "${text}")
  string(REGEX REPLACE "\n# ?" "\n" text "${text}")
  string(REGEX REPLACE "^# ?" "" text "${text}")
  message("Usage:\n${text}\nSee Docs/Sphinx/source/PerformanceTesting.rst.")
endfunction()

set(command run)
set(positional "")
set(gpu OFF)
set(gpu_arch "")
set(device_compiler "")
set(reps 5)
set(rounds 3)
set(smoke OFF)
set(core "")

list(LENGTH args nargs)
set(i 0)

while(i LESS nargs)
  list(GET args ${i} a)
  math(EXPR next "${i} + 1")

  if(i EQUAL 0 AND (a STREQUAL "run" OR a STREQUAL "list" OR a STREQUAL "compare"))
    set(command ${a})
  elseif(a STREQUAL "--help" OR a STREQUAL "-h")
    usage()
    return()
  elseif(a STREQUAL "--smoke")
    set(smoke ON)
  elseif(a STREQUAL "--no-pin")
    set(core OFF)
  elseif(a MATCHES "^--(gpu|gpu-arch|cuda-compiler|hip-compiler|reps|rounds|core)$")
    if(next GREATER_EQUAL nargs)
      message(FATAL_ERROR "run-performance: ${a} needs a value (see --help)")
    endif()

    list(GET args ${next} value)
    set(i ${next})

    if(a STREQUAL "--gpu")
      string(TOUPPER "${value}" gpu)

      if(NOT gpu MATCHES "^(CUDA|HIP)$")
        message(FATAL_ERROR "run-performance: --gpu must be cuda or hip (got '${value}')")
      endif()
    elseif(a STREQUAL "--gpu-arch")
      set(gpu_arch "${value}")
    elseif(a MATCHES "compiler")
      set(device_compiler "${value}")
    else()
      string(REPLACE "--" "" var "${a}")
      set(${var} "${value}")
    endif()
  elseif(a MATCHES "^-")
    message(FATAL_ERROR "run-performance: unknown option '${a}' (see --help)")
  else()
    list(APPEND positional "${a}")
  endif()

  math(EXPR i "${i} + 1")
endwhile()

# A host run is pinned to one core; a GPU run's host side uses them all.
if(core STREQUAL "")
  if(gpu STREQUAL "OFF")
    set(core 2)
  else()
    set(core OFF)
  endif()
endif()

# ── compare: hand over to Compare.cmake ───────────────────────────────────────────────────────────────
if(command STREQUAL "compare")
  set(refs "")
  set(CASES "")

  foreach(p IN LISTS positional)
    list(LENGTH refs nrefs)
    execute_process(COMMAND git rev-parse --verify --quiet "${p}^{commit}" WORKING_DIRECTORY "${REPO}"
                    RESULT_VARIABLE not_a_ref OUTPUT_QUIET ERROR_QUIET)

    if(not_a_ref EQUAL 0 AND nrefs LESS 2 AND NOT p MATCHES "[*?]")
      list(APPEND refs "${p}")
    else()
      list(APPEND CASES "${p}")
    endif()
  endforeach()

  list(LENGTH refs nrefs)
  set(BASE origin/dev)
  set(HEAD WORKTREE)

  if(nrefs GREATER_EQUAL 1)
    list(GET refs 0 BASE)
  endif()

  if(nrefs GREATER_EQUAL 2)
    list(GET refs 1 HEAD)
  endif()

  set(GPU ${gpu})
  set(SMOKE ${smoke})
  set(REPS ${reps})
  set(ROUNDS ${rounds})
  set(CORE ${core})

  if(gpu_arch)
    set(GPU_ARCH ${gpu_arch})
  endif()

  if(device_compiler AND NOT gpu STREQUAL "OFF")
    set(CMAKE_${gpu}_COMPILER "${device_compiler}")
  endif()

  # The comparison's exit status, for Scripts/run-performance.sh (see Compare.cmake).
  set(PERF_STATUS_FILE "${REPO}/build/performance/.compare-status")
  file(REMOVE "${PERF_STATUS_FILE}")

  include("${CMAKE_CURRENT_LIST_DIR}/Compare.cmake")
  return()
endif()

# ── run and list: build the probes (incrementally) ───────────────────────────────────────────────────
if(gpu STREQUAL "OFF")
  set(flavour host)
else()
  string(TOLOWER ${gpu} flavour)
endif()

set(build_dir "${REPO}/build/performance/${flavour}")
set(configure_args -DCMAKE_BUILD_TYPE=Release -DEBGEOMETRY_PERF_GPU=${gpu})

if(gpu_arch)
  list(APPEND configure_args -DCMAKE_${gpu}_ARCHITECTURES=${gpu_arch})
endif()

set(fresh "")

if(device_compiler AND NOT gpu STREQUAL "OFF")
  list(APPEND configure_args -DCMAKE_${gpu}_COMPILER=${device_compiler})

  # A device compiler is fixed when a build directory is first configured: start afresh if it changed.
  if(EXISTS "${build_dir}/CMakeCache.txt")
    file(STRINGS "${build_dir}/CMakeCache.txt" cached REGEX "^CMAKE_${gpu}_COMPILER:")
    string(REGEX REPLACE "^[^=]*=" "" cached "${cached}")

    if(NOT cached STREQUAL device_compiler)
      set(fresh --fresh)
    endif()
  endif()
endif()

if(NOT EXISTS "${build_dir}/CMakeCache.txt" OR fresh OR gpu_arch)
  message(STATUS "Configuring the probes (${flavour}) in ${build_dir}")
  execute_process(COMMAND ${CMAKE_COMMAND} ${fresh} -S "${REPO}/Performance" -B "${build_dir}" ${configure_args}
                  RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)

  if(NOT rc EQUAL 0)
    message(FATAL_ERROR "run-performance: configuring failed:\n${out}\n${err}")
  endif()
endif()

message(STATUS "Building the probes (${flavour}) ...")
execute_process(COMMAND ${CMAKE_COMMAND} --build "${build_dir}" --parallel RESULT_VARIABLE rc OUTPUT_VARIABLE out
                ERROR_VARIABLE err)

if(NOT rc EQUAL 0)
  message(FATAL_ERROR "run-performance: building failed:\n${out}\n${err}")
endif()

# ── run and list: the probes ──────────────────────────────────────────────────────────────────────────
file(GLOB probes "${build_dir}/bin/*.ex")
list(FILTER probes EXCLUDE REGEX "/PerfCompare\\.ex$")
list(SORT probes)

set(probe_args --side all)

foreach(g IN LISTS positional)
  list(APPEND probe_args --case "${g}")
endforeach()

# Probes with no selected case are skipped, rather than printing an empty table.
set(selected_probes "")

foreach(probe IN LISTS probes)
  if(command STREQUAL "list")
    # Straight to stdout, so the list can be piped.
    execute_process(COMMAND "${probe}" --list ${probe_args})
  else()
    execute_process(COMMAND "${probe}" --list ${probe_args} OUTPUT_VARIABLE names ERROR_QUIET
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(NOT names STREQUAL "")
      list(APPEND selected_probes "${probe}")
    endif()
  endif()
endforeach()

if(command STREQUAL "list")
  return()
endif()

if(NOT selected_probes)
  message(FATAL_ERROR "run-performance: no case matches '${positional}' (see 'Scripts/run-performance.sh list')")
endif()

find_program(TASKSET taskset)
set(pin "")

if(TASKSET AND NOT core STREQUAL "OFF")
  set(pin ${TASKSET} -c ${core})
endif()

execute_process(COMMAND git rev-parse --short HEAD WORKING_DIRECTORY "${REPO}" OUTPUT_VARIABLE sha
                OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
execute_process(COMMAND git status --porcelain --untracked-files=no WORKING_DIRECTORY "${REPO}" OUTPUT_VARIABLE dirty
                ERROR_QUIET)

if(dirty)
  set(sha "${sha}+worktree")
endif()

# The probes report this commit, not the one their build directory was first configured with.
set(ENV{EBGEOMETRY_PERF_GIT_SHA} "${sha}")

string(TIMESTAMP stamp "%Y%m%d-%H%M%S")
set(results "${REPO}/build/performance/results/${stamp}-${sha}-${flavour}.jsonl")
file(MAKE_DIRECTORY "${REPO}/build/performance/results")

set(run_args ${probe_args} --reps ${reps} --json-out "${results}")

if(smoke)
  list(APPEND run_args --smoke)
endif()

# The facts every probe shares (commit, compiler, threads, how to read the columns) come once, from
# the first probe.
set(banner "")

foreach(probe IN LISTS selected_probes)
  execute_process(COMMAND ${pin} "${probe}" ${run_args} ${banner} RESULT_VARIABLE rc)
  set(banner --no-banner)

  if(NOT rc EQUAL 0)
    get_filename_component(name "${probe}" NAME_WE)
    message(FATAL_ERROR "run-performance: ${name} exited with ${rc}")
  endif()
endforeach()

message("\nResults: ${results}")
