#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Run clang-tidy over the library headers via the test/example translation units
# (a header-only library has no .cpp of its own, so the headers are analysed
# through the TUs that instantiate them). Used by the manual-stage clang-tidy
# pre-commit hook and runnable directly.
set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

# Always (re)configure: the debug preset has its own build/debug directory (see
# CMakePresets.json), so this never picks up a stale configuration left behind
# by a different preset, and CMake's own up-to-date check keeps a no-op
# reconfigure fast.
cmake --preset debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON >/dev/null

# clang can fail to auto-detect the installed GCC toolchain (then it cannot find
# libstdc++ headers); point it at the active GCC install explicitly.
gcc_install_dir="$(dirname "$(gcc -print-libgcc-file-name)")"

run-clang-tidy -p build/debug -quiet \
  -extra-arg="--gcc-install-dir=${gcc_install_dir}" \
  '(Tests/|Examples/).*\.cpp'
