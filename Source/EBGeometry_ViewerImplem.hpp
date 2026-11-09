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

#ifdef EBGEOMETRY_VIEWER_SDL2
#include "EBGeometry_ViewerFont.hpp"
#endif

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

// The window's helpers are compiled only with the window: kept out of every other translation unit that
// includes the library, they cannot change how the compiler inlines code there (they made GCC stop inlining
// a tape's primitive dispatch in programs that never open a window).
#ifdef EBGEOMETRY_VIEWER_SDL2

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
keys(const bool a_device, const bool a_regions = false)
{
  std::vector<KeyGroup> groups{{"Camera and light",
                                {{"Drag", "Orbit the camera"},
                                 {"Ctrl + drag", "Pan the camera"},
                                 {"Wheel", "Zoom"},
                                 {"1, 2, 3", "Look from the front, the side, the top"},
                                 {"O", "Switch between perspective and orthographic"},
                                 {"R", "Reset the camera, the light and the picked surfaces"},
                                 {"Right drag", "Move the light"},
                                 {"+, -", "Narrower or wider field of view"},
                                 {"Double-click", "Center the camera on the point under the cursor"},
                                 {"Left, Right", "Weaker or stronger highlights"},
                                 {"Shift + Left", "Broader highlights"},
                                 {"Shift + Right", "Tighter highlights"}}},
                               {"Rendering",
                                {{"M", "Cycle the view: shaded, depth, x-ray, cost of firstHit(), of firstCrossing()"},
                                 {"Shift + M", "Cycle the view backwards"},
                                 {"P", "Cycle the coloring: by normal, uniform, by primitive"},
                                 {"S", "Turn shadows on or off"},
                                 {"A", "Turn ambient occlusion on or off"},
                                 {"F", "Turn the floor on or off"},
                                 {"B", "Turn the sky on or off"},
                                 {"Shift + Up", "More rays per pixel (up to 8 x 8)"},
                                 {"Shift + Down", "Fewer rays per pixel (down to 1)"}}},
                               {"Section and contours",
                                {{"X", "Cut across x, then y, then z"},
                                 {"Ctrl + X", "Remove the cut"},
                                 {"N", "Reverse the cut (remove the other side)"},
                                 {"I", "Lines of equal distance: off, as set, coarse, fine"},
                                 {"Ctrl + wheel", "Move the plane in fine steps"},
                                 {"Up, Down", "Move the plane in coarse steps"}}},
                               {"Picking and measuring",
                                {{"Click", "Pick the object under the cursor (the background clears it)"},
                                 {"Shift + wheel", "Step through the surfaces under the cursor, front to back"},
                                 {"T", "See through the picked surface, or make it opaque again"},
                                 {"Shift + T", "Make every surface opaque again"},
                                 {"Shift + click", "Measure between two points (the background clears it)"}}},
                               {"Window",
                                {{"C", "Print the camera and settings as C++"},
                                 {"W", "Save the full frame as a PPM image"},
                                 {"Tab, Shift + Tab", "Show the next or the previous menu"},
                                 {"F1 to F6", "Show a menu: camera, rendering, section, picking, regions, window"},
                                 {"H", "Show or hide the menu (the status lines and the axes stay)"},
                                 {"Esc, Q", "Close the window"}}}};

  if (a_device) {
    groups[1].m_keys.push_back({"G", "Render on the GPU or the CPU"});
  }

  // A shape whose regions can be shown and hidden (see RegionView) gets a group of its own, before the window's.
  if (a_regions) {
    groups.insert(
      groups.end() - 1,
      KeyGroup{"Regions",
               {{"K", "Color by region, material, surface label, surface, part, interface or group (P colors by it)"},
                {"L", "Show only the picked surface (what it is follows K), or everything again"},
                {"V", "Show one region at a time, in turn, then all"},
                {"Shift + V", "Hide the picked region (Ctrl + V shows them all again)"},
                {"Ctrl + V", "Show every region"},
                {"Enter", "Add the picked piece (its surface, part and the regions on its sides) to the current group"},
                {"Shift + Enter", "Add the picked surface, all its parts and sides, to the current group"},
                {"Ctrl + Enter", "Add the interface of the picked piece (its two regions) to the current group"},
                {"Backspace", "Remove what holds the picked piece from the current group"},
                {"J", "Make the next group current; past the last, start a new one"}}});
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
 * square: x red, y green, z blue, each labeled at its tip, the axes furthest from the viewer drawn first.
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

  const T        center    = T(0.5) * T(a_size);
  const T        length    = T(30 * a_scale);
  const uint32_t colors[3] = {0xE64646FFU, 0x50BE50FFU, 0x5A82F0FFU};
  const char     labels[3] = {'X', 'Y', 'Z'};

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
        static_cast<int>(center + f * length * dx), static_cast<int>(center + f * length * dy), a_scale, colors[axis]);
    }

    // The label just past the tip, centered there.
    const int tipX = static_cast<int>(center + (length + T(9 * a_scale)) * dx) - GlyphWidth * a_scale / 2;
    const int tipY = static_cast<int>(center + (length + T(9 * a_scale)) * dy) - GlyphHeight * a_scale / 2;

    const unsigned char* glyph = Glyphs[labels[axis] - 32];

    for (int y = 0; y < GlyphHeight * a_scale; y++) {
      for (int x = 0; x < GlyphWidth * a_scale; x++) {
        if (((glyph[y / a_scale] >> (GlyphWidth - 1 - x / a_scale)) & 1U) != 0U) {
          stamp(tipX + x, tipY + y, 0, colors[axis]);
        }
      }
    }
  }
}

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
   * @brief Set to stop the render early.
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
   * @brief Stop the render and wait for the thread.
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
 * @brief What a click on a line of the viewer's panel of regions and groups does.
 * @details Internal helper; not part of the public API.
 */
enum class PanelAction : uint8_t
{
  None,           ///< Nothing.
  ToggleRegion,   ///< Show or hide a region.
  CurrentGroup,   ///< Make a group the one added to.
  RemoveSelector, ///< Remove a selector from the scheme.
  NewGroup        ///< Start a group.
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
 * @param[in] a_names       Names of the regions, materials, surfaces and groups, or null (numbers).
 */
template <class T, class S>
inline void
run(const S&           a_shape,
    const S*           a_deviceShape,
    const Camera<T>&   a_camera,
    const Settings<T>& a_settings,
    const RegionNames* a_names)
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

  for (const KeyGroup& group : keys(a_deviceShape != nullptr, hasRegionVisibility<S>)) {
    std::printf("\n%s\n", group.m_title);

    for (const Key& key : group.m_keys) {
      std::printf("  %-14s %s\n", key.m_keys, key.m_what);
    }
  }

  std::printf("\n");
  std::fflush(stdout);

  Camera<T>   camera   = a_camera;
  Settings<T> settings = a_settings;

  // Each frame records its pixels' objects, for outlining a picked one.
  settings.m_recordPrimitives = true;

  // The scene's length scale, for moving the section plane.
  const auto box = a_shape.computeBoundingVolume();

  // The shapes shown: copies, so that showing and hiding regions (see RegionView) changes them; every render is
  // given the ones current when it starts, as it is the settings and the camera.
  S shape       = a_shape;
  S deviceShape = (a_deviceShape != nullptr) ? *a_deviceShape : a_shape;

  // The region shown alone (0: all), stepped through with V.
  uint32_t soloRegion = 0;

