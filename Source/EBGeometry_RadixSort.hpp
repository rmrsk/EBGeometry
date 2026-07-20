// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_RadixSort.hpp
 * @brief  Declaration of a stable LSD radix sort and an exclusive scan, decomposed into pure,
 *         host/device-shareable per-block steps.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_RADIXSORT_HPP
#define EBGEOMETRY_RADIXSORT_HPP

// Std includes
#include <cstdint>
#include <vector>

// Our includes
#include "EBGeometry_GPU.hpp"

namespace EBGeometry {

/**
 * @brief Namespace for a stable least-significant-digit (LSD) radix sort of 64-bit keys with
 * 32-bit value payloads, plus the exclusive scan it is built on.
 * @details This is a device-executable sort primitive: the device-side equivalent of the
 * @c std::sort -by-SFC-code step in PackedBVH's direct space-filling-curve constructor (which a
 * device build cannot call), sorting e.g. Morton codes (SFC::Morton::encode()) with cloud/
 * primitive indices as payloads. It is decomposed into pure per-block phase functions (digit(),
 * blockRangeBegin(), blockHistogram(), blockScatter(), blockExclusiveScan(), blockAddOffset())
 * that are all @c EBGEOMETRY_HOST_DEVICE, so a device (GPU) implementation and the host
 * references here execute the very same step arithmetic -- the host functions (hostPass(),
 * hostStableSort(), hostExclusiveScan()) simply run the phases as sequential loops over the
 * blocks. One pass sorts one 8-bit digit; a full 64-bit sort is 8 passes, each stable, so the
 * final order equals @c std::stable_sort by key -- equal keys keep their input order, and for
 * distinct keys the order is identical to the host constructor's @c std::sort.
 *
 * The exclusive scan lives in this header because the radix sort's histogram phase is its first
 * and defining consumer -- the BVH node-emission stages next to BVH::PackedNode need no scan at
 * all.
 */
namespace RadixSort {

/**
 * @brief Number of key bits sorted per pass.
 */
inline constexpr uint32_t RadixBits = 8;

/**
 * @brief Number of distinct digit values per pass (2^RadixBits).
 */
inline constexpr uint32_t RadixSize = uint32_t(1) << RadixBits;

/**
 * @brief Number of passes required to fully sort a 64-bit key.
 */
inline constexpr uint32_t NumPasses64 = 64 / RadixBits;

/**
 * @brief Extract the digit a pass sorts by.
 * @param[in] a_key  64-bit sort key.
 * @param[in] a_pass Pass number (0 = least significant digit, ..., NumPasses64 - 1).
 * @return The 8-bit digit (a_key >> (RadixBits * a_pass)) & (RadixSize - 1).
 */
[[nodiscard]] EBGEOMETRY_HOST_DEVICE inline uint32_t
digit(uint64_t a_key, uint32_t a_pass) noexcept;

/**
 * @brief First item index of a block's contiguous slab in the block decomposition of an array.
 * @details Block b owns items [blockRangeBegin(n, B, b), blockRangeBegin(n, B, b+1)); the slabs
 * are contiguous, ordered, and exactly cover [0, n) for any 1 <= B, including B > n (empty slabs).
 * @param[in] a_numItems  Total number of items.
 * @param[in] a_numBlocks Number of blocks. Must be > 0.
 * @param[in] a_block     Block index, in [0, a_numBlocks].
 * @return First item index owned by a_block.
 */
[[nodiscard]] EBGEOMETRY_HOST_DEVICE inline uint32_t
blockRangeBegin(uint32_t a_numItems, uint32_t a_numBlocks, uint32_t a_block) noexcept;

/**
 * @brief Histogram phase for one block of one pass: count the block's keys per digit.
 * @details Writes the block's count of every digit d to a_histogram[d * a_numBlocks + a_block]
 * (digit-major layout, so after an exclusive scan of the whole table the scatter base of a
 * (digit, block) pair is a single lookup). Only this block's a_numBlocks-strided entries are
 * written, so all blocks may run concurrently on one shared table.
 * @param[in]  a_keys      Keys to histogram.
 * @param[in]  a_numItems  Total number of keys.
 * @param[in]  a_pass      Pass number (selects the digit).
 * @param[in]  a_numBlocks Number of blocks. Must be > 0.
 * @param[in]  a_block     This block's index, in [0, a_numBlocks).
 * @param[out] a_histogram Digit-major histogram table of size RadixSize * a_numBlocks.
 */
EBGEOMETRY_HOST_DEVICE inline void
blockHistogram(const uint64_t* a_keys,
               uint32_t        a_numItems,
               uint32_t        a_pass,
               uint32_t        a_numBlocks,
               uint32_t        a_block,
               uint32_t*       a_histogram) noexcept;

/**
 * @brief Scatter phase for one block of one pass: stably write the block's pairs to their sorted
 * positions.
 * @details Walks the block's slab in input order (the source of per-pass stability: within a
 * (digit, block) pair, items are emitted in encounter order) and writes each (key, value) pair to
 * a_outKeys/a_outValues at the next free slot of its digit, starting from the exclusively-scanned
 * histogram base a_scannedHistogram[digit * a_numBlocks + a_block]. Distinct blocks write disjoint
 * output ranges, so all blocks may run concurrently.
 * @param[in]  a_keys             Input keys.
 * @param[in]  a_values           Input value payloads (parallel to a_keys).
 * @param[in]  a_numItems         Total number of pairs.
 * @param[in]  a_pass             Pass number (selects the digit).
 * @param[in]  a_numBlocks        Number of blocks. Must be > 0.
 * @param[in]  a_block            This block's index, in [0, a_numBlocks).
 * @param[in]  a_scannedHistogram Exclusive scan of the digit-major histogram table (size
 *                                RadixSize * a_numBlocks) built by blockHistogram().
 * @param[out] a_outKeys          Output keys (size a_numItems; distinct from a_keys).
 * @param[out] a_outValues        Output values (size a_numItems; distinct from a_values).
 */
EBGEOMETRY_HOST_DEVICE inline void
blockScatter(const uint64_t* a_keys,
             const uint32_t* a_values,
             uint32_t        a_numItems,
             uint32_t        a_pass,
             uint32_t        a_numBlocks,
             uint32_t        a_block,
             const uint32_t* a_scannedHistogram,
             uint64_t*       a_outKeys,
             uint32_t*       a_outValues) noexcept;

/**
 * @brief Exclusive-scan step for one block: in-place exclusive prefix sum of the block's slab.
 * @details Replaces a_data[a_begin, a_end) by its exclusive prefix sums (a_data[a_begin] becomes
 * 0) and returns the slab's total, i.e. the block sum a multi-block scan then scans and adds back
 * via blockAddOffset().
 * @param[in,out] a_data  Array being scanned.
 * @param[in]     a_begin First index of the block's slab.
 * @param[in]     a_end   One past the last index of the block's slab.
 * @return Sum of the slab's original entries.
 */
EBGEOMETRY_HOST_DEVICE inline uint32_t
blockExclusiveScan(uint32_t* a_data, uint32_t a_begin, uint32_t a_end) noexcept;

/**
 * @brief Exclusive-scan add-back step for one block: add a block's scanned base offset to its slab.
 * @param[in,out] a_data   Array being scanned.
 * @param[in]     a_begin  First index of the block's slab.
 * @param[in]     a_end    One past the last index of the block's slab.
 * @param[in]     a_offset Exclusive prefix sum of the preceding blocks' totals.
 */
EBGEOMETRY_HOST_DEVICE inline void
blockAddOffset(uint32_t* a_data, uint32_t a_begin, uint32_t a_end, uint32_t a_offset) noexcept;

/**
 * @brief Host reference: in-place exclusive prefix sum (like std::exclusive_scan with initial
 * value 0), block-decomposed.
 * @details Runs blockExclusiveScan() on every block, exclusively scans the block totals, and adds
 * them back with blockAddOffset() -- the same three phases a device implementation launches, as
 * sequential loops. The result is independent of a_numBlocks (exposed so tests can force the
 * multi-block path).
 * @param[in,out] a_data       Array to scan.
 * @param[in]     a_numEntries Number of entries in a_data.
 * @param[in]     a_numBlocks  Number of blocks to decompose into. Must be > 0.
 */
EBGEOMETRY_HOST inline void
hostExclusiveScan(uint32_t* a_data, uint32_t a_numEntries, uint32_t a_numBlocks = 1) noexcept;

/**
 * @brief Host reference for one radix-sort pass: histogram, scan, scatter.
 * @details Runs blockHistogram() on every block, hostExclusiveScan() over the digit-major table,
 * and blockScatter() on every block -- the exact phase sequence a device implementation launches
 * as kernels. The output is a stable ordering of the pairs by the pass's digit, independent of
 * a_numBlocks (exposed so tests can force the multi-block path).
 * @param[in]  a_keys      Input keys.
 * @param[in]  a_values    Input value payloads (parallel to a_keys).
 * @param[in]  a_numItems  Number of pairs.
 * @param[in]  a_pass      Pass number (selects the digit).
 * @param[in]  a_numBlocks Number of blocks to decompose into. Must be > 0.
 * @param[out] a_outKeys   Output keys (size a_numItems; distinct from a_keys).
 * @param[out] a_outValues Output values (size a_numItems; distinct from a_values).
 */
EBGEOMETRY_HOST inline void
hostPass(const uint64_t* a_keys,
         const uint32_t* a_values,
         uint32_t        a_numItems,
         uint32_t        a_pass,
         uint32_t        a_numBlocks,
         uint64_t*       a_outKeys,
         uint32_t*       a_outValues) noexcept;

/**
 * @brief Host reference: full stable LSD radix sort of (key, value) pairs, in place.
 * @details NumPasses64 hostPass() invocations ping-ponging between the input arrays and internal
 * scratch (the pass count is even, so the result lands back in the input arrays). Deliberately
 * built on the same phase functions a device implementation uses, not on std::stable_sort -- the
 * unit tests compare it against std::stable_sort to certify the phase math. Equal keys keep their
 * input order.
 * @param[in,out] a_keys   Keys to sort by, ascending on return.
 * @param[in,out] a_values Value payloads, permuted identically to a_keys.
 */
EBGEOMETRY_HOST inline void
hostStableSort(std::vector<uint64_t>& a_keys, std::vector<uint32_t>& a_values) noexcept;

} // namespace RadixSort

} // namespace EBGeometry

#include "EBGeometry_RadixSortImplem.hpp"

#endif
