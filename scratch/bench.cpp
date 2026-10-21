// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Scratch: frame times of the tracer on the host (OpenMP) and through GPU::transform().
#include "Tracer.hpp"

#include <chrono>
#include <cstdio>
#include <cstdlib>

// The same solid written out by hand, no tape: max(max(box, ball), -min(cylinders)).
struct DirectCSG
{
  EBGeometry::BoxSDF<Tracer::T>              m_box;
  EBGeometry::SphereSDF<Tracer::T>           m_ball;
  EBGeometry::InfiniteCylinderSDF<Tracer::T> m_cyl[3];

  EBGEOMETRY_HOST_DEVICE
  Tracer::T
  signedDistance(const Tracer::Vec3& p) const noexcept
  {
    const Tracer::T body  = Tracer::Math::max(m_box.signedDistance(p), m_ball.signedDistance(p));
    const Tracer::T holes = Tracer::Math::min(
      m_cyl[0].signedDistance(p), Tracer::Math::min(m_cyl[1].signedDistance(p), m_cyl[2].signedDistance(p)));

    return Tracer::Math::max(body, -holes);
  }
};

int
main()
{
  using namespace Tracer;
  using Clock = std::chrono::steady_clock;

  EBGeometry::Pool          pool(EBGeometry::hostMemoryResource());
  const EBGeometry::Tape<T> tape(pool, classicCSG());

  pool.freeze();

  const EBGeometry::Pool    devicePool = EBGeometry::Pool::mirror(pool, EBGeometry::GPU::memoryResource());
  const EBGeometry::Tape<T> deviceTape = tape.rebasedView(devicePool);
  const bool                haveDevice = EBGeometry::GPU::enabled && EBGeometry::GPU::deviceAvailable();

  using Vec = Vec3;

  const DirectCSG direct{EBGeometry::BoxSDF<T>(Vec(-1, -1, -1), Vec(1, 1, 1)),
                         EBGeometry::SphereSDF<T>(Vec::zeros(), T(1.35)),
                         {EBGeometry::InfiniteCylinderSDF<T>(Vec::zeros(), T(0.55), 0),
                          EBGeometry::InfiniteCylinderSDF<T>(Vec::zeros(), T(0.55), 1),
                          EBGeometry::InfiniteCylinderSDF<T>(Vec::zeros(), T(0.55), 2)}};

  Orbit cam;

  cam.update();

  std::printf("backend %s, device %s\n", EBGeometry::GPU::backendName, haveDevice ? "yes" : "no");

  struct Size
  {
    int w, h, ss;
  };

  for (const Size s : {Size{240, 180, 1}, Size{960, 720, 1}, Size{960, 720, 2}, Size{1920, 1440, 2}}) {
    std::vector<uint32_t> indices(size_t(s.w) * size_t(s.h));

    for (size_t k = 0; k < indices.size(); k++) {
      indices[k] = uint32_t(k);
    }

    std::vector<uint32_t> host, device;

    const auto time = [](auto&& f) {
      f(); // Warm-up.

      double best = 1e30;

      for (int r = 0; r < 5; r++) {
        const auto t0 = Clock::now();
        f();
        best = std::min(best, std::chrono::duration<double, std::milli>(Clock::now() - t0).count());
      }

      return best;
    };

    const double hostMs =
      time([&] { renderHost(Pixel<EBGeometry::Tape<T>>{{tape, Settings()}, cam, s.w, s.h, s.ss}, host); });

    std::printf("%4dx%-4d ss=%d   CPU %8.2f ms", s.w, s.h, s.ss, hostMs);

    if (haveDevice) {
      const double devMs = time([&] {
        renderDevice(Pixel<EBGeometry::Tape<T>>{{deviceTape, Settings()}, cam, s.w, s.h, s.ss}, indices, device);
      });

      size_t differ = 0;

      for (size_t k = 0; k < host.size(); k++) {
        differ += (host[k] != device[k]);
      }

      int maxDiff = 0;

      for (size_t k = 0; k < host.size(); k++) {
        for (int c = 1; c < 4; c++) {
          maxDiff = std::max(maxDiff, std::abs(int((host[k] >> (8 * c)) & 0xFF) - int((device[k] >> (8 * c)) & 0xFF)));
        }
      }

      std::vector<uint32_t> dhost, ddev;

      const double dHostMs =
        time([&] { renderHost(Pixel<DirectCSG>{{direct, Settings()}, cam, s.w, s.h, s.ss}, dhost); });
      const double dDevMs =
        time([&] { renderDevice(Pixel<DirectCSG>{{direct, Settings()}, cam, s.w, s.h, s.ss}, indices, ddev); });

      std::printf("   GPU %8.2f ms  (%4.1fx)  differ %zu, max %d/255   | no tape: CPU %7.2f ms  GPU %6.2f ms (%5.1fx)",
                  devMs,
                  hostMs / devMs,
                  differ,
                  maxDiff,
                  dHostMs,
                  dDevMs,
                  dHostMs / dDevMs);
    }

    std::printf("\n");
  }
}
