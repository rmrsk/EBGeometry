// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file    EBGeometry_GPUAlgorithms.hpp
 * @brief   Building blocks for builds that run with GPU::parallelFor(): a bounding-box reduction, a bitonic
 *          sort and an exclusive scan.
 * @details Each runs one or more GPU::parallelFor() launches, so it runs on the device in a CUDA or HIP
 *          translation unit and as host loops otherwise, and returns once the work is done. The arrays
 *          they read and write must be memory the launch can reach: device-accessible in a CUDA or HIP
 *          build (device memory will do), and host memory otherwise. They use no atomics, so their results
 *          do not depend on thread scheduling.
 * @author  Robert Marskar
 */

#ifndef EBGEOMETRY_GPUALGORITHMS_HPP
#define EBGEOMETRY_GPUALGORITHMS_HPP

// Std includes
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <type_traits>

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_GPURuntime.hpp"
#include "EBGeometry_GPUTransform.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

namespace GPU {

/**
 * @brief Values per chunk in exclusiveScan()'s passes.
 */
inline constexpr uint32_t ScanChunk = 256;

/**
 * @brief Scratch elements exclusiveScan() needs for @p a_count values: one per chunk, and one for the total.
 * @param[in] a_count Number of values.
 * @return The scratch size, in uint32_t elements.
 */
[[nodiscard]] EBGEOMETRY_HOST
inline uint32_t
exclusiveScanScratchSize(const uint32_t a_count) noexcept
{
  return (a_count + ScanChunk - 1) / ScanChunk + 1U;
}

/**
 * @brief Points per chunk in boundingBox()'s first pass.
 */
inline constexpr uint32_t BoundingBoxChunk = 256;

/**
 * @brief Scratch elements boundingBox() needs for @p a_count points: two corners per chunk.
 * @param[in] a_count Number of points.
 * @return The scratch size, in Vec3T elements.
 */
[[nodiscard]] EBGEOMETRY_HOST
inline uint32_t
boundingBoxScratchSize(const uint32_t a_count) noexcept
{
  return 2U * Math::max(1U, (a_count + BoundingBoxChunk - 1) / BoundingBoxChunk);
}

/**
 * @brief The functors behind boundingBox() and bitonicSort().
 * @details Internal; not part of the public API.
 */
namespace AlgorithmsDetail {

/**
 * @brief boundingBox()'s source for an array of points.
 * @tparam T Precision.
 */
template <class T>
struct FromPointer
{
  /// @brief The points.
  const Vec3T<T>* m_points;

  /**
   * @brief Point @p a_i.
   * @param[in] a_i Index.
   * @return The point.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  Vec3T<T>
  operator()(const uint32_t a_i) const noexcept
  {
    return m_points[a_i];
  }
};

/**
 * @brief boundingBox(), pass 1: the box of each chunk of points.
 * @tparam T      Precision.
 * @tparam Source Where the points come from: a trivially copyable functor returning point i.
 */
template <class T, class Source>
struct ChunkBox
{
  /// @brief The points.
  Source m_points;

  /// @brief Chunk corners: lower corner of chunk c at 2c, upper at 2c + 1.
  Vec3T<T>* m_corners;

  /// @brief Number of points.
  uint32_t m_count;

  /**
   * @brief Box of chunk @p a_c.
   * @param[in] a_c Chunk.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_c) const noexcept
  {
    const uint32_t begin = static_cast<uint32_t>(a_c) * BoundingBoxChunk;
    const uint32_t end   = Math::min(m_count, begin + BoundingBoxChunk);

    Vec3T<T> lo = +Vec3T<T>::max();
    Vec3T<T> hi = -Vec3T<T>::max();

    bool finite = true;

    for (uint32_t i = begin; i < end; i++) {
      const Vec3T<T> p = m_points(i);

      finite = finite && std::isfinite(p[0]) && std::isfinite(p[1]) && std::isfinite(p[2]);

      lo = min(lo, p);
      hi = max(hi, p);
    }

    // A chunk with a non-finite point marks its box with a lower corner of -infinity, which the merge keeps.
    if (!finite) {
      lo[0] = -Math::Limits<T>::infinity();
    }

    m_corners[2 * a_c]     = lo;
    m_corners[2 * a_c + 1] = hi;
  }
};

/**
 * @brief boundingBox(), pass 2: one thread merges the chunk boxes into chunk 0's. One thread is enough at the sizes
 * the builds see (a few thousand chunks); a cloud near the 2^31-point limit would want a tree of merges.
 * @tparam T Precision.
 */
template <class T>
struct MergeChunks
{
  /// @brief Chunk corners, as ChunkBox wrote them.
  Vec3T<T>* m_corners;

  /// @brief Number of chunks.
  uint32_t m_chunks;

