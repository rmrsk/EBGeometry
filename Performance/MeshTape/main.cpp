// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

// Performance cases for a mesh in a tape against the mesh alone: the armadillo as a TriMeshSDF, as a Tape holding only
// it, and as a RegionView of that tape (what ebgstudio draws of a model read through the language). Each is asked the
// same things: firstHit() and firstCrossing() on rays from random points in the mesh's box grown to twice its size
// towards its center, and one whole frame with shadows and ambient occlusion. The tape and the view give the mesh's
// answers, so each of their cases is noted relative to the mesh's (mesh/tape, mesh/view): the cost of reaching a mesh
// through a tape, which a probe of one kind of shape never shows. A host build runs the host cases (a frame on one
// thread); a CUDA or HIP build runs host/device pairs. In a probe of its own, so that adding it changes no other
// probe's translation unit (see Performance/Fused). See ../README.md and PerformanceTesting.rst.

#include <algorithm>
#include <array>
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

// The mesh's branching factor and leaf width, spelled out, as in ../RayCrossing/main.cpp.
constexpr std::size_t K = 4;
constexpr std::size_t W = 4;

// Rays per case, as in ../RayCrossing/main.cpp.
constexpr std::size_t HostQueries  = 100000;
constexpr std::size_t PairQueries  = 262144;
constexpr std::size_t SmokeQueries = 1000;

// The march, for the parts of a shape without a ray form (none here: it sets the renderer's tolerance).
constexpr double        Tolerance = 1e-4;
constexpr std::uint32_t MaxSteps  = 512;

// The frame: its side in pixels (host, device pair and smoke), rays per pixel side and host threads, as in
// ../Render/main.cpp.
constexpr std::size_t HostSide      = 192;
constexpr std::size_t PairSide      = 512;
constexpr std::size_t SmokeSide     = 16;
constexpr std::size_t FrameSampling = 1;
constexpr std::size_t FrameThreads  = 1;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

template <class T>
using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

template <class T>
using Mesh = EBGeometry::TriMeshSDF<T, K, W>;

// A tape with the mesh as a primitive type of its own (it is not the default TriMeshSDF in every build).
template <class T>
using MeshTape = EBGeometry::Tape<T, Mesh<T>>;

template <class T>
using MeshView = EBGeometry::RegionView<MeshTape<T>>;

// The ray from a_origin towards a_target.
template <class T>
EBGEOMETRY_HOST_DEVICE
EBGeometry::RayT<T>
rayTowards(const Vec3<T>& a_origin, const Vec3<T>& a_target) noexcept
{
  const Vec3<T> d = a_target - a_origin;

  return EBGeometry::RayT<T>{a_origin, d / d.length()};
}

// Where the ray from a point towards the target first crosses the shape (-1 for no crossing), through firstHit().
template <class T, class Shape>
struct FirstHit
{
  Shape   m_shape;
  Vec3<T> m_target;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3<T>& a_origin) const noexcept
  {
    const EBGeometry::HitT<T> hit =
      EBGeometry::firstHit(m_shape, rayTowards(a_origin, m_target), T(0), EBGeometry::Math::Limits<T>::infinity());

    return hit.hit() ? hit.m_t : -T(1);
  }
};

// The same, through firstCrossing().
template <class T, class Shape>
struct FirstCrossing
{
  Shape   m_shape;
  Vec3<T> m_target;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3<T>& a_origin) const noexcept
  {
    const EBGeometry::CrossingT<T> crossing =
      EBGeometry::firstCrossing(m_shape, rayTowards(a_origin, m_target), T(0), EBGeometry::Math::Limits<T>::infinity());

    return crossing.hit() ? crossing.m_t : -T(1);
  }
};

