# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
#
# SPDX-License-Identifier: GPL-3.0-or-later

# The performance probes: one program per folder with a main.cpp, plus PerfCompare, which compares two
# sets of results. Built from the project (EBGEOMETRY_BUILD_PERFORMANCE=ON, or the perf preset) or on
# its own (cmake -S Performance -B build). See README.md and Compare.cmake.
cmake_minimum_required(VERSION 3.16)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  project(EBGeometryPerformance CXX)

  if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
  endif()

  set(EBGEOMETRY_HOME "${CMAKE_CURRENT_SOURCE_DIR}/.." CACHE PATH "Path to the directory containing EBGeometry.hpp")
  add_library(EBGeometry::EBGeometry INTERFACE IMPORTED)
  target_include_directories(EBGeometry::EBGeometry INTERFACE "${EBGEOMETRY_HOME}")
  target_compile_features(EBGeometry::EBGeometry INTERFACE cxx_std_17)

  # The renderer draws on std::thread; older C libraries need -pthread for it.
  find_package(Threads REQUIRED)
  target_link_libraries(EBGeometry::EBGeometry INTERFACE Threads::Threads)

  # On its own, the probes take the host's full instruction set, as a performance build should.
  set(EBGEOMETRY_PERF_ARCH_FLAGS "-march=native" CACHE STRING "Architecture flags for a standalone host build")
  enable_testing()
else()
  set(EBGEOMETRY_PERF_ARCH_FLAGS "" CACHE STRING "Architecture flags for a standalone host build")
endif()

set(EBGEOMETRY_PERF_MESH_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../common-3d-test-models/data" CACHE PATH
    "Directory holding armadillo.obj (the common-3d-test-models submodule)")
set(EBGEOMETRY_PERF_SMOKE_MESH_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../Tests/data" CACHE PATH
    "Directory holding dodecahedron.obj, for --smoke")

# The commit the probes are built from. Compare.cmake passes it for a tree exported without .git.
if(NOT DEFINED EBGEOMETRY_PERF_GIT_SHA)
  execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE EBGEOMETRY_PERF_GIT_SHA
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET)

  if(NOT EBGEOMETRY_PERF_GIT_SHA)
    set(EBGEOMETRY_PERF_GIT_SHA "unknown")
  endif()
endif()

# The same GPU switch as the examples: the project's EBGEOMETRY_ENABLE_HIP/CUDA, or, standalone,
# EBGEOMETRY_PERF_GPU. A GPU build compiles every probe as CUDA or HIP; its device cases then run on
# the device, and its host cases on the host.
set(EBGEOMETRY_PERF_GPU OFF CACHE STRING "Compile the probes for a GPU when built on their own: OFF, CUDA or HIP")
set_property(CACHE EBGEOMETRY_PERF_GPU PROPERTY STRINGS OFF CUDA HIP)

if(EBGEOMETRY_ENABLE_HIP)
  set(_perf_gpu HIP)
elseif(EBGEOMETRY_ENABLE_CUDA)
  set(_perf_gpu CUDA)
else()
  set(_perf_gpu ${EBGEOMETRY_PERF_GPU})
endif()