  /**
   * @brief Merge every chunk's box.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(std::size_t /*a_thread*/) const noexcept
  {
    Vec3T<T> lo = m_corners[0];
    Vec3T<T> hi = m_corners[1];

    for (uint32_t c = 1; c < m_chunks; c++) {
      lo = min(lo, m_corners[2 * c]);
      hi = max(hi, m_corners[2 * c + 1]);
    }

    m_corners[0] = lo;
    m_corners[1] = hi;
  }
};

/**
 * @brief bitonicSort(): one compare-exchange pass, one thread per pair.
 * @tparam Key  Key type.
 * @tparam Less Strict order on keys.
 */
template <class Key, class Less>
struct BitonicPass
{
  /// @brief The keys.
  Key* m_keys;

  /// @brief The order.
  Less m_less;

  /// @brief Size of the bitonic sequences being merged.
  uint32_t m_k;

  /// @brief Distance between the two keys of a pair.
  uint32_t m_j;

  /**
   * @brief Pair @p a_t.
   * @param[in] a_t Pair index, below half the key count.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_t) const noexcept
  {
    const auto     t = static_cast<uint32_t>(a_t);
    const uint32_t i = (t / m_j) * 2 * m_j + t % m_j;
    const uint32_t l = i + m_j;

    Key& a = m_keys[i];
    Key& b = m_keys[l];

    const bool ascending = (i & m_k) == 0;

    if (ascending ? m_less(b, a) : m_less(a, b)) {
      const Key swap = a;

      a = b;
      b = swap;
    }
  }
};

/**
 * @brief exclusiveScan(), pass 1: the sum of each chunk of values.
 */
struct ChunkSums
{
  /// @brief The values.
  const uint32_t* m_values;

  /// @brief Chunk sums, one per chunk.
  uint32_t* m_sums;

  /// @brief Number of values.
  uint32_t m_count;

  /**
   * @brief Sum of chunk @p a_c.
   * @param[in] a_c Chunk.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_c) const noexcept
  {
    const uint32_t begin = static_cast<uint32_t>(a_c) * ScanChunk;
    const uint32_t end   = Math::min(m_count, begin + ScanChunk);

    uint32_t sum = 0;

    for (uint32_t i = begin; i < end; i++) {
      sum += m_values[i];
    }

    m_sums[a_c] = sum;
  }
};

/**
 * @brief exclusiveScan(), pass 2: one thread turns the chunk sums into each chunk's offset, and the total after them.
 */
struct ScanSums
{
  /// @brief Chunk sums in, chunk offsets out; one more entry receives the total.
  uint32_t* m_sums;

  /// @brief Number of chunks.
  uint32_t m_chunks;

  /**
   * @brief Scan the chunk sums.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(std::size_t /*a_thread*/) const noexcept
  {
    uint32_t running = 0;

    for (uint32_t c = 0; c < m_chunks; c++) {
      const uint32_t sum = m_sums[c];

      m_sums[c] = running;
      running += sum;
    }

    m_sums[m_chunks] = running;
  }
};

/**
 * @brief exclusiveScan(), pass 3: each chunk's values replaced by their exclusive prefix sums.
 */
struct ScanChunks
{
  /// @brief The values, in place.
  uint32_t* m_values;

  /// @brief Each chunk's offset.
  const uint32_t* m_offsets;

  /// @brief Number of values.
  uint32_t m_count;

