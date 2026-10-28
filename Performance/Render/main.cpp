// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for the renderer: one whole frame, with shadows and ambient occlusion, of the
// classic CSG solid compiled into a Tape and of the armadillo as a TriMeshSDF, in float and double. A host
// build renders the frame on one thread (Renderer::render()); a CUDA or HIP build shades its pixels as a
// host/device pair, in the row order a device render launches them. The ray queries themselves are timed
// by the Ray and RayCSG probes. See ../README.md and PerformanceTesting.rst.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// The branching factor, triangle-group width and leaf size of the mesh, spelled out rather than taken
// from the defaults (see ../Mesh/main.cpp).
constexpr std::size_t K             = 4;
constexpr std::size_t W             = 4;
constexpr std::size_t MaxLeafGroups = 4;

// The march, for the parts of a shape without a ray form.
constexpr double        Tolerance = 1e-4;
constexpr std::uint32_t MaxSteps  = 512;

// The frame: its side in pixels (host, device pair and smoke), rays per pixel side and host threads.
constexpr std::size_t HostSide      = 192;
constexpr std::size_t PairSide      = 512;
constexpr std::size_t SmokeSide     = 16;
constexpr std::size_t FrameSampling = 1;
constexpr std::size_t FrameThreads  = 1;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

// The classic CSG solid: a cube with its corners cut by a ball, drilled through along x, y and z.
template <class T>
EBGeometry::Expr<T>
classicCSG()
{
  const EBGeometry::Expr<T> cube = EBGeometry::BoxSDF<T>(Vec3<T>(T(-1), T(-1), T(-1)), Vec3<T>(T(1), T(1), T(1)));
  const EBGeometry::Expr<T> ball = EBGeometry::SphereSDF<T>(Vec3<T>::zeros(), T(1.35));

  std::vector<EBGeometry::Expr<T>> drills;

  for (std::size_t axis = 0; axis < 3; axis++) {
    drills.push_back(EBGeometry::InfiniteCylinderSDF<T>(Vec3<T>::zeros(), T(0.55), axis));
  }

  return EBGeometry::Difference(EBGeometry::Intersection(cube, ball), EBGeometry::Union(drills));
}

// The renderer's settings for a frame of a_side x a_side pixels.
template <class T>
EBGeometry::Render::Settings<T>
frameSettings(std::size_t a_side)
{
  EBGeometry::Render::Settings<T> settings;

  settings.m_width            = static_cast<std::uint32_t>(a_side);
  settings.m_height           = settings.m_width;
  settings.m_supersampling    = FrameSampling;
  settings.m_threads          = FrameThreads;
  settings.m_rays.m_tolerance = T(Tolerance);
  settings.m_rays.m_maxSteps  = MaxSteps;

  return settings;
}

void
frameParams(Perf::Case& c, std::size_t a_side)
{
  c.param("side", a_side);
  c.param("supersampling", FrameSampling);
  c.param("maxSteps", MaxSteps);
  c.param("toleranceE6", static_cast<std::size_t>(Tolerance * 1e6));
}

// A pixel as a number, for the checksum and the host/device comparison: its red, green and blue
// channels, each in [0, 1], in one value.
template <class T>
EBGEOMETRY_HOST_DEVICE
T
pixelValue(std::uint32_t a_rgba)
{
  const T r = T((a_rgba >> 24) & 0xFFU);
  const T g = T((a_rgba >> 16) & 0xFFU);
  const T b = T((a_rgba >> 8) & 0xFFU);

  return (r * T(65536) + g * T(256) + b) / T(16777215);
}

// The renderer's pixel functor, returning the pixel as a number.
template <class T, class S>
struct Pixel
{
  EBGeometry::Render::PixelQuery<T, S> m_query;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const std::uint32_t& a_index) const noexcept
  {
    return pixelValue<T>(m_query(a_index).m_rgba);
  }
};

