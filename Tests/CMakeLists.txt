include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.7.1
)
FetchContent_MakeAvailable(Catch2)

# Catch2's WithinAbs/WithinRel matchers always operate on `double` internally
# (MatcherBase<double>), regardless of which type-specific overload built them -- so
# REQUIRE_THAT(<float value>, WithinRel(...)/WithinAbs(...)) triggers an implicit
# float->double promotion *inside Catch2's own header*, which -Wdouble-promotion flags under
# Clang (not GCC) once EBGEOMETRY_TEST_BOTH_PRECISIONS actually instantiates a TEMPLATE_TEST_CASE
# with T=float. This isn't a bug in EBGeometry's own code, so mark Catch2's headers as system
# includes (-isystem) instead of chasing every call site.
foreach(_ebgeometry_catch2_target Catch2 Catch2WithMain)
  get_target_property(_ebgeometry_catch2_includes ${_ebgeometry_catch2_target} INTERFACE_INCLUDE_DIRECTORIES)
  set_target_properties(${_ebgeometry_catch2_target} PROPERTIES
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${_ebgeometry_catch2_includes}")
endforeach()

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

# Tests written with TEMPLATE_TEST_CASE(..., EBGEOMETRY_TEST_PRECISIONS) (see
# TestFloatingPointUtils.hpp) instantiate over `double` only by default, for fast local
# iteration. CI turns this on to additionally verify `float` behaviorally (not just that it
# compiles, via InstantiateAll.cpp) -- see the Unit-Tests/Sanitizers/Release-Test jobs in
# .github/workflows/CI.yml.
option(EBGEOMETRY_TEST_BOTH_PRECISIONS "Run the unit test suite under both float and double" OFF)

# ── Helper: register one test executable ──────────────────────────────────────
function(ebgeometry_add_test name)
  add_executable(${name} ${name}.cpp)
  target_link_libraries(${name} PRIVATE EBGeometry::EBGeometry Catch2::Catch2WithMain)
  # Strong warning set for the library's real template instantiations. -Wconversion,
  # -Wsign-conversion, -Wshadow and -Wold-style-cast are deliberately not enabled yet;
  # they have a review backlog (see the narrowing-conversion clang-tidy findings).
  # Gated to CXX so that a test compiled as CUDA/HIP (the GPU device blocks below) does not
  # forward these host-compiler flags straight to nvcc/hipcc, which do not understand them.
  target_compile_options(${name} PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-Wall;-Wextra;-Wpedantic;-Wcast-qual;-Wcast-align;-Wnon-virtual-dtor;-Wdouble-promotion;-Woverloaded-virtual;-Wnull-dereference;-Wno-unused-parameter;-Werror>
    # GCC's -Wnull-dereference has confirmed false positives in heavily-inlined
    # shared_ptr/Vec3T template code that only surface at -O2/-O3 (Release);
    # Clang does not warn on the same code at the same optimisation level. Not an
    # issue at -O0 (Debug), so only suppressed for Release-type configs.
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:Release>>:-Wno-null-dereference>)
  # Expose the test-data directory as a compile-time string so tests can load files.
  target_compile_definitions(${name} PRIVATE
    EBGEOMETRY_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
  )

  if(EBGEOMETRY_TEST_BOTH_PRECISIONS)
    target_compile_definitions(${name} PRIVATE EBGEOMETRY_TEST_BOTH_PRECISIONS)
  endif()

  if(EBGEOMETRY_ENABLE_SANITIZERS)
    target_compile_options(${name} PRIVATE ${EBGEOMETRY_SANITIZER_COMPILE_OPTIONS})
    target_link_options(${name} PRIVATE -fsanitize=address,undefined)
  endif()

  catch_discover_tests(${name} TEST_SPEC "*" PROPERTIES LABELS "unit")
endfunction()

ebgeometry_add_test(TestVec)
ebgeometry_add_test(TestMath)
ebgeometry_add_test(TestBoundingVolumes)
ebgeometry_add_test(TestAnalyticSDF)
ebgeometry_add_test(TestDCEL)
ebgeometry_add_test(TestSFC)
ebgeometry_add_test(TestTriangle)
ebgeometry_add_test(TestPolygonSoup)
ebgeometry_add_test(TestParser)
ebgeometry_add_test(TestBVH)
ebgeometry_add_test(TestCSG)
ebgeometry_add_test(TestTransform)
ebgeometry_add_test(TestTape)
ebgeometry_add_test(TestFunctionQueries)
ebgeometry_add_test(TestPolygon2D)
ebgeometry_add_test(TestTriangleSoA)
ebgeometry_add_test(TestPointSoA)
ebgeometry_add_test(TestPointAoSoA)
ebgeometry_add_test(TestPointCloud)
ebgeometry_add_test(TestPointCloudBVH)
ebgeometry_add_test(TestPointCloudHashGrid)
ebgeometry_add_test(TestSimpleTimer)
ebgeometry_add_test(TestRandom)
ebgeometry_add_test(TestMemoryResource)
ebgeometry_add_test(TestPool)
ebgeometry_add_test(TestPODVector)
ebgeometry_add_test(TestPoolRebase)

# ── SIMD settings checked across translation units ────────────────────────────
# Programs of two translation units built with different settings, for the startup check in
# EBGeometry_SIMDCheck.hpp:
#   Mixed   -- different SIMD flags, without EBGEOMETRY_HOST_TUNED_DEFAULTS: one warning, and main()
#              runs;
#   Allowed -- the same, with EBGEOMETRY_ALLOW_MIXED_SIMD: no warning;
#   Layout  -- EBGEOMETRY_HOST_TUNED_DEFAULTS in one unit only: aborts before main();
#   LayoutFlags -- EBGEOMETRY_HOST_TUNED_DEFAULTS in both, different SIMD flags: aborts too.
# The per-source options come after the build's own SIMD flags and definitions, so they win. x86 and
# host-only builds only, since the flags and the host-tuned defaults are.
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|i[3-6]86" AND NOT (EBGEOMETRY_ENABLE_CUDA OR EBGEOMETRY_ENABLE_HIP))
  foreach(_ebgeometry_simd_check Mixed Allowed Layout LayoutFlags)
    set(_ebgeometry_target TestSIMDCheck${_ebgeometry_simd_check})
    set(_ebgeometry_main ${CMAKE_CURRENT_BINARY_DIR}/${_ebgeometry_target}Main.cpp)
    set(_ebgeometry_other ${CMAKE_CURRENT_BINARY_DIR}/${_ebgeometry_target}Other.cpp)

    # Copies of the two sources per program, since per-source options apply to a source file in
    # every target that uses it.
    configure_file(TestSIMDCheckMain.cpp ${_ebgeometry_main} COPYONLY)
    configure_file(TestSIMDCheckOther.cpp ${_ebgeometry_other} COPYONLY)

    add_executable(${_ebgeometry_target} ${_ebgeometry_main} ${_ebgeometry_other})
    target_link_libraries(${_ebgeometry_target} PRIVATE EBGeometry::EBGeometry)

    if(_ebgeometry_simd_check STREQUAL "Layout")
      set_source_files_properties(${_ebgeometry_main} PROPERTIES COMPILE_DEFINITIONS EBGEOMETRY_HOST_TUNED_DEFAULTS)
      set_source_files_properties(${_ebgeometry_other} PROPERTIES COMPILE_OPTIONS "-UEBGEOMETRY_HOST_TUNED_DEFAULTS")
    elseif(_ebgeometry_simd_check STREQUAL "LayoutFlags")
      set_source_files_properties(${_ebgeometry_main} PROPERTIES
        COMPILE_DEFINITIONS EBGEOMETRY_HOST_TUNED_DEFAULTS COMPILE_OPTIONS "-mno-sse4.1")
      set_source_files_properties(${_ebgeometry_other} PROPERTIES
        COMPILE_DEFINITIONS EBGEOMETRY_HOST_TUNED_DEFAULTS COMPILE_OPTIONS "-msse4.1;-mno-avx")
    else()
      # Without the host-tuned defaults, which the build may otherwise define for every target.
      set_source_files_properties(${_ebgeometry_main} PROPERTIES COMPILE_OPTIONS "-UEBGEOMETRY_HOST_TUNED_DEFAULTS;-mno-sse4.1")
      set_source_files_properties(${_ebgeometry_other} PROPERTIES
        COMPILE_OPTIONS "-UEBGEOMETRY_HOST_TUNED_DEFAULTS;-msse4.1;-mno-avx")
    endif()

    if(_ebgeometry_simd_check STREQUAL "Allowed")
      target_compile_definitions(${_ebgeometry_target} PRIVATE EBGEOMETRY_ALLOW_MIXED_SIMD)
    endif()

    if(EBGEOMETRY_ENABLE_SANITIZERS)
      target_compile_options(${_ebgeometry_target} PRIVATE ${EBGEOMETRY_SANITIZER_COMPILE_OPTIONS})
      target_link_options(${_ebgeometry_target} PRIVATE -fsanitize=address,undefined)
    endif()

    if(_ebgeometry_simd_check MATCHES "^Layout")
      # CTest counts a program killed by a signal as failed whatever PASS_REGULAR_EXPRESSION says.
      add_test(NAME ${_ebgeometry_target}
        COMMAND ${CMAKE_COMMAND} -DPROGRAM=$<TARGET_FILE:${_ebgeometry_target}>
                "-DMESSAGE=EBGeometry: translation units disagree on the settings that set type layouts"
                -P ${CMAKE_CURRENT_SOURCE_DIR}/ExpectAbortMessage.cmake)
    else()
      add_test(NAME ${_ebgeometry_target} COMMAND ${_ebgeometry_target})
      set_tests_properties(${_ebgeometry_target} PROPERTIES PASS_REGULAR_EXPRESSION "main\\(\\) reached")
    endif()

    set_tests_properties(${_ebgeometry_target} PROPERTIES LABELS "unit")
  endforeach()

  # PASS_REGULAR_EXPRESSION matches any one of its expressions, so the warning is checked with a
  # second test property: Mixed fails without it, Allowed fails with it.
  set_tests_properties(TestSIMDCheckMixed PROPERTIES
    PASS_REGULAR_EXPRESSION "EBGeometry warning: translation units were compiled with different SIMD flags(.|\n)*main\\(\\) reached")
  set_tests_properties(TestSIMDCheckAllowed PROPERTIES FAIL_REGULAR_EXPRESSION "EBGeometry warning")
endif()

# ── GPU device blocks in the per-class tests ──────────────────────────────────
# Several per-class test files carry a [gpu]-tagged device check (see TestGPU.hpp). On an ordinary
# host build those checks run in emulation: the "device" is host memory that reports itself
# device-accessible, and the kernel is a host loop. When a GPU backend is enabled we compile just
# those files with the backend compiler (per-file LANGUAGE) so the checks run as real kernels.
if(EBGEOMETRY_ENABLE_CUDA OR EBGEOMETRY_ENABLE_HIP)
  set(EBGEOMETRY_GPU_TESTS TestVec TestBoundingVolumes TestPool TestPointAoSoA TestTriangleSoA TestDCEL TestBVH TestPointCloudBVH TestPointCloudHashGrid TestAnalyticSDF TestCSG TestFunctionQueries TestTape)

  foreach(_ebgeometry_gpu_test ${EBGEOMETRY_GPU_TESTS})
    if(EBGEOMETRY_ENABLE_CUDA)
      # Deliberately without --expt-relaxed-constexpr: the library does not require it of its users
      # (Scripts/CheckDeviceMath.py), and building the tests without it is what checks that.
      set_source_files_properties(${_ebgeometry_gpu_test}.cpp PROPERTIES LANGUAGE CUDA)
    else()
      set_source_files_properties(${_ebgeometry_gpu_test}.cpp PROPERTIES LANGUAGE HIP)
    endif()

    # Re-register just this binary's [gpu]-tagged Catch2 cases (every device-portability check
    # across these files carries that tag, regardless of its own name) under a "gpu:"-prefixed name
    # and a "gpu-device" CTest label, alongside the normal "unit"-labeled registration from
    # ebgeometry_add_test() above. This lets CI run exactly the device-callable surface via
    # `ctest -L gpu-device` without duplicating this file's EBGEOMETRY_GPU_TESTS list in the
    # workflow YAML. On a runner with no physical GPU these SKIP() cleanly (see TestGPU.hpp's
    # deviceAvailable()) rather than failing; the host builds have already run them in emulation.
    catch_discover_tests(${_ebgeometry_gpu_test}
      TEST_SPEC "[gpu]"
      TEST_PREFIX "gpu:"
      PROPERTIES LABELS "gpu-device")
  endforeach()

  # Aggregate target so the GPU CI lanes can build every device-bearing test with one stable name,
  # keeping the device-test list here as the single source of truth rather than duplicating it in CI.
  add_custom_target(EBGeometry_GPUDeviceTests)
  add_dependencies(EBGeometry_GPUDeviceTests ${EBGEOMETRY_GPU_TESTS})
endif()

# Compile-only target that explicitly instantiates every public class template so
# clang-tidy (and the strong warning set) analyse them all regardless of what the
# tests exercise. Built for its compile command, never run. Add new public classes
# to InstantiateAll.cpp so the analysis keeps full coverage.
add_library(InstantiateAll OBJECT InstantiateAll.cpp)
target_link_libraries(InstantiateAll PRIVATE EBGeometry::EBGeometry)
target_compile_options(InstantiateAll PRIVATE
  -Wall -Wextra -Wpedantic -Wcast-qual -Wcast-align -Wnon-virtual-dtor
  -Wdouble-promotion -Woverloaded-virtual -Wnull-dereference
  -Wno-unused-parameter -Werror
  # See the matching comment in ebgeometry_add_test() above.
  $<$<CONFIG:Release>:-Wno-null-dereference>)
