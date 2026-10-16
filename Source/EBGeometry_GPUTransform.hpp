// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_GPUTransform.hpp
 * @brief  GPU::parallelFor() and GPU::transform(), which run a functor over many indices or inputs: in
 *         a kernel in a CUDA or HIP build, and in a host loop otherwise.
 * @details GPU::transform() works like std::transform or thrust::transform: it writes a_query(in[i])
 * to out[i], into an output the caller provides. A query is a trivially copyable functor whose call operator is
 * EBGEOMETRY_HOST_DEVICE and maps one input to one result. It holds what it evaluates by value: an
 * analytic shape, or a pool-resident object rebased onto memory the device can read (a mirror in
 * GPU::memoryResource(), or a frozen managed pool; see the memory model in the user documentation).
 * One source then serves both builds: a CUDA or HIP translation unit launches a kernel, and any other
 * runs the same functor in a host loop, so a plain C++ build needs no GPU toolkit. The GPU:: prefix
 * names where these calls can run, not where they always do. GPU::enabled says which of the two a
 * translation unit gets, and GPU::deviceAvailable() whether a device is visible at run time.
 *
 * The calls are layered, each built on the one below:
 * - GPU::parallelFor(n, f) runs f(i) for every i < n, one thread per index.
 * - GPU::transform(query, in, out) over two PODSpan's: no allocation and no copies, the data is
 *   wherever the caller put it (managed memory, a buffer an application already owns on the device,
 *   a PODVector bound against a mirrored pool). A span carries no memory location, so nothing checks
 *   that the device can reach it. Timing this call times the kernel.
 * - GPU::transform(query, pool, in, out) over two PODVector's in one pool: the span overload, after
 *   checking that the pool is frozen and in memory the device can access. No allocation and no
 *   copies either.
 * - GPU::transform(query, in, out) over two std::vector's: a convenience that also allocates device
 *   memory, copies the inputs in and the results back, and frees. Timing it includes those copies.
 *
 * Every launch takes its block size as a template parameter (GPU::DefaultBlockSize unless given), so
 * that the kernel can carry it as launch bounds. Every call is synchronous: it returns once the
 * kernel has finished. Every transform requires as many results as inputs, and results of exactly the
 * query's result type.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_GPUTRANSFORM_HPP
#define EBGEOMETRY_GPUTRANSFORM_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_GPURuntime.hpp"
#include "EBGeometry_MemoryResource.hpp"
#include "EBGeometry_PODVector.hpp"