  // The group of the scheme that Enter adds to (see RegionSettings::m_scheme).
  uint32_t currentGroup = 0;

  // The names of the regions, materials, surfaces and groups, as the program gave them.
  const RegionNames names = (a_names != nullptr) ? *a_names : RegionNames();

  // A scheme's selector, in words and as C++.
  const auto selectorName = [&names](const SurfaceSelector& a_selector) {
    std::string name;

    const auto add = [&name](const std::string& a_condition) { name += (name.empty() ? "" : ", ") + a_condition; };

    if (a_selector.m_surface != SurfaceSelector::Any) {
      add(names.surface(a_selector.m_surface));
    }
    if (a_selector.m_part != SurfaceSelector::Any) {
      add("part " + std::to_string(a_selector.m_part));
    }
    if (a_selector.m_regionA != SurfaceSelector::Any) {
      add((a_selector.m_regionB != SurfaceSelector::Any)
            ? names.region(a_selector.m_regionA) + " | " + names.region(a_selector.m_regionB)
            : names.region(a_selector.m_regionA) + " | any");
    }
    if (a_selector.m_label != SurfaceSelector::Any) {
      add("label " + names.surfaceLabel(a_selector.m_label));
    }
    if (a_selector.m_materialA != SurfaceSelector::Any) {
      add((a_selector.m_materialB != SurfaceSelector::Any)
            ? names.material(a_selector.m_materialA) + " | " + names.material(a_selector.m_materialB)
            : names.material(a_selector.m_materialA) + " | any");
    }

    return name.empty() ? std::string("everything") : name;
  };

  const auto selectorCode = [](const SurfaceSelector& a_selector) {
    const auto value = [](const uint32_t a_value) {
      return (a_value == SurfaceSelector::Any) ? std::string("SurfaceSelector::Any") : std::to_string(a_value) + "U";
    };

    const SurfaceSelector any;

    const bool noLabelOrMaterial = a_selector.m_label == any.m_label && a_selector.m_materialA == any.m_materialA &&
                                   a_selector.m_materialB == any.m_materialB;
    const bool noSurface = a_selector.m_surface == any.m_surface && a_selector.m_part == any.m_part;

    if (noLabelOrMaterial && noSurface && a_selector.m_regionA != any.m_regionA) {
      return (a_selector.m_regionB != any.m_regionB)
               ? "SurfaceSelector::interface(" + value(a_selector.m_regionA) + ", " + value(a_selector.m_regionB) + ")"
               : "SurfaceSelector::region(" + value(a_selector.m_regionA) + ")";
    }

    if (noLabelOrMaterial && a_selector.m_part == any.m_part && a_selector.m_regionA == any.m_regionA &&
        a_selector.m_surface != any.m_surface) {
      return "SurfaceSelector::surface(" + value(a_selector.m_surface) + ")";
    }

    return "SurfaceSelector{" + value(a_selector.m_surface) + ", " + value(a_selector.m_part) + ", " +
           value(a_selector.m_regionA) + ", " + value(a_selector.m_regionB) + ", " + value(a_selector.m_label) + ", " +
           value(a_selector.m_materialA) + ", " + value(a_selector.m_materialB) + "}";
  };

  // The panel of regions and groups, drawn over the image's top right corner while the regions' menu is shown: its
  // lines, and what a click on each does.
  std::vector<std::string>                      panelLines;
  std::vector<std::pair<PanelAction, uint32_t>> panelActions;
  SDL_Texture*                                  panel       = nullptr;
  int                                           panelWidth  = 0;
  int                                           panelHeight = 0;
  int                                           panelScale  = 1;
  std::vector<uint32_t>                         panelPixels;
  const T                                       diag  = (box.getHighCorner() - box.getLowCorner()).length();
  const T                                       scale = (std::isfinite(diag) && diag > T(0)) ? diag : camera.m_distance;

  // The section plane: off (0), as the settings give it (1, only when they turn it on), or across the x, y
  // or z axis through the box's center (2 to 4); moved along its normal by an offset, and reversed (the
  // other side removed) when flipped.
  const bool     givenSection  = a_settings.m_section;
  const Vec3T<T> center        = (std::isfinite(diag) && diag > T(0)) ? box.getCentroid() : camera.m_target;
  int            sectionMode   = givenSection ? 1 : 0;
  bool           sectionFlip   = false;
  T              sectionOffset = T(0);

  const char* sectionNames[] = {"Off", "As given", "Across x", "Across y", "Across z"};

  // The views, in the order of ViewMode, for the overlay and the window's title.
  const char* viewNames[] = {
    "Shaded view", "Depth view", "X-ray view", "Cost of firstHit() per pixel", "Cost of firstCrossing() per pixel"};

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

  // Stepping through the surfaces under the cursor (Shift + wheel): the crossings there, front to back, the one
  // picked, and the primitives in front of it, seen through while stepping (a click ends it).
  std::vector<HitT<T>>  depthHits;
  std::size_t           depthIndex = 0;
  std::vector<uint32_t> depthHidden;

  // Measuring: none, one point, or two (Shift and a click marks them).
  int      measured = 0;
  Vec3T<T> measureA = Vec3T<T>::zeros();
  Vec3T<T> measureB = Vec3T<T>::zeros();

  // The point under the cursor on the section plane, and the shape's value there.
  bool     cursorOn    = false;
  Vec3T<T> cursorPoint = Vec3T<T>::zeros();
  T        cursorValue = T(0);

  // Lines of equal distance on the section plane: off (0), as the settings give them (1, only when they set a
  // spacing), every 1/20 of the box's diagonal (2), every 1/50 (3).
  const T givenIso = a_settings.m_isoSpacing;
  int     isoMode  = (givenIso > T(0)) ? 1 : 0;

  // Images saved with W, numbered from 1.
  int saved = 0;

  // What the overlay reports about the frame last shown.
  Stats       shownStats;
  double      shownSeconds = 0.0;
  std::string shownWhat;
  uint32_t    shownWidth  = 0;
  uint32_t    shownHeight = 0;

  // The frame with the picked object outlined.
  std::vector<uint32_t> outlined;
  bool                  warned  = false;
  bool                  running = true;
  bool                  changed = true;

  // The next refinement stage to render (0: none left), and when the camera last moved.
  int  pending    = 0;
  auto lastChange = Clock::now();

  std::unique_ptr<Background> job;

  SDL_Texture* texture   = nullptr;
  uint32_t     texWidth  = 0;
  uint32_t     texHeight = 0;

  // The overlay: what the window shows and how, and the keys, drawn over the image's top left corner.
  bool overlayOn = true;

  // The menu the overlay shows (see keys()): one at a time, so that the overlay stays short.
  std::size_t           menu          = 0;
  SDL_Texture*          overlay       = nullptr;
  int                   overlayWidth  = 0;
  int                   overlayHeight = 0;
  std::vector<uint32_t> overlayPixels;

  // The axis triad, in the bottom left corner; shown and hidden with the overlay.
  SDL_Texture*          axes     = nullptr;
  int                   axesSize = 0;
  std::vector<uint32_t> axesPixels;

