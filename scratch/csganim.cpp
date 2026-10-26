// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Scratch: an animation of CSG on a composite shape, for the README. A fixed camera watches the
// xyzrgb dragon revealed by an intersection, set on a table by a union that turns smooth, and cut open
// by a difference. Writes frame_NNNN.ppm and captions.txt (frame, caption) into the current
// directory; make_csg_gif.py turns them into a GIF.
//
//   ./csganim <xyzrgb_dragon.obj> [width] [height] [supersampling]
#include "Tracer.hpp"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

// Tuning knobs, overridable with -D while composing the shot.
#ifndef CAM_YAW
#define CAM_YAW 24
#endif
#ifndef CAM_PITCH
#define CAM_PITCH 17
#endif
#ifndef CAM_DIST
#define CAM_DIST 4.7
#endif
#ifndef CAM_TY
#define CAM_TY 0.32
#endif
#ifndef CUT_Z
#define CUT_Z 0.05
#endif

namespace {

using Tracer::T;
using Tracer::Vec3;

// Smooth step from 0 to 1 as a_x goes from 0 to 1, for easing the motions.
T
ease(const T a_x)
{
  const T x = EBGeometry::Math::clamp(a_x, T(0), T(1));

  return x * x * (T(3) - T(2) * x);
}

// What one frame shows, and its parts on their own, for colouring.
struct Frame
{
  EBGeometry::Expr<T> shape;
  EBGeometry::Expr<T> table;
  EBGeometry::Expr<T> cutter;
  std::string         caption;
};

// Colours a surface point by the part it lies on: dragon or table, mixed across the smooth union's
// blend, and the faces a cutter leaves.
struct Parts
{
  EBGeometry::Tape<T> dragon;
  EBGeometry::Tape<T> table;
  EBGeometry::Tape<T> cutter;

  EBGEOMETRY_HOST_DEVICE
  Vec3
  albedo(const Vec3& p, const Vec3& a_default) const noexcept
  {
    const T dd = dragon.signedDistance(p);
    const T dt = table.signedDistance(p);
    const T dc = cutter.signedDistance(p);

    const T toTable = EBGeometry::Math::clamp(T(0.5) + (dd - dt) / T(0.12), T(0), T(1));
    const T onCut   = EBGeometry::Math::clamp(T(1) - std::abs(dc) / T(0.01), T(0), T(1));

    const Vec3 c = (T(1) - toTable) * a_default + toTable * Vec3(0.10, 0.28, 0.50);

    return (T(1) - onCut) * c + onCut * Vec3(0.85, 0.62, 0.10);
  }
};

} // namespace

