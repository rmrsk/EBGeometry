// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_ViewerImplem.hpp
 * @brief  Implementation of EBGeometry_Viewer.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_VIEWERIMPLEM_HPP
#define EBGEOMETRY_VIEWERIMPLEM_HPP

// Std includes
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef EBGEOMETRY_VIEWER_SDL2
#include <chrono>
#include <string>

// The program keeps its own main(); SDL must not replace it.
#ifndef SDL_MAIN_HANDLED
#define SDL_MAIN_HANDLED
#endif
#include <SDL.h>
#endif

// Our includes
#include "EBGeometry_GPUTransform.hpp"
#include "EBGeometry_Viewer.hpp"

namespace EBGeometry::Render {

namespace ViewerDetail {

/**
 * @brief Whether the program's arguments ask for no window.
 * @details Internal helper; not part of the public API.
 * @param[in] a_argc Argument count.
 * @param[in] a_argv Arguments.
 * @return True if one of them is "--no-view".
 */
inline bool
noViewRequested(const int a_argc, char** a_argv) noexcept
{
  for (int i = 1; i < a_argc && a_argv != nullptr; i++) {
    if (a_argv[i] != nullptr && std::strcmp(a_argv[i], "--no-view") == 0) {
      return true;
    }
  }

  return false;
}

/**
 * @brief Whether a window could open: on Linux and other X11/Wayland systems, whether a display is set.
 * @details Internal helper; not part of the public API.
 * @return True if a display is (or may be) available.
 */
inline bool
displayAvailable() noexcept
{
#if defined(__unix__) && !defined(__APPLE__)
  const char* x11     = std::getenv("DISPLAY");
  const char* wayland = std::getenv("WAYLAND_DISPLAY");

  return (x11 != nullptr && x11[0] != '\0') || (wayland != nullptr && wayland[0] != '\0');
#else
  return true;
#endif
}

#ifdef EBGEOMETRY_VIEWER_SDL2

/**
 * @brief The window's event loop.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in] a_shape       Shape, valid on the host.
 * @param[in] a_deviceShape Device view of it, or null.
 * @param[in] a_camera      Initial camera.
 * @param[in] a_settings    Settings.
 */
template <class T, class S>
inline void
run(const S& a_shape, const S* a_deviceShape, const Camera<T>& a_camera, const Settings<T>& a_settings)
{
  SDL_SetMainReady();

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    std::printf("EBGeometry: the viewer could not start SDL (%s), so it does not open.\n", SDL_GetError());

    return;
  }

  const int width  = static_cast<int>(a_settings.m_width);
  const int height = static_cast<int>(a_settings.m_height);

  SDL_Window* window =
    SDL_CreateWindow("EBGeometry", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_RESIZABLE);
  SDL_Renderer* screen = (window != nullptr) ? SDL_CreateRenderer(window, -1, 0) : nullptr;

  if (window == nullptr || screen == nullptr) {
    std::printf("EBGeometry: the viewer could not open a window (%s).\n", SDL_GetError());

    if (window != nullptr) {
      SDL_DestroyWindow(window);
    }

    SDL_Quit();

    return;
  }

  std::printf("EBGeometry viewer: drag to orbit, wheel to zoom; S shadows, A occlusion, F floor, P colour by "
              "primitive, X section (arrows move it)%s, R reset, Esc or Q to close.\n",
              (a_deviceShape != nullptr) ? ", G GPU/CPU" : "");

  Camera<T>   camera   = a_camera;
  Settings<T> settings = a_settings;

  // The scene's length scale, for moving the section plane.
  const auto box   = a_shape.computeBoundingVolume();
  const T    diag  = (box.getHighCorner() - box.getLowCorner()).length();
  const T    scale = (std::isfinite(diag) && diag > T(0)) ? diag : camera.m_distance;

  T sectionOffset = T(0);

  bool onDevice   = a_deviceShape != nullptr && GPU::deviceAvailable();
  bool dragging   = false;
  bool fullFrame  = false;
  bool warned     = false;
  bool running    = true;
  bool needRedraw = true;

  auto lastChange = std::chrono::steady_clock::now();

  SDL_Texture* texture    = nullptr;
  uint32_t     texWidth   = 0;
  uint32_t     texHeight  = 0;
  double       lastFrameS = 0;

