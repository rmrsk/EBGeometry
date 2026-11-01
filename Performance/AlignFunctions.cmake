# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
#
# SPDX-License-Identifier: GPL-3.0-or-later

# Align every function of the probes to 64 bytes, a cache line, so that a probe's time depends on its code
# and not on where the linker happens to place it. Performance/CMakeLists.txt includes this file, and
# Compare.cmake injects the candidate's copy into both sides' configure (CMAKE_PROJECT_INCLUDE), so that a
# baseline whose own probes predate it is built the same way.
#
# Why: without it, code added anywhere in a probe's binary shifts the functions after it, and a hot loop
# that crosses a cache-line or fetch-block boundary on one side but not the other changes its time with
# identical instructions. Measured on the tape evaluator (GCC 13, -O3 -march=native): a change that left
# every tape function's instructions identical moved them by 176 bytes, and Tape/csg float, smooth double
# and chain16 double ran 5-9% slower over 10 rounds; built with -falign-functions=64, base and head timed the
# same. Loops stay at the compiler's default alignment.
include_guard(GLOBAL)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-falign-functions=64> $<$<COMPILE_LANGUAGE:HIP>:-falign-functions=64>
                      $<$<COMPILE_LANGUAGE:CUDA>:-Xcompiler=-falign-functions=64>)
endif()
