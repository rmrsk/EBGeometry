# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
#
# SPDX-License-Identifier: GPL-3.0-or-later

# Standalone build file for the EBGeometry_DCEL example. It can be built on its own
# (e.g. copied out as a template for a new project) or as part of the top-level
# EBGeometry build.
cmake_minimum_required(VERSION 3.16)
project(EBGeometry_DCEL CXX)

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

add_executable(EBGeometry_DCEL main.cpp)
target_link_libraries(EBGeometry_DCEL PRIVATE EBGeometry::EBGeometry)
target_compile_options(EBGeometry_DCEL PRIVATE -Wall -Wextra -Wno-unused-parameter)

# ---------------------------------------------------------------------------
# Floating-point precision for this example.
#
# This is the knob you edit to build in single or double precision: set it to
# `float` or `double`. It is handed to the compiler as EBGEOMETRY_PRECISION and
# picked up in main.cpp by `using T = EBGEOMETRY_PRECISION`.
# ---------------------------------------------------------------------------
set(EBGEOMETRY_PRECISION double)
target_compile_definitions(EBGeometry_DCEL PRIVATE EBGEOMETRY_PRECISION=${EBGEOMETRY_PRECISION})
