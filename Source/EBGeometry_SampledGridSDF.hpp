// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

/**
 * @file   EBGeometry_SampledGridSDF.hpp
 * @brief  SampledGridSDF: a distance field sampled on a regular grid and interpolated trilinearly, held in a pool;
 * and a height field (terrain) as such a grid.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_SAMPLEDGRIDSDF_HPP
#define EBGEOMETRY_SAMPLEDGRIDSDF_HPP

// Std includes
#include <cmath>
#include <cstdint>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_DistanceQuality.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_PODVector.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Signed distance field sampled on a regular grid: values at the grid's nodes, interpolated trilinearly. From
 * CT or MRI, an image stack, another code's level set; or a height field, a terrain, with heightField().
 * @details The solid is where the interpolated field is negative, inside the grid's box. The value is a bound: the
 * field divided by its largest slope (found when it is built), which never changes faster than the distance to
 * where the field is zero, and outside the box the distance to the box combined with that bound at the box's
 * nearest point. A field of exact distances has slopes of about 1 and loses little; any other field (a density, a
 * level set far from its zero) is still a valid shape, with smaller values. The samples live in a pool, so a grid
 * is a handle (see PoolHandle): copying it copies no samples, and the pool must outlive it and every copy and
 * rebasedView(). Its signedDistance() runs on the host and on a device. By default the grid is empty and must not
 * be evaluated.
 * @tparam T Floating-point precision.
 */
template <class T>
class SampledGridSDF : public PoolHandle
{
  static_assert(std::is_floating_point_v<T>, "SampledGridSDF<T>: T must be a floating-point type");

public:
  /**
   * @brief How far signedDistance() can be trusted as a distance: never more than the Euclidean distance.
   */
  static constexpr DistanceQuality distanceQuality = DistanceQuality::Bound;

  /**
   * @brief The grid's samples as offsets into its pool, and its size and placement, with no location (see
   * PoolHandle).
   */
  struct Layout
  {
    /**
     * @brief The samples, x fastest, then y, then z, already divided by the field's largest slope.
     */
    PODVector<T> m_values;

    /**
     * @brief The position of node (0, 0, 0).
     */
    Vec3T<T> m_origin = Vec3T<T>::zeros();

    /**
     * @brief The spacing of the nodes along x, y and z.
     */
    Vec3T<T> m_spacing = Vec3T<T>::ones();

    /**
     * @brief The number of nodes along x, y and z.
     */
    uint32_t m_nodes[3] = {0U, 0U, 0U};
  };

  /**
   * @brief An empty grid, which must not be evaluated.
   */
  SampledGridSDF() noexcept = default;

  /**
   * @brief Rebuild a handle from a stored layout and the location of the pool that holds it.
   * @details How a tape, a BVH union or a fused operation evaluates a grid it stores as a layout. @p a_location must
   * belong to the pool the layout was reserved from, to a mirror of it, or to a pool holding its image.
   * @param[in] a_layout   The samples and the grid.
   * @param[in] a_location Where they live.
   */
  EBGEOMETRY_HOST_DEVICE
  SampledGridSDF(const Layout& a_layout, const PoolLocation& a_location) noexcept
    : PoolHandle(a_location), m_layout(a_layout)
  {}

