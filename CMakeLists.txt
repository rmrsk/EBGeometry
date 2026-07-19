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

option(EBGEOMETRY_ENABLE_CUDA
  "Compile the CUDA device-portability smoke test (requires a CUDA toolkit / nvcc)" OFF)

set(EBGEOMETRY_SIMD "avx" CACHE STRING
  "SIMD level to compile against: avx512 | avx | sse41 | none")
set_property(CACHE EBGEOMETRY_SIMD PROPERTY STRINGS avx512 avx sse41 none)

# ── Propagate options to all consumers via the interface target ───────────────
if(EBGEOMETRY_ENABLE_ASSERTIONS)
  target_compile_definitions(EBGeometry INTERFACE EBGEOMETRY_ENABLE_ASSERTIONS)
endif()

# SIMD flags are x86 host-compiler flags, so scope them to C++ compilation only ($<COMPILE_LANGUAGE:CXX>).
# This is identical for ordinary C++ consumers, but keeps -mavx/-msse4.1 from leaking onto a CUDA
# (or other offload) compile of a translation unit that links EBGeometry, where nvcc would reject them.
if(EBGEOMETRY_SIMD STREQUAL "avx512")
  set(EBGEOMETRY_SIMD_FLAGS -mavx512f -mavx2 -mavx -mfma -msse4.1)
elseif(EBGEOMETRY_SIMD STREQUAL "avx")
  set(EBGEOMETRY_SIMD_FLAGS -mavx -mfma -msse4.1)
elseif(EBGEOMETRY_SIMD STREQUAL "sse41")
  set(EBGEOMETRY_SIMD_FLAGS -msse4.1)
endif()

if(EBGEOMETRY_SIMD_FLAGS)
  target_compile_options(EBGeometry INTERFACE "$<$<COMPILE_LANGUAGE:CXX>:${EBGEOMETRY_SIMD_FLAGS}>")
endif()

# ── Optional CUDA device-portability smoke test ───────────────────────────────
# Header-only EBGeometry compiles nothing itself, so the GPU port's host/device annotations are
# exercised by building a tiny .cu translation unit with nvcc. This is a compile-only check: no GPU
# is required to build it. Off by default so a plain host configure never needs a CUDA compiler.
if(EBGEOMETRY_ENABLE_CUDA)
  enable_language(CUDA)
  set(CMAKE_CUDA_STANDARD 17)
  set(CMAKE_CUDA_STANDARD_REQUIRED ON)
  if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
    set(CMAKE_CUDA_ARCHITECTURES 70)
  endif()

  add_executable(EBGeometry_GPUSmoke Tests/GPU/EBGeometry_GPUSmoke.cu)
  target_link_libraries(EBGeometry_GPUSmoke PRIVATE EBGeometry::EBGeometry)
  # Vec/BoundingVolumes reach for constexpr std helpers (std::numeric_limits, std::min/max) inside
  # __host__ __device__ code; --expt-relaxed-constexpr lets nvcc call those from device code.
  target_compile_options(EBGeometry_GPUSmoke PRIVATE
    $<$<COMPILE_LANGUAGE:CUDA>:--expt-relaxed-constexpr>)
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
