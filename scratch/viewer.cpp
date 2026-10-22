// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Scratch: interactive SDL2 viewer for the meshes in a folder of .obj files (by default the
// common-3d-test-models data), one at a time, centred and scaled to about [-1, 1] on a floor.
//
//   ./viewer [folder | file.obj]            view the meshes, starting at file.obj if given
//   ./viewer --snapshot [folder | file.obj] write <name>.ppm, ray traced, for the file or each mesh in
//                                           the folder, and exit
//
//   left drag    orbit             wheel    zoom
//   N / P        next / previous mesh (also Right / Left)
//   U            turn the mesh so that the next axis points up (+y, +z, -y, -z, +x, -x)
//   S / A / F    toggle shadows, ambient occlusion, floor
//   G            switch between GPU and CPU rendering (GPU builds)
//   M            switch between ray tracing, the hybrid and sphere tracing
//   R            reset the camera  Esc / Q  quit
//
// While the camera moves, frames are rendered at a quarter of the resolution without supersampling
// (ray tracing stays at full resolution); once it has been still for a moment, one full-resolution
// 2x2-supersampled frame replaces it. Open meshes render correctly by ray tracing only, since their
// signed distance has no meaningful sign.
#include "RayTracer.hpp"

#include <SDL.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace {

using Tracer::T;
using Tracer::Vec3;

using HybridT = RayTracer::Hybrid<EBGeometry::Tape<T>>;

// One mesh, ready for all three methods: a tape over a TriMeshSDF for sphere tracing, a BVH over its
// triangles for ray tracing, and a BVH over boxes covering its surface for the hybrid, all in one pool
// that is mirrored to the device.
struct Model
{
  std::string name;
  size_t      triangles = 0;
  Vec3        shift;
  T           scale  = 1;
  T           floorY = -1;

  std::unique_ptr<EBGeometry::Pool> pool;
  std::unique_ptr<EBGeometry::Pool> devicePool;

  std::optional<EBGeometry::Tape<T>>  tape;
  std::optional<RayTracer::Mesh::BVH> meshBVH;
  std::optional<HybridT::BoxBVH>      boxBVH;
};

// The names of the up axes load() takes, and the rotation that turns each into +y.
const char* upNames[6] = {"+y", "+z", "-y", "-z", "+x", "-x"};

Vec3
turnUp(const Vec3& v, const int a_up)
{
  switch (a_up) {
  case 1:
    return Vec3(v[0], v[2], -v[1]);
  case 2:
    return Vec3(v[0], -v[1], -v[2]);
  case 3:
    return Vec3(v[0], -v[2], v[1]);
  case 4:
    return Vec3(-v[1], v[0], v[2]);
  case 5:
    return Vec3(v[1], -v[0], v[2]);
  default:
    return v;
  }
}

// Reads a mesh file, turned so that axis a_up (an index into upNames) points up, and placed in the world
// as world = scale * (mesh + shift): its bounding box centred at the origin, its longest side 2 long.
// Throws if the file cannot be read.
std::unique_ptr<Model>
load(const std::string& a_file, const int a_up)
{
  using Clock = std::chrono::steady_clock;

  const auto start = Clock::now();

  auto m = std::make_unique<Model>();

  m->name = std::filesystem::path(a_file).stem().string();

  const auto file = EBGeometry::Parser::readIntoPolygonSoup<T>(a_file);

  std::vector<Vec3> turned;

  turned.reserve(file.getVertexCoordinates().size());

  for (const auto& v : file.getVertexCoordinates()) {
    turned.push_back(turnUp(v, a_up));
  }

  const EBGeometry::PolygonSoup<T> soup(m->name, std::move(turned), file.getFacets());

  const auto& verts = soup.getVertexCoordinates();

  if (verts.empty()) {
    throw std::runtime_error("no vertices");
  }

  Vec3 lo = verts[0];
  Vec3 hi = verts[0];

  for (const auto& v : verts) {
    lo = min(lo, v);
    hi = max(hi, v);
  }

  const Vec3 extent = hi - lo;

  m->shift  = T(-0.5) * (lo + hi);
  m->scale  = T(2) / std::max({extent[0], extent[1], extent[2]});
  m->floorY = T(-0.5) * m->scale * extent[1] - T(0.01);

  m->pool = std::make_unique<EBGeometry::Pool>(EBGeometry::hostMemoryResource());

  // Meshes with holes or other defects load with a warning; holes are fine for ray tracing.
  const auto triangles = soup.convertToTriangles(EBGeometry::Parser::OnDefect::Warn);

  m->triangles = triangles.size();

  const EBGeometry::TriMeshSDF<T,
                               EBGeometry::BVH::DefaultBranchingRatio<T>(),
                               EBGeometry::TriangleSoA::DefaultWidth<T>()>
    mesh(triangles, *m->pool, EBGeometry::BVH::Construction::SAH, size_t(4));

  m->tape.emplace(*m->pool, EBGeometry::Scale(EBGeometry::Translate(mesh, m->shift), m->scale));
  m->meshBVH.emplace(RayTracer::buildMesh(soup, *m->pool));

  std::vector<std::pair<RayTracer::AABB, RayTracer::AABB>> prims;

  for (const auto& b : RayTracer::meshLeafBoxes(soup, 128, m->shift, m->scale, T(1e-3))) {
    prims.emplace_back(b, b);
  }

  m->boxBVH.emplace(
    *m->pool, std::move(prims), EBGeometry::BVH::BinnedSAHPartitioner<T, RayTracer::AABB, RayTracer::AABB, 4>);

  m->pool->freeze();

  // In a host build GPU::memoryResource() is host memory and the mirror a plain copy.
  m->devicePool =
    std::make_unique<EBGeometry::Pool>(EBGeometry::Pool::mirror(*m->pool, EBGeometry::GPU::memoryResource()));

  const double ms = std::chrono::duration<double, std::milli>(Clock::now() - start).count();

  std::printf("%s (%s up): %zu triangles, loaded in %.0f ms\n", m->name.c_str(), upNames[a_up], m->triangles, ms);

  return m;
}

