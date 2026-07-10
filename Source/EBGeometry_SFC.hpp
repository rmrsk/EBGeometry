// SPDX-FileCopyrightText: 2024 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_SFC.hpp
 * @brief  Declaration of various space-filling curves
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_SFC_HPP
#define EBGEOMETRY_SFC_HPP

// Std includes
#include <array>
#include <cstdint>
#include <vector>

// Our includes
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Namespace for space-filling curves (SFCs) used to linearly order 3D grid cells.
 * @details A space-filling curve maps a 3D grid index (Index, three unsigned integers each in
 * [0, ValidSpan]) to a single 64-bit code (Code) and back, such that cells that are close together
 * in space tend to also be close together in code order. This is used to build spatially coherent
 * orderings of primitives (e.g. TreeBVH::bottomUpSortAndPartition<S>(), which sorts primitives by
 * S::encode(...) of their binned cell index before partitioning into a tree).
 *
 * Every SFC in this namespace (e.g. Morton, Nested) implements exactly two static member
 * functions:
 * @code{.cpp}
 * [[nodiscard]] static Code  encode(const Index& a_point) noexcept;
 * [[nodiscard]] static Index decode(const Code& a_code) noexcept;
 * @endcode
 * decode(encode(p)) must reproduce p for every p with components in [0, ValidSpan]; encode() need
 * not be injective outside that range (out-of-range components are a precondition violation, not a
 * defined-but-lossy encoding). Any new SFC struct added to this namespace, or supplied by a caller
 * as the template argument to code that is templated on an SFC (such as
 * TreeBVH::bottomUpSortAndPartition<S>()), must implement both functions with these exact
 * signatures to be usable as a drop-in replacement.
 */
namespace SFC {

/**
 * @brief Alias for SFC code
 */
using Code = uint64_t;

/**
 * @brief Alias for 3D cell index
 */
using Index = std::array<unsigned int, 3>;

/**
 * @brief Maximum available bits per dimension. 21 is the largest value for which 3*ValidBits (63)
 * still fits in a 64-bit Morton code with one bit to spare.
 */
static constexpr unsigned int ValidBits = 21;

/**
 * @brief Maximum permitted span along any spatial coordinate.
 */
static constexpr Code ValidSpan = (static_cast<uint64_t>(1) << ValidBits) - 1;

/**
 * @brief Implementation of the Morton SFC
 */
struct Morton
{
  /**
   * @brief Encode an input point into a Morton index with a 64-bit representation.
   * @param[in] a_point 3D grid index to encode. Each component must be <= ValidSpan.
   * @return 64-bit Morton code for a_point.
   */
  [[nodiscard]] inline static uint64_t
  encode(const Index& a_point) noexcept;

  /**
   * @brief Decode the 64-bit Morton code into an Index.
   * @param[in] a_code Morton code to decode. Must be a code actually produced by encode(), i.e. no
   * bits above position 3*ValidBits-1 may be set.
   * @return 3D grid index corresponding to a_code.
   */
  [[nodiscard]] inline static Index
  decode(const uint64_t& a_code) noexcept;

protected:
  /**
   * @brief Mask for magic-bits encoding of 3D Morton code
   */
  static constexpr uint_fast64_t Mask_64[6]{
    0x1fffff, 0x1f00000000ffff, 0x1f0000ff0000ff, 0x100f00f00f00f00f, 0x10c30c30c30c30c3, 0x1249249249249249};
};

/**
 * @brief Implementation of a nested index SFC.
 * @details The SFC is encoded by the code = i + j * N + k * N * N in 3D, where i,j,k are the block indices.
 */
struct Nested
{
  /**
   * @brief Encode the input point into the SFC code.
   * @param[in] a_point 3D grid index to encode. Each component must be <= ValidSpan.
   * @return 64-bit nested SFC code for a_point.
   */
  [[nodiscard]] inline static uint64_t
  encode(const Index& a_point) noexcept;

  /**
   * @brief Decode the 64-bit SFC code into an Index.
   * @param[in] a_code SFC code to decode. Must be a code actually produced by encode(), i.e. at
   * most (ValidSpan+1)^3 - 1.
   * @return 3D grid index corresponding to a_code.
   */
  [[nodiscard]] inline static Index
  decode(const uint64_t& a_code) noexcept;
};

/**
 * @brief Bin a set of points into the space-filling curve's integer grid, normalizing by their own
 * bounding range.
 * @details Converts real-valued points into SFC::Index values suitable for SFC::Morton::encode() or
 * SFC::Nested::encode(). The binning itself is curve-independent -- every curve grids the same way;
 * only the subsequent encode() differs -- so this takes no curve type (see order() for the
 * curve-parameterized ordering built on top of it). If every point coincides on some axis (a planar
 * cloud or duplicate points), that axis's normalization divisor would be zero; it is clamped to 1
 * (the numerator is also exactly zero there for every point, so any nonzero divisor yields the same,
 * correct bin index of 0), avoiding a divide-by-zero.
 * @tparam T Floating-point precision.
 * @param[in] a_points Points to bin (e.g. bounding-volume centroids, or a raw point cloud).
 * @return One SFC::Index per input point, in the same order.
 */
template <class T>
[[nodiscard]] inline std::vector<Index>
computeBins(const std::vector<Vec3T<T>>& a_points) noexcept;

/**
 * @brief Return the index permutation that orders a_points along a space-filling curve.
 * @details Bins the points (computeBins), encodes each cell with S::encode(), and returns the
 * indices sorted by ascending code -- so a_points[result[0]], a_points[result[1]], ... walk the
 * curve. This is the one-call form of the "bin, encode, sort" pattern; the points themselves are
 * not moved or copied. @p a_sfc is an unused tag whose only purpose is to let S be deduced, exactly
 * as for PackedBVH's SFC-build constructor -- pass e.g. SFC::Nested{} to select a different curve,
 * or omit it for the Morton default.
 * @tparam T Floating-point precision.
 * @tparam S Space-filling curve type (SFC::Morton, SFC::Nested, ...). Defaults to SFC::Morton.
 * @param[in] a_points Points to order.
 * @param[in] a_sfc    Unused tag value; see @p S.
 * @return Indices into a_points, sorted by ascending SFC code.
 */
template <class T, class S = Morton>
[[nodiscard]] inline std::vector<uint32_t>
order(const std::vector<Vec3T<T>>& a_points, S a_sfc = S{}) noexcept;

} // namespace SFC

} // namespace EBGeometry

#include "EBGeometry_SFCImplem.hpp"

#endif