  /**
   * @brief Build a grid from its samples, on the host.
   * @param[in,out] a_pool    Pool the samples are reserved from; must outlive this object.
   * @param[in]     a_origin  The position of node (0, 0, 0), finite.
   * @param[in]     a_spacing The spacing of the nodes along x, y and z, finite and positive.
   * @param[in]     a_nx      Number of nodes along x, at least 2.
   * @param[in]     a_ny      Number of nodes along y, at least 2.
   * @param[in]     a_nz      Number of nodes along z, at least 2.
   * @param[in]     a_values  The field at the nodes, x fastest, then y, then z: a_nx a_ny a_nz finite values,
   * negative inside, in units of length (a distance, ideally).
   */
  EBGEOMETRY_HOST
  SampledGridSDF(Pool&                 a_pool,
                 const Vec3T<T>&       a_origin,
                 const Vec3T<T>&       a_spacing,
                 const uint32_t        a_nx,
                 const uint32_t        a_ny,
                 const uint32_t        a_nz,
                 const std::vector<T>& a_values)
  {
    EBGEOMETRY_REQUIRE(std::isfinite(a_origin[0]) && std::isfinite(a_origin[1]) && std::isfinite(a_origin[2]),
                       "SampledGridSDF: the origin must be finite (%g, %g, %g)",
                       double(a_origin[0]),
                       double(a_origin[1]),
                       double(a_origin[2]));
    EBGEOMETRY_REQUIRE(std::isfinite(a_spacing[0]) && std::isfinite(a_spacing[1]) && std::isfinite(a_spacing[2]) &&
                         a_spacing[0] > T(0) && a_spacing[1] > T(0) && a_spacing[2] > T(0),
                       "SampledGridSDF: the spacing must be finite and positive (%g, %g, %g)",
                       double(a_spacing[0]),
                       double(a_spacing[1]),
                       double(a_spacing[2]));
    EBGEOMETRY_REQUIRE(a_nx >= 2U && a_ny >= 2U && a_nz >= 2U,
                       "SampledGridSDF: the grid must have at least 2 nodes along each axis (%u, %u, %u)",
                       a_nx,
                       a_ny,
                       a_nz);

    const uint64_t count = uint64_t(a_nx) * uint64_t(a_ny) * uint64_t(a_nz);

    EBGEOMETRY_REQUIRE(count <= uint64_t(UINT32_MAX),
                       "SampledGridSDF: the grid has more nodes than 2^32 - 1 (%u x %u x %u)",
                       a_nx,
                       a_ny,
                       a_nz);
    EBGEOMETRY_REQUIRE(uint64_t(a_values.size()) == count,
                       "SampledGridSDF: the grid of %u x %u x %u nodes needs as many values (%zu)",
                       a_nx,
                       a_ny,
                       a_nz,
                       a_values.size());

    // The interpolant's largest slope. In a cell, its derivative along an axis is a weighted mean of the
    // differences along the cell's four edges in that direction, so it is at most the largest of them over the
    // spacing, and the gradient's length in the cell is at most the length of those three bounds. Taken cell by
    // cell, a field of distances gives about 1; the axes' largest differences over the whole grid would give up to
    // sqrt(3).
    for (size_t i = 0; i < a_values.size(); i++) {
      EBGEOMETRY_REQUIRE(std::isfinite(a_values[i]), "SampledGridSDF: value %zu is not finite", i);
    }

    const size_t sx = 1U;
    const size_t sy = size_t(a_nx);
    const size_t sz = size_t(a_nx) * size_t(a_ny);

    const auto at = [&a_values](const size_t a_i) { return double(a_values[a_i]); };

    double lipschitz = 0.0;

    for (uint32_t k = 0; k + 1U < a_nz; k++) {
      for (uint32_t j = 0; j + 1U < a_ny; j++) {
        for (uint32_t i = 0; i + 1U < a_nx; i++) {
          const size_t c = i * sx + j * sy + k * sz;

          double slope[3] = {0.0, 0.0, 0.0};

          const size_t step[3]      = {sx, sy, sz};
          const size_t across[3][2] = {{sy, sz}, {sx, sz}, {sx, sy}};

          for (size_t dir = 0; dir < 3; dir++) {
            for (size_t e = 0; e < 4; e++) {
              const size_t base = c + ((e & 1U) ? across[dir][0] : 0U) + ((e & 2U) ? across[dir][1] : 0U);

              slope[dir] = Math::max(slope[dir], std::abs(at(base + step[dir]) - at(base)));
            }

            slope[dir] /= double(a_spacing[dir]);
          }

          lipschitz = Math::max(lipschitz, std::sqrt(slope[0] * slope[0] + slope[1] * slope[1] + slope[2] * slope[2]));
        }
      }
    }

    if (!(lipschitz > 0.0)) {
      lipschitz = 1.0;
    }

    std::vector<T> scaled(a_values.size());

    for (size_t i = 0; i < a_values.size(); i++) {
      scaled[i] = T(double(a_values[i]) / lipschitz);
    }

    m_layout.m_origin   = a_origin;
    m_layout.m_spacing  = a_spacing;
    m_layout.m_nodes[0] = a_nx;
    m_layout.m_nodes[1] = a_ny;
    m_layout.m_nodes[2] = a_nz;

    this->fill(a_pool, scaled);
  }

