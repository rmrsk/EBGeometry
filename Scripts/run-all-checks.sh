#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Reproduces what CI.yml checks, in one command, for local use before pushing:
# every pre-commit hook (default and manual stage) plus every CMake preset's
# test suite. Takes several minutes -- mostly the Debug-mode example runs and
# the documentation build, if the required tools (LaTeX, Doxygen, Sphinx) are
# installed. Requires `pre-commit` to already be installed (`pip install
# pre-commit`); everything else is fetched/checked by the hooks themselves.
set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

section() {
  echo ""
  echo "==================================================================="
  echo "  $1"
  echo "==================================================================="
}

section "pre-commit: default-stage hooks (clang-format, reuse, codespell, doxygen-check)"
pre-commit run --all-files

# Force a clean slate for the Sphinx output directory before rebuilding: a stray
# `sphinx-autobuild` process (or any other watcher) rebuilding into the same
# directory concurrently can leave behind a doctree cache from a different Sphinx
# install/version, which throws an internal toctree-collector error on the next
# build. If this script's Sphinx steps fail with a crash inside
# sphinx.environment.collectors.toctree, check for and kill any such process first.
rm -rf Docs/Sphinx/build

section "pre-commit: manual-stage hooks (clang-tidy, build-tests, check-docs, doc figures/build)"
pre-commit run --all-files --hook-stage manual

section "CMake: debug preset -- unit tests"
cmake --preset debug
cmake --build --preset debug --parallel "$(nproc)"
ctest --preset debug

section "CMake: debug preset -- examples (assertions on)"
ctest --preset examples

section "CMake: debug-san preset -- unit tests under AddressSanitizer + UBSan"
cmake --preset debug-san
cmake --build --preset debug-san --parallel "$(nproc)"
ctest --preset debug-san

section "CMake: release-test preset -- unit tests + examples, optimised"
cmake --preset release-test
cmake --build --preset release-test --parallel "$(nproc)"
ctest --preset release-test

echo ""
echo "All checks passed."
