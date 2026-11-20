// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// Test helpers shared by the sphere cloud tests (TestSphereCloudBVH, TestSphereCloudBVHDevice): random clouds,
// queries and rays, brute-force references, the query functors that run on the host and on a device, and the
// memory a build through GPU::parallelFor() writes.

#ifndef EBGEOMETRY_TESTSPHERECLOUDBVHHELPERS_HPP
#define EBGEOMETRY_TESTSPHERECLOUDBVHHELPERS_HPP

#include "EBGeometry.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace EBGeometryTestSphereCloud {

using namespace EBGeometry;

// A polydisperse cloud: centers uniform in the unit cube, radii uniform in [0.005, 0.05], so that some spheres
// overlap and most do not.
template <class T>
std::vector<SphereCloud::Sphere<T>>
makeSpheres(const std::size_t a_n, const unsigned a_seed)
{
  std::mt19937                      rng(a_seed);
  std::uniform_real_distribution<T> unit(T(0), T(1));
  std::uniform_real_distribution<T> radius(T(0.005), T(0.05));

  std::vector<SphereCloud::Sphere<T>> spheres(a_n);

  for (auto& sphere : spheres) {
    sphere.m_center = Vec3T<T>(unit(rng), unit(rng), unit(rng));
    sphere.m_radius = radius(rng);
  }

  return spheres;
}

// Query points over [-0.2, 1.2]^3: outside every sphere, and inside some.
template <class T>
std::vector<Vec3T<T>>
makeQueries(const std::size_t a_n, const unsigned a_seed)
{
  std::mt19937                      rng(a_seed);
  std::uniform_real_distribution<T> unit(T(-0.2), T(1.2));

  std::vector<Vec3T<T>> queries(a_n);

  for (auto& query : queries) {
    query = Vec3T<T>(unit(rng), unit(rng), unit(rng));
  }

  return queries;
}

// The signed distance from a point to one sphere's surface, the oracle's own arithmetic.
template <class T>
T
sphereDistance(const SphereCloud::Sphere<T>& a_sphere, const Vec3T<T>& a_point)
{
  const Vec3T<T> d = a_point - a_sphere.m_center;

  return std::sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]) - a_sphere.m_radius;
}

// The smallest signed distance over every sphere: an oracle independent of the library.
template <class T>
T
bruteForceDistance(const std::vector<SphereCloud::Sphere<T>>& a_spheres, const Vec3T<T>& a_point)
{
  T best = std::numeric_limits<T>::max();

  for (const auto& sphere : a_spheres) {
    best = std::min(best, sphereDistance(sphere, a_point));
  }

  return best;
}

// The closest sphere's index and the signed distance to it, for a device kernel.
template <class T>
struct ClosestSphereIndex
{
  SphereCloudBVH<T> m_cloud;

  EBGEOMETRY_HOST_DEVICE
  std::size_t
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return m_cloud.closestSphere(a_point).index;
  }
};

template <class T>
struct SignedDistance
{
  SphereCloudBVH<T> m_cloud;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return m_cloud.signedDistance(a_point);
  }
};

// Rays for the ray tests: from outside the cloud's box towards a point in it, and from the centers of spheres
// (inside them) in random directions, with unit directions.
template <class T>
std::vector<RayT<T>>
makeRays(const std::vector<SphereCloud::Sphere<T>>& a_spheres, const std::size_t a_n, const unsigned a_seed)
{
  std::mt19937                      rng(a_seed);
  std::uniform_real_distribution<T> unit(T(0), T(1));
  std::uniform_real_distribution<T> side(T(-1), T(1));

  std::vector<RayT<T>> rays;

  for (std::size_t i = 0; i < a_n; i++) {
    Vec3T<T> origin;
    Vec3T<T> direction;

    if (i % 2 == 0) {
      const Vec3T<T> target(unit(rng), unit(rng), unit(rng));

      origin    = Vec3T<T>(T(0.5), T(0.5), T(0.5)) + T(1.5) * Vec3T<T>(side(rng), side(rng), side(rng));
      direction = target - origin;
    }
    else {
      origin    = a_spheres[i % a_spheres.size()].m_center;
      direction = Vec3T<T>(side(rng), side(rng), side(rng));
    }

    rays.push_back(RayT<T>{origin, direction / direction.length()});
  }

  return rays;
}