  // Where a point of the scene is in the window, in its pixels, as PixelQuery casts the rays; false for a point
  // behind a perspective camera.
  const auto project = [&](const Vec3T<T>& a_point, int& a_x, int& a_y) {
    Vec3T<T> forward;
    Vec3T<T> right;
    Vec3T<T> up;

    camera.frame(forward, right, up);

    const Vec3T<T> relative = a_point - camera.eye();
    const T        depth    = camera.m_orthographic ? camera.m_distance : dot(relative, forward);

    // A point at or just in front of the eye would project far outside any window.
    if (depth <= T(1e-6) * camera.m_distance) {
      return false;
    }

    const T tanHalf = std::tan(T(0.5) * camera.m_fieldOfView * pi<T> / T(180));
    const T w       = T(settings.m_width);
    const T h       = T(settings.m_height);
    const T x       = dot(relative, right) / depth / tanHalf;
    const T y       = dot(relative, up) / depth / tanHalf;

    a_x = static_cast<int>(Math::clamp(T(0.5) * (x * h + w), T(-1e6), T(1e6)));
    a_y = static_cast<int>(Math::clamp(T(0.5) * (h - y * h), T(-1e6), T(1e6)));

    return true;
  };

  // Draw the image, then the overlay over it.
  const auto present = [&]() {
    SDL_RenderClear(screen);

    if (texture != nullptr) {
      SDL_RenderCopy(screen, texture, nullptr, nullptr);
    }

    if (overlay != nullptr) {
      const SDL_Rect where{8, 8, overlayWidth, overlayHeight};

      SDL_RenderCopy(screen, overlay, nullptr, &where);
    }

    if (panel != nullptr && !panelLines.empty()) {
      int outputWidth  = 0;
      int outputHeight = 0;

      SDL_GetRendererOutputSize(screen, &outputWidth, &outputHeight);

      const SDL_Rect where{outputWidth - panelWidth - 8, 8, panelWidth, panelHeight};

      SDL_RenderCopy(screen, panel, nullptr, &where);
    }

    if (axes != nullptr) {
      int outputWidth  = 0;
      int outputHeight = 0;

      SDL_GetRendererOutputSize(screen, &outputWidth, &outputHeight);

      const SDL_Rect where{8, outputHeight - axesSize - 8, axesSize, axesSize};

      SDL_RenderCopy(screen, axes, nullptr, &where);
    }

    // The measurement, over the image: a marker at each point and a line between them.
    if (measured > 0) {
      int ax = 0;
      int ay = 0;
      int bx = 0;
      int by = 0;

      const bool seeA = project(measureA, ax, ay);
      const bool seeB = measured > 1 && project(measureB, bx, by);

      SDL_SetRenderDrawColor(screen, 255, 184, 26, 255);

      if (seeA && seeB) {
        for (int d = -1; d <= 1; d++) {
          SDL_RenderDrawLine(screen, ax + d, ay, bx + d, by);
          SDL_RenderDrawLine(screen, ax, ay + d, bx, by + d);
        }
      }

      const auto mark = [&screen](const int a_x, const int a_y) {
        const SDL_Rect marker{a_x - 4, a_y - 4, 9, 9};

        SDL_RenderFillRect(screen, &marker);
      };

      if (seeA) {
        mark(ax, ay);
      }

      if (seeB) {
        mark(bx, by);
      }

      SDL_SetRenderDrawColor(screen, 0, 0, 0, 255);
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
      point                   = center;
    }

    if (sectionFlip) {
      normal = -normal;
    }

    // The arrow keys move the plane towards the removed side, so that more of the shape shows.
    frame.m_section       = sectionMode != 0;
    frame.m_sectionNormal = normal;
    frame.m_sectionPoint  = point + sectionOffset * normal;

    // While stepping through the surfaces under the cursor, those in front of the one picked are seen through, in
    // the entries of m_hidden left free.
    std::size_t next = 0;

    for (uint32_t i = 0; i < Settings<T>::HiddenPrimitives && next < depthHidden.size(); i++) {
      if (frame.m_hidden[i] == Settings<T>::NoPrimitive) {
        frame.m_hidden[i] = depthHidden[next++];
      }
    }

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
    const bool timed = settings.m_viewMode == ViewMode::CostHit || settings.m_viewMode == ViewMode::CostCrossing;

    // A cost view traces one ray per pixel, unshaded: the full settings add nothing to it.
    return a_stage == 1 || (!timed && (settings.m_supersampling > 1U || settings.m_shadows || settings.m_occlusion));
  };

  // Everything a render reads is passed by value, so that a background render never sees the camera or
  // the settings change under it.
  const auto render = [](const S&           a_host,
                         const S&           a_device,
                         const Settings<T>& a_frame,
                         const Camera<T>    a_view,
                         const bool         a_onDevice,
                         Image&             a_image,
                         Background*        a_job) {
    const Renderer<T> renderer(a_frame);

    if (a_onDevice && a_job != nullptr) {
      return renderer.renderOnDevice(a_device, a_view, a_image, a_job->m_cancel, &a_job->m_rows);
    }
    if (a_onDevice) {
      return renderer.renderOnDevice(a_device, a_view, a_image);
    }
    if (a_job != nullptr) {
      return renderer.render(a_host, a_view, a_image, a_job->m_cancel, &a_job->m_rows);
    }

    return renderer.render(a_host, a_view, a_image);
  };

  // The point under the cursor (window coordinates) on the section plane, within the shape's box, and the
  // shape's value there: the ray PixelQuery casts through it, met with the plane.
  const auto readCursor = [&](const int a_x, const int a_y) {
    int windowWidth  = 0;
    int windowHeight = 0;

    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    const Settings<T> frame = stageSettings(2);

    Vec3T<T> forward;
    Vec3T<T> right;
    Vec3T<T> up;

    camera.frame(forward, right, up);

    const T tanHalf = std::tan(T(0.5) * camera.m_fieldOfView * pi<T> / T(180));
    const T w       = T(frame.m_width);
    const T h       = T(frame.m_height);
    const T column  = T(a_x) * w / T(Math::max(windowWidth, 1));
    const T row     = T(a_y) * h / T(Math::max(windowHeight, 1));
    const T x       = (T(2) * column - w) / h * tanHalf;
    const T y       = (h - T(2) * row) / h * tanHalf;

    const Vec3T<T> origin =
      camera.m_orthographic ? camera.eye() + camera.m_distance * (x * right + y * up) : camera.eye();
    const Vec3T<T> along  = camera.m_orthographic ? forward : RenderDetail::unit(forward + x * right + y * up);
    const Vec3T<T> normal = RenderDetail::unit(frame.m_sectionNormal);
    const T        rate   = dot(along, normal);

    cursorOn = false;

    if (rate != T(0)) {
      const Vec3T<T> p = origin + (dot(frame.m_sectionPoint - origin, normal) / rate) * along;

      cursorOn = std::isfinite(diag) && diag > T(0);

      for (int k = 0; k < 3 && cursorOn; k++) {
        const T margin = T(0.05) * (box.getHighCorner()[k] - box.getLowCorner()[k]);

        cursorOn = p[k] >= box.getLowCorner()[k] - margin && p[k] <= box.getHighCorner()[k] + margin;
      }

      if (cursorOn) {
        cursorPoint = p;
        cursorValue = RenderDetail::shownAs(shape, settings).signedDistance(p);
      }
    }
  };

