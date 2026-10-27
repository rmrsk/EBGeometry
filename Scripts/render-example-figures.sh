#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Renders the picture on each example's Sphinx page: the first frame the viewer shows when the example
# calls Render::view(), written to Docs/Sphinx/source/_static/example_<Name>.jpg (tracked in git).
#
# Each example is compiled with Render::view() swapped for a stand-in that renders one frame with the
# same camera and settings and writes it to a file, then run in its own folder (so its default input is
# found). Run it by hand after a change to an example's geometry or to the renderer; nothing runs it
# automatically.
#
# Requires: a C++17 compiler (CXX, default g++) and ImageMagick's convert. The examples that read a mesh
# by default (MeshSDF, MeshCSG, Shell, Materials, Repetition, Twist) need the common-3d-test-models
# submodule: git submodule update --init.
#
# Usage:
#   Scripts/render-example-figures.sh                  # every example that calls Render::view()
#   Scripts/render-example-figures.sh BasicCSG Shapes  # only these

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT="${ROOT}/Docs/Sphinx/source/_static"
WORK="$(mktemp -d)"
trap 'rm -rf "${WORK}"' EXIT

cat > "${WORK}/snapshot.hpp" <<'HPP'
#pragma once
#include <EBGeometry.hpp>
#include <cstdio>
#include <cstdlib>

// Stands in for Render::view(): renders the first full frame and writes it as a PPM to $SNAPSHOT.
namespace Snapshot {
template <class S>
void
view(const S& a_shape, const EBGeometry::Render::Camera<EBGeometry::ShapePrecision<S>>& a_camera,
     const EBGeometry::Render::Settings<EBGeometry::ShapePrecision<S>>& a_settings, int, char**)
{
  EBGeometry::Render::Image image;
  EBGeometry::Render::Renderer<EBGeometry::ShapePrecision<S>>(a_settings).render(a_shape, a_camera, image);

  FILE* file = std::fopen(std::getenv("SNAPSHOT"), "wb");
  if (file == nullptr) {
    std::fprintf(stderr, "render-example-figures: cannot write %s\n", std::getenv("SNAPSHOT"));
    std::exit(1);
  }
  std::fprintf(file, "P6\n%u %u\n255\n", image.m_width, image.m_height);
  for (const uint32_t p : image.m_pixels) {
    const unsigned char rgb[3] = {(unsigned char)(p >> 24), (unsigned char)(p >> 16), (unsigned char)(p >> 8)};
    std::fwrite(rgb, 1, 3, file);
  }
  std::fclose(file);
}

template <class S>
void
view(const S& a_shape, int a_argc, char** a_argv)
{
  using T = EBGeometry::ShapePrecision<S>;
  Snapshot::view(a_shape, EBGeometry::Render::ViewerDetail::initialCamera<T>(a_shape),
                 EBGeometry::Render::Settings<T>(), a_argc, a_argv);
}

// A device view is ignored: the frame is rendered on the host.
template <class S>
void
view(const S& a_shape, const S&, const EBGeometry::Render::Camera<EBGeometry::ShapePrecision<S>>& a_camera,
     const EBGeometry::Render::Settings<EBGeometry::ShapePrecision<S>>& a_settings, int a_argc, char** a_argv)
{
  Snapshot::view(a_shape, a_camera, a_settings, a_argc, a_argv);
}
} // namespace Snapshot
HPP

if [ "$#" -gt 0 ]; then
  examples=("$@")
else
  mapfile -t examples < <(grep -l '^[[:space:]]*EBGeometry::Render::view(' "${ROOT}"/Examples/*/main.cpp | xargs -n1 dirname | xargs -n1 basename)
fi

for example in "${examples[@]}"; do
  if ! grep -q '^[[:space:]]*EBGeometry::Render::view(' "${ROOT}/Examples/${example}/main.cpp" 2>/dev/null; then
    echo "render-example-figures: Examples/${example}/main.cpp does not exist or opens no viewer" >&2
    exit 1
  fi

  echo "Rendering ${example}"
  sed 's/EBGeometry::Render::view(/Snapshot::view(/' "${ROOT}/Examples/${example}/main.cpp" > "${WORK}/${example}.cpp"
  "${CXX:-g++}" -std=c++17 -O3 -march=native -I"${ROOT}" -include "${WORK}/snapshot.hpp" \
    "${WORK}/${example}.cpp" -o "${WORK}/${example}.ex" -pthread
  (cd "${ROOT}/Examples/${example}" && SNAPSHOT="${WORK}/${example}.ppm" "${WORK}/${example}.ex" > /dev/null)
  convert "${WORK}/${example}.ppm" -resize 640x640 -strip -quality 85 "${OUT}/example_${example}.jpg"
done
