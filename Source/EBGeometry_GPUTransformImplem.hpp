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
#include "EBGeometry_Macros.hpp"

namespace EBGeometry {
namespace GPU {

/**
 * @brief Implementation details of GPU::parallelFor() and GPU::transform().
 */
namespace TransformDetail {

/**
 * @brief Compile-time checks on a block size.
 * @tparam BlockSize Threads per block.
 */
template <unsigned int BlockSize>
inline constexpr void
checkBlockSize() noexcept
{
  static_assert(BlockSize > 0 && BlockSize % 64 == 0,
                "GPU: a block size must be a positive multiple of 64 (an AMD wavefront, two NVIDIA warps)");
  static_assert(BlockSize <= 1024, "GPU: a block size must not exceed 1024 threads");
}

/**
 * @brief The functor the span transform runs: a_out[i] = a_query(a_in[i]).
 * @tparam Query Query functor type.
 * @tparam In    Input type, const or not.
 * @tparam Out   Result type.
 */
template <class Query, class In, class Out>
struct Apply
{
  /// @brief The query.
  Query m_query;

  /// @brief The inputs.
  In* m_in;

  /// @brief The results.
  Out* m_out;

  /**
   * @brief Run the query on input i.
   * @param[in] a_i Index of the input.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(std::size_t a_i) const
  {
    m_out[a_i] = m_query(m_in[a_i]);
  }
};

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)

/**
 * @brief Kernel: a_f(i) for every i < a_n, one thread per index.
 * @details Launch bounds tell the compiler that no block exceeds BlockSize threads. Without them it
 * must assume the largest block (1024) and allocate registers for it.
 * @tparam BlockSize Threads per block.
 * @tparam F         Functor type.
 * @param[in] a_f The functor, passed by value.
 * @param[in] a_n Number of indices.
 */
template <unsigned int BlockSize, class F>
EBGEOMETRY_GLOBAL
void
__launch_bounds__(BlockSize) parallelForKernel(const F a_f, const std::size_t a_n)
{
  const std::size_t i = static_cast<std::size_t>(blockIdx.x) * blockDim.x + threadIdx.x;

  if (i < a_n) {
    a_f(i);
  }
}

#endif

} // namespace TransformDetail

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

inline MemoryResource&
memoryResource() noexcept
{
#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  return deviceMemoryResource();
#else
  return emulatedDeviceMemoryResource();
#endif
}

template <unsigned int BlockSize, class F>
inline void
parallelFor(const std::size_t a_n, const F& a_f)
{
  TransformDetail::checkBlockSize<BlockSize>();

  static_assert(std::is_trivially_copyable_v<F>, "GPU::parallelFor: a functor must be trivially copyable");

  // A grid holds at most 2^31 - 1 blocks along x in CUDA and HIP. Checked in every build, so that a
  // count a kernel could not cover fails the same way on the host.
  constexpr std::size_t maxBlocks = (std::size_t(1) << 31) - 1;

  EBGEOMETRY_REQUIRE(a_n <= maxBlocks * BlockSize,
                     "GPU::parallelFor: at most (2^31 - 1) * %u indices at this block size (%zu)",
                     BlockSize,
                     a_n);

  if (a_n == 0) {
    return;
  }

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  const auto blocks = static_cast<unsigned int>((a_n + BlockSize - 1) / BlockSize);

  TransformDetail::parallelForKernel<BlockSize, F><<<blocks, BlockSize>>>(a_f, a_n);

  EBGEOMETRY_GPU_CHECK(getLastError());
  EBGEOMETRY_GPU_CHECK(deviceSynchronize());
#else
  // Copied once, as a kernel launch copies its arguments.
  const F f = a_f;

  for (std::size_t i = 0; i < a_n; i++) {
    f(i);
  }
#endif
}

template <unsigned int BlockSize, class Query, class In, class Out>
inline void
transform(const Query& a_query, const PODSpan<In> a_in, const PODSpan<Out> a_out)
{
  using Result = TransformResult<Query, std::remove_const_t<In>>;

  static_assert(std::is_trivially_copyable_v<Query>, "GPU::transform: a query must be trivially copyable");
  static_assert(!std::is_const_v<Out>, "GPU::transform: the results must be writable");
  static_assert(std::is_same_v<Out, Result>, "GPU::transform: the results must have the query's result type");

  EBGEOMETRY_REQUIRE(a_in.size() == a_out.size(),
                     "GPU::transform: there must be as many results as inputs (%u inputs, %u results)",
                     a_in.size(),
                     a_out.size());

  // A thread reads its input before it writes its result, so the results may overwrite the inputs in
  // place, element for element. A partial overlap would have one thread read what another writes.
  const auto inBegin  = reinterpret_cast<std::uintptr_t>(a_in.m_ptr);
  const auto outBegin = reinterpret_cast<std::uintptr_t>(a_out.m_ptr);
  const auto inEnd    = inBegin + std::uintptr_t(a_in.size()) * sizeof(In);
  const auto outEnd   = outBegin + std::uintptr_t(a_out.size()) * sizeof(Out);

  const bool inPlace  = std::is_same_v<std::remove_const_t<In>, Out> && inBegin == outBegin;
  const bool disjoint = inEnd <= outBegin || outEnd <= inBegin;

  EBGEOMETRY_REQUIRE(a_in.size() == 0 || inPlace || disjoint,
                     "GPU::transform: the inputs and the results must be the same array or not overlap");

  parallelFor<BlockSize>(a_in.size(), TransformDetail::Apply<Query, In, Out>{a_query, a_in.m_ptr, a_out.m_ptr});
}

template <unsigned int BlockSize, class Query, class In, class Out>
inline void
transform(const Query& a_query, const Pool& a_pool, const PODVector<In>& a_in, const PODVector<Out>& a_out)
{
  EBGEOMETRY_REQUIRE(a_pool.isFrozen(), "GPU::transform: the pool must be frozen");
  EBGEOMETRY_REQUIRE(a_pool.resource().isDeviceAccessible(),
                     "GPU::transform: the pool must be in device-accessible memory (a mirror into "
                     "GPU::memoryResource(), or managed memory)");

  // A PODVector holds an offset, not its pool, so this is as far as membership can be checked: an array
  // reserved after the pool was mirrored, or from a larger pool, ends past it.
  EBGEOMETRY_REQUIRE(a_in.endByte() <= a_pool.usedBytes() && a_out.endByte() <= a_pool.usedBytes(),
                     "GPU::transform: the arrays must fit inside the pool (they end at bytes %llu and %llu, the "
                     "pool holds %zu)",
                     static_cast<unsigned long long>(a_in.endByte()),
                     static_cast<unsigned long long>(a_out.endByte()),
                     a_pool.usedBytes());

  // The inputs are bound against a const base, so the query sees them read-only.
  const void* const inBase = a_pool.base();

  transform<BlockSize>(a_query, a_in.bind(inBase), a_out.bind(a_pool.base()));
}

template <unsigned int BlockSize, class Query, class In, class Out>
inline void
transform(const Query& a_query, const std::vector<In>& a_in, std::vector<Out>& a_out)
{
  static_assert(std::is_trivially_copyable_v<In>, "GPU::transform: an input must be trivially copyable");
  static_assert(std::is_trivially_copyable_v<Out>, "GPU::transform: a result must be trivially copyable");
  static_assert(!std::is_same_v<In, bool> && !std::is_same_v<Out, bool>,
                "GPU::transform: std::vector<bool> has no contiguous storage; use char or a struct instead");
  static_assert(std::is_same_v<Out, TransformResult<Query, In>>,
                "GPU::transform: the results must have the query's result type");

  const std::size_t n = a_in.size();

  EBGEOMETRY_REQUIRE(a_out.size() == n,
                     "GPU::transform: there must be as many results as inputs (%zu inputs, %zu results)",
                     n,
                     a_out.size());
  EBGEOMETRY_REQUIRE(n <= std::size_t(UINT32_MAX), "GPU::transform: at most 2^32 - 1 inputs (%zu)", n);

  if (n == 0) {
    return;
  }

  const auto count = static_cast<uint32_t>(n);

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  // EBGEOMETRY_GPU_CHECK aborts on an error, so no path leaves this block with in or out allocated.
  In*  in  = nullptr;
  Out* out = nullptr;

  EBGEOMETRY_GPU_CHECK(memAlloc(reinterpret_cast<void**>(&in), sizeof(In) * n));
  EBGEOMETRY_GPU_CHECK(memAlloc(reinterpret_cast<void**>(&out), sizeof(Out) * n));
  EBGEOMETRY_GPU_CHECK(memcpy(in, a_in.data(), sizeof(In) * n, MemcpyHostToDevice));

  transform<BlockSize>(a_query, PODSpan<const In>{in, count}, PODSpan<Out>{out, count});

  EBGEOMETRY_GPU_CHECK(memcpy(a_out.data(), out, sizeof(Out) * n, MemcpyDeviceToHost));
  EBGEOMETRY_GPU_CHECK(memFree(in));
  EBGEOMETRY_GPU_CHECK(memFree(out));
#else
  transform<BlockSize>(a_query, PODSpan<const In>{a_in.data(), count}, PODSpan<Out>{a_out.data(), count});
#endif
}

} // namespace GPU
} // namespace EBGeometry

#endif
