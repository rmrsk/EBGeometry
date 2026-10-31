#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Builds a small project that uses EBGeometry the two ways Building.rst documents -- add_subdirectory
# and install + find_package -- and checks that neither imposes SIMD flags on it. Used by CI.
#
# Usage:
#   Scripts/check-cmake-consumer.sh [work-directory]

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
WORK="${1:-$(mktemp -d)}"

rm -rf "${WORK}"
mkdir -p "${WORK}/consumer"

cat > "${WORK}/consumer/main.cpp" <<'CPP'
#include <cstdio>

#include "EBGeometry.hpp"

int
main()
{
  const EBGeometry::SphereSDF<double> sphere(EBGeometry::Vec3T<double>::zeros(), 1.0);
  const double                        d = sphere.signedDistance(EBGeometry::Vec3T<double>(2.0, 0.0, 0.0));

  std::printf("%g\n", d);

  return d == 1.0 ? 0 : 1;
}
CPP

# Checks that a consumer's compile command carries no SIMD flag, then builds and runs it.
check_consumer() {
  local build="$1"
  shift

  cmake -S "${WORK}/consumer" -B "${build}" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON "$@" > /dev/null

  if grep -E -- "-mavx|-msse4|-mfma|-march" "${build}/compile_commands.json"; then
    echo "error: EBGeometry imposed SIMD flags on a consuming project (${build})" >&2
    exit 1
  fi

  cmake --build "${build}" > /dev/null
  "${build}/consumer"
}

echo "== add_subdirectory"
cat > "${WORK}/consumer/CMakeLists.txt" <<CMAKE
cmake_minimum_required(VERSION 3.16)
project(Consumer CXX)
add_subdirectory("${REPO_ROOT}" EBGeometry)
add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE EBGeometry::EBGeometry)
CMAKE
check_consumer "${WORK}/build-subdirectory"

echo "== install + find_package"
# Installed from a top-level configure, whose SIMD default is avx: the installed package must not
# carry it.
cmake -S "${REPO_ROOT}" -B "${WORK}/build-ebgeometry" -DCMAKE_INSTALL_PREFIX="${WORK}/prefix" > /dev/null
cmake --install "${WORK}/build-ebgeometry" > /dev/null

cat > "${WORK}/consumer/CMakeLists.txt" <<'CMAKE'
cmake_minimum_required(VERSION 3.16)
project(Consumer CXX)
find_package(EBGeometry 1.0 REQUIRED)
add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE EBGeometry::EBGeometry)
CMAKE
check_consumer "${WORK}/build-package" -DCMAKE_PREFIX_PATH="${WORK}/prefix"

echo "Both ways of consuming EBGeometry build, run, and carry no SIMD flags."
