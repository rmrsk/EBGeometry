// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// A tornado of half a million spheres or more, all of one radius, colored by the speed of the wind where each one sits:
// how a union of very many spheres scales. The spheres are packed as in SphereClouds, each at the first random position
// where it overlaps no sphere placed before it, here in the shape of a tornado. They are sorted into 16 bins of wind
// speed; each bin is a BVHUnion of its spheres, and the bins are the leaves of one tape, a union through a BVH over
// them, so the viewer gives each bin its own color. See README.md.
//
//   ./Tornado.ex [--count=<n>] [--radius=<r>]
//
// --count is the number of spheres (default 500000; try 8000000) and --radius their radius. By default the radius
// follows the count, 0.003 at 8 million, so that the tornado looks as dense at any count. A sphere that finds no free
// position is left out, so the program prints how many it placed.

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <EBGeometry.hpp>
#include <Source/EBGeometry_Random.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T      = EBGEOMETRY_PRECISION;
using Vec3   = EBGeometry::Vec3T<T>;
using Sphere = EBGeometry::SphereSDF<T>;
using Bin    = EBGeometry::BVHUnion<T, EBGeometry::BVH::DefaultBranchingRatio<T>(), Sphere>;

// A tape whose leaves are bins: their type is listed among the tape's primitives.
using Tornado = EBGeometry::Tape<T, Bin>;

namespace Math = EBGeometry::Math;

// The bins of wind speed, one color each: as many as the viewer has colors for its leaves.
constexpr int numBins = int(EBGeometry::Render::Settings<T>::PrimitiveColors);

// The tornado stands along y, height H. Its funnel's wall is centered on radius funnelRadius(h), and its axis sways
// by up to sway. Over it hangs a cloud base out to cloudRadius; around its foot a debris cloud rises up to
// debrisHeight, out to debrisRadius. numBands spiral bands wind bandTwist turns up the funnel.
constexpr T             H            = T(10);
constexpr T             rBottom      = T(0.22);
constexpr T             rTop         = T(2.4);
constexpr T             flare        = T(0.9);
constexpr T             sway         = T(1.0);
constexpr int           numBands     = 5;
constexpr T             bandTwist    = T(2.5);
constexpr T             cloudRadius  = T(5.5);
constexpr T             cloudDepth   = T(1.0);
constexpr T             debrisRadius = T(2.8);
constexpr T             debrisHeight = T(3.2);
constexpr T             twoPi        = T(6.283185307179586);
constexpr std::uint64_t seed         = 123456789ULL;
constexpr std::size_t   chunk        = std::size_t(1) << 20;

// The wind: the swirl's circulation, and the peak speeds of the updraft in the core and the inflow along the ground.
constexpr T circulation = T(60);
constexpr T updraft     = T(25);
constexpr T inflow      = T(12);

