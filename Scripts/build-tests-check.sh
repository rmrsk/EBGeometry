#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Compile the unit test suite (debug preset) so template-instantiation errors
# in Source/ headers are caught before push instead of first in CI. Used by
# the manual-stage build-tests pre-commit hook and runnable directly.
set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

# Always (re)configure: the debug preset has its own build/debug directory (see
# CMakePresets.json), so this never picks up a stale configuration left behind
# by a different preset, and CMake's own up-to-date check keeps a no-op
# reconfigure fast.
cmake --preset debug >/dev/null

cmake --build --preset debug --parallel "$(nproc)"
