# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
#
# SPDX-License-Identifier: GPL-3.0-or-later

# Checks the build-cost tools (Tools/BuildCost.cpp, Tools/BuildCompare.cpp). Run by ctest with
#   cmake -DBUILD_COST=<BuildCost.ex> -DBUILD_COMPARE=<BuildCompare.ex> -DDIR=<an empty directory> -P TestBuildCost.cmake
# BuildCost must record a compile (and pass the command's exit status on); BuildCompare must compare two
# records per unit and per kernel, and flag what grew.
cmake_minimum_required(VERSION 3.16)

foreach(var BUILD_COST BUILD_COMPARE DIR)
  if(NOT ${var})
    message(FATAL_ERROR "TestBuildCost.cmake: set ${var}")
  endif()
endforeach()

file(REMOVE_RECURSE "${DIR}")
file(MAKE_DIRECTORY "${DIR}")

# A "compile" that is cmake -E echo: BuildCost records it, with its source and object, and exits as it did.
execute_process(COMMAND "${BUILD_COST}" --record=${DIR}/record.jsonl ${CMAKE_COMMAND} -E echo -c
                        /src/Performance/Probe/main.cpp -o probe.o
                RESULT_VARIABLE rc OUTPUT_QUIET)

if(NOT rc EQUAL 0)
  message(FATAL_ERROR "TestBuildCost.cmake: BuildCost exited with ${rc} for a command that succeeded")
endif()

file(READ "${DIR}/record.jsonl" record)

foreach(expected [["type":"compile"]] [["source":"/src/Performance/Probe/main.cpp"]] [["object":"probe.o"]]
                 [["cpu_s":]] [["peak_mb":]] [["exit":0]])
  string(FIND "${record}" "${expected}" at)

  if(at EQUAL -1)
    message(FATAL_ERROR "TestBuildCost.cmake: the record lacks ${expected}:\n${record}")
  endif()
endforeach()

execute_process(COMMAND "${BUILD_COST}" --record=${DIR}/record.jsonl ${CMAKE_COMMAND} -E false RESULT_VARIABLE rc)

if(rc EQUAL 0)
  message(FATAL_ERROR "TestBuildCost.cmake: BuildCost exited with 0 for a command that failed")
endif()

# Two records: one unit grows by 50% in CPU time (MAJOR), one stays; a kernel gains spills and registers.
file(WRITE "${DIR}/base.jsonl"
[[{"type":"compile","source":"/a/Performance/Ray/main.cpp","object":"r.o","compiler":"nvcc","cpu_s":10.0,"wall_s":10.0,"peak_mb":1000.0,"exit":0}
{"type":"compile","source":"/a/Performance/Tape/main.cpp","object":"t.o","compiler":"nvcc","cpu_s":20.0,"wall_s":20.0,"peak_mb":2000.0,"exit":0}
{"type":"compile","source":"/a/Performance/Tools/PerfCompare.cpp","object":"p.o","compiler":"c++","cpu_s":5.0,"wall_s":5.0,"peak_mb":100.0,"exit":0}
{"type":"kernel","source":"/a/Performance/Ray/main.cpp","name":"kernelA(int)","registers":168,"stack":1000,"spill_stores":4,"spill_loads":4}
{"type":"kernel","source":"/a/Performance/Ray/main.cpp","name":"kernelB(int)","registers":40,"stack":100,"spill_stores":0,"spill_loads":0}
]])
file(WRITE "${DIR}/head.jsonl"
[[{"type":"compile","source":"/b/Performance/Ray/main.cpp","object":"r.o","compiler":"nvcc","cpu_s":15.0,"wall_s":15.0,"peak_mb":1050.0,"exit":0}
{"type":"compile","source":"/b/Performance/Tape/main.cpp","object":"t.o","compiler":"nvcc","cpu_s":20.5,"wall_s":20.5,"peak_mb":2000.0,"exit":0}
{"type":"kernel","source":"/b/Performance/Ray/main.cpp","name":"kernelA(int)","registers":168,"stack":1000,"spill_stores":12,"spill_loads":16}
{"type":"kernel","source":"/b/Performance/Ray/main.cpp","name":"kernelB(int)","registers":44,"stack":100,"spill_stores":0,"spill_loads":0}
]])

execute_process(COMMAND "${BUILD_COMPARE}" --base "${DIR}/base.jsonl" --head "${DIR}/head.jsonl" --title "Probe builds"
                OUTPUT_VARIABLE report RESULT_VARIABLE rc)

if(NOT rc EQUAL 0)
  message(FATAL_ERROR "TestBuildCost.cmake: BuildCompare exited with ${rc}:\n${report}")
endif()

foreach(expected "### Probe builds" "| `Ray` | 10.0 s | 15.0 s | +50.0% |" "**MAJOR**" "| `Tape` | 20.0 s | 20.5 s | +2.5% |"
                 "2 in both, 2 changed (2 flagged" "4/4 → 12/16 | **spills**" "40 → 44" "**registers**")
  string(FIND "${report}" "${expected}" at)

  if(at EQUAL -1)
    message(FATAL_ERROR "TestBuildCost.cmake: the report lacks '${expected}':\n${report}")
  endif()
endforeach()

string(FIND "${report}" "PerfCompare" at)

if(NOT at EQUAL -1)
  message(FATAL_ERROR "TestBuildCost.cmake: the harness's own tools must not be reported:\n${report}")
endif()
