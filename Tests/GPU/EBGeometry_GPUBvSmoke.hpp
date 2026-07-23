// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_GPUBvSmoke.hpp
 * @brief  Backend-agnostic body of the compile-only GPU smoke test for the bounding-volume types.
 * @details Compile-only device-portability smoke test for the bounding-volume value types. This
 * body constructs an @ref EBGeometry::BoundingVolumes::AABBT and an
 * @ref EBGeometry::BoundingVolumes::SphereT from explicit corners/centre inside a kernel and
 * evaluates their query surface -- @c getDistance / @c getDistance2, @c intersects, @c getCentroid,
 * @c getVolume / @c getArea, @c getOverlappingVolume, and the corner accessors -- plus the free
 * @c intersects / @c getOverlappingVolume. No GPU is needed to build it, and it also runs correctly
 * on a device when one is present.
 *
 * The body is shared between the two backend translation units EBGeometry_GPUBvSmoke.cu (CUDA) and
 * EBGeometry_GPUBvSmoke.hip (HIP), which simply include this file; all runtime API calls go through
 * the backend-neutral @ref EBGeometry::GPU wrappers.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_GPUBVSMOKE_HPP
#define EBGEOMETRY_GPUBVSMOKE_HPP

#include <type_traits>

#include "Source/EBGeometry_BoundingVolumes.hpp"
#include "Source/EBGeometry_GPURuntime.hpp"

using EBGeometry::Vec3T;
using EBGeometry::BoundingVolumes::AABBT;
using EBGeometry::BoundingVolumes::SphereT;

namespace GPU = EBGeometry::GPU;

static_assert(std::is_trivially_copyable_v<AABBT<double>>, "AABBT<double> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<SphereT<double>>, "SphereT<double> must be trivially copyable");

/**
 * @brief Kernel that exercises the device-callable AABBT/SphereT surface and reduces it to a scalar.
 * @param[out] a_out Single-element device buffer receiving the reduction.
 */
EBGEOMETRY_GLOBAL
void
bvSmokeKernel(double* a_out)
{
  const AABBT<double>   box(Vec3T<double>(0.0, 0.0, 0.0), Vec3T<double>(1.0, 1.0, 1.0));
  const AABBT<double>   other(Vec3T<double>(0.5, 0.5, 0.5), Vec3T<double>(2.0, 2.0, 2.0));
  const SphereT<double> sphere(Vec3T<double>(0.5, 0.5, 0.5), 0.5);

  const Vec3T<double> p(2.0, 2.0, 2.0);

  double d = box.getDistance(p) + box.getDistance2(p) + box.getVolume() + box.getArea();
  d += box.getCentroid().length() + box.getLowCorner().length() + box.getHighCorner().length();
  d += (box.intersects(other) ? 1.0 : 0.0) + box.getOverlappingVolume(other);

  d += sphere.getDistance(p) + sphere.getDistance2(p) + sphere.getVolume() + sphere.getArea();
  d += sphere.getCentroid().length() + sphere.getRadius();
  d += (sphere.intersects(sphere) ? 1.0 : 0.0) + sphere.getOverlappingVolume(sphere);

  d += (intersects(box, other) ? 1.0 : 0.0) + getOverlappingVolume(box, other);

  a_out[0] = d;
}

/**
 * @brief Host entry point. Launches the kernel and reads the result back.
 * @details The runtime API results are deliberately discarded: this program is a compile-only
 * portability check that must also remain runnable (as a harmless no-op) on a machine without any
 * GPU device.
 * @return Zero on success.
 */
int
main()
{
  double* deviceOut = nullptr;
  (void)GPU::memAlloc(reinterpret_cast<void**>(&deviceOut), sizeof(double));

  bvSmokeKernel<<<1, 1>>>(deviceOut);
  (void)GPU::deviceSynchronize();

  double hostOut = 0.0;
  (void)GPU::memcpy(&hostOut, deviceOut, sizeof(double), GPU::MemcpyDeviceToHost);
  (void)GPU::memFree(deviceOut);

  return 0;
}

#endif // EBGEOMETRY_GPUBVSMOKE_HPP
