// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_GPU.hpp
 * @brief  GPU portability layer: host/device decoration macros and backend detection.
 * @details This header is the single home for the thin backend-abstraction macros used by the
 * EBGeometry GPU port. It performs no allocation and pulls in no backend runtime headers; it only
 * detects which offload compiler (if any) is translating the current translation unit and expands
 * the decoration macros accordingly. On an ordinary host C++17 compiler every decoration macro
 * expands to nothing, so annotated code is byte-for-byte identical to unannotated code.
 *
 * It also defines the inlining and launch macros: @ref EBGEOMETRY_DEVICE_NOINLINE (out of line in a
 * device pass), @ref EBGEOMETRY_HOST_ALWAYS_INLINE and @ref EBGEOMETRY_HOST_NOINLINE (GCC/Clang
 * attributes in a host pass, so these two are not empty on a host compiler), and
 * @ref EBGEOMETRY_GRID_CONSTANT (a kernel parameter read in place, CUDA only).
 *
 * @par Decoration macros
 * - @ref EBGEOMETRY_HOST_DEVICE — callable from host and device (value types, the tape interpreter).
 * - @ref EBGEOMETRY_HOST — host-only. Unannotated functions are already implicitly host-only under
 *   CUDA/HIP; this macro is applied to host-only public API (file parsers, mesh readers, host-side
 *   builders) to make that intent explicit and to let the device compiler reject accidental
 *   device use.
 * - @ref EBGEOMETRY_GLOBAL — device kernel entry point.
 *
 * @par Supported backends
 * - CUDA  (@c __CUDACC__): decorations expand to @c __host__ / @c __device__ / @c __global__.
 * - HIP   (@c __HIPCC__):  same expansions as CUDA (HIP uses the identical attributes).
 *
 * Any other compiler, including a SYCL or OpenACC compiler, sees host code: the macros expand to
 * nothing, and nothing in EBGeometry is callable inside a SYCL kernel or an OpenACC compute region.
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
#endif

// ── Host-tuned defaults are for host-only builds ──────────────────────────────
// EBGEOMETRY_HOST_TUNED_DEFAULTS makes TriangleSoA::DefaultWidth follow the SIMD flags. The host
// and device passes of one CUDA or HIP compile see different ISA macros, so the same type would get
// two layouts within one translation unit. The test keys on EBGEOMETRY_CUDA/EBGEOMETRY_HIP (from
// __CUDACC__/__HIPCC__), which both passes define, and deliberately not on EBGEOMETRY_DEVICE_COMPILE,
// which only the device pass does: that would let the host pass through and reintroduce the split
// this guard exists to prevent.
#if defined(EBGEOMETRY_HOST_TUNED_DEFAULTS) && (defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP))
#error \
  "EBGEOMETRY_HOST_TUNED_DEFAULTS is for host-only builds; it must not be defined in a CUDA or HIP translation unit"
#endif

// ── Function decoration ───────────────────────────────────────────────────────
// CUDA and HIP require an explicit prefix on any function that may be called from device code. On
// every other compiler the decoration macros expand to nothing.
#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)

/** @brief Mark a function as callable from both host and device code (CUDA/HIP). */
#define EBGEOMETRY_HOST_DEVICE __host__ __device__

/** @brief Mark a function as callable from host code only (CUDA/HIP). */
#define EBGEOMETRY_HOST __host__

/** @brief Mark a function as a device kernel entry point (CUDA/HIP). */
#define EBGEOMETRY_GLOBAL __global__

#else

/** @brief Mark a function as callable from both host and device code (no-op on host compilers). */
#define EBGEOMETRY_HOST_DEVICE

/** @brief Mark a function as callable from host code only (no-op on host compilers). */
#define EBGEOMETRY_HOST

/** @brief Mark a function as a device kernel entry point (no-op on host compilers). */
#define EBGEOMETRY_GLOBAL

#endif

// ── Device-compilation-pass detection ─────────────────────────────────────────
// True only while the offload compiler is generating device code for the current __host__ __device__
// function (CUDA/HIP compile each such function twice). Used to select device-safe code paths, e.g.
// the assertion in EBGeometry_Macros.hpp.
#if defined(__CUDA_ARCH__) || defined(__HIP_DEVICE_COMPILE__)
#define EBGEOMETRY_DEVICE_COMPILE
#endif

