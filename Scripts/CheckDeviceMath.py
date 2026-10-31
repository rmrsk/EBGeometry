#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
# SPDX-License-Identifier: GPL-3.0-or-later
"""Check that library code under Source/ keeps to the device toolchain contract.

nvcc rejects a call to a constexpr host function (std::min, std::array::operator[],
std::numeric_limits<T>::max(), std::move, ...) from device code, unless every translation unit is
compiled with --expt-relaxed-constexpr, which EBGeometry does not require of its users. HIP-clang
accepts such calls, so the HIP CI lane cannot catch them. This script enforces two rules:

1. Nowhere under Source/: std::min, std::max, std::clamp, std::numeric_limits, std::array. Use
   Math::min, Math::max, Math::clamp, Math::Limits (EBGeometry_Math.hpp) and Array
   (EBGeometry_Array.hpp), which are host- and device-callable.
2. Inside a function marked EBGEOMETRY_HOST_DEVICE: no call to a std:: function other than the
   math functions that CUDA and HIP provide for device code (ALLOWED_DEVICE_CALLS below). This rule
   also covers the device code in Tests/ (functions marked __global__ or __device__ as well), which
   the GPU CI lanes compile without --expt-relaxed-constexpr.

Comments and string literals are ignored. Exits with status 1 and lists every violation.
"""

import re
import sys
from pathlib import Path

BANNED = [
    (re.compile(r"\bstd::(min|max|clamp)\s*[<(]"), "use Math::min/Math::max/Math::clamp"),
    (re.compile(r"\bstd::numeric_limits\b"), "use Math::Limits"),
    (re.compile(r"\bstd::array\b"), "use Array"),
]

# std:: functions CUDA and HIP declare for device code, and integer types used as casts.
ALLOWED_DEVICE_CALLS = {
    "abs", "fabs", "sqrt", "cbrt", "exp", "exp2", "expm1", "log", "log2", "log10", "log1p", "pow",
    "sin", "cos", "tan", "asin", "acos", "atan", "atan2", "sinh", "cosh", "tanh", "hypot",
    "floor", "ceil", "round", "trunc", "fmod", "fmin", "fmax", "copysign", "isfinite", "isnan",
    "isinf", "signbit", "fma",
    "size_t", "ptrdiff_t", "int8_t", "uint8_t", "int16_t", "uint16_t", "int32_t", "uint32_t",
    "int64_t", "uint64_t", "uintptr_t",
}

STD_CALL = re.compile(r"\bstd::(\w+)\s*(?:<[^;{}()]*>)?\s*\(")


def strip_comments_and_strings(text):
    """Replace comments and string/char literals by spaces, keeping line breaks and offsets."""
    out = []
    i, n = 0, len(text)

    while i < n:
        c = text[i]
        nxt = text[i + 1] if i + 1 < n else ""

        if c == "/" and nxt == "/":
            j = text.find("\n", i)
            j = n if j < 0 else j
            out.append(" " * (j - i))
            i = j
        elif c == "/" and nxt == "*":
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append("".join(ch if ch == "\n" else " " for ch in text[i:j]))
            i = j
        elif c in "\"'":
            j = i + 1

            while j < n and text[j] != c:
                j += 2 if text[j] == "\\" else 1

            j = min(j + 1, n)
            out.append(c + " " * (j - i - 2) + c if j - i >= 2 else " " * (j - i))
            i = j
        else:
            out.append(c)
            i += 1

    return "".join(out)


def device_bodies(code, markers=r"EBGEOMETRY_HOST_DEVICE"):
    """Yield (start, end) offsets of the bodies of functions marked with one of the markers."""
    for m in re.finditer(r"\b(" + markers + r")\b", code):
        # The body is the first '{' after the signature, unless a ';' (a declaration) comes first.
        # Skip braces inside parentheses (default arguments such as '= {}').
        depth = 0
        i = m.end()

        while i < len(code):
            ch = code[i]

            if ch == "(":
                depth += 1
            elif ch == ")":
                depth -= 1
            elif depth == 0 and ch == ";":
                break
            elif depth == 0 and ch == "{":
                level = 0
                j = i

                while j < len(code):
                    if code[j] == "{":
                        level += 1
                    elif code[j] == "}":
                        level -= 1

                        if level == 0:
                            break

                    j += 1

                yield i, j
                break

            i += 1


def check(path, library):
    text = path.read_text(encoding="utf-8")
    code = strip_comments_and_strings(text)
    problems = set()

    def line_of(offset):
        return code.count("\n", 0, offset) + 1

    if library:
        for pattern, advice in BANNED:
            for m in pattern.finditer(code):
                problems.add((line_of(m.start()), f"'{m.group(0).rstrip('(<').strip()}' is banned under Source/; {advice}"))

    markers = r"EBGEOMETRY_HOST_DEVICE" if library else r"EBGEOMETRY_HOST_DEVICE|__global__|__device__"

    for start, end in device_bodies(code, markers):
        for m in STD_CALL.finditer(code, start, end):
            if m.group(1) not in ALLOWED_DEVICE_CALLS:
                problems.add((line_of(m.start()), f"std::{m.group(1)} called in an EBGEOMETRY_HOST_DEVICE function"))

    return sorted(problems)


def main():
    root = Path(__file__).resolve().parent.parent
    failed = False

    files = [(path, True) for path in sorted((root / "Source").glob("*.hpp"))]
    files += [(path, False) for path in sorted((root / "Tests").glob("*.[ch]pp"))]

    for path, library in files:
        for line, message in check(path, library):
            print(f"{path.relative_to(root)}:{line}: {message}")
            failed = True

    if failed:
        print("\nSee 'Writing device code' in Docs/Sphinx/source/ContributionGuidelines.rst.")
        return 1

    print("Device toolchain contract holds for Source/ and the device code in Tests/.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
