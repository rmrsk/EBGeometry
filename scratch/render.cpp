// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Scratch: sphere-traced still image of the classic CSG solid, written as csg.ppm.
#include "Tracer.hpp"

#include <cstdio>
#include <cstdlib>

int
main(int argc, char* argv[])
{
  using namespace Tracer;

  const int W = 960;
  const int H = 720;

  EBGeometry::Pool          pool(EBGeometry::hostMemoryResource());
  const EBGeometry::Tape<T> tape(pool, classicCSG());

  Orbit cam;

  cam.yaw   = T(argc > 1 ? std::atof(argv[1]) : 35);
  cam.pitch = T(argc > 2 ? std::atof(argv[2]) : 28);
  cam.update();

  const Pixel<EBGeometry::Tape<T>> pixel{{tape, Settings()}, cam, W, H, 2};

  std::vector<uint32_t> pixels;

  renderHost(pixel, pixels);

  std::FILE* f = std::fopen("csg.ppm", "wb");

  std::fprintf(f, "P6\n%d %d\n255\n", W, H);

  for (const uint32_t p : pixels) {
    const unsigned char rgb[3] = {uint8_t(p >> 24), uint8_t(p >> 16), uint8_t(p >> 8)};

    std::fwrite(rgb, 1, 3, f);
  }

  std::fclose(f);

  std::printf("tape: %u clauses, wrote csg.ppm (%dx%d)\n", tape.getNumClauses(), W, H);
}
