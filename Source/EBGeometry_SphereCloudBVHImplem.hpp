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
template <class Visit>
EBGEOMETRY_HOST_DEVICE
inline void
SphereCloudBVH<T, K, W>::forEachContact(const uint32_t a_index, const T a_gap, Visit&& a_visit) const noexcept
{
  EBGEOMETRY_EXPECT(a_index < m_items.size());

  // The visitor is passed on as an lvalue, not forwarded: it is called once per contact, and a visitor that adds
  // up (a count, a force) must be the caller's own object.
  this->forEachContact(this->sphere(a_index), a_gap, a_visit, a_index);
}

template <class T, size_t K, size_t W>
template <class Visit>
EBGEOMETRY_HOST_DEVICE
inline void
SphereCloudBVH<T, K, W>::forEachContact(const Sphere&  a_sphere,
                                        const T        a_gap,
                                        Visit&&        a_visit,
                                        const uint32_t a_exclude) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_sphere.m_center[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_sphere.m_center[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_sphere.m_center[2]));
  EBGEOMETRY_EXPECT(std::isfinite(a_sphere.m_radius) && a_sphere.m_radius >= T(0));
  EBGEOMETRY_EXPECT(std::isfinite(a_gap));

  const Packed                     bvh    = this->getBVH();
  const PODSpan<const SphereGroup> groups = bvh.getPrimitives();

  // An empty cloud has no nodes, and the traversal would read the root.
  if (bvh.getNodes().size() == 0) {
    return;
  }

  const Vec3T<T>& center = a_sphere.m_center;
  const T         radius = a_sphere.m_radius;

  // As in closestSphere(), the lane loop has the constant bound W with the padding test inside it.
  const auto evalLeaf =
    [&groups, &center, radius, a_gap, a_exclude](auto& a_state, std::size_t a_off, std::size_t a_cnt) noexcept {
      for (std::size_t g = 0; g < a_cnt; g++) {
        const SphereGroup&  group       = groups[static_cast<std::uint32_t>(a_off + g)];
        const Array<T, W>   separations = group.getSeparations(center, radius);
        const std::uint32_t numValid    = group.numValid();

        for (std::uint32_t lane = 0; lane < W; lane++) {
          if (lane < numValid && separations[lane] < a_gap) {
            const std::uint32_t index = group.getSphereId(lane);

            if (index != a_exclude) {
              a_state(Hit{index, separations[lane]});
            }
          }
        }
      }
    };

  // A sphere j in contact has |c - c_j| - r_j < r + gap, and its box holds it whole, so the box is within
  // r + gap of the center (or holds the center). The bound is fixed, so every box within it is entered, in the
  // order stored. With r + gap <= 0, only the boxes holding the center are.
  //
  // In floating point a box corner c_j + r_j can round inward, and the box's key has rounding of its own, so the
  // reach is padded by a few ulps of the coordinates' size: the query sphere's, and the root box's, which holds
  // every c_j + r_j. Without the pad, a pair whose separation is within rounding of the gap could be pruned from
  // one side and found from the other, and contacts would not be mutual. The leaf test is exact, so the pad only
  // enters a box more now and then; it finds no extra contact.
  const AABB     root = bvh.computeBoundingVolume();
  const Vec3T<T> lo   = root.getLowCorner();
  const Vec3T<T> hi   = root.getHighCorner();

  T largest = T(0);

  for (std::size_t d = 0; d < 3; d++) {
    largest = Math::max(largest, Math::max(Math::max(std::abs(lo[d]), std::abs(hi[d])), std::abs(center[d])));
  }

  const T scale = largest + radius + std::abs(a_gap);
  const T reach = Math::max(T(0), radius + a_gap) + T(16) * Math::Limits<T>::epsilon() * scale;

  const auto bound = [reach](const auto&) noexcept -> T { return reach * reach; };

  bvh.traverse(BVH::PointQuery<T>{center}, a_visit, evalLeaf, bound, BVH::DescendAll{}, BVH::AsStored{});
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
inline SphereCloudBVH<T, K, W>::SphereCloudBVH(const Layout& a_layout, const PoolLocation& a_location) noexcept
{
  m_bvhLayout = a_layout.m_bvh;
  m_items     = a_layout.m_spheres;
  m_leafOff   = a_layout.m_leafOff;
  m_leafCnt   = a_layout.m_leafCnt;
  m_order     = a_layout.m_order;
  m_depth     = a_layout.m_depth;

  this->setLocation(a_location);
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline typename SphereCloudBVH<T, K, W>::Layout
SphereCloudBVH<T, K, W>::layout() const noexcept
{
  return Layout{m_bvhLayout, m_items, m_leafOff, m_leafCnt, m_order, m_depth};
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
inline SurfaceRef<T>
SphereCloudBVH<T, K, W>::closestSurface(const Vec3T<T>& a_point) const noexcept
{
  const Hit hit = this->closestSphere(a_point);

  SurfaceRef<T> surface;

  if (hit.valid()) {
    surface.m_surface    = hit.index;
    surface.m_localPoint = a_point;
  }

  return surface;
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST
inline uint32_t
SphereCloudBVH<T, K, W>::getNumSurfaces() const noexcept
{
  return Math::max(this->maxSpheres(), this->numSpheres());
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