// Whether a host and a device pixel agree: every channel within a tenth of its range. A ray that grazes
// the silhouette can hit on one side and miss on the other, and then they differ.
template <class T>
std::pair<double, bool>
agreePixel(T a_host, T a_device)
{
  const auto channels = [](T a_value) {
    const auto v = static_cast<std::uint32_t>(std::lround(static_cast<double>(a_value) * 16777215.0));

    return std::array<int, 3>{int((v >> 16) & 0xFFU), int((v >> 8) & 0xFFU), int(v & 0xFFU)};
  };

  const auto h = channels(a_host);
  const auto d = channels(a_device);

  int worst = 0;

  for (std::size_t i = 0; i < 3; i++) {
    worst = std::max(worst, std::abs(h[i] - d[i]));
  }

  return {double(worst) / 255.0, worst > 25};
}

// One whole frame of a shape: on one host thread through the renderer, and its pixels as a host/device
// pair. a_make(case, pool) builds the shape in the pool. An orthographic frame (operation
// "frame-orthographic") casts parallel rays from a plane instead of rays from the eye.
template <class T, class Shape, class Make>
void
addFrame(Perf::Suite& a_suite, const std::string& a_input, Make a_make, const bool a_orthographic = false)
{
  const std::string operation = a_orthographic ? "frame-orthographic" : "frame";

  const std::string prec = Perf::precisionName<T>();

  a_suite.add(Perf::caseName({"Render", a_input, operation, prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const Shape       shape = a_make(c, pool);
    const std::size_t side  = c.size(HostSide, SmokeSide);

    const EBGeometry::Render::Renderer<T> renderer(frameSettings<T>(side));

    auto camera = EBGeometry::Render::Camera<T>::fit(shape.computeBoundingVolume());

    camera.m_orthographic = a_orthographic;

    EBGeometry::Render::Image image;

    c.measure([&] { (void)renderer.render(shape, camera, image); });

    // Every pixel enters the checksum.
    std::vector<T> pixels;

    pixels.reserve(image.m_pixels.size());

    for (const std::uint32_t rgba : image.m_pixels) {
      pixels.push_back(pixelValue<T>(rgba));
    }

    c.checksum(Perf::checksumOf(pixels));
    c.poolBytes(pool.usedBytes());
    c.param("threads", FrameThreads);
    frameParams(c, side);
  });

  a_suite.add(Perf::caseName({"Render", a_input, operation, prec, "hostdevice"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const Shape       shape    = a_make(c, pool);
    const std::size_t side     = c.size(PairSide, SmokeSide);
    const auto        settings = frameSettings<T>(side);

    auto camera = EBGeometry::Render::Camera<T>::fit(shape.computeBoundingVolume());

    camera.m_orthographic = a_orthographic;

    std::vector<std::uint32_t> indices(side * side);

    for (std::size_t i = 0; i < indices.size(); i++) {
      indices[i] = static_cast<std::uint32_t>(i);
    }

    const auto out = Perf::measureHostAndDevice<T>(
      c,
      pool,
      indices,
      [&](const EBGeometry::Pool& a_pool) {
        return Pixel<T, Shape>{EBGeometry::Render::PixelQuery<T, Shape>{shape.rebasedView(a_pool), camera, settings}};
      },
      agreePixel<T>);

    c.checksum(Perf::checksumOf(out));
    frameParams(c, side);
  });
}

template <class T>
void
addPrecision(Perf::Suite& a_suite)
{
  using Tri = EBGeometry::TriMeshSDF<T, K, W>;

  const auto csg = [](Perf::Case&, EBGeometry::Pool& a_pool) { return EBGeometry::Tape<T>(a_pool, classicCSG<T>()); };

  addFrame<T, EBGeometry::Tape<T>>(a_suite, "csg", csg);
  addFrame<T, EBGeometry::Tape<T>>(a_suite, "csg", csg, true);

  addFrame<T, Tri>(a_suite, "mesh", [](Perf::Case& c, EBGeometry::Pool& a_pool) {
    c.param("K", K);
    c.param("W", W);
    c.param("maxLeafGroups", MaxLeafGroups);

    return Tri(EBGeometry::Parser::readIntoTriangles<T>(Perf::meshFile(c.smoke())),
               a_pool,
               EBGeometry::BVH::Construction::SAH,
               MaxLeafGroups);
  });
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "Render");

  addPrecision<float>(suite);
  addPrecision<double>(suite);

  return suite.run();
}
