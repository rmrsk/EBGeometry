# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
#
# SPDX-License-Identifier: GPL-3.0-or-later

# The comparison's build-cost tools: BuildCost, the compiler launcher that records what each compile cost,
# and BuildCompare, which compares two such records. Compare.cmake builds this folder on its own, before the
# probes (whose compiles go through BuildCost); the Performance project includes it for its test.
cmake_minimum_required(VERSION 3.16)

project(EBGeometryPerfTools LANGUAGES CXX)

foreach(tool BuildCost BuildCompare)
  add_executable(${tool} ${tool}.cpp)
  target_compile_features(${tool} PRIVATE cxx_std_17)
  set_target_properties(${tool} PROPERTIES SUFFIX ".ex" RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
endforeach()
