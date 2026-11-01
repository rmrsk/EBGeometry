// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for a mesh in a composition, as in Examples/MeshCSG: the armadillo standing on a table (a
// rounded box on four cylinders), joined by a plain union through a tape, and by a smooth union through a tape
// (marched whole) and fused (the plain union traced, the blend zone marched; EBGeometry_SmoothRayForms.hpp). Each
// is timed as ray queries (every ray starts at a random point in the scene's box grown to twice its size and
// points at the box's center) and as whole frames of the renderer, with shadows and ambient occlusion. The fused
// smooth union names the tape's (tape/fused). A host build runs the host cases
// (frames on one thread); a CUDA or HIP build runs host/device pairs, the rays in Morton and random order. The
// plain union's frame is the device render kernel of a tape holding a mesh, whose speed has flipped with
// unrelated changes before. See ../README.md and PerformanceTesting.rst.

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

// The mesh's branching factor and leaf width, spelled out (see ../Mesh/main.cpp).
constexpr std::size_t K = 4;
constexpr std::size_t W = 4;

// The scene, as in Examples/MeshCSG: the mesh scaled to 2.4 along its longest side, and the smooth union's
// blend length.
constexpr double MeshSize  = 2.4;
constexpr double Smoothing = 0.3;

// The numbers of rays, at full and at smoke size: a smooth union's ray costs several of the mesh's signed
// distances.
constexpr std::size_t HostQueries  = 20000;
constexpr std::size_t PairQueries  = 65536;
constexpr std::size_t SmokeQueries = 500;

// The march, for the parts of a shape without a ray form.
constexpr double        Tolerance = 1e-4;
constexpr std::uint32_t MaxSteps  = 512;

// The frame: its side in pixels (host, device pair and smoke), rays per pixel side and host threads.
constexpr std::size_t HostSide      = 128;
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

// The mesh (the armadillo, the dodecahedron under --smoke) placed on the table, and the table's parts.
template <class T>
struct Scene
{
  Mesh<T> mesh;
  T       scale;
  Vec3<T> shift;
  T       hx;
  T       hz;
  T       smooth;

  Scene(Perf::Case& a_case, EBGeometry::Pool& a_pool)
    : mesh(EBGeometry::Parser::readIntoTriMeshSDF<T, K, W>(Perf::meshFile(a_case.smoke()), a_pool)),
      scale(T(1)),
      shift(Vec3<T>::zeros()),
      hx(T(0)),
      hz(T(0)),
      smooth(T(Smoothing))
  {
    const Vec3<T> lo = mesh.computeBoundingVolume().getLowCorner();
    const Vec3<T> hi = mesh.computeBoundingVolume().getHighCorner();

    scale = T(MeshSize) / (hi - lo)[(hi - lo).maxDir(true)];
    shift = Vec3<T>(T(-0.5) * (lo[0] + hi[0]), -lo[1], T(-0.5) * (lo[2] + hi[2]));
    hx    = T(0.5) * scale * (hi - lo)[0] + T(0.15);
    hz    = T(0.5) * scale * (hi - lo)[2] + T(0.15);

    a_case.param("K", K);
    a_case.param("W", W);
    a_case.param("smoothingE3", static_cast<std::size_t>(Smoothing * 1e3));
  }

  EBGeometry::RoundedBoxSDF<T>
  board() const
  {
    return EBGeometry::RoundedBoxSDF<T>(Vec3<T>(T(0), T(-0.06), T(0)), Vec3<T>(2 * hx, T(0.12), 2 * hz), T(0.03));
  }

  EBGeometry::CylinderSDF<T>
  leg(const T a_x, const T a_z) const
  {
    return EBGeometry::CylinderSDF<T>(Vec3<T>(a_x, T(-0.75), a_z), Vec3<T>(a_x, T(-0.06), a_z), T(0.06));
  }

