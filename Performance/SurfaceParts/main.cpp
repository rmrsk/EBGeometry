// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for the parts of a tape's surfaces and their labels: closestSurface() on a union of boxes
// (drilled into the union, each box reporting its face) and on a mesh whose faces are labeled by group (each
// point's face found by a second search of the mesh), and firstSurface() along rays at that mesh, in float and
// double. Each composition's closestSurface() case has a closestLeaf() case, and its firstSurface() case a
// firstHit() case, that names it (Case::relativeTo()), so that a comparison notes what the parts cost. A host
// build runs the host cases; a CUDA or HIP build runs them as host/device pairs, which agree when they find the
// same value (two parts whose values tie may differ). In a probe of its own, so that adding it changes no other
// probe's translation unit (see Performance/Fused). See ../README.md and PerformanceTesting.rst.

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// The BVHs' branching factor and the mesh's SIMD width.
constexpr std::size_t K = 4;
constexpr std::size_t W = 4;

// Boxes in the union, queries per case, at full and at smoke size; the mesh cases' queries traverse it twice.
constexpr std::size_t Boxes           = 4096;
constexpr std::size_t SmokeBoxes      = 64;
constexpr std::size_t HostQueries     = 200000;
constexpr std::size_t PairQueries     = 1048576;
constexpr std::size_t MeshHostQueries = 20000;
constexpr std::size_t MeshPairQueries = 262144;
constexpr std::size_t SmokeQueries    = 1000;

// The mesh's faces are put in this many groups, face id modulo the count.
constexpr uint32_t MeshGroups = 8;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

template <class T>
using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

template <class T>
using Boxes3 = EBGeometry::BVHUnion<T, K, EBGeometry::BoxSDF<T>>;

template <class T>
using Mesh = EBGeometry::TriMeshSDF<T, K, W>;

// A query's answer: the surface, part and label, folded into one number, and a value.
template <class T>
struct PartHit
{
  uint32_t m_code;
  T        m_value;
};

// The operations: the drilled query, and the one that stops at the leaf (or the plain ray hit).
enum class Operation
{
  ClosestSurface,
  ClosestLeaf,
  FirstSurface,
  FirstHit
};

// What an operation is called in a case's name.
inline std::string
operationName(const Operation a_operation)
{
  switch (a_operation) {
  case Operation::ClosestSurface: {
    return "closestSurface";
  }
  case Operation::ClosestLeaf: {
    return "closestLeaf";
  }
  case Operation::FirstSurface: {
    return "firstSurface";
  }
  default: {
    return "firstHit";
  }
  }
}

// A point query at a point, with the tape's value there.
template <class T, class TapeType, bool Drill>
struct PointQuery
{
  TapeType m_tape;

  EBGEOMETRY_HOST_DEVICE
  PartHit<T>
  operator()(const Vec3<T>& a_point) const noexcept
  {
    if constexpr (Drill) {
      const EBGeometry::SurfaceRef<T> s = m_tape.closestSurface(a_point);

      return PartHit<T>{s.m_surface + 7U * s.m_part + 131U * s.m_surfaceLabel.value(), m_tape.signedDistance(a_point)};
    }
    else {
      return PartHit<T>{m_tape.closestLeaf(a_point).m_leaf, m_tape.signedDistance(a_point)};
    }
  }
};

// A ray query along a ray: the crossing's distance, with the surface or the leaf.
template <class T, class TapeType, bool Drill>
struct RayQuery
{
  TapeType m_tape;
  Vec3<T>  m_target;

  EBGEOMETRY_HOST_DEVICE
  PartHit<T>
  operator()(const Vec3<T>& a_origin) const noexcept
  {
    const Vec3<T>             d = m_target - a_origin;
    const EBGeometry::RayT<T> ray{a_origin, d / d.length()};

    if constexpr (Drill) {
      const EBGeometry::SurfaceHitT<T> h = m_tape.firstSurface(ray, T(0), T(1e3));

      return PartHit<T>{h.m_hit.hit() ? h.m_surface.m_part + 131U * h.m_surface.m_surfaceLabel.value() : 0U,
                        h.m_hit.m_t};
    }
    else {
      const EBGeometry::HitT<T> h = m_tape.firstHit(ray, T(0), T(1e3));

      return PartHit<T>{h.hit() ? h.m_primitive : 0U, h.m_t};
    }
  }
};

