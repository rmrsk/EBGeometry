// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Scratch: interactive SDL2 viewer for the classic CSG solid, or the Armadillo pierced by a cylinder.
//
//   ./viewer                                classic CSG solid
//   ./viewer tunnel <armadillo.obj>         Armadillo with a tunnel through the chest (Difference)
//   ./viewer rod    <armadillo.obj>         Armadillo with a rod through the body (Union)
//
//   left drag    orbit             wheel    zoom
//   S / A / F    toggle shadows, ambient occlusion, floor
//   G            switch between GPU and CPU rendering (GPU builds)
//   R            reset the camera  Esc / Q  quit
//
// While the camera moves, frames are rendered at a quarter of the resolution without supersampling;
// once it has been still for a moment, one full-resolution 2x2-supersampled frame replaces it.
#include "Tracer.hpp"

#include <SDL.h>

#include <chrono>
#include <cstdio>
#include <string>

int
main(int argc, char* argv[])
{
  using namespace Tracer;
  using Clock = std::chrono::steady_clock;

  const int W       = 960;
  const int H       = 720;
  const int Preview = 4; // Downscale factor while moving.

  const std::string scene  = argc > 1 ? argv[1] : "csg";
  const bool        isMesh = (scene == "tunnel" || scene == "rod");

  if ((scene != "csg" && !isMesh) || (isMesh && argc < 3)) {
    std::fprintf(stderr, "usage: %s [csg | tunnel <armadillo.obj> | rod <armadillo.obj>]\n", argv[0]);

    return 1;
  }

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  Orbit    home;
  Settings settings;

  // The Armadillo (151 units tall, y up) is read into a TriMeshSDF in the pool, centred and scaled to about
  // [-1, 1], and pierced by a cylinder. Far from it, the tracer steps by the distance to the tape's box.
  const auto build = [&]() -> EBGeometry::Expr<T> {
    if (!isMesh) {
      return classicCSG();
    }

    const auto mesh = EBGeometry::Parser::readIntoTriMeshSDF<T>(argv[2], pool);

    const EBGeometry::Expr<T> armadillo =
      EBGeometry::Scale(EBGeometry::Translate(mesh, Vec3(T(-0.01), T(-21.43), T(0.01))), T(1) / T(75));

    home.yaw          = 200;
    home.pitch        = 12;
    home.distance     = T(5.6);
    home.target       = Vec3(0, T(-0.05), 0);
    settings.floorY   = T(-1.01);
    settings.boxBound = true;

    if (scene == "rod") {
      return EBGeometry::Union(armadillo,
                               EBGeometry::CylinderSDF<T>(Vec3(-1.3, 0.25, 0.05), Vec3(1.3, 0.25, 0.05), T(0.09)));
    }

    return EBGeometry::Difference(armadillo,
                                  EBGeometry::CylinderSDF<T>(Vec3(0, 0.25, -1.5), Vec3(0, 0.25, 1.5), T(0.16)));
  };

  const EBGeometry::Tape<T> tape(pool, build());

  settings.boxLo = tape.computeBoundingVolume().getLowCorner();
  settings.boxHi = tape.computeBoundingVolume().getHighCorner();

  // The device renders a view of the same tape that resolves against a mirror of the pool in device
  // memory. In a host build GPU::memoryResource() is host memory and GPU::transform() a host loop.
  const bool haveDevice = EBGeometry::GPU::enabled && EBGeometry::GPU::deviceAvailable();

  pool.freeze();

  const EBGeometry::Pool    devicePool = EBGeometry::Pool::mirror(pool, EBGeometry::GPU::memoryResource());
  const EBGeometry::Tape<T> deviceTape = tape.rebasedView(devicePool);

  std::vector<uint32_t> fullIndices(size_t(W) * size_t(H));
  std::vector<uint32_t> previewIndices(size_t(W / Preview) * size_t(H / Preview));

  for (size_t k = 0; k < fullIndices.size(); k++) {
    fullIndices[k] = uint32_t(k);
  }

  for (size_t k = 0; k < previewIndices.size(); k++) {
    previewIndices[k] = uint32_t(k);
  }

  std::printf("backend: %s, device %s\n", EBGeometry::GPU::backendName, haveDevice ? "available" : "not available");

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

  Orbit cam = home;

  cam.update();

  std::vector<uint32_t> pixels;

  bool dragging  = false;
  bool moved     = true; // Something changed since the last full-resolution frame.
  bool sharp     = false;
  auto lastInput = Clock::now();
  bool running   = true;
  bool useGPU    = haveDevice;

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
          case SDLK_g: {
            useGPU = haveDevice && !useGPU;
            moved  = true;

            break;
          }
          case SDLK_r: {
            cam   = home;
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
    const bool hq = settled && !moved;
    const int  w  = hq ? W : W / Preview;
    const int  h  = hq ? H : H / Preview;

    SDL_Texture* texture = hq ? full : preview;

    const auto start = Clock::now();

    if (useGPU) {
      const Pixel<EBGeometry::Tape<T>> pixel{{deviceTape, settings}, cam, w, h, hq ? 2 : 1};

      renderDevice(pixel, hq ? fullIndices : previewIndices, pixels);
    }
    else {
      const Pixel<EBGeometry::Tape<T>> pixel{{tape, settings}, cam, w, h, hq ? 2 : 1};

      renderHost(pixel, pixels);
    }

    const double ms = std::chrono::duration<double, std::milli>(Clock::now() - start).count();

    SDL_UpdateTexture(texture, nullptr, pixels.data(), w * int(sizeof(uint32_t)));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);

    char title[160];

    std::snprintf(title,
                  sizeof(title),
                  "EBGeometry viewer  |  %s  %dx%d%s  %.1f ms  |  shadows %s, AO %s, floor %s",
                  useGPU ? "GPU" : "CPU",
                  w,
                  h,
                  hq ? " (2x2 SS)" : " (preview)",
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