  // The four legs' corners.
  std::array<std::pair<T, T>, 4>
  corners() const
  {
    return {std::pair<T, T>{-hx + T(0.12), -hz + T(0.12)},
            std::pair<T, T>{-hx + T(0.12), hz - T(0.12)},
            std::pair<T, T>{hx - T(0.12), -hz + T(0.12)},
            std::pair<T, T>{hx - T(0.12), hz - T(0.12)}};
  }

  // The scene as an expression: the plain or the smooth union of the placed mesh and the table.
  EBGeometry::Expr<T>
  expr(const bool a_smooth) const
  {
    const EBGeometry::Expr<T> placed = EBGeometry::Scale(EBGeometry::Translate(mesh, shift), scale);

    std::vector<EBGeometry::Expr<T>> table{board()};

    for (const auto& c : corners()) {
      table.push_back(leg(c.first, c.second));
    }

    return a_smooth ? EBGeometry::SmoothUnion(placed, EBGeometry::Union(table), smooth)
                    : EBGeometry::Union(placed, EBGeometry::Union(table));
  }

  // The smooth union, fused.
  auto
  fused() const
  {
    const auto c = corners();

    return EBGeometry::Fused::SmoothUnion(EBGeometry::Fused::Scale(EBGeometry::Fused::Translate(mesh, shift), scale),
                                          EBGeometry::Fused::Union(board(),
                                                                   leg(c[0].first, c[0].second),
                                                                   leg(c[1].first, c[1].second),
                                                                   leg(c[2].first, c[2].second),
                                                                   leg(c[3].first, c[3].second)),
                                          smooth);
  }
};

// The ray from a_origin towards a_target.
template <class T>
EBGEOMETRY_HOST_DEVICE
EBGeometry::RayT<T>
rayTowards(const Vec3<T>& a_origin, const Vec3<T>& a_target) noexcept
{
  const Vec3<T> d = a_target - a_origin;

  return EBGeometry::RayT<T>{a_origin, d / d.length()};
}

// Where the ray from a point towards the target first crosses the shape (-1 for no crossing).
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

// Whether a host and a device crossing agree, as in ../RayCSG/main.cpp: both miss, or both cross within the
// march's tolerance (twice RayOptions::m_tolerance times the scene's box diagonal), or they differ only by a
// graze: each crossing is on the surface, and the ray goes no deeper than the tolerance inside before the
// later answer. Swept in steps of the shape's distance, which cannot step over a solid, and at least the
// tolerance.
template <class T, class Shape>
struct AgreeCrossing
{
  Shape   m_shape;
  Vec3<T> m_target;

  std::pair<double, bool>
  operator()(T a_host, T a_device, const Vec3<T>& a_origin) const
  {
    const double diff     = std::abs(static_cast<double>(a_host) - static_cast<double>(a_device));
    const bool   mismatch = (a_host < T(0)) != (a_device < T(0)) || diff > this->tolerance();

    return {diff, mismatch && !this->grazes(a_origin, a_host, a_device)};
  }

private:
  double
  scale() const
  {
    const auto box = m_shape.computeBoundingVolume();

    return static_cast<double>((box.getHighCorner() - box.getLowCorner()).length());
  }

  double
  tolerance() const
  {
    return 2.0 * Tolerance * this->scale();
  }

