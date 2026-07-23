// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_GPUSoASmoke.hpp
 * @brief  Backend-agnostic body of the compile-only GPU smoke test for the SoA/AoSoA leaf types.
 * @details Compile-only device-portability smoke test for the SIMD leaf types
 * @ref EBGeometry::PointAoSoA and @ref EBGeometry::TriangleAoSoA (and, through them, the embedded
 * @ref EBGeometry::PointSoAT / @ref EBGeometry::TriangleSoAT). The groups are packed on the host --
 * pack() is a host-only operation -- and, being trivially copyable, are byte-mirrored to the device.
 * A kernel then exercises their device-callable query surface: PointAoSoA's getMinimumDistance2 /
 * getMaximumDistance2 / getDistances2 / getMetaData / computeBoundingVolume, and TriangleAoSoA's
 * signedDistance (both the plain and the metadata-reporting overload) / getMetaData /
 * computeBoundingVolume. The result is reduced to a single scalar. No GPU is needed to build it, and
 * it also runs correctly on a device when one is present.
 *
 * The width W is pinned to 4 (rather than the ISA-dependent PointSoA::DefaultWidth) so the group
 * types are identical across the host and device compilation passes of the same translation unit.
 *
 * The body is shared between the two backend translation units EBGeometry_GPUSoASmoke.cu (CUDA) and
 * EBGeometry_GPUSoASmoke.hip (HIP), which simply include this file; all runtime API calls go through
 * the backend-neutral @ref EBGeometry::GPU wrappers.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_GPUSOASMOKE_HPP
#define EBGEOMETRY_GPUSOASMOKE_HPP

#include <array>
#include <cstdint>
#include <type_traits>

#include "Source/EBGeometry_BoundingVolumes.hpp"
#include "Source/EBGeometry_GPURuntime.hpp"
#include "Source/EBGeometry_PointAoSoA.hpp"
#include "Source/EBGeometry_Triangle.hpp"
#include "Source/EBGeometry_TriangleAoSoA.hpp"
#include "Source/EBGeometry_Vec.hpp"

using EBGeometry::PointAoSoA;
using EBGeometry::Triangle;
using EBGeometry::TriangleAoSoA;
using EBGeometry::Vec3T;
using EBGeometry::BoundingVolumes::AABBT;

namespace GPU = EBGeometry::GPU;

// Pinned width: independent of the target ISA so host and device passes agree on the type.
constexpr size_t SmokeWidth = 4;

using PointGroup    = PointAoSoA<double, uint32_t, SmokeWidth>;
using TriangleGroup = TriangleAoSoA<double, uint32_t, SmokeWidth>;

static_assert(std::is_trivially_copyable_v<PointGroup>, "PointAoSoA must be trivially copyable");
static_assert(std::is_trivially_copyable_v<TriangleGroup>, "TriangleAoSoA must be trivially copyable");

/**
 * @brief Kernel that exercises the device-callable PointAoSoA/TriangleAoSoA surface.
 * @param[in]  a_points    Device-resident, host-packed point group.
 * @param[in]  a_triangles Device-resident, host-packed triangle group.
 * @param[out] a_out       Single-element device buffer receiving the scalar reduction.
 */
EBGEOMETRY_GLOBAL
void
soaSmokeKernel(const PointGroup* a_points, const TriangleGroup* a_triangles, double* a_out)
{
  const Vec3T<double> p(0.25, 0.25, 0.25);

  double d = 0.0;

  d += a_points->getMinimumDistance2(p) + a_points->getMaximumDistance2(p);
  d += a_points->getDistances2(p)[0];
  d += static_cast<double>(a_points->getMetaData(0));
  d += a_points->template computeBoundingVolume<AABBT<double>>().getVolume();

  uint32_t closestMeta = 0;
  d += a_triangles->signedDistance(p);
  d += a_triangles->signedDistance(p, closestMeta);
  d += static_cast<double>(closestMeta);
  d += static_cast<double>(a_triangles->getMetaData(0));
  d += a_triangles->template computeBoundingVolume<AABBT<double>>().getVolume();

  a_out[0] = d;
}

/**
 * @brief Host entry point. Packs the groups on the host, mirrors them to the device, launches the
 * kernel, and reads the result back.
 * @details The runtime API results are deliberately discarded: this program is a compile-only
 * portability check that must also remain runnable (as a harmless no-op) on a machine without any
 * GPU device.
 * @return Zero on success.
 */
int
main()
{
  // Pack a point group on the host.
  std::array<Vec3T<double>, SmokeWidth> positions = {Vec3T<double>(0.0, 0.0, 0.0),
                                                     Vec3T<double>(1.0, 0.0, 0.0),
                                                     Vec3T<double>(0.0, 1.0, 0.0),
                                                     Vec3T<double>(0.0, 0.0, 1.0)};
  std::array<uint32_t, SmokeWidth>      pointMeta = {10, 11, 12, 13};

  PointGroup hostPoints;
  hostPoints.pack(positions.data(), pointMeta.data(), SmokeWidth);

  // Pack a triangle group on the host.
  std::array<Triangle<double, uint32_t>, SmokeWidth> triangles;
  for (uint32_t i = 0; i < SmokeWidth; i++) {
    const Vec3T<double> base(static_cast<double>(i), 0.0, 0.0);

    triangles[i] = Triangle<double, uint32_t>(
      std::array<Vec3T<double>, 3>{base, base + Vec3T<double>(1, 0, 0), base + Vec3T<double>(0, 1, 0)});
    triangles[i].setMetaData(20 + i);
  }

  TriangleGroup hostTriangles;
  hostTriangles.pack(triangles.data(), SmokeWidth);

  // Mirror both trivially-copyable groups to the device.
  PointGroup*    devicePoints    = nullptr;
  TriangleGroup* deviceTriangles = nullptr;
  double*        deviceOut       = nullptr;

  (void)GPU::memAlloc(reinterpret_cast<void**>(&devicePoints), sizeof(PointGroup));
  (void)GPU::memAlloc(reinterpret_cast<void**>(&deviceTriangles), sizeof(TriangleGroup));
  (void)GPU::memAlloc(reinterpret_cast<void**>(&deviceOut), sizeof(double));

  (void)GPU::memcpy(devicePoints, &hostPoints, sizeof(PointGroup), GPU::MemcpyHostToDevice);
  (void)GPU::memcpy(deviceTriangles, &hostTriangles, sizeof(TriangleGroup), GPU::MemcpyHostToDevice);

  soaSmokeKernel<<<1, 1>>>(devicePoints, deviceTriangles, deviceOut);
  (void)GPU::deviceSynchronize();

  double hostOut = 0.0;
  (void)GPU::memcpy(&hostOut, deviceOut, sizeof(double), GPU::MemcpyDeviceToHost);

  (void)GPU::memFree(devicePoints);
  (void)GPU::memFree(deviceTriangles);
  (void)GPU::memFree(deviceOut);

  return 0;
}

#endif // EBGEOMETRY_GPUSOASMOKE_HPP