namespace {

// The funnel's radius at height h: narrow at the ground, widening up the funnel, flaring out into the cloud base.
T
funnelRadius(const T a_h)
{
  const T s = Math::clamp(a_h / H, T(0), T(1));

  return rBottom + (rTop - rBottom) * std::pow(s, T(2.2)) + flare * std::pow(s, T(10));
}

// The axis' offset from the y axis at height h: none at the ground, a rope-like S-curve upward.
Vec3
axisAt(const T a_h)
{
  const T s = a_h / H;

  return Vec3(sway * s * std::sin(T(1.3) * twoPi * s), T(0), T(0.6) * sway * s * std::sin(T(0.7) * twoPi * s + T(0.5)));
}

// The wind speed at distance r from the axis, at height h: a Burgers-Rott vortex (a model of a tornado's core) whose
// swirl peaks on the funnel's wall and falls off as 1/r outside it, so the narrow foot spins fastest; an updraft in
// the core rising off the ground; and inflow along the ground.
T
windSpeed(const T a_r, const T a_h)
{
  const T r      = Math::max(a_r, T(1.E-6)); // the swirl's 1/r, kept finite on the axis
  const T rc     = funnelRadius(a_h) / T(1.12);
  const T x      = (r * r) / (rc * rc);
  const T swirl  = circulation / (twoPi * r) * (T(1) - std::exp(-x));
  const T up     = updraft * (T(1) - std::exp(-a_h / T(1.5))) * std::exp(-T(0.25) * x);
  const T inward = inflow * std::exp(-a_h / T(0.6)) * std::sqrt(x) * std::exp(T(0.5) * (T(1) - x));

  return std::sqrt(swirl * swirl + up * up + inward * inward);
}

// A probability peaked where phase is a multiple of 2 pi, falling to floor between: the bands and arms.
T
peak(const T a_phase, const T a_power, const T a_floor)
{
  return a_floor + (T(1) - a_floor) * std::pow(T(0.5) * (T(1) + std::cos(a_phase)), a_power);
}

// A candidate center from six uniform numbers in [0,1) and the wind speed there, or false if it falls between bands.
bool
candidate(const Vec3& a_u, const Vec3& a_v, Vec3& a_center, T& a_speed)
{
  const T theta = twoPi * a_u[1];

  T rho;
  T h;
  T keep;

  if (a_v[2] < T(0.25)) {
    // The debris cloud: an inverted cone around the foot, a wall thrown out wider and thicker the higher it rises
    // (more spheres where it is wide), fading toward its top, in two arms spiraling up and out.
    const T s         = std::sqrt(a_u[2]);
    const T thickness = T(0.1) + T(0.5) * s;

    h    = debrisHeight * s;
    rho  = funnelRadius(h) + T(0.1) + debrisRadius * std::pow(s, T(1.5)) + thickness * (a_u[0] - T(0.5));
    keep = peak(T(2) * theta - T(2.5) * std::log(rho) - T(1.5) * h, T(3), T(0.05)) * (T(1) - T(0.7) * s);
  }
  else if (a_v[2] < T(0.37)) {
    // The cloud base: a lens from the funnel's mouth out to cloudRadius, sagging toward the funnel and thinning to
    // nothing at its rim, with spiral arms.
    const T inner = funnelRadius(H);
    const T t     = std::sqrt(a_u[0]);

    rho  = inner + (cloudRadius - inner) * t;
    h    = H - T(0.6) * (T(1) - t) * (T(1) - t) + cloudDepth * std::sqrt(T(1) - t * t) * a_u[2];
    keep = peak(T(3) * theta - T(3) * std::log(rho), T(3), T(0.06)) * (T(1) - t * t * t);
  }
  else {
    // The funnel's wall: more spheres where it is wide, densest in the middle of its thickness.
    const T s         = std::pow(a_u[2], T(0.6));
    const T thickness = T(0.12) + T(0.58) * s;

    h    = H * s;
    rho  = funnelRadius(h) + thickness * (a_u[0] + a_v[1] - T(1));
    keep = peak(T(numBands) * theta - twoPi * bandTwist * s, T(3), T(0.04));
  }

  if (a_v[0] > keep) {
    return false;
  }

  a_center = axisAt(h) + Vec3(rho * std::cos(theta), h, rho * std::sin(theta));
  a_speed  = windSpeed(rho, h);

  return true;
}

// The colormap, slow (t = 0) to fast (t = 1): rust through terracotta, ochre and sand to bone white. Its sRGB stops are
// interpolated and converted to the linear RGB the renderer takes.
Vec3
desert(const T a_t)
{
  const Vec3 stops[] = {Vec3(T(0.40), T(0.12), T(0.06)),
                        Vec3(T(0.66), T(0.27), T(0.13)),
                        Vec3(T(0.80), T(0.50), T(0.20)),
                        Vec3(T(0.89), T(0.72), T(0.45)),
                        Vec3(T(0.96), T(0.90), T(0.78))};

  const T   x = Math::clamp(a_t, T(0), T(1)) * T(4);
  const int i = Math::min(int(x), 3);
  const T   f = x - T(i);

  const Vec3 srgb = (T(1) - f) * stops[i] + f * stops[i + 1];

  return Vec3(std::pow(srgb[0], T(2.2)), std::pow(srgb[1], T(2.2)), std::pow(srgb[2], T(2.2)));
}

// A hash grid of cells one diameter wide, for the overlap test: a sphere that overlaps one centered in a cell has its
// center in that cell or a neighbor. Each cell keeps the head of a linked list of its spheres, through m_next.
struct Grid
{
  T                                      m_cell;
  std::unordered_map<std::uint64_t, int> m_head;
  std::vector<int>                       m_next;

