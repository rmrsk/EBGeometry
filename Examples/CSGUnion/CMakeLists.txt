# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
#
# SPDX-License-Identifier: GPL-3.0-or-later

# Standalone build file for the CSGUnion example. It can be built on its own
# (e.g. copied out as a template for a new project) or as part of the top-level
# EBGeometry build.
cmake_minimum_required(VERSION 3.16)
project(CSGUnion CXX)

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

add_executable(CSGUnion main.cpp)

# Give the binary a .ex extension so it's covered by the repo-wide *.ex .gitignore rule,
# without needing a per-example ignore entry. Always place it directly in this
# source directory (not wherever -B/--preset pointed the build tree), so all three
# documented build methods (direct compile, GNU make, CMake) produce the binary in
# the same place: ./CSGUnion.ex.
set_target_properties(CSGUnion PROPERTIES
  SUFFIX ".ex"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(CSGUnion PRIVATE EBGeometry::EBGeometry)
target_compile_options(CSGUnion PRIVATE -Wall -Wextra -Wno-unused-parameter)

# Examples are meant to be fast to run, not to match the enclosing project's build
# type (which may be an unoptimised Debug build, e.g. under the top-level project's
# debug preset). Always build optimized for the local machine's ISA.
target_compile_options(CSGUnion PRIVATE -O3 -march=native)

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
