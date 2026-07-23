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
    target_compile_options(${name} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(${name} PRIVATE -fsanitize=address,undefined)
  endif()

  catch_discover_tests(${name} TEST_SPEC "*" PROPERTIES LABELS "unit")
endfunction()

ebgeometry_add_test(TestVec)
ebgeometry_add_test(TestBoundingVolumes)
ebgeometry_add_test(TestAnalyticSDF)
ebgeometry_add_test(TestDCEL)
ebgeometry_add_test(TestSFC)
ebgeometry_add_test(TestTriangle)
ebgeometry_add_test(TestSTL)
ebgeometry_add_test(TestPLY)
ebgeometry_add_test(TestOBJ)
ebgeometry_add_test(TestVTK)
ebgeometry_add_test(TestBVH)
ebgeometry_add_test(TestCSG)
ebgeometry_add_test(TestTransform)
ebgeometry_add_test(TestOctree)
ebgeometry_add_test(TestPolygon2D)
ebgeometry_add_test(TestTriangleSoA)
ebgeometry_add_test(TestPointSoA)
ebgeometry_add_test(TestPointAoSoA)
ebgeometry_add_test(TestPointCloudBVH)
ebgeometry_add_test(TestPointCloudHashGrid)
ebgeometry_add_test(TestSimpleTimer)
ebgeometry_add_test(TestRandom)
ebgeometry_add_test(TestMemoryResource)
ebgeometry_add_test(TestPool)
ebgeometry_add_test(TestPODVector)
ebgeometry_add_test(TestPoolRebase)

# ── GPU device blocks in the per-class tests ──────────────────────────────────
# Several per-class test files carry a device-portability check behind an
# #if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP) guard (see TestGPU.hpp). On an ordinary
# host build those blocks preprocess away and the files compile as plain C++ like any other test.
# When a GPU backend is enabled we compile just those files with the backend compiler (per-file
# LANGUAGE) so the guarded device kernels are built and, on hardware, run.
if(EBGEOMETRY_ENABLE_CUDA OR EBGEOMETRY_ENABLE_HIP)
  set(EBGEOMETRY_GPU_TESTS TestVec TestBoundingVolumes TestPool TestPointAoSoA TestTriangleSoA)

  foreach(_ebgeometry_gpu_test ${EBGEOMETRY_GPU_TESTS})
    if(EBGEOMETRY_ENABLE_CUDA)
      set_source_files_properties(${_ebgeometry_gpu_test}.cpp PROPERTIES LANGUAGE CUDA)
      # constexpr std helpers (std::array, ...) reached inside __host__ __device__ code need this on nvcc.
      target_compile_options(${_ebgeometry_gpu_test} PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:--expt-relaxed-constexpr>)
    else()
      set_source_files_properties(${_ebgeometry_gpu_test}.cpp PROPERTIES LANGUAGE HIP)
    endif()
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
