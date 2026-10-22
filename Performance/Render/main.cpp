// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for the ray queries and the renderer: firstHit() along the camera rays of an image,
// through the classic CSG solid compiled into a Tape and through the armadillo as a TriMeshSDF, and one
// whole frame of the CSG solid with shadows and ambient occlusion, in float and double. A host build runs
// the host cases; a CUDA or HIP build runs firstHit() as host/device pairs. See ../README.md and
// PerformanceTesting.rst.

#include <cmath>
#include <cstddef>
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

// The march: the defaults of RayOptions, spelled out, since they select how far each ray steps.
constexpr double        Tolerance = 1e-4;
constexpr std::uint32_t MaxSteps  = 512;

// Image sides: camera rays for the CSG solid and for the mesh (whose rays cost about a hundred times
// more), and the frame; each at full and at smoke size.
constexpr std::size_t CSGSide       = 512;
constexpr std::size_t UnionSide     = 256;
constexpr std::size_t MixedSide     = 256;
constexpr std::size_t UnionSpheres  = 64;
constexpr std::size_t MeshSide      = 96;
constexpr std::size_t FrameSide     = 192;
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

// UnionSpheres spheres on a 4 x 4 x 4 lattice: one union, which the tape's ray program follows through a
// BVH.
template <class T>
EBGeometry::Expr<T>
latticeOfSpheres()
{
  std::vector<EBGeometry::Expr<T>> spheres;

  for (std::size_t i = 0; i < UnionSpheres; i++) {
    spheres.push_back(
      EBGeometry::SphereSDF<T>(Vec3<T>(T(i % 4), T((i / 4) % 4), T(i / 16)), T(0.3) + T(0.05) * T(i % 3)));
  }

  return EBGeometry::Union(spheres);
}

// A box with a torus and a smooth union cut out of it: the ray program follows the box and the cut, and
// sphere traces the torus and the smooth union (no ray form) along the ray.
template <class T>
EBGeometry::Expr<T>
mixedCut()
{
  const auto smooth = EBGeometry::SmoothUnion(EBGeometry::SphereSDF<T>(Vec3<T>(T(1), T(0), T(0)), T(0.5)),
                                              EBGeometry::SphereSDF<T>(Vec3<T>(T(1), T(0.6), T(0)), T(0.4)),
                                              T(0.2));

  return EBGeometry::Difference(
    EBGeometry::BoxSDF<T>(Vec3<T>(T(-1), T(-1), T(-1)), Vec3<T>(T(1), T(1), T(1))),
    EBGeometry::Union(EBGeometry::TorusSDF<T>(Vec3<T>(T(0), T(1), T(0)), T(0.9), T(0.25)), smooth));
}

// The rays through the pixel centres of an n x n image, from the camera Render::Camera::fit() places
// for a box: about half of them hit the shape.
template <class T>
std::vector<EBGeometry::RayT<T>>
cameraRays(const EBGeometry::BoundingVolumes::AABBT<T>& a_box, std::size_t a_n)
{
  const auto camera = EBGeometry::Render::Camera<T>::fit(a_box);

  Vec3<T> forward;
  Vec3<T> right;
  Vec3<T> up;

  camera.frame(forward, right, up);

  const T tanHalf = std::tan(T(0.5) * camera.m_fieldOfView * EBGeometry::pi<T> / T(180));
  const T n       = T(a_n);

  std::vector<EBGeometry::RayT<T>> rays;

  rays.reserve(a_n * a_n);

  for (std::size_t row = 0; row < a_n; row++) {
    for (std::size_t column = 0; column < a_n; column++) {
      const T       x = (T(2) * (T(column) + T(0.5)) - n) / n * tanHalf;
      const T       y = (n - T(2) * (T(row) + T(0.5))) / n * tanHalf;
      const Vec3<T> d = forward + x * right + y * up;

      rays.push_back(EBGeometry::RayT<T>{camera.eye(), d / d.length()});
    }
  }

  return rays;
}

// The query: the distance to a ray's first crossing, -1 for a miss and -2 for an unresolved ray.
template <class Shape, class T>
struct FirstHit
{
  Shape                     m_shape;
  T                         m_far;
  EBGeometry::RayOptions<T> m_options;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const EBGeometry::RayT<T>& a_ray) const noexcept
  {
    const EBGeometry::HitT<T> hit = EBGeometry::firstHit(m_shape, a_ray, T(0), m_far, m_options);

    return hit.hit() ? hit.m_t : ((hit.m_status == EBGeometry::HitStatus::Miss) ? T(-1) : T(-2));
  }
};

// Whether a host and a device hit agree: both miss (or are unresolved) alike, or they hit within twice
// the march's tolerance, since a device that rounds differently stops at a slightly different step.
template <class T>
std::pair<double, bool>
agreeRayHit(T a_host, T a_device, double a_tolerance)
{
  const double diff = std::abs(static_cast<double>(a_host) - static_cast<double>(a_device));

  if (a_host < T(0) || a_device < T(0)) {
    return {a_host == a_device ? 0.0 : diff, a_host != a_device};
  }

  return {diff, diff > 2.0 * a_tolerance};
}

// The march's options for a shape whose box has a given diagonal.
template <class T>
EBGeometry::RayOptions<T>
options(T a_scale)
{
  EBGeometry::RayOptions<T> o;

  o.m_tolerance = T(Tolerance);
  o.m_maxSteps  = MaxSteps;
  o.m_scale     = a_scale;

  return o;
}

