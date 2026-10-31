// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for ray tracing a composition from its operands' spans (EBGeometry_RaySpans.hpp): a
// scene of 25 balls and 25 boxes, less 10 balls, fused and through a tape (whose union of 50 is a BVH
// union), traced and marched (RayOptions::m_march), each traced case naming its marched one; and a crowd of
// 4096 small balls through a tape, one BVH union, traced only (a hit looks up its ball among the union's
// operands, which must not cost a pass over all of them). Every ray
// starts at a random point in the scene's box grown to twice its size and points at the box's center. A
// host build runs the host cases; a CUDA or HIP build runs host/device pairs in Morton and random order.
// See ../README.md and PerformanceTesting.rst.

#include <cmath>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// The scene's solids: balls and boxes in the union, and the balls cut out of it.
constexpr std::size_t NumBalls = 25;
constexpr std::size_t NumBoxes = 25;
constexpr std::size_t NumHoles = 10;

// The crowd's balls, at full and at smoke size.
constexpr std::size_t CrowdBalls      = 4096;
constexpr std::size_t SmokeCrowdBalls = 64;

// The numbers of rays, at full and at smoke size.
constexpr std::size_t HostQueries  = 50000;
constexpr std::size_t PairQueries  = 262144;
constexpr std::size_t SmokeQueries = 1000;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

template <class T>
using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

// The scene's solids, from random points: a center in [-1, 1]^3 and a size in [0, 1]^3 each.
template <class T>
struct Scene
{
  std::vector<EBGeometry::SphereSDF<T>> balls;
  std::vector<EBGeometry::BoxSDF<T>>    boxes;
  std::vector<EBGeometry::SphereSDF<T>> holes;

  Scene()
  {
    const std::size_t n = NumBalls + NumBoxes + NumHoles;

    const auto centers = Perf::pointsInBox<T>(AABB<T>(-Vec3<T>::ones(), Vec3<T>::ones()), n, Perf::Seed::RayScene);
    const auto sizes   = Perf::pointsInBox<T>(AABB<T>(Vec3<T>::zeros(), Vec3<T>::ones()), n, Perf::Seed::RayScene + 1);

    for (std::size_t i = 0; i < n; i++) {
      if (i < NumBalls) {
        balls.emplace_back(centers[i], T(0.1) + T(0.15) * sizes[i][0]);
      }
      else if (i < NumBalls + NumBoxes) {
        const Vec3<T> half = T(0.08) * Vec3<T>::ones() + T(0.12) * sizes[i];

        boxes.emplace_back(centers[i] - half, centers[i] + half);
      }
      else {
        holes.emplace_back(centers[i], T(0.15) + T(0.15) * sizes[i][0]);
      }
    }
  }

  // The scene as an expression, for a tape.
  EBGeometry::Expr<T>
  expr() const
  {
    std::vector<EBGeometry::Expr<T>> solids;
    std::vector<EBGeometry::Expr<T>> cuts;

    solids.insert(solids.end(), balls.begin(), balls.end());
    solids.insert(solids.end(), boxes.begin(), boxes.end());
    cuts.insert(cuts.end(), holes.begin(), holes.end());

    return EBGeometry::Difference(EBGeometry::Union(solids), EBGeometry::Union(cuts));
  }

  // The scene fused.
  auto
  fused() const
  {
    return this->fuse(
      std::make_index_sequence<NumBalls>{}, std::make_index_sequence<NumBoxes>{}, std::make_index_sequence<NumHoles>{});
  }

private:
  template <std::size_t... Is, std::size_t... Js, std::size_t... Ks>
  auto
  fuse(std::index_sequence<Is...>, std::index_sequence<Js...>, std::index_sequence<Ks...>) const
  {
    return EBGeometry::Fused::Difference(EBGeometry::Fused::Union(balls[Is]..., boxes[Js]...),
                                         EBGeometry::Fused::Union(holes[Ks]...));
  }
};

// The crowd: balls with centers in [-1, 1]^3 and radii in [0.02, 0.04], as one union.
template <class T>
EBGeometry::Expr<T>
crowd(const std::size_t a_n)
{
  const auto centers = Perf::pointsInBox<T>(AABB<T>(-Vec3<T>::ones(), Vec3<T>::ones()), a_n, Perf::Seed::RayCrowd);
  const auto sizes   = Perf::pointsInBox<T>(AABB<T>(Vec3<T>::zeros(), Vec3<T>::ones()), a_n, Perf::Seed::RayCrowd + 1);

  std::vector<EBGeometry::Expr<T>> balls;

  for (std::size_t i = 0; i < a_n; i++) {
    balls.emplace_back(EBGeometry::SphereSDF<T>(centers[i], T(0.02) + T(0.02) * sizes[i][0]));
  }

  return EBGeometry::Union(balls);
}

