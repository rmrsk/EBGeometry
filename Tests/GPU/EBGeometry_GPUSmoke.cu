// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_GPUSmoke.cu
 * @brief  Compile-only CUDA smoke test for the GPU-port device annotations.
 * @details Tier 0 of the GPU port only annotates the value vocabulary (Vec2T/Vec3T and the
 * bounding-volume types) with EBGEOMETRY_HOST_DEVICE. This translation unit exercises exactly that
 * surface from inside a __global__ kernel, so that building it with nvcc proves the annotated
 * methods are callable from device code. It is a compile-only check -- no GPU is needed to build
 * it -- but it also runs correctly on a device when one is present.
 * @author Robert Marskar
 */

#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_Vec.hpp"

using T = double;

using EBGeometry::Vec3T;
using EBGeometry::BoundingVolumes::AABBT;
using EBGeometry::BoundingVolumes::SphereT;

/**
 * @brief Kernel that exercises the host/device value vocabulary on the device.
 * @param[out] a_out Single-element device buffer receiving a combined scalar result.
 */
EBGEOMETRY_GLOBAL void
smokeKernel(T* a_out)
{
  const Vec3T<T> a(T(1), T(2), T(3));
  const Vec3T<T> b(T(4), T(5), T(6));
  const Vec3T<T> c = a + b;

  const AABBT<T>  box(Vec3T<T>(T(0), T(0), T(0)), Vec3T<T>(T(1), T(1), T(1)));
  const SphereT<T> sphere(Vec3T<T>(T(0), T(0), T(0)), T(1));

  a_out[0] = EBGeometry::dot(c, c) + box.getDistance2(a) + sphere.getDistance(b) + c.length();
}

/**
 * @brief Host entry point. Launches the kernel and reads back the result.
 * @return Zero on success.
 */
int
main()
{
  T* deviceOut = nullptr;
  cudaMalloc(reinterpret_cast<void**>(&deviceOut), sizeof(T));

  smokeKernel<<<1, 1>>>(deviceOut);
  cudaDeviceSynchronize();

  T hostOut = T(0);
  cudaMemcpy(&hostOut, deviceOut, sizeof(T), cudaMemcpyDeviceToHost);
  cudaFree(deviceOut);

  return 0;
}
