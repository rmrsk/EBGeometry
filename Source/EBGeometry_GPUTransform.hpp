// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_GPUTransform.hpp
 * @brief  GPU::transform(), which runs a query on many inputs: in a kernel in a CUDA or HIP build, and
 *         in a host loop otherwise.
 * @details A query is a trivially copyable functor whose call operator is EBGEOMETRY_HOST_DEVICE and
 * maps one input to one result. It holds what it evaluates by value: an analytic shape, or a
 * pool-resident object rebased onto memory the device can read (a mirror in device memory, or a
 * frozen managed pool; see the memory model in the user documentation). One source then serves both
 * builds: GPU::transform() launches a kernel when the translation unit is compiled as CUDA or HIP, and
 * runs the same functor in a host loop otherwise, so a plain C++ build needs no GPU toolkit.
 * GPU::enabled says which of the two a translation unit gets, and GPU::deviceAvailable() whether a
 * device is visible at run time.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_GPUTRANSFORM_HPP
#define EBGEOMETRY_GPUTRANSFORM_HPP

// Std includes
#include <cstddef>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_GPURuntime.hpp"

namespace EBGeometry {
namespace GPU {

/**
 * @brief Whether this translation unit is compiled as CUDA or HIP, so that transform() runs a kernel.
 */
#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
inline constexpr bool enabled = true;
#else
inline constexpr bool enabled = false;
#endif

/**
 * @brief Where transform() runs its queries in this translation unit: "HIP", "CUDA" or "host".
 */
#if defined(EBGEOMETRY_HIP)
inline constexpr const char* backendName = "HIP";
#elif defined(EBGEOMETRY_CUDA)
inline constexpr const char* backendName = "CUDA";
#else
inline constexpr const char* backendName = "host";
#endif

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
 * @brief The result type of a query on an input.
 * @tparam Query Query functor type.
 * @tparam In    Input type.
 */
template <class Query, class In>
using TransformResult = std::decay_t<std::invoke_result_t<const Query&, const In&>>;

/**
 * @brief Run a query on every input: result i is a_query(a_inputs[i]).
 * @details In a CUDA or HIP build, the inputs are copied to device memory, a kernel runs the query
 * once per input (one thread each, in blocks of 128) on the current device, and the results are
 * copied back. Every runtime call is checked with EBGEOMETRY_GPU_CHECK, which aborts with the failing
 * call on an error, and a visible device is required (see deviceAvailable()). The query reaches the
 * kernel by value, so anything pool-resident it holds must have been rebased onto memory the device
 * can read.
 *
 * In a host build the query is copied once, as a kernel launch would copy it, and run in a loop.
 * Either way the query and its result must be trivially copyable, which is checked at compile time in
 * both builds, so a query that could not reach a kernel fails to compile on the host too.
 * @tparam Query Trivially copyable functor with an EBGEOMETRY_HOST_DEVICE call operator taking a
 * const In&.
 * @tparam In    Trivially copyable input type.
 * @param[in] a_query  The query.
 * @param[in] a_inputs The inputs.
 * @return One result per input, in input order.
 */
template <class Query, class In>
[[nodiscard]] EBGEOMETRY_HOST
inline std::vector<TransformResult<Query, In>>
transform(const Query& a_query, const std::vector<In>& a_inputs);

} // namespace GPU
} // namespace EBGeometry

#include "EBGeometry_GPUTransformImplem.hpp"

#endif