// The sum of the codes and the finite values.
template <class T>
double
checksumOf(const std::vector<PartHit<T>>& a_hits)
{
  double sum = 0.0;

  for (const auto& h : a_hits) {
    sum += static_cast<double>(h.m_code) + (std::isfinite(h.m_value) ? static_cast<double>(h.m_value) : 0.0);
  }

  return sum;
}

// A host and a device answer agree when their values are within Perf::agreementTolerance().
template <class T>
std::pair<double, bool>
agreeValue(const PartHit<T>& a_host, const PartHit<T>& a_device)
{
  if (!std::isfinite(a_host.m_value) || !std::isfinite(a_device.m_value)) {
    return {0.0, std::isfinite(a_host.m_value) != std::isfinite(a_device.m_value)};
  }

  const double vh   = static_cast<double>(a_host.m_value);
  const double diff = std::abs(vh - static_cast<double>(a_device.m_value));

  return {diff, diff > Perf::agreementTolerance<T>(std::abs(vh))};
}

// Random points in a box grown by a quarter of its size on every side.
template <class T>
std::vector<Vec3<T>>
pointsAround(const AABB<T>& a_box, const std::size_t a_n)
{
  const Vec3<T> grow = T(0.25) * (a_box.getHighCorner() - a_box.getLowCorner());

  return Perf::pointsInBox<T>(
    AABB<T>(a_box.getLowCorner() - grow, a_box.getHighCorner() + grow), a_n, Perf::Seed::TapeQueries);
}

