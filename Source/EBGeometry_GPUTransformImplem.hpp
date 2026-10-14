// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_GPUTransformImplem.hpp
 * @brief  Implementation of EBGeometry_GPUTransform.hpp.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_GPUTRANSFORMIMPLEM_HPP
#define EBGEOMETRY_GPUTRANSFORMIMPLEM_HPP

// Our includes
#include "EBGeometry_GPUTransform.hpp"

namespace EBGeometry {
namespace GPU {

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)

/**
 * @brief Implementation details of GPU::transform().
 */
namespace TransformDetail {

/**
 * @brief Kernel: a_out[i] = a_query(a_in[i]), one thread per input.
 * @tparam Query Query functor type.
 * @tparam In    Input type.
 * @tparam Out   Result type.
 * @param[in]  a_query The query, passed by value.
 * @param[in]  a_in    Inputs in device memory.
 * @param[out] a_out   Results in device memory.
 * @param[in]  a_n     Number of inputs.
 */
template <class Query, class In, class Out>
EBGEOMETRY_GLOBAL
void
transformKernel(const Query a_query, const In* a_in, Out* a_out, const std::size_t a_n)
{
  const std::size_t i = static_cast<std::size_t>(blockIdx.x) * blockDim.x + threadIdx.x;

  if (i < a_n) {
    a_out[i] = a_query(a_in[i]);
  }
}

} // namespace TransformDetail

#endif

inline bool
deviceAvailable() noexcept
{
#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  int count = 0;

  return getDeviceCount(&count) == Success && count > 0;
#else
  return false;
#endif
}

template <class Query, class In>
inline std::vector<TransformResult<Query, In>>
transform(const Query& a_query, const std::vector<In>& a_inputs)
{
  using Out = TransformResult<Query, In>;

  static_assert(std::is_trivially_copyable_v<Query>, "GPU::transform: a query must be trivially copyable");
  static_assert(std::is_trivially_copyable_v<In>, "GPU::transform: an input must be trivially copyable");
  static_assert(std::is_trivially_copyable_v<Out>, "GPU::transform: a result must be trivially copyable");

  const std::size_t n = a_inputs.size();

  std::vector<Out> results(n);

  if (n == 0) {
    return results;
  }

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  In*  in  = nullptr;
  Out* out = nullptr;

  EBGEOMETRY_GPU_CHECK(memAlloc(reinterpret_cast<void**>(&in), sizeof(In) * n));
  EBGEOMETRY_GPU_CHECK(memAlloc(reinterpret_cast<void**>(&out), sizeof(Out) * n));
  EBGEOMETRY_GPU_CHECK(memcpy(in, a_inputs.data(), sizeof(In) * n, MemcpyHostToDevice));

  constexpr std::size_t threads = 128;

  const auto blocks = static_cast<unsigned int>((n + threads - 1) / threads);

  TransformDetail::transformKernel<Query, In, Out><<<blocks, static_cast<unsigned int>(threads)>>>(a_query, in, out, n);

  EBGEOMETRY_GPU_CHECK(getLastError());
  EBGEOMETRY_GPU_CHECK(deviceSynchronize());
  EBGEOMETRY_GPU_CHECK(memcpy(results.data(), out, sizeof(Out) * n, MemcpyDeviceToHost));
  EBGEOMETRY_GPU_CHECK(memFree(in));
  EBGEOMETRY_GPU_CHECK(memFree(out));
#else
  // Copied once, as a kernel launch copies its arguments.
  const Query query = a_query;

  for (std::size_t i = 0; i < n; i++) {
    results[i] = query(a_inputs[i]);
  }
#endif

  return results;
}

} // namespace GPU
} // namespace EBGeometry

#endif
