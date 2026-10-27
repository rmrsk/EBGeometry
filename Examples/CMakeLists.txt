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

  # An example that its own CMakeLists.txt compiled for a GPU (following EBGEOMETRY_ENABLE_HIP/CUDA) is
  # collected in one target that the GPU CI lanes build. It still runs anywhere: without a visible GPU
  # it skips its device part.
  get_property(gpu_examples GLOBAL PROPERTY EBGEOMETRY_GPU_EXAMPLES)

  if(name IN_LIST gpu_examples)
    add_dependencies(EBGeometry_GPUDeviceExamples ${name})
  endif()

  # An example that opens the viewer (Render::view()) is run with --no-view, so ctest never waits on a
  # window, whatever the build and the display.
  file(READ ${CMAKE_CURRENT_SOURCE_DIR}/${name}/main.cpp example_source)
  string(FIND "${example_source}" "Render::view(" opens_viewer)

  set(example_arguments)

  if(NOT opens_viewer EQUAL -1)
    set(example_arguments --no-view)
  endif()

  add_test(
    NAME    ${name}
    COMMAND $<TARGET_FILE:${name}> ${example_arguments}
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

# In a GPU build, the examples that can run their queries on a device are compiled as CUDA or HIP and
# collected in this target. BuildBVH and HostTuning stay host-only: they are about the builders and the
# host-tuned types, neither of which a device uses. SmoothCSG, Transforms, ValueOperations, Repetition,
# MeshCSG and PackedSpheres do too: their tapes and unions reach a device as BasicCSG's and ManyObjects' do.
if(EBGEOMETRY_ENABLE_CUDA OR EBGEOMETRY_ENABLE_HIP)
  add_custom_target(EBGeometry_GPUDeviceExamples)
endif()

# Every folder here with a main.cpp is an example; the folders are the only list. CI loops over the
# same folders (.github/workflows/CI.yml), so adding an example needs no edit here or there.
# CONFIGURE_DEPENDS makes an existing build tree pick up a new example folder on the next build.
file(GLOB example_mains CONFIGURE_DEPENDS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*/main.cpp)
list(SORT example_mains)

foreach(example_main IN LISTS example_mains)
  get_filename_component(example ${example_main} DIRECTORY)
  ebgeometry_add_example(${example})
endforeach()
