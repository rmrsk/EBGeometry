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
#   BASE       baseline commit or branch                         default: origin/main
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
#   JOBS       parallel compiles per build                        default: the hardware threads, capped by
#              memory (2 GB per compile, 6 GB with GPU)
#   TEST_BUILDS  ON: also build the heaviest test targets (TestRay*, TestTape*, TestRender*) in Debug
#              with assertions on both sides, and report what they cost (several minutes)   default: OFF
#   NO_SKIP    ON: run every case, also those of probes that compiled to the same code on both
#              sides (skipped by default, and listed as not run)            default: OFF
#   CONFIRM_ROUNDS  rounds more of each case flagged minor or MAJOR, judged on all its rounds; 0 to
#              judge the flags of ROUNDS rounds as they are                 default: 8
#   NO_CACHE   ON: build both sides afresh, rather than reusing a cached build of the same code with the
#              same compilers and settings                                  default: OFF
#   CACHE_DIR  where the builds are cached (the last CACHE_KEEP)            default: build/performance/cache
#   CACHE_KEEP how many cached builds are kept                              default: 6
#   BUILD_MINOR / BUILD_MAJOR  percent more compile CPU time or memory that is flagged in the build
#              costs (reported, not judged)                           default: 10 / 25
#   RECORD     ON: write the result as this comparison's section of     default: OFF
#              the performance receipt, Performance/receipt.md (see Receipt.cmake), to commit with the pull
#              request; CI checks it. Only for a full comparison of committed code: not with SMOKE or
#              CASES, and not while the measured paths have uncommitted changes.

cmake_minimum_required(VERSION 3.16)

