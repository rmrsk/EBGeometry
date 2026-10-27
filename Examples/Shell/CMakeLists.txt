# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
#
# SPDX-License-Identifier: GPL-3.0-or-later

# Standalone build file for the Shell example. It can be built on its own
# (e.g. copied out as a template for a new project) or as part of the top-level
# EBGeometry build.
cmake_minimum_required(VERSION 3.16)
project(Shell CXX)

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

add_executable(Shell main.cpp)

# Give the binary a .ex extension so it's covered by the repo-wide *.ex .gitignore rule,
# without needing a per-example ignore entry. Always place it directly in this
# source directory (not wherever -B/--preset pointed the build tree), so all three
# documented build methods (direct compile, GNU make, CMake) produce the binary in
# the same place: ./Shell.ex.
set_target_properties(Shell PROPERTIES
  SUFFIX ".ex"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(Shell PRIVATE EBGeometry::EBGeometry)

# The renderer draws on std::thread; older C libraries need -pthread for it.
find_package(Threads REQUIRED)
target_link_libraries(Shell PRIVATE Threads::Threads)
target_compile_options(Shell PRIVATE -Wall -Wextra -Wno-unused-parameter)


# ---------------------------------------------------------------------------
# Floating-point precision for this example.
#
# Cache variable, overridable at configure time with
# -DEBGEOMETRY_PRECISION=float. It is handed to the compiler as
# EBGEOMETRY_PRECISION and picked up in main.cpp by `using T = EBGEOMETRY_PRECISION`.
# ---------------------------------------------------------------------------
set(EBGEOMETRY_PRECISION double CACHE STRING "Floating-point precision (float or double)")
set_property(CACHE EBGEOMETRY_PRECISION PROPERTY STRINGS float double)
target_compile_definitions(Shell PRIVATE EBGEOMETRY_PRECISION=${EBGEOMETRY_PRECISION})

# ---------------------------------------------------------------------------
# EBGEOMETRY_EXPECT() runtime assertions for this example.
#
# OFF by default. Enable at configure time with -DEBGEOMETRY_ENABLE_ASSERTIONS=ON.
# When built from the top-level EBGeometry project, this option already exists
# (defined by the root CMakeLists.txt) and its value is inherited as-is.
# ---------------------------------------------------------------------------
option(EBGEOMETRY_ENABLE_ASSERTIONS "Enable EBGEOMETRY_EXPECT() runtime assertion checks" OFF)
if(EBGEOMETRY_ENABLE_ASSERTIONS)
  target_compile_definitions(Shell PRIVATE EBGEOMETRY_ENABLE_ASSERTIONS)
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
  target_compile_definitions(Shell PRIVATE EBGEOMETRY_VIEWER_SDL2 $<$<COMPILE_LANGUAGE:CUDA>:SDL_DISABLE_IMMINTRIN_H>)
  target_link_libraries(Shell PRIVATE SDL2::SDL2)
elseif(NOT EBGEOMETRY_VIEWER STREQUAL "OFF" AND NOT EBGEOMETRY_VIEWER STREQUAL "SDL2")
  message(FATAL_ERROR "EBGEOMETRY_VIEWER must be OFF or SDL2 (got '${EBGEOMETRY_VIEWER}')")
endif()
