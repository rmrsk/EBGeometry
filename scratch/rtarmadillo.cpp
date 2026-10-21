// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Scratch: the Armadillo scenes three ways, timed on the GPU (or the host without one):
//   sphere    sphere tracing the tape (TriMeshSDF, bounding-box skip): the reference;
//   ray       pure ray tracing: BVH over triangles, Moller-Trumbore, CSG by walking crossings;
//   hybrid-N  rays walk a BVH of boxes covering the surface (mesh leaves of < N triangles, and the
//             cylinder's box) and sphere trace the tape only inside the boxes they enter.
//
//   ./rtarmadillo <armadillo.obj> [tunnel|rod]
#include "RayTracer.hpp"

#include <array>
#include <chrono>
#include <cstdio>
#include <string>

using namespace Tracer;

// The same scene as a hand-written SDF, no tape: scale * TriMeshSDF(p / scale - shift), then max(A, -B)
// for the tunnel or min(A, B) for the rod.
template <class Tri>
struct DirectCSG
{
  Tri                        m_tri;
  EBGeometry::CylinderSDF<T> m_cyl;
  Vec3                       m_shift;
  T                          m_scale;
  bool                       m_union;

  EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3& p) const noexcept
  {
    const T a = m_scale * m_tri.signedDistance(p / m_scale - m_shift);
    const T b = m_cyl.signedDistance(p);

    return m_union ? EBGeometry::Math::min(a, b) : EBGeometry::Math::max(a, -b);
  }
};

static void
writePPM(const std::string& a_file, const std::vector<uint32_t>& a_pixels, const int a_w, const int a_h)
{
  std::FILE* f = std::fopen(a_file.c_str(), "wb");

  std::fprintf(f, "P6\n%d %d\n255\n", a_w, a_h);

  for (const uint32_t p : a_pixels) {
    const unsigned char rgb[3] = {uint8_t(p >> 24), uint8_t(p >> 16), uint8_t(p >> 8)};

    std::fwrite(rgb, 1, 3, f);
  }

  std::fclose(f);
}