  bool
  grazes(const Vec3<T>& a_origin, T a_host, T a_device) const
  {
    const double tol = this->tolerance();

    const bool hostHits   = a_host >= T(0);
    const bool deviceHits = a_device >= T(0);

    const EBGeometry::RayT<T> ray = rayTowards(a_origin, m_target);

    const auto value = [&](double t) { return static_cast<double>(m_shape.signedDistance(ray.at(static_cast<T>(t)))); };

    const double far = static_cast<double>((m_target - a_origin).length()) + this->scale();
    const double first =
      (hostHits && deviceHits) ? std::min<double>(a_host, a_device) : static_cast<double>(hostHits ? a_host : a_device);
    const double last = (hostHits && deviceHits) ? std::max<double>(a_host, a_device) : far;

    if (std::abs(value(first)) > tol || (hostHits && deviceHits && std::abs(value(last)) > tol)) {
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

// Ray origins in the box grown to twice its size about its center.
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

// A pixel as a number, for the checksum and the host/device comparison: its red, green and blue channels,
// each in [0, 1], in one value.
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

// The largest difference between two pixels' color channels, out of 255.
template <class T>
int
channelDifference(T a_first, T a_second)
{
  const auto channels = [](T a_value) {
    const auto v = static_cast<std::uint32_t>(std::lround(static_cast<double>(a_value) * 16777215.0));

    return std::array<int, 3>{int((v >> 16) & 0xFFU), int((v >> 8) & 0xFFU), int(v & 0xFFU)};
  };

  const auto a = channels(a_first);
  const auto b = channels(a_second);

  int worst = 0;

  for (std::size_t i = 0; i < 3; i++) {
    worst = std::max(worst, std::abs(a[i] - b[i]));
  }

  return worst;
}

// Whether a host and a device pixel agree: every channel within a tenth of its range; or the pixel is on an
// edge, or grazes something. A ray that grazes an edge (a silhouette, the border of a shadow) can land on either
// side of it on the host and on the device, and the pixel then takes the color of the other side: one of its four
// neighbors shows that color. A shadow or occlusion ray that grazes the shape can leave a speck of shadow one
// pixel wide on one side only, and a normal taken where the mesh's nearest triangle changes can turn: the host's
// own color then changes when the camera turns by a hundredth of a pixel. Only a pixel that differs is rendered
// again, on the host.
template <class T, class S>
struct AgreePixel
{
  S                               m_shape;
  EBGeometry::Render::Camera<T>   m_camera;
  EBGeometry::Render::Settings<T> m_settings;

  std::pair<double, bool>
  operator()(T a_host, T a_device, const std::uint32_t& a_index) const
  {
    const int    worst = channelDifference(a_host, a_device);
    const double diff  = double(worst) / 255.0;

    if (worst <= 25) {
      return {diff, false};
    }

    const std::uint32_t side = m_settings.m_width;
    const std::uint32_t x    = a_index % side;
    const std::uint32_t y    = a_index / side;

    const EBGeometry::Render::PixelQuery<T, S> query{m_shape, m_camera, m_settings};

    const std::array<std::pair<int, int>, 4> steps{{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};

    for (const auto& step : steps) {
      const int nx = int(x) + step.first;
      const int ny = int(y) + step.second;

      if (nx >= 0 && ny >= 0 && nx < int(side) && ny < int(side)) {
        const T neighbor = pixelValue<T>(query(std::uint32_t(ny) * side + std::uint32_t(nx)).m_rgba);

        if (channelDifference(neighbor, a_device) <= 25) {
          return {diff, false};
        }
      }
    }

    // A hundredth of a pixel, in degrees of the camera's yaw and pitch.
    const T turn = T(0.01) * m_camera.m_fieldOfView / T(side);

    for (const auto& step : steps) {
      EBGeometry::Render::Camera<T> turned = m_camera;

      turned.m_yaw += T(step.first) * turn;
      turned.m_pitch += T(step.second) * turn;

      const T moved = pixelValue<T>(EBGeometry::Render::PixelQuery<T, S>{m_shape, turned, m_settings}(a_index).m_rgba);

      if (channelDifference(moved, a_host) > 25 || channelDifference(moved, a_device) <= 25) {
        return {diff, false};
      }
    }

    return {diff, true};
  }
};

// The cases of one shape in one precision: rays (the host case, and host/device pairs in each order) and a
// frame (on one host thread, and its pixels as a host/device pair). a_build(case, pool) builds the shape in the
// pool; a_relativeTo, when not empty, names the input computing the same answers by other means, with
// a_relativeLabel.
template <class T, class Build>
void
addShape(Perf::Suite&       a_suite,
         const std::string& a_input,
         Build              a_build,
         const std::string& a_relativeTo    = "",
         const std::string& a_relativeLabel = "")
{
  const std::string prec = Perf::precisionName<T>();

  const auto relate = [=](Perf::Case& c, const std::string& a_operation, const std::string& a_side) {
    if (!a_relativeTo.empty()) {
      c.relativeTo(Perf::caseName({"MeshCSG", a_relativeTo, a_operation, prec, a_side}), a_relativeLabel);
    }
  };

  a_suite.add(Perf::caseName({"MeshCSG", a_input, "firstHit", prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const auto     shape  = a_build(c, pool);
    const auto     box    = shape.computeBoundingVolume();
    const Vec3<T>  center = T(0.5) * (box.getLowCorner() + box.getHighCorner());
    const auto     q      = origins<T>(box, c.size(HostQueries, SmokeQueries));
    std::vector<T> out;

    Perf::measureHostQuery(c, FirstHit<T, std::decay_t<decltype(shape)>>{shape, center}, q, out);

    c.checksum(Perf::checksumOf(out));
    c.poolBytes(pool.usedBytes());
    relate(c, "firstHit", "host");
  });

  for (const Perf::Order order : Perf::Orders) {
    const std::string operation = std::string("firstHit-") + Perf::orderName(order);

    a_suite.add(Perf::caseName({"MeshCSG", a_input, operation, prec, "hostdevice"}), [=](Perf::Case& c) {
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
        [&](const EBGeometry::Pool& a_pool) { return FirstHit<T, S>{shape.rebasedView(a_pool), center}; },
        AgreeCrossing<T, S>{shape, center});

      c.checksum(Perf::checksumOf(out));
      relate(c, operation, "hostdevice");
    });
  }

  a_suite.add(Perf::caseName({"MeshCSG", a_input, "frame", prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const auto        shape = a_build(c, pool);
    const std::size_t side  = c.size(HostSide, SmokeSide);

    const EBGeometry::Render::Renderer<T> renderer(frameSettings<T>(side));

    const auto camera = EBGeometry::Render::Camera<T>::fit(shape.computeBoundingVolume());

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
    relate(c, "frame", "host");
  });

  a_suite.add(Perf::caseName({"MeshCSG", a_input, "frame", prec, "hostdevice"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const auto        shape    = a_build(c, pool);
    const std::size_t side     = c.size(PairSide, SmokeSide);
    const auto        settings = frameSettings<T>(side);
    const auto        camera   = EBGeometry::Render::Camera<T>::fit(shape.computeBoundingVolume());

    using S = std::decay_t<decltype(shape)>;

    std::vector<std::uint32_t> indices(side * side);

    for (std::size_t i = 0; i < indices.size(); i++) {
      indices[i] = static_cast<std::uint32_t>(i);
    }

    const auto out = Perf::measureHostAndDevice<T>(
      c,
      pool,
      indices,
      [&](const EBGeometry::Pool& a_pool) {
        return Pixel<T, S>{EBGeometry::Render::PixelQuery<T, S>{shape.rebasedView(a_pool), camera, settings}};
      },
      AgreePixel<T, S>{shape, camera, settings});

    c.checksum(Perf::checksumOf(out));
    frameParams(c, side);
    relate(c, "frame", "hostdevice");
  });
}

template <class T>
void
addPrecision(Perf::Suite& a_suite)
{
  addShape<T>(a_suite, "union-tape", [](Perf::Case& c, EBGeometry::Pool& a_pool) {
    return MeshTape<T>(a_pool, Scene<T>(c, a_pool).expr(false));
  });

  addShape<T>(a_suite, "smooth-tape", [](Perf::Case& c, EBGeometry::Pool& a_pool) {
    return MeshTape<T>(a_pool, Scene<T>(c, a_pool).expr(true));
  });

  addShape<T>(
    a_suite,
    "smooth-fused",
    [](Perf::Case& c, EBGeometry::Pool& a_pool) { return Scene<T>(c, a_pool).fused(); },
    "smooth-tape",
    "tape/fused");
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "MeshCSG");

  addPrecision<float>(suite);
  addPrecision<double>(suite);

  return suite.run();
}