  /**
   * @brief A height field (a terrain): the solid between a floor and the surface y = h(x, z), with the heights
   * given on a grid in the xz-plane and interpolated bilinearly.
   * @details Stored as a grid of two layers of nodes, at the floor and above the highest point, whose trilinear
   * interpolation is exactly y - h(x, z); the value is that divided by sqrt(1 + s^2), with s the steepest slope of
   * the surface, a bound.
   * @param[in,out] a_pool    Pool the samples are reserved from; must outlive the grid.
   * @param[in]     a_corner  The corner of the heights' grid with the smallest x and z, and the floor's height as its
   * y: the heights' node (0, 0) is at x and z of a_corner.
   * @param[in]     a_dx      The spacing of the heights along x, finite and positive.
   * @param[in]     a_dz      The spacing of the heights along z, finite and positive.
   * @param[in]     a_nx      Number of heights along x, at least 2.
   * @param[in]     a_nz      Number of heights along z, at least 2.
   * @param[in]     a_heights The heights, x fastest: a_nx a_nz finite values, each above the floor.
   * @return The grid.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static SampledGridSDF
  heightField(Pool&                 a_pool,
              const Vec3T<T>&       a_corner,
              const T               a_dx,
              const T               a_dz,
              const uint32_t        a_nx,
              const uint32_t        a_nz,
              const std::vector<T>& a_heights)
  {
    EBGEOMETRY_REQUIRE(uint64_t(a_heights.size()) == uint64_t(a_nx) * uint64_t(a_nz),
                       "SampledGridSDF::heightField: the grid of %u x %u heights needs as many values (%zu)",
                       a_nx,
                       a_nz,
                       a_heights.size());

    T top = a_corner[1];

    for (size_t i = 0; i < a_heights.size(); i++) {
      EBGEOMETRY_REQUIRE(std::isfinite(a_heights[i]) && a_heights[i] > a_corner[1],
                         "SampledGridSDF::heightField: height %zu (%g) must be finite and above the floor (%g)",
                         i,
                         double(a_heights[i]),
                         double(a_corner[1]));

      top = Math::max(top, a_heights[i]);
    }

    // The top layer is a spacing above the highest point, so that the surface is inside the grid's box.
    const T dy    = top - a_corner[1] + Math::max(a_dx, a_dz);
    const T upper = a_corner[1] + dy;

    std::vector<T> values(2U * a_heights.size());

    for (size_t i = 0; i < a_heights.size(); i++) {
      const size_t x = i % a_nx;
      const size_t z = i / a_nx;

      values[x + 2U * a_nx * z]        = a_corner[1] - a_heights[i];
      values[x + a_nx + 2U * a_nx * z] = upper - a_heights[i];
    }

    return SampledGridSDF(a_pool, a_corner, Vec3T<T>(a_dx, dy, a_dz), a_nx, 2U, a_nz, values);
  }

  /**
   * @brief Copy constructor.
   */
  SampledGridSDF(const SampledGridSDF&) noexcept = default;

  /**
   * @brief Move constructor.
   */
  SampledGridSDF(SampledGridSDF&&) noexcept = default;

  /**
   * @brief Copy assignment.
   * @return Reference to (*this).
   */
  SampledGridSDF&
  operator=(const SampledGridSDF&) noexcept = default;

  /**
   * @brief Move assignment.
   * @return Reference to (*this).
   */
  SampledGridSDF&
  operator=(SampledGridSDF&&) noexcept = default;