// Where a ray is inside the union of the spheres, over the whole line: each sphere's interval, the oracle's own
// arithmetic, merged where they meet. A_ambiguous is set where rounding could decide otherwise: a ray within rounding
// of touching a sphere, or two intervals that meet within rounding.
template <class T>
std::vector<std::pair<T, T>>
bruteForceSpans(const std::vector<SphereCloud::Sphere<T>>& a_spheres, const RayT<T>& a_ray, bool& a_ambiguous)
{
  const T eps   = std::numeric_limits<T>::epsilon();
  const T reach = T(10);

  std::vector<std::pair<T, T>> intervals;

  for (const auto& sphere : a_spheres) {
    const Vec3T<T>& d  = a_ray.m_direction;
    const Vec3T<T>  oc = a_ray.m_origin - sphere.m_center;

    // The squared distance of the center from the line, through the cross product.
    const T lx = oc[1] * d[2] - oc[2] * d[1];
    const T ly = oc[2] * d[0] - oc[0] * d[2];
    const T lz = oc[0] * d[1] - oc[1] * d[0];

    const T b    = oc[0] * d[0] + oc[1] * d[1] + oc[2] * d[2];
    const T r    = sphere.m_radius;
    const T disc = r * r - (lx * lx + ly * ly + lz * lz);

    // The rounding of the discriminant near a tangent ray: a few ulps of |oc| r.
    const T oc2 = oc[0] * oc[0] + oc[1] * oc[1] + oc[2] * oc[2];

    if (std::abs(disc) <= T(64) * eps * (r * r + std::sqrt(oc2) * r)) {
      a_ambiguous = true;
    }

    if (disc > T(0)) {
      intervals.emplace_back(-b - std::sqrt(disc), -b + std::sqrt(disc));
    }
  }

  std::sort(intervals.begin(), intervals.end());

  std::vector<std::pair<T, T>> merged;

  for (const auto& interval : intervals) {
    if (!merged.empty() && std::abs(interval.first - merged.back().second) <= T(64) * eps * reach) {
      a_ambiguous = true;
    }

    if (!merged.empty() && interval.first <= merged.back().second) {
      merged.back().second = std::max(merged.back().second, interval.second);
    }
    else {
      merged.push_back(interval);
    }
  }

  return merged;
}

// The first crossing of the union's surface in [a_tMin, a_tMax], and whether it enters: the first span end in the
// interval. +infinity for none.
template <class T>
std::pair<T, bool>
bruteForceFirstCrossing(const std::vector<std::pair<T, T>>& a_spans, const T a_tMin, const T a_tMax)
{
  for (const auto& span : a_spans) {
    if (span.first >= a_tMin) {
      return {(span.first <= a_tMax) ? span.first : std::numeric_limits<T>::infinity(), true};
    }

    if (span.second >= a_tMin) {
      return {(span.second <= a_tMax) ? span.second : std::numeric_limits<T>::infinity(), false};
    }
  }

  return {std::numeric_limits<T>::infinity(), false};
}

// The distance to a ray's first crossing, or -1 for none, for a device kernel.
template <class T, class Shape>
struct RayDistance
{
  Shape m_shape;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const RayT<T>& a_ray) const noexcept
  {
    const HitT<T> hit = firstHit(m_shape, a_ray, T(0), T(10));

    return hit.hit() ? hit.m_t : -T(1);
  }
};

// The cloud index of the sphere a ray first crosses, or InvalidIndex for none, for a device kernel.
template <class T>
struct RaySphere
{
  SphereCloudBVH<T> m_cloud;

  EBGEOMETRY_HOST_DEVICE
  std::size_t
  operator()(const RayT<T>& a_ray) const noexcept
  {
    const HitT<T> hit = firstHit(m_cloud, a_ray, T(0), T(10));

    return hit.hit() ? hit.m_primitive : SphereCloud::InvalidIndex;
  }
};

// The separation of two spheres, the oracle's own arithmetic: |c_a - c_b| - (r_a + r_b).
template <class T>
T
separation(const SphereCloud::Sphere<T>& a_a, const SphereCloud::Sphere<T>& a_b)
{
  return sphereDistance(SphereCloud::Sphere<T>{a_b.m_center, a_a.m_radius + a_b.m_radius}, a_a.m_center);
}

// One sphere's contacts as forEachContact() reports them, in visiting order.
template <class T, class Cloud>
std::vector<SphereCloud::Hit<T>>
contactsOf(const Cloud& a_cloud, const uint32_t a_index, const T a_gap)
{
  std::vector<SphereCloud::Hit<T>> contacts;

  a_cloud.forEachContact(a_index, a_gap, [&contacts](const SphereCloud::Hit<T>& a_hit) { contacts.push_back(a_hit); });

  return contacts;
}