int
main(int argc, char* argv[])
{
  using Clock = std::chrono::steady_clock;

  if (argc < 2) {
    std::fprintf(stderr, "usage: %s <armadillo.obj> [tunnel|rod]\n", argv[0]);

    return 1;
  }

  const std::string file  = argv[1];
  const std::string scene = argc > 2 ? argv[2] : "tunnel";
  const bool        rod   = (scene == "rod");

  const int W  = 960;
  const int H  = 960;
  const int SS = 2;

  const Vec3 shift(T(-0.01), T(-21.43), T(0.01));
  const T    scale = T(1) / T(75);

  const Vec3 ca = rod ? Vec3(-1.3, 0.25, 0.05) : Vec3(0, 0.25, -1.5);
  const Vec3 cb = rod ? Vec3(1.3, 0.25, 0.05) : Vec3(0, 0.25, 1.5);
  const T    cr = rod ? T(0.09) : T(0.16);

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  // The tape, as in the sphere tracer.
  const auto                tri       = EBGeometry::Parser::readIntoTriMeshSDF<T>(file, pool);
  const EBGeometry::Expr<T> armadillo = EBGeometry::Scale(EBGeometry::Translate(tri, shift), scale);
  const EBGeometry::Expr<T> cylinder  = EBGeometry::CylinderSDF<T>(ca, cb, cr);
  const EBGeometry::Tape<T> tape(
    pool, rod ? EBGeometry::Union(armadillo, cylinder) : EBGeometry::Difference(armadillo, cylinder));

  // The triangles for pure ray tracing.
  auto       t0      = Clock::now();
  const auto meshBVH = RayTracer::buildMesh(file, pool);
  auto       t1      = Clock::now();

  std::printf("triangle BVH for ray tracing built in %.0f ms\n",
              std::chrono::duration<double, std::milli>(t1 - t0).count());

  // Surface-covering boxes for the hybrid, at three leaf sizes.
  const std::array<size_t, 3>                                 leafSizes = {8, 32, 128};
  std::vector<RayTracer::Hybrid<EBGeometry::Tape<T>>::BoxBVH> boxBVHs;

  for (const size_t n : leafSizes) {
    t0 = Clock::now();

    std::vector<RayTracer::AABB> boxes = RayTracer::meshLeafBoxes(file, n, shift, scale, T(1e-3));

    boxes.push_back(RayTracer::cylinderBox(ca, cb, cr, T(1e-3)));

    const size_t numBoxes = boxes.size();

    std::vector<std::pair<RayTracer::AABB, RayTracer::AABB>> prims;

    for (const auto& b : boxes) {
      prims.emplace_back(b, b);
    }

    boxBVHs.emplace_back(
      pool, std::move(prims), EBGeometry::BVH::BinnedSAHPartitioner<T, RayTracer::AABB, RayTracer::AABB, 4>);

    t1 = Clock::now();

    std::printf("hybrid-%zu: %zu surface boxes, built in %.0f ms\n",
                n,
                numBoxes,
                std::chrono::duration<double, std::milli>(t1 - t0).count());
  }

  pool.freeze();

  const bool              haveDevice = EBGeometry::GPU::enabled && EBGeometry::GPU::deviceAvailable();
  const EBGeometry::Pool  devicePool = EBGeometry::Pool::mirror(pool, EBGeometry::GPU::memoryResource());
  const EBGeometry::Pool& usePool    = haveDevice ? devicePool : pool;

  Orbit cam;

  cam.yaw      = 200;
  cam.pitch    = 12;
  cam.distance = T(5.6);
  cam.target   = Vec3(0, T(-0.05), 0);
  cam.update();

  Settings display;

  display.floorY = T(-1.01);

  Settings sphereSettings = display;

  sphereSettings.boxBound = true;
  sphereSettings.boxLo    = tape.computeBoundingVolume().getLowCorner();
  sphereSettings.boxHi    = tape.computeBoundingVolume().getHighCorner();

  Settings shapeOnly;

  shapeOnly.floor = false;

  const EBGeometry::Tape<T> viewTape = tape.rebasedView(usePool);

  std::vector<uint32_t> indices(size_t(W) * size_t(H));

  for (size_t k = 0; k < indices.size(); k++) {
    indices[k] = uint32_t(k);
  }

  const auto render = [&](const char* a_name, const auto& a_pixel) {
    std::vector<uint32_t> pixels(indices.size());

    double best = 1e30;

    for (int r = 0; r < 3; r++) {
      const auto s = Clock::now();

      if (haveDevice) {
        EBGeometry::GPU::transform(a_pixel, indices, pixels);
      }
      else {
#pragma omp parallel for schedule(dynamic, 256)
        for (int k = 0; k < W * H; k++) {
          pixels[size_t(k)] = a_pixel(uint32_t(k));
        }
      }

      best = std::min(best, std::chrono::duration<double, std::milli>(Clock::now() - s).count());

      if (best > 5000) {
        break;
      }
    }

    const std::string out = "rt_" + scene + "_" + a_name + ".ppm";

    writePPM(out, pixels, W, H);

    std::printf("%-12s %9.1f ms on the %s  -> %s\n", a_name, best, haveDevice ? "GPU" : "CPU", out.c_str());
  };

  std::printf("\n%dx%d, %dx%d supersampling, %s scene:\n", W, H, SS, SS, scene.c_str());

  render("sphere", Pixel<EBGeometry::Tape<T>>{{viewTape, sphereSettings}, cam, W, H, SS});

  const RayTracer::Mesh     mesh{meshBVH.rebasedView(usePool), shift, scale};
  const RayTracer::Cylinder cyl(ca, cb, cr);

  render("ray", RayTracer::Pixel<RayTracer::Scene>{{mesh, cyl, rod, display}, cam, W, H, SS});

  for (size_t i = 0; i < leafSizes.size(); i++) {
    const RayTracer::Hybrid<EBGeometry::Tape<T>> hybrid{
      boxBVHs[i].rebasedView(usePool), Scene<EBGeometry::Tape<T>>{viewTape, shapeOnly}, display};

    render(("hybrid-" + std::to_string(leafSizes[i])).c_str(),
           RayTracer::Pixel<RayTracer::Hybrid<EBGeometry::Tape<T>>>{hybrid, cam, W, H, SS});
  }

  // Without the tape: the same sphere tracer and hybrid over a hand-written SDF.
  using Direct = DirectCSG<std::decay_t<decltype(tri)>>;

  const Direct direct{tri.rebasedView(usePool), EBGeometry::CylinderSDF<T>(ca, cb, cr), shift, scale, rod};

  render("sphere-notape", Pixel<Direct>{{direct, sphereSettings}, cam, W, H, SS});

  const RayTracer::Hybrid<Direct> directHybrid{
    boxBVHs[2].rebasedView(usePool), Scene<Direct>{direct, shapeOnly}, display};

  render("hybrid-128-notape", RayTracer::Pixel<RayTracer::Hybrid<Direct>>{directHybrid, cam, W, H, SS});

  // The hybrid's two parts: rays through the boxes only (every box entered counts as a hit), to time the
  // traversal without any sphere tracing.
  {
    Settings noShade = display;

    noShade.shadows   = false;
    noShade.occlusion = false;

    const RayTracer::Hybrid<Direct> noShadows{
      boxBVHs[2].rebasedView(usePool), Scene<Direct>{direct, shapeOnly}, noShade};

    render("hybrid-128-notape-noshadow-noAO", RayTracer::Pixel<RayTracer::Hybrid<Direct>>{noShadows, cam, W, H, SS});
    render("ray-noshadow-noAO", RayTracer::Pixel<RayTracer::Scene>{{mesh, cyl, rod, noShade}, cam, W, H, SS});
  }
}