  while (running) {
    SDL_Event event;

    bool changed = false;

    while (SDL_PollEvent(&event) != 0) {
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
          camera.m_yaw -= T(0.4) * T(event.motion.xrel);
          camera.m_pitch = Math::clamp(camera.m_pitch + T(0.4) * T(event.motion.yrel), T(-89), T(89));

          changed = true;
        }

        break;
      }
      case SDL_MOUSEWHEEL: {
        camera.m_distance *= (event.wheel.y > 0) ? T(0.9) : T(1) / T(0.9);

        changed = true;

        break;
      }
      case SDL_WINDOWEVENT: {
        needRedraw = true;

        break;
      }
      case SDL_KEYDOWN: {
        changed = true;

        switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
        case SDLK_q: {
          running = false;

          break;
        }
        case SDLK_s: {
          settings.m_shadows = !settings.m_shadows;

          break;
        }
        case SDLK_a: {
          settings.m_occlusion = !settings.m_occlusion;

          break;
        }
        case SDLK_f: {
          settings.m_floor = !settings.m_floor;

          break;
        }
        case SDLK_p: {
          settings.m_colourBy = (settings.m_colourBy == ColourBy::Uniform) ? ColourBy::Primitive : ColourBy::Uniform;

          break;
        }
        case SDLK_x: {
          settings.m_section = !settings.m_section;
          sectionOffset      = T(0);

          break;
        }
        case SDLK_UP: {
          sectionOffset += T(0.02) * scale;

          break;
        }
        case SDLK_DOWN: {
          sectionOffset -= T(0.02) * scale;

          break;
        }
        case SDLK_g: {
          onDevice = !onDevice && a_deviceShape != nullptr && GPU::deviceAvailable();

          break;
        }
        case SDLK_r: {
          camera = a_camera;

          break;
        }
        default: {
          changed = false;

          break;
        }
        }

        break;
      }
      default: {
        break;
      }
      }
    }

    if (changed) {
      lastChange = std::chrono::steady_clock::now();
      fullFrame  = false;
    }

    // Draw a quick frame while the camera moves, and the full one once it has rested for a moment.
    const bool resting = !dragging && std::chrono::steady_clock::now() - lastChange > std::chrono::milliseconds(150);

    const bool drawQuick = changed || (needRedraw && texture == nullptr);
    const bool drawFull  = resting && !fullFrame;

    if (running && (drawQuick || drawFull)) {
      Settings<T> frame = settings;

      // The section plane faces the camera through its target, moved by the arrow keys.
      Vec3T<T> forward;
      Vec3T<T> right;
      Vec3T<T> up;

      camera.frame(forward, right, up);

      frame.m_sectionNormal = -forward;
      frame.m_sectionPoint  = camera.m_target - sectionOffset * forward;

      if (!drawFull) {
        frame.m_width         = Math::max(1U, settings.m_width / 2U);
        frame.m_height        = Math::max(1U, settings.m_height / 2U);
        frame.m_supersampling = 1;
        frame.m_shadows       = false;
        frame.m_occlusion     = false;
      }

      const Renderer<T> renderer(frame);

      Image image;

      const auto start = std::chrono::steady_clock::now();

      const Stats stats =
        onDevice ? renderer.renderOnDevice(*a_deviceShape, camera, image) : renderer.render(a_shape, camera, image);

      lastFrameS = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();

      if (stats.m_cameraInside && !warned) {
        std::printf("EBGeometry viewer: the camera is inside the solid, so what it sees is seen from inside "
                    "(tinted red).\n");

        warned = true;
      }

      if (texture == nullptr || texWidth != image.m_width || texHeight != image.m_height) {
        if (texture != nullptr) {
          SDL_DestroyTexture(texture);
        }

        texture   = SDL_CreateTexture(screen,
                                    SDL_PIXELFORMAT_RGBA8888,
                                    SDL_TEXTUREACCESS_STREAMING,
                                    static_cast<int>(image.m_width),
                                    static_cast<int>(image.m_height));
        texWidth  = image.m_width;
        texHeight = image.m_height;
      }

      SDL_UpdateTexture(texture, nullptr, image.m_pixels.data(), static_cast<int>(4U * image.m_width));

      const std::string title = std::string("EBGeometry: ") + (onDevice ? "GPU" : "CPU") + ", " +
                                std::to_string(int(1000.0 * lastFrameS)) + " ms" + (drawFull ? "" : " (preview)") +
                                ((stats.m_insideHits > 0U) ? ", red: seen from inside the solid" : "") +
                                ((stats.m_unresolved > 0U) ? ", some rays unresolved" : "");

      SDL_SetWindowTitle(window, title.c_str());

      fullFrame  = drawFull;
      needRedraw = true;
    }

    if (needRedraw && texture != nullptr) {
      SDL_RenderClear(screen);
      SDL_RenderCopy(screen, texture, nullptr, nullptr);
      SDL_RenderPresent(screen);

      needRedraw = false;
    }
    else {
      SDL_Delay(10);
    }
  }

  if (texture != nullptr) {
    SDL_DestroyTexture(texture);
  }

  SDL_DestroyRenderer(screen);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

