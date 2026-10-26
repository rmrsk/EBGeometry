// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Walking a BVH with your own search: PackedBVH::traverse() over a scene of balls (a primitive type of
// this program's own), asked three questions. Which ball surface is nearest to a point (a point query),
// which ball a ray hits first (a ray query, with a visit callback that counts the nodes the walk enters),
// and whether the segment between two points is blocked (any hit, the walk ending at the first). The last
// part asks the first two on a GPU. See README.md.

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

#include <EBGeometry.hpp>
#include <Source/EBGeometry_Random.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T    = EBGEOMETRY_PRECISION;
using Vec3 = EBGeometry::Vec3T<T>;
using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

// The scene: numBalls balls of radius ballRadius at random points in the unit cube.
constexpr std::size_t   numBalls   = 20000;
constexpr T             ballRadius = T(0.005);
constexpr std::uint64_t ballSeed   = 20261025ULL;

// The BVH's branching factor: 4 children per node.
constexpr std::size_t K = 4;

namespace {

// The primitive: anything trivially copyable. The BVH stores it by value and hands it back to your leaf
// callback; it needs no member functions at all.
struct Ball
{
  Vec3 center;
  T    radius;
};

using BVH = EBGeometry::BVH::PackedBVH<T, Ball, K>;

// ── The code that runs on either side ────────────────────────────────────────────────────────────────
//
// A search is a query type, a state, a leaf callback and a bound. The walk (traverse()) is the library's;
// the rest is yours. Written as trivially copyable functors with EBGEOMETRY_HOST_DEVICE call operators,
// the same searches run on the host and in a kernel through GPU::transform().

// What a search for the nearest ball keeps: the smallest distance to a ball surface so far, and which
// ball it was (its index in the BVH's primitive array).
struct Nearest
{
  T             distance;
  std::uint32_t ball;
};

// The nearest ball surface to a point. The query is a PointQuery: the key of a box is the squared
// distance from the point to it. The bound is the best distance so far, squared to match: a box farther
// than the nearest surface found cannot hold a nearer one, and is skipped.
struct NearestBall
{
  BVH m_bvh;

  EBGEOMETRY_HOST_DEVICE
  Nearest
  operator()(const Vec3& a_point) const noexcept
  {
    const auto balls = m_bvh.getPrimitives();

    Nearest best{EBGeometry::Math::Limits<T>::max(), 0};

    m_bvh.traverse(
      EBGeometry::BVH::PointQuery<T>{a_point},
      best,
      // At a leaf: the distance to each of its balls' surfaces.
      [&](Nearest& a_best, std::size_t a_offset, std::size_t a_count) noexcept {
        for (std::size_t i = a_offset; i < a_offset + a_count; i++) {
          const Ball& b = balls[static_cast<std::uint32_t>(i)];
          const T     d = (a_point - b.center).length() - b.radius;

          if (std::abs(d) < std::abs(a_best.distance)) {
            a_best = Nearest{d, static_cast<std::uint32_t>(i)};
          }
        }
      },
      // The bound, in the key's units (a squared distance).
      [](const Nearest& a_best) noexcept { return a_best.distance * a_best.distance; });

    return best;
  }
};

// Where a ray first hits a ball surface: the distance along the ray (infinity for a miss) and the ball.
struct Hit
{
  T             t;
  std::uint32_t ball;
};

// The distance along the ray origin + t * direction (unit direction) to where it enters a ball, or
// infinity if it misses it or the ball is behind the origin.
EBGEOMETRY_HOST_DEVICE
T
enterBall(const Ball& a_ball, const Vec3& a_origin, const Vec3& a_direction) noexcept
{
  const Vec3 oc   = a_origin - a_ball.center;
  const T    b    = oc.dot(a_direction);
  const T    disc = b * b - (oc.dot(oc) - a_ball.radius * a_ball.radius);

  if (disc < T(0)) {
    return EBGeometry::Math::Limits<T>::infinity();
  }

  const T t = -b - std::sqrt(disc);

  return (t >= T(0)) ? t : EBGeometry::Math::Limits<T>::infinity();
}

// The first ball a ray hits. The query is a RayQuery: the key of a box is where the ray enters it, and
// a box it misses is never entered. The bound is the nearest hit so far, so a box the ray enters beyond
// it is skipped; NearestFirst (the default order) finds a near hit early. m_nodes, if given, counts
// every node the walk enters, through the visit callback.
struct FirstHit
{
  BVH   m_bvh;
  Vec3  m_direction;
  long* m_nodes = nullptr;

