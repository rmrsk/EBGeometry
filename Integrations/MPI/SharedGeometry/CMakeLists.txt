# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
#
# SPDX-License-Identifier: BSD-3-Clause

# Standalone build file for the SharedGeometry MPI integration. It needs an MPI installation (find_package(MPI)) and
# is not part of the top-level EBGeometry build.
cmake_minimum_required(VERSION 3.16)
project(SharedGeometry CXX)

# An optimized build unless asked otherwise.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# EBGEOMETRY_HOME is the directory that contains EBGeometry.hpp (the repository root); it defaults to the in-tree
# location three levels up.
set(EBGEOMETRY_HOME "${CMAKE_CURRENT_SOURCE_DIR}/../../.." CACHE PATH "Path to the directory containing EBGeometry.hpp")

find_package(MPI REQUIRED COMPONENTS CXX)

add_executable(SharedGeometry main.cpp)

# The binary is SharedGeometry.ex in this folder, as the GNUmakefile makes it (covered by the repository's *.ex
# .gitignore rule).
set_target_properties(SharedGeometry PROPERTIES
  SUFFIX ".ex"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_features(SharedGeometry PRIVATE cxx_std_17)
target_include_directories(SharedGeometry PRIVATE "${EBGEOMETRY_HOME}")
target_link_libraries(SharedGeometry PRIVATE MPI::MPI_CXX)
target_compile_options(SharedGeometry PRIVATE -Wall -Wextra -Wno-unused-parameter)

# Floating-point precision (float or double), handed to main.cpp as EBGEOMETRY_PRECISION.
set(EBGEOMETRY_PRECISION double CACHE STRING "Floating-point precision (float or double)")
set_property(CACHE EBGEOMETRY_PRECISION PROPERTY STRINGS float double)
target_compile_definitions(SharedGeometry PRIVATE EBGEOMETRY_PRECISION=${EBGEOMETRY_PRECISION})

# EBGEOMETRY_EXPECT() runtime assertions, OFF by default.
option(EBGEOMETRY_ENABLE_ASSERTIONS "Enable EBGEOMETRY_EXPECT() runtime assertion checks" OFF)
if(EBGEOMETRY_ENABLE_ASSERTIONS)
  target_compile_definitions(SharedGeometry PRIVATE EBGEOMETRY_ENABLE_ASSERTIONS)
endif()
