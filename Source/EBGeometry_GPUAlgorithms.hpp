// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file    EBGeometry_GPUAlgorithms.hpp
 * @brief   Building blocks for builds that run with GPU::parallelFor(): a bounding-box reduction, a
 *          bitonic sort and an exclusive prefix sum.
 * @details Each runs one or more GPU::parallelFor() launches, so it runs on the device in a CUDA or HIP
 *          translation unit and as host loops otherwise, and returns once the work is done. The arrays
 *          they read and write must be memory the launch can reach: device-accessible in a CUDA or HIP
 *          build, and host memory otherwise. They use no atomics, so their results do not depend on
 *          thread scheduling.
 * @author  Robert Marskar
 */

#ifndef EBGEOMETRY_GPUALGORITHMS_HPP
#define EBGEOMETRY_GPUALGORITHMS_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <type_traits>

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_GPUTransform.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

namespace GPU {

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
 * @brief boundingBox(), pass 1: the box of each chunk of points.
 * @tparam T Precision.
 */
template <class T>
struct ChunkBox
{
  /// @brief The points.
  const Vec3T<T>* m_points;

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

    for (uint32_t i = begin; i < end; i++) {
      lo = min(lo, m_points[i]);
      hi = max(hi, m_points[i]);
    }

    m_corners[2 * a_c]     = lo;
    m_corners[2 * a_c + 1] = hi;
  }
};

/**
 * @brief boundingBox(), pass 2: one thread merges the chunk boxes into chunk 0's.
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
 * @brief exclusiveScan(), up-sweep: a[i] += a[i - d] at the last element of every block of 2d.
 */
struct ScanUp
{
  /// @brief The values.
  uint32_t* m_data;

  /// @brief Half the block size.
  uint32_t m_d;

  /**
   * @brief Block @p a_t.
   * @param[in] a_t Block index.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_t) const noexcept
  {
    const uint32_t i = (static_cast<uint32_t>(a_t) + 1) * 2 * m_d - 1;

    m_data[i] += m_data[i - m_d];
  }
};

/**
 * @brief exclusiveScan(), between the sweeps: one thread stores the total and clears the last element.
 */
struct ScanClear
{
  /// @brief The values.
  uint32_t* m_data;

  /// @brief Where to store the total, or null.
  uint32_t* m_total;

  /// @brief Number of values.
  uint32_t m_count;

  /**
   * @brief Store the total and clear the last element.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(std::size_t /*a_thread*/) const noexcept
  {
    if (m_total != nullptr) {
      *m_total = m_data[m_count - 1];
    }

    m_data[m_count - 1] = 0;
  }
};

/**
 * @brief exclusiveScan(), down-sweep: swap and add within every block of 2d.
 */
struct ScanDown
{
  /// @brief The values.
  uint32_t* m_data;

  /// @brief Half the block size.
  uint32_t m_d;

  /**
   * @brief Block @p a_t.
   * @param[in] a_t Block index.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_t) const noexcept
  {
    const uint32_t i    = (static_cast<uint32_t>(a_t) + 1) * 2 * m_d - 1;
    const uint32_t left = m_data[i - m_d];

    m_data[i - m_d] = m_data[i];
    m_data[i] += left;
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
 * @brief The smallest box holding every point: one box per chunk of BoundingBoxChunk points, then one
 * thread merging them.
 * @details The minimum and maximum are exact, so the box is the one a host loop over the points finds.
 * The result is read back through @p a_scratch, which must therefore also be host-accessible (managed
 * memory in a CUDA or HIP build).
 * @tparam T Precision.
 * @param[in]  a_points  The points.
 * @param[in]  a_count   Number of points.
 * @param[out] a_scratch boundingBoxScratchSize(a_count) elements of scratch.
 * @return The box; the empty box for no points.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST
inline BoundingVolumes::AABBT<T>
boundingBox(const Vec3T<T>* a_points, const uint32_t a_count, Vec3T<T>* a_scratch)
{
  if (a_count == 0) {
    return BoundingVolumes::AABBT<T>();
  }

  const uint32_t chunks = boundingBoxScratchSize(a_count) / 2;

  parallelFor(chunks, AlgorithmsDetail::ChunkBox<T>{a_points, a_scratch, a_count});
  parallelFor(1, AlgorithmsDetail::MergeChunks<T>{a_scratch, chunks});

  return BoundingVolumes::AABBT<T>(a_scratch[0], a_scratch[1]);
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
 * @brief Replace @p a_count values by their exclusive prefix sums in place: a[i] becomes a[0] + ... +
 * a[i - 1], and a[0] becomes 0.
 * @details The work-efficient (Blelloch) scan: an up-sweep and a down-sweep of log2(n) passes each, every
 * pass one GPU::parallelFor() over the blocks it combines. The sums are taken in a fixed order, so the
 * result does not depend on thread scheduling. The values must sum to less than 2^32. Aborts, in every
 * build, if @p a_count is not a power of two; pad with zeros.
 * @param[in,out] a_data  The values.
 * @param[in]     a_count Number of values: a power of two, or zero.
 * @param[out]    a_total Where to store the sum of all the values, or null; memory the launch can write.
 */
EBGEOMETRY_HOST
inline void
exclusiveScan(uint32_t* a_data, const uint32_t a_count, uint32_t* a_total = nullptr)
{
  EBGEOMETRY_REQUIRE((a_count & (a_count - 1)) == 0,
                     "GPU::exclusiveScan: the value count must be a power of two (%u)",
                     unsigned(a_count));

  if (a_count == 0) {
    return;
  }

  for (uint32_t d = 1; d < a_count; d *= 2) {
    parallelFor(a_count / (2 * d), AlgorithmsDetail::ScanUp{a_data, d});
  }

  parallelFor(1, AlgorithmsDetail::ScanClear{a_data, a_total, a_count});

  for (uint32_t d = a_count / 2; d >= 1; d /= 2) {
    parallelFor(a_count / (2 * d), AlgorithmsDetail::ScanDown{a_data, d});
  }
}

} // namespace GPU

} // namespace EBGeometry

#endif
