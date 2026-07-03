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
  target_compile_options(${name} PRIVATE -Wall -Wextra -Wno-unused-parameter)
  # Expose the test-data directory as a compile-time string so tests can load files.
  target_compile_definitions(${name} PRIVATE
    EBGEOMETRY_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
  )
  catch_discover_tests(${name})
endfunction()

ebgeometry_add_test(TestVec)
ebgeometry_add_test(TestBoundingVolumes)
ebgeometry_add_test(TestAnalyticSDF)
ebgeometry_add_test(TestDCEL)
ebgeometry_add_test(TestSFC)
