# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
#
# SPDX-License-Identifier: GPL-3.0-or-later

# Standalone build file for the CSGUnion example. It can be built on its own
# (e.g. copied out as a template for a new project) or as part of the top-level
# EBGeometry build.
cmake_minimum_required(VERSION 3.16)
project(CSGUnion CXX)

# Built on its own, the example defaults to an optimised build. Inside the EBGeometry project it
# follows that project's build type, and its SIMD flags come from the EBGeometry target
# (EBGEOMETRY_SIMD). Add e.g. -DCMAKE_CXX_FLAGS=-march=native to tune for this machine.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR AND NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Locate the header-only library. EBGEOMETRY_HOME is the directory that contains
# EBGeometry.hpp (the repository root); it defaults to the in-tree location two
# levels up. When built from the top-level EBGeometry project the
# EBGeometry::EBGeometry target already exists and is used as-is.
if(NOT TARGET EBGeometry::EBGeometry)
  set(EBGEOMETRY_HOME "${CMAKE_CURRENT_SOURCE_DIR}/../.." CACHE PATH
      "Path to the directory containing EBGeometry.hpp")
  add_library(EBGeometry::EBGeometry INTERFACE IMPORTED)
  target_include_directories(EBGeometry::EBGeometry INTERFACE "${EBGEOMETRY_HOME}")
  target_compile_features(EBGeometry::EBGeometry INTERFACE cxx_std_17)
endif()

# ---------------------------------------------------------------------------
# Where the device part runs.
#
# OFF (the default) compiles main.cpp as plain C++, which runs the example on the host. With
# -DEBGEOMETRY_EXAMPLE_GPU=HIP or CUDA it is compiled for a GPU, and the same run also does its
# queries on the device (EBGeometry::GPU::transform) and compares them with the host's; without a
# visible GPU that part is skipped. The target is CMAKE_HIP_ARCHITECTURES (default gfx90a) or
# CMAKE_CUDA_ARCHITECTURES (default 70). HIP needs CMake >= 3.21 (>= 3.28 to drive it with hipcc
# rather than clang). Inside the EBGeometry project the choice follows EBGEOMETRY_ENABLE_HIP and
# EBGEOMETRY_ENABLE_CUDA. The language is enabled before the target is created, which takes its
# architectures from these defaults.
# ---------------------------------------------------------------------------
set(EBGEOMETRY_EXAMPLE_GPU OFF CACHE STRING "Compile the example for a GPU: OFF, CUDA or HIP")
set_property(CACHE EBGEOMETRY_EXAMPLE_GPU PROPERTY STRINGS OFF CUDA HIP)

if(EBGEOMETRY_ENABLE_HIP)
  set(_example_gpu HIP)
elseif(EBGEOMETRY_ENABLE_CUDA)
  set(_example_gpu CUDA)
else()
  set(_example_gpu ${EBGEOMETRY_EXAMPLE_GPU})
endif()

if(_example_gpu STREQUAL "HIP" OR _example_gpu STREQUAL "CUDA")
  get_property(_example_languages GLOBAL PROPERTY ENABLED_LANGUAGES)

  if(NOT _example_gpu IN_LIST _example_languages)
    if(NOT DEFINED CMAKE_HIP_ARCHITECTURES)
      set(CMAKE_HIP_ARCHITECTURES gfx90a)
    endif()

    if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
      set(CMAKE_CUDA_ARCHITECTURES 70)
    endif()

    enable_language(${_example_gpu})
  endif()
elseif(NOT _example_gpu STREQUAL "OFF")
  message(FATAL_ERROR "EBGEOMETRY_EXAMPLE_GPU must be OFF, CUDA or HIP (got '${_example_gpu}')")
endif()

add_executable(CSGUnion main.cpp)

if(_example_gpu STREQUAL "HIP" OR _example_gpu STREQUAL "CUDA")
  set_source_files_properties(main.cpp PROPERTIES LANGUAGE ${_example_gpu})
  set_target_properties(CSGUnion PROPERTIES
    ${_example_gpu}_STANDARD 17
    ${_example_gpu}_STANDARD_REQUIRED ON)

  # Lets the EBGeometry project collect the examples it compiled for a GPU.
  set_property(GLOBAL APPEND PROPERTY EBGEOMETRY_GPU_EXAMPLES CSGUnion)
endif()

# Give the binary a .ex extension so it's covered by the repo-wide *.ex .gitignore rule,
# without needing a per-example ignore entry. Always place it directly in this
# source directory (not wherever -B/--preset pointed the build tree), so all three
# documented build methods (direct compile, GNU make, CMake) produce the binary in
# the same place: ./CSGUnion.ex.
set_target_properties(CSGUnion PROPERTIES
  SUFFIX ".ex"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(CSGUnion PRIVATE EBGeometry::EBGeometry)
target_compile_options(CSGUnion PRIVATE $<$<COMPILE_LANGUAGE:CXX,HIP>:-Wall -Wextra -Wno-unused-parameter>)

# ---------------------------------------------------------------------------
# Floating-point precision for this example.
#
# Cache variable, overridable at configure time with
# -DEBGEOMETRY_PRECISION=float. It is handed to the compiler as
# EBGEOMETRY_PRECISION and picked up in main.cpp by `using T = EBGEOMETRY_PRECISION`.
# ---------------------------------------------------------------------------
set(EBGEOMETRY_PRECISION double CACHE STRING "Floating-point precision (float or double)")
set_property(CACHE EBGEOMETRY_PRECISION PROPERTY STRINGS float double)
target_compile_definitions(CSGUnion PRIVATE EBGEOMETRY_PRECISION=${EBGEOMETRY_PRECISION})

# ---------------------------------------------------------------------------
# EBGEOMETRY_EXPECT() runtime assertions for this example.
#
# OFF by default. Enable at configure time with -DEBGEOMETRY_ENABLE_ASSERTIONS=ON.
# When built from the top-level EBGeometry project, this option already exists
# (defined by the root CMakeLists.txt) and its value is inherited as-is.
# ---------------------------------------------------------------------------
option(EBGEOMETRY_ENABLE_ASSERTIONS "Enable EBGEOMETRY_EXPECT() runtime assertion checks" OFF)
if(EBGEOMETRY_ENABLE_ASSERTIONS)
  target_compile_definitions(CSGUnion PRIVATE EBGEOMETRY_ENABLE_ASSERTIONS)
endif()