// The cases of one operation on one composition, on the host and as a host/device pair. a_build(case, pool)
// returns the tape and the box its points are drawn around; a ray query aims at the box's center.
template <class T, Operation Op, class Build>
void
addOperation(Perf::Suite&       a_suite,
             const std::string& a_input,
             const std::size_t  a_host,
             const std::size_t  a_pair,
             const std::string& a_relativeTo,
             const std::string& a_ratio,
             Build              a_build)
{
  using TapeType = typename std::invoke_result_t<Build, Perf::Case&, EBGeometry::Pool&>::first_type;

  constexpr bool drill = (Op == Operation::ClosestSurface || Op == Operation::FirstSurface);
  constexpr bool ray   = (Op == Operation::FirstSurface || Op == Operation::FirstHit);

  using Query = std::conditional_t<ray, RayQuery<T, TapeType, drill>, PointQuery<T, TapeType, drill>>;

  const std::string prec      = Perf::precisionName<T>();
  const std::string operation = operationName(Op);

  const auto makeQuery = [](const TapeType& a_tape, const AABB<T>& a_box) {
    if constexpr (ray) {
      return Query{a_tape, T(0.5) * (a_box.getLowCorner() + a_box.getHighCorner())};
    }
    else {
      return Query{a_tape};
    }
  };

  a_suite.add(Perf::caseName({"SurfaceParts", a_input, operation, prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const auto made   = a_build(c, pool);
    const auto points = pointsAround(made.second, c.size(a_host, SmokeQueries));

    std::vector<PartHit<T>> hits;

    Perf::measureHostQuery(c, makeQuery(made.first, made.second), points, hits);

    c.checksum(checksumOf(hits));
    c.poolBytes(pool.usedBytes());

    if (!a_relativeTo.empty()) {
      c.relativeTo(Perf::caseName({"SurfaceParts", a_input, a_relativeTo, prec, "host"}), a_ratio);
    }
  });

  a_suite.add(Perf::caseName({"SurfaceParts", a_input, operation, prec, "hostdevice"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const auto made   = a_build(c, pool);
    const auto points = pointsAround(made.second, c.size(a_pair, SmokeQueries));

    const auto hits = Perf::measureHostAndDevice<PartHit<T>>(
      c,
      pool,
      points,
      [&](const EBGeometry::Pool& a_pool) { return makeQuery(made.first.rebasedView(a_pool), made.second); },
      agreeValue<T>);

    c.checksum(checksumOf(hits));

    if (!a_relativeTo.empty()) {
      c.relativeTo(Perf::caseName({"SurfaceParts", a_input, a_relativeTo, prec, "hostdevice"}), a_ratio);
    }
  });
}

template <class T>
void
addPrecision(Perf::Suite& a_suite)
{
  // A union of boxes of random sizes at random places in the unit cube: closestSurface() drills into the union and
  // reads the box's face.
  const auto boxes = [](Perf::Case& c, EBGeometry::Pool& a_pool) {
    const std::size_t n       = c.size(Boxes, SmokeBoxes);
    const T           scale   = T(std::cbrt(double(Boxes) / double(n)));
    const auto        centers = Perf::pointsInBox<T>(Perf::cube(T(0), T(1)), n, Perf::Seed::Cloud);
    const auto        sizes   = Perf::pointsInBox<T>(Perf::cube(T(0.002), T(0.008)), n, Perf::Seed::SphereRadii);

    std::vector<EBGeometry::BoxSDF<T>> shapes;

    for (std::size_t i = 0; i < n; i++) {
      shapes.emplace_back(centers[i] - scale * sizes[i], centers[i] + scale * sizes[i]);
    }

    const Boxes3<T> scene(a_pool, shapes);

    c.param("boxes", n);
    c.param("K", K);

    return std::make_pair(EBGeometry::Tape<T, Boxes3<T>>(a_pool, EBGeometry::Expr<T>(scene)),
                          AABB<T>(Vec3<T>::zeros(), Vec3<T>::ones()));
  };

  addOperation<T, Operation::ClosestSurface>(a_suite, "boxes", HostQueries, PairQueries, "", "", boxes);
  addOperation<T, Operation::ClosestLeaf>(
    a_suite, "boxes", HostQueries, PairQueries, "closestSurface", "surface/leaf", boxes);

  // The armadillo (the dodecahedron under --smoke) drilled through along x, its faces labeled in MeshGroups
  // groups: closestSurface() finds each point's face with a second search of the mesh.
  const auto mesh = [](Perf::Case& c, EBGeometry::Pool& a_pool) {
    const Mesh<T> shape = EBGeometry::Parser::readIntoTriMeshSDF<T, K, W>(Perf::meshFile(c.smoke()), a_pool);
    const AABB<T> box   = shape.computeBoundingVolume();
    const Vec3<T> size  = box.getHighCorner() - box.getLowCorner();

    EBGeometry::FaceGroups groups;

    for (uint32_t g = 0; g < MeshGroups; g++) {
      groups.m_names.push_back("group" + std::to_string(g));
    }

    const uint32_t faces = shape.getNumParts();

    for (uint32_t f = 0; f < faces; f++) {
      groups.m_group.push_back(f % MeshGroups);
    }

    const EBGeometry::InfiniteCylinderSDF<T> drill(
      T(0.5) * (box.getLowCorner() + box.getHighCorner()), T(0.12) * size[1], 0);

    c.param("K", K);
    c.param("W", W);
    c.param("groups", MeshGroups);

    return std::make_pair(
      EBGeometry::Tape<T, Mesh<T>>(a_pool, EBGeometry::Difference(EBGeometry::SurfaceLabel(shape, groups), drill)),
      box);
  };

  addOperation<T, Operation::ClosestSurface>(a_suite, "mesh-groups", MeshHostQueries, MeshPairQueries, "", "", mesh);
  addOperation<T, Operation::ClosestLeaf>(
    a_suite, "mesh-groups", MeshHostQueries, MeshPairQueries, "closestSurface", "surface/leaf", mesh);
  addOperation<T, Operation::FirstSurface>(a_suite, "mesh-groups", MeshHostQueries, MeshPairQueries, "", "", mesh);
  addOperation<T, Operation::FirstHit>(
    a_suite, "mesh-groups", MeshHostQueries, MeshPairQueries, "firstSurface", "surface/hit", mesh);
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "SurfaceParts");

  addPrecision<float>(suite);
  addPrecision<double>(suite);

  return suite.run();
}
