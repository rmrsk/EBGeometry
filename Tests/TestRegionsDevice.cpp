// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// The device tests of regions (TestRegions.cpp): a tape's region queries, and a Domain (with its tape) taken whole by
// a kernel, against the host. They are apart from TestRegions.cpp so that a GPU build compiles only them with the
// device compiler.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

#include <cstdint>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>

using namespace EBGeometry;

namespace {

// A cable: a copper core inside a PTFE jacket; the outside, region 0, has no material.
template <class T>
Expr<T>
cable()
{
  const auto core   = MaterialLabel(SphereSDF<T>(Vec3T<T>::zeros(), T(0.5)), "copper");
  const auto jacket = MaterialLabel(SphereSDF<T>(Vec3T<T>::zeros(), T(1)), "PTFE");

  return Overlay(jacket, core);
}

// What the device answers at a point: the region, the three regions' functions, the jacket's surface there with
// its sides, and the first change of region along +x.
template <class T>
struct RegionAnswer
{
  uint32_t m_region;
  T        m_values[3];
  T        m_visited[3]; // the same, from regionDistances()
  uint32_t m_minus;
  uint32_t m_plus;
  uint32_t m_surface;
  uint32_t m_group;
  T        m_t;
  uint32_t m_from;
  uint32_t m_to;

  // Per subdomain of the domain: its function, its surface's role and boundary condition there.
  T           m_subdomainValues[2];
  SurfaceRole m_roles[2];
  uint32_t    m_conditions[2];

  // The domain's subdomain there, and every subdomain's function (the two subdomains', then the excluded regions').
  uint32_t m_subdomain;
  T        m_functions[3];
};

// Device functor: the region queries of a tape at a point, and what a downstream code computing on a domain asks
// there, the domain taken whole.
template <class T, class TapeType>
struct RegionQuery
{
  TapeType  m_tape;
  Domain<T> m_domain;

  EBGEOMETRY_HOST_DEVICE
  RegionAnswer<T>
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    RegionAnswer<T> answer;

    answer.m_region = m_tape.region(a_point);

    for (uint32_t k = 0; k < 3; k++) {
      answer.m_values[k] = m_tape.signedDistance(a_point, k);
    }

    m_tape.regionDistances(a_point, [&](const uint32_t a_region, const T a_value) {
      if (a_region < 3U) {
        answer.m_visited[a_region] = a_value;
      }
    });

    const SurfaceRef<T> surface = m_tape.closestSurface(a_point, 1U);

    answer.m_minus   = surface.m_ownerRegion;
    answer.m_plus    = surface.m_neighborRegion;
    answer.m_surface = surface.m_surface;
    answer.m_group   = m_domain.surfaceGroupOf(surface);

    for (uint32_t d = 0; d < 2; d++) {
      const SurfaceRef<T> s = m_domain.closestSurface(a_point, d);

      answer.m_subdomainValues[d] = m_domain.signedDistance(a_point, d);
      answer.m_roles[d]           = m_domain.role(s);
      answer.m_conditions[d]      = m_domain.surfaceGroupOf(s);
    }

    T values[3];

    m_domain.signedDistances(a_point, values);

    answer.m_subdomain = m_domain.subdomainAt(a_point);

    for (uint32_t l = 0; l < 3; l++) {
      answer.m_functions[l] = values[l];
    }

    const SurfaceHitT<T> change = m_tape.firstRegionChange(RayT<T>{a_point, Vec3T<T>(T(1), T(0), T(0))}, T(0), T(4));

    answer.m_t    = change.m_hit.hit() ? change.m_hit.m_t : T(-1);
    answer.m_from = change.m_surface.m_ownerRegion;
    answer.m_to   = change.m_surface.m_neighborRegion;

    return answer;
  }
};

// Device functor: the surface group of a surface between a region and the outside, from surface groups taken whole,
// from a view rebuilt from their layout and location, and from default-constructed (empty) ones. A CUDA build compiles
// each with warnings as errors.
struct GroupAnswer
{
  uint32_t m_whole;
  uint32_t m_rebuilt;
  uint32_t m_empty;
};

template <class T>
struct GroupQuery
{
  SurfaceGroups m_groups;

  EBGEOMETRY_HOST_DEVICE
  GroupAnswer
  operator()(const uint32_t a_region) const noexcept
  {
    SurfaceRef<T> surface;

    surface.m_ownerRegion    = a_region;
    surface.m_neighborRegion = 0U;

    const SurfaceGroups rebuilt(m_groups.layout(), m_groups.location());
    const SurfaceGroups empty;

    return GroupAnswer{
      m_groups.surfaceGroupOf(surface), rebuilt.surfaceGroupOf(surface), empty.surfaceGroupOf(surface)};
  }
};

} // namespace

