// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file    EBGeometry_SphereCloudBVH.hpp
 * @brief   A BVH over spheres of differing radii, built where its pool is: the signed distance to their union
 *          and the closest sphere.
 * @author  Robert Marskar
 */

#ifndef EBGEOMETRY_SPHERECLOUDBVH_HPP
#define EBGEOMETRY_SPHERECLOUDBVH_HPP

// Std includes
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_CloudBVH.hpp"
#include "EBGeometry_GPUAlgorithms.hpp"
#include "EBGeometry_PackedBVH.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_SphereAoSoA.hpp"
#include "EBGeometry_SphereCloud.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

namespace PointCloudDetail {

/**
 * @brief A source of points that are a sphere's radius in every coordinate: GPU::boundingBoxOf() over it finds the
 * smallest radius and whether every radius is finite.
 * @details Internal; not part of the public API.
 * @tparam T      Floating-point precision.
 * @tparam Source The spheres' source.
 */
template <class T, class Source>
struct RadiiAsPoints
{
  /// @brief The spheres.
  Source m_source;

  /**
   * @brief The radius of a sphere, in every coordinate.
   * @param[in] a_index Cloud index.
   * @return The point.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  Vec3T<T>
  operator()(const uint32_t a_index) const noexcept
  {
    const T radius = m_source(a_index).m_radius;

    return Vec3T<T>(radius, radius, radius);
  }
};

/**
 * @brief The primitive of SphereCloudBVH: a sphere, keyed at its center and packed into SphereAoSoA groups; see
 * PointPrimitive.
 * @details Internal; not part of the public API.
 * @tparam T Floating-point precision.
 * @tparam W Spheres per group.
 */
template <class T, size_t W>
struct SpherePrimitive
{
  /// @brief What the source returns: a sphere.
  using Item = SphereCloud::Sphere<T>;

  /// @brief The leaf group.
  using Group = SphereAoSoA<T, W>;

  /// @brief The source reading an array of primitives.
  using FromArray = SphereCloud::FromArray<T>;

  /// @brief The class built over these primitives, for the messages.
  static constexpr const char* ClassName = "SphereCloudBVH";

  /// @brief One primitive, for the messages.
  static constexpr const char* Singular = "sphere";

  /// @brief Primitives, for the messages.
  static constexpr const char* Plural = "spheres";

  /// @brief build()'s parameter giving the room for a rebuild, for the messages.
  static constexpr const char* MaxParameter = "a_maxSpheres";

  /**
   * @brief The point a sphere's Morton key is taken at.
   * @param[in] a_item The sphere.
   * @return Its center.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline const Vec3T<T>&
  center(const Item& a_item) noexcept
  {
    return a_item.m_center;
  }

  /**
   * @brief Grow a box to hold a sphere.
   * @param[in,out] a_lo   The box's lower corner.
   * @param[in,out] a_hi   The box's upper corner.
   * @param[in]     a_item The sphere.
   */
  EBGEOMETRY_HOST_DEVICE
  static inline void
  grow(Vec3T<T>& a_lo, Vec3T<T>& a_hi, const Item& a_item) noexcept
  {
    const Vec3T<T> radius = a_item.m_radius * Vec3T<T>::ones();

    a_lo = min(a_lo, a_item.m_center - radius);
    a_hi = max(a_hi, a_item.m_center + radius);
  }

  /**
   * @brief Abort with a message unless the spheres of a host array can be built over: a finite center, and a
   * finite radius of at least 0.
   * @param[in] a_who   The calling function, for the message.
   * @param[in] a_items The spheres.
   */
  EBGEOMETRY_HOST
  static inline void
  requireValid(const char* a_who, const std::vector<Item>& a_items) noexcept
  {
    for (std::size_t i = 0; i < a_items.size(); i++) {
      const Item& sphere = a_items[i];

      EBGEOMETRY_REQUIRE(std::isfinite(sphere.m_center[0]) && std::isfinite(sphere.m_center[1]) &&
                           std::isfinite(sphere.m_center[2]),
                         "%s: sphere %zu of %zu has a non-finite coordinate",
                         a_who,
                         i,
                         a_items.size());
      EBGEOMETRY_REQUIRE(std::isfinite(sphere.m_radius) && sphere.m_radius >= T(0),
                         "%s: sphere %zu of %zu has a non-finite or negative radius",
                         a_who,
                         i,
                         a_items.size());
    }
  }

