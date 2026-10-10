# Helper: add one EBGeometry standalone example.
#
# Each example carries its own CMakeLists.txt so it can also be built on its own
# (see Examples/*/CMakeLists.txt). Here we pull it in as a subdirectory
# and layer on the repo-side concerns: optional sanitizers and test registration.
#
# The binary is run with its source directory as the working directory so that
# relative mesh paths (e.g. "../../common-3d-test-models/data/*.obj")
# in each example resolve correctly.
function(ebgeometry_add_example name)
  add_subdirectory(${name})

  if(EBGEOMETRY_ENABLE_SANITIZERS)
    target_compile_options(${name} PRIVATE ${EBGEOMETRY_SANITIZER_COMPILE_OPTIONS})
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

# Every folder here with a main.cpp is an example; the folders are the only list. CI loops over the
# same folders (.github/workflows/CI.yml), so adding an example needs no edit here or there.
file(GLOB example_mains RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*/main.cpp)
list(SORT example_mains)

foreach(example_main IN LISTS example_mains)
  get_filename_component(example ${example_main} DIRECTORY)
  ebgeometry_add_example(${example})
endforeach()