// The .obj files in a folder, sorted by name. a_start is set to the index of a_path if it is one of them.
std::vector<std::string>
listMeshes(const std::string& a_path, size_t& a_start)
{
  namespace fs = std::filesystem;

  const fs::path path(a_path);
  const fs::path dir = fs::is_directory(path) ? path : path.parent_path();

  std::vector<std::string> files;

  for (const auto& entry : fs::directory_iterator(dir.empty() ? fs::path(".") : dir)) {
    if (entry.is_regular_file() && entry.path().extension() == ".obj") {
      files.push_back(entry.path().string());
    }
  }

  std::sort(files.begin(), files.end());

  a_start = 0;

  for (size_t i = 0; i < files.size(); i++) {
    if (!fs::is_directory(path) && fs::equivalent(files[i], path)) {
      a_start = i;
    }
  }

  return files;
}

// The camera a mesh starts with: a three-quarter view from slightly above.
Tracer::Orbit
homeCamera()
{
  Tracer::Orbit cam;

  cam.yaw      = 35;
  cam.pitch    = 20;
  cam.distance = T(5.2);
  cam.target   = Vec3::zeros();

  return cam;
}

} // namespace

int
main(int argc, char* argv[])
{
  using namespace Tracer;
  using Clock = std::chrono::steady_clock;

  const int W       = 960;
  const int H       = 720;
  const int Preview = 4; // Downscale factor while moving.

  int argi = 1;

  const bool snapshot = argc > argi && std::string(argv[argi]) == "--snapshot";

  argi += snapshot ? 1 : 0;

  const std::string path = argc > argi ? argv[argi] : "../common-3d-test-models/data";

  if (!std::filesystem::exists(path)) {
    std::fprintf(stderr, "usage: %s [--snapshot] [folder | file.obj]\n  %s: not found\n", argv[0], path.c_str());

    return 1;
  }

  size_t                         current = 0;
  const std::vector<std::string> files   = listMeshes(path, current);

  if (files.empty()) {
    std::fprintf(stderr, "no .obj files in %s\n", path.c_str());

    return 1;
  }

  // The device renders views of the model's structures that resolve against the mirrored pool.
  const bool haveDevice = EBGeometry::GPU::enabled && EBGeometry::GPU::deviceAvailable();

  std::printf("backend: %s, device %s\n", EBGeometry::GPU::backendName, haveDevice ? "available" : "not available");

  std::vector<uint32_t> fullIndices(size_t(W) * size_t(H));
  std::vector<uint32_t> previewIndices(size_t(W / Preview) * size_t(H / Preview));

  for (size_t k = 0; k < fullIndices.size(); k++) {
    fullIndices[k] = uint32_t(k);
  }

  for (size_t k = 0; k < previewIndices.size(); k++) {
    previewIndices[k] = uint32_t(k);
  }

  Settings settings;

  std::vector<uint32_t> pixels;

  // Renders one frame of a_model into pixels: method 0 sphere tracing, 1 ray tracing, 2 hybrid.
  const auto render =
    [&](const Model& a_model, const Orbit& a_cam, const int a_method, const bool a_gpu, const int w, const int h, const int ss) {
      const std::vector<uint32_t>& indices = (w == W) ? fullIndices : previewIndices;
      const EBGeometry::Pool&      usePool = a_gpu ? *a_model.devicePool : *a_model.pool;
      const EBGeometry::Tape<T>    tape    = a_model.tape->rebasedView(usePool);

      Settings shape = settings;

      shape.floorY   = a_model.floorY;
      shape.boxBound = true;
      shape.boxLo    = tape.computeBoundingVolume().getLowCorner();
      shape.boxHi    = tape.computeBoundingVolume().getHighCorner();

      // The display settings without the sphere tracer's box skip, and the shape alone for the hybrid.
      Settings display = shape;

      display.boxBound = false;

      Settings shapeOnly = display;

      shapeOnly.floor = false;

      const auto draw = [&](const auto& a_pixel) {
        pixels.resize(indices.size());

        if (a_gpu) {
          EBGeometry::GPU::transform(a_pixel, indices, pixels);
        }
        else {
          const int n = int(indices.size());

#pragma omp parallel for schedule(dynamic, 256)
          for (int k = 0; k < n; k++) {
            pixels[size_t(k)] = a_pixel(uint32_t(k));
          }
        }
      };

      if (a_method == 1) {
        const RayTracer::Mesh mesh{a_model.meshBVH->rebasedView(usePool), a_model.shift, a_model.scale};

        draw(RayTracer::Pixel<RayTracer::Scene>{{mesh, RayTracer::Cylinder{}, false, display, true}, a_cam, w, h, ss});
      }
      else if (a_method == 2) {
        const HybridT hybrid{a_model.boxBVH->rebasedView(usePool), Scene<EBGeometry::Tape<T>>{tape, shapeOnly}, display};

        draw(RayTracer::Pixel<HybridT>{hybrid, a_cam, w, h, ss});
      }
      else {
        draw(Pixel<EBGeometry::Tape<T>>{{tape, shape}, a_cam, w, h, ss});
      }
    };

  if (snapshot) {
    Orbit cam = homeCamera();

    cam.update();

    const bool one = !std::filesystem::is_directory(path);

    for (const auto& file : one ? std::vector<std::string>{files[current]} : files) {
      try {
        const auto model = load(file, 0);

        render(*model, cam, 1, haveDevice, W, H, 2);

        const std::string out = model->name + ".ppm";

        FILE* f = std::fopen(out.c_str(), "wb");

        std::fprintf(f, "P6\n%d %d\n255\n", W, H);

        for (const uint32_t p : pixels) {
          const unsigned char rgb[3] = {
            static_cast<unsigned char>(p >> 24), static_cast<unsigned char>(p >> 16), static_cast<unsigned char>(p >> 8)};

          std::fwrite(rgb, 1, 3, f);
        }

        std::fclose(f);
        std::printf("  wrote %s\n", out.c_str());
      }
      catch (const std::exception& e) {
        std::fprintf(stderr, "%s: %s\n", file.c_str(), e.what());
      }
    }

    return 0;
  }

  // Loads files[a_index], or the next one after it that loads, in direction a_step. Each file keeps the
  // up axis it was last shown with.
  std::unique_ptr<Model> model;
  std::vector<int>       up(files.size(), 0);

  const auto open = [&](size_t a_index, const int a_step) {
    for (size_t tries = 0; tries < files.size(); tries++) {
      try {
        model   = load(files[a_index], up[a_index]);
        current = a_index;

        return true;
      }
      catch (const std::exception& e) {
        std::fprintf(stderr, "%s: %s\n", files[a_index].c_str(), e.what());
      }

      a_index = (a_index + files.size() + size_t(a_step)) % files.size();
    }

    return false;
  };

  if (!open(current, 1)) {
    return 1;
  }

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());

    return 1;
  }

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"); // Bilinear upscaling of the preview.

  SDL_Window* window =
    SDL_CreateWindow("EBGeometry viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W, H, SDL_WINDOW_SHOWN);
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  if (window == nullptr || renderer == nullptr) {
    std::fprintf(stderr, "SDL: %s\n", SDL_GetError());

    return 1;
  }

  SDL_Texture* full = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, W, H);
  SDL_Texture* preview =
    SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, W / Preview, H / Preview);

  Orbit cam = homeCamera();

  bool dragging  = false;
  bool moved     = true; // Something changed since the last full-resolution frame.
  bool sharp     = false;
  auto lastInput = Clock::now();
  bool running   = true;
  bool useGPU    = haveDevice;
  int  method    = 1; // 0 sphere tracing, 1 ray tracing, 2 hybrid.

  const char* methodNames[3] = {"sphere tracing", "ray tracing", "hybrid"};

  while (running) {
    // Block while nothing needs drawing, so an idle viewer uses no CPU.
    SDL_Event event;

    const bool haveEvent = (!moved && sharp) ? SDL_WaitEvent(&event) != 0 : SDL_PollEvent(&event) != 0;

    if (haveEvent) {
      do {
        switch (event.type) {
        case SDL_QUIT: {
          running = false;

          break;
        }
        case SDL_MOUSEBUTTONDOWN: {
          dragging = dragging || event.button.button == SDL_BUTTON_LEFT;

          break;
        }
        case SDL_MOUSEBUTTONUP: {
          dragging = dragging && event.button.button != SDL_BUTTON_LEFT;

          break;
        }
        case SDL_MOUSEMOTION: {
          if (dragging) {
            cam.yaw -= T(0.4) * T(event.motion.xrel);
            cam.pitch = Math::clamp(cam.pitch + T(0.4) * T(event.motion.yrel), T(-89), T(89));
            moved     = true;
          }

          break;
        }
        case SDL_MOUSEWHEEL: {
          cam.distance = Math::clamp(cam.distance * std::pow(T(0.9), T(event.wheel.y)), T(1.5), T(30));
          moved        = true;

          break;
        }
        case SDL_KEYDOWN: {
          switch (event.key.keysym.sym) {
          case SDLK_ESCAPE:
          case SDLK_q: {
            running = false;

            break;
          }
          case SDLK_n:
          case SDLK_RIGHT:
          case SDLK_p:
          case SDLK_LEFT: {
            const bool forward = event.key.keysym.sym == SDLK_n || event.key.keysym.sym == SDLK_RIGHT;
            const int  step    = forward ? 1 : -1;

            SDL_SetWindowTitle(window, "EBGeometry viewer  |  loading...");
            open((current + files.size() + size_t(step)) % files.size(), step);

            moved = true;

            break;
          }
          case SDLK_u: {
            up[current] = (up[current] + 1) % 6;

            SDL_SetWindowTitle(window, "EBGeometry viewer  |  loading...");
            open(current, 1);

            moved = true;

            break;
          }
          case SDLK_s: {
            settings.shadows = !settings.shadows;
            moved            = true;

            break;
          }
          case SDLK_a: {
            settings.occlusion = !settings.occlusion;
            moved              = true;

            break;
          }
          case SDLK_f: {
            settings.floor = !settings.floor;
            moved          = true;

            break;
          }
          case SDLK_m: {
            method = (method + 1) % 3;
            moved  = true;

            break;
          }
          case SDLK_g: {
            useGPU = haveDevice && !useGPU;
            moved  = true;

            break;
          }
          case SDLK_r: {
            cam   = homeCamera();
            moved = true;

            break;
          }
          default: {
            break;
          }
          }

          break;
        }
        default: {
          break;
        }
        }
      } while (SDL_PollEvent(&event) != 0);
    }

    if (moved) {
      lastInput = Clock::now();
      sharp     = false;
    }

    const bool settled = !dragging && (Clock::now() - lastInput) > std::chrono::milliseconds(150);

    if (!moved && (sharp || !settled)) {
      SDL_Delay(5);

      continue;
    }

    cam.update();

    // A preview while the camera moves, one sharp frame once it has settled.
    // Ray tracing is fast enough to stay at full resolution while moving (without supersampling).
    const bool hq      = settled && !moved;
    const bool fullRes = hq || method == 1;
    const int  w       = fullRes ? W : W / Preview;
    const int  h       = fullRes ? H : H / Preview;
    const int  ss      = hq ? 2 : 1;

    SDL_Texture* texture = fullRes ? full : preview;

    const auto start = Clock::now();

    render(*model, cam, method, useGPU, w, h, ss);

    const double ms = std::chrono::duration<double, std::milli>(Clock::now() - start).count();

    SDL_UpdateTexture(texture, nullptr, pixels.data(), w * int(sizeof(uint32_t)));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);

    char title[300];

    std::snprintf(title,
                  sizeof(title),
                  "%s (%zu/%zu, %zu triangles, %s up)  |  %s, %s  %dx%d%s  %.1f ms  |  shadows %s, AO %s, floor %s",
                  model->name.c_str(),
                  current + 1,
                  files.size(),
                  model->triangles,
                  upNames[up[current]],
                  methodNames[method],
                  useGPU ? "GPU" : "CPU",
                  w,
                  h,
                  hq ? " (2x2 SS)" : (fullRes ? " (moving)" : " (preview)"),
                  ms,
                  settings.shadows ? "on" : "off",
                  settings.occlusion ? "on" : "off",
                  settings.floor ? "on" : "off");
    SDL_SetWindowTitle(window, title);

    sharp = hq;
    moved = false;
  }

  SDL_DestroyTexture(preview);
  SDL_DestroyTexture(full);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
