#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Runs the performance probes under Performance/: every case by default, a subset on demand, or a
# comparison of two commits. All the logic is in Performance/RunPerformance.cmake, which needs only
# CMake, git and a compiler; see --help or Docs/Sphinx/source/PerformanceTesting.rst.
#
#   Scripts/run-performance.sh                          # every case
#   Scripts/run-performance.sh 'PointCloudBVH/*'        # a subset
#   Scripts/run-performance.sh compare origin/dev       # this tree against origin/dev
set -euo pipefail

cd "$(dirname "$0")/.."

status_file=build/performance/.compare-status
rm -f "$status_file"

rc=0
cmake -P Performance/RunPerformance.cmake -- "$@" || rc=$?

# A comparison writes its verdict's exit status here (2 when the verdict is red), so that it can end on
# the verdict line rather than on a CMake error.
if [ "$rc" -eq 0 ] && [ -f "$status_file" ]; then
  rc=$(cat "$status_file")
fi

rm -f "$status_file"
exit "$rc"