// A sphere's contact count and the sum of (index + 1) over them, for a device kernel: an exact fingerprint of
// which spheres were found.
template <class T>
struct ContactFingerprint
{
  SphereCloudBVH<T> m_cloud;
  T                 m_gap;

  EBGEOMETRY_HOST_DEVICE
  std::uint64_t
  operator()(const uint32_t a_index) const noexcept
  {
    std::uint64_t count = 0;
    std::uint64_t sum   = 0;

    m_cloud.forEachContact(a_index, m_gap, [&count, &sum](const SphereCloud::Hit<T>& a_hit) {
      count += 1;
      sum += a_hit.index + 1U;
    });

    return (count << 40U) + sum;
  }
};

// A sphere's summed separation over its contacts, for a device kernel.
template <class T>
struct ContactSeparationSum
{
  SphereCloudBVH<T> m_cloud;
  T                 m_gap;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const uint32_t a_index) const noexcept
  {
    T sum = T(0);

    m_cloud.forEachContact(a_index, m_gap, [&sum](const SphereCloud::Hit<T>& a_hit) { sum += a_hit.distance; });

    return sum;
  }
};

// Memory a build through GPU::parallelFor() writes and the host reads: managed memory in a CUDA or HIP build, where
// the build's steps run as kernels, and emulated device memory otherwise, where they run as host loops.
inline MemoryResource&
parallelBuildResource() noexcept
{
#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  return managedMemoryResource();
#else
  return emulatedDeviceMemoryResource();
#endif
}

// Two BVHs are the same tree, array for array: every node and its box, and every group's spheres and ids.
template <class T>
void
requireSameTree(const SphereCloudBVH<T>& a_cloud, const SphereCloudBVH<T>& a_reference)
{
  const auto bvh = a_cloud.getBVH();
  const auto ref = a_reference.getBVH();

  const auto nodes    = bvh.getNodes();
  const auto refNodes = ref.getNodes();

  REQUIRE(nodes.size() == refNodes.size());

  for (uint32_t i = 0; i < nodes.size(); i++) {
    INFO("node " << i);
    REQUIRE(nodes[i].isLeaf() == refNodes[i].isLeaf());

    if (nodes[i].isLeaf()) {
      REQUIRE(nodes[i].getPrimitivesOffset() == refNodes[i].getPrimitivesOffset());
      REQUIRE(nodes[i].getNumPrimitives() == refNodes[i].getNumPrimitives());
    }
    else {
      for (std::size_t k = 0; k < BVH::DefaultBranchingRatio<T>(); k++) {
        REQUIRE(nodes[i].getChildOffsets()[k] == refNodes[i].getChildOffsets()[k]);
      }
    }
  }

  const auto box    = bvh.computeBoundingVolume();
  const auto refBox = ref.computeBoundingVolume();

  REQUIRE(box.getLowCorner() == refBox.getLowCorner());
  REQUIRE(box.getHighCorner() == refBox.getHighCorner());

  const auto groups    = bvh.getPrimitives();
  const auto refGroups = ref.getPrimitives();

  REQUIRE(groups.size() == refGroups.size());

  for (uint32_t g = 0; g < groups.size(); g++) {
    REQUIRE(groups[g].numValid() == refGroups[g].numValid());

    const auto groupBox    = groups[g].template computeBoundingVolume<BoundingVolumes::AABBT<T>>();
    const auto refGroupBox = refGroups[g].template computeBoundingVolume<BoundingVolumes::AABBT<T>>();

    REQUIRE(groupBox.getLowCorner() == refGroupBox.getLowCorner());
    REQUIRE(groupBox.getHighCorner() == refGroupBox.getHighCorner());

    for (std::size_t lane = 0; lane < groups[g].numValid(); lane++) {
      REQUIRE(groups[g].getSphereId(lane) == refGroups[g].getSphereId(lane));
    }
  }

  REQUIRE(a_cloud.numSpheres() == a_reference.numSpheres());

  for (uint32_t i = 0; i < a_cloud.numSpheres(); i++) {
    REQUIRE(a_cloud.sphere(i).m_center == a_reference.sphere(i).m_center);
    REQUIRE(a_cloud.sphere(i).m_radius == a_reference.sphere(i).m_radius);
  }
}

} // namespace EBGeometryTestSphereCloud

#endif