get_filename_component(REPO "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

foreach(pair "BASE;origin/main" "HEAD;HEAD" "GPU;OFF" "CASES;*" "ROUNDS;2" "REPS;3" "MINOR;3" "MAJOR;10" "SMOKE;OFF"
             "WORK_DIR;${REPO}/build/performance/compare" "FAIL_ON_MAJOR;ON" "RECORD;OFF" "TEST_BUILDS;OFF" "NO_SKIP;OFF"
             "NO_CACHE;OFF" "CONFIRM_ROUNDS;8" "CACHE_DIR;${REPO}/build/performance/cache" "CACHE_KEEP;6"
             "BUILD_MINOR;10" "BUILD_MAJOR;25")
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
file(REMOVE_RECURSE "${WORK_DIR}/base" "${WORK_DIR}/head" "${WORK_DIR}/base-build" "${WORK_DIR}/head-build"
     "${WORK_DIR}/src" "${WORK_DIR}/build" "${WORK_DIR}/stripped"
     "${WORK_DIR}/base-tests" "${WORK_DIR}/head-tests" "${WORK_DIR}/tools-build")
file(MAKE_DIRECTORY "${WORK_DIR}")

set(borrowed "")

foreach(side base head)
  string(TOUPPER ${side} SIDE_VAR)
  set(ref "${${SIDE_VAR}}")

  set(src "${WORK_DIR}/${side}")

  # The working tree as it is: its files, tracked or not (but not ignored), uncommitted changes included,
  # copied out like an exported commit, so that it is built from the same path as the baseline (below).
  if(side STREQUAL "head" AND ref STREQUAL "WORKTREE")
    execute_process(COMMAND git rev-parse --short HEAD WORKING_DIRECTORY "${REPO}" OUTPUT_VARIABLE sha
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(sha "${sha}+worktree")
    execute_process(COMMAND git ls-files --cached --others --exclude-standard WORKING_DIRECTORY "${REPO}"
                    OUTPUT_VARIABLE files RESULT_VARIABLE rc)

    if(NOT rc EQUAL 0)
      message(FATAL_ERROR "Compare.cmake: cannot list the files of the working tree ${REPO}")
    endif()

    string(REPLACE "\n" ";" files "${files}")

    foreach(f IN LISTS files)
      # A deleted file is still listed until the deletion is staged; a submodule is a directory.
      if(NOT f STREQUAL "" AND NOT IS_DIRECTORY "${REPO}/${f}" AND EXISTS "${REPO}/${f}")
        get_filename_component(dir "${src}/${f}" DIRECTORY)
        file(COPY "${REPO}/${f}" DESTINATION "${dir}")
      endif()
    endforeach()
  else()
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

# The build-cost tools, from the candidate (a baseline may predate them): BuildCost, the compiler launcher
# that records each compile's CPU time, peak memory and, for nvcc, its kernels' registers, stack and spills;
# BuildCompare, which compares the two sides' records (Tools/).
run_or_die(COMMAND ${CMAKE_COMMAND} -S "${head_src}/Performance/Tools" -B "${WORK_DIR}/tools-build"
                   -DCMAKE_BUILD_TYPE=Release)
run_or_die(COMMAND ${CMAKE_COMMAND} --build "${WORK_DIR}/tools-build" --parallel)
set(build_cost "${WORK_DIR}/tools-build/bin/BuildCost.ex")

# Where the builds run. A compile's CPU time depends on the core it runs on: on a CPU with fast and slow cores
# (Intel's P- and E-cores), and with the other hardware thread of its core busy, two builds of the same code
# differed by up to 48% per probe. So on Linux the compiles run on one hardware thread of each fast core
# (the cores in /sys/devices/cpu_core/cpus on such a CPU, every core otherwise), pinned with taskset: two
# builds of the same code then agree within about 3% per probe. As many compiles run at once as those
# threads, but no more than the memory holds (2 GB per compile, 6 GB for a device compile).
cmake_host_system_information(RESULT machine_cores QUERY NUMBER_OF_PHYSICAL_CORES)
cmake_host_system_information(RESULT machine_threads QUERY NUMBER_OF_LOGICAL_CORES)
cmake_host_system_information(RESULT machine_mib QUERY TOTAL_PHYSICAL_MEMORY)

set(build_cpus "")
find_program(TASKSET taskset)

if(TASKSET AND EXISTS "/sys/devices/system/cpu/cpu0/topology/thread_siblings_list")
  set(candidates "")

  if(EXISTS "/sys/devices/cpu_core/cpus")
    file(STRINGS "/sys/devices/cpu_core/cpus" ranges LIMIT_COUNT 1)
  else()
    file(STRINGS "/sys/devices/system/cpu/online" ranges LIMIT_COUNT 1)
  endif()

  string(REPLACE "," ";" ranges "${ranges}")

  foreach(range IN LISTS ranges)
    if(range MATCHES "^([0-9]+)-([0-9]+)$")
      foreach(cpu RANGE ${CMAKE_MATCH_1} ${CMAKE_MATCH_2})
        list(APPEND candidates ${cpu})
      endforeach()
    elseif(range MATCHES "^[0-9]+$")
      list(APPEND candidates ${range})
    endif()
  endforeach()

  # Only the CPUs this process may run on (a container's cpuset, or a taskset around the whole run).
  file(STRINGS "/proc/self/status" allowed_line REGEX "^Cpus_allowed_list:")

  if(allowed_line MATCHES "^Cpus_allowed_list:[ \t]*(.*)$")
    string(REPLACE "," ";" allowed_ranges "${CMAKE_MATCH_1}")
    set(allowed "")

    foreach(range IN LISTS allowed_ranges)
      if(range MATCHES "^([0-9]+)-([0-9]+)$")
        foreach(cpu RANGE ${CMAKE_MATCH_1} ${CMAKE_MATCH_2})
          list(APPEND allowed ${cpu})
        endforeach()
      elseif(range MATCHES "^[0-9]+$")
        list(APPEND allowed ${range})
      endif()
    endforeach()

    if(allowed)
      set(kept "")

      foreach(cpu IN LISTS candidates)
        if(cpu IN_LIST allowed)
          list(APPEND kept ${cpu})
        endif()
      endforeach()

      set(candidates ${kept})
    endif()
  endif()

  # One hardware thread per core: the first of its siblings.
  foreach(cpu IN LISTS candidates)
    file(STRINGS "/sys/devices/system/cpu/cpu${cpu}/topology/thread_siblings_list" siblings LIMIT_COUNT 1)
    string(REGEX MATCH "^[0-9]+" first "${siblings}")

    if(first STREQUAL "${cpu}")
      list(APPEND build_cpus ${cpu})
    endif()
  endforeach()
endif()

# If pinning to them fails anyway, the builds run unpinned rather than not at all.
if(build_cpus)
  string(REPLACE ";" "," cpu_text "${build_cpus}")
  execute_process(COMMAND ${TASKSET} -c ${cpu_text} true RESULT_VARIABLE pin_rc OUTPUT_QUIET ERROR_QUIET)

  if(NOT pin_rc EQUAL 0)
    set(build_cpus "")
  endif()
endif()

list(LENGTH build_cpus nbuild_cpus)

# The memory: the machine's, or a container's limit if lower (cgroup v2's memory.max; on a cgroup v1 host
# the machine's memory is used).
if(EXISTS "/sys/fs/cgroup/memory.max")
  file(STRINGS "/sys/fs/cgroup/memory.max" cgroup_max LIMIT_COUNT 1)

  if(cgroup_max MATCHES "^[0-9]+$")
    math(EXPR cgroup_mib "${cgroup_max} / 1048576")

    if(cgroup_mib LESS machine_mib)
      set(machine_mib ${cgroup_mib})
    endif()
  endif()
endif()

if(NOT DEFINED JOBS OR "${JOBS}" STREQUAL "")
  if(nbuild_cpus GREATER 0)
    set(JOBS ${nbuild_cpus})
  else()
    set(JOBS ${machine_cores})
  endif()

  if(GPU STREQUAL "OFF")
    math(EXPR by_memory "${machine_mib} / 2048")
  else()
    math(EXPR by_memory "${machine_mib} / 6144")
  endif()

  if(JOBS GREATER by_memory)
    set(JOBS ${by_memory})
  endif()

  if(JOBS LESS 1)
    set(JOBS 1)
  endif()
endif()

set(build_pin "")

if(nbuild_cpus GREATER 0)
  string(REPLACE ";" "," cpu_text "${build_cpus}")
  set(build_pin ${TASKSET} -c ${cpu_text})
  if(EXISTS "/sys/devices/cpu_core/cpus")
    set(where_text "on hardware threads ${cpu_text} (one per fast core)")
  else()
    set(where_text "on hardware threads ${cpu_text} (one per core)")
  endif()
else()
  set(where_text "unpinned")
endif()

math(EXPR machine_gb "(${machine_mib} + 512) / 1024")
set(machine_text "Machine: ${machine_cores} cores, ${machine_threads} hardware threads, ${machine_gb} GB. Compiled ${where_text}, ${JOBS} at a time.")

# Every compile goes through BuildCost, which appends to the side's record (an initial cache, since a
# launcher with arguments is a list).
# build_key(<source> <build dir> <configure arguments> <var>): the cache key of a configured side (see the build
# loop below), a hash of its code and of how it is built.
function(build_key a_src a_build a_args a_var)
  set(text "cmake ${CMAKE_VERSION}\nargs ${a_args}\n")

  # The compilers the configure found, and their versions; and the flags it took from the environment
  # (CXXFLAGS, CUDAFLAGS, HIPFLAGS seed them).
  file(STRINGS "${a_build}/CMakeCache.txt" flags REGEX "^CMAKE_(CXX|CUDA|HIP)_FLAGS[A-Z_]*:")
  string(APPEND text "flags ${flags}\n")
  file(STRINGS "${a_build}/CMakeCache.txt" compilers REGEX "^CMAKE_(CXX|CUDA|HIP)_COMPILER:")

  foreach(line IN LISTS compilers)
    string(REGEX REPLACE "^[^=]*=" "" compiler "${line}")

    if(EXISTS "${compiler}")
      execute_process(COMMAND "${compiler}" --version OUTPUT_VARIABLE version ERROR_QUIET)
      string(APPEND text "compiler ${compiler}\n${version}\n")
    endif()
  endforeach()

  # The CPU, since the probes are built for the host's instruction set.
  if(EXISTS "/proc/cpuinfo")
    file(STRINGS "/proc/cpuinfo" cpu REGEX "^(model name|flags)" LIMIT_COUNT 2)
    string(APPEND text "cpu ${cpu}\n")
  endif()

  # The code: every file of the library and the probes, but their Markdown (the receipt, the README).
  file(GLOB_RECURSE files RELATIVE "${a_src}" "${a_src}/Source/*" "${a_src}/Performance/*")
  list(APPEND files EBGeometry.hpp "${WORK_DIR}/AlignFunctions.cmake")
  list(FILTER files EXCLUDE REGEX "[.]md$")
  list(SORT files)

  foreach(f IN LISTS files)
    if(IS_ABSOLUTE "${f}")
      file(SHA1 "${f}" hash)
    else()
      file(SHA1 "${a_src}/${f}" hash)
    endif()

    string(APPEND text "${f} ${hash}\n")
  endforeach()

  string(SHA1 key "${text}")
  set(${a_var} "${key}" PARENT_SCOPE)
endfunction()

# prune_cache(): keep the CACHE_KEEP most recently used cached builds, and remove any left half-written by
# an interrupted run.
function(prune_cache)
  file(GLOB partial LIST_DIRECTORIES true "${CACHE_DIR}/*.partial")

  if(partial)
    file(REMOVE_RECURSE ${partial})
  endif()

  file(GLOB entries LIST_DIRECTORIES true "${CACHE_DIR}/*/complete")
  set(stamped "")

  foreach(e IN LISTS entries)
    file(STRINGS "${e}" when LIMIT_COUNT 1)
    get_filename_component(dir "${e}" DIRECTORY)
    list(APPEND stamped "${when}|${dir}")
  endforeach()

  list(SORT stamped COMPARE NATURAL ORDER DESCENDING)
  set(i 0)

  foreach(item IN LISTS stamped)
    if(i GREATER_EQUAL CACHE_KEEP)
      string(REGEX REPLACE "^[^|]*[|]" "" dir "${item}")
      file(REMOVE_RECURSE "${dir}")
    endif()

    math(EXPR i "${i} + 1")
  endforeach()
endfunction()

function(cost_cache a_file a_record)
  file(WRITE "${a_file}" "")

  foreach(lang CXX CUDA HIP)
    file(APPEND "${a_file}" "set(CMAKE_${lang}_COMPILER_LAUNCHER \"${build_cost};--record=${a_record}\" CACHE STRING \"\")\n")
  endforeach()

  file(REMOVE "${a_record}")
endfunction()

# Both sides are built with the candidate's function alignment (AlignFunctions.cmake), injected into each
# configure so that it holds for a baseline whose own probes predate it (a copy, since the candidate's tree
# moves while it is built).
configure_file("${head_src}/Performance/AlignFunctions.cmake" "${WORK_DIR}/AlignFunctions.cmake" COPYONLY)

# Each side is built from the same path, ${WORK_DIR}/src, into ${WORK_DIR}/build, and then moved to its own:
# paths enter the binaries (__FILE__ in the library's checks, nvcc's names for anonymous namespaces), so the
# two sides then compile the same code to the same bytes, and a probe whose code did not change need not run
# (below). For the same reason the commit is passed at run time (EBGEOMETRY_PERF_GIT_SHA in the environment),
# not compiled in, and the smoke meshes' path is the shared one, read only by a smoke run, which builds each
# side with its own.
# A side's build is cached under CACHE_DIR, keyed by what decides its bytes: the code it is built from (the
# library and the probes, the Markdown files aside), the configure arguments, the compilers' versions, CMake's
# version and the CPU (the probes are built for the host's instruction set). A later comparison of the same code
# with the same settings, the baseline until main's code moves on or a receipt rerun after a docs-only commit,
# copies the cached programs and their build costs instead of building them again.
get_filename_component(CACHE_DIR "${CACHE_DIR}" ABSOLUTE BASE_DIR "${REPO}")

foreach(side base head)
  message(STATUS "Building ${side} (${${side}_sha}) ...")
  cost_cache("${WORK_DIR}/${side}-cost.cmake" "${WORK_DIR}/${side}-build-costs.jsonl")

  file(RENAME "${${side}_src}" "${WORK_DIR}/src")
  set(build_src "${WORK_DIR}/src")
  set(build_dir "${WORK_DIR}/build")

  if(SMOKE)
    set(smoke_dir "${${side}_src}/Tests/data")
  else()
    set(smoke_dir "${build_src}/Tests/data")
  endif()

  set(configure_args
      -DCMAKE_BUILD_TYPE=Release -DEBGEOMETRY_PERF_GIT_SHA=compare
      -DEBGEOMETRY_PERF_MESH_DIR=${REPO}/common-3d-test-models/data -DEBGEOMETRY_PERF_SMOKE_MESH_DIR=${smoke_dir}
      -DCMAKE_PROJECT_INCLUDE=${WORK_DIR}/AlignFunctions.cmake ${gpu_args})
  run_or_die(COMMAND ${CMAKE_COMMAND} -C "${WORK_DIR}/${side}-cost.cmake" -S "${build_src}/Performance" -B "${build_dir}"
                     ${configure_args})

  build_key("${build_src}" "${build_dir}" "${configure_args}" key)
  set(entry "${CACHE_DIR}/${key}")
  set(${side}_cached OFF)

  if(NOT NO_CACHE AND EXISTS "${entry}/complete")
    message(STATUS "Reusing the cached build of ${side}: ${entry}")
    file(REMOVE_RECURSE "${build_dir}")
    file(MAKE_DIRECTORY "${WORK_DIR}/${side}-build")
    file(COPY "${entry}/bin" DESTINATION "${WORK_DIR}/${side}-build")
    configure_file("${entry}/build-costs.jsonl" "${WORK_DIR}/${side}-build-costs.jsonl" COPYONLY)
    string(TIMESTAMP now "%s" UTC)
    file(WRITE "${entry}/complete" "${now}\n")
    set(${side}_cached ON)
  else()
    run_or_die(COMMAND ${build_pin} ${CMAKE_COMMAND} --build "${build_dir}" --parallel ${JOBS})
    file(RENAME "${build_dir}" "${WORK_DIR}/${side}-build")

    # Into the cache: written beside it and moved in whole, so that an interrupted copy is never reused.
    if(NOT NO_CACHE)
      prune_cache()
      file(REMOVE_RECURSE "${entry}" "${entry}.partial")
      file(MAKE_DIRECTORY "${entry}.partial")
      file(COPY "${WORK_DIR}/${side}-build/bin" DESTINATION "${entry}.partial")
      configure_file("${WORK_DIR}/${side}-build-costs.jsonl" "${entry}.partial/build-costs.jsonl" COPYONLY)
      string(TIMESTAMP now "%s" UTC)
      file(WRITE "${entry}.partial/complete" "${now}\n")
      file(RENAME "${entry}.partial" "${entry}")
      prune_cache()
    endif()
  endif()

  file(RENAME "${WORK_DIR}/src" "${${side}_src}")
endforeach()

# The heaviest test targets, in Debug with assertions, as a developer and CI's device lanes build them:
# what they cost, not their results. The targets are found by name on each side, so a split test file
# (TestRayMesh, ...) is still compared with the file it came from, in the total.
if(TEST_BUILDS)
  set(test_gpu_args "")

  if(NOT GPU STREQUAL "OFF")
    list(APPEND test_gpu_args -DEBGEOMETRY_ENABLE_${GPU}=ON)

    if(DEFINED GPU_ARCH)
      list(APPEND test_gpu_args -DCMAKE_${GPU}_ARCHITECTURES=${GPU_ARCH})
    endif()

    if(DEFINED CMAKE_${GPU}_COMPILER)
      list(APPEND test_gpu_args -DCMAKE_${GPU}_COMPILER=${CMAKE_${GPU}_COMPILER})
    endif()
  endif()

  foreach(side base head)
    message(STATUS "Building ${side}'s heaviest tests in Debug ...")
    cost_cache("${WORK_DIR}/${side}-test-cost.cmake" "${WORK_DIR}/${side}-test-costs.jsonl")
    run_or_die(
      COMMAND ${CMAKE_COMMAND} -C "${WORK_DIR}/${side}-test-cost.cmake" -S "${${side}_src}" -B "${WORK_DIR}/${side}-tests"
              -DCMAKE_BUILD_TYPE=Debug -DEBGEOMETRY_ENABLE_ASSERTIONS=ON -DEBGEOMETRY_BUILD_TESTS=ON
              -DEBGEOMETRY_BUILD_EXAMPLES=OFF -DFETCHCONTENT_BASE_DIR=${WORK_DIR}/deps ${test_gpu_args}
    )
    # The target list: "... Name" lines (Makefiles) or "Name: phony" lines (Ninja).
    execute_process(COMMAND ${CMAKE_COMMAND} --build "${WORK_DIR}/${side}-tests" --target help OUTPUT_VARIABLE targets)
    string(REPLACE "\n" ";" target_lines "${targets}")
    set(test_targets "")

    foreach(line IN LISTS target_lines)
      if(line MATCHES "^(\\.\\.\\. )?(Test(Ray|Tape|Render)[A-Za-z0-9]*)(: phony.*)?$")
        list(APPEND test_targets ${CMAKE_MATCH_2})
      endif()
    endforeach()

    list(REMOVE_DUPLICATES test_targets)

    # Built together, in parallel. With Makefiles, "cmake --build --target A --target B" builds the targets one
    # after another (the top-level Makefile is .NOTPARALLEL), so their per-target goals are made directly from
    # CMakeFiles/Makefile2 (the build system was just generated); other generators build several
    # targets in parallel themselves.
    if(test_targets)
      set(test_dir "${WORK_DIR}/${side}-tests")
      file(STRINGS "${test_dir}/CMakeCache.txt" make_line REGEX "^CMAKE_MAKE_PROGRAM:")
      string(REGEX REPLACE "^[^=]*=" "" make_program "${make_line}")

      if(EXISTS "${test_dir}/CMakeFiles/Makefile2" AND EXISTS "${make_program}")
        set(goals "")

        foreach(t IN LISTS test_targets)
          list(APPEND goals Tests/CMakeFiles/${t}.dir/all)
        endforeach()

        run_or_die(COMMAND ${build_pin} ${make_program} -C "${test_dir}" -f CMakeFiles/Makefile2 -j${JOBS} ${goals})
      else()
        set(target_args "")

        foreach(t IN LISTS test_targets)
          list(APPEND target_args --target ${t})
        endforeach()

        run_or_die(COMMAND ${build_pin} ${CMAKE_COMMAND} --build "${test_dir}" --parallel ${JOBS} ${target_args})
      endif()
    endif()
  endforeach()
endif()

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

# The probes that compiled to the same code on both sides: their stripped binaries are equal byte for byte
# (strip removes the symbol tables, where nvcc leaves a temporary file's name). Such a probe runs the same
# instructions on the same inputs on both sides (the meshes are the repository's, shared), so it can only
# measure noise, and is not run. Not in a smoke run, which checks that every case runs; not with NO_SKIP.
set(identical "")

if(NOT SMOKE AND NOT NO_SKIP)
  find_program(STRIP strip)

  if(STRIP)
    file(MAKE_DIRECTORY "${WORK_DIR}/stripped")

    foreach(p IN LISTS head_probes)
      get_filename_component(probe "${p}" DIRECTORY)
      set(same OFF)

      foreach(side base head)
        set(exe "${WORK_DIR}/${side}-build/bin/${probe}.ex")

        if(NOT EXISTS "${exe}")
          set(same "")
          break()
        endif()

        execute_process(COMMAND ${STRIP} -o "${WORK_DIR}/stripped/${side}-${probe}.ex" "${exe}" RESULT_VARIABLE rc
                        OUTPUT_QUIET ERROR_QUIET)

        if(NOT rc EQUAL 0)
          set(same "")
          break()
        endif()
      endforeach()

      if(NOT same STREQUAL "")
        execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files "${WORK_DIR}/stripped/base-${probe}.ex"
                                "${WORK_DIR}/stripped/head-${probe}.ex" RESULT_VARIABLE differs)

        if(differs EQUAL 0)
          list(APPEND identical "${probe}")
        endif()
      endif()
    endforeach()

    file(REMOVE_RECURSE "${WORK_DIR}/stripped")
  else()
    message(STATUS "No strip program found: every probe runs")
  endif()
endif()

# ── Run ───────────────────────────────────────────────────────────────────────────────────────────────
find_program(TASKSET taskset)
set(pin "")

if(TASKSET AND NOT CORE STREQUAL "OFF")
  set(pin ${TASKSET} -c ${CORE})
endif()

set(jobs "")
set(case_args "")
set(not_run "")
set(nnot_run 0)

# CASES may hold several globs (a CMake list).
foreach(glob IN LISTS CASES)
  list(APPEND case_args --case "${glob}")
endforeach()

foreach(p IN LISTS head_probes)
  get_filename_component(probe "${p}" DIRECTORY)
  execute_process(COMMAND "${WORK_DIR}/head-build/bin/${probe}.ex" --list ${case_args}
                  OUTPUT_VARIABLE names OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REPLACE "\n" ";" names "${names}")

  list(FILTER names EXCLUDE REGEX "^$")
  list(LENGTH names ncases)

  if(ncases EQUAL 0)
    continue()
  endif()

  if(probe IN_LIST identical)
    list(APPEND not_run "${probe} (${ncases})")
    math(EXPR nnot_run "${nnot_run} + ${ncases}")
    continue()
  endif()

  foreach(case_name IN LISTS names)
    list(APPEND jobs "${probe}|${case_name}")
  endforeach()
endforeach()

list(LENGTH jobs njobs)

if(njobs EQUAL 0 AND nnot_run EQUAL 0)
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

# run_rounds(<first> <last> <jobs>): rounds first..last of the given jobs, alternating between the sides (the
# order swaps each round), each case in a fresh process; results appended to <side>.jsonl.
function(run_rounds a_first a_last a_jobs)
  list(LENGTH a_jobs count)
  set(failed_here "")

  foreach(round RANGE ${a_first} ${a_last})
    message(STATUS "Round ${round}/${a_last}: ${count} cases per side")

    math(EXPR odd "${round} % 2")

    if(odd)
      set(order base head)
    else()
      set(order head base)
    endif()

    foreach(job IN LISTS a_jobs)
      string(REPLACE "|" ";" job "${job}")
      list(GET job 0 probe)
      list(GET job 1 name)

      foreach(side IN LISTS order)
        set(exe "${WORK_DIR}/${side}-build/bin/${probe}.ex")

        if(NOT EXISTS "${exe}")
          continue()
        endif()

        execute_process(COMMAND ${CMAKE_COMMAND} -E env EBGEOMETRY_PERF_GIT_SHA=${${side}_sha} ${pin} "${exe}" --case
                                "${name}" --reps ${REPS} --json ${smoke_arg}
                        OUTPUT_VARIABLE out RESULT_VARIABLE rc)

        if(NOT rc EQUAL 0)
          message(WARNING "Compare.cmake: ${side} ${probe} --case ${name} exited with ${rc}")
          list(APPEND failed_here "${side}: ${name} (exit ${rc})")
        endif()

        file(APPEND "${WORK_DIR}/${side}.jsonl" "${out}")
      endforeach()
    endforeach()
  endforeach()

  set(failed ${failed} ${failed_here} PARENT_SCOPE)
endfunction()

run_rounds(1 ${ROUNDS} "${jobs}")

# Flags confirmed: a case flagged minor or MAJOR after ROUNDS rounds (a judged run, not a smoke one) runs
# CONFIRM_ROUNDS more, and is judged on all of them. A flag from a noisy round then rarely survives, and one
# that does is a change, not luck; no rerun by hand is needed before reading the verdict.
set(confirmed "")

if(NOT SMOKE AND ROUNDS GREATER_EQUAL 2 AND CONFIRM_ROUNDS GREATER 0 AND njobs GREATER 0)
  execute_process(
    COMMAND "${WORK_DIR}/head-build/bin/PerfCompare.ex" --base "${WORK_DIR}/base.jsonl" --head "${WORK_DIR}/head.jsonl"
            --minor ${MINOR} --major ${MAJOR} --verdict-only --flagged-file "${WORK_DIR}/flagged.txt"
    OUTPUT_QUIET RESULT_VARIABLE rc)

  if(rc MATCHES "^(0|2)$" AND EXISTS "${WORK_DIR}/flagged.txt")
    file(STRINGS "${WORK_DIR}/flagged.txt" flagged)
    set(again_jobs "")

    foreach(job IN LISTS jobs)
      string(REGEX REPLACE "^[^|]*[|]" "" name "${job}")

      if(name IN_LIST flagged)
        list(APPEND again_jobs "${job}")
        list(APPEND confirmed "${name}")
      endif()
    endforeach()

    if(again_jobs)
      list(LENGTH again_jobs nagain)
      math(EXPR first "${ROUNDS} + 1")
      math(EXPR last "${ROUNDS} + ${CONFIRM_ROUNDS}")
      message(STATUS "Confirming ${nagain} flagged case(s) with ${CONFIRM_ROUNDS} more rounds")
      run_rounds(${first} ${last} "${again_jobs}")
    endif()
  endif()
endif()

list(LENGTH failed nfailed)

# ── Report ────────────────────────────────────────────────────────────────────────────────────────────
# When every probe compiled to the same code on both sides, nothing ran, and there is nothing to judge: the
# verdict is green.
if(njobs EQUAL 0)
  file(WRITE "${WORK_DIR}/report.md"
       "### Performance comparison\n\nEvery probe compiled to the same code on both sides; no case was run.\n")
  file(WRITE "${WORK_DIR}/level.txt" "0\n")
  set(verdict 0)
  message("Every probe compiled to the same code on both sides; no case was run.")
else()
  execute_process(
    COMMAND "${WORK_DIR}/head-build/bin/PerfCompare.ex" --base "${WORK_DIR}/base.jsonl" --head "${WORK_DIR}/head.jsonl"
            --minor ${MINOR} --major ${MAJOR} --markdown "${WORK_DIR}/report.md" ${smoke_arg} --no-verdict
            --failed-runs ${nfailed} --level-file "${WORK_DIR}/level.txt"
    RESULT_VARIABLE verdict)
endif()

# The report as PerfCompare wrote it, before the commands to repeat it (local paths) are added: a receipt
# carries this.
set(report_text "")

if(EXISTS "${WORK_DIR}/report.md")
  file(READ "${WORK_DIR}/report.md" report_text)
endif()

# What the builds cost (BuildCompare): reported with the comparison, and carried by the receipt, but not part
# of its verdict.
set(build_sections "")
set(build_parts "build-costs|Probe builds (Release)")

if(TEST_BUILDS)
  list(APPEND build_parts "test-costs|Heaviest test builds (Debug, assertions)")
endif()

foreach(part IN LISTS build_parts)
  string(REPLACE "|" ";" part "${part}")
  list(GET part 0 stem)
  list(GET part 1 title)

  execute_process(
    COMMAND "${WORK_DIR}/tools-build/bin/BuildCompare.ex" --base "${WORK_DIR}/base-${stem}.jsonl"
            --head "${WORK_DIR}/head-${stem}.jsonl" --title "Build costs: ${title}" --machine "${machine_text}"
            --minor ${BUILD_MINOR} --major ${BUILD_MAJOR} --markdown "${WORK_DIR}/${stem}.md"
    OUTPUT_QUIET RESULT_VARIABLE build_rc)

  if(build_rc EQUAL 0 AND EXISTS "${WORK_DIR}/${stem}.md")
    file(READ "${WORK_DIR}/${stem}.md" section)
    string(APPEND build_sections "${section}")
  else()
    string(APPEND build_sections "\n### Build costs: ${title}\n\nNot reported: BuildCompare could not read the records.\n")
  endif()
endforeach()

string(APPEND report_text "${build_sections}")
file(APPEND "${WORK_DIR}/report.md" "${build_sections}")
message("${build_sections}")

set(notes "")

if(borrowed)
  string(APPEND notes "\n${borrowed}\n")
endif()

if(failed)
  string(REPLACE ";" "\n  " failed_text "${failed}")
  string(APPEND notes "\nThese runs failed (crashed or exited with an error):\n  ${failed_text}\n")
endif()

foreach(side base head)
  if(${side}_cached)
    string(APPEND notes "\nThe ${side}'s build was reused from the cache (the same code, compilers and settings); its build "
                        "costs are those recorded when it was built.\n")
  endif()
endforeach()

if(confirmed)
  list(LENGTH confirmed nconfirmed)
  math(EXPR total_rounds "${ROUNDS} + ${CONFIRM_ROUNDS}")
  string(REPLACE ";" ", " confirmed_text "${confirmed}")
  string(APPEND notes "\nFlagged after ${ROUNDS} rounds, and judged on ${total_rounds} (${nconfirmed} cases): ${confirmed_text}\n")
endif()

if(not_run)
  string(REPLACE ";" ", " not_run_text "${not_run}")
  string(APPEND notes "\nThese probes compiled to the same code on both sides and were not run (${nnot_run} cases): "
                      "${not_run_text}\n")
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

if(TEST_BUILDS)
  string(APPEND again " --test-builds")
endif()

if(NO_SKIP)
  string(APPEND again " --no-skip")
endif()

if(NO_CACHE)
  string(APPEND again " --no-cache")
endif()

if(NOT CONFIRM_ROUNDS EQUAL 8)
  string(APPEND again " --confirm-rounds ${CONFIRM_ROUNDS}")
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
if(njobs EQUAL 0)
  message("VERDICT: green (no case was run)")
  set(verdict_again 0)
else()
  execute_process(
    COMMAND "${WORK_DIR}/head-build/bin/PerfCompare.ex" --base "${WORK_DIR}/base.jsonl" --head "${WORK_DIR}/head.jsonl"
            --minor ${MINOR} --major ${MAJOR} ${smoke_arg} --verdict-only --failed-runs ${nfailed}
    RESULT_VARIABLE verdict_again)
endif()

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