// The ray from a_origin towards a_target.
template <class T>
EBGEOMETRY_HOST_DEVICE
EBGeometry::RayT<T>
rayTowards(const Vec3<T>& a_origin, const Vec3<T>& a_target) noexcept
{
  const Vec3<T> d = a_target - a_origin;

  return EBGeometry::RayT<T>{a_origin, d / d.length()};
}

// Where the ray from a point towards the target first crosses the shape (-1 for no crossing), traced, or
// marched if m_march.
template <class T, class Shape>
struct FirstHit
{
  Shape   m_shape;
  Vec3<T> m_target;
  bool    m_march;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3<T>& a_origin) const noexcept
  {
    EBGeometry::RayOptions<T> options;

    options.m_march = m_march;

    const EBGeometry::HitT<T> hit = EBGeometry::firstHit(
      m_shape, rayTowards(a_origin, m_target), T(0), EBGeometry::Math::Limits<T>::infinity(), options);

    return hit.hit() ? hit.m_t : -T(1);
  }
};

// Whether a host and a device crossing agree: both miss, or both cross within a tolerance, the march's for
// marched rays (twice RayOptions::m_tolerance times the scene's box diagonal, under 1e-3 here) and a
// rounding error's for traced ones. A ray that only grazes the surface agrees whatever the two sides answer:
// where it touches the surface without going deeper than the tolerance (the march's for a marched ray,
// Perf::agreementTolerance() at the scene's scale for a traced one), one side's arithmetic may round to a
// crossing and the other's to a miss (a fused multiply-add on the device, say), and both are valid answers
// to the precision the query promises.
template <class T, class Shape>
struct AgreeCrossing
{
  Shape   m_shape;
  Vec3<T> m_target;
  bool    m_march;

  std::pair<double, bool>
  operator()(T a_host, T a_device, const Vec3<T>& a_origin) const
  {
    const double diff     = std::abs(static_cast<double>(a_host) - static_cast<double>(a_device));
    const bool   mismatch = (a_host < T(0)) != (a_device < T(0)) || diff > 1e-3;

    return {diff, mismatch && !this->grazes(a_origin, a_host, a_device)};
  }

private:
  // Whether two answers, of which at least one is a crossing, differ only by a graze: the earlier crossing
  // is where the ray touches the surface (within the tolerance of it), and the ray goes no deeper than the
  // tolerance inside before the later answer (the other crossing, or, for a miss, the end of the ray's way
  // through the scene). Swept in steps of the shape's distance, which cannot step over a solid, and at
  // least the tolerance.
  bool
  grazes(const Vec3<T>& a_origin, T a_host, T a_device) const
  {
    const auto   box   = m_shape.computeBoundingVolume();
    const double scale = static_cast<double>((box.getHighCorner() - box.getLowCorner()).length());
    const double tol   = m_march ? 2.0 * static_cast<double>(EBGeometry::RayOptions<T>{}.m_tolerance) * scale
                                 : Perf::agreementTolerance<T>(scale);

    const bool hostHits   = a_host >= T(0);
    const bool deviceHits = a_device >= T(0);

    const EBGeometry::RayT<T> ray = rayTowards(a_origin, m_target);

    const auto value = [&](double t) { return static_cast<double>(m_shape.signedDistance(ray.at(static_cast<T>(t)))); };

    const double far = static_cast<double>((m_target - a_origin).length()) + scale;
    const double first =
      (hostHits && deviceHits) ? std::min<double>(a_host, a_device) : static_cast<double>(hostHits ? a_host : a_device);
    const double last = (hostHits && deviceHits) ? std::max<double>(a_host, a_device) : far;

    if (std::abs(value(first)) > tol) {
      return false;
    }

    for (double t = 0.0; t < last - tol;) {
      const double v = value(t);

      if (v < -tol) {
        return false;
      }

      t += std::max(std::abs(v), tol);
    }

    return true;
  }
};