  /**
   * @brief Check every radius, where the build runs: finite, and at least 0.
   * @details The centers are checked with the box of the centers. Through GPU::parallelFor(), the smallest radius
   * and whether every radius is finite come from GPU::boundingBoxOf(), and a bad one is reported as "a sphere"; on
   * the host, it is reported by its index.
   * @tparam Source The spheres' source.
   * @param[in] a_source    The spheres.
   * @param[in] a_numItems  Number of spheres.
   * @param[in] a_parallel  Whether the build runs through GPU::parallelFor().
   * @param[in] a_scratch   GPU::boundingBoxScratchSize(a_numItems) elements, for a build through
   *                        GPU::parallelFor().
   * @param[in] a_who       The calling function, for the message.
   */
  template <class Source>
  EBGEOMETRY_HOST
  static inline void
  requireValidWhereBuilt(const Source&  a_source,
                         const uint32_t a_numItems,
                         const bool     a_parallel,
                         Vec3T<T>*      a_scratch,
                         const char*    a_who) noexcept
  {
    if (a_numItems == 0) {
      return;
    }

    if (a_parallel) {
      const BoundingVolumes::AABBT<T> box =
        GPU::boundingBoxOf<T>(RadiiAsPoints<T, Source>{a_source}, a_numItems, a_scratch);

      const T smallest = box.getLowCorner()[0];
      const T largest  = box.getHighCorner()[0];

      EBGEOMETRY_REQUIRE(std::isfinite(smallest) && std::isfinite(largest) && smallest >= T(0),
                         "%s: a sphere has a non-finite or negative radius",
                         a_who);

      return;
    }

    for (uint32_t i = 0; i < a_numItems; i++) {
      const T radius = a_source(i).m_radius;

      EBGEOMETRY_REQUIRE(std::isfinite(radius) && radius >= T(0),
                         "%s: sphere %u of %u has a non-finite or negative radius",
                         a_who,
                         unsigned(i),
                         unsigned(a_numItems));
    }
  }
};

} // namespace PointCloudDetail

/**
 * @brief A BVH over spheres of differing radii (polydisperse particles), with the signed distance to their
 * union and the closest sphere.
 * @details A shape: signedDistance() and computeBoundingVolume(), negative inside a sphere. The distance is
 * min_i(|p - c_i| - r_i) over the spheres: exact outside their union, and inside it a bound in magnitude (where
 * spheres overlap, the union's surface can be farther than every sphere's own), as for a Union of the
 * spheres. Spheres of one radius need no SphereCloudBVH: the distance from a PointCloudBVH of the centers,
 * less the radius, is the same.
 *
 * The build is PointCloudBVH's linear build (see there), over the centers: the Morton keys and cuts come
 * from the centers, and every box is grown by its spheres' radii. It runs where the pool's memory is, on a
 * device included, and every side gives the same tree. The leaves are SphereAoSoA groups carrying each
 * sphere's **cloud index**, which closestSphere() reports.
 *
 * @note Device portability, as for PointCloudBVH: the packed BVH and the cloud's arrays are reserved from
 * one Pool and the class stores only offsets into it, so it is trivially copyable. Freeze and mirror the
 * pool, call rebasedView(), and pass the value into a kernel; signedDistance(), closestSphere() and
 * computeBoundingVolume() are device-callable. The pool must outlive the BVH and every copy of it.
 *
 * @tparam T Floating-point precision.
 * @tparam K BVH branching factor. Defaults to BVH::DefaultBranchingRatio<T>().
 * @tparam W Spheres per SoA leaf group. Defaults to PointSoA::DefaultWidth<T>().
 */
template <class T, size_t K = BVH::DefaultBranchingRatio<T>(), size_t W = PointSoA::DefaultWidth<T>()>
class SphereCloudBVH : public PointCloudDetail::CloudBVH<T, K, W, PointCloudDetail::SpherePrimitive<T, W>>
{
public:
  /**
   * @brief A sphere: a center and a radius.
   */
  using Sphere = SphereCloud::Sphere<T>;

  /**
   * @brief One query result: the cloud index of a sphere and the signed distance to its surface.
   */
  using Hit = SphereCloud::Hit<T>;

  /**
   * @brief The SoA leaf primitive: a group of up to W spheres carrying their cloud indices.
   */
  using SphereGroup = SphereAoSoA<T, W>;

  /**
   * @brief The packed BVH this class holds over its sphere groups.
   */
  using Packed = BVH::PackedBVH<T, SphereGroup, K>;

  /**
   * @brief Axis-aligned bounding box type.
   */
  using AABB = BoundingVolumes::AABBT<T>;

