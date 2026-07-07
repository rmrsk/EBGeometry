cmake_minimum_required(VERSION 3.16)
project(EBGeometry VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Interface / header-only library target ────────────────────────────────────
add_library(EBGeometry INTERFACE)
add_library(EBGeometry::EBGeometry ALIAS EBGeometry)
target_include_directories(EBGeometry INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)

# ── Options ───────────────────────────────────────────────────────────────────
option(EBGEOMETRY_BUILD_TESTS
  "Build the EBGeometry unit-test suite (requires Catch2, fetched automatically)" OFF)

option(EBGEOMETRY_BUILD_EXAMPLES
  "Build the standalone EBGeometry examples" OFF)

option(EBGEOMETRY_ENABLE_ASSERTIONS
  "Enable EBGEOMETRY_EXPECT() runtime assertion checks" OFF)

option(EBGEOMETRY_ENABLE_SANITIZERS
  "Enable AddressSanitizer and UndefinedBehaviourSanitizer for tests and examples" OFF)

set(EBGEOMETRY_SIMD "avx" CACHE STRING
  "SIMD level to compile against: avx512 | avx | sse41 | none")
set_property(CACHE EBGEOMETRY_SIMD PROPERTY STRINGS avx512 avx sse41 none)

# ── Propagate options to all consumers via the interface target ───────────────
if(EBGEOMETRY_ENABLE_ASSERTIONS)
  target_compile_definitions(EBGeometry INTERFACE EBGEOMETRY_ENABLE_ASSERTIONS)
endif()

if(EBGEOMETRY_SIMD STREQUAL "avx512")
  target_compile_options(EBGeometry INTERFACE -mavx512f -mavx2 -mavx -mfma -msse4.1)
elseif(EBGEOMETRY_SIMD STREQUAL "avx")
  target_compile_options(EBGeometry INTERFACE -mavx -mfma -msse4.1)
elseif(EBGEOMETRY_SIMD STREQUAL "sse41")
  target_compile_options(EBGeometry INTERFACE -msse4.1)
endif()

# ── Tests ─────────────────────────────────────────────────────────────────────
if(EBGEOMETRY_BUILD_TESTS)
  enable_testing()
  add_subdirectory(Tests)
endif()

# ── Examples ──────────────────────────────────────────────────────────────────
if(EBGEOMETRY_BUILD_EXAMPLES)
  if(NOT EBGEOMETRY_BUILD_TESTS)
    # enable_testing is needed for add_test even when tests are not built
    enable_testing()
  endif()
  add_subdirectory(Examples)
endif()
