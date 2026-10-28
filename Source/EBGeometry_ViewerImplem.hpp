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
#include <vector>

// The program keeps its own main(); SDL must not replace it.
#ifndef SDL_MAIN_HANDLED
#define SDL_MAIN_HANDLED
#endif
#include <SDL.h>
#endif

// Our includes
#include "EBGeometry_GPUTransform.hpp"
#include "EBGeometry_Viewer.hpp"
#include "EBGeometry_ViewerFont.hpp"

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

/**
 * @brief A key of the viewer, and what it does.
 * @details Internal helper; not part of the public API.
 */
struct Key
{
  /**
   * @brief The key or keys, as written for a reader.
   */
  const char* m_keys;

  /**
   * @brief What they do.
   */
  const char* m_what;
};

/**
 * @brief The viewer's keys, in the order they are listed.
 * @details Internal helper; not part of the public API.
 * @param[in] a_device Whether the window has a device view to switch to.
 * @return The keys.
 */
inline std::vector<Key>
keys(const bool a_device)
{
  std::vector<Key> list{{"drag", "orbit the camera"},
                        {"wheel", "zoom"},
                        {"1 2 3", "look from the front, the side, the top"},
                        {"O", "orthographic or perspective camera"},
                        {"R", "reset the camera"},
                        {"right drag", "move the light (or Shift + drag)"},
                        {"Left Right", "turn the light"},
                        {"P", "colouring: by normal, uniform, by primitive"},
                        {"S", "shadows on or off"},
                        {"A", "ambient occlusion on or off"},
                        {"F", "floor on or off"},
                        {"X", "section plane on or off"},
                        {"Up Down", "move the section plane"}};

  if (a_device) {
    list.push_back({"G", "render on the GPU or the CPU"});
  }

  list.push_back({"C", "print the camera and settings as C++"});
  list.push_back({"H", "show or hide the overlay"});
  list.push_back({"Esc Q", "close the window"});

  return list;
}

/**
 * @brief A number with a given count of decimals.
 * @details Internal helper; not part of the public API.
 * @param[in] a_value    Number.
 * @param[in] a_decimals Decimals.
 * @return The text.
 */
inline std::string
fixed(const double a_value, const int a_decimals)
{
  char text[64];

  std::snprintf(text, sizeof(text), "%.*f", a_decimals, a_value);

  return text;
}

/**
 * @brief Lines of text, light on a translucent dark panel, as RGBA pixels (0xRRGGBBAA, row by row).
 * @details Internal helper; not part of the public API. Each glyph pixel becomes a_scale x a_scale pixels.
 * Characters outside printable ASCII are left blank.
 * @param[in]  a_lines  Lines.
 * @param[in]  a_scale  Pixels per glyph pixel, along each axis.
 * @param[out] a_pixels The panel.
 * @param[out] a_width  Its width.
 * @param[out] a_height Its height.
 */