  std::uint64_t
  key(const Vec3& a_center, const long a_di, const long a_dj, const long a_dk) const noexcept
  {
    // 21 bits per axis: a million cells either side of the origin. Beyond that (a radius far below the default)
    // distant cells share keys, which slows the test but never changes its answer: it compares true distances.
    const auto u = [this](const T a_x, const long a_d) {
      return std::uint64_t(long(std::floor(a_x / m_cell)) + a_d + (1L << 20)) & ((1ULL << 21) - 1);
    };

    return (u(a_center[0], a_di) << 42) | (u(a_center[1], a_dj) << 21) | u(a_center[2], a_dk);
  }

  bool
  free(const Vec3& a_center, const std::vector<Vec3>& a_centers) const
  {
    for (long di = -1; di <= 1; di++) {
      for (long dj = -1; dj <= 1; dj++) {
        for (long dk = -1; dk <= 1; dk++) {
          const auto it = m_head.find(key(a_center, di, dj, dk));

          for (int n = (it == m_head.end()) ? -1 : it->second; n >= 0; n = m_next[std::size_t(n)]) {
            if ((a_center - a_centers[std::size_t(n)]).length2() < m_cell * m_cell) {
              return false;
            }
          }
        }
      }
    }

    return true;
  }

  void
  insert(const Vec3& a_center, const int a_n)
  {
    auto [it, fresh] = m_head.try_emplace(key(a_center, 0, 0, 0), a_n);

    m_next.push_back(fresh ? -1 : it->second);
    it->second = a_n;
  }
};

double
secondsSince(const std::chrono::steady_clock::time_point a_start)
{
  return std::chrono::duration<double>(std::chrono::steady_clock::now() - a_start).count();
}

} // namespace