  EBGEOMETRY_HOST_DEVICE
  Hit
  operator()(const Vec3& a_origin) const noexcept
  {
    const auto balls = m_bvh.getPrimitives();

    Hit hit{EBGeometry::Math::Limits<T>::infinity(), 0};

    const auto leaf = [&](Hit& a_hit, std::size_t a_offset, std::size_t a_count) noexcept {
      for (std::size_t i = a_offset; i < a_offset + a_count; i++) {
        const T t = enterBall(balls[static_cast<std::uint32_t>(i)], a_origin, m_direction);

        if (t < a_hit.t) {
          a_hit = Hit{t, static_cast<std::uint32_t>(i)};
        }
      }
    };

    const auto bound = [](const Hit& a_hit) noexcept { return a_hit.t; };

    // The visit callback sees every node the walk enters (its index, the node, its key) and may refuse
    // it. Here it only counts; it could equally skip nodes flagged in an array of your own.
    long* nodes = m_nodes;

    const auto visit = [nodes](Hit&, std::uint32_t, const BVH::Node&, const T&) noexcept {
      if (nodes != nullptr) {
        (*nodes)++;
      }

      return true;
    };

    m_bvh.traverse(EBGeometry::BVH::RayQuery<T>(a_origin, m_direction), hit, leaf, bound, visit);

    return hit;
  }
};

// Whether the segment from a_from to a_to passes through any ball. A ray along the segment, with the
// bound starting at the segment's length (nothing beyond a_to counts). At the first ball hit the bound
// drops to -infinity, which ends the walk; AsStored visits children as stored, without sorting, since
// any hit will do. Marked EBGEOMETRY_HOST_DEVICE like the functors above, so that the lambdas inside it may
// be called by traverse() in a GPU build too.
EBGEOMETRY_HOST_DEVICE
bool
blocked(const BVH& a_bvh, const Vec3& a_from, const Vec3& a_to)
{
  const auto balls     = a_bvh.getPrimitives();
  const T    length    = (a_to - a_from).length();
  const Vec3 direction = (a_to - a_from) / length;

  T reach = length;

  a_bvh.traverse(
    EBGeometry::BVH::RayQuery<T>(a_from, direction),
    reach,
    [&](T& a_reach, std::size_t a_offset, std::size_t a_count) noexcept {
      for (std::size_t i = a_offset; i < a_offset + a_count; i++) {
        if (enterBall(balls[static_cast<std::uint32_t>(i)], a_from, direction) <= length) {
          a_reach = -EBGeometry::Math::Limits<T>::infinity();
        }
      }
    },
    [](const T& a_reach) noexcept { return a_reach; },
    EBGeometry::BVH::DescendAll{},
    EBGeometry::BVH::AsStored{});

  return reach < T(0);
}

} // namespace

