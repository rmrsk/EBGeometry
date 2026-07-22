// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_GPUVecSmoke.hpp
 * @brief  Backend-agnostic body of the compile-only GPU smoke test for the Vec value types.
 * @details The first migration step of the GPU port makes @ref EBGeometry::Vec2T /
 * @ref EBGeometry::Vec3T device-callable and trivially copyable. This body exercises exactly that:
 * it passes a couple of @ref EBGeometry::Vec3T values by value into a kernel and evaluates the
 * arithmetic/query surface -- arithmetic operators, @c dot, @c cross, @c length, the free
 * @c min / @c max / @c clamp, comparisons, and @c minDir / @c maxDir -- entirely in device code.
 * Building it with an offload compiler proves those methods are callable from a kernel; it is a
 * compile-only check (no GPU is needed to build it) but also runs correctly on a device when one is
 * present.
 *
 * The body is shared between the two backend translation units EBGeometry_GPUVecSmoke.cu (CUDA) and
 * EBGeometry_GPUVecSmoke.hip (HIP), which simply include this file; all runtime API calls go through
 * the backend-neutral @ref EBGeometry::GPU wrappers.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_GPUVECSMOKE_HPP
#define EBGEOMETRY_GPUVECSMOKE_HPP

#include <type_traits>

#include "Source/EBGeometry_GPURuntime.hpp"
#include "Source/EBGeometry_Vec.hpp"

using EBGeometry::Vec3T;

namespace GPU = EBGeometry::GPU;

static_assert(std::is_trivially_copyable_v<Vec3T<double>>, "Vec3T<double> must be trivially copyable");

/**
 * @brief Kernel that exercises the device-callable Vec3T surface and reduces it to one scalar.
 * @param[in]  a_a   First vector (by value; trivially copyable).
 * @param[in]  a_b   Second vector (by value).
 * @param[out] a_out Single-element device buffer receiving the reduction.
 */
EBGEOMETRY_GLOBAL
void
vecSmokeKernel(Vec3T<double> a_a, Vec3T<double> a_b, double* a_out)
{
  Vec3T<double> c = a_a + a_b - a_b * 2.0 + a_a / 2.0;
  c += a_a;
  c[0] = c[1];

  const Vec3T<double> mn = min(a_a, a_b);
  const Vec3T<double> mx = max(a_a, a_b);
  const Vec3T<double> cl = clamp(c, mn, mx);

  double d = dot(a_a, a_b) + cross(a_a, a_b).length() + c.length2() + cl.dot(mx);

  d += (a_a < a_b) ? mn.dot(mx) : cl.dot(a_a);
  d += static_cast<double>(a_a.minDir(true) + a_a.maxDir(false));

  a_out[0] = d;
}

/**
 * @brief Host entry point. Launches the kernel with two vectors and reads the result back.
 * @details The runtime API results are deliberately discarded: this program is a compile-only
 * portability check that must also remain runnable (as a harmless no-op) on a machine without any
 * GPU device.
 * @return Zero on success.
 */
int
main()
{
  const Vec3T<double> a(1.0, 2.0, 3.0);
  const Vec3T<double> b = Vec3T<double>::ones();

  double* deviceOut = nullptr;
  (void)GPU::memAlloc(reinterpret_cast<void**>(&deviceOut), sizeof(double));

  vecSmokeKernel<<<1, 1>>>(a, b, deviceOut);
  (void)GPU::deviceSynchronize();

  double hostOut = 0.0;
  (void)GPU::memcpy(&hostOut, deviceOut, sizeof(double), GPU::MemcpyDeviceToHost);
  (void)GPU::memFree(deviceOut);

  return 0;
}

#endif // EBGEOMETRY_GPUVECSMOKE_HPP