TEMPLATE_TEST_CASE("SurfaceGroups: the groups on a device, whole, rebuilt from a layout or empty, match the host",
                   "[SurfaceGroups][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  Pool pool(hostMemoryResource());

  // Region 2's surfaces in group 0, region 1's in group 1, the rest in group 2.
  const SurfaceGroups groups(
    pool, {SurfaceSelector::region(2), SurfaceSelector::region(1), SurfaceSelector::rest()}, {0U, 1U, 2U}, 3U);

  pool.freeze();

  const Pool device = Pool::mirror(pool, deviceTestResource());

  const std::vector<uint32_t> regions{1U, 2U, 3U, 4U};

  const auto onDevice = evaluateOnDevice<GroupAnswer>(GroupQuery<T>{groups.rebasedView(device)}, regions);
  const auto onHost   = evaluateOnHost<GroupAnswer>(GroupQuery<T>{groups}, regions);

  const uint32_t expected[4] = {1U, 0U, 2U, 2U};

  for (size_t i = 0; i < regions.size(); i++) {
    INFO("region " << regions[i]);

    for (const GroupAnswer& a : {onDevice[i], onHost[i]}) {
      CHECK(a.m_whole == expected[i]);
      CHECK(a.m_rebuilt == expected[i]);
      CHECK(a.m_empty == SurfaceGroups::None);
    }
  }
}

TEMPLATE_TEST_CASE("Tape: the region queries on a device match the host, and a domain taken whole by a kernel",
                   "[Overlay][Tape][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Tape = Tape<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  Pool pool(hostMemoryResource());

  // The cable cut by a slab (x > 0, |z| < 0.3).
  const Tape tape(pool, Difference(cable<T>(), BoxSDF<T>(Vec3(T(0), T(-2), T(-0.3)), Vec3(T(2), T(2), T(0.3)))));

  // The electrostatic domain: the air and the jacket, two subdomains; every conductor surface dirichlet, the
  // jacket's outside a jump. Built in the tape's pool before it is mirrored.
  DomainSetup setup;

  (void)setup.addSubdomain({0});
  (void)setup.addSubdomain({1});

  const uint32_t dirichlet = setup.addSurfaceGroup();
  const uint32_t jump      = setup.addSurfaceGroup();

  setup.add(dirichlet, SurfaceSelector::region(2));
  setup.add(jump, SurfaceSelector::interface(0, 1));

  const Domain domain(pool, tape, setup);

  pool.freeze();

  const Pool device = Pool::mirror(pool, deviceTestResource());

  std::vector<Vec3> points;

  for (int i = 0; i < 24; i++) {
    for (int j = 0; j < 6; j++) {
      points.emplace_back(T(0.1) * T(i) - T(1.2) + T(0.013), T(0.007), T(0.15) * T(j) - T(0.4));
    }
  }

  const auto onDevice = evaluateOnDevice<RegionAnswer<T>>(
    RegionQuery<T, Tape>{tape.rebasedView(device), domain.rebasedView(device)}, points);
  const auto onHost = evaluateOnHost<RegionAnswer<T>>(RegionQuery<T, Tape>{tape, domain}, points);

  size_t differ = 0;

  for (size_t i = 0; i < points.size(); i++) {
    INFO("point " << points[i]);

    const RegionAnswer<T>& d = onDevice[i];
    const RegionAnswer<T>& h = onHost[i];

    // Where two choices are equally right (a point on a surface), the device's rounding may pick the other.
    differ += (d.m_region != h.m_region || d.m_minus != h.m_minus || d.m_plus != h.m_plus ||
               d.m_surface != h.m_surface || d.m_group != h.m_group || d.m_from != h.m_from || d.m_to != h.m_to ||
               d.m_roles[0] != h.m_roles[0] || d.m_roles[1] != h.m_roles[1] || d.m_conditions[0] != h.m_conditions[0] ||
               d.m_conditions[1] != h.m_conditions[1] || d.m_subdomain != h.m_subdomain)
                ? 1U
                : 0U;

    for (uint32_t k = 0; k < 3; k++) {
      CHECK_THAT(d.m_values[k], withinAbsT<T>(h.m_values[k], looseMargin<T>()));

      // In one pass, the same values as region by region, on each side.
      CHECK(d.m_visited[k] == d.m_values[k]);
      CHECK(h.m_visited[k] == h.m_values[k]);
    }

    for (uint32_t k = 0; k < 2; k++) {
      CHECK_THAT(d.m_subdomainValues[k], withinAbsT<T>(h.m_subdomainValues[k], looseMargin<T>()));
    }

    for (uint32_t l = 0; l < 3; l++) {
      CHECK_THAT(d.m_functions[l], withinAbsT<T>(h.m_functions[l], looseMargin<T>()));
    }

    CHECK_THAT(d.m_t, withinAbsT<T>(h.m_t, looseMargin<T>()));
  }

  CHECK(differ <= 2U);

  // The jacket's subdomain meets the air (an interface: two subdomains) and the core (a boundary: the copper is in no
  // subdomain), with the jump and the dirichlet conditions.
  bool interface = false;
  bool boundary  = false;

  for (const RegionAnswer<T>& h : onHost) {
    interface = interface || (h.m_roles[1] == SurfaceRole::Interface && h.m_conditions[1] == jump);
    boundary  = boundary || (h.m_roles[1] == SurfaceRole::Boundary && h.m_conditions[1] == dirichlet);
  }

  CHECK(interface);
  CHECK(boundary);
}