// ── Out-of-line device functions ──────────────────────────────────────────────
// HIP inlines every device function by default (-amdgpu-function-calls=false), so a function called
// from many places is copied into each of them. A function that calls itself one level down from
// many places (the tape's evaluate<D>) then grows as the number of call sites to the power of the
// depth. Marking it out of line in the device pass keeps it a real call there; the host pass is left
// to its own inliner. Fully inlined, an optimised HIP compile of TestTape was stopped after ten minutes and
// 2.5 GB; with evaluate<D> out of line it takes half a minute.
#if defined(EBGEOMETRY_DEVICE_COMPILE)
/** @brief Keep a function out of line in the device pass (CUDA/HIP); nothing on the host. */
#define EBGEOMETRY_DEVICE_NOINLINE __noinline__
#else
/** @brief Keep a function out of line in the device pass (CUDA/HIP); nothing on the host. */
#define EBGEOMETRY_DEVICE_NOINLINE
#endif

// ── Inlined host kernels ──────────────────────────────────────────────────────
// GCC weighs a large kernel against the code around each call and may keep it out of line, which for
// a SIMD kernel costs a round trip of every register through memory. The host pass of a kernel that
// must be inlined is marked so; the device pass is left to its own inliner. Use it together with
// `inline` (or on a function defined in its class), as GCC warns about always_inline otherwise.
#if !defined(EBGEOMETRY_DEVICE_COMPILE) && (defined(__GNUC__) || defined(__clang__))
/** @brief Always inline a function in a host pass (GCC and Clang); nothing in a device pass. */
#define EBGEOMETRY_HOST_ALWAYS_INLINE __attribute__((always_inline))
#else
/** @brief Always inline a function in a host pass (GCC and Clang); nothing in a device pass. */
#define EBGEOMETRY_HOST_ALWAYS_INLINE
#endif

// ── Out-of-line host functions ────────────────────────────────────────────────
// GCC's choice of what to inline into a large function can change with unrelated code elsewhere in the
// translation unit. Where a hot function must be split at a fixed point (the tape's primitive dispatch), the
// host pass of the part beyond it is marked so; the device pass is left to its own inliner.
#if !defined(EBGEOMETRY_DEVICE_COMPILE) && (defined(__GNUC__) || defined(__clang__))
/** @brief Keep a function out of line in a host pass (GCC and Clang); nothing in a device pass. */
#define EBGEOMETRY_HOST_NOINLINE __attribute__((noinline))
#else
/** @brief Keep a function out of line in a host pass (GCC and Clang); nothing in a device pass. */
#define EBGEOMETRY_HOST_NOINLINE
#endif

// ── Kernel parameters read in place ───────────────────────────────────────────
// A kernel receives its arguments in a read-only parameter space. A function that needs the address of
// one (a member function of a query that is not inlined, such as a Tape's evaluator) makes the compiler
// copy the whole argument to each thread's local memory first: for a Tape, a few hundred bytes per
// thread, which made it about 30 times slower on a device than the same object written by hand. CUDA
// 11.7 and later can give the address of a __grid_constant__ parameter without the copy, on
// compute capability 7.0 and later; nvcc rejects the qualifier in a device pass for an older
// architecture, so there (__CUDA_ARCH__ below 700) the macro is empty and the parameter is copied.
// The host pass and the newer device passes of the same build keep it. HIP has no such
// qualifier, so there the macro is empty.
#if defined(EBGEOMETRY_CUDA) && defined(__CUDACC_VER_MAJOR__) &&                              \
  (__CUDACC_VER_MAJOR__ > 11 || (__CUDACC_VER_MAJOR__ == 11 && __CUDACC_VER_MINOR__ >= 7)) && \
  !(defined(__CUDA_ARCH__) && __CUDA_ARCH__ < 700)
/** @brief Let a kernel read a const parameter in place, through its address, rather than copy it (CUDA). */
#define EBGEOMETRY_GRID_CONSTANT __grid_constant__
#else
/** @brief Let a kernel read a const parameter in place, through its address, rather than copy it (CUDA). */
#define EBGEOMETRY_GRID_CONSTANT
#endif

#endif // EBGEOMETRY_GPU_HPP
