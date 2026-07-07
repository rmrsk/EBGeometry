# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
#
# SPDX-License-Identifier: GPL-3.0-or-later

# Standalone build file for the EBGeometry_RandomCity example. It can be built on its own
# (e.g. copied out as a template for a new project) or as part of the top-level
# EBGeometry build.
cmake_minimum_required(VERSION 3.16)
project(EBGeometry_RandomCity CXX)

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

add_executable(EBGeometry_RandomCity main.cpp)

# Give the binary a .ex extension so it's covered by the repo-wide *.ex .gitignore rule,
# without needing a per-example ignore entry.
set_target_properties(EBGeometry_RandomCity PROPERTIES SUFFIX ".ex")
target_link_libraries(EBGeometry_RandomCity PRIVATE EBGeometry::EBGeometry)
target_compile_options(EBGeometry_RandomCity PRIVATE -Wall -Wextra -Wno-unused-parameter)

# Examples are meant to be fast to run, not to match the enclosing project's build
# type (which may be an unoptimised Debug build, e.g. under the top-level project's
# debug preset). Always build optimized for the local machine's ISA.
target_compile_options(EBGeometry_RandomCity PRIVATE -O3 -march=native)

# ---------------------------------------------------------------------------
# Floating-point precision for this example.
#
# This is the knob you edit to build in single or double precision: set it to
# `float` or `double`. It is handed to the compiler as EBGEOMETRY_PRECISION and
# picked up in main.cpp by `using T = EBGEOMETRY_PRECISION`.
# ---------------------------------------------------------------------------
set(EBGEOMETRY_PRECISION double)
target_compile_definitions(EBGeometry_RandomCity PRIVATE EBGEOMETRY_PRECISION=${EBGEOMETRY_PRECISION})
