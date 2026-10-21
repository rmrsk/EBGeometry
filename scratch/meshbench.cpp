// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Scratch: what sphere tracing a mesh costs. (1) How many distance evaluations a pixel takes, by kind
// of ray; (2) what one mesh distance query costs on the GPU, near the surface and far from it, for
// MeshSDF and TriMeshSDF directly and for a MeshSDF inside a Tape.
#include "Tracer.hpp"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <random>

using namespace Tracer;

// Counts calls on the host (one counter per thread, summed by the caller).
static thread_local uint64_t tl_calls = 0;

struct Counting
{
  EBGeometry::Tape<T> m_tape;

  T
  signedDistance(const Vec3& p) const noexcept
  {
    tl_calls++;

    return m_tape.signedDistance(p);
  }
};

template <class Q>
struct Eval
{
  Q m_q;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3& p) const noexcept
  {
    return m_q.signedDistance(p);
  }
};

int
main(int argc, char* argv[])
{
  using Clock = std::chrono::steady_clock;

  const std::string file = argc > 1 ? argv[1] : "armadillo.obj";

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const auto mesh = EBGeometry::Parser::readIntoMeshSDF<T>(file, pool);
  const auto tri  = EBGeometry::Parser::readIntoTriMeshSDF<T>(file, pool, EBGeometry::BVH::Construction::SAH);

  const Vec3 shift(T(-0.01), T(-21.43), T(0.01));
  const T    scale = T(1) / T(75);

  const EBGeometry::Tape<T> tape(pool, EBGeometry::Scale(EBGeometry::Translate(mesh, shift), scale));

  pool.freeze();

  // ── (1) Evaluations per pixel, on the host ──────────────────────────────────────────────────────
  {
    Orbit cam;

    cam.yaw      = 200;
    cam.pitch    = 12;
    cam.distance = T(5.6);
    cam.target   = Vec3(0, T(-0.05), 0);
    cam.update();

    const int N = 200;

    for (int variant = 0; variant < 3; variant++) {
      Settings s;

      s.floorY    = T(-1.01);
      s.shadows   = variant != 1;
      s.occlusion = variant != 1;
      s.floor     = variant != 2;

      const Pixel<Counting> pixel{{Counting{tape}, s}, cam, N, N, 1};

      uint64_t total = 0;
      uint64_t worst = 0;

      const auto t0 = Clock::now();

#pragma omp parallel for schedule(dynamic, 16) reduction(+ : total) reduction(max : worst)
      for (int k = 0; k < N * N; k++) {
        const uint64_t before = tl_calls;

        (void)pixel(uint32_t(k));

        const uint64_t calls = tl_calls - before;

        total += calls;
        worst = calls > worst ? calls : worst;
      }

      const double ms = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();

      std::printf("pixels %dx%d  %-32s evaluations/pixel: mean %6.1f  max %5lu   (host %.0f ms, %.2f us/eval/thread)\n",
                  N,
                  N,
                  variant == 0 ? "floor + shadows + AO:"
                               : (variant == 1 ? "floor, no shadows/AO:" : "no floor, shadows + AO:"),
                  double(total) / double(N * N),
                  (unsigned long)worst,
                  ms,
                  ms * 1e3 * 24 / double(total));
    }
  }

  // ── (2) One mesh query on the GPU ───────────────────────────────────────────────────────────────
  if (!(EBGeometry::GPU::enabled && EBGeometry::GPU::deviceAvailable())) {
    std::printf("no GPU\n");

    return 0;
  }

  const EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(pool, EBGeometry::GPU::memoryResource());

  const Eval<std::decay_t<decltype(mesh)>>    dMesh{mesh.rebasedView(devicePool)};
  const Eval<std::decay_t<decltype(tri)>>     dTri{tri.rebasedView(devicePool)};
  const Eval<EBGeometry::Tape<T>>             dTape{tape.rebasedView(devicePool)};
  const EBGeometry::BoundingVolumes::AABBT<T> box = mesh.computeBoundingVolume();
  const Vec3                                  lo  = box.getLowCorner();
  const Vec3                                  hi  = box.getHighCorner();
  const Vec3                                  ext = hi - lo;

  std::mt19937                      rng(42);
  std::uniform_real_distribution<T> u(0, 1);

  const size_t n = 1u << 20;

  // Point sets in mesh coordinates: near the surface (|d| < 1% of the height), anywhere in the box,
  // and in a shell between 1 and 3 box sizes from the centre (where floor and shadow rays spend their steps).
  std::vector<Vec3> nearPts, boxPts, farPts;

  while (boxPts.size() < n) {
    boxPts.emplace_back(lo[0] + u(rng) * ext[0], lo[1] + u(rng) * ext[1], lo[2] + u(rng) * ext[2]);
  }

  {
    std::vector<T> d(boxPts.size());

    EBGeometry::GPU::transform(dMesh, boxPts, d);

    for (size_t i = 0; i < boxPts.size() && nearPts.size() < n; i++) {
      if (std::abs(d[i]) < T(0.01) * ext[1]) {
        nearPts.push_back(boxPts[i]);
      }
    }

    while (nearPts.size() < n) {
      nearPts.push_back(nearPts[nearPts.size() % 1000]);
    }
  }

  const Vec3 c = T(0.5) * (lo + hi);

  while (farPts.size() < n) {
    const Vec3 p(T(2) * u(rng) - 1, T(2) * u(rng) - 1, T(2) * u(rng) - 1);

    if (p.length() > T(0.1) && p.length() < T(1)) {
      farPts.push_back(c + (T(1.5) + T(1.5) * u(rng)) * ext.length() * T(0.5) * (p / p.length()));
    }
  }

  const auto toTape = [&](const std::vector<Vec3>& in) {
    std::vector<Vec3> out;

    for (const auto& p : in) {
      out.push_back(scale * (p + shift));
    }

    return out;
  };

  const auto time = [&](const auto& q, const std::vector<Vec3>& pts) {
    std::vector<T> out(pts.size());

    EBGeometry::GPU::transform(q, pts, out);

    double best = 1e30;

    for (int r = 0; r < 5; r++) {
      const auto t0 = Clock::now();

      EBGeometry::GPU::transform(q, pts, out);

      best = std::min(best, std::chrono::duration<double, std::milli>(Clock::now() - t0).count());
    }

    return best;
  };

  std::printf("\n1M distance queries on the GPU (incl. ~2 ms of host<->device copies):\n");
  std::printf("%-26s %10s %10s %10s\n", "", "near", "in box", "far");

  std::printf("%-26s %8.2f ms %8.2f ms %8.2f ms\n",
              "MeshSDF (polygons)",
              time(dMesh, nearPts),
              time(dMesh, boxPts),
              time(dMesh, farPts));
  std::printf("%-26s %8.2f ms %8.2f ms %8.2f ms\n",
              "TriMeshSDF (SoA triangles)",
              time(dTri, nearPts),
              time(dTri, boxPts),
              time(dTri, farPts));
  std::printf("%-26s %8.2f ms %8.2f ms %8.2f ms\n",
              "Tape(Scale(Translate(MeshSDF)))",
              time(dTape, toTape(nearPts)),
              time(dTape, toTape(boxPts)),
              time(dTape, toTape(farPts)));
}
