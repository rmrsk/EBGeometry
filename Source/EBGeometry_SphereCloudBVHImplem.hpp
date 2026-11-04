// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file    EBGeometry_SphereCloudBVHImplem.hpp
 * @brief   Implementation of EBGeometry_SphereCloudBVH.hpp
 * @author  Robert Marskar
 */

#ifndef EBGEOMETRY_SPHERECLOUDBVHIMPLEM_HPP
#define EBGEOMETRY_SPHERECLOUDBVHIMPLEM_HPP

// Std includes
#include <cmath>
#include <cstdint>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_Array.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_SphereCloudBVH.hpp"

namespace EBGeometry {

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST
inline SphereCloudBVH<T, K, W>::SphereCloudBVH(Pool&                      a_pool,
                                               const std::vector<Sphere>& a_spheres,
                                               std::size_t                a_targetLeafSize)
{
  *this = build(a_pool, a_spheres, a_targetLeafSize);
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST
inline SphereCloudBVH<T, K, W>::SphereCloudBVH(Pool&                        a_pool,
                                               const std::vector<Vec3T<T>>& a_centers,
                                               const std::vector<T>&        a_radii,
                                               std::size_t                  a_targetLeafSize)
{
  *this = build(a_pool, a_centers, a_radii, a_targetLeafSize);
}

template <class T, size_t K, size_t W>
template <class Source>
EBGEOMETRY_HOST
inline SphereCloudBVH<T, K, W>
SphereCloudBVH<T, K, W>::build(Pool&             a_pool,
                               const Source&     a_source,
                               const uint32_t    a_numSpheres,
                               const std::size_t a_targetLeafSize,
                               const uint32_t    a_maxSpheres)
{
  static_assert(std::is_floating_point_v<T>, "SphereCloudBVH requires a floating-point type T");
  static_assert(K >= 2, "SphereCloudBVH requires a branching factor K >= 2");
  static_assert(W >= 1, "SphereCloudBVH requires a SIMD width W >= 1");
  static_assert(std::is_trivially_copyable_v<Source>,
                "SphereCloudBVH::build: the source of spheres must be trivially copyable");

  SphereCloudBVH bvh;

  bvh.initialize(a_pool, a_source, a_numSpheres, a_targetLeafSize, a_maxSpheres, "SphereCloudBVH::build");

  return bvh;
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST
inline SphereCloudBVH<T, K, W>
SphereCloudBVH<T, K, W>::build(Pool&                      a_pool,
                               const std::vector<Sphere>& a_spheres,
                               const std::size_t          a_targetLeafSize,
                               const uint32_t             a_maxSpheres)
{
  SphereCloudBVH bvh;

  bvh.initialize(a_pool, a_spheres, a_targetLeafSize, a_maxSpheres, "SphereCloudBVH::build");

  return bvh;
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST
inline SphereCloudBVH<T, K, W>
SphereCloudBVH<T, K, W>::build(Pool&                        a_pool,
                               const std::vector<Vec3T<T>>& a_centers,
                               const std::vector<T>&        a_radii,
                               const std::size_t            a_targetLeafSize,
                               const uint32_t               a_maxSpheres)
{
  EBGEOMETRY_REQUIRE(a_centers.size() == a_radii.size(),
                     "SphereCloudBVH::build: %zu centers but %zu radii",
                     a_centers.size(),
                     a_radii.size());

  // A build through GPU::parallelFor() needs the spheres checked by index and in memory a kernel can read: the one
  // array of spheres does both.
  if (a_pool.resource().isDeviceAccessible()) {
    std::vector<Sphere> spheres(a_centers.size());

    for (std::size_t i = 0; i < spheres.size(); i++) {
      spheres[i] = Sphere{a_centers[i], a_radii[i]};
    }

    return build(a_pool, spheres, a_targetLeafSize, a_maxSpheres);
  }

  EBGEOMETRY_REQUIRE(a_centers.size() < std::size_t(SphereCloud::InvalidIndex),
                     "SphereCloudBVH::build: too many spheres for 32-bit indices (%zu spheres, limit %zu)",
                     a_centers.size(),
                     std::size_t(SphereCloud::InvalidIndex) - 1);

  using Source = SphereCloud::FromCentersAndRadii<T, PointCloud::FromArray<T>, SphereCloud::FromRadii<T>>;

  return build(a_pool,
               Source{PointCloud::FromArray<T>{a_centers.data()}, SphereCloud::FromRadii<T>{a_radii.data()}},
               static_cast<uint32_t>(a_centers.size()),
               a_targetLeafSize,
               a_maxSpheres);
}

template <class T, size_t K, size_t W>
template <class Source>
EBGEOMETRY_HOST
inline void
SphereCloudBVH<T, K, W>::rebuild(const Source& a_source, const uint32_t a_numSpheres)
{
  static_assert(std::is_trivially_copyable_v<Source>,
                "SphereCloudBVH::rebuild: the source of spheres must be trivially copyable");

  this->reinitialize(a_source, a_numSpheres, "SphereCloudBVH::rebuild");
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
SphereCloudBVH<T, K, W>::maxSpheres() const noexcept
{
  return m_maxItems;
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline T
SphereCloudBVH<T, K, W>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  return this->closestSphere(a_point).distance;
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline typename SphereCloudBVH<T, K, W>::Hit
SphereCloudBVH<T, K, W>::closestSphere(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  // Resolved once per query: a query reserves nothing, so the pool block cannot move under them.
  const Packed                     bvh    = this->getBVH();
  const PODSpan<const SphereGroup> groups = bvh.getPrimitives();

  Hit best;

  // An empty cloud has no nodes, and the traversal would read the root.
  if (bvh.getNodes().size() == 0) {
    return best;
  }

  // The lane loop has the constant bound W, with the padding test inside it, so that the compiler unrolls it and
  // keeps the group's distances in registers on a device (as PointCloudBVH's leaf scan does). Ties go to the lower
  // cloud index, so the answer does not depend on the order the leaves are visited in.
  const auto evalLeaf = [&groups, &a_point](Hit& a_best, std::size_t a_off, std::size_t a_cnt) noexcept {
    for (std::size_t g = 0; g < a_cnt; g++) {
      const SphereGroup&  group     = groups[static_cast<std::uint32_t>(a_off + g)];
      const Array<T, W>   distances = group.getSignedDistances(a_point);
      const std::uint32_t numValid  = group.numValid();

      for (std::uint32_t lane = 0; lane < W; lane++) {
        if (lane < numValid) {
          const T             distance = distances[lane];
          const std::uint32_t index    = group.getSphereId(lane);

          if (distance < a_best.distance || (distance == a_best.distance && index < a_best.index)) {
            a_best = Hit{index, distance};
          }
        }
      }
    }
  };

  // A box farther than the best signed distance holds no closer sphere, since each box holds its spheres whole.
  // Once the point is inside a sphere the bound is 0, and only the boxes holding the point are entered.
  const auto bound = [](const Hit& a_best) noexcept -> T {
    const T distance = Math::max(T(0), a_best.distance);

    return distance * distance;
  };

  bvh.traverse(BVH::PointQuery<T>{a_point}, best, evalLeaf, bound);

  return best;
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline typename SphereCloudBVH<T, K, W>::AABB
SphereCloudBVH<T, K, W>::computeBoundingVolume() const noexcept
{
  return this->getBVH().computeBoundingVolume();
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
SphereCloudBVH<T, K, W>::numSpheres() const noexcept
{
  return m_items.size();
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline typename SphereCloudBVH<T, K, W>::Sphere
SphereCloudBVH<T, K, W>::sphere(const uint32_t a_index) const noexcept
{
  EBGEOMETRY_EXPECT(a_index < m_items.size());

  return m_items.at(this->base(), a_index);
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline typename SphereCloudBVH<T, K, W>::Packed
SphereCloudBVH<T, K, W>::getBVH() const noexcept
{
  return Packed(m_bvhLayout, this->location());
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST
inline SphereCloudBVH<T, K, W>
SphereCloudBVH<T, K, W>::rebasedView(const Pool& a_pool) const noexcept
{
  SphereCloudBVH view = *this;

  view.rebase(a_pool);

  return view;
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST
inline SphereCloudBVH<T, K, W>
SphereCloudBVH<T, K, W>::deepCopy(Pool& a_dstPool) const
{
  SphereCloudBVH copy = *this;

  this->copyInto(copy, a_dstPool);

  return copy;
}

} // namespace EBGeometry

#endif
