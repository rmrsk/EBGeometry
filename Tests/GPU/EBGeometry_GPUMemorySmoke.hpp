// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_GPUMemorySmoke.hpp
 * @brief  Backend-agnostic body of the compile-only GPU smoke test for the POD memory foundation.
 * @details Sub-tier 0 of the GPU port adds the placement-independent memory foundation
 * (MemoryResource / Pool / PODVector). This body exercises exactly that surface across the
 * host/device boundary: it builds a PODVector into a host @ref EBGeometry::Pool, mirrors the pool
 * into a @ref EBGeometry::DeviceMemoryResource with @ref EBGeometry::Pool::mirror (one host-to-device
 * copy), and launches a kernel that reads the array back via @c base + @c offset -- proving the
 * offset-based storage resolves correctly against a device base with no pointer patching. Building
 * it with an offload compiler proves the foundation's device-visible methods are callable from
 * device code. It is a compile-only check -- no GPU is needed to build it -- but it also runs
 * correctly on a device when one is present.
 *
 * The body is shared between the two backend translation units EBGeometry_GPUMemorySmoke.cu (CUDA)
 * and EBGeometry_GPUMemorySmoke.hip (HIP), which simply include this file; all runtime API calls go
 * through the backend-neutral @ref EBGeometry::GPU wrappers.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_GPUMEMORYSMOKE_HPP
#define EBGEOMETRY_GPUMEMORYSMOKE_HPP

#include <cstdint>

#include "Source/EBGeometry_GPURuntime.hpp"
#include "Source/EBGeometry_MemoryResource.hpp"
#include "Source/EBGeometry_PODVector.hpp"
#include "Source/EBGeometry_Pool.hpp"

using EBGeometry::PODVector;
using EBGeometry::Pool;

namespace GPU = EBGeometry::GPU;

/**
 * @brief A trivially-copyable device-visible element (deliberately independent of the value
 * vocabulary, which sub-tier 0 does not yet annotate for device use).
 */
struct SmokeElement
{
  double m_a;
  double m_b;
};

static_assert(std::is_trivially_copyable_v<SmokeElement>, "SmokeElement must be trivially copyable");

/**
 * @brief Kernel that reads a PODVector via base + offset and reduces it into a single scalar.
 * @param[in]  a_vec  PODVector descriptor (by value; trivially copyable, offsets are base-relative).
 * @param[in]  a_base Device base pointer of the mirrored pool.
 * @param[out] a_out  Single-element device buffer receiving the reduction.
 */
EBGEOMETRY_GLOBAL void
memorySmokeKernel(PODVector<SmokeElement> a_vec, void* a_base, double* a_out)
{
  double sum = 0.0;

  for (uint32_t i = 0; i < a_vec.size(); i++) {
    const SmokeElement element = a_vec.at(a_base, i);

    sum += element.m_a + element.m_b;
  }

  a_out[0] = sum;
}

/**
 * @brief Host entry point. Builds a pool on the host, mirrors it to the device, launches the
 * kernel, and reads the result back.
 * @details The runtime API results are deliberately discarded: this program is a compile-only
 * portability check that must also remain runnable (as a harmless no-op) on a machine without any
 * GPU device.
 * @return Zero on success.
 */
int
main()
{
  // Build a small POD array into a host pool.
  Pool hostPool(EBGeometry::hostMemoryResource());

  PODVector<SmokeElement> vec;
  vec.reserveFrom(hostPool, 8);

  for (uint32_t i = 0; i < 8; i++) {
    vec.push_back(hostPool.base(), SmokeElement{double(i), double(2 * i)});
  }

  hostPool.freeze();

  // Mirror the whole pool to the device in one copy; offsets resolve unchanged against the new base.
  Pool devicePool = Pool::mirror(hostPool, EBGeometry::deviceMemoryResource());

  double* deviceOut = nullptr;
  (void)GPU::memAlloc(reinterpret_cast<void**>(&deviceOut), sizeof(double));

  memorySmokeKernel<<<1, 1>>>(vec, devicePool.base(), deviceOut);
  (void)GPU::deviceSynchronize();

  double hostOut = 0.0;
  (void)GPU::memcpy(&hostOut, deviceOut, sizeof(double), GPU::MemcpyDeviceToHost);
  (void)GPU::memFree(deviceOut);

  return 0;
}

#endif // EBGEOMETRY_GPUMEMORYSMOKE_HPP