  /**
   * @brief Scan chunk @p a_c.
   * @param[in] a_c Chunk.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_c) const noexcept
  {
    const uint32_t begin = static_cast<uint32_t>(a_c) * ScanChunk;
    const uint32_t end   = Math::min(m_count, begin + ScanChunk);

    uint32_t running = m_offsets[a_c];

    for (uint32_t i = begin; i < end; i++) {
      const uint32_t value = m_values[i];

      m_values[i] = running;
      running += value;
    }
  }
};

} // namespace AlgorithmsDetail

/**
 * @brief The order operator< gives, as a trivially copyable functor callable on a device.
 */
struct Ascending
{
  /**
   * @brief Whether @p a_lhs sorts before @p a_rhs.
   * @tparam Key Key type, with operator<.
   * @param[in] a_lhs Left key.
   * @param[in] a_rhs Right key.
   * @return a_lhs < a_rhs.
   */
  template <class Key>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  bool
  operator()(const Key& a_lhs, const Key& a_rhs) const noexcept
  {
    return a_lhs < a_rhs;
  }
};

/**
 * @brief The smallest box holding every point, from a source of points: one box per chunk of BoundingBoxChunk
 * points, then one thread merging them.
 * @details The minimum and maximum are exact, so the box is the one a host loop over the points finds. The source
 * is any trivially copyable functor whose const call operator returns point i, callable where the launches run.
 * The result is copied back from @p a_scratch to the host (from device memory in a CUDA or HIP build). If a point
 * has a non-finite coordinate, a corner of the box is not finite: check it, since the launches check nothing else.
 * @tparam T      Precision.
 * @tparam Source Source type.
 * @param[in]  a_points  The points.
 * @param[in]  a_count   Number of points.
 * @param[out] a_scratch boundingBoxScratchSize(a_count) elements of scratch, which the launches can write.
 * @return The box; the empty box for no points.
 */
template <class T, class Source>
[[nodiscard]] EBGEOMETRY_HOST
inline BoundingVolumes::AABBT<T>
boundingBoxOf(const Source& a_points, const uint32_t a_count, Vec3T<T>* a_scratch)
{
  static_assert(std::is_trivially_copyable_v<Source>, "GPU::boundingBoxOf: the source must be trivially copyable");

  if (a_count == 0) {
    return BoundingVolumes::AABBT<T>();
  }

  const uint32_t chunks = boundingBoxScratchSize(a_count) / 2;

  parallelFor(chunks, AlgorithmsDetail::ChunkBox<T, Source>{a_points, a_scratch, a_count});
  parallelFor(1, AlgorithmsDetail::MergeChunks<T>{a_scratch, chunks});

  Vec3T<T> corners[2];

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  EBGEOMETRY_GPU_CHECK(GPU::memcpy(corners, a_scratch, sizeof(corners), GPU::MemcpyDeviceToHost));
#else
  corners[0] = a_scratch[0];
  corners[1] = a_scratch[1];
#endif

  return BoundingVolumes::AABBT<T>(corners[0], corners[1]);
}

/**
 * @brief The smallest box holding an array of points; see boundingBoxOf().
 * @tparam T Precision.
 * @param[in]  a_points  The points, which the launches can read.
 * @param[in]  a_count   Number of points.
 * @param[out] a_scratch boundingBoxScratchSize(a_count) elements of scratch, which the launches can write.
 * @return The box; the empty box for no points.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST
inline BoundingVolumes::AABBT<T>
boundingBox(const Vec3T<T>* a_points, const uint32_t a_count, Vec3T<T>* a_scratch)
{
  return boundingBoxOf(AlgorithmsDetail::FromPointer<T>{a_points}, a_count, a_scratch);
}

/**
 * @brief Sort @p a_count keys in place with a bitonic network: log2(n) (log2(n) + 1) / 2 passes of
 * n / 2 compare-exchanges each.
 * @details Not stable, so for a reproducible result @p a_less must order every pair of distinct keys.
 * Pad an arbitrary count up to a power of two with keys that sort last. Aborts, in every build, if
 * @p a_count is not a power of two.
 * @tparam Key  Trivially copyable key type.
 * @tparam Less Trivially copyable strict order with an EBGEOMETRY_HOST_DEVICE call operator.
 * @param[in,out] a_keys  The keys.
 * @param[in]     a_count Number of keys: a power of two, or zero.
 * @param[in]     a_less  The order.
 */
template <class Key, class Less = Ascending>
EBGEOMETRY_HOST
inline void
bitonicSort(Key* a_keys, const uint32_t a_count, const Less& a_less = Less())
{
  static_assert(std::is_trivially_copyable_v<Key>, "GPU::bitonicSort: the key type must be trivially copyable");

  EBGEOMETRY_REQUIRE(
    (a_count & (a_count - 1)) == 0, "GPU::bitonicSort: the key count must be a power of two (%u)", unsigned(a_count));

  for (uint32_t k = 2; k <= a_count && k != 0; k *= 2) {
    for (uint32_t j = k / 2; j > 0; j /= 2) {
      parallelFor(a_count / 2, AlgorithmsDetail::BitonicPass<Key, Less>{a_keys, a_less, k, j});
    }
  }
}

/**
 * @brief Replace @p a_count values by their exclusive prefix sums, in place, and return their total: one sum per
 * chunk of ScanChunk values, one thread scanning the sums, then each chunk scanned from its offset.
 * @details Integer sums in a fixed order, so the result does not depend on thread scheduling. The values and the
 * scratch must be memory the launches can reach; the total is copied back to the host (from device memory in a
 * CUDA or HIP build).
 * @param[in,out] a_values  The values.
 * @param[in]     a_count   Number of values.
 * @param[out]    a_scratch exclusiveScanScratchSize(a_count) elements of scratch, which the launches can write.
 * @return The sum of the values.
 */
[[nodiscard]] EBGEOMETRY_HOST
inline uint32_t
exclusiveScan(uint32_t* a_values, const uint32_t a_count, uint32_t* a_scratch)
{
  if (a_count == 0) {
    return 0;
  }

  const uint32_t chunks = exclusiveScanScratchSize(a_count) - 1U;

  parallelFor(chunks, AlgorithmsDetail::ChunkSums{a_values, a_scratch, a_count});
  parallelFor(1, AlgorithmsDetail::ScanSums{a_scratch, chunks});
  parallelFor(chunks, AlgorithmsDetail::ScanChunks{a_values, a_scratch, a_count});

  uint32_t total = 0;

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  EBGEOMETRY_GPU_CHECK(GPU::memcpy(&total, a_scratch + chunks, sizeof(total), GPU::MemcpyDeviceToHost));
#else
  total = a_scratch[chunks];
#endif

  return total;
}

} // namespace GPU

} // namespace EBGeometry

#endif