int
main(int argc, char* argv[])
{
  using namespace Tracer;
  using Clock = std::chrono::steady_clock;

  if (argc < 2) {
    std::fprintf(stderr, "usage: %s <xyzrgb_dragon.obj> [width] [height] [supersampling]\n", argv[0]);

    return 1;
  }

  const int W  = argc > 2 ? std::atoi(argv[2]) : 800;
  const int H  = argc > 3 ? std::atoi(argv[3]) : 450;
  const int SS = argc > 4 ? std::atoi(argv[4]) : 2;

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const auto t0 = Clock::now();

  const auto mesh = EBGeometry::Parser::readIntoTriMeshSDF<T>(
    argv[1], pool, EBGeometry::BVH::Construction::SAH, EBGeometry::Parser::OnDefect::Warn);

  // Scale the dragon to 2.4 along its longest side and stand it with its feet on y = 0, centred in x and z.
  const auto lo    = mesh.computeBoundingVolume().getLowCorner();
  const auto hi    = mesh.computeBoundingVolume().getHighCorner();
  const Vec3 ext   = hi - lo;
  const T    scale = T(2.4) / Math::max(ext[0], Math::max(ext[1], ext[2]));
  const Vec3 shift(T(-0.5) * (lo[0] + hi[0]), -lo[1], T(-0.5) * (lo[2] + hi[2]));
  const T    top   = scale * ext[1]; // The dragon's height above the table.

  const EBGeometry::Expr<T> dragon = EBGeometry::Scale(EBGeometry::Translate(mesh, shift), scale);

  std::printf("dragon read in %.0f ms, %.2f x %.2f x %.2f after scaling\n",
              std::chrono::duration<double, std::milli>(Clock::now() - t0).count(),
              scale * ext[0],
              scale * ext[1],
              scale * ext[2]);

  // The table: a rounded top on four legs, the top's upper face at y = 0, the legs down to the floor.
  const T floorY = T(-0.75);
  const T hx     = T(0.5) * scale * ext[0] + T(0.15);
  const T hz     = T(0.5) * scale * ext[2] + T(0.15);

  const auto table = [&](const T a_lift) -> EBGeometry::Expr<T> {
    std::vector<EBGeometry::Expr<T>> parts;

    parts.push_back(EBGeometry::RoundedBoxSDF<T>(Vec3(0, T(-0.06), 0), Vec3(2 * hx, T(0.12), 2 * hz), T(0.03)));

    for (const T sx : {T(-1), T(1)}) {
      for (const T sz : {T(-1), T(1)}) {
        const Vec3 foot(sx * (hx - T(0.12)), floorY, sz * (hz - T(0.12)));

        parts.push_back(
          EBGeometry::CylinderSDF<T>(foot, foot + Vec3(0, -floorY - T(0.06), 0), T(0.06)));
      }
    }

    return EBGeometry::Translate(EBGeometry::Union(parts), Vec3(0, a_lift, 0));
  };

  const T fillet = T(0.3); // The final smoothing length; the polynomial blend deepens by at most a quarter of it.

  // The cutter: a box on the camera's side of the plane z = c, with c moving in from beyond the table's
  // near edge to just past the middle.
  const T cutA = hz + T(0.05);
  const T cutB = T(CUT_Z);

  // Parts that are not in the frame yet: the table below the floor, the cutter far away.
  const EBGeometry::Expr<T> hiddenTable = table(floorY - T(0.3));
  const EBGeometry::Expr<T> noCutter    = EBGeometry::SphereSDF<T>(Vec3(100, 100, 100), T(0.1));

  std::vector<Frame> frames;

  const auto hold = [&](const int a_n) {
    for (int i = 0; i < a_n; i++) {
      frames.push_back(frames.back());
    }
  };

  char buf[200];

  // 1. Reveal: the dragon intersected with a half-space whose top rises through it.
  for (int i = 0; i <= 40; i++) {
    const T y = T(-0.02) + (top + T(0.04)) * ease(T(i) / T(40));

    const EBGeometry::Expr<T> cut =
      EBGeometry::BoxSDF<T>(Vec3(-10, -10, -10), Vec3(10, y, 10));

    frames.push_back({EBGeometry::Intersection(dragon, cut), hiddenTable, noCutter, "Intersection(dragon, box)"});
  }

  hold(10);

  // 2. The table rises from below the floor and joins the dragon by a union.
  for (int i = 0; i <= 30; i++) {
    const T lift = (floorY - T(0.3)) * (T(1) - ease(T(i) / T(30)));

    frames.push_back({EBGeometry::Union(dragon, table(lift)), table(lift), noCutter, "Union(dragon, table)"});
  }

  hold(10);

  // 3. The union turns smooth: a fillet grows where the feet meet the table.
  for (int i = 1; i <= 30; i++) {
    const T k = fillet * ease(T(i) / T(30));

    std::snprintf(buf, sizeof(buf), "SmoothUnion(dragon, table, %.2f)", double(k));
    frames.push_back({EBGeometry::SmoothUnion(dragon, table(0), k), table(0), noCutter, buf});
  }

  hold(10);

  const EBGeometry::Expr<T> joined = EBGeometry::SmoothUnion(dragon, table(0), fillet);

  // 4. A section: the difference with a box that slices off the near part, showing the cut faces.
  for (int i = 1; i <= 45; i++) {
    const T z = cutA + (cutB - cutA) * ease(T(i) / T(45));

    const EBGeometry::Expr<T> box = EBGeometry::BoxSDF<T>(Vec3(-10, -10, z), Vec3(10, 10, 10));

    frames.push_back({EBGeometry::Difference(joined, box), table(0), box, "Difference(SmoothUnion(...), box)"});
  }

  hold(30);

  // Tapes for every frame and its parts, all in the pool with the dragon; then mirror the pool to the
  // device once.
  const EBGeometry::Tape<T> dragonTape(pool, dragon);

  std::vector<EBGeometry::Tape<T>> tapes;
  std::vector<EBGeometry::Tape<T>> tableTapes;
  std::vector<EBGeometry::Tape<T>> cutterTapes;

  for (const auto& f : frames) {
    tapes.emplace_back(pool, f.shape);
    tableTapes.emplace_back(pool, f.table);
    cutterTapes.emplace_back(pool, f.cutter);
  }

  pool.freeze();

  const bool haveDevice = EBGeometry::GPU::enabled && EBGeometry::GPU::deviceAvailable();

  const EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(pool, EBGeometry::GPU::memoryResource());

  std::printf("%zu frames, %dx%d, %dx%d supersampling, on the %s\n",
              frames.size(),
              W,
              H,
              SS,
              SS,
              haveDevice ? "GPU" : "CPU");

  Orbit cam;

  cam.yaw      = T(CAM_YAW);
  cam.pitch    = T(CAM_PITCH);
  cam.distance = T(CAM_DIST);
  cam.focal    = T(2.9);
  cam.target   = Vec3(0, T(CAM_TY), 0);
  cam.update();

  Settings settings;

  settings.floorY = floorY;

  std::vector<uint32_t> indices(size_t(W) * size_t(H));
  std::vector<uint32_t> pixels;

  for (size_t k = 0; k < indices.size(); k++) {
    indices[k] = uint32_t(k);
  }

  FILE* captions = std::fopen("captions.txt", "w");

  const auto start = Clock::now();

  // ONLY=20,75,140 renders just those frames, while composing the shot.
  std::vector<bool> wanted(frames.size(), std::getenv("ONLY") == nullptr);

  if (std::getenv("ONLY") != nullptr) {
    for (const char* c = std::getenv("ONLY"); *c != '\0';) {
      char*        end = nullptr;
      const size_t f   = size_t(std::strtoul(c, &end, 10));

      if (f < frames.size()) {
        wanted[f] = true;
      }

      c = (*end == ',') ? end + 1 : end;

      if (end == c && *c != '\0') {
        break;
      }
    }
  }

  for (size_t f = 0; f < frames.size(); f++) {
    if (!wanted[f]) {
      continue;
    }

    const auto view = [&](const EBGeometry::Tape<T>& a_tape) {
      return haveDevice ? a_tape.rebasedView(devicePool) : a_tape;
    };

    const Parts parts{view(dragonTape), view(tableTapes[f]), view(cutterTapes[f])};

    Settings s = settings;

    s.boxBound = true;
    s.boxLo    = tapes[f].computeBoundingVolume().getLowCorner();
    s.boxHi    = tapes[f].computeBoundingVolume().getHighCorner();

    const Pixel<EBGeometry::Tape<T>, Parts> pixel{{view(tapes[f]), s, parts}, cam, W, H, SS};

    if (haveDevice) {
      renderDevice(pixel, indices, pixels);
    }
    else {
      renderHost(pixel, pixels);
    }

    std::snprintf(buf, sizeof(buf), "frame_%04zu.ppm", f);

    FILE* out = std::fopen(buf, "wb");

    std::fprintf(out, "P6\n%d %d\n255\n", W, H);

    for (const uint32_t p : pixels) {
      const unsigned char rgb[3] = {
        static_cast<unsigned char>(p >> 24), static_cast<unsigned char>(p >> 16), static_cast<unsigned char>(p >> 8)};

      std::fwrite(rgb, 1, 3, out);
    }

    std::fclose(out);
    std::fprintf(captions, "%zu\t%s\n", f, frames[f].caption.c_str());

    if (f % 20 == 0) {
      std::printf("  frame %zu, %.1f s\n", f, std::chrono::duration<double>(Clock::now() - start).count());
      std::fflush(stdout);
    }
  }

  std::fclose(captions);
  std::printf("done in %.1f s\n", std::chrono::duration<double>(Clock::now() - start).count());

  return 0;
}