inline void
drawText(const std::vector<std::string>& a_lines,
         const int                       a_scale,
         std::vector<uint32_t>&          a_pixels,
         int&                            a_width,
         int&                            a_height)
{
  const int margin = 6 * a_scale;

  std::size_t columns = 0;

  for (const std::string& line : a_lines) {
    columns = Math::max(columns, line.size());
  }

  a_width  = static_cast<int>(columns) * GlyphWidth * a_scale + 2 * margin;
  a_height = static_cast<int>(a_lines.size()) * GlyphHeight * a_scale + 2 * margin;

  a_pixels.assign(std::size_t(a_width) * std::size_t(a_height), 0x101418C8U);

  for (std::size_t row = 0; row < a_lines.size(); row++) {
    for (std::size_t column = 0; column < a_lines[row].size(); column++) {
      const auto c = static_cast<unsigned char>(a_lines[row][column]);

      if (c < 32U || c > 126U) {
        continue;
      }

      const unsigned char* glyph = Glyphs[c - 32U];

      for (int y = 0; y < GlyphHeight * a_scale; y++) {
        for (int x = 0; x < GlyphWidth * a_scale; x++) {
          if (((glyph[y / a_scale] >> (GlyphWidth - 1 - x / a_scale)) & 1U) != 0U) {
            const int px = margin + static_cast<int>(column) * GlyphWidth * a_scale + x;
            const int py = margin + static_cast<int>(row) * GlyphHeight * a_scale + y;

            a_pixels[std::size_t(py) * std::size_t(a_width) + std::size_t(px)] = 0xF0F0F0FFU;
          }
        }
      }
    }
  }
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

  SDL_Window*   window = SDL_CreateWindow("EBGeometry",
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        width,
                                        height,
                                        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
  SDL_Renderer* screen = (window != nullptr) ? SDL_CreateRenderer(window, -1, 0) : nullptr;

  if (window == nullptr || screen == nullptr) {
    std::printf("EBGeometry: the viewer could not open a window (%s).\n", SDL_GetError());

    if (window != nullptr) {
      SDL_DestroyWindow(window);
    }

    SDL_Quit();

    return;
  }

  std::printf("EBGeometry viewer keys:\n");

  for (const Key& key : keys(a_deviceShape != nullptr)) {
    std::printf("  %-11s %s\n", key.m_keys, key.m_what);
  }

  std::fflush(stdout);

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
  bool lighting = false;
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

  // The overlay: what the window shows and how, and the keys, drawn over the image's top left corner.
  bool                  overlayOn     = true;
  SDL_Texture*          overlay       = nullptr;
  int                   overlayWidth  = 0;
  int                   overlayHeight = 0;
  std::vector<uint32_t> overlayPixels;

  // Draw the image, then the overlay over it.
  const auto present = [&]() {
    SDL_RenderClear(screen);

    if (texture != nullptr) {
      SDL_RenderCopy(screen, texture, nullptr, nullptr);
    }

    if (overlayOn && overlay != nullptr) {
      const SDL_Rect where{8, 8, overlayWidth, overlayHeight};

      SDL_RenderCopy(screen, overlay, nullptr, &where);
    }

    SDL_RenderPresent(screen);
  };

  std::string status;

  // Render at the window's size in pixels (more than its size in screen units on a high-DPI display), so
  // that a larger window gets a sharper image rather than a stretched one.
  const auto fitToWindow = [&]() {
    int w = 0;
    int h = 0;

    if (SDL_GetRendererOutputSize(screen, &w, &h) == 0 && w > 0 && h > 0 &&
        (uint32_t(w) != settings.m_width || uint32_t(h) != settings.m_height)) {
      settings.m_width  = uint32_t(w);
      settings.m_height = uint32_t(h);

      changed = true;
    }
  };

  fitToWindow();

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

  // Turn the light: a_across degrees about the vertical (the image's, for a light that follows the camera; the
  // scene's up axis otherwise), then a_up degrees towards it, stopping short of straight overhead or below.
  const auto turnLight = [&](const T a_across, const T a_up) {
    const Vec3T<T> k = settings.m_lightFromCamera ? Vec3T<T>(T(0), T(1), T(0)) : RenderDetail::unit(camera.m_up);

    // Rodrigues' rotation of a vector about a unit axis.
    const auto rotate = [](const Vec3T<T>& a_v, const Vec3T<T>& a_axis, const T a_degrees) {
      const T a = a_degrees * pi<T> / T(180);

      return std::cos(a) * a_v + std::sin(a) * cross(a_axis, a_v) + (T(1) - std::cos(a)) * dot(a_axis, a_v) * a_axis;
    };

    Vec3T<T> v = rotate(RenderDetail::unit(settings.m_light), k, a_across);

    const Vec3T<T> side      = cross(v, k);
    const T        elevation = std::asin(Math::clamp(dot(v, k), T(-1), T(1))) * T(180) / pi<T>;
    const T        target    = Math::clamp(elevation + a_up, T(-85), T(85));

    if (side.length() > T(0)) {
      v = rotate(v, RenderDetail::unit(side), target - elevation);
    }

    settings.m_light = v;
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
             std::to_string(a_image.m_width) + "x" + std::to_string(a_image.m_height) + " in " +
             std::to_string(int(1000.0 * a_seconds)) + " ms" +
             // A few grazing rays can seem to leave the solid; say so only when more than one hit in a
             // thousand does.
             ((a_stats.m_insideHits * 1000U > a_stats.m_hits) ? ", red: seen from inside the solid" : "") +
             ((a_stats.m_unresolved > 0U) ? ", some rays unresolved" : "");

    SDL_SetWindowTitle(window, status.c_str());

    // The overlay's text, at a size that stays readable on a window with many pixels.
    const auto percent = [](const uint64_t a_part, const uint64_t a_whole) {
      return fixed(100.0 * double(a_part) / double(Math::max(a_whole, uint64_t(1))), 1) + "%";
    };
    const auto onOff = [](const bool a_on) { return a_on ? "on" : "off"; };

    const char* colouring = (settings.m_colourBy == ColourBy::Normal)      ? "by normal"
                            : (settings.m_colourBy == ColourBy::Primitive) ? "by primitive"
                                                                           : "uniform";

    std::vector<std::string> lines{
      std::string("EBGeometry viewer: ") + (onDevice ? "GPU" : "CPU") + ", " + a_what + " " +
        std::to_string(a_image.m_width) + " x " + std::to_string(a_image.m_height) + " in " +
        fixed(1000.0 * a_seconds, 0) + " ms",
      "rays " + std::to_string(a_stats.m_rays) + ", hits " + percent(a_stats.m_hits, a_stats.m_rays) +
        ", from inside " + std::to_string(a_stats.m_insideHits) + ", unresolved " +
        std::to_string(a_stats.m_unresolved),
      "",
      "camera   yaw " + fixed(double(camera.m_yaw), 1) + ", pitch " + fixed(double(camera.m_pitch), 1) + ", distance " +
        fixed(double(camera.m_distance), 3),
      "         field of view " + fixed(double(camera.m_fieldOfView), 1) + ", " +
        (camera.m_orthographic ? "orthographic" : "perspective") + ", near clip " + fixed(double(camera.m_nearClip), 3),
      std::string("light    ") + (settings.m_lightFromCamera ? "from the camera" : "in the scene") + ", (" +
        fixed(double(settings.m_light[0]), 2) + ", " + fixed(double(settings.m_light[1]), 2) + ", " +
        fixed(double(settings.m_light[2]), 2) + ")",
      std::string("colour   ") + colouring + ", shadows " + onOff(settings.m_shadows) + ", occlusion " +
        onOff(settings.m_occlusion) + ", floor " + onOff(settings.m_floor) + ", sky " + onOff(settings.m_sky),
      std::string("section  ") + (settings.m_section ? "on, at " + fixed(double(sectionOffset), 3) : "off") + ", " +
        std::to_string(settings.m_supersampling) + " x " + std::to_string(settings.m_supersampling) + " rays per pixel",
      ""};

    for (const Key& key : keys(a_deviceShape != nullptr)) {
      char line[128];

      std::snprintf(line, sizeof(line), "%-11s %s", key.m_keys, key.m_what);
      lines.emplace_back(line);
    }

    int outputWidth  = 0;
    int outputHeight = 0;

    SDL_GetRendererOutputSize(screen, &outputWidth, &outputHeight);

    const int scale     = Math::max(1, outputHeight / 900);
    const int oldWidth  = overlayWidth;
    const int oldHeight = overlayHeight;

    drawText(lines, scale, overlayPixels, overlayWidth, overlayHeight);

    if (overlay == nullptr || oldWidth != overlayWidth || oldHeight != overlayHeight) {
      if (overlay != nullptr) {
        SDL_DestroyTexture(overlay);
      }

      overlay =
        SDL_CreateTexture(screen, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, overlayWidth, overlayHeight);

      SDL_SetTextureBlendMode(overlay, SDL_BLENDMODE_BLEND);
    }

    SDL_UpdateTexture(overlay, nullptr, overlayPixels.data(), 4 * overlayWidth);

    present();
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
        // The right button, or the left with Shift held, moves the light; the left alone orbits the camera.
        const bool shift = (SDL_GetModState() & KMOD_SHIFT) != 0;

        if (event.button.button == SDL_BUTTON_RIGHT || (event.button.button == SDL_BUTTON_LEFT && shift)) {
          lighting = true;
        }
        else if (event.button.button == SDL_BUTTON_LEFT) {
          dragging = true;
        }

        break;
      }
      case SDL_MOUSEBUTTONUP: {
        if (event.button.button == SDL_BUTTON_LEFT || event.button.button == SDL_BUTTON_RIGHT) {
          dragging = false;
          lighting = false;
        }

        break;
      }
      case SDL_MOUSEMOTION: {
        if (lighting) {
          turnLight(T(0.4) * T(event.motion.xrel), -T(0.4) * T(event.motion.yrel));

          changed = true;
        }
        else if (dragging) {
          camera.m_yaw -= T(0.4) * T(event.motion.xrel);
          camera.m_pitch = Math::clamp(camera.m_pitch + T(0.4) * T(event.motion.yrel), T(-90), T(90));

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
        if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
          fitToWindow();
        }
        else {
          present();
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
          settings.m_colourBy = (settings.m_colourBy == ColourBy::Normal)    ? ColourBy::Uniform
                                : (settings.m_colourBy == ColourBy::Uniform) ? ColourBy::Primitive
                                                                             : ColourBy::Normal;

          break;
        }
        case SDLK_o: {
          camera.m_orthographic = !camera.m_orthographic;

          break;
        }
        case SDLK_1:
        case SDLK_2:
        case SDLK_3: {
          // Front, right and top: the yaw and pitch of Camera::fit()'s views, the target and distance kept.
          const View view = (event.key.keysym.sym == SDLK_1)   ? View::Front
                            : (event.key.keysym.sym == SDLK_2) ? View::Right
                                                               : View::Top;

          camera.m_yaw   = (view == View::Right) ? T(90) : T(0);
          camera.m_pitch = (view == View::Top) ? T(90) : T(0);

          break;
        }
        case SDLK_LEFT:
        case SDLK_RIGHT: {
          turnLight((event.key.keysym.sym == SDLK_RIGHT) ? T(15) : T(-15), T(0));

          break;
        }
        case SDLK_h: {
          overlayOn = !overlayOn;
          changed   = false;

          present();

          break;
        }
        case SDLK_c: {
          // The view as C++, to paste into a program: the camera, and the settings of the full frame.
          std::printf("EBGeometry viewer: the current view, as C++:\n%s", toCode(camera, stageSettings(2)).c_str());
          std::fflush(stdout);

          changed = false;

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
    if (job == nullptr && pending > 0 && !dragging && !lighting &&
        Clock::now() - lastChange > std::chrono::milliseconds(100)) {
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

  if (overlay != nullptr) {
    SDL_DestroyTexture(overlay);
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
