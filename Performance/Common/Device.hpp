// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   Device.hpp
 * @brief  Measuring a query on the device, and the host/device pairs: the same query over the same
 * inputs on both sides, each side's time, and whether their answers agree.
 */

#ifndef EBGEOMETRY_PERF_DEVICE_HPP
#define EBGEOMETRY_PERF_DEVICE_HPP

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include <EBGeometry.hpp>

#include "Case.hpp"
#include "Environment.hpp"
#include "Schema.hpp"
#include "Statistics.hpp"

namespace Perf {

/**
 * @brief Time a query on the device over a set of inputs, the kernel alone, as the case's own time.
 * @details The structure's pool is frozen and mirrored into GPU::memoryResource() once; the inputs and
 * room for the results go into a pool of their own, mirrored the same way. a_makeQuery receives the
 * device copy of the structure's pool and returns the query (holding the structure's rebasedView()).
 * The timed call is GPU::transform() over pool-resident arrays, which allocates and copies nothing, so
 * the time is the kernel's. The results are copied back to the host, untimed.
 * @tparam Out       Result type.
 * @tparam In        Input type.
 * @tparam MakeQuery Callable taking const EBGeometry::Pool& and returning the query.
 * @param[in,out] a_case          The running case.
 * @param[in,out] a_structurePool The pool the queried structure lives in; frozen here.
 * @param[in]     a_in            The inputs (fewer than 2^32).
 * @param[in]     a_makeQuery     Makes the query against a pool.
 * @return The results, on the host.
 */
template <class Out, class In, class MakeQuery>
std::vector<Out>
measureOnDevice(Case& a_case, EBGeometry::Pool& a_structurePool, const std::vector<In>& a_in, MakeQuery&& a_makeQuery)
{
  using EBGeometry::PODVector;
  using EBGeometry::Pool;

  const auto n = static_cast<std::uint32_t>(a_in.size());

  Pool             ioPool(EBGeometry::hostMemoryResource());
  PODVector<In>    in;
  PODVector<Out>   out;
  std::vector<Out> zeros(n, Out{});

  in.reserveFrom(ioPool, n);
  out.reserveFrom(ioPool, n);
  in.assign(ioPool.base(), a_in.data(), n);
  out.assign(ioPool.base(), zeros.data(), n);
  ioPool.freeze();

  if (!a_structurePool.isFrozen()) {
    a_structurePool.freeze();
  }

  const Pool deviceStructure = Pool::mirror(a_structurePool, EBGeometry::GPU::memoryResource());
  const Pool deviceIO        = Pool::mirror(ioPool, EBGeometry::GPU::memoryResource());
  const auto query           = a_makeQuery(deviceStructure);

  a_case.measure([&] { EBGeometry::GPU::transform(query, deviceIO, in, out); }, n);

  const Pool       back = Pool::mirror(deviceIO, EBGeometry::hostMemoryResource());
  std::vector<Out> results(n);

  for (std::uint32_t i = 0; i < n; i++) {
    results[i] = out.at(back.base(), i);
  }

  return results;
}

/**
 * @brief A host/device pair: the same query over the same inputs on the host and on the device.
 * @details The inputs are divided into chunks of GPU::DefaultBlockSize consecutive inputs: on the
 * device each chunk is one block of threads, and on the host the chunks are shared among hostThreads()
 * OpenMP threads, so both sides divide the work the same way. The case's own time is the device's (the
 * kernel alone, as for measureOnDevice()); it also records the metrics
 *
 *   Key::HostMs          the host's median time
 *   Key::HostThreads     the threads the host side ran on
 *   Key::HostOverDevice  host time / device time: the device's speedup, on this machine
 *   Key::MaxDiff         the largest difference between a host and a device result
 *   Key::Mismatches      the number of inputs on which the two disagree
 *
 * so that a divergence between the sides, in speed or in answers, shows in one place.
 * @tparam Out       Result type.
 * @tparam In        Input type.
 * @tparam MakeQuery Callable taking const EBGeometry::Pool& and returning the query against that pool.
 * @tparam Agree     Callable (const Out& host, const Out& device) returning std::pair<double, bool>: the
 * size of the difference, and whether it is a disagreement (see Probe.hpp for the usual ones).
 * @param[in,out] a_case          The running case.
 * @param[in,out] a_structurePool The pool the queried structure lives in; frozen here.
 * @param[in]     a_in            The inputs.
 * @param[in]     a_makeQuery     Makes the query against a pool: the host pool, or the device copy.
 * @param[in]     a_agree         Compares a host and a device result.
 * @return The host results.
 */
template <class Out, class In, class MakeQuery, class Agree>
std::vector<Out>
measureHostAndDevice(Case&                  a_case,
                     EBGeometry::Pool&      a_structurePool,
                     const std::vector<In>& a_in,
                     MakeQuery&&            a_makeQuery,
                     Agree&&                a_agree)
{
  const std::vector<Out> device = measureOnDevice<Out>(a_case, a_structurePool, a_in, a_makeQuery);

  const auto       query   = a_makeQuery(a_structurePool);
  const auto       n       = a_in.size();
  const int        threads = hostThreads();
  std::vector<Out> host(n);

  // A chunk at a time on hostThreads() threads (one without OpenMP), handed out dynamically as a GPU
  // schedules its blocks, so that the GPU is compared with the whole CPU rather than one core.
  const std::vector<double> hostTimes = a_case.repeat([&] {
    constexpr std::size_t chunk  = EBGeometry::GPU::DefaultBlockSize;
    const auto            chunks = static_cast<std::int64_t>((n + chunk - 1) / chunk);

#ifdef _OPENMP
#pragma omp parallel for num_threads(threads) schedule(dynamic)
#endif
    for (std::int64_t k = 0; k < chunks; k++) {
      const std::size_t begin = static_cast<std::size_t>(k) * chunk;
      const std::size_t end   = std::min(begin + chunk, n);

      for (std::size_t i = begin; i < end; i++) {
        host[i] = query(a_in[i]);
      }
    }
  });

  double      maxDiff    = 0.0;
  std::size_t mismatches = 0;

  for (std::size_t i = 0; i < n; i++) {
    const auto [diff, mismatch] = a_agree(host[i], device[i]);

    maxDiff = std::max(maxDiff, diff);
    mismatches += mismatch ? 1 : 0;
  }

  const double hostMs   = median(hostTimes);
  const double deviceMs = a_case.summary().median;

  a_case.metric(Key::HostMs, hostMs);
  a_case.metric(Key::HostThreads, static_cast<double>(threads));
  a_case.metric(Key::HostOverDevice, deviceMs > 0.0 ? hostMs / deviceMs : 0.0);
  a_case.metric(Key::MaxDiff, maxDiff);
  a_case.metric(Key::Mismatches, static_cast<double>(mismatches));

  return host;
}

} // namespace Perf

#endif
