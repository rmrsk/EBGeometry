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
#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <thread>

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
 * @brief A frame rendered on a thread of its own, so that the window keeps answering while it renders.
 * @details Internal helper; not part of the public API.
 */
struct Background
{
  /**
   * @brief The rendering thread.
   */
  std::thread m_thread;

  /**
   * @brief Set to stop a host render early.
   */
  std::atomic<bool> m_cancel{false};

  /**
   * @brief Set by the thread when it has finished.
   */
  std::atomic<bool> m_done{false};

  /**
   * @brief Rows finished so far.
   */
  std::atomic<uint32_t> m_rows{0};

  /**
   * @brief The frame.
   */
  Image m_image;

  /**
   * @brief What it saw.
   */
  Stats m_stats;

  /**
   * @brief How long it took, in seconds.
   */
  double m_seconds = 0;

  /**
   * @brief Its refinement stage.
   */
  int m_stage = 0;

  /**
   * @brief Its height in rows.
   */
  uint32_t m_height = 0;

  /**
   * @brief Stop the render (a device render after the band of rows under way) and wait for the thread.
   */
  void
  stop() noexcept
  {
    m_cancel = true;

    if (m_thread.joinable()) {
      m_thread.join();
    }
  }

  /**
   * @brief Waits for the thread.
   */
  ~Background() noexcept
  {
    this->stop();
  }
};