#endif

/**
 * @brief Open the window, or say why not.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in] a_shape       Shape, valid on the host.
 * @param[in] a_deviceShape Device view of it, or null.
 * @param[in] a_camera      Initial camera.
 * @param[in] a_settings    Settings.
 * @param[in] a_argc        Argument count.
 * @param[in] a_argv        Arguments.
 */
template <class T, class S>
inline void
start(const S&           a_shape,
      const S*           a_deviceShape,
      const Camera<T>&   a_camera,
      const Settings<T>& a_settings,
      const int          a_argc,
      char**             a_argv)
{
  static_assert(isShape<S>, "Render::view: the object to look at must be a shape (see isShape)");

  if (noViewRequested(a_argc, a_argv)) {
    std::printf("EBGeometry: --no-view was given, so the viewer does not open.\n");

    return;
  }

#ifdef EBGEOMETRY_VIEWER_SDL2
  if (!displayAvailable()) {
    std::printf("EBGeometry: there is no display, so the viewer does not open.\n");

    return;
  }

  run(a_shape, a_deviceShape, a_camera, a_settings);
#else
  (void)a_shape;
  (void)a_deviceShape;
  (void)a_camera;
  (void)a_settings;

  std::printf("EBGeometry: built without the viewer; build with VIEWER=SDL2 (make) or -DEBGEOMETRY_VIEWER=SDL2 "
              "(CMake) to look at the shape.\n");
#endif
}

/**
 * @brief The camera view() starts from: fitted to the shape's box if it is bounded, the default otherwise.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in] a_shape Shape.
 * @return The camera.
 */
template <class T, class S>
inline Camera<T>
initialCamera(const S& a_shape)
{
  const auto     box = a_shape.computeBoundingVolume();
  const Vec3T<T> lo  = box.getLowCorner();
  const Vec3T<T> hi  = box.getHighCorner();
  const T        d   = (hi - lo).length();

  if (std::isfinite(d) && lo[0] <= hi[0] && lo[1] <= hi[1] && lo[2] <= hi[2]) {
    return Camera<T>::fit(box);
  }

  return Camera<T>();
}

} // namespace ViewerDetail

template <class S>
inline void
view(const S& a_shape, int a_argc, char** a_argv)
{
  using T = ShapePrecision<S>;

  ViewerDetail::start<T, S>(a_shape, nullptr, ViewerDetail::initialCamera<T>(a_shape), Settings<T>(), a_argc, a_argv);
}

template <class S>
inline void
view(const S&                           a_shape,
     const Camera<ShapePrecision<S>>&   a_camera,
     const Settings<ShapePrecision<S>>& a_settings,
     int                                a_argc,
     char**                             a_argv)
{
  ViewerDetail::start<ShapePrecision<S>, S>(a_shape, nullptr, a_camera, a_settings, a_argc, a_argv);
}

template <class S>
inline void
view(const S&                           a_shape,
     const S&                           a_deviceShape,
     const Camera<ShapePrecision<S>>&   a_camera,
     const Settings<ShapePrecision<S>>& a_settings,
     int                                a_argc,
     char**                             a_argv)
{
  ViewerDetail::start<ShapePrecision<S>, S>(a_shape, &a_deviceShape, a_camera, a_settings, a_argc, a_argv);
}

} // namespace EBGeometry::Render

#endif
