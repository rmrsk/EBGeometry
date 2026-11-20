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
#include <cstdint>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_DistanceQuality.hpp"
#include "EBGeometry_GPU.hpp"
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
                 uint32_t              a_nx,
                 uint32_t              a_ny,
                 uint32_t              a_nz,
                 const std::vector<T>& a_values)
  {
    (void)a_pool;
    (void)a_values;

    m_layout.m_origin   = a_origin;
    m_layout.m_spacing  = a_spacing;
    m_layout.m_nodes[0] = a_nx;
    m_layout.m_nodes[1] = a_ny;
    m_layout.m_nodes[2] = a_nz;
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
              T                     a_dx,
              T                     a_dz,
              uint32_t              a_nx,
              uint32_t              a_nz,
              const std::vector<T>& a_heights)
  {
    (void)a_pool;
    (void)a_corner;
    (void)a_dx;
    (void)a_dz;
    (void)a_nx;
    (void)a_nz;
    (void)a_heights;

    return SampledGridSDF();
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
    (void)a_dstPool;

    return *this;
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
    (void)a_point;

    return T(0);
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
