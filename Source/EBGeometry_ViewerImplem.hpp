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
#include <algorithm>
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
 * @brief A group of the viewer's keys, under a title.
 * @details Internal helper; not part of the public API.
 */
struct KeyGroup
{
  /**
   * @brief The title.
   */
  const char* m_title;

  /**
   * @brief The keys, in the order they are listed.
   */
  std::vector<Key> m_keys;
};

/**
 * @brief The viewer's keys, in groups: the camera and the light, rendering, the section, and the window.
 * @details Internal helper; not part of the public API. The overlay lists each group under its title with
 * the state its keys change, and the terminal lists them when the window opens.
 * @param[in] a_device Whether the window has a device view to switch to.
 * @return The groups.
 */
inline std::vector<KeyGroup>
keys(const bool a_device)
{
  std::vector<KeyGroup> groups{{"Camera and light",
                                {{"Drag", "Orbit the camera"},
                                 {"Ctrl + drag", "Pan the camera"},
                                 {"Wheel", "Zoom"},
                                 {"1, 2, 3", "Look from the front, the side, the top"},
                                 {"O", "Switch between perspective and orthographic"},
                                 {"R", "Reset the camera"},
                                 {"Right drag", "Move the light"},
                                 {"Left, Right", "Weaker or stronger highlights"},
                                 {"Shift + Left", "Broader highlights"},
                                 {"Shift + Right", "Tighter highlights"}}},
                               {"Rendering",
                                {{"P", "Cycle the colouring: by normal, uniform, by primitive"},
                                 {"S", "Turn shadows on or off"},
                                 {"A", "Turn ambient occlusion on or off"},
                                 {"F", "Turn the floor on or off"},
                                 {"Shift + Up", "More rays per pixel (up to 8 x 8)"},
                                 {"Shift + Down", "Fewer rays per pixel (down to 1)"}}},
                               {"Section and contours",
                                {{"X", "Cut across x, then y, then z"},
                                 {"Ctrl + X", "Remove the cut"},
                                 {"N", "Reverse the cut (remove the other side)"},
                                 {"Ctrl + wheel", "Move the plane in fine steps"},
                                 {"Up, Down", "Move the plane in coarse steps"}}},
                               {"Picking", {{"Click", "Pick the object under the cursor (the background clears it)"}}},
                               {"Window",
                                {{"C", "Print the camera and settings as C++"},
                                 {"H", "Show or hide this overlay and the axes"},
                                 {"Esc, Q", "Close the window"}}}};

  if (a_device) {
    groups[1].m_keys.push_back({"G", "Render on the GPU or the CPU"});
  }

  return groups;
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

  // A small negative number rounds to "-0.000"; write it without the sign.
  std::string written = text;

  if (written[0] == '-' && written.find_first_of("123456789") == std::string::npos) {
    written.erase(0, 1);
  }

  return written;
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

/**
 * @brief The scene's x, y and z axes as the camera sees them, drawn from a common origin on a transparent
 * square: x red, y green, z blue, each labelled at its tip, the axes furthest from the viewer drawn first.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in]  a_right   Unit right of the image, in the scene.
 * @param[in]  a_up      Unit up of the image, in the scene.
 * @param[in]  a_forward Unit direction the camera looks in.
 * @param[in]  a_scale   Pixels per glyph pixel (the square is 96 a_scale pixels wide).
 * @param[out] a_pixels  The square, RGBA (0xRRGGBBAA, row by row).
 * @param[out] a_size    Its side.
 */
template <class T>
inline void
drawAxes(const Vec3T<T>&        a_right,
         const Vec3T<T>&        a_up,
         const Vec3T<T>&        a_forward,
         const int              a_scale,
         std::vector<uint32_t>& a_pixels,
         int&                   a_size)
{
  a_size = 96 * a_scale;

  a_pixels.assign(std::size_t(a_size) * std::size_t(a_size), 0x00000000U);

  const T        centre     = T(0.5) * T(a_size);
  const T        length     = T(30 * a_scale);
  const uint32_t colours[3] = {0xE64646FFU, 0x50BE50FFU, 0x5A82F0FFU};
  const char     labels[3]  = {'X', 'Y', 'Z'};

  // Set a square of pixels, clipped to the image.
  const auto stamp = [&a_pixels, a_size](const int a_x, const int a_y, const int a_half, const uint32_t a_rgba) {
    for (int y = a_y - a_half; y <= a_y + a_half; y++) {
      for (int x = a_x - a_half; x <= a_x + a_half; x++) {
        if (x >= 0 && y >= 0 && x < a_size && y < a_size) {
          a_pixels[std::size_t(y) * std::size_t(a_size) + std::size_t(x)] = a_rgba;
        }
      }
    }
  };

  // The farthest axis first, so that a nearer one is drawn over it.
  int order[3] = {0, 1, 2};

  std::sort(order, order + 3, [&a_forward](const int a_a, const int a_b) { return a_forward[a_a] > a_forward[a_b]; });

  for (const int axis : order) {
    const T   dx    = a_right[axis];
    const T   dy    = -a_up[axis];
    const int steps = static_cast<int>(length) + 1;

    for (int i = 0; i <= steps; i++) {
      const T f = T(i) / T(steps);

      stamp(
        static_cast<int>(centre + f * length * dx), static_cast<int>(centre + f * length * dy), a_scale, colours[axis]);
    }

    // The label just past the tip, centred there.
    const int tipX = static_cast<int>(centre + (length + T(9 * a_scale)) * dx) - GlyphWidth * a_scale / 2;
    const int tipY = static_cast<int>(centre + (length + T(9 * a_scale)) * dy) - GlyphHeight * a_scale / 2;

    const unsigned char* glyph = Glyphs[labels[axis] - 32];

    for (int y = 0; y < GlyphHeight * a_scale; y++) {
      for (int x = 0; x < GlyphWidth * a_scale; x++) {
        if (((glyph[y / a_scale] >> (GlyphWidth - 1 - x / a_scale)) & 1U) != 0U) {
          stamp(tipX + x, tipY + y, 0, colours[axis]);
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

  std::printf("EBGeometry viewer keys\n");

  for (const KeyGroup& group : keys(a_deviceShape != nullptr)) {
    std::printf("\n%s\n", group.m_title);

    for (const Key& key : group.m_keys) {
      std::printf("  %-14s %s\n", key.m_keys, key.m_what);
    }
  }

  std::printf("\n");
  std::fflush(stdout);

  Camera<T>   camera   = a_camera;
  Settings<T> settings = a_settings;

  // The scene's length scale, for moving the section plane.
  const auto box   = a_shape.computeBoundingVolume();
  const T    diag  = (box.getHighCorner() - box.getLowCorner()).length();
  const T    scale = (std::isfinite(diag) && diag > T(0)) ? diag : camera.m_distance;

  // The section plane: off (0), as the settings give it (1, only when they turn it on), or across the x, y
  // or z axis through the box's centre (2 to 4); moved along its normal by an offset, and reversed (the
  // other side removed) when flipped.
  const bool     givenSection  = a_settings.m_section;
  const Vec3T<T> centre        = (std::isfinite(diag) && diag > T(0)) ? box.getCentroid() : camera.m_target;
  int            sectionMode   = givenSection ? 1 : 0;
  bool           sectionFlip   = false;
  T              sectionOffset = T(0);

  const char* sectionNames[] = {"Off", "As given", "Across x", "Across y", "Across z"};

  // The preview's resolution, as a fraction of the window's, adapted to how long previews take.
  T previewScale = T(0.5);

  bool onDevice = a_deviceShape != nullptr && GPU::deviceAvailable();
  bool dragging = false;
  bool panning  = false;
  bool lighting = false;

  // A left click that moves the mouse less than a few pixels picks what is under it.
  int  pressX = 0;
  int  pressY = 0;
  bool moved  = false;

  bool    picked = false;
  HitT<T> pick;
  bool    warned  = false;
  bool    running = true;
  bool    changed = true;

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

  // The axis triad, in the bottom left corner; shown and hidden with the overlay.
  SDL_Texture*          axes     = nullptr;
  int                   axesSize = 0;
  std::vector<uint32_t> axesPixels;

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

    if (overlayOn && axes != nullptr) {
      int outputWidth  = 0;
      int outputHeight = 0;

      SDL_GetRendererOutputSize(screen, &outputWidth, &outputHeight);

      const SDL_Rect where{8, outputHeight - axesSize - 8, axesSize, axesSize};

      SDL_RenderCopy(screen, axes, nullptr, &where);
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

    Vec3T<T> normal = RenderDetail::unit(a_settings.m_sectionNormal);
    Vec3T<T> point  = a_settings.m_sectionPoint;

    if (sectionMode >= 2) {
      normal                  = Vec3T<T>::zeros();
      normal[sectionMode - 2] = T(1);
      point                   = centre;
    }

    if (sectionFlip) {
      normal = -normal;
    }

    // The arrow keys move the plane towards the removed side, so that more of the shape shows.
    frame.m_section       = sectionMode != 0;
    frame.m_sectionNormal = normal;
    frame.m_sectionPoint  = point + sectionOffset * normal;

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
    const auto point = [](const Vec3T<T>& a_v, const int a_decimals) {
      return "(" + fixed(double(a_v[0]), a_decimals) + ", " + fixed(double(a_v[1]), a_decimals) + ", " +
             fixed(double(a_v[2]), a_decimals) + ")";
    };

    const char* colouring = (settings.m_colourBy == ColourBy::Normal)      ? "Coloured by normal"
                            : (settings.m_colourBy == ColourBy::Primitive) ? "Coloured by primitive"
                                                                           : "Coloured uniformly";

    // Each group's state, in the order of keys(): the camera and the light, rendering, the section, picking.
    const std::vector<std::vector<std::string>> states{
      {"Yaw " + fixed(double(camera.m_yaw), 1) + ", pitch " + fixed(double(camera.m_pitch), 1) + ", distance " +
         fixed(double(camera.m_distance), 3),
       "Target " + point(camera.m_target, 3),
       "Field of view " + fixed(double(camera.m_fieldOfView), 1) + ", " +
         (camera.m_orthographic ? "orthographic" : "perspective") + ", near clip " +
         fixed(double(camera.m_nearClip), 3),
       std::string("Light ") + (settings.m_lightFromCamera ? "from the camera, " : "in the scene, ") +
         point(settings.m_light, 2),
       "Highlights of strength " + fixed(double(settings.m_specular), 2) + ", tightness " +
         fixed(double(settings.m_shininess), 0)},
      {std::string(colouring) + ", rendered on the " + (onDevice ? "GPU" : "CPU"),
       "Supersampling " + std::to_string(settings.m_supersampling) + " x " + std::to_string(settings.m_supersampling) +
         " (" + std::to_string(settings.m_supersampling * settings.m_supersampling) + " rays per pixel)",
       std::string("Shadows ") + onOff(settings.m_shadows) + ", ambient occlusion " + onOff(settings.m_occlusion) +
         ", floor " + onOff(settings.m_floor) + ", sky " + onOff(settings.m_sky)},
      {std::string(sectionNames[sectionMode]) + ((sectionMode != 0) ? std::string(sectionFlip ? ", reversed" : "") +
                                                                        ", moved " + fixed(double(sectionOffset), 3)
                                                                    : std::string()),
       "Contour " + fixed(double(settings.m_contourWidth), 1) + " pixels wide, sheet " +
         fixed(100.0 * double(settings.m_sheetOpacity), 0) + "% opaque, cut face " +
         fixed(100.0 * double(settings.m_cutOpacity), 0) + "% opaque"},
      {picked ? (primitivesAreFaces<S> ? "Object 0 (face " + std::to_string(pick.m_primitive) + ")"
                                       : "Object " + std::to_string(pick.m_primitive)) +
                  " at " + point(pick.m_point, 3) + ", normal " + point(pick.m_normal, 2)
              : std::string("Nothing picked")},
      {}};

    std::vector<std::string> lines{
      std::string("EBGeometry viewer: ") + a_what + ", " + std::to_string(a_image.m_width) + " x " +
        std::to_string(a_image.m_height) + " pixels in " + fixed(1000.0 * a_seconds, 0) + " ms",
      "Rays " + std::to_string(a_stats.m_rays) + ", hits " + percent(a_stats.m_hits, a_stats.m_rays) +
        ", seen from inside " + std::to_string(a_stats.m_insideHits) + ", unresolved " +
        std::to_string(a_stats.m_unresolved)};

    const std::vector<KeyGroup> groups = keys(a_deviceShape != nullptr);

    for (std::size_t g = 0; g < groups.size(); g++) {
      lines.emplace_back("");
      lines.emplace_back(groups[g].m_title);

      for (const std::string& state : states[g]) {
        lines.emplace_back("  " + state);
      }

      for (const Key& key : groups[g].m_keys) {
        char line[128];

        std::snprintf(line, sizeof(line), "  %-14s %s", key.m_keys, key.m_what);
        lines.emplace_back(line);
      }
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

    // The axes as the camera of this frame sees them.
    Vec3T<T> forward;
    Vec3T<T> right;
    Vec3T<T> up;

    camera.frame(forward, right, up);

    const int oldAxes = axesSize;

    drawAxes(right, up, forward, scale, axesPixels, axesSize);

    if (axes == nullptr || oldAxes != axesSize) {
      if (axes != nullptr) {
        SDL_DestroyTexture(axes);
      }

      axes = SDL_CreateTexture(screen, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, axesSize, axesSize);

      SDL_SetTextureBlendMode(axes, SDL_BLENDMODE_BLEND);
    }

    SDL_UpdateTexture(axes, nullptr, axesPixels.data(), 4 * axesSize);

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
        // A drag orbits the camera, or pans it with Ctrl held; a drag with the right button moves the light.
        if (event.button.button == SDL_BUTTON_LEFT) {
          panning  = (SDL_GetModState() & KMOD_CTRL) != 0;
          dragging = !panning;
          pressX   = event.button.x;
          pressY   = event.button.y;
          moved    = false;
        }
        else if (event.button.button == SDL_BUTTON_RIGHT) {
          lighting = true;
        }

        break;
      }
      case SDL_MOUSEBUTTONUP: {
        if (event.button.button == SDL_BUTTON_LEFT) {
          if ((dragging || panning) && !moved) {
            // The window's point in the frame's pixels (more of them on a high-DPI display).
            int windowWidth  = 0;
            int windowHeight = 0;

            SDL_GetWindowSize(window, &windowWidth, &windowHeight);

            const Settings<T> frame  = stageSettings(2);
            const T           column = T(event.button.x) * T(frame.m_width) / T(Math::max(windowWidth, 1));
            const T           row    = T(event.button.y) * T(frame.m_height) / T(Math::max(windowHeight, 1));

            pick   = Renderer<T>(frame).pick(a_shape, camera, column, row);
            picked = pick.hit();

            // The object picked: a tape's leaf, a fused composition's part, a union's member; a mesh SDF is one
            // object, whose primitive is the face picked on it.
            if (!picked) {
              settings.m_highlightPrimitive = Settings<T>::NoPrimitive;
            }
            else if (primitivesAreFaces<S>) {
              settings.m_highlightPrimitive = Settings<T>::AllPrimitives;

              std::printf("EBGeometry viewer: picked object 0, face %u, at (%g, %g, %g), normal (%g, %g, %g)\n",
                          unsigned(pick.m_primitive),
                          double(pick.m_point[0]),
                          double(pick.m_point[1]),
                          double(pick.m_point[2]),
                          double(pick.m_normal[0]),
                          double(pick.m_normal[1]),
                          double(pick.m_normal[2]));
            }
            else {
              settings.m_highlightPrimitive = pick.m_primitive;

              std::printf("EBGeometry viewer: picked object %u at (%g, %g, %g), normal (%g, %g, %g)\n",
                          unsigned(pick.m_primitive),
                          double(pick.m_point[0]),
                          double(pick.m_point[1]),
                          double(pick.m_point[2]),
                          double(pick.m_normal[0]),
                          double(pick.m_normal[1]),
                          double(pick.m_normal[2]));
            }

            std::fflush(stdout);

            changed = true;
          }

          dragging = false;
          panning  = false;
        }
        else if (event.button.button == SDL_BUTTON_RIGHT) {
          lighting = false;
        }

        break;
      }
      case SDL_MOUSEMOTION: {
        moved = moved || std::abs(event.motion.x - pressX) > 3 || std::abs(event.motion.y - pressY) > 3;

        if (lighting) {
          turnLight(T(0.4) * T(event.motion.xrel), -T(0.4) * T(event.motion.yrel));

          changed = true;
        }
        else if (panning) {
          // Move the target so that the point under the mouse follows it: at the target's distance, the
          // window's height spans 2 distance tan(fov / 2).
          int windowWidth  = 0;
          int windowHeight = 0;

          SDL_GetWindowSize(window, &windowWidth, &windowHeight);

          Vec3T<T> forward;
          Vec3T<T> right;
          Vec3T<T> up;

          camera.frame(forward, right, up);

          const T perPixel = T(2) * camera.m_distance * std::tan(T(0.5) * camera.m_fieldOfView * pi<T> / T(180)) /
                             T(Math::max(windowHeight, 1));

          camera.m_target -= perPixel * (T(event.motion.xrel) * right - T(event.motion.yrel) * up);

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
        // With Ctrl held the wheel moves the section plane in fine steps; otherwise it zooms. A sideways
        // scroll (y = 0) does neither.
        if (event.wheel.y != 0) {
          if ((SDL_GetModState() & KMOD_CTRL) != 0 && sectionMode != 0) {
            sectionOffset += T(0.005) * scale * T(event.wheel.y);
          }
          else {
            camera.m_distance *= (event.wheel.y > 0) ? T(0.9) : T(1) / T(0.9);
          }

          changed = true;
        }

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
          // X turns the plane on across x, then steps it to y, z and x again; Ctrl and X turn it off.
          if ((event.key.keysym.mod & KMOD_CTRL) != 0) {
            sectionMode = 0;
          }
          else {
            sectionMode = (sectionMode < 2) ? 2 : 2 + (sectionMode - 1) % 3;
          }

          sectionOffset = T(0);

          break;
        }
        case SDLK_n: {
          sectionFlip = !sectionFlip;

          break;
        }
        case SDLK_UP:
        case SDLK_DOWN: {
          // With Shift held, more or fewer rays per pixel (1 x 1 to 8 x 8); otherwise the section plane moves
          // in coarse steps.
          const bool upward = event.key.keysym.sym == SDLK_UP;

          if ((event.key.keysym.mod & KMOD_SHIFT) != 0) {
            settings.m_supersampling =
              upward ? Math::min(settings.m_supersampling + 1U, 8U) : Math::max(settings.m_supersampling, 2U) - 1U;
          }
          else {
            sectionOffset += (upward ? T(0.02) : T(-0.02)) * scale;
          }

          break;
        }
        case SDLK_LEFT:
        case SDLK_RIGHT: {
          // The highlights: stronger or weaker, or with Shift held, tighter or broader (the specular exponent).
          const bool more = event.key.keysym.sym == SDLK_RIGHT;

          if ((event.key.keysym.mod & KMOD_SHIFT) != 0) {
            settings.m_shininess =
              Math::clamp(more ? settings.m_shininess * T(1.5) : settings.m_shininess / T(1.5), T(2), T(512));
          }
          else {
            // In steps of 0.05, kept on them so that repeated steps do not drift.
            const T steps = std::round(settings.m_specular * T(20)) + (more ? T(1) : T(-1));

            settings.m_specular = Math::clamp(steps / T(20), T(0), T(1.5));
          }

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
    if (job == nullptr && pending > 0 && !dragging && !panning && !lighting &&
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

  if (axes != nullptr) {
    SDL_DestroyTexture(axes);
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