template <class T>
T
diagonal(const EBGeometry::BoundingVolumes::AABBT<T>& a_box)
{
  return (a_box.getHighCorner() - a_box.getLowCorner()).length();
}

void
rayParams(Perf::Case& c, std::size_t a_side)
{
  c.param("side", a_side);
  c.param("maxSteps", MaxSteps);
  c.param("toleranceE6", static_cast<std::size_t>(Tolerance * 1e6));
}

// firstHit() along the camera rays through a shape: on the host, and as a host/device pair. a_make(case,
// pool) builds the shape in the pool.
template <class T, class Shape, class Make>
void
addFirstHit(Perf::Suite& a_suite, const std::string& a_input, std::size_t a_side, Make a_make)
{
  const std::string prec = Perf::precisionName<T>();

  a_suite.add(Perf::caseName({"Render", a_input, "firstHit", prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const Shape    shape = a_make(c, pool);
    const auto     box   = shape.computeBoundingVolume();
    const auto     side  = c.size(a_side, SmokeSide);
    const auto     rays  = cameraRays<T>(box, side);
    std::vector<T> ts;

    Perf::measureHostQuery(c, FirstHit<Shape, T>{shape, T(4) * diagonal(box), options(diagonal(box))}, rays, ts);

    c.checksum(Perf::checksumOf(ts));
    c.poolBytes(pool.usedBytes());
    rayParams(c, side);
  });

  a_suite.add(Perf::caseName({"Render", a_input, "firstHit-camera", prec, "hostdevice"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const Shape shape = a_make(c, pool);
    const auto  box   = shape.computeBoundingVolume();
    const auto  side  = c.size(a_side, SmokeSide);
    const auto  rays  = cameraRays<T>(box, side);
    const T     scale = diagonal(box);

    const auto ts = Perf::measureHostAndDevice<T>(
      c,
      pool,
      rays,
      [&](const EBGeometry::Pool& a_pool) {
        return FirstHit<Shape, T>{shape.rebasedView(a_pool), T(4) * scale, options(scale)};
      },
      [scale](T a_host, T a_device) { return agreeRayHit<T>(a_host, a_device, Tolerance * double(scale)); });

    c.checksum(Perf::checksumOf(ts));
    rayParams(c, side);
  });
}

// One whole frame of a shape on one host thread: the camera rays, then shading with shadows and ambient
// occlusion.
template <class T>
void
addFrame(Perf::Suite& a_suite)
{
  a_suite.add(Perf::caseName({"Render", "csg", "frame", Perf::precisionName<T>(), "host"}), [](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const EBGeometry::Tape<T> tape(pool, classicCSG<T>());

    EBGeometry::Render::Settings<T> settings;

    settings.m_width         = static_cast<std::uint32_t>(c.size(FrameSide, SmokeSide));
    settings.m_height        = settings.m_width;
    settings.m_supersampling = FrameSampling;
    settings.m_threads       = FrameThreads;
    settings.m_rays          = options(T(0));

    const EBGeometry::Render::Renderer<T> renderer(settings);
    const auto                            camera = EBGeometry::Render::Camera<T>::fit(tape.computeBoundingVolume());

    EBGeometry::Render::Image image;

    c.measure([&] { (void)renderer.render(tape, camera, image); });

    // Every pixel enters the checksum, as a number.
    std::vector<double> pixels(image.m_pixels.begin(), image.m_pixels.end());

    c.checksum(Perf::checksumOf(pixels));
    c.poolBytes(pool.usedBytes());
    c.param("side", settings.m_width);
    c.param("supersampling", FrameSampling);
    c.param("threads", FrameThreads);
    c.param("maxSteps", MaxSteps);
  });
}

template <class T>
void
addPrecision(Perf::Suite& a_suite)
{
  using Tri = EBGeometry::TriMeshSDF<T, K, W>;

  addFirstHit<T, EBGeometry::Tape<T>>(a_suite, "csg", CSGSide, [](Perf::Case&, EBGeometry::Pool& a_pool) {
    return EBGeometry::Tape<T>(a_pool, classicCSG<T>());
  });

  addFirstHit<T, EBGeometry::Tape<T>>(a_suite, "union64", UnionSide, [](Perf::Case& c, EBGeometry::Pool& a_pool) {
    c.param("spheres", UnionSpheres);

    return EBGeometry::Tape<T>(a_pool, latticeOfSpheres<T>());
  });

  addFirstHit<T, EBGeometry::Tape<T>>(a_suite, "mixed", MixedSide, [](Perf::Case&, EBGeometry::Pool& a_pool) {
    return EBGeometry::Tape<T>(a_pool, mixedCut<T>());
  });

  addFirstHit<T, Tri>(a_suite, "mesh", MeshSide, [](Perf::Case& c, EBGeometry::Pool& a_pool) {
    c.param("K", K);
    c.param("W", W);
    c.param("maxLeafGroups", MaxLeafGroups);

    return Tri(EBGeometry::Parser::readIntoTriangles<T>(Perf::meshFile(c.smoke())),
               a_pool,
               EBGeometry::BVH::Construction::SAH,
               MaxLeafGroups);
  });

  addFrame<T>(a_suite);
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