  // The overlay and the axes, from the frame last shown and the window's state; redrawn without rendering
  // when only that state changes (the point under the cursor, a measurement).
  const auto drawOverlay = [&]() {
    // The overlay's text, at a size that stays readable on a window with many pixels.
    const auto percent = [](const uint64_t a_part, const uint64_t a_whole) {
      return fixed(100.0 * double(a_part) / double(Math::max(a_whole, uint64_t(1))), 1) + "%";
    };
    const auto onOff = [](const bool a_on) { return a_on ? "on" : "off"; };
    const auto point = [](const Vec3T<T>& a_v, const int a_decimals) {
      return "(" + fixed(double(a_v[0]), a_decimals) + ", " + fixed(double(a_v[1]), a_decimals) + ", " +
             fixed(double(a_v[2]), a_decimals) + ")";
    };

    const char* coloring = (settings.m_colorBy == ColorBy::Normal)      ? "Colored by normal"
                           : (settings.m_colorBy == ColorBy::Primitive) ? "Colored by primitive"
                                                                        : "Colored uniformly";

    const bool costView = settings.m_viewMode == ViewMode::CostHit || settings.m_viewMode == ViewMode::CostCrossing;

    // A time in nanoseconds, in the unit that keeps it short.
    const auto duration = [](const double a_ns) {
      return (a_ns >= 1e6)   ? fixed(1e-6 * a_ns, 1) + " ms"
             : (a_ns >= 1e3) ? fixed(1e-3 * a_ns, 1) + " us"
                             : fixed(a_ns, 0) + " ns";
    };

    const std::string view =
      std::string(viewNames[static_cast<int>(settings.m_viewMode)]) +
      (costView ? ", " + duration(shownStats.m_costLow) + " to " + duration(shownStats.m_costHigh) +
                    " (dark to bright)" + (onDevice ? ", elapsed on the GPU, waits included" : "")
                : std::string());

    // The primitives seen through.
    std::string hidden;

    for (uint32_t i = 0; i < Settings<T>::HiddenPrimitives; i++) {
      if (settings.m_hidden[i] != Settings<T>::NoPrimitive) {
        hidden += (hidden.empty() ? "" : ", ") + std::to_string(settings.m_hidden[i]);
      }
    }

    const std::string object = primitivesAreFaces<S> ? "face" : "object";

    // Each group's state, in the order of keys(): the camera and the light, rendering, the section, picking.
    std::vector<std::vector<std::string>> states{
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
      {view,
       std::string(coloring) + ", rendered on the " + (onDevice ? "GPU" : "CPU"),
       "Supersampling " + std::to_string(settings.m_supersampling) + " x " + std::to_string(settings.m_supersampling) +
         " (" + std::to_string(settings.m_supersampling * settings.m_supersampling) + " rays per pixel)",
       std::string("Shadows ") + onOff(settings.m_shadows) + ", ambient occlusion " + onOff(settings.m_occlusion) +
         ", floor " + onOff(settings.m_floor) + ", sky " + onOff(settings.m_sky)},
      {std::string(sectionNames[sectionMode]) + ((sectionMode != 0) ? std::string(sectionFlip ? ", reversed" : "") +
                                                                        ", moved " + fixed(double(sectionOffset), 3)
                                                                    : std::string()),
       "Contour " + fixed(double(settings.m_contourWidth), 1) + " pixels wide, sheet " +
         fixed(100.0 * double(settings.m_sheetOpacity), 0) + "% opaque, cut face " +
         fixed(100.0 * double(settings.m_cutOpacity), 0) + "% opaque",
       (settings.m_isoSpacing > T(0)) ? "Lines of equal distance every " + fixed(double(settings.m_isoSpacing), 4)
                                      : std::string("No lines of equal distance"),
       cursorOn ? "Under the cursor " + point(cursorPoint, 3) + ", distance " + fixed(double(cursorValue), 4)
                : std::string("Nothing under the cursor on the plane")},
      {picked ? (primitivesAreFaces<S> ? "Object 0 (face " + std::to_string(pick.m_primitive) + ")"
                                       : "Object " + std::to_string(pick.m_primitive)) +
                  " at " + point(pick.m_point, 3) + ", normal " + point(pick.m_normal, 2)
              : std::string("Nothing picked"),
       hidden.empty() ? std::string("Every surface opaque") : "Seen through: " + object + " " + hidden,
       depthHits.empty() ? std::string("Not stepping through surfaces")
                         : "Surface " + std::to_string(depthIndex + 1) + " of " + std::to_string(depthHits.size()) +
                             " under the cursor",
       (measured == 2) ? "Measured " + fixed(double((measureB - measureA).length()), 4) + ", along x, y, z " +
                           point(measureB - measureA, 4)
       : (measured == 1) ? "Measuring from " + point(measureA, 3) + "; Shift + click the second point"
                         : std::string("Nothing measured")},
      {}};

    // The regions shown and hidden, for a shape whose regions can be (see RegionView), and the panel of regions and
    // groups while the regions' menu is shown.
    panelLines.clear();
    panelActions.clear();

    if constexpr (hasRegionVisibility<S>) {
      std::string shown;
      std::string hiddenRegions;

      const RegionSettings& regions = settings.m_regions;
      const Scheme&         scheme  = regions.m_scheme;

      for (uint32_t k = 1; k < shape.getNumRegions(); k++) {
        std::string& list = regions.isVisible(k, shape.getNumRegions()) ? shown : hiddenRegions;

        list += (list.empty() ? "" : ", ") + names.region(k);
      }

      const char* colorings[] = {"region", "material", "surface label", "surface", "part", "interface", "group"};

      const auto pieceName = [&](const SurfaceRef<T>& a_piece) {
        if (a_piece.m_surface == SurfaceRef<T>::None) {
          return std::string("no surface");
        }

        return names.surface(a_piece.m_surface) + ", part " + std::to_string(a_piece.m_part) + ", inside " +
               names.region(a_piece.m_insideRegion) + ", outside " + names.region(a_piece.m_outsideRegion);
      };

      const auto keyName = [&](const uint32_t a_key) {
        if (a_key == RegionSettings::None) {
          return std::string("nothing");
        }

        switch (regions.m_coloring) {
        case RegionColoring::Region: {
          return names.region(a_key);
        }
        case RegionColoring::Material: {
          return names.material(a_key);
        }
        case RegionColoring::SurfaceLabel: {
          return names.surfaceLabel(a_key);
        }
        case RegionColoring::Surface: {
          return names.surface(a_key);
        }
        case RegionColoring::Interface: {
          return "interface " + names.region(a_key >> 16U) + " | " + names.region(a_key & 0xFFFFU);
        }
        case RegionColoring::Group: {
          return names.group(a_key);
        }
        default: {
          return "part " + std::to_string(a_key);
        }
        }
      };

      states.insert(
        states.end() - 1,
        {std::string("Colored by ") + colorings[static_cast<int>(regions.m_coloring)] +
           ((regions.m_only != RegionSettings::None) ? "; showing only " + keyName(regions.m_only) : std::string()),
         picked ? "Picked: " + keyName(pick.m_primitive) + "; " + pieceName(shape.pieceAt(pick.m_point, pick.m_normal))
                : std::string("Nothing picked"),
         [&regions, &keyName]() {
           std::string through;

           for (uint32_t i = 0; i < RegionSettings::MaxSeenThrough; i++) {
             if (regions.m_through[i] != RegionSettings::None) {
               through += (through.empty() ? "Seen through: " : ", ") + keyName(regions.m_through[i]);
             }
           }

           return through.empty() ? std::string("Every surface opaque (T sees through the picked one)") : through;
         }(),
         hiddenRegions.empty()
           ? "Every region shown (" + std::to_string(shape.getNumRegions() - 1U) + ")"
           : "Shown: " + (shown.empty() ? std::string("none") : shown) + "; hidden: " + hiddenRegions,
         (scheme.getNumGroups() == 0U) ? std::string("No groups (Enter starts one with the picked piece)")
                                       : "Adding to group " + names.group(currentGroup) + " (J: next)"});

      // The panel: each region, shown or hidden, and each group with its selectors; a click on a line acts on it.
      const std::vector<KeyGroup> menus = keys(a_deviceShape != nullptr, true);

      if (overlayOn && menu < menus.size() && std::string(menus[menu].m_title) == "Regions") {
        const auto line = [&](const std::string& a_text, const PanelAction a_action, const uint32_t a_value) {
          panelLines.push_back(a_text);
          panelActions.emplace_back(a_action, a_value);
        };

        line("Regions (click: show or hide)", PanelAction::None, 0U);

        for (uint32_t k = 1; k < shape.getNumRegions(); k++) {
          line(std::string(regions.isVisible(k, shape.getNumRegions()) ? "  [x] " : "  [ ] ") + names.region(k),
               PanelAction::ToggleRegion,
               k);
        }

        line("", PanelAction::None, 0U);
        line("Groups (click: add to it; a selector: remove it)", PanelAction::None, 0U);

        for (uint32_t g = 0; g < scheme.getNumGroups(); g++) {
          line(std::string(g == currentGroup ? "  > " : "    ") + names.group(g), PanelAction::CurrentGroup, g);

          for (uint32_t i = 0; i < scheme.getNumSelectors(); i++) {
            if (scheme.getGroupOfSelector(i) == g) {
              line("      " + selectorName(scheme.getSelector(i)), PanelAction::RemoveSelector, i);
            }
          }
        }

        if (scheme.getNumGroups() < Scheme::MaxGroups) {
          line("    + new group", PanelAction::NewGroup, 0U);
        }
      }
    }

    std::vector<std::string> lines{
      std::string("EBGeometry viewer: ") + shownWhat + ", " + std::to_string(shownWidth) + " x " +
        std::to_string(shownHeight) + " pixels in " + fixed(1000.0 * shownSeconds, 0) + " ms",
      "Rays " + std::to_string(shownStats.m_rays) + ", hits " + percent(shownStats.m_hits, shownStats.m_rays) +
        ", seen from inside " + std::to_string(shownStats.m_insideHits) + ", unresolved " +
        std::to_string(shownStats.m_unresolved)};

    const std::vector<KeyGroup> groups = keys(a_deviceShape != nullptr, hasRegionVisibility<S>);

    // The status: what is picked, and the menus, the one shown in brackets. Every key works whichever is shown.
    if (states.size() > 3 && !states[3].empty()) {
      lines.emplace_back(states[3].front());
    }

    std::string tabs;

    for (std::size_t g = 0; g < groups.size(); g++) {
      const bool shown = overlayOn && g == menu;

      tabs += (g > 0 ? "  " : "") + std::string(shown ? "[" : "") + "F" + std::to_string(g + 1) + " " +
              groups[g].m_title + (shown ? "]" : "");
    }

    lines.emplace_back(tabs + (overlayOn ? "   (Tab: next, H: hide)" : "   (H: show)"));

    if (overlayOn && menu < groups.size()) {
      lines.emplace_back("");
      lines.emplace_back(groups[menu].m_title);

      for (const std::string& state : states[menu]) {
        lines.emplace_back("  " + state);
      }

      for (const Key& key : groups[menu].m_keys) {
        char line[128];

        std::snprintf(line, sizeof(line), "  %-18s %s", key.m_keys, key.m_what);
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

    // The panel of regions and groups, at the overlay's size.
    if (!panelLines.empty()) {
      const int oldPanelWidth  = panelWidth;
      const int oldPanelHeight = panelHeight;

      drawText(panelLines, scale, panelPixels, panelWidth, panelHeight);

      panelScale = scale;

      if (panel == nullptr || oldPanelWidth != panelWidth || oldPanelHeight != panelHeight) {
        if (panel != nullptr) {
          SDL_DestroyTexture(panel);
        }

        panel =
          SDL_CreateTexture(screen, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, panelWidth, panelHeight);

        SDL_SetTextureBlendMode(panel, SDL_BLENDMODE_BLEND);
      }

      SDL_UpdateTexture(panel, nullptr, panelPixels.data(), 4 * panelWidth);
    }

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

    // A picked object, tinted gold in the render, is also outlined: the pixels next to it, within two pixels,
    // that are not part of it turn gold. A mesh SDF is one object, whatever face a pixel shows.
    const uint32_t* pixels = a_image.m_pixels.data();

    if (picked && a_image.m_primitives.size() == a_image.m_pixels.size()) {
      const auto inside = [&](const int a_x, const int a_y) {
        const uint32_t id = a_image.m_primitives[std::size_t(a_y) * a_image.m_width + std::size_t(a_x)];

        return primitivesAreFaces<S> ? id != 0xFFFFFFFFU : id == pick.m_primitive;
      };

      const int width  = static_cast<int>(a_image.m_width);
      const int height = static_cast<int>(a_image.m_height);
      const int reach  = 2;

      outlined = a_image.m_pixels;

      for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
          if (inside(x, y)) {
            continue;
          }

          bool near = false;

          for (int dy = -reach; dy <= reach && !near; dy++) {
            for (int dx = -reach; dx <= reach && !near; dx++) {
              const int u = x + dx;
              const int v = y + dy;

              near = u >= 0 && v >= 0 && u < width && v < height && inside(u, v);
            }
          }

          if (near) {
            outlined[std::size_t(y) * a_image.m_width + std::size_t(x)] = 0xFFB81AFFU;
          }
        }
      }

      pixels = outlined.data();
    }

    SDL_UpdateTexture(texture, nullptr, pixels, static_cast<int>(4U * a_image.m_width));

    status = std::string("EBGeometry: ") + viewNames[static_cast<int>(settings.m_viewMode)] + ", " +
             (onDevice ? "GPU" : "CPU") + ", " + a_what + " " + std::to_string(a_image.m_width) + "x" +
             std::to_string(a_image.m_height) + " in " + std::to_string(int(1000.0 * a_seconds)) + " ms" +
             // A few grazing rays can seem to leave the solid; say so only when more than one hit in a
             // thousand does.
             ((a_stats.m_insideHits * 1000U > a_stats.m_hits) ? ", red: seen from inside the solid" : "") +
             ((a_stats.m_unresolved > 0U) ? ", some rays unresolved" : "");

    SDL_SetWindowTitle(window, status.c_str());

    shownStats   = a_stats;
    shownSeconds = a_seconds;
    shownWhat    = a_what;
    shownWidth   = a_image.m_width;
    shownHeight  = a_image.m_height;

    drawOverlay();
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

            // A click on the panel of regions and groups acts on the line under it.
            if constexpr (hasRegionVisibility<S>) {
              int outputWidth  = 0;
              int outputHeight = 0;

              SDL_GetRendererOutputSize(screen, &outputWidth, &outputHeight);

              const int x = event.button.x * outputWidth / Math::max(windowWidth, 1);
              const int y = event.button.y * outputHeight / Math::max(windowHeight, 1);

              const int left = outputWidth - panelWidth - 8;
              const int row  = (y - 8 - 6 * panelScale) / (GlyphHeight * panelScale);

              if (!panelLines.empty() && x >= left && x < left + panelWidth && y >= 8 && y < 8 + panelHeight) {
                if (y >= 8 + 6 * panelScale && row >= 0 && std::size_t(row) < panelActions.size()) {
                  RegionSettings& regions = settings.m_regions;
                  Scheme&         scheme  = regions.m_scheme;

                  const uint32_t value = panelActions[std::size_t(row)].second;

                  switch (panelActions[std::size_t(row)].first) {
                  case PanelAction::ToggleRegion: {
                    regions.setVisible(value, !regions.isVisible(value, shape.getNumRegions()));

                    break;
                  }
                  case PanelAction::CurrentGroup: {
                    currentGroup = value;

                    break;
                  }
                  case PanelAction::RemoveSelector: {
                    scheme.remove(scheme.getGroupOfSelector(value), scheme.getSelector(value));

                    break;
                  }
                  case PanelAction::NewGroup: {
                    currentGroup = scheme.addGroup();

                    break;
                  }
                  default: {
                    break;
                  }
                  }
                }

                dragging = false;
                panning  = false;
                changed  = true;

                break;
              }
            }

            const Settings<T> frame  = stageSettings(2);
            const T           column = T(event.button.x) * T(frame.m_width) / T(Math::max(windowWidth, 1));
            const T           row    = T(event.button.y) * T(frame.m_height) / T(Math::max(windowHeight, 1));

            const HitT<T> hit = Renderer<T>(frame).pick(shape, camera, column, row);

            // Shift and a click marks a point to measure from or to, not a pick.
            if ((SDL_GetModState() & KMOD_SHIFT) != 0) {
              if (!hit.hit()) {
                measured = 0;
              }
              else if (measured == 1) {
                measureB = hit.m_point;
                measured = 2;

                std::printf("EBGeometry viewer: measured %g from (%g, %g, %g) to (%g, %g, %g)\n",
                            double((measureB - measureA).length()),
                            double(measureA[0]),
                            double(measureA[1]),
                            double(measureA[2]),
                            double(measureB[0]),
                            double(measureB[1]),
                            double(measureB[2]));
                std::fflush(stdout);
              }
              else {
                measureA = hit.m_point;
                measured = 1;
              }

              drawOverlay();
              present();

              dragging = false;
              panning  = false;

              break;
            }

            // A click while stepping through surfaces keeps the one stepped to, and stops stepping.
            if (!depthHits.empty()) {
              depthHits.clear();
              depthHidden.clear();

              dragging = false;
              panning  = false;
              changed  = true;

              break;
            }

            pick   = hit;
            picked = pick.hit();

            // A double-click also centers the camera on the point.
            if (picked && event.button.clicks >= 2) {
              camera.m_target = pick.m_point;
            }

            // The object picked: a tape's leaf or a BVHUnion's member (any other shape names primitive 0, so the
            // whole of it); a mesh SDF is one object, whose primitive is the face picked on it. It is tinted gold,
            // and outlined (see show()).
            settings.m_highlightPrimitive = !picked                 ? Settings<T>::NoPrimitive
                                            : primitivesAreFaces<S> ? Settings<T>::AllPrimitives
                                                                    : pick.m_primitive;
            if (picked && primitivesAreFaces<S>) {
              std::printf("EBGeometry viewer: picked object 0, face %u, at (%g, %g, %g), normal (%g, %g, %g)\n",
                          unsigned(pick.m_primitive),
                          double(pick.m_point[0]),
                          double(pick.m_point[1]),
                          double(pick.m_point[2]),
                          double(pick.m_normal[0]),
                          double(pick.m_normal[1]),
                          double(pick.m_normal[2]));
            }
            else if (picked) {
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
          depthHits.clear();
          depthHidden.clear();

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
          depthHits.clear();
          depthHidden.clear();

          camera.m_yaw -= T(0.4) * T(event.motion.xrel);
          camera.m_pitch = Math::clamp(camera.m_pitch + T(0.4) * T(event.motion.yrel), T(-90), T(90));

          changed = true;
        }
        else if (sectionMode != 0) {
          readCursor(event.motion.x, event.motion.y);
          drawOverlay();
          present();
        }

        break;
      }
      case SDL_MOUSEWHEEL: {
        // With Ctrl held the wheel moves the section plane in fine steps; otherwise it zooms. A sideways
        // scroll (y = 0) does neither.
        if (event.wheel.y != 0) {
          if ((SDL_GetModState() & KMOD_SHIFT) != 0) {
            // Shift and the wheel step through the surfaces under the cursor: the first step picks the nearest,
            // rolling down steps deeper, rolling up back towards the eye. Each is picked in turn, and the primitives
            // in front of it are seen through; for a mesh SDF, whose primitives are its faces, only the faces under
            // the cursor.
            int windowWidth  = 0;
            int windowHeight = 0;
            int mouseX       = 0;
            int mouseY       = 0;

            SDL_GetWindowSize(window, &windowWidth, &windowHeight);
            SDL_GetMouseState(&mouseX, &mouseY);

            Settings<T> frame = stageSettings(2);

            // The surfaces are listed as the user's settings show them, without those stepped past.
            frame.m_hidden = settings.m_hidden;

            const T column = T(mouseX) * T(frame.m_width) / T(Math::max(windowWidth, 1));
            const T row    = T(mouseY) * T(frame.m_height) / T(Math::max(windowHeight, 1));

            if (depthHits.empty()) {
              depthHits  = Renderer<T>(frame).pickAll(shape, camera, column, row);
              depthIndex = 0;
            }
            else if (event.wheel.y < 0 && depthIndex + 1 < depthHits.size()) {
              depthIndex++;
            }
            else if (event.wheel.y > 0 && depthIndex > 0) {
              depthIndex--;
            }

            depthHidden.clear();

            if (!depthHits.empty()) {
              pick   = depthHits[depthIndex];
              picked = true;

              settings.m_highlightPrimitive = primitivesAreFaces<S> ? Settings<T>::AllPrimitives : pick.m_primitive;

              for (std::size_t i = 0; i < depthIndex; i++) {
                const uint32_t id = depthHits[i].m_primitive;

                if (id != pick.m_primitive &&
                    std::find(depthHidden.begin(), depthHidden.end(), id) == depthHidden.end()) {
                  depthHidden.push_back(id);
                }
              }
            }
          }
          else if ((SDL_GetModState() & KMOD_CTRL) != 0 && sectionMode != 0) {
            sectionOffset += T(0.005) * scale * T(event.wheel.y);
          }
          else {
            camera.m_distance *= (event.wheel.y > 0) ? T(0.9) : T(1) / T(0.9);

            depthHits.clear();
            depthHidden.clear();
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
        // A key that changes nothing (H, C, a modifier on its own) keeps what earlier events in this batch
        // changed, rather than clearing it.
        const bool pending = changed;

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
          settings.m_colorBy = (settings.m_colorBy == ColorBy::Normal)    ? ColorBy::Uniform
                               : (settings.m_colorBy == ColorBy::Uniform) ? ColorBy::Primitive
                                                                          : ColorBy::Normal;

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
        case SDLK_b: {
          settings.m_sky = !settings.m_sky;

          break;
        }
        case SDLK_PLUS:
        case SDLK_KP_PLUS:
        case SDLK_EQUALS:
        case SDLK_MINUS:
        case SDLK_KP_MINUS: {
          // A narrower view (plus) shows less, larger; on a keyboard without a plus key, = does.
          const bool narrower = event.key.keysym.sym != SDLK_MINUS && event.key.keysym.sym != SDLK_KP_MINUS;

          camera.m_fieldOfView = Math::clamp(camera.m_fieldOfView + (narrower ? T(-5) : T(5)), T(10), T(120));

          break;
        }
        case SDLK_i: {
          // Off, as the settings give them (skipped when they set no spacing), every 1/20 of the box's
          // diagonal, every 1/50.
          isoMode = (isoMode + 1) % 4;
          isoMode = (isoMode == 1 && givenIso <= T(0)) ? 2 : isoMode;

          settings.m_isoSpacing = (isoMode == 0)   ? T(0)
                                  : (isoMode == 1) ? givenIso
                                                   : scale / ((isoMode == 2) ? T(20) : T(50));

          break;
        }
        case SDLK_w: {
          // The full frame at the window's size, rendered now, as a PPM image in the working directory.
          Image             image;
          const Settings<T> frame = stageSettings(2);

          (void)render(shape, deviceShape, frame, camera, onDevice, image, nullptr);

          // The next name not taken, so that an earlier run's images are kept.
          const auto taken = [](const std::string& a_name) {
            std::FILE* existing = std::fopen(a_name.c_str(), "rb");

            if (existing != nullptr) {
              std::fclose(existing);
            }

            return existing != nullptr;
          };

          std::string file = "ebgeometry-view-" + std::to_string(++saved) + ".ppm";

          while (taken(file)) {
            file = "ebgeometry-view-" + std::to_string(++saved) + ".ppm";
          }

          if (writeImage(image, file)) {
            std::printf("EBGeometry viewer: saved %s (%u x %u)\n", file.c_str(), image.m_width, image.m_height);
          }
          else {
            std::printf("EBGeometry viewer: could not write %s\n", file.c_str());
          }

          std::fflush(stdout);

          changed = pending;

          break;
        }
        case SDLK_TAB: {
          const std::size_t count = keys(a_deviceShape != nullptr, hasRegionVisibility<S>).size();

          menu      = ((event.key.keysym.mod & KMOD_SHIFT) != 0) ? (menu + count - 1) % count : (menu + 1) % count;
          overlayOn = true;
          changed   = pending;

          drawOverlay();
          present();

          break;
        }
        case SDLK_F1:
        case SDLK_F2:
        case SDLK_F3:
        case SDLK_F4:
        case SDLK_F5:
        case SDLK_F6: {
          const std::size_t count = keys(a_deviceShape != nullptr, hasRegionVisibility<S>).size();

          menu      = Math::min(static_cast<std::size_t>(event.key.keysym.sym - SDLK_F1), count - 1);
          overlayOn = true;
          changed   = pending;

          drawOverlay();
          present();

          break;
        }
        case SDLK_h: {
          overlayOn = !overlayOn;
          changed   = pending;

          drawOverlay();
          present();

          break;
        }
        case SDLK_c: {
          // The view as C++, to paste into a program: the camera, and the settings of the full frame.
          // The settings as the program gave them, but for what the window changed; not the viewer's own.
          Settings<T> shown = stageSettings(2);

          shown.m_recordPrimitives = a_settings.m_recordPrimitives;

          std::printf("EBGeometry viewer: the current view, as C++:\n%s", toCode(camera, shown).c_str());

          // The scheme built by clicking, to paste into the program (into the settings' region settings).
          if constexpr (hasRegionVisibility<S>) {
            const Scheme& scheme = settings.m_regions.m_scheme;

            if (scheme.getNumGroups() > 0U) {
              std::printf("Scheme scheme;\n");

              for (uint32_t g = 0; g < scheme.getNumGroups(); g++) {
                std::printf("const uint32_t group%u = scheme.addGroup(); // %s\n", unsigned(g), names.group(g).c_str());
              }

              for (uint32_t i = 0; i < scheme.getNumSelectors(); i++) {
                std::printf("scheme.add(group%u, %s); // %s\n",
                            unsigned(scheme.getGroupOfSelector(i)),
                            selectorCode(scheme.getSelector(i)).c_str(),
                            selectorName(scheme.getSelector(i)).c_str());
              }
            }
          }

          std::fflush(stdout);

          changed = pending;

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
        case SDLK_m: {
          // M steps forward through the five views, Shift and M back.
          const int step = ((event.key.keysym.mod & KMOD_SHIFT) != 0) ? 4 : 1;

          settings.m_viewMode = static_cast<ViewMode>((static_cast<int>(settings.m_viewMode) + step) % 5);

          break;
        }
        case SDLK_l: {
          if constexpr (hasRegionVisibility<S>) {
            RegionSettings& regions = settings.m_regions;

            regions.m_only =
              (regions.m_only == RegionSettings::None && picked) ? pick.m_primitive : RegionSettings::None;
          }
          else {
            changed = pending;
          }

          break;
        }
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
        case SDLK_BACKSPACE: {
          if constexpr (hasRegionVisibility<S>) {
            Scheme&             scheme = settings.m_regions.m_scheme;
            const SurfaceRef<T> piece  = picked ? shape.pieceAt(pick.m_point, pick.m_normal) : SurfaceRef<T>();

            if (piece.m_surface == SurfaceRef<T>::None) {
              changed = pending;

              break;
            }

            const SurfaceSelector exact = SurfaceSelector::piece(piece);

            if (event.key.keysym.sym == SDLK_BACKSPACE) {
              // Every selector of the current group that holds the piece goes.
              for (uint32_t i = scheme.getNumSelectors(); i > 0U; i--) {
                const SurfaceSelector selector = scheme.getSelector(i - 1U);

                if (scheme.getGroupOfSelector(i - 1U) == currentGroup && selector.matches(piece)) {
                  scheme.remove(currentGroup, selector);
                }
              }
            }
            else {
              const SurfaceSelector selector =
                ((event.key.keysym.mod & KMOD_CTRL) != 0) ? SurfaceSelector::interface(exact.m_regionA, exact.m_regionB)
                : ((event.key.keysym.mod & KMOD_SHIFT) != 0) ? SurfaceSelector::surface(exact.m_surface)
                                                             : exact;

              if (scheme.getNumGroups() == 0U) {
                currentGroup = scheme.addGroup();
              }

              if (scheme.getNumSelectors() < Scheme::MaxSelectors) {
                scheme.remove(currentGroup, selector);
                scheme.add(currentGroup, selector);
              }
              else {
                std::printf("EBGeometry viewer: the scheme holds %u selectors already\n",
                            unsigned(Scheme::MaxSelectors));
                std::fflush(stdout);
              }
            }

            // Show the groups.
            if (settings.m_regions.m_coloring != RegionColoring::Group) {
              settings.m_regions.setColoring(RegionColoring::Group);
            }

            settings.m_colorBy = ColorBy::Primitive;
          }
          else {
            changed = pending;
          }

          break;
        }
        case SDLK_j: {
          if constexpr (hasRegionVisibility<S>) {
            Scheme& scheme = settings.m_regions.m_scheme;

            if (currentGroup + 1U < scheme.getNumGroups()) {
              currentGroup++;
            }
            else if (scheme.getNumGroups() < Scheme::MaxGroups) {
              currentGroup = scheme.addGroup();
            }
            else {
              currentGroup = 0;
            }
          }
          else {
            changed = pending;
          }

          break;
        }
        case SDLK_k: {
          if constexpr (hasRegionVisibility<S>) {
            // Group only with a scheme to color by.
            const int count = (settings.m_regions.m_scheme.getNumGroups() > 0U) ? 7 : 6;

            settings.m_regions.setColoring(
              static_cast<RegionColoring>((static_cast<int>(settings.m_regions.m_coloring) + 1) % count));

            settings.m_colorBy = ColorBy::Primitive;
          }
          else {
            changed = pending;
          }

          break;
        }
        case SDLK_v: {
          if constexpr (hasRegionVisibility<S>) {
            RegionSettings& regions = settings.m_regions;

            if ((event.key.keysym.mod & KMOD_CTRL) != 0) {
              soloRegion       = 0;
              regions.m_hidden = 0U;
            }
            else if ((event.key.keysym.mod & KMOD_SHIFT) != 0) {
              // The picked key is a region's when coloring by region.
              if (picked && regions.m_coloring == RegionColoring::Region && pick.m_primitive < shape.getNumRegions()) {
                regions.setVisible(pick.m_primitive, false);

                picked = false;
              }
            }
            else {
              soloRegion = (soloRegion + 1U < shape.getNumRegions()) ? soloRegion + 1U : 0U;

              if (soloRegion == 0U) {
                regions.m_hidden = 0U;
              }
              else {
                regions.showOnly(soloRegion);
              }
            }
          }
          else {
            changed = pending;
          }

          break;
        }
        case SDLK_t: {
          // A shape with regions sees through in itself, so that a surface between a region seen through and another
          // shows as the other's (see RegionView::withSeenThrough()).
          if constexpr (hasRegionVisibility<S>) {
            RegionSettings& regions = settings.m_regions;

            if ((event.key.keysym.mod & KMOD_SHIFT) != 0) {
              regions.clearSeenThrough();
            }
            else if (picked) {
              regions.setSeenThrough(pick.m_primitive, !regions.isSeenThrough(pick.m_primitive));

              picked = false;

              settings.m_highlightPrimitive = Settings<T>::NoPrimitive;
            }
            else {
              changed = pending;
            }

            break;
          }

          // Shift and T make every surface opaque again; T alone toggles the picked surface.
          if ((event.key.keysym.mod & KMOD_SHIFT) != 0) {
            for (uint32_t i = 0; i < Settings<T>::HiddenPrimitives; i++) {
              settings.m_hidden[i] = Settings<T>::NoPrimitive;
            }
          }
          else if (picked) {
            const uint32_t id = pick.m_primitive;

            uint32_t* slot = nullptr;
            bool      was  = false;

            for (uint32_t i = 0; i < Settings<T>::HiddenPrimitives; i++) {
              if (settings.m_hidden[i] == id) {
                settings.m_hidden[i] = Settings<T>::NoPrimitive;
                was                  = true;
              }
              else if (slot == nullptr && settings.m_hidden[i] == Settings<T>::NoPrimitive) {
                slot = &settings.m_hidden[i];
              }
            }

            if (!was && slot == nullptr) {
              std::printf("EBGeometry viewer: at most %u surfaces can be seen through; Shift + T shows them all.\n",
                          unsigned(Settings<T>::HiddenPrimitives));
              std::fflush(stdout);
            }
            else if (!was) {
              *slot = id;

              // What is seen through is no longer picked.
              picked                        = false;
              settings.m_highlightPrimitive = Settings<T>::NoPrimitive;
            }
          }
          else {
            changed = pending;
          }

          break;
        }
        case SDLK_r: {
          // The camera, the light and the picked surfaces, as the window opened with them.
          camera                        = a_camera;
          settings.m_light              = a_settings.m_light;
          settings.m_hidden             = a_settings.m_hidden;
          settings.m_volumePrimitive    = a_settings.m_volumePrimitive;
          settings.m_highlightPrimitive = a_settings.m_highlightPrimitive;
          picked                        = false;

          depthHits.clear();
          depthHidden.clear();

          break;
        }
        default: {
          changed = pending;

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

      const auto start = Clock::now();
      // What was under the cursor may have moved with the camera or the plane; read again on the next move.
      cursorOn = false;

      const Stats stats   = render(shape, deviceShape, stageSettings(0), camera, onDevice, image, nullptr);
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
      next->m_thread =
        std::thread([next, render, shape, deviceShape, stage = stageSettings(pending), camera, onDevice]() {
          const auto start = Clock::now();

          next->m_stats   = render(shape, deviceShape, stage, camera, onDevice, next->m_image, next);
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

  if (panel != nullptr) {
    SDL_DestroyTexture(panel);
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
 * @param[in] a_names       Names of the regions, materials, surfaces and groups, or null.
 */
template <class T, class S>
inline void
start(const S&           a_shape,
      const S*           a_deviceShape,
      const Camera<T>&   a_camera,
      const Settings<T>& a_settings,
      const int          a_argc,
      char**             a_argv,
      const RegionNames* a_names = nullptr)
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

  run(a_shape, a_deviceShape, a_camera, a_settings, a_names);
#else
  (void)a_shape;
  (void)a_deviceShape;
  (void)a_camera;
  (void)a_settings;
  (void)a_names;

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

/**
 * @brief Open the window on a shape that lives in a pool: on the GPU, through a mirror of the pool, in a GPU build with
 * a device and a window to open; on the host otherwise.
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type, with rebasedView(const Pool&).
 * @param[in]     a_shape    The shape, valid on the host.
 * @param[in,out] a_pool     The pool it lives in (frozen when mirrored).
 * @param[in]     a_camera   Initial camera.
 * @param[in]     a_settings Settings.
 * @param[in]     a_argc     Argument count.
 * @param[in]     a_argv     Arguments.
 * @param[in]     a_names    Names of the regions, materials, surfaces and groups, or null.
 */
template <class S>
inline void
startInPool(const S&                           a_shape,
            Pool&                              a_pool,
            const Camera<ShapePrecision<S>>&   a_camera,
            const Settings<ShapePrecision<S>>& a_settings,
            int                                a_argc,
            char**                             a_argv,
            const RegionNames*                 a_names)
{
  using T = ShapePrecision<S>;

#ifdef EBGEOMETRY_VIEWER_SDL2
  // Mirror only when a window will open on a device; the mirror lives as long as the window.
  if (GPU::enabled && GPU::deviceAvailable() && !ViewerDetail::noViewRequested(a_argc, a_argv) &&
      ViewerDetail::displayAvailable()) {
    a_pool.freeze();

    const Pool mirror      = Pool::mirror(a_pool, GPU::memoryResource());
    const S    deviceShape = a_shape.rebasedView(mirror);

    start<T, S>(a_shape, &deviceShape, a_camera, a_settings, a_argc, a_argv, a_names);

    return;
  }
#endif

  (void)a_pool;

  start<T, S>(a_shape, nullptr, a_camera, a_settings, a_argc, a_argv, a_names);
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
  ViewerDetail::startInPool(a_shape, a_pool, a_camera, a_settings, a_argc, a_argv, nullptr);
}

template <class S>
inline void
view(const S&                           a_shape,
     const Camera<ShapePrecision<S>>&   a_camera,
     const Settings<ShapePrecision<S>>& a_settings,
     const RegionNames&                 a_names,
     int                                a_argc,
     char**                             a_argv)
{
  ViewerDetail::start<ShapePrecision<S>, S>(a_shape, nullptr, a_camera, a_settings, a_argc, a_argv, &a_names);
}

template <class S>
inline void
view(const S&                           a_shape,
     Pool&                              a_pool,
     const Camera<ShapePrecision<S>>&   a_camera,
     const Settings<ShapePrecision<S>>& a_settings,
     const RegionNames&                 a_names,
     int                                a_argc,
     char**                             a_argv)
{
  ViewerDetail::startInPool(a_shape, a_pool, a_camera, a_settings, a_argc, a_argv, &a_names);
}

} // namespace EBGeometry::Render

#endif