int
main()
{
  // ── Build the BVH ──────────────────────────────────────────────────────────────────────────────────
  //
  // A PackedBVH is built from (primitive, bounding box) pairs into a pool, which must outlive the BVH and
  // every copy of it. buildBottomUp() sorts the boxes along a space-filling curve and groups them,
  // here at most 4 balls per leaf.
  const std::vector<Vec3> centers = EBGeometry::Random::samplePoints<T>(numBalls, ballSeed);

  std::vector<std::pair<Ball, AABB>> balls;

  for (const auto& c : centers) {
    balls.emplace_back(Ball{c, ballRadius}, AABB(c - ballRadius * Vec3::ones(), c + ballRadius * Vec3::ones()));
  }

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const BVH bvh = BVH::buildBottomUp(pool, balls, std::size_t(4));

  std::cout << numBalls << " balls of radius " << ballRadius << " in the unit cube, in a BVH of "
            << bvh.getNodes().size() << " nodes\n\n";

  // ── Use it on the host ─────────────────────────────────────────────────────────────────────────────
  const Vec3 point(T(0.5), T(0.5), T(0.5));

  const Nearest nearest = NearestBall{bvh}(point);

  std::cout << "Nearest ball surface to " << point << ": ball " << nearest.ball << ", at distance " << nearest.distance
            << "\n";

  // A ray along +x from the cube's -x face, and how many of the BVH's nodes the walk entered to find
  // its first hit.
  const Vec3 origin(T(0), T(0.5), T(0.5));
  long       entered = 0;

  const Hit hit = FirstHit{bvh, Vec3(T(1), T(0), T(0)), &entered}(origin);

  std::cout << "Ray from " << origin << " along +x: ";

  if (hit.t < EBGeometry::Math::Limits<T>::infinity()) {
    std::cout << "hits ball " << hit.ball << " at t = " << hit.t;
  }
  else {
    std::cout << "hits nothing";
  }

  std::cout << ", after entering " << entered << " of " << bvh.getNodes().size() << " nodes\n";

  // Line of sight between pairs of points.
  const Vec3 a(T(0.1), T(0.1), T(0.1));
  const Vec3 b(T(0.9), T(0.9), T(0.9));
  const Vec3 c(T(0.1), T(0.1), T(0.11));

  std::cout << "Segment " << a << " to " << b << ": " << (blocked(bvh, a, b) ? "blocked" : "clear") << "\n";
  std::cout << "Segment " << a << " to " << c << ": " << (blocked(bvh, a, c) ? "blocked" : "clear") << "\n\n";

  // ── The same queries on a GPU ──────────────────────────────────────────────────────────────────────
  //
  // Freeze the pool, mirror it in one piece into GPU::memoryResource() (device memory in a CUDA or HIP
  // build), and take a rebasedView() of the BVH that resolves against the copy. GPU::transform() runs a
  // functor once per input in a kernel, one query per thread. In a plain C++ build the same lines run on
  // the host.
  if (EBGeometry::GPU::enabled && !EBGeometry::GPU::deviceAvailable()) {
    std::cout << "On the device: skipped, no GPU is visible\n";

    return 0;
  }

  pool.freeze();

  const EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(pool, EBGeometry::GPU::memoryResource());
  const BVH              onDevice   = bvh.rebasedView(devicePool);

  const std::vector<Vec3> points = {point, Vec3(T(0.2), T(0.8), T(0.3))};
  const std::vector<Vec3> starts = {origin, Vec3(T(0), T(0.25), T(0.75))};

  std::vector<Nearest> nearestOnDevice(points.size());
  std::vector<Hit>     hitsOnDevice(starts.size());

  EBGeometry::GPU::transform(NearestBall{onDevice}, points, nearestOnDevice);
  EBGeometry::GPU::transform(FirstHit{onDevice, Vec3(T(1), T(0), T(0))}, starts, hitsOnDevice);

  std::cout << "Through GPU::transform (backend: " << EBGeometry::GPU::backendName << "):\n";

  for (std::size_t i = 0; i < points.size(); i++) {
    std::cout << "  nearest ball to " << points[i] << ": ball " << nearestOnDevice[i].ball << ", at distance "
              << nearestOnDevice[i].distance << "\n";
  }

  for (std::size_t i = 0; i < starts.size(); i++) {
    std::cout << "  ray from " << starts[i] << " along +x: ";

    if (hitsOnDevice[i].t < EBGeometry::Math::Limits<T>::infinity()) {
      std::cout << "ball " << hitsOnDevice[i].ball << " at t = " << hitsOnDevice[i].t << "\n";
    }
    else {
      std::cout << "nothing\n";
    }
  }

  return 0;
}
