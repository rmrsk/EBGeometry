#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Compile the unit test suite (debug preset) so template-instantiation errors
# in Source/ headers are caught before push instead of first in CI. Used by
# the manual-stage build-tests pre-commit hook and runnable directly.
set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

if [ ! -d build ]; then
  echo "build-tests-check: configuring build/ (debug preset) ..."
  cmake --preset debug >/dev/null
fi

cmake --build --preset debug --parallel "$(nproc)"
