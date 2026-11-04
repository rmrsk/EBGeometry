// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for SphereCloudBVH::forEachContact(), over 500,000 spheres of differing radii (centers uniform in
// the unit cube), in float and double: what a particle code's step does, each particle's contacts within a gap and
// the force of those it overlaps, one particle per query, the particles in Morton order. A host build runs it on the
// host; a CUDA or HIP build runs it as a host/device pair. The cases live apart from Performance/SphereCloudBVH so
// that they cannot move the inlining of its cases. See ../README.md and PerformanceTesting.rst.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// The parameters that select code, spelled out rather than taken from the defaults: the branching factor, the
// group width and the target spheres per leaf (its default, 16 W).
constexpr std::size_t K              = 4;
constexpr std::size_t W              = 4;
constexpr std::size_t TargetLeafSize = 16 * W;

// The cloud's size, at full and at smoke size.
constexpr std::size_t CloudSpheres      = 500000;
constexpr std::size_t SmokeCloudSpheres = 2000;

// The largest separation counted as a contact: about a mean radius, so that a sphere has a few contacts.
constexpr double Gap = 0.004;

template <class T>
using BVH = EBGeometry::SphereCloudBVH<T, K, W>;

template <class T>
using Sphere = EBGeometry::SphereCloud::Sphere<T>;

template <class T>
using Hit = EBGeometry::SphereCloud::Hit<T>;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

// The spheres, as Performance/SphereCloudBVH draws them: centers uniform in the unit cube, radii uniform in
// [0.002, 0.006].
template <class T>
std::vector<Sphere<T>>
spheres(const std::size_t a_n)
{
  const auto centers = Perf::pointsInBox<T>(Perf::cube(T(0), T(1)), a_n, Perf::Seed::Cloud);
  const auto radii   = Perf::pointsInBox<T>(Perf::cube(T(0.002), T(0.006)), a_n, Perf::Seed::SphereRadii);

  std::vector<Sphere<T>> result(a_n);

  for (std::size_t i = 0; i < a_n; i++) {
    result[i] = Sphere<T>{centers[i], radii[i][0]};
  }

  return result;
}

// What one particle's query gives: its contacts' count and the sum of their indices (wrapping), and the force of
// those it overlaps, each pushing along the line of centers in proportion to the overlap.
template <class T>
struct Contacts
{
  std::uint32_t count;
  std::uint32_t indexSum;
  Vec3<T>       force;
};

template <class T>
struct ContactForce
{
  BVH<T> m_cloud;
  T      m_gap;

  EBGEOMETRY_HOST_DEVICE
  Contacts<T>
  operator()(const std::uint32_t a_index) const noexcept
  {
    const Sphere<T> self = m_cloud.sphere(a_index);

    Contacts<T> result{0U, 0U, Vec3<T>::zeros()};

    m_cloud.forEachContact(a_index, m_gap, [this, &self, &result](const Hit<T>& a_hit) {
      result.count += 1U;
      result.indexSum += a_hit.index;

      if (a_hit.distance < T(0)) {
        const Vec3<T> d      = self.m_center - m_cloud.sphere(a_hit.index).m_center;
        const T       length = d.length();

        if (length > T(0)) {
          result.force += (-a_hit.distance / length) * d;
        }
      }
    });

    return result;
  }
};

// Whether a host and a device result agree: the same contacts exactly, and the forces within agreementTolerance().
template <class T>
std::pair<double, bool>
agreeContacts(const Contacts<T>& a_host, const Contacts<T>& a_device)
{
  double diff = 0.0;

  for (std::size_t d = 0; d < 3; d++) {
    diff = std::max(diff, std::abs(static_cast<double>(a_host.force[d]) - static_cast<double>(a_device.force[d])));
  }

  const double scale = static_cast<double>(a_host.force.length());

  return {diff,
          a_host.count != a_device.count || a_host.indexSum != a_device.indexSum ||
            diff > Perf::agreementTolerance<T>(scale)};
}

// A case's checksums: the sum of the forces' components, and an exact hash of every particle's contacts.
template <class T>
void
record(Perf::Case& a_case, const std::vector<Contacts<T>>& a_results)
{
  std::vector<T>      forces;
  Perf::IndexChecksum indices;

  forces.reserve(3 * a_results.size());

  for (const auto& r : a_results) {
    forces.push_back(r.force[0]);
    forces.push_back(r.force[1]);
    forces.push_back(r.force[2]);
    indices.add(r.count);
    indices.add(r.indexSum);
  }

  a_case.checksum(Perf::checksumOf(forces));
  a_case.indexChecksum(indices.value());
}

template <class T>
void
addCases(Perf::Suite& a_suite)
{
  const std::string prefix = "SphereContacts/polydisperse";
  const std::string prec   = Perf::precisionName<T>();

  const auto cloud = [](Perf::Case& c) {
    const std::size_t n = c.size(CloudSpheres, SmokeCloudSpheres);

    c.param("spheres", n);
    c.param("K", K);
    c.param("W", W);
    c.param("targetLeafSize", TargetLeafSize);
    c.param("gap", Gap);

    return spheres<T>(n);
  };

  // The particles in Morton order of their centers, as a particle code keeps them.
  const auto order = [](const std::vector<Sphere<T>>& a_spheres) {
    std::vector<Vec3<T>> centers(a_spheres.size());

    for (std::size_t i = 0; i < a_spheres.size(); i++) {
      centers[i] = a_spheres[i].m_center;
    }

    return Perf::mortonOrder(centers);
  };

  a_suite.add(Perf::caseName({prefix, "force-morton", prec, "host"}), [=](Perf::Case& c) {
    const auto               s = cloud(c);
    const auto               q = order(s);
    EBGeometry::Pool         pool(EBGeometry::hostMemoryResource());
    std::vector<Contacts<T>> results;

    Perf::measureHostQuery(c, ContactForce<T>{BVH<T>(pool, s, TargetLeafSize), T(Gap)}, q, results);
    c.poolBytes(pool.usedBytes());
    record(c, results);
  });

  a_suite.add(Perf::caseName({prefix, "force-morton", prec, "hostdevice"}), [=](Perf::Case& c) {
    const auto       s = cloud(c);
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
    const BVH<T>     bvh(pool, s, TargetLeafSize);

    c.poolBytes(pool.usedBytes());

    const auto results = Perf::measureHostAndDevice<Contacts<T>>(
      c,
      pool,
      order(s),
      [&](const EBGeometry::Pool& a_pool) { return ContactForce<T>{bvh.rebasedView(a_pool), T(Gap)}; },
      agreeContacts<T>);

    record(c, results);
  });
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "SphereContacts");

  addCases<float>(suite);
  addCases<double>(suite);

  return suite.run();
}