int
main(int argc, char* argv[])
{
  std::size_t count  = 500000;
  T           radius = T(-1);

  for (int i = 1; i < argc; i++) {
    const std::string arg(argv[i]);

    if (arg.rfind("--count=", 0) == 0) {
      count = std::size_t(std::stoull(arg.substr(8)));
    }
    else if (arg.rfind("--radius=", 0) == 0) {
      radius = T(std::stod(arg.substr(9)));
    }

    // Other arguments, such as --no-view, are for Render::view().
  }

  if (radius <= T(0)) {
    radius = T(0.003) * std::cbrt(T(8000000) / T(count));
  }

  // ── Pack the spheres ───────────────────────────────────────────────────────────────────────────────
  //
  // As in SphereClouds, each sphere at the next random position where it overlaps none placed before it, up to 30
  // positions tried per sphere asked for. The positions are drawn a chunk at a time, so that many need little memory;
  // the wind speed at each sphere is kept beside it, in an array of the program's own.
  auto start = std::chrono::steady_clock::now();

  const std::size_t numTries = 30 * count;

  std::vector<Vec3> centers;
  std::vector<T>    speeds;
  std::vector<Vec3> shape;
  std::vector<Vec3> accept;
  Grid              grid{T(2) * radius, {}, {}};

  centers.reserve(count);
  speeds.reserve(count);
  grid.m_head.reserve(2 * count);
  grid.m_next.reserve(count);

  for (std::size_t next = 0; next < numTries && centers.size() < count; next++) {
    if (next % chunk == 0) {
      shape  = EBGeometry::Random::samplePoints<T>(chunk, seed + 2 * (next / chunk));
      accept = EBGeometry::Random::samplePoints<T>(chunk, seed + 2 * (next / chunk) + 1);
    }

    Vec3 center;
    T    speed;

    if (candidate(shape[next % chunk], accept[next % chunk], center, speed) && grid.free(center, centers)) {
      grid.insert(center, int(centers.size()));
      centers.push_back(center);
      speeds.push_back(speed);
    }
  }

  std::cout << std::fixed << std::setprecision(2) << "Placed " << centers.size() << " of " << count
            << " spheres of radius " << std::setprecision(4) << radius << " in " << std::setprecision(2)
            << secondsSince(start) << " s\n";

  if (centers.empty()) {
    return 0;
  }

  // ── Sort them by speed ─────────────────────────────────────────────────────────────────────────────
  //
  // numBins equal steps of speed, from the slowest sphere to the 99th percentile (the fastest few, at the very foot of
  // the funnel, join the last bin), so that the colors spread over the whole range.
  std::vector<T> sorted = speeds;

  std::sort(sorted.begin(), sorted.end());

  const T slowest = sorted.front();
  const T fastest = sorted[std::size_t(T(0.99) * T(sorted.size() - 1))];
  const T range   = Math::max(fastest - slowest, T(1.E-6));

  std::vector<std::vector<Sphere>> bins(numBins);

  for (std::size_t i = 0; i < centers.size(); i++) {
    const int bin = Math::clamp(int(T(numBins) * (speeds[i] - slowest) / range), 0, numBins - 1);

    bins[std::size_t(bin)].emplace_back(centers[i], radius);
  }

  // ── Build the tornado ──────────────────────────────────────────────────────────────────────────────
  //
  // The viewer colors a tape by leaf, one color per leaf; a BVHUnion is one leaf, however many spheres it holds. So
  // each bin is a BVHUnion of its spheres (a BVH over them), and the bins are the operands of one union, built through
  // a BVH over the bins' boxes. The tape numbers its leaves in the order of the operands, the bins' order of speed,
  // and leaf i takes its color from m_primitiveColors[i]. An empty bin is left out, and its color with it.
  start = std::chrono::steady_clock::now();

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  EBGeometry::Render::Settings<T>  settings;
  std::vector<EBGeometry::Expr<T>> leaves;

  settings.m_colorBy = EBGeometry::Render::ColorBy::Primitive;

  std::cout << "Bins of wind speed (lowest speed, spheres):\n";

  for (int b = 0; b < numBins; b++) {
    const std::vector<Sphere>& spheres = bins[std::size_t(b)];

    std::cout << "  " << std::setw(2) << b << ": " << std::setw(6) << slowest + range * T(b) / T(numBins) << ", "
              << spheres.size() << '\n';

    if (!spheres.empty()) {
      settings.m_primitiveColors[uint32_t(leaves.size())] = desert(T(b) / T(numBins - 1));
      leaves.emplace_back(Bin(pool, spheres));
    }
  }

  const Tornado tornado(pool, EBGeometry::Union(leaves, EBGeometry::Accelerate::BVH));

  std::cout << "Built " << leaves.size() << " BVHUnions and the tape in " << secondsSince(start) << " s, box "
            << tornado.computeBoundingVolume() << '\n';

  // ── Look at it ─────────────────────────────────────────────────────────────────────────────────────
  //
  // A window on the tornado, standing along y (built with VIEWER=SDL2; otherwise, with --no-view or without a display,
  // view() prints why and returns). It opens colored by leaf, the bins of speed; X cuts it open. Given the pool,
  // view() renders on the GPU in a GPU build.
  const auto camera = EBGeometry::Render::Camera<T>::fit(tornado.computeBoundingVolume(), Vec3(T(0), T(1), T(0)));

  EBGeometry::Render::view(tornado, pool, camera, settings, argc, argv);

  return 0;
}