  /**
   * @brief The grid's samples and placement without its location, for storing it inside another pool-resident
   * object.
   * @return The layout.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  Layout
  layout() const noexcept
  {
    return m_layout;
  }

  /**
   * @brief Produce a copy of this grid that resolves against @p a_pool: this grid's pool, or a mirror of it.
   * @details Copies nothing.
   * @param[in] a_pool The pool to resolve against.
   * @return The view.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  SampledGridSDF
  rebasedView(const Pool& a_pool) const noexcept
  {
    SampledGridSDF view = *this;

    view.setLocation(this->rebasedLocation(a_pool, m_layout.m_values.endByte(), "SampledGridSDF"));

    return view;
  }

  /**
   * @brief Duplicate this grid's samples into @p a_dstPool.
   * @param[in,out] a_dstPool Pool the copy's samples are reserved from; must outlive the copy.
   * @return An independent grid.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  SampledGridSDF
  deepCopy(Pool& a_dstPool) const
  {
    // Read the samples out before reserving anything: a_dstPool may be this grid's own pool, and a reserve there
    // can move the block.
    const T*             values = m_layout.m_values.data(this->base());
    const std::vector<T> samples(values, values + m_layout.m_values.size());

    SampledGridSDF copy;

    copy.m_layout          = m_layout;
    copy.m_layout.m_values = PODVector<T>{};

    copy.fill(a_dstPool, samples);

    return copy;
  }

  /**
   * @brief Signed distance function for the grid: a bound.
   * @param[in] a_point Position.
   * @return A signed value whose magnitude never exceeds the distance from a_point to the surface; negative inside.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    const BoundingVolumes::AABBT<T> box = this->computeBoundingVolume();

    // The nearest point of the box, and the distance to the box, outside it or (negative) inside it.
    Vec3T<T> nearest;
    T        outside = T(0);
    T        inside  = Math::Limits<T>::max();

    for (size_t dir = 0; dir < 3; dir++) {
      const T lo = box.getLowCorner()[dir];
      const T hi = box.getHighCorner()[dir];

      nearest[dir] = Math::clamp(a_point[dir], lo, hi);

      const T gap = a_point[dir] - nearest[dir];

      outside += gap * gap;
      inside = Math::min(inside, Math::min(a_point[dir] - lo, hi - a_point[dir]));
    }

    const T field = this->interpolate(nearest);

    // Outside the box: the solid is inside it, so the distance is at least the box's, and at least the field's
    // bound at the box's nearest point, across it.
    if (outside > T(0)) {
      const T across = Math::max(field, T(0));

      return std::sqrt(outside + across * across);
    }

    // Inside: the solid is the field's inside cut by the box, a bound being the larger of the two.
    return Math::max(field, -inside);
  }

  /**
   * @brief The interpolated field at a point inside the grid's box, already divided by its largest slope.
   * @param[in] a_point Position, inside the box.
   * @return The field.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  interpolate(const Vec3T<T>& a_point) const noexcept
  {
    uint32_t cell[3];
    T        weight[3];

    for (size_t dir = 0; dir < 3; dir++) {
      const T x = (a_point[dir] - m_layout.m_origin[dir]) / m_layout.m_spacing[dir];
      const T c = Math::clamp(std::floor(x), T(0), T(m_layout.m_nodes[dir] - 2U));

      cell[dir]   = uint32_t(c);
      weight[dir] = Math::clamp(x - c, T(0), T(1));
    }

    const uint32_t nx = m_layout.m_nodes[0];
    const uint32_t ny = m_layout.m_nodes[1];

    const T* v = m_layout.m_values.data(this->base()) + cell[0] + nx * (cell[1] + ny * cell[2]);

    const uint32_t sy = nx;
    const uint32_t sz = nx * ny;

    const T x00 = v[0] + weight[0] * (v[1] - v[0]);
    const T x10 = v[sy] + weight[0] * (v[sy + 1U] - v[sy]);
    const T x01 = v[sz] + weight[0] * (v[sz + 1U] - v[sz]);
    const T x11 = v[sy + sz] + weight[0] * (v[sy + sz + 1U] - v[sy + sz]);

    const T y0 = x00 + weight[1] * (x10 - x00);
    const T y1 = x01 + weight[1] * (x11 - x01);

    return y0 + weight[2] * (y1 - y0);
  }

  /**
   * @brief The number of nodes along an axis.
   * @param[in] a_dir The axis, 0, 1 or 2.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  getNumNodes(const size_t a_dir) const noexcept
  {
    return m_layout.m_nodes[a_dir];
  }

  /**
   * @brief The shape's axis-aligned bounding box: the grid's box.
   * @return The bounding box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    const Vec3T<T> extent(m_layout.m_spacing[0] * T(m_layout.m_nodes[0] - 1U),
                          m_layout.m_spacing[1] * T(m_layout.m_nodes[1] - 1U),
                          m_layout.m_spacing[2] * T(m_layout.m_nodes[2] - 1U));

    return BoundingVolumes::AABBT<T>(m_layout.m_origin, m_layout.m_origin + extent);
  }

private:
  /**
   * @brief Reserves the samples from a pool and copies them in.
   * @param[in,out] a_pool    The pool.
   * @param[in]     a_samples The samples, already scaled.
   */
  EBGEOMETRY_HOST
  void
  fill(Pool& a_pool, const std::vector<T>& a_samples)
  {
    this->attach(a_pool, "SampledGridSDF");

    m_layout.m_values.reserveFrom(a_pool, uint32_t(a_samples.size()));
    m_layout.m_values.assign(this->base(), a_samples.data(), uint32_t(a_samples.size()));
  }

  /**
   * @brief The samples and the grid.
   */
  Layout m_layout;
};

/**
 * @brief A SampledGridSDF must be trivially copyable: a rebasedView() is byte-copied into a device address space.
 */
static_assert(std::is_trivially_copyable_v<SampledGridSDF<float>>, "SampledGridSDF<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<SampledGridSDF<double>>,
              "SampledGridSDF<double> must be trivially copyable");

} // namespace EBGeometry

#endif
