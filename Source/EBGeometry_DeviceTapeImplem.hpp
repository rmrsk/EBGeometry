// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_DeviceTapeImplem.hpp
 * @brief  Implementation of EBGeometry_DeviceTape.hpp.
 * @details Holds the out-of-line DeviceTape definitions: the pooled-upload constructor (a sizing
 * pass and a copy pass, both driven by the EBGEOMETRY_TAPE_PARAM_SOA X-macro registry), the move
 * operations, and the destructor. Included last from EBGeometry_DeviceTape.hpp, inside the same
 * EBGEOMETRY_CUDA guard, so it is inert on a plain host compiler.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_DEVICETAPEIMPLEM_HPP
#define EBGEOMETRY_DEVICETAPEIMPLEM_HPP

// Our includes
#include "EBGeometry_GPU.hpp"

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_DOXYGEN)

// Std includes
#include <cstdio>
#include <cstdlib>
#include <type_traits>
#include <vector>

// CUDA runtime API.
#include <cuda_runtime.h>

// Our includes
#include "EBGeometry_DeviceTape.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Tape.hpp"

namespace EBGeometry {

namespace DeviceTapeDetail {

/**
 * @brief Byte alignment of every sub-array section inside the DeviceTape pool.
 * @details 256 bytes is at least the natural alignment of every stored type and matches the base
 * alignment cudaMalloc itself guarantees, so base + paddedOffset preserves it for every section.
 */
inline constexpr size_t DeviceTapeAlign = 256;

/**
 * @brief Round a byte count up to the next multiple of @ref DeviceTapeAlign.
 * @param[in] a_bytes Unpadded byte count.
 * @return Padded byte count.
 */
[[nodiscard]] EBGEOMETRY_HOST inline size_t
padUp(size_t a_bytes) noexcept
{
  return (a_bytes + (DeviceTapeAlign - 1)) & ~(DeviceTapeAlign - 1);
}

/**
 * @brief Padded byte footprint of one host vector in the DeviceTape pool.
 * @details An empty vector reserves no section at all (zero bytes), consistently with
 * @ref uploadArray handing out a null device pointer for it.
 * @tparam U Element type.
 * @param[in] a_host Host array.
 * @return Padded byte count (zero for an empty vector).
 */
template <class U>
[[nodiscard]] EBGEOMETRY_HOST size_t
paddedBytes(const std::vector<U>& a_host) noexcept
{
  return a_host.empty() ? size_t(0) : padUp(a_host.size() * sizeof(U));
}

/**
 * @brief Copy one host vector into the device pool at @p a_offset, advance @p a_offset by the
 * padded section size, and return the typed device pointer to the copied section.
 * @details An empty vector yields no copy, no offset advance, and a null pointer -- mirroring what
 * Tape::view() hands out for an empty std::vector (whose .data() may be null), and matching the
 * interpreter contract that an array no clause references is never dereferenced.
 * @tparam U Element type (must be trivially copyable).
 * @param[in]      a_base   Device base address of the pool.
 * @param[in, out] a_offset Current byte offset into the pool; advanced by the padded section size.
 * @param[in]      a_host   Host array to upload.
 * @return Device pointer to the section holding the copied array (null for an empty array).
 */
template <class U>
[[nodiscard]] EBGEOMETRY_HOST const U*
uploadArray(unsigned char* a_base, size_t& a_offset, const std::vector<U>& a_host)
{
  static_assert(std::is_trivially_copyable_v<U>, "DeviceTape: uploaded element must be trivially copyable");

  if (a_host.empty()) {
    return nullptr;
  }

  const size_t numBytes = a_host.size() * sizeof(U);

  EBGEOMETRY_CUDA_CHECK(cudaMemcpy(a_base + a_offset, a_host.data(), numBytes, cudaMemcpyHostToDevice));

  const U* devicePtr = reinterpret_cast<const U*>(a_base + a_offset);

  a_offset += padUp(numBytes);

  return devicePtr;
}

} // namespace DeviceTapeDetail

template <class T>
DeviceTape<T>::DeviceTape(const Tape<T>& a_tape)
{
  // Always-on eligibility gate: in release the interpreter's device HOST_CALLBACK branch is a
  // no-op (EBGeometry_TapeImplem.hpp), so evaluating a non-eligible tape on device would silently
  // leave value slots unwritten. The EXPECT gives debug builds the standard diagnostic first.
  EBGEOMETRY_EXPECT(a_tape.isDeviceEligible());

  if (!a_tape.isDeviceEligible()) {
    std::fprintf(stderr, "EBGeometry::DeviceTape: tape contains HOST_CALLBACK clauses and cannot run on device\n");
    std::abort();
  }

  // A default-constructed Tape reports device-eligible but has zero clauses and zero slots;
  // evaluating its view would write coordScratch[0] out of bounds with assertions off. Every tape
  // produced by compile() has at least one clause, so this gate only rejects misuse.
  EBGEOMETRY_EXPECT(!a_tape.getClauses().empty());

  if (a_tape.getClauses().empty()) {
    std::fprintf(stderr, "EBGeometry::DeviceTape: tape is empty (not produced by compile())\n");
    std::abort();
  }

  using namespace DeviceTapeDetail;

  // ── Pass 1: size the pool ───────────────────────────────────────────────────
  size_t numBytes = 0;

  numBytes += paddedBytes(a_tape.m_clauses);

#define EBGEOMETRY_TAPE_DEVICE_BYTES(OP, MEMBER) numBytes += paddedBytes(a_tape.m_##MEMBER);
  EBGEOMETRY_TAPE_PARAM_SOA(EBGEOMETRY_TAPE_DEVICE_BYTES)
#undef EBGEOMETRY_TAPE_DEVICE_BYTES

  numBytes += paddedBytes(a_tape.m_scalarParams);
  numBytes += paddedBytes(a_tape.m_bvhNodes);
  numBytes += paddedBytes(a_tape.m_bvhChildren);
  numBytes += paddedBytes(a_tape.m_bvhLeaves);
  numBytes += paddedBytes(a_tape.m_bvhBlocks);
  // m_hostNodes is empty by device-eligibility (asserted above) -- never uploaded.

  m_numBytes = numBytes;

  if (numBytes > 0) {
    EBGEOMETRY_CUDA_CHECK(cudaMalloc(&m_pool, numBytes));
  }

  // ── Pass 2: copy each array and assemble the device view ────────────────────
  unsigned char* base   = static_cast<unsigned char*>(m_pool);
  size_t         offset = 0;

  m_view.clauses    = uploadArray(base, offset, a_tape.m_clauses);
  m_view.numClauses = static_cast<uint32_t>(a_tape.m_clauses.size());

#define EBGEOMETRY_TAPE_DEVICE_UPLOAD(OP, MEMBER) m_view.MEMBER = uploadArray(base, offset, a_tape.m_##MEMBER);
  EBGEOMETRY_TAPE_PARAM_SOA(EBGEOMETRY_TAPE_DEVICE_UPLOAD)
#undef EBGEOMETRY_TAPE_DEVICE_UPLOAD

  m_view.scalarParams = uploadArray(base, offset, a_tape.m_scalarParams);
  m_view.hostNodes    = nullptr;

  m_view.bvhNodes    = uploadArray(base, offset, a_tape.m_bvhNodes);
  m_view.bvhChildren = uploadArray(base, offset, a_tape.m_bvhChildren);
  m_view.bvhLeaves   = uploadArray(base, offset, a_tape.m_bvhLeaves);
  m_view.bvhBlocks   = uploadArray(base, offset, a_tape.m_bvhBlocks);

  m_view.numMainClauses = a_tape.m_numMainClauses;
  m_view.numCoordSlots  = a_tape.m_numCoordSlots;
  m_view.numValueSlots  = a_tape.m_numValueSlots;
  m_view.rootValueSlot  = a_tape.m_rootValueSlot;
  m_view.deviceEligible = true;

  EBGEOMETRY_EXPECT(offset == m_numBytes);
}

template <class T>
DeviceTape<T>::DeviceTape(DeviceTape&& a_other) noexcept
  : m_pool(a_other.m_pool), m_numBytes(a_other.m_numBytes), m_view(a_other.m_view)
{
  a_other.m_pool     = nullptr;
  a_other.m_numBytes = 0;
  a_other.m_view     = TapeView<T>();
}

template <class T>
DeviceTape<T>&
DeviceTape<T>::operator=(DeviceTape&& a_other) noexcept
{
  if (this != &a_other) {
    if (m_pool != nullptr) {
      (void)cudaFree(m_pool);
    }

    m_pool     = a_other.m_pool;
    m_numBytes = a_other.m_numBytes;
    m_view     = a_other.m_view;

    a_other.m_pool     = nullptr;
    a_other.m_numBytes = 0;
    a_other.m_view     = TapeView<T>();
  }

  return *this;
}

template <class T>
DeviceTape<T>::~DeviceTape() noexcept
{
  if (m_pool != nullptr) {
    (void)cudaFree(m_pool);
  }
}

} // namespace EBGeometry

#endif // EBGEOMETRY_CUDA || EBGEOMETRY_DOXYGEN

#endif // EBGEOMETRY_DEVICETAPEIMPLEM_HPP
