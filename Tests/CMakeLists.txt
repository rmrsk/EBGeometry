include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.7.1
)
FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

# ── Helper: register one test executable ──────────────────────────────────────
function(ebgeometry_add_test name)
  add_executable(${name} ${name}.cpp)
  target_link_libraries(${name} PRIVATE EBGeometry::EBGeometry Catch2::Catch2WithMain)
  # Strong warning set for the library's real template instantiations. -Wconversion,
  # -Wsign-conversion, -Wshadow and -Wold-style-cast are deliberately not enabled yet;
  # they have a review backlog (see the narrowing-conversion clang-tidy findings).
  target_compile_options(${name} PRIVATE
    -Wall -Wextra -Wpedantic -Wcast-qual -Wcast-align -Wnon-virtual-dtor
    -Wdouble-promotion -Woverloaded-virtual -Wnull-dereference
    -Wno-unused-parameter -Werror)
  # Expose the test-data directory as a compile-time string so tests can load files.
  target_compile_definitions(${name} PRIVATE
    EBGEOMETRY_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
  )

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

# Compile-only target that explicitly instantiates every public class template so
# clang-tidy (and the strong warning set) analyse them all regardless of what the
# tests exercise. Built for its compile command, never run. Add new public classes
# to InstantiateAll.cpp so the analysis keeps full coverage.
add_library(InstantiateAll OBJECT InstantiateAll.cpp)
target_link_libraries(InstantiateAll PRIVATE EBGeometry::EBGeometry)
target_compile_options(InstantiateAll PRIVATE
  -Wall -Wextra -Wpedantic -Wcast-qual -Wcast-align -Wnon-virtual-dtor
  -Wdouble-promotion -Woverloaded-virtual -Wnull-dereference
  -Wno-unused-parameter -Werror)
