// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   Environment.hpp
 * @brief  What a probe records about the build and the machine it runs on: the commit, the compiler,
 * the host SIMD level, the CPU, and the threads the host side of a host/device pair uses.
 * @details Plain C++17 (with OpenMP where the build has it) and no dependency on EBGeometry.
 */

#ifndef EBGEOMETRY_PERF_ENVIRONMENT_HPP
#define EBGEOMETRY_PERF_ENVIRONMENT_HPP

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <string>

#ifdef _OPENMP
#include <omp.h>
#endif

#if defined(__GLIBC__)
#include <malloc.h>
#endif

// The commit the program was built from: set by the build (Performance/CMakeLists.txt).
#ifndef EBGEOMETRY_PERF_GIT_SHA
#define EBGEOMETRY_PERF_GIT_SHA "unknown"
#endif

namespace Perf {

/**
 * @brief The commit the cases measure: the environment variable EBGEOMETRY_PERF_GIT_SHA if set (the
 * front door sets it on every run, so that an incrementally rebuilt probe reports the commit it was
 * rebuilt from), else the one the probe was configured with.
 * @return The commit.
 */
inline std::string
gitSha()
{
  const char* env = std::getenv("EBGEOMETRY_PERF_GIT_SHA");

  return env != nullptr && *env != '\0' ? env : EBGEOMETRY_PERF_GIT_SHA;
}

/**
 * @brief The threads the host side of a host/device pair runs on, in a build with OpenMP: as many as
 * OMP_NUM_THREADS says if it is set, else every hardware thread but one. Hyperthreads make the heavier
 * queries markedly faster than one thread per core, but a thread on every hardware thread leaves
 * nothing for the GPU runtime and the operating system, so the loop, which waits for its slowest
 * thread, keeps losing whole scheduler ticks to a preempted one; one spare hardware thread was fastest
 * in every case measured (see PerformanceTesting.rst).
 * @return The thread count (1 without OpenMP).
 */
inline int
hostThreads()
{
#ifdef _OPENMP
  const char* env = std::getenv("OMP_NUM_THREADS");

  if (env != nullptr && *env != '\0') {
    return omp_get_max_threads();
  }

  return std::max(1, omp_get_num_procs() - 1);
#else
  return 1;
#endif
}

/**
 * @brief The size from which glibc serves an allocation with fresh pages from the system (mmap) rather
 * than from memory the process has already touched: 128 KiB, glibc's own starting value.
 */
constexpr std::size_t MmapThreshold = 128 * 1024;

/**
 * @brief Fix how large allocations are served, for the whole run, so that every build is timed alike.
 * @details glibc serves an allocation above its mmap threshold with fresh pages, which fault on first
 * use, and one below it from memory the process has already touched; and it raises the threshold as the
 * program frees large blocks. A build case makes a fresh pool every repetition, so whether that pool
 * paid the page faults depended on its size and on what was freed before it: a pool a few percent
 * smaller or larger could flip a build between the two and change its time by 15% without any change to
 * the build itself. Fixing the threshold (which also stops glibc from moving it) gives every large
 * allocation fresh pages, so every repetition pays the page faults of a first build, as a program that
 * builds its geometry once does. Elsewhere than glibc this does nothing.
 * @return A description of the policy, for the run's build line (Key::Allocator).
 */
inline std::string
fixAllocatorPolicy()
{
#if defined(__GLIBC__)
  if (mallopt(M_MMAP_THRESHOLD, static_cast<int>(MmapThreshold)) == 1) {
    return "glibc, fresh pages from " + std::to_string(MmapThreshold / 1024) + " KiB";
  }

  return "glibc, default (mallopt failed)";
#else
  return "system default";
#endif
}

/**
 * @brief The allocation policy of this run: fixed by fixAllocatorPolicy() on the first call (Suite's
 * constructor makes it, before any case allocates), and the same description on every later one.
 * @return The description, for the run's build line.
 */
inline const std::string&
allocatorPolicy()
{
  static const std::string policy = fixAllocatorPolicy();

  return policy;
}

/**
 * @brief The CPU model, from /proc/cpuinfo where there is one.
 * @return The model name, or "unknown".
 */
inline std::string
cpuModel()
{
  std::ifstream in("/proc/cpuinfo");
  std::string   line;

  while (std::getline(in, line)) {
    if (line.rfind("model name", 0) == 0) {
      const auto colon = line.find(':');

      return colon == std::string::npos ? line : line.substr(colon + 2);
    }
  }

  return "unknown";
}

/**
 * @brief The host SIMD level this translation unit was compiled for.
 * @return One of avx512, avx2, avx, sse41, none.
 */
constexpr const char*
simdLevel() noexcept
{
#if defined(__AVX512F__)
  return "avx512";
#elif defined(__AVX2__)
  return "avx2";
#elif defined(__AVX__)
  return "avx";
#elif defined(__SSE4_1__)
  return "sse41";
#else
  return "none";
#endif
}

/**
 * @brief Whether the build defines EBGEOMETRY_HOST_TUNED_DEFAULTS, which changes TriangleSoA::DefaultWidth.
 * @return True if it does.
 */
constexpr bool
hostTunedDefaults() noexcept
{
#ifdef EBGEOMETRY_HOST_TUNED_DEFAULTS
  return true;
#else
  return false;
#endif
}

/**
 * @brief The compiler and its version.
 * @return A short description.
 */
inline std::string
compilerName()
{
#if defined(__clang__)
  return std::string("clang ") + __clang_version__;
#elif defined(__GNUC__)
  return std::string("gcc ") + __VERSION__;
#else
  return "unknown";
#endif
}

} // namespace Perf

#endif
