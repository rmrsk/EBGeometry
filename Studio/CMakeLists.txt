# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
#
# SPDX-License-Identifier: GPL-3.0-or-later

# ebgstudio, the command line of the .ebg language. It builds on its own, here in Studio/ (the binary is
# Studio/ebgstudio, as the GNUmakefile builds it), or as part of the EBGeometry project with
# EBGEOMETRY_BUILD_STUDIO (on whenever the tests are built), which installs it with the library.
cmake_minimum_required(VERSION 3.16)
project(ebgstudio CXX)

# Whether it is built on its own, not from the EBGeometry project.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(_studio_alone ON)
else()
  set(_studio_alone OFF)
endif()

# On its own it defaults to an optimized build.
if(_studio_alone AND NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# The header-only library: the EBGeometry project's target, or the directory that contains EBGeometry.hpp
# (EBGEOMETRY_HOME, by default the repository root one level up).
if(NOT TARGET EBGeometry::EBGeometry)
  set(EBGEOMETRY_HOME "${CMAKE_CURRENT_SOURCE_DIR}/.." CACHE PATH "Path to the directory containing EBGeometry.hpp")
  add_library(EBGeometry::EBGeometry INTERFACE IMPORTED)
  target_include_directories(EBGeometry::EBGeometry INTERFACE "${EBGEOMETRY_HOME}")
  target_compile_features(EBGeometry::EBGeometry INTERFACE cxx_std_17)
endif()

# Where render and view draw: on its own, EBGEOMETRY_STUDIO_GPU (OFF, CUDA or HIP; CMAKE_CUDA_ARCHITECTURES by
# default the GPU in this machine, CMAKE_HIP_ARCHITECTURES gfx90a); in the EBGeometry project, its
# EBGEOMETRY_ENABLE_CUDA or EBGEOMETRY_ENABLE_HIP.
set(EBGEOMETRY_STUDIO_GPU OFF CACHE STRING "Compile ebgstudio for a GPU: OFF, CUDA or HIP")
set_property(CACHE EBGEOMETRY_STUDIO_GPU PROPERTY STRINGS OFF CUDA HIP)

if(EBGEOMETRY_ENABLE_HIP)
  set(_studio_gpu HIP)
elseif(EBGEOMETRY_ENABLE_CUDA)
  set(_studio_gpu CUDA)
else()
  set(_studio_gpu ${EBGEOMETRY_STUDIO_GPU})
endif()

if(_studio_gpu STREQUAL "HIP" OR _studio_gpu STREQUAL "CUDA")
  get_property(_studio_languages GLOBAL PROPERTY ENABLED_LANGUAGES)

  if(NOT _studio_gpu IN_LIST _studio_languages)
    if(NOT DEFINED CMAKE_HIP_ARCHITECTURES)
      set(CMAKE_HIP_ARCHITECTURES gfx90a)
    endif()

    if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
      # The GPU in this machine, so that the binary carries its machine code; 70 when none is visible.
      execute_process(COMMAND nvidia-smi --query-gpu=compute_cap --format=csv,noheader
                      OUTPUT_VARIABLE _cuda_cap RESULT_VARIABLE _cuda_smi ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
      string(REGEX MATCH "^[0-9]+\\.[0-9]+" _cuda_cap "${_cuda_cap}")

      if(_cuda_smi EQUAL 0 AND _cuda_cap)
        string(REPLACE "." "" CMAKE_CUDA_ARCHITECTURES "${_cuda_cap}")
      else()
        set(CMAKE_CUDA_ARCHITECTURES 70)
      endif()
    endif()

    enable_language(${_studio_gpu})
  endif()
elseif(NOT _studio_gpu STREQUAL "OFF")
  message(FATAL_ERROR "EBGEOMETRY_STUDIO_GPU must be OFF, CUDA or HIP (got '${_studio_gpu}')")
endif()

add_executable(ebgstudio main.cpp)
target_link_libraries(ebgstudio PRIVATE EBGeometry::EBGeometry)

if(_studio_gpu STREQUAL "HIP" OR _studio_gpu STREQUAL "CUDA")
  set_source_files_properties(main.cpp PROPERTIES LANGUAGE ${_studio_gpu})
  set_target_properties(ebgstudio PROPERTIES ${_studio_gpu}_STANDARD 17 ${_studio_gpu}_STANDARD_REQUIRED ON)
endif()

# On its own, the binary goes here in Studio/, where the GNUmakefile puts it too.
if(_studio_alone)
  set_target_properties(ebgstudio PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

# The renderer draws on std::thread; older C libraries need -pthread for it.
find_package(Threads REQUIRED)
target_link_libraries(ebgstudio PRIVATE Threads::Threads)

set(EBGEOMETRY_STUDIO_PRECISION float CACHE STRING "Precision ebgstudio builds models in (float or double)")
set_property(CACHE EBGEOMETRY_STUDIO_PRECISION PROPERTY STRINGS float double)
target_compile_definitions(ebgstudio PRIVATE EBGEOMETRY_STUDIO_PRECISION=${EBGEOMETRY_STUDIO_PRECISION})

# Warnings fail the build only with the tests (CI and development): an install with a newer compiler warns.
if(EBGEOMETRY_BUILD_TESTS)
  target_compile_options(ebgstudio PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-Wall;-Wextra;-Wpedantic;-Wdouble-promotion;-Werror>)
else()
  target_compile_options(ebgstudio PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-Wall;-Wextra>)
endif()

if(EBGEOMETRY_ENABLE_SANITIZERS)
  target_compile_options(ebgstudio PRIVATE ${EBGEOMETRY_SANITIZER_COMPILE_OPTIONS})
  target_link_options(ebgstudio PRIVATE -fsanitize=address,undefined)
endif()

# The window of `ebgstudio view`: in the EBGeometry project the EBGeometry target carries EBGEOMETRY_VIEWER; on its
# own, EBGEOMETRY_VIEWER is SDL2 when SDL2 is found (OFF builds no window: view says so and exits 1).
if(_studio_alone)
  find_package(SDL2 QUIET)

  if(SDL2_FOUND)
    set(_studio_viewer SDL2)
  else()
    set(_studio_viewer OFF)
  endif()

  set(EBGEOMETRY_VIEWER ${_studio_viewer} CACHE STRING "The window of ebgstudio view: OFF or SDL2")
  set_property(CACHE EBGEOMETRY_VIEWER PROPERTY STRINGS OFF SDL2)

  if(EBGEOMETRY_VIEWER STREQUAL "SDL2")
    find_package(SDL2 REQUIRED)
    target_compile_definitions(ebgstudio PRIVATE EBGEOMETRY_VIEWER_SDL2
                                                 $<$<COMPILE_LANGUAGE:CUDA>:SDL_DISABLE_IMMINTRIN_H>)
    target_link_libraries(ebgstudio PRIVATE SDL2::SDL2)
  elseif(NOT EBGEOMETRY_VIEWER STREQUAL "OFF")
    message(FATAL_ERROR "EBGEOMETRY_VIEWER must be OFF or SDL2 (got '${EBGEOMETRY_VIEWER}')")
  endif()
endif()

if(_studio_alone OR EBGEOMETRY_INSTALL)
  include(GNUInstallDirs)
  install(TARGETS ebgstudio RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

# The tests, with the EBGeometry project's.
if(_studio_alone)
  return()
endif()

set(_studio_repo "${CMAKE_CURRENT_SOURCE_DIR}/..")

# Every model in Studio/Models reads with no error and no warning, every domain it has is complete, and it renders:
# they are users' starting points. A model's files are those of a folder: each that no other file there includes (the
# model, or its whole setup) is checked, and each that includes model.ebg (a setup of its own).
file(GLOB _studio_models LIST_DIRECTORIES true "${CMAKE_CURRENT_SOURCE_DIR}/Models/*")

foreach(_model IN LISTS _studio_models)
  if(NOT IS_DIRECTORY "${_model}")
    continue()
  endif()

  get_filename_component(_name "${_model}" NAME)
  file(GLOB _files "${_model}/*.ebg")

  set(_included)

  foreach(_file IN LISTS _files)
    file(STRINGS "${_file}" _includes REGEX "include\\(\"[^\"]*\"\\)")

    foreach(_line IN LISTS _includes)
      string(REGEX REPLACE ".*include\\(\"([^\"]*)\"\\).*" "\\1" _target "${_line}")
      list(APPEND _included "${_target}")
    endforeach()
  endforeach()

  foreach(_file IN LISTS _files)
    get_filename_component(_base "${_file}" NAME)
    get_filename_component(_stem "${_file}" NAME_WE)
    file(STRINGS "${_file}" _setup REGEX "include\\(\"model\\.ebg\"\\)")

    if(NOT _base IN_LIST _included OR _setup)
      set(_test ebgstudio-model-${_name}-${_stem})

      add_test(NAME ${_test} COMMAND ebgstudio check "${_file}" --all-domains)
      add_test(NAME ${_test}-render
               COMMAND ebgstudio render "${_file}" -o "${CMAKE_CURRENT_BINARY_DIR}/${_name}-${_stem}.ppm" --size 64 64)
      set_tests_properties(${_test} ${_test}-render PROPERTIES
                           LABELS "unit;studio" FAIL_REGULAR_EXPRESSION "warning|error|incomplete")
    endif()
  endforeach()
endforeach()

set(_studio_data "${_studio_repo}/Tests/data/language")

add_test(NAME ebgstudio-check-cubes COMMAND ebgstudio check "${_studio_data}/cubes/setup.ebg" --inventory)
add_test(NAME ebgstudio-print-cubes COMMAND ebgstudio print "${_studio_data}/cubes/setup.ebg" --flatten)
add_test(NAME ebgstudio-params-cubes COMMAND ebgstudio params "${_studio_data}/cubes/setup.ebg" --json -D "shift=(0, 2, 0)")
add_test(NAME ebgstudio-render-cubes
         COMMAND ebgstudio render "${_studio_data}/cubes/setup.ebg" -o "${CMAKE_CURRENT_BINARY_DIR}/cubes.ppm" --size 64 64)
add_test(NAME ebgstudio-check-example
         COMMAND ebgstudio check "${_studio_repo}/Examples/Language/cable/setup.ebg" --domain cfd --domain electrostatics)
set_tests_properties(ebgstudio-check-cubes ebgstudio-print-cubes ebgstudio-params-cubes ebgstudio-render-cubes
                     ebgstudio-check-example PROPERTIES LABELS "unit;studio")

# The writer's tests, with the unit tests.
if(TARGET Catch2::Catch2WithMain)
  add_executable(TestStudioWriter TestWriter.cpp)
  target_link_libraries(TestStudioWriter PRIVATE EBGeometry::EBGeometry Catch2::Catch2WithMain)
  target_compile_options(TestStudioWriter PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-Wall;-Wextra;-Wpedantic;-Wdouble-promotion;-Werror>)
  target_compile_definitions(TestStudioWriter PRIVATE EBGEOMETRY_TEST_DATA_DIR="${_studio_repo}/Tests/data")
  catch_discover_tests(TestStudioWriter TEST_SPEC "*" PROPERTIES LABELS "unit;studio")
endif()