  /**
   * @brief Build a BVH over spheres on the host: shorthand for build(a_pool, a_spheres, a_targetLeafSize).
   * @details A sphere's cloud index, which the queries report, is its position in @p a_spheres.
   * @param[in,out] a_pool           Pool the BVH's arrays are reserved from; must outlive this object.
   * @param[in]     a_spheres        The spheres. At most 2^31; centers and radii finite, radii at least 0.
   * @param[in]     a_targetLeafSize Target spheres per leaf. Must be at least 1.
   */
  EBGEOMETRY_HOST
  inline SphereCloudBVH(Pool& a_pool, const std::vector<Sphere>& a_spheres, std::size_t a_targetLeafSize = 16 * W);

  /**
   * @brief Build a BVH over spheres given as centers and radii, on the host: shorthand for build(a_pool,
   * a_centers, a_radii, a_targetLeafSize).
   * @details A sphere's cloud index is its position in @p a_centers and @p a_radii.
   * @param[in,out] a_pool           Pool the BVH's arrays are reserved from; must outlive this object.
   * @param[in]     a_centers        The centers. At most 2^31, all finite.
   * @param[in]     a_radii          The radii, as many as the centers. All finite and at least 0.
   * @param[in]     a_targetLeafSize Target spheres per leaf. Must be at least 1.
   */
  EBGEOMETRY_HOST
  inline SphereCloudBVH(Pool&                        a_pool,
                        const std::vector<Vec3T<T>>& a_centers,
                        const std::vector<T>&        a_radii,
                        std::size_t                  a_targetLeafSize = 16 * W);

  /**
   * @brief Build a BVH over spheres, where the pool's memory is.
   * @details As PointCloudBVH::build(), over the centers, with every box grown by the radii: it runs
   * through GPU::parallelFor() for a pool in device-accessible memory (reading @p a_source on the device in
   * a CUDA or HIP translation unit), and on the host otherwise, and both give the same tree. A sphere with
   * a non-finite center or radius, or a negative radius, is rejected (on the host by its index). With
   * @p a_maxSpheres, the arrays are reserved for any cloud of that many spheres, so that rebuild()
   * reserves nothing and works on a frozen pool.
   * @tparam Source A source of spheres: a trivially copyable type whose const call operator takes a cloud
   * index (uint32_t) and returns that sphere; SphereCloud::FromArray, or SphereCloud::FromCentersAndRadii
   * for centers and radii kept apart.
   * @param[in,out] a_pool           Pool the BVH's arrays are reserved from; must outlive the BVH.
   * @param[in]     a_source         The spheres.
   * @param[in]     a_numSpheres     Number of spheres. At most 2^31.
   * @param[in]     a_targetLeafSize Target spheres per leaf (a node of at most max(a_targetLeafSize, K)
   *                                 spheres is a leaf). Must be at least 1.
   * @param[in]     a_maxSpheres     Most spheres a rebuild() may have; 0 for no rebuild(), and the arrays
   *                                 this cloud needs only. At least a_numSpheres otherwise.
   * @return The BVH.
   */
  template <class Source>
  [[nodiscard]] EBGEOMETRY_HOST
  static inline SphereCloudBVH
  build(Pool&         a_pool,
        const Source& a_source,
        uint32_t      a_numSpheres,
        std::size_t   a_targetLeafSize = 16 * W,
        uint32_t      a_maxSpheres     = 0);

