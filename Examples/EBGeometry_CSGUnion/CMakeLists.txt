# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
#
# SPDX-License-Identifier: GPL-3.0-or-later

# Standalone build file for the EBGeometry_CSGUnion example. It can be built on its own
# (e.g. copied out as a template for a new project) or as part of the top-level
# EBGeometry build.
cmake_minimum_required(VERSION 3.16)
project(EBGeometry_CSGUnion CXX)

# Floating-point precision used by the example (float or double).
set(EBGEOMETRY_PRECISION "double" CACHE STRING "Floating-point precision for the EBGeometry examples")
set_property(CACHE EBGEOMETRY_PRECISION PROPERTY STRINGS float double)

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

add_executable(EBGeometry_CSGUnion main.cpp)
target_link_libraries(EBGeometry_CSGUnion PRIVATE EBGeometry::EBGeometry)
target_compile_definitions(EBGeometry_CSGUnion PRIVATE EBGEOMETRY_PRECISION=${EBGEOMETRY_PRECISION})
target_compile_options(EBGeometry_CSGUnion PRIVATE -Wall -Wextra -Wno-unused-parameter)