if(_perf_gpu STREQUAL "HIP" OR _perf_gpu STREQUAL "CUDA")
  get_property(_perf_languages GLOBAL PROPERTY ENABLED_LANGUAGES)

  if(NOT _perf_gpu IN_LIST _perf_languages)
    if(NOT DEFINED CMAKE_HIP_ARCHITECTURES)
      set(CMAKE_HIP_ARCHITECTURES gfx90a)
    endif()

    if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
      # The GPU in this machine, so that the binary carries its machine code; 70 when none is visible.
      execute_process(COMMAND nvidia-smi --query-gpu=compute_cap --format=csv,noheader
                      OUTPUT_VARIABLE _cuda_cap RESULT_VARIABLE _cuda_smi ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
      string(REGEX MATCH "^[0-9]+\\.[0-9]+" _cuda_cap "${_cuda_cap}")
    
      if(_cuda_smi EQUAL 0 AND _cuda_cap)
        string(REPLACE "." "" CMAKE_CUDA_ARCHITECTURES "${_cuda_cap}")
      else()
        set(CMAKE_CUDA_ARCHITECTURES 70)
      endif()
    endif()

    enable_language(${_perf_gpu})
  endif()
elseif(NOT _perf_gpu STREQUAL "OFF")
  message(FATAL_ERROR "EBGEOMETRY_PERF_GPU must be OFF, CUDA or HIP (got '${_perf_gpu}')")
endif()

if(_perf_gpu STREQUAL "HIP" OR _perf_gpu STREQUAL "CUDA")
  set(_perf_gpu_build ON)
else()
  set(_perf_gpu_build OFF)
endif()

# Host-side vector flags for a GPU build, so that a host/device comparison does not set unvectorised
# host code against the device (safe: the default branching factor and widths never follow the ISA).
# HIP (clang) takes AVX2 and FMA, which every x86 host of a GPU has. nvcc cannot: with any host SIMD
# macro set, GCC's immintrin.h pulls in the AMX headers, which nvcc's front end fails to parse (GCC 12
# and 13 alike), so a CUDA build's host side stays scalar, as CI's CUDA lane is.
if(_perf_gpu STREQUAL "HIP")
  set(_perf_gpu_host_default "-mavx2;-mfma")
else()
  set(_perf_gpu_host_default "")
endif()

set(EBGEOMETRY_PERF_GPU_HOST_FLAGS "${_perf_gpu_host_default}" CACHE STRING "Host-side vector flags for the probes in a GPU build")

# OpenMP for the host side of the host/device pairs, so that a GPU is compared with every CPU core
# rather than one. On by default for CUDA, where nvcc's host compiler is the C++ compiler OpenMP was
# found for; off by default for HIP, whose clang host side would need the LLVM OpenMP runtime rather
# than the one found for the C++ compiler (not yet tested). Host builds run no pairs and need none.
if(_perf_gpu STREQUAL "CUDA")
  set(_perf_openmp_default ON)
else()
  set(_perf_openmp_default OFF)
endif()

option(EBGEOMETRY_PERF_OPENMP "Run the host side of the host/device pairs on every core (OpenMP)" ${_perf_openmp_default})

if(EBGEOMETRY_PERF_OPENMP AND _perf_gpu_build)
  find_package(OpenMP COMPONENTS CXX)

  if(NOT OpenMP_CXX_FOUND)
    message(STATUS "Performance: OpenMP not found; the host side of the pairs runs on one thread")
  endif()
endif()

# Every folder here with a main.cpp is a probe; the folders are the only list. (Common/ holds the
# harness, Tools/ PerfCompare.)
file(GLOB perf_mains CONFIGURE_DEPENDS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*/main.cpp)
list(SORT perf_mains)

set(EBGEOMETRY_PERF_PROBES "")

foreach(perf_main IN LISTS perf_mains)
  get_filename_component(name ${perf_main} DIRECTORY)
  set(target Perf${name})

  add_executable(${target} ${perf_main})
  target_link_libraries(${target} PRIVATE EBGeometry::EBGeometry)
  target_compile_definitions(${target} PRIVATE
    EBGEOMETRY_PERF_GIT_SHA="${EBGEOMETRY_PERF_GIT_SHA}"
    EBGEOMETRY_PERF_MESH_DIR="${EBGEOMETRY_PERF_MESH_DIR}"
    EBGEOMETRY_PERF_SMOKE_MESH_DIR="${EBGEOMETRY_PERF_SMOKE_MESH_DIR}")
  set_target_properties(${target} PROPERTIES
    OUTPUT_NAME ${name}
    SUFFIX ".ex"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")

  if(EBGEOMETRY_PERF_ARCH_FLAGS AND NOT _perf_gpu_build)
    separate_arguments(_perf_arch UNIX_COMMAND "${EBGEOMETRY_PERF_ARCH_FLAGS}")
    target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${_perf_arch}>)
  endif()

  # In a GPU build the host side still gets vector instructions, so that a host/device comparison
  # does not set unvectorised host code against the device. Safe, because the default branching
  # factor and widths never follow the ISA (see ConfigurationOptions.rst).
  if(_perf_gpu STREQUAL "CUDA" AND EBGEOMETRY_PERF_GPU_HOST_FLAGS)
    foreach(flag IN LISTS EBGEOMETRY_PERF_GPU_HOST_FLAGS)
      target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:-Xcompiler=${flag}>)
    endforeach()
  elseif(_perf_gpu STREQUAL "HIP" AND EBGEOMETRY_PERF_GPU_HOST_FLAGS)
    target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:HIP>:${EBGEOMETRY_PERF_GPU_HOST_FLAGS}>)
  endif()

  if(EBGEOMETRY_PERF_OPENMP AND _perf_gpu_build AND OpenMP_CXX_FOUND)
    separate_arguments(_perf_omp UNIX_COMMAND "${OpenMP_CXX_FLAGS}")

    if(_perf_gpu STREQUAL "CUDA")
      foreach(flag IN LISTS _perf_omp)
        target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:-Xcompiler=${flag}>)
      endforeach()
    else()
      target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:HIP>:${_perf_omp}>)
    endif()

    target_link_libraries(${target} PRIVATE OpenMP::OpenMP_CXX)
  endif()

  if(_perf_gpu STREQUAL "HIP" OR _perf_gpu STREQUAL "CUDA")
    set_source_files_properties(${perf_main} PROPERTIES LANGUAGE ${_perf_gpu})
    set_target_properties(${target} PROPERTIES ${_perf_gpu}_STANDARD 17 ${_perf_gpu}_STANDARD_REQUIRED ON)
  endif()

  # ctest only checks that every case runs, at the small --smoke sizes; it times nothing.
  add_test(NAME ${target}-smoke COMMAND $<TARGET_FILE:${target}> --smoke)
  set_tests_properties(${target}-smoke PROPERTIES TIMEOUT 300 LABELS "performance")

  list(APPEND EBGEOMETRY_PERF_PROBES ${target})
endforeach()

# Compares two sets of results (Compare.cmake builds and calls it; see README.md).
add_executable(PerfCompare Tools/PerfCompare.cpp)
target_compile_features(PerfCompare PRIVATE cxx_std_17)
set_target_properties(PerfCompare PROPERTIES SUFFIX ".ex" RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")

add_custom_target(EBGeometry_Performance DEPENDS ${EBGEOMETRY_PERF_PROBES} PerfCompare)