  /**
   * @brief Build a BVH over spheres on the host, where the pool's memory is; see the build() above.
   * @details For a pool in device memory, the spheres are first copied to scratch there.
   * @param[in,out] a_pool           Pool the BVH's arrays are reserved from; must outlive the BVH.
   * @param[in]     a_spheres        The spheres. At most 2^31; centers and radii finite, radii at least 0.
   * @param[in]     a_targetLeafSize Target spheres per leaf. Must be at least 1.
   * @param[in]     a_maxSpheres     Most spheres a rebuild() may have (see the build() above).
   * @return The BVH.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static inline SphereCloudBVH
  build(Pool&                      a_pool,
        const std::vector<Sphere>& a_spheres,
        std::size_t                a_targetLeafSize = 16 * W,
        uint32_t                   a_maxSpheres     = 0);

  /**
   * @brief Build a BVH over spheres given as centers and radii, on the host, where the pool's memory is; see
   * the build() above.
   * @details For a pool in device memory, the centers and radii are first copied to scratch there.
   * @param[in,out] a_pool           Pool the BVH's arrays are reserved from; must outlive the BVH.
   * @param[in]     a_centers        The centers. At most 2^31, all finite.
   * @param[in]     a_radii          The radii, as many as the centers. All finite and at least 0.
   * @param[in]     a_targetLeafSize Target spheres per leaf. Must be at least 1.
   * @param[in]     a_maxSpheres     Most spheres a rebuild() may have (see the build() above).
   * @return The BVH.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static inline SphereCloudBVH
  build(Pool&                        a_pool,
        const std::vector<Vec3T<T>>& a_centers,
        const std::vector<T>&        a_radii,
        std::size_t                  a_targetLeafSize = 16 * W,
        uint32_t                     a_maxSpheres     = 0);

  /**
   * @brief Build the BVH again, in the arrays build() reserved, from new spheres.
   * @details As PointCloudBVH::rebuild(): the same tree as a new build() from these spheres, reserving
   * nothing in the pool, so it works on a frozen pool. Needs room reserved by build()'s a_maxSpheres; more
   * spheres than maxSpheres() is rejected. A view taken with rebasedView() before the rebuild still
   * describes the old tree: take a new one. Call it on the BVH build() returned, not on a view or a
   * deepCopy().
   * @tparam Source A source of spheres, as for build().
   * @param[in] a_source     The spheres.
   * @param[in] a_numSpheres Number of spheres, at most maxSpheres().
   */
  template <class Source>
  EBGEOMETRY_HOST
  inline void
  rebuild(const Source& a_source, uint32_t a_numSpheres);

  /**
   * @brief The most spheres rebuild() can take.
   * @return build()'s a_maxSpheres; 0 for a BVH built without room for a rebuild.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  maxSpheres() const noexcept;

  /**
   * @brief Signed distance to the union of the spheres: min_i(|p - c_i| - r_i); see the class.
   * @param[in] a_point Query point.
   * @return The signed distance, negative inside a sphere; Math::Limits<T>::max() for an empty cloud.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  signedDistance(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief The sphere whose surface is nearest to a point, in the signed sense of signedDistance().
   * @details Ties go to the lower cloud index.
   * @param[in] a_point Query point.
   * @return The sphere's cloud index and the signed distance to its surface; a miss for an empty cloud.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Hit
  closestSphere(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief The box of all the spheres.
   * @return The root box: every sphere, grown by its radius. An empty box for an empty cloud.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline AABB
  computeBoundingVolume() const noexcept;

  /**
   * @brief Number of spheres in the cloud.
   * @return The number of spheres.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  numSpheres() const noexcept;

  /**
   * @brief The sphere with a cloud index.
   * @param[in] a_index Cloud index, less than numSpheres().
   * @return The sphere.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Sphere
  sphere(uint32_t a_index) const noexcept;

  /**
   * @brief The held packed BVH over the sphere groups.
   * @return A PackedBVH that shares this object's location.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Packed
  getBVH() const noexcept;

  /**
   * @brief Produce a copy of this object that resolves against @p a_pool.
   * @details For a pool mirrored from this object's pool (Pool::mirror()): the returned value reads the
   * mirror, on a device included. Copies nothing.
   * @param[in] a_pool The pool to resolve against: this object's pool, or a mirror of it.
   * @return The view.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline SphereCloudBVH
  rebasedView(const Pool& a_pool) const noexcept;

  /**
   * @brief Duplicate this object's storage into @p a_dstPool.
   * @param[in,out] a_dstPool Pool the copy's arrays are reserved from; must outlive the copy.
   * @return An independent BVH over the same spheres. It has no room for a rebuild().
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline SphereCloudBVH
  deepCopy(Pool& a_dstPool) const;

  /**
   * @brief The base address this object's arrays are offsets from; see PoolHandle::base().
   * @return The base address: const void* for a const object, void* otherwise.
   */
  using PoolHandle::base;

private:
  /**
   * @brief The build and its state.
   */
  using Base = PointCloudDetail::CloudBVH<T, K, W, PointCloudDetail::SpherePrimitive<T, W>>;

  using Base::m_bvhLayout;
  using Base::m_items;
  using Base::m_maxItems;

  /**
   * @brief An empty BVH, to be reserved and filled.
   */
  SphereCloudBVH() noexcept = default;
};

/**
 * @brief A SphereCloudBVH must be trivially copyable, as a PointCloudBVH must: a rebasedView() is byte-copied into a
 * device address space with no pointer patching.
 */
static_assert(std::is_trivially_copyable_v<SphereCloudBVH<float>>, "SphereCloudBVH<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<SphereCloudBVH<double>>,
              "SphereCloudBVH<double> must be trivially copyable");

} // namespace EBGeometry

#include "EBGeometry_SphereCloudBVHImplem.hpp"

#endif
