// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_GPU.hpp
 * @brief  GPU portability layer: host/device decoration macros and backend detection.
 * @details This header is the single home for the thin backend-abstraction macros used by the
 * EBGeometry GPU port. It performs no allocation and pulls in no backend runtime headers; it only
 * detects which offload compiler (if any) is translating the current translation unit and expands
 * the decoration macros accordingly. On an ordinary host C++17 compiler every macro expands to
 * nothing, so annotated code is byte-for-byte identical to unannotated code.
 *
 * @par Decoration macros
 * - @ref EBGEOMETRY_HOST_DEVICE — callable from host and device (value types, the tape interpreter).
 * - @ref EBGEOMETRY_DEVICE — device-only.
 * - @ref EBGEOMETRY_HOST — host-only. Unannotated functions are already implicitly host-only under
 *   CUDA/HIP; this macro is applied to host-only public API (file parsers, mesh readers, host-side
 *   builders) to make that intent explicit and to let the device compiler reject accidental
 *   device use.
 * - @ref EBGEOMETRY_GLOBAL — device kernel entry point.
 *
 * @par Supported backends
 * - CUDA  (@c __CUDACC__): decorations expand to @c __host__ / @c __device__ / @c __global__.
 * - HIP   (@c __HIPCC__):  same expansions as CUDA (HIP uses the identical attributes).
 * - SYCL  (@c SYCL_LANGUAGE_VERSION): no decoration is required — a function is callable inside a
 *   kernel as long as it is defined and calls nothing host-only — so the decoration macros expand
 *   to nothing, exactly as on a plain host compiler.
 * - OpenACC (@c _OPENACC): device callability is expressed with @c "acc routine" directives rather
 *   than a function prefix; @ref EBGEOMETRY_ROUTINE carries that directive.
 *
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_GPU_HPP
#define EBGEOMETRY_GPU_HPP

// ── Backend detection ─────────────────────────────────────────────────────────
// A single #if/#elif chain, so at most one backend flag is defined per translation unit. HIP is
// tested BEFORE CUDA: hipcc on the NVIDIA platform defines both __HIPCC__ and __CUDACC__, and there
// the hip* entry points exist and forward to cuda*, so hip-on-NVIDIA must resolve to HIP. The flags
// are value-less: every consumer tests them with #if defined(...), never by arithmetic value.
#if defined(__HIPCC__)
#define EBGEOMETRY_HIP
#elif defined(__CUDACC__)
#define EBGEOMETRY_CUDA
#elif defined(SYCL_LANGUAGE_VERSION) || defined(__SYCL_DEVICE_ONLY__)
#define EBGEOMETRY_SYCL
#elif defined(_OPENACC)
#define EBGEOMETRY_OPENACC
#endif

// ── Function decoration ───────────────────────────────────────────────────────
// CUDA and HIP require an explicit prefix on any function that may be called from device code.
// SYCL and OpenACC do not use a prefix (SYCL: none needed; OpenACC: see EBGEOMETRY_ROUTINE), so on
// those backends — and on a plain host compiler — the decoration macros expand to nothing.
#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)

/** @brief Mark a function as callable from both host and device code (CUDA/HIP). */
#define EBGEOMETRY_HOST_DEVICE __host__ __device__

/** @brief Mark a function as callable from device code only (CUDA/HIP). */
#define EBGEOMETRY_DEVICE __device__

/** @brief Mark a function as callable from host code only (CUDA/HIP). */
#define EBGEOMETRY_HOST __host__

/** @brief Mark a function as a device kernel entry point (CUDA/HIP). */
#define EBGEOMETRY_GLOBAL __global__

#else

/** @brief Mark a function as callable from both host and device code (no-op on host compilers). */
#define EBGEOMETRY_HOST_DEVICE

/** @brief Mark a function as callable from device code only (no-op on host compilers). */
#define EBGEOMETRY_DEVICE

/** @brief Mark a function as callable from host code only (no-op on host compilers). */
#define EBGEOMETRY_HOST

/** @brief Mark a function as a device kernel entry point (no-op on host compilers). */
#define EBGEOMETRY_GLOBAL

#endif

// ── OpenACC routine directive ─────────────────────────────────────────────────
// OpenACC expresses device callability with a "#pragma acc routine seq" preceding the function,
// which _Pragma() lets us emit from a macro. On every other backend it expands to nothing.
#if defined(EBGEOMETRY_OPENACC)

/** @brief Emit an OpenACC "acc routine seq" directive (no-op on non-OpenACC backends). */
#define EBGEOMETRY_ROUTINE _Pragma("acc routine seq")

#else

/** @brief Emit an OpenACC "acc routine seq" directive (no-op on non-OpenACC backends). */
#define EBGEOMETRY_ROUTINE

#endif

// ── Inlining ──────────────────────────────────────────────────────────────────
// A single spelling for "inline, and force-inline on backends that honour it". CUDA/HIP accept
// __forceinline__; elsewhere plain inline is used.
#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)

/** @brief Inline hint, force-inlined on CUDA/HIP device compiles. */
#define EBGEOMETRY_INLINE __forceinline__ inline

#else

/** @brief Inline hint (plain @c inline on host compilers). */
#define EBGEOMETRY_INLINE inline

#endif

// ── Device-compilation-pass detection ─────────────────────────────────────────
// True only while the offload compiler is generating device code for the current __host__ __device__
// function (CUDA/HIP compile each such function twice). Used to select device-safe code paths, e.g.
// the assertion in EBGeometry_Macros.hpp.
#if defined(__CUDA_ARCH__) || defined(__HIP_DEVICE_COMPILE__)
#define EBGEOMETRY_DEVICE_COMPILE
#endif

#endif // EBGEOMETRY_GPU_HPP
