// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_DeviceTape.hpp
 * @brief  Declaration of DeviceTape: an RAII owner of one Tape uploaded to CUDA device memory.
 * @details DeviceTape copies a device-eligible @ref EBGeometry::Tape into a single pooled device
 * allocation and hands out a trivially-copyable @ref EBGeometry::TapeView whose pointers are
 * device pointers, ready to pass by value as a kernel argument. Everything in this header is
 * guarded on @c EBGEOMETRY_CUDA (nvcc translating the current TU), so the header is inert -- and
 * @c EBGeometry.hpp can include it unconditionally -- on a plain host compiler. Consequently, in
 * this tier DeviceTape is usable only from @c .cu translation units; host-compiler consumption
 * against @c libcudart is a possible later extension.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_DEVICETAPE_HPP
#define EBGEOMETRY_DEVICETAPE_HPP

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Tape.hpp"

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_DOXYGEN)

// Std includes
#include <cstdio>
#include <cstdlib>
#include <type_traits>

// CUDA runtime API (cudaMalloc/cudaMemcpy/cudaFree). nvcc pre-includes this, but spelling it out
// keeps the dependency explicit.
#include <cuda_runtime.h>

// Our includes
#include "EBGeometry_Macros.hpp"

/**
 * @brief Always-on CUDA runtime API error check: prints the failing call and aborts.
 * @details Host-side only. Deliberately NOT gated on EBGEOMETRY_ENABLE_ASSERTIONS (unlike
 * EBGEOMETRY_EXPECT): a failed allocation or copy leaves DeviceTape's view pointing at garbage,
 * which a release build must not silently evaluate. Abort (rather than an exception) matches the
 * library's no-exceptions convention.
 * @param x CUDA runtime API call (or any expression) yielding a cudaError_t.
 */
#define EBGEOMETRY_CUDA_CHECK(x)                                                      \
  do {                                                                                \
    const cudaError_t ebgeometry_cuda_err = (x);                                      \
    if (ebgeometry_cuda_err != cudaSuccess) {                                         \
      std::fprintf(stderr,                                                            \
                   "EBGeometry CUDA error: %s\n  call: %s\n  file: %s\n  line: %d\n", \
                   cudaGetErrorString(ebgeometry_cuda_err),                           \
                   #x,                                                                \
                   __FILE__,                                                          \
                   __LINE__);                                                         \
      std::abort();                                                                   \
    }                                                                                 \
  } while (0)

namespace EBGeometry {

/**
 * @brief RAII owner of one @ref Tape uploaded to CUDA device memory.
 * @details A single pooled @c cudaMalloc holds every tape array back to back; @ref view returns a
 * trivially-copyable @ref TapeView whose pointers are DEVICE pointers -- pass it BY VALUE as a
 * kernel argument and call @c view.value(p, coordScratch, valueScratch) in the kernel. The source
 * @ref Tape is fully copied at construction and need NOT stay alive afterwards (device-eligible
 * tapes hold no host-node back-pointers by definition). Copies are synchronous on the default
 * stream (async/stream support is a later tier). Move-only.
 *
 * See the golden test @c Tests/GPU/EBGeometry_GPUTape.cu for a complete upload-launch-compare
 * usage example.
 * @tparam T Floating-point precision.
 */
template <class T>
class DeviceTape
{
  static_assert(std::is_floating_point_v<T>, "DeviceTape<T>: T must be a floating-point type");

public:
  /**
   * @brief Deleted default constructor: a DeviceTape always wraps an uploaded Tape.
   */
  DeviceTape() = delete;

  /**
   * @brief Deleted copy constructor (move-only; the device pool has a single owner).
   */
  DeviceTape(const DeviceTape&) = delete;

  /**
   * @brief Deleted copy assignment (move-only; the device pool has a single owner).
   */
  DeviceTape&
  operator=(const DeviceTape&) = delete;

  /**
   * @brief Upload @p a_tape to the current CUDA device.
   * @details Aborts (always-on, independent of EBGEOMETRY_ENABLE_ASSERTIONS) if the tape is not
   * device-eligible or any CUDA API call fails.
   * @param[in] a_tape Compiled tape to upload; must satisfy Tape::isDeviceEligible.
   */
  EBGEOMETRY_HOST explicit DeviceTape(const Tape<T>& a_tape);

  /**
   * @brief Move constructor. Steals the device pool and view; the source is left empty.
   * @param[in, out] a_other Tape to move from.
   */
  EBGEOMETRY_HOST
  DeviceTape(DeviceTape&& a_other) noexcept;

  /**
   * @brief Move assignment. Frees this tape's pool, then steals from @p a_other.
   * @param[in, out] a_other Tape to move from.
   * @return This DeviceTape.
   */
  EBGEOMETRY_HOST DeviceTape&
  operator=(DeviceTape&& a_other) noexcept;

  /**
   * @brief Destructor. Frees the device pool (the result of cudaFree is deliberately discarded in
   * a destructor).
   */
  EBGEOMETRY_HOST ~DeviceTape() noexcept;

  /**
   * @brief Get the device-pointer view of the uploaded tape.
   * @details The view's pointers are device pointers into this DeviceTape's pool and stay valid
   * exactly as long as this DeviceTape is alive. Pass the view by value to kernels.
   * @return A @ref TapeView whose array pointers are device pointers.
   */
  [[nodiscard]] EBGEOMETRY_HOST TapeView<T>
                                view() const noexcept
  {
    return m_view;
  }

  /**
   * @brief Get the total number of bytes held by the device pool (diagnostics).
   * @return Pool size in bytes.
   */
  [[nodiscard]] EBGEOMETRY_HOST size_t
  getNumBytes() const noexcept
  {
    return m_numBytes;
  }

private:
  /**
   * @brief Single device allocation holding every uploaded tape array.
   */
  void* m_pool = nullptr;

  /**
   * @brief Total pool size in bytes.
   */
  size_t m_numBytes = 0;

  /**
   * @brief View with device pointers plus the counts copied from the source tape.
   */
  TapeView<T> m_view{};
};

} // namespace EBGeometry

#include "EBGeometry_DeviceTapeImplem.hpp"

#endif // EBGEOMETRY_CUDA || EBGEOMETRY_DOXYGEN

#endif // EBGEOMETRY_DEVICETAPE_HPP
