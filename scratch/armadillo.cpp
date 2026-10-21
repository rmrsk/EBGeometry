// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Scratch: the Armadillo pierced by a cylinder, sphere traced on the GPU (or the host without one).
//
//   ./armadillo <armadillo.obj> [tunnel|rod] [yaw] [pitch] [out.ppm]
#include "Tracer.hpp"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>

int
main(int argc, char* argv[])
{
  using namespace Tracer;
  using Clock = std::chrono::steady_clock;

  if (argc < 2) {
    std::fprintf(stderr, "usage: %s <armadillo.obj> [tunnel|rod] [yaw] [pitch] [out.ppm]\n", argv[0]);

    return 1;
  }

  const std::string file = argv[1];
  const std::string mode = argc > 2 ? argv[2] : "tunnel";
  const std::string out  = argc > 5 ? argv[5] : "armadillo_" + mode + ".ppm";

  const int W = 960;
  const int H = 960;

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  // The mesh SDF lives in the pool; the tape refers to it there.
  const auto t0 = Clock::now();
  // MESH=tri reads a TriMeshSDF (SoA triangles) instead of a MeshSDF (polygons); BOXBOUND=1 steps by the
  // distance to the tape's bounding box when farther than 0.1 from it.
  const bool useTri   = std::getenv("MESH") != nullptr && std::string(std::getenv("MESH")) == "tri";
  const bool boxBound = std::getenv("BOXBOUND") != nullptr && std::string(std::getenv("BOXBOUND")) == "1";
  const auto polygons = EBGeometry::Parser::readIntoMeshSDF<T>(file, pool);
  const auto tris     = EBGeometry::Parser::readIntoTriMeshSDF<T>(file, pool);
  const auto mesh     = polygons;
  const auto t1       = Clock::now();

  // Centre the Armadillo (151 units tall, y up) and scale it to about [-1, 1].
  const EBGeometry::Expr<T> armadillo =
    useTri ? EBGeometry::Scale(EBGeometry::Translate(tris, Vec3(T(-0.01), T(-21.43), T(0.01))), T(1) / T(75))
           : EBGeometry::Scale(EBGeometry::Translate(polygons, Vec3(T(-0.01), T(-21.43), T(0.01))), T(1) / T(75));

  // A tunnel through the chest from front to back, or a rod through the body from side to side.
  const EBGeometry::Expr<T> shape =
    (mode == "rod")
      ? EBGeometry::Union(armadillo, EBGeometry::CylinderSDF<T>(Vec3(-1.3, 0.25, 0.05), Vec3(1.3, 0.25, 0.05), T(0.09)))
      : EBGeometry::Difference(armadillo, EBGeometry::CylinderSDF<T>(Vec3(0, 0.25, -1.5), Vec3(0, 0.25, 1.5), T(0.16)));

  const EBGeometry::Tape<T> tape(pool, shape);

  pool.freeze();

  const bool haveDevice = EBGeometry::GPU::enabled && EBGeometry::GPU::deviceAvailable();

  Orbit cam;

  cam.yaw      = T(argc > 3 ? std::atof(argv[3]) : 200);
  cam.pitch    = T(argc > 4 ? std::atof(argv[4]) : 12);
  cam.distance = T(5.6);
  cam.target   = Vec3(0, T(-0.05), 0);
  cam.update();

  Settings settings;

  settings.floorY   = T(-1.01);
  settings.boxBound = boxBound;
  settings.boxLo    = tape.computeBoundingVolume().getLowCorner();
  settings.boxHi    = tape.computeBoundingVolume().getHighCorner();

  std::vector<uint32_t> pixels;

  const auto t2 = Clock::now();

  if (haveDevice) {
    const EBGeometry::Pool    devicePool = EBGeometry::Pool::mirror(pool, EBGeometry::GPU::memoryResource());
    const EBGeometry::Tape<T> deviceTape = tape.rebasedView(devicePool);

    std::vector<uint32_t> indices(size_t(W) * size_t(H));

    for (size_t k = 0; k < indices.size(); k++) {
      indices[k] = uint32_t(k);
    }

    renderDevice(Pixel<EBGeometry::Tape<T>>{{deviceTape, settings}, cam, W, H, 2}, indices, pixels);
  }
  else {
    renderHost(Pixel<EBGeometry::Tape<T>>{{tape, settings}, cam, W, H, 2}, pixels);
  }

  const auto t3 = Clock::now();

  std::FILE* f = std::fopen(out.c_str(), "wb");

  std::fprintf(f, "P6\n%d %d\n255\n", W, H);

  for (const uint32_t p : pixels) {
    const unsigned char rgb[3] = {uint8_t(p >> 24), uint8_t(p >> 16), uint8_t(p >> 8)};

    std::fwrite(rgb, 1, 3, f);
  }

  std::fclose(f);

  const auto ms = [](auto a, auto b) { return std::chrono::duration<double, std::milli>(b - a).count(); };

  std::printf("[%s, box bound %s] ", useTri ? "TriMeshSDF" : "MeshSDF", boxBound ? "on" : "off");
  std::printf("%s: %zu faces read + BVH built in %.0f ms; %dx%d 2x2 SS rendered on the %s in %.0f ms -> %s\n",
              mode.c_str(),
              size_t(mesh.getMesh().numFaces()),
              ms(t0, t1),
              W,
              H,
              haveDevice ? "GPU" : "CPU",
              ms(t2, t3),
              out.c_str());
}
