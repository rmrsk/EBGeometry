# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
#
# SPDX-License-Identifier: GPL-3.0-or-later

# Standalone build file for the ValueOperations example. It can be built on its own
# (e.g. copied out as a template for a new project) or as part of the top-level
# EBGeometry build.
cmake_minimum_required(VERSION 3.16)
project(ValueOperations CXX)

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
# -DEBGEOMETRY_EXAMPLE_GPU=HIP or CUDA it is compiled for a GPU, and the same run does its device part
# there: the window renders on the device, and an example with queries through EBGeometry::GPU::transform
# runs them there too; without a visible GPU that part stays on the host. The target is
# CMAKE_HIP_ARCHITECTURES (default gfx90a) or CMAKE_CUDA_ARCHITECTURES (default: the GPU in this machine,
# or 70 without one). HIP needs CMake >= 3.21 (>= 3.28 to drive it with hipcc
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

    enable_language(${_example_gpu})
  endif()
elseif(NOT _example_gpu STREQUAL "OFF")
  message(FATAL_ERROR "EBGEOMETRY_EXAMPLE_GPU must be OFF, CUDA or HIP (got '${_example_gpu}')")
endif()

add_executable(ValueOperations main.cpp)

if(_example_gpu STREQUAL "HIP" OR _example_gpu STREQUAL "CUDA")
  set_source_files_properties(main.cpp PROPERTIES LANGUAGE ${_example_gpu})
  set_target_properties(ValueOperations PROPERTIES
    ${_example_gpu}_STANDARD 17
    ${_example_gpu}_STANDARD_REQUIRED ON)

  # Lets the EBGeometry project collect the examples it compiled for a GPU.
  set_property(GLOBAL APPEND PROPERTY EBGEOMETRY_GPU_EXAMPLES ValueOperations)
endif()

# Give the binary a .ex extension so it's covered by the repo-wide *.ex .gitignore rule,
# without needing a per-example ignore entry. Always place it directly in this
# source directory (not wherever -B/--preset pointed the build tree), so all three
# documented build methods (direct compile, GNU make, CMake) produce the binary in
# the same place: ./ValueOperations.ex.
set_target_properties(ValueOperations PROPERTIES
  SUFFIX ".ex"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(ValueOperations PRIVATE EBGeometry::EBGeometry)

# The renderer draws on std::thread; older C libraries need -pthread for it.
find_package(Threads REQUIRED)
target_link_libraries(ValueOperations PRIVATE Threads::Threads)
target_compile_options(ValueOperations PRIVATE $<$<COMPILE_LANGUAGE:CXX,HIP>:-Wall -Wextra -Wno-unused-parameter>)

# ---------------------------------------------------------------------------
# Floating-point precision for this example.
#
# Cache variable, overridable at configure time with
# -DEBGEOMETRY_PRECISION=float. It is handed to the compiler as
# EBGEOMETRY_PRECISION and picked up in main.cpp by `using T = EBGEOMETRY_PRECISION`.
# ---------------------------------------------------------------------------
set(EBGEOMETRY_PRECISION double CACHE STRING "Floating-point precision (float or double)")
set_property(CACHE EBGEOMETRY_PRECISION PROPERTY STRINGS float double)
target_compile_definitions(ValueOperations PRIVATE EBGEOMETRY_PRECISION=${EBGEOMETRY_PRECISION})

# ---------------------------------------------------------------------------
# EBGEOMETRY_EXPECT() runtime assertions for this example.
#
# OFF by default. Enable at configure time with -DEBGEOMETRY_ENABLE_ASSERTIONS=ON.
# When built from the top-level EBGeometry project, this option already exists
# (defined by the root CMakeLists.txt) and its value is inherited as-is.
# ---------------------------------------------------------------------------
option(EBGEOMETRY_ENABLE_ASSERTIONS "Enable EBGEOMETRY_EXPECT() runtime assertion checks" OFF)
if(EBGEOMETRY_ENABLE_ASSERTIONS)
  target_compile_definitions(ValueOperations PRIVATE EBGEOMETRY_ENABLE_ASSERTIONS)
endif()

# ---------------------------------------------------------------------------
# The interactive viewer (Render::view() at the end of main()).
#
# OFF by default: view() then prints a line and returns. -DEBGEOMETRY_VIEWER=SDL2 opens a window on what the
# example builds, which needs SDL2 (e.g. libsdl2-dev). When built from the top-level EBGeometry project, this
# option already exists there and the EBGeometry target carries it.
# ---------------------------------------------------------------------------
set(EBGEOMETRY_VIEWER OFF CACHE STRING "Interactive viewer for Render::view(): OFF or SDL2")
set_property(CACHE EBGEOMETRY_VIEWER PROPERTY STRINGS OFF SDL2)

if(EBGEOMETRY_VIEWER STREQUAL "SDL2" AND CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  find_package(SDL2 REQUIRED)
  target_compile_definitions(ValueOperations PRIVATE EBGEOMETRY_VIEWER_SDL2 $<$<COMPILE_LANGUAGE:CUDA>:SDL_DISABLE_IMMINTRIN_H>)
  target_link_libraries(ValueOperations PRIVATE SDL2::SDL2)
elseif(NOT EBGEOMETRY_VIEWER STREQUAL "OFF" AND NOT EBGEOMETRY_VIEWER STREQUAL "SDL2")
  message(FATAL_ERROR "EBGEOMETRY_VIEWER must be OFF or SDL2 (got '${EBGEOMETRY_VIEWER}')")
endif()
