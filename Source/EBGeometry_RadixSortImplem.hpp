// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_RadixSortImplem.hpp
 * @brief  Implementation of EBGeometry_RadixSort.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_RADIXSORTIMPLEM_HPP
#define EBGEOMETRY_RADIXSORTIMPLEM_HPP

// Std includes
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_RadixSort.hpp"

namespace EBGeometry {

namespace RadixSort {

EBGEOMETRY_HOST_DEVICE inline uint32_t
digit(uint64_t a_key, uint32_t a_pass) noexcept
{
  EBGEOMETRY_EXPECT(a_pass < NumPasses64);

  return static_cast<uint32_t>(a_key >> (RadixBits * a_pass)) & (RadixSize - 1);
}

EBGEOMETRY_HOST_DEVICE inline uint32_t
blockRangeBegin(uint32_t a_numItems, uint32_t a_numBlocks, uint32_t a_block) noexcept
{
  EBGEOMETRY_EXPECT(a_numBlocks > 0);
  EBGEOMETRY_EXPECT(a_block <= a_numBlocks);

  return static_cast<uint32_t>((static_cast<uint64_t>(a_numItems) * a_block) / a_numBlocks);
}

EBGEOMETRY_HOST_DEVICE inline void
blockHistogram(const uint64_t* a_keys,
               uint32_t        a_numItems,
               uint32_t        a_pass,
               uint32_t        a_numBlocks,
               uint32_t        a_block,
               uint32_t*       a_histogram) noexcept
{
  EBGEOMETRY_EXPECT(a_numBlocks > 0);
  EBGEOMETRY_EXPECT(a_block < a_numBlocks);

  uint32_t count[RadixSize] = {0};

  const uint32_t begin = blockRangeBegin(a_numItems, a_numBlocks, a_block);
  const uint32_t end   = blockRangeBegin(a_numItems, a_numBlocks, a_block + 1);

  for (uint32_t i = begin; i < end; i++) {
    count[digit(a_keys[i], a_pass)]++;
  }

  for (uint32_t d = 0; d < RadixSize; d++) {
    a_histogram[d * a_numBlocks + a_block] = count[d];
  }
}

EBGEOMETRY_HOST_DEVICE inline void
blockScatter(const uint64_t* a_keys,
             const uint32_t* a_values,
             uint32_t        a_numItems,
             uint32_t        a_pass,
             uint32_t        a_numBlocks,
             uint32_t        a_block,
             const uint32_t* a_scannedHistogram,
             uint64_t*       a_outKeys,
             uint32_t*       a_outValues) noexcept
{
  EBGEOMETRY_EXPECT(a_numBlocks > 0);
  EBGEOMETRY_EXPECT(a_block < a_numBlocks);

  uint32_t cursor[RadixSize];

  for (uint32_t d = 0; d < RadixSize; d++) {
    cursor[d] = a_scannedHistogram[d * a_numBlocks + a_block];
  }

  const uint32_t begin = blockRangeBegin(a_numItems, a_numBlocks, a_block);
  const uint32_t end   = blockRangeBegin(a_numItems, a_numBlocks, a_block + 1);

  for (uint32_t i = begin; i < end; i++) {
    const uint32_t d   = digit(a_keys[i], a_pass);
    const uint32_t dst = cursor[d]++;

    EBGEOMETRY_EXPECT(dst < a_numItems);

    a_outKeys[dst]   = a_keys[i];
    a_outValues[dst] = a_values[i];
  }
}

EBGEOMETRY_HOST_DEVICE inline uint32_t
blockExclusiveScan(uint32_t* a_data, uint32_t a_begin, uint32_t a_end) noexcept
{
  EBGEOMETRY_EXPECT(a_begin <= a_end);

  uint32_t sum = 0;

  for (uint32_t i = a_begin; i < a_end; i++) {
    const uint32_t value = a_data[i];

    a_data[i] = sum;
    sum += value;
  }

  return sum;
}

EBGEOMETRY_HOST_DEVICE inline void
blockAddOffset(uint32_t* a_data, uint32_t a_begin, uint32_t a_end, uint32_t a_offset) noexcept
{
  EBGEOMETRY_EXPECT(a_begin <= a_end);

  for (uint32_t i = a_begin; i < a_end; i++) {
    a_data[i] += a_offset;
  }
}

EBGEOMETRY_HOST inline void
hostExclusiveScan(uint32_t* a_data, uint32_t a_numEntries, uint32_t a_numBlocks) noexcept
{
  EBGEOMETRY_EXPECT(a_numBlocks > 0);

  std::vector<uint32_t> blockSums(a_numBlocks);

  for (uint32_t b = 0; b < a_numBlocks; b++) {
    blockSums[b] = blockExclusiveScan(
      a_data, blockRangeBegin(a_numEntries, a_numBlocks, b), blockRangeBegin(a_numEntries, a_numBlocks, b + 1));
  }

  (void)blockExclusiveScan(blockSums.data(), 0, a_numBlocks);

  for (uint32_t b = 0; b < a_numBlocks; b++) {
    blockAddOffset(a_data,
                   blockRangeBegin(a_numEntries, a_numBlocks, b),
                   blockRangeBegin(a_numEntries, a_numBlocks, b + 1),
                   blockSums[b]);
  }
}

EBGEOMETRY_HOST inline void
hostPass(const uint64_t* a_keys,
         const uint32_t* a_values,
         uint32_t        a_numItems,
         uint32_t        a_pass,
         uint32_t        a_numBlocks,
         uint64_t*       a_outKeys,
         uint32_t*       a_outValues) noexcept
{
  EBGEOMETRY_EXPECT(a_numBlocks > 0);

  std::vector<uint32_t> histogram(static_cast<size_t>(RadixSize) * a_numBlocks, 0);

  for (uint32_t b = 0; b < a_numBlocks; b++) {
    blockHistogram(a_keys, a_numItems, a_pass, a_numBlocks, b, histogram.data());
  }

  hostExclusiveScan(histogram.data(), RadixSize * a_numBlocks, a_numBlocks);

  for (uint32_t b = 0; b < a_numBlocks; b++) {
    blockScatter(a_keys, a_values, a_numItems, a_pass, a_numBlocks, b, histogram.data(), a_outKeys, a_outValues);
  }
}

EBGEOMETRY_HOST inline void
hostStableSort(std::vector<uint64_t>& a_keys, std::vector<uint32_t>& a_values) noexcept
{
  EBGEOMETRY_EXPECT(a_keys.size() == a_values.size());
  EBGEOMETRY_EXPECT(a_keys.size() <= size_t(std::numeric_limits<uint32_t>::max()));

  const uint32_t numItems = static_cast<uint32_t>(a_keys.size());

  // A modest block count exercises the block-decomposed path on typical inputs while degrading
  // gracefully to one block for tiny ones; the sorted result is independent of the choice.
  const uint32_t numBlocks = (numItems < 4096) ? 1 : (numItems < 262144 ? 16 : 64);

  std::vector<uint64_t> scratchKeys(numItems);
  std::vector<uint32_t> scratchValues(numItems);

  uint64_t* keysA   = a_keys.data();
  uint32_t* valuesA = a_values.data();
  uint64_t* keysB   = scratchKeys.data();
  uint32_t* valuesB = scratchValues.data();

  for (uint32_t pass = 0; pass < NumPasses64; pass++) {
    hostPass(keysA, valuesA, numItems, pass, numBlocks, keysB, valuesB);

    std::swap(keysA, keysB);
    std::swap(valuesA, valuesB);
  }

  // NumPasses64 is even, so the final swap leaves the sorted data back in a_keys/a_values.
  EBGEOMETRY_EXPECT(keysA == a_keys.data());
}

} // namespace RadixSort

} // namespace EBGeometry

#endif
