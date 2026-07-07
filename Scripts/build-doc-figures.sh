#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Renders every Sphinx documentation figure from its LaTeX/TikZ source.
#
# The figures live under Docs/Sphinx/source/_static/<Name>/<Name>.tex. Only the
# .tex source is tracked in git; the rendered PNG (Docs/Sphinx/source/_static/<Name>.png,
# referenced directly by the RST files via ".. figure:: /_static/<Name>.png") is a build
# artifact and is git-ignored. Run this script before building the Sphinx HTML/PDF docs,
# whether locally, in the pre-commit hook, or in CI.
#
# Requires: pdflatex (texlive-latex-extra or similar) and pdftoppm (poppler-utils).
#
# Usage:
#   Scripts/build-doc-figures.sh

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
STATIC_DIR="${REPO_ROOT}/Docs/Sphinx/source/_static"
DPI=300

for tool in pdflatex pdftoppm; do
  if ! command -v "${tool}" >/dev/null 2>&1; then
    echo "error: required tool '${tool}' not found on PATH" >&2
    echo "       install texlive-latex-extra (pdflatex) and poppler-utils (pdftoppm)" >&2
    exit 1
  fi
done

if [[ ! -d "${STATIC_DIR}" ]]; then
  echo "error: ${STATIC_DIR} does not exist" >&2
  exit 1
fi

shopt -s nullglob
tex_files=("${STATIC_DIR}"/*/*.tex)
shopt -u nullglob

if [[ ${#tex_files[@]} -eq 0 ]]; then
  echo "error: no figure .tex sources found under ${STATIC_DIR}/*/*.tex" >&2
  exit 1
fi

echo "Building ${#tex_files[@]} documentation figure(s) from LaTeX sources..."

failed=0
for tex_file in "${tex_files[@]}"; do
  fig_dir="$(dirname "${tex_file}")"
  name="$(basename "${tex_file}" .tex)"
  pdf_file="${fig_dir}/${name}.pdf"
  png_target="${STATIC_DIR}/${name}.png"

  echo "  - ${name}"

  if ! pdflatex -interaction=nonstopmode -halt-on-error \
       -output-directory "${fig_dir}" "${tex_file}" > "${fig_dir}/${name}.pdflatex.log" 2>&1; then
    echo "    FAILED: pdflatex could not compile ${tex_file}" >&2
    echo "    see ${fig_dir}/${name}.pdflatex.log for details" >&2
    failed=1
    continue
  fi

  if ! pdftoppm -png -singlefile -r "${DPI}" "${pdf_file}" "${STATIC_DIR}/${name}" \
       > "${fig_dir}/${name}.pdftoppm.log" 2>&1; then
    echo "    FAILED: pdftoppm could not rasterize ${pdf_file}" >&2
    echo "    see ${fig_dir}/${name}.pdftoppm.log for details" >&2
    failed=1
    continue
  fi

  if [[ ! -f "${png_target}" ]]; then
    echo "    FAILED: expected output ${png_target} was not created" >&2
    failed=1
    continue
  fi

  rm -f "${fig_dir}/${name}.pdflatex.log" "${fig_dir}/${name}.pdftoppm.log"
done

if [[ ${failed} -ne 0 ]]; then
  echo "One or more figures failed to build." >&2
  exit 1
fi

echo "All documentation figures built successfully."