namespace {

// What the device answers near a welded contact: the region, the three regions' functions, each cube's nearest
// surface with the region across it, and the first change of region along +x.
template <class T>
struct WeldAnswer
{
  uint32_t m_region;
  T        m_values[3];
  uint32_t m_leaf[2];
  uint32_t m_across[2];
  uint32_t m_from;
  uint32_t m_to;
  uint32_t m_leafCrossed;
};

template <class T, class TapeType>
struct WeldQuery
{
  TapeType m_tape;

  EBGEOMETRY_HOST_DEVICE
  WeldAnswer<T>
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    WeldAnswer<T> answer;

    answer.m_region = m_tape.region(a_point);

    for (uint32_t k = 0; k < 3; k++) {
      answer.m_values[k] = m_tape.signedDistance(a_point, k);
    }

    for (uint32_t k = 0; k < 2; k++) {
      const SurfaceRef<T> surface = m_tape.closestSurface(a_point, k + 1U);

      answer.m_leaf[k]   = surface.m_leaf;
      answer.m_across[k] = surface.m_neighborRegion;
    }

    const SurfaceHitT<T> change = m_tape.firstRegionChange(RayT<T>{a_point, Vec3T<T>(T(1), T(0), T(0))}, T(0), T(4));

    answer.m_from        = change.m_surface.m_ownerRegion;
    answer.m_to          = change.m_surface.m_neighborRegion;
    answer.m_leafCrossed = change.m_surface.m_leaf;

    return answer;
  }
};

} // namespace

TEMPLATE_TEST_CASE("Tape: regions welded across a gap answer the same on a device as on the host",
                   "[Overlay][Tape][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Tape = EBGeometry::Tape<T, TapeFeature::Weld>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  // Two unit cubes along x, a gap of 1e-4 of the diagonal apart, welded at a tolerance of 1e-3 of it.
  const T gap = T(1e-4) * std::sqrt(T(6));

  TapeOptions options;

  options.m_interfaceTolerance = 1e-3;

  Pool pool(hostMemoryResource());

  const Tape tape(pool,
                  Overlay(BoxSDF<T>(Vec3::zeros(), Vec3::ones()),
                          BoxSDF<T>(Vec3(T(1) + gap, T(0), T(0)), Vec3(T(2) + gap, T(1), T(1)))),
                  options);

  pool.freeze();

  const Pool device = Pool::mirror(pool, deviceTestResource());

  // Points across the contact, the gap included, at a few heights.
  std::vector<Vec3> points;

  for (int i = -8; i <= 8; i++) {
    for (int j = 0; j < 3; j++) {
      points.emplace_back(T(1) + T(0.5) * gap + T(i) * T(0.25) * gap, T(0.3) + T(0.2) * T(j), T(0.45));
    }
  }

  const auto onDevice = evaluateOnDevice<WeldAnswer<T>>(WeldQuery<T, Tape>{tape.rebasedView(device)}, points);
  const auto onHost   = evaluateOnHost<WeldAnswer<T>>(WeldQuery<T, Tape>{tape}, points);

  size_t differ = 0;

  for (size_t i = 0; i < points.size(); i++) {
    INFO("point " << points[i]);

    const WeldAnswer<T>& d = onDevice[i];
    const WeldAnswer<T>& h = onHost[i];

    // The host: no point near the contact is in the outside, and the interface is the second cube's face.
    CHECK(h.m_region != 0U);
    CHECK(h.m_leaf[0] == 1U);
    CHECK(h.m_across[0] == 2U);

    // Where two choices are equally right (a point on a face), the device's rounding may pick the other.
    differ += (d.m_region != h.m_region || d.m_leaf[0] != h.m_leaf[0] || d.m_leaf[1] != h.m_leaf[1] ||
               d.m_across[0] != h.m_across[0] || d.m_across[1] != h.m_across[1] || d.m_from != h.m_from ||
               d.m_to != h.m_to || d.m_leafCrossed != h.m_leafCrossed)
                ? 1U
                : 0U;

    for (uint32_t k = 0; k < 3; k++) {
      CHECK_THAT(d.m_values[k], withinAbsT<T>(h.m_values[k], looseMargin<T>()));
    }
  }

  CHECK(differ <= 2U);
}