namespace EBGeometry {
namespace GPU {

/**
 * @brief Whether this translation unit is compiled as CUDA or HIP, so that parallelFor() and
 * transform() run a kernel.
 */
#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
inline constexpr bool enabled = true;
#else
inline constexpr bool enabled = false;
#endif

/**
 * @brief Where parallelFor() and transform() run in this translation unit: "HIP", "CUDA" or "host".
 */
#if defined(EBGEOMETRY_HIP)
inline constexpr const char* backendName = "HIP";
#elif defined(EBGEOMETRY_CUDA)
inline constexpr const char* backendName = "CUDA";
#else
inline constexpr const char* backendName = "host";
#endif

/**
 * @brief The number of threads per block a launch uses unless it is given another.
 * @details Not tuned: the best block size depends on the device and on the query (a BVH traversal
 * uses many registers per thread). Pass another as the first template argument to measure it, e.g.
 * GPU::transform<256>(query, points).
 */
inline constexpr unsigned int DefaultBlockSize = 128;

/**
 * @brief Whether a GPU is visible to the runtime.
 * @details Always false in a host build. In a CUDA or HIP build, false also when the runtime itself
 * reports an error (no driver, for instance) rather than a device count.
 * @return True if at least one device is visible.
 */
[[nodiscard]] EBGEOMETRY_HOST
inline bool
deviceAvailable() noexcept;

/**
 * @brief The memory a query's data must live in for parallelFor() and transform() to read it.
 * @details deviceMemoryResource() in a CUDA or HIP build, and emulatedDeviceMemoryResource() (host
 * memory that reports itself device-accessible) otherwise. Mirroring a frozen pool into it and taking
 * a rebasedView() onto the mirror is then the same code in both builds: a device copy that a kernel
 * reads, or a host copy that the host loop reads.
 * @return The memory resource.
 */
[[nodiscard]] EBGEOMETRY_HOST
inline MemoryResource&
memoryResource() noexcept;

/**
 * @brief The result type of a query on an input.
 * @tparam Query Query functor type.
 * @tparam In    Input type.
 */
template <class Query, class In>
using TransformResult = std::decay_t<std::invoke_result_t<const Query&, const In&>>;

/**
 * @brief Run a functor on every index: a_f(i) for i = 0, ..., a_n - 1, in no particular order.
 * @details In a CUDA or HIP build, a kernel runs the functor once per index (one thread each, in
 * blocks of BlockSize threads) on the current device, and the call returns once it has finished. Every
 * runtime call is checked with EBGEOMETRY_GPU_CHECK, which aborts with the failing call on an error,
 * and a visible device is required (see deviceAvailable()). The functor reaches the kernel by value,
 * so anything it reads or writes must be memory the device can access.
 *
 * In a host build the functor is copied once, as a kernel launch would copy it, and run in a loop.
 * The functor must be trivially copyable in both builds, which is checked at compile time. A grid holds
 * at most 2^31 - 1 blocks, so a_n may not exceed (2^31 - 1) * BlockSize; a larger count aborts with a
 * message, in both builds.
 * @tparam BlockSize Threads per block: a multiple of 64 (an AMD wavefront, and two NVIDIA warps), at
 * most 1024. The kernel carries it as launch bounds, so the compiler can give each thread the
 * registers that block size allows.
 * @tparam F         Trivially copyable functor with an EBGEOMETRY_HOST_DEVICE call operator taking a
 * std::size_t.
 * @param[in] a_n Number of indices.
 * @param[in] a_f The functor.
 */
template <unsigned int BlockSize = DefaultBlockSize, class F>
EBGEOMETRY_HOST
inline void
parallelFor(std::size_t a_n, const F& a_f);

/**
 * @brief Run a query on every input in place: a_out[i] = a_query(a_in[i]), with no allocation and no
 * copies.
 * @details The inputs and the results stay wherever the caller put them, so this is the call to time
 * when a query's cost is what is being measured, and the one to use for repeated queries over the
 * same points. A span is a pointer and a count: bind a PODVector against the base of a pool mirrored
 * into memoryResource() (inputs) or of a pool reserved there (results), or wrap managed memory or a
 * buffer the application already owns on the device. In a host build the spans are host memory. The
 * query runs through parallelFor(), with its requirements.
 * @tparam BlockSize Threads per block; see parallelFor().
 * @tparam Query     Trivially copyable functor with an EBGEOMETRY_HOST_DEVICE call operator taking a
 * const In&.
 * @tparam In        Input type, const or not.
 * @tparam Out       Result type: the query's result type, not const.
 * @param[in]  a_query The query.
 * @param[in]  a_in    The inputs, in memory the device can read.
 * @param[out] a_out   The results, in memory the device can write; as many as there are inputs.
 */
template <unsigned int BlockSize = DefaultBlockSize, class Query, class In, class Out>
EBGEOMETRY_HOST
inline void
transform(const Query& a_query, PODSpan<In> a_in, PODSpan<Out> a_out);

/**
 * @brief Run a query on every input of a pool-resident array, writing the results into another array
 * in the same pool: out[i] = a_query(in[i]), with no allocation and no copies.
 * @details The span overload, with the memory checked first. A PODVector is an offset into a pool, so
 * this overload takes the pool both arrays live in and binds them against its base. It aborts with a
 * message, in every build, unless the pool is frozen (as rebasedView() requires of a device-accessible
 * pool) and its resource is device-accessible: a mirror into memoryResource(), or a pool built in
 * managed memory. A pool in hostMemoryResource() is therefore rejected in a host build too, where
 * memoryResource() is the emulated device resource. Freezing forbids reservations, not writes, so the
 * results can be written into a frozen pool. Arrays in two different pools, or not in a pool at all,
 * go through the span overload.
 * @tparam BlockSize Threads per block; see parallelFor().
 * @tparam Query     Trivially copyable functor with an EBGEOMETRY_HOST_DEVICE call operator taking a
 * const In&.
 * @tparam In        Input type.
 * @tparam Out       Result type: the query's result type.
 * @param[in] a_query The query.
 * @param[in] a_pool  The frozen, device-accessible pool holding both arrays.
 * @param[in] a_in    The inputs.
 * @param[in] a_out   The results' array, as many elements as a_in; its elements are written.
 */
template <unsigned int BlockSize = DefaultBlockSize, class Query, class In, class Out>
EBGEOMETRY_HOST
inline void
transform(const Query& a_query, const Pool& a_pool, const PODVector<In>& a_in, const PODVector<Out>& a_out);

/**
 * @brief Run a query on every input, from host memory to host memory: a_out[i] = a_query(a_in[i]).
 * @details A convenience over the span overload. In a CUDA or HIP build it allocates device memory
 * for the inputs and the results, copies the inputs in, runs the query in a kernel, copies the results
 * into a_out and frees, so timing it includes two allocations and two copies; time the span or pool
 * overload to measure the query itself. In a host build the query runs in a loop directly on the
 * vectors. a_out is not resized: it must already hold as many elements as a_in, which aborts with a
 * message otherwise, and can be reused across calls. Neither the input nor the result may be bool
 * (std::vector<bool> has no contiguous storage to copy), which is checked at compile time in both
 * builds.
 * @tparam BlockSize Threads per block; see parallelFor().
 * @tparam Query     Trivially copyable functor with an EBGEOMETRY_HOST_DEVICE call operator taking a
 * const In&.
 * @tparam In        Trivially copyable input type.
 * @tparam Out       Result type: the query's result type.
 * @param[in]  a_query The query.
 * @param[in]  a_in    The inputs.
 * @param[out] a_out   The results, in input order; as many elements as a_in.
 */
template <unsigned int BlockSize = DefaultBlockSize, class Query, class In, class Out>
EBGEOMETRY_HOST
inline void
transform(const Query& a_query, const std::vector<In>& a_in, std::vector<Out>& a_out);

} // namespace GPU
} // namespace EBGeometry

#include "EBGeometry_GPUTransformImplem.hpp"

#endif
