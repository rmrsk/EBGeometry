# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Runs PROGRAM and succeeds only if it fails (any nonzero status or signal) and its stderr contains
# MESSAGE. CTest counts a program killed by a signal as failed whatever PASS_REGULAR_EXPRESSION says,
# so a test that must abort runs through this script.

execute_process(COMMAND "${PROGRAM}" RESULT_VARIABLE result ERROR_VARIABLE err OUTPUT_QUIET)

if(result EQUAL 0)
  message(FATAL_ERROR "${PROGRAM} exited normally, but it should have aborted")
endif()

string(FIND "${err}" "${MESSAGE}" found)

if(found EQUAL -1)
  message(FATAL_ERROR "${PROGRAM} failed (${result}) without the expected message:\n${err}")
endif()
