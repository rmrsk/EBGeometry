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
#include "EBGeometry_GPU.hpp"
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
 * Every SFC in this namespace (e.g. Morton, Nested, Hilbert) implements exactly two static member
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
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE inline static uint64_t
  encode(const Index& a_point) noexcept;

  /**
   * @brief Decode the 64-bit Morton code into an Index.
   * @param[in] a_code Morton code to decode. Must be a code actually produced by encode(), i.e. no
   * bits above position 3*ValidBits-1 may be set.
   * @return 3D grid index corresponding to a_code.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE inline static Index
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
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE inline static uint64_t
  encode(const Index& a_point) noexcept;

  /**
   * @brief Decode the 64-bit SFC code into an Index.
   * @param[in] a_code SFC code to decode. Must be a code actually produced by encode(), i.e. at
   * most (ValidSpan+1)^3 - 1.
   * @return 3D grid index corresponding to a_code.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE inline static Index
  decode(const uint64_t& a_code) noexcept;
};

/**
 * @brief Implementation of the 3D Hilbert space-filling curve.
 * @details Uses Skilling's algorithm (J. Skilling, "Programming the Hilbert curve", AIP Conf. Proc.
 * 707, 381 (2004)): encode() maps the 3D grid index into the Hilbert "transpose" representation and
 * bit-interleaves it into the linear Hilbert distance; decode() reverses both steps. Unlike Morton
 * (Z-order), the Hilbert curve has no long-range jumps -- consecutive codes always map to spatially
 * adjacent cells (Manhattan distance 1) -- so an ordering along it produces more compact, better
 * localized runs of primitives, which is why it is often preferred for building spatially coherent
 * leaves. Encodes/decodes the same [0, ValidSpan] per-axis range as Morton and Nested, using the
 * full 3*ValidBits = 63 bits of the code.
 */
struct Hilbert
{
  /**
   * @brief Encode the input point into the linear Hilbert distance.
   * @param[in] a_point 3D grid index to encode. Each component must be <= ValidSpan.
   * @return 64-bit Hilbert code (distance along the curve) for a_point.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE inline static uint64_t
  encode(const Index& a_point) noexcept;

  /**
   * @brief Decode the linear Hilbert distance back into an Index.
   * @param[in] a_code Hilbert code to decode. Must be a code actually produced by encode(), i.e. no
   * bits above position 3*ValidBits-1 may be set.
   * @return 3D grid index corresponding to a_code.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE inline static Index
  decode(const uint64_t& a_code) noexcept;
};

/**
 * @brief Compute the per-axis normalization divisor computeBins()/computeBin() use to grid points.
 * @details Hoisted out of computeBins() (bit-identical arithmetic, including the zero-axis clamp)
 * so that host and device code binning one point at a time produce exactly the codes the host bulk
 * path produces. The divisor is delta = (max - min) / ValidSpan per axis; an axis on which every
 * point coincides (a planar cloud or duplicate points) would give delta = 0, which is clamped to 1
 * (the numerator in computeBin() is also exactly zero there for every point, so any nonzero
 * divisor yields the same, correct bin index of 0), avoiding a divide-by-zero.
 * @tparam T Floating-point precision.
 * @param[in] a_minCoord Componentwise minimum over the points to be binned.
 * @param[in] a_maxCoord Componentwise maximum over the points to be binned.
 * @return Per-axis divisor for computeBin(), every component nonzero.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE inline Vec3T<T>
binningDelta(const Vec3T<T>& a_minCoord, const Vec3T<T>& a_maxCoord) noexcept;

/**
 * @brief Bin a single point into the space-filling curve's integer grid.
 * @details The per-point form of computeBins(): given the cloud minimum and the divisor returned
 * by binningDelta(), grids one point with arithmetic bit-identical to the bulk path (the divisor
 * is divided by, exactly as in computeBins() -- deliberately not a reciprocal multiplication,
 * which could differ in the last ulp). Each floored index is clamped into [0, ValidSpan], the
 * range encode() requires: the two floating-point divisions can round a max-boundary point's index
 * to ValidSpan + 1 (and a min-boundary point's to a tiny negative, which would wrap on the
 * unsigned cast); the clamp defends both.
 * @tparam T Floating-point precision.
 * @param[in] a_point    Point to bin.
 * @param[in] a_minCoord Componentwise minimum over the points being binned.
 * @param[in] a_delta    Per-axis divisor from binningDelta(a_minCoord, a_maxCoord).
 * @return The point's 3D grid index, each component in [0, ValidSpan].
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE inline Index
computeBin(const Vec3T<T>& a_point, const Vec3T<T>& a_minCoord, const Vec3T<T>& a_delta) noexcept;

/**
 * @brief Bin a set of points into the space-filling curve's integer grid, normalizing by their own
 * bounding range.
 * @details Converts real-valued points into SFC::Index values suitable for SFC::Morton::encode() or
 * SFC::Nested::encode(). The binning itself is curve-independent -- every curve grids the same way;
 * only the subsequent encode() differs -- so this takes no curve type (see order() for the
 * curve-parameterized ordering built on top of it). If every point coincides on some axis (a planar
 * cloud or duplicate points), that axis's normalization divisor would be zero; it is clamped to 1
 * (the numerator is also exactly zero there for every point, so any nonzero divisor yields the same,
 * correct bin index of 0), avoiding a divide-by-zero. Implemented as a min/max reduction plus
 * binningDelta() and one computeBin() per point -- those helpers are the per-point form of the same
 * arithmetic, usable from device code, and binning through them is bit-identical to this bulk path.
 * @tparam T Floating-point precision.
 * @param[in] a_points Points to bin (e.g. bounding-volume centroids, or a raw point cloud).
 * @return One SFC::Index per input point, in the same order.
 */
template <class T>
[[nodiscard]] inline std::vector<Index>
computeBins(const std::vector<Vec3T<T>>& a_points) noexcept;

/**
 * @brief Return the index permutation that orders a_points along a space-filling curve.
 * @details Bins the points (computeBins), encodes each cell with Curve::encode(), and returns the
 * indices sorted by ascending code -- so a_points[result[0]], a_points[result[1]], ... walk the
 * curve. This is the one-call form of the "bin, encode, sort" pattern; the points themselves are not
 * moved or copied. @p Curve is a pure template parameter (encode() is static, so no instance is
 * needed) and comes first so it can be named while @p T is still deduced from a_points -- e.g.
 * order<SFC::Nested>(points), or just order(points) for the Morton default.
 * @tparam Curve Space-filling curve type (SFC::Morton, SFC::Nested, ...). Defaults to SFC::Morton.
 * @tparam T     Floating-point precision (deduced from a_points).
 * @param[in] a_points Points to order.
 * @return Indices into a_points, sorted by ascending SFC code.
 */
template <class Curve = Morton, class T>
[[nodiscard]] inline std::vector<uint32_t>
order(const std::vector<Vec3T<T>>& a_points) noexcept;

} // namespace SFC

} // namespace EBGeometry

#include "EBGeometry_SFCImplem.hpp"

#endif