/**
 * @brief The window's event loop.
 * @details Internal helper; not part of the public API. While the camera moves it draws a quick preview
 * (one ray per pixel, no shadows), at a resolution it lowers or raises to keep each preview at a few tens
 * of milliseconds. Once the camera rests it refines in stages on a background thread, each shown when it
 * is done: full resolution without shadows, then the full settings. Moving the camera again cancels the
 * refinement at once, so the window never stops answering.
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
  using Clock = std::chrono::steady_clock;

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

  // The preview's resolution, as a fraction of the window's, adapted to how long previews take.
  T previewScale = T(0.5);

  bool onDevice = a_deviceShape != nullptr && GPU::deviceAvailable();
  bool dragging = false;
  bool warned   = false;
  bool running  = true;
  bool changed  = true;

  // The next refinement stage to render (0: none left), and when the camera last moved.
  int  pending    = 0;
  auto lastChange = Clock::now();

  std::unique_ptr<Background> job;

  SDL_Texture* texture   = nullptr;
  uint32_t     texWidth  = 0;
  uint32_t     texHeight = 0;

  std::string status;

  // The settings of a frame at a refinement stage: 0 the preview, 1 full resolution without shadows, 2
  // the full settings.
  const auto stageSettings = [&](const int a_stage) {
    Settings<T> frame = settings;

    // The section plane faces the camera through its target, moved by the arrow keys.
    Vec3T<T> forward;
    Vec3T<T> right;
    Vec3T<T> up;

    camera.frame(forward, right, up);

    frame.m_sectionNormal = -forward;
    frame.m_sectionPoint  = camera.m_target - sectionOffset * forward;

    if (a_stage < 2) {
      frame.m_supersampling = 1;
      frame.m_shadows       = false;
      frame.m_occlusion     = false;
    }

    if (a_stage == 0) {
      frame.m_width  = Math::max(1U, static_cast<uint32_t>(T(settings.m_width) * previewScale));
      frame.m_height = Math::max(1U, static_cast<uint32_t>(T(settings.m_height) * previewScale));
    }

    return frame;
  };

  // Whether a stage would draw anything the one before it did not.
  const auto adds = [&](const int a_stage) {
    return a_stage == 1 || settings.m_supersampling > 1U || settings.m_shadows || settings.m_occlusion;
  };

  // Everything a render reads is passed by value, so that a background render never sees the camera or
  // the settings change under it.
  const auto render =
    [&a_shape, a_deviceShape](
      const Settings<T>& a_frame, const Camera<T> a_view, const bool a_onDevice, Image& a_image, Background* a_job) {
      const Renderer<T> renderer(a_frame);

      if (a_onDevice && a_job != nullptr) {
        return renderer.renderOnDevice(*a_deviceShape, a_view, a_image, a_job->m_cancel, &a_job->m_rows);
      }
      if (a_onDevice) {
        return renderer.renderOnDevice(*a_deviceShape, a_view, a_image);
      }
      if (a_job != nullptr) {
        return renderer.render(a_shape, a_view, a_image, a_job->m_cancel, &a_job->m_rows);
      }

      return renderer.render(a_shape, a_view, a_image);
    };

  const auto show = [&](const Image& a_image, const Stats& a_stats, const double a_seconds, const char* a_what) {
    if (a_stats.m_cameraInside && !warned) {
      std::printf("EBGeometry viewer: the camera is inside the solid, so what it sees is seen from inside "
                  "(tinted red).\n");

      warned = true;
    }

    if (texture == nullptr || texWidth != a_image.m_width || texHeight != a_image.m_height) {
      if (texture != nullptr) {
        SDL_DestroyTexture(texture);
      }

      texture   = SDL_CreateTexture(screen,
                                  SDL_PIXELFORMAT_RGBA8888,
                                  SDL_TEXTUREACCESS_STREAMING,
                                  static_cast<int>(a_image.m_width),
                                  static_cast<int>(a_image.m_height));
      texWidth  = a_image.m_width;
      texHeight = a_image.m_height;
    }

    SDL_UpdateTexture(texture, nullptr, a_image.m_pixels.data(), static_cast<int>(4U * a_image.m_width));

    status = std::string("EBGeometry: ") + (onDevice ? "GPU" : "CPU") + ", " + a_what + " " +
             std::to_string(int(1000.0 * a_seconds)) + " ms" +
             ((a_stats.m_insideHits > 0U) ? ", red: seen from inside the solid" : "") +
             ((a_stats.m_unresolved > 0U) ? ", some rays unresolved" : "");

    SDL_SetWindowTitle(window, status.c_str());
    SDL_RenderClear(screen);
    SDL_RenderCopy(screen, texture, nullptr, nullptr);
    SDL_RenderPresent(screen);
  };

  while (running) {
    SDL_Event event;

    // Wait a little for the first event, then take every one that is queued.
    bool got = SDL_WaitEventTimeout(&event, 16) != 0;

    while (got) {
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
        if (texture != nullptr) {
          SDL_RenderClear(screen);
          SDL_RenderCopy(screen, texture, nullptr, nullptr);
          SDL_RenderPresent(screen);
        }

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

      got = SDL_PollEvent(&event) != 0;
    }

    if (!running) {
      break;
    }

    // The camera or the settings changed: drop any refinement, and draw a preview now.
    if (changed) {
      if (job != nullptr) {
        job->stop();
        job.reset();
      }

      Image image;

      const auto  start   = Clock::now();
      const Stats stats   = render(stageSettings(0), camera, onDevice, image, nullptr);
      const auto  seconds = std::chrono::duration<double>(Clock::now() - start).count();

      // Keep previews between about 10 and 40 ms.
      if (seconds > 0.04 && previewScale > T(0.125)) {
        previewScale *= T(0.5);
      }
      else if (seconds < 0.01 && previewScale < T(1)) {
        previewScale = Math::min(T(1), T(2) * previewScale);
      }

      show(image, stats, seconds, "preview");

      changed    = false;
      pending    = 1;
      lastChange = Clock::now();
    }

    // Refine on a background thread once the camera has rested a moment.
    if (job == nullptr && pending > 0 && !dragging && Clock::now() - lastChange > std::chrono::milliseconds(100)) {
      job = std::make_unique<Background>();

      Background* next = job.get();

      next->m_stage  = pending;
      next->m_height = stageSettings(pending).m_height;
      next->m_thread = std::thread([next, render, stage = stageSettings(pending), camera, onDevice]() {
        const auto start = Clock::now();

        next->m_stats   = render(stage, camera, onDevice, next->m_image, next);
        next->m_seconds = std::chrono::duration<double>(Clock::now() - start).count();
        next->m_done    = true;
      });

      pending = 0;
    }

    if (job != nullptr) {
      if (job->m_done) {
        job->stop();

        if (job->m_stats.m_complete) {
          show(job->m_image, job->m_stats, job->m_seconds, (job->m_stage == 1) ? "frame" : "full frame");

          pending = (job->m_stage == 1 && adds(2)) ? 2 : 0;
        }

        job.reset();
      }
      else if (job->m_stage == 2) {
        const uint32_t    percent = (100U * job->m_rows.load()) / Math::max(1U, job->m_height);
        const std::string title   = status + " (refining " + std::to_string(percent) + "%)";

        SDL_SetWindowTitle(window, title.c_str());
      }
    }
  }

  job.reset();

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
view(const S& a_shape, const S& a_deviceShape, int a_argc, char** a_argv)
{
  using T = ShapePrecision<S>;

  ViewerDetail::start<T, S>(
    a_shape, &a_deviceShape, ViewerDetail::initialCamera<T>(a_shape), Settings<T>(), a_argc, a_argv);
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

template <class S>
inline void
view(const S& a_shape, Pool& a_pool, int a_argc, char** a_argv)
{
  using T = ShapePrecision<S>;

  view(a_shape, a_pool, ViewerDetail::initialCamera<T>(a_shape), Settings<T>(), a_argc, a_argv);
}

template <class S>
inline void
view(const S&                           a_shape,
     Pool&                              a_pool,
     const Camera<ShapePrecision<S>>&   a_camera,
     const Settings<ShapePrecision<S>>& a_settings,
     int                                a_argc,
     char**                             a_argv)
{
  using T = ShapePrecision<S>;

#ifdef EBGEOMETRY_VIEWER_SDL2
  // Mirror only when a window will open on a device; the mirror lives as long as the window.
  if (GPU::enabled && GPU::deviceAvailable() && !ViewerDetail::noViewRequested(a_argc, a_argv) &&
      ViewerDetail::displayAvailable()) {
    a_pool.freeze();

    const Pool mirror      = Pool::mirror(a_pool, GPU::memoryResource());
    const S    deviceShape = a_shape.rebasedView(mirror);

    ViewerDetail::start<T, S>(a_shape, &deviceShape, a_camera, a_settings, a_argc, a_argv);

    return;
  }
#endif

  (void)a_pool;

  ViewerDetail::start<T, S>(a_shape, nullptr, a_camera, a_settings, a_argc, a_argv);
}

} // namespace EBGeometry::Render

#endif
