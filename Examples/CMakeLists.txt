# Helper: add one EBGeometry standalone example.
#
# The binary is run with its source directory as the working directory so that
# the hardcoded "../Resources/" relative paths in each example resolve correctly.
function(ebgeometry_add_example name)
  add_executable(${name} ${name}/main.cpp)
  target_link_libraries(${name} PRIVATE EBGeometry::EBGeometry)
  target_compile_options(${name} PRIVATE -Wall -Wextra -Wno-unused-parameter)

  if(EBGEOMETRY_ENABLE_SANITIZERS)
    target_compile_options(${name} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(${name} PRIVATE -fsanitize=address,undefined)
  endif()

  add_test(
    NAME    ${name}
    COMMAND $<TARGET_FILE:${name}>
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${name}
  )
  # Examples load large mesh files and run in unoptimised debug builds; allow
  # up to 5 minutes each, and tag them so they can be run separately from the
  # fast unit-test suite (ctest -L example).
  set_tests_properties(${name} PROPERTIES
    TIMEOUT 300
    LABELS  "example"
  )
endfunction()

ebgeometry_add_example(EBGeometry_DCEL)
ebgeometry_add_example(EBGeometry_F18)
ebgeometry_add_example(EBGeometry_OctreeBoundingVolume)
ebgeometry_add_example(EBGeometry_PackedSpheres)
ebgeometry_add_example(EBGeometry_RandomCity)
ebgeometry_add_example(EBGeometry_Shapes)
