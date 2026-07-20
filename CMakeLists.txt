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

option(EBGEOMETRY_ENABLE_HIP
  "Compile the HIP device-portability smoke test (requires a ROCm/HIP toolchain)" OFF)

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

# ── Optional GPU device-portability smoke test (CUDA or HIP) ──────────────────
# Header-only EBGeometry compiles nothing itself, so the GPU port's host/device annotations are
# exercised by building tiny offload translation units (thin .cu/.hip includers of the shared
# Tests/GPU/*.hpp bodies) with the backend compiler. This is a compile-only check: no GPU is
# required to build it. Off by default so a plain host configure never needs a GPU compiler.
if(EBGEOMETRY_ENABLE_CUDA AND EBGEOMETRY_ENABLE_HIP)
  message(FATAL_ERROR
    "EBGEOMETRY_ENABLE_CUDA and EBGEOMETRY_ENABLE_HIP are mutually exclusive: configure one "
    "GPU backend per build directory (each preset already has its own build/<preset> dir).")
endif()

if(EBGEOMETRY_ENABLE_CUDA OR EBGEOMETRY_ENABLE_HIP)
  if(EBGEOMETRY_ENABLE_CUDA)
    # The default architecture must be chosen BEFORE enable_language: CMake's compiler
    # determination otherwise caches its own detected/toolchain default first, making a later
    # if(NOT DEFINED ...) guard dead code.
    if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
      set(CMAKE_CUDA_ARCHITECTURES 70)
    endif()
    enable_language(CUDA)
    set(CMAKE_CUDA_STANDARD 17)
    set(CMAKE_CUDA_STANDARD_REQUIRED ON)
    set(EBGEOMETRY_GPU_SOURCE_EXT cu)
  else()
    # enable_language(HIP) exists from CMake 3.21; driving it with hipcc needs 3.28. The
    # project floor is 3.16 (host-only); make the HIP-specific floor explicit here.
    if(CMAKE_VERSION VERSION_LESS 3.21)
      message(FATAL_ERROR "EBGEOMETRY_ENABLE_HIP requires CMake >= 3.21 (>= 3.28 for hipcc)")
    endif()

    # Set before enable_language for the same reason as the CUDA default above; without this the
    # effective default is whatever rocm_agent_enumerator or the compiler reports (gfx906, a
    # deprecated Vega part, on a GPU-less box with the Ubuntu-archive hipcc).
    if(NOT DEFINED CMAKE_HIP_ARCHITECTURES)
      set(CMAKE_HIP_ARCHITECTURES gfx90a)
    endif()
    enable_language(HIP)
    set(CMAKE_HIP_STANDARD 17)
    set(CMAKE_HIP_STANDARD_REQUIRED ON)
    set(EBGEOMETRY_GPU_SOURCE_EXT hip)
  endif()

  add_executable(EBGeometry_GPUSmoke Tests/GPU/EBGeometry_GPUSmoke.${EBGEOMETRY_GPU_SOURCE_EXT})
  target_link_libraries(EBGeometry_GPUSmoke PRIVATE EBGeometry::EBGeometry)
  # Vec/BoundingVolumes reach for constexpr std helpers (std::numeric_limits, std::min/max) inside
  # __host__ __device__ code; --expt-relaxed-constexpr lets nvcc call those from device code. The
  # genex keeps the flag off HIP compiles, where clang's HIP semantics already allow calling
  # constexpr functions from device code without any flag.
  target_compile_options(EBGeometry_GPUSmoke PRIVATE
    $<$<COMPILE_LANGUAGE:CUDA>:--expt-relaxed-constexpr>)

  # Tier 7: device-runtime golden test (DeviceTape upload + kernel evaluation vs CPU references).
  # Compiles without a GPU (the CI GPU lanes are compile-only); at runtime it exits with code 77
  # (SKIP_RETURN_CODE below) when no GPU device is present, so ctest reports it as "Skipped".
  add_executable(EBGeometry_GPUTape Tests/GPU/EBGeometry_GPUTape.${EBGEOMETRY_GPU_SOURCE_EXT})
  target_link_libraries(EBGeometry_GPUTape PRIVATE EBGeometry::EBGeometry)
  target_compile_options(EBGeometry_GPUTape PRIVATE
    $<$<COMPILE_LANGUAGE:CUDA>:--expt-relaxed-constexpr>)

  if(EBGEOMETRY_BUILD_TESTS)
    enable_testing() # idempotent; the Tests block below also calls it
    add_test(NAME GPUTapeGolden COMMAND EBGeometry_GPUTape)
    set_tests_properties(GPUTapeGolden PROPERTIES
      LABELS "gpu"
      SKIP_RETURN_CODE 77
      TIMEOUT 120)
  endif()
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