// The scene's box, its center, and ray origins in the box grown to twice its size about the center.
template <class T>
std::vector<Vec3<T>>
origins(const AABB<T>& a_box, std::size_t a_n)
{
  const Vec3<T> center = T(0.5) * (a_box.getLowCorner() + a_box.getHighCorner());
  const Vec3<T> half   = a_box.getHighCorner() - center;

  return Perf::pointsInBox<T>(AABB<T>(center - T(2) * half, center + T(2) * half), a_n, Perf::Seed::RayOrigins);
}

// One shape, traced or marched, in one precision: the host case, and host/device pairs in each order.
// a_build(case, pool) builds the shape in the pool. A traced case names its marched one, if a_marched says
// there is one.
template <class T, class Build>
void
addShape(
  Perf::Suite& a_suite, const std::string& a_input, const bool a_march, Build a_build, const bool a_marched = true)
{
  const std::string prec      = Perf::precisionName<T>();
  const std::string operation = a_march ? "firstHit-marched" : "firstHit";

  a_suite.add(Perf::caseName({"RayCSG", a_input, operation, prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const auto     shape  = a_build(c, pool);
    const auto     box    = shape.computeBoundingVolume();
    const Vec3<T>  center = T(0.5) * (box.getLowCorner() + box.getHighCorner());
    const auto     q      = origins<T>(box, c.size(HostQueries, SmokeQueries));
    std::vector<T> out;

    Perf::measureHostQuery(c, FirstHit<T, std::decay_t<decltype(shape)>>{shape, center, a_march}, q, out);

    c.checksum(Perf::checksumOf(out));
    c.poolBytes(pool.usedBytes());
    c.param("march", a_march ? 1 : 0);

    if (!a_march && a_marched) {
      c.relativeTo(Perf::caseName({"RayCSG", a_input, "firstHit-marched", prec, "host"}), "marched/traced");
    }
  });

  for (const Perf::Order order : Perf::Orders) {
    const std::string ordered = operation + "-" + Perf::orderName(order);

    a_suite.add(Perf::caseName({"RayCSG", a_input, ordered, prec, "hostdevice"}), [=](Perf::Case& c) {
      EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

      const auto    shape  = a_build(c, pool);
      const auto    box    = shape.computeBoundingVolume();
      const Vec3<T> center = T(0.5) * (box.getLowCorner() + box.getHighCorner());
      const auto    q      = Perf::inOrder(order, origins<T>(box, c.size(PairQueries, SmokeQueries)));

      using S = std::decay_t<decltype(shape)>;

      const auto out = Perf::measureHostAndDevice<T>(
        c,
        pool,
        q,
        [&](const EBGeometry::Pool& a_pool) { return FirstHit<T, S>{shape.rebasedView(a_pool), center, a_march}; },
        AgreeCrossing<T, S>{shape, center, a_march});

      c.checksum(Perf::checksumOf(out));
      c.param("march", a_march ? 1 : 0);

      if (!a_march && a_marched) {
        c.relativeTo(
          Perf::caseName(
            {"RayCSG", a_input, std::string("firstHit-marched-") + Perf::orderName(order), prec, "hostdevice"}),
          "marched/traced");
      }
    });
  }
}

template <class T>
void
addPrecision(Perf::Suite& a_suite)
{
  const auto fused = [](Perf::Case&, EBGeometry::Pool&) { return Scene<T>().fused(); };

  // The union of 50 is a BVH union in the tape (TapeOptions::m_bvhUnionThreshold).
  const auto tape = [](Perf::Case& c, EBGeometry::Pool& a_pool) {
    const EBGeometry::Tape<T> t(a_pool, Scene<T>().expr());

    c.param("bvhUnions", t.getNumBVHUnions());

    return t;
  };

  addShape<T>(a_suite, "scene-fused", false, fused);
  addShape<T>(a_suite, "scene-fused", true, fused);
  addShape<T>(a_suite, "scene-tape", false, tape);
  addShape<T>(a_suite, "scene-tape", true, tape);

  const auto crowdTape = [](Perf::Case& c, EBGeometry::Pool& a_pool) {
    const std::size_t         n = c.size(CrowdBalls, SmokeCrowdBalls);
    const EBGeometry::Tape<T> t(a_pool, crowd<T>(n));

    c.param("balls", n);
    c.param("bvhUnions", t.getNumBVHUnions());

    return t;
  };

  addShape<T>(a_suite, "crowd-tape", false, crowdTape, false);
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "RayCSG");

  addPrecision<float>(suite);
  addPrecision<double>(suite);

  return suite.run();
}