// Ray origins: random points in a box grown to twice its size about its center, as in ../RayCrossing/main.cpp.
template <class T>
std::vector<Vec3<T>>
origins(const AABB<T>& a_box, std::size_t a_n)
{
  const Vec3<T> center = T(0.5) * (a_box.getLowCorner() + a_box.getHighCorner());
  const Vec3<T> half   = a_box.getHighCorner() - center;

  return Perf::pointsInBox<T>(AABB<T>(center - T(2) * half, center + T(2) * half), a_n, Perf::Seed::RayOrigins);
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

// A pixel as a number, for the checksum and the host/device comparison: its red, green and blue channels, each in
// [0, 1], in one value.
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

// Whether a host and a device pixel agree: every channel within a tenth of its range, as in ../Render/main.cpp.
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

// The mesh: the armadillo (the dodecahedron under --smoke).
template <class T>
Mesh<T>
readMesh(Perf::Case& c, EBGeometry::Pool& a_pool)
{
  c.param("K", K);
  c.param("W", W);

  return EBGeometry::Parser::readIntoTriMeshSDF<T, K, W>(Perf::meshFile(c.smoke()), a_pool);
}

// The shape of an input, made in a pool: the mesh, the tape holding it, or the view of that tape.
template <class T, class Shape>
Shape
makeShape(Perf::Case& c, EBGeometry::Pool& a_pool)
{
  const Mesh<T> mesh = readMesh<T>(c, a_pool);

  if constexpr (std::is_same_v<Shape, Mesh<T>>) {
    return mesh;
  }
  else if constexpr (std::is_same_v<Shape, MeshTape<T>>) {
    return MeshTape<T>(a_pool, EBGeometry::Expr<T>(mesh));
  }
  else {
    return MeshView<T>(MeshTape<T>(a_pool, EBGeometry::Expr<T>(mesh)));
  }
}

// The mesh's case of an operation, which a tape's or a view's case is noted relative to (its name less the side).
std::string
meshCase(const std::string& a_operation, const std::string& a_prec)
{
  return Perf::caseName({"MeshTape", "mesh", a_operation, a_prec});
}

// One ray operation on one input in one precision: the host case and a host/device pair.
template <class T, class Shape, template <class, class> class Query>
void
addQuery(Perf::Suite& a_suite, const std::string& a_input, const std::string& a_operation, const std::string& a_label)
{
  const std::string prec = Perf::precisionName<T>();

  for (const std::string side : {"host", "hostdevice"}) {
    a_suite.add(Perf::caseName({"MeshTape", a_input, a_operation, prec, side}), [=](Perf::Case& c) {
      EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

      const Shape   shape  = makeShape<T, Shape>(c, pool);
      const AABB<T> box    = shape.computeBoundingVolume();
      const Vec3<T> center = T(0.5) * (box.getLowCorner() + box.getHighCorner());

      if (side == "host") {
        const auto     q = origins<T>(box, c.size(HostQueries, SmokeQueries));
        std::vector<T> out;

        Perf::measureHostQuery(c, Query<T, Shape>{shape, center}, q, out);

        c.checksum(Perf::checksumOf(out));
        c.poolBytes(pool.usedBytes());
      }
      else {
        const auto q   = origins<T>(box, c.size(PairQueries, SmokeQueries));
        const auto out = Perf::measureHostAndDevice<T>(
          c,
          pool,
          q,
          [&](const EBGeometry::Pool& a_pool) { return Query<T, Shape>{shape.rebasedView(a_pool), center}; },
          Perf::agreeDistance<T>);

        c.checksum(Perf::checksumOf(out));
      }

      if (!a_label.empty()) {
        c.relativeTo(meshCase(a_operation, prec) + "/" + side, a_label);
      }
    });
  }
}

// One whole frame of an input: on one host thread through the renderer, and its pixels as a host/device pair, as in
// ../Render/main.cpp.
template <class T, class Shape>
void
addFrame(Perf::Suite& a_suite, const std::string& a_input, const std::string& a_label)
{
  const std::string prec = Perf::precisionName<T>();

  a_suite.add(Perf::caseName({"MeshTape", a_input, "frame", prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const Shape       shape = makeShape<T, Shape>(c, pool);
    const std::size_t side  = c.size(HostSide, SmokeSide);

    const EBGeometry::Render::Renderer<T> renderer(frameSettings<T>(side));
    const auto                            camera = EBGeometry::Render::Camera<T>::fit(shape.computeBoundingVolume());

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

    if (!a_label.empty()) {
      c.relativeTo(meshCase("frame", prec) + "/host", a_label);
    }
  });

  a_suite.add(Perf::caseName({"MeshTape", a_input, "frame", prec, "hostdevice"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const Shape       shape    = makeShape<T, Shape>(c, pool);
    const std::size_t side     = c.size(PairSide, SmokeSide);
    const auto        settings = frameSettings<T>(side);
    const auto        camera   = EBGeometry::Render::Camera<T>::fit(shape.computeBoundingVolume());

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

    if (!a_label.empty()) {
      c.relativeTo(meshCase("frame", prec) + "/hostdevice", a_label);
    }
  });
}

template <class T>
void
addPrecision(Perf::Suite& a_suite)
{
  addQuery<T, Mesh<T>, FirstHit>(a_suite, "mesh", "firstHit", "");
  addQuery<T, Mesh<T>, FirstCrossing>(a_suite, "mesh", "firstCrossing", "");
  addFrame<T, Mesh<T>>(a_suite, "mesh", "");

  addQuery<T, MeshTape<T>, FirstHit>(a_suite, "tape", "firstHit", "mesh/tape");
  addQuery<T, MeshTape<T>, FirstCrossing>(a_suite, "tape", "firstCrossing", "mesh/tape");
  addFrame<T, MeshTape<T>>(a_suite, "tape", "mesh/tape");

  addQuery<T, MeshView<T>, FirstHit>(a_suite, "view", "firstHit", "mesh/view");
  addFrame<T, MeshView<T>>(a_suite, "view", "mesh/view");
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "MeshTape");

  addPrecision<float>(suite);
  addPrecision<double>(suite);

  return suite.run();
}
